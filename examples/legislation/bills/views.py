from django.shortcuts import get_object_or_404,render_to_response
from django.core.paginator import Paginator, InvalidPage, EmptyPage
from models import *

def all(request):
    stages = BillStage.objects.select_related().defer('content','source').order_by('bill__number')
    paginator = Paginator(stages, 1000) 
    page = request.GET.get('page')
    try:
        page = int(request.GET.get('page', '1'))
    except ValueError:
        page = 1
    try:
        stages = paginator.page(page)
    except (EmptyPage, InvalidPage):
        stages = paginator.page(paginator.num_pages)
    return render_to_response('list.html',{'stages':stages})
    
def show(request,id):
    stage = BillStage.objects.select_related().get(id=id)
    return render_to_response('show.html',{'stage' : stage})
    