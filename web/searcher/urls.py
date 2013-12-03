from django.conf.urls import patterns, include, url
from searcher import views
urlpatterns = patterns('',
                       url(r'^download/(?P<hash>\w+)/', views.download, name ="searcher.views.download"),
                       url(r'', views.search, name ="searcher.views.search"),
                       )
