import uuid
from django.conf import settings
from django.db import models
from django.contrib.auth.models import Group, Permission
from django.contrib.contenttypes.models import ContentType

# Create your models here.
from django.utils.text import slugify

class Organization(models.Model):
    slug = models.CharField(max_length=512, unique=True)
    name = models.CharField(max_length=512)

    def save(self, *args, **kwargs):
        self.slug = slugify(self.name)
        super().save(*args, **kwargs)

    def __str__(self):
        return self.name

class Tag(models.Model):
    name = models.CharField(max_length=512)
    organization = models.ForeignKey(Organization)

    def __str__(self):
        return self.name

class User(models.Model):
    name = models.CharField(max_length=128, blank=True)
    email = models.EmailField()
    organization = models.ForeignKey(Organization)
    owner = models.BooleanField(default=False)
    tag_creator = models.BooleanField(default=False)
    tags = models.ManyToManyField(Tag, through='UserTag')
    auth_user = models.ForeignKey(settings.AUTH_USER_MODEL, related_name="docloud_users")

    def __str__(self):
        return self.email

class UserTag(models.Model):
    tag = models.ForeignKey(Tag)
    user = models.ForeignKey(User)
    owns_tag = models.BooleanField(default=False)

    def __str__(self):
        return "%s (owns: %s) %s" % (self.user.name, self.owns_tag, self.tag.name)

class Installation(models.Model):
    uuid = models.CharField(primary_key=True, max_length=32, default=lambda: uuid.uuid1().hex)
    user = models.ForeignKey(User, related_name="installations")

    def __str__(self):
        return "%s inst: %s" % (self.user.name, self.uuid)

class UUIDLink(models.Model):
    uuid = models.CharField(primary_key=True, max_length=32, default=lambda: uuid.uuid1().hex)
    data = models.TextField()

    class JoinOrganization:
        def __init__(self, organization_id):
            pass

content_type = ContentType.objects.get_for_model(User)
is_customer_permission, created = Permission.objects.get_or_create(codename='is_customer',
                                                          name='Is customer',
                                                          content_type=content_type)
customer_group, created = Group.objects.get_or_create(name = "customer")
customer_group.permissions=[is_customer_permission]
customer_group.save()
