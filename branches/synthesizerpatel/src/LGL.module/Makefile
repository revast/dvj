include			Makefile.options

PLATFORM	= $(shell uname)

# AVCODEC_LIBS provided by ffmpeg
AVCODEC_LIBS=-lavformat -lavcodec -lswscale -lavutil

################################################################################
# LINUX 
################################################################################

ifeq ($(PLATFORM), Linux)

AR = ar rcs
CC = g++

default:	linux

CCFLAGS 	=:  -g -Wall -fsingle-precision-constant
DEFINES		=  -DLGL_LINUX
INCLUDES	=  -I$(OBJDIR) -I$(ROOT_SRC_DIR)

CFLAGS		=	$(MODEFLAGS) \
				$(DEFINES) \
				`sdl-config --cflags`
					
CXXFLAGS	=	-include PrecompiledHeaders.h $(CFLAGS_LIN)

LGL_LIBS	= 	$(AVCODEC_LIBS) \
				`sdl-config --libs` \
				`pkg-config --cflags --libs jack` \
				-lSDL_image \
				-lSDL_net \
				-lGL \
				-lfftw3f \
				-lm \
				-lsamplerate \
				-lcwiid
endif # $(PLATFORM),Linux)

################################################################################
# OSX
################################################################################

ifeq ($(PLATFORM), Darwin) # AKA OSX

AR			=	ar rcs
CC			=	g++ 
MODEFLAGS	= 	-c -g 

DEFINES		=  -DLGL_LINUX -DLGL_OSX -Dfopen64=fopen

CFLAGS		= $(MODEFLAGS) \
				$(DEFINES) \
				-I$(ROOT_SRC_DIR) \
				`sdl-config --cflags --libs` \
				-I/usr/local/include/
				
CPPFLAGS	= 	$(CFLAGS)

LIBS		=	$(AVCODEC_LIBS) \
				-L/usr/local/lib/ \
				-lSDL_image \
				-lSDL_net \
				-lSDL \
				-lsamplerate \
				-ljack \
				-lz \
				-lbz2 \
				-lfftw3f \
				-framework AppKit \
				-framework Foundation \
				-framework CoreFoundation \
				-framework OpenGL \
				-lSDLmain

endif # $(PLATFORM),Darwin)


################################################################################
# WIN32
################################################################################
ifeq ($(PLATFORM), MINGW32_NT-5.1) # AKA W32
default:		w32
lib:			w32-lib
SUFFIX_BIN		= exe
TARGET_BIN_W32	= ../$(PREFIX_BIN).$(SUFFIX_BIN)
OSNAME = win32
AR_W32			= ar rcs
CC_W32			= g++
#MODEFLAGS_W32		= -O2 -march=pentium4 -mfpmath=sse -fomit-frame-pointer -ffast-math -pipe
MODEFLAGS_W32		= -g
DEFINES_W32		= -DLGL_WIN32
CFLAGS_W32		= -Wall -I$(OBJDIR_W32) -I$(ROOT_SRC_DIR)/lib/w32/include -I$(ROOT_SRC_DIR)/lib/w32/include/SDL $(MODEFLAGS_W32) -I$(ROOT_SRC_DIR) -fsingle-precision-constant $(DEFINES_W32)
CPPFLAGS_W32		= $(CFLAGS_W32)
#AR_W32			= i586-mingw32msvc-ar rcs
#CC_W32			= i586-mingw32msvc-g++
#MODEFLAGS_W32		= -O2 -march=pentium4 -mfpmath=sse -fomit-frame-pointer -ffast-math -pipe
#MODEFLAGS		= -g
#DEFINES		= -DLGL_WIN32
#CFLAGS		= -Wall -I$(OBJDIR_W32) `/usr/local/cross-tools/i386-mingw32msvc/bin/i386-mingw32msvc-sdl-config --cflags` -Iinclude/w32 $(MODEFLAGS_W32) -I$(ROOT_SRC_DIR) -fsingle-precision-constant $(DEFINES_W32)
#CPPFLAGS	= -include PrecompiledHeaders.h $(CFLAGS_W32)
#LGLLIBRARIES = -Llib/w32 -lmingw32 -lSDLmain -lSDL -lSDL_image -lSDL_net -lglu32 -lopengl32 -mwindows
endif # ($(PLATFORM), MINGW32_NT-5.1) 


################################################################################
# General
################################################################################

SOURCES		= $(wildcard *.cpp)
HEADERS		= $(wildcard *.h)
OBJECTS		= $(patsubst %.cpp, $(OBJDIR)%.o, $(SRC))

#OBJDIR		= $(SUFFIX_BIN)
#SUFFIX_LIB  = $(SUFFIX_BIN).lib
#TARGET_LIB	= $(OBJDIR)$(default)/module.$(SUFFIX_LIB)
#PCH			= PrecompiledHeaders.h.gch
#OBJ_FULL	= $(wildcard $(OBJDIR)*.o)

################################################################################
# DEFINE ALL SUBDIRS THAT MUST BE COMPILED INTO MODULES / LIBRARIES
################################################################################
#MODULE_DIRS			= $(wildcard *.module)
#MODULE_DIRS_CLEAN	= $(patsubst %.module, %.module.clean, $(MODULE_DIRS))
#MODULE_DIRS_SPOTLESS= $(patsubst %.module, %.module.spotless, $(MODULE_DIRS))
#MODULE_LIBS			= $(patsubst %.module, %.module/$(OBJDIR)/$(OSNAME)/module.lin.lib, $(MODULE_DIRS))
#MODULE_PCHS			= $(patsubst %.module, %.module/PrecompiledHeaders.pch, $(MODULE_DIRS))

.PHONY: default all common

all: dvj

dvj: source

common: $(OBJECTS) 
	echo "COMMON"
	
source: $(SOURCES)
	$(CC) $(CPPFLAGS) -o $(patsubst %.cpp, %.o, $<) $< $(LIBS) 
	 
	
#PrecompiledHeaders.h.gch: PrecompiledHeaders.pch
#	echo "	$(CC) PrecompiledHeaders.gch"
#	$(CC) $(CFLAGS) -x c++-header -c PrecompiledHeaders.pch -o PrecompiledHeaders.h.gch
#	#rm -f ../$(PCH)
#	
#PrecompiledHeaders.pch: $(HDR)
#	@ls -1 *.h | sed "s/\(.*\)/#include \"\1\"/" > PrecompiledHeaders.pch


#
#.PHONY: all basic 
#
#all: default basic
#
#basic: tags $(OBJDIR_ALL_SHORT)
#
#ifeq ($(CREATE_BINARY), 1)
#output: lib bin
#endif
#
#lib: basic $(TARGET_LIB)
#
##BUILDING THE BINARY FILE
#
#$(TARGET_BIN): $(TARGET_LIB)
#	@echo "	$(CC) $(TARGET_BIN)"
#	@$(CC) $(CPPFLAGS) $(OBJ_FULL) $(LGLLIBRARIES) -o $(TARGET_BIN)
#
##BUILDING THE LIBRARY FILE
#
#$(TARGET_LIB): $(MODULE_LIBS) $(OBJ)
#	@echo "	$(AR) $(TARGET_LIB)"
#	@$(AR) $(TARGET_LIB) $(MODULE_LIBS) $(OBJ_FULL)
#ifeq ($(CREATE_BINARY), 0)
#	@echo "	cp $(OBJDIR)*.o ../$(OBJDIR)"
#	@cp $(OBJDIR)*.o ../$(OBJDIR)
#endif
#
##BUILDING OBJECT FILES
#
#$(OBJDIR)%.o: %.cpp $(PCH) $(MODULES)
#	@echo "	$(CC) $*.cpp"
#	@$(CC) $(CPPFLAGS) -c $*.cpp -o $(OBJDIR)$*.o
#
##BUILDING PRECOMPILED HEADERS
#
#
#PrecompiledHeaders.pch: $(HDR)
#	@ls -1 *.h | sed "s/\(.*\)/#include \"\1\"/" > PrecompiledHeaders.pch
#	
##BUILDING MODULES
#
##.PHONY: %.module.lin.lib %.module.osx.lib %.module.w32.lib
##.PHONY: phony
#
##%.module/$(OBJDIR_SHORT)/lin/module.lin.lib: phony
##	@echo "	make $*.module"
##	@make -C $*.module lin-lib
##
##%.module/$(OBJDIR_SHORT)/osx/module.osx.lib: phony
##	@echo "	make $*.module"
##	@make -C $*.module osx-lib
##
##ifeq ($(PLATFORM), MINGW32_NT-5.1)
##%.module/$(OBJDIR_SHORT)/w32/module.w32.lib: phony
###	@echo "	make $*.module"
##	make -C $*.module w32-lib
##else
##%.module/$(OBJDIR_SHORT)/w32/module.w32.lib: phony
##	@echo "	make $*.module"
##	@make -C $*.module w32-lib
##endif
#
#
##BUILDING THE OBJECT DIRECTORIES
#
#$(OBJDIR):
#	@echo "	Creating $(OBJDIR)"
#	@mkdir $(OBJDIR)
#
##BUILDING VIM TAGS
#
#ifeq ($(PLATFORM), MINGW32_NT-5.1)
#	tags: $(SRC) $(HDR)
#	#	echo "	Updating vim tags... (win32-native)"
#	#	ctags *.cpp *.h
#else
#	tags: $(SRC) $(HDR)
#		@echo "	Updating vim tags..."
#		@ctags *.cpp *.h reference/*.h 2>/dev/null 1>/dev/null || ctags *.cpp *.h 2>/dev/null 1>/dev/null || ctags *.cpp 2>/dev/null 1>/dev/null
#endif
#
###CLEAN MODULE DIRECTORIES
##
##.PHONY: %.module.clean %.module.spotless
##
##%.module.clean:
##	@echo "	make clean $*.module"
##	@make -C $*.module clean
##
##%.module.spotless:
##	@echo "	make spotless $*.module"
##	@make -C $*.module spotless
##
##
##STANDARD MAKEFILE COMMANDS
##
##.PHONY: run runall clean spotless
#
##ifeq ($(PLATFORM), MINGW32_NT-5.1)
##clean: $(MODULE_DIRS_CLEAN)
##	rm -f PrecompiledHeaders.pch $(TARGET_LIB_ALL)
##	rm -f $(OBJDIR_LIN)* $(OBJDIR)* $(OBJDIR_W32)* *.lib
##else
##clean: $(MODULE_DIRS_CLEAN)
##	@echo "	rm -f PrecompiledHeaders.pch $(TARGET_LIB_ALL)"
##	@rm -f PrecompiledHeaders.pch $(TARGET_LIB_ALL)
##	@echo "	rm -f $(OBJDIR_LIN)* $(OBJDIR)* $(OBJDIR_W32)* *.lib"
##	@rm -f $(OBJDIR_LIN)* $(OBJDIR)* $(OBJDIR)* *.lib
##endif
#
##ifeq ($(PLATFORM), MINGW32_NT-5.1)
##spotless: clean $(MODULE_DIRS_SPOTLESS)
##	rm -rf obj tags
##ifeq ($(CREATE_BINARY), 1)
##	rm -f $(TARGET_BIN_ALL)
##endif
##else
##spotless: clean $(MODULE_DIRS_SPOTLESS)
##	@echo "	rm -rf obj tags"
##	@rm -rf obj tags
##ifeq ($(CREATE_BINARY), 1)
##	@echo "	rm -f $(TARGET_BIN_ALL)"
##	@rm -f $(TARGET_BIN_ALL)
##endif
##endif
#
#
#
#
#
##Makefile.depend: *.h *.cpp
##	@echo "	makedepend"
##	@touch Makefile.depend && makedepend -fMakefile.depend *.cpp 2>/dev/null 1>/dev/null
#
##include Makefile.depend
#
