default: all

.PHONY: all

SRC_DVJ		= ../../dvj.osx
DST_DVJ		= root.dvj/dvj.app/Contents/MacOS/dvj
DST_ZTRON	= root.ztron/ZEBBLERTRON.app/Contents/MacOS/ZEBBLERTRON

all:
	@rm -f dvj.dmg
	@rm -f ZEBBLERTRON.dmg
	@cp $(SRC_DVJ) $(DST_DVJ)
	@install_name_tool -change /usr/local/lib/libSDL_image-1.2.0.dylib @executable_path/lib/libSDL_image-1.2.0.dylib $(DST_DVJ)
	@install_name_tool -change /usr/local/lib/libpng12.0.dylib @executable_path/lib/libpng12.0.dylib $(DST_DVJ)
	@install_name_tool -change /usr/local/lib/libjpeg.62.dylib @executable_path/lib/libjpeg.62.dylib $(DST_DVJ)
	@install_name_tool -change /usr/local/lib/libSDL_net-1.2.0.dylib @executable_path/lib/libSDL_net-1.2.0.dylib $(DST_DVJ)
	@install_name_tool -change /usr/local/lib/libSDL-1.3.0.dylib @executable_path/lib/libSDL-1.3.0.dylib $(DST_DVJ)
	@install_name_tool -change /usr/local/lib/libsamplerate.0.dylib @executable_path/lib/libsamplerate.0.dylib $(DST_DVJ)
	@install_name_tool -change /usr/local/lib/libjack.0.dylib @executable_path/lib/libjack.0.dylib $(DST_DVJ)
	@install_name_tool -change /usr/local/lib/libjackserver.0.dylib @executable_path/lib/libjackserver.0.dylib $(DST_DVJ)
	./mkdmg root.dvj dvj
	@cp $(DST_DVJ) $(DST_ZTRON)
	./mkdmg root.ztron ZEBBLERTRON

