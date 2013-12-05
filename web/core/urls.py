from django.conf.urls import patterns, url, include

urlpatterns = patterns('',
                       url(r'^org/(?P<org_slug>[\w-]+)/', include("core.urls_tags", namespace="tags")),
                       url(r'^auth/', include("core.urls_auth", namespace="auth")),
                       )
