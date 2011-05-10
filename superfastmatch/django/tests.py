from django.test import TestCase

#An example NewsArticle Document with a clean property to strip html tags
import re
from superfastmatch.django.models import *

class NewsArticle(Document):
    @property
    def clean(self):
        return re.sub('<[^<]+?>', '', self.content)

class PressRelease(Document):
    @property
    def clean(self):
        return re.sub('<[^<]+?>', '', self.content)

CONTENT1 = '<html><body><h1>Princess reveals beautiful wedding dress</h1><p>Today we saw the princess...</p></body>'
CONTENT2 = '<html><body><h1>Princess reveals Karen Burton beautiful wedding dress</h1><p>Today we saw the beautiful princess...</p></body>'
SEARCH1 = 'Karen Burton beautiful Wedding Dress'

class DocumentTest(TestCase):
    def tearDown(self):
        # Need to trigger the index deletes after each test
        # TODO: Make Doument Manager support bulk deletes
        for c in Content.objects.all():
            c.delete()

    def test_creation(self):
        article = NewsArticle(content=CONTENT1)
        article.save()
        self.assertEqual(Content.objects.count(),1)
    
    def test_change_document(self):
        article = NewsArticle(content=CONTENT1)
        article.save()
        cleaned_content = article.clean
        article.content = CONTENT2
        article.save()
        self.assertNotEqual(article.clean,cleaned_content)
        self.assertEqual(Content.objects.count(),1)

    def test_delete_document(self):
        article = NewsArticle(content=CONTENT1)
        article.save()
        article.delete()
        self.assertEqual(NewsArticle.objects.count(),0)
        self.assertEqual(Content.objects.count(),0)    

    def test_search_document_single_type(self):
        article = NewsArticle(content=CONTENT1)    
        article.save()
        results = NewsArticle.objects.search(SEARCH1)
        self.assertEqual(len(results[NewsArticle]),1)
        self.assertEqual(len(results),1)

    def test_search_document_two_types(self):
        article = NewsArticle(content=CONTENT1)     
        article.save()
        press_release = PressRelease(content=CONTENT2)
        press_release.save()
        results = NewsArticle.objects.search(SEARCH1)
        self.assertEqual(len(results[NewsArticle]),1)
        self.assertEqual(len(results[PressRelease]),1)

    def test_search_document_specified_type(self):
        article = NewsArticle(content=CONTENT1)    
        article.save()
        press_release = PressRelease(content=CONTENT2)
        press_release.save()
        results = NewsArticle.objects.search(SEARCH1,[NewsArticle])
        self.assertEqual(len(results[NewsArticle]),1)
        self.assertEqual(len(results[PressRelease]),0)

    def test_search_document_specified_types(self):
        article = NewsArticle(content=CONTENT1)    
        article.save()
        press_release = PressRelease(content=CONTENT2)
        press_release.save()
        results = NewsArticle.objects.search(SEARCH1,[NewsArticle,PressRelease])
        self.assertEqual(len(results[NewsArticle]),1)
        self.assertEqual(len(results[PressRelease]),1)
    
    def test_search_changed_document(self):
        article = NewsArticle(content=CONTENT1)    
        article.save()
        press_release = PressRelease(content=CONTENT2)
        press_release.save()
        results1 = NewsArticle.objects.search(SEARCH1,[NewsArticle,PressRelease])
        article.content=CONTENT2
        article.save()
        results2 = NewsArticle.objects.search(SEARCH1,[NewsArticle,PressRelease])
        self.assertNotEqual(results1,results2)
    
    def test_search_multiple_same_type_documents(self):
        article1 = NewsArticle(content=CONTENT1)
        article1.save()
        article2 = NewsArticle(content=CONTENT1)
        article2.save()
        article3 = NewsArticle(content=CONTENT2)
        article3.save()
        results1 = NewsArticle.objects.search(SEARCH1,[NewsArticle,PressRelease])
        results2 = NewsArticle.objects.search(SEARCH1,[NewsArticle])
        results3 = NewsArticle.objects.search(SEARCH1)
        self.assertEqual(results1,results2)
        self.assertEqual(results2,results3)

    def test_associate_documents(self):
        article = NewsArticle(content=CONTENT1)
        article.save()
        press_release = PressRelease(content=CONTENT2)
        press_release.save()
        NewsArticle.objects.associate()
        self.assertEqual(len(article.similar),1)
        self.assertEqual(len(press_release.similar),1)
        self.assertEqual(press_release.similar[0],article)
        self.assertEqual(article.similar[0],press_release)