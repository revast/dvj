#STANDARD MAKEFILE COMMANDS

osx-lib: ../../obj/osx/lgl_syphon.o

../../obj/osx/lgl_syphon.o: lgl_syphon.mm
	mkdir -p obj/osx
	gcc -c lgl_syphon.mm -o obj/osx/module.osx.lib -I Syphon.framework/Headers

#.PHONY: osx-lib

#osx-lib:
#	./m

