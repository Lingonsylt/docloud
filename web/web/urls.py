from django.conf.urls import patterns, include, url
from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('',
     url(r'^search/', include("searcher.urls")),
     url(r'^index/', include("indexer.urls")),

    url(r'^admin/', include(admin.site.urls)),
)
