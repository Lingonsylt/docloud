import java.net.{InetAddress, NetworkInterface}
import java.security.MessageDigest
import java.util.Formatter
import org.apache.commons.io.FileUtils
import java.io._
import org.apache.poi.openxml4j.opc.OPCPackage
import org.apache.poi.xwpf.extractor.XWPFWordExtractor
import org.apache.poi.xwpf.usermodel.XWPFDocument
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
  }

  def jsonRequest(path : String, data : JsValue) : JsValue = {
    println("dataJsValue: " + data.prettyPrint)
    //Request.Post(API_URL + path)
    Request.Post("http://localhost:8983/solr/update/json?commit=true")
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
    jsonRequest("check", files.map(
      (file : Map[String, Any]) => {
        (file("hash"), file("path")) match {
          case (hash: String, path: String) => {
            Map("id" -> hash,
                "path_s" -> path)
          }
        }
      }
    ).toJson)
  }

  def indexFile(file: File) {
    //println(file.getAbsolutePath)
    try {
      val hdoc = new XWPFDocument(OPCPackage.open(new FileInputStream(file.getAbsolutePath)))
      val extractor = new XWPFWordExtractor(hdoc)

      val text = extractor getText()
      val json = List(Map("id" -> file.getName,
                          "content_txt" -> text))

      //println(json.toJson)

      //println(
      Request.Post("http://localhost:8983/solr/update/json?commit=true")
        .bodyString(json.toJson + "", ContentType.APPLICATION_JSON)
        .execute().returnContent()
    } catch {
      case ex: java.lang.IllegalArgumentException => {
        println(ex + ", " + file.getName)
      }
    }
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
