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

#include "LGL.module/LGL.h"

#include <string>

using namespace std;

void
CreateDotJackdrc();

void
CreateDotDVJTree();

void
LoadDVJRC();

void
LoadMusicRootPath();

void
LoadKeyboardInput();

void
PrepareKeyMap();

ConfigFile* dvjrcConfigFile=NULL;
ConfigFile* inputKeyboardConfigFile=NULL;
char dotDvj[2048];
char musicRootPath[2048];
char dvjSessionFlacPath[2048];
char dvjSessionTracklistPath[2048];
char dvjSessionDrawLogPath[2048];

void
ConfigInit()
{
	sprintf(dotDvj,"%s/.dvj",LGL_GetHomeDir());
	dvjSessionFlacPath[0]='\0';
	dvjSessionTracklistPath[0]='\0';
	dvjSessionDrawLogPath[0]='\0';

	CreateDotJackdrc();
	CreateDotDVJTree();
	LoadDVJRC();
	LoadMusicRootPath();
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
		fprintf(fd,"purgeInactiveMemory=false\n");
		fprintf(fd,"\n");
		fprintf(fd,"loadScreenPath=~/.dvj/data/image/loadscreen.png\n");
		fprintf(fd,"\n");
		fprintf(fd,"dvjSessionRecordAudio=1\n");
		fprintf(fd,"dvjSessionFlacPath=~/.dvj/record\n");
		fprintf(fd,"dvjSessionTracklistPath=~/Desktop/dvj_session_tracklist.txt\n");
		fprintf(fd,"dvjSessionDrawLogPath=~/Desktop/dvj_session_drawlog.txt\n");
		fprintf(fd,"\n");
		fprintf(fd,"#projectorQuadrentResX=0\n");
		fprintf(fd,"#projectorQuadrentResY=0\n");
		fprintf(fd,"\n");
		fprintf(fd,"fpsMax=60\n");
		fprintf(fd,"\n");
		fprintf(fd,"videoBufferFrames=20\n");
		fprintf(fd,"videoBufferFramesFreqSense=5\n");
		fprintf(fd,"\n");
		fprintf(fd,"#3.2 MBps is highest quality for 1920x480@30fps.\n");
		fprintf(fd,"#Generated files may be larger.\n");
		fprintf(fd,"cachedVideoAveBitrateMBps=3.2\n");
		fprintf(fd,"\n");
		fprintf(fd,"visualBrightnessAtCenter=0.5\n");
		fprintf(fd,"\n");
		fprintf(fd,"colorCoolR=0.0\n");
		fprintf(fd,"colorCoolG=0.0\n");
		fprintf(fd,"colorCoolB=0.5\n");
		fprintf(fd,"\n");
		fprintf(fd,"colorWarmR=0.4\n");
		fprintf(fd,"colorWarmG=0.2\n");
		fprintf(fd,"colorWarmB=1.0\n");
		fprintf(fd,"\n");
		fprintf(fd,"audioInPassThru=0\n");
		fprintf(fd,"wireMemory=0\n");
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
	sprintf(dvjrc,"%s/config.txt",dotDvj);

	if(LGL_FileExists(dvjrc)==false)
	{
		CreateDefaultDVJRC(dvjrc);
	}

	dvjrcConfigFile = new ConfigFile(dvjrc);
}

void
LoadMusicRootPath()
{
	sprintf(musicRootPath,"%s/Music/iTunes/iTunes Music",LGL_GetHomeDir());
	if(LGL_DirectoryExists(musicRootPath)==false)
	{
		sprintf(musicRootPath,"%s/Music",LGL_GetHomeDir());
	}
	if(LGL_DirectoryExists(musicRootPath)==false)
	{
		sprintf(musicRootPath,"%s",LGL_GetHomeDir());
	}

	if(FILE* fd=fopen(GetMusicRootConfigFilePath(),"r"))
	{
		char line[2048];
		fgets(line,2048,fd);
		if(line[0]!='\0')
		{
			if(line[strlen(line)-1]=='\n')
			{
				line[strlen(line)-1]='\0';
			}
			while(line[strlen(line)-1]=='/')
			{
				line[strlen(line)-1]='\0';
			}
			if(LGL_DirectoryExists(line))
			{
				strcpy(musicRootPath,line);
			}
		}
		fclose(fd);
	}
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
CreateDotJackdrc()
{
#ifndef LGL_OSX
	//This fn currently is written to use coreaudio, and thus only works with OSX.
	//TODO: Make this work beyond OSX.
	return;
#endif	//LGL_OSX

	//Kill any pre-existing JACK processes
	char cmd[2048];
	strcpy(cmd,"killall jackd > /dev/null 2>&1");
	system(cmd);
	LGL_DelayMS(50);
	strcpy(cmd,"killall jackd -9 > /dev/null 2>&1");
	system(cmd);
	LGL_DelayMS(50);

	//Run jackd -d coreaudio -l -d baka > /tmp/jack.output	
	const char* jackOutputPath = "/tmp/jack.output";
	bool jackdInCurrentDir = LGL_FileExists("jackd");
	sprintf(cmd,"%s -d coreaudio -l -p 0 > '%s' 2>&1",jackdInCurrentDir ? "./jackd" : "jackd",jackOutputPath);
	printf("CreateDotJackdrc(): %s\n",cmd);
	int result = system(cmd);
	if(result!=0)
	{
		printf("CreateDotJackdrc(): cmd failed: %s\n",cmd);
		return;
	}

	//Scan jack.output for string "Aggregate".
	bool aggregate=false;
	bool xponent=false;
	if(FILE* fd = fopen(jackOutputPath,"r"))
	{
		const int bufLen=2048;
		char buf[bufLen];
		for(;;)
		{
			fgets(buf,bufLen,fd);
			if(feof(fd))
			{
				break;
			}
			if(strstr(buf,"Aggregate"))
			{
				printf("JACK: Aggregate device detected!\n");
				aggregate=true;
			}
			if(strstr(buf,"Xponent"))
			{
				printf("JACK: Xponent detected!\n");
				xponent=true;
			}
			if(strstr(buf,"Default input and output devices are not the same"))
			{
				printf("JACK: Aggregate device improperly configured!\n");
				aggregate=false;
				break;
			}
		}
		fclose(fd);
	}
	else
	{
		printf("CreateDotJackdrc(): Couldn't open: %s\n",jackOutputPath);
		return;
	}

	if(xponent==false)
	{
		aggregate=false;
	}

	//Write correct ~/.jackdrc
	char dotJackdrcPath[2048];
	sprintf(dotJackdrcPath,"%s/.jackdrc",LGL_GetHomeDir());
	if(LGL_FileExists(dotJackdrcPath))
	{
		LGL_FileDelete(dotJackdrcPath);
	}

	if(FILE* fd = fopen(dotJackdrcPath,"w"))
	{
		if(aggregate)
		{
			//Don't be tempted to put ~:Aggregate:0 below in quotes. This will fail.
			fprintf(fd,"./jackd -Z -R -t5000 -d coreaudio -p 512 -d ~:Aggregate:0 -c 6 -i 2 -o 4\n");
		}
		else
		{
			fprintf(fd,"./jackd -Z -R -t5000 -d coreaudio -p 512\n");
		}
		fclose(fd);
	}

	return;
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

	char dvjrc[2048];
	sprintf(dvjrc,"%s/config.txt",dotDvj);

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
	
	char dvjMusicRoot[2048];
	sprintf(dvjMusicRoot,"%s/MusicRoot",dotDvj);
	if(LGL_DirectoryExists(dvjMusicRoot)==false)
	{
		LGL_DirectoryCreate(dvjMusicRoot);
	}

	char dvjMusicRootDVJTutorials[2048];
	sprintf(dvjMusicRootDVJTutorials,"%s/DVJ Tutorials",dvjMusicRoot);
	if(LGL_DirectoryExists(dvjMusicRootDVJTutorials)==false)
	{
		LGL_DirectoryCreate(dvjMusicRootDVJTutorials);
	}

#ifdef	LGL_OSX
	char dvjMusicRootITunes[2048];
	sprintf(dvjMusicRootITunes,"%s/iTunes",dvjMusicRoot);
	if(LGL_DirectoryExists(dvjMusicRootITunes)==false)
	{
		char iTunesPath[2048];
		sprintf(iTunesPath,"%s/Music/iTunes/iTunes Music",LGL_GetHomeDir());
		char cmd[2048];
		sprintf(cmd,"ln -s '%s' '%s'",iTunesPath,dvjMusicRootITunes);
		system(cmd);
	}
#endif	//LGL_OSX
	
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
	
	char dvjVideoRandomLow[2048];
	sprintf(dvjVideoRandomLow,"%s/low",dvjVideoRandom);
	if(LGL_DirectoryExists(dvjVideoRandomLow)==false)
	{
		LGL_DirectoryCreate(dvjVideoRandomLow);
	}
	
	char dvjVideoRandomHigh[2048];
	sprintf(dvjVideoRandomHigh,"%s/high",dvjVideoRandom);
	if(LGL_DirectoryExists(dvjVideoRandomHigh)==false)
	{
		LGL_DirectoryCreate(dvjVideoRandomHigh);
	}

	char dvjVideoTmp[2048];
	sprintf(dvjVideoTmp,"%s/tmp",dvjVideo);
	if(LGL_DirectoryExists(dvjVideoTmp)==false)
	{
		LGL_DirectoryCreate(dvjVideoTmp);
	}

	char dvjData[2048];
	sprintf(dvjData,"%s/data",dotDvj);
	if(LGL_DirectoryExists(dvjData)==false)
	{
		LGL_DirectoryCreate(dvjData);
	}

	char dvjDataImage[2048];
	sprintf(dvjDataImage,"%s/image",dvjData);
	if(LGL_DirectoryExists(dvjDataImage)==false)
	{
		LGL_DirectoryCreate(dvjDataImage);
	}

#ifdef	LGL_OSX
	char documentsDvj[2048];
	sprintf(documentsDvj,"%s/Documents/dvj",LGL_GetHomeDir());
	if(LGL_DirectoryExists(documentsDvj)==false)
	{
		char cmd[2048];
		sprintf(cmd,"ln -s '%s' '%s'",dotDvj,documentsDvj);
printf("Running cmd:\n\t'%s'\n",cmd);
		system(cmd);
	}
#endif	//LGL_OSX
}

const char*
GetMusicRootPath()
{
	return(musicRootPath);
}

const char*
GetMusicRootConfigFilePath()
{
	static char* musicRootPathConfigPath;
	if(musicRootPathConfigPath==NULL)
	{
		musicRootPathConfigPath= new char[2048];
		sprintf(musicRootPathConfigPath,"%s/musicRootPath.txt",dotDvj);
	}
	return(musicRootPathConfigPath);
}

const char*
GetDvjCacheDirName()
{
	const char* cacheName = "dvj_cache";
	return(cacheName);
}

void
SetMusicRootPath
(
	const char*	path
)
{
	if(LGL_DirectoryExists(path)==false)
	{
		return;
	}

	if(FILE* fd=fopen(GetMusicRootConfigFilePath(),"w"))
	{
		fprintf(fd,"%s\n",path);
		fclose(fd);
	}

	strcpy(musicRootPath,path);
}

bool
GetPurgeInactiveMemory()
{
	return(dvjrcConfigFile->read<bool>("purgeInactiveMemory",false));
}

void
GetLoadScreenPath
(
	char*	loadScreenPath
)
{
	const char* defaultLoadScreenPath = "~/.dvj/data/image/loadscreen.png";
	std::string pathStr = dvjrcConfigFile->read<std::string>("loadScreenPath",defaultLoadScreenPath);
	strcpy(loadScreenPath,pathStr.c_str());
	if(loadScreenPath[0]=='~')
	{
		char tmp[2048];
		sprintf(tmp,"%s/%s",LGL_GetHomeDir(),&(loadScreenPath[2]));
		strcpy(loadScreenPath,tmp);
	}

	if(char* dot = strrchr(loadScreenPath,'.'))
	{
		if(strcasecmp(dot,".png")==0)
		{
			return;
		}
	}

	strcpy(loadScreenPath,defaultLoadScreenPath);
}

bool
GetDVJSessionRecordAudio()
{
	int record=LGL_Clamp(0,dvjrcConfigFile->read<int>("dvjSessionRecordAudio",0),1);
	return(record==1);
}

const char*
GetDVJSessionFlacPath()
{
	if(dvjSessionFlacPath[0]=='\0')
	{
		const char* defaultDVJSessionFlacPath = "";//~/Desktop/dvj_session.flac";
		std::string pathStr = dvjrcConfigFile->read<std::string>("dvjSessionFlacPath",defaultDVJSessionFlacPath);
		strcpy(dvjSessionFlacPath,pathStr.c_str());
		if(dvjSessionFlacPath[0]=='~')
		{
			char tmp[2048];
			sprintf(tmp,"%s/%s",LGL_GetHomeDir(),&(dvjSessionFlacPath[2]));
			strcpy(dvjSessionFlacPath,tmp);
		}
		strcat(dvjSessionFlacPath,"/dvj_session.flac");
	}

	return(dvjSessionFlacPath);
}

const char*
GetDVJSessionTracklistPath()
{
	if(dvjSessionTracklistPath[0]=='\0')
	{
		const char* defaultDVJSessionTracklistPath = "";//~/Desktop/dvj_session_tracklist.txt";
		std::string pathStr = dvjrcConfigFile->read<std::string>("dvjSessionTracklistPath",defaultDVJSessionTracklistPath);
		strcpy(dvjSessionTracklistPath,pathStr.c_str());
		if(dvjSessionTracklistPath[0]=='~')
		{
			char tmp[2048];
			sprintf(tmp,"%s/%s",LGL_GetHomeDir(),&(dvjSessionTracklistPath[2]));
			strcpy(dvjSessionTracklistPath,tmp);
		}
	}

	return(dvjSessionTracklistPath);
}

const char*
GetDVJSessionDrawLogPath()
{
	if(dvjSessionDrawLogPath[0]=='\0')
	{
		const char* defaultDVJSessionDrawLogPath = "";//~/Desktop/dvj_session.flac";
		std::string pathStr = dvjrcConfigFile->read<std::string>("dvjSessionDrawLogPath",defaultDVJSessionDrawLogPath);
		strcpy(dvjSessionDrawLogPath,pathStr.c_str());
		if(dvjSessionDrawLogPath[0]=='~')
		{
			char tmp[2048];
			sprintf(tmp,"%s/%s",LGL_GetHomeDir(),&(dvjSessionDrawLogPath[2]));
			strcpy(dvjSessionDrawLogPath,tmp);
		}
	}

	return(dvjSessionDrawLogPath);
}

float
GetCachedVideoAveBitrateMBps()
{
	return
	(
		LGL_Clamp
		(
			0.1f,
			dvjrcConfigFile->read<float>("cachedVideoAveBitrateMBps",3.2f),
			10.0f
		)
	);
}

float
GetVisualBrightnessAtCenter()
{
	return
	(
		LGL_Clamp
		(
			0.0f,
			dvjrcConfigFile->read<float>("visualBrightnessAtCenter",1.0f),
			1.0f
		)
	);
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

int
GetProjectorQuadrentResX()
{
	int resX=dvjrcConfigFile->read<int>("projectorQuadrentResX",0);
	if(resX<=0)
	{
		resX=LGL_DisplayResolutionX(0);
	}
	return(resX);
}

int
GetProjectorQuadrentResY()
{
	int resY=dvjrcConfigFile->read<int>("projectorQuadrentResY",0);
	if(resY<=0)
	{
		resY=LGL_DisplayResolutionY(0)/2;
	}
	return(resY);
}

int
GetFPSMax()
{
	int fpsMax=LGL_Clamp(1,dvjrcConfigFile->read<int>("fpsMax",60),60);
	return(fpsMax);
}

bool
GetWireMemory()
{
	int wire=LGL_Clamp(0,dvjrcConfigFile->read<int>("wireMemory",0),1);
	return(wire!=0);
}

bool
GetAudioInPassThru()
{
	int passThru=LGL_Clamp(0,dvjrcConfigFile->read<int>("audioInPassThru",0),1);
	return(passThru!=0);
}

int
GetVideoBufferFrames()
{
	int videoBufferFrames=dvjrcConfigFile->read<int>("videoBufferFrames",20);
	if(videoBufferFrames<=2)
	{
		videoBufferFrames=2;
	}
	return(videoBufferFrames);
}

int
GetVideoBufferFramesFreqSense()
{
	int videoBufferFramesFreqSense=dvjrcConfigFile->read<int>("videoBufferFramesFreqSense",5);
	if(videoBufferFramesFreqSense<=2)
	{
		videoBufferFramesFreqSense=2;
	}
	return(videoBufferFramesFreqSense);
}

#define MAP_STRING_TO_SDLK(X) if(strcasecmp(str,#X)==0) return(X)

int
MapStringToSDLK
(
	const char* str
)
{
	MAP_STRING_TO_SDLK(LGL_KEY_UNKNOWN);
	//MAP_STRING_TO_SDLK(LGL_KEY_FIRST);
	MAP_STRING_TO_SDLK(LGL_KEY_BACKSPACE);
	MAP_STRING_TO_SDLK(LGL_KEY_TAB);
	MAP_STRING_TO_SDLK(LGL_KEY_CLEAR);
	MAP_STRING_TO_SDLK(LGL_KEY_RETURN);
	MAP_STRING_TO_SDLK(LGL_KEY_PAUSE);
	MAP_STRING_TO_SDLK(LGL_KEY_ESCAPE);
	MAP_STRING_TO_SDLK(LGL_KEY_SPACE);
	MAP_STRING_TO_SDLK(LGL_KEY_EXCLAM);
	MAP_STRING_TO_SDLK(LGL_KEY_QUOTEDBL);
	MAP_STRING_TO_SDLK(LGL_KEY_HASH);
	MAP_STRING_TO_SDLK(LGL_KEY_DOLLAR);
	MAP_STRING_TO_SDLK(LGL_KEY_AMPERSAND);
	MAP_STRING_TO_SDLK(LGL_KEY_QUOTE);
	MAP_STRING_TO_SDLK(LGL_KEY_LEFTPAREN);
	MAP_STRING_TO_SDLK(LGL_KEY_RIGHTPAREN);
	MAP_STRING_TO_SDLK(LGL_KEY_ASTERISK);
	MAP_STRING_TO_SDLK(LGL_KEY_PLUS);
	MAP_STRING_TO_SDLK(LGL_KEY_COMMA);
	MAP_STRING_TO_SDLK(LGL_KEY_MINUS);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_PERIOD);
	MAP_STRING_TO_SDLK(LGL_KEY_SLASH);
	MAP_STRING_TO_SDLK(LGL_KEY_0);
	MAP_STRING_TO_SDLK(LGL_KEY_1);
	MAP_STRING_TO_SDLK(LGL_KEY_2);
	MAP_STRING_TO_SDLK(LGL_KEY_3);
	MAP_STRING_TO_SDLK(LGL_KEY_4);
	MAP_STRING_TO_SDLK(LGL_KEY_5);
	MAP_STRING_TO_SDLK(LGL_KEY_6);
	MAP_STRING_TO_SDLK(LGL_KEY_7);
	MAP_STRING_TO_SDLK(LGL_KEY_8);
	MAP_STRING_TO_SDLK(LGL_KEY_9);
	MAP_STRING_TO_SDLK(LGL_KEY_COLON);
	MAP_STRING_TO_SDLK(LGL_KEY_SEMICOLON);
	MAP_STRING_TO_SDLK(LGL_KEY_LESS);
	MAP_STRING_TO_SDLK(LGL_KEY_EQUALS);
	MAP_STRING_TO_SDLK(LGL_KEY_PERIOD);
	MAP_STRING_TO_SDLK(LGL_KEY_GREATER);
	MAP_STRING_TO_SDLK(LGL_KEY_QUESTION);
	MAP_STRING_TO_SDLK(LGL_KEY_AT);
	//Skip uppercase letters
	MAP_STRING_TO_SDLK(LGL_KEY_LEFTBRACKET);
	MAP_STRING_TO_SDLK(LGL_KEY_BACKSLASH);
	MAP_STRING_TO_SDLK(LGL_KEY_RIGHTBRACKET);
	MAP_STRING_TO_SDLK(LGL_KEY_CARET);
	MAP_STRING_TO_SDLK(LGL_KEY_UNDERSCORE);
	MAP_STRING_TO_SDLK(LGL_KEY_BACKQUOTE);
	MAP_STRING_TO_SDLK(LGL_KEY_A);
	MAP_STRING_TO_SDLK(LGL_KEY_B);
	MAP_STRING_TO_SDLK(LGL_KEY_C);
	MAP_STRING_TO_SDLK(LGL_KEY_D);
	MAP_STRING_TO_SDLK(LGL_KEY_E);
	MAP_STRING_TO_SDLK(LGL_KEY_F);
	MAP_STRING_TO_SDLK(LGL_KEY_G);
	MAP_STRING_TO_SDLK(LGL_KEY_H);
	MAP_STRING_TO_SDLK(LGL_KEY_I);
	MAP_STRING_TO_SDLK(LGL_KEY_J);
	MAP_STRING_TO_SDLK(LGL_KEY_K);
	MAP_STRING_TO_SDLK(LGL_KEY_L);
	MAP_STRING_TO_SDLK(LGL_KEY_M);
	MAP_STRING_TO_SDLK(LGL_KEY_N);
	MAP_STRING_TO_SDLK(LGL_KEY_O);
	MAP_STRING_TO_SDLK(LGL_KEY_P);
	MAP_STRING_TO_SDLK(LGL_KEY_Q);
	MAP_STRING_TO_SDLK(LGL_KEY_R);
	MAP_STRING_TO_SDLK(LGL_KEY_S);
	MAP_STRING_TO_SDLK(LGL_KEY_T);
	MAP_STRING_TO_SDLK(LGL_KEY_U);
	MAP_STRING_TO_SDLK(LGL_KEY_V);
	MAP_STRING_TO_SDLK(LGL_KEY_W);
	MAP_STRING_TO_SDLK(LGL_KEY_X);
	MAP_STRING_TO_SDLK(LGL_KEY_Y);
	MAP_STRING_TO_SDLK(LGL_KEY_Z);
	MAP_STRING_TO_SDLK(LGL_KEY_DELETE);
	//End of ASCII mapped keysyms

	//Numeric keypad
	MAP_STRING_TO_SDLK(LGL_KEY_KP_0);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_1);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_2);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_3);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_4);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_5);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_6);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_7);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_8);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_9);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_PERIOD);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_DIVIDE);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_MULTIPLY);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_MINUS);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_PLUS);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_ENTER);
	MAP_STRING_TO_SDLK(LGL_KEY_KP_EQUALS);

	//Arrows + Home/End pad
	MAP_STRING_TO_SDLK(LGL_KEY_UP);
	MAP_STRING_TO_SDLK(LGL_KEY_DOWN);
	MAP_STRING_TO_SDLK(LGL_KEY_RIGHT);
	MAP_STRING_TO_SDLK(LGL_KEY_LEFT);
	MAP_STRING_TO_SDLK(LGL_KEY_INSERT);
	MAP_STRING_TO_SDLK(LGL_KEY_HOME);
	MAP_STRING_TO_SDLK(LGL_KEY_END);
	MAP_STRING_TO_SDLK(LGL_KEY_PAGEUP);
	MAP_STRING_TO_SDLK(LGL_KEY_PAGEDOWN);

	//Function keys
	MAP_STRING_TO_SDLK(LGL_KEY_F1);
	MAP_STRING_TO_SDLK(LGL_KEY_F2);
	MAP_STRING_TO_SDLK(LGL_KEY_F3);
	MAP_STRING_TO_SDLK(LGL_KEY_F4);
	MAP_STRING_TO_SDLK(LGL_KEY_F5);
	MAP_STRING_TO_SDLK(LGL_KEY_F6);
	MAP_STRING_TO_SDLK(LGL_KEY_F7);
	MAP_STRING_TO_SDLK(LGL_KEY_F8);
	MAP_STRING_TO_SDLK(LGL_KEY_F9);
	MAP_STRING_TO_SDLK(LGL_KEY_F10);
	MAP_STRING_TO_SDLK(LGL_KEY_F11);
	MAP_STRING_TO_SDLK(LGL_KEY_F12);
	MAP_STRING_TO_SDLK(LGL_KEY_F13);
	MAP_STRING_TO_SDLK(LGL_KEY_F14);
	MAP_STRING_TO_SDLK(LGL_KEY_F15);

	//Key state modifier keys
	MAP_STRING_TO_SDLK(LGL_KEY_NUMLOCK);
	MAP_STRING_TO_SDLK(LGL_KEY_CAPSLOCK);
	MAP_STRING_TO_SDLK(LGL_KEY_SCROLLLOCK);
	MAP_STRING_TO_SDLK(LGL_KEY_RSHIFT);
	MAP_STRING_TO_SDLK(LGL_KEY_LSHIFT);
	MAP_STRING_TO_SDLK(LGL_KEY_RCTRL);
	MAP_STRING_TO_SDLK(LGL_KEY_LCTRL);
	MAP_STRING_TO_SDLK(LGL_KEY_RALT);
	MAP_STRING_TO_SDLK(LGL_KEY_LALT);
	MAP_STRING_TO_SDLK(LGL_KEY_MODE);		//"Alt Gr" key
	MAP_STRING_TO_SDLK(LGL_KEY_APPLICATION);	//Multi-key compose key

	//Miscellaneous function keys
	MAP_STRING_TO_SDLK(LGL_KEY_HELP);
	MAP_STRING_TO_SDLK(LGL_KEY_PRINTSCREEN);
	MAP_STRING_TO_SDLK(LGL_KEY_SYSREQ);
	MAP_STRING_TO_SDLK(LGL_KEY_STOP);
	MAP_STRING_TO_SDLK(LGL_KEY_MENU);
	MAP_STRING_TO_SDLK(LGL_KEY_POWER);		//Power Macintosh power key
	MAP_STRING_TO_SDLK(LGL_KEY_EURO);		//Some european keyboards
	MAP_STRING_TO_SDLK(LGL_KEY_UNDO);		//Atari keyboard has Undo

	printf("Warning! Invalid key defined in ~/.dvj/input/keyboard.txt: '%s'\n",str);

	return(LGL_KEY_UNKNOWN);
}

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
	dvjKeyMap[NOOP].Set
		("NOOP",				"LGL_KEY_UNKNOWN");
	dvjKeyMap[FOCUS_CHANGE].Set
		("focusChange",				"LGL_KEY_TAB");
	dvjKeyMap[FOCUS_BOTTOM].Set
		("focusBottom",				"LGL_KEY_UNKNOWN");
	dvjKeyMap[FOCUS_TOP].Set
		("focusTop",				"LGL_KEY_UNKNOWN");
	dvjKeyMap[XFADER_SPEAKERS_DELTA_DOWN].Set
		("xfaderSpeakersDeltaDown",		"LGL_KEY_DELETE");
	dvjKeyMap[XFADER_SPEAKERS_DELTA_UP].Set
		("xfaderSpeakersDeltaUp",		"LGL_KEY_INSERT");
	dvjKeyMap[XFADER_HEADPHONES_DELTA_DOWN].Set
		("xfaderHeadphonesDeltaDown",		"LGL_KEY_END");
	dvjKeyMap[XFADER_HEADPHONES_DELTA_UP].Set
		("xfaderHeadphonesDeltaUp",		"LGL_KEY_HOME");
	dvjKeyMap[SYNC_TOP_TO_BOTTOM].Set
		("syncTopToBottom",			"LGL_KEY_UNKNOWN");
	dvjKeyMap[SYNC_BOTTOM_TO_TOP].Set
		("syncBottomToTop",			"LGL_KEY_UNKNOWN");
	dvjKeyMap[MASTER_TO_HEADPHONES].Set
		("masterToHeadphones",			"LGL_KEY_UNKNOWN");
	dvjKeyMap[FILE_SCROLL_DOWN_MANY].Set
		("fileScrollDownMany",			"LGL_KEY_PAGEDOWN");
	dvjKeyMap[FILE_SCROLL_UP_MANY].Set
		("fileScrollUpMany",			"LGL_KEY_PAGEUP");
	dvjKeyMap[FILE_SCROLL_DOWN_ONE].Set
		("fileScrollDownOne",			"LGL_KEY_DOWN");
	dvjKeyMap[FILE_SCROLL_UP_ONE].Set
		("fileScrollUpOne",			"LGL_KEY_UP");
	dvjKeyMap[FILE_SELECT].Set
		("fileSelect",				"LGL_KEY_RETURN");
	dvjKeyMap[FILE_MARK_UNOPENED].Set
		("fileMarkUnopened",			"LGL_KEY_BACKQUOTE");
	dvjKeyMap[FILE_REFRESH].Set
		("fileRefresh",				"LGL_KEY_F5");
	dvjKeyMap[DECODE_ABORT].Set
		("decodeAbort",				"LGL_KEY_BACKSPACE");
	dvjKeyMap[WAVEFORM_EJECT].Set
		("waveformEject",			"LGL_KEY_BACKSPACE");
	dvjKeyMap[WAVEFORM_TOGGLE_PAUSE].Set
		("waveformTogglePause",			"LGL_KEY_RETURN");
	dvjKeyMap[WAVEFORM_NUDGE_LEFT_1].Set
		("waveformNudgeLeft1",			"LGL_KEY_LEFT");
	dvjKeyMap[WAVEFORM_NUDGE_RIGHT_1].Set
		("waveformNudgeRight1",			"LGL_KEY_RIGHT");
	dvjKeyMap[WAVEFORM_NUDGE_LEFT_2].Set
		("waveformNudgeLeft2",			"LGL_KEY_R");
	dvjKeyMap[WAVEFORM_NUDGE_RIGHT_2].Set
		("waveformNudgeRight2",			"LGL_KEY_Y");
	dvjKeyMap[WAVEFORM_PITCHBEND_DELTA_DOWN_SLOW].Set
		("waveformPitchbendDeltaDownSlow",	"LGL_KEY_DOWN");
	dvjKeyMap[WAVEFORM_PITCHBEND_DELTA_UP_SLOW].Set
		("waveformPitchbendDeltaUpSlow",	"LGL_KEY_UP");
	dvjKeyMap[WAVEFORM_PITCHBEND_DELTA_DOWN_FAST].Set
		("waveformPitchbendDeltaDownFast",	"LGL_KEY_PAGEDOWN");
	dvjKeyMap[WAVEFORM_PITCHBEND_DELTA_UP_FAST].Set
		("waveformPitchbendDeltaUpFast",	"LGL_KEY_PAGEUP");
	dvjKeyMap[WAVEFORM_EQ_LOW_DELTA_DOWN].Set
		("waveformEQLowDeltaDown",		"LGL_KEY_Z");
	dvjKeyMap[WAVEFORM_EQ_LOW_DELTA_UP].Set
		("waveformEQLowDeltaUp",		"LGL_KEY_X");
	dvjKeyMap[WAVEFORM_EQ_LOW_KILL].Set
		("waveformEQLowKill",			"LGL_KEY_C");
	dvjKeyMap[WAVEFORM_EQ_MID_DELTA_DOWN].Set
		("waveformEQMidDeltaDown",		"LGL_KEY_A");
	dvjKeyMap[WAVEFORM_EQ_MID_DELTA_UP].Set
		("waveformEQMidDeltaUp",		"LGL_KEY_S");
	dvjKeyMap[WAVEFORM_EQ_MID_KILL].Set
		("waveformEQMidKill",			"LGL_KEY_D");
	dvjKeyMap[WAVEFORM_EQ_HIGH_DELTA_DOWN].Set
		("waveformEQHiDeltaDown",		"LGL_KEY_Q");
	dvjKeyMap[WAVEFORM_EQ_HIGH_DELTA_UP].Set
		("waveformEQHiDeltaUp",			"LGL_KEY_W");
	dvjKeyMap[WAVEFORM_EQ_HIGH_KILL].Set
		("waveformEQHiKill",			"LGL_KEY_E");
	dvjKeyMap[WAVEFORM_GAIN_DELTA_DOWN].Set
		("waveformGainDeltaDown",		"LGL_KEY_MINUS");
	dvjKeyMap[WAVEFORM_GAIN_DELTA_UP].Set
		("waveformGainDeltaUp",			"LGL_KEY_EQUALS");
	dvjKeyMap[WAVEFORM_GAIN_KILL].Set
		("waveformGainKill",			"LGL_KEY_UNKNOWN");
	dvjKeyMap[WAVEFORM_VOLUME_INVERT].Set
		("waveformVolumeInvert",		"LGL_KEY_PERIOD");
	dvjKeyMap[WAVEFORM_RAPID_VOLUME_INVERT].Set
		("waveformRapidVolumeInvert",		"LGL_KEY_I");
	dvjKeyMap[WAVEFORM_RAPID_SOLO_INVERT].Set
		("waveformRapidSoloInvert",		"LGL_KEY_U");
	dvjKeyMap[WAVEFORM_VOLUME_SOLO].Set
		("waveformVolumeSolo",			"LGL_KEY_COMMA");
	dvjKeyMap[WAVEFORM_REWIND].Set
		("waveformRewind",			"LGL_KEY_LEFTBRACKET");
	dvjKeyMap[WAVEFORM_FF].Set
		("waveformFF",				"LGL_KEY_RIGHTBRACKET");
	dvjKeyMap[WAVEFORM_RECORD_SPEED_BACK].Set
		("waveformRecordSpeedBack",		"LGL_KEY_SEMICOLON");
	dvjKeyMap[WAVEFORM_RECORD_SPEED_FORWARD].Set
		("waveformRecordSpeedForward",		"LGL_KEY_QUOTE");
	dvjKeyMap[WAVEFORM_STUTTER].Set
		("waveformStutter",			"LGL_KEY_UNKNOWN");
	dvjKeyMap[WAVEFORM_SAVEPOINT_PREV].Set
		("waveformSavePointPrev",		"LGL_KEY_F");
	dvjKeyMap[WAVEFORM_SAVEPOINT_NEXT].Set
		("waveformSavePointNext",		"LGL_KEY_H");
	dvjKeyMap[WAVEFORM_SAVEPOINT_SET].Set
		("waveformSavePointSet",		"LGL_KEY_G");
	dvjKeyMap[WAVEFORM_SAVEPOINT_UNSET].Set
		("waveformSavePointUnset",		"LGL_KEY_G");
	dvjKeyMap[WAVEFORM_SAVEPOINT_SHIFT_LEFT].Set
		("waveformSavePointShiftLeft",		"LGL_KEY_V");
	dvjKeyMap[WAVEFORM_SAVEPOINT_SHIFT_RIGHT].Set
		("waveformSavePointShiftRight",		"LGL_KEY_N");
	dvjKeyMap[WAVEFORM_SAVEPOINT_SHIFT_ALL_LEFT].Set
		("waveformSavePointShiftAllLeft",	"LGL_KEY_UNKNOWN");
	dvjKeyMap[WAVEFORM_SAVEPOINT_SHIFT_ALL_RIGHT].Set
		("waveformSavePointShiftAllRight",	"LGL_KEY_UNKNOWN");
	dvjKeyMap[WAVEFORM_SAVEPOINT_JUMP_NOW].Set
		("waveformSavePointJumpNow",		"LGL_KEY_B");
	dvjKeyMap[WAVEFORM_SAVEPOINT_JUMP_AT_MEASURE].Set
		("waveformSavePointJumpAtMeasure",	"LGL_KEY_T");
	dvjKeyMap[WAVEFORM_LOOP_MEASURES_HALF].Set
		("waveformLoopMeasuresHalf",		"LGL_KEY_J");
	dvjKeyMap[WAVEFORM_LOOP_MEASURES_DOUBLE].Set
		("waveformLoopMeasuresDouble",		"LGL_KEY_K");
	dvjKeyMap[WAVEFORM_LOOP_TOGGLE].Set
		("waveformLoopToggle",			"LGL_KEY_L");
	dvjKeyMap[WAVEFORM_LOOP_THEN_RECALL].Set
		("waveformLoopThenRecall",		"LGL_KEY_O");
	dvjKeyMap[WAVEFORM_AUTO_DIVERGE_THEN_RECALL].Set
		("waveformAutoDivergeThenRecall",	"LGL_KEY_M");
	dvjKeyMap[WAVEFORM_VIDEO_SELECT].Set
		("waveformVideoSelect",			"LGL_KEY_SPACE");
	dvjKeyMap[WAVEFORM_AUDIO_INPUT_MODE].Set
		("waveformAudioInputMode",		"LGL_KEY_F12");
	dvjKeyMap[WAVEFORM_VIDEO_ASPECT_RATIO_NEXT].Set
		("waveformVideoAspectRatioNext",	"LGL_KEY_SLASH");
	dvjKeyMap[WAVEFORM_SYNC_BPM].Set
		("waveformSyncBPM",			"LGL_KEY_BACKSLASH");
	dvjKeyMap[RECORDING_START].Set
		("recordingStart",			"LGL_KEY_F10");
	dvjKeyMap[FULL_SCREEN_TOGGLE].Set
		("fullScreenToggle",			"LGL_KEY_F4");
	dvjKeyMap[VISUALIZER_FULL_SCREEN_TOGGLE].Set
		("visualizerFullScreenToggle",		"LGL_KEY_F3");
	dvjKeyMap[SCREENSHOT].Set
		("screenshot",				"LGL_KEY_F2");
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
		fprintf(fd,"# For a list of all possible LGL_KEY_* keys, see:\n");
		//fprintf(fd,"# \thttp://sector7.xor.aps.anl.gov/programming/sdl/html/sdlkey.html\n");
		fprintf(fd,"# TODO... Ask interim.descriptor@gmail.com, for now\n");
		fprintf(fd,"#\n");
		fprintf(fd,"\n");
		for(int a=0;a<ACTION_LAST;a++)
		{
			if(const char* key = dvjKeyMap[a].Key)
			{
				if(const char* valueStr = dvjKeyMap[a].ValueStr)
				{
					fprintf(fd,"%s",key);
					int numSpaces = maxLength - strlen(key);
					for(int b=0;b<numSpaces;b++)
					{
						fprintf(fd, " ");
					}
					fprintf(fd," = %s\n",valueStr);
				}
			}
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
int GetInputKeyboardMasterToHeadphones()
	{ return(dvjKeyMap[MASTER_TO_HEADPHONES].ValueInt); }
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
int GetInputKeyboardWaveformGainKill()
	{ return(dvjKeyMap[WAVEFORM_GAIN_KILL].ValueInt); }
int GetInputKeyboardWaveformVolumeInvertKey()
	{ return(dvjKeyMap[WAVEFORM_VOLUME_INVERT].ValueInt); }
int GetInputKeyboardWaveformRapidVolumeInvertKey()
	{ return(dvjKeyMap[WAVEFORM_RAPID_VOLUME_INVERT].ValueInt); }
int GetInputKeyboardWaveformRapidSoloInvertKey()
	{ return(dvjKeyMap[WAVEFORM_RAPID_SOLO_INVERT].ValueInt); }
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
	{ return(dvjKeyMap[WAVEFORM_SAVEPOINT_PREV].ValueInt); }
int GetInputKeyboardWaveformSavePointNextKey()
	{ return(dvjKeyMap[WAVEFORM_SAVEPOINT_NEXT].ValueInt); }
int GetInputKeyboardWaveformSavePointSetKey()
	{ return(dvjKeyMap[WAVEFORM_SAVEPOINT_SET].ValueInt); }
int GetInputKeyboardWaveformSavePointUnsetKey()
	{ return(dvjKeyMap[WAVEFORM_SAVEPOINT_UNSET].ValueInt); }
int GetInputKeyboardWaveformSavePointShiftLeftKey()
	{ return(dvjKeyMap[WAVEFORM_SAVEPOINT_SHIFT_LEFT].ValueInt); }
int GetInputKeyboardWaveformSavePointShiftRightKey()
	{ return(dvjKeyMap[WAVEFORM_SAVEPOINT_SHIFT_RIGHT].ValueInt); }
int GetInputKeyboardWaveformSavePointShiftAllLeftKey()
	{ return(dvjKeyMap[WAVEFORM_SAVEPOINT_SHIFT_ALL_LEFT].ValueInt); }
int GetInputKeyboardWaveformSavePointShiftAllRightKey()
	{ return(dvjKeyMap[WAVEFORM_SAVEPOINT_SHIFT_ALL_RIGHT].ValueInt); }
int GetInputKeyboardWaveformSavePointJumpNowKey()
	{ return(dvjKeyMap[WAVEFORM_SAVEPOINT_JUMP_NOW].ValueInt); }
int GetInputKeyboardWaveformSavePointJumpAtMeasureKey()
	{ return(dvjKeyMap[WAVEFORM_SAVEPOINT_JUMP_AT_MEASURE].ValueInt); }
int GetInputKeyboardWaveformLoopMeasuresHalfKey()
	{ return(dvjKeyMap[WAVEFORM_LOOP_MEASURES_HALF].ValueInt); }
int GetInputKeyboardWaveformLoopMeasuresDoubleKey()
	{ return(dvjKeyMap[WAVEFORM_LOOP_MEASURES_DOUBLE].ValueInt); }
int GetInputKeyboardWaveformLoopToggleKey()
	{ return(dvjKeyMap[WAVEFORM_LOOP_TOGGLE].ValueInt); }
int GetInputKeyboardWaveformLoopThenRecallKey()
	{ return(dvjKeyMap[WAVEFORM_LOOP_THEN_RECALL].ValueInt); }
int GetInputKeyboardWaveformAutoDivergeRecallKey()
	{ return(dvjKeyMap[WAVEFORM_AUTO_DIVERGE_THEN_RECALL].ValueInt); }
int GetInputKeyboardWaveformVideoSelectKey()
	{ return(dvjKeyMap[WAVEFORM_VIDEO_SELECT].ValueInt); }
int GetInputKeyboardWaveformAudioInputModeKey()
	{ return(dvjKeyMap[WAVEFORM_AUDIO_INPUT_MODE].ValueInt); }
int GetInputKeyboardWaveformVideoAspectRatioNextKey()
	{ return(dvjKeyMap[WAVEFORM_VIDEO_ASPECT_RATIO_NEXT].ValueInt); }
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

