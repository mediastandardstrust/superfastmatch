from paver.easy import *
import paver.doctools
from paver.setuputils import setup

setup(
    name="SuperFastMatch",
    packages=['superfastmatch'],
    version="0.2",
    url="http://mediastandardstrust.github.com/superfastmatch",
    author="Media Standards Trust",
    author_email="donovanhide@gmail.com"
)

options(
    sphinx=Bunch(
        builddir=".build"
    )
)

@task

@task
@needs('paver.doctools.html')
def html(options):
    """Build the docs and put them into our package."""
    destdir = path('superfastmatch/docs')
    destdir.rmtree()
    builtdocs = path("docs") / options.builddir / "html"
    builtdocs.move(destdir)