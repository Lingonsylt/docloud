from functools import reduce
import json
import os
from django.conf import settings
from django.contrib.auth.decorators import login_required
from django.db import transaction
from django.db.models import Q
from django.http import HttpResponse
from django.views.decorators.csrf import csrf_exempt
from core.models import FileInfo, Tag
from web import solr
import logging
logger = logging.getLogger("django.request")

@csrf_exempt
@login_required
def index_query(request):
    json_request = json.loads(request.body.decode("UTF-8"))
    result = {"success":True, "results" :[]}
    for file_index_request in json_request:
        tags = Tag.objects.filter(users__id=request.loggedin().id).\
            filter(reduce(lambda accum, tag_id: accum | Q(pk = int(tag_id)), file_index_request["tags"], Q()))

        with transaction.atomic():
            file_info, created = FileInfo.objects.get_or_create(hash=file_index_request["hash"],
                                               installation=request.loggedin().active_installation,
                                               path=file_index_request["path"])

            if created or list(file_info.tags.all()) != list(tags):
                file_info.tags = tags
                file_info.save()
                _update_solr(file_index_request["hash"], request.loggedin())

        query_response = solr.get(file_index_request["hash"], {"fl":["parsed_b"]})
        doc = query_response.get("doc", None)
        index = True
        if doc is not None:
            index = not doc.get("parsed_b", False)

        result["results"].append({"hash": file_index_request["hash"],
                        "index": index})
    return HttpResponse(json.dumps(result), content_type="application/json")

@csrf_exempt
@login_required
def index_delete(request):
    result = {"success":True, "results" :[]}
    json_request = json.loads(request.body.decode("UTF-8"))
    for file_delete_request in json_request:
        FileInfo.objects.filter(hash=file_delete_request["hash"],
                        installation=request.loggedin().active_installation,
                        path=file_delete_request["path"]).delete()
        _update_solr(file_delete_request["hash"], request.loggedin())
        result["results"].append({"hash":file_delete_request["hash"], "success":True})
    return HttpResponse(json.dumps(result), content_type="application/json")

def _update_solr(hash_, user):
    paths = []
    tags = []
    for file_info in FileInfo.objects.filter(hash=hash_):
        if file_info.path not in paths:
            paths.append(file_info.path)
        for tag in file_info.tags.all():
            if tag.id not in tags:
                tags.append(tag.id)

    if paths and tags:
        request_data = [{"id":hash_,
                         "links_ss":{"set": paths},
                         "tags_is":{"set": tags}}]
        response = solr.update(request_data)
        if response["responseHeader"]["status"] != 0:
            logger.warning("error when creating document (%s, %s): %s" % (user, hash_, response))
            logger.debug("create request data: %s" % request_data)
    else:
        response = solr.delete(hash_)
        if response["responseHeader"]["status"] != 0:
            logger.warning("error when deleting document (%s, %s): %s" % (user, hash_, response))

@csrf_exempt
@login_required
def index_update(request):
    metadata = json.loads(request.POST.get("metadata"))

    with open(os.path.join(settings.DATA_STORAGE_DIR, metadata["hash"]), "wb") as f:
        f.write(request.FILES["file"].read())

    request.FILES["file"].seek(0)
    extract_result = solr.extract({"literal.parsed_b": "true",
                                   "literal.id": metadata["hash"],
                                   "uprefix": "attr_",
                                   "fmap.content": "content_txt"},
                                request.FILES["file"])

    _update_solr(metadata["hash"], request.loggedin())

    result = {"success": extract_result["responseHeader"]["status"] == 0}
    return HttpResponse(json.dumps(result), content_type="application/json")

@login_required
def tags(request):
    return HttpResponse(json.dumps([{"name":tag.name, "id":tag.id} for tag in request.loggedin().tags.all()]), content_type="application/json")