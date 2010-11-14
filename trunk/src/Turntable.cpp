/*
 *
 * Turntable.cpp
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

#include "Turntable.h"
#include "FileInterface.h"
#include "Common.h"
#include "Config.h"
#include "Input.h"
#include <string.h>
#include <stdlib.h>

#define	BEATMANIA

#define	GLITCH_PURE_VIDEO_MULTIPLIER (5.0f)

const int lm = 8;	//Low => Mid transition
const int mh = 40;	//Mid => High transition
int ENTIRE_WAVE_ARRAY_COUNT;

int TurntableObj::Master=0;
VisualizerObj* TurntableObj::Visualizer=NULL;
LGL_Image* TurntableObj::NoiseImage[NOISE_IMAGE_COUNT_256_64];
LGL_Image* TurntableObj::LoopImage=NULL;
bool TurntableObj::FileEverOpened=false;
bool TurntableObj::SurroundMode=false;

const char* audioExtension = "flac";

void
findCachedPath
(
	char*		foundPath,
	const char*	srcPath,
	const char*	extension
)
{
	if(LGL_FileExtensionIsImage(srcPath))
	{
		strcpy(foundPath,srcPath);
		return;
	}

	//Comments assume srcPath is /home/id/mp3/hajnal.mov, extension is mjpeg.avi

	strcpy(foundPath,srcPath);
	char dotExtension[64];
	sprintf(dotExtension,".%s",extension);
	if
	(
		strstr(srcPath,dotExtension) &&
		strcmp(dotExtension,".mjpeg.avi")==0
	)
	{
		//hajnal.mov has .mjpeg.avi extension in it, indicating we look no further.
		return;
	}

	char soundSrcDir[2048];
	strcpy(soundSrcDir,srcPath);
	if(char* lastSlash = strrchr(soundSrcDir,'/'))
	{
		lastSlash[0]='\0';
	}
	else
	{
		soundSrcDir[0]='\0';
	}
	
	char soundName[2048];
	if(strrchr(soundSrcDir,'/'))
	{
		strcpy(soundName,&(strrchr(srcPath,'/')[1]));
	}
	else
	{
		strcpy(soundName,srcPath);
	}

	sprintf(foundPath,"%s/%s.%s",soundSrcDir,soundName,extension);
	if(LGL_FileExists(foundPath))
	{
		//Found /home/id/mp3/hajnal.mov.mjpeg.avi
		return;
	}

	const char* dvjCacheDirName = GetDvjCacheDirName();

	sprintf(foundPath,"%s/%s/%s.%s",soundSrcDir,dvjCacheDirName,soundName,extension);
	if(LGL_FileExists(foundPath))
	{
		//Found /home/id/mp3/dvj_cache/hajnal.mov.mjpeg.avi
		if(LGL_FirstFileMoreRecentlyModified(srcPath,foundPath))
		{
			LGL_FileDelete(foundPath);
		}
		else
		{
			return;
		}
	}

	if
	(
		strstr(srcPath,".mjpeg.avi") &&
		strcmp(extension,"flac")==0
	)
	{
		sprintf(foundPath,"%s/%s/%s",soundSrcDir,dvjCacheDirName,soundName);
		sprintf(strstr(foundPath,".mjpeg.avi"),".%s",extension);
		if(LGL_FileExists(foundPath))
		{
			//Found /home/id/mp3/dvj_cache/hajnal.mov.mjpeg.avi (for audio??)
			if(LGL_FirstFileMoreRecentlyModified(srcPath,foundPath))
			{
				LGL_FileDelete(foundPath);
			}
			else
			{
				return;
			}
		}
	}

	sprintf(foundPath,"%s/.dvj/video/tracks/%s.%s",LGL_GetHomeDir(),soundName,extension);
	if(LGL_FileExists(foundPath))
	{
		//Found /home/id/.dvj/video/tracks/hajnal.mov.mjpeg.avi
		return;
	}

	if
	(
		strstr(srcPath,".mjpeg.avi") &&
		strcmp(extension,"flac")==0
	)
	{
		sprintf(foundPath,"%s/.dvj/video/tracks/%s",LGL_GetHomeDir(),soundName);
		sprintf(strstr(foundPath,".mjpeg.avi"),".%s",extension);
		if(LGL_FileExists(foundPath))
		{
			//Found /home/id/mp3/dvj/hajnal.mov.mjpeg.avi
			return;
		}
	}

	strcpy(foundPath,srcPath);
	if
	(
		strstr(extension,".mjpeg.avi") &&
		LGL_FileExists(foundPath)
	)
	{
		//Found /home/id/mp3/hajnal.mov
		return;
	}

	//Found nothing
	foundPath[0]='\0';
}

void
findVideoPath
(
	char*		foundPath,
	const char*	srcPath
)
{
	findCachedPath(foundPath,srcPath,"mjpeg.avi");
	if(LGL_FileExists(foundPath))
	{
		return;
	}

	if(LGL_VideoIsMJPEG(srcPath))
	{
		static LGL_Semaphore lnSym("lnSym");
		LGL_ScopeLock lock(lnSym);
		char soundSrcDir[2048];
		strcpy(soundSrcDir,srcPath);
		if(char* lastSlash = strrchr(soundSrcDir,'/'))
		{
			lastSlash[0]='\0';
		}
		else
		{
			soundSrcDir[0]='\0';
		}
		char soundName[2048];
		if(strrchr(soundSrcDir,'/'))
		{
			strcpy(soundName,&(strrchr(srcPath,'/')[1]));
		}
		else
		{
			strcpy(soundName,srcPath);
		}
		sprintf(foundPath,"%s/%s/%s.mjpeg.avi",soundSrcDir,GetDvjCacheDirName(),soundName);
		if(LGL_FileExists(foundPath)==false)
		{
			LGL_FileDelete(foundPath);	//For stale symlinks...
			char cmd[4096];
			sprintf
			(
				cmd,
				"ln -s '%s' '%s'",
				srcPath,
				foundPath
			);
			system(cmd);
		}
		return;
	}
}

void
findAudioPath
(
	char*		foundPath,
	const char*	srcPath
)
{
	findCachedPath(foundPath,srcPath,audioExtension);
}

int
videoEncoderThread
(
	void*	ptr
)
{
	TurntableObj* tt = (TurntableObj*)ptr;
	LGL_ThreadSetPriority(0.1f,"videoEncoderThread");

	char encoderSrc[2048];
	strcpy(encoderSrc,tt->VideoEncoderPathSrc);

	char encoderSrcDir[2048];
	strcpy(encoderSrcDir,encoderSrc);
	if(char* lastSlash = strrchr(encoderSrcDir,'/'))
	{
		lastSlash[0]='\0';
	}
	else
	{
		encoderSrcDir[0]='\0';
	}

	const char* dvjCacheDirName = GetDvjCacheDirName();

	if
	(
		encoderSrc[0]!='\0' &&
		LGL_FileExists(encoderSrc)
	)
	{
		//We've been requested to encode a video. Joy!

		char videoPath[2048];
		if(strstr(encoderSrcDir,"iTunes"))
		{
			sprintf(videoPath,"%s/.dvj/video/tracks",LGL_GetHomeDir());
		}
		else
		{
			sprintf(videoPath,"%s/%s",encoderSrcDir,dvjCacheDirName);
		}
		char videoTmpPath[2048];
		sprintf(videoTmpPath,"%s/tmp",videoPath);

		if(LGL_DirectoryExists(videoPath)==false)
		{
			LGL_DirectoryCreateChain(videoPath);
		}
		if(LGL_DirectoryExists(videoTmpPath)==false)
		{
			LGL_DirectoryCreateChain(videoTmpPath);
		}

		int rand = LGL_RandInt(0,32768);

		char encoderDstTmp[2048];
		sprintf(encoderDstTmp,"%s/dvj-deleteme-mjpeg-encode-%i.avi",videoTmpPath,rand);

		char encoderDst[2048];
		sprintf
		(
			encoderDst,
			"%s/%s.mjpeg.avi",
			videoPath,
			&(strrchr(encoderSrc,'/')[1])
		);

		char encoderAudioDstTmp[2048];
		sprintf(encoderAudioDstTmp,"%s/dvj-deleteme-%s-encode-%i.%s",videoTmpPath,audioExtension,rand,audioExtension);

		char encoderAudioDst[2048];
		sprintf
		(
			encoderAudioDst,
			"%s/%s.%s",
			videoPath,
			&(strrchr(encoderSrc,'/')[1]),
			audioExtension
		);

		if(strstr(encoderSrc,".mjpeg.avi"))
		{
			strcpy(encoderDst,encoderSrc);
		}
		else if(LGL_FileExists(encoderDst)==false)
		{
			char foundVideo[2048];
			findVideoPath
			(
				foundVideo,
				encoderSrc
			);
			if(LGL_FileExists(foundVideo))
			{
				strcpy(encoderDst,foundVideo);
			}
		}

		if(LGL_FileExists(encoderAudioDst)==false)
		{
			char foundAudio[2048];
			findAudioPath
			(
				foundAudio,
				encoderSrc
			);
			if(LGL_FileExists(foundAudio))
			{
				strcpy(encoderAudioDst,foundAudio);
			}
		}

		//Video Encoding Loop
		if(LGL_FileExists(encoderDst)==false || LGL_FileExists(encoderAudioDst)==false)
		{
			{
				LGL_ScopeLock lock(tt->VideoEncoderSemaphore);
				tt->VideoEncoder = new LGL_VideoEncoder
				(
					encoderSrc,
					encoderDstTmp,
					encoderAudioDstTmp
				);
			}

			LGL_VideoEncoder* encoder = tt->VideoEncoder;
			encoder->SetBitrateMaxMBps(GetCachedVideoAveBitrateMBps());
			encoder->SetEncodeAudio(LGL_FileExists(encoderAudioDst)==false);
			encoder->SetEncodeVideo(LGL_FileExists(encoderDst)==false);
			if
			(
				encoder->IsValid() &&
				(
					encoder->GetEncodeAudio() ||
					encoder->GetEncodeVideo()
				)
			)
			{
//printf("Encoding!\n");
//if(encoder->GetEncodeAudio()) printf("Encoding Audio! (%s)\n",encoderAudioDst);
//if(encoder->GetEncodeVideo()) printf("Encoding Video! (%s)\n",encoderDst);
				tt->VideoEncoderAudioOnly = encoder->GetEncodeVideo()==false && encoder->GetEncodeAudio()==true;
				LGL_Timer timer;
				LGL_Timer timerUpdateEta;
				const float updateEtaInterval=1.0f;
				for(;;)
				{
					if(tt->VideoEncoderTerminateSignal==1)
					{
						break;
					}
					if(tt->VideoEncoderBeginSignal==0)
					{
						LGL_DelayMS(100);
						timer.Reset();
						continue;
					}
					char videoEncoderPathSrc[2048];
					strcpy(videoEncoderPathSrc,tt->VideoEncoderPathSrc);
					if(strcmp(encoderSrc,videoEncoderPathSrc)!=0)
					{
						//Turntable has decided it doesn't want the video we're currently encodeing. Pity.
						if(encoder->GetEncodeVideo())
						{
							LGL_FileDelete(encoderDstTmp);
						}
						if(encoder->GetEncodeAudio())
						{
							LGL_FileDelete(encoderAudioDstTmp);
						}
						tt->VideoEncoderPercent=-1.0f;
						tt->VideoEncoderEtaSeconds=-1.0f;
						break;
					}

					encoder->Encode(1);
					tt->VideoEncoderPercent=encoder->GetPercentFinished();
					if(timerUpdateEta.SecondsSinceLastReset()>=updateEtaInterval)
					{
						tt->VideoEncoderEtaSeconds=(timer.SecondsSinceLastReset()/tt->VideoEncoderPercent)-timer.SecondsSinceLastReset();
						timerUpdateEta.Reset();
					}

					if(encoder->IsFinished())
					{
						char dotDvjPath[2048];
						sprintf
						(
							dotDvjPath,
							"%s/.dvj/video/tracks",
							LGL_GetHomeDir()
						);
						if(encoder->GetEncodeVideo())
						{
							char targetPath[2048];
							sprintf
							(
								targetPath,
								"%s/%s.mjpeg.avi",
								dotDvjPath,
								&(strrchr(encoderSrc,'/')[1])
							);
							LGL_FileDelete(targetPath);	//For stale symlinks...

							LGL_FileDirMove(encoderDstTmp,encoderDst);

							if(strcmp(encoderDst,targetPath)==0)
							{
								//
							}
							else
							{
								char cmd[2048];
								sprintf
								(
									cmd,
									"ln -s '%s' '%s'",
									encoderDst,
									targetPath
								);
								system(cmd);
							}
						}
						if(encoder->GetEncodeAudio())
						{
							//Audio too!
							char targetPath[2048];
							sprintf
							(
								targetPath,
								"%s/%s.%s",
								dotDvjPath,
								&(strrchr(encoderSrc,'/')[1]),
								audioExtension
							);
							LGL_FileDelete(targetPath);	//For stale symlinks...

							LGL_FileDirMove(encoderAudioDstTmp,encoderAudioDst);

							if(strcmp(encoderAudioDst,targetPath)==0)
							{
								//
							}
							else
							{
								char cmd[2048];
								sprintf
								(
									cmd,
									"ln -s '%s' '%s'",
									encoderAudioDst,
									targetPath
								);
								system(cmd);
							}
						}
						break;
					}

					//TODO: LGL_Delay()?
				}

				if(LGL_FileExists(encoderDstTmp))
				{
					LGL_FileDelete(encoderDstTmp);
				}
			}

			if(encoder->IsUnsupportedCodec())
			{
				const char* codecName = encoder->GetCodecName() ? encoder->GetCodecName() : "Unknown";
				strcpy
				(
					tt->VideoEncoderUnsupportedCodecName,
					codecName
				);
				tt->VideoEncoderUnsupportedCodecTime=5.0f;
			}
		}

		if(strcmp(encoderSrc,tt->VideoEncoderPathSrc)==0)
		{
			tt->VideoEncoderPathSrc[0]='\0';
		}

		if
		(
			LGL_FileExists(encoderDst) //&&
			//LGL_FileExists(encoderAudioDst)
		)
		{
			tt->VideoEncoderPercent=2.0f;
		}
		else
		{
			tt->VideoEncoderPercent=-1.0f;
		}

		tt->VideoEncoderEtaSeconds=-1.0f;
	}

	tt->VideoEncoderTerminateSignal=-1;
	tt->VideoEncoderAudioOnly=false;

	return(0);
}

TurntableObj::
TurntableObj
(
	float	left,	float	right,
	float	bottom,	float	top,
	DatabaseObj* database
) :	VideoEncoderSemaphore("VideoEncoderSemaphore")
{
	Mode=0;

	SetViewport(left,right,bottom,top);
	SetFocus(false);
	
	Sound=NULL;
	sprintf(SoundName,"No Track Selected");
	SoundBufferLength=4*44100*60*20;
	SoundBufferCurrentPageIndex=0;
	//SoundBufferLength=(unsigned long)(1024*1024*200*(1.0/0.987875));	//20 minutes
	SoundBuffer=(Uint8*)malloc(SoundBufferLength);
	bzero(SoundBuffer,SoundBufferLength);	//Not necessary, but used for testing

	//This is disabled because it was causing semaphore locks in LGL's audio update loop, which I believe was skipping the audio
	//LGL_AddAudioStream(&GrainStream);
	GrainStreamActive=false;
	GrainStreamActiveSeconds=0.0f;
	GrainStreamInactiveSeconds=0.0f;
	GrainStreamCrossfader=0.0f;
	GrainStreamMainVolumeMultiplier=1.0f;
	GrainStreamGrainsPerSecond=100.0f;
	GrainStreamStartDelayVariance=0.001f;
	GrainStreamSourcePoint=0.0f;
	GrainStreamSourcePointVariance=0.01f;
	GrainStreamLength=0.15f;
	GrainStreamLengthVariance=0.01f;
	GrainStreamPitch=1.0f;
	GrainStreamPitchVariance=0.001f;
	GrainStreamVolume=1.5f/(GrainStreamGrainsPerSecond*GrainStreamLength);
	GrainStreamVolumeVariance=0.25f*GrainStreamVolume;
	GrainStreamSpawnDelaySeconds=0.0f;

	CenterX=.5f*(ViewportLeft+ViewportRight);
	CenterY=.5f*(ViewportBottom+ViewportTop);

	FilterTextMostRecent[0]='\0';

	BadFileFlash=0.0f;

	Channel=-1;
	PauseMultiplier=0;
	Nudge=0;
	MixerNudge=0;
	FinalSpeed=0.0f;
	FinalSpeedLastFrame=0.0f;
	RewindFF=false;
	RecordHoldLastFrame=false;
	RecordSpeedAsZeroUntilZero=false;
	LuminScratch=false;
	LuminScratchSamplePositionDesired=-10000;
	MixerVolumeFront=1.0f;
	MixerVolumeBack=1.0f;
	for(int a=0;a<3;a++)
	{
		MixerEQ[a]=1.0f;
	}
	VolumeSlider=1.0f;
	VolumeMultiplierNow=1.0f;
	VolumeInvertBinary=false;
	RapidVolumeInvert=false;
	RapidSoloInvert=false;
	LoopAlphaSeconds=-1.0;
	QuantizePeriodMeasuresExponent=-3;
	QuantizePeriodNoBPMSeconds=1.0;
	LoopActive=false;
	LoopThenRecallActive=false;
	AutoDivergeRecallActive=false;
	SavePointIndex=0;
	MetaDataSavedThisFrame=NULL;
	RecordScratch=false;
	LuminScratch=false;

	for(int a=0;a<3;a++)
	{
		EQFinal[a]=1.0f;
		EQKnob[a]=1.0f;
		EQKill[a]=false;
	}
	for(int a=0;a<513;a++) FreqResponse[a]=1.0f;

	Which=0;

	ImageSetPrefix[0]='\0';
	MovieClipPrefix[0]='\0';
	Visualizer=NULL;

	BPMMaster=0.0f;

	TrackListFileUpdates.clear();

	VideoFront=new LGL_VideoDecoder(NULL);
	VideoFront->SetFrameBufferAddRadius(GetVideoBufferFrames());
	VideoBack=NULL;
	VideoLo=new LGL_VideoDecoder(NULL);
	VideoLo->SetFrameBufferAddRadius(GetVideoBufferFramesFreqSense());
	VideoLo->SetFrameBufferAddBackwards(false);
	VideoHi=new LGL_VideoDecoder(NULL);
	VideoHi->SetFrameBufferAddRadius(GetVideoBufferFramesFreqSense());
	VideoHi->SetFrameBufferAddBackwards(false);
	VideoAdvanceRate=1.0f;
	VideoBrightness=1.0f;
	OscilloscopeBrightness=0.0f;
	FreqSenseBrightness=0.0f;
	AudioInputMode=false;

	VideoEncoder=NULL;
	VideoEncoderThread=NULL;

	ENTIRE_WAVE_ARRAY_COUNT=LGL_WindowResolutionX();

	ClearRecallOrigin();

	WhiteFactor=1.0f;
	NoiseFactor=1.0f;
	NoiseFactorVideo=0.0f;
	if(LoopImage==NULL)
	{
		LoopImage = new LGL_Image("data/image/loop.png");
		for(int a=0;a<NOISE_IMAGE_COUNT_256_64;a++)
		{
			char path[1024];
			sprintf
			(
				path,
				"data/image/noise/256x64/%02i.png",
				a
			);
			assert(LGL_FileExists(path));
			NoiseImage[a] = new LGL_Image(path);
		}
	}

	LowRez=false;
	AspectRatioMode=0;

	Database=database;
	char musicRoot[2048];
	strcpy(musicRoot,GetMusicRootPath());
	DatabaseFilter.SetDir(musicRoot);
	DatabaseFilteredEntries=Database->GetEntryListFromFilter(&DatabaseFilter);

	int dirCount=0;
	int fileCount=0;
	for(unsigned int a=0;a<DatabaseFilteredEntries.size();a++)
	{
		if(DatabaseFilteredEntries[a]->IsDir)
		{
			dirCount++;
		}
		else
		{
			fileCount++;
		}
	}

	FileTop=0;
	if(dirCount<fileCount)
	{
		for(unsigned int a=0;a<DatabaseFilteredEntries.size();a++)
		{
			if(DatabaseFilteredEntries[a]->IsDir==false)
			{
				FileTop=a;
				break;
			}
		}
	}
	FileSelectInt=FileTop;
	FileSelectFloat=(float)FileTop;

	VideoEncoderPercent=-1.0f;
	VideoEncoderEtaSeconds=-1.0f;
	VideoEncoderAudioOnly=false;
	VideoEncoderPathSrc[0]='\0';
	VideoEncoderThread=NULL;
	VideoEncoderTerminateSignal=0;
	VideoEncoderBeginSignal=0;
	VideoEncoderUnsupportedCodecTime=0.0f;
	VideoEncoderUnsupportedCodecName[0]='\0';
}

TurntableObj::
~TurntableObj()
{
	if(VideoEncoderThread)
	{
		if(VideoEncoderTerminateSignal==0)
		{
			VideoEncoderTerminateSignal=1;
		}

		for(;;)
		{
			if(VideoEncoderTerminateSignal==-1)
			{
				LGL_ThreadWait(VideoEncoderThread);
				VideoEncoderThread=NULL;
				VideoEncoderTerminateSignal=0;
				break;
			}
			LGL_DelayMS(50);
		}
	}

	for(unsigned int a=0;a<TrackListFileUpdates.size();a++)
	{
		delete TrackListFileUpdates[a];
		TrackListFileUpdates[a]=NULL;
	}
	TrackListFileUpdates.clear();
	if(VideoFront)
	{
		delete VideoFront;
		VideoFront=NULL;
	}
	if(VideoBack)
	{
		delete VideoBack;
		VideoBack=NULL;
	}
	if(VideoLo)
	{
		delete VideoLo;
		VideoLo=NULL;
	}
	if(VideoHi)
	{
		delete VideoHi;
		VideoHi=NULL;
	}
	if(Sound)
	{
		Sound->PrepareForDelete();
		for(;;)
		{
			if(Sound->ReadyForDelete())
			{
				break;
			}
			else
			{
				LGL_DelayMS(50);
			}
		}
		delete Sound;
		Sound=NULL;
	}
	if(SoundBuffer!=NULL)
	{
		free(SoundBuffer);
		SoundBuffer=NULL;
	}
}

void
TurntableObj::
NextFrame
(
	float	secondsElapsed
)
{
	if(LGL_AudioJackXrun())
	{
		VideoFront->SetFrameBufferAddRadius(VideoFront->GetFrameBufferAddRadius()/2);
	}

	if(LGL_RamFreeMB()<100)
	{
		if(VideoFront->GetFrameBufferAddRadius()>2)
		{
			VideoFront->SetFrameBufferAddRadius(VideoFront->GetFrameBufferAddRadius()-1);
		}
	}
	else if(LGL_RamFreeMB()>200)
	{
		if(VideoFrontRadiusIncreaseDelayTimer.SecondsSinceLastReset()>0.5f)
		{
			int radiusDesired = GetVideoBufferFrames();
			int radiusNow = VideoFront->GetFrameBufferAddRadius();
			if(radiusNow<radiusDesired)
			{
				VideoFront->SetFrameBufferAddRadius(radiusNow+1);
				VideoFrontRadiusIncreaseDelayTimer.Reset();
			}
		}
	}

	unsigned int target =
		(Focus ? TARGET_FOCUS : 0) |
		((Which==0) ? TARGET_TOP : TARGET_BOTTOM);
	float candidate = 0.0f;

#ifdef	LGL_OSX
	//Apple doesn't allow us to mlockall().
	//As such, we must loop on each page in our soundbuffer,
	//to ensure it remains in active memory... UGH.
	//TODO: Does mlock() work? I doubt it...
	int loops=0;
	const int pageSize=2048;
	for(unsigned long a=SoundBufferCurrentPageIndex;a<SoundBufferLength;a+=pageSize)
	{
		Uint8 pageMeInFucker = SoundBuffer[a];
		candidate+=pageMeInFucker*0;

		SoundBufferCurrentPageIndex=a+pageSize;
		loops++;
		if(loops==1000)
		{
			break;
		}
	}
	if(SoundBufferCurrentPageIndex>=SoundBufferLength)
	{
		SoundBufferCurrentPageIndex=0;
	}
#endif	//LGL_OSX

	if
	(
		LGL_MouseX()>=ViewportLeft &&
		LGL_MouseX()<=ViewportRight &&
		LGL_MouseY()>=ViewportBottom &&
		LGL_MouseY()<=ViewportTop
	)
	{
		if(LGL_MouseMotion())
		{
			GetInputMouse().SetFocusNext(Which);
		}
	}

	if(MetaDataSavedThisFrame)
	{
		delete MetaDataSavedThisFrame;
		MetaDataSavedThisFrame=NULL;
	}

	bool volumeFull=false;
	bool endpointsSticky=true;
	float localVolumeMultiplier=1.0f;
	bool noiseIncreasing=false;
	bool glitchPurePrev=GlitchPure;
	FinalSpeedLastFrame=FinalSpeed;
	
	VideoEncoderUnsupportedCodecTime = LGL_Max(0.0f,VideoEncoderUnsupportedCodecTime-secondsElapsed);

	//Volume
	candidate=Input.WaveformVolumeSlider(target);
	if(candidate!=-1.0f)
	{
		VolumeSlider=candidate;
	}

	//EQ Delta
	if(Mode==2)
	{
		for(int a=0;a<3;a++)
		{
			if(a==0) candidate = Input.WaveformEQLowDelta(target);
			else if(a==1) candidate = Input.WaveformEQMidDelta(target);
			else if(a==2) candidate = Input.WaveformEQHighDelta(target);
			if(candidate!=0.0f)
			{
				EQKnob[a]=LGL_Clamp(0.0f,EQKnob[a]+candidate,2.0f);
			}
		}
	}

	//EQ
	{
		for(int a=0;a<3;a++)
		{
			if(a==0) candidate = Input.WaveformEQLow(target);
			else if(a==1) candidate = Input.WaveformEQMid(target);
			else if(a==2) candidate = Input.WaveformEQHigh(target);
			if(candidate!=-1.0f)
			{
				EQKnob[a]=LGL_Clamp(0.0f,candidate*2.0f,2.0f);
			}
		}
	}

	//EQ Kill
	{
		EQKill[0]=Input.WaveformEQLowKill(target);
		EQKill[1]=Input.WaveformEQMidKill(target);
		EQKill[2]=Input.WaveformEQHighKill(target);
	}

	bool eqChanged=false;

	for(int a=0;a<3;a++)
	{
		float oldFinal = EQFinal[a];
		EQFinal[a]=EQKill[a] ? 0 : EQKnob[a];
		if(EQFinal[a]!=oldFinal) eqChanged=true;
	}

	if(eqChanged)
	{
		UpdateSoundFreqResponse();
	}

	VolumeSolo=Input.WaveformVolumeSolo(target);

	if(VideoEncoderTerminateSignal==-1)
	{
		LGL_ThreadWait(VideoEncoderThread);
		VideoEncoderThread=NULL;
		VideoEncoderTerminateSignal=0;
		{
			LGL_ScopeLock lock(VideoEncoderSemaphore);
			delete VideoEncoder;
			VideoEncoder=NULL;
		}
	}

	if(Mode==0)
	{
		//File Select
		bool filterDelta = false;

		if
		(
			Focus &&
			LGL_KeyDown(LGL_KEY_BACKSPACE)
		)
		{
			if(Mode0BackspaceTimer.SecondsSinceLastReset()>0.5f)
			{
				//FilterText.SetString();
			}
		}
		else
		{
			Mode0BackspaceTimer.Reset();
		}

		BadFileFlash=LGL_Max(0.0f,BadFileFlash-2.0f*LGL_SecondsSinceLastFrame());

		if(Focus)
		{
			if(LGL_KeyStroke(LGL_KEY_ESCAPE))
			{
				FilterText.SetString();
			}

			FilterText.GrabFocus();
		}
		else
		{
			FilterText.ReleaseFocus();
		}
		
		if(strcmp(FilterText.GetString(),FilterTextMostRecent)!=0)
		{
			filterDelta=true;
			strncpy(FilterTextMostRecent,FilterText.GetString(),sizeof(FilterTextMostRecent));
			FilterTextMostRecent[sizeof(FilterTextMostRecent)-1]='\0';

			char oldSelection[2048];
			if(FileSelectInt < (int)DatabaseFilteredEntries.size())
			{
				strncpy(oldSelection,DatabaseFilteredEntries[FileSelectInt]->PathShort,sizeof(oldSelection));
			}
			else
			{
				oldSelection[0]='\0';
			}
			oldSelection[sizeof(oldSelection)-1]='\0';

			char filterText[2048];
			strncpy(filterText,FilterText.GetString(),sizeof(filterText));
			filterText[sizeof(filterText)-1]='\0';
			while(char* space=strchr(filterText,' '))
			{
				space[0]='|';
			}

			char pattern[2048];
			pattern[0]='\0';

			DatabaseFilter.SetBPMCenter(-1);
			DatabaseFilter.SetBPMRange(10);

			FileInterfaceObj fi;
			fi.ReadLine(filterText);
			for(unsigned int a=0;a<fi.Size();a++)
			{
				const char* item=fi[a];
				int len = strlen(item);
				if(len>0)
				{
					if(item[0]=='~')
					{
						//BPM directive
						if
						(
							item[1] >= '0' && item[1] <='9' &&
							item[2] >= '0' && item[2] <='9' &&
							item[3] >= '0' && item[3] <='9'
						)
						{
							//We found a BPM center
							char centerStr[2048];
							strcpy(centerStr,&(item[1]));
							if(char* tilde = strchr(centerStr,'~'))
							{
								tilde[0]='\0';
							}
							float center=atof(centerStr);
							DatabaseFilter.SetBPMCenter(center);

							if
							(
								item[4]=='~' &&
								item[5] >= '0' && item[5] <='9'
							)
							{
								//We found a BPM range
								char rangeStr[2048];
								strcpy(rangeStr,&(item[5]));
								float range=atof(rangeStr);
								DatabaseFilter.SetBPMRange(range);
							}
						}
						else
						{
							if(BPMMaster>0)
							{
								DatabaseFilter.SetBPMCenter(BPMMaster);
							}
						}
					}
					else
					{
						if(pattern[0]!='\0')
						{
							strncat(pattern," ",sizeof(pattern)-strlen(pattern)-1);
						}
						strncat(pattern,item,sizeof(pattern)-strlen(pattern)-1);
					}
				}
			}

			DatabaseFilter.SetPattern(pattern);
			DatabaseFilteredEntries=Database->GetEntryListFromFilter(&DatabaseFilter);

			FileTop=0;
			FileSelectInt=0;
			FileSelectFloat=0;

			FileSelectToString(oldSelection);
		}

		if
		(
			Input.FileMarkUnopened(target) &&
			DatabaseFilteredEntries.empty()==false
		)
		{
			DatabaseFilteredEntries[FileSelectInt]->AlreadyPlayed=false;
		}

		int fileSelect = Input.FileSelect(target);
		if
		(
			fileSelect > 0 &&
			DatabaseFilteredEntries.empty()==false
		)
		{
			char targetPath[2048];
			targetPath[0]='\0';
			bool targetIsDir;

			strcpy(targetPath,DatabaseFilteredEntries[FileSelectInt]->PathFull);
			targetIsDir=DatabaseFilteredEntries[FileSelectInt]->IsDir;
			bool loadable=DatabaseFilteredEntries[FileSelectInt]->Loadable;
			bool targetPathIsDotDot = false;
			if(strcmp(targetPath,"..")==0)
			{
				targetPathIsDotDot = true;
				strcpy(targetPath,DatabaseFilter.Dir);
				if(char* slash = strrchr(targetPath,'/'))
				{
					slash[0]='\0';
				}
			}

			if
			(
				targetIsDir==false &&
				targetPath[0]!='\0'
			)
			{
				strcpy(SoundSrcPathShort,DatabaseFilteredEntries[FileSelectInt]->PathShort);
				char filename[2048];
				sprintf
				(
					 filename,
					 "%s",
					 targetPath
				);
				strcpy(SoundSrcPath,filename);
				strcpy(SoundSrcDir,SoundSrcPath);
				if(char* lastSlash = strrchr(SoundSrcDir,'/'))
				{
					lastSlash[0]='\0';
				}
				else
				{
					SoundSrcDir[0]='\0';
				}
				
				char videoTracksPath[2048];
				sprintf(videoTracksPath,"%s/%s",SoundSrcDir,GetDvjCacheDirName());
				char filenameCached[2048];
				findAudioPath(filenameCached,SoundSrcPath);

				if
				(
					loadable &&
					LGL_FileExists(filename)
				)
				{
					strcpy(SoundName,&(strrchr(filename,'/')[1]));
					LoadAllCachedData();

					const char* filenameSnd = filename;
					if(LGL_FileExists(filenameCached))
					{
						filenameSnd=filenameCached;
					}
					LGL_DrawLogWrite("!dvj::NewSound|%s|%i\n",filenameSnd,Which);
					Sound=new LGL_Sound
					(
						 filenameSnd,
						 true,
						 2,
						 SoundBuffer,
						 SoundBufferLength
					);
					Sound->SetVolumePeak(CachedVolumePeak);
					DatabaseFilteredEntries[FileSelectInt]->AlreadyPlayed=true;
					if(Sound->IsUnloadable())
					{
						LGL_DrawLogWrite("!dvj::DeleteSound|%i\n",Which);
						Sound->PrepareForDelete();
						Mode=3;
						BadFileFlash=1.0f;
						if(LGL_VideoDecoder* dec = GetVideo())
						{
							LGL_DrawLogWrite("!dvj::DeleteVideo|%s\n",dec->GetPath());
						}
						return;
					}
					else
					{
						Mode=1;
						DatabaseEntryNow=DatabaseFilteredEntries[FileSelectInt];
						DecodeTimer.Reset();
						SecondsLast=0.0f;
						SecondsNow=0.0f;
						VolumeInvertBinary=false;
						RapidVolumeInvert=false;
						RapidSoloInvert=false;
						LoopAlphaSeconds=-1.0;
						//QuantizePeriodMeasuresExponent=QuantizePeriodMeasuresExponent
						QuantizePeriodNoBPMSeconds=1.0;
						LoopActive=false;
						LoopThenRecallActive=false;
						AutoDivergeRecallActive=false;
						SavePointIndex=0;
						for(int a=0;a<18;a++)
						{
							SavePointSeconds[a]=-1.0f;
							SavePointUnsetNoisePercent[a]=0.0f;
							SavePointUnsetFlashPercent[a]=0.0f;
						}
						FilterText.ReleaseFocus();
						ClearRecallOrigin();

						char* update=new char[1024];
						sprintf(update,"ALPHA: %s",SoundName);
						TrackListFileUpdates.push_back(update);

						WhiteFactor=1.0f;
						NoiseFactor=1.0f;
						NoiseFactorVideo=1.0f;
					}
				}
				else
				{
					BadFileFlash=1.0f;
					DatabaseFilteredEntries[FileSelectInt]->Loadable=false;
				}
			}
			else if
			(
				targetIsDir &&
				targetPath[0]!='\0' &&
				fileSelect != 2
			)
			{
				FilterTextMostRecent[0]='\0';
				FilterText.SetString("");
				char oldDir[2048];
				strcpy(oldDir,&(strrchr(DatabaseFilter.GetDir(),'/')[1]));

				DatabaseFilter.SetDir(targetPath);
				DatabaseFilter.SetPattern(FilterText.GetString());
				DatabaseFilteredEntries=Database->GetEntryListFromFilter(&DatabaseFilter);

				FileTop=0;
				FileSelectInt=0;
				FileSelectFloat=0.0f;
				FilterText.SetString();
				NoiseFactor=1.0f;
				WhiteFactor=1.0f;

				if(targetPathIsDotDot)
				{
					FileSelectToString(oldDir);
					FileTop-=2;
					if(FileTop<0) FileTop=0;
				}
			}
		}

		//CAN CRASH. Disabled until fixed.
		if(0 && Input.FileRefresh(target))
		{
			//
		}

		if
		(
			LGL_MouseX()>=ViewportLeft &&
			LGL_MouseX()<=ViewportRight &&
			LGL_MouseY()>=ViewportBottom &&
			LGL_MouseY()<=ViewportTop
		)
		{
			if(LGL_MouseStroke(LGL_MOUSE_LEFT))
			{
				GetInputMouse().SetFileSelectNext();
			}
		}

		int tmp=Input.FileIndexHighlight(target);
		if(tmp!=-1)
		{
			FileSelectFloat=FileTop+tmp;
			FileSelectInt=FileTop+tmp;
		}

		FileSelectFloat=
			FileSelectInt +
			Input.FileScroll(target);

		if(FileSelectFloat<0)
		{
			FileSelectFloat=0;
		}
		if(FileSelectFloat+0.5f >= DatabaseFilteredEntries.size()-1)
		{
			FileSelectFloat=DatabaseFilteredEntries.size()-1;
		}

		if(FileSelectInt != (int)floor(FileSelectFloat+.5f))
		{
			BadFileFlash=0.0f;
			FileSelectInt=(int)floor(FileSelectFloat+.5f);
		}
	
		if(FileSelectInt>=FileTop+4)
		{
			FileTop=FileSelectInt-4;
		}
		if(FileSelectInt<FileTop)
		{
			FileTop=FileSelectInt;
		}
	}
	else if(Mode==1)
	{
		//Decoding...

		LGL_Assert(Sound!=NULL);

		if
		(
			Sound->IsUnloadable() ||
			Input.DecodeAbort(target)
		)
		{
			//Abort load. Select new track.
			LGL_DrawLogWrite("!dvj::DeleteSound|%i\n",Which);
			Sound->PrepareForDelete();
			Channel=-1;
			Mode=3;
			BadFileFlash=1.0f;
			if(LGL_VideoDecoder* dec = GetVideo())
			{
				LGL_DrawLogWrite("!dvj::DeleteVideo|%s\n",dec->GetPath());
			}
			return;
		}
	}
	else if(Mode==2)
	{
		SecondsLast=SecondsNow;
		SecondsNow=Sound->GetPositionSeconds(Channel);

		VolumeKill = Input.WaveformGainKill(target);

		float rewindFFFactor=Input.WaveformRewindFF(target);
		float recordSpeed=Input.WaveformRecordSpeed(target);
		bool recordHold=Input.WaveformRecordHold(target);
		if(recordSpeed==0.0f)
		{
			RecordSpeedAsZeroUntilZero=false;
		}
		if(recordHold)
		{
			RecordHoldReleaseTimer.Reset();
		}
		else
		{
			if(RecordSpeedAsZeroUntilZero)
			{
				recordSpeed=0.0f;
			}
		}
		if
		(
			RecordHoldLastFrame &&
			recordHold==false
		)
		{
			Sound->SetSpeed(Channel,Pitchbend*PauseMultiplier,true);
			RecordSpeedAsZeroUntilZero=true;
		}
		RecordHoldLastFrame=recordHold;
		if(0 && AudioInputMode)
		{
			rewindFFFactor=0.0f;
			recordSpeed=0.0f;
			recordHold=false;
		}

		if
		(
			//VideoEncoderBeginSignal==0 ||
			Sound->IsLoaded()
		)
		{
			VideoEncoderBeginSignal=1;
		}

		//Scratch / Queue
		{
			if(rewindFFFactor!=0.0f)
			{
				//RW/FF
				endpointsSticky=true;
				recordSpeed=rewindFFFactor;
				float normalVolFactor=(fabsf(rewindFFFactor)<2.0f) ? 1.0f : LGL_Max(0.0f,1.0f-fabsf(fabsf(rewindFFFactor)-2.0f)/2.0f);
				localVolumeMultiplier=
					(0.0f+normalVolFactor)*1.00f+
					(1.0f-normalVolFactor)*0.25f;
				//LoopActive=false;
				//Sound->SetWarpPoint(Channel);
				if(Looping()==false)
				{
					LoopAlphaSeconds=-1.0f;
					//ClearRecallOrigin();
				}
				if
				(
					(long)Sound->GetPositionSamples(Channel) == 0 &&
					RewindFF==false &&
					rewindFFFactor<0.0f &&
					Sound->IsLoaded()
				)
				{
					Sound->SetPositionSamples(Channel,Sound->GetLengthSamples()-1);
				}
				if
				(
					(long)Sound->GetPositionSamples(Channel) == (long)Sound->GetLengthSamples()-1 &&
					RewindFF==false &&
					rewindFFFactor>0.0f
				)
				{
					Sound->SetPositionSamples(Channel,0);
				}
			}
			else if
			(
				RewindFF &&
				rewindFFFactor==0.0f
			)
			{
				//Exit RW/FF
				//Change our speed instantly.
				Sound->SetSpeed(Channel,Pitchbend*PauseMultiplier,true);
			}

			RewindFF=(rewindFFFactor!=0.0f);

			if
			(
				RewindFF ||
				recordHold ||
				recordSpeed!=0.0f
			)
			{
				//Velocity Queue
				if(Looping())
				{
					//LoopActive=false;
				}
				if(Looping()==false)
				{
					LoopAlphaSeconds=-1.0f;
					//ClearRecallOrigin();
					Sound->SetWarpPoint(Channel);
				}
				RecordScratch=true;
				float driveFactor=Input.WaveformRecordHold(target) ? 0.0f : 1.0f;//LGL_Min(RecordHoldReleaseTimer.SecondsSinceLastReset()*4.0f,1.0f);

				float driveSpeed = 
					(1.0f-driveFactor) * recordSpeed +
					(0.0f+driveFactor) * (Pitchbend*PauseMultiplier) * (RewindFF ? 0.0f : 1.0f);

				FinalSpeed=
					driveSpeed+
					recordSpeed+
					Nudge+
					MixerNudge;
			}
			else
			{
				RecordScratch=false;
			}
		}

		//Volume
		VolumeInvertBinary=Input.WaveformVolumeInvert(target);

		Sound->SetRapidInvertProperties
		(
			Channel,
			GetBeginningOfCurrentMeasureSeconds(),
			GetQuantizePeriodSeconds()
		);
		if(GetBPM()>0)
		{
			if(Input.WaveformRapidVolumeInvert(target))
			{
				RapidVolumeInvert=true;
			}
			else
			{
				RapidVolumeInvert=false;
			}
			Sound->SetRapidVolumeInvert(Channel,RapidVolumeInvert);
		}
		else
		{
			RapidVolumeInvert=false;
		}

		RapidSoloInvert = Input.WaveformRapidSoloInvert(target);

		//Save Points
		if(AudioInputMode==false)
		{
			if(Input.WaveformSavePointPrev(target))
			{
				//Prev Save Point
				SavePointIndex--;
				if(SavePointIndex<0)
				{
					SavePointIndex=11;
				}
			}
			if(Input.WaveformSavePointNext(target))
			{
				//Next Save Point
				SavePointIndex++;
				if(SavePointIndex>11)
				{
					SavePointIndex=0;
				}
			}
			int savepointCandidate=Input.WaveformSavePointPick(target);
			if(savepointCandidate!=-9999)
			{
				SavePointIndex=LGL_Clamp(0,savepointCandidate+2,11);
			}
			if
			(
				SavePointIndex>=0 &&
				Input.WaveformSavePointSet(target)
			)
			{
				//Set Save Point
				if(SavePointSeconds[SavePointIndex]==-1.0f)
				{
					SavePointSeconds[SavePointIndex]=Sound->GetPositionSeconds(Channel);
					SavePointUnsetFlashPercent[SavePointIndex]=1.0f;
					SaveMetaData();
				}
			}
			if
			(
				SavePointIndex>=0 &&
				Input.WaveformSavePointUnsetPercent(target)==1.0f &&
				SavePointSeconds[SavePointIndex]!=-1.0f
			)
			{
				//Unset Save Point
printf("Unset!\n");
				SavePointSeconds[SavePointIndex]=-1.0f;
				SavePointUnsetFlashPercent[SavePointIndex]=1.0f;
				SaveMetaData();
			}

			for(int a=0;a<18;a++)
			{
				SavePointUnsetFlashPercent[a]=LGL_Max(0,SavePointUnsetFlashPercent[a]-4.0f*LGL_SecondsSinceLastFrame());
				SavePointUnsetNoisePercent[a]=LGL_Max(0,SavePointUnsetNoisePercent[a]-2.0f*LGL_SecondsSinceLastFrame());
				if(a==SavePointIndex)
				{
					SavePointUnsetNoisePercent[SavePointIndex]=LGL_Max
					(
						SavePointUnsetNoisePercent[SavePointIndex],
						LGL_Min(1,(SavePointSeconds[SavePointIndex]>-1.0f) ? (Input.WaveformSavePointUnsetPercent(target)*2.0f) : 0)
					);
				}
			}

			if
			(
				SavePointIndex>=0 &&
				SavePointSeconds[SavePointIndex]!=-1.0f
			)
			{
				//Shift current save point
				candidate=Input.WaveformSavePointShift(target);
				if(candidate!=0.0f)
				{
					SavePointSeconds[SavePointIndex]=LGL_Max
					(
						-0.99f,
						SavePointSeconds[SavePointIndex]+candidate
					);
					SaveMetaData();
				}
			}

			candidate=Input.WaveformSavePointShiftAll(target);
			if(candidate!=0.0f)
			{
				//Shift all save points
				for(int a=0;a<12;a++)
				{
					if(SavePointSeconds[a]!=-1.0f)
					{
						SavePointSeconds[a]+=candidate;
					}
				}
				SaveMetaData();
			}
			if
			(
				BPMAvailable() &&
				Input.WaveformSavePointShiftAllHere(target)
			)
			{
				double delta = Sound->GetPositionSeconds(Channel)-GetBPMFirstBeatSeconds();
				for(int a=0;a<12;a++)
				{
					if(SavePointSeconds[a]!=-1.0f)
					{
						SavePointSeconds[a]+=delta;
					}
				}
				SaveMetaData();
			}
			if
			(
				Input.WaveformSavePointJumpNow(target) &&
				SavePointSeconds[SavePointIndex]!=-1.0f
			)
			{
				if(Looping())
				{
					LoopAlphaSeconds=-1.0f;
					LoopActive=false;
					LoopThenRecallActive=false;
					Sound->SetWarpPoint(Channel);
				}
				double savePointSeconds=SavePointSeconds[SavePointIndex];
				if(savePointSeconds < 0.0f)
				{
					savePointSeconds=0;
				}

				if(Sound->GetLengthSeconds()>savePointSeconds)
				{
					Sound->SetPositionSeconds(Channel,savePointSeconds);
					SmoothWaveformScrollingSample=savePointSeconds*Sound->GetHz();
				}
			}
			if
			(
				Input.WaveformSavePointJumpAtMeasure(target) &&
				SavePointSeconds[SavePointIndex]!=-1.0f &&
				GetBPM()!=0 &&
				PauseMultiplier!=0
			)
			{
				if(Looping())
				{
					LoopActive=false;
					Sound->SetWarpPoint(Channel);
					if(Looping()==false)
					{
						LoopAlphaSeconds=-1.0f;
						ClearRecallOrigin();
					}
				}
				if(Sound->GetWarpPointSecondsAlpha(Channel)>0.0f)
				{
					Sound->SetWarpPoint(Channel);
				}
				else
				{
					double savePointSeconds=SavePointSeconds[SavePointIndex];
					if(savePointSeconds >= 0.0f)
					{
						//Jump at start of next measure
						double beatStart=GetBPMFirstBeatSeconds();
						double measureLength=GetMeasureLengthSeconds();
						while(beatStart>0)
						{
							beatStart-=measureLength;
						}
						double savePointSecondsQuantized=beatStart;
						double closest=99999.0;
						for(double test=beatStart;test<savePointSeconds+2*measureLength;test+=0.25*measureLength)
						{
							if(fabsf(test-savePointSeconds)<closest)
							{
								closest=fabsf(test-savePointSeconds);
								savePointSecondsQuantized=test;
							}
						}
						double nowSeconds=Sound->GetPositionSeconds(Channel);
						double savePointMeasureStart=beatStart;
						while(savePointMeasureStart+measureLength<savePointSecondsQuantized)
						{
							savePointMeasureStart+=measureLength;
						}
						double savePointSecondsIntoMeasure=savePointSecondsQuantized-savePointMeasureStart;
						double nowMeasureStart=beatStart;
						while(nowMeasureStart+measureLength<nowSeconds)
						{
							nowMeasureStart+=measureLength;
						}
						double timeToWarp=nowMeasureStart+savePointSecondsIntoMeasure;
						while(timeToWarp<nowSeconds)
						{
							timeToWarp+=measureLength;
						}

						Sound->SetWarpPoint(Channel,timeToWarp,savePointSecondsQuantized);
						Sound->SetDivergeRecallBegin(Channel,Pitchbend);
					}
				}
			}

			candidate = Input.WaveformJumpToPercent(target);
			if(candidate!=-1.0f)
			{
				if(Sound->IsLoaded())
				{
					candidate=LGL_Clamp(0.0f,candidate,1.0f);
					float candidateSeconds=Sound->GetLengthSeconds()*candidate;
					double jumpAlphaSeconds=Sound->GetPositionSeconds(Channel);
					double jumpOmegaSeconds=candidateSeconds;
					if(BPMAvailable() && PauseMultiplier!=0.0f)
					{
						double currentMeasureStart=GetBeginningOfCurrentMeasureSeconds();
						double candidateMeasureStart=GetBeginningOfArbitraryMeasureSeconds(candidateSeconds);
						double quantizePeriodSeconds=GetQuantizePeriodSeconds();

						jumpAlphaSeconds=currentMeasureStart;
						jumpOmegaSeconds=candidateMeasureStart;
						for(;;)
						{
							float posSeconds = Sound->GetPositionSeconds(Channel);
							if(jumpAlphaSeconds < posSeconds)
							{
								jumpAlphaSeconds += quantizePeriodSeconds;
								jumpOmegaSeconds += quantizePeriodSeconds;
							}
							else
							{
								break;
							}
						}

						/*
						double beatStart=GetBPMFirstMeasureSeconds();
						double measureLength=GetMeasureLengthSeconds();
						double percent=GetPercentOfCurrentMeasure();
						if(beatStart<candidateSeconds)
						{
							double measureNow=beatStart;
							while(measureNow+measureLength<candidateSeconds)
							{
								measureNow+=measureLength;
							}
							candidateSeconds=measureNow+percent*measureLength;
						}
						*/
					}
					//Sound->SetPositionSeconds(Channel,candidateSeconds);

					if
					(
						jumpAlphaSeconds>=0.0f &&
						jumpOmegaSeconds>=0.0f &&
						jumpOmegaSeconds<=Sound->GetLengthSeconds()-5.0f
					)
					{
						Sound->SetWarpPoint
						(
							Channel,
							jumpAlphaSeconds,
							jumpOmegaSeconds,
							false
						);
					}
				}
			}
		}

		//Pitch
		if(AudioInputMode==false)
		{
			Nudge=Input.WaveformNudge(target);

			candidate = Input.WaveformPitchbend(target);
			if(candidate!=0.0f)
			{
				if
				(
					PitchbendLastSetBySlider ||
					fabsf(Pitchbend-candidate)<0.0025f
				)
				{
					Pitchbend=candidate;
					PitchbendLastSetBySlider=true;
				}
			}
			candidate=Input.WaveformPitchbendDelta(target);
			if(candidate!=0.0f)
			{
				Pitchbend=LGL_Clamp
				(
					0.01f,
					Pitchbend+candidate,
					4.0f
				);
				PitchbendLastSetBySlider=false;
			}
		}

		//Glitch
		if
		(
			//AudioInputMode==false &&
			Input.WaveformStutter(target)
		)
		{
			bool glitchDuoPrev=GlitchDuo;
			GlitchDuo=true;

			float fractionOfBeat=1.0f;

			GlitchPitch = 2*LGL_Max(0.0f,Input.WaveformStutterPitch(target));
			float glitchSpeed = LGL_Max(0.0f,Input.WaveformStutterSpeed(target));

			if(glitchSpeed>=0.0f)
			{
				fractionOfBeat = 1.0f/powf(2.0f,floorf(glitchSpeed*2.0f));
			}
			if
			(
				(
					glitchSpeed>0 &&
					GetBeatThisFrame(fractionOfBeat)
				) ||
				glitchDuoPrev==false
			)
			{
				GlitchBegin=Sound->GetPositionSamples(Channel);
			}

			if(glitchSpeed<0)
			{
				GlitchLength=512+2048;
			}
			else
			{
				GlitchLength=512+2048*(1.0f-glitchSpeed);
			}
		}
		else
		{
			GlitchDuo=false;
		}

		//Looping
		bool loopActiveLastFrame = LoopActive;
		bool loopThenRecallActiveLastFrame = LoopThenRecallActive;

		if(LoopActive && Sound->GetWarpPointSecondsAlpha(Channel)<0)
		{
			LoopActive=false;
		}
		LoopActive = 
		(
			(Input.WaveformLoopToggle(target) ? !LoopActive : LoopActive) &&
			RapidVolumeInvert==false
		);
		LoopThenRecallActive =
		(
			Input.WaveformLoopThenRecallActive(target) &&
			RapidVolumeInvert==false
		);

		bool loopChanged=
			(
				loopActiveLastFrame != LoopActive ||
				loopThenRecallActiveLastFrame != LoopThenRecallActive
			);

		if
		(
			LoopThenRecallActive &&
			loopThenRecallActiveLastFrame==false
		)
		{
			LoopActive=false;
			SetRecallOrigin();
		}

		if
		(
			BPMAvailable() &&
			1 //AudioInputMode==false
		)
		{
			//Looping with BPM
			const int exponentMin=-9;
			const int exponentMax=6;
			const int exponentAll=9999;

			if
			(
				Input.WaveformLoopAll(target) &&
				Sound->IsLoaded()
			)
			{
				QuantizePeriodMeasuresExponent=exponentAll;
				loopChanged=true;
				LoopActive=true;
			}

			if(Input.WaveformLoopMeasuresHalf(target))
			{
				if(QuantizePeriodMeasuresExponent==exponentAll)
				{
					QuantizePeriodMeasuresExponent=exponentMax;
				}
				else
				{
					QuantizePeriodMeasuresExponent=LGL_Max(QuantizePeriodMeasuresExponent-1,exponentMin);
				}
				loopChanged=true;
			}

			if(Input.WaveformLoopMeasuresDouble(target))
			{
				QuantizePeriodMeasuresExponent=LGL_Min(QuantizePeriodMeasuresExponent+1,exponentMax);
				loopChanged=true;
			}

			candidate=Input.WaveformLoopMeasuresExponent(target);
			if(candidate!=WAVEFORM_LOOP_MEASURES_EXPONENT_NULL)
			{
				if
				(
					LoopActive &&
					QuantizePeriodMeasuresExponent==candidate
				)
				{
					LoopActive=false;
				}
				else
				{
					LoopActive=true;
				}
				QuantizePeriodMeasuresExponent=LGL_Clamp(exponentMin,candidate,exponentMax);
				loopChanged=true;
			}

			if(loopChanged)
			{
				if(Looping()==false)
				{
					Sound->SetWarpPoint(Channel);
					LoopAlphaSeconds=-1.0f;
					if
					(
						loopThenRecallActiveLastFrame &&
						PauseMultiplier!=0.0f
					)
					{
						Recall();
					}
					else
					{
						ClearRecallOrigin();
					}
				}
				else
				{
					if(QuantizePeriodMeasuresExponent==exponentAll)
					{
						LoopAlphaSeconds = (SavePointSeconds[8+2] != -1.0f) ? SavePointSeconds[8+2] : GetBPMFirstBeatSeconds();
						LoopOmegaSeconds = (SavePointSeconds[9+2] != -1.0f) ? SavePointSeconds[9+2] : GetBPMLastMeasureSeconds();
						if(LoopOmegaSeconds<LoopAlphaSeconds)
						{
							LoopOmegaSeconds=LoopAlphaSeconds;
						}
					}
					else
					{
						LoopAlphaSeconds = GetBeginningOfCurrentMeasureSeconds();
						float deltaSeconds = GetQuantizePeriodSeconds();

						for(;;)
						{
							float posSeconds = Sound->GetPositionSeconds(Channel);
							if(LoopAlphaSeconds + deltaSeconds < posSeconds)
							{
								LoopAlphaSeconds += deltaSeconds;
							}
							else
							{
								break;
							}
						}

						LoopOmegaSeconds=LGL_Min
						(
							LoopAlphaSeconds+deltaSeconds,
							GetBPMLastMeasureSeconds()
						);
						if(LoopOmegaSeconds<LoopAlphaSeconds)
						{
							LoopOmegaSeconds=LoopAlphaSeconds;
						}
					}

					Sound->SetWarpPoint
					(
						Channel,
						LoopOmegaSeconds,
						LoopAlphaSeconds,
						true
					);
				}
			}
		}
		else if
		(
			BPMAvailable()==false &&
			1 //AudioInputMode==false
		)
		{
			//Looping without BPM
			if(loopChanged)
			{
				if(LoopAlphaSeconds==-1.0f && Looping())
				{
					LoopAlphaSeconds=Sound->GetPositionSeconds(Channel);
				}
				else if(LoopAlphaSeconds!=1.0f && Looping()==false)
				{
					LoopAlphaSeconds=-1.0f;
				}
			}

			float secondsMin=powf(2.0f,-9);
			float secondsMax=LGL_Max(0.0f,Sound->GetLengthSeconds()-0.01f);//powf(2.0f,6);

			if
			(
				Input.WaveformLoopAll(target) &&
				Sound->IsLoaded()
			)
			{
				LoopAlphaSeconds=0.0f;
				QuantizePeriodNoBPMSeconds=secondsMax;
				loopChanged=true;
				LoopActive=true;
			}
			if(Input.WaveformLoopSecondsLess(target))
			{
				QuantizePeriodNoBPMSeconds=LGL_Max
				(
					QuantizePeriodNoBPMSeconds-
					QuantizePeriodNoBPMSeconds*2.0f*LGL_SecondsSinceLastFrame(),
					secondsMin
				);
				loopChanged=true;
			}

			if(Input.WaveformLoopSecondsMore(target))
			{
				QuantizePeriodNoBPMSeconds=LGL_Min
				(
					QuantizePeriodNoBPMSeconds+
					QuantizePeriodNoBPMSeconds*2.0f*LGL_SecondsSinceLastFrame(),
					secondsMax
				);
				loopChanged=true;
			}

			candidate=Input.WaveformLoopMeasuresExponent(target);
			if(candidate!=WAVEFORM_LOOP_MEASURES_EXPONENT_NULL)
			{
				QuantizePeriodNoBPMSeconds=LGL_Clamp(secondsMin,candidate,secondsMax);
				loopChanged=true;
			}

			if(loopChanged)
			{
				if(Looping()==false)
				{
					Sound->SetWarpPoint(Channel);
					if
					(
						loopThenRecallActiveLastFrame &&
						PauseMultiplier!=0.0f
					)
					{
						Recall();
					}
					else
					{
						ClearRecallOrigin();
					}
				}
				else
				{
					float deltaSeconds = QuantizePeriodNoBPMSeconds;
					LoopOmegaSeconds=LGL_Min
					(
						LoopAlphaSeconds+deltaSeconds,
						Sound->GetLengthSeconds()
					);
					if(LoopOmegaSeconds<LoopAlphaSeconds)
					{
						LoopOmegaSeconds=LoopAlphaSeconds;
					}
					Sound->SetWarpPoint
					(
						Channel,
						LoopOmegaSeconds,
						LoopAlphaSeconds,
						true
					);
				}
			}
		}

		//AutoDivergeRecall
		int autoDivergeRecall = Input.WaveformAutoDivergeRecall(target);
		if(autoDivergeRecall==-2)
		{
			AutoDivergeRecallActive=!AutoDivergeRecallActive;
		}
		else if(autoDivergeRecall==-1)
		{
			AutoDivergeRecallActive=false;
		}
		else if (autoDivergeRecall==0)
		{
			//
		}
		else
		{
			AutoDivergeRecallActive=true;
		}

		//Rapid Volume Invert vs QuantizePeriodMeasuresExponent
		if(RapidVolumeInvert || RapidSoloInvert)
		{
			const int rapidVolumeInvertMeasuresExponentMin=-6;
			const int rapidVolumeInvertMeasuresExponentMax=-1;
			if(QuantizePeriodMeasuresExponent<rapidVolumeInvertMeasuresExponentMin)
			{
				QuantizePeriodMeasuresExponent=rapidVolumeInvertMeasuresExponentMin;
			}
			if(QuantizePeriodMeasuresExponent>rapidVolumeInvertMeasuresExponentMax)
			{
				QuantizePeriodMeasuresExponent=rapidVolumeInvertMeasuresExponentMax;
			}
		}

		//Video
		if(VideoEncoderPercent==2.0f)
		{
			VideoEncoderPercent=-1.0f;
			char videoFileName[1024];
			findVideoPath
			(
				videoFileName,
				SoundSrcPath
			);
			VideoFileExists=LGL_FileExists(videoFileName);
			if(VideoFileExists)
			{
				VideoSwitchInterval=0.0f;
				SelectNewVideo();
			}
			else
			{
				VideoSwitchInterval=1.0f;
			}
		}

		if(Input.WaveformVideoSelect(target))
		{
			SelectNewVideo();
		}

		int mode=Input.WaveformAudioInputMode(target);
		if(mode!=-1)
		{
			if(mode==2)
			{
				AudioInputMode=!AudioInputMode;
			}
			else
			{
				AudioInputMode=(mode==1);
			}
			if(0 && AudioInputMode)
			{
				PauseMultiplier=0.0f;
			}
			SelectNewVideo();
		}

		bool next = Input.WaveformVideoAspectRatioNext(target);
		if(next)
		{
			AspectRatioMode=(AspectRatioMode+1)%3;
		}

		float newBright=Input.WaveformVideoBrightness(target);
		if(newBright!=-1.0f)
		{
			VideoBrightness=newBright;
		}
		VideoBrightness=LGL_Clamp
		(
			0.0f,
			VideoBrightness+Input.WaveformVideoBrightnessDelta(target),
			1.0f
		);

		newBright=Input.WaveformOscilloscopeBrightness(target);
		if(newBright!=-1.0f)
		{
			OscilloscopeBrightness=newBright;
		}
		OscilloscopeBrightness=LGL_Clamp
		(
			0.0f,
			OscilloscopeBrightness+Input.WaveformOscilloscopeBrightnessDelta(target),
			1.0f
		);

		newBright=Input.WaveformFreqSenseBrightness(target);
		if(newBright==-1.0f)
		{
			if(Input.WaveformFreqSenseBrightnessDelta(target)!=0.0f)
			{
				newBright=LGL_Clamp
				(
					0.0f,
					FreqSenseBrightness+Input.WaveformFreqSenseBrightnessDelta(target),
					1.0f
				);
			}
		}
		if(newBright!=-1.0f)
		{
			float tmp = FreqSenseBrightness;
			FreqSenseBrightness=newBright;
			if
			(
				tmp==0.0f &&
				FreqSenseBrightness >= 0.0f &&
				(
					strcmp(VideoLo->GetPath(),"NULL")==0 ||
					strcmp(VideoHi->GetPath(),"NULL")==0
				)
			)
			{
				SelectNewVideo();
			}
		}

		float newRate=Input.WaveformVideoAdvanceRate(target);
		if(newRate!=-1.0f)
		{
			float oldTime=GetVideoTimeSeconds();

			VideoAdvanceRate=newRate;
			VideoOffsetSeconds=0.0f;
			VideoOffsetSeconds=oldTime-GetVideoTimeSeconds();
		}

		//Speed
		if(RecordScratch)
		{
			if
			(
				endpointsSticky &&
				(long)Sound->GetPositionSamples(Channel) == 0 &&
				FinalSpeed<0.0f
			)
			{
				PauseMultiplier=0;
			}
		}
		else
		{
			if((long)Sound->GetPositionSamples(Channel) == (long)Sound->GetLengthSamples()-1)
			{
				PauseMultiplier=0;
			}

			FinalSpeed=PauseMultiplier*(Pitchbend+MixerNudge)+Nudge;
		}

		Sound->SetStickyEndpoints(Channel,endpointsSticky);
		Sound->SetSpeed
		(
			Channel,
			FinalSpeed
		);
			
		//Pause
		if
		(
			//AudioInputMode==false &&
			Input.WaveformTogglePause(target)
		)
		{
			//Toggle PauseMultiplier
			PauseMultiplier=(PauseMultiplier+1)%2;
			if
			(
				PauseMultiplier==1 &&
				(long)Sound->GetPositionSamples(Channel)==(long)Sound->GetLengthSamples()-1
			)
			{
				Sound->SetPositionSeconds(Channel,0.0f);
			}
			if(PauseMultiplier==1)
			{
				//Change our speed instantly.
				Sound->SetSpeed(Channel,Pitchbend*PauseMultiplier,true);
			}
		}

		//Eject
		int eject = Input.WaveformEject(target);
		if(eject > 0)
		{
			if
			(
				EjectTimer.SecondsSinceLastReset()>=1.0f ||
				eject == 2
			)
			{
				//Select New Track
				LGL_DrawLogWrite("!dvj::DeleteSound|%i\n",Which);
				Sound->PrepareForDelete();
				Channel=-1;
				Mode=3;
				if(LGL_VideoDecoder* dec = GetVideo())
				{
					LGL_DrawLogWrite("!dvj::DeleteVideo|%s\n",dec->GetPath());
				}

				char* update=new char[1024];
				sprintf(update,"OMEGA: %s",SoundName);
				TrackListFileUpdates.push_back(update);

				WhiteFactor=1.0f;

				return;
			}
			else
			{
				noiseIncreasing=true;
				NoiseFactor = LGL_Max(NoiseFactor,EjectTimer.SecondsSinceLastReset());
			}
		}
		else
		{
			EjectTimer.Reset();
		}
	}
	else if(Mode==3)
	{
		if(VideoEncoderTerminateSignal==0)
		{
			VideoEncoderTerminateSignal=1;
		}

		if
		(
			VideoEncoderThread==NULL &&
			Sound->ReadyForDelete()
		)
		{
			char oldSelection[2048];
			strcpy(oldSelection,SoundSrcPathShort);
			delete Sound;
			Sound=NULL;
			Channel=-1;
			DatabaseEntryNow=NULL;
			VideoEncoderPathSrc[0]='\0';
			VideoEncoderUnsupportedCodecTime=0.0f;
			if(VideoFront)
			{
				VideoFront->GetImage()->SetFrameNumber(-1);
				VideoFront->SetVideo(NULL);
			}
			if(VideoBack)
			{
				VideoBack->GetImage()->SetFrameNumber(-1);
				VideoBack->SetVideo(NULL);
			}
			Mode=0;
			FilterTextMostRecent[0]='\0';
			FilterText.SetString();
			DatabaseFilter.SetPattern("");
			DatabaseFilteredEntries=Database->GetEntryListFromFilter(&DatabaseFilter);

			FileTop=0;
			FileSelectInt=0;
			FileSelectFloat=0;

			FileSelectToString(oldSelection);

			AudioInputMode=false;
			Mode0BackspaceTimer.Reset();
			RecordScratch=false;
			LuminScratch=false;
		}
		else
		{
			NoiseFactor=1.0f;
			NoiseFactorVideo=1.0f;
		}
	}

	if(Focus)
	{
		if(Mode==2)
		{
			GlitchPure=
			(
				LGL_GetWiimote(1).ButtonDown(LGL_WIIMOTE_2) &&
				!(LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_MINUS)) &&
				!(LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_PLUS))
			);
			GlitchPureDuo=true;//(LGL_GetWiimote(1).ButtonDown(LGL_WIIMOTE_B)==false);
			if(GlitchPure)
			{
				GlitchPureSpeed=Pitchbend*LGL_Clamp(0,1.0f-LGL_GetWiimote(1).GetAccelRaw().GetY(),4.0f);
				if(glitchPurePrev==false)
				{
					GlitchBegin=Sound->GetPositionSamples(Channel);
					GlitchPureVideoNow=GlitchBegin/44100.0f;
				}
				else
				{
					GlitchPureVideoNow+=2*GlitchPureSpeed*LGL_Min(1.0f/120.0f,LGL_SecondsSinceLastFrame());
					while(GlitchPureVideoNow>(GlitchBegin/44100.0f)+(GlitchLength/44100.0f)*GLITCH_PURE_VIDEO_MULTIPLIER)
					{
						GlitchPureVideoNow-=(GlitchLength/44100.0f)*GLITCH_PURE_VIDEO_MULTIPLIER;
					}
				}
			}

			VolumeMultiplierNow=LGL_Clamp(0.0f,VolumeMultiplierNow+Input.WaveformGainDelta(target),16.0f);
			candidate = Input.WaveformGain(target);
			if(candidate!=-1.0f)
			{
				VolumeMultiplierNow = candidate;
			}

			//bool wasScratching = LuminScratch || RecordScratch;
			if(Input.WaveformPointerScratch(target)!=-1.0f)
			{
				//Lumin Scratch
				float centerSample=Sound->GetPositionSamples(Channel);
				float leftSample=centerSample-64*512*Pitchbend*2*Sound->GetHz()/44100.0f;
				float rightSample=centerSample+64*512*Pitchbend*2*Sound->GetHz()/44100.0f;

				float gSamplePercent=0.5f+1.05f*(Input.WaveformPointerScratch(target)-0.5f);
				gSamplePercent = LGL_Clamp(0,(gSamplePercent-0.2f)*(1.0f/0.6f),1);

				LuminScratch=true;
				LuminScratchSamplePositionDesired=
					(long)(leftSample+gSamplePercent*(rightSample-leftSample));
				while(LuminScratchSamplePositionDesired<0)
				{
					LuminScratchSamplePositionDesired+=Sound->GetLengthSamples();
				}
				while(LuminScratchSamplePositionDesired>=Sound->GetLengthSamples())
				{
					LuminScratchSamplePositionDesired-=Sound->GetLengthSamples();
				}
				GlitchBegin=Sound->GetPositionGlitchBeginSamples(Channel)-.005f*64*512*Pitchbend*2;
			}
			else
			{
				LuminScratch=false;
				LuminScratchSamplePositionDesired=-10000;
			}

			if
			(
				LuminScratch==false &&
				RecordScratch==false &&
				RecallIsSet() &&
				(
					LGL_GetWiimote(0).ButtonRelease(LGL_WIIMOTE_MINUS) ||
					LGL_GetWiimote(0).ButtonRelease(LGL_WIIMOTE_PLUS)
				)
			)
			{
				//Just finished Scratching... Recall?
				/*
				if(LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_B)==false)
				{
					Recall();
				}
				else
				{
					if(wasScratching)
					{
						ClearRecallOrigin();
					}
				}
				*/
			}

			if
			(
				false &&
				Sound->IsLoaded()
			)
			{
				//Granular Synthesis Freeze
				if(GrainStreamActive==false)
				{
					GrainStreamActiveSeconds=0.0f;
					GrainStreamInactiveSeconds=0.0f;
					GrainStreamSourcePoint=Sound->GetPositionSeconds(Channel);
					GrainStreamPitch=1.0f;
				}
				else
				{
					GrainStreamActiveSeconds+=secondsElapsed;
					//GrainStreamSourcePoint+=2.0f*secondsElapsed*LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS);
					if(1)
					{
						while(GrainStreamSourcePoint<0.0f)
						{
							GrainStreamSourcePoint+=Sound->GetLengthSeconds();
						}
						while(GrainStreamSourcePoint>=Sound->GetLengthSeconds())
						{
							GrainStreamSourcePoint-=Sound->GetLengthSeconds();
						}
					}
					else
					{
						GrainStreamSourcePoint=LGL_Clamp(0.0f,GrainStreamSourcePoint,Sound->GetLengthSeconds());
					}
					GrainStreamPitch=1.0f;//+0.2f*LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_YAXIS);
					if(GrainStreamPitch<0.01f)
					{
						GrainStreamPitch=0.01f;
					}
				}

				GrainStreamActive=true;
			}
			else
			{
				GrainStreamActive=false;
				GrainStreamInactiveSeconds+=secondsElapsed;
			}

			bool savePointJump=false;
			bool savePointRecall=false;

			/*
			if(LGL_GetWiimote(0).ButtonRelease(LGL_WIIMOTE_B))
			{
				ClearRecallOrigin();
			}
			*/

			/*
			if
			(
				(
					LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_B) &&
					LGL_GetWiimote(0).GetFlickXPositive()
				)
				||
				LGL_GetWiimote(0).ButtonStroke(LGL_WIIMOTE_HOME)
			)
			{
				savePointJump=true;
			}

			if
			(
				(
					LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_B) &&
					LGL_GetWiimote(1).GetFlickXPositive()
				) ||
				(
					LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_B)==false &&
					LGL_GetWiimote(0).ButtonRelease(LGL_WIIMOTE_HOME)
				)
			)
			{
				savePointRecall=true;
			}
			*/

			if
			(
				LuminScratch==false &&
				RecordScratch==false
			)
			{
				if
				(
					RecallIsSet() &&
					savePointRecall
				)
				{
					//Jump to where we'd be, had we not done any Save Point jumping
					Recall();
				}

				if
				(
					savePointJump &&
					SavePointIndex>=0 &&
					SavePointSeconds[SavePointIndex]>=0.0
				)
				{
					//Jump to Save Point
					if(RecallIsSet()==false)
					{
						SetRecallOrigin();
					}
					if
					(
						GetBPM()==0 ||
						PauseMultiplier==0
					)
					{
						double pos = SavePointSeconds[SavePointIndex];
						if
						(
							pos < 0.0f &&
							Sound->IsLoaded()==false
						)
						{
							pos=0.0f;
						}
						else if(pos <0.0f)
						{
							pos += Sound->GetLengthSeconds();
						}
						Sound->SetPositionSeconds(Channel,pos);
					}
					else
					{
						double beatStart=GetBPMFirstBeatSeconds();
						double measureLength=GetMeasureLengthSeconds();
						double savePointSeconds=SavePointSeconds[SavePointIndex];
						if
						(
							savePointSeconds < 0.0f &&
							Sound->IsLoaded()==false
						)
						{
							savePointSeconds=0.0f;
						}
						else if(savePointSeconds < 0.0f)
						{
							savePointSeconds += Sound->GetLengthSeconds();
						}
						double savePointSecondsQuantized=beatStart;
						double closest=99999.0;
						for(double test=beatStart;test<savePointSeconds+2*measureLength;test+=0.25*measureLength)
						{
							if(fabsf(test-savePointSeconds)<closest)
							{
								closest=fabsf(test-savePointSeconds);
								savePointSecondsQuantized=test;
							}
						}
						double nowSeconds=Sound->GetPositionSeconds(Channel);
						double savePointMeasureStart=beatStart;
						while(savePointMeasureStart+measureLength<savePointSecondsQuantized)
						{
							savePointMeasureStart+=measureLength;
						}
						double savePointSecondsIntoMeasure=savePointSecondsQuantized-savePointMeasureStart;
						double nowMeasureStart=beatStart;
						while(nowMeasureStart+measureLength<nowSeconds)
						{
							nowMeasureStart+=measureLength;
						}
						double timeToWarp=nowMeasureStart+savePointSecondsIntoMeasure;
						while(timeToWarp<nowSeconds)
						{
							timeToWarp+=measureLength;
						}
						Sound->SetWarpPoint(Channel,timeToWarp,savePointSecondsQuantized);
						Sound->SetDivergeRecallBegin(Channel,Pitchbend);
					}
				}
			}

			/*
			//Must continue to hold down WIIMOTE_B, to warp
			if
			(
				LGL_GetWiimote(0).Connected() &&
				LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_B)==false
			)
			{
				Sound->SetWarpPoint(Channel);
			}

			if
			(
				RecallIsSet() &&
				wasScratching &&
				LGL_GetWiimote(0).ButtonStroke(LGL_WIIMOTE_B)
			)
			{
				//Clear Recall
				ClearRecallOrigin();
			}
			*/
		}

		if
		(
			0 &&
			VideoSwitchInterval> 0.001f &&
			GetBeatThisFrame(VideoSwitchInterval)
		)
		{
			SelectNewVideo(true);
		}
	}

	//Update State

	if(Mode==0)
	{
		//File Selection

		if(Focus==false)
		{
			BadFileFlash=0.0f;
		}
	}
	else if(Mode==1)
	{
		//Loading...
		
		if
		(
			Sound->GetLengthSamples()>0 &&
			LGL_WriteFileAsyncQueueCount()==0
		)
		{
			Mode=2;

			LoadMetaData();

			BPMRecalculationRequired=true;

			Channel=Sound->Play(1,true,0);
			Sound->SetSpeedInterpolationFactor
			(
				Channel,
				//16.0/44100.0
				//20.0/44100.0
				32.0f/44100.0f
			);

			if(FileEverOpened==false)
			{
				FileEverOpened=true;

				//Enforce surround
				SurroundMode=Sound->GetChannelCount()==4;
				if(SurroundMode)
				{
					LGL_AudioMasterToHeadphones(false);
				}
				
				//Start recording!
				char path[2048];
				strcpy(path,GetDVJSessionFlacPath());
				if(GetDVJSessionRecordAudio())
				{
					LGL_RecordDVJToFileStart(path,SurroundMode);
				}
				/*
				LGL_DrawLogStart(GetDVJSessionDrawLogPath());
				{
					char drawLogPath[2048];
					sprintf(drawLogPath,"%s/drawlog.txt",recordPath);

					if(LGL_FileExists(drawLogPath))
					{
						LGL_FileDelete(drawLogPath);
					}
					LGL_DrawLogStart(drawLogPath);

					LGL_DrawLogWrite
					(
						"!dvj::Record.mp3|%s/%s.mp3\n",
						recordPath,
						LGL_DateAndTimeOfDayOfExecution()
					);
				}
				*/
			}
			/*
			if(LGL_AudioChannels()==2)
			{
				Sound->SetDownMixToMono
				(
					Channel,
					true
				);
			}
			else
			{
				//We have 4 channels. Don't downmix to mono.
			}
			*/
			PauseMultiplier=0;
			Pitchbend=1;
			PitchbendLastSetBySlider=false;
			Nudge=0;
			MixerNudge=0;
			GlitchPure=false;
			GlitchPureDuo=false;
			GlitchDuo=false;
			GlitchVolume=0;
			GlitchBegin=0;
			GlitchLength=0;
			GlitchPitch=1.0f;
			SmoothWaveformScrollingSample=0.0f;
			VideoOffsetSeconds=LGL_RandFloat(0,1000.0f);

			VideoEncoderPercent=-1.0f;
			VideoEncoderEtaSeconds=-1.0f;
			VideoEncoderAudioOnly=false;

			VideoEncoderTerminateSignal=0;
			VideoEncoderBeginSignal=0;
			strcpy(VideoEncoderPathSrc,SoundSrcPath);
			VideoEncoderThread=LGL_ThreadCreate(videoEncoderThread,this);

			//Load Video if possible
			SelectNewVideo();

			char videoFileName[1024];
			findVideoPath
			(
				videoFileName,
				SoundSrcPath
			);
			VideoFileExists=LGL_FileExists(videoFileName);
			if(VideoFileExists)
			{
				VideoSwitchInterval=0.0f;
				SelectNewVideo();
			}
			else
			{
				VideoSwitchInterval=1.0f;
			}

			UpdateSoundFreqResponse();
		}
	}
	else if(Mode==2)
	{
		if(Sound->IsPlaying(Channel)==false)
		{
			Channel=Sound->Play(1,false,0);
			if(Sound->IsLoaded())
			{
				Sound->SetPositionSeconds(Channel,SmoothWaveformScrollingSample*Sound->GetHz());
				FinalSpeed=0.0f;
				Sound->SetSpeed
				(
					Channel,
					FinalSpeed
				);
				PauseMultiplier=0;
			}
			else
			{
				Sound->SetPositionSeconds(Channel,Sound->GetLengthSeconds()-1);
				SmoothWaveformScrollingSample=Sound->GetPositionSamples(Channel);
			}
			Sound->SetSpeedInterpolationFactor
			(
				Channel,
				16.0/44100.0
			);
		}

		if
		(
			Sound->GetLengthSeconds()>CachedLengthSeconds &&
			Sound->IsLoaded()
		)
		{
			CachedLengthSeconds=Sound->GetLengthSeconds();
			if(CachedLengthSeconds!=0.0f)
			{
				SaveCachedMetadata();
			}
		}

		//Smooth waveform scrolling

		float speed=Sound->GetSpeed(Channel);
		double proposedDelta = (LGL_AudioAvailable()?1:0)*speed*Sound->GetHz()*(1.0f/LGL_GetFPSMax());
		double currentSample=Sound->GetPositionSamples(Channel);
		double diff=fabs(currentSample-(SmoothWaveformScrollingSample+proposedDelta));
		double diffMax=LGL_AudioCallbackSamples()*16*LGL_Max(1,fabsf(Sound->GetSpeed(Channel)));
		double deltaFrame = LGL_SecondsSinceLastFrame();
		if(deltaFrame <= 5.0f/60.0f)
		{
			deltaFrame = 1.0f/60.0f;
		}
		if
		(
			PauseMultiplier==0 &&
			RecordScratch==false &&
			fabsf(Sound->GetSpeed(Channel))<1.0f
		)
		{
			proposedDelta=(LGL_AudioAvailable()?1:0)*LGL_Sign(currentSample-SmoothWaveformScrollingSample)*1.0f*Sound->GetHz()*deltaFrame;
			if(fabsf(proposedDelta)>fabsf(diff))
			{
				proposedDelta=fabsf(diff)*LGL_Sign(proposedDelta);
			}

			diff=fabs(currentSample-(SmoothWaveformScrollingSample+proposedDelta));
			diffMax=LGL_AudioCallbackSamples()*16*LGL_Max(1,fabsf(Sound->GetSpeed(Channel)));
		}

		if
		(
			LGL_SecondsSinceLastFrame()>0.25f ||
			fabsf(FinalSpeedLastFrame-FinalSpeed)>1.5f
		)
		{
			//A chance to seek to the exact sample!
			diff=999999;
			//badFlash=1.0f;
		}

#if 0
LGL_ClipRectDisable();
double diffSigned=currentSample-(SmoothWaveformScrollingSample+proposedDelta);
LGL_DrawLineToScreen
(
	0.75f-0.25f*diffSigned/diffMax,0.6f,
	0.75f-0.25f*diffSigned/diffMax,0.7f,
	1,1,1,1
);
LGL_DrawRectToScreen
(
	0.7f,0.8f,
	0.7f,0.8f,
	badFlash,0,0,1
);
badFlash-=LGL_SecondsSinceLastFrame();
if(badFlash<0) badFlash=0.0f;
LGL_ClipRectEnable(ViewportLeft,ViewportRight,ViewportBottom,ViewportTop);
#endif

		float constantThreashold=1024;//diffMax/2;
		if(diff<constantThreashold)
		{
			//We're accurate enough
			SmoothWaveformScrollingSample+=proposedDelta;
		}
		else if(diff<diffMax)
		{
			//Let's change our scrolling speed to get more accurate
			float deltaFactor=(diff-constantThreashold)/(diffMax-constantThreashold);
			float deltaMultiplier;
			if(Sound->GetSpeed(Channel)>0.0f)
			{
				deltaMultiplier=(currentSample>SmoothWaveformScrollingSample+proposedDelta) ? (1.0f+deltaFactor) : (1.0f-deltaFactor);
			}
			else if (Sound->GetSpeed(Channel)==0.0f)
			{
				deltaMultiplier=1.0f;
			}
			else
			{
				deltaMultiplier=(currentSample<SmoothWaveformScrollingSample+proposedDelta) ? (1.0f+deltaFactor) : (1.0f-deltaFactor);
			}
			//deltaMultiplier=powf(deltaMultiplier,2);
			proposedDelta*=deltaMultiplier*LGL_SecondsSinceLastFrame()*60.0f;
			SmoothWaveformScrollingSample+=proposedDelta;
		}
		else
		{
			//Fuck, we're really far off. Screw smooth scrolling, just jump to currentSample
			SmoothWaveformScrollingSample=currentSample;
		}
		SmoothWaveformScrollingSample=LGL_Clamp(0,SmoothWaveformScrollingSample,Sound->GetLengthSamples());

		if(Looping())
		{
			double loopSamples = Sound->GetHz()*(LoopOmegaSeconds - LoopAlphaSeconds);
			if(loopSamples>0)
			{
				while(SmoothWaveformScrollingSample>LoopOmegaSeconds*Sound->GetHz())
				{
					SmoothWaveformScrollingSample-=loopSamples;
				}
				while(SmoothWaveformScrollingSample<LoopAlphaSeconds*Sound->GetHz())
				{
					SmoothWaveformScrollingSample+=loopSamples;
				}
			}
			else
			{
				if(SmoothWaveformScrollingSample>LoopOmegaSeconds*Sound->GetHz())
				{
					SmoothWaveformScrollingSample=0;
				}
			}
		}

		if
		(
			GlitchDuo ||
			GlitchPure ||
			LuminScratch
		)
		{
			float volume=1;
			float GlitchInterpolation=20.0f/44100.0f;
			if(LuminScratch)
			{
				GlitchLength=Sound->GetLengthSamples();
				GlitchDuo=false;
				GlitchInterpolation=40.0f/44100.0f;
				if(LuminScratchSamplePositionDesired!=-10000)
				{
					GlitchInterpolation=40.0/44100.0;
				}
				GlitchLength=(int)floor(.01f*64*512*2);
			}

			if(GlitchPure)
			{
				GlitchLength=2048+2048*LGL_Clamp(0,1.0f-LGL_GetWiimote(0).GetAccelRaw().GetY(),4.0f);
				Sound->SetGlitchAttributes
				(
					Channel,
					true,
					(long)GlitchBegin,
					(int)GlitchLength,
					volume,
					GlitchPureSpeed*GlitchPitch,
					GlitchPureDuo,
					GlitchInterpolation,
					false,
					-10000
				);
			}
			else
			{
				if(glitchPurePrev)
				{
					Sound->SetGlitchSamplesNow(Channel,Sound->GetPositionSamples(Channel));
				}
				Sound->SetGlitchAttributes
				(
					Channel,
					true,
					(long)GlitchBegin,
					(int)GlitchLength,
					volume,
					Pitchbend*GlitchPitch,
					GlitchDuo,
					GlitchInterpolation,
					LuminScratch,
					LuminScratchSamplePositionDesired
				);
			}
		}
		else
		{
			Sound->SetGlitchAttributes
			(
				Channel,
				false,
				0,
				0,
				0,
				1,
				false,
				1,
				false,
				LuminScratchSamplePositionDesired
			);
			GlitchLength=0;
		}

		if(GrainStreamActive)
		{
			float timeStamp=GrainStreamSpawnDelaySeconds;
			float joyVarY=0.0f;//2.0f*fabsf(LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS));
			while(timeStamp<secondsElapsed)
			{
				LGL_AudioGrain* grain=new LGL_AudioGrain;
				grain->SetWaveformFromLGLSound
				(
					Sound,
					GrainStreamSourcePoint,
					GrainStreamLength*GrainStreamPitch,
					GrainStreamSourcePointVariance,
					GrainStreamLengthVariance*GrainStreamPitch
				);
				grain->SetStartDelaySeconds(timeStamp,GrainStreamStartDelayVariance*joyVarY);
				grain->SetVolumeSurround
				(
					GrainStreamVolume*MixerVolumeFront,
					GrainStreamVolume*MixerVolumeFront,
					GrainStreamVolume*MixerVolumeBack,
					GrainStreamVolume*MixerVolumeBack,
					GrainStreamVolumeVariance*MixerVolumeFront,
					GrainStreamVolumeVariance*MixerVolumeFront,
					GrainStreamVolumeVariance*MixerVolumeBack,
					GrainStreamVolumeVariance*MixerVolumeBack
				);
				grain->SetPitch(GrainStreamPitch,GrainStreamPitchVariance);
				GrainStream.AddNextGrain(grain);
				timeStamp+=1.0f/GrainStreamGrainsPerSecond;
			}
			GrainStreamSpawnDelaySeconds=timeStamp-secondsElapsed;
		}
		else
		{
			GrainStreamSpawnDelaySeconds=0.0f;
		}

		if(GetVideo()!=NULL)
		{
			float time = Sound->GetPositionSeconds(Channel);
			if(LuminScratch)
			{
				time = LuminScratchSamplePositionDesired / 44100.0f;
			}
		}
	}

	if(Mode==2)
	{
		if(GrainStreamActive)
		{
			GrainStreamCrossfader=LGL_Max(GrainStreamCrossfader,GrainStreamActiveSeconds/GrainStreamLength);
			if(GrainStreamCrossfader>1.0f)
			{
				GrainStreamCrossfader=1.0f;
			}
		}
		else
		{
			GrainStreamCrossfader=LGL_Min(GrainStreamCrossfader,GrainStreamCrossfader-GrainStreamInactiveSeconds/GrainStreamLength);
			if(GrainStreamCrossfader<0.0f)
			{
				GrainStreamCrossfader=0.0f;
			}
			if(Sound!=NULL)
			{
				GrainStreamSourcePoint=Sound->GetPositionSeconds(Channel);
			}
			else
			{
				GrainStreamSourcePoint=0.0f;
			}
		}
	}

	if(Mode==2)
	{
		float vol = VolumeKill ? 0.0f : 0.5f*VolumeSlider*VolumeMultiplierNow*(1.0f-GrainStreamCrossfader);
		if(FinalSpeed!=0.0f)
		{
			vol *= (1.0f-NoiseFactor);
		}
		if(AudioInputMode)
		{
			vol = 0.0f;
		}

		float volFront = VolumeInvertBinary ?
			(MixerVolumeFront==0.0f ? 1.0f : 0.0f) :
			MixerVolumeFront;
		float volBack = VolumeInvertBinary ?
			(MixerVolumeBack==0.0f ? 1.0f : 0.0f) :
			MixerVolumeBack;
		if(volumeFull)
		{
			volFront=1.0f;
			volBack=1.0f;
		}

		volFront*=localVolumeMultiplier;
		volBack*=localVolumeMultiplier;

		if(LGL_AudioChannels()==2)
		{
			Sound->SetVolumeStereo
			(
				Channel,
				volFront*vol,
				volBack*vol
			);
		}
		else if(LGL_AudioChannels()==4 || LGL_AudioChannels()==6)
		{
			Sound->SetVolumeSurround
			(
				Channel,
				volFront*vol,
				volFront*vol,
				volBack*vol,
				volBack*vol
			);
		}
		else
		{
			LGL_Assert(LGL_AudioChannels()==2 || LGL_AudioChannels()==4 || LGL_AudioChannels()==6);
		}
	}

	WhiteFactor = LGL_Max(0.0f,WhiteFactor-4.0f*LGL_Min(LGL_SecondsSinceLastFrame(),1.0f/60.0f));
	if(noiseIncreasing==false)
	{
		NoiseFactor = LGL_Max(0.0f,NoiseFactor-2.0f*LGL_Min(LGL_SecondsSinceLastFrame(),1.0f/60.0f));
	}
}

long lastSampleLeft=0;
//float badFlash=0.0f;

void
TurntableObj::
DrawFrame
(
	float	glow,
	bool	visualsQuadrent,
	float	visualizerZoomOutPercent
)
{
	float coolR;
	float coolG;
	float coolB;
	GetColorCool(coolR,coolG,coolB);

	float warmR;
	float warmG;
	float warmB;
	GetColorWarm(warmR,warmG,warmB);

	if(visualsQuadrent==false)
	{
		LGL_ClipRectEnable(ViewportLeft,ViewportRight,ViewportBottom,ViewportTop);
	}
	else
	{
		LGL_ClipRectEnable(0.0f,0.5f,0.5f,1.0f);
	}

	//On Left
	float left=	ViewportLeft;
	float right=	0.5f-0.501f*WAVE_WIDTH_PERCENT*ViewportWidth;
	float width=	right-left;

	float bottom=	ViewportBottom+0.125*ViewportHeight;
	float top=	ViewportBottom+0.875*ViewportHeight;
	float height=	top-bottom;

	float rectAlpha=1.0f;
	if(Mode==2)
	{
		float centerX=0.5f*(right+left);
		if(VideoEncoderUnsupportedCodecTime>0.0f)
		{
			float bright=LGL_Min(VideoEncoderUnsupportedCodecTime,1.0f);
			LGL_GetFont().DrawString
			(
				centerX,bottom+0.90f*height,.015,
				bright,0,0,bright,
				true,.5,
				"Unsupported Codec"
			);
			
			float fontHeight=0.015f;
			float fontWidth=LGL_GetFont().GetWidthString(fontHeight,VideoEncoderUnsupportedCodecName);
			float fontWidthMax=0.175f;
			fontHeight=LGL_Min(fontHeight,fontHeight*fontWidthMax/fontWidth);
			LGL_GetFont().DrawString
			(
				centerX,bottom+0.10f*height,fontHeight,
				bright,0,0,bright,
				true,.5,
				VideoEncoderUnsupportedCodecName
			);
		}
		LGL_VideoDecoder* vid = GetVideo();
		LGL_Image* image = vid ? vid->GetImage() : NULL;
		if
		(
			vid!=NULL &&
			image!=NULL &&
			image->GetFrameNumber()!=-1
		)
		{
			VideoEncoderBeginSignal=1;
		}
//VideoEncoderBeginSignal=1;

		if(VideoEncoderBeginSignal==0)
		{
			LGL_GetFont().DrawString
			(
				centerX,bottom+0.90f*height,.015f,
				1,1,1,1,
				true,.5f,
				"Loading Audio..."
			);
			int seconds=(int)Sound->GetSecondsUntilLoaded();
			if(seconds>=0 && seconds<9999.0f)
			{
				int minutes=0;
				while(seconds>=60)
				{
					minutes++;
					seconds-=60;
				}
				LGL_GetFont().DrawString
				(
					centerX,bottom+0.5f*height-0.5f*0.125f*height,0.125f*height,
					1,1,1,1,
					true,.5f,
					"%i:%.2i",
					minutes,
					seconds
				);
			}

			float percent=Sound->GetPercentLoadedSmooth();
			float pct=LGL_Clamp(0.0f,percent,1.0f);
			LGL_DrawRectToScreen
			(
				left,left+pct*width*0.5f,
				bottom,bottom+0.15f*height,
				(1.0f-pct)*coolR+(0.0f+pct)*warmR,
				(1.0f-pct)*coolG+(0.0f+pct)*warmG,
				(1.0f-pct)*coolB+(0.0f+pct)*warmB,
				1.0f
			);
			LGL_DrawRectToScreen
			(
				right,right-pct*width*0.5f,
				bottom,bottom+0.15f*height,
				(1.0f-pct)*coolR+(0.0f+pct)*warmR,
				(1.0f-pct)*coolG+(0.0f+pct)*warmG,
				(1.0f-pct)*coolB+(0.0f+pct)*warmB,
				1.0f
			);
			LGL_GetFont().DrawString
			(
				centerX+0.025f*width,bottom+0.025f*height,0.09f*height,
				1,1,1,1,
				true,0.5f,
				"%.0f%%",LGL_Clamp(0.0f,percent,0.99f)*100.0f
			);
		}
		else if
		(
			VideoEncoderPercent!=-1.0f ||
			VideoEncoderAudioOnly
		)
		{
			if(VideoEncoderAudioOnly==false)
			{
				LGL_ScopeLock lock(VideoEncoderSemaphore);
				if(VideoEncoder)
				{
					if(LGL_Image* img = VideoEncoder->GetImage())
					{
						img->DrawToScreen
						(
							left,
							right,
							bottom,
							top,
							0,
							0.25f,
							0.25f,
							0.25f,
							0.0f
						);
					}
				}
			}

			LGL_GetFont().DrawString
			(
				centerX,bottom+0.90f*height,.015f,
				1,1,1,1,
				true,.5f,
				"Caching %s...",
				VideoEncoderAudioOnly ? "Audio" : "Video"
			);

			int seconds=(int)VideoEncoderEtaSeconds;
			if(seconds>=0)
			{
				int minutes=0;
				while(seconds>=60)
				{
					minutes++;
					seconds-=60;
				}
				LGL_GetFont().DrawString
				(
					centerX,bottom+0.5f*height-0.5f*0.125f*height,0.125f*height,
					1,1,1,1,
					true,.5f,
					"%i:%.2i",
					minutes,
					seconds
				);
			}

			float pct=LGL_Clamp(0.0f,VideoEncoderPercent,1.0f);
			LGL_DrawRectToScreen
			(
				left,left+pct*width*0.5f,
				bottom,bottom+0.15f*height,
				(1.0f-pct)*coolR+(0.0f+pct)*warmR,
				(1.0f-pct)*coolG+(0.0f+pct)*warmG,
				(1.0f-pct)*coolB+(0.0f+pct)*warmB,
				1.0f
			);
			LGL_DrawRectToScreen
			(
				right,right-pct*width*0.5f,
				bottom,bottom+0.15f*height,
				(1.0f-pct)*coolR+(0.0f+pct)*warmR,
				(1.0f-pct)*coolG+(0.0f+pct)*warmG,
				(1.0f-pct)*coolB+(0.0f+pct)*warmB,
				1.0f
			);
			LGL_GetFont().DrawString
			(
				centerX+0.025f*width,bottom+0.025f*height,0.09f*height,
				1,1,1,1,
				true,0.5f,
				"%.0f%%",LGL_Clamp(0.0f,VideoEncoderPercent,0.99f)*100.0f
			);
		}
		else
		{
			if
			(
				LGL_MouseX()>=left &&
				LGL_MouseX()<=right &&
				LGL_MouseY()>=bottom &&
				LGL_MouseY()<=top
			)
			{
				if(LGL_MouseStroke(LGL_MOUSE_LEFT))
				{
					if(LGL_KeyDown(LGL_KEY_SHIFT)==false)
					{
						GetInputMouse().SetWaveformVideoAspectRatioNextNext();
					}
					else
					{
						GetInputMouse().SetWaveformVideoSelectNext();
					}
				}
			}

			Visualizer->DrawVideos
			(
				this,
				left,
				right,
				bottom,
				top,
				true
			);
			if(FreqSenseBrightness>0.0f)
			{
				float volAve;
				float volMax;
				float freqFactor;
				GetFreqMetaData(volAve,volMax,freqFactor);

				if
				(
					GetFreqBrightness(false,freqFactor,2*volAve)==0.0f &&
					GetFreqBrightness(true,freqFactor,2*volAve)==0.0f
				)
				{
					freqFactor=0.0f;
				}

				float r=
					(1.0f-freqFactor)*coolR+
					(0.0f+freqFactor)*warmR;
				float g=
					(1.0f-freqFactor)*coolG+
					(0.0f+freqFactor)*coolG;
				float b=
					(1.0f-freqFactor)*coolB+
					(0.0f+freqFactor)*coolB;

				LGL_DrawLineToScreen
				(
					left,bottom,
					right,bottom,
					r,g,b,1.0f,
					1.0f,
					true
				);
				LGL_DrawLineToScreen
				(
					right,bottom,
					right,top,
					r,g,b,1.0f,
					1.0f,
					true
				);
				LGL_DrawLineToScreen
				(
					right,top,
					left,top,
					r,g,b,1.0f,
					1.0f,
					true
				);
				LGL_DrawLineToScreen
				(
					left,top,
					left,bottom,
					r,g,b,1.0f,
					1.0f,
					true
				);
			}
		}
		rectAlpha=0.0f;
	}

	if(Focus)
	{
		float r=coolR;
		float g=coolG;
		float b=coolB;
		float localGlow=0.25f+0.75*glow;
		LGL_DrawRectToScreen
		(
			ViewportLeft,ViewportRight,
			ViewportBottom,ViewportTop,
			0.5f*r*localGlow,0.5f*g*localGlow,0.5f*b*localGlow,rectAlpha
		);
	}
	
	if(Mode==0)
	{
		//File Selection

		unsigned int fileNum=DatabaseFilteredEntries.size();
		const char* nameArray[5];
		bool isDirBits[5];
		bool loadableBits[5];
		bool alreadyPlayedBits[5];
		float bpm[5];
		for(unsigned int a=(unsigned int)FileTop;a<(unsigned int)FileTop+5 && a<fileNum;a++)
		{
			nameArray[a-FileTop]=DatabaseFilteredEntries[a]->PathShort;
			isDirBits[a-FileTop]=DatabaseFilteredEntries[a]->IsDir;
			loadableBits[a-FileTop]=DatabaseFilteredEntries[a]->Loadable;
			alreadyPlayedBits[a-FileTop]=DatabaseFilteredEntries[a]->AlreadyPlayed;
			bpm[a-FileTop]=DatabaseFilteredEntries[a]->BPM;
		}

		char drawDirPath[2048];
		if(strstr(DatabaseFilter.Dir,LGL_GetHomeDir()))
		{
#ifdef	LGL_OSX_BAKA
			sprintf(drawDirPath,"%s/",&(DatabaseFilter.Dir[strlen(LGL_GetHomeDir())+1]));
#else
			sprintf(drawDirPath,"~%s/",&(DatabaseFilter.Dir[strlen(LGL_GetHomeDir())]));
#endif
		}
		else
		{
			strcpy(drawDirPath,DatabaseFilter.Dir);
		}

		LGL_DrawLogWrite
		(
			//             01 02 03 04 05 06 07 08 09 10 11 12 13   14   15   16   17   18   19   20  21
			"DirTreeDraw|%.3f|%s|%s|%s|%s|%s|%s|%s|%i|%i|%i|%i|%i|%.4f|%.4f|%.3f|%.2f|%.2f|%.2f|%.2f|%.2f\n",
			LGL_SecondsSinceExecution()*Focus,		//01
			FilterText.GetString(),				//02
			drawDirPath,					//03
			(fileNum>0)?nameArray[0]:"",			//04
			(fileNum>1)?nameArray[1]:"",			//05
			(fileNum>2)?nameArray[2]:"",			//06
			(fileNum>3)?nameArray[3]:"",			//07
			(fileNum>4)?nameArray[4]:"",			//08
			(
				(isDirBits[0]<<0) +			//09
				(isDirBits[1]<<1) +
				(isDirBits[2]<<2) +
				(isDirBits[3]<<3) +
				(isDirBits[4]<<4)
			),
			(
				(loadableBits[0]<<0) +			//10
				(loadableBits[1]<<1) +
				(loadableBits[2]<<2) +
				(loadableBits[3]<<3) +
				(loadableBits[4]<<4)
			),
			(
				(alreadyPlayedBits[0]<<0) +		//11
				(alreadyPlayedBits[1]<<1) +
				(alreadyPlayedBits[2]<<2) +
				(alreadyPlayedBits[3]<<3) +
				(alreadyPlayedBits[4]<<4)
			),
			fileNum-FileTop,				//12
			FileSelectInt-FileTop,				//13
			ViewportBottom,					//14
			ViewportTop,					//15
			BadFileFlash,					//16
			bpm[0],						//17
			bpm[1],						//18
			bpm[2],						//19
			bpm[3],						//20
			bpm[4]						//21
		);
		LGL_DrawLogPause();
		Turntable_DrawDirTree
		(
			Which,
			LGL_SecondsSinceExecution()*Focus,
			FilterText.GetString(),
			drawDirPath,
			nameArray,
			isDirBits,
			loadableBits,
			alreadyPlayedBits,
			fileNum-FileTop,
			FileSelectInt-FileTop,
			ViewportBottom,
			ViewportTop,
			BadFileFlash,
			bpm
		);
		LGL_DrawLogPause(false);
	}
	if(Mode==1)
	{
		//Loading...
/*
		LGL_DrawRectToScreen
		(
			ViewportLeft,ViewportLeft+ViewportWidth*Sound->GetPercentLoadedSmooth(),
			ViewportBottom+ViewportHeight*.25,ViewportBottom+ViewportHeight*.5,
			0,0,.25+.25*glow,.5
		);
*/
		
		char SoundNameSafe[2048];
		strcpy(SoundNameSafe,SoundName);
		int SoundNameSafeLen=strlen(SoundNameSafe);
		for(int s=0;s<SoundNameSafeLen;s++)
		{
			if(SoundNameSafe[s]=='%')
			{
				SoundNameSafe[s]=' ';
			}
		}
		LGL_GetFont().DrawString
		(
			CenterX,ViewportBottom+ViewportHeight*.66,.015,
			1,1,1,1,
			true,.5,
			"Decoding '%s'...",
			SoundNameSafe
		);
		LGL_GetFont().DrawString
		(
			CenterX,ViewportBottom+ViewportHeight*.36,.015,
			1,1,1,1,
			true,.5,
			"%.2f",
			Sound->GetPercentLoadedSmooth()*100
		);

		float minutes=0;
		float seconds=ceil(Sound->GetSecondsUntilLoaded());
		while(seconds>=60)
		{
			minutes++;
			seconds-=60;
		}

		char temp[2048];
		if(seconds<10)
		{
			sprintf
			(
				temp,
				"%.0f:0%.0f",
				minutes,seconds
			);
		}
		else
		{
			sprintf
			(
				temp,
				"%.0f:%.0f",
				minutes,seconds
			);
		}
		LGL_GetFont().DrawString
		(
			CenterX,ViewportBottom+ViewportHeight*.10f,.015f,
			1,1,1,1,
			true,.5f,
			temp
		);
	}
	if(Mode==2)
	{
		if(LGL_AudioOutDisconnected())
		{
			LGL_GetFont().DrawString
			(
				CenterX,
				ViewportBottom+ViewportHeight*0.5f-ViewportHeight*0.2f*0.5f,
				ViewportHeight*0.2f,
				1,0,0,1,
				true,
				0.5f,
				"Audio Connection Lost"
			);

			LGL_GetFont().DrawString
			(
				CenterX,
				ViewportBottom+ViewportHeight*0.1f-ViewportHeight*0.1f*0.5f,
				ViewportHeight*0.1f,
				1,0,0,1,
				true,
				0.5f,
				"Audio will resume upon reconnecting external soundcard"
			);
		}
		else
		{
			unsigned int target =
				(Focus ? TARGET_FOCUS : 0) |
				((Which==0) ? TARGET_TOP : TARGET_BOTTOM);

			double currentSample=SmoothWaveformScrollingSample;

			unsigned int savePointBitfield=0;
			for(int a=0;a<18;a++)
			{
				savePointBitfield|=(SavePointSeconds[a]==-1.0f)?0:(1<<a);
			}

			float videoSecondsBufferedLeft=0.0f;
			float videoSecondsBufferedRight=0.0f;
			if(VideoFront)
			{
				videoSecondsBufferedLeft=VideoFront->GetSecondsBufferedLeft();
				videoSecondsBufferedRight=VideoFront->GetSecondsBufferedRight();
			}

			LGL_DrawLogWrite
			(
				"dttprehps|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f\n",
				SavePointSeconds[0],
				SavePointSeconds[1],
				SavePointSeconds[2],
				SavePointSeconds[3],
				SavePointSeconds[4],
				SavePointSeconds[5],
				SavePointSeconds[6],
				SavePointSeconds[7],
				SavePointSeconds[8],
				SavePointSeconds[9],
				SavePointSeconds[10],
				SavePointSeconds[11],
				SavePointSeconds[12],
				SavePointSeconds[13],
				SavePointSeconds[14],
				SavePointSeconds[15],
				SavePointSeconds[16],
				SavePointSeconds[17]
			);
			LGL_DrawLogWrite
			(
				"dttpreflash|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f\n",
				SavePointUnsetFlashPercent[0],
				SavePointUnsetFlashPercent[1],
				SavePointUnsetFlashPercent[2],
				SavePointUnsetFlashPercent[3],
				SavePointUnsetFlashPercent[4],
				SavePointUnsetFlashPercent[5],
				SavePointUnsetFlashPercent[6],
				SavePointUnsetFlashPercent[7],
				SavePointUnsetFlashPercent[8],
				SavePointUnsetFlashPercent[9],
				SavePointUnsetFlashPercent[10],
				SavePointUnsetFlashPercent[11],
				SavePointUnsetFlashPercent[12],
				SavePointUnsetFlashPercent[13],
				SavePointUnsetFlashPercent[14],
				SavePointUnsetFlashPercent[15],
				SavePointUnsetFlashPercent[16],
				SavePointUnsetFlashPercent[17]
			);
			LGL_DrawLogWrite
			(
				"dttprenoise|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f\n",
				SavePointUnsetNoisePercent[0],
				SavePointUnsetNoisePercent[1],
				SavePointUnsetNoisePercent[2],
				SavePointUnsetNoisePercent[3],
				SavePointUnsetNoisePercent[4],
				SavePointUnsetNoisePercent[5],
				SavePointUnsetNoisePercent[6],
				SavePointUnsetNoisePercent[7],
				SavePointUnsetNoisePercent[8],
				SavePointUnsetNoisePercent[9],
				SavePointUnsetNoisePercent[10],
				SavePointUnsetNoisePercent[11],
				SavePointUnsetNoisePercent[12],
				SavePointUnsetNoisePercent[13],
				SavePointUnsetNoisePercent[14],
				SavePointUnsetNoisePercent[15],
				SavePointUnsetNoisePercent[16],
				SavePointUnsetNoisePercent[17]
			);
			LGL_VideoDecoder* dec = GetVideo();
			LGL_DrawLogWrite
			(
				//   01 02 03 04   05   06   07   08   09   10   11   12   13   14   15   16   17   18   19 20   21   22   23   24 25 26 27   28   29   30   31   32   33 34 35   36   37 38   39 40 41   42   43 44 45  46   47   48   49
				"dtt|%i|%c|%s|%c|%.0f|%.0f|%.0f|%.0f|%.5f|%.5f|%.3f|%.0f|%.3f|%.3f|%.4f|%.4f|%.4f|%.4f|%.3f|%.4f|%c|%.3f|%.2f|%.3f|%i|%i|%i|%.2f|%.2f|%.2f|%.3f|%.3f|%.3f|%c|%c|%.3f|%.3f|%i|%.3f|%c|%s|%.2f|%.2f|%c|%c|%.3f|%.3f|%.3f|%.3f\n",
				Which,							//01
				Sound->IsLoaded() ? 'T' : 'F',				//02
				dec ? dec->GetPathShort() : NULL,			//03
				(GlitchDuo || LuminScratch || GlitchPure) ? 'T' : 'F',	//04
				GlitchBegin,						//05
				GlitchLength,						//06
				currentSample,						//07
				(double)Sound->GetLengthSamples(),			//08
				Sound->GetSpeed(Channel),				//09
				Pitchbend,						//10
				GrainStreamCrossfader,					//11
				GrainStreamSourcePoint,					//12
				GrainStreamLength,					//13
				GrainStreamPitch,					//14
				ViewportLeft,						//15
				ViewportRight,						//16
				ViewportBottom,						//17
				ViewportTop,						//18
				VolumeMultiplierNow,					//19
				CenterX,						//20
				PauseMultiplier ? 'T' : 'F',				//21
				Nudge,							//22
				0.0f,							//23
				LGL_SecondsSinceExecution(),				//24
				SavePointIndex,						//25
				SavePointIndex,						//26
				savePointBitfield,					//27
				GetBPM(),						//28
				GetBPMAdjusted(),					//29
				GetBPMFirstBeatSeconds(),				//30
				EQFinal[0],						//31
				EQFinal[1],						//32
				EQFinal[2],						//33
				LowRez ? 'T' : 'F',					//34
				AudioInputMode ? 'T' : 'F',				//35
				Looping() ? LoopAlphaSeconds : -1.0f,			//36
				Sound->GetWarpPointSecondsAlpha(Channel),		//37
				QuantizePeriodMeasuresExponent,				//38
				QuantizePeriodNoBPMSeconds,				//39
				Input.WaveformRecordHold(target) ? 'T' : 'F',		//40
				SoundName,						//41
				videoSecondsBufferedLeft,				//42
				videoSecondsBufferedRight,				//43
				(Which==Master) ? 'T' : 'F',				//44
				(RapidVolumeInvert) ? 'T' : 'F',			//45
				GetBeginningOfCurrentMeasureSeconds(),			//46
				VideoBrightness,					//47
				OscilloscopeBrightness,					//48
				FreqSenseBrightness					//49
			);
			
			bool waveArrayFilledBefore=(EntireWaveArrayFillIndex==ENTIRE_WAVE_ARRAY_COUNT);

			LGL_DrawLogPause();
			Turntable_DrawWaveform
			(
				Sound,							//01
				Sound->IsLoaded(),					//02
				dec ? dec->GetPathShort() : NULL,			//03
				GlitchDuo || LuminScratch || GlitchPure,		//04
				GlitchBegin,						//05
				GlitchLength,						//06
				currentSample,						//07
				Sound->GetLengthSamples(),				//08
				Sound->GetSpeed(Channel),				//09
				Pitchbend,						//10
				GrainStreamCrossfader,					//11
				GrainStreamSourcePoint,					//12
				GrainStreamLength,					//13
				GrainStreamPitch,					//14
				ViewportLeft,						//15
				ViewportRight,						//16
				ViewportBottom,						//17
				ViewportTop,						//18
				VolumeMultiplierNow,					//19
				CenterX,						//20
				PauseMultiplier,					//21
				Nudge,							//22
				0.0f,							//23
				LGL_SecondsSinceExecution(),				//24
				SavePointSeconds,					//25
				SavePointIndex,						//26
				SavePointIndex,						//27
				savePointBitfield,					//28
				SavePointUnsetNoisePercent,				//29
				SavePointUnsetFlashPercent,				//30
				GetBPM(),						//31
				GetBPMAdjusted(),					//32
				GetBPMFirstBeatSeconds(),				//33
				EQFinal[0],						//34
				EQFinal[1],						//35
				EQFinal[2],						//36
				LowRez,							//37
				EntireWaveArrayFillIndex,				//38
				ENTIRE_WAVE_ARRAY_COUNT,				//39
				EntireWaveArrayMagnitudeAve,				//40
				EntireWaveArrayMagnitudeMax,				//41
				EntireWaveArrayFreqFactor,				//42
				CachedLengthSeconds,					//43
				NoiseImage[rand()%NOISE_IMAGE_COUNT_256_64],		//44
				LoopImage,						//45
				AudioInputMode,						//46
				Looping() ? LoopAlphaSeconds : -1.0f,			//47
				Sound->GetWarpPointSecondsAlpha(Channel),		//48
				QuantizePeriodMeasuresExponent,				//49
				QuantizePeriodNoBPMSeconds,				//50
				Input.WaveformRecordHold(target),			//51
				SoundName,						//52
				videoSecondsBufferedLeft,				//53
				videoSecondsBufferedRight,				//54
				Which==Master,						//55
				RapidVolumeInvert,					//56
				GetBeginningOfCurrentMeasureSeconds(),			//57
				VideoBrightness,					//58
				OscilloscopeBrightness,					//59
				FreqSenseBrightness,					//60
				Channel							//61
			);
			LGL_DrawLogPause(false);
		
			bool waveArrayFilledAfter=(EntireWaveArrayFillIndex==ENTIRE_WAVE_ARRAY_COUNT);

			if(waveArrayFilledBefore!=waveArrayFilledAfter)
			{
				//We just finished filled our wave array data!
				//Save it so we can instantly load it next time.
				SaveAllCachedData();
			}
		}
	}

	if(NoiseFactor>0.0f)
	{
		int which = LGL_RandInt(0,NOISE_IMAGE_COUNT_256_64-1);
		float factor = NoiseFactor;
		NoiseImage[which]->DrawToScreen
		(
			ViewportLeft,
			ViewportRight,
			ViewportBottom,
			ViewportTop,
			0,
			factor,
			factor,
			factor,
			factor
		);
	}
	if(WhiteFactor>0.0f)
	{
		float factor = WhiteFactor;
		LGL_DrawRectToScreen
		(
			ViewportLeft,
			ViewportRight,
			ViewportBottom,
			ViewportTop,
			factor,
			factor,
			factor,
			0.0f
		);
	}

	LGL_ClipRectDisable();
}

void
TurntableObj::
DrawVU
(
	float	left,
	float	right,
	float	bottom,
	float	top,
	float	glow
)
{
	if
	(
		Sound==NULL ||
		Channel<0
	)
	{
		return;
	}

	float vu=Sound->GetVU(Channel);
	LGL_DrawRectToScreen
	(
		left,right,
		bottom,
		bottom+vu*(top-bottom),
		.5f,.25f,1.0f,1.0f
	);
}

void
TurntableObj::
DrawWave
(
	float	left,
	float	right,
	float	bottom,
	float	top,
	float	brightness,
	bool	preview
)
{
	float w = right-left;
	float h = top-bottom;
	float midX = 0.5f*(left+right);
	float midY = 0.5f*(bottom+top);

	int projDisplay = LGL_Max(0,LGL_DisplayCount()-1);
	int projW;
	int projH;
	if(LGL_DisplayCount()==1)
	{
		projW = Visualizer->GetViewportVisualsWidth() * LGL_DisplayResolutionX();
		projH = Visualizer->GetViewportVisualsHeight() * LGL_DisplayResolutionY();
	}
	else
	{
		projW = LGL_DisplayResolutionX(projDisplay);
		projH = LGL_DisplayResolutionY(projDisplay);
	}
	float projAR = projW/(float)projH;
	float targetAR = w*LGL_DisplayResolutionX()/(float)(h*LGL_DisplayResolutionY());

	float myL = left;
	float myR = right;
	float myB = bottom;
	float myT = top;
	
	myL = midX - 0.5f * w * (projAR/targetAR);
	myR = midX + 0.5f * w * (projAR/targetAR);

	//Make sure we're not too wide
	float targetLimitL = midX - 0.5f * w * (projAR/targetAR);
	float targetLimitR = midX + 0.5f * w * (projAR/targetAR);
	if(myL<targetLimitL)
	{
		float scaleFactor = (midX-targetLimitL)/(midX-myL);
		myB = midY - 0.5f * h * scaleFactor;
		myT = midY + 0.5f * h * scaleFactor;
		myL = targetLimitL;
		myR = targetLimitR;
	}

	//Make sure we're not too tall
	float targetLimitB = midY - 0.5f * h * (targetAR/projAR);
	float targetLimitT = midY + 0.5f * h * (targetAR/projAR);
	if(myB<targetLimitB)
	{
		float scaleFactor = (midY-targetLimitB)/(midY-myB);
		myL = midX - (midX-myL) * scaleFactor;
		myR = midX + (myR-midX) * scaleFactor;
		myB = targetLimitB;
		myT = targetLimitT;
	}

	left = myL;
	right = myR;
	bottom = myB;
	top = myT;

	if(Sound==NULL)
	{
		return;
	}

	float speedHeightScalar = LGL_Clamp(0.0f,FinalSpeed*100.0f,1.0f);
	if(speedHeightScalar == 0.0f)
	{
		return;
	}

	float midHeight = 0.5f*(top+bottom);
	bottom =
		(1.0f-speedHeightScalar)*midHeight +
		(0.0f+speedHeightScalar)*bottom;
	top =	(1.0f-speedHeightScalar)*midHeight +
		(0.0f+speedHeightScalar)*top;

	LGL_AudioGrain grain;
	grain.SetWaveformFromLGLSound
	(
		Sound,
		GetTimeSeconds(),
		LGL_Min(FinalSpeed,1.0f)/240.0f
	);

	float coolR;
	float coolG;
	float coolB;
	GetColorCool(coolR,coolG,coolB);

	float warmR;
	float warmG;
	float warmB;
	GetColorWarm(warmR,warmG,warmB);

	float volAve;
	float volMax;
	float freqFactor;
	grain.GetMetadata(volAve,volMax,freqFactor);

	float red=
		(1.0f-freqFactor)*coolR+
		(0.0f+freqFactor)*warmR;
	float green=
		(1.0f-freqFactor)*coolG+
		(0.0f+freqFactor)*warmG;
	float blue=
		(1.0f-freqFactor)*coolB+
		(0.0f+freqFactor)*warmB;

	//Hot...
	red = red * powf(1.0f+freqFactor,2.0f);
	green = green * powf(1.0f+freqFactor,2.0f);
	blue = blue * powf(1.0f+freqFactor,2.0f);

	if(volMax>=0.99f)
	{
		red=1.0f;
		green=0.0f;
		blue=0.0f;
	}

	if
	(
		GetFreqBrightness(false,freqFactor,2*volAve*1.0f) ||
		GetFreqBrightness(true,freqFactor,2*volAve*1.0f)
	)
	{
		float thickness = preview ? 1.0f : 3.0f;
		thickness*=(1.0f+freqFactor*6.0f*volAve*4.0f);

		float height = top-bottom;
		top+=0.5f*height*(freqFactor-0.5f);
		bottom+=0.5f*height*(freqFactor-0.5f);

		grain.DrawWaveform
		(
			left,
			right,
			bottom,
			top,
			red*brightness,green*brightness,blue*brightness,brightness,
			thickness,
			true
		);
	}
}

void
TurntableObj::
DrawSpectrum
(
	float	left,
	float	right,
	float	bottom,
	float	top,
	float	glow
)
{
	//
}

void
TurntableObj::
DrawWholeTrack
(
	float	left,
	float	right,
	float	bottom,
	float	top,
	float	glow
)
{
	//
}

void
TurntableObj::
SetViewport
(
	float	left,	float	right,
	float	bottom,	float	top
)
{
	ViewportLeft=left;
	ViewportRight=right;
	ViewportBottom=bottom;
	ViewportTop=top;
	ViewportWidth=right-left;
	ViewportHeight=top-bottom;

	Left=left;
	Right=right;
	Bottom=bottom;
	Top=top;
	Width=right-left;
	Height=top-bottom;
	CenterX=Left+.5*Width;
	CenterY=Bottom+.5*Height;
}

void
TurntableObj::
SetFocus
(
	bool	inFocus
)
{
	Focus=inFocus;
}

bool
TurntableObj::
GetFocus() const
{
	return(Focus);
}

void
TurntableObj::
SetTurntableNumber
(
	int	num
)
{
	Which=num;
}

void
TurntableObj::
SetVisualizer
(
	VisualizerObj* viz
)
{
	Visualizer=viz;
}

float
TurntableObj::
GetVisualBrightnessPreview()
{
	return(1.0f);
}

float
TurntableObj::
GetVisualBrightnessFinal()
{
	if(MixerVideoMute)
	{
		return(0.0f);
	}

	bool muted=false;
	if(GetBPM()>0)
	{
		if(RapidVolumeInvert)
		{
			long sampleNow = SmoothWaveformScrollingSample;
			long bpmFirstBeatCurrentMeasureSamples = GetBeginningOfCurrentMeasureSeconds()*Sound->GetHz();
			double secondsPerLoopPeriod = GetQuantizePeriodSeconds();
			long samplesPerLoopPeriod = secondsPerLoopPeriod*Sound->GetHz();
			muted=(((long)fabs(sampleNow-bpmFirstBeatCurrentMeasureSamples)/samplesPerLoopPeriod)%2)==1;
			if(sampleNow<bpmFirstBeatCurrentMeasureSamples)
			{
				muted=!muted;
			}
		}
	}
	if(Sound)
	{
		if(Sound->GetRespondToRapidSoloInvertCurrentValue(Channel)==0)
		{
			muted=true;
		}
	}
	float crossfadeFactorFront = GetMixerCrossfadeFactorFront();
	float crossfadeFactor;
	if(crossfadeFactorFront < 0.5f)
	{
		float factor = crossfadeFactorFront*2.0f;
		crossfadeFactor =
			(0.0f + factor) * GetVisualBrightnessAtCenter() +
			(1.0f - factor) * 0.0f;
	}
	else if(crossfadeFactorFront == 0.5f)
	{
		crossfadeFactor = GetVisualBrightnessAtCenter();
	}
	else //crossfadeFactorFront >= 0.5f
	{
		float factor = (crossfadeFactorFront-0.5f)*2.0f;
		crossfadeFactor =
			(0.0f + factor) * 1.0f +
			(1.0f - factor) * GetVisualBrightnessAtCenter();
	}
	return
	(
		GetMixerVolumeFront()*
		crossfadeFactor*
		(muted ? 0 : 1)
	);
}

float
TurntableObj::
GetVideoBrightnessPreview()
{
	return
	(
		GetVisualBrightnessPreview()*
		VideoBrightness
	);
}

float
TurntableObj::
GetVideoBrightnessFinal()
{
	return
	(
		GetVisualBrightnessFinal()*
		VideoBrightness
	);
}

float
TurntableObj::
GetOscilloscopeBrightnessPreview()
{
	return
	(
		GetVisualBrightnessPreview()*
		OscilloscopeBrightness
	);
}

float
TurntableObj::
GetOscilloscopeBrightnessFinal()
{
	return
	(
		GetVisualBrightnessFinal()*
		OscilloscopeBrightness
	);
}

float
TurntableObj::
GetFreqSenseBrightnessPreview()
{
	return
	(
		GetVisualBrightnessPreview()*
		FreqSenseBrightness
	);
}

float
TurntableObj::
GetFreqSenseBrightnessFinal()
{
	return
	(
		GetVisualBrightnessFinal()*
		FreqSenseBrightness
	);
}

void
TurntableObj::
SetMixerVolumeFront
(
	float	scalar
)
{
	MixerVolumeFront=LGL_Clamp(0,scalar,1);
}

void
TurntableObj::
SetMixerVolumeBack
(
	float	scalar
)
{
	MixerVolumeBack=LGL_Clamp(0,scalar,1);
}

