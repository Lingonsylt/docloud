from django.conf.urls import patterns, include, url
from indexer import views
urlpatterns = patterns('',
                       url(r'^query/', views.index_query, name = "search"),
                       url(r'^update/', views.index_update, name = "update"),
                       url(r'^tags/', views.tags, name = "tags"),
                       )
