import json
import os
from django.conf import settings
from django.http import HttpResponse
from django.views.decorators.csrf import csrf_exempt
from web import solr

@csrf_exempt
def index_query(request):
    json_request = json.loads(request.body.decode("UTF-8"))
    results = []
    for file_index_request in json_request:
        query_response = solr.get(file_index_request["hash"], {"fl":["links_ss", "parsed_b"]})
        doc = query_response.get("doc", None)
        if doc is None:
            create_response = solr.update([{"id":file_index_request["hash"],
                                            "links_ss":[file_index_request["path"]]}])
            results.append({"hash": file_index_request["hash"],
                            "index": True})
        else:
            if file_index_request["path"] not in doc["links_ss"]:
                update_response = solr.update([{"id":file_index_request["hash"],
                                                "links_ss":{"add": file_index_request["path"]}}])
            results.append({"hash": file_index_request["hash"],
                            "index": not doc.get("parsed_b", False)})
    return HttpResponse(json.dumps(results), content_type="application/json")

@csrf_exempt
def index_update(request):
    metadata = json.loads(request.POST.get("metadata"))
    current_doc = solr.get(metadata["hash"], {"fl":"links_ss"})["doc"]

    with open(os.path.join(settings.DATA_STORAGE_DIR, metadata["hash"]), "wb") as f:
        f.write(request.FILES["file"].read())

    request.FILES["file"].seek(0)
    extract_result = solr.extract({"literal.parsed_b": "true",
                                   "literal.id": metadata["hash"],
                                   "uprefix": "attr_",
                                   "literal.links_ss": current_doc["links_ss"],
                                   "fmap.content": "content_txt"},
                                request.FILES["file"])

    result = {"success": extract_result["responseHeader"]["status"] == 0}
    return HttpResponse(json.dumps(result), content_type="application/json")