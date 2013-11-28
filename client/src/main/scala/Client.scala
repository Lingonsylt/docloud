import java.net.{InetAddress, NetworkInterface}
import java.security.MessageDigest
import java.util.Formatter
import org.apache.commons.io.FileUtils
import java.io._
import org.apache.http.client.HttpClient
import org.apache.http.client.methods.HttpPost
import org.apache.http.entity.mime.content.ByteArrayBody
import org.apache.http.HttpResponse
import org.apache.http.impl.client.DefaultHttpClient
import org.apache.http.entity.mime.MultipartEntity
import org.apache.http.util.EntityUtils
import scala.collection.JavaConverters._
import spray.json._
import DefaultJsonProtocol._
import org.apache.http.client.fluent.Request
import org.apache.http.entity.ContentType

object Client {
  val searchPaths = List("C:\\Users\\lingon\\Desktop")
  val fileExtentions = List("docx")
  val mac = getMAC
  val API_URL = "http://localhost:9000/"

  def main(args: Array[String]) {
    //println("Hello!")
    val result = searchPaths.map(
      (path: String) =>
        FileUtils.iterateFiles(new File(path), fileExtentions.toArray, true).asScala.toList.map(
          (file: Any) =>
            file match {
              case file2: File =>
                getFileInfo(file2)
              case _ => throw new ClassCastException
            }
        )
    ).flatten

    val queryResult = queryFiles(result)
    println("queryResult: " + queryResult.prettyPrint)

    val qr : Seq[JsValue] = queryResult.convertTo[Seq[JsValue]]
    qr.map(
      (element: JsValue) => {
        val index : Boolean = element.asJsObject.fields("index").convertTo[Boolean]
        if (index) {
          val str = element.asJsObject.fields("hash").convertTo[String]
          result.map(
            (file : Map[String, Any]) => {
              val hash : String = file("hash").asInstanceOf[String]
              if(str == hash) {
                indexFile(file)
              }
            }
          )
        }
      }
    )
  }

  def jsonRequest(path : String, data : JsValue) : JsValue = {
    println("dataJsValue: " + data.prettyPrint)
    Request.Post(API_URL + path)
      .bodyString(data + "", ContentType.APPLICATION_JSON)
      .execute().returnContent().asString.asJson
  }

  def getFileInfo(file: File) : Map[String, Any] = {
    val bis = new BufferedInputStream(new FileInputStream(file))
    val fileBytes = Stream.continually(bis.read).takeWhile(-1 !=).map(_.toByte).toArray
    Map("hash" -> getHash(fileBytes),
        "path" -> file.getAbsolutePath,
        "bytes" -> fileBytes)
  }

  def queryFiles(files : List[Map[String, Any]]) : JsValue = {
    jsonRequest("index/query", files.map(
      (file : Map[String, Any]) => {
        (file("hash"), file("path")) match {
          case (hash: String, path: String) => {
            Map("hash" -> hash,
                "path" -> path)
          }
        }
      }
    ).toJson)
  }

  def indexFile(file: Map[String, Any]) {
    val bytes : Array[Byte] = file("bytes").asInstanceOf[Array[Byte]]
    val hash : String = file("hash").asInstanceOf[String]
    val client : HttpClient = new DefaultHttpClient()
    val post : HttpPost = new HttpPost(API_URL + "index/update")

    val entity : MultipartEntity = new MultipartEntity()
    entity.addPart("file", new ByteArrayBody(bytes, ContentType.APPLICATION_OCTET_STREAM, hash))
    post.setEntity(entity)

    val response : HttpResponse = client.execute(post)

    val responseJson = EntityUtils.toString(response.getEntity).asJson
    println(responseJson.prettyPrint)
  }

  def getMAC : String = {
    val localNetworkInterface = NetworkInterface.getByInetAddress(InetAddress.getLocalHost)
    localNetworkInterface.getHardwareAddress.toList.map(b => String.format("%02x",b.asInstanceOf[AnyRef])).mkString(":")
  }

  def getHash(fileContent: Array[Byte]) : String = {
    val formatter = new Formatter()
    MessageDigest.getInstance("SHA1").digest(fileContent).map(
      (b: Byte) => formatter.format("%02x", b.asInstanceOf[AnyRef])
    )
    formatter.toString
  }
}
