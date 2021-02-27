# Makefile to build, test and install pynauty

include Makefile.common

pynauty: nauty-objects
	$(PYTHON) setup.py build

.PHONY: tests
tests: pynauty
ifdef VIRTUAL_ENV
tests: install
	$(PYTHON) $(MODULE_TEST) pytest
	$(PYTHON) -m pytest 
else
	PYTHONPATH="${LIBPATH}:$(PYTHONPATH)" $(PYTHON) $(MODULE_TEST) pytest
	PYTHONPATH="${LIBPATH}:$(PYTHONPATH)" $(PYTHON) -m pytest
endif

minimal-test: pynauty
ifdef VIRTUAL_ENV
minimal-test: install
	$(PYTHON) tests/test_minimal.py
else
	PYTHONPATH="../${LIBPATH}:$(PYTHONPATH)" $(PYTHON) tests/test_minimal.py
endif

update-packaging-helpers:
ifdef VIRTUAL_ENV
	$(PIP) install --upgrade pip
	$(PIP) install --upgrade setuptools
	$(PIP) install --upgrade setuptools_scm
	$(PIP) install --upgrade setuptools_git
	$(PIP) install --upgrade wheel
	$(PIP) install --upgrade build
	$(PIP) install --upgrade twine
	$(PIP) install --upgrade auditwheel
else
	@echo using globally installed packaging helpers
endif

install: pynauty # docs
ifdef VIRTUAL_ENV
	$(PIP) install --upgrade .
else
	$(PIP) install --user --upgrade .
endif

uninstall:
	$(PIP) uninstall pynauty

.PHONY: docs
docs: pynauty
ifdef VIRTUAL_ENV
	$(PIP) install --upgrade sphinx
endif
	$(MAKE) -C docs html

.PHONY: dist
dist: pynauty minimal-test docs
	$(MAKE) clean-nauty
	#$(PYTHON) setup.py sdist
	#$(PYTHON) setup.py bdist_wheel
	$(PYTHON) -m build
	@cd dist/ ; ../src/fix-wheel-tag.sh
	@echo Packages created:
	@ls -l dist/

upload: dist
	$(TWINE) upload --repository testpypi dist/*

clean-docs:
	$(MAKE) -C docs clean

clean:
	rm -fr build
	rm -fr dist
	rm -f MANIFEST
	rm -fr tests/{__pycache__,data_graphs.pyc}
	rm -fr .pytest_cache/
	rm -fr pynauty.egg-info

virtenv-create:
	$(PYTHON) -m venv $(VENV_DIR)
	@echo Created virtualenv: $(VENV_DIR)/
	@echo To activate it type: source $(PWD)/$(VENV_DIR)/bin/activate

virtenv-delete:
	rm -fr $(VENV_DIR)
	@echo Deleted virtualenv: $(VENV_DIR)/
	@echo If it is still active, deactivate it!

clobber: clean clean-nauty clean-docs virtenv-delete

# nauty targets

nauty-config:
	$(MAKE) -C $(SOURCE_DIR) -f Makefile.nauty $@

nauty-objects:
	$(MAKE) -C $(SOURCE_DIR) -f Makefile.nauty $@

nauty-programs:
	$(MAKE) -C $(SOURCE_DIR) -f Makefile.nauty $@

clean-nauty:
	$(MAKE) -C $(SOURCE_DIR) -f Makefile.nauty $@

# vim: filetype=make syntax=make
