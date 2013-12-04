from django.conf.urls import patterns, url
from searcher import views
urlpatterns = patterns('',
                       url(r'^ladda-ner/(?P<hash>\w+)/', views.download, name ="views.download"),
                       url(r'', views.search, name ="views.search"),
                       )
