package controllers

import play.api._
import play.api.mvc._
import org.apache.http.client.fluent.Request
import org.apache.http.entity.ContentType
import play.api.data._
import play.api.data.Forms._
import play.api.libs.json._
import java.io.{FileInputStream, File}
import play.api.libs.Files.TemporaryFile
import org.apache.poi.xwpf.usermodel.XWPFDocument
import org.apache.poi.openxml4j.opc.OPCPackage
import org.apache.poi.xwpf.extractor.XWPFWordExtractor
import play.api.mvc.MultipartFormData.FilePart
import org.apache.http.client.HttpClient
import org.apache.http.impl.client.DefaultHttpClient
import org.apache.http.client.methods.HttpPost
import org.apache.http.entity.mime.MultipartEntity
import org.apache.http.entity.mime.content.{ByteArrayBody}
import org.apache.http.HttpResponse
import org.apache.http.util.EntityUtils
import org.codehaus.jackson.JsonParseException

case class QueryData(query: String)

object Application extends Controller {
  val SOLR_API_URL = "http://localhost:8983/solr/"
  val FILE_STORAGE_DIRECTORY = "C:\\devel\\docloud\\server\\data\\"

  val queryForm = Form(
    mapping(
      "query" -> text
    )(QueryData.apply)(QueryData.unapply)
  )
  
  def index = Action {
    Ok(views.html.index("Your new application is ready."))
  }

  def search_view = Action {
    Ok(views.html.search(queryForm))
  }

  def search = Action { implicit request =>
    queryForm.bindFromRequest.fold(
      formWithErrors => {
        // binding failure, you retrieve the form containing errors:
        BadRequest(views.html.search(formWithErrors))
      },
      queryData => {
        val delete = queryData.query.contains("#delete")
        val modQuery = if (delete) queryData.query.replace("#delete", "") else queryData.query
        val query = if (modQuery == "") "*" else modQuery
        if (delete) {
          Request.Get("http://localhost:8983/solr/update?stream.body=%3Cdelete%3E%3Cquery%3E*:*%3C/query%3E%3C/delete%3E&commit=true")
            .execute()
        }
        val response = Request.Get("http://localhost:8983/solr/collection1/select?wt=json&hl=true&hl.fl=content_txt&fl=id,links_ss&hl.fragsize=200&q=content_txt:" + query)
          .execute().returnContent().asString

        val jsonResponse = Json.parse(response)
        val items = (jsonResponse \ "response" \ "docs").as[List[JsValue]]
        val highlights = (jsonResponse \ "highlighting").as[Map[String,JsValue]]
        val hlItems = items.map { item =>
          val itemMap = item.as[Map[String,JsValue]]
          val hash = itemMap("id").as[String]
          val highlight = (highlights(hash) \ "content_txt").as[List[String]].head
          Json.toJson(itemMap + ("highlight" -> new JsString(highlight)))
        }
        Ok(views.html.search(queryForm.bindFromRequest(), hlItems))
      }
    )
  }

  def index_query = Action(parse.json) { request =>
    val data: Seq[JsValue] = request.body.as[Seq[JsValue]]
    val res : Seq[JsValue] = data.map(
      (element: JsValue) => {
        val hash = (element \ "hash").as[String]
        val result = Json.parse(Request.Get(SOLR_API_URL + "collection1/select?wt=json&q=id:" + hash)
          .execute().returnContent().asString)
        Json.obj("hash" -> hash, "index" -> (if ((result \ "response" \ "numFound").as[Int] == 0) true else false))
      }
    )
    Ok(Json.toJson(res))
  }

  def index_update = Action(parse.multipartFormData) { request =>
    val metadata = Json.parse(request.body.dataParts("metadata").head)
    val hash = (metadata \ "hash").as[String]
    val path = (metadata \ "path").as[String]
    val file : FilePart[TemporaryFile] = request.body.file("file").get
    val fileBytes = org.apache.commons.io.FileUtils.readFileToByteArray(file.ref.file)

    val status = try {
      val response = multipartRequest("update/extract?wt=json&literal.id=" + hash + "&uprefix=attr_&fmap.content=content_txt&literal.links_ss=" + java.net.URLEncoder.encode(path, "UTF-8") + "&commit=true", path, fileBytes)
      (response \ "responseHeader" \ "status").as[Int]
    } catch {
      case ex: JsonParseException => 1
    }
    status match {
      case 0 => Ok(Json.obj("success" -> true))
      case _ => Ok(Json.obj("success" -> false))
    }
  }

  def parseFile(file: File) : Option[String] = {
    try {
      val hdoc = new XWPFDocument(OPCPackage.open(new FileInputStream(file.getAbsolutePath)))
      val extractor = new XWPFWordExtractor(hdoc)
      Some(extractor getText())
    } catch {
      case ex: java.lang.IllegalArgumentException => {
        println(ex + ", " + file.getName)
        None
      }
    }
  }

  def jsonRequest(path : String, data : JsValue) : JsValue = {
    Json.parse(Request.Post(SOLR_API_URL + path)
      .bodyString(data + "", ContentType.APPLICATION_JSON)
      .execute().returnContent().asString)
  }

  def multipartRequest(url: String, filename: String, bytes : Array[Byte]) : JsValue = {
    val client : HttpClient = new DefaultHttpClient()
    val post : HttpPost = new HttpPost(SOLR_API_URL + url)

    val entity : MultipartEntity = new MultipartEntity()
    entity.addPart("file", new ByteArrayBody(bytes, ContentType.APPLICATION_OCTET_STREAM, filename))
    post.setEntity(entity)

    val response : HttpResponse = client.execute(post)
    val responseBody = EntityUtils.toString(response.getEntity)
    try {
      Json.parse(responseBody)
    } catch {
      case ex: JsonParseException => {
        println(responseBody)
        throw ex
      }
    }
  }

  def download(hash: String, filename: String) = Action {
    Ok.sendFile(
      content = new java.io.File(FILE_STORAGE_DIRECTORY + hash),
      fileName = _ => filename
    )
  }
}