from django.conf.urls import patterns, include, url
from . import views_manage
from django.shortcuts import render

urlpatterns = patterns('',
                       url(r'^registrera/', views_manage.RegisterView.as_view(),
                           name ="register"),
                       url(r'^inloggning/(?P<token>\w+)/', views_manage.token_login, name ="token_login"),
                       url(r'^inloggning/', lambda r: render(r, "manage/login.html", {"TITLE":"Ej inloggad"}), name ="login"),
                       url(r'^org/(?P<org_slug>[\w-]+)/', views_manage.organization, name ="organization"),
                       url(r'^ladda-ner-inloggningslank/(?P<token>\w+)/', views_manage.link_download, name ="download_token_link"),
                       )
