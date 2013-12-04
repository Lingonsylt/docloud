from django.conf.urls import patterns, include, url
from indexer import views
urlpatterns = patterns('',
                       url(r'^query/', views.index_query, name = "views.search"),
                       url(r'^update/', views.index_update, name = "views.search"),
                       )
