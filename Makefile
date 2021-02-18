PYTHON = python3
PIP = $(PYTHON) -m pip

SOURCE_DIR = src
PYNAUTY_VERSION = $(shell $(PYTHON) -m $(SOURCE_DIR).pynauty pynauty-version)
NAUTY_VERSION = $(shell $(PYTHON) -m $(SOURCE_DIR).pynauty nauty-version)
NAUTY_TARFILE = $(shell $(PYTHON) -m $(SOURCE_DIR).pynauty nauty-tarfile)
NAUTY_SHA1SUM = $(shell $(PYTHON) -m $(SOURCE_DIR).pynauty nauty-checksum)
NAUTY_URL = $(shell $(PYTHON) -m $(SOURCE_DIR).pynauty nauty-url)
NAUTY_DIR = $(shell $(PYTHON) -m $(SOURCE_DIR).pynauty nauty-dir)
INSTALL_PYTEST = $(shell $(PYTHON) -c "import pytest" 2> /dev/null; echo $$?)

python_version_full := $(wordlist 2,4,$(subst ., ,$(shell $(PYTHON) --version 2>&1)))
python_version_major := $(word 1,${python_version_full})
python_version_minor := $(word 2,${python_version_full})
python_version_patch := $(word 3,${python_version_full})
platform := "$(shell uname -a)"
machine := $(shell uname -m)
LIBPATH = build/lib.linux-$(machine)-${python_version_major}.${python_version_minor}

VENV_DIR = .venv-pynauty

help:
	@echo Available targets:
	@echo
	@echo '  pynauty        - build the pynauty extension module'
	@echo '  tests          - run all tests loading from build/'
	@echo '  clean          - remove all files created by build/packaging except' $(VENV_DIR)/
	@echo '  virtenv-create - create virtualenv' $(VENV_DIR)/
	@echo '  virtenv-ins    - install pynauty into active virtualenv' $(VENV_DIR)/
	@echo '  virtenv-tests  - run all tests loading from' $(VENV_DIR)/
	@echo '  virtenv-unins  - uninstall pynauty from active virtualenv' $(VENV_DIR)/
	@echo '  virtenv-delete - delete virtualenv:' $(VENV_DIR)/
	@echo '  user-ins       - install pynauty into ~/.local/'
	@echo '  user-unins     - uninstall pynauty from ~/.local/'
	@echo '  dist           - create a source distribution'
	@echo '  docs           - build pyanauty documentation'
	@echo '  clean-docs     - remove pyanauty documentation'
	@echo '  fetch-nauty    - download Nauty source files'
	@echo '  nauty-objects  - compile only nauty.o nautil.o naugraph.o schreier.o naurng.o'
	@echo '  clean-nauty    - a "distclean" for nauty'
	@echo '  remove-nauty   - remove all nauty related files'
	@echo '  clobber        - clean + remove-nauty + clean-docs + virtenv-delete'
	@echo
	@echo 'Pynauty version:' ${PYNAUTY_VERSION}
	@echo 'Nauty version:  ' ${NAUTY_VERSION}
	@echo 'Nauty source:   ' ${NAUTY_URL}
	@echo 'Python version: ' ${python_version_full}
	@echo 'Pip used:       ' ${PIP}
	@echo 'Platform:       ' ${platform}

pynauty: nauty-objects
	$(PYTHON) setup.py build

.PHONY: tests
tests: pynauty
ifeq ($(INSTALL_PYTEST),1)
	$(PIP) install pytest 
endif
	PYTHONPATH="../${LIBPATH}:$(PYTHONPATH)" $(PYTHON) -m pytest
	@echo foo
	
	

.PHONY: virtenv-create
virtenv-create:
	$(PYTHON) -m venv $(VENV_DIR) 
	@echo Created virtualenv: $(VENV_DIR)/
	@echo To activate it type: source $(PWD)/$(VENV_DIR)/bin/activate

virtenv-delete:
	rm -fr $(VENV_DIR)
	@echo Deleted virtualenv: $(VENV_DIR)/
	@echo If it is still active, deactivate it!

virtenv-ins: pynauty
ifdef VIRTUAL_ENV
	$(PIP) install --upgrade .
else
	@echo ERROR: no VIRTUAL_ENV environment varaible found.
	@echo cannot install, aborting ...
	@exit 1
endif

virtenv-tests:
ifdef VIRTUAL_ENV
ifeq ($(INSTALL_PYTEST),1)
	$(PIP) install pytest 
endif
	$(PYTHON) -m pytest
else
	@echo ERROR: no VIRTUAL_ENV environment varaible found.
	@echo cannot run tests, aborting ...
	@exit 1
endif

virtenv-unins:
ifdef VIRTUAL_ENV
	$(PIP) uninstall pynauty
else
	@echo ERROR: no VIRTUAL_ENV environment varaible found.
	@echo cannot uninstall, aborting ...
	@exit 1
endif

user-ins: pynauty
	$(PIP) install --user --upgrade .

user-unins:
	$(PIP) uninstall pynauty

.PHONY: dist
dist: pynauty tests docs
	$(PYTHON) setup.py sdist

.PHONY: docs
docs: pynauty
	cd docs; make html

clean-docs:
	cd docs; make clean

clean:
	rm -fr build
	rm -fr dist
	rm -f MANIFEST
	rm -fr tests/{__pycache__,data_graphs.pyc}
	rm -fr .pytest_cache/
	rm -fr pynauty.egg-info

clobber: clean remove-nauty clean-docs virtenv-delete

# nauty targets

fetch-nauty:
	cd $(SOURCE_DIR); make $@

nauty-config:
	cd $(SOURCE_DIR); make $@

nauty-objects:
	cd $(SOURCE_DIR); make $@

nauty-programs:
	cd $(SOURCE_DIR); make $@

clean-nauty:
	cd $(SOURCE_DIR); make $@

remove-nauty:
	cd $(SOURCE_DIR); make $@

