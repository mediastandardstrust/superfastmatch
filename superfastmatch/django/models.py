"""
This set of models is a Django interface to the SuperFastMatch algorithm and the Kyoto Cabinet/Tycoon document index.

The following `settings <http://docs.djangoproject.com/en/dev/ref/settings>`_ can be used to configure how to communicate with Kyoto Tycoon::
    
    SUPERFASTMATCH_HOST = '127.0.0.1'
    SUPERFASTMATCH_PORT = 1977

and you must remember to include both :mod:`superfastmatch.django` and 
`django.contrib.contenttypes <http://docs.djangoproject.com/en/dev/ref/contrib/contenttypes/>`_ 
in `INSTALLED_APPS <http://docs.djangoproject.com/en/dev/ref/settings/#installed-apps>`_. For Example::

    INSTALLED_APPS=(
                        'superfastmatch.django',
                        'django.contrib.contenttypes'
                    )

Example model definition:

>>> import re
>>> from superfastmatch.django.models import Document
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

"""

from django.db import models
from django.contrib.contenttypes.models import ContentType
from django.contrib.contenttypes import generic
from django.core.exceptions import ObjectDoesNotExist
from django.conf import settings
from superfastmatch.kyototycoon import KyotoTycoon

class DocumentManager(models.Manager):
    """Manager for running bulk operations on Documents"""

    def associate(self):
        """Method for updating associations between all documents"""
        pass

    def search(self,text):
        """Method for searching for text in all documents"""
        pass

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
    def clean(self):
        """Override this property to provide custom cleaning of content"""
        return self.content

    def save(self, *args, **kwargs):
        """Overridden save makes sure document index is kept up to date"""
        super(Document, self).save(*args, **kwargs)
        cleaned,created = self.cleaned_content.get_or_create(defaults={'content_object':self})
        if cleaned.content != self.clean:
            cleaned.content = self.clean
            cleaned.save()

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

    def save(self,*args,**kwargs):
        """Updates document index"""
        super(Content,self).save(*args, **kwargs)
        tycoon = KyotoTycoon(host=getattr(settings,'SUPERFASTMATCH_HOST','127.0.0.1'),port=getattr(settings,'SUPERFASTMATCH_PORT',1977))
        tycoon.open()

    class Meta:
        db_table = 'superfastmatch_content'
        unique_together = ('content_type', 'object_id')

class Association(models.Model):
    """
    Many to Many relation between :class:`Content` instances. 
    There is no need to access this class directly
    """
    
    from_content = models.ForeignKey('Content',related_name='from_document')
    to_content = models.ForeignKey('Content',related_name='to_document')
    common_characters = models.PositiveIntegerField(blank=False,null=False)
    common_percentage = models.FloatField(blank=False,null=False)
    
    class Meta:
        db_table = 'superfastmatch_association'