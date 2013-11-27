package controllers

import play.api._
import play.api.mvc._
import org.apache.http.client.fluent.Request
import org.apache.http.entity.ContentType
import play.data.DynamicForm
import play.api.data._
import play.api.data.Forms._
import play.api.libs.json.JsValue

case class QueryData(query: String)

object Application extends Controller {
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
        val response = Request.Get("http://localhost:8983/solr/collection1/select?wt=json&q=content_txt:" + queryData.query)
          .execute().returnContent()
        Ok(views.html.search(queryForm.bindFromRequest(), response + ""))
      }
    )
  }

  def index_query = Action { request =>
    val body: AnyContent = request.body
    val jsonData: Option[JsValue] = body.asJson
    Ok(views.html.index("Your new application is ready."))
  }
}