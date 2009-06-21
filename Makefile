default: all

all:
	@$(MAKE) -C src all

.PHONY: run runall clean distclean

clean:
	rm -f dvj 
	@$(MAKE) -C src clean

spotless:
	@$(MAKE) -C src distclean

