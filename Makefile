.PHONY: clean all kip nro rebuild

all: nro kip

nro:
	@$(MAKE) rebuild
	@$(MAKE) -f Makefile_nro

kip:
	@$(MAKE) rebuild
	$(MAKE) -f Makefile_kip

clean:
	$(MAKE) -f Makefile_kip clean
	$(MAKE) -f Makefile_nro clean

files := $(wildcard source/*) $(wildcard include/*)
REBUILD := $(foreach f, $(files), $(if $(findstring __KIP__, $(shell cat $(f))), $(f))) # Gets a list of all the files that have __KIP__ in them

rebuild:
	@touch $(REBUILD) # Renews the last modified date for all the files in REBUILD so that they get rebuilt
