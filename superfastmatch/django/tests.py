from django.test import TestCase
from django.conf import settings


#An example NewsArticle Document with a clean property to strip html tags
import re
import os
import codecs
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
BOOK1 = codecs.open(os.path.join(os.path.split(__file__)[0],'fixtures/great_expectations.txt'),"r","utf-8").read()
BOOK2 = codecs.open(os.path.join(os.path.split(__file__)[0],'fixtures/oliver_twist.txt'),"r","utf-8").read()

class DocumentTest(TestCase):
    def setUp(self):
        settings.SUPERFASTMATCH_WINDOW_SIZE=15
        settings.SUPERFASTMATCH_HASH_WIDTH=32
        settings.SUPERFASTMATCH_PORT=1977
        
    def tearDown(self):
        # Need to trigger the index deletes after each test
        # TODO: Make Doument Manager support bulk deletes
        for c in Content.objects.all():
            c.delete()
        settings.SUPERFASTMATCH_WINDOW_SIZE=15
        settings.SUPERFASTMATCH_HASH_WIDTH=32
        settings.SUPERFASTMATCH_PORT=1977

    def test_creation(self):
        article = NewsArticle(content=CONTENT1)
        article.save()
        self.assertEqual(Content.objects.count(),1)
        
    # def test_load_big_documents(self):
    #     book1 = NewsArticle(content=BOOK1)
    #     book1.save()
    #     book2 = NewsArticle(content=BOOK2)
    #     book2.save()
    
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
    
    def test_search_correct_order(self):
        for i in range(0,15):
            article2 = NewsArticle(content=CONTENT2)
            article2.save()
            article1 = NewsArticle(content=CONTENT1)
            article1.save()
        results = NewsArticle.objects.search(SEARCH1)
        scores = results[NewsArticle].values()
        for i in range(0,15):
            self.assertEqual(scores[i],37)
        for i in range(15,20):
            self.assertEqual(scores[i],25)

    def test_associate_documents(self):
        article = NewsArticle(content=CONTENT1)
        article.save()
        press_release = PressRelease(content=CONTENT2)
        press_release.save()
        NewsArticle.objects.associate(multiprocess=False)
        self.assertEqual(len(article.similar),1)
        self.assertEqual(len(press_release.similar),1)
        self.assertLess(0.0,press_release.similar[article])
        self.assertLess(0.0,article.similar[press_release])
        try:
            press_release.similar[press_release]
            self.assertTrue(False)
        except KeyError:
            pass
            
    def test_fragments(self):
        article = NewsArticle(content=CONTENT1)
        article.save()
        press_release = PressRelease(content=CONTENT2)
        press_release.save()
        NewsArticle.objects.associate(multiprocess=False)
        self.assertEqual(article.fragments[press_release][0].text," beautiful wedding dressToday we saw the ")

    def test_window_size_15(self):
        article = NewsArticle(content=CONTENT1)
        article.save()
        press_release = PressRelease(content=CONTENT2)
        press_release.save()
        NewsArticle.objects.associate(multiprocess=False)
        self.assertEqual(len(article.fragments[press_release]),2)
    
    def test_window_size_25(self):
        settings.SUPERFASTMATCH_WINDOW_SIZE=25
        article = NewsArticle(content=CONTENT1)
        article.save()
        press_release = PressRelease(content=CONTENT2)
        press_release.save()
        NewsArticle.objects.associate(multiprocess=False)
        self.assertEqual(len(article.fragments[press_release]),1)
        
    def test_hash_width_24(self):
        settings.SUPERFASTMATCH_HASH_WIDTH=24
        article = NewsArticle(content=CONTENT1)
        article.save()
        press_release = PressRelease(content=CONTENT2)
        press_release.save()
        NewsArticle.objects.associate(multiprocess=False)
        self.assertEqual(len(article.fragments[press_release]),2)
        
        