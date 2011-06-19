LIBPATH = $(wildcard build/lib.*)

WEBDIR = ~/public_html/software/pynauty
TARFILE = $(wildcard dist/pynauty-*)

NAUTY_BUILT = nauty/.nauty-objects-built

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

pynauty: $(NAUTY_BUILT)
	python setup.py build

test: pynauty
	cd tests; PYTHONPATH="../${LIBPATH}:$(PYTHONPATH)" python test_pynauty.py

install: pynauty
ifdef VIRTUAL_ENV
	python setup.py install
else
	@echo ERROR: no VIRTUAL_ENV environment varaible found.
	@echo cannot install, aborting ...
	@exit 1
endif

distro: docu
	python setup.py sdist

docu: docu-clean pynauty
	cd docs; PYTHONPATH="../$(LIBPATH):$(PYTHONPATH)" make html

docu-clean:
	cd docs; make clean

webins: distro docu
	install -m 644 ${TARFILE} ${WEBDIR}
	rsync -av docs/_build/html/ ${WEBDIR}
	chmod -R a+rX ${WEBDIR}

clean: docu-clean
	rm -fr build
	rm -fr dist
	rm -f MANIFEST

clobber: clean clean-nauty

# nauty stuff

GTOOLS = copyg listg labelg dretog amtog geng complg shortg showg NRswitchg \
  biplabg addedgeg deledgeg countg pickg genrang newedgeg catg genbg directg \
  multig planarg gentourng

nauty-config:
	cd nauty; [ -f makefile ] || ./configure

nauty-programs: nauty-config
	cd nauty; make
	touch $(NAUTY_BUILT)

$(NAUTY_BUILT): nauty-objects
nauty-objects: nauty-config
	cd nauty; make nauty.o nautil.o naugraph.o
	touch $(NAUTY_BUILT)

clean-nauty:
	cd nauty; make clean; \
	rm -f makefile dreadnaut ${GTOOLS}
	rm -f $(NAUTY_BUILT)

