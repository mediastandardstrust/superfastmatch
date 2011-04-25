from django.db import models

class Document(models.Model):
    source = models.URLField(verify_exists=False,blank=True,null=True)
    content = models.TextField(blank=False,null=False)
    cleaned_content = models.TextField(blank=False,null=False)
    
    class Meta:
        abstract = True