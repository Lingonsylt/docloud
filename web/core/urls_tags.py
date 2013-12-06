from django.conf.urls import patterns, url
from .views import auth, tags
from django.shortcuts import render

urlpatterns = patterns('',
                       url(r'^radera-tagganvandare/(?P<tag_id>\d+)/(?P<user_id>\d+)/', tags.delete_usertag, name ="delete_usertag"),
                       url(r'^radera-tagg/(?P<tag_id>\d+)/', tags.delete_tag, name ="delete_tag"),
                       url(r'', tags.organization, name ="organization"),
                       )

