package controllers

import play.api._
import play.api.mvc._
import org.apache.http.client.fluent.Request
import org.apache.http.entity.ContentType
import play.data.DynamicForm
import play.api.data._
import play.api.data.Forms._
import play.api.libs.json._
import java.io.{FileInputStream, File}
import play.api.libs.Files.TemporaryFile
import org.apache.poi.xwpf.usermodel.XWPFDocument
import org.apache.poi.openxml4j.opc.OPCPackage
import org.apache.poi.xwpf.extractor.XWPFWordExtractor
import play.api.mvc.MultipartFormData.FilePart

case class QueryData(query: String)

object Application extends Controller {
  val SOLR_API_URL = "http://localhost:8983/solr/"

  val queryForm = Form(
    mapping(
      "query" -> text
    )(QueryData.apply)(QueryData.unapply)
  )
  
  def index = Action {
    Ok(views.html.index("Your new application is ready."))
  }

  def search_view = Action {
    Ok(views.html.search(queryForm, ""))
  }

  def search = Action { implicit request =>
    queryForm.bindFromRequest.fold(
      formWithErrors => {
        // binding failure, you retrieve the form containing errors:
        BadRequest(views.html.search(formWithErrors, ""))
      },
      queryData => {
        val delete = queryData.query.contains("#delete")
        val modQuery = if (delete) queryData.query.replace("#delete", "") else queryData.query
        val query = if (modQuery == "") "*" else modQuery
        if (delete) {
          Request.Get("http://localhost:8983/solr/update?stream.body=%3Cdelete%3E%3Cquery%3E*:*%3C/query%3E%3C/delete%3E&commit=true")
            .execute()
        }
        val response = Request.Get("http://localhost:8983/solr/collection1/select?wt=json&q=content_txt:" + query)
          .execute().returnContent()
        Ok(views.html.search(queryForm.bindFromRequest(), response + ""))
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
    Ok(Json.toJson(request.body.files.map(
      (file: FilePart[TemporaryFile]) => {
        file.filename
        val parsed = parseFile(file.ref.file)
        if (parsed.isDefined) {
          val solrRequest = Json.toJson(Seq(Json.obj("id" -> file.filename,
            "content_txt" -> parsed.get)))
          jsonRequest("update/json?commit=true", solrRequest)
          Json.obj("success" -> true, "hash" -> file.filename)
        } else {
          Json.obj("success" -> false, "hash" -> file.filename)
        }
      }
    )))
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
}