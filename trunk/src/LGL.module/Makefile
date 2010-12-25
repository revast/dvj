include			Makefile.options

#DETERMINE TARGET ARCHITECTURE

PLATFORM		= $(shell uname)

ifeq ($(PLATFORM), Linux)
default:		lin
lib:			lin-lib
endif

ifeq ($(PLATFORM), Darwin)
default:		osx
lib:			osx-lib
endif

ifeq ($(PLATFORM), MINGW32_NT-5.1)
default:		w32
lib:			w32-lib
endif



#DEFINE ARCHITECTURE-SPECIFIC PATHS

SUFFIX_BIN_LIN		= lin
SUFFIX_BIN_OSX		= osx
SUFFIX_BIN_W32		= exe

TARGET_BIN_LIN		= ../$(PREFIX_BIN).$(SUFFIX_BIN_LIN)
TARGET_BIN_OSX		= ../$(PREFIX_BIN).$(SUFFIX_BIN_OSX)
TARGET_BIN_W32		= ../$(PREFIX_BIN).$(SUFFIX_BIN_W32)
TARGET_BIN_ALL		= $(TARGET_BIN_LIN) $(TARGET_BIN_OSX) $(TARGET_BIN_W32)

OBJDIR			= obj/
OBJDIR_LIN		= $(OBJDIR)lin/
OBJDIR_OSX		= $(OBJDIR)osx/
OBJDIR_W32		= $(OBJDIR)w32/
OBJDIR_ALL		= $(OBJDIR_LIN) $(OBJDIR_OSX) $(OBJDIR_W32) $(OBJDIR)

OBJDIR_SHORT		= obj
OBJDIR_LIN_SHORT	= $(OBJDIR)lin
OBJDIR_OSX_SHORT	= $(OBJDIR)osx
OBJDIR_W32_SHORT	= $(OBJDIR)w32
OBJDIR_ALL_SHORT	= $(OBJDIR_LIN_SHORT) $(OBJDIR_OSX_SHORT) $(OBJDIR_W32_SHORT) $(OBJDIR_SHORT)

SUFFIX_LIB_LIN		= lin.lib
SUFFIX_LIB_OSX		= osx.lib
SUFFIX_LIB_W32		= w32.lib

TARGET_LIB_LIN		= $(OBJDIR)lin/module.$(SUFFIX_LIB_LIN)
TARGET_LIB_OSX		= $(OBJDIR)osx/module.$(SUFFIX_LIB_OSX)
TARGET_LIB_W32		= $(OBJDIR)w32/module.$(SUFFIX_LIB_W32)
TARGET_LIB_ALL		= $(TARGET_LIB_LIN) $(TARGET_LIB_OSX) $(TARGET_LIB_W32)

PCH_LIN			= $(OBJDIR_LIN)PrecompiledHeaders.h.gch
PCH_OSX			= $(OBJDIR_OSX)PrecompiledHeaders.h.gch
PCH_W32			= $(OBJDIR_W32)PrecompiledHeaders.h.gch

SRC			= $(wildcard *.cpp)
HDR			= $(wildcard *.h)

OBJ_LIN			= $(patsubst %.cpp, $(OBJDIR_LIN)%.o, $(SRC))
OBJ_OSX			= $(patsubst %.cpp, $(OBJDIR_OSX)%.o, $(SRC))
OBJ_W32			= $(patsubst %.cpp, $(OBJDIR_W32)%.o, $(SRC))
OBJ_ALL			= $(OBJ_LIN) $(OBJ_OSX) $(OBJ_W32)

OBJ_FULL_LIN		= $(wildcard $(OBJDIR_LIN)*.o)
OBJ_FULL_OSX		= $(wildcard $(OBJDIR_OSX)*.o)
OBJ_FULL_W32		= $(wildcard $(OBJDIR_W32)*.o)
OBJ_FULL_ALL		= $(OBJ_FULL_LIN) $(OBJ_FULL_OSX) $(OBJ_FULL_W32)



#DEFINE ALL SUBDIRS THAT MUST BE COMPILED INTO MODULES / LIBRARIES

MODULE_DIRS		= $(wildcard *.module)
MODULE_DIRS_CLEAN	= $(patsubst %.module, %.module.clean, $(MODULE_DIRS))
MODULE_DIRS_SPOTLESS	= $(patsubst %.module, %.module.spotless, $(MODULE_DIRS))

MODULE_LIBS_LIN		= $(patsubst %.module, %.module/$(OBJDIR_SHORT)/lin/module.lin.lib, $(MODULE_DIRS))
MODULE_LIBS_OSX		= $(patsubst %.module, %.module/$(OBJDIR_SHORT)/osx/module.osx.lib, $(MODULE_DIRS))
MODULE_LIBS_W32		= $(patsubst %.module, %.module/$(OBJDIR_SHORT)/w32/module.w32.lib, $(MODULE_DIRS))
MODULE_LIBS_ALL		= $(MODULE_LIBS_LIN) $(MODULE_LIBS_OSX) $(MODULE_LIBS_W32)

MODULE_PCHS		= $(patsubst %.module, %.module/PrecompiledHeaders.pch, $(MODULE_DIRS))



#DEFINE LIN BUILD OPTIONS

AR_LIN			= ar rcs
CC_LIN			= g++
#MODEFLAGS_LIN		= -O4 -march=pentium4 -mfpmath=sse -fomit-frame-pointer -ffast-math -pipe
#MODEFLAGS_LIN		= -g -pg
MODEFLAGS_LIN		= -g
DEFINES_LIN		= -DLGL_LINUX -D__LINUX_ALSASEQ__
CFLAGS_LIN		= -Wall -I$(OBJDIR_LIN) `sdl-config --cflags` -I/usr/include/SDL $(MODEFLAGS_LIN) -I$(ROOT_SRC_DIR) -fsingle-precision-constant $(DEFINES_LIN)
CPPFLAGS_LIN		= -include PrecompiledHeaders.h $(CFLAGS_LIN)
AVCODECLIBRARIES_LIN	= -lavformat -lavcodec -lswscale -lavutil -lavcore
LGLLIBRARIES_LIN	= $(AVCODECLIBRARIES_LIN) `sdl-config --libs` `pkg-config --cflags --libs jack` -lSDL_image -lSDL_net -lGL -lfftw3f -lm $(AVCODECLIBRARIES_LIN) -lsamplerate -lcwiid -lasound


#DEFINE OSX BUILD OPTIONS

AR_OSX			= ar rcs
CC_OSX			= g++-4.0
MODEFLAGS_OSX		= -g
DEFINES_OSX		= -DLGL_LINUX -DLGL_OSX -Dfopen64=fopen -D__MACOSX_CORE__
CFLAGS_OSX		= -Wall -I$(OBJDIR_OSX) `sdl-config --cflags` $(MODEFLAGS_OSX) -I$(ROOT_SRC_DIR) $(DEFINES_OSX) -I/opt/local/include -L/opt/local/lib/ -L/usr/local/lib/ -I/usr/local/include/ -I/System/Library/Frameworks/CoreServices.framework/Versions/A/Frameworks/CarbonCore.framework/Versions/A/Headers
#-fsingle-precision-constant
CPPFLAGS_OSX		= -include PrecompiledHeaders.h $(CFLAGS_OSX)
LGLLIBRARIES_OSX	= -lSDL_image -lSDL_net -framework OpenGL -lSDL -framework AppKit -framework CoreMidi -framework CoreAudio -framework CoreFoundation -lSDLmain -lsamplerate -ljack -lavcore -lavcodec -lavformat -lswscale -lavutil -lz -lbz2 -lfftw3f



#DEFINE W32 BUILD OPTIONS

ifeq ($(PLATFORM), MINGW32_NT-5.1)
AR_W32			= ar rcs
CC_W32			= g++
#MODEFLAGS_W32		= -O2 -march=pentium4 -mfpmath=sse -fomit-frame-pointer -ffast-math -pipe
MODEFLAGS_W32		= -g
DEFINES_W32		= -DLGL_WIN32
CFLAGS_W32		= -Wall -I$(OBJDIR_W32) -I$(ROOT_SRC_DIR)/lib/w32/include -I$(ROOT_SRC_DIR)/lib/w32/include/SDL $(MODEFLAGS_W32) -I$(ROOT_SRC_DIR) -fsingle-precision-constant $(DEFINES_W32)
CPPFLAGS_W32		= $(CFLAGS_W32)
else
AR_W32			= i586-mingw32msvc-ar rcs
CC_W32			= i586-mingw32msvc-g++
#MODEFLAGS_W32		= -O2 -march=pentium4 -mfpmath=sse -fomit-frame-pointer -ffast-math -pipe
MODEFLAGS_W32		= -g
DEFINES_W32		= -DLGL_WIN32
CFLAGS_W32		= -Wall -I$(OBJDIR_W32) `/usr/local/cross-tools/i386-mingw32msvc/bin/i386-mingw32msvc-sdl-config --cflags` -Iinclude/w32 $(MODEFLAGS_W32) -I$(ROOT_SRC_DIR) -fsingle-precision-constant $(DEFINES_W32)
CPPFLAGS_W32		= -include PrecompiledHeaders.h $(CFLAGS_W32)
endif
LGLLIBRARIES_W32	= -Llib/w32 -lmingw32 -lSDLmain -lSDL -lSDL_image -lSDL_net -lglu32 -lopengl32 -mwindows



#DEFINE WHAT WE MUST BUILD

#.PHONY: all basic lin lin-lib lin-bin osx osx-lib osx-bin w32 w32-lib w32-bin

all: default
#basic: Makefile.depend tags $(OBJDIR_ALL_SHORT)
basic: tags $(OBJDIR_ALL_SHORT)

ifeq ($(CREATE_BINARY), 1)
lin: lin-lib lin-bin
osx: osx-lib osx-bin
w32: w32-lib w32-bin
else
lin: lin-lib
osx: osx-lib
w32: w32-lib
endif

lin-lib: basic $(TARGET_LIB_LIN)
osx-lib: basic $(TARGET_LIB_OSX)
w32-lib: basic $(TARGET_LIB_W32)

lin-bin: basic $(TARGET_BIN_LIN)
osx-bin: basic $(TARGET_BIN_OSX)
w32-bin: basic $(TARGET_BIN_W32)



#BUILDING THE BINARY FILE

$(TARGET_BIN_LIN): $(TARGET_LIB_LIN)
	@echo "	$(CC_LIN) $(TARGET_BIN_LIN)"
	@$(CC_LIN) $(CPPFLAGS_LIN) $(OBJ_FULL_LIN) $(LGLLIBRARIES_LIN) -o $(TARGET_BIN_LIN)

$(TARGET_BIN_OSX): $(TARGET_LIB_OSX)
	@echo "	$(CC_OSX) $(TARGET_BIN_OSX)"
	@$(CC_OSX) $(OBJ_FULL_OSX) $(LGLLIBRARIES_OSX) -o $(TARGET_BIN_OSX)

ifeq ($(PLATFORM), MINGW32_NT-5.1)
$(TARGET_BIN_W32): $(TARGET_LIB_W32)
#	@echo "	$(CC) $(TARGET_BIN_W32)"
	$(CC_W32) $(OBJ_FULL_W32) $(LGLLIBRARIES_W32) -o $(TARGET_BIN_W32)
else
$(TARGET_BIN_W32): $(TARGET_LIB_W32)
	@echo "	$(CC_W32) $(TARGET_BIN_W32)"
#	$(CC_W32) $(TARGET_LIB_W32) $(LGLLIBRARIES) -o $(TARGET_BIN_W32)
	@$(CC_W32) $(OBJ_FULL_W32) $(LGLLIBRARIES_W32) -o $(TARGET_BIN_W32)
endif



#BUILDING THE LIBRARY FILE

$(TARGET_LIB_LIN): $(MODULE_LIBS_LIN) $(OBJ_LIN)
	@echo "	$(AR_LIN) $(TARGET_LIB_LIN)"
	@$(AR_LIN) $(TARGET_LIB_LIN) $(MODULE_LIBS_LIN) $(OBJ_FULL_LIN)
ifeq ($(CREATE_BINARY), 0)
	@echo "	cp $(OBJDIR_LIN)*.o ../$(OBJDIR_LIN)"
	@cp $(OBJDIR_LIN)*.o ../$(OBJDIR_LIN)
endif

$(TARGET_LIB_OSX): $(MODULE_LIBS_OSX) $(OBJ_OSX)
	@echo "	$(AR_OSX) $(TARGET_LIB_OSX)"
	@$(AR_OSX) $(TARGET_LIB_OSX) $(MODULE_LIBS_OSX) $(OBJ_FULL_OSX)
ifeq ($(CREATE_BINARY), 0)
	@echo "	cp $(OBJDIR_OSX)*.o ../$(OBJDIR_OSX)"
	@cp $(OBJDIR_OSX)*.o ../$(OBJDIR_OSX)
endif

ifeq ($(PLATFORM), MINGW32_NT-5.1)
$(TARGET_LIB_W32): $(MODULE_LIBS_W32) $(OBJ_W32)
#	@echo "	$(AR_W32) $(TARGET_LIB_W32)"
	$(AR_W32) $(TARGET_LIB_W32) $(MODULE_LIBS_W32) $(OBJ_FULL_W32)
ifeq ($(CREATE_BINARY), 0)
#	@echo "	cp $(OBJDIR_W32)*.o ../$(OBJDIR_W32)"
	cp $(OBJDIR_W32)*.o ../$(OBJDIR_W32)
endif
else
$(TARGET_LIB_W32): $(MODULE_LIBS_W32) $(OBJ_W32)
	@echo "	$(AR_W32) $(TARGET_LIB_W32)"
	@$(AR_W32) $(TARGET_LIB_W32) $(MODULE_LIBS_W32) $(OBJ_FULL_W32)
ifeq ($(CREATE_BINARY), 0)
	@echo "cp $(OBJDIR_W32)*.o ../$(OBJDIR_W32)"
	@cp $(OBJDIR_W32)*.o ../$(OBJDIR_W32)
endif
endif



#BUILDING OBJECT FILES

$(OBJDIR_LIN)%.o: %.cpp $(PCH_LIN) $(MODULE_PCHS)
	@echo "	$(CC_LIN) $*.cpp"
	@$(CC_LIN) $(CPPFLAGS_LIN) -c $*.cpp -o $(OBJDIR_LIN)$*.o

$(OBJDIR_OSX)%.o: %.cpp $(PCH_OSX) $(MODULE_PCHS)
	@echo "	$(CC_OSX) $*.cpp"
	@$(CC_OSX) $(CPPFLAGS_OSX) -c $*.cpp -o $(OBJDIR_OSX)$*.o

ifeq ($(PLATFORM), MINGW32_NT-5.1)
$(OBJDIR_W32)%.o: %.cpp $(PCH_W32) $(MODULE_PCHS)
#	@echo "	$(CC_LIN) $*.cpp"
	$(CC_W32) $(CPPFLAGS_W32) -c $*.cpp -o $(OBJDIR_W32)$*.o
else
$(OBJDIR_W32)%.o: %.cpp $(PCH_W32) $(MODULE_PCHS)
	@echo "	$(CC_W32) $*.cpp"
	@$(CC_W32) $(CPPFLAGS_W32) -c $*.cpp -o $(OBJDIR_W32)$*.o
endif




#BUILDING PRECOMPILED HEADERS

$(OBJDIR_LIN)PrecompiledHeaders.h.gch: PrecompiledHeaders.pch
	@echo "	$(CC_LIN) PrecompiledHeaders.gch"
	@$(CC_LIN) $(CFLAGS_LIN) -x c++-header -c PrecompiledHeaders.pch -o $(OBJDIR_LIN)PrecompiledHeaders.h.gch
	@rm -f ../$(PCH_LIN)

$(OBJDIR_OSX)PrecompiledHeaders.h.gch: PrecompiledHeaders.pch
	@echo "	$(CC_OSX) PrecompiledHeaders.gch"
	@$(CC_OSX) $(CFLAGS_OSX) -x c++-header -c PrecompiledHeaders.pch -o $(OBJDIR_OSX)PrecompiledHeaders.h.gch
	@rm -f ../PrecompiledHeaders.pch

ifeq ($(PLATFORM), MINGW32_NT-5.1)
$(OBJDIR_W32)PrecompiledHeaders.h.gch: PrecompiledHeaders.pch
#	@echo "	$(CC_W32) PrecompiledHeaders.gch"
	$(CC_W32) $(CFLAGS_W32) -x c++-header -c PrecompiledHeaders.pch -o $(OBJDIR_W32)PrecompiledHeaders.h.gch
else
$(OBJDIR_W32)PrecompiledHeaders.h.gch: PrecompiledHeaders.pch
	@echo "	$(CC_W32) PrecompiledHeaders.gch"
	@$(CC_W32) $(CFLAGS_W32) -x c++-header -c PrecompiledHeaders.pch -o $(OBJDIR_W32)PrecompiledHeaders.h.gch
endif

PrecompiledHeaders.pch: $(HDR)
	@ls -1 *.h | sed "s/\(.*\)/#include \"\1\"/" > PrecompiledHeaders.pch
	


#BUILDING MODULES

#.PHONY: %.module.lin.lib %.module.osx.lib %.module.w32.lib
.PHONY: phony

%.module/$(OBJDIR_SHORT)/lin/module.lin.lib: phony
	@echo "	make $*.module"
	@make -C $*.module lin-lib

%.module/$(OBJDIR_SHORT)/osx/module.osx.lib: phony
	@echo "	make $*.module"
	@make -C $*.module osx-lib

ifeq ($(PLATFORM), MINGW32_NT-5.1)
%.module/$(OBJDIR_SHORT)/w32/module.w32.lib: phony
#	@echo "	make $*.module"
	make -C $*.module w32-lib
else
%.module/$(OBJDIR_SHORT)/w32/module.w32.lib: phony
	@echo "	make $*.module"
	@make -C $*.module w32-lib
endif


#BUILDING THE OBJECT DIRECTORIES

ifeq ($(PLATFORM), MINGW32_NT-5.1)
$(OBJDIR_SHORT):
#	@echo "	Creating $(OBJDIR_SHORT)"
	$(shell mkdir $(OBJDIR_SHORT))

$(OBJDIR_LIN_SHORT): $(OBJDIR_SHORT)
#	@echo "	Creating $(OBJDIR_LIN_SHORT)"
	$(shell mkdir $(OBJDIR_LIN_SHORT))

$(OBJDIR_OSX_SHORT): $(OBJDIR_SHORT)
#	@echo "	Creating $(OBJDIR_OSX_SHORT)"
	$(shell mkdir $(OBJDIR_OSX_SHORT))

$(OBJDIR_W32_SHORT): $(OBJDIR_SHORT)
#	@echo "	Creating $(OBJDIR_W32_SHORT)"
	$(shell mkdir $(OBJDIR_W32_SHORT)))
else
$(OBJDIR):
	@echo "	Creating $(OBJDIR)"
	@mkdir $(OBJDIR)

$(OBJDIR_LIN): $(OBJDIR)
	@echo "	Creating $(OBJDIR_LIN)"
	@mkdir $(OBJDIR_LIN)

$(OBJDIR_OSX): $(OBJDIR)
	@echo "	Creating $(OBJDIR_OSX)"
	@mkdir $(OBJDIR_OSX)

$(OBJDIR_W32): $(OBJDIR)
	@echo "	Creating $(OBJDIR_W32)"
	@mkdir $(OBJDIR_W32)
endif





#BUILDING VIM TAGS

ifeq ($(PLATFORM), MINGW32_NT-5.1)
tags: $(SRC) $(HDR)
#	echo "	Updating vim tags... (win32-native)"
#	ctags *.cpp *.h
else
tags: $(SRC) $(HDR)
	@echo "	Updating vim tags..."
	@ctags *.cpp *.h reference/*.h 2>/dev/null 1>/dev/null || ctags *.cpp *.h 2>/dev/null 1>/dev/null || ctags *.cpp 2>/dev/null 1>/dev/null
endif





#CLEAN MODULE DIRECTORIES

.PHONY: %.module.clean %.module.spotless

%.module.clean:
	@echo "	make clean $*.module"
	@make -C $*.module clean

%.module.spotless:
	@echo "	make spotless $*.module"
	@make -C $*.module spotless


#STANDARD MAKEFILE COMMANDS

.PHONY: run runall clean spotless

run: default
	./e

runall: default
	./e

ifeq ($(PLATFORM), MINGW32_NT-5.1)
clean: $(MODULE_DIRS_CLEAN)
	rm -f PrecompiledHeaders.pch $(TARGET_LIB_ALL)
	rm -f $(OBJDIR_LIN)* $(OBJDIR_OSX)* $(OBJDIR_W32)* *.lib
else
clean: $(MODULE_DIRS_CLEAN)
	@echo "	rm -f PrecompiledHeaders.pch $(TARGET_LIB_ALL)"
	@rm -f PrecompiledHeaders.pch $(TARGET_LIB_ALL)
	@echo "	rm -f $(OBJDIR_LIN)* $(OBJDIR_OSX)* $(OBJDIR_W32)* *.lib"
	@rm -f $(OBJDIR_LIN)* $(OBJDIR_OSX)* $(OBJDIR_W32)* *.lib
endif

ifeq ($(PLATFORM), MINGW32_NT-5.1)
spotless: clean $(MODULE_DIRS_SPOTLESS)
	rm -rf obj tags
ifeq ($(CREATE_BINARY), 1)
	rm -f $(TARGET_BIN_ALL)
endif
else
spotless: clean $(MODULE_DIRS_SPOTLESS)
	@echo "	rm -rf obj tags"
	@rm -rf obj tags
ifeq ($(CREATE_BINARY), 1)
	@echo "	rm -f $(TARGET_BIN_ALL)"
	@rm -f $(TARGET_BIN_ALL)
endif
endif





#Makefile.depend: *.h *.cpp
#	@echo "	makedepend"
#	@touch Makefile.depend && makedepend -fMakefile.depend *.cpp 2>/dev/null 1>/dev/null

#include Makefile.depend

