from django.conf.urls.defaults import patterns, include, url

from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('',
    # Examples:
    # url(r'^$', 'legislation.views.home', name='home'),
    # url(r'^legislation/', include('legislation.foo.urls')),

    url(r'^admin/', include(admin.site.urls)),
)
