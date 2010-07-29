curl http://trac.macports.org/export/52572/trunk/dports/graphics/jpeg/files/patch-jpeglib.h >patch-jpeglib.h
curl http://trac.macports.org/export/52572/trunk/dports/graphics/jpeg/files/patch-config.guess >patch-config.guess
curl http://trac.macports.org/export/52572/trunk/dports/graphics/jpeg/files/patch-ltconfig >patch-ltconfig
curl http://trac.macports.org/export/52572/trunk/dports/graphics/jpeg/files/patch-ltmain.sh >patch-ltmain.sh
curl http://trac.macports.org/export/52572/trunk/dports/graphics/jpeg/files/patch-config.sub >patch-config.sub
curl http://trac.macports.org/export/52572/trunk/dports/graphics/jpeg/files/patch-makefile.cfg >patch-makefile.cfg

patch -p0 <patch-jpeglib.h
patch -p0 <patch-config.guess
patch -p0 <patch-ltconfig
patch -p0 <patch-ltmain.sh
patch -p0 <patch-config.sub
patch -p0 <patch-makefile.cfg

rm patch-jpeglib.h
rm patch-config.guess
rm patch-ltconfig
rm patch-ltmain.sh
rm patch-config.sub
rm patch-makefile.cfg

