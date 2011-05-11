from django.core.management.base import BaseCommand
from bills.models import *

class Command(BaseCommand):
    help = 'associates US Congress Bills'

    def handle(self, *args, **options):
        BillStage.objects.associate()