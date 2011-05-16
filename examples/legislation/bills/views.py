from django.shortcuts import get_object_or_404,render_to_response
from itertools import groupby
from models import *

def all(request):
    data = {
                'stages': BillStage.objects.select_related().defer('content','source')[0:100]
            }
    return render_to_response('list.html',data)
    
def show(request,id):
    stage = BillStage.objects.select_related().get(id=id)
    return render_to_response('show.html',{'stage' : stage})
    