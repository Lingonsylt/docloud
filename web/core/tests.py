from django.test import TestCase
from core.models import User

class TestDB(TestCase):
    def test_db_connection(self):
        self.assertEqual(list(User.objects.all()), [])