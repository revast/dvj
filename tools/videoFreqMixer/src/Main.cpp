/*
 *
 * videoFreqMixer
 *
 * Copyright Chris Nelson, 2008
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "LGL.module/LGL.h"

#include "../../../src/Common.cpp"	//This is so screwy, but it works.

#define	BEAT_RESEARCH
//#define	GANTZ_GRAF_MASHUP

//const char* targetMusicPath="/home/emf/music/hypermind.mp3";
/*
const char* targetMusicPath="/home/emf/music/Terminal Dusk - Crimson (04) - Eight Frozen Modules - Left Me.mp3";
const char* targetVideoBGPath="data/bg.avi";
const char* targetVideoLoPath="data/lo.avi";
const char* targetVideoHiPath="data/hi.avi";
const char* targetVideoOutPath="data/out.avi";
*/

char targetMusicPath[1024];
char targetVideoBGPath[1024];
char targetVideoLoPath[1024];
char targetVideoHiPath[1024];
char targetVideoOutPath[1024];

LGL_Sound*
LoadRecordSound
(
	const
	char*	audioFile
)
{
	long soundBufferLength=4*44100*60*80;
	Uint8* soundBuffer=(Uint8*)malloc(soundBufferLength);
	assert(soundBuffer);
	LGL_Sound* recordSound=new LGL_Sound(audioFile,true,2,soundBuffer,soundBufferLength);
	recordSound->SetHogCPU();
	for(;;)
	{
		LGL_ProcessInput();
		if(LGL_KeyStroke(SDLK_ESCAPE))
		{
			delete recordSound;
			exit(0);
		}
		if(recordSound->IsLoaded())
		{
			break;
		}
		float pct = recordSound->GetPercentLoaded();
		LGL_DrawRectToScreen
		(
			0,pct,
			0,0.05f,
			.4f*pct,.2f*pct,0.5f+0.5f*pct,1.0f
		);
		LGL_GetFont().DrawString
		(
			0.49f,0.015f,0.02f,
			1,1,1,1,
			false,
			.75f,
			"%.0f%%",
			pct*100.0f
		);
		LGL_GetFont().DrawString
		(
			0.49f,0.065f,0.025f,
			1,1,1,1,
			true,
			.75f,
			"Loading '%s'",
			audioFile
		);
		LGL_DelaySeconds(1.0f/120.0f-LGL_SecondsSinceThisFrame());
		LGL_SwapBuffers();
	}
	if(recordSound->GetLengthSeconds()==0)
	{
		printf("Error! Soundtrack '%s' has zero length!\n",audioFile);
		exit(0);
	}
	return(recordSound);
}

int
encodeVideo
(
	int	argc,
	char**	argv
);

const int ResX=720;
const int ResY=480;

