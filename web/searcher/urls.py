from django.conf.urls import patterns, url
from searcher import views
urlpatterns = patterns('',
                       url(r'^ladda-ner/(?P<hash_>\w+)/', views.download, name ="download"),
                       url(r'', views.search, name ="search"),
                       )
