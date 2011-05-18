import re
import urllib
import urllib2
import time
import multiprocessing
import logging
import os
from lxml.html import fromstring as fromhtmlstring
from lxml.etree import tostring,fromstring,XMLParser,parse
from django.core.management.base import BaseCommand
from bills.models import *
from bills.lib.urllib2helpers import CacheHandler

logger = logging.getLogger('superfastmatch')

CACHE_DIR=".cache"

def load_bill(doc):
    try:
        regex = re.compile(r"^(?P<origin>\w)(?P<form>\D)?(?P<number>\d+)_?(?P<stage>\w+)")
        source = doc['link']
        details = re.match(regex,doc['text'])
        parser = XMLParser(recover=True)
        content = parse(os.path.join(CACHE_DIR,source),parser)
        logger.info("%s %s"%(source,details.groups()))
        number = int(details.group('number'))
        origin = details.group('origin').lower()
        form = details.group('form').lower() if details.group('form') else 'b'
        stage = details.group('stage').lower()
        try:
            session = int(content.xpath('//session')[0].text.strip()[0])
        except ValueError:
            session = 1 if content.xpath('//session')[0].text.lower().strip().find('first')!=-1 else 2
        Bill.objects.add_bill(
                    congress=doc['congress'],
                    session=session,
                    number=number,
                    origin=origin,
                    form=form,
                    stage=stage,
                    source=source,
                    content=tostring(content)
               )
    except AssertionError,ex:
        logger.exception("%s has bad XML"%(source,))
    except Exception,ex:
        logger.exception("%s has bad XML"%(source,))

def get_bill(link):
    url=link['link']
    filename = os.path.join(CACHE_DIR,url)
    path = os.path.dirname(filename)
    if not os.path.exists(path):
        os.makedirs(path)
    if not os.path.exists(filename):
        logger.info('Downloading %s'%url)
        urllib.urlretrieve (url,filename)

class Command(BaseCommand):
    help = 'scrapes US Congress Bills'
        
    def handle(self, *args, **options):
        pool = multiprocessing.Pool(processes=multiprocessing.cpu_count()*2)
        links=[]
        for congress in range(108,113):
            docs_url = 'http://thomas.loc.gov/home/gpoxmlc%s/' % congress
            logger.info("Downloading %s" %docs_url)
            docs = fromhtmlstring(urllib.urlopen(docs_url).read())
            docs.make_links_absolute(docs_url)
            for doc in docs.cssselect('a[href$="xml"]'):
                links.append({'link':doc.get('href'),'text':doc.text,'congress':congress})
        pool.map(get_bill,links,chunksize=10)
        pool.map(load_bill,links,chunksize=10)
        # map(load_bill,links)
