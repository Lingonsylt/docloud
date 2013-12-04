# Register your models here.
from django.contrib import admin
from .models import Organization, User, Tag, UserTag

class TagInline(admin.StackedInline):
    model = UserTag
    extra = 3

class UserAdmin(admin.ModelAdmin):
    #fieldsets = [
    #    (None,               {'fields': ['question']}),
    #    ('Date information', {'fields': ['pub_date'], 'classes': ['collapse']}),
    #    ]
    inlines = [TagInline]
admin.site.register(User, UserAdmin)

admin.site.register(Organization)
admin.site.register(Tag)
