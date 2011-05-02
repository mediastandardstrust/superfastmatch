import re
import urllib
from lxml.html import fromstring as fromhtmlstring
from lxml.etree import XMLParser,parse,tostring
from django.core.management.base import BaseCommand
from bills.models import *

class Command(BaseCommand):
    help = 'scrapes US Congress Bills'

    def handle(self, *args, **options):
        regex = re.compile(r"^(?P<origin>\w)(?P<form>\D)?(?P<number>\d+)_?(?P<stage>\w+)")
        parser = XMLParser(recover=True)   
        for congress in range(108,113):
            docs_url = 'http://thomas.loc.gov/home/gpoxmlc%s/' % congress
            docs = fromhtmlstring(urllib.urlopen(docs_url).read())
            docs.make_links_absolute(docs_url)
            for doc in docs.cssselect('a[href$="xml"]'):
                source = doc.get('href')
                details = re.match(regex,doc.text)
                content = parse(source,parser)
                if content.getroot() is None:
                    print "Skipping %s - no content" %source
                    break
                print "%s %s"%(doc.get('href'),details.groups())
                number = int(details.group('number'))
                origin = details.group('origin').lower()
                form = details.group('form').lower() if details.group('form') else 'b'
                stage = details.group('stage').lower()
                if stage == 'enr':
                    session = 1 if content.xpath('//session')[0].text.lower().strip().find('first')!=-1 else 2
                else:
                    session = int(content.xpath('//session')[0].text.strip()[0])
                bill,created = Bill.objects.get_or_create(congress = congress,
                                                          session  = session,
                                                          number   = number,
                                                          origin   = origin,
                                                          form     = form,
                                                          defaults={
                                                            'congress'  : congress,
                                                            'session'   : session,
                                                            'number'    : number,
                                                            'origin'    : origin,
                                                            'form'      : form
                                                          })
                stage,created = BillStage.objects.get_or_create(bill     = bill,
                                                                stage    = stage,
                                                                source   = source,
                                                                defaults ={
                                                                    'bill'      : bill,
                                                                    'source'    : source,
                                                                    'stage'     : stage,
                                                                    'content'   : tostring(content)
                                                                })