from django.contrib import admin
from models import *

class BillAdmin(admin.ModelAdmin):
    pass
    
admin.site.register(Bill,BillAdmin)