int
main
(
	int	argc,
	char**	argv
)
{
	bool encodeAll=(argc==2 && strcmp(argv[1],"--all")==0);

#ifndef	BEAT_RESEARCH
	if(argc!=5 && encodeAll==false)
	{
		printf("Usage: ./videoFreqMixer.lin music.mp3 bg.avi lo.avi hi.avi\n\n");
		exit(0);
	}
#endif	//BEAT_RESEARCH

	//Init LGL
	LGL_Init
	(
		ResX,
		ResY,
		false,
		2,
		argv[0]
	);

	if(encodeAll)
	{
		//Encode a video for each track that has metasync data
		LGL_DirTree dirTree("data/metadata");
		for(unsigned int a=0;a<dirTree.GetFileCount();a++)
		{
			char metaDataPath[1024];
			sprintf(metaDataPath,"%s/%s",dirTree.GetPath(),dirTree.GetFileName(a));

			char metaDataPathShort[1024];
			sprintf(metaDataPathShort,"%s",dirTree.GetFileName(a));

			char musicPath[1024];
			sprintf(musicPath,"music/%s",metaDataPathShort);
			if(strstr(musicPath,".musefuse-metadata.txt"))
			{
				strstr(musicPath,".musefuse-metadata.txt")[0]='\0';
			}

			char musicPathShort[1024];
			strcpy(musicPathShort,metaDataPathShort);
			if(strstr(musicPathShort,".musefuse-metadata.txt"))
			{
				strstr(musicPathShort,".musefuse-metadata.txt")[0]='\0';
			}

			char videoTrackPath[1024];
			sprintf(videoTrackPath,"tracks/%s.mjpeg.avi",musicPathShort);

			char videoOutPath[1024];
			sprintf(videoOutPath,"out/%s.mjpeg.avi",musicPathShort);

			if
			(
				LGL_FileExists(musicPath) &&
				LGL_FileExists(videoTrackPath)==false &&
				LGL_FileExists(videoOutPath)==false
			)
			{
				char* myArgv[5];
				for(int a=0;a<5;a++) myArgv[a]=new char[1024];
				strcpy(myArgv[0],argv[0]);
				strcpy(myArgv[1],musicPath);
				strcpy(myArgv[2],"NULL");
				strcpy(myArgv[3],"rnd");
				strcpy(myArgv[4],"rnd");

printf("Encoding %s\n",musicPath);
				encodeVideo(5,(char**)myArgv);
exit(0);
				for(int a=0;a<5;a++) delete myArgv[a];
			}
			else
			{
printf("Skipping %s (%i %i %i)\n",musicPath,
				LGL_FileExists(musicPath) ? 1 : 0,
				(LGL_FileExists(videoTrackPath)==false) ? 1 : 0,
				(LGL_FileExists(videoOutPath)==false) ? 1 : 0
);
printf("\t[0]: %s\n",musicPath);
printf("\t[1]: %s\n",videoTrackPath);
printf("\t[2]: %s\n",videoOutPath);
			}
		}
	}
	else
	{
		encodeVideo(argc,argv);
	}
}

