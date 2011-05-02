from django.contrib import admin
from models import *

class BillStageInline(admin.StackedInline):
    model = BillStage

class BillAdmin(admin.ModelAdmin):
    inlines = [BillStageInline,]
    list_display = ('title','congress','session','origin','number','form')
    list_filter = ('congress','session','origin','form')
    search_fields = ('congress','origin','form','number')
    
admin.site.register(Bill,BillAdmin)
