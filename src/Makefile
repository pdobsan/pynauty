
PYTHON = python3
PIP = $(PYTHON) -m pip

PYNAUTY_VERSION = $(shell $(PYTHON) -m pynauty pynauty-version)
NAUTY_VERSION = $(shell $(PYTHON) -m pynauty nauty-version)
NAUTY_TARFILE = $(shell $(PYTHON) -m pynauty nauty-tarfile)
NAUTY_SHA1SUM = $(shell $(PYTHON) -m pynauty nauty-checksum)
NAUTY_URL = $(shell $(PYTHON) -m pynauty nauty-url)
NAUTY_DIR = $(shell $(PYTHON) -m pynauty nauty-dir)


help:
	@echo Available targets:
	@echo ' fetch-nauty  - fetch nauty source distribution'
	@echo ' clean        - delete nauty source and related files'
	@echo
	@echo 'Pynauty version:' ${PYNAUTY_VERSION}
	@echo 'Nauty version:  ' ${NAUTY_VERSION}
	@echo 'Nauty source:   ' ${NAUTY_URL}
	@echo 'Nauty tarfile:  ' ${NAUTY_TARFILE}
	@echo 'Nauty checksum :' ${NAUTY_SHA1SUM}

.PHONY: fetch-nauty
fetch-nauty:
	curl -O ${NAUTY_URL}
	sha1sum -c ${NAUTY_SHA1SUM}
	tar xf ${NAUTY_TARFILE}

.PHONY: clean
clean:
	rm -fr ${NAUTY_VERSION}
	rm -f ${NAUTY_TARFILE}
