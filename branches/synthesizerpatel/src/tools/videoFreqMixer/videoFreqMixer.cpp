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

//const char* targetMusicPath="/home/emf/music/hypermind.mp3";
/*
const char* targetMusicPath="/home/emf/music/Terminal Dusk - Crimson (04) - Eight Frozen Modules - Left Me.mp3";
const char* targetVideoLoPath="data/lo.avi";
const char* targetVideoHiPath="data/hi.avi";
const char* targetVideoOutPath="data/out.avi";
*/

char targetMusicPath[1024];
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

const int ResX=852;
const int ResY=480;

int
main
(
	int	argc,
	char**	argv
)
{
	bool encodeAll=(argc==2 && strcmp(argv[1],"--all")==0);
	bool encode=true;
	if(argc==5 && strstr(argv[4],"--noencode"))
	{
		encode=false;
	}

	if
	(
		(
			argc!=4 &&
			encode==true
		) &&
		encodeAll==false
	)
	{
		printf("Usage: ./videoFreqMixer.lin music.mp3 lo.avi hi.avi\n\n");
		printf("Usage: ./videoFreqMixer.lin music.mp3 rnd rnd\n\n");
		printf("Usage: ./videoFreqMixer.lin music.mp3 rnd rnd --noencode\n\n");
		exit(0);
	}

	//Init LGL
	LGL_Init
	(
		ResX,
		ResY,
		0,
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
	bool encode=true;
	if(argc==5 && strstr(argv[4],"--noencode"))
	{
		encode=false;
	}

	strcpy(targetMusicPath,argv[1]);

	for(int a=2;a<=3;a++)
	{
		char* target;
		if(a==2) target=targetVideoLoPath;
		if(a==3) target=targetVideoHiPath;

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

	//Load Videos
	LGL_Video* vidBG=NULL;
	LGL_Video* vidLo=NULL;
	LGL_Video* vidHi=NULL;
	if(LGL_FileExists(targetVideoLoPath))
	{
		vidLo=new LGL_Video(targetVideoLoPath);
		printf("Video[Lo]: %s [%s]\n",targetVideoLoPath,LGL_FileExists(targetVideoLoPath)?"OK":"NO FILE");
	}
	else
	{
		printf("Video[Lo]: DOES NOT EXIST '%s'\n",targetVideoLoPath);
	}
	if(LGL_FileExists(targetVideoHiPath))
	{
		vidHi=new LGL_Video(targetVideoHiPath);
		printf("Video[Hi]: %s [%s]\n",targetVideoHiPath,LGL_FileExists(targetVideoHiPath)?"OK":"NO FILE");
	}
	else
	{
		printf("Video[Hi]: DOES NOT EXIST '%s'\n",targetVideoHiPath);
	}

	//Fire up mencoder
	char cmd[1024];
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

	if(encode==false)
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

	float vidBGTime=LGL_RandFloat(10.0f,vidBG?(vidBG->GetLengthSeconds()-30.0f):20.0f);
	float vidLoTime=LGL_RandFloat(10.0f,vidLo?(vidLo->GetLengthSeconds()-30.0f):20.0f);
	float vidHiTime=LGL_RandFloat(10.0f,vidHi?(vidHi->GetLengthSeconds()-30.0f):20.0f);

	float secondsStart=0.0f;
	float secondsEnd=9999.0f;
	//Loop
	for(int frame=secondsStart*60.0f;frame<snd->GetLengthSeconds()*60 && frame<secondsEnd*60.0f;frame++)
	{
		//Check for early out
		LGL_ProcessInput();
		if(LGL_KeyStroke(SDLK_ESCAPE))
		{
			fclose(mencoderFD);
			LGL_FileDelete(targetVideoOutPath);
			exit(-1);
		}

		//Analyze
		const long sampleNow=(long)(frame*44100.0f/60.0f);
		const long sampleLast=(long)LGL_Min(snd->GetLengthSamples(),(sampleNow+2*(44100/60)));
		float zeroCrossingFactor;
		float magnitudeAve;
		float magnitudeMax;

		snd->GetMetadata
		(
			sampleNow/44100.0f,
			sampleLast/44100.0f,
			zeroCrossingFactor,
			magnitudeAve,
			magnitudeMax
		);

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
				factor=(4.0f)*(zeroCrossingFactor*(0.5f+0.5f*magnitudeLoud))*magnitudeThreashold*(0.0f+fadeFactor);
				time=vidHiTime;
			}
			if(vid==NULL) continue;

			if(factor>0.0f)
			{
				//vid->SetPrimaryDecoder();
				vid->SetTime(time);
				//vid->ForceImageUpToDate();
				while(!vid->ImageUpToDate())
				{
					LGL_DelayMS(50);
				}
				LGL_Image* image=vid->LockImage();
				while(image==NULL)
				{
					vid->UnlockImage(NULL);	//Still gotta unlock it, even though it's NULL...
					LGL_DelayMS(50);
					image=vid->LockImage();
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
					vidLoTime=LGL_Max(0.0f,LGL_RandFloat(10.0f,vid->GetLengthSeconds()-30.0f));
				}
				else if(a==2)
				{
					vidHiTime=LGL_Max(0.0f,LGL_RandFloat(10.0f,vid->GetLengthSeconds()-30.0f));
				}
			}
		}

		//Advance videos
		vidBGTime+=1.0f/60.0f;
		vidLoTime+=1.0f/60.0f;
		vidHiTime+=2.0f/60.0f;
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
		float pct = frame/(60.0f*snd->GetLengthSeconds());
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

