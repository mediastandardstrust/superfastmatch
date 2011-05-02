from django.test import TestCase

#An example NewsArticle Document with a clean property to strip html tags
import re
from superfastmatch.django.models import *

class NewsArticle(Document):
    @property
    def clean(self):
        return re.sub('<[^<]+?>', '', self.content)

CONTENT1='<html><body><h1>Princess reveals wedding dress</h1><p>Today we saw the princess...</p></body>'
CONTENT2='<html><body><h1>Princess reveals Karen Burton wedding dress</h1><p>Today we saw the beautiful princess...</p></body>'

class DocumentTest(TestCase):
    def test_creation(self):
        article = NewsArticle(content=CONTENT1)
        article.save()
        self.assertEqual(Content.objects.count(),1)
        
    def test_change_document(self):
        article = NewsArticle(content=CONTENT1)
        article.save()
        cleaned_content = article.clean
        article.content=CONTENT2
        article.save()
        self.assertNotEqual(article.clean,cleaned_content)
        self.assertEqual(Content.objects.count(),1)
        
    def test_delete_document(self):
        article = NewsArticle(content=CONTENT1)
        article.save()
        article.delete()
        self.assertEqual(NewsArticle.objects.count(),0)
        self.assertEqual(Content.objects.count(),0)    