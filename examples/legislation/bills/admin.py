from django.contrib import admin
from models import *

class BillStageInline(admin.StackedInline):
    model = BillStage
    readonly_fields = ('content','bill','stage','source')

class BillStageAdmin(admin.ModelAdmin):
    list_display = ('__unicode__',)
    readonly_fields = ('content','bill','stage','source')

class BillAdmin(admin.ModelAdmin):
    inlines = [BillStageInline,]
    list_display = ('title','congress','session','origin','number','form')
    readonly_fields = ('title','congress','session','origin','number','form')
    list_filter = ('congress','session','origin','form')
    search_fields = ('congress','origin','form','number')
    
admin.site.register(BillStage,BillStageAdmin)
admin.site.register(Bill,BillAdmin)
