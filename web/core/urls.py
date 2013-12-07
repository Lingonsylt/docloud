from django.conf.urls import patterns, url, include
from core.views import index
urlpatterns = patterns('',
                       url(r'^org/(?P<org_slug>[\w-]+)/', include("core.urls_tags", namespace="tags")),
                       url(r'^auth/', include("core.urls_auth", namespace="auth")),
                       url(r'^', index.index, name="index"),
                       )
