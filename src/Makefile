# Sub-Makefile to deal with Nauty related stuff

help:
	@echo Available targets:
	@echo '  nauty-objects  - compile only nauty.o nautil.o naugraph.o schreier.o naurng.o'
	@echo '  nauty-programs - build all nauty programs'
	@echo '  clean-nauty    - a "distclean" for nauty'
	@echo
	@echo 'Pynauty version:' ${PYNAUTY_VERSION}
	@echo 'Nauty version:  ' ${NAUTY_VERSION}
	@echo 'Nauty directory:' ${NAUTY_DIR}

nauty-config: $(NAUTY_DIR)/config.log
$(NAUTY_DIR)/config.log:
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
