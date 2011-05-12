import re
import urllib
import time
import multiprocessing
import logging
from lxml.html import fromstring as fromhtmlstring
from lxml.etree import XMLParser,parse,tostring
from django.core.management.base import BaseCommand
from bills.models import *

logger = logging.getLogger('superfastmatch')

def do_scrape(doc):
    try:
        regex = re.compile(r"^(?P<origin>\w)(?P<form>\D)?(?P<number>\d+)_?(?P<stage>\w+)")
        parser = XMLParser(recover=True)
        source = doc['link']
        details = re.match(regex,doc['text'])
        content = parse(source,parser)
        if content.getroot() is None:
            logger.info("Skipping %s - no content" %source)
            return None
        logger.info("%s %s"%(doc['link'],details.groups()))
        number = int(details.group('number'))
        origin = details.group('origin').lower()
        form = details.group('form').lower() if details.group('form') else 'b'
        stage = details.group('stage').lower()
        if stage == 'enr':
            session = 1 if content.xpath('//session')[0].text.lower().strip().find('first')!=-1 else 2
        else:
            session = int(content.xpath('//session')[0].text.strip()[0])
        return {
                    'congress'  : doc['congress'],
                    'session'   : session,
                    'number'    : number,
                    'origin'    : origin,
                    'form'      : form,
                    'stage'     : stage,
                    'source'    : source,
                    'content'   : tostring(content)
               }
    except Exception,ex:
        logger.exception("Oops")

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
        for bill in pool.imap(do_scrape,links,chunksize=10):
            if bill:
                Bill.objects.add_bill(**bill)