int
encodeVideo
(
	int	argc,
	char**	argv
)
{
#ifdef	BEAT_RESEARCH
	strcpy(targetMusicPath,"BeatResearch.mp3");
#else
	strcpy(targetMusicPath,argv[1]);
#endif

	for(int a=2;a<=4;a++)
	{
		char* target;
		if(a==2) target=targetVideoBGPath;
		if(a==3) target=targetVideoLoPath;
		if(a==4) target=targetVideoHiPath;

		if(strcmp(argv[a],"rnd")==0)
		{
			LGL_DirTree allVideos("data/all");
			int index=LGL_RandInt(0,allVideos.GetFileCount()-1);
			sprintf(target,"data/all/%s",allVideos.GetFileName(index));
			printf("Video[%i]: %s [%s]\n",a-2,target,LGL_FileExists(target)?"OK":"NO FILE");
		}
		else
		{
			strcpy(target,argv[a]);
		}
	}

	char* p=targetMusicPath;
	while(strstr(p,"/"))
	{
		p=&(strstr(p,"/")[1]);
	}
	sprintf(targetVideoOutPath,"out/%s.mjpeg.avi",p);

	//Load Sound
	LGL_Sound* snd=LoadRecordSound(targetMusicPath);
	snd->LockBufferForReading(10);
	Uint8* buf8=snd->GetBuffer();
	Sint16* buf16=(Sint16*)buf8;
	unsigned long len16=(snd->GetBufferLength()/2);

	//Load Videos
	LGL_Video* vidBG=NULL;
	LGL_Video* vidLo=NULL;
	LGL_Video* vidHi=NULL;
	if(LGL_FileExists(targetVideoBGPath))
	{
		vidBG=new LGL_Video(targetVideoBGPath);
		printf("Video[BG]: %s [%s]\n",targetVideoBGPath,LGL_FileExists(targetVideoBGPath)?"OK":"NO FILE");
	}
	if(LGL_FileExists(targetVideoLoPath))
	{
		vidLo=new LGL_Video(targetVideoLoPath);
		printf("Video[Lo]: %s [%s]\n",targetVideoLoPath,LGL_FileExists(targetVideoLoPath)?"OK":"NO FILE");
	}
	if(LGL_FileExists(targetVideoHiPath))
	{
		vidHi=new LGL_Video(targetVideoHiPath);
		printf("Video[Hi]: %s [%s]\n",targetVideoHiPath,LGL_FileExists(targetVideoHiPath)?"OK":"NO FILE");
	}

	//Fire up mencoder
	char cmd[1024];
#ifdef	BEAT_RESEARCH
	//60fps mpeg4
	sprintf
	(
		cmd,
		"mencoder -audiofile \"%s\" -oac copy -ovc lavc -of avi -vf harddup -lavcopts vcodec=mpeg4:vrc_buf_size=5848:vrc_maxrate=9800:vbitrate=5000:autoaspect=1 -ofps 60000/1001 -o %s.avi - -nosound -demuxer rawvideo -rawvideo fps=60:w=%i:h=%i:format=rgb24 -idx -flip",
		targetMusicPath,
		targetMusicPath,
		ResX,
		ResY

	);
	//NTSC-DVD
	/*
	sprintf
	(
		cmd,
		"mencoder -audiofile \"%s\" -oac lavc -ovc lavc -of mpeg -mpegopts format=dvd:tsaf -vf scale=720:480,harddup -srate 48000 -af lavcresample=48000 -lavcopts vcodec=mpeg2video:vrc_buf_size=1835:vrc_maxrate=9800:vbitrate=5000:keyint=18:vstrict=0:acodec=ac3:abitrate=384:aspect=16/9 -ofps 30000/1001 -o %s.mpg - -nosound -demuxer rawvideo -rawvideo fps=60:w=%i:h=%i:format=rgb24 -idx -flip",
		targetMusicPath,
		targetMusicPath,
		ResX,
		ResY

	);
	*/
#else	//BEAT_RESEARCH
	const char* cmdInput;
	if(strstr(targetMusicPath,".ogg"))
	{
		cmdInput="mencoder - -nosound -demuxer rawvideo -rawvideo fps=60:w=%i:h=%i:format=rgb24 -idx -flip -ovc lavc -lavcopts vcodec=mjpeg:vbitrate=%i:autoaspect=1:threads=1 -vf scale=%i:%i,harddup -noskip -of avi -o \"%s\" 1>/dev/null 2>/dev/null";
		sprintf
		(
			cmd,
			cmdInput,
			ResX,
			ResY,
			4000,	//bitrateHQ,
			ResX,
			ResY,
			targetVideoOutPath
		);
	}
	else
	{
		cmdInput="mencoder - -audiofile \"%s\" -oac copy -demuxer rawvideo -rawvideo fps=60:w=%i:h=%i:format=rgb24 -idx -flip -ovc lavc -lavcopts vcodec=mjpeg:vbitrate=%i:autoaspect=1:threads=1 -vf scale=%i:%i,harddup -noskip -of avi -o \"%s\" 1>/dev/null 2>/dev/null";
		sprintf
		(
			cmd,
			cmdInput,
			targetMusicPath,
			ResX,
			ResY,
			4000,	//bitrateHQ,
			ResX,
			ResY,
			targetVideoOutPath
		);
	}
#endif	//BEAT_RESEARCH
	//const char* cmdInput="mencoder - -audiofile \"%s\" -oac copy -demuxer rawvideo -rawvideo fps=60:w=%i:h=%i:format=rgb24 -idx -flip -ovc lavc -lavcopts vcodec=mpeg4:mbd=2:trell=yes:v4mv=yes:vbitrate=%i:autoaspect=1:threads=1 -vf scale=%i:%i,harddup -noskip -of avi -o \"%s\" 1>/dev/null 2>/dev/null";
printf("cmd:\n\t%s\n",cmd);
	FILE* mencoderFD=popen(cmd,"w");
	assert(mencoderFD);

	float vidBGTime=LGL_RandFloat(10.0f,vidBG?(vidBG->GetLengthSeconds()-30.0f):20.0f);
	float vidLoTime=LGL_RandFloat(10.0f,vidLo?(vidLo->GetLengthSeconds()-30.0f):20.0f);
	float vidHiTime=LGL_RandFloat(10.0f,vidHi?(vidHi->GetLengthSeconds()-30.0f):20.0f);

	float secondsStart=0.0f;
	float secondsEnd=9999.0f;

LGL_Video* setVid=NULL;
#ifdef	BEAT_RESEARCH
double	timeStartGlobal[22];
double	timeStartLocal[22];
const char*	setVids[22];
const char*	lowVids[22];
const char*	hiVids[22];
float		hiScalar[22];
float		hiScalarSpeed[22];

int i=0;

//00: Gantz
i=0;
timeStartGlobal[i]=0.0f;
timeStartLocal[i]=2*60 + 39.06 + 0.075;
setVids[i]="tracks/Met_Biddler - Biddler On The Roof (04) - Missy Elliot + Dance Graf + Hella Good = Awesome Song (102.00 - 116.65).mp3.mjpeg.avi";
lowVids[i]="";
hiVids[i]="";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

//01: Toxic
i++;
timeStartGlobal[i]=4*60 + 13.004 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (07) - Phuturocity (03).mp3.mjpeg.avi";
hiVids[i]="data/all/Motion Loops (07) - Phuturocity (05).mp3.mjpeg.avi";
hiScalar[i]=2.0f;
hiScalarSpeed[i]=4.0f;

//02: Rukus
i++;
timeStartGlobal[i]=3*60 + 0.875 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (08) - Stimulant Control (05).mp3.mjpeg.avi";
hiVids[i]="data/all/Motion Loops (08) - Stimulant Control (07).mp3.mjpeg.avi";
hiScalar[i]=2.0f;
hiScalarSpeed[i]=4.0f;

//03: Champion Dub
i++;
timeStartGlobal[i]=1*60 + 22.288f + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (08) - Stimulant Control (02).mp3.mjpeg.avi";
hiVids[i]="tracks/BT - This Binary Universe (05) - See You On The Other Side.mp3.mjpeg.avi";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

//04: Platonia
i++;
timeStartGlobal[i]=2*60 + 17.169f + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (07) - Phuturocity (02).mp3.mjpeg.avi'";
hiVids[i]="data/all/David The VJ - Free Loops.mp3.mjpeg.avi";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

//05: Thequenique
i++;
timeStartGlobal[i]=1*60 + 22.315 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
//lowVids[i]="tracks/Plaid - Greedy Baby (12) - New Family.mp3.mjpeg.avi";
lowVids[i]="data/all/Huseinpeyda.mp3.mjpeg.avi";
hiVids[i]="data/all/Motion Loops (03) - Infinite Dream (03).mp3.mjpeg.avi";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

//06: Android
i++;
timeStartGlobal[i]=2*60 + 30.830 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (03) - Infinite Dream (11).mp3.mjpeg.avi";
hiVids[i]="data/all/Motion Loops (03) - Infinite Dream (07).mp3.mjpeg.avi";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

//07: Less
i++;
timeStartGlobal[i]=0*60 + 54.866 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (08) - Stimulant Control (07).mp3.mjpeg.avi";
hiVids[i]="tracks/Meat Beat Manifesto - In Dub (08) - Happiness Supreme Dub.mp3.mjpeg.avi";
hiScalar[i]=2.0f;
hiScalarSpeed[i]=4.0f;

//08: Reptile Dub
i++;
timeStartGlobal[i]=4*60 + 0.002 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
//lowVids[i]="data/all/Huseinpeyda.mp3.mjpeg.avi";
lowVids[i]="tracks/BT - This Binary Universe (04) - 1.618.mp3.mjpeg.avi";
hiVids[i]="data/all/Motion Loops (08) - Stimulant Control (04).mp3.mjpeg.avi";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

//09: Smart Bomb
i++;
timeStartGlobal[i]=3*60 + 8.570 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (08) - Stimulant Control (02).mp3.mjpeg.avi";
//hiVids[i]="data/all/Bit_Meddler - Anamorph.mp3.mjpeg.avi";
hiVids[i]="data/all/Motion Loops (07) - Phuturocity (08).mp3.mjpeg.avi";
hiScalar[i]=2.0f;
hiScalarSpeed[i]=4.0f;

//10: Zealot
i++;
timeStartGlobal[i]=1*60 + 49.720 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (03) - Infinite Dream (04).mp3.mjpeg.avi";
hiVids[i]="data/all/Motion Loops (03) - Infinite Dream (05).mp3.mjpeg.avi";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

//11: Crux
i++;
timeStartGlobal[i]=1*60 + 22.294 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/VJ Mission - Timbale.mp3.mjpeg.avi";
hiVids[i]="data/all/Motion Loops (08) - Stimulant Control (11).mp3.mjpeg.avi";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

//12: Creeping Up On You
i++;
timeStartGlobal[i]=1*60 + 44.996 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (08) - Stimulant Control (01).mp3.mjpeg.avi";
hiVids[i]="data/all/Bit_Meddler - Anamorph.mp3.mjpeg.avi";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=2.0f;

//13: Poison Is The Mindkiller
i++;
timeStartGlobal[i]=4*60 + 21.391 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Rez - Area 5 HD Gameplay.mp3.mjpeg.avi";
hiVids[i]="data/all/Hackers - Selected Scenes.mp3.mjpeg.avi";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=2.0f;

//14: Gnats
i++;
timeStartGlobal[i]=3*60 + 24.417 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (03) - Infinite Dream (08).mp3.mjpeg.avi";
hiVids[i]="data/all/Motion Loops (03) - Infinite Dream (09).mp3.mjpeg.avi";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

//15: Dumbfound
i++;
timeStartGlobal[i]=2*60 + 9.168 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (04) - Quantum Reality (01).mp3.mjpeg.avi";
hiVids[i]="data/all/Motion Loops (03) - Infinite Dream (12).mp3.mjpeg.avi";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

//16: Dermetfak
i++;
timeStartGlobal[i]=3*60 + 13.924 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (04) - Quantum Reality (03).mp3.mjpeg.avi";
hiVids[i]="data/all/Motion Loops (04) - Quantum Reality (02).mp3.mjpeg.avi";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

//17: 3 Points
i++;
timeStartGlobal[i]=2*60 + 48.049 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (07) - Phuturocity (10).mp3.mjpeg.avi";
hiVids[i]="data/all/Motion Loops (08) - Stimulant Control (03).mp3.mjpeg.avi";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

//18: Eight Zero One
i++;
timeStartGlobal[i]=1*60 + 50.765 + timeStartGlobal[i-1];
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (07) - Phuturocity (01).mp3.mjpeg.avi";
hiVids[i]="data/all/Motion Loops (04) - Quantum Reality (06).mp3.mjpeg.avi";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

//19: Snarepusher
const double SNAREPUSHER_OFFSET=1.253 - 0.075;
i++;
timeStartGlobal[i]=3*60 + 2.646 + timeStartGlobal[i-1] - SNAREPUSHER_OFFSET;
timeStartLocal[i]=0.075;
setVids[i]="tracks/Interim Descriptor - Snarepusher.mp3.mjpeg.avi";
lowVids[i]="";
hiVids[i]="";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

//20: What It Is Without
i++;
timeStartGlobal[i]=5*60 + 23.940 + timeStartGlobal[i-1] + SNAREPUSHER_OFFSET;
timeStartLocal[i]=0.0f;
setVids[i]=NULL;
lowVids[i]="data/all/Motion Loops (03) - Infinite Dream (06).mp3.mjpeg.avi";
hiVids[i]="data/all/Motion Loops (04) - Quantum Reality (11).mp3.mjpeg.avi";
hiScalar[i]=4.0f;
hiScalarSpeed[i]=2.0f;

//21: Lunatics
const double LUNATICS_OFFSET=7.645;
i++;
timeStartGlobal[i]=5*60 + 21.526 + timeStartGlobal[i-1] - LUNATICS_OFFSET;
timeStartLocal[i]=0.0075;
setVids[i]="tracks/Collide - The Lunatics Have Taken Over.mp3.mjpeg.avi";
lowVids[i]="";
hiVids[i]="";
hiScalar[i]=1.0f;
hiScalarSpeed[i]=4.0f;

for(int a=0;a<22;a++)
{
	if(lowVids[i]!=NULL && strlen(lowVids[i])>0)
	{
		assert(LGL_FileExists(lowVids[i]));
	}
	if(hiVids[i]!=NULL && strlen(hiVids[i])>0)
	{
		assert(LGL_FileExists(hiVids[i]));
	}
	if(setVids[i]!=NULL && strlen(setVids[i])>0)
	{
		assert(LGL_FileExists(setVids[i]));
	}
}

int brIndexNow=-1;
setVid = new LGL_Video(setVids[0]);
bool useSetVid=false;
secondsEnd=3891.0f;
#endif

#ifdef	GANTZ_GRAF_MASHUP
secondsStart=0.0f;
secondsEnd=9999.0f;
//Missy starts at 3:27.15
//Gantz starts at 3:54.80
//Britney starts at 5:37.05
float missyStart=3*60 + 27.15f;
float gantzStart=3*60 + 54.80f;
float britneyStart=5*60 + 37.05f;

LGL_Video* missyVid=new LGL_Video("tracks/Missy Elliott - The Videos (01) - Get Ur Freak On.mp3.mjpeg.avi");
LGL_Video* gantzVid=new LGL_Video("tracks/Autechre - Gantz Graf (Chris Cunningham).mp3.mjpeg.avi");
LGL_Video* britneyVid=new LGL_Video("tracks/Britney Spears - Greatest Hits (My Perogative) (11) - I Am A Slave 4 U.mp3.mjpeg.avi");
LGL_Video* rndVid=vidHi;
#endif	//GANTZ_GRAF_MASHUP

float frameRate=60.0f;

	bool firstFrame=true;

	//Loop
	for(int frame=secondsStart*frameRate;frame<snd->GetLengthSeconds()*60 && frame<secondsEnd*frameRate;frame++)
	{
#ifdef	GANTZ_GRAF_MASHUP
float secondsNow=frame/frameRate;
if(secondsNow<missyStart)
{
	//Meh
}
else if(secondsNow<gantzStart)
{
	vidLo=missyVid;
	vidHi=rndVid;
}
else if(secondsNow<britneyStart)
{
	vidLo=missyVid;
	vidHi=gantzVid;
}
else
{
	vidLo=rndVid;
	vidHi=britneyVid;
}
#endif	//GANTZ_GRAF_MASHUP

#ifdef	BEAT_RESEARCH
	float secondsNow=frame/frameRate;
	if
	(
		brIndexNow==-1 ||
		(
			brIndexNow<21 &&
			secondsNow>=timeStartGlobal[brIndexNow+1]
		)
	)
	{
		//Next video!
		brIndexNow++;
		printf("BR Video %i\n",brIndexNow);
		if(setVids[brIndexNow])
		{
			setVid->SetVideo(setVids[brIndexNow]);
			useSetVid=true;
		}
		else
		{
			char target[1024];
			if(strcmp(lowVids[brIndexNow],"rnd")==0)
			{
				LGL_DirTree allVideos("data/all");
				int index=LGL_RandInt(0,allVideos.GetFileCount()-1);
				sprintf(target,"%s/%s",allVideos.GetPath(),allVideos.GetFileName(index));
				printf("Video[lo: %s [%s]\n",target,LGL_FileExists(target)?"OK":"NO FILE");
				if(vidLo==NULL)
				{
					vidLo=new LGL_Video(target);
				}
				else
				{
					vidLo->SetVideo(target);
				}
			}
			else
			{
				if(vidLo==NULL)
				{
					vidLo=new LGL_Video(lowVids[brIndexNow]);
				}
				else
				{
					vidLo->SetVideo(lowVids[brIndexNow]);
				}
			}
			if(strcmp(hiVids[brIndexNow],"rnd")==0)
			{
				LGL_DirTree allVideos("data/all");
				int index=LGL_RandInt(0,allVideos.GetFileCount()-1);
				sprintf(target,"%s/%s",allVideos.GetPath(),allVideos.GetFileName(index));
				printf("Video[hi]: %s [%s]\n",target,LGL_FileExists(target)?"OK":"NO FILE");
				if(vidHi==NULL)
				{
					vidHi=new LGL_Video(target);
				}
				else
				{
					vidHi->SetVideo(target);
				}
			}
			else
			{
				if(vidHi==NULL)
				{
					vidHi=new LGL_Video(hiVids[brIndexNow]);
				}
				else
				{
					vidHi->SetVideo(hiVids[brIndexNow]);
				}
			}
			useSetVid=false;
		}
	}
#endif	//BEAT_RESEARCH

		//Check for early out
		LGL_ProcessInput();
		if(LGL_KeyStroke(SDLK_ESCAPE))
		{
			fclose(mencoderFD);
			LGL_FileDelete(targetVideoOutPath);
			exit(-1);
		}

		//Analyze
		const long sampleNow=(long)(frame*44100.0f/frameRate);
		const long sampleLast=(long)LGL_Min(snd->GetLengthSamples(),(sampleNow+2*(44100/60)));
		const int sampleSkipFactor=1;
		float zeroCrossingFactor;
		float magnitudeAve;
		float magnitudeMax;
		bool overdriven;
		analyzeWaveSegment
		(
			buf16,
			len16,
			true,
			sampleNow,
			sampleLast,
			sampleSkipFactor,
			1.0f,	//volumeMultiplierNow
			zeroCrossingFactor,
			magnitudeAve,
			magnitudeMax,
			overdriven
		);

#ifdef	BEAT_RESEARCH
if(brIndexNow<20)
{
	magnitudeAve=LGL_Min(1.0f,magnitudeAve*2);
	magnitudeMax=LGL_Min(1.0f,magnitudeMax*2);
}
#endif	//BEAT_RESEARCH

if(firstFrame)
{
	//First frame shall be black
	firstFrame=false;
}
else if(useSetVid)
{
	LGL_Video* vid=setVid;
	vid->SetPrimaryDecoder();
	float time=timeStartLocal[brIndexNow] + (secondsNow-timeStartGlobal[brIndexNow]);
	vid->SetTime(time);
	vid->ForceImageUpToDate();
	int a=0;
	while(!vid->ImageUpToDate())
	{
		LGL_DelayMS(50);
		if(a==59)
		{
			printf("Frame not decoded: %s\n",vid->GetPathShort());
			break;
		}
		a++;
	}
	a=0;
	LGL_Image* image=vid->LockImage();
	while(image==NULL)
	{
		vid->UnlockImage(NULL);	//Still gotta unlock it, even though it's NULL...
		LGL_DelayMS(50);
		image=vid->LockImage();
		if(a==59)
		{
			printf("Image not locked: %s\n",vid->GetPathShort());
			break;
		}
		a++;
	}
	assert(image);
	float bright=1.0f;
	image->DrawToScreen
	(
		0.0f,1.0f,
		0.0f,1.0f,
		0.0f,
		bright,bright,bright,0.0f
	);
	vid->UnlockImage(image);
}
else
{
		//Draw Videos
		for(int a=0;a<3;a++)
		{
			LGL_Video* vid=NULL;
			float factor=0.0f;
			float time=0.0f;
			float fadeFactor=0.0f;

			if(zeroCrossingFactor<0.25f)
			{
				fadeFactor=0.0f;
			}
			else if(zeroCrossingFactor<0.75f)
			{
				fadeFactor=2.0f*(zeroCrossingFactor-0.25f);
			}
			else
			{
				fadeFactor=1.0f;
			}

			if(a==0)
			{
				vid=vidBG;
				float magnitudeThreashold=LGL_Clamp(0.0f,(magnitudeAve-0.1f)*2,1.0f);
				factor=0.1f*(1.0f-magnitudeThreashold);
				time=vidBGTime;
			}
			else if(a==1)
			{
				vid=vidLo;
				float magnitudeThreashold=LGL_Clamp(0.0f,(magnitudeAve-0.1f)*8,1.0f);
				factor=0.25f*magnitudeThreashold*(1.0f-fadeFactor*fadeFactor);
				time=vidLoTime;
			}
			else if(a==2)
			{
				vid=vidHi;
				float magnitudeThreashold=LGL_Clamp(0.0f,(magnitudeAve-0.1f)*8,1.0f);
				float magnitudeLoud=LGL_Clamp(0.0f,(magnitudeMax-0.5f)*4,1.0f);
				factor=4.0f*(zeroCrossingFactor*(0.5f+0.5f*magnitudeLoud))*magnitudeThreashold*(0.0f+fadeFactor);
#ifdef	BEAT_RESEARCH
				factor=LGL_Min(8.0f,factor*hiScalar[brIndexNow]);
#endif
				time=vidHiTime;
			}
			if(vid==NULL) continue;

			if(factor>0.0f)
			{
				vid->SetPrimaryDecoder();
				vid->SetTime(time);
				vid->ForceImageUpToDate();
				int a=0;
				while(!vid->ImageUpToDate())
				{
					LGL_DelayMS(50);
					if(a==59)
					{
						printf("Frame not decoded: %s\n",vid->GetPathShort());
						break;
					}
					a++;
				}
				a=0;
				LGL_Image* image=vid->LockImage();
				while(image==NULL)
				{
					vid->UnlockImage(NULL);	//Still gotta unlock it, even though it's NULL...
					LGL_DelayMS(50);
					image=vid->LockImage();
					if(a==59)
					{
						printf("Image not locked: %s\n",vid->GetPathShort());
						break;
					}
					a++;
				}
				assert(image);
				while(factor>0.0f)
				{
					float bright=LGL_Min(1.0f,factor);
					image->DrawToScreen
					(
						0.0f,1.0f,
						0.0f,1.0f,
						0.0f,
						bright,bright,bright,0.0f
					);
					factor-=1.0f;
				}
				vid->UnlockImage(image);
			}
			else
			{
				//We're not drawing, so pick a new time
				if(a==1)
				{
					vidLoTime=LGL_RandFloat(10.0f,vid->GetLengthSeconds()-30.0f);
				}
				else if(a==2)
				{
					vidHiTime=LGL_RandFloat(10.0f,vid->GetLengthSeconds()-30.0f);
				}
			}
		}
}

		//Advance videos
		vidBGTime+=1.0f/frameRate;
		vidLoTime+=1.0f/frameRate;
#ifdef	BEAT_RESEARCH
		vidHiTime+=hiScalarSpeed[brIndexNow]/frameRate;
#else
		vidHiTime+=4.0f/frameRate;
#endif	//BEAT_RESEARCH
		if(vidBG && vidBGTime>vidBG->GetLengthSeconds()-10.0f)
		{
			vidBGTime=10.0f;
		}
		if(vidLo && vidLoTime>vidLo->GetLengthSeconds()-10.0f)
		{
			vidLoTime=10.0f;
		}
		if(vidHi && vidHiTime>vidHi->GetLengthSeconds()-10.0f)
		{
			vidHiTime=10.0f;
		}

		//Capture Buffer
		unsigned char* pixels=(unsigned char*)malloc(3*ResX*ResY);
		if(pixels==NULL)
		{
			printf("LGL_ScreenShot(): Error! Unable to malloc() pixel buffer.\n");
			exit(-1);
		}

		glReadBuffer(GL_BACK_LEFT);
		glReadPixels
		(
			0, 0,
			ResX,ResY,
			GL_RGB, GL_UNSIGNED_BYTE,
			pixels
		);

		//Send Buffer to mencoder
		fwrite(pixels,3*ResX*ResY,1,mencoderFD);
		free(pixels);

		//Status
		float pct = frame/(frameRate*snd->GetLengthSeconds());
		LGL_DrawRectToScreen
		(
			0,pct,
			0,0.05f,
			.4f*pct,.2f*pct,0.5f+0.5f*pct,1.0f
		);
		LGL_GetFont().DrawString
		(
			0.49f,0.015f,0.02f,
			1,1,1,1,
			false,
			.75f,
			"%.1f%%",
			pct*100.0f
		);

		//Swap
		LGL_SwapBuffers();
	}

	snd->PrepareForDelete();
	while(snd->ReadyForDelete()==false)
	{
		LGL_DelayMS(5);
	}
	delete snd;
	if(vidBG) delete vidBG;
	if(vidLo) delete vidLo;
	if(vidHi) delete vidHi;

	fclose(mencoderFD);
	return(0);
}

