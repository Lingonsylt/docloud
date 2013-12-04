import uuid
from django.db import models

# Create your models here.
class Organization(models.Model):
    name = models.CharField(max_length=512, required=True)

    def __str__(self):
        return self.name

class Tag(models.Model):
    name = models.CharField(max_length=512, required=True)
    organization = models.ForeignKey(Organization, required=True)

    def __str__(self):
        return self.name

class User(models.Model):
    name = models.CharField(max_length=128)
    email = models.EmailField(required=True)
    organization = models.ForeignKey(Organization, required=True)
    owner = models.BooleanField(default=False)
    tag_creator = models.BooleanField(default=False)
    tags = models.ManyToManyField(Tag, through='UserTag')

    def __str__(self):
        return self.name

class UserTag(models.Model):
    tag = models.ForeignKey(Tag, required=True)
    user = models.ForeignKey(User, required=True)
    owns_tag = models.BooleanField(default=False)

    def __str__(self):
        return "%s (owns: %s) %s" % (self.user.name, self.owns_tag, self.tag.name)

class Installation(models.Model):
    uuid = models.CharField(primary_key=True, max_length=32, default=lambda: uuid.uuid1().hex)
    user = models.ForeignKey(User, required=True)

    def __str__(self):
        return "%s inst: %s" % (self.user.name, self.uuid)