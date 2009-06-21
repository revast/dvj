
PLATFORM	= $(shell uname)

# AVCODEC_LIBS provided by ffmpeg
AVCODEC_LIBS=-lavformat -lavcodec -lswscale -lavutil

################################################################################
# LINUX 
################################################################################

ifeq ($(PLATFORM), Linux)

AR = ar rcs
CC = g++

CCFLAGS 	=  -c -Wall -fsingle-precision-constant
DEFINES		=  -DLGL_UNIX -DLGL_LINUX
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
MODEFLAGS	= 	-c

DEFINES		=  -DLGL_UNIX -DLGL_OSX -Dfopen64=fopen

CFLAGS		=	$(MODEFLAGS) \
				$(DEFINES) \
				`sdl-config --cflags` \
				-I/usr/local/include/
				
CPPFLAGS	= 	$(CFLAGS)

LIBS		=	$(AVCODEC_LIBS) \
				-L/usr/local/lib/SDL/ \
				-L/usr/local/lib/ \
				-lSDL_image \
				-lSDL_net \
				-lSDL \
				-lSDLmain \
				-lsamplerate \
				-ljack \
				-lz \
				-lbz2 \
				-lfftw3f \
				-framework AppKit \
				-framework Foundation \
				-framework OpenGL 

endif # $(PLATFORM),Darwin)


################################################################################
# WIN32
################################################################################
ifeq ($(PLATFORM), MINGW32_NT-5.1) # AKA W32

DEFINES		= 	-DLGL_WIN32
SUFFIX_BIN	= 	exe
AR			= 	ar rcs
CC			= 	g++
MODEFLAG	= 	-g -Wall
DEFINES		= 	-DLGL_WIN32
CFLAGS		= 	 $(MODEFLAGS) \
				 $(DEFINES) \
				-I$(ROOT_SRC_DIR)/lib/w32/include \
				-I$(ROOT_SRC_DIR)/lib/w32/include/SDL \
				-I$(ROOT_SRC_DIR) \
				-fsingle-precision-constant 
CPPFLAGS	= 	$(CFLAGS)
LIBS		= 	-Llib/w32 \
				-lmingw32 \
				-lSDLmain \
				-lSDL \
				-lSDL_image \
				-lSDL_net \
				-lglu32 \
				-lopengl32 \
				-mwindows
endif # ($(PLATFORM), MINGW32_NT-5.1) 


################################################################################
# General
################################################################################

SOURCES		:= $(wildcard *.cpp)
OBJECTS		:= $(subst .cpp,.o,$(SOURCES))

.PHONY: default all clean

all: dvj

dvj: $(OBJECTS)
	$(CC) -o ../dvj $(OBJECTS) $(LIBS)
	
.c.o:
	$(CC) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f *.o
