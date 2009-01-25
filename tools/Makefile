default:
	@$(MAKE) -C diskWriter all
	@$(MAKE) -C frameRateTest all
	@$(MAKE) -C importMusicVideo all
	@$(MAKE) -C logDrawer all
	@$(MAKE) -C videoFreqMixer all

all:
	@$(MAKE) -C diskWriter all
	@$(MAKE) -C frameRateTest all
	@$(MAKE) -C importMusicVideo all
	@$(MAKE) -C logDrawer all
	@$(MAKE) -C videoFreqMixer all

.PHONY: run runall clean spotless

run:	default
	@$(MAKE) -C diskWriter run
	@$(MAKE) -C frameRateTest run
	@$(MAKE) -C importMusicVideo run
	@$(MAKE) -C logDrawer run
	@$(MAKE) -C videoFreqMixer run

runall:	all
	@$(MAKE) -C diskWriter runall
	@$(MAKE) -C frameRateTest runall
	@$(MAKE) -C importMusicVideo runall
	@$(MAKE) -C logDrawer runall
	@$(MAKE) -C videoFreqMixer runall

clean:
	@$(MAKE) -C diskWriter clean
	@$(MAKE) -C frameRateTest clean
	@$(MAKE) -C importMusicVideo clean
	@$(MAKE) -C logDrawer clean
	@$(MAKE) -C videoFreqMixer clean

spotless:
	@$(MAKE) -C diskWriter spotless
	@$(MAKE) -C frameRateTest spotless
	@$(MAKE) -C importMusicVideo spotless
	@$(MAKE) -C logDrawer spotless
	@$(MAKE) -C videoFreqMixer spotless

