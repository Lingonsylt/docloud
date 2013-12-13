# -*- coding: utf-8 -*-
from south.utils import datetime_utils as datetime
from south.db import db
from south.v2 import SchemaMigration
from django.db import models


class Migration(SchemaMigration):

    def forwards(self, orm):
        # Adding model 'FileInfo'
        db.create_table('core_fileinfo', (
            ('id', self.gf('django.db.models.fields.AutoField')(primary_key=True)),
            ('hash', self.gf('django.db.models.fields.CharField')(max_length=128)),
            ('path', self.gf('django.db.models.fields.CharField')(max_length=1024)),
            ('installation', self.gf('django.db.models.fields.related.ForeignKey')(to=orm['core.Installation'])),
            ('indexed', self.gf('django.db.models.fields.BooleanField')(default=False)),
        ))
        db.send_create_signal('core', ['FileInfo'])

        # Adding M2M table for field tags on 'FileInfo'
        m2m_table_name = db.shorten_name('core_fileinfo_tags')
        db.create_table(m2m_table_name, (
            ('id', models.AutoField(verbose_name='ID', primary_key=True, auto_created=True)),
            ('fileinfo', models.ForeignKey(orm['core.fileinfo'], null=False)),
            ('tag', models.ForeignKey(orm['core.tag'], null=False))
        ))
        db.create_unique(m2m_table_name, ['fileinfo_id', 'tag_id'])


    def backwards(self, orm):
        # Deleting model 'FileInfo'
        db.delete_table('core_fileinfo')

        # Removing M2M table for field tags on 'FileInfo'
        db.delete_table(db.shorten_name('core_fileinfo_tags'))


    models = {
        'auth.group': {
            'Meta': {'object_name': 'Group'},
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'name': ('django.db.models.fields.CharField', [], {'unique': 'True', 'max_length': '80'}),
            'permissions': ('django.db.models.fields.related.ManyToManyField', [], {'to': "orm['auth.Permission']", 'blank': 'True', 'symmetrical': 'False'})
        },
        'auth.permission': {
            'Meta': {'object_name': 'Permission', 'unique_together': "(('content_type', 'codename'),)", 'ordering': "('content_type__app_label', 'content_type__model', 'codename')"},
            'codename': ('django.db.models.fields.CharField', [], {'max_length': '100'}),
            'content_type': ('django.db.models.fields.related.ForeignKey', [], {'to': "orm['contenttypes.ContentType']"}),
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'name': ('django.db.models.fields.CharField', [], {'max_length': '50'})
        },
        'auth.user': {
            'Meta': {'object_name': 'User'},
            'date_joined': ('django.db.models.fields.DateTimeField', [], {'default': 'datetime.datetime.now'}),
            'email': ('django.db.models.fields.EmailField', [], {'blank': 'True', 'max_length': '75'}),
            'first_name': ('django.db.models.fields.CharField', [], {'blank': 'True', 'max_length': '30'}),
            'groups': ('django.db.models.fields.related.ManyToManyField', [], {'to': "orm['auth.Group']", 'related_name': "'user_set'", 'symmetrical': 'False', 'blank': 'True'}),
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'is_active': ('django.db.models.fields.BooleanField', [], {'default': 'True'}),
            'is_staff': ('django.db.models.fields.BooleanField', [], {'default': 'False'}),
            'is_superuser': ('django.db.models.fields.BooleanField', [], {'default': 'False'}),
            'last_login': ('django.db.models.fields.DateTimeField', [], {'default': 'datetime.datetime.now'}),
            'last_name': ('django.db.models.fields.CharField', [], {'blank': 'True', 'max_length': '30'}),
            'password': ('django.db.models.fields.CharField', [], {'max_length': '128'}),
            'user_permissions': ('django.db.models.fields.related.ManyToManyField', [], {'to': "orm['auth.Permission']", 'related_name': "'user_set'", 'symmetrical': 'False', 'blank': 'True'}),
            'username': ('django.db.models.fields.CharField', [], {'unique': 'True', 'max_length': '30'})
        },
        'contenttypes.contenttype': {
            'Meta': {'object_name': 'ContentType', 'unique_together': "(('app_label', 'model'),)", 'ordering': "('name',)", 'db_table': "'django_content_type'"},
            'app_label': ('django.db.models.fields.CharField', [], {'max_length': '100'}),
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'model': ('django.db.models.fields.CharField', [], {'max_length': '100'}),
            'name': ('django.db.models.fields.CharField', [], {'max_length': '100'})
        },
        'core.fileinfo': {
            'Meta': {'object_name': 'FileInfo'},
            'hash': ('django.db.models.fields.CharField', [], {'max_length': '128'}),
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'indexed': ('django.db.models.fields.BooleanField', [], {'default': 'False'}),
            'installation': ('django.db.models.fields.related.ForeignKey', [], {'to': "orm['core.Installation']"}),
            'path': ('django.db.models.fields.CharField', [], {'max_length': '1024'}),
            'tags': ('django.db.models.fields.related.ManyToManyField', [], {'to': "orm['core.Tag']", 'symmetrical': 'False'})
        },
        'core.installation': {
            'Meta': {'object_name': 'Installation'},
            'user': ('django.db.models.fields.related.ForeignKey', [], {'to': "orm['core.User']", 'related_name': "'installations'"}),
            'uuid': ('django.db.models.fields.CharField', [], {'default': "'99988506611f11e3b70a90e6ba0d52ef'", 'primary_key': 'True', 'max_length': '32'})
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
            'organization': ('django.db.models.fields.related.ForeignKey', [], {'to': "orm['core.Organization']", 'related_name': "'tags'"}),
            'users': ('django.db.models.fields.related.ManyToManyField', [], {'to': "orm['core.User']", 'symmetrical': 'False', 'through': "orm['core.UserTag']"})
        },
        'core.user': {
            'Meta': {'object_name': 'User'},
            'auth_user': ('django.db.models.fields.related.ForeignKey', [], {'to': "orm['auth.User']", 'related_name': "'docloud_users'"}),
            'email': ('django.db.models.fields.EmailField', [], {'max_length': '75'}),
            'has_password': ('django.db.models.fields.BooleanField', [], {'default': 'False'}),
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'name': ('django.db.models.fields.CharField', [], {'blank': 'True', 'max_length': '128'}),
            'organization': ('django.db.models.fields.related.ForeignKey', [], {'to': "orm['core.Organization']", 'related_name': "'users'"}),
            'owner': ('django.db.models.fields.BooleanField', [], {'default': 'False'}),
            'tag_creator': ('django.db.models.fields.BooleanField', [], {'default': 'False'}),
            'tags': ('django.db.models.fields.related.ManyToManyField', [], {'to': "orm['core.Tag']", 'symmetrical': 'False', 'through': "orm['core.UserTag']"})
        },
        'core.usertag': {
            'Meta': {'object_name': 'UserTag'},
            'id': ('django.db.models.fields.AutoField', [], {'primary_key': 'True'}),
            'owns_tag': ('django.db.models.fields.BooleanField', [], {'default': 'False'}),
            'tag': ('django.db.models.fields.related.ForeignKey', [], {'to': "orm['core.Tag']", 'related_name': "'usertags'"}),
            'user': ('django.db.models.fields.related.ForeignKey', [], {'to': "orm['core.User']", 'related_name': "'usertags'"})
        }
    }

    complete_apps = ['core']