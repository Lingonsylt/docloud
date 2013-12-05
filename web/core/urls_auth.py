from django.conf.urls import patterns, url
from .views import auth
from django.shortcuts import render

urlpatterns = patterns('',
                       url(r'^registrera/', auth.RegisterView.as_view(),
                           name ="register"),
                       url(r'^inloggning/(?P<token>\w+)/', auth.token_login, name ="token_login"),
                       url(r'^inloggning/', lambda r: render(r, "auth/login.html", {"TITLE":"Ej inloggad"}), name ="login"),
                       url(r'^ladda-ner-inloggningslank/(?P<token>\w+)/', auth.link_download, name ="download_token_link"),
                       )
