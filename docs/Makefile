# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = source
BUILDDIR      = ../src/pynauty/docs

# Put it first so that "make" without argument is like "make help".
help:
	@$(MODULE_TEST) sphinx "In order to build the docmentation you need to have the Sphinx package installed".
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

# pynauty's own clean to avoid unnecessary call to MODULE_TEST
clean:
	rm -fr $(BUILDDIR)

.PHONY: help Makefile

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(MODULE_TEST) sphinx "In order to build the documentation you need to have the Sphinx package installed".
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
