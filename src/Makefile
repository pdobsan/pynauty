# Sub-Makefile to deal with Nauty related stuff

OSTYPE = $(shell uname)
ifeq ($(OSTYPE), Linux)
SHA1SUM = sha1sum
else ifeq ($(OSTYPE), Darwin)
SHA1SUM = shasum
endif

help:
	@echo Available targets:
	@echo '  fetch-nauty    - download Nauty source files'
	@echo '  nauty-objects  - compile only nauty.o nautil.o naugraph.o schreier.o naurng.o'
	@echo '  nauty-programs - build all nauty programs'
	@echo '  clean-nauty    - a "distclean" for nauty'
	@echo '  remove-nauty   - remove all nauty related files'
	@echo
	@echo 'Pynauty version:' ${PYNAUTY_VERSION}
	@echo 'Nauty version:  ' ${NAUTY_VERSION}
	@echo 'Nauty directory:' ${NAUTY_DIR}
	@echo 'Nauty source:   ' ${NAUTY_URL}
	@echo 'Nauty tarfile:  ' ${NAUTY_TARFILE}
	@echo 'Nauty checksum :' ${NAUTY_SHA1SUM}

fetch-nauty:
ifeq (,$(wildcard ./${NAUTY_TARFILE}))
	curl -O ${NAUTY_URL}
	@if $(SHA1SUM) -c --status ${NAUTY_SHA1SUM}; then tar xf ${NAUTY_TARFILE}; else rm -f ${NAUTY_TARFILE}; fi
else
	@if $(SHA1SUM) -c --status ${NAUTY_SHA1SUM}; then tar xf ${NAUTY_TARFILE}; else echo "ERROR: checksum failed." ; rm -f ${NAUTY_TARFILE}; echo " ===> run fetch-nauty again!" ; fi
endif

$(NAUTY_DIR)/configure.ac: | fetch-nauty

nauty-config: $(NAUTY_DIR)/config.log
$(NAUTY_DIR)/config.log: $(NAUTY_DIR)/configure.ac
	cd $(NAUTY_DIR); ./configure CFLAGS='-O4 -fPIC'

nauty-objects: nauty-config
	cd $(NAUTY_DIR); make nauty.o nautil.o naugraph.o schreier.o naurng.o

nauty-programs: nauty-config
	cd $(NAUTY_DIR); make

GTOOLS = copyg listg labelg dretog amtog geng complg shortg showg NRswitchg \
  biplabg addedgeg deledgeg countg pickg genrang newedgeg catg genbg directg \
  multig planarg gentourng ranlabg runalltests subdivideg watercluster2 \
  linegraphg naucompare

clean-nauty:
	cd $(NAUTY_DIR); rm -f *.o dreadnaut ${GTOOLS} nauty.a nauty1.a
	cd $(NAUTY_DIR); rm -f makefile config.log config.status gtools.h naututil.h nauty.h

remove-nauty:
	rm -fr ${NAUTY_DIR}
	rm -f ${NAUTY_TARFILE}
