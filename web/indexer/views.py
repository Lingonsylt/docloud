import json
import os
from django.conf import settings
from django.contrib.auth.decorators import login_required
from django.http import HttpResponse
from django.views.decorators.csrf import csrf_exempt
from web import solr
import logging
logger = logging.getLogger("django.request")

@csrf_exempt
@login_required
def index_query(request):
    json_request = json.loads(request.body.decode("UTF-8"))
    result = {"success":True, "results" :[]}
    allowed_tag_ids = [tag.id for tag in request.loggedin().tags.all()]
    for file_index_request in json_request:
        for tag_id in file_index_request["tags"]:
            if tag_id not in allowed_tag_ids:
                logger.info("%s sent disallowed tagid: %s, skipping file: %s (allowed: %s)" % (request.loggedin(), tag_id, file_index_request, allowed_tag_ids))
                continue
        query_response = solr.get(file_index_request["hash"], {"fl":["links_ss", "parsed_b", "tags_is"]})
        doc = query_response.get("doc", None)
        if doc is None:
            request_data = [{"id":file_index_request["hash"],
                             "links_ss":[file_index_request["path"]],
                             "tags_is":file_index_request["tags"]}]
            create_response = solr.update(request_data)
            if create_response["responseHeader"]["status"] != 0:
                logger.warning("error when creating document (%s, %s): %s" % (request.loggedin(), file_index_request, create_response))
                logger.debug("create request data: %s" % request_data)
                result["success"] = False
                break

            result["results"].append({"hash": file_index_request["hash"],
                            "index": True})
        else:
            request_documents = []
            if file_index_request["path"] not in doc["links_ss"]:
                request_documents.append({"id":file_index_request["hash"], "links_ss": {"add": file_index_request["path"]}})

            for tag_id in file_index_request["tags"]:
                if "tags_is" not in doc or tag_id not in doc["tags_is"]:
                    request_documents.append({"id": file_index_request["hash"], "tags_is": {"add": tag_id}})

            if request_documents:
                logger.info("updating: %s" % request_documents)
                update_response = solr.update(request_documents)
                if update_response["responseHeader"]["status"] != 0:
                    logger.warning("error when updating document (%s, %s): %s" % (request.loggedin(), file_index_request, update_response))
                    logger.debug("update request data: %s" % request_documents)
                    result["success"] = False
                    break

            result["results"].append({"hash": file_index_request["hash"],
                            "index": not doc.get("parsed_b", False)})
    return HttpResponse(json.dumps(result), content_type="application/json")

@csrf_exempt
def index_update(request):
    metadata = json.loads(request.POST.get("metadata"))
    current_doc = solr.get(metadata["hash"], {"fl":["links_ss", "tags_is"]})["doc"]

    with open(os.path.join(settings.DATA_STORAGE_DIR, metadata["hash"]), "wb") as f:
        f.write(request.FILES["file"].read())

    request.FILES["file"].seek(0)
    extract_result = solr.extract({"literal.parsed_b": "true",
                                   "literal.id": metadata["hash"],
                                   "uprefix": "attr_",
                                   "literal.links_ss": current_doc["links_ss"],
                                   "literal.tags_is": current_doc["tags_is"],
                                   "fmap.content": "content_txt"},
                                request.FILES["file"])

    result = {"success": extract_result["responseHeader"]["status"] == 0}
    return HttpResponse(json.dumps(result), content_type="application/json")

@login_required
def tags(request):
    return HttpResponse(json.dumps([{"name":tag.name, "id":tag.id} for tag in request.loggedin().tags.all()]), content_type="application/json")