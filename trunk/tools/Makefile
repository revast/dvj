default:
	@$(MAKE) -C diskWriter all
	@$(MAKE) -C logDrawer all
	@$(MAKE) -C moviePrep all

all:
	@$(MAKE) -C diskWriter all
	@$(MAKE) -C logDrawer all
	@$(MAKE) -C moviePrep all

.PHONY: run runall clean spotless

run:	default
	@$(MAKE) -C diskWriter run
	@$(MAKE) -C logDrawer run
	@$(MAKE) -C moviePrep run

runall:	all
	@$(MAKE) -C diskWriter runall
	@$(MAKE) -C logDrawer runall
	@$(MAKE) -C moviePrep runall

clean:
	@$(MAKE) -C diskWriter clean
	@$(MAKE) -C logDrawer clean
	@$(MAKE) -C moviePrep clean

spotless:
	@$(MAKE) -C diskWriter spotless
	@$(MAKE) -C logDrawer spotless
	@$(MAKE) -C moviePrep spotless

