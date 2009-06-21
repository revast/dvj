default: all

all:
	@$(MAKE) -C src all

.PHONY: run runall clean spotless

run: runall

runall: all
	@$(MAKE) -C src run

clean:
	@$(MAKE) -C src clean

spotless:
	@$(MAKE) -C src spotless

