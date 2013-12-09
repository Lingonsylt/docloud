from functools import reduce
from django.conf import settings
from django.contrib.auth.decorators import login_required
from django.shortcuts import render
from django.http import HttpResponse
import os
from core.models import Tag
from web import solr
import logging
logger = logging.getLogger("django.request")
from django.db.models import Q

@login_required
def search(request):
    query = request.GET.get('q', "")
    delete = query.find("#delete") != -1
    if delete:
        query = query.replace("#delete", "")

    docs = []
    allowed_tag_ids = [tag.id for tag in request.loggedin().tags.all()]
    if query is not "" and allowed_tag_ids:
        query_params = {"hl":"true",
                        "hl.fl":"content_txt",
                        "fl":["id", "links_ss", "tags_is"],
                        "hl.fragsize" :"200",
                        "fq": "tags_is:(%s)" % " OR ".join(map(str, allowed_tag_ids))}
        q = "content_txt:%s" % query
        query_response = solr.query(q, query_params)
        if query_response["responseHeader"]["status"] != 0:
            logger.error("query (q: %s, params: %s) resulted in error: %s" % (q, query_params, query_response))
        else:
            highlights = query_response["highlighting"]
            docs = query_response["response"]["docs"]
            for doc in docs:
                doc["highlight"] = highlights[doc["id"]]["content_txt"][0]
                doc['links'] = []
                for link in doc["links_ss"]:
                    separator = "\\" if link.find("\\") != -1 else "/"
                    doc['links'].append({"path": link[:link.rindex(separator)+1],
                                         "filename" : link[link.rindex(separator)+1:]})

    if delete:
        solr.deleteAll()

    for doc in docs:
        doc["tags"] = Tag.objects.filter(reduce(lambda accum, item: accum | Q(pk = item), doc["tags_is"], Q()))

    return render(request, "searcher/search.html", {'query': query, 'docs': docs})

@login_required
def download(request, hash_):
    # TODO: Check authorization for the file
    f = open(os.path.join(settings.DATA_STORAGE_DIR, hash_), "rb")
    response = HttpResponse(f, content_type="application/octet-stream")
    response['Content-Disposition'] = 'attachment; filename="%s"' % request.GET["filename"]
    return response