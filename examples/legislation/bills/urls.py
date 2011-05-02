from django.conf.urls.defaults import *

urlpatterns = patterns('',
    (r'^$','bills.views.all'),
    (r'^bill/(?P<id>\d+)/$','bills.views.show'),
)
