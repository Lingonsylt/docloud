from django.conf.urls import patterns, url
from django.contrib.auth import logout
from .views import auth
from django.shortcuts import redirect

urlpatterns = patterns('',
                       url(r'^inloggning/logga-ut/', lambda r: [logout(r), redirect("index")][1], name ="logout"),
                       url(r'^inloggning/registrera/', auth.auth_register, name ="auth_register"),
                       url(r'^inloggning/login/', auth.auth_login, name ="auth_login"),
                       url(r'^inloggning/(?P<token>\w+)/', auth.token_login, name ="token_login"),
                       url(r'^inloggning/', auth.auth, name ="auth"),
                       url(r'^ladda-ner-inloggningslank/(?P<token>\w+)/', auth.link_download, name ="download_token_link"),
                       url(r'^profil/andra-losenord/', auth.profile_change_password, name ="profile_change_password"),
                       url(r'^profil/andra-email/', auth.profile_change_email, name ="profile_change_email"),
                       url(r'^profil/', auth.profile, name ="profile"),
                       )
