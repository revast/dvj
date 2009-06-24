
PLATFORM	= $(shell uname)

################################################################################
# GENERAL
################################################################################

AR = ar rcs

CC = g++

MODEFLAGS 	= -c 

AVCODEC_LIBS=	-lavformat -lavcodec -lswscale -lavutil # provided by ffmpeg

INCLUDES	=  -I/usr/local/include/ -ILGL -I.

LIBS		=	$(AVCODEC_LIBS) \
				`sdl-config --libs` \
				`pkg-config --cflags --libs jack` \
				-lSDL_image \
				-lSDL_net \
				-lfftw3f \
				-lsamplerate \
				
CFLAGS		=	$(MODEFLAGS) \
				$(INCLUDES) \
				$(DEFINES) \
				`sdl-config --cflags`
				
CXXFLAGS	=	$(CFLAGS)

################################################################################
# LINUX 

ifeq ($(PLATFORM), Linux)

CCFLAGS 	+=  -fsingle-precision-constant

DEFINES		+=  -DLGL_UNIX -DLGL_LINUX

LGL_LIBS	+= 	-lGL \
				-lm \
				-lcwiid
				
endif # $(PLATFORM),Linux)

################################################################################
# OSX

ifeq ($(PLATFORM), Darwin) # AKA OSX

DEFINES		+=  -DLGL_UNIX -DLGL_OSX -Dfopen64=fopen

LIBS		+=  -lz \
				-lbz2 \
				-framework AppKit \
				-framework Foundation \
				-framework OpenGL 

endif # $(PLATFORM),Darwin)


################################################################################
# WIN32
################################################################################
ifeq ($(PLATFORM), MINGW32_NT-5.1) 

SUFFIX_BIN	= 	.exe

DEFINES		= 	-DLGL_WIN32

INCLUDES	+=	-I$/lib/w32/include \
				-I$/lib/w32/include/SDL 
				
CFLAGS		= 	 $(MODEFLAGS) \
				 $(DEFINES) \
				-fsingle-precision-constant 
				
CXXFLAGS	= 	$(CFLAGS)

LIBS		= 	-Llib/w32 \
				-lmingw32 \
				-lglu32 \
				-lopengl32 \
				-mwindows
				
endif # ($(PLATFORM), MINGW32_NT-5.1) 


################################################################################
# General
################################################################################


COMMON_SRC			:=	$(wildcard *.cpp)
LGL_SRC				:=	$(wildcard LGL/*.cpp)
DISKWRITER_SRC		:=	$(wildcard tools/diskWriter/*.cpp)
FRAMERATETEST_SRC	:=	$(wildcard tools/frameRateTest/*.cpp)
IMPORTMUSICVIDEO_SRC:=	$(wildcard tools/importMusicVideo/*.cpp) 
LOGDRAWER_SRC		:=  $(wildcard tools/logDrawer/*.cpp)
VIDEOFREQMIXER_SRC	:=  $(wildcard tools/videoFreqMixer/*.cpp) 

COMMON_OBJ			:=	$(subst .cpp,.o, $(COMMON_SRC))
LGL_OBJ				:=	$(subst .cpp,.o, $(LGL_SRC))
DISKWRITER_OBJ		:=	$(subst .cpp,.o, $(DISKWRITER_SRC))
FRAMERATETEST_OBJ	:=	$(subst .cpp,.o, $(FRAMERATETEST_SRC))
IMPORTMUSICVIDEO_OBJ:=	$(subst .cpp,.o, $(IMPORTMUSICVIDEO_SRC))
LOGDRAWER_OBJ		:=  $(subst .cpp,.o, $(LOGDRAWER_SRC))
VIDEOFREQMIXER_OBJ	:=  $(subst .cpp,.o, $(VIDEOFREQMIXER_SRC))

.PHONY: all clean

default: all

all: dvj tools 

dvj: lgl common 
	$(CC) -o ../dvj$(SUFFIX_BIN) $(COMMON_OBJ) $(LGL_OBJ) $(LIBS)
	
lgl: $(LGL_OBJ)

common: $(COMMON_OBJ)

tools: diskWriter frameRateTest importMusicVideo logDrawer videoFreqMixer

diskWriter: $(DISKWRITER_OBJ)
	$(CC) -o ../$@$(SUFFIX_BIN) $? $(LGL_OBJ) $(LIBS)
	
frameRateTest: $(FRAMERATETEST_OBJ)
	$(CC) -o ../$@$(SUFFIX_BIN) $? $(LGL_OBJ) $(LIBS)
	
importMusicVideo: $(IMPORTMUSICVIDEO_OBJ)
	$(CC) -o ../$@$(SUFFIX_BIN) $? FileInterface.o Particle.o $(LGL_OBJ) $(LIBS)
	
logDrawer: $(LOGDRAWER_OBJ)
	$(CC) -o ../$@$(SUFFIX_BIN) $? FileInterface.o Particle.o $(LGL_OBJ) $(LIBS)
	
videoFreqMixer: $(VIDEOFREQMIXER_OBJ)
	$(CC) -o ../$@$(SUFFIX_BIN) $? $(LGL_OBJ) $(LIBS)
	
.c.o:
	$(CC) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(COMMON_OBJ) $(TOOLS_OBJ) $(LGL_OBJ) 