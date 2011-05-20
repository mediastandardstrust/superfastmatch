"""
This set of models is a Django interface to the SuperFastMatch algorithm and the Kyoto Cabinet/Tycoon document index.

The following `settings <http://docs.djangoproject.com/en/dev/ref/settings>`_ can be used to configure how to communicate with Kyoto Tycoon::
    
    SUPERFASTMATCH_HOST() = '127.0.0.1'
    SUPERFASTMATCH_PORT() = 1977
    
as well as a minimum percentage of the document required to be copied for an association to be created and the window size for matching::

    SUPERFASTMATCH_MIN_THRESHOLD = 0.02
    SUPERFASTMATCH_WINDOW_SIZE = 15
    SUPERFASTMATCH_HASH_WIDTH = 32

and you must remember to include both :mod:`superfastmatch` and 
`django.contrib.contenttypes <http://docs.djangoproject.com/en/dev/ref/contrib/contenttypes/>`_ 
in `INSTALLED_APPS <http://docs.djangoproject.com/en/dev/ref/settings/#installed-apps>`_. For example::

    INSTALLED_APPS=(
                        'superfastmatch',
                        'django.contrib.contenttypes'
                    )

Example model definition:

>>> import re
>>> from superfastmatch.models import Document
>>> class NewsArticle(Document):
...   @property
...   def clean(self):
...     return re.sub('<[^<]+?>', '', self.content)

Example usage:

>>> article_one = NewsArticle(content="An example news article")
>>> article_one.save()
>>> article_two = NewsArticle(content="Another example news article with a bit more content")
>>> article_two.save()
>>> NewsArticle.objects.search('example news article')
defaultdict(<type 'dict'>, {<class 'superfastmatch.tests.NewsArticle'>: OrderedDict([(<NewsArticle: NewsArticle object>, 21), (<NewsArticle: NewsArticle object>, 21)])})
>>> article_one.delete()
>>> article_two.delete()
"""

from django.db import models,transaction,reset_queries
from django.contrib.contenttypes.models import ContentType
from django.contrib.contenttypes import generic
from django.core.exceptions import ObjectDoesNotExist
from django.conf import settings
from django.utils.text import truncate_words
from collections import defaultdict
from itertools import groupby
import logging
import multiprocessing
from superfastmatch.ordereddict import OrderedDict
from superfastmatch.kyototycoon import KyotoTycoon
from superfastmatch.matcher import match

logger = logging.getLogger('superfastmatch')

def get_window_size():
    return getattr(settings,'SUPERFASTMATCH_WINDOW_SIZE',15)

def get_min_threshold():
    return getattr(settings,'SUPERFASTMATCH_MIN_THRESHOLD',0.02)

def get_hash_width():
    return getattr(settings,'SUPERFASTMATCH_HASH_WIDTH',32)

def get_tycoon():
    return KyotoTycoon(host=getattr(settings,'SUPERFASTMATCH_HOST','127.0.0.1'),port=getattr(settings,'SUPERFASTMATCH_PORT',1978))

def do_associate(content_id):
    # Prevent debug mode SQL logging memory leak
    reset_queries()
    content = Content.objects.get(id=content_id)
    Association.objects.filter(from_content=content).delete()
    results = Document.objects.search(content.content)
    association_count = 0
    fragment_count = 0
    for result in results.values():
        contents=Document.objects.get_contents([doc for doc,_ in result.items()],defer=False)
        for document,score in result.items():
            to_content=contents[document]
            if content!=to_content:
                association_count+=1
                association=Association(from_content=content,
                                        to_content=to_content,
                                        common_characters=score,
                                        common_percentage=float(score)/len(content.content)
                                    )
                association.save()
                for m in match(content.content,to_content.content,get_window_size()):
                    fragment_count+=1
                    text=content.content[m[0]:m[0]+m[2]]
                    Fragment.objects.create(association=association,
                                            from_start=m[0],
                                            to_start=m[1],
                                            length=m[2],
                                            text=text,
                                            hash=text.__hash__(),
                                            score=0 # TODO implement average hash count algorithm
                                            )
    logger.info("Associations: %d Fragments:%d Length: %d Content: %s"% (association_count,fragment_count,len(content.content),truncate_words(content.content,10)))


class DocumentManager(models.Manager):
    """Manager for running bulk operations on Documents"""

    # Slightly modified from    
    # http://blog.roseman.org.uk/2010/02/22/django-patterns-part-4-forwards-generic-relations/
    def _fill_content_objects(self,queryset,select_related=False):
        generics = {}
        for item in queryset:
            generics.setdefault(item.content_type_id, set()).add(item.object_id)
        content_types = ContentType.objects.in_bulk(generics.keys())
        relations = {}
        for ct, fk_list in generics.items():
            ct_model = content_types[ct].model_class()
            if select_related:
                relations[ct] = ct_model.objects.select_related().in_bulk(list(fk_list))
            else:
                relations[ct] = ct_model.objects.in_bulk(list(fk_list))
        for item in queryset:
            setattr(item, '_content_object_cache',relations[item.content_type_id][item.object_id])

    def get_contents(self,documents,defer=True):
        """
        Returns a dictionary with each Document as a key and each respective Content as a value
        """
        results = {}
        documents=sorted(documents,key=lambda d:d.__class__)
        for c,g in groupby(documents,key=lambda d:d.__class__):
            docs = dict((d.id,d) for d in g)
            contents = Content.objects.filter(content_type = ContentType.objects.get_for_model(c) )\
                                      .filter(object_id__in = docs.keys())
            if defer:
                contents = contents.defer('content')
            for content in contents:
                results[docs[content.object_id]] = content
        return results

    def associate(self,multiprocess=True):
        """Method for updating associations between all documents"""
        content_ids = Content.objects.values_list('id',flat=True)
        if multiprocess:
            pool = multiprocessing.Pool(processes=multiprocessing.cpu_count()*2)
            pool.map(do_associate,content_ids)
        else:
            map(do_associate,content_ids)

    def search(self,text,document_types=[]):
        """
        Method for searching for text in all or specified documents
        
        :param text: The text to search for.
        :param document_types: A list or tuple of the document subclasses to include in the results. If not specified, all document types are returned.
        """
        
        doc_types=[ContentType.objects.get_for_model(m).id for m in document_types]
        min_threshold = int(round(int(get_min_threshold())*len(text)))
        tycoon = get_tycoon()
        tycoon.open()
        results = tycoon.search(text,doc_types,hash_width=get_hash_width(),min_threshold=min_threshold,window_size=get_window_size())
        tycoon.close()
        documents = defaultdict(dict)
        for content_type_id,object_ids in results.items():
            contents = Content.objects.filter(content_type=content_type_id)\
                                      .filter(object_id__in=object_ids.keys())\
                                      .select_related()
            self._fill_content_objects(contents,select_related=True)
            document_type = contents[0].content_object.__class__
            for content in contents:
                documents[document_type][content.content_object]=results[content_type_id][content.content_object.id]
            documents[document_type]=OrderedDict(sorted(documents[document_type].items(), key=lambda t: t[1], reverse=True))
        return documents
                
    def get_fragments(self,documents,select_related=True):
        """
        Returns a dictionary with each source Document as a key and an ordered dictionary as value
        The ordered dictionary has a similar Document as a key and a list of fragments as a value.
        """        
        results={}
        docs = dict([v.id,k] for k,v in self.get_contents(documents).iteritems())
        fragments=Fragment.objects.filter(association__from_content__in=docs.keys())\
                                  .order_by('association__from_content','-length')\
                                  .select_related(depth=2)\
                                  .defer('association__from_content__content','association__to_content__content')
        self._fill_content_objects([f.association.to_content for f in fragments],select_related=select_related)
        for content,g in groupby(fragments,lambda f:f.association.from_content):
              results[docs[content.id]]=OrderedDict()
              frags=sorted(g,key=lambda f:f.association.to_content_id)
              for to_content,f in groupby(frags,key=lambda f:f.association.to_content):
                  results[docs[content.id]][to_content.content_object]=list(f)
        return results

    def get_similar_documents(self,documents,select_related=True,defer=True):
        """
        Returns a dictionary with each source Document as a key and an ordered dictionary as a value.
        The ordered dictionary has a similar Document as a key and the common percentage as a value.
        """
        results={}
        docs = dict([v.id,k] for k,v in self.get_contents(documents).iteritems())
        associations = Association.objects.filter(from_content__in=docs.keys())\
                                  .order_by('from_content','-common_percentage')\
                                  .select_related(depth=1)
        if defer:
            associations=associations.defer('from_content__content','to_content__content')
        self._fill_content_objects([a.to_content for a in associations],select_related=select_related)
        for content,g in groupby(associations,lambda a:a.from_content):
            results[docs[content.id]]=OrderedDict((a.to_content.content_object,a.common_percentage) for a in g)
        return results
    

class Document(models.Model):
    """
    Abstract base class for SuperFastMatch documents. 
    Derive from this class and provide a suitable override of the :attr:`clean` property
    """    
    
    content = models.TextField(blank=False,null=False)
    """Contains the uncleaned text content"""
    
    cleaned_content = generic.GenericRelation('Content')
    """Returns the cleaned :class:`Content` instance for the document"""
    
    objects = DocumentManager()
    """Returns the :class:`DocumentManager`"""
    
    @property
    def similar(self):
        """Returns an ordered dictionary with a Document as a key and the common percentage as a value."""
        try:
            return self.__class__.objects.get_similar_documents([self],defer=False)[self]
        except KeyError:
            return None
            
    @property
    def fragments(self):
        """Returns an ordered dictionary with a Document as a key and a list of Fragments as the value"""
        try:
            return self.__class__.objects.get_fragments([self])[self]
        except KeyError:
            return None
            
    @property    
    def clean(self):
        """Override this property to provide custom cleaning of content"""
        return self.content

    def save(self, *args, **kwargs):
        """Overridden save makes sure document index is kept up to date"""
        super(Document, self).save(*args, **kwargs)
        create=False
        try:
            cleaned = self.cleaned_content.get()
            if cleaned.content != self.clean:
                cleaned.delete() 
                create=True
        except Content.DoesNotExist:
            create=True
        if create:
            cleaned = Content(content_object=self,content=self.clean)
            cleaned.save()
    
    def delete(self,*args,**kwargs):
        """Overridden delete makes sure document index is kept up to date"""
        self.cleaned_content.get().delete()
        super(Document, self).delete(*args, **kwargs)
    
    class Meta:
        abstract = True

class Content(models.Model):
    """
    Model for storing cleaned :class:`Document` content. 
    There is no need to access this class directly
    """
    
    content_type = models.ForeignKey(ContentType)
    object_id = models.PositiveIntegerField()
    content_object = generic.GenericForeignKey('content_type', 'object_id')
    content = models.TextField(blank=False,null=False)
    similar = models.ManyToManyField('self',through='Association',symmetrical=False,related_name='similar_content')

    @transaction.commit_manually
    def save(self,*args,**kwargs):
        """Updates document index every time a Content instance is saved"""
        try:
            verify_exists = False if not self.id else True
            super(Content,self).save(*args, **kwargs)
            tycoon = get_tycoon()
            tycoon.open()
            tycoon.add(self.content_type_id,self.object_id,self.content,hash_width=get_hash_width(),window_size=get_window_size(),verify_exists=verify_exists)
            tycoon.close()
        except Exception,ex:
            print ex.message
            transaction.rollback()
        else:
            transaction.commit()
    
    @transaction.commit_manually
    def delete(self,*args,**kwargs):
        """Deletes item from document index before Content instance is deleted"""
        try:
            tycoon = get_tycoon()
            tycoon.open()
            tycoon.delete(self.content_type_id,self.object_id,self.content,hash_width=get_hash_width(),window_size=get_window_size())
            tycoon.close()
            super(Content,self).delete(*args,**kwargs)
        except Exception,ex:
            print ex.message
            transaction.rollback()
        else:
            transaction.commit()
            
    class Meta:
        unique_together = ('content_type', 'object_id')

class Fragment(models.Model):
    """
    A fragment of text that is reused between Documents.
    """
    
    association = models.ForeignKey('Association')
    from_start = models.PositiveIntegerField(blank=False,null=False)
    to_start = models.PositiveIntegerField(blank=False,null=False)    
    length = models.PositiveIntegerField(blank=False,null=False)
    text = models.TextField(blank=False,null=False)
    hash = models.BigIntegerField(blank=False,null=False,db_index=True)
    score = models.PositiveIntegerField(blank=False,null=False,db_index=True)
    
    def __unicode__(self):
        return self.text

class Association(models.Model):
    """
    Many to Many relation between :class:`Content` instances. 
    There is no need to access this class directly.
    """
    
    from_content = models.ForeignKey('Content',related_name='from_document')
    to_content = models.ForeignKey('Content',related_name='to_document')
    common_characters = models.PositiveIntegerField(blank=False,null=False,db_index=True)
    common_percentage = models.FloatField(blank=False,null=False,db_index=True)
    
    class Meta:
        unique_together = ('from_content', 'to_content')
        