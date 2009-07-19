/*
 *
 * Config.cpp
 *
 * Copyright Chris Nelson (interim.descriptor@gmail.com), 2009
 *
 * This file is part of dvj.
 *
 * dvj is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dvj is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dvj.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "Config.h"
#include "ConfigFile.h"

#include <string>

using namespace std;

void
CreateDotDVJTree();

void
LoadDVJRC();

void
LoadKeyboardInput();

void
PrepareKeyMap();

ConfigFile* dvjrcConfigFile=NULL;
ConfigFile* inputKeyboardConfigFile=NULL;
char dotDvj[2048];

void
ConfigInit()
{
	sprintf(dotDvj,"%s/.dvj",LGL_GetHomeDir());

	CreateDotDVJTree();
	LoadDVJRC();
	LoadKeyboardInput();
}

void
CreateDefaultDVJRC
(
	const char*	path
)
{
	if(FILE* fd=fopen(path,"w"))
	{
		fprintf(fd,"#\n");
		fprintf(fd,"# dvjrc\n");
		fprintf(fd,"#\n");
		fprintf(fd,"\n");
		fprintf(fd,"colorCoolR=0.0\n");
		fprintf(fd,"colorCoolG=0.0\n");
		fprintf(fd,"colorCoolB=0.5\n");
		fprintf(fd,"\n");
		fprintf(fd,"colorWarmR=0.4\n");
		fprintf(fd,"colorWarmG=0.2\n");
		fprintf(fd,"colorWarmB=1.0\n");
		fprintf(fd,"\n");
		fclose(fd);
	}
}

void
CreateDefaultKeyboardInput
(
	const char*	path
);

void
LoadDVJRC()
{
	char dvjrc[2048];
	sprintf(dvjrc,"%s/dvjrc",dotDvj);

	if(LGL_FileExists(dvjrc)==false)
	{
		CreateDefaultDVJRC(dvjrc);
	}

	dvjrcConfigFile = new ConfigFile(dvjrc);
}

void
LoadKeyboardInput()
{
	char inputKeyboard[2048];
	sprintf(inputKeyboard,"%s/input/keyboard.txt",dotDvj);

	if(LGL_FileExists(inputKeyboard)==false)
	{
		CreateDefaultKeyboardInput(inputKeyboard);
	}

	inputKeyboardConfigFile = new ConfigFile(inputKeyboard);

	PrepareKeyMap();
}

void
CreateDotDVJTree()
{
	char dotDvj[2048];
	sprintf(dotDvj,"%s/.dvj/",LGL_GetHomeDir());
	if(LGL_DirectoryExists(dotDvj)==false)
	{
		LGL_DirectoryCreate(dotDvj);
	}
	LGL_DirectoryCreate(dotDvj);

	char dvjrc[2048];
	sprintf(dvjrc,"%s/dvjrc",dotDvj);

	if(LGL_FileExists(dvjrc)==false)
	{
		CreateDefaultDVJRC(dvjrc);
	}

	char dvjCache[2048];
	sprintf(dvjCache,"%s/cache",dotDvj);
	if(LGL_DirectoryExists(dvjCache)==false)
	{
		LGL_DirectoryCreate(dvjCache);
	}

	char dvjInput[2048];
	sprintf(dvjInput,"%s/input",dotDvj);
	if(LGL_DirectoryExists(dvjInput)==false)
	{
		LGL_DirectoryCreate(dvjInput);
	}
	char dvjMetadata[2048];
	sprintf(dvjMetadata,"%s/metadata",dotDvj);
	if(LGL_DirectoryExists(dvjMetadata)==false)
	{
		LGL_DirectoryCreate(dvjMetadata);
	}

	char dvjCacheFileLength[2048];
	sprintf(dvjCacheFileLength,"%s/fileLength",dvjCache);
	if(LGL_DirectoryExists(dvjCacheFileLength)==false)
	{
		LGL_DirectoryCreate(dvjCacheFileLength);
	}

	char dvjCacheMetadata[2048];
	sprintf(dvjCacheMetadata,"%s/metadata",dvjCache);
	if(LGL_DirectoryExists(dvjCacheMetadata)==false)
	{
		LGL_DirectoryCreate(dvjCacheMetadata);
	}

	char dvjCacheWaveArrayData[2048];
	sprintf(dvjCacheWaveArrayData,"%s/waveArrayData",dvjCache);
	if(LGL_DirectoryExists(dvjCacheWaveArrayData)==false)
	{
		LGL_DirectoryCreate(dvjCacheWaveArrayData);
	}
	
	char dvjRecord[2048];
	sprintf(dvjRecord,"%s/record",dotDvj);
	if(LGL_DirectoryExists(dvjRecord)==false)
	{
		LGL_DirectoryCreate(dvjRecord);
	}
	
	char dvjRecordOld[2048];
	sprintf(dvjRecordOld,"%s/old",dvjRecord);
	if(LGL_DirectoryExists(dvjRecordOld)==false)
	{
		LGL_DirectoryCreate(dvjRecordOld);
	}
	
	char dvjVideo[2048];
	sprintf(dvjVideo,"%s/video",dotDvj);
	if(LGL_DirectoryExists(dvjVideo)==false)
	{
		LGL_DirectoryCreate(dvjVideo);
	}

	char dvjVideoTracks[2048];
	sprintf(dvjVideoTracks,"%s/tracks",dvjVideo);
	if(LGL_DirectoryExists(dvjVideoTracks)==false)
	{
		LGL_DirectoryCreate(dvjVideoTracks);
	}

	char dvjVideoRandom[2048];
	sprintf(dvjVideoRandom,"%s/random",dvjVideo);
	if(LGL_DirectoryExists(dvjVideoRandom)==false)
	{
		LGL_DirectoryCreate(dvjVideoRandom);
	}

	char dvjVideoTmp[2048];
	sprintf(dvjVideoTmp,"%s/tmp",dvjVideo);
	if(LGL_DirectoryExists(dvjVideoTmp)==false)
	{
		LGL_DirectoryCreate(dvjVideoTmp);
	}
}

void
GetColorCool
(
	float&	r,
	float&	g,
	float&	b
)
{
	r=dvjrcConfigFile->read<float>("colorCoolR",0.0f);
	g=dvjrcConfigFile->read<float>("colorCoolG",0.0f);
	b=dvjrcConfigFile->read<float>("colorCoolB",0.5f);
}

void
GetColorWarm
(
	float&	r,
	float&	g,
	float&	b
)
{
	r=dvjrcConfigFile->read<float>("colorWarmR",0.4f);
	g=dvjrcConfigFile->read<float>("colorWarmG",0.2f);
	b=dvjrcConfigFile->read<float>("colorWarmB",1.0f);
}

#define MAP_STRING_TO_SDLK(X) if(strcasecmp(str,#X)==0) return(X)

int
MapStringToSDLK
(
	const char* str
)
{
	MAP_STRING_TO_SDLK(SDLK_UNKNOWN);
	MAP_STRING_TO_SDLK(SDLK_FIRST);
	MAP_STRING_TO_SDLK(SDLK_BACKSPACE);
	MAP_STRING_TO_SDLK(SDLK_TAB);
	MAP_STRING_TO_SDLK(SDLK_CLEAR);
	MAP_STRING_TO_SDLK(SDLK_RETURN);
	MAP_STRING_TO_SDLK(SDLK_PAUSE);
	MAP_STRING_TO_SDLK(SDLK_ESCAPE);
	MAP_STRING_TO_SDLK(SDLK_SPACE);
	MAP_STRING_TO_SDLK(SDLK_EXCLAIM);
	MAP_STRING_TO_SDLK(SDLK_QUOTEDBL);
	MAP_STRING_TO_SDLK(SDLK_HASH);
	MAP_STRING_TO_SDLK(SDLK_DOLLAR);
	MAP_STRING_TO_SDLK(SDLK_AMPERSAND);
	MAP_STRING_TO_SDLK(SDLK_QUOTE);
	MAP_STRING_TO_SDLK(SDLK_LEFTPAREN);
	MAP_STRING_TO_SDLK(SDLK_RIGHTPAREN);
	MAP_STRING_TO_SDLK(SDLK_ASTERISK);
	MAP_STRING_TO_SDLK(SDLK_PLUS);
	MAP_STRING_TO_SDLK(SDLK_COMMA);
	MAP_STRING_TO_SDLK(SDLK_MINUS);
	MAP_STRING_TO_SDLK(SDLK_PERIOD);
	MAP_STRING_TO_SDLK(SDLK_SLASH);
	MAP_STRING_TO_SDLK(SDLK_0);
	MAP_STRING_TO_SDLK(SDLK_1);
	MAP_STRING_TO_SDLK(SDLK_2);
	MAP_STRING_TO_SDLK(SDLK_3);
	MAP_STRING_TO_SDLK(SDLK_4);
	MAP_STRING_TO_SDLK(SDLK_5);
	MAP_STRING_TO_SDLK(SDLK_6);
	MAP_STRING_TO_SDLK(SDLK_7);
	MAP_STRING_TO_SDLK(SDLK_8);
	MAP_STRING_TO_SDLK(SDLK_9);
	MAP_STRING_TO_SDLK(SDLK_COLON);
	MAP_STRING_TO_SDLK(SDLK_SEMICOLON);
	MAP_STRING_TO_SDLK(SDLK_LESS);
	MAP_STRING_TO_SDLK(SDLK_EQUALS);
	MAP_STRING_TO_SDLK(SDLK_GREATER);
	MAP_STRING_TO_SDLK(SDLK_QUESTION);
	MAP_STRING_TO_SDLK(SDLK_AT);
	//Skip uppercase letters
	MAP_STRING_TO_SDLK(SDLK_LEFTBRACKET);
	MAP_STRING_TO_SDLK(SDLK_BACKSLASH);
	MAP_STRING_TO_SDLK(SDLK_RIGHTBRACKET);
	MAP_STRING_TO_SDLK(SDLK_CARET);
	MAP_STRING_TO_SDLK(SDLK_UNDERSCORE);
	MAP_STRING_TO_SDLK(SDLK_BACKQUOTE);
	MAP_STRING_TO_SDLK(SDLK_a);
	MAP_STRING_TO_SDLK(SDLK_b);
	MAP_STRING_TO_SDLK(SDLK_c);
	MAP_STRING_TO_SDLK(SDLK_d);
	MAP_STRING_TO_SDLK(SDLK_e);
	MAP_STRING_TO_SDLK(SDLK_f);
	MAP_STRING_TO_SDLK(SDLK_g);
	MAP_STRING_TO_SDLK(SDLK_h);
	MAP_STRING_TO_SDLK(SDLK_i);
	MAP_STRING_TO_SDLK(SDLK_j);
	MAP_STRING_TO_SDLK(SDLK_k);
	MAP_STRING_TO_SDLK(SDLK_l);
	MAP_STRING_TO_SDLK(SDLK_m);
	MAP_STRING_TO_SDLK(SDLK_n);
	MAP_STRING_TO_SDLK(SDLK_o);
	MAP_STRING_TO_SDLK(SDLK_p);
	MAP_STRING_TO_SDLK(SDLK_q);
	MAP_STRING_TO_SDLK(SDLK_r);
	MAP_STRING_TO_SDLK(SDLK_s);
	MAP_STRING_TO_SDLK(SDLK_t);
	MAP_STRING_TO_SDLK(SDLK_u);
	MAP_STRING_TO_SDLK(SDLK_v);
	MAP_STRING_TO_SDLK(SDLK_w);
	MAP_STRING_TO_SDLK(SDLK_x);
	MAP_STRING_TO_SDLK(SDLK_y);
	MAP_STRING_TO_SDLK(SDLK_z);
	MAP_STRING_TO_SDLK(SDLK_DELETE);
	//End of ASCII mapped keysyms

	//International keyboard syms
	MAP_STRING_TO_SDLK(SDLK_WORLD_0);		//0xA0
	MAP_STRING_TO_SDLK(SDLK_WORLD_1);
	MAP_STRING_TO_SDLK(SDLK_WORLD_2);
	MAP_STRING_TO_SDLK(SDLK_WORLD_3);
	MAP_STRING_TO_SDLK(SDLK_WORLD_4);
	MAP_STRING_TO_SDLK(SDLK_WORLD_5);
	MAP_STRING_TO_SDLK(SDLK_WORLD_6);
	MAP_STRING_TO_SDLK(SDLK_WORLD_7);
	MAP_STRING_TO_SDLK(SDLK_WORLD_8);
	MAP_STRING_TO_SDLK(SDLK_WORLD_9);
	MAP_STRING_TO_SDLK(SDLK_WORLD_10);
	MAP_STRING_TO_SDLK(SDLK_WORLD_11);
	MAP_STRING_TO_SDLK(SDLK_WORLD_12);
	MAP_STRING_TO_SDLK(SDLK_WORLD_13);
	MAP_STRING_TO_SDLK(SDLK_WORLD_14);
	MAP_STRING_TO_SDLK(SDLK_WORLD_15);
	MAP_STRING_TO_SDLK(SDLK_WORLD_16);
	MAP_STRING_TO_SDLK(SDLK_WORLD_17);
	MAP_STRING_TO_SDLK(SDLK_WORLD_18);
	MAP_STRING_TO_SDLK(SDLK_WORLD_19);
	MAP_STRING_TO_SDLK(SDLK_WORLD_20);
	MAP_STRING_TO_SDLK(SDLK_WORLD_21);
	MAP_STRING_TO_SDLK(SDLK_WORLD_22);
	MAP_STRING_TO_SDLK(SDLK_WORLD_23);
	MAP_STRING_TO_SDLK(SDLK_WORLD_24);
	MAP_STRING_TO_SDLK(SDLK_WORLD_25);
	MAP_STRING_TO_SDLK(SDLK_WORLD_26);
	MAP_STRING_TO_SDLK(SDLK_WORLD_27);
	MAP_STRING_TO_SDLK(SDLK_WORLD_28);
	MAP_STRING_TO_SDLK(SDLK_WORLD_29);
	MAP_STRING_TO_SDLK(SDLK_WORLD_30);
	MAP_STRING_TO_SDLK(SDLK_WORLD_31);
	MAP_STRING_TO_SDLK(SDLK_WORLD_32);
	MAP_STRING_TO_SDLK(SDLK_WORLD_33);
	MAP_STRING_TO_SDLK(SDLK_WORLD_34);
	MAP_STRING_TO_SDLK(SDLK_WORLD_35);
	MAP_STRING_TO_SDLK(SDLK_WORLD_36);
	MAP_STRING_TO_SDLK(SDLK_WORLD_37);
	MAP_STRING_TO_SDLK(SDLK_WORLD_38);
	MAP_STRING_TO_SDLK(SDLK_WORLD_39);
	MAP_STRING_TO_SDLK(SDLK_WORLD_40);
	MAP_STRING_TO_SDLK(SDLK_WORLD_41);
	MAP_STRING_TO_SDLK(SDLK_WORLD_42);
	MAP_STRING_TO_SDLK(SDLK_WORLD_43);
	MAP_STRING_TO_SDLK(SDLK_WORLD_44);
	MAP_STRING_TO_SDLK(SDLK_WORLD_45);
	MAP_STRING_TO_SDLK(SDLK_WORLD_46);
	MAP_STRING_TO_SDLK(SDLK_WORLD_47);
	MAP_STRING_TO_SDLK(SDLK_WORLD_48);
	MAP_STRING_TO_SDLK(SDLK_WORLD_49);
	MAP_STRING_TO_SDLK(SDLK_WORLD_50);
	MAP_STRING_TO_SDLK(SDLK_WORLD_51);
	MAP_STRING_TO_SDLK(SDLK_WORLD_52);
	MAP_STRING_TO_SDLK(SDLK_WORLD_53);
	MAP_STRING_TO_SDLK(SDLK_WORLD_54);
	MAP_STRING_TO_SDLK(SDLK_WORLD_55);
	MAP_STRING_TO_SDLK(SDLK_WORLD_56);
	MAP_STRING_TO_SDLK(SDLK_WORLD_57);
	MAP_STRING_TO_SDLK(SDLK_WORLD_58);
	MAP_STRING_TO_SDLK(SDLK_WORLD_59);
	MAP_STRING_TO_SDLK(SDLK_WORLD_60);
	MAP_STRING_TO_SDLK(SDLK_WORLD_61);
	MAP_STRING_TO_SDLK(SDLK_WORLD_62);
	MAP_STRING_TO_SDLK(SDLK_WORLD_63);
	MAP_STRING_TO_SDLK(SDLK_WORLD_64);
	MAP_STRING_TO_SDLK(SDLK_WORLD_65);
	MAP_STRING_TO_SDLK(SDLK_WORLD_66);
	MAP_STRING_TO_SDLK(SDLK_WORLD_67);
	MAP_STRING_TO_SDLK(SDLK_WORLD_68);
	MAP_STRING_TO_SDLK(SDLK_WORLD_69);
	MAP_STRING_TO_SDLK(SDLK_WORLD_70);
	MAP_STRING_TO_SDLK(SDLK_WORLD_71);
	MAP_STRING_TO_SDLK(SDLK_WORLD_72);
	MAP_STRING_TO_SDLK(SDLK_WORLD_73);
	MAP_STRING_TO_SDLK(SDLK_WORLD_74);
	MAP_STRING_TO_SDLK(SDLK_WORLD_75);
	MAP_STRING_TO_SDLK(SDLK_WORLD_76);
	MAP_STRING_TO_SDLK(SDLK_WORLD_77);
	MAP_STRING_TO_SDLK(SDLK_WORLD_78);
	MAP_STRING_TO_SDLK(SDLK_WORLD_79);
	MAP_STRING_TO_SDLK(SDLK_WORLD_80);
	MAP_STRING_TO_SDLK(SDLK_WORLD_81);
	MAP_STRING_TO_SDLK(SDLK_WORLD_82);
	MAP_STRING_TO_SDLK(SDLK_WORLD_83);
	MAP_STRING_TO_SDLK(SDLK_WORLD_84);
	MAP_STRING_TO_SDLK(SDLK_WORLD_85);
	MAP_STRING_TO_SDLK(SDLK_WORLD_86);
	MAP_STRING_TO_SDLK(SDLK_WORLD_87);
	MAP_STRING_TO_SDLK(SDLK_WORLD_88);
	MAP_STRING_TO_SDLK(SDLK_WORLD_89);
	MAP_STRING_TO_SDLK(SDLK_WORLD_90);
	MAP_STRING_TO_SDLK(SDLK_WORLD_91);
	MAP_STRING_TO_SDLK(SDLK_WORLD_92);
	MAP_STRING_TO_SDLK(SDLK_WORLD_93);
	MAP_STRING_TO_SDLK(SDLK_WORLD_94);
	MAP_STRING_TO_SDLK(SDLK_WORLD_95);		//0xFF

	//Numeric keypad
	MAP_STRING_TO_SDLK(SDLK_KP0);
	MAP_STRING_TO_SDLK(SDLK_KP1);
	MAP_STRING_TO_SDLK(SDLK_KP2);
	MAP_STRING_TO_SDLK(SDLK_KP3);
	MAP_STRING_TO_SDLK(SDLK_KP4);
	MAP_STRING_TO_SDLK(SDLK_KP5);
	MAP_STRING_TO_SDLK(SDLK_KP6);
	MAP_STRING_TO_SDLK(SDLK_KP7);
	MAP_STRING_TO_SDLK(SDLK_KP8);
	MAP_STRING_TO_SDLK(SDLK_KP9);
	MAP_STRING_TO_SDLK(SDLK_KP_PERIOD);
	MAP_STRING_TO_SDLK(SDLK_KP_DIVIDE);
	MAP_STRING_TO_SDLK(SDLK_KP_MULTIPLY);
	MAP_STRING_TO_SDLK(SDLK_KP_MINUS);
	MAP_STRING_TO_SDLK(SDLK_KP_PLUS);
	MAP_STRING_TO_SDLK(SDLK_KP_ENTER);
	MAP_STRING_TO_SDLK(SDLK_KP_EQUALS);

	//Arrows + Home/End pad
	MAP_STRING_TO_SDLK(SDLK_UP);
	MAP_STRING_TO_SDLK(SDLK_DOWN);
	MAP_STRING_TO_SDLK(SDLK_RIGHT);
	MAP_STRING_TO_SDLK(SDLK_LEFT);
	MAP_STRING_TO_SDLK(SDLK_INSERT);
	MAP_STRING_TO_SDLK(SDLK_HOME);
	MAP_STRING_TO_SDLK(SDLK_END);
	MAP_STRING_TO_SDLK(SDLK_PAGEUP);
	MAP_STRING_TO_SDLK(SDLK_PAGEDOWN);

	//Function keys
	MAP_STRING_TO_SDLK(SDLK_F1);
	MAP_STRING_TO_SDLK(SDLK_F2);
	MAP_STRING_TO_SDLK(SDLK_F3);
	MAP_STRING_TO_SDLK(SDLK_F4);
	MAP_STRING_TO_SDLK(SDLK_F5);
	MAP_STRING_TO_SDLK(SDLK_F6);
	MAP_STRING_TO_SDLK(SDLK_F7);
	MAP_STRING_TO_SDLK(SDLK_F8);
	MAP_STRING_TO_SDLK(SDLK_F9);
	MAP_STRING_TO_SDLK(SDLK_F10);
	MAP_STRING_TO_SDLK(SDLK_F11);
	MAP_STRING_TO_SDLK(SDLK_F12);
	MAP_STRING_TO_SDLK(SDLK_F13);
	MAP_STRING_TO_SDLK(SDLK_F14);
	MAP_STRING_TO_SDLK(SDLK_F15);

	//Key state modifier keys
	MAP_STRING_TO_SDLK(SDLK_NUMLOCK);
	MAP_STRING_TO_SDLK(SDLK_CAPSLOCK);
	MAP_STRING_TO_SDLK(SDLK_SCROLLOCK);
	MAP_STRING_TO_SDLK(SDLK_RSHIFT);
	MAP_STRING_TO_SDLK(SDLK_LSHIFT);
	MAP_STRING_TO_SDLK(SDLK_RCTRL);
	MAP_STRING_TO_SDLK(SDLK_LCTRL);
	MAP_STRING_TO_SDLK(SDLK_RALT);
	MAP_STRING_TO_SDLK(SDLK_LALT);
	MAP_STRING_TO_SDLK(SDLK_RMETA);
	MAP_STRING_TO_SDLK(SDLK_LMETA);
	MAP_STRING_TO_SDLK(SDLK_LSUPER);	//Left "Windows" key
	MAP_STRING_TO_SDLK(SDLK_RSUPER);	//Right "Windows" key
	MAP_STRING_TO_SDLK(SDLK_MODE);		//"Alt Gr" key
	MAP_STRING_TO_SDLK(SDLK_COMPOSE);	//Multi-key compose key

	//Miscellaneous function keys
	MAP_STRING_TO_SDLK(SDLK_HELP);
	MAP_STRING_TO_SDLK(SDLK_PRINT);
	MAP_STRING_TO_SDLK(SDLK_SYSREQ);
	MAP_STRING_TO_SDLK(SDLK_BREAK);
	MAP_STRING_TO_SDLK(SDLK_MENU);
	MAP_STRING_TO_SDLK(SDLK_POWER);		//Power Macintosh power key
	MAP_STRING_TO_SDLK(SDLK_EURO);		//Some european keyboards
	MAP_STRING_TO_SDLK(SDLK_UNDO);		//Atari keyboard has Undo

	printf("Warning! Invalid key defined in ~/.dvj/input/keyboard.txt: '%s'\n",str);

	return(SDLK_UNKNOWN);
}

typedef enum
{
	FOCUS_CHANGE = 0,
	FOCUS_BOTTOM,
	FOCUS_TOP,
	XFADER_SPEAKERS_DELTA_DOWN,
	XFADER_SPEAKERS_DELTA_UP,
	XFADER_HEADPHONES_DELTA_DOWN,
	XFADER_HEADPHONES_DELTA_UP,
	SYNC_TOP_TO_BOTTOM,
	SYNC_BOTTOM_TO_TOP,
	FILE_SCROLL_DOWN_MANY,
	FILE_SCROLL_UP_MANY,
	FILE_SCROLL_DOWN_ONE,
	FILE_SCROLL_UP_ONE,
	FILE_SELECT,
	FILE_MARK_UNOPENED,
	FILE_REFRESH,
	DECODE_ABORT,
	WAVEFORM_EJECT,
	WAVEFORM_TOGGLE_PAUSE,
	WAVEFORM_NUDGE_LEFT_1,
	WAVEFORM_NUDGE_RIGHT_1,
	WAVEFORM_NUDGE_LEFT_2,
	WAVEFORM_NUDGE_RIGHT_2,
	WAVEFORM_PITCHBEND_DELTA_DOWN_SLOW,
	WAVEFORM_PITCHBEND_DELTA_UP_SLOW,
	WAVEFORM_PITCHBEND_DELTA_DOWN_FAST,
	WAVEFORM_PITCHBEND_DELTA_UP_FAST,
	WAVEFORM_EQ_LOW_DELTA_DOWN,
	WAVEFORM_EQ_LOW_DELTA_UP,
	WAVEFORM_EQ_LOW_KILL,
	WAVEFORM_EQ_MID_DELTA_DOWN,
	WAVEFORM_EQ_MID_DELTA_UP,
	WAVEFORM_EQ_MID_KILL,
	WAVEFORM_EQ_HIGH_DELTA_DOWN,
	WAVEFORM_EQ_HIGH_DELTA_UP,
	WAVEFORM_EQ_HIGH_KILL,
	WAVEFORM_GAIN_DELTA_DOWN,
	WAVEFORM_GAIN_DELTA_UP,
	WAVEFORM_VOLUME_INVERT,
	WAVEFORM_VOLUME_SOLO,
	WAVEFORM_REWIND,
	WAVEFORM_FF,
	WAVEFORM_RECORD_SPEED_BACK,
	WAVEFORM_RECORD_SPEED_FORWARD,
	WAVEFORM_STUTTER,
	WAVEFORM_SAVE_POINT_PREV,
	WAVEFORM_SAVE_POINT_NEXT,
	WAVEFORM_SAVE_POINT_SET,
	WAVEFORM_SAVE_POINT_UNSET,
	WAVEFORM_SAVE_POINT_SHIFT_LEFT,
	WAVEFORM_SAVE_POINT_SHIFT_RIGHT,
	WAVEFORM_SAVE_POINT_SHIFT_ALL_LEFT,
	WAVEFORM_SAVE_POINT_SHIFT_ALL_RIGHT,
	WAVEFORM_SAVE_POINT_JUMP_NOW,
	WAVEFORM_SAVE_POINT_JUMP_AT_MEASURE,
	WAVEFORM_VIDEO_SELECT,
	WAVEFORM_VIDEO_FREQ_SENSE_MODE,
	WAVEFORM_SYNC_BPM,
	RECORDING_START,
	FULL_SCREEN_TOGGLE,
	VISUALIZER_FULL_SCREEN_TOGGLE,
	SCREENSHOT,
	ACTION_LAST
} DVJ_Action;

class dvjKeyboardMapObj
{

public:

	void		Set(const char* inKey, const char* inValueStr)
			{
				Key=inKey;
				ValueStr=inValueStr;

				if(MapStringToSDLK(ValueStr)==-1)
				{
					printf("ERROR! Programmer populated dvjKeyboardMapObj with invalid value '%s'\n",ValueStr);
					exit(-1);
				}

				string str=ValueStr;
				if(inputKeyboardConfigFile)
				{
					str = inputKeyboardConfigFile->read<string>
					(
						Key,
						ValueStr
					);
				}
				ValueInt=MapStringToSDLK(str.c_str());
			}

	const char*	Key;
	const char*	ValueStr;
	int		ValueInt;
};

dvjKeyboardMapObj dvjKeyMap[ACTION_LAST];

void
PrepareKeyMap()
{
	dvjKeyMap[FOCUS_CHANGE].Set
		("focusChange",				"SDLK_TAB");
	dvjKeyMap[FOCUS_BOTTOM].Set
		("focusBottom",				"SDLK_UNKNOWN");
	dvjKeyMap[FOCUS_TOP].Set
		("focusTop",				"SDLK_UNKNOWN");
	dvjKeyMap[XFADER_SPEAKERS_DELTA_DOWN].Set
		("xfaderSpeakersDeltaDown",		"SDLK_DELETE");
	dvjKeyMap[XFADER_SPEAKERS_DELTA_UP].Set
		("xfaderSpeakersDeltaUp",		"SDLK_INSERT");
	dvjKeyMap[XFADER_HEADPHONES_DELTA_DOWN].Set
		("xfaderHeadphonesDeltaDown",		"SDLK_END");
	dvjKeyMap[XFADER_HEADPHONES_DELTA_UP].Set
		("xfaderHeadphonesDeltaUp",		"SDLK_HOME");
	dvjKeyMap[SYNC_TOP_TO_BOTTOM].Set
		("syncTopToBottom",			"SDLK_UNKNOWN");
	dvjKeyMap[SYNC_BOTTOM_TO_TOP].Set
		("syncBottomToTop",			"SDLK_UNKNOWN");
	dvjKeyMap[FILE_SCROLL_DOWN_MANY].Set
		("fileScrollDownMany",			"SDLK_PAGEDOWN");
	dvjKeyMap[FILE_SCROLL_UP_MANY].Set
		("fileScrollUpMany",			"SDLK_PAGEUP");
	dvjKeyMap[FILE_SCROLL_DOWN_ONE].Set
		("fileScrollDownOne",			"SDLK_DOWN");
	dvjKeyMap[FILE_SCROLL_UP_ONE].Set
		("fileScrollUpOne",			"SDLK_UP");
	dvjKeyMap[FILE_SELECT].Set
		("fileSelect",				"SDLK_RETURN");
	dvjKeyMap[FILE_MARK_UNOPENED].Set
		("fileMarkUnopened",			"SDLK_BACKQUOTE");
	dvjKeyMap[FILE_REFRESH].Set
		("fileRefresh",				"SDLK_F5");
	dvjKeyMap[DECODE_ABORT].Set
		("decodeAbort",				"SDLK_BACKSPACE");
	dvjKeyMap[WAVEFORM_EJECT].Set
		("waveformEject",			"SDLK_BACKSPACE");
	dvjKeyMap[WAVEFORM_TOGGLE_PAUSE].Set
		("waveformTogglePause",			"SDLK_RETURN");
	dvjKeyMap[WAVEFORM_NUDGE_LEFT_1].Set
		("waveformNudgeLeft1",			"SDLK_LEFT");
	dvjKeyMap[WAVEFORM_NUDGE_RIGHT_1].Set
		("waveformNudgeRight1",			"SDLK_RIGHT");
	dvjKeyMap[WAVEFORM_NUDGE_LEFT_2].Set
		("waveformNudgeLeft2",			"SDLK_r");
	dvjKeyMap[WAVEFORM_NUDGE_RIGHT_2].Set
		("waveformNudgeRight2",			"SDLK_y");
	dvjKeyMap[WAVEFORM_PITCHBEND_DELTA_DOWN_SLOW].Set
		("waveformPitchbendDeltaDownSlow",	"SDLK_DOWN");
	dvjKeyMap[WAVEFORM_PITCHBEND_DELTA_UP_SLOW].Set
		("waveformPitchbendDeltaUpSlow",	"SDLK_UP");
	dvjKeyMap[WAVEFORM_PITCHBEND_DELTA_DOWN_FAST].Set
		("waveformPitchbendDeltaDownFast",	"SDLK_PAGEDOWN");
	dvjKeyMap[WAVEFORM_PITCHBEND_DELTA_UP_FAST].Set
		("waveformPitchbendDeltaUpFast",	"SDLK_PAGEUP");
	dvjKeyMap[WAVEFORM_EQ_LOW_DELTA_DOWN].Set
		("waveformEQLowDeltaDown",		"SDLK_z");
	dvjKeyMap[WAVEFORM_EQ_LOW_DELTA_UP].Set
		("waveformEQLowDeltaUp",		"SDLK_x");
	dvjKeyMap[WAVEFORM_EQ_LOW_KILL].Set
		("waveformEQLowKill",			"SDLK_c");
	dvjKeyMap[WAVEFORM_EQ_MID_DELTA_DOWN].Set
		("waveformEQMidDeltaDown",		"SDLK_a");
	dvjKeyMap[WAVEFORM_EQ_MID_DELTA_UP].Set
		("waveformEQMidDeltaUp",		"SDLK_s");
	dvjKeyMap[WAVEFORM_EQ_MID_KILL].Set
		("waveformEQMidKill",			"SDLK_d");
	dvjKeyMap[WAVEFORM_EQ_HIGH_DELTA_DOWN].Set
		("waveformEQHiDeltaDown",		"SDLK_q");
	dvjKeyMap[WAVEFORM_EQ_HIGH_DELTA_UP].Set
		("waveformEQHiDeltaUp",			"SDLK_w");
	dvjKeyMap[WAVEFORM_EQ_HIGH_KILL].Set
		("waveformEQHiKill",			"SDLK_e");
	dvjKeyMap[WAVEFORM_GAIN_DELTA_DOWN].Set
		("waveformGainDeltaDown",		"SDLK_MINUS");
	dvjKeyMap[WAVEFORM_GAIN_DELTA_UP].Set
		("waveformGainDeltaUp",			"SDLK_EQUALS");
	dvjKeyMap[WAVEFORM_VOLUME_INVERT].Set
		("waveformVolumeInvert",		"SDLK_PERIOD");
	dvjKeyMap[WAVEFORM_VOLUME_SOLO].Set
		("waveformVolumeSolo",			"SDLK_COMMA");
	dvjKeyMap[WAVEFORM_REWIND].Set
		("waveformRewind",			"SDLK_LEFTBRACKET");
	dvjKeyMap[WAVEFORM_FF].Set
		("waveformFF",				"SDLK_RIGHTBRACKET");
	dvjKeyMap[WAVEFORM_RECORD_SPEED_BACK].Set
		("waveformRecordSpeedBack",		"SDLK_SEMICOLON");
	dvjKeyMap[WAVEFORM_RECORD_SPEED_FORWARD].Set
		("waveformRecordSpeedForward",		"SDLK_QUOTE");
	dvjKeyMap[WAVEFORM_STUTTER].Set
		("waveformStutter",			"SDLK_UNKNOWN");
	dvjKeyMap[WAVEFORM_SAVE_POINT_PREV].Set
		("waveformSavePointPrev",		"SDLK_f");
	dvjKeyMap[WAVEFORM_SAVE_POINT_NEXT].Set
		("waveformSavePointNext",		"SDLK_h");
	dvjKeyMap[WAVEFORM_SAVE_POINT_SET].Set
		("waveformSavePointSet",		"SDLK_g");
	dvjKeyMap[WAVEFORM_SAVE_POINT_UNSET].Set
		("waveformSavePointUnset",		"SDLK_g");
	dvjKeyMap[WAVEFORM_SAVE_POINT_SHIFT_LEFT].Set
		("waveformSavePointShiftLeft",		"SDLK_v");
	dvjKeyMap[WAVEFORM_SAVE_POINT_SHIFT_RIGHT].Set
		("waveformSavePointShiftRight",		"SDLK_n");
	dvjKeyMap[WAVEFORM_SAVE_POINT_SHIFT_ALL_LEFT].Set
		("waveformSavePointShiftAllLeft",	"SDLK_UNKNOWN");
	dvjKeyMap[WAVEFORM_SAVE_POINT_SHIFT_ALL_RIGHT].Set
		("waveformSavePointShiftAllRight",	"SDLK_UNKNOWN");
	dvjKeyMap[WAVEFORM_SAVE_POINT_JUMP_NOW].Set
		("waveformSavePointJumpNow",		"SDLK_b");
	dvjKeyMap[WAVEFORM_SAVE_POINT_JUMP_AT_MEASURE].Set
		("waveformSavePointJumpAtMeasure",	"SDLK_t");
	dvjKeyMap[WAVEFORM_VIDEO_SELECT].Set
		("waveformVideoSelect",			"SDLK_SPACE");
	dvjKeyMap[WAVEFORM_VIDEO_FREQ_SENSE_MODE].Set
		("waveformVideoFreqSenseMode",		"SDLK_SLASH");
	dvjKeyMap[WAVEFORM_SYNC_BPM].Set
		("waveformSyncBPM",			"SDLK_BACKSLASH");
	dvjKeyMap[RECORDING_START].Set
		("recordingStart",			"SDLK_F10");
	dvjKeyMap[FULL_SCREEN_TOGGLE].Set
		("fullScreenToggle",			"SDLK_F4");
	dvjKeyMap[VISUALIZER_FULL_SCREEN_TOGGLE].Set
		("visualizerFullScreenToggle",		"SDLK_F3");
	dvjKeyMap[SCREENSHOT].Set
		("screenshot",				"SDLK_F2");
}

void
CreateDefaultKeyboardInput
(
	const char*	path
)
{
	//Prime dvjKeyMap with default values
	PrepareKeyMap();

	int maxLength=0;
	for(int a=0;a<ACTION_LAST;a++)
	{
		maxLength=LGL_Max(strlen(dvjKeyMap[a].Key),maxLength);
	}

	if(FILE* fd=fopen(path,"w"))
	{
		fprintf(fd,"#\n");
		fprintf(fd,"# keyboard.txt\n");
		fprintf(fd,"#\n");
		fprintf(fd,"# For a list of all possible SDLK_* keys, see:\n");
		fprintf(fd,"# \thttp://sector7.xor.aps.anl.gov/programming/sdl/html/sdlkey.html\n");
		fprintf(fd,"#\n");
		fprintf(fd,"\n");
		for(int a=0;a<ACTION_LAST;a++)
		{
			fprintf(fd,"%s",dvjKeyMap[a].Key);
			int numSpaces = maxLength - strlen(dvjKeyMap[a].Key);
			for(int b=0;b<numSpaces;b++)
			{
				fprintf(fd, " ");
			}
			fprintf(fd," = %s\n",dvjKeyMap[a].ValueStr);
		}
		fprintf(fd,"\n");
		fclose(fd);
	}
}

int GetInputKeyboardFocusChangeKey()
	{ return(dvjKeyMap[FOCUS_CHANGE].ValueInt); }
int GetInputKeyboardFocusBottomKey()
	{ return(dvjKeyMap[FOCUS_BOTTOM].ValueInt); }
int GetInputKeyboardFocusTopKey()
	{ return(dvjKeyMap[FOCUS_TOP].ValueInt); }
int GetInputKeyboardXfaderSpeakersDeltaDownKey()
	{ return(dvjKeyMap[XFADER_SPEAKERS_DELTA_DOWN].ValueInt); }
int GetInputKeyboardXfaderSpeakersDeltaUpKey()
	{ return(dvjKeyMap[XFADER_SPEAKERS_DELTA_UP].ValueInt); }
int GetInputKeyboardXfaderHeadphonesDeltaDownKey()
	{ return(dvjKeyMap[XFADER_HEADPHONES_DELTA_DOWN].ValueInt); }
int GetInputKeyboardXfaderHeadphonesDeltaUpKey()
	{ return(dvjKeyMap[XFADER_HEADPHONES_DELTA_UP].ValueInt); }
int GetInputKeyboardSyncTopToBottomKey()
	{ return(dvjKeyMap[SYNC_TOP_TO_BOTTOM].ValueInt); }
int GetInputKeyboardSyncBottomToTopKey()
	{ return(dvjKeyMap[SYNC_BOTTOM_TO_TOP].ValueInt); }
int GetInputKeyboardFileScrollDownManyKey()
	{ return(dvjKeyMap[FILE_SCROLL_DOWN_MANY].ValueInt); }
int GetInputKeyboardFileScrollUpManyKey()
	{ return(dvjKeyMap[FILE_SCROLL_UP_MANY].ValueInt); }
int GetInputKeyboardFileScrollDownOneKey()
	{ return(dvjKeyMap[FILE_SCROLL_DOWN_ONE].ValueInt); }
int GetInputKeyboardFileScrollUpOneKey()
	{ return(dvjKeyMap[FILE_SCROLL_UP_ONE].ValueInt); }
int GetInputKeyboardFileSelectKey()
	{ return(dvjKeyMap[FILE_SELECT].ValueInt); }
int GetInputKeyboardFileMarkUnopenedKey()
	{ return(dvjKeyMap[FILE_MARK_UNOPENED].ValueInt); }
int GetInputKeyboardFileRefreshKey()
	{ return(dvjKeyMap[FILE_REFRESH].ValueInt); }
int GetInputKeyboardDecodeAbortKey()
	{ return(dvjKeyMap[DECODE_ABORT].ValueInt); }
int GetInputKeyboardWaveformEjectKey()
	{ return(dvjKeyMap[WAVEFORM_EJECT].ValueInt); }
int GetInputKeyboardWaveformTogglePauseKey()
	{ return(dvjKeyMap[WAVEFORM_TOGGLE_PAUSE].ValueInt); }
int GetInputKeyboardWaveformNudgeLeft1Key()
	{ return(dvjKeyMap[WAVEFORM_NUDGE_LEFT_1].ValueInt); }
int GetInputKeyboardWaveformNudgeRight1Key()
	{ return(dvjKeyMap[WAVEFORM_NUDGE_RIGHT_1].ValueInt); }
int GetInputKeyboardWaveformNudgeLeft2Key()
	{ return(dvjKeyMap[WAVEFORM_NUDGE_LEFT_2].ValueInt); }
int GetInputKeyboardWaveformNudgeRight2Key()
	{ return(dvjKeyMap[WAVEFORM_NUDGE_RIGHT_2].ValueInt); }
int GetInputKeyboardWaveformPitchbendDeltaDownSlowKey()
	{ return(dvjKeyMap[WAVEFORM_PITCHBEND_DELTA_DOWN_SLOW].ValueInt); }
int GetInputKeyboardWaveformPitchbendDeltaUpSlowKey()
	{ return(dvjKeyMap[WAVEFORM_PITCHBEND_DELTA_UP_SLOW].ValueInt); }
int GetInputKeyboardWaveformPitchbendDeltaDownFastKey()
	{ return(dvjKeyMap[WAVEFORM_PITCHBEND_DELTA_DOWN_FAST].ValueInt); }
int GetInputKeyboardWaveformPitchbendDeltaUpFastKey()
	{ return(dvjKeyMap[WAVEFORM_PITCHBEND_DELTA_UP_FAST].ValueInt); }
int GetInputKeyboardWaveformEQLowDeltaDownKey()
	{ return(dvjKeyMap[WAVEFORM_EQ_LOW_DELTA_DOWN].ValueInt); }
int GetInputKeyboardWaveformEQLowDeltaUpKey()
	{ return(dvjKeyMap[WAVEFORM_EQ_LOW_DELTA_UP].ValueInt); }
int GetInputKeyboardWaveformEQLowKillKey()
	{ return(dvjKeyMap[WAVEFORM_EQ_LOW_KILL].ValueInt); }
int GetInputKeyboardWaveformEQMidDeltaDownKey()
	{ return(dvjKeyMap[WAVEFORM_EQ_MID_DELTA_DOWN].ValueInt); }
int GetInputKeyboardWaveformEQMidDeltaUpKey()
	{ return(dvjKeyMap[WAVEFORM_EQ_MID_DELTA_UP].ValueInt); }
int GetInputKeyboardWaveformEQMidKillKey()
	{ return(dvjKeyMap[WAVEFORM_EQ_MID_KILL].ValueInt); }
int GetInputKeyboardWaveformEQHighDeltaDownKey()
	{ return(dvjKeyMap[WAVEFORM_EQ_HIGH_DELTA_DOWN].ValueInt); }
int GetInputKeyboardWaveformEQHighDeltaUpKey()
	{ return(dvjKeyMap[WAVEFORM_EQ_HIGH_DELTA_UP].ValueInt); }
int GetInputKeyboardWaveformEQHighKillKey()
	{ return(dvjKeyMap[WAVEFORM_EQ_HIGH_KILL].ValueInt); }
int GetInputKeyboardWaveformGainDeltaDownKey()
	{ return(dvjKeyMap[WAVEFORM_GAIN_DELTA_DOWN].ValueInt); }
int GetInputKeyboardWaveformGainDeltaUpKey()
	{ return(dvjKeyMap[WAVEFORM_GAIN_DELTA_UP].ValueInt); }
int GetInputKeyboardWaveformVolumeInvertKey()
	{ return(dvjKeyMap[WAVEFORM_VOLUME_INVERT].ValueInt); }
int GetInputKeyboardWaveformVolumeSoloKey()
	{ return(dvjKeyMap[WAVEFORM_VOLUME_SOLO].ValueInt); }
int GetInputKeyboardWaveformRewindKey()
	{ return(dvjKeyMap[WAVEFORM_REWIND].ValueInt); }
int GetInputKeyboardWaveformFFKey()
	{ return(dvjKeyMap[WAVEFORM_FF].ValueInt); }
int GetInputKeyboardWaveformRecordSpeedBackKey()
	{ return(dvjKeyMap[WAVEFORM_RECORD_SPEED_BACK].ValueInt); }
int GetInputKeyboardWaveformRecordSpeedForwardKey()
	{ return(dvjKeyMap[WAVEFORM_RECORD_SPEED_FORWARD].ValueInt); }
int GetInputKeyboardWaveformStutterKey()
	{ return(dvjKeyMap[WAVEFORM_STUTTER].ValueInt); }
int GetInputKeyboardWaveformSavePointPrevKey()
	{ return(dvjKeyMap[WAVEFORM_SAVE_POINT_PREV].ValueInt); }
int GetInputKeyboardWaveformSavePointNextKey()
	{ return(dvjKeyMap[WAVEFORM_SAVE_POINT_NEXT].ValueInt); }
int GetInputKeyboardWaveformSavePointSetKey()
	{ return(dvjKeyMap[WAVEFORM_SAVE_POINT_SET].ValueInt); }
int GetInputKeyboardWaveformSavePointUnsetKey()
	{ return(dvjKeyMap[WAVEFORM_SAVE_POINT_UNSET].ValueInt); }
int GetInputKeyboardWaveformSavePointShiftLeftKey()
	{ return(dvjKeyMap[WAVEFORM_SAVE_POINT_SHIFT_LEFT].ValueInt); }
int GetInputKeyboardWaveformSavePointShiftRightKey()
	{ return(dvjKeyMap[WAVEFORM_SAVE_POINT_SHIFT_RIGHT].ValueInt); }
int GetInputKeyboardWaveformSavePointShiftAllLeftKey()
	{ return(dvjKeyMap[WAVEFORM_SAVE_POINT_SHIFT_ALL_LEFT].ValueInt); }
int GetInputKeyboardWaveformSavePointShiftAllRightKey()
	{ return(dvjKeyMap[WAVEFORM_SAVE_POINT_SHIFT_ALL_RIGHT].ValueInt); }
int GetInputKeyboardWaveformSavePointJumpNowKey()
	{ return(dvjKeyMap[WAVEFORM_SAVE_POINT_JUMP_NOW].ValueInt); }
int GetInputKeyboardWaveformSavePointJumpAtMeasureKey()
	{ return(dvjKeyMap[WAVEFORM_SAVE_POINT_JUMP_AT_MEASURE].ValueInt); }
int GetInputKeyboardWaveformVideoSelectKey()
	{ return(dvjKeyMap[WAVEFORM_VIDEO_SELECT].ValueInt); }
int GetInputKeyboardWaveformVideoFreqSenseModeKey()
	{ return(dvjKeyMap[WAVEFORM_VIDEO_FREQ_SENSE_MODE].ValueInt); }
int GetInputKeyboardWaveformSyncBPMKey()
	{ return(dvjKeyMap[WAVEFORM_SYNC_BPM].ValueInt); }
int GetInputKeyboardRecordingStartKey()
	{ return(dvjKeyMap[RECORDING_START].ValueInt); }
int GetInputKeyboardFullScreenToggleKey()
	{ return(dvjKeyMap[FULL_SCREEN_TOGGLE].ValueInt); }
int GetInputKeyboardVisualizerFullScreenToggleKey()
	{ return(dvjKeyMap[VISUALIZER_FULL_SCREEN_TOGGLE].ValueInt); }
int GetInputKeyboardScreenshotKey()
	{ return(dvjKeyMap[SCREENSHOT].ValueInt); }

//int GetInputKeyboardWaveformKey() { return(dvjKeyMap[WAVEFORM_].ValueInt); }

