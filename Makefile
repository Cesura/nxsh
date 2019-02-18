.PHONY: clean all sys nro rebuild dist

all: nro sys

nro:
	@$(MAKE) rebuild
	@$(MAKE) -f Makefile_nro

sys:
	@$(MAKE) rebuild
	@$(MAKE) -f Makefile_sys

dist:
	@$(MAKE) rebuild
	@$(MAKE) -f Makefile_nro dist
	@$(MAKE) rebuild
	@$(MAKE) -f Makefile_sys dist

clean:
	@$(MAKE) -f Makefile_sys clean
	@$(MAKE) -f Makefile_nro clean
	@rm -rf release

files := $(wildcard source/*) $(wildcard include/*)
REBUILD := $(foreach f, $(files), $(if $(findstring __SYS__, $(shell cat $(f))), $(f))) # Gets a list of all the files that have __SYS__ in them

rebuild:
	@touch $(REBUILD) # Renews the last modified date for all the files in REBUILD so that they get rebuilt
