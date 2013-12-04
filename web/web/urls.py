from django.conf.urls import patterns, include, url
from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('',
     url(r'^search/', include("searcher.urls", namespace = "searcher")),
     url(r'^index/', include("indexer.urls", namespace = "indexer")),
     url(r'^manage/', include("core.urls_manage", namespace = "manage")),

    url(r'^admin/', include(admin.site.urls)),
)
