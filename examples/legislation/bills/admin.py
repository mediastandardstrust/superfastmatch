from django.contrib import admin
from models import *

class BillStageInline(admin.StackedInline):
    model = BillStage

class BillAdmin(admin.ModelAdmin):
    inlines = [BillStageInline,]
    
admin.site.register(Bill,BillAdmin)
