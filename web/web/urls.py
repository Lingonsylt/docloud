from django.conf.urls import patterns, include, url
from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('',
     url(r'^sok/', include("searcher.urls", namespace = "searcher")),
     url(r'^index/', include("indexer.urls", namespace = "indexer")),
     url(r'^admin/', include(admin.site.urls)),
     url(r'', include("core.urls")),
)
