# Makefile to build, test and install pynauty

include Makefile.common

pynauty:
	$(PYTHON) setup.py build

.PHONY: tests
tests: pynauty
	PYTHONPATH="${LIBPATH}:$(PYTHONPATH)" $(PYTHON) $(MODULE_TEST) pytest
	PYTHONPATH="${LIBPATH}:$(PYTHONPATH)" $(PYTHON) -m pytest src/pynauty/tests

minimal-test: pynauty
	PYTHONPATH="${LIBPATH}:$(PYTHONPATH)" $(PYTHON) src/pynauty/tests/test_minimal.py

install: tests
ifdef VIRTUAL_ENV
	$(PIP) install --upgrade .
else
	$(PIP) install --user --upgrade .
endif

uninstall:
	$(PIP) uninstall pynauty

.PHONY: docs
docs: install
	$(PYTHON) $(MODULE_TEST) sphinx
	$(MAKE) -C docs html

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
