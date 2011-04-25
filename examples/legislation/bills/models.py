from django.db import models
from superfastmatch.django.models import Document

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
    ('is','Introduced in Senate'),
    ('ats','Agreed to Senate'),
)

class Bill(models.Model):    
    congress = models.PositiveIntegerField(blank=False, null=False)
    session = models.PositiveIntegerField(blank=False, null=False)
    number = models.PositiveIntegerField(blank=False, null=False)
    origin = models.CharField(max_length=1,choices=ORIGINS,blank=False,null=False)
    form = models.CharField(max_length=1,choices=FORMS,blank=False,null=False)

class BillStage(Document):
    bill = models.ForeignKey(Bill)
    stage = models.CharField(max_length=3,choices=STAGES,blank=False,null=False)