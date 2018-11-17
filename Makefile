.PHONY: clean all kip nro

all: nro kip

nro:
	@$(MAKE) -f Makefile_nro

kip:
	$(MAKE) -f Makefile_kip

clean:
	$(MAKE) -f Makefile_kip clean
	$(MAKE) -f Makefile_nro clean
