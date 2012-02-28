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

#include "LGL.module/LGL.h"

#include "Visualizer.h"

using namespace std;

void
CreateDefaultDVJRC
(
	const char*	path
);

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
PrepareInputMap();

void
LoadOscInput();

ConfigFile* dvjrcConfigFile=NULL;
ConfigFile* inputKeyboardConfigFile=NULL;
ConfigFile* inputOscConfigFile=NULL;
char dotDvj[2048];
char musicRootPath[2048];
char* musicRootPathOverride=NULL;
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

	LoadDVJRC();
	CreateDotJackdrc();
	CreateDotDVJTree();
	LoadMusicRootPath();
	LoadKeyboardInput();
	LoadOscInput();
}

void
CreateDefaultDVJRC
(
	const char*	path
)
{
	CreateDotDVJTree();

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
		fprintf(fd,"dvjSessionTracklistPath=~/.dvj/record/dvj_session_tracklist.txt\n");
		fprintf(fd,"dvjSessionDrawLogPath=~/.dvj/record/dvj_session_drawlog.txt\n");
		fprintf(fd,"\n");
		fprintf(fd,"#projectorQuadrentResX=0\n");
		fprintf(fd,"#projectorQuadrentResY=0\n");
		fprintf(fd,"\n");
		fprintf(fd,"#projMapSimple=1\n");
		fprintf(fd,"#projMapGridSideLengthX=2\n");
		fprintf(fd,"#projMapGridSideLengthY=2\n");
		fprintf(fd,"\n");
		fprintf(fd,"fpsMax=60\n");
		fprintf(fd,"audioSamplePeriod=512\n");
		fprintf(fd,"audioMaxLengthMinutes=20\n");
		fprintf(fd,"\n");
		fprintf(fd,"drawTurntablePreviews=1\n");
		fprintf(fd,"videoBufferFrames=120\n");
		fprintf(fd,"videoBufferFramesFreqSense=2\n");
		fprintf(fd,"preloadVideoMaxMB=0\n");
		fprintf(fd,"preloadFreqSenseMaxMB=0\n");
		fprintf(fd,"\n");
		fprintf(fd,"#3.2 MBps is highest quality for 1920x480@30fps.\n");
		fprintf(fd,"#Generated files may be larger.\n");
		fprintf(fd,"cachedVideoAveBitrateMBps=3.2\n");
		fprintf(fd,"\n");
		fprintf(fd,"visualBrightnessAtCenter=1.0\n");
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
		fprintf(fd,"alwaysUseAudioInForFreqSense=0\n");
		fprintf(fd,"wireMemory=1\n");
		fprintf(fd,"escDuringScanExits=1\n");
		fprintf(fd,"audioSwapOutputStreams=0\n");
		fprintf(fd,"debugVideoCaching=0\n");
		fprintf(fd,"\n");
		fprintf(fd,"#Fader Options (Fader00-Fader11 valid):\n");
		fprintf(fd,"#NULL\n");
		fprintf(fd,"#LowEQ\n");
		fprintf(fd,"#MidEQ\n");
		fprintf(fd,"#HighEQ\n");
		fprintf(fd,"#Gain\n");
		fprintf(fd,"#Video\n");
		fprintf(fd,"#VideoFreqSense\n");
		fprintf(fd,"#Syphon\n");
		fprintf(fd,"#Oscilloscope\n");
		fprintf(fd,"#LEDFreqSense\n");
		fprintf(fd,"#LEDColorLow\n");
		fprintf(fd,"#LEDColorHigh\n");
		fprintf(fd,"#LEDColorHighWash\n");
		fprintf(fd,"#LEDGroup\n");
		fprintf(fd,"\n");
		fprintf(fd,"Fader00=LowEQ\n");
		fprintf(fd,"Fader01=MidEQ\n");
		fprintf(fd,"Fader02=HighEQ\n");
		fprintf(fd,"Fader03=Gain\n");
		fprintf(fd,"Fader04=Video\n");
		fprintf(fd,"Fader05=VideoFreqSense\n");
		fprintf(fd,"Fader06=Syphon\n");
		fprintf(fd,"Fader07=LEDFreqSense\n");
		fprintf(fd,"Fader08=LEDColorLow\n");
		fprintf(fd,"Fader09=LEDColorHigh\n");
		fprintf(fd,"Fader10=LEDColorHighWash\n");
		fprintf(fd,"Fader11=LEDGroup\n");
		fprintf(fd,"\n");
		fprintf(fd,"syphonServerEnabled=0\n");
		fprintf(fd,"hideProjectorWindows=0\n");
		fprintf(fd,"\n");
		fprintf(fd,"ledClient_000_Host = localhost\n");
		fprintf(fd,"ledClient_000_Port = 0\n");
		fprintf(fd,"ledClient_000_Channel = 0\n");
		fprintf(fd,"ledClient_000_Group = 0\n");
		fprintf(fd,"\n");
		fprintf(fd,"ledClient_001_Host = localhost\n");
		fprintf(fd,"ledClient_001_Port = 0\n");
		fprintf(fd,"ledClient_001_Channel = 0\n");
		fprintf(fd,"ledClient_001_Group = 0\n");
		fprintf(fd,"\n");
		fprintf(fd,"# You may add up to ledClient_0255, if you so desire.\n");
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
CreateDefaultOscInput
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
	char inputKeyboardTxt[2048];
	sprintf(inputKeyboardTxt,"%s/input/keyboard.txt",dotDvj);

	if(LGL_FileExists(inputKeyboardTxt)==false)
	{
		CreateDefaultKeyboardInput(inputKeyboardTxt);
	}

	inputKeyboardConfigFile = new ConfigFile(inputKeyboardTxt);

	PrepareInputMap();
}

void
LoadOscInput()
{
	char inputOscTxt[2048];
	sprintf(inputOscTxt,"%s/input/osc.txt",dotDvj);

	if(LGL_FileExists(inputOscTxt)==false)
	{
		CreateDefaultOscInput(inputOscTxt);
	}

	inputOscConfigFile = new ConfigFile(inputOscTxt);

	PrepareInputMap();
}

void
CreateDotJackdrc()
{
#ifndef LGL_OSX
	//This fn currently is written to use coreaudio, and thus only works with OSX.
	//TODO: Make this work beyond OSX.
	return;
#endif	//LGL_OSX

	setenv("JACK_DRIVER_DIR","./lib/jack",1);

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
	if(LGL_FileExists(jackOutputPath))
	{
		LGL_FileDelete(jackOutputPath);
	}
	printf("CreateDotJackdrc(): %s\n",cmd);
	int result = system(cmd);
	if(result!=0)
	{
		if(LGL_FileExists(jackOutputPath)==false)
		{
			printf("CreateDotJackdrc(): cmd failed: %s\n",cmd);
			return;
		}
	}

	//Scan jack.output for string "Aggregate".
	bool aggregate=false;
	bool xponent=false;
	bool numarkdjio=false;
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
			if(strstr(buf,"Numark"))
			{
				printf("JACK: Numark DJ IO device detected!\n");
				numarkdjio=true;
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

	int samplePeriod=GetAudioSamplePeriod();

	if(FILE* fd = fopen(dotJackdrcPath,"w"))
	{
		if(aggregate)
		{
			//Don't be tempted to put ~:Aggregate:0 below in quotes. This will fail.
			fprintf(fd,"./jackd -Z -R -t9999 -d coreaudio -p %i -d ~:Aggregate:0 -c 6 -i 2 -o 4\n",samplePeriod);
		}
		else if(numarkdjio)
		{
			fprintf(fd,"./jackd -Z -R -t9999 -d coreaudio -p %i -o 4\n",samplePeriod);
		}
		else
		{
			fprintf(fd,"./jackd -Z -R -t9999 -d coreaudio -p %i -o 2\n",samplePeriod);
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
	if(musicRootPathOverride)
	{
		return(musicRootPathOverride);
	}
	else
	{
		return(musicRootPath);
	}
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

void
SetMusicRootPathOverride
(
	const char*	path
)
{
	if(musicRootPathOverride)
	{
		delete musicRootPathOverride;
		musicRootPathOverride=NULL;
	}

	if
	(
		path==NULL ||
		path[0]=='\0'
	)
	{
		return;
	}

	musicRootPathOverride=new char[strlen(path)+1];
	strcpy(musicRootPathOverride,path);
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

float colorCoolR=-1.0f;
float colorCoolG=-1.0f;
float colorCoolB=-1.0f;

void
GetColorCool
(
	float&	r,
	float&	g,
	float&	b
)
{
	if(colorCoolR==-1.0f)
	{
		colorCoolR=dvjrcConfigFile->read<float>("colorCoolR",0.0f);
		colorCoolG=dvjrcConfigFile->read<float>("colorCoolG",0.0f);
		colorCoolB=dvjrcConfigFile->read<float>("colorCoolB",0.5f);
	}

	r=colorCoolR;
	g=colorCoolG;
	b=colorCoolB;
}

float colorWarmR=-1.0f;
float colorWarmG=-1.0f;
float colorWarmB=-1.0f;

void
GetColorWarm
(
	float&	r,
	float&	g,
	float&	b
)
{
	if(colorWarmR==-1.0f)
	{
		colorWarmR=dvjrcConfigFile->read<float>("colorWarmR",0.4f);
		colorWarmG=dvjrcConfigFile->read<float>("colorWarmG",0.2f);
		colorWarmB=dvjrcConfigFile->read<float>("colorWarmB",1.0f);
	}

	r=colorWarmR;
	g=colorWarmG;
	b=colorWarmB;
}

float
GetProjectorQuadrentDefaultAspectRatioX()
{
	float aspX=dvjrcConfigFile->read<float>("projectorQuadrentDefaultAspectRatioX",0);
	return(aspX);
}

float
GetProjectorQuadrentDefaultAspectRatioY()
{
	float aspY=dvjrcConfigFile->read<float>("projectorQuadrentDefaultAspectRatioY",0);
	return(aspY);
}

bool
GetProjMapSimple()
{
	int simple=dvjrcConfigFile->read<int>("projMapSimple",1);
	return(simple!=0);
}

int
GetProjMapGridSideLengthX()
{
	int sideLength=dvjrcConfigFile->read<int>("projMapGridSideLengthX",2);
	if(sideLength<=2)
	{
		sideLength=2;
	}
	return(sideLength);
}

int
GetProjMapGridSideLengthY()
{
	int sideLength=dvjrcConfigFile->read<int>("projMapGridSideLengthY",2);
	if(sideLength<=2)
	{
		sideLength=2;
	}
	return(sideLength);
}

int
GetFPSMax()
{
	int fpsMax=LGL_Clamp(1,dvjrcConfigFile->read<int>("fpsMax",60),60);
	return(fpsMax);
}

int
GetAudioSamplePeriod()
{
	int period=LGL_Clamp(32,dvjrcConfigFile->read<int>("audioSamplePeriod",512),2048);
	return(period);
}

int
GetAudioMaxLengthMinutes()
{
	int minutes=LGL_Clamp(1,dvjrcConfigFile->read<int>("audioMaxLengthMinutes",20),1000);
	return(minutes);
}

bool
GetDrawTurntablePreviews()
{
	int previews=LGL_Clamp(0,dvjrcConfigFile->read<int>("drawTurntablePreviews",1),1);
	return(previews==1);
}

bool
GetWireMemory()
{
	int wire=LGL_Clamp(0,dvjrcConfigFile->read<int>("wireMemory",0),1);
	return(wire!=0);
}

bool
GetEscDuringScanExits()
{
	int exits=LGL_Clamp(0,dvjrcConfigFile->read<int>("escDuringScanExits",1),1);
	return(exits!=0);
}

bool
GetAudioSwapOutputStreams()
{
	int swap=LGL_Clamp(0,dvjrcConfigFile->read<int>("audioSwapOutputStreams",0),1);
	return(swap!=0);
}

bool
GetAudioInPassThru()
{
	int passThru=LGL_Clamp(0,dvjrcConfigFile->read<int>("audioInPassThru",0),1);
	return(passThru!=0);
}

bool
GetConfigAlwaysUseAudioInForFreqSense()
{
	int passThru=LGL_Clamp(0,dvjrcConfigFile->read<int>("alwaysUseAudioInForFreqSense",0),1);
	return(passThru!=0);
}

int
GetVideoBufferFrames()
{
	int videoBufferFrames=dvjrcConfigFile->read<int>("videoBufferFrames",120);
	if(videoBufferFrames<=2)
	{
		videoBufferFrames=2;
	}
	return(videoBufferFrames);
}

int
GetPreloadVideoMaxMB()
{
	int maxMB=dvjrcConfigFile->read<int>("preloadVideoMaxMB",0);
	return(maxMB);
}

int
GetPreloadFreqSenseMaxMB()
{
	int maxMB=dvjrcConfigFile->read<int>("preloadFreqSenseMaxMB",0);
	return(maxMB);
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
);

int
MapStringToSDLK
(
	const char* str
)
{
	MAP_STRING_TO_SDLK(LGL_KEY_UNKNOWN);
	MAP_STRING_TO_SDLK(LGL_KEY_NONE);
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

class dvjInputMapObj
{

public:

	void		Set
			(
				const char*	inActionStr,
				bool		inEachTurntable,
				const char*	inValueStr,
				const char*	inOscDefaultStr
			)
			{
				ActionGlobalStr=inActionStr;
				EachTurntable=inEachTurntable;
				KeyboardStr=inValueStr;
				OscDefaultStr=inOscDefaultStr;

				sprintf(ActionTT0Str,"%s:TT0",ActionGlobalStr);
				sprintf(ActionTT1Str,"%s:TT1",ActionGlobalStr);
				sprintf(ActionTTFocusStr,"%s:TTFocus",ActionGlobalStr);

				if(MapStringToSDLK(KeyboardStr)==-1)
				{
					printf("ERROR! Programmer populated dvjInputMapObj with invalid value '%s'\n",KeyboardStr);
					exit(-1);
				}

				string str=KeyboardStr;
				if(inputKeyboardConfigFile)
				{
					str = inputKeyboardConfigFile->read<string>
					(
						ActionGlobalStr,
						KeyboardStr
					);
				}
				KeyboardInt=MapStringToSDLK(str.c_str());

				//OSC
				strcpy(OscDefaultAddressPatternGlobal,"");
				strcpy(OscDefaultAddressPatternTT0,"");
				strcpy(OscDefaultAddressPatternTT1,"");
				strcpy(OscDefaultAddressPatternTTFocus,"");
				strcpy(OscUserAddressPatternGlobal,"");
				strcpy(OscUserAddressPatternTT0,"");
				strcpy(OscUserAddressPatternTT1,"");
				strcpy(OscUserAddressPatternTTFocus,"");

				if(OscDefaultStr[0]!='\0')
				{
					//Default
					if(EachTurntable)
					{
						sprintf(OscDefaultAddressPatternTT0,"/dvj/tt_0/%s",OscDefaultStr);
						sprintf(OscDefaultAddressPatternTT1,"/dvj/tt_1/%s",OscDefaultStr);
						//sprintf(OscDefaultAddressPatternTTFocus,"/dvj/tt_focus/%s",OscDefaultStr);
					}
					else
					{
						sprintf(OscDefaultAddressPatternGlobal,"/dvj/global/%s",OscDefaultStr);
					}

					//User
					if(inputOscConfigFile)
					{
						std::string tmpStdString;
						char tmpStr[2048];

						if(EachTurntable)
						{
							//TT0
							{
								tmpStdString = inputOscConfigFile->read<std::string>(ActionTT0Str,"");
								strcpy(tmpStr,tmpStdString.c_str());
								if
								(
									tmpStr[0]!='\0' &&
									strcmp(tmpStr,OscDefaultAddressPatternTT0)!=0
								)
								{
									strcpy(OscUserAddressPatternTT0,tmpStr);
								}
							}
							//TT1
							{
								tmpStdString = inputOscConfigFile->read<std::string>(ActionTT1Str,"");
								strcpy(tmpStr,tmpStdString.c_str());
								if
								(
									tmpStr[0]!='\0' &&
									strcmp(tmpStr,OscDefaultAddressPatternTT1)!=0
								)
								{
									strcpy(OscUserAddressPatternTT1,tmpStr);
								}
							}
							//TTFocus
							/*
							{
								tmpStdString = inputOscConfigFile->read<std::string>(ActionTTFocusStr,"");
								strcpy(tmpStr,tmpStdString.c_str());
								if
								(
									tmpStr[0]!='\0' &&
									strcmp(tmpStr,OscDefaultAddressPatternTTFocus)!=0
								)
								{
									strcpy(OscUserAddressPatternTTFocus,tmpStr);
								}
							}
							*/
						}
						else
						{
							//Global
							{
								tmpStdString = inputOscConfigFile->read<std::string>(ActionGlobalStr,"");
								strcpy(tmpStr,tmpStdString.c_str());
								if
								(
									tmpStr[0]!='\0' &&
									strcmp(tmpStr,OscDefaultAddressPatternGlobal)!=0
								)
								{
									strcpy(OscUserAddressPatternGlobal,tmpStr);
								}
							}
						}
					}
				}
			}
	
	std::vector<const char*>
			GetOscAddressPatternList(int target)
			{
				std::vector<const char*> ret;
/*
printf("Default Global: %s\n",OscDefaultAddressPatternGlobal);
printf("Default TT0   : %s\n",OscDefaultAddressPatternTT0);
printf("Default TT1   : %s\n",OscDefaultAddressPatternTT1);
//printf("Default Focus : %s\n",OscDefaultAddressPatternTTFocus);
printf("User Global   : %s\n",OscUserAddressPatternGlobal);
printf("User TT0      : %s\n",OscUserAddressPatternTT0);
printf("User TT1      : %s\n",OscUserAddressPatternTT1);
//printf("User Focus    : %s\n",OscUserAddressPatternTTFocus);
*/

				if(EachTurntable)
				{
					if(target & TARGET_TOP)
					{
						if(OscDefaultAddressPatternTT0[0]!='\0')
						{
							ret.push_back(OscDefaultAddressPatternTT0);
						}
						if(OscUserAddressPatternTT0[0]!='\0')
						{
							ret.push_back(OscUserAddressPatternTT0);
						}
					}
					if(target & TARGET_BOTTOM)
					{
						if(OscDefaultAddressPatternTT1)
						{
							ret.push_back(OscDefaultAddressPatternTT1);
						}
						if(OscUserAddressPatternTT1)
						{
							ret.push_back(OscUserAddressPatternTT1);
						}
					}
					/*
					if(target & TARGET_FOCUS)
					{
						if(OscDefaultAddressPatternTTFocus[0]!='\0')
						{
							ret.push_back(OscDefaultAddressPatternTTFocus);
						}
						if(OscUserAddressPatternTTFocus[0]!='\0')
						{
							ret.push_back(OscUserAddressPatternTTFocus);
						}
					}
					*/
				}
				else
				{
					if(OscDefaultAddressPatternGlobal[0]!='\0')
					{
						ret.push_back(OscDefaultAddressPatternGlobal);
					}
					if(OscUserAddressPatternGlobal[0]!='\0')
					{
						ret.push_back(OscUserAddressPatternGlobal);
					}
				}

				return(ret);
			}

	const char*	ActionGlobalStr;
	char		ActionTT0Str[1024];
	char		ActionTT1Str[1024];
	char		ActionTTFocusStr[1024];
	bool		EachTurntable;
	const char*	KeyboardStr;
	int		KeyboardInt;
	const char*	OscDefaultStr;
	char		OscDefaultAddressPatternGlobal[512];
	char		OscDefaultAddressPatternTT0[512];
	char		OscDefaultAddressPatternTT1[512];
	char		OscDefaultAddressPatternTTFocus[512];
	char		OscUserAddressPatternGlobal[2048];
	char		OscUserAddressPatternTT0[2048];
	char		OscUserAddressPatternTT1[2048];
	char		OscUserAddressPatternTTFocus[2048];
};

dvjInputMapObj dvjInputMap[ACTION_LAST];

void
PrepareInputMap()
{
	dvjInputMap[NOOP].Set
		("NOOP",				false,	"LGL_KEY_UNKNOWN",	"");
	dvjInputMap[FOCUS_CHANGE].Set
		("focusChange",				false,	"LGL_KEY_TAB",		"focus_change");
	dvjInputMap[FOCUS_BOTTOM].Set
		("focusBottom",				false,	"LGL_KEY_UNKNOWN",	"");
	dvjInputMap[FOCUS_TOP].Set
		("focusTop",				false,	"LGL_KEY_UNKNOWN",	"");
	dvjInputMap[XFADER_SPEAKERS].Set
		("xfaderSpeakers",			false,	"LGL_KEY_NONE",		"xfader_speakers");
	dvjInputMap[XFADER_SPEAKERS_DELTA_DOWN].Set
		("xfaderSpeakersDeltaDown",		false,	"LGL_KEY_DELETE",	"");
	dvjInputMap[XFADER_SPEAKERS_DELTA_UP].Set
		("xfaderSpeakersDeltaUp",		false,	"LGL_KEY_INSERT",	"");
	dvjInputMap[XFADER_HEADPHONES].Set
		("xfaderHeadphones",			false,	"LGL_KEY_NONE",		"xfader_headphones");
	dvjInputMap[XFADER_HEADPHONES_DELTA_DOWN].Set
		("xfaderHeadphonesDeltaDown",		false,	"LGL_KEY_END",		"");
	dvjInputMap[XFADER_HEADPHONES_DELTA_UP].Set
		("xfaderHeadphonesDeltaUp",		false,	"LGL_KEY_HOME",		"");
	dvjInputMap[MASTER_TO_HEADPHONES].Set
		("masterToHeadphones",			false,	"LGL_KEY_NONE",		"");
	dvjInputMap[FILE_SCROLL].Set
		("fileScroll",				true,	"LGL_KEY_NONE",		"file/scroll");
	dvjInputMap[FILE_SCROLL_DOWN_MANY].Set
		("fileScrollDownMany",			true,	"LGL_KEY_PAGEDOWN",	"");
	dvjInputMap[FILE_SCROLL_UP_MANY].Set
		("fileScrollUpMany",			true,	"LGL_KEY_PAGEUP",	"");
	dvjInputMap[FILE_SCROLL_PREV].Set
		("fileScrollPrev",			true,	"LGL_KEY_UP",		"file/scroll/prev");
	dvjInputMap[FILE_SCROLL_NEXT].Set
		("fileScrollNext",			true,	"LGL_KEY_DOWN",		"file/scroll/next");
	dvjInputMap[FILE_SELECT].Set
		("fileSelect",				true,	"LGL_KEY_RETURN",	"file/select");
	dvjInputMap[FILE_MARK_UNOPENED].Set
		("fileMarkUnopened",			true,	"LGL_KEY_BACKQUOTE",	"file/mark_unopened");
	dvjInputMap[FILE_REFRESH].Set
		("fileRefresh",				true,	"LGL_KEY_NONE",		"");
	dvjInputMap[WAVEFORM_EJECT].Set
		("waveformEject",			true,	"LGL_KEY_BACKSPACE",	"eject");
	dvjInputMap[WAVEFORM_PAUSE_TOGGLE].Set
		("waveformPauseToggle",			true,	"LGL_KEY_RETURN",	"pause/toggle");
	dvjInputMap[WAVEFORM_NUDGE].Set
		("waveformNudge",			true,	"LGL_KEY_NONE",		"nudge");
	dvjInputMap[WAVEFORM_NUDGE_BACKWARD].Set
		("waveformNudgeBackward",		true,	"LGL_KEY_LEFT",		"nudge/backward");
	dvjInputMap[WAVEFORM_NUDGE_FORWARD].Set
		("waveformNudgeForward",		true,	"LGL_KEY_RIGHT",	"nudge/forward");
	dvjInputMap[WAVEFORM_PITCHBEND].Set
		("waveformPitchbend",			true,	"LGL_KEY_NONE",		"pitchbend");
	dvjInputMap[WAVEFORM_PITCHBEND_DELTA_DOWN_SLOW].Set
		("waveformPitchbendDeltaLowerSlow",	true,	"LGL_KEY_UNKNOWN",	"");	//"pitchbend/lower/slow");
	dvjInputMap[WAVEFORM_PITCHBEND_DELTA_UP_SLOW].Set
		("waveformPitchbendDeltaHigherSlow",	true,	"LGL_KEY_UNKNOWN",	"");	//"pitchbend/higher/slow");
	dvjInputMap[WAVEFORM_PITCHBEND_DELTA_DOWN].Set
		("waveformPitchbendDeltaLower",		true,	"LGL_KEY_DOWN",		"");	//"pitchbend/lower");
	dvjInputMap[WAVEFORM_PITCHBEND_DELTA_UP].Set
		("waveformPitchbendDeltaHigher",	true,	"LGL_KEY_UP",		"")	;//"pitchbend/higher");
	dvjInputMap[WAVEFORM_PITCHBEND_DELTA_DOWN_FAST].Set
		("waveformPitchbendDeltaLowerFast",	true,	"LGL_KEY_PAGEDOWN",	"");	//"pitchbend/lower/fast");
	dvjInputMap[WAVEFORM_PITCHBEND_DELTA_UP_FAST].Set
		("waveformPitchbendDeltaHigherFast",	true,	"LGL_KEY_PAGEUP",	"");	//"pitchbend/higher/fast");
	dvjInputMap[WAVEFORM_EQ_LOW].Set
		("waveformEQLow",			true,	"LGL_KEY_NONE",		"eq/low");
	dvjInputMap[WAVEFORM_EQ_LOW_DELTA_DOWN].Set
		("waveformEQLowDeltaDown",		true,	"LGL_KEY_UNKNOWN",	"");
	dvjInputMap[WAVEFORM_EQ_LOW_DELTA_UP].Set
		("waveformEQLowDeltaUp",		true,	"LGL_KEY_UNKNOWN",	"");
	dvjInputMap[WAVEFORM_EQ_LOW_KILL].Set
		("waveformEQLowKill",			true,	"LGL_KEY_UNKNOWN",	"eq/low/kill");
	dvjInputMap[WAVEFORM_EQ_MID].Set
		("waveformEQMid",			true,	"LGL_KEY_NONE",		"eq/mid");
	dvjInputMap[WAVEFORM_EQ_MID_DELTA_DOWN].Set
		("waveformEQMidDeltaDown",		true,	"LGL_KEY_UNKNOWN",	"");
	dvjInputMap[WAVEFORM_EQ_MID_DELTA_UP].Set
		("waveformEQMidDeltaUp",		true,	"LGL_KEY_UNKNOWN",	"");
	dvjInputMap[WAVEFORM_EQ_MID_KILL].Set
		("waveformEQMidKill",			true,	"LGL_KEY_UNKNOWN",	"eq/mid/kill");
	dvjInputMap[WAVEFORM_EQ_HIGH].Set
		("waveformEQHigh",			true,	"LGL_KEY_NONE",		"eq/high");
	dvjInputMap[WAVEFORM_EQ_HIGH_DELTA_DOWN].Set
		("waveformEQHiDeltaDown",		true,	"LGL_KEY_UNKNOWN",	"");
	dvjInputMap[WAVEFORM_EQ_HIGH_DELTA_UP].Set
		("waveformEQHiDeltaUp",			true,	"LGL_KEY_UNKNOWN",	"");
	dvjInputMap[WAVEFORM_EQ_HIGH_KILL].Set
		("waveformEQHiKill",			true,	"LGL_KEY_UNKNOWN",	"eq/high/kill");
	dvjInputMap[WAVEFORM_GAIN].Set
		("waveformGain",			true,	"LGL_KEY_NONE",		"gain");
	dvjInputMap[WAVEFORM_GAIN_DELTA_DOWN].Set
		("waveformGainDeltaDown",		true,	"LGL_KEY_MINUS",	"");
	dvjInputMap[WAVEFORM_GAIN_DELTA_UP].Set
		("waveformGainDeltaUp",			true,	"LGL_KEY_EQUALS",	"");
	dvjInputMap[WAVEFORM_GAIN_KILL].Set
		("waveformGainKill",			true,	"LGL_KEY_UNKNOWN",	"gain/kill");
	dvjInputMap[WAVEFORM_VOLUME].Set
		("waveformVolume",			true,	"LGL_KEY_NONE",		"volume");
	dvjInputMap[WAVEFORM_VOLUME_INVERT].Set
		("waveformVolumeInvert",		true,	"LGL_KEY_SPACE",	"volume_invert");
	dvjInputMap[WAVEFORM_RHYTHMIC_VOLUME_INVERT].Set
		("waveformRhymthicVolumeInvert",	true,	"LGL_KEY_I",		"rhythmic_volume_invert");
	dvjInputMap[WAVEFORM_RHYTHMIC_VOLUME_INVERT_OTHER].Set
		("waveformRhythmicVolumeInvertOther",	true,	"LGL_KEY_U",		"rhythmic_volume_invert_other");
	dvjInputMap[WAVEFORM_VOLUME_SOLO].Set
		("waveformVolumeSolo",			true,	"LGL_KEY_UNKNOWN",	"volume_solo");
	dvjInputMap[WAVEFORM_SEEK].Set
		("waveformSeek",			true,	"LGL_KEY_NONE",		"seek");
	dvjInputMap[WAVEFORM_SEEK_BACKWARD_SLOW].Set
		("waveformSeekBackwardSlow",		true,	"LGL_KEY_SEMICOLON",	"seek/backward/slow");
	dvjInputMap[WAVEFORM_SEEK_BACKWARD_FAST].Set
		("waveformSeekBackwardFast",		true,	"LGL_KEY_LEFTBRACKET",	"seek/backward/fast");
	dvjInputMap[WAVEFORM_SEEK_FORWARD_SLOW].Set
		("waveformSeekForwardSlow",		true,	"LGL_KEY_QUOTE",	"seek/forward/slow");
	dvjInputMap[WAVEFORM_SEEK_FORWARD_FAST].Set
		("waveformSeekForwardFast",		true,	"LGL_KEY_RIGHTBRACKET",	"seek/forward/fast");
	dvjInputMap[WAVEFORM_SCRATCH_SPEED].Set
		("waveformScratchSpeed",		true,	"LGL_KEY_NONE",		"scratch/speed");
	dvjInputMap[WAVEFORM_SAVEPOINT_PREV].Set
		("waveformSavePointPrev",		true,	"LGL_KEY_F",		"save_point/prev");
	dvjInputMap[WAVEFORM_SAVEPOINT_NEXT].Set
		("waveformSavePointNext",		true,	"LGL_KEY_H",		"save_point/next");
	dvjInputMap[WAVEFORM_SAVEPOINT_SET].Set
		("waveformSavePointSet",		true,	"LGL_KEY_G",		"save_point/set");
	dvjInputMap[WAVEFORM_SAVEPOINT_SHIFT_BACKWARD].Set
		("waveformSavePointShiftBackward",	true,	"LGL_KEY_R",		"save_point/shift/backward");
	dvjInputMap[WAVEFORM_SAVEPOINT_SHIFT_FORWARD].Set
		("waveformSavePointShiftForward",	true,	"LGL_KEY_Y",		"save_point/shift/forward");
	dvjInputMap[WAVEFORM_SAVEPOINT_SHIFT_ALL_BACKWARD].Set
		("waveformSavePointShiftAllBackward",	true,	"LGL_KEY_UNKNOWN",	"save_point/shift_all/backward");
	dvjInputMap[WAVEFORM_SAVEPOINT_SHIFT_ALL_FORWARD].Set
		("waveformSavePointShiftAllForward",	true,	"LGL_KEY_UNKNOWN",	"save_point/shift_all/forward");
	dvjInputMap[WAVEFORM_SAVEPOINT_JUMP_NOW].Set
		("waveformSavePointJumpNow",		true,	"LGL_KEY_T",		"save_point/jump/now");
	dvjInputMap[WAVEFORM_SAVEPOINT_JUMP_AT_MEASURE].Set
		("waveformSavePointJumpAtMeasure",	true,	"LGL_KEY_B",		"save_point/jump/at_measure");
	dvjInputMap[WAVEFORM_QUANTIZATION_PERIOD_HALF].Set
		("waveformQuantizationPeriodHalf",	true,	"LGL_KEY_J",		"quantization_period/half");
	dvjInputMap[WAVEFORM_QUANTIZATION_PERIOD_DOUBLE].Set
		("waveformQuantizationPeriodDouble",	true,	"LGL_KEY_K",		"quantization_period/double");
	dvjInputMap[WAVEFORM_STUTTER].Set
		("waveformStutter",			true,	"LGL_KEY_NONE",		"");
	dvjInputMap[WAVEFORM_LOOP_TOGGLE].Set
		("waveformLoopToggle",			true,	"LGL_KEY_L",		"loop/toggle");
	dvjInputMap[WAVEFORM_LOOP_THEN_RECALL].Set
		("waveformLoopThenRecall",		true,	"LGL_KEY_O",		"loop_then_recall");
	dvjInputMap[WAVEFORM_REVERSE].Set
		("waveformReverse",			true,	"LGL_KEY_P",		"reverse");
	dvjInputMap[WAVEFORM_AUTO_DIVERGE_THEN_RECALL].Set
		("waveformAutoDivergeThenRecall",	true,	"LGL_KEY_NONE",		"");
	dvjInputMap[WAVEFORM_VIDEO_SELECT].Set
		("waveformVideoSelect",			true,	"LGL_KEY_PERIOD",	"video_select");
	dvjInputMap[WAVEFORM_VIDEO_BRIGHTNESS].Set
		("waveformVideoBrightness",		true,	"LGL_KEY_NONE",		"video_brightness");
	dvjInputMap[WAVEFORM_VIDEO_FREQSENSE_BRIGHTNESS].Set
		("waveformVideoFreqSenseBrightness",	true,	"LGL_KEY_NONE",		"video_freqsense_brightness");
	dvjInputMap[WAVEFORM_SYPHON_BRIGHTNESS].Set
		("waveformSyphonBrightness",		true,	"LGL_KEY_NONE",		"syphon_brightness");
	dvjInputMap[WAVEFORM_LED_FREQSENSE_BRIGHTNESS].Set
		("waveformLEDFreqSenseBrightness",	true,	"LGL_KEY_NONE",		"led_freqsense_brightness");
	dvjInputMap[WAVEFORM_LED_COLOR_LOW].Set
		("waveformLEDColorLow",			true,	"LGL_KEY_NONE",		"led_color_low");
	dvjInputMap[WAVEFORM_LED_COLOR_HIGH].Set
		("waveformLEDColorHigh",		true,	"LGL_KEY_NONE",		"led_color_high");
	dvjInputMap[WAVEFORM_LED_COLOR_HIGH_WASH].Set
		("waveformLEDColorHighWash",		true,	"LGL_KEY_NONE",		"led_color_high_wash");
	dvjInputMap[WAVEFORM_LED_GROUP].Set
		("waveformLEDGroup",			true,	"LGL_KEY_NONE",		"led_group");
	dvjInputMap[WAVEFORM_OSCILLOSCOPE_BRIGHTNESS].Set
		("waveformOscilloscopeBrightness",	true,	"LGL_KEY_NONE",		"oscilloscope_brightness");
	dvjInputMap[WAVEFORM_AUDIO_INPUT_TOGGLE].Set
		("waveformAudioInputToggle",		true,	"LGL_KEY_F11",		"audio_input/toggle");
	dvjInputMap[WAVEFORM_VIDEO_ASPECT_RATIO_NEXT].Set
		("waveformVideoAspectRatioNext",	true,	"LGL_KEY_SLASH",	"video_aspect_ratio/next");
	dvjInputMap[WAVEFORM_SYNC].Set
		("waveformSync",			true,	"LGL_KEY_BACKSLASH",	"sync");
	dvjInputMap[FULL_SCREEN_TOGGLE].Set
		("fullScreenToggle",			false,	"LGL_KEY_NONE",		"");
	dvjInputMap[VISUALIZER_FULL_SCREEN_TOGGLE].Set
		("visualizerFullScreenToggle",		false,	"LGL_KEY_F4",		"");
	dvjInputMap[SCREENSHOT].Set
		("screenshot",				false,	"LGL_KEY_F2",		"");
}

void
CreateDefaultKeyboardInput
(
	const char*	path
)
{
	//Prime dvjInputMap with default values
	PrepareInputMap();

	int maxLength=0;
	for(int a=0;a<ACTION_LAST;a++)
	{
		maxLength=LGL_Max(strlen(dvjInputMap[a].ActionGlobalStr),maxLength);
	}

	if(FILE* fd=fopen(path,"w"))
	{
		fprintf(fd,"#\n");
		fprintf(fd,"# keyboard.txt\n");
		fprintf(fd,"#\n");
		fprintf(fd,"# For a list of all possible LGL_KEY_* values, see:\n");
		fprintf(fd,"# TODO... Ask interim.descriptor@gmail.com, for now\n");
		fprintf(fd,"#\n");
		fprintf(fd,"\n");
		for(int a=0;a<ACTION_LAST;a++)
		{
			if(const char* key = dvjInputMap[a].ActionGlobalStr)
			{
				if(dvjInputMap[a].KeyboardInt != LGL_KEY_NONE)
				{
					if(const char* valueStr = dvjInputMap[a].KeyboardStr)
					{
						fprintf(fd,"%s",key);
						int numSpaces = maxLength - (int)strlen(key);
						for(int b=0;b<numSpaces;b++)
						{
							fprintf(fd, " ");
						}
						fprintf(fd," = %s\n",valueStr);
					}
				}
			}
		}
		fprintf(fd,"\n");
		fclose(fd);
	}
}

void
CreateDefaultOscInput
(
	const char*	path
)
{
	//Prime dvjInputMap with default values
	PrepareInputMap();

	int maxLength=0;
	for(int a=0;a<ACTION_LAST;a++)
	{
		maxLength=LGL_Max(strlen(dvjInputMap[a].ActionGlobalStr),maxLength);
	}
	maxLength+=strlen(":TT0");

	if(FILE* fd=fopen(path,"w"))
	{
		fprintf(fd,"#\n");
		fprintf(fd,"# osc.txt\n");
		fprintf(fd,"#\n");
		fprintf(fd,"\n");
		fprintf(fd,"\n");
		fprintf(fd,"\n");
		fprintf(fd,"#\n");
		fprintf(fd,"# Global Section\n");
		fprintf(fd,"#\n");
		fprintf(fd,"\n");
		fprintf(fd,"oscServerPort      = 7000\n");
		fprintf(fd,"\n");
		fprintf(fd,"# When an listed client connects to our server, we try to make an outgoing connection back to its server, trying these ports:\n");
		fprintf(fd,"oscClientAutoPort0 = 7000\n");
		fprintf(fd,"oscClientAutoPort1 = 7001\n");
		fprintf(fd,"oscClientAutoPort2 = 0\n");
		fprintf(fd,"oscClientAutoPort3 = 0\n");
		fprintf(fd,"\n");
		fprintf(fd,"oscClient_000_Host = localhost\n");
		fprintf(fd,"oscClient_000_Port = 0\n");
		fprintf(fd,"\n");
		fprintf(fd,"oscClient_001_Host = localhost\n");
		fprintf(fd,"oscClient_001_Port = 0\n");
		fprintf(fd,"\n");
		fprintf(fd,"oscClient_002_Host = localhost\n");
		fprintf(fd,"oscClient_002_Port = 0\n");
		fprintf(fd,"\n");
		fprintf(fd,"oscClient_003_Host = localhost\n");
		fprintf(fd,"oscClient_003_Port = 0\n");
		fprintf(fd,"\n");
		fprintf(fd,"# You may add up to oscClient_0255, if you so desire.\n");
		fprintf(fd,"\n");
		fprintf(fd,"\n");
		fprintf(fd,"\n");
		fprintf(fd,"#\n");
		fprintf(fd,"# Address Pattern Section\n");
		fprintf(fd,"#\n");
		fprintf(fd,"# If you override these values, dvj will use your value as well as the default.\n");
		fprintf(fd,"#\n");
		fprintf(fd,"# For a list of all default values, move this file away from its current location, and run dvj to regenerate it.\n");
		fprintf(fd,"#\n");
		fprintf(fd,"\n");
		for(int a=0;a<ACTION_LAST;a++)
		{
			std::vector<const char*> valueStrList;
			std::vector<const char*> keyStrList;
			if(dvjInputMap[a].EachTurntable)
			{
				valueStrList.push_back(dvjInputMap[a].OscDefaultAddressPatternTT0);
				keyStrList.push_back(dvjInputMap[a].ActionTT0Str);
				valueStrList.push_back(dvjInputMap[a].OscDefaultAddressPatternTT1);
				keyStrList.push_back(dvjInputMap[a].ActionTT1Str);
				//valueStrList.push_back(dvjInputMap[a].OscDefaultAddressPatternTTFocus);
				//keyStrList.push_back(dvjInputMap[a].ActionTTFocusStr);
			}
			else
			{
				valueStrList.push_back(dvjInputMap[a].OscDefaultAddressPatternGlobal);
				keyStrList.push_back(dvjInputMap[a].ActionGlobalStr);
			}

			for(unsigned int v=0;v<valueStrList.size();v++)
			{
				const char* valueStr = valueStrList[v];
				const char* keyStr = keyStrList[v];
				if
				(
					valueStr &&
					valueStr[0]!='\0' &&
					keyStr &&
					keyStr[0]!='\0'
				)
				{
					fprintf(fd,"%s",keyStr);
					int numSpaces = maxLength - (int)strlen(keyStr);
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
	{ return(dvjInputMap[FOCUS_CHANGE].KeyboardInt); }
int GetInputKeyboardFocusBottomKey()
	{ return(dvjInputMap[FOCUS_BOTTOM].KeyboardInt); }
int GetInputKeyboardFocusTopKey()
	{ return(dvjInputMap[FOCUS_TOP].KeyboardInt); }
int GetInputKeyboardXfaderSpeakersDeltaDownKey()
	{ return(dvjInputMap[XFADER_SPEAKERS_DELTA_DOWN].KeyboardInt); }
int GetInputKeyboardXfaderSpeakersDeltaUpKey()
	{ return(dvjInputMap[XFADER_SPEAKERS_DELTA_UP].KeyboardInt); }
int GetInputKeyboardXfaderHeadphonesDeltaDownKey()
	{ return(dvjInputMap[XFADER_HEADPHONES_DELTA_DOWN].KeyboardInt); }
int GetInputKeyboardXfaderHeadphonesDeltaUpKey()
	{ return(dvjInputMap[XFADER_HEADPHONES_DELTA_UP].KeyboardInt); }
int GetInputKeyboardMasterToHeadphones()
	{ return(dvjInputMap[MASTER_TO_HEADPHONES].KeyboardInt); }
int GetInputKeyboardFileScrollDownManyKey()
	{ return(dvjInputMap[FILE_SCROLL_DOWN_MANY].KeyboardInt); }
int GetInputKeyboardFileScrollUpManyKey()
	{ return(dvjInputMap[FILE_SCROLL_UP_MANY].KeyboardInt); }
int GetInputKeyboardFileScrollNextKey()
	{ return(dvjInputMap[FILE_SCROLL_NEXT].KeyboardInt); }
int GetInputKeyboardFileScrollPrevKey()
	{ return(dvjInputMap[FILE_SCROLL_PREV].KeyboardInt); }
int GetInputKeyboardFileSelectKey()
	{ return(dvjInputMap[FILE_SELECT].KeyboardInt); }
int GetInputKeyboardFileMarkUnopenedKey()
	{ return(dvjInputMap[FILE_MARK_UNOPENED].KeyboardInt); }
int GetInputKeyboardFileRefreshKey()
	{ return(dvjInputMap[FILE_REFRESH].KeyboardInt); }
int GetInputKeyboardWaveformEjectKey()
	{ return(dvjInputMap[WAVEFORM_EJECT].KeyboardInt); }
int GetInputKeyboardWaveformPauseToggleKey()
	{ return(dvjInputMap[WAVEFORM_PAUSE_TOGGLE].KeyboardInt); }
int GetInputKeyboardWaveformNudgeBackwardKey()
	{ return(dvjInputMap[WAVEFORM_NUDGE_BACKWARD].KeyboardInt); }
int GetInputKeyboardWaveformNudgeForwardKey()
	{ return(dvjInputMap[WAVEFORM_NUDGE_FORWARD].KeyboardInt); }
int GetInputKeyboardWaveformPitchbendDeltaDownSlowKey()
	{ return(dvjInputMap[WAVEFORM_PITCHBEND_DELTA_DOWN_SLOW].KeyboardInt); }
int GetInputKeyboardWaveformPitchbendDeltaUpSlowKey()
	{ return(dvjInputMap[WAVEFORM_PITCHBEND_DELTA_UP_SLOW].KeyboardInt); }
int GetInputKeyboardWaveformPitchbendDeltaDownKey()
	{ return(dvjInputMap[WAVEFORM_PITCHBEND_DELTA_DOWN].KeyboardInt); }
int GetInputKeyboardWaveformPitchbendDeltaUpKey()
	{ return(dvjInputMap[WAVEFORM_PITCHBEND_DELTA_UP].KeyboardInt); }
int GetInputKeyboardWaveformPitchbendDeltaDownFastKey()
	{ return(dvjInputMap[WAVEFORM_PITCHBEND_DELTA_DOWN_FAST].KeyboardInt); }
int GetInputKeyboardWaveformPitchbendDeltaUpFastKey()
	{ return(dvjInputMap[WAVEFORM_PITCHBEND_DELTA_UP_FAST].KeyboardInt); }
int GetInputKeyboardWaveformEQLowDeltaDownKey()
	{ return(dvjInputMap[WAVEFORM_EQ_LOW_DELTA_DOWN].KeyboardInt); }
int GetInputKeyboardWaveformEQLowDeltaUpKey()
	{ return(dvjInputMap[WAVEFORM_EQ_LOW_DELTA_UP].KeyboardInt); }
int GetInputKeyboardWaveformEQLowKillKey()
	{ return(dvjInputMap[WAVEFORM_EQ_LOW_KILL].KeyboardInt); }
int GetInputKeyboardWaveformEQMidDeltaDownKey()
	{ return(dvjInputMap[WAVEFORM_EQ_MID_DELTA_DOWN].KeyboardInt); }
int GetInputKeyboardWaveformEQMidDeltaUpKey()
	{ return(dvjInputMap[WAVEFORM_EQ_MID_DELTA_UP].KeyboardInt); }
int GetInputKeyboardWaveformEQMidKillKey()
	{ return(dvjInputMap[WAVEFORM_EQ_MID_KILL].KeyboardInt); }
int GetInputKeyboardWaveformEQHighDeltaDownKey()
	{ return(dvjInputMap[WAVEFORM_EQ_HIGH_DELTA_DOWN].KeyboardInt); }
int GetInputKeyboardWaveformEQHighDeltaUpKey()
	{ return(dvjInputMap[WAVEFORM_EQ_HIGH_DELTA_UP].KeyboardInt); }
int GetInputKeyboardWaveformEQHighKillKey()
	{ return(dvjInputMap[WAVEFORM_EQ_HIGH_KILL].KeyboardInt); }
int GetInputKeyboardWaveformGainDeltaDownKey()
	{ return(dvjInputMap[WAVEFORM_GAIN_DELTA_DOWN].KeyboardInt); }
int GetInputKeyboardWaveformGainDeltaUpKey()
	{ return(dvjInputMap[WAVEFORM_GAIN_DELTA_UP].KeyboardInt); }
int GetInputKeyboardWaveformGainKill()
	{ return(dvjInputMap[WAVEFORM_GAIN_KILL].KeyboardInt); }
int GetInputKeyboardWaveformVolumeInvertKey()
	{ return(dvjInputMap[WAVEFORM_VOLUME_INVERT].KeyboardInt); }
int GetInputKeyboardWaveformRhythmicVolumeInvertKey()
	{ return(dvjInputMap[WAVEFORM_RHYTHMIC_VOLUME_INVERT].KeyboardInt); }
int GetInputKeyboardWaveformRhythmicVolumeInvertOtherKey()
	{ return(dvjInputMap[WAVEFORM_RHYTHMIC_VOLUME_INVERT_OTHER].KeyboardInt); }
int GetInputKeyboardWaveformVolumeSoloKey()
	{ return(dvjInputMap[WAVEFORM_VOLUME_SOLO].KeyboardInt); }
int GetInputKeyboardWaveformSeekBackwardSlowKey()
	{ return(dvjInputMap[WAVEFORM_SEEK_BACKWARD_SLOW].KeyboardInt); }
int GetInputKeyboardWaveformSeekBackwardFastKey()
	{ return(dvjInputMap[WAVEFORM_SEEK_BACKWARD_FAST].KeyboardInt); }
int GetInputKeyboardWaveformSeekForwardSlowKey()
	{ return(dvjInputMap[WAVEFORM_SEEK_FORWARD_SLOW].KeyboardInt); }
int GetInputKeyboardWaveformSeekForwardFastKey()
	{ return(dvjInputMap[WAVEFORM_SEEK_FORWARD_FAST].KeyboardInt); }
int GetInputKeyboardWaveformSavePointPrevKey()
	{ return(dvjInputMap[WAVEFORM_SAVEPOINT_PREV].KeyboardInt); }
int GetInputKeyboardWaveformSavePointNextKey()
	{ return(dvjInputMap[WAVEFORM_SAVEPOINT_NEXT].KeyboardInt); }
int GetInputKeyboardWaveformSavePointSetKey()
	{ return(dvjInputMap[WAVEFORM_SAVEPOINT_SET].KeyboardInt); }
int GetInputKeyboardWaveformSavePointShiftBackwardKey()
	{ return(dvjInputMap[WAVEFORM_SAVEPOINT_SHIFT_BACKWARD].KeyboardInt); }
int GetInputKeyboardWaveformSavePointShiftForwardKey()
	{ return(dvjInputMap[WAVEFORM_SAVEPOINT_SHIFT_FORWARD].KeyboardInt); }
int GetInputKeyboardWaveformSavePointShiftAllBackwardKey()
	{ return(dvjInputMap[WAVEFORM_SAVEPOINT_SHIFT_ALL_BACKWARD].KeyboardInt); }
int GetInputKeyboardWaveformSavePointShiftAllForwardKey()
	{ return(dvjInputMap[WAVEFORM_SAVEPOINT_SHIFT_ALL_FORWARD].KeyboardInt); }
int GetInputKeyboardWaveformSavePointJumpNowKey()
	{ return(dvjInputMap[WAVEFORM_SAVEPOINT_JUMP_NOW].KeyboardInt); }
int GetInputKeyboardWaveformSavePointJumpAtMeasureKey()
	{ return(dvjInputMap[WAVEFORM_SAVEPOINT_JUMP_AT_MEASURE].KeyboardInt); }
int GetInputKeyboardWaveformQuantizationPeriodHalfKey()
	{ return(dvjInputMap[WAVEFORM_QUANTIZATION_PERIOD_HALF].KeyboardInt); }
int GetInputKeyboardWaveformQuantizationPeriodDoubleKey()
	{ return(dvjInputMap[WAVEFORM_QUANTIZATION_PERIOD_DOUBLE].KeyboardInt); }
int GetInputKeyboardWaveformStutterKey()
	{ return(dvjInputMap[WAVEFORM_STUTTER].KeyboardInt); }
int GetInputKeyboardWaveformLoopToggleKey()
	{ return(dvjInputMap[WAVEFORM_LOOP_TOGGLE].KeyboardInt); }
int GetInputKeyboardWaveformLoopThenRecallKey()
	{ return(dvjInputMap[WAVEFORM_LOOP_THEN_RECALL].KeyboardInt); }
int GetInputKeyboardWaveformReverseKey()
	{ return(dvjInputMap[WAVEFORM_REVERSE].KeyboardInt); }
int GetInputKeyboardWaveformAutoDivergeRecallKey()
	{ return(dvjInputMap[WAVEFORM_AUTO_DIVERGE_THEN_RECALL].KeyboardInt); }
int GetInputKeyboardWaveformVideoSelectKey()
	{ return(dvjInputMap[WAVEFORM_VIDEO_SELECT].KeyboardInt); }
int GetInputKeyboardWaveformAudioInputToggleKey()
	{ return(dvjInputMap[WAVEFORM_AUDIO_INPUT_TOGGLE].KeyboardInt); }
int GetInputKeyboardWaveformVideoAspectRatioNextKey()
	{ return(dvjInputMap[WAVEFORM_VIDEO_ASPECT_RATIO_NEXT].KeyboardInt); }
int GetInputKeyboardWaveformSyncKey()
	{ return(dvjInputMap[WAVEFORM_SYNC].KeyboardInt); }
int GetInputKeyboardFullScreenToggleKey()
	{ return(dvjInputMap[FULL_SCREEN_TOGGLE].KeyboardInt); }
int GetInputKeyboardVisualizerFullScreenToggleKey()
	{ return(dvjInputMap[VISUALIZER_FULL_SCREEN_TOGGLE].KeyboardInt); }
int GetInputKeyboardScreenshotKey()
	{ return(dvjInputMap[SCREENSHOT].KeyboardInt); }

int
GetOscServerPort()
{
	int port=inputOscConfigFile->read<int>("oscServerPort",7000);
	return(port);
}

std::vector<IpEndpointName>
GetOscClientList()
{
	std::vector<IpEndpointName> ret;

	char keyHost[512];
	char keyPort[512];

	for(int a=0;a<256;a++)
	{
		sprintf(keyHost,"oscClient_%03i_Host",a);
		sprintf(keyPort,"oscClient_%03i_Port",a);
		std::string hostStr = inputOscConfigFile->read<std::string>(keyHost,"");
		int port=inputOscConfigFile->read<int>(keyPort,0);
		if
		(
			port>=1024 &&
			hostStr.c_str()[0]!='\0'
		)
		{
			ret.push_back
			(
				IpEndpointName
				(
					hostStr.c_str(),
					port
				)
			);
		}
	}

	return(ret);
}

DVJ_GuiElement	faders[FADER_MAX];
bool		fadersCached=false;

DVJ_GuiElement
GetFader
(
	int	index
)
{
	index=LGL_Clamp(0,index,FADER_MAX-1);

	if(fadersCached==false)
	{
		for(int i=0;i<FADER_MAX;i++)
		{
			//Defaults
			DVJ_GuiElement ret = GUI_ELEMENT_NULL;
			if(i==0)
			{
				ret=GUI_ELEMENT_EQ_LOW;
			}
			else if(i==1)
			{
				ret=GUI_ELEMENT_EQ_MID;
			}
			else if(i==2)
			{
				ret=GUI_ELEMENT_EQ_HIGH;
			}
			else if(i==3)
			{
				ret=GUI_ELEMENT_EQ_GAIN;
			}
			else if(i==4)
			{
				ret=GUI_ELEMENT_VIDEO;
			}

			char key[512];
			sprintf(key,"Fader%02i",i);

			std::string valStr = dvjrcConfigFile->read<std::string>(key,"NULL");
			const char* val=valStr.c_str();

			//Key/Vals
			if(strcasecmp(val,"NULL")==0)
			{
				ret=GUI_ELEMENT_NULL;
			}
			else if(strcasecmp(val,"LowEQ")==0)
			{
				ret=GUI_ELEMENT_EQ_LOW;
			}
			else if(strcasecmp(val,"MidEQ")==0)
			{
				ret=GUI_ELEMENT_EQ_MID;
			}
			else if(strcasecmp(val,"HighEQ")==0)
			{
				ret=GUI_ELEMENT_EQ_HIGH;
			}
			else if(strcasecmp(val,"Gain")==0)
			{
				ret=GUI_ELEMENT_EQ_GAIN;
			}
			else if(strcasecmp(val,"Video")==0)
			{
				ret=GUI_ELEMENT_VIDEO;
			}
			else if(strcasecmp(val,"VideoFreqSense")==0)
			{
				ret=GUI_ELEMENT_VIDEO_FREQSENSE;
			}
			else if(strcasecmp(val,"Syphon")==0)
			{
				ret=GUI_ELEMENT_SYPHON;
			}
			else if(strcasecmp(val,"Oscilloscope")==0)
			{
				ret=GUI_ELEMENT_OSCILLOSCOPE;
			}
			else if(strcasecmp(val,"LEDFreqSense")==0)
			{
				ret=GUI_ELEMENT_LED_FREQSENSE;
			}
			else if(strcasecmp(val,"LEDColorLow")==0)
			{
				ret=GUI_ELEMENT_LED_COLOR_LOW;
			}
			else if(strcasecmp(val,"LEDColorHigh")==0)
			{
				ret=GUI_ELEMENT_LED_COLOR_HIGH;
			}
			else if(strcasecmp(val,"LEDColorHighWash")==0)
			{
				ret=GUI_ELEMENT_LED_COLOR_HIGH_WASH;
			}
			else if(strcasecmp(val,"LEDGroup")==0)
			{
				ret=GUI_ELEMENT_LED_GROUP;
			}

			faders[i]=ret;
		}

		fadersCached=true;
	}


	return(faders[index]);
}

bool
GetSyphonServerEnabled()
{
	int enabled=dvjrcConfigFile->read<int>("syphonServerEnabled",0);
	return(enabled!=0);
}

bool
GetHideProjectorWindows()
{
	int hide=dvjrcConfigFile->read<int>("hideProjectorWindows",0);
	return(hide!=0);
}

std::vector<LEDClient>
GetLEDClientList()
{
	std::vector<LEDClient> ret;

	char keyHost[512];
	char keyPort[512];
	char keyChannel[512];
	char keyGroup[512];

	for(int a=0;a<256;a++)
	{
		sprintf(keyHost,"ledClient_%03i_Host",a);
		sprintf(keyPort,"ledClient_%03i_Port",a);
		sprintf(keyChannel,"ledClient_%03i_Channel",a);
		sprintf(keyGroup,"ledClient_%03i_Group",a);
		std::string hostStr = dvjrcConfigFile->read<std::string>(keyHost,"");
		int port=dvjrcConfigFile->read<int>(keyPort,0);
		int channel=dvjrcConfigFile->read<int>(keyChannel,0);
		int group=dvjrcConfigFile->read<int>(keyGroup,0);
		if
		(
			hostStr.c_str()[0]!='\0' &&
			port>=1024 &&
			group>=0
		)
		{
			LEDClient client;
			client.Endpoint=
				IpEndpointName
				(
					hostStr.c_str(),
					port
				);
			client.Channel=channel;
			client.Group=group;
			ret.push_back(client);
		}
	}

	return(ret);
}

std::vector<int>
GetOscClientAutoPortList()
{
	std::vector<int> ret;
	for(int a=0;a<4;a++)
	{
		char key[512];
		sprintf(key,"oscClientAutoPort%i",a);
		int port=inputOscConfigFile->read<int>(key,0);
		if(port!=0)
		{
			ret.push_back(port);
		}
	}
	return(ret);
}

std::vector<const char*>
GetOscAddressPatternList
(
	DVJ_Action	action,
	int		target
)
{
	return(dvjInputMap[action].GetOscAddressPatternList(target));
}

bool
GetDebugVideoCaching()
{
	int debug=dvjrcConfigFile->read<int>("debugVideoCaching",0);
	return(debug!=0);
}

bool
GetDebugRecordHold()
{
	int debug=dvjrcConfigFile->read<int>("debugRecordHold",0);
	return(debug!=0);
}

bool
GetTestFreqSenseTime()
{
	int debug=dvjrcConfigFile->read<int>("testFreqSenseTime",0);
	return(debug!=0);
}

bool
GetDebugUseMultiTouchWithOneDisplay()
{
	int debug=dvjrcConfigFile->read<int>("debugUseMultiTouchWithOneDisplay",0);
	return(debug!=0);
}

//int GetInputKeyboardWaveformKey() { return(dvjInputMap[WAVEFORM_].KeyboardInt); }

