ECOS_REPOSITORY=$(CURDIR)/packages

export ECOS_REPOSITORY
.PHONY: redboot ecosram ecosrom

clean:
	-rm -r redboot
	-rm -r ecosram
	-rm -r ecosrom

install: 
	-mkdir -p ../../include/ecos
	-cp -b -f -r ecosrom/install/include/* ../../include/ecos
	-mkdir -p ../../lib/ecos/rom
	-mkdir -p ../../lib/ecos/ram
	-cp -b -f -r ecosrom/install/lib/* ../../lib/ecos/rom
	-cp -b -f -r ecosram/install/lib/* ../../lib/ecos/ram
	-mkdir -p ../../examples/ecos/redboot
	-cp -b -f redboot/install/bin/redboot.bin ../../examples/ecos/redboot

redboot:
	-mkdir redboot
	$(MAKE) redbootbuild -e -C redboot -f $(CURDIR)/makefile

ecosram:
	-mkdir ecosram
	$(MAKE) ecosrambuild -e -C ecosram -f $(CURDIR)/makefile

ecosrom:
	-mkdir ecosrom
	$(MAKE) ecosrombuild -e -C ecosrom -f $(CURDIR)/makefile

ecosnolcdrom:
	-mkdir ecosrom
	$(MAKE) ecosromnolcdbuild -e -C ecosrom -f $(CURDIR)/makefile
 
redbootbuild:
	ecosconfig new gbax redboot
	ecosconfig import ../packages/hal/arm/gbax/current/misc/redboot_ROM.ecm
	ecosconfig tree
	$(MAKE)

ecosrambuild:
	ecosconfig new gbax
	ecosconfig tree
	$(MAKE) build

ecosrombuild:
	ecosconfig new gbax
	ecosconfig import ../packages/hal/arm/gbax/current/misc/ecos_ROM.ecm
	ecosconfig tree
	$(MAKE) build

ecosromnolcdbuild:
	ecosconfig new gbax
	ecosconfig import ../packages/hal/arm/gbax/current/misc/ecos_noLCD_ROM.ecm
	ecosconfig tree
	$(MAKE) build

