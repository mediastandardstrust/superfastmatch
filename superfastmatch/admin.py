from django.contrib import admin
from django.utils.text import truncate_words
from models import *

def admin_link(target):
    return '<a href="../../%s/%s/%d">%s</a>' % (target._meta.app_label,target._meta.module_name,target.id,target)

def site_link(target):
    return '<a href="%s">View on site</a>' % target.get_absolute_url()
    
class AssociationAdmin(admin.ModelAdmin):
    list_display = ('from_document','to_document','common_characters','common_percentage')
    list_display_links = ('common_characters','common_percentage')

    def from_document(self,obj):
        return '%s | %s' % (admin_link(obj.from_content.content_object),site_link(obj.from_content.content_object))
    from_document.allow_tags = True

    def to_document(self,obj):
        return admin_link(obj.to_content.content_object)
    to_document.allow_tags = True

class FragmentAdmin(admin.ModelAdmin):
    list_display = ('short_text','association_link','length')
    
    def association_link(self,obj):
        return site_link(obj.association.from_content.content_object)
    association_link.allow_tags=True
    
    def short_text(self,obj):
        return truncate_words(obj.text,15)
        
admin.site.register(Association,AssociationAdmin)
admin.site.register(Fragment,FragmentAdmin)