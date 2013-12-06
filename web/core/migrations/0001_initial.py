# -*- coding: utf-8 -*-
from south.utils import datetime_utils as datetime
from south.db import db
from south.v2 import SchemaMigration
from django.db import models


class Migration(SchemaMigration):

    def forwards(self, orm):
        # Adding model 'Organization'
        db.create_table('core_organization', (
            ('id', self.gf('django.db.models.fields.AutoField')(primary_key=True)),
            ('slug', self.gf('django.db.models.fields.CharField')(unique=True, max_length=512)),
            ('name', self.gf('django.db.models.fields.CharField')(max_length=512)),
        ))
        db.send_create_signal('core', ['Organization'])

        # Adding model 'Tag'
        db.create_table('core_tag', (
            ('id', self.gf('django.db.models.fields.AutoField')(primary_key=True)),
            ('name', self.gf('django.db.models.fields.CharField')(max_length=512)),
            ('organization', self.gf('django.db.models.fields.related.ForeignKey')(related_name='tags', to=orm['core.Organization'])),
        ))
        db.send_create_signal('core', ['Tag'])

        # Adding model 'User'
        db.create_table('core_user', (
            ('id', self.gf('django.db.models.fields.AutoField')(primary_key=True)),
            ('name', self.gf('django.db.models.fields.CharField')(max_length=128, blank=True)),
            ('organization', self.gf('django.db.models.fields.related.ForeignKey')(related_name='users', to=orm['core.Organization'])),
            ('email', self.gf('django.db.models.fields.EmailField')(max_length=75)),
            ('owner', self.gf('django.db.models.fields.BooleanField')(default=False)),
            ('tag_creator', self.gf('django.db.models.fields.BooleanField')(default=False)),
            ('auth_user', self.gf('django.db.models.fields.related.ForeignKey')(related_name='docloud_users', to=orm['auth.User'])),
        ))
        db.send_create_signal('core', ['User'])

        # Adding model 'UserTag'
        db.create_table('core_usertag', (
            ('id', self.gf('django.db.models.fields.AutoField')(primary_key=True)),
            ('tag', self.gf('django.db.models.fields.related.ForeignKey')(to=orm['core.Tag'])),
            ('user', self.gf('django.db.models.fields.related.ForeignKey')(to=orm['core.User'])),
            ('owns_tag', self.gf('django.db.models.fields.BooleanField')(default=False)),
        ))
        db.send_create_signal('core', ['UserTag'])

        # Adding model 'Installation'
        db.create_table('core_installation', (
            ('uuid', self.gf('django.db.models.fields.CharField')(default='3b95a38a5de111e38c5390e6ba0d52ef', primary_key=True, max_length=32)),
            ('user', self.gf('django.db.models.fields.related.ForeignKey')(related_name='installations', to=orm['core.User'])),
        ))
        db.send_create_signal('core', ['Installation'])

        # Adding model 'UUIDLink'
        db.create_table('core_uuidlink', (
            ('uuid', self.gf('django.db.models.fields.CharField')(default='3b95ca9a5de111e391e890e6ba0d52ef', primary_key=True, max_length=32)),
            ('data', self.gf('django.db.models.fields.TextField')()),
        ))
        db.send_create_signal('core', ['UUIDLink'])


    def backwards(self, orm):
        # Deleting model 'Organization'
        db.delete_table('core_organization')

        # Deleting model 'Tag'
        db.delete_table('core_tag')

        # Deleting model 'User'
        db.delete_table('core_user')

        # Deleting model 'UserTag'
        db.delete_table('core_usertag')

        # Deleting model 'Installation'
        db.delete_table('core_installation')

        # Deleting model 'UUIDLink'
        db.delete_table('core_uuidlink')


    models = {
        'auth.group': {
            'Meta': {'object_name': 'Group'},
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'name': ('django.db.models.fields.CharField', [], {'unique': 'True', 'max_length': '80'}),
            'permissions': ('django.db.models.fields.related.ManyToManyField', [], {'symmetrical': 'False', 'to': "orm['auth.Permission']", 'blank': 'True'})
        },
        'auth.permission': {
            'Meta': {'ordering': "('content_type__app_label', 'content_type__model', 'codename')", 'unique_together': "(('content_type', 'codename'),)", 'object_name': 'Permission'},
            'codename': ('django.db.models.fields.CharField', [], {'max_length': '100'}),
            'content_type': ('django.db.models.fields.related.ForeignKey', [], {'to': "orm['contenttypes.ContentType']"}),
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'name': ('django.db.models.fields.CharField', [], {'max_length': '50'})
        },
        'auth.user': {
            'Meta': {'object_name': 'User'},
            'date_joined': ('django.db.models.fields.DateTimeField', [], {'default': 'datetime.datetime.now'}),
            'email': ('django.db.models.fields.EmailField', [], {'max_length': '75', 'blank': 'True'}),
            'first_name': ('django.db.models.fields.CharField', [], {'max_length': '30', 'blank': 'True'}),
            'groups': ('django.db.models.fields.related.ManyToManyField', [], {'related_name': "'user_set'", 'blank': 'True', 'to': "orm['auth.Group']", 'symmetrical': 'False'}),
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'is_active': ('django.db.models.fields.BooleanField', [], {'default': 'True'}),
            'is_staff': ('django.db.models.fields.BooleanField', [], {'default': 'False'}),
            'is_superuser': ('django.db.models.fields.BooleanField', [], {'default': 'False'}),
            'last_login': ('django.db.models.fields.DateTimeField', [], {'default': 'datetime.datetime.now'}),
            'last_name': ('django.db.models.fields.CharField', [], {'max_length': '30', 'blank': 'True'}),
            'password': ('django.db.models.fields.CharField', [], {'max_length': '128'}),
            'user_permissions': ('django.db.models.fields.related.ManyToManyField', [], {'related_name': "'user_set'", 'blank': 'True', 'to': "orm['auth.Permission']", 'symmetrical': 'False'}),
            'username': ('django.db.models.fields.CharField', [], {'unique': 'True', 'max_length': '30'})
        },
        'contenttypes.contenttype': {
            'Meta': {'ordering': "('name',)", 'unique_together': "(('app_label', 'model'),)", 'object_name': 'ContentType', 'db_table': "'django_content_type'"},
            'app_label': ('django.db.models.fields.CharField', [], {'max_length': '100'}),
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'model': ('django.db.models.fields.CharField', [], {'max_length': '100'}),
            'name': ('django.db.models.fields.CharField', [], {'max_length': '100'})
        },
        'core.installation': {
            'Meta': {'object_name': 'Installation'},
            'user': ('django.db.models.fields.related.ForeignKey', [], {'related_name': "'installations'", 'to': "orm['core.User']"}),
            'uuid': ('django.db.models.fields.CharField', [], {'default': "'3b99c2505de111e38c7390e6ba0d52ef'", 'primary_key': 'True', 'max_length': '32'})
        },
        'core.organization': {
            'Meta': {'object_name': 'Organization'},
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'name': ('django.db.models.fields.CharField', [], {'max_length': '512'}),
            'slug': ('django.db.models.fields.CharField', [], {'unique': 'True', 'max_length': '512'})
        },
        'core.tag': {
            'Meta': {'object_name': 'Tag'},
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'name': ('django.db.models.fields.CharField', [], {'max_length': '512'}),
            'organization': ('django.db.models.fields.related.ForeignKey', [], {'related_name': "'tags'", 'to': "orm['core.Organization']"}),
            'users': ('django.db.models.fields.related.ManyToManyField', [], {'symmetrical': 'False', 'through': "orm['core.UserTag']", 'to': "orm['core.User']"})
        },
        'core.user': {
            'Meta': {'object_name': 'User'},
            'auth_user': ('django.db.models.fields.related.ForeignKey', [], {'related_name': "'docloud_users'", 'to': "orm['auth.User']"}),
            'email': ('django.db.models.fields.EmailField', [], {'max_length': '75'}),
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'name': ('django.db.models.fields.CharField', [], {'max_length': '128', 'blank': 'True'}),
            'organization': ('django.db.models.fields.related.ForeignKey', [], {'related_name': "'users'", 'to': "orm['core.Organization']"}),
            'owner': ('django.db.models.fields.BooleanField', [], {'default': 'False'}),
            'tag_creator': ('django.db.models.fields.BooleanField', [], {'default': 'False'}),
            'tags': ('django.db.models.fields.related.ManyToManyField', [], {'symmetrical': 'False', 'through': "orm['core.UserTag']", 'to': "orm['core.Tag']"})
        },
        'core.usertag': {
            'Meta': {'object_name': 'UserTag'},
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'owns_tag': ('django.db.models.fields.BooleanField', [], {'default': 'False'}),
            'tag': ('django.db.models.fields.related.ForeignKey', [], {'to': "orm['core.Tag']"}),
            'user': ('django.db.models.fields.related.ForeignKey', [], {'to': "orm['core.User']"})
        },
        'core.uuidlink': {
            'Meta': {'object_name': 'UUIDLink'},
            'data': ('django.db.models.fields.TextField', [], {}),
            'uuid': ('django.db.models.fields.CharField', [], {'default': "'3b99c2515de111e3a2c290e6ba0d52ef'", 'primary_key': 'True', 'max_length': '32'})
        }
    }

    complete_apps = ['core']