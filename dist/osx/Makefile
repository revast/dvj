default: all

.PHONY: all

SRC_DVJ		= ../../dvj
SRC_GLSL	= ../../data/glsl
MACOS_DVJ	= root.dvj/dvj.app/Contents/MacOS
MACOS_ZTRON	= root.ztron/ZEBBLERTRON.app/Contents/MacOS
LIB_DVJ		= $(MACOS_DVJ)/lib
LIB_ZTRON	= $(MACOS_ZTRON)/lib
LIB_LOCAL	= /usr/local/lib
BIN_LOCAL	= /usr/local/bin
DST_DVJ		= $(MACOS_DVJ)/dvj
DVJ_GLSL	= $(MACOS_DVJ)/data/glsl
DST_ZTRON	= $(MACOS_ZTRON)/ZEBBLERTRON
ZTRON_GLSL	= $(MACOS_ZTRON)/data/glsl

ROOT_DVJ	= root.dvj
ROOT_DVJ_NOSVN	= $(ROOT_DVJ).nosvn
ROOT_ZTRON	= root.ztron
ROOT_ZTRON_NOSVN	= $(ROOT_ZTRON).nosvn

all:
	rm -fv dvj.dmg
	rm -fv ZEBBLERTRON.dmg
	cp -v $(SRC_DVJ) $(DST_DVJ)
	cp -v $(SRC_GLSL)/* $(DVJ_GLSL)
	cp -v $(BIN_LOCAL)/jackd $(MACOS_DVJ)
	cp -v $(LIB_LOCAL)/libSDL_image-1.2.0.dylib $(LIB_DVJ)
	cp -v $(LIB_LOCAL)/libpng12.0.dylib $(LIB_DVJ)
	cp -v $(LIB_LOCAL)/libjpeg.62.dylib $(LIB_DVJ)
	cp -v $(LIB_LOCAL)/libSDL_net-1.2.0.dylib $(LIB_DVJ)
	cp -v $(LIB_LOCAL)/libSDL-1.3.0.dylib $(LIB_DVJ)
	cp -v $(LIB_LOCAL)/libsamplerate.0.dylib $(LIB_DVJ)
	cp -v $(LIB_LOCAL)/libjack.0.dylib $(LIB_DVJ)
	cp -v $(LIB_LOCAL)/libjackserver.0.dylib $(LIB_DVJ)
	cp -v $(LIB_LOCAL)/jack/jack_coreaudio.so $(LIB_DVJ)/jack
	install_name_tool -change $(LIB_LOCAL)/libSDL_image-1.2.0.dylib @executable_path/lib/libSDL_image-1.2.0.dylib $(DST_DVJ)
	install_name_tool -change $(LIB_LOCAL)/libjpeg.62.dylib @executable_path/lib/libjpeg.62.dylib $(DST_DVJ)
	install_name_tool -change $(LIB_LOCAL)/libSDL_net-1.2.0.dylib @executable_path/lib/libSDL_net-1.2.0.dylib $(DST_DVJ)
	install_name_tool -change $(LIB_LOCAL)/libSDL-1.3.0.dylib @executable_path/lib/libSDL-1.3.0.dylib $(DST_DVJ)
	install_name_tool -change $(LIB_LOCAL)/libsamplerate.0.dylib @executable_path/lib/libsamplerate.0.dylib $(DST_DVJ)
	install_name_tool -change $(LIB_LOCAL)/libjack.0.dylib @executable_path/lib/libjack.0.dylib $(DST_DVJ)
	install_name_tool -change $(LIB_LOCAL)/libjackserver.0.dylib @executable_path/lib/libjackserver.0.dylib $(DST_DVJ)
	install_name_tool -change $(LIB_LOCAL)/libjackserver.0.dylib @executable_path/lib/libjackserver.0.dylib $(MACOS_DVJ)/jackd
	install_name_tool -change $(LIB_LOCAL)/libpng12.0.dylib @executable_path/lib/libpng12.0.dylib $(LIB_DVJ)/libSDL_image-1.2.0.dylib
	install_name_tool -change $(LIB_LOCAL)/libjpeg.62.dylib @executable_path/lib/libjpeg.62.dylib $(LIB_DVJ)/libSDL_image-1.2.0.dylib
	install_name_tool -change $(LIB_LOCAL)/libSDL-1.3.0.dylib @executable_path/lib/libSDL-1.3.0.dylib $(LIB_DVJ)/libSDL_image-1.2.0.dylib
	install_name_tool -change $(LIB_LOCAL)/libSDL-1.3.0.dylib @executable_path/lib/libSDL-1.3.0.dylib $(LIB_DVJ)/libSDL_net-1.2.0.dylib
	cp -v $(DST_DVJ) $(DST_ZTRON)
	cp -v $(DVJ_GLSL)/* $(ZTRON_GLSL)
	cp -v $(MACOS_DVJ)/jackd $(MACOS_ZTRON)
	cp -v $(LIB_DVJ)/*.dylib $(LIB_ZTRON)
	cp -v $(LIB_DVJ)/jack/*.so $(LIB_ZTRON)/jack
	rm -rf $(ROOT_DVJ_NOSVN)
	svn export $(ROOT_DVJ) $(ROOT_DVJ_NOSVN)
	./mkdmg $(ROOT_DVJ_NOSVN) dvj
	rm -rf $(ROOT_DVJ_NOSVN)
	rm -rf $(ROOT_ZTRON_NOSVN)
	svn export $(ROOT_ZTRON) $(ROOT_ZTRON_NOSVN)
	./mkdmg $(ROOT_ZTRON_NOSVN) ZEBBLERTRON
	rm -rf $(ROOT_ZTRON_NOSVN)

