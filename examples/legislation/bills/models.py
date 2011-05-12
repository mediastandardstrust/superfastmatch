from django.db import models
from lxml.etree import fromstring
from superfastmatch.django.models import *

CONGRESSES = (
    (108,'108th'),
    (109,'109th'),
    (110,'110th'),
    (111,'111th'),
    (112,'112th')
)

SESSIONS = (
    (1,'First'),
    (2,'Second')
)

ORIGINS = (
    ('s','Senate'),
    ('h','House')
)

FORMS = (
    ('b','Bill'),
    ('j','Joint Resolution'),
    ('c','Concurrent Resolution'),
    ('r','Resolution'),
    ('a','Amendment')
)

STAGES = (
    ('ih','Introduced in House '),
    ('rh','Reported in House'),
    ('eh','Engrossed in House'),
    ('rfh','Referred in House'),
    ('enr','Enrolled Bill'),
    ('pcs','Placed on Calendar Senate'),
    ('eas','Engrossed Amendment Senate'),
    ('rfs','Referred in Senate'),
    ('rds','Received in Senate'),
    ('rs','Reported to Senate'),
    ('is','Introduced in Senate'),
    ('ats','Agreed to Senate'),
)

class BillManager(models.Manager):
    def add_bill(self,congress,session,number,origin,form,source,stage,content):
        bill,created = Bill.objects.get_or_create(congress = congress,
                                                  session  = session,
                                                  number   = number,
                                                  origin   = origin,
                                                  form     = form,
                                                  defaults={
                                                    'congress'  : congress,
                                                    'session'   : session,
                                                    'number'    : number,
                                                    'origin'    : origin,
                                                    'form'      : form
                                                  })
        bill_stage,created = BillStage.objects.get_or_create(bill    = bill,
                                                             stage   = stage,
                                                             source  = source,
                                                             defaults ={
                                                                'bill'      : bill,
                                                                'stage'     : stage,
                                                                'source'    : source,
                                                                'content'   : content
                                                            })
        return (bill,bill_stage)

class Bill(models.Model):    
    congress = models.PositiveIntegerField(choices=CONGRESSES,blank=False, null=False)
    session = models.PositiveIntegerField(choices=SESSIONS,blank=False, null=False)
    number = models.PositiveIntegerField(blank=False, null=False)
    origin = models.CharField(max_length=1,choices=ORIGINS,blank=False,null=False)
    form = models.CharField(max_length=1,choices=FORMS,blank=False,null=False)
    
    objects = BillManager()
    
    @property
    def title(self):
        return "%s Congress %s Session - %s%s%s" %(self.get_congress_display(),self.get_session_display(),self.origin.upper(),self.form.upper(),self.number)
    
    def __unicode__(self):
        return self.title
        
    class Meta:
        ordering = ['congress','session','number']

class BillStage(Document):
    bill = models.ForeignKey(Bill,related_name='stages')
    stage = models.CharField(max_length=3,choices=STAGES,blank=False,null=False)
    source = models.URLField(verify_exists=False,blank=True,null=True)
    
    @models.permalink
    def get_absolute_url(self):
        return ('bills.views.show',(self.id,))
    
    def __unicode__(self):
        return self.get_stage_display()
        
    @property
    def clean(self):
        texts = fromstring(self.content).xpath('//header|//text')
        cleaned=""
        for text in texts:
            if text.text is not None:
                cleaned += "".join(text.itertext()).replace('\n','')
                cleaned += '\n' if text.tag == 'text' else ' '
        return cleaned