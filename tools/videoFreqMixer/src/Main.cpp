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
#include "../../../src/ConfigFile.cpp"	//This is so screwy, but it works.
#include "../../../src/Config.cpp"	//This is so screwy, but it works.
#include "../../../src/FileInterface.cpp"	//No less screwy. No less functional.

char targetVideoLoPath[1024];
char targetVideoHiPath[1024];
char targetVideoOutPath[1024];
char targetVideoOutPathTmp[1024];

void
DrawStatus(float pct)
{
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

	if(pct>0.001f)
	{
		float pctPerSecond = pct/LGL_SecondsSinceExecution();
		float secondsLeft = ((1.0f-pct)*(1.0f/pctPerSecond));
		float minutesLeft = floorf(secondsLeft/60.0f);
		secondsLeft = secondsLeft - minutesLeft*60.0f;
		LGL_GetFont().DrawString
		(
			0.49f,0.065f,0.02f,
			1,1,1,1,
			false,
			.75f,
			"%.0f:%.2i",minutesLeft,(int)secondsLeft
		);
	}
}

LGL_Sound*
LoadRecordSound
(
	const
	char*	audioFile
)
{
	long soundBufferLength=4*44100*60*8;
	Uint8* soundBuffer=(Uint8*)malloc(soundBufferLength);
	assert(soundBuffer);
	LGL_Sound* recordSound=new LGL_Sound(audioFile,true,2,soundBuffer,soundBufferLength);
	recordSound->SetHogCPU();
	for(;;)
	{
		LGL_ProcessInput();
		if(LGL_KeyStroke(SDLK_ESCAPE))
		{
			exit(-1);
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
		printf("Error! Music '%s' has zero length!\n",audioFile);
		exit(0);
	}
	return(recordSound);
}

int	ResX=852;
int	ResY=480;
int	ResFPS=60;

LGL_Sound*	snd=NULL;
char		sndPathListen[2048];
char		sndPathEncode[2048];

LGL_VideoDecoder*	vidLo=NULL;
LGL_VideoDecoder*	vidHi=NULL;
char		vidLoPath[2048];
char		vidHiPath[2048];
LGL_DirTree*	dirTreeLo=NULL;
LGL_DirTree*	dirTreeHi=NULL;
float		vidLoBrightScalar=1.0f;
float		vidHiBrightScalar=1.0f;
float		vidLoBrightConst=-1.0f;
float		vidHiBrightConst=-1.0f;
float		vidLoBrightBase=0.0f;
float		vidHiBrightBase=0.0f;
float		vidLoBrightBaseDelta=0.0f;
float		vidHiBrightBaseDelta=0.0f;
float		vidLoBrightBaseTarget=-1.0f;
float		vidHiBrightBaseTarget=-1.0f;
float		vidLoPlaybackRate=1.0f;
float		vidHiPlaybackRate=2.0f;
float		vidLoPlaybackRateCounter=0.0f;
float		vidHiPlaybackRateCounter=0.0f;
float		vidLoTimeProxy=0.0f;
float		vidHiTimeProxy=0.0f;
float		vidLoPlaybackRateLerpAlphaTime=-1.0f;
float		vidLoPlaybackRateLerpAlphaRate=-1.0f;
float		vidLoPlaybackRateLerpOmegaTime=-1.0f;
float		vidLoPlaybackRateLerpOmegaRate=-1.0f;
float		vidHiPlaybackRateLerpAlphaTime=-1.0f;
float		vidHiPlaybackRateLerpAlphaRate=-1.0f;
float		vidHiPlaybackRateLerpOmegaTime=-1.0f;
float		vidHiPlaybackRateLerpOmegaRate=-1.0f;
bool		vidLoRndOnBlack=true;
bool		vidHiRndOnBlack=true;
float		vidLoPrevBrightness=0.0f;
float		vidHiPrevBrightness=0.0f;
bool		vidEncode=true;
float		vidEncodeTime=-0.01f;
char		vidEncodeCodec[2048];
bool		vidDrawPath=false;
char		rem[2048];


FILE*		scriptFD=NULL;
char		scriptPath[1024];
int		scriptLine=0;
float		scriptTimeNext=-1.0f;

void
printErrorPrefix()
{
	printf("%s[%i]: ERROR! ",scriptPath,scriptLine);
}

void
verifyArgNum
(
	const char*	directiveName,
	int		argNumReq,
	int		argNumGot
)
{
	if(argNumReq!=argNumGot)
	{
		printErrorPrefix();
		printf("'%s' directive must have %i argument (not %i)\n",directiveName,argNumReq,argNumGot);
		exit(-1);
	}
}

void
verifyInitTime
(
	const char*	directiveName
)
{
	if(scriptTimeNext>=0.0f)
	{
		printErrorPrefix();
		printf("'%s' cannot be defined after time zero!\n",directiveName);
		exit(-1);
	}
}

void
verifyFileExists
(
	const char*	directiveName,
	const char*	path
)
{
	if(LGL_FileExists(path)==false)
	{
		printErrorPrefix();
		printf("'%s' file '%s' doesn't exist!\n",directiveName,path);
		exit(-1);
	}
}

void
getVideoPathForRes
(
	char*	vidPath
)
{
return;
/*
	if(ResY!=480)
	{
		return;
	}

	char vidPathOrig[2048];
	strcpy(vidPathOrig,vidPath);

	char vidPathDir[2048];
	strcpy(vidPathDir,vidPath);
	if(char* lastSlash = strrchr(vidPathDir,'/'))
	{
		lastSlash[0]='\0';
	}
	else
	{
		return;
	}

	char vidPathShort[2048];
	strcpy(vidPathShort,&(strrchr(vidPath,'/')[1]));

	char vidPathDirRes[2048];
	sprintf(vidPathDirRes,"%i",ResY);

	if(LGL_DirectoryExists(vidPathDirRes)==false)
	{
		LGL_DirectoryCreate(vidPathDirRes);
	}

	char vidPathDirResName[2048];
	sprintf(vidPathDirResName,"%s/%s",vidPathDirRes,vidPathShort);

	if(LGL_FileExists(vidPathDirResName)==false)
	{
		bool scale=true;
		LGL_VideoDecoder testVid(vidPathOrig);
		LGL_Image* testImg = testVid.GetImage();
		bool printed=false;
		LGL_Timer timer;
		while(testImg==NULL)
		{
			testImg=testVid.GetImage();
			LGL_DelayMS(20);
			if(timer.SecondsSinceLastReset()>1.0f && printed==false)
			{
				printf("Blocked on '%s'\n",vidPathOrig);
				printed=true;
			}
		}
		if(testImg->GetHeight()<=480)
		{
			scale=false;
		}

		char cmd[2048];
		if(scale)
		{
			sprintf
			(
				cmd,
				"mencoder '%s' -vf scale -zoom -xy %i -o '%s' -ovc lavc -lavcopts vcodec=ljpeg -oac copy",
				vidPathOrig,
				ResX,
				vidPathDirResName
			);
		}
		else
		{
			//Just symlink
			LGL_FileDelete(vidPathDirResName);	//Removes any stale symlinks
			sprintf
			(
				cmd,
				"ln -s '../%s' '%s'",
				vidPathOrig,
				vidPathDirResName
			);
		}
		printf("Running command:\n\t%s\n\n",cmd);
		system(cmd);
	}

	assert(LGL_FileExists(vidPathDirResName));
	strcpy(vidPath,vidPathDirResName);
*/
}

bool
getRandomVideoPath
(
	char*		vidPath,
	LGL_DirTree*	dirTree
)
{
	if(LGL_DirectoryExists(vidPath)==false)
	{
		printErrorPrefix();
		printf("'%s' doesn't exist!\n",vidPath);
		exit(-1);
	}

	bool moreThanOne=(dirTree->GetFileCount()>1);
	
	//Ensure all res-files exist
	for(unsigned int a=0;a<dirTree->GetFileCount();a++)
	{
		char tmpPath[2048];
		sprintf(tmpPath,"%s/%s",dirTree->GetPath(),dirTree->GetFileName(a));
		getVideoPathForRes(tmpPath);
	}

	int index=LGL_RandInt(0,dirTree->GetFileCount()-1);
	sprintf(vidPath,"%s/%s",dirTree->GetPath(),dirTree->GetFileName(index));

	getVideoPathForRes(vidPath);

	return(moreThanOne);
}

void
processScript()
{
	if(scriptFD==NULL)
	{
		return;
	}
	if(scriptTimeNext<=vidEncodeTime)
	{
		rem[0]='\0';
	}
	while(scriptTimeNext<=vidEncodeTime)
	{
		char buf[1024];
		if(fgets(buf,1024,scriptFD)==NULL)
		{
			fclose(scriptFD);
			scriptFD=NULL;
			break;
		}
		scriptLine++;

		FileInterfaceObj fi;
		fi.ReadLine(buf);

		if(fi.Size()==0)
		{
			continue;
		}
		else if(fi[0][0]=='\0')
		{
			continue;
		}
		else if(strstr(buf,"//")==buf)
		{
			//Meh
		}
		else if(strcasecmp(fi[0],"rem")==0)
		{
			if(fi.Size()==2)
			{
				strcpy(rem,fi[1]);
			}
		}
		else if(strcasecmp(fi[0],"time")==0)
		{
			verifyArgNum("time",1,fi.Size()-1);
			float candidate=atof(fi[1]);
			if(candidate<scriptTimeNext)
			{
				printErrorPrefix();
				printf("'TIME' %.2f is less than previous time %.2f\n",candidate,scriptTimeNext);
				exit(-1);
			}
			scriptTimeNext=candidate;
		}
		else if(strcasecmp(fi[0],"encSize")==0)
		{
			verifyArgNum("encSize",1,fi.Size()-1);
			if(strcasecmp(fi[1],"1080p")==0)
			{
				ResX=1920;
				ResY=1080;
			}
			else if(strcasecmp(fi[1],"720p")==0)
			{
				ResX=1280;
				ResY=720;
			}
			else if(strcasecmp(fi[1],"480p")==0)
			{
				ResX=852;
				//ResX=1920;
				ResY=480;
			}
			else
			{
				printErrorPrefix();
				printf("'encSize' '%s' isn't a valid option! Pick 1080p, 720p, or 480p\n",fi[1]);
				exit(-1);
			}
		}
		else if(strcasecmp(fi[0],"encFPS")==0)
		{
			verifyArgNum("encFPS",1,fi.Size()-1);
			ResFPS=atof(fi[1]);
		}
		else if(strcasecmp(fi[0],"encCodec")==0)
		{
			verifyArgNum("encCodec",1,fi.Size()-1);
			strcpy(vidEncodeCodec,fi[1]);
		}
		else if(strcasecmp(fi[0],"ListenMusic")==0)
		{
			verifyArgNum("ListenMusic",1,fi.Size()-1);
			if(snd!=NULL)
			{
				printErrorPrefix();
				printf("'ListenMusic' cannot be defined twice!\n");
				exit(-1);
			}
			verifyInitTime("ListenMusic");
			verifyFileExists("ListenMusic",fi[1]);

			strcpy(sndPathListen,fi[1]);
		}
		else if(strcasecmp(fi[0],"EncodeMusic")==0)
		{
			verifyArgNum("EncodeMusic",1,fi.Size()-1);
			if(snd!=NULL)
			{
				printErrorPrefix();
				printf("'EncodeMusic' cannot be defined twice!\n");
				exit(-1);
			}
			verifyInitTime("EncodeMusic");
			verifyFileExists("EncodeMusic",fi[1]);

			strcpy(sndPathEncode,fi[1]);
		}
		else if(strcasecmp(fi[0],"encode")==0)
		{
			verifyArgNum("encode",1,fi.Size()-1);
			vidEncode=atoi(fi[1])!=0;
		}
		else if(strcasecmp(fi[0],"vidLoPath")==0)
		{
			verifyArgNum("vidLoPath",1,fi.Size()-1);

			char vidPath[2048];
			strcpy(vidPath,fi[1]);
			if(LGL_DirectoryExists(fi[1]))
			{
				strcpy(vidLoPath,vidPath);
				delete dirTreeLo;
				dirTreeLo = new LGL_DirTree(vidPath);
				getRandomVideoPath(vidPath,dirTreeLo);
			}
			else if(LGL_FileExists(fi[1]))
			{
				//Cool
				getVideoPathForRes(vidPath);
			}
			else
			{
				printErrorPrefix();
				printf("'vidLoPath' '%s' doesn't exist!\n",fi[1]);
				exit(-1);
			}
			verifyFileExists("vidLoPath",vidPath);

			if(vidLo)
			{
				vidLo->SetVideo(vidPath);;
			}
			else
			{
				vidLo = new LGL_VideoDecoder(vidPath);
				vidLoTimeProxy=LGL_RandFloat(0.0f,vidLo->GetLengthSeconds()-1);
			}
			vidLo->SetFrameBufferAddRadius(2);
			vidLoTimeProxy=LGL_RandFloat(0.0f,vidLo->GetLengthSeconds());
		}
		else if(strcasecmp(fi[0],"vidHiPath")==0)
		{
			verifyArgNum("vidHiPath",1,fi.Size()-1);

			char vidPath[2048];
			strcpy(vidPath,fi[1]);
			if(LGL_DirectoryExists(fi[1]))
			{
				strcpy(vidHiPath,vidPath);
				delete dirTreeHi;
				dirTreeHi = new LGL_DirTree(vidPath);
				getRandomVideoPath(vidPath,dirTreeHi);
			}
			else if(LGL_FileExists(fi[1]))
			{
				//Cool
				getVideoPathForRes(vidPath);
			}
			else
			{
				printErrorPrefix();
				printf("'vidHiPath' '%s' doesn't exist!\n",fi[1]);
				exit(-1);
			}
			verifyFileExists("vidHiPath",vidPath);

			if(vidHi)
			{
				vidHi->SetVideo(vidPath);;
			}
			else
			{
				vidHi = new LGL_VideoDecoder(vidPath);
				vidHiTimeProxy=LGL_RandFloat(0.0f,vidHi->GetLengthSeconds()-1);
			}
			vidHi->SetFrameBufferAddRadius(2);
			vidHiTimeProxy=LGL_RandFloat(0.0f,vidHi->GetLengthSeconds());
		}
		else if(strcasecmp(fi[0],"vidLoRndOnBlack")==0)
		{
			verifyArgNum("vidLoRndOnBlack",1,fi.Size()-1);
			vidLoRndOnBlack=atoi(fi[1])!=0;
		}
		else if(strcasecmp(fi[0],"vidHiRndOnBlack")==0)
		{
			verifyArgNum("vidHiRndOnBlack",1,fi.Size()-1);
			vidHiRndOnBlack=atoi(fi[1])!=0;
		}
		else if(strcasecmp(fi[0],"vidLoPlaybackRate")==0)
		{
			verifyArgNum("vidLoPlaybackRate",1,fi.Size()-1);
			vidLoPlaybackRate=atof(fi[1]);
			vidLoPlaybackRateLerpAlphaTime=-1.0f;
		}
		else if(strcasecmp(fi[0],"vidHiPlaybackRate")==0)
		{
			verifyArgNum("vidHiPlaybackRate",1,fi.Size()-1);
			vidHiPlaybackRate=atof(fi[1]);
			vidHiPlaybackRateLerpAlphaTime=-1.0f;
		}
		else if(strcasecmp(fi[0],"vidLoPlaybackRateLerp")==0)
		{
			verifyArgNum("vidLoPlaybackRateLerp",3,fi.Size()-1);
			vidLoPlaybackRate=atof(fi[1]);
			vidLoPlaybackRateLerpAlphaTime=vidEncodeTime;
			vidLoPlaybackRateLerpAlphaRate=atof(fi[1]);
			vidLoPlaybackRateLerpOmegaTime=vidLoPlaybackRateLerpAlphaTime+atof(fi[3]);
			vidLoPlaybackRateLerpOmegaRate=atof(fi[2]);
		}
		else if(strcasecmp(fi[0],"vidHiPlaybackRateLerp")==0)
		{
			verifyArgNum("vidHiPlaybackRateLerp",3,fi.Size()-1);
			vidHiPlaybackRate=atof(fi[1]);
			vidHiPlaybackRateLerpAlphaTime=vidEncodeTime;
			vidHiPlaybackRateLerpAlphaRate=atof(fi[1]);
			vidHiPlaybackRateLerpOmegaTime=vidHiPlaybackRateLerpAlphaTime+atof(fi[3]);
			vidHiPlaybackRateLerpOmegaRate=atof(fi[2]);
		}
		else if(strcasecmp(fi[0],"vidLoBrightScalar")==0)
		{
			verifyArgNum("vidLoBrightScalar",1,fi.Size()-1);
			vidLoBrightScalar=atof(fi[1]);
		}
		else if(strcasecmp(fi[0],"vidHiBrightScalar")==0)
		{
			verifyArgNum("vidHiBrightScalar",1,fi.Size()-1);
			vidHiBrightScalar=atof(fi[1]);
		}
		else if(strcasecmp(fi[0],"vidLoBrightConst")==0)
		{
			verifyArgNum("vidLoBrightConst",1,fi.Size()-1);
			vidLoBrightConst=atof(fi[1]);
		}
		else if(strcasecmp(fi[0],"vidHiBrightConst")==0)
		{
			verifyArgNum("vidHiBrightConst",1,fi.Size()-1);
			vidHiBrightConst=atof(fi[1]);
		}
		else if(strcasecmp(fi[0],"vidLoBrightBase")==0)
		{
			verifyArgNum("vidLoBrightBase",1,fi.Size()-1);
			vidLoBrightBase=atof(fi[1]);
			vidLoBrightBaseDelta=0;
		}
		else if(strcasecmp(fi[0],"vidHiBrightBase")==0)
		{
			verifyArgNum("vidHiBrightBase",1,fi.Size()-1);
			vidHiBrightBase=atof(fi[1]);
			vidHiBrightBaseDelta=0;
		}
		else if(strcasecmp(fi[0],"vidLoBrightBaseLerp")==0)
		{
			verifyArgNum("vidLoBrightBaseLerp",3,fi.Size()-1);
			vidLoBrightBase=atof(fi[1]);
			vidLoBrightBaseTarget=atof(fi[2]);
			vidLoBrightBaseDelta=(1.0f/(ResFPS*atof(fi[3]))) * (vidLoBrightBaseTarget-vidLoBrightBase);
		}
		else if(strcasecmp(fi[0],"vidHiBrightBaseLerp")==0)
		{
			verifyArgNum("vidHiBrightBaseLerp",3,fi.Size()-1);
			vidHiBrightBase=atof(fi[1]);
			vidHiBrightBaseTarget=atof(fi[2]);
			vidHiBrightBaseDelta=(1.0f/(ResFPS*atof(fi[3]))) * (vidHiBrightBaseTarget-vidHiBrightBase);
		}
		else if(strcasecmp(fi[0],"vidLoTime")==0)
		{
			verifyArgNum("vidLoTime",1,fi.Size()-1);
			if(vidLo==NULL)
			{
				printErrorPrefix();
				printf("vidLoTime cannot be specified before vidLoPath!\n");
				exit(-1);
			}

			vidLoTimeProxy=atof(fi[1]);
		}
		else if(strcasecmp(fi[0],"vidHiTime")==0)
		{
			verifyArgNum("vidHiTime",1,fi.Size()-1);
			if(vidHi==NULL)
			{
				printErrorPrefix();
				printf("vidHiTime cannot be specified before vidHiPath!\n");
				exit(-1);
			}

			vidHiTimeProxy=atof(fi[1]);
		}
		else if(strcasecmp(fi[0],"drawVideoPath")==0)
		{
			verifyArgNum("vidHiTime",1,fi.Size()-1);
			vidDrawPath=(fi[1][0]!='0');
		}
		else
		{
			printErrorPrefix();
			printf("Unknown directive '%s'\n",fi[0]);
			exit(-1);
		}
	}
}

int
main
(
	int	argc,
	char**	argv
)
{
	if(argc!=2)
	{
		printf("Usage: ./videoFreqMixer.lin script.txt\n\n");
		exit(0);
	}

	strcpy(scriptPath,argv[1]);
	strcpy(vidEncodeCodec,"ljpeg");
	scriptFD = fopen(scriptPath,"r");
	if(scriptFD==NULL)
	{
		printf("ERROR: Cannot open '%s'\n",argv[1]);
		exit(-1);
	}

	processScript();

	char title[2048];
	sprintf(title,"%s %s\n",argv[0],sndPathListen);

	//Init LGL
	LGL_Init
	(
		ResX,
		ResY,
		0,
		argv[0]
	);

#if 0
	LGL_VideoDecoder* baka=new LGL_VideoDecoder("test/test1.avi");
	baka->SetFrameBufferAddRadius(0);
	baka->SetTime(5.0f);
	for(;;)
	{
		LGL_Image* img = baka->GetImage();
		bool imgNULL = img==NULL;
		if
		(
			imgNULL==false
		)
		{
			break;
		}
		printf("UTD, NULL: %s, %s\n",
			"OK"
			(imgNULL==false) ? "OK" : "NULL"
		);
		LGL_DelayMS(50);
	}
	LGL_Image* img = baka->GetImage();
	img->DrawToScreen();
	LGL_SwapBuffers();
	for(;;)
	{
		LGL_ProcessInput();
		if(LGL_KeyDown(SDLK_SPACE))
		{
			break;
		}
		LGL_DelayMS(50);
	}

for(int a=0;a<40;a++) printf("\n");

	baka->ImageFront=NULL;
	//baka->ImageBack=NULL;

	baka->SetVideo("test/test2.avi");
	baka->SetTime(5.0f);
	for(;;)
	{
		LGL_Image* img = baka->GetImage();
		bool imgNULL = img==NULL;
		if
		(
			imgNULL==false
		)
		{
			break;
		}
		LGL_DelayMS(50);
	}
printf("Drawing!\n");
	img = baka->GetImage();
	img->DrawToScreen();

	baka->ImageFront->DrawToScreen(0.1,0.2,0.1,0.2);
	baka->ImageBack->DrawToScreen(0.8,0.9,0.1,0.2);

	printf("Front: %s\n",baka->ImageFront->GetPath());
	printf("Back: %s\n",baka->ImageBack->GetPath());

	LGL_SwapBuffers();
	for(;;)
	{
		LGL_ProcessInput();
		if(LGL_KeyDown(SDLK_SPACE))
		{
			break;
		}
		LGL_DelayMS(50);
	}
	exit(0);
#endif

	vidEncodeTime=0.0f;
	processScript();
			
	snd=LoadRecordSound(sndPathListen);
	if(snd==NULL)
	{
		printf("'listenMusic' file '%s' failed to load!\n",sndPathListen);
		exit(-1);
	}

	if(snd==NULL)
	{
		printf("ERROR! Must specify snd in script '%s' before first 'time' directive!\n",scriptPath);
		exit(-1);
	}
	if(vidLo==NULL)
	{
		printf("ERROR! Must specify vidLoPath in script '%s' before first 'time' directive!\n",scriptPath);
		exit(-1);
	}
	if(vidHi==NULL)
	{
		printf("ERROR! Must specify vidHiPath in script '%s' before first 'time' directive!\n",scriptPath);
		exit(-1);
	}

	char* p=sndPathListen;
	while(strstr(p,"/"))
	{
		p=&(strstr(p,"/")[1]);
	}
	if(LGL_DirectoryExists("tmp")==false)
	{
		LGL_DirectoryCreate("tmp");
	}
	sprintf(targetVideoOutPathTmp,"tmp/tmp.mjpeg.avi");
	sprintf(targetVideoOutPath,"out/%s.mjpeg.avi",p);

	//Lock Sound
	snd->LockBufferForReading(10);

	//Fire up mencoder
	char cmd[1024];
	const char* cmdInput;
	if(strstr(sndPathListen,".ogg"))
	{
		cmdInput="/home/emf/install-dev/mplayer/mencoder - -nosound -demuxer rawvideo -rawvideo fps=60:w=%i:h=%i:format=rgb24 -idx -flip -ovc lavc -lavcopts vcodec=%s:vhq:vbitrate=%i:autoaspect=1:threads=1 -vf scale=%i:%i,harddup -noskip -of avi -o \"%s\"";// 1>/dev/null 2>/dev/null";
		sprintf
		(
			cmd,
			cmdInput,
			ResX,
			ResY,
			vidEncodeCodec,
			16000,	//bitrateHQ,
			ResX,
			ResY,
			targetVideoOutPathTmp
		);
	}
	else
	{
		cmdInput="/home/emf/install-dev/mplayer/mencoder - -audiofile \"%s\" -oac copy -demuxer rawvideo -rawvideo fps=60:w=%i:h=%i:format=rgb24 -idx -flip -ovc lavc -lavcopts vcodec=%s:vhq:vbitrate=%i:autoaspect=1:threads=1 -vf scale=%i:%i,harddup -noskip -of avi -o \"%s\"";// 1>/dev/null 2>/dev/null";
		sprintf
		(
			cmd,
			cmdInput,
			sndPathEncode,
			ResX,
			ResY,
			vidEncodeCodec,
			16000,	//bitrateHQ,
			ResX,
			ResY,
			targetVideoOutPathTmp
		);
	}

	if(vidEncode==false)
	{
		sprintf
		(
			cmd,
			"cat - > /dev/null"
		);
	}

	//const char* cmdInput="mencoder - -audiofile \"%s\" -oac copy -demuxer rawvideo -rawvideo fps=60:w=%i:h=%i:format=rgb24 -idx -flip -ovc lavc -lavcopts vcodec=mpeg4:mbd=2:trell=yes:v4mv=yes:vbitrate=%i:autoaspect=1:threads=1 -vf scale=%i:%i,harddup -noskip -of avi -o \"%s\" 1>/dev/null 2>/dev/null";
printf("cmd:\n\t%s\n",cmd);
	FILE* mencoderFD=popen(cmd,"w");
	assert(mencoderFD);

	float secondsStart=0.0f;
	float secondsEnd=9999.0f;
	//Loop
	for(int frame=secondsStart*60.0f;frame<snd->GetLengthSeconds()*60 && frame<secondsEnd*60.0f;frame++)
	{
		//Check for early out
		LGL_ProcessInput();
		if(LGL_KeyStroke(SDLK_ESCAPE))
		{
			break;
		}

		/*
		if
		(
			LGL_KeyStroke(SDLK_SPACE)==false &&
			LGL_KeyDown(SDLK_n)==false
		)
		{
			frame--;
			continue;
		}
printf("[%i]\n",frame);
		*/

		//Script Stuff
		vidEncodeTime=frame/60.0f;
		processScript();

		if(vidLoPlaybackRateLerpAlphaTime!=-1.0f)
		{
			float lerp =
				(vidEncodeTime-vidHiPlaybackRateLerpAlphaTime) /
				(vidLoPlaybackRateLerpOmegaTime-vidLoPlaybackRateLerpAlphaTime);
			if(lerp>1.0f) lerp=1.0f;
			vidLoPlaybackRate=
				(1.0f-lerp)*vidLoPlaybackRateLerpAlphaRate +
				(0.0f+lerp)*vidLoPlaybackRateLerpOmegaRate;
			if(lerp==1.0f)
			{
				vidLoPlaybackRateLerpAlphaTime=-1.0f;
			}
		}
		if(vidHiPlaybackRateLerpAlphaTime!=-1.0f)
		{
			float lerp =
				(vidEncodeTime-vidHiPlaybackRateLerpAlphaTime) /
				(vidHiPlaybackRateLerpOmegaTime-vidHiPlaybackRateLerpAlphaTime);
			if(lerp>1.0f) lerp=1.0f;
			vidHiPlaybackRate=
				(1.0f-lerp)*vidHiPlaybackRateLerpAlphaRate +
				(0.0f+lerp)*vidHiPlaybackRateLerpOmegaRate;

			if(lerp==1.0f)
			{
				vidHiPlaybackRateLerpAlphaTime=-1.0f;
			}
		}

		//Analyze
		const float timeNow=(frame/60.0f);
		const float timeRadius=2*(1.0f/60.0f);
		float zeroCrossingFactor;
		float magnitudeAve;
		float magnitudeMax;

		snd->GetMetadata
		(
			LGL_Max(0,timeNow-timeRadius),
			LGL_Min(snd->GetLengthSeconds(),timeNow+timeRadius),
			zeroCrossingFactor,
			magnitudeAve,
			magnitudeMax
		);

		//Draw Videos
		for(int a=0;a<2;a++)
		{
			bool advance=true;

			LGL_VideoDecoder* vid=(a==0) ? vidLo : vidHi;
			if(vid==NULL) continue;

			float timeProxy = (a==0) ? vidLoTimeProxy : vidHiTimeProxy;
			if(timeProxy>vid->GetLengthSeconds()-1)
			{
				if(a==0)
				{
					vidLoTimeProxy=LGL_RandFloat(0.0f,vidLo->GetLengthSeconds()-1);
				}
				else if(a==1)
				{
					vidHiTimeProxy=LGL_RandFloat(0.0f,vidHi->GetLengthSeconds()-1);
				}
				timeProxy = (a==0) ? vidLoTimeProxy : vidHiTimeProxy;
			}

			if(vidLoBrightBaseDelta!=0.0f)
			{
				vidLoBrightBase+=vidLoBrightBaseDelta;
				if
				(
					(vidLoBrightBaseDelta<0) ?
					(vidLoBrightBase<=vidLoBrightBaseTarget) :
					(vidLoBrightBase>=vidLoBrightBaseTarget)
				)
				{
					vidLoBrightBase=vidLoBrightBaseTarget;
					vidLoBrightBaseDelta=0.0f;
				}
			}
			if(vidHiBrightBaseDelta!=0.0f)
			{
				vidHiBrightBase+=vidHiBrightBaseDelta;
				if
				(
					(vidHiBrightBaseDelta<0) ?
					(vidHiBrightBase<=vidHiBrightBaseTarget) :
					(vidHiBrightBase>=vidHiBrightBaseTarget)
				)
				{
					vidHiBrightBase=vidHiBrightBaseTarget;
					vidHiBrightBaseDelta=0.0f;
				}
			}

			float brightness = ((a==0) ? vidLoBrightBase : vidHiBrightBase) + (GetFreqBrightness(a==1,zeroCrossingFactor,magnitudeAve) * ((a==0) ? vidLoBrightScalar : vidHiBrightScalar));
			if(((a==0) ? vidLoBrightConst : vidHiBrightConst) != -1.0f)
			{
				brightness = (a==0) ? vidLoBrightConst : vidHiBrightConst;
			}
			float factor=brightness;
			if(factor>0.0f)
			{
				vid->SetTime(timeProxy);
				LGL_Image* image=vid->GetImage();

				long frameNum = (long)(vid->GetTime()*vid->GetFPS());
				while(image->GetFrameNumber() != frameNum)
				{
					LGL_DelayMS(1);
					image=vid->GetImage();
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
			}
			else
			{
				//We're not drawing, so maybe pick a new time
				if(a==0)
				{
					if(vidLoPrevBrightness!=0.0f)
					{
						if(vidLoRndOnBlack)
						{
							char vidPath[2048];
							strcpy(vidPath,vidLoPath);
							if(dirTreeLo==NULL) dirTreeLo = new LGL_DirTree(vidPath);
							bool moreThanOneVidInDir=getRandomVideoPath(vidPath,dirTreeLo);
							if(moreThanOneVidInDir)
							{
								char oldPath[2048];
								strcpy(oldPath,vidLo->GetPathShort());
								if(strstr(vidPath,oldPath)==NULL)
								{
									if(vidLo)
									{
										vidLo->SetVideo(vidPath);;
									}
									else
									{
										vidLo = new LGL_VideoDecoder(vidPath);
									}
									vidLo->SetFrameBufferAddRadius(2);
									vidLoTimeProxy=LGL_RandFloat(0.0f,vidLo->GetLengthSeconds());
									advance=false;
								}
							}
							else
							{
								vidLoTimeProxy=LGL_RandFloat(0.0f,vidLo->GetLengthSeconds());
								advance=false;
							}
						}
					}
				}
				else if(a==1)
				{
					if(vidHiPrevBrightness!=0.0f)
					{
						if(vidHiRndOnBlack)
						{
							char vidPath[2048];
							strcpy(vidPath,vidHiPath);
							if(dirTreeHi==NULL) dirTreeHi = new LGL_DirTree(vidPath);
							bool moreThanOneVidInDir=getRandomVideoPath(vidPath,dirTreeHi);
							if(moreThanOneVidInDir)
							{
								char oldPath[2048];
								strcpy(oldPath,vidHi->GetPathShort());
								if(strstr(vidPath,oldPath)==NULL)
								{
									if(vidHi)
									{
										vidHi->SetVideo(vidPath);;
									}
									else
									{
										vidHi = new LGL_VideoDecoder(vidPath);
									}
									vidHi->SetFrameBufferAddRadius(2);
									vidHiTimeProxy=LGL_RandFloat(0.0f,vidHi->GetLengthSeconds());
									advance=false;
								}
							}
							else
							{
								vidHiTimeProxy=LGL_RandFloat(0.0f,vidHi->GetLengthSeconds());
								advance=false;
							}
						}
					}
				}
			}

			if(advance)
			{
				if(a==0)
				{
					float deltaTime = vidLoPlaybackRate/60.0f;
					float neoTime = vidLoTimeProxy+deltaTime;
					if(neoTime>vidLo->GetLengthSeconds())
					{
						neoTime-=vidLo->GetLengthSeconds();
					}
					int frameBefore = (int)floorf(vidLoPlaybackRateCounter);
					vidLoPlaybackRateCounter+=deltaTime*vidLo->GetFPS();
					int frameAfter = (int)floorf(vidLoPlaybackRateCounter);
					if(frameBefore!=frameAfter)
					{
						while((int)floorf(vidLo->GetTime()*vidLo->GetFPS())==(int)floorf(neoTime*vidLo->GetFPS()))
						{
							neoTime+=0.01f;
						}
						vidLoTimeProxy=neoTime;
					}
				}
				else if(a==1)
				{
					float deltaTime = vidHiPlaybackRate/60.0f;
					float neoTime = vidHiTimeProxy+deltaTime;
					if(neoTime>vidHi->GetLengthSeconds())
					{
						neoTime-=vidHi->GetLengthSeconds();
					}
					int frameBefore = (int)floorf(vidHiPlaybackRateCounter);
					vidHiPlaybackRateCounter+=deltaTime*vidHi->GetFPS();
					int frameAfter = (int)floorf(vidHiPlaybackRateCounter);
					if(frameBefore!=frameAfter)
					{
						while((int)(vidHi->GetTime()*vidHi->GetFPS())==(int)(neoTime*vidHi->GetFPS()))
						{
							neoTime+=0.01f;
						}
						vidHiTimeProxy=neoTime;
					}
				}
			}

			if(a==0)
			{
				vidLoPrevBrightness=brightness;
			}
			else if(a==1)
			{
				vidHiPrevBrightness=brightness;
			}
		}
		
		if(vidDrawPath)
		{
			for(int a=0;a<2;a++)
			{
				LGL_VideoDecoder* vid=(a==0) ? vidLo : vidHi;
				LGL_GetFont().DrawString
				(
					0.05f,(a==0)?0.91f:0.95,0.02f,
					1,1,1,1,
					false,
					1.0f,
					"%s | %.2f",
					vid->GetPathShort(),
					vid->GetTime()
				);
			}
			LGL_GetFont().DrawString
			(
				0.05f,0.87f,0.02f,
				1,1,1,1,
				false,
				1.0f,
				"%.2f",
				vidEncodeTime
			);
			LGL_GetFont().DrawString
			(
				0.05f,0.83f,0.02f,
				1,1,1,1,
				false,
				1.0f,
				"%s",
				rem
			);
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
		float pct = frame/(60.0f*snd->GetLengthSeconds());
		DrawStatus(pct);

		//Swap
		LGL_SwapBuffers();
	}

	snd->PrepareForDelete();
	while(snd->ReadyForDelete()==false)
	{
		LGL_DelayMS(5);
	}
	delete snd;
	if(vidLo) delete vidLo;
	if(vidHi) delete vidHi;

	fclose(mencoderFD);

	if
	(
		LGL_KeyDown(SDLK_ESCAPE)==false ||
		LGL_KeyDown(SDLK_BACKSPACE)
	)
	{
		LGL_FileDirMove(targetVideoOutPathTmp,targetVideoOutPath);
		return(0);
	}
	else
	{
		return(-1);
	}
}

