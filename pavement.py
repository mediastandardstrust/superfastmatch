from paver.easy import *
import paver.doctools
from paver.setuputils import setup

CURRENT_VERSION="0.2"

setup(
    name="superfastmatch",
    packages=['superfastmatch','superfastmatch.django'],
    version=CURRENT_VERSION,
    url="http://mediastandardstrust.github.com/superfastmatch",
    author="Media Standards Trust",
    author_email="donovanhide@gmail.com"
)

options(
    minilib=Bunch( 
          extra_files=['doctools', 'virtual'] 
    ),
    sphinx=Bunch(
        builddir=".build"
    ),
    docs_env=Bunch(
        dest_dir=".env/docs_env",
        script_name='.env/docs_env.py',
        packages_to_install=["sphinx"],
        no_site_packages=True
    ),
    examples_env=Bunch(
        dest_dir=".env/examples_env",
        script_name='.env/examples_env.py',
        packages_to_install=["django","lxml","dist/superfastmatch-%s.tar.gz"%CURRENT_VERSION],
        no_site_packages=True
    )
)

@task
@needs('docs','generate_setup', 'minilib', 'setuptools.command.sdist')
def sdist():
    """Overrides sdist to make sure that our docs and setup.py are generated."""
    pass

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
    call_task("paver.doctools.doc_clean")
    sh("python %s && source %s/bin/activate && paver html" % (options.docs_env.script_name,options.docs_env.dest_dir))

@task
def github_docs(options):
    """Build the docs and upload to GitHub - You'll need to be a committer!"""
    call_task("docs")
    sh("git checkout gh-pages && \
        cp -r superfastmatch/docs/ . && \
        git add . && \
        git commit . -m 'Rendered documentation for Github Pages.' && \
        git push origin gh-pages && \
        git checkout master" % options)

@task 
@needs('sdist')
def examples(options):
    """Build examples environment"""
    path(options.examples_env.dest_dir).makedirs()
    options.virtualenv=options.examples_env
    call_task("paver.virtual.bootstrap")
    sh("python %s && source %s/bin/activate" % (options.examples_env.script_name,options.docs_env.dest_dir))

@task
@needs('examples')
def legislation(options):
    """Runs legislation example"""
    sh("source %s/bin/activate && \
        cd examples/legislation && \
        ./manage.py syncdb && \
        ./manage.py runserver" % options.examples_env.dest_dir)

