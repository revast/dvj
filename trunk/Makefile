default:
	@$(MAKE) -C src all
	@$(MAKE) -C tools all

all:
	@$(MAKE) -C src all
	@$(MAKE) -C tools all

.PHONY: run runall clean spotless

run:	default
	@$(MAKE) -C src run

runall:	all
	@$(MAKE) -C src runall
	@$(MAKE) -C tools runall

clean:
	@$(MAKE) -C src clean
	@$(MAKE) -C tools clean

spotless:
	@$(MAKE) -C src spotless
	@$(MAKE) -C tools spotless

