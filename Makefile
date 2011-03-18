VIRTENV	= ~/python/virtualenvs/pynauty
LIBPATH = $(wildcard build/lib.*)

help:
	@echo Available targets:
	@echo '  pynauty       - build pynauty extension module'
	@echo '  test          - run tests'
	@echo '  install       - install pynauty'
	@echo '  distro        - create a source distribution'
	@echo '  webins        - install the distro and docs on the web'
	@echo '  docu          - build pyanauty documentation'
	@echo '  clean         - remove all python related temp files and dirs'
	@echo '  nauty-progs   - build all nauty programs'
	@echo '  nauty-objects - compile only nauty.o nautil.o naugraph.o'
	@echo '  clean-nauty   - a "distclean" for nauty'
	@echo '  clobber       - clean clean-nauty'

pynauty:
	python setup.py build

test: pynauty
	cd tests; PYTHONPATH=../${LIBPATH} python test_pynauty.py

install:
	. $(VIRTENV)/bin/activate; python setup.py install

distro:
	python setup.py sdist

docu: pynauty
	cd docs; make html

WEBDIR = ~/public_html/software/pynauty
TARFILE = $(wildcard dist/pynauty-*)

webins: distro docu
	install -m 644 ${TARFILE} ${WEBDIR}
	rsync -av docs/_build/html/ ${WEBDIR}
	chmod -R a+rX ${WEBDIR}

clean:
	rm -fr build
	rm -fr dist
	rm -f MANIFEST
	cd docs; make clean

clobber: clean clean-nauty

# nauty stuff

GTOOLS = copyg listg labelg dretog amtog geng complg shortg showg NRswitchg \
  biplabg addedgeg deledgeg countg pickg genrang newedgeg catg genbg directg \
  multig planarg gentourng

nauty-programs:
	cd nauty; ./configure; make

nauty-objects:
	cd nauty; ./configure; make nauty.o nautil.o naugraph.o

clean-nauty:
	cd nauty; make clean; \
	rm -f makefile dreadnaut ${GTOOLS}