void
TurntableObj::
SetMixerCrossfadeFactorFront
(
	float	factor
)
{
	MixerCrossfadeFactorFront=LGL_Clamp(0,factor,1);
}

void
TurntableObj::
SetMixerCrossfadeFactorBack
(
	float	factor
)
{
	MixerCrossfadeFactorBack=LGL_Clamp(0,factor,1);
}

float
TurntableObj::
GetMixerVolumeFront()
{
	if(VolumeKill)
	{
		return(0.0f);
	}

	if(VolumeInvertBinary)
	{
		if(MixerVolumeFront==0.0f)
		{
			return(1.0f);
		}
		else
		{
			return(0.0f);
		}
	}
	else
	{
		return(LGL_Clamp(0,MixerVolumeFront,1));
	}
}

float
TurntableObj::
GetMixerVolumeBack()
{
	return(LGL_Clamp(0,MixerVolumeBack,1));
}

float
TurntableObj::
GetMixerCrossfadeFactorFront()
{
	return(LGL_Clamp(0,MixerCrossfadeFactorFront,1));
}

float
TurntableObj::
GetMixerCrossfadeFactorBack()
{
	return(LGL_Clamp(0,MixerCrossfadeFactorBack,1));
}

float
TurntableObj::
GetGain()
{
	return(VolumeMultiplierNow*(VolumeKill?0.0f:1.0f));
}

void
TurntableObj::
SetMixerEQ
(
	float	low,
	float	mid,
	float	high
)
{
	if
	(
		MixerEQ[0]!=low ||
		MixerEQ[1]!=mid ||
		MixerEQ[2]!=high
	)
	{
		MixerEQ[0]=low;
		MixerEQ[1]=mid;
		MixerEQ[2]=high;

		if(Mode==2)
		{
			UpdateSoundFreqResponse();
		}
	}
}

void
TurntableObj::
SetMixerNudge
(
	float	mixerNudge
)
{
	MixerNudge=mixerNudge;
}

bool
TurntableObj::
GetMixerVideoMute()
{
	return(MixerVideoMute);
}

void
TurntableObj::
SetMixerVideoMute
(
	bool	mixerVideoMute
)
{
	MixerVideoMute=mixerVideoMute;
}

std::vector<char*>
TurntableObj::
GetTrackListFileUpdates()
{
	std::vector<char*> ret=TrackListFileUpdates;
	for(unsigned int a=0;a<TrackListFileUpdates.size();a++)
	{
		delete TrackListFileUpdates[a];
		TrackListFileUpdates[a]=NULL;
	}
	TrackListFileUpdates.clear();
	return(ret);
}

LGL_VideoDecoder*
TurntableObj::
GetVideo()
{
	if(Sound==NULL)
	{
		return(NULL);
	}

	if
	(
		VideoEncoderPercent==-1.0f ||
		VideoEncoderAudioOnly
	)
	{
		if(VideoFront!=NULL)
		{
			return(VideoFront);
		}
		else
		{
			return(VideoBack);
		}
	}

	return(NULL);
}

LGL_VideoDecoder*
TurntableObj::
GetVideoFront()
{
	return(VideoFront);
}

LGL_VideoDecoder*
TurntableObj::
GetVideoBack()
{
	return(VideoBack);
}

LGL_VideoDecoder*
TurntableObj::
GetVideoLo()
{
	return(VideoLo);
}

LGL_VideoDecoder*
TurntableObj::
GetVideoHi()
{
	return(VideoHi);
}

float
TurntableObj::
GetVideoTimeSeconds()
{
	if(GlitchPure)
	{
		float arbitraryPercent=(GlitchPureVideoNow-(GlitchBegin/44100.0f))/((GlitchLength/44100.0f)*GLITCH_PURE_VIDEO_MULTIPLIER);
		return((GlitchBegin/44100.0f)+arbitraryPercent*(GlitchLength/44100.0f)*GLITCH_PURE_VIDEO_MULTIPLIER);
	}
	else
	{
		if(VideoFileExists)
		{
			if
			(
				Looping() &&
				LoopOmegaSeconds - LoopAlphaSeconds < 0.1f
			)
			{
				float fps = 1.0f;
				if(LGL_VideoDecoder* dec = GetVideo())
				{
					fps = dec->GetFPS();
				}
				float delta = (LGL_FramesSinceExecution()%4)/fps;
				return
				(
					LoopAlphaSeconds + 
					delta
				);
			}
			else
			{
				return(GetTimeSeconds());
			}
		}
		else
		{
			return(GetTimeSeconds()*VideoAdvanceRate+VideoOffsetSeconds);
		}
	}
}

bool
TurntableObj::
GetVideoSolo()
{
	float volFront = VolumeInvertBinary ?
		(MixerVolumeFront==0.0f ? 1.0f : 0.0f) :
		MixerVolumeFront;
	volFront*=(VolumeKill?0.0f:1.0f);
	return(volFront>0.0f);
}

float
TurntableObj::
GetTimeSeconds()
{
	if(Sound==NULL || Mode!=2)
	{
		return(0);
	}
	else
	{
		if(LuminScratch)
		{
			return(LuminScratchSamplePositionDesired / 44100.0f);
		}
		else
		{
			return(SmoothWaveformScrollingSample/Sound->GetHz());
			//return(Sound->GetPositionSeconds(Channel));
		}
	}
}

bool
TurntableObj::
GetFreqMetaData
(
	float&	volAve,
	float&	volMax,
	float&	freqFactor
)
{
	bool ret=false;
	
	if(AudioInputMode)
	{
		ret=LGL_AudioInMetadata(volAve,volMax,freqFactor);
	}
	else
	{
		if(Sound)
		{
			ret=
				Sound->GetMetadata
				(
					GetTimeSeconds(),
					GetTimeSeconds()+1.0f/60.0f,
					freqFactor,
					volAve,
					volMax
				);
		}
		else
		{
			volAve=0.0f;
			volMax=0.0f;
			freqFactor=0.0f;
			ret=false;
		}
	}

	return(ret);
}

float
TurntableObj::
GetVolumePeak()
{
	if(Sound)
	{
		return(Sound->GetVolumePeak());
	}
	else
	{
		return(1.0f);
	}
}

bool
TurntableObj::
GetSavePointIndexAtNull() const
{
	return(SavePointIndex==-1);
}

void
TurntableObj::
LoadMetaData()
{
	if(Sound==NULL)
	{
		return;
	}

	char metaDataPath[2048];
	GetMetaDataPath(metaDataPath);
	FILE* fd=fopen(metaDataPath,"r");
	if(fd)
	{
		const int dataLen=2048;
		char data[dataLen];
		fgets(data,dataLen,fd);
		fclose(fd);
		LoadMetaData(data);
	}
}

void
TurntableObj::
LoadMetaData(const char* data)
{
	if(Sound==NULL)
	{
		return;
	}

	FileInterfaceObj fi;
	fi.ReadLine(data);
	if(fi.Size()==0)
	{
		return;
	}
	if
	(
		strcasecmp(fi[0],"HomePoints")==0 ||
		strcasecmp(fi[0],"SavePoints")==0
	)
	{
		if(fi.Size()!=19)
		{
			printf("TurntableObj::LoadMetaData('%s'): Warning!\n",SoundName);
			printf("\tSavePoints has strange fi.size() of '%i' (Expecting 11)\n",fi.Size());
		}
		for(unsigned int a=0;a<fi.Size()-1 && a<18;a++)
		{
			SavePointSeconds[a]=atof(fi[a+1]);
		}
	}
}

void
TurntableObj::
SaveMetaData()
{
	if(Sound==NULL)
	{
		return;
	}

	if(DatabaseEntryNow)
	{
		DatabaseEntryNow->BPM = GetBPM();
	}

	char metaDataPath[2048];
	GetMetaDataPath(metaDataPath);

	char data[4096];
	sprintf
	(
		data,
		"SavePoints|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f\n",
		SavePointSeconds[0],
		SavePointSeconds[1],
		SavePointSeconds[2],
		SavePointSeconds[3],
		SavePointSeconds[4],
		SavePointSeconds[5],
		SavePointSeconds[6],
		SavePointSeconds[7],
		SavePointSeconds[8],
		SavePointSeconds[9],
		SavePointSeconds[10],
		SavePointSeconds[11],
		SavePointSeconds[12],
		SavePointSeconds[13],
		SavePointSeconds[14],
		SavePointSeconds[15],
		SavePointSeconds[16],
		SavePointSeconds[17]
	);

	LGL_WriteFileAsync(metaDataPath,data,strlen(data));

	if(MetaDataSavedThisFrame)
	{
		delete MetaDataSavedThisFrame;
		MetaDataSavedThisFrame=NULL;
	}
	MetaDataSavedThisFrame = new char[strlen(data)+2];
	strcpy(MetaDataSavedThisFrame,data);
}

const char*
TurntableObj::
GetMetaDataSavedThisFrame()	const
{
	//Allow MetaData to propagate to other turntables
	return(MetaDataSavedThisFrame);
}

void
TurntableObj::
LoadAllCachedData()
{
	//Initialize to default values
	EntireWaveArrayFillIndex=0;
	for(int a=0;a<ENTIRE_WAVE_ARRAY_COUNT_MAX;a++)
	{
		EntireWaveArrayMagnitudeAve[a]=0.0f;
		EntireWaveArrayMagnitudeMax[a]=0.0f;
		EntireWaveArrayFreqFactor[a]=0.0f;
	}
	CachedLengthSeconds=0.0f;
	CachedVolumePeak=0.0f;

	//Load data if possible
	LoadWaveArrayData();
	LoadCachedMetadata();
}

void
TurntableObj::
SaveAllCachedData()
{
	SaveWaveArrayData();
	SaveCachedMetadata();
}

void
TurntableObj::
LoadWaveArrayData()
{
	char waveArrayDataPath[1024];
	sprintf(waveArrayDataPath,"%s/.dvj/cache/waveArrayData/%s.dvj-wavearraydata-%i.bin",LGL_GetHomeDir(),SoundName,ENTIRE_WAVE_ARRAY_COUNT);

	if(LGL_FileExists(waveArrayDataPath)==false)
	{
		return;
	}

	if(LGL_FirstFileMoreRecentlyModified(SoundSrcPath,waveArrayDataPath))
	{
		LGL_FileDelete(waveArrayDataPath);
		return;
	}

	int expectedSize=sizeof(float)*ENTIRE_WAVE_ARRAY_COUNT*3;
	if
	(
		LGL_FileExists(waveArrayDataPath) &&
		expectedSize!=LGL_FileLengthBytes(waveArrayDataPath)
	)
	{
		//Bad data... Whatever.
		LGL_FileDelete(waveArrayDataPath);
		return;
	}

	if(FILE* fd=fopen(waveArrayDataPath,"rb"))
	{
		fread(EntireWaveArrayMagnitudeAve,sizeof(float),ENTIRE_WAVE_ARRAY_COUNT,fd);
		fread(EntireWaveArrayMagnitudeMax,sizeof(float),ENTIRE_WAVE_ARRAY_COUNT,fd);
		fread(EntireWaveArrayFreqFactor,sizeof(float),ENTIRE_WAVE_ARRAY_COUNT,fd);
		EntireWaveArrayFillIndex=ENTIRE_WAVE_ARRAY_COUNT;
		fclose(fd);
	}
}

void
TurntableObj::
SaveWaveArrayData()
{
	char waveArrayDataPath[1024];
	sprintf(waveArrayDataPath,"%s/.dvj/cache/waveArrayData/%s.dvj-wavearraydata-%i.bin",LGL_GetHomeDir(),SoundName,ENTIRE_WAVE_ARRAY_COUNT);
	FILE* fd=fopen(waveArrayDataPath,"wb");
	if(fd)
	{
		fwrite(EntireWaveArrayMagnitudeAve,sizeof(float),ENTIRE_WAVE_ARRAY_COUNT,fd);
		fwrite(EntireWaveArrayMagnitudeMax,sizeof(float),ENTIRE_WAVE_ARRAY_COUNT,fd);
		fwrite(EntireWaveArrayFreqFactor,sizeof(float),ENTIRE_WAVE_ARRAY_COUNT,fd);
		EntireWaveArrayFillIndex=ENTIRE_WAVE_ARRAY_COUNT;
		fclose(fd);
	}
}

void
TurntableObj::
GetMetaDataPath
(
	char*	dst
)
{
	sprintf(dst,"%s/.dvj/metadata/%s.dvj-metadata.txt",LGL_GetHomeDir(),SoundName);
}

void
TurntableObj::
GetCacheMetaDataPath
(
	char*	dst
)
{
	sprintf(dst,"%s/.dvj/cache/metadata/%s.dvj-metadata.txt",LGL_GetHomeDir(),SoundName);
}

void
TurntableObj::
LoadCachedMetadata()
{
	char cachedPath[2048];
	GetCacheMetaDataPath(cachedPath);

	if(LGL_FileExists(cachedPath)==false)
	{
		return;
	}

	if(LGL_FirstFileMoreRecentlyModified(SoundSrcPath,cachedPath))
	{
		LGL_FileDelete(cachedPath);
	}
	else
	{
		FILE* fd=fopen(cachedPath,"r");
		if(fd)
		{
			char buf[1024];
			fgets(buf,1024,fd);
			CachedLengthSeconds=atof(buf);
			fgets(buf,1024,fd);
			CachedVolumePeak=atof(buf);
			fclose(fd);
		}
	}
}

void
TurntableObj::
SaveCachedMetadata()
{
	if
	(
		Sound==NULL ||
		Sound->IsLoaded()==false
	)
	{
		return;
	}

	CachedLengthSeconds=Sound->GetLengthSeconds();

	char cachedLengthPath[2048];
	GetCacheMetaDataPath(cachedLengthPath);
	FILE* fd=fopen(cachedLengthPath,"w");
	if(fd)
	{
		fprintf(fd,"%.3f\n",CachedLengthSeconds);
		fprintf(fd,"%f\n",Sound->GetVolumePeak());
		fclose(fd);
	}
}

const
char*
TurntableObj::
GetSoundPath()
{
	if(Sound==NULL)
	{
		return(NULL);
	}
	else
	{
		return(SoundSrcPath);
	}
}

const
char*
TurntableObj::
GetSoundPathShort()
{
	if(Sound==NULL)
	{
		return(NULL);
	}
	else
	{
		return(SoundName);
	}
}

bool
TurntableObj::
GetPaused()
{
	if(Sound==NULL)
	{
		return(true);
	}
	else
	{
		return(PauseMultiplier==0);
	}
}

float
TurntableObj::
GetFinalSpeed()
{
	if(Sound==NULL)
	{
		return(0.0f);
	}
	else
	{
		return(FinalSpeed);
	}
}

bool
TurntableObj::
GetRecordScratch()
{
	return(RecordScratch || LuminScratch);
}

bool
TurntableObj::
GetSoundLoaded()
{
	return(Sound!=NULL);
}

bool
TurntableObj::
GetSoundLoadedFully()
{
	return
	(
		Sound!=NULL &&
		Sound->IsLoaded()
	);
}

int
TurntableObj::
GetMode()
{
	return(Mode);
}

void
TurntableObj::
SetLowRez
(
	bool	lowRez
)
{
	LowRez=lowRez;
}

void
TurntableObj::
ProcessHintFile
(
	char*	path
)
{
	ImageSetPrefix[0]='\0';
	MovieClipPrefix[0]='\0';
	
	char HintPath[512];
	sprintf(HintPath,"%s",path);
	FILE* HintFile=fopen(HintPath,"r");
	if(HintFile)
	{
		char tempstring[256];
		while(!feof(HintFile))
		{
			fgets(tempstring,255,HintFile);
			if(tempstring[0]=='$')
			{
				if(strchr(tempstring,'='))
				{
					char *tempstring2=strchr(tempstring,'=');
					char value[256];
					strcpy(value,&(tempstring2[1]));
					char argument[256];
					tempstring2[0]='\0';
					strcpy(argument,tempstring);
					if(value[strlen(value)-1]=='\n')
					{
						value[strlen(value)-1]='\0';
					}
					//int valueint=atoi(value);
					//float valuefloat=atof(value);

					if(strcmp(argument,"$ImageSetPrefix")==0)
					{
						sprintf(ImageSetPrefix,"%s",value);
					}
					if(strcmp(argument,"$MovieClipPrefix")==0)
					{
						sprintf(MovieClipPrefix,"%s",value);
					}
				}
			}
		}
		fclose(HintFile);
	}
}

bool
TurntableObj::
Looping()
{
	return(LoopActive || LoopThenRecallActive);
}

void
TurntableObj::
SetRecallOrigin()
{
	if(Sound==NULL)
	{
		ClearRecallOrigin();
		return;
	}

	RecallOrigin=Sound->GetPositionSeconds(Channel);
	Sound->SetDivergeRecallBegin(Channel,Pitchbend);
}

void
TurntableObj::
ClearRecallOrigin()
{
	if(Channel>=0 && Sound!=NULL) Sound->SetDivergeRecallOff(Channel);
	RecallOrigin=-1.0f;
}

bool
TurntableObj::
RecallIsSet()
{
	return(Sound!=NULL && RecallOrigin>=0.0f);
}

void
TurntableObj::
Recall()
{
	if(RecallIsSet()==false)
	{
		return;
	}

	//Change our speed instantly.
	Sound->SetSpeed(Channel,Pitchbend*PauseMultiplier,true);
	Sound->SetDivergeRecallEnd(Channel);

	SmoothWaveformScrollingSample=Sound->GetPositionSamples(Channel);
	
	ClearRecallOrigin();
}

double
TurntableObj::GetQuantizePeriodSeconds()
{
	return(GetMeasureLengthSeconds()*pow(2,QuantizePeriodMeasuresExponent));
}

double
TurntableObj::GetBeatLengthSeconds()
{
	return((60.0/(double)GetBPM()));
}

double
TurntableObj::GetMeasureLengthSeconds()
{
	return(4*GetBeatLengthSeconds());
}

void
TurntableObj::
UpdateSoundFreqResponse()
{
	for(int a=0;a<lm;a++)
	{
		FreqResponse[a]=LGL_Min(2.0f,EQFinal[0]*MixerEQ[0]);
	}
	for(int a=lm;a<mh;a++)
	{
		FreqResponse[a]=LGL_Min(2.0f,EQFinal[1]*MixerEQ[1]);
	}
	for(int a=mh;a<513;a++)
	{
		FreqResponse[a]=LGL_Min(2.0f,EQFinal[2]*MixerEQ[2]);
	}

	if
	(
		Sound &&
		Channel != -1
	)
	{
		Sound->SetFreqResponse(Channel,FreqResponse);
	}
}

bool
TurntableObj::
GetAudioInputMode()
{
	return(AudioInputMode);
}

float
TurntableObj::
GetEQLo()
{
	return(EQFinal[0]);
}

float
TurntableObj::
GetEQMid()
{
	return(EQFinal[1]);
}

float
TurntableObj::
GetEQHi()
{
	return(EQFinal[2]);
}

bool
TurntableObj::
GetMaster()
{
	return(Master==Which);
}

void
TurntableObj::
SetMaster()
{
	Master=Which;
}

int
TurntableObj::
GetSoundChannel()
{
	return(Sound ? Channel : -1);
}

void
TurntableObj::
SwapVideos()
{
	LGL_VideoDecoder* temp=VideoFront;
	VideoFront=VideoBack;
	VideoBack=temp;
}

bool
TurntableObj::
GetSurroundMode()
{
	return(SurroundMode);
}

int
TurntableObj::
GetAspectRatioMode()
{
	return(AspectRatioMode);
}

void
TurntableObj::
SelectNewVideo
(
	bool	forceRandom
)
{
	char path[2048];
	if(FreqSenseBrightness>0.0f)
	{
		//Change the freq-videos
		Visualizer->GetNextVideoPathRandomLow(path);
		VideoLo->SetVideo(path);

		Visualizer->GetNextVideoPathRandomHigh(path);
		VideoHi->SetVideo(path);
		LGL_DrawLogWrite("!dvj::NewVideo|%s\n",VideoLo->GetPath());
		LGL_DrawLogWrite("!dvj::NewVideo|%s\n",VideoHi->GetPath());
	}

	if(VideoEncoderThread==NULL)
	{
		//Change the normal videos
		char videoFileName[1024];
		findVideoPath
		(
			videoFileName,
			SoundSrcPath
		);

		if
		(
			forceRandom ||
			VideoFileExists==false
		)
		{
			//Get next random video from Visualizer.
			if(LGL_RandInt(0,1)==0)
			{
				Visualizer->GetNextVideoPathRandomLow(path);
			}
			else
			{
				Visualizer->GetNextVideoPathRandomHigh(path);
			}

			if(path[0]!='\0')
			{
				if(VideoBack==NULL)
				{
					VideoBack=new LGL_VideoDecoder(path);
					VideoBack->SetFrameBufferAddRadius(GetVideoBufferFrames());
				}
				else
				{
					VideoBack->SetVideo(path);
				}
			}
			else
			{
				//There aren't any videos available. That's fine.
				return;
			}

			VideoOffsetSeconds=LGL_RandFloat(0,1000.0f);

			assert(VideoBack);
		}
		else
		{
			if
			(
				VideoFront &&
				strcmp(VideoFront->GetPath(),videoFileName)==0
			)
			{
				return;
			}

			VideoFront->SetVideo(videoFileName);
			VideoOffsetSeconds=0;
		}
		//assert(VideoBack);
		//SwapVideos();
		//assert(VideoFront);
		LGL_DrawLogWrite("!dvj::NewVideo|%s\n",VideoFront->GetPath());
	}
}

bool
TurntableObj::
BPMAvailable()
{
	return
	(
		Mode==2 &&
		SavePointSeconds[0]!=-1.0f &&
		SavePointSeconds[1]!=-1.0f &&
		SavePointSeconds[1]>SavePointSeconds[0]
	);
}

float
TurntableObj::
GetBPM()
{
	if(BPMAvailable()==false)
	{
		return(0.0f);
	}
	else if(BPMRecalculationRequired)
	{
		int bpmMin=100;
		float p0=SavePointSeconds[0];
		float p1=SavePointSeconds[1];
		float dp=p1-p0;
		int measuresGuess=1;
		float bpmGuess;
		if(dp!=0)
		{
			for(int a=0;a<10;a++)
			{
				bpmGuess=(4*measuresGuess)/(dp/60.0f);
				if(bpmGuess>=bpmMin)
				{
					BPM=bpmGuess;
					break;
				}
				measuresGuess*=2;
			}
		}
	}
	return(BPM);
}

float
TurntableObj::
GetBPMAdjusted()
{
	if(BPMAvailable()==false)
	{
		return(0.0f);
	}
	else
	{
		return(GetBPM()*Pitchbend);
	}
}

float
TurntableObj::
GetBPMFirstBeatSeconds()
{
	if(BPMAvailable())
	{
		return(SavePointSeconds[0]);
	}
	else
	{
		return(0.0f);
	}
}

float
TurntableObj::
GetBPMFirstMeasureSeconds()
{
	if(BPMAvailable())
	{
		double firstBeatSeconds = GetBPMFirstBeatSeconds();
		double deltaMeasure = GetMeasureLengthSeconds();
		double candidate = firstBeatSeconds;
		while(candidate - deltaMeasure > 0)
		{
			candidate -= deltaMeasure;
		}
		return(candidate);
	}
	else
	{
		return(0.0f);
	}
}

float
TurntableObj::
GetBPMLastMeasureSeconds()
{
	if(BPMAvailable())
	{
		double firstBeatSeconds = GetBPMFirstBeatSeconds();
		double deltaMeasure = GetMeasureLengthSeconds();
		double candidate = firstBeatSeconds;
		while(candidate + deltaMeasure < Sound->GetLengthSeconds()-0.01f)
		{
			candidate += deltaMeasure;
		}
		return(candidate);
	}
	else
	{
		return(0.0f);
	}
}

void
TurntableObj::
SetBPMAdjusted
(
	float	bpmAdjusted
)
{
	float bpm = GetBPMAdjusted();
	float best = 99999.0f;	//Start crazy
	for(int a=0;a<3;a++)
	{
		float candidate = bpmAdjusted/(0.5f*powf(2,a)*bpm);
		if(fabsf(1.0f-candidate) < fabsf(1.0f-best))
		{
			best = candidate;
		}
	}
	Pitchbend=best*Pitchbend;
	PitchbendLastSetBySlider=false;
}

void
TurntableObj::
SetBPMMaster
(
	float bpmMaster
)
{
	if(BPMMaster!=bpmMaster)
	{
		//Reevaluate filtered entries
		FilterTextMostRecent[0]='\0';
	}

	BPMMaster=bpmMaster;
}

bool
TurntableObj::
GetBeatThisFrame
(
	float fractionOfBeat
)
{
	if(fractionOfBeat==0.0f)
	{
		return(false);
	}
	if(Sound==NULL)
	{
		return(false);
	}
	if(BPMAvailable()==false)
	{
		return(false);
	}

	double startBeat = GetBPMFirstMeasureSeconds();
	double deltaBeat = fractionOfBeat * (60.0/(double)GetBPM());
	double windowStart = SecondsLast;
	double windowEnd = SecondsNow;
	double candidate = startBeat;

	if
	(
		Looping() &&
		fabsf(SecondsNow-LoopAlphaSeconds) < 0.1 &&
		Sound->GetWarpPointIsSet(Channel)==false
	)
	{
		return(true);
	}

	while(candidate < windowEnd)
	{
		if
		(
			candidate>=windowStart &&
			candidate< windowEnd
		)
		{
			return(true);
		}
		candidate+=deltaBeat;
	}

	return(false);
}

double
TurntableObj::
GetPercentOfCurrentMeasure
(
	float	measureMultiplier
)
{
	if
	(
		BPMAvailable()==false ||
		Channel<0
	)
	{
		return(-1.0);
	}

	double startMeasure = GetBeginningOfCurrentMeasureSeconds(measureMultiplier);
	double deltaMeasure = measureMultiplier*GetMeasureLengthSeconds();

	return((SecondsNow-startMeasure)/deltaMeasure);
}

double
TurntableObj::
GetBeginningOfCurrentMeasureSeconds
(
	float	measureMultiplier
)
{
	return(GetBeginningOfArbitraryMeasureSeconds(SecondsNow,measureMultiplier));
}

double
TurntableObj::
GetBeginningOfArbitraryMeasureSeconds
(
	float	seconds,
	float	measureMultiplier
)
{
	if
	(
		BPMAvailable()==false ||
		Channel<0
	)
	{
		return(-1.0);
	}
	
	double deltaMeasure = measureMultiplier*GetMeasureLengthSeconds();

	double candidate = GetBPMFirstMeasureSeconds();
	if(candidate==seconds)
	{
		//Huzzah!
	}
	else if(candidate < seconds)
	{
		while(candidate+deltaMeasure<seconds)
		{
			candidate+=deltaMeasure;
		}
	}
	else
	{
		while(candidate-0.01f>seconds)
		{
			candidate-=deltaMeasure;
		}
	}

	return(candidate);
}

bool
TurntableObj::
GetSolo()
{
	return(VolumeSolo);
}

bool
TurntableObj::
GetRapidSoloInvert()
{
	return(RapidSoloInvert);
}

void
TurntableObj::
SetRespondToRapidSoloInvert
(
	int	soloChannel
)
{
	if(Sound)
	{
		Sound->SetRespondToRapidSoloInvertChannel
		(
			Channel,
			soloChannel
		);
	}
}

void
TurntableObj::
BlankFilterTextIfMode0()
{
	if(Mode==0) FilterText.SetString();
}

void
TurntableObj::
FileSelectToString
(
	const char*	str
)
{
	if(str==NULL)
	{
		return;
	}

	for(unsigned int a=0;a<DatabaseFilteredEntries.size();a++)
	{
		if(strcmp(DatabaseFilteredEntries[a]->PathShort,str)==0)
		{
			FileTop=(DatabaseFilteredEntries.size()>4) ? a : 0;
			FileSelectInt=a;
			FileSelectFloat=FileSelectInt;

			if((unsigned int)FileTop>DatabaseFilteredEntries.size()-5)
			{
				FileTop=LGL_Max(0,DatabaseFilteredEntries.size()-5);
			}

			break;
		}
	}
}

float
TurntableObj::
GetNoiseFactorVideo()
{
	return(NoiseFactorVideo);
}

void
TurntableObj::
SetNoiseFactorVideo
(
	float	noiseFactorVideo
)
{
	NoiseFactorVideo = LGL_Clamp(0.0f,noiseFactorVideo,1.0f);
}

