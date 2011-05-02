from django.shortcuts import get_object_or_404,render_to_response
from itertools import groupby
from models import *

def all(request):
    stages = BillStage.objects.select_related().defer('content')
    bills={}
    for bill,stages in groupby(stages,lambda s:s.bill):
        bills[bill]=list(stages)
    return render_to_response('list.html',{'bills' : bills})
    
def show(request,id):
    stage = BillStage.objects.get(id=id)
    return render_to_response('show.html',{'stage' : stage})
    