from .settings import *
DATABASES = {
    'default': {
        'ENGINE': 'django.db.backends.postgresql_psycopg2',
        'NAME': "docloud",
        'HOST': "localhost",
        'USER': "docloudadmin",
        'PASSWORD': "tensta",
    },
}

STATIC_URL = '/docloud/static/'
