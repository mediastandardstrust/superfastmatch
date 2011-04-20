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
    ),
    docs_env=Bunch(
        dest_dir=".env/docs_env",
        script_name='.env/docs_env.py',
        packages_to_install=["sphinx"],
        no_site_packages=True
    )
)

@task
@needs('paver.doctools.html')
def html(options):
    """Build the docs and put them into our package."""
    destdir = path('superfastmatch/docs')
    destdir.rmtree()
    builtdocs = path("docs") / options.builddir / "html"
    builtdocs.move(destdir)

@task
def docs(options):
    """Build environment for creating docs and then build the docs"""
    path(options.docs_env.dest_dir).makedirs()
    options.virtualenv=options.docs_env
    call_task("paver.virtual.bootstrap")
    sh("python %s && source %s/bin/activate && paver html" % (options.docs_env.script_name,options.docs_env.dest_dir))

@task
def github_docs(options):
    """Build the docs and upload to GitHub
    You'll need to be a committer!"""
    call_task("docs")
    sh("cp -r superfastmatch/docs/ temp/ && \
        git checkout gh-pages && \
        mv temp/* . && \
        git commit . -m 'Rendered documentation for Github Pages.' && \
        git push origin gh-pages && \
        git checkout master" % options)
