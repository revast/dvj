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
#include "InputMouse.h"
#include "InputTester.h"
#include "InputXponent.h"

#include <string.h>
#include <stdlib.h>

#define	BEATMANIA

#define	GLITCH_PURE_VIDEO_MULTIPLIER (5.0f)

int ENTIRE_WAVE_ARRAY_COUNT;

int TurntableObj::Master=0;
VisualizerObj* TurntableObj::Visualizer=NULL;
LGL_Image* TurntableObj::NoiseImage[NOISE_IMAGE_COUNT_256_64];
LGL_Image* TurntableObj::LoopImage=NULL;
bool TurntableObj::FileEverOpened=false;
LGL_Timer TurntableObj::FileEverOpenedTimer;
bool TurntableObj::SurroundMode=false;

const char* audioExtension = "flac";

SavepointObj::
SavepointObj()
{
	Seconds=-1.0f;
	BPM=BPM_UNDEF;
	UnsetNoisePercent=0.0f;
	UnsetFlashPercent=0.0f;
}

SavepointObj::
~SavepointObj()
{
	//
}

void
findCachedPath
(
	char*		foundPath,
	const char*	srcPath,
	const char*	extension,
	std::vector<const char*>*
			searchList
);

void
findCachedPath
(
	char*		foundPath,
	const char*	srcPath,
	const char*	extension,
	std::vector<const char*>*
			searchList
)
{
	if(foundPath==NULL)
	{
		return;
	}
	foundPath[0]='\0';

	if(srcPath==NULL)
	{
		return;
	}

	if(LGL_FileExtensionIsImage(srcPath))
	{
		if(GetDebugVideoCaching())
		{
			printf("found: File is image!\n");
			LGL_DebugPrintf("found: File is image!\n");
		}
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
		if(GetDebugVideoCaching())
		{
			printf("found: srcPath has .mjpeg.avi extension\n");
			LGL_DebugPrintf("found: srcPath has .mjpeg.avi extension\n");
		}
		if(searchList)
		{
			const int neoSize = strlen(foundPath)+1;
			char* neo = new char[neoSize];
			strncpy(neo,foundPath,neoSize-1);
			neo[neoSize-1]='\0';
			searchList->push_back(neo);
		}
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
	if(searchList)
	{
		const int neoSize = strlen(foundPath)+1;
		char* neo = new char[neoSize];
		strncpy(neo,foundPath,neoSize-1);
		neo[neoSize-1]='\0';
		searchList->push_back(neo);
	}
	else
	{
		if(LGL_FileExists(foundPath))
		{
			//Found /home/id/mp3/hajnal.mov.mjpeg.avi
			if(GetDebugVideoCaching())
			{
				printf("found: %s\n",foundPath);
				LGL_DebugPrintf("found: %s\n",foundPath);
			}
			return;
		}
		else
		{
			if(GetDebugVideoCaching())
			{
				printf("no exist: %s\n",foundPath);
				LGL_DebugPrintf("no exist: %s\n",foundPath);
			}
		}
	}

	const char* dvjCacheDirName = GetDvjCacheDirName();

	sprintf(foundPath,"%s/%s/%s.%s",soundSrcDir,dvjCacheDirName,soundName,extension);
	if(searchList)
	{
		const int neoSize = strlen(foundPath)+1;
		char* neo = new char[neoSize];
		strncpy(neo,foundPath,neoSize-1);
		neo[neoSize-1]='\0';
		searchList->push_back(neo);
	}
	else
	{
		if(LGL_FileExists(foundPath))
		{
			//Found /home/id/mp3/dvj_cache/hajnal.mov.mjpeg.avi
			if(LGL_FirstFileMoreRecentlyModified(srcPath,foundPath))
			{
				LGL_FileDelete(foundPath);
			}
			else
			{
				if(GetDebugVideoCaching())
				{
					printf("found: %s\n",foundPath);
					LGL_DebugPrintf("found: %s\n",foundPath);
				}
				return;
			}
		}
		else
		{
			if(GetDebugVideoCaching())
			{
				printf("no exist: %s\n",foundPath);
				LGL_DebugPrintf("no exist: %s\n",foundPath);
			}
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
		if(searchList)
		{
			const int neoSize = strlen(foundPath)+1;
			char* neo = new char[neoSize];
			strncpy(neo,foundPath,neoSize-1);
			neo[neoSize-1]='\0';
			searchList->push_back(neo);
		}
		else
		{
			if(LGL_FileExists(foundPath))
			{
				//Found /home/id/mp3/dvj_cache/hajnal.mov.mjpeg.avi (for audio??)
				if(LGL_FirstFileMoreRecentlyModified(srcPath,foundPath))
				{
					LGL_FileDelete(foundPath);
				}
				else
				{
					if(GetDebugVideoCaching())
					{
						printf("found: %s\n",foundPath);
						LGL_DebugPrintf("found: %s\n",foundPath);
					}
					return;
				}
			}
			else
			{
				if(GetDebugVideoCaching())
				{
					printf("no exist: %s\n",foundPath);
					LGL_DebugPrintf("no exist: %s\n",foundPath);
				}
			}
		}
	}

	sprintf(foundPath,"%s/.dvj/video/tracks/%s.%s",LGL_GetHomeDir(),soundName,extension);
	if(searchList)
	{
		const int neoSize = strlen(foundPath)+1;
		char* neo = new char[neoSize];
		strncpy(neo,foundPath,neoSize-1);
		neo[neoSize-1]='\0';
		searchList->push_back(neo);
	}
	else
	{
		if(LGL_FileExists(foundPath))
		{
			//Found /home/id/.dvj/video/tracks/hajnal.mov.mjpeg.avi
			if(GetDebugVideoCaching())
			{
				printf("found: %s\n",foundPath);
				LGL_DebugPrintf("found: %s\n",foundPath);
			}
			return;
		}
		else
		{
			if(GetDebugVideoCaching())
			{
				printf("no exist: %s\n",foundPath);
				LGL_DebugPrintf("no exist: %s\n",foundPath);
			}
		}
	}

	if
	(
		strstr(srcPath,".mjpeg.avi") &&
		strcmp(extension,"flac")==0
	)
	{
		sprintf(foundPath,"%s/.dvj/video/tracks/%s",LGL_GetHomeDir(),soundName);
		sprintf(strstr(foundPath,".mjpeg.avi"),".%s",extension);
		if(searchList)
		{
			const int neoSize = strlen(foundPath)+1;
			char* neo = new char[neoSize];
			strncpy(neo,foundPath,neoSize-1);
			neo[neoSize-1]='\0';
			searchList->push_back(neo);
		}
		else
		{
			if(LGL_FileExists(foundPath))
			{
				//Found /home/id/mp3/dvj/hajnal.mov.mjpeg.avi
				if(GetDebugVideoCaching())
				{
					printf("found: %s",foundPath);
					LGL_DebugPrintf("found: %s\n",foundPath);
				}
				return;
			}
			else
			{
				if(GetDebugVideoCaching())
				{
					printf("no exist: %s",foundPath);
					LGL_DebugPrintf("no exist: %s\n",foundPath);
				}
			}
		}
	}

	strcpy(foundPath,srcPath);
	if(strstr(extension,".mjpeg.avi"))
	{
		if(searchList)
		{
			const int neoSize = strlen(foundPath)+1;
			char* neo = new char[neoSize];
			strncpy(neo,foundPath,neoSize-1);
			neo[neoSize-1]='\0';
			searchList->push_back(neo);
		}
		else
		{
			if(LGL_FileExists(foundPath))
			{
				//Found /home/id/mp3/hajnal.mov
				if(GetDebugVideoCaching())
				{
					printf("found: %s\n",foundPath);
					LGL_DebugPrintf("found: %s\n",foundPath);
				}
				return;
			}
			else
			{
				if(GetDebugVideoCaching())
				{
					printf("no exist: %s\n",foundPath);
					LGL_DebugPrintf("no exist: %s\n",foundPath);
				}
			}
		}
	}

	//Found nothing
	foundPath[0]='\0';
}

void
findVideoPath
(
	char*		foundPath,
	const char*	srcPath
);

void
findVideoPath
(
	char*		foundPath,
	const char*	srcPath
)
{
	findCachedPath(foundPath,srcPath,"mjpeg.avi",NULL);
}

std::vector<const char*>
listVideoSearchPaths
(
	const char*	srcPath
)
{
	std::vector<const char*> pathAttempts;
	char foundPath[2048];
	findCachedPath(foundPath,srcPath,"mjpeg.avi",&pathAttempts);
	return(pathAttempts);
}

void
findAudioPath
(
	char*		foundPath,
	const char*	srcPath
);

void
findAudioPath
(
	char*		foundPath,
	const char*	srcPath
)
{
	findCachedPath(foundPath,srcPath,audioExtension,NULL);
}

int
videoEncoderThread
(
	void*	ptr
);

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
			char foundPath[2048];
			findVideoPath
			(
				foundPath,
				encoderSrc
			);

			bool isMjpeg=LGL_VideoIsMJPEG(encoderSrc);

			if(isMjpeg)
			{
				static LGL_Semaphore lnSym("lnSym");
				LGL_ScopeLock lock(__FILE__,__LINE__,lnSym);
				char soundSrcDir[2048];
				strcpy(soundSrcDir,encoderSrc);
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
					strcpy(soundName,&(strrchr(encoderSrc,'/')[1]));
				}
				else
				{
					strcpy(soundName,encoderSrc);
				}
				sprintf(foundPath,"%s/%s/%s.mjpeg.avi",soundSrcDir,GetDvjCacheDirName(),soundName);
				if(LGL_FileExists(foundPath)==false)
				{
					if(GetDebugVideoCaching())
					{
						LGL_DebugPrintf("Attempting symlink at path: '%s'",foundPath);
						LGL_DebugPrintf("Attempting symlink target: '%s'",encoderSrc);
					}
					LGL_FileDelete(foundPath);	//For stale symlinks...
					char cmd[4096];
					char dir[4096];
					sprintf(dir,"%s/%s",soundSrcDir,GetDvjCacheDirName());
					LGL_DirectoryCreate(dir);
					sprintf
					(
						cmd,
						"ln -s '%s' '%s'",
						encoderSrc,
						foundPath
					);
					system(cmd);
				}
				if(GetDebugVideoCaching())
				{
					LGL_DebugPrintf("srcPath: %s\n",encoderSrc);
					LGL_DebugPrintf("Symlink exists? '%s'",LGL_FileExists(foundPath) ? "YES" : "NO");
					LGL_DebugPrintf("Symlink is symlink? '%s'",LGL_PathIsSymlink(foundPath) ? "YES" : "NO");
					if(LGL_PathIsSymlink(foundPath))
					{
						char symlinkTarget[2048];
						bool ok=LGL_ResolveSymlink(symlinkTarget,2048,foundPath);
						LGL_DebugPrintf("Symlink target: '%s' (%s)",symlinkTarget,ok ? "OK" : "FAIL");
					}
				}
			}
			if(LGL_FileExists(foundPath))
			{
				strcpy(encoderDst,foundPath);
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
			if(LGL_FileExists(encoderDst)==false)
			{
				sprintf(tt->VideoEncoderReason,"Video doesn't exist");
			}
			else if(LGL_FileExists(encoderAudioDst)==false)
			{
				sprintf(tt->VideoEncoderReason,"Audio doesn't exist");
			}

			LGL_VideoEncoder::SetBitrateMaxMBps(GetCachedVideoConstBitrateMBps());

			{
				LGL_ScopeLock lock(__FILE__,__LINE__,tt->VideoEncoderSemaphore);
				tt->VideoEncoder = new LGL_VideoEncoder
				(
					encoderSrc,
					encoderDstTmp,
					encoderAudioDstTmp
				);
			}

			LGL_VideoEncoder* encoder = tt->VideoEncoder;
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

				//Main encode loop
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

				LGL_LoopCounterOmega();

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
			char videoFileName[1024];
			findVideoPath
			(
				videoFileName,
				tt->SoundSrcPath
			);
			tt->VideoFileExists=LGL_FileExists(videoFileName);
			if(tt->VideoFileExists)
			{
				strcpy(tt->FoundVideoPath,videoFileName);
			}
			else
			{
				tt->FoundVideoPath[0]='\0';
			}
			tt->VideoEncoderPercent=2.0f;
		}
		else
		{
			tt->VideoEncoderPercent=-1.0f;
		}

		tt->VideoEncoderEtaSeconds=-1.0f;
	}

	tt->VideoEncoderTerminateSignal=-1;
	tt->VideoEncoderEndSignal=1;
	tt->VideoEncoderAudioOnly=false;

	return(0);
}

int
loadAllCachedDataThread
(
	void*	ptr
);

int
loadAllCachedDataThread
(
	void*	ptr
)
{
	TurntableObj* tt = (TurntableObj*)ptr;

	tt->LoadAllCachedData();

	return(0);
}

int
findAudioPathThread
(
	void*	ptr
);

int
findAudioPathThread
(
	void*	ptr
)
{
	TurntableObj* tt = (TurntableObj*)ptr;

	LGL_FileInfo fileInfo(tt->SoundSrcPath);
	if(fileInfo.Type==LGL_FILETYPE_SYMLINK)
	{
		if(LGL_DirectoryExists(tt->SoundSrcPath))
		{
			strcpy(tt->FoundAudioPath,tt->SoundSrcPath);
			tt->FoundAudioPathIsDir=true;
			tt->FindAudioPathDone=true;
			return(0);
		}
	}
	else if(fileInfo.Type==LGL_FILETYPE_FILE)
	{
		if(LGL_PathIsAlias(tt->SoundSrcPath))
		{
			LGL_ResolveAlias(tt->FoundAudioPath,2048,tt->SoundSrcPath);
			if(LGL_DirectoryExists(tt->FoundAudioPath))
			{
				char checkDotSymlink[2048];
				sprintf(checkDotSymlink,"%s.symlink",tt->SoundSrcPath);
				if
				(
					LGL_FileExists(checkDotSymlink)==false &&
					LGL_DirectoryExists(checkDotSymlink)==false
				)
				{
					LGL_FileDelete(checkDotSymlink);
					symlink(tt->FoundAudioPath,checkDotSymlink);
				}
				strcpy(tt->FoundAudioPath,checkDotSymlink);
				tt->Database->Refresh(tt->FoundAudioPath);
				tt->FoundAudioPathIsDir=true;
				tt->FindAudioPathDone=true;
				return(0);
			}
			else
			{
				strcpy(tt->SoundSrcPath,tt->FoundAudioPath);
			}
		}
	}
	
	findAudioPath(tt->FoundAudioPath,tt->SoundSrcPath);
	if(LGL_FileExists(tt->FoundAudioPath)==false)
	{
		strcpy(tt->FoundAudioPath,tt->SoundSrcPath);
	}

	findVideoPath
	(
		tt->FoundVideoPath,
		tt->SoundSrcPath
	);
	tt->VideoFileExists=LGL_FileExists(tt->FoundVideoPath);

	tt->FindAudioPathDone=true;

	return(0);
}

int
warmMemoryThread
(
	void*	ptr
);

int
warmMemoryThread
(
	void*	ptr
)
{
	return(0);	//No need for this, since I can mlock().
/*
	TurntableObj* tt = (TurntableObj*)ptr;

	for(;;)
	{
		//Apple doesn't allow us to mlockall().
		//As such, we must loop on each page in our soundbuffer,
		//to ensure it remains in active memory... UGH.
		//TODO: Does mlock() work? I doubt it...

		if(tt->WarmMemoryThreadTerminateSignal==1)
		{
			break;
		}

		const int stepSize=100;
		Uint32* buffer = (Uint32*)tt->SoundBuffer;
		unsigned long index=tt->SoundSampleNow;
		float candidate=0;
		for(int a=0;a<441;a++)
		{
			if(index>tt->SoundBufferLength/4-stepSize)
			{
				break;
			}

			Uint32 pageMeIn = buffer[index];
			candidate+=pageMeIn*1;

			index+=stepSize;
		}

		LGL_DelayMS(20);
	}

	return(0);
*/
}

int
updateDatabaseFilterThread
(
	void*	ptr
);

int
updateDatabaseFilterThread
(
	void*	ptr
)
{
	TurntableObj* tt = (TurntableObj*)ptr;

	tt->DatabaseFilteredEntriesNext=tt->Database->GetEntryListFromFilter
	(
		&(tt->UpdateFilterListDatabaseFilterNow),
		&(tt->UpdateFilterListAbortSignal)
	);
	tt->DatabaseFilteredEntriesNextReady=true;

	return(0);
}



TurntableObj::
TurntableObj
(
	float	left,	float	right,
	float	bottom,	float	top,
	DatabaseObj* database
) :
	VideoEncoderSemaphore("VideoEncoderSemaphore"),
	ListSelector(2)
{
	Mode=0;
	Mode0Timer.Reset();

	SoundName[0]='\0';
	SoundSrcPath[0]='\0';
	SoundSrcNameDisplayed[0]='\0';
	SoundSrcDir[0]='\0';

	SetViewport(left,right,bottom,top);
	SetFocus(false);
	
	Sound=NULL;
	sprintf(SoundName,"No Track Selected");
	SoundBufferLength=4*44100*60*20;
	if(int minutes=GetAudioMaxLengthMinutes())
	{
		SoundBufferLength=4*44100*60*minutes;
	}

	//SoundBufferLength=(unsigned long)(1024*1024*200*(1.0/0.987875));	//20 minutes
	SoundBuffer=(Uint8*)malloc(SoundBufferLength);
	bzero(SoundBuffer,SoundBufferLength);	//Not necessary, but used for testing
	mlock(SoundBuffer,SoundBufferLength);
	SoundSampleNow=0;

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

	DatabaseFilteredEntriesNextReady=false;

	FilterTextMostRecent[0]='\0';
	FilterTextMostRecentBPM=-1;

	Channel=-1;
	PauseMultiplier=0;
	Pitchbend=1.0f;
	PitchbendLastSetByXponentSlider=false;
	Nudge=0;
	MixerNudge=0;
	MixerVideoMute=false;
	ReverseMultiplier=1.0f;
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
	RhythmicVolumeInvert=false;
	RhythmicSoloInvert=false;
	LoopAlphaSeconds=-1.0;
	QuantizePeriodMeasuresExponent=-4;
	QuantizePeriodNoBPMSeconds=1.0;
	LoopActive=false;
	LoopThenRecallActive=false;
	AutoDivergeRecallActive=false;
	SavepointIndex=0;
	while(Savepoints.size()<12)
	{
		Savepoints.push_back(SavepointObj());
	}
	MetadataSavedThisFrame=NULL;
	MetadataFileToRam=NULL;
	RecordScratch=false;
	LuminScratch=false;

	for(int a=0;a<3;a++)
	{
		EQFinal[a]=1.0f;
		EQKnob[a]=1.0f;
		EQKill[a]=false;
		EQPeak[a]=0.0f;
	}
	VUPeak=0.0f;
	for(int a=0;a<513;a++) FreqResponse[a]=1.0f;

	Which=0;

	ImageSetPrefix[0]='\0';
	MovieClipPrefix[0]='\0';
	Visualizer=NULL;

	BPMMaster=0.0f;

	TrackListFileUpdates.clear();

	Video=new LGL_VideoDecoder(NULL);
	Video->SetFrameBufferAddRadius(GetVideoBufferFrames());
	Video->SetPreloadMaxMB(GetPreloadVideoMaxMB());
	Video->SetPreloadFromCurrentTime(true);
	Video->SetReadAheadMB(16);
	Video->SetReadAheadDelayMS(200);
	Video->SetDecodeInThread(true);
	VideoLo=NULL;
	VideoHi=NULL;
	if(VideoLo==NULL)
	{
		VideoLo=new LGL_VideoDecoder(NULL);
		VideoLo->SetFrameBufferAddRadius(GetVideoBufferFramesFreqSense());
		VideoLo->SetFrameBufferAddBackwards(false);
		VideoLo->SetPreloadMaxMB(GetPreloadFreqSenseMaxMB());
		VideoLo->SetPreloadFromCurrentTime(false);
		//VideoLo->SetReadAheadMB(0);
		//VideoLo->SetReadAheadDelayMS(10000);
		VideoLo->SetReadAheadMB(16);
		VideoLo->SetReadAheadDelayMS(200);
		VideoLo->SetDecodeInThread(LGL_CPUCount()>=4);
	}
	if(VideoHi==NULL)
	{
		VideoHi=new LGL_VideoDecoder(NULL);
		VideoHi->SetFrameBufferAddRadius(GetVideoBufferFramesFreqSense());
		VideoHi->SetFrameBufferAddBackwards(false);
		VideoHi->SetPreloadMaxMB(GetPreloadFreqSenseMaxMB());
		VideoHi->SetPreloadFromCurrentTime(false);
		//VideoHi->SetReadAheadMB(0);
		//VideoHi->SetReadAheadDelayMS(10000);
		VideoHi->SetReadAheadMB(16);
		VideoHi->SetReadAheadDelayMS(200);
		VideoHi->SetDecodeInThread(LGL_CPUCount()>=4);
	}
	VideoLoPath[0]='\0';
	VideoHiPath[0]='\0';
	VideoLoPathShort[0]='\0';
	VideoHiPathShort[0]='\0';
	VideoAdvanceRate=1.0f;
	VideoBrightness=1.0f;
	SyphonBrightness=0.0f;
	OscilloscopeBrightness=0.0f;
	FreqSenseBrightness=1.0f;
	FreqSensePathBrightness=0.0f;

	FreqSenseLEDGroupFloat=0.0f;
	for(int g=0;g<LED_GROUP_MAX;g++)
	{
		FreqSenseLEDBrightness[g]=0.0f;
		FreqSenseLEDColorScalarLow[g]=4.0f/6.0f;
		FreqSenseLEDColorScalarHigh[g]=5.0f/6.0f;
		FreqSenseLEDBrightnessWash[g]=0.0f;
	}

	AudioInputMode=false;
	TesterEverEnabled=false;

	VideoEncoder=NULL;
	VideoEncoderThread=NULL;
	VideoEncoderReason[0]='\0';

	ENTIRE_WAVE_ARRAY_COUNT=LGL_WindowResolutionX();

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
	LoadAllCachedDataThread=NULL;
	FindAudioPathThread=NULL;
	WarmMemoryThread=NULL;//LGL_ThreadCreate(warmMemoryThread,this);
	UpdateFilterListViaThread=true;
	UpdateFilterListThread=NULL;
	UpdateFilterListDatabaseFilterNext.SetBPMCenter(-9999.0f);
	UpdateFilterListAbortSignal=false;
	UpdateFilterListResetHighlightedRow=false;
	UpdateFilterListDesiredSelection[0]='\0';
	WarmMemoryThreadTerminateSignal=0;
	UpdateFilterListThreadTerminateSignal=0;
	AspectRatioMode=0;
	EncodeEveryTrack=0;
	EncodeEveryTrackIndex=0;

	InputUnsetDebounce=false;

	Database=database;
	char musicRoot[2048];
	strcpy(musicRoot,GetMusicRootPath());
	DatabaseFilter.SetDir(musicRoot);
	DatabaseFilteredEntries=Database->GetEntryListFromFilter(&DatabaseFilter);
	UpdateListSelector();

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

	//ListSelector TODO: Start at first non-dir row

	VideoEncoderPercent=-1.0f;
	VideoEncoderEtaSeconds=-1.0f;
	VideoEncoderAudioOnly=false;
	VideoEncoderPathSrc[0]='\0';
	VideoEncoderThread=NULL;
	VideoEncoderTerminateSignal=0;
	VideoEncoderBeginSignal=0;
	VideoEncoderUnsupportedCodecTime=0.0f;
	VideoEncoderUnsupportedCodecName[0]='\0';

	WaveformLeft=ViewportLeft+0.5f*ViewportWidth-(0.5f*ViewportWidth*WAVE_WIDTH_PERCENT);
	WaveformRight=ViewportLeft+0.5f*ViewportWidth+(0.5f*ViewportWidth*WAVE_WIDTH_PERCENT);
	WaveformBottom=ViewportBottom+0.125f*ViewportHeight;
	WaveformTop=ViewportBottom+0.875f*ViewportHeight;
	WaveformWidth=WaveformRight-WaveformLeft;
	WaveformHeight=WaveformTop-WaveformBottom;

	DenyPreviewNameDisplayed[0]='\0';

	ListSelector.SetWindowScope
	(
		WaveformLeft,
		WaveformRight,
		ViewportBottom,
		ViewportBottom+0.825f*ViewportHeight
	);
	//ListSelector.SetWindowScope(ViewportLeft+ViewportHeight,ViewportRight,ViewportBottom,ViewportBottom+0.8f*ViewportHeight);
	ListSelector.SetColLeftEdge(1,0.075f);

	UpdateSoundFreqResponse();
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
	if(Video)
	{
		delete Video;
		Video=NULL;
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
	if(WarmMemoryThread)
	{
		WarmMemoryThreadTerminateSignal=1;
		LGL_ThreadWait(WarmMemoryThread);
		WarmMemoryThread=NULL;
	}

	if(UpdateFilterListThread)
	{
		UpdateFilterListThreadTerminateSignal=1;
		LGL_ThreadWait(UpdateFilterListThread);
		UpdateFilterListThread=NULL;
	}

	if(Sound)
	{
		Sound->PrepareForDelete();
		for(;;)
		{
			if
			(
				Sound->ReadyForDelete()
			)
			{
				break;
			}
			else
			{
				LGL_DelayMS(10);
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
	//Debug
	{
		if(Mode==0)
		{
			//LGL_DebugPrintf("VideoLo: %s\n",VideoLo ? VideoLo->GetPath() : "NULL Vid");
			//LGL_DebugPrintf("VideoHi: %s\n",VideoHi ? VideoHi->GetPath() : "NULL Vid");
		}
		if(Mode==2)
		{
			//LGL_DebugPrintf("Beg: %.2f\n",GetBeginningOfCurrentMeasureSeconds());
			//LGL_DebugPrintf("PCM: %f\n",GetPercentOfCurrentMeasure());
			//LGL_DebugPrintf("MLS: %.2f\n",GetMeasureLengthSeconds());
			//LGL_DebugPrintf("Video: %s\n",Video ? Video->GetPath() : "NULL VIDEO");
			//LGL_DebugPrintf("Rate: %.2f\n",SmoothWaveformScrollingSampleRate);
		}
	}

	//Ensure FreqVideos are NULL when Mode Zero
	/*
	if(Mode==0)
	{
		for(int a=0;a<2;a++)
		{
			LGL_VideoDecoder* video = NULL;
			if(a==0)
			{
				video=VideoLo;
			}
			else if(a==1)
			{
				video=VideoHi;
			}

			if
			(
				video &&
				video->GetPath() &&
				video->GetPath()[0]!='\0'
			)
			{
				video->InvalidateAllFrameBuffers();
				video->SetVideo(NULL);
				video->SetUserString("NULL");
			}
		}
	}
	*/

	//Deal with low memory
	if(0)
	{
		if(LGL_AudioJackXrun())
		{
			Video->SetFrameBufferAddRadius(Video->GetFrameBufferAddRadius()/2);
		}

		if(LGL_RamFreeMB()<100)
		{
			if(Video->GetFrameBufferAddRadius()>2)
			{
				//Video->SetFrameBufferAddRadius(Video->GetFrameBufferAddRadius()-1);
			}
		}
		else if(LGL_RamFreeMB()>200)
		{
			if(VideoRadiusIncreaseDelayTimer.SecondsSinceLastReset()>0.5f)
			{
				int radiusDesired = GetVideoBufferFrames();
				int radiusNow = Video->GetFrameBufferAddRadius();
				if(radiusNow<radiusDesired)
				{
					Video->SetFrameBufferAddRadius(radiusNow+1);
					VideoRadiusIncreaseDelayTimer.Reset();
				}
			}
		}
	}

	UpdateDatabaseFilterFn();
	if(GetInputTester().GetEnable())
	{
		TesterEverEnabled=true;
		LGL_SetDebugMode();
	}

	if(Sound && Channel>=0)
	{
		SoundSampleNow=Sound->GetPositionSamples(Channel);
	}
	else
	{
		SoundSampleNow=0;
	}

	if(Mode==0)
	{
		for(int r=ListSelector.GetVisibleRowIndexTop();r<=ListSelector.GetVisibleRowIndexBottom();r++)
		{
			dvjListSelectorCell* cell = ListSelector.GetCellColRow(0,r);
			if(DatabaseEntryObj* entry = (DatabaseEntryObj*)cell->UserData)
			{
				if(entry->IsDir==false)
				{
					if(entry->BPM>0)
					{
						ListSelector.SetCellColRowString
						(
							0,
							r,
							"%.0f",
							entry->BPM
						);
					}
					else
					{
						ListSelector.SetCellColRowString
						(
							0,
							r,
							""
						);
					}

					if(entry->Loadable==false)
					{
						ListSelector.SetCellColRowStringRGB
						(
							1,
							r,
							1.0f,
							0.0f,
							0.0f
						);
					}
					else if(entry->AlreadyPlayed)
					{
						ListSelector.SetCellColRowStringRGB
						(
							1,
							r,
							0.25f,
							0.25f,
							0.25f
						);
					}
				}
			}
		}
		ListSelector.NextFrame();
	}

	if(LGL_KeyStroke(LGL_KEY_F8))
	{
		EncodeEveryTrack=!EncodeEveryTrack;
		EncodeEveryTrackIndex=ListSelector.GetHighlightedRow();
	}

	unsigned int target = GetTarget();
	float candidate = 0.0f;

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

	if(MetadataSavedThisFrame)
	{
		delete MetadataSavedThisFrame;
		MetadataSavedThisFrame=NULL;
	}

	bool endpointsSticky=true;
	float fastSeekVolumeMultiplier=1.0f;
	bool noiseIncreasing=false;
	bool glitchPurePrev=GlitchPure;
	FinalSpeedLastFrame=FinalSpeed;
	
	VideoEncoderUnsupportedCodecTime = LGL_Max(0.0f,VideoEncoderUnsupportedCodecTime-secondsElapsed);

	//Volume
	candidate=GetInput().WaveformVolumeSlider(target);
	if(candidate!=-1.0f)
	{
		float volumePrev=VolumeSlider;
		VolumeSlider=candidate;
		float delta = VolumeSlider-volumePrev;
		VUPeak+=delta;
		for(int a=0;a<3;a++)
		{
			EQPeak[a]+=delta*0.5f;
		}
	}

	//Volume Delta
	{
		float volumeMultiplierNowPrev = VolumeMultiplierNow;
		VolumeMultiplierNow=LGL_Clamp(0.0f,VolumeMultiplierNow+GetInput().WaveformGainDelta(target),16.0f);
		candidate = GetInput().WaveformGain(target);
		if(candidate!=-1.0f)
		{
			VolumeMultiplierNow = candidate;
		}
		if(volumeMultiplierNowPrev!=VolumeMultiplierNow)
		{
			float delta = 0.5f*(VolumeMultiplierNow - volumeMultiplierNowPrev);
			VUPeak+=delta;
			for(int a=0;a<3;a++)
			{
				EQPeak[a]+=delta*0.5f;
			}
		}
	}

	VUPeak=LGL_Min(1.0f,VUPeak);

	//EQ Delta
	{
		for(int a=0;a<3;a++)
		{
			if(a==0) candidate = GetInput().WaveformEQLowDelta(target);
			else if(a==1) candidate = GetInput().WaveformEQMidDelta(target);
			else if(a==2) candidate = GetInput().WaveformEQHighDelta(target);
			if(candidate!=0.0f)
			{
				float knobPrev = EQKnob[a];
				EQKnob[a]=LGL_Clamp(0.0f,EQKnob[a]+candidate,2.0f);
				EQPeak[a]+=0.5f*(GetGain()/2.0f)*(EQKnob[a]-knobPrev);
			}
		}
	}

	//EQ
	{
		for(int a=0;a<3;a++)
		{
			if(a==0) candidate = GetInput().WaveformEQLow(target);
			else if(a==1) candidate = GetInput().WaveformEQMid(target);
			else if(a==2) candidate = GetInput().WaveformEQHigh(target);
			if(candidate!=-1.0f)
			{
				float knobPrev = EQKnob[a];
				EQKnob[a]=LGL_Clamp(0.0f,candidate*2.0f,2.0f);
				EQPeak[a]+=0.5f*(GetGain()/2.0f)*(EQKnob[a]-knobPrev);
			}
		}
	}

	//EQ Kill
	{
		EQKill[0]=GetInput().WaveformEQLowKill(target);
		EQKill[1]=GetInput().WaveformEQMidKill(target);
		EQKill[2]=GetInput().WaveformEQHighKill(target);
	}

	//EQ Peaks
	{
		float eqVuL = GetEQVUL();
		if(eqVuL>EQPeak[0])
		{
			EQPeak[0]=eqVuL;
			EQPeakDropTimer[0].Reset();
		}
		else
		{
			float dropRate = EQPeakDropTimer[0].SecondsSinceLastReset()-2.0f;
			if(dropRate>0.0f)
			{
				EQPeak[0]-=dropRate*LGL_SecondsSinceLastFrame();
			}
		}

		float eqVuM = GetEQVUM();
		if(eqVuM>EQPeak[1])
		{
			EQPeak[1]=eqVuM;
			EQPeakDropTimer[1].Reset();
		}
		else
		{
			float dropRate = EQPeakDropTimer[1].SecondsSinceLastReset()-2.0f;
			if(dropRate>0.0f)
			{
				EQPeak[1]-=dropRate*LGL_SecondsSinceLastFrame();
			}
		}

		float eqVuH = GetEQVUH();
		if(eqVuH>EQPeak[2])
		{
			EQPeak[2]=eqVuH;
			EQPeakDropTimer[2].Reset();
		}
		else
		{
			float dropRate = EQPeakDropTimer[2].SecondsSinceLastReset()-2.0f;
			if(dropRate>0.0f)
			{
				EQPeak[2]-=dropRate*LGL_SecondsSinceLastFrame();
			}
		}

		float vu = GetVU();
		if(vu>VUPeak)
		{
			VUPeak=vu;
			VUPeakDropTimer.Reset();
		}
		else
		{
			float dropRate = VUPeakDropTimer.SecondsSinceLastReset()-2.0f;
			if(dropRate>0.0f)
			{
				VUPeak-=dropRate*LGL_SecondsSinceLastFrame();
			}
		}

		for(int a=0;a<3;a++)
		{
			EQPeak[a]=LGL_Min(1.0f,EQPeak[a]);
		}
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

	VolumeSolo=GetInput().WaveformVolumeSolo(target);

	if(VideoEncoderTerminateSignal==-1)
	{
		LGL_ThreadWait(VideoEncoderThread);
		VideoEncoderThread=NULL;
		VideoEncoderTerminateSignal=0;
		{
			LGL_ScopeLock lock(__FILE__,__LINE__,VideoEncoderSemaphore);
			delete VideoEncoder;
			VideoEncoder=NULL;
		}
	}

	SecondsLast=SecondsNow;
	SecondsNow=GetTimeSeconds();

	//MetadataFileToRamDeathRow
	{
		if(MetadataFileToRamDeathRow.empty()==false)
		{
			if(MetadataFileToRamDeathRow[0]->GetStatus()!=0)
			{
				delete MetadataFileToRamDeathRow[0];
				MetadataFileToRamDeathRow.erase(MetadataFileToRamDeathRow.begin());
			}
		}
	}

	//SavepointIndex
	const bool SAVEPOINT_PREV_NEXT_WRAP=true;
	bool savepointIndexDelta=false;
	{
		/*
		if(SavepointIndex>SavepointIndexAtPlus())
		{
			SavepointIndex=SavepointIndexAtPlus();
		}
		*/
		if(GetInput().WaveformSavepointPrev(target))
		{
			//Prev Savepoint
			SavepointIndex--;
			if(SavepointIndex<0)
			{
				SavepointIndex=0;
				if(SAVEPOINT_PREV_NEXT_WRAP)
				{
					if
					(
						Savepoints.size()>=11 &&
						Savepoints[11].Seconds!=-1.0f
					)
					{
						SavepointIndex=11;
					}
					else
					{
						for(int a=Savepoints.size()-1;a>=0;a--)
						{
							SavepointIndex=a;
							if
							(
								(
									Mode!=2 &&
									Savepoints[SavepointIndex].Seconds!=-1.0f
								) ||
								(
									Mode==2 &&
									SavepointIndexAtPlus()
								)
							)
							{
								break;
							}
						}
					}
				}
			}
			savepointIndexDelta=true;
		}
		if(GetInput().WaveformSavepointNext(target))
		{
			//Next Savepoint
			if
			(
				SavepointIndex>=Savepoints.size() ||
				(
					Mode!=2 &&
					GetCurrentSavepointSeconds()==-1.0f
				) ||
				(
					Mode==2 &&
					SavepointIndexAtPlus()
				)
			)
			{
				if(SAVEPOINT_PREV_NEXT_WRAP)
				{
					SavepointIndex=0;
				}
			}
			else
			{
				SavepointIndex++;
			}
			if(SavepointIndex>11)
			{
				SavepointIndex=0;
			}
			savepointIndexDelta=true;
		}
		int savepointCandidate=GetInput().WaveformSavepointPick(target);
		if(savepointCandidate!=-9999)
		{
			int savepointIndexPrev=SavepointIndex;
			SavepointIndex=LGL_Clamp(0,savepointCandidate,Savepoints.size()-1);
			if(savepointIndexPrev!=SavepointIndex)
			{
				savepointIndexDelta=true;
			}
		}
	}

	if(Mode!=1)
	{
		FilterText.ReleaseFocus();
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

		if(Focus)
		{
			if(LGL_KeyStroke(LGL_KEY_ESCAPE))
			{
				FilterText.SetString();
			}

			if
			(
				LGL_KeyDown(LGL_KEY_BACKSPACE)==false ||
				LGL_KeyTimer(LGL_KEY_BACKSPACE) < Mode0Timer.SecondsSinceLastReset()
			)
			{
				FilterText.GrabFocus();
			}
		}
		else
		{
			FilterText.ReleaseFocus();
		}

		//Get rid of '-' and '='
		{
			char str[2048];
			strncpy(str,FilterText.GetString(),sizeof(str)-1);
			str[sizeof(str)-1]='\0';

			bool delta=false;
			int len=strlen(str);
			while
			(
				len>0 &&
				len<sizeof(str)-1 &&
				(
					str[len-1]=='-' ||
					str[len-1]=='=' ||
					str[len-1]==',' ||
					str[len-1]=='.' ||
					str[len-1]=='/'
				)
			)
			{
				delta=true;
				str[len-1]='\0';
				len--;
			}

			if(delta)
			{
				FilterText.SetString(str);
			}
		}
		
		if(strcmp(FilterText.GetString(),FilterTextMostRecent)!=0)
		{
			filterDelta=true;
			strncpy(FilterTextMostRecent,FilterText.GetString(),sizeof(FilterTextMostRecent));
			FilterTextMostRecent[sizeof(FilterTextMostRecent)-1]='\0';
			FilterTextMostRecentBPM = (int)floorf(BPMMaster+0.5f);

			char oldSelection[2048];
			if(GetHighlightedNameDisplayed())
			{
				strncpy
				(
					oldSelection,
					GetHighlightedNameDisplayed(),
					sizeof(oldSelection)-1
				);
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
				int len = (int)strlen(item);
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

			//Filter text changed!
			if(UpdateFilterListViaThread)
			{
				DatabaseFilter.Assign(UpdateFilterListDatabaseFilterNext);
				UpdateFilterListResetHighlightedRow=false;
				UpdateFilterListDesiredSelection[0]='\0';
			}
			else
			{
				GetEntryListFromFilterDance(oldSelection);
			}
		}

		if
		(
			GetInput().FileMarkUnopened(target) &&
			DatabaseFilteredEntries.empty()==false
		)
		{
			DatabaseFilteredEntries[ListSelector.GetHighlightedRow()]->AlreadyPlayed=false;
		}

		int fileSelect = GetInput().FileSelect(target);
		if(EncodeEveryTrack)
		{
			if(DatabaseFilteredEntries[ListSelector.GetHighlightedRow()]->AlreadyPlayed)
			{
				if(ListSelector.GetHighlightedRow()==EncodeEveryTrackIndex)
				{
					EncodeEveryTrackIndex++;
				}
			}
			else
			{
				fileSelect = (EncodeEveryTrackIndex==ListSelector.GetHighlightedRow());
			}
		}
		if
		(
			fileSelect > 0 &&
			DatabaseFilteredEntries.empty()==false
		)
		{
			const int fileSelectIndex = ListSelector.GetHighlightedRow();
			bool targetIsDir=DatabaseFilteredEntries[fileSelectIndex]->IsDir;

			if
			(
				targetIsDir==false &&
				GetHighlightedPath()
			)
			{
				DeriveSoundStrings();

				WhiteFactor=1.0f;
				NoiseFactor=1.0f;
				NoiseFactorVideo=1.0f;
				FilterText.ReleaseFocus();

				DatabaseFilteredEntries[ListSelector.GetHighlightedRow()]->AlreadyPlayed=true;

				//Exile any active MetadataFileToRams to Death Row
				if(MetadataFileToRam)
				{
					MetadataFileToRamDeathRow.push_back(MetadataFileToRam);
					MetadataFileToRam=NULL;
				}

				if(Channel!=-1)
				{
					if(Sound)
					{
						TurntableObj* otherTT = GetOtherTT();
						bool bpmAvailable=false;
						for(int a=0;a<Savepoints.size();a++)
						{
							if(Savepoints[a].Seconds>=0)
							{
								if(Savepoints[a].BPM>0)
								{
									bpmAvailable=true;
								}
							}
						}

						if
						(
							(
								LGL_MidiClockBPM()<=0 &&
								(
									otherTT->Mode!=2 ||
									otherTT->GetFinalSpeed()==0.0f ||
									otherTT->BPMAvailable()==false
								)
							) ||
							bpmAvailable==false
						)
						{
							float savepointSeconds=GetCurrentSavepointSeconds();
							if
							(
								savepointSeconds>=0.0f &&
								Sound->GetLengthSeconds()-1.0f > savepointSeconds
							)
							{
								Sound->SetPositionSeconds(Channel,savepointSeconds);
								Sound->SetSpeed(Channel,0);
							}
							else
							{
								Sound->Stop(Channel);
								Channel=-1;
							}
							PauseMultiplier=0;
						}
						else
						{
							PauseMultiplier=1;
						}
					}
				}

				Mode1Timer.Reset();
				Mode=1;

				return;
			}
			else if
			(
				targetIsDir &&
				GetHighlightedPath() &&
				fileSelect != 2
			)
			{
				char targetPath[2048];
				targetPath[0]='\0';
				strcpy(targetPath,GetHighlightedPath());
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

				FilterTextMostRecent[0]='\0';
				FilterTextMostRecentBPM = (int)floorf(BPMMaster+0.5f);
				FilterText.SetString("");
				char oldDir[2048];
				strcpy(oldDir,&(strrchr(DatabaseFilter.GetDir(),'/')[1]));

				DatabaseFilter.SetDir(targetPath);
				DatabaseFilter.SetPattern(FilterText.GetString());

				//Changing Directories!
				if(UpdateFilterListViaThread)
				{
					DatabaseFilter.Assign(UpdateFilterListDatabaseFilterNext);
					UpdateFilterListResetHighlightedRow=true;
					UpdateFilterListDesiredSelection[0]='\0';
				}
				else
				{
					GetEntryListFromFilterDance(NULL);
				}

				FilterText.SetString();
				NoiseFactor=1.0f;
				WhiteFactor=1.0f;

				if(targetPathIsDotDot)
				{
					ListSelectorToString(oldDir);
				}
			}
		}

		//CAN CRASH. Disabled until fixed.
		if(0 && GetInput().FileRefresh(target))
		{
			//
		}

		if
		(
			Mode==0 &&
			LGL_MouseX()>=ListSelector.GetWindowScopeLeft() &&
			LGL_MouseX()<=ListSelector.GetWindowScopeRight() &&
			LGL_MouseY()>=ListSelector.GetWindowScopeBottom() &&
			LGL_MouseY()<=ListSelector.GetWindowScopeTop()
		)
		{
			GetInputMouse().SetHoverElement(GUI_ELEMENT_FILE_SELECT);
			if(LGL_MouseStroke(LGL_MOUSE_LEFT))
			{
				GetInputMouse().SetFileSelectNext();
			}
		}

		ListSelector.ScrollHighlightedRow
		(
			GetInput().FileScroll(target)
		);

		if(EncodeEveryTrack)
		{
			if(EncodeEveryTrackIndex>(int)DatabaseFilteredEntries.size())
			{
				EncodeEveryTrack=false;
			}
			else
			{
				ListSelector.SetHighlightedRow(EncodeEveryTrackIndex);
			}
		}

		//Handle video previews
		{
			if
			(
				Video->GetUserString() &&
				GetHighlightedNameDisplayed() &&
				strcmp(Video->GetUserString(),GetHighlightedNameDisplayed())!=0
			)
			{
				Video->SetUserString(GetHighlightedNameDisplayed());

				std::vector<const char*> pathAttempts = listVideoSearchPaths
				(
					GetHighlightedPath()
				);
				Video->InvalidateAllFrameBuffers();
				Video->SetVideo(pathAttempts);
				if(Channel==-1)
				{
					Video->SetTime(30.0f+LGL_RandFloat(0.0f,30.0f));
				}
				for(int a=0;a<pathAttempts.size();a++)
				{
					delete pathAttempts[a];
				}
				pathAttempts.clear();
			}

			if
			(
				Sound &&
				Channel!=-1
			)
			{
				Video->SetTime(GetTimeSeconds());
			}
			else
			{
				Video->SetTime(Video->GetTime()+LGL_SecondsSinceLastFrame());
			}
		}

		//Handle audio previews
		{
			if(Sound==NULL)
			{
				PauseMultiplier=0;
				if(DatabaseFilteredEntries[ListSelector.GetHighlightedRow()]->IsDir==false)
				{
					AttemptToCreateSound();
				}
			}
			else
			{
				if(Sound->ReadyForDelete())
				{
					delete Sound;
					Sound=NULL;

					if(FindAudioPathThread)
					{
						LGL_ThreadWait(FindAudioPathThread);
						FindAudioPathThread=NULL;
					}
				}

				if(Sound)
				{
					bool prepareForDelete=false;

					if(Sound->IsUnloadable())
					{
						prepareForDelete=true;
						DatabaseFilteredEntries[ListSelector.GetHighlightedRow()]->Loadable=false;
					}
					else if(Sound->PreparingForDelete()==false)
					{
						if
						(
							Channel==-1 &&
							GetHighlightedNameDisplayed() &&
							strcmp(GetHighlightedNameDisplayed(),DenyPreviewNameDisplayed)!=0
						)
						{
							DenyPreviewNameDisplayed[0]='\0';

							float targetSeconds = LGL_Max(0.0f,GetSecondsToSync());
							if(Sound->GetLengthSeconds()>targetSeconds+1.0f)
							{
								Channel=Sound->Play(0,true,0);
								UpdateSoundFreqResponse();
								if(targetSeconds>=0)
								{
									Sound->SetPositionSeconds(Channel,targetSeconds);
								}
								FinalSpeed=(Pitchbend+MixerNudge)+Nudge;
								Sound->SetSpeed(Channel,FinalSpeed,true);
							}
						}
						if
						(
							GetHighlightedNameDisplayed()==NULL ||
							strcmp(SoundSrcNameDisplayed,GetHighlightedNameDisplayed())!=0
						)
						{
							prepareForDelete=true;
						}
					}

					if(prepareForDelete)
					{
						Sound->PrepareForDelete();
						Channel=-1;
					}
					else
					{
						if(Channel>=0)
						{
							if
							(
								savepointIndexDelta ||
								Focus!=FocusPrev ||
								PauseMultiplier==0
							)
							{
								float targetSeconds = GetSecondsToSync();
								if
								(
									targetSeconds>=0 &&
									targetSeconds<Sound->GetLengthSeconds()-1.0f
								)
								{
									Sound->SetPositionSeconds(Channel,targetSeconds);

									PauseMultiplier=1;
								}
							}

							FinalSpeed=PauseMultiplier*(Pitchbend+MixerNudge)+Nudge;
							Sound->SetSpeed(Channel,FinalSpeed,true);
						}
					}
				}
			}
		}
	}
	else if(Mode==1)
	{
		//Decoding...

		if
		(
			Sound &&
			Sound->IsUnloadable()
		)
		{
			DatabaseFilteredEntries[ListSelector.GetHighlightedRow()]->Loadable=false;
		}
		bool loadable=DatabaseFilteredEntries[ListSelector.GetHighlightedRow()]->Loadable;
		if
		(
			loadable==false ||
			GetInput().WaveformEject(target) ||
			Mode1Timer.SecondsSinceLastReset() > (EncodeEveryTrack ? 20.0f : 5.0f)
		)
		{
			//Abort load. Select new track.
			LGL_DrawLogWrite("!dvj::DeleteSound|%i\n",Which);
			if(Sound)
			{
				Sound->PrepareForDelete();
			}
			Mode=3;
			ListSelector.SetBadRowFlash();
			if(LGL_VideoDecoder* dec = GetVideo())
			{
				LGL_DrawLogWrite("!dvj::DeleteVideo|%s\n",dec->GetPath());
			}
			if(EncodeEveryTrack)
			{
				EncodeEveryTrackIndex++;
			}
			return;
		}
	}
	else if(Mode==2)
	{
		VolumeKill = GetInput().WaveformGainKill(target);

		if
		(
			LGL_MouseX()>=WaveformLeft &&
			LGL_MouseX()<=WaveformRight &&
			LGL_MouseY()>=WaveformBottom &&
			LGL_MouseY()<=WaveformTop
		)
		{
			GetInputMouse().SetHoverElement(GUI_ELEMENT_WAVEFORM);
			if(LGL_MouseStroke(LGL_MOUSE_LEFT))
			{
				GetInputMouse().SetDragTarget((Which==0) ? TARGET_TOP : TARGET_BOTTOM);
				GetInputMouse().SetDragElement(GUI_ELEMENT_WAVEFORM);
			}
		}

		float rewindFFFactor=GetInput().WaveformRewindFF(target);
		float recordSpeed=GetInput().WaveformRecordSpeed(target);
		bool recordHold=GetInput().WaveformRecordHold(target);
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
			Sound->SetSpeed(Channel,Pitchbend*PauseMultiplier*ReverseMultiplier,true);
			RecordSpeedAsZeroUntilZero=true;
		}
		RecordHoldLastFrame=recordHold;

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
				fastSeekVolumeMultiplier=
					(0.0f+normalVolFactor)*1.00f+
					(1.0f-normalVolFactor)*0.25f;
				if(Looping()==false)
				{
					LoopAlphaSeconds=-1.0f;
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
				Sound->SetSpeed(Channel,Pitchbend*PauseMultiplier*ReverseMultiplier,true);
				SetSmoothWaveformScrollingSample(Sound->GetPositionSamples(Channel));
				SmoothWaveformScrollingSampleRate=SmoothWaveformScrollingSampleRateRemembered;
				SmoothWaveformScrollingSampleExactUponDifferent=Sound->GetPositionSamples(Channel);;
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
					Sound->SetWarpPoint(Channel);
				}
				RecordScratch=true;
				float driveFactor=GetInput().WaveformRecordHold(target) ? 0.0f : 1.0f;//LGL_Min(RecordHoldReleaseTimer.SecondsSinceLastReset()*4.0f,1.0f);

				float driveSpeed = 
					(1.0f-driveFactor) * recordSpeed +
					(0.0f+driveFactor) * (Pitchbend*PauseMultiplier*ReverseMultiplier) * (RewindFF ? 0.0f : 1.0f);

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
		VolumeInvertBinary=GetInput().WaveformVolumeInvert(target);

		if(GetBPM()>0)
		{
			Sound->SetRhythmicInvertProperties
			(
				Channel,
				GetBeginningOfCurrentMeasureSeconds(),
				GetQuantizePeriodSeconds()
			);
			if(GetInput().WaveformRhythmicVolumeInvert(target))
			{
				RhythmicVolumeInvert=true;
			}
			else
			{
				RhythmicVolumeInvert=false;
			}
		}
		else
		{
			RhythmicVolumeInvert=false;
		}
		Sound->SetRhythmicVolumeInvert(Channel,RhythmicVolumeInvert);

		RhythmicSoloInvert = BPMAvailable() && GetInput().WaveformRhythmicVolumeInvertOther(target);

		//Savepoints & Jumping
		if(AudioInputMode==false)
		{
			//Set Savepoint?
			if
			(
				GetInput().WaveformSavepointSet(target)
			)
			{
				if(SavepointIndexAtPlus())
				{
					Savepoints[SavepointIndex].Seconds=GetTimeSeconds();
					Savepoints[SavepointIndex].UnsetFlashPercent=1.0f;
					SortSavepoints();
					SaveMetadataNew();
				}
/*
				if(Savepoints[SavepointIndex].Seconds<=0.0f)
				{
					Savepoints[SavepointIndex].Seconds=GetTimeSeconds();
					Savepoints[SavepointIndex].UnsetFlashPercent=1.0f;
					SaveMetadataNew();
				}
*/
			}

			//Savepoint BPM
			if
			(
				GetCurrentSavepoint() &&
				GetCurrentSavepoint()->Seconds>=0.0f
			)
			{
				//Set Savepoint's BPM at needle?
				if
				(
					GetInput().WaveformSavepointSetBPMAtNeedle(target)
				)
				{
					if(Savepoints[SavepointIndex].Seconds!=-1.0f)
					{
						if
						(
							GetTimeSeconds() >
							Savepoints[SavepointIndex].Seconds
						)
						{
							Savepoints[SavepointIndex].BPM=GetBPMFromDeltaSeconds
							(
								GetTimeSeconds()-
								Savepoints[SavepointIndex].Seconds
							);
						}
						else if
						(
							GetTimeSeconds() <
							Savepoints[SavepointIndex].Seconds
						)
						{
							Savepoints[SavepointIndex].BPM=BPM_UNDEF;
							Savepoints[SavepointIndex].UnsetNoisePercent=1.0f;
						}
						else
						{
							Savepoints[SavepointIndex].BPM=BPM_UNDEF;//NONE;
						}
						Savepoints[SavepointIndex].UnsetFlashPercent=1.0f;
						SaveMetadataNew();
					}
				}

				//Set Savepoint as BPM_UNDEF?
				if(GetInput().WaveformSavepointSetBPMUndef(target))
				{
					Savepoints[SavepointIndex].BPM=BPM_UNDEF;
					SaveMetadataNew();
				}

				//Set Savepoint as BPM_NONE?
				if(GetInput().WaveformSavepointSetBPMNone(target))
				{
					Savepoints[SavepointIndex].BPM=BPM_NONE;
					SaveMetadataNew();
				}
			}

			//Unset Savepoint?
			if
			(
				GetInput().WaveformSavepointUnsetPercent(target)==1.0f &&
				Savepoints[SavepointIndex].Seconds!=-1.0f &&
				InputUnsetDebounce==false
			)
			{
				InputUnsetDebounce=true;
				if(GetInputTester().GetEnable())
				{
					printf("BAD UNSET!! Input: %.2f\n",GetInput().WaveformSavepointUnsetPercent(target));
					printf("BAD UNSET!! InputTester: %.2f\n",GetInputTester().WaveformSavepointUnsetPercent(target));
					assert(GetInputTester().GetEnable()==false);
				}
				Savepoints[SavepointIndex].Seconds=-1.0f;
				Savepoints[SavepointIndex].BPM=BPM_UNDEF;
				Savepoints[SavepointIndex].UnsetFlashPercent=1.0f;
				//Savepoints.erase((std::vector<SavepointObj>::iterator)(&(Savepoints[SavepointIndex])));
				if(SavepointIndex>Savepoints.size()-1)
				{
					//SavepointIndex=LGL_Max(0,Savepoints.size()-1);
				}
				SavepointIndex=0;
				LGL_LoopCounterAlpha();
				while(SavepointIndexAtPlus()==false)
				{
					LGL_LoopCounterDelta();
					SavepointIndex++;
				}
				LGL_LoopCounterOmega();
				SortSavepoints();
				SaveMetadataNew();
			}

			if(InputUnsetDebounce)
			{
				if(GetInput().WaveformSavepointUnsetPercent(target)<=0.0f)
				{
					InputUnsetDebounce=false;
				}
			}

			//Update Flash/Noise Percents
			for(int a=0;a<Savepoints.size();a++)
			{
				Savepoints[a].UnsetFlashPercent=LGL_Max(0,Savepoints[a].UnsetFlashPercent-4.0f*LGL_SecondsSinceLastFrame());
				Savepoints[a].UnsetNoisePercent=LGL_Max(0,Savepoints[a].UnsetNoisePercent-2.0f*LGL_SecondsSinceLastFrame());
				if
				(
					a==SavepointIndex &&
					InputUnsetDebounce==false
				)
				{
					Savepoints[a].UnsetNoisePercent=LGL_Max
					(
						Savepoints[a].UnsetNoisePercent,
						LGL_Min(1,(Savepoints[a].Seconds>-1.0f) ? (GetInput().WaveformSavepointUnsetPercent(target)*2.0f) : 0)
					);
				}
			}

			//Shift current savepoint?
			if
			(
				Savepoints[SavepointIndex].Seconds!=-1.0f
			)
			{
				candidate=GetInput().WaveformSavepointShift(target);
				if(candidate!=0.0f)
				{
					Savepoints[SavepointIndex].Seconds=
						LGL_Clamp
						(
							0.0f,
							Savepoints[SavepointIndex].Seconds+candidate,
							Sound->GetLengthSeconds()-0.01f
						);
					SaveMetadataNew();
				}
			}

			if
			(
				Savepoints[SavepointIndex].Seconds!=-1.0f
			)
			{
				candidate=GetInput().WaveformSavepointShiftBPM(target);
				if(candidate!=0.0f)
				{
					if
					(
						Savepoints[SavepointIndex].BPM==BPM_UNDEF ||
						Savepoints[SavepointIndex].BPM==BPM_NONE
					)
					{
						Savepoints[SavepointIndex].BPM=150.0f;
					}

					if(Savepoints[SavepointIndex].BPM>0)
					{
						Savepoints[SavepointIndex].BPM+=candidate;
						LGL_LoopCounterAlpha();
						while(Savepoints[SavepointIndex].BPM<100.0f)
						{
							LGL_LoopCounterDelta();
							Savepoints[SavepointIndex].BPM*=2.0f;
						}
						LGL_LoopCounterOmega();
						LGL_LoopCounterAlpha();
						while(Savepoints[SavepointIndex].BPM>=200.0f)
						{
							LGL_LoopCounterDelta();
							Savepoints[SavepointIndex].BPM/=2.0f;
						}
						LGL_LoopCounterOmega();
					}
					SaveMetadataNew();
				}
			}

			//Shift all savepoints

			//Handle JumpNow
			if
			(
				GetInput().WaveformSavepointJumpNow(target) &&
				Savepoints[SavepointIndex].Seconds!=-1.0f
			)
			{
				if(Looping())
				{
					LoopAlphaSeconds=-1.0f;
					LoopActive=false;
					LoopThenRecallActive=false;
					Sound->SetWarpPoint(Channel);
				}
				double savepointSeconds=Savepoints[SavepointIndex].Seconds;
				if(savepointSeconds < 0.0f)
				{
					savepointSeconds=0;
				}

				if(Sound->GetLengthSeconds()>=savepointSeconds)
				{
					Sound->SetPositionSeconds(Channel,savepointSeconds);
					SetSmoothWaveformScrollingSample(savepointSeconds*Sound->GetHz());
				}
			}

			//Handle JumpAtMeasure
			if
			(
				GetInput().WaveformSavepointJumpAtMeasure(target) &&
				Savepoints[SavepointIndex].Seconds!=-1.0f &&
				GetBPM()>0 &&
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
					}
				}
				if(Sound->GetWarpPointSecondsAlpha(Channel)>0.0f)
				{
					Sound->SetWarpPoint(Channel);
				}
				else
				{
					double savepointSeconds=Savepoints[SavepointIndex].Seconds;
					if(savepointSeconds >= 0.0f)
					{
						//Jump at start of next measure
						double beatStart=GetBPMAnchorMeasureSeconds();
						double measureLength=GetMeasureLengthSeconds();
						LGL_LoopCounterAlpha();
						while(beatStart>0)
						{
							LGL_LoopCounterDelta();
							beatStart-=measureLength;
						}
						LGL_LoopCounterOmega();

						double savepointSecondsQuantized=beatStart;
						double closest=99999.0;
						for(double test=beatStart;test<savepointSeconds+2*measureLength;test+=0.25*measureLength)
						{
							if(fabsf(test-savepointSeconds)<closest)
							{
								closest=fabsf(test-savepointSeconds);
								savepointSecondsQuantized=test;
							}
						}
						double nowSeconds=GetTimeSeconds();
						double savepointMeasureStart=beatStart;
						LGL_LoopCounterAlpha();
						while(savepointMeasureStart+measureLength<savepointSecondsQuantized)
						{
							LGL_LoopCounterDelta();
							savepointMeasureStart+=measureLength;
						}
						LGL_LoopCounterOmega();

						//double savepointSecondsIntoMeasure=savepointSecondsQuantized-savepointMeasureStart;
						double nowMeasureStart=beatStart;

						//Loop optimization
						double dist = nowSeconds - (nowMeasureStart+measureLength);
						double delta = floorf(dist/measureLength);
						nowMeasureStart += delta*measureLength;

						LGL_LoopCounterAlpha();
						while(nowMeasureStart+measureLength<nowSeconds)
						{
							LGL_LoopCounterDelta();
							nowMeasureStart+=measureLength;
						}
						LGL_LoopCounterOmega();

						double timeToWarp=nowMeasureStart+measureLength;//+savepointSecondsIntoMeasure;
						LGL_LoopCounterAlpha();
						while(timeToWarp<nowSeconds)
						{
							LGL_LoopCounterDelta();
							timeToWarp+=measureLength;
						}
						LGL_LoopCounterOmega();

						LGL_LoopCounterAlpha();
						for(;;)
						{
							LGL_LoopCounterDelta();
							bool ret = Sound->SetWarpPoint
							(
								Channel,
								timeToWarp,
								savepointSecondsQuantized
							);

							if(ret)
							{
								break;
							}
							else
							{
								timeToWarp+=measureLength;
							}
						}
						LGL_LoopCounterOmega();
					}
				}
			}

			//Handle JumpToPercent
			{
				candidate = GetInput().WaveformJumpToPercent(target);
				if(candidate!=-1.0f)
				{
					if(Sound->IsLoaded())
					{
						double quantizePeriodSeconds=GetQuantizePeriodSeconds();
						bool lockWarp=false;
						if(candidate>1.0f)
						{
							lockWarp=true;
							candidate-=1.0f;
						}
						candidate=LGL_Clamp(0.0f,candidate,1.0f);
						float candidateSeconds=Sound->GetLengthSeconds()*candidate;
						double jumpAlphaSeconds=GetTimeSeconds();
						double jumpOmegaSeconds=candidateSeconds;
						if(BPMAvailable() && PauseMultiplier!=0.0f)
						{
							double currentMeasureStart=GetBeginningOfCurrentMeasureSeconds();
							double candidateMeasureStart=GetBeginningOfArbitraryMeasureSeconds(candidateSeconds);

							jumpAlphaSeconds=currentMeasureStart;
							jumpOmegaSeconds=candidateMeasureStart;
							LGL_LoopCounterAlpha();
							for(;;)
							{
								LGL_LoopCounterDelta();
								float posSeconds = GetTimeSeconds();
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
							LGL_LoopCounterOmega();
						}
						//Sound->SetPositionSeconds(Channel,candidateSeconds);

						if
						(
							jumpAlphaSeconds>=0.0f &&
							jumpOmegaSeconds>=0.0f &&
							jumpOmegaSeconds<=Sound->GetLengthSeconds()-5.0f
						)
						{
							LGL_LoopCounterAlpha();
							for(;;)
							{
								LGL_LoopCounterDelta();
								bool ret=Sound->SetWarpPoint
								(
									Channel,
									jumpAlphaSeconds,
									jumpOmegaSeconds,
									false,
									lockWarp
								);

								if(ret)
								{
									break;
								}
								else
								{
									jumpAlphaSeconds += quantizePeriodSeconds;
									jumpOmegaSeconds += quantizePeriodSeconds;
								}
							}
							LGL_LoopCounterOmega();
						}
					}
				}
			}
		}

		//BPM
		if(GetPaused())
		{
			GetInput().WaveformHintBPMCandidate(target,GetBPM());
			if(const char* candStr = GetInput().WaveformBPMCandidate(target))
			{
				Pitchbend=1.0f;
				float candidate = candStr ? atof(candStr) : 0.0f;
				if(candidate>=100 && candidate<=200)
				{
					const float secondsPerBeat = (60.0f/candidate);
					Sound->SetPositionSeconds(Channel,Savepoints[SavepointIndex].Seconds+secondsPerBeat*4*16);
				}
				else if(GetCurrentSavepointSeconds()>=0.0f)
				{
					Sound->SetPositionSeconds(Channel,GetCurrentSavepointSeconds());
				}
			}

			float bpmSet = GetInput().WaveformBPM(target);
			if(bpmSet>0.0f)
			{
				if(Savepoints[SavepointIndex].Seconds==-1.0f)
				{
					Savepoints[SavepointIndex].Seconds=GetTimeSeconds();
				}

				float secondsPerBeat = 60.0f/bpmSet;
				Savepoints[SavepointIndex].BPM=bpmSet;
				Sound->SetPositionSeconds(Channel,Savepoints[SavepointIndex].Seconds+secondsPerBeat*4*16);

				SaveMetadataNew();
			}
		}
		else
		{
			GetInput().WaveformClearBPMCandidate(target);
		}

		//Pitch
		{
			Nudge=GetInput().WaveformNudge(target);

			candidate = GetInput().WaveformPitchbend(target);
			float candidateXponent = GetInputXponent().WaveformPitchbend(target);
			if(candidate!=0.0f)
			{
				if
				(
					candidateXponent==0.0f ||
					PitchbendLastSetByXponentSlider ||
					fabsf(Pitchbend-candidate)<0.0025f
				)
				{
					Pitchbend=candidate;
					if(candidateXponent!=0.0f)
					{
						PitchbendLastSetByXponentSlider=true;
					}
				}
			}
			candidate=GetInput().WaveformPitchbendDelta(target);
			if(candidate!=0.0f)
			{
				Pitchbend=LGL_Clamp
				(
					0.01f,
					Pitchbend+candidate,
					4.0f
				);
				PitchbendLastSetByXponentSlider=false;
			}
		}

		//Glitch
		if(0 && GetInput().WaveformStutter(target))	//DEPRECATED & INACTIVE
		{
			bool glitchDuoPrev=GlitchDuo;
			GlitchDuo=true;

			float fractionOfBeat=1.0f;

			GlitchPitch = 2*LGL_Max(0.0f,GetInput().WaveformStutterPitch(target));
			float glitchSpeed = LGL_Max(0.0f,GetInput().WaveformStutterSpeed(target));

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
		const int exponentMin=-9;
		const int exponentMax=6;
		const int exponentAll=9999;

		bool loopActiveLastFrame = LoopActive;
		bool loopThenRecallActiveLastFrame = LoopThenRecallActive;

		if(LoopActive && Sound->GetWarpPointSecondsAlpha(Channel)<0)
		{
			LoopActive=false;
		}
		bool loopToggle = GetInput().WaveformLoopToggle(target);
		LoopActive = 
		(
			(loopToggle ? !LoopActive : LoopActive) &&
			RhythmicVolumeInvert==false
		);
		if
		(
			LoopActive==false &&
			loopActiveLastFrame &&
			QuantizePeriodMeasuresExponent==exponentAll
		)
		{
			QuantizePeriodMeasuresExponent=QuantizePeriodMeasuresExponentRemembered;
		}
		LoopThenRecallActive =
		(
			GetInput().WaveformLoopThenRecallActive(target) &&
			Sound->GetWarpPointIsLocked(Channel)==false &&
			RhythmicVolumeInvert==false
		);

		bool loopChanged=
			(
				loopActiveLastFrame != LoopActive ||
				loopThenRecallActiveLastFrame != LoopThenRecallActive
			);

		if(LoopThenRecallActive != loopThenRecallActiveLastFrame)
		{
			if
			(
				LoopThenRecallActive
			)
			{
				LoopActive=false;
				Diverge();
			}
			else
			{
				Recall();
			}
		}

		if(BPMAvailable())
		{
			//Looping with BPM

			if
			(
				GetInput().WaveformLoopAll(target) &&
				Sound->IsLoaded()
			)
			{
				if(QuantizePeriodMeasuresExponent!=exponentAll)
				{
					QuantizePeriodMeasuresExponentRemembered=QuantizePeriodMeasuresExponent;
				}
				QuantizePeriodMeasuresExponent=exponentAll;
				loopChanged=true;
				LoopActive=true;
			}

			if(GetInput().WaveformQuantizationPeriodHalf(target))
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

			if(GetInput().WaveformQuantizationPeriodDouble(target))
			{
				QuantizePeriodMeasuresExponent=LGL_Min(QuantizePeriodMeasuresExponent+1,exponentMax);
				loopChanged=true;
			}

			candidate=GetInput().WaveformLoopMeasuresExponent(target);
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
				}
				else
				{
					float deltaSeconds = GetQuantizePeriodSeconds();
					if(QuantizePeriodMeasuresExponent==exponentAll)
					{
						if(Savepoints.size()>0)
						{
							LoopAlphaSeconds = Savepoints[0].Seconds;
						}
						else
						{
							LoopAlphaSeconds = GetBPMFirstBeatSeconds();
						}

						if
						(
							Savepoints.size()>1 &&
							Savepoints[1].Seconds!=-1.0f
						)
						{
							for(int a=Savepoints.size()-1;a>=0;a--)
							{
								if(Savepoints[a].Seconds!=-1.0f)
								{
									LoopOmegaSeconds = Savepoints[a].Seconds;
									break;
								}
							}
						}
						else
						{
							LoopOmegaSeconds=LoopAlphaSeconds+GetQuantizePeriodSeconds();
						}

						if(LoopOmegaSeconds<LoopAlphaSeconds)
						{
							LoopOmegaSeconds=LoopAlphaSeconds;
						}
					}
					else
					{
						LoopAlphaSeconds = GetBeginningOfCurrentMeasureSeconds(pow(2,QuantizePeriodMeasuresExponent));
						/*
						for(int q=0;;q++)
						{
							float posSeconds = GetTimeSeconds();
							if(LoopAlphaSeconds + deltaSeconds < posSeconds)
							{
								LoopAlphaSeconds += deltaSeconds;
							}
							else
							{
								break;
							}

							if(q==99)
							{
								printf("Former for(;;) loop takes too many iterations!\n");
								LGL_AssertIfDebugMode();
							}
						}
						*/

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

					LGL_LoopCounterAlpha();
					for(int z=0;z<100;z++)
					{
						LGL_LoopCounterDelta();
						bool ret=false;

						if(z>90 && LGL_GetDebugMode())
						{
							printf
							(
								"Attempt %i: %.4f vs %.4f\n",
								z,
								LoopOmegaSeconds,
								Sound->GetPositionSeconds(Channel)
							);
						}

						if(1)//LoopOmegaSeconds > Sound->GetPositionSeconds(Channel)+0.1f)
						{
							if(z>90 && LGL_GetDebugMode())
							{
								printf
								(
									"[%i]: Trying to SetWarpPoint... (Locked? %i)\n",
									z,
									Sound->GetWarpPointIsLocked(Channel)
								);
							}
							ret=Sound->SetWarpPoint
							(
								Channel,
								LoopOmegaSeconds,
								LoopAlphaSeconds,
								true
							);
						}

						if(ret)
						{
							break;
						}
						else
						{
							if(Sound->GetWarpPointIsLocked(Channel))
							{
								break;
							}
							/*
							else
							{
								printf("Warning! When setting WarpPoint, ret was false, but GetWarpPointIsLocked() was also false. How?!\n");
								LGL_AssertIfDebugMode();
							}
							*/
							float lastMeasure = GetBPMLastMeasureSeconds();

							float posSeconds = GetTimeSeconds();
							posSeconds+=z*0.005f;
							float dist = posSeconds - LoopOmegaSeconds;
							int mult = LGL_Max(0,ceilf(dist/deltaSeconds));

							LoopAlphaSeconds+=deltaSeconds*mult;
							LoopOmegaSeconds=LGL_Min
							(
								LoopAlphaSeconds+deltaSeconds,
								lastMeasure
							);

							if(LoopAlphaSeconds>lastMeasure)
							{
								break;
							}

							if(z>90 && LGL_GetDebugMode())
							{
								printf("[%i]: %.2f => %.2f (%.2f) (%.6f)\n",
									z,
									LoopAlphaSeconds,
									LoopOmegaSeconds,
									deltaSeconds,
									Sound->GetLengthSeconds()
								);
							}
						}

						if(z==99)
						{
							printf("Warning! Failed to set warp point, or early-out!\n");
							LoopActive=false;
							//LGL_AssertIfDebugMode();
							break;
						}
					}
					LGL_LoopCounterOmega();
				}
			}
		}
		else if(BPMAvailable()==false)
		{
			//Looping without BPM
			if(loopChanged)
			{
				if(LoopAlphaSeconds==-1.0f && Looping())
				{
					LoopAlphaSeconds=GetTimeSeconds();
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
				GetInput().WaveformLoopAll(target) &&
				Sound->IsLoaded()
			)
			{
				LoopAlphaSeconds=0.0f;
				QuantizePeriodNoBPMSeconds=secondsMax;
				loopChanged=true;
				LoopActive=true;
			}
			if(GetInput().WaveformLoopSecondsLess(target))
			{
				QuantizePeriodNoBPMSeconds=LGL_Max
				(
					QuantizePeriodNoBPMSeconds-
					QuantizePeriodNoBPMSeconds*2.0f*LGL_SecondsSinceLastFrame(),
					secondsMin
				);
				loopChanged=true;
			}

			if(GetInput().WaveformLoopSecondsMore(target))
			{
				QuantizePeriodNoBPMSeconds=LGL_Min
				(
					QuantizePeriodNoBPMSeconds+
					QuantizePeriodNoBPMSeconds*2.0f*LGL_SecondsSinceLastFrame(),
					secondsMax
				);
				loopChanged=true;
			}

			candidate=GetInput().WaveformLoopMeasuresExponent(target);
			if(candidate!=WAVEFORM_LOOP_MEASURES_EXPONENT_NULL)
			{
				QuantizePeriodNoBPMSeconds=LGL_Clamp(secondsMin,candidate,secondsMax);
				loopChanged=true;
			}

			if(loopChanged)
			{
				if(Looping())
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
		int autoDivergeRecall = GetInput().WaveformAutoDivergeRecall(target);
		if(autoDivergeRecall==-2)
		{
			if(AutoDivergeRecallActive==false)
			{
				Diverge();
			}
			else
			{
				Recall();
			}
			AutoDivergeRecallActive=!AutoDivergeRecallActive;
		}
		else if(autoDivergeRecall==-1)
		{
			if(AutoDivergeRecallActive)
			{
				Recall();
			}
			AutoDivergeRecallActive=false;
		}
		else if (autoDivergeRecall==0)
		{
			//
		}
		else
		{
			if(AutoDivergeRecallActive==false)
			{
				Diverge();
			}
			AutoDivergeRecallActive=true;
		}

		//Rhythmic Volume Invert vs QuantizePeriodMeasuresExponent
		if(RhythmicVolumeInvert || RhythmicSoloInvert)
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

		if(GetInput().WaveformVideoSelectLow(target))
		{
			SelectNewVideoLow();
		}
		if(GetInput().WaveformVideoSelectHigh(target))
		{
			SelectNewVideoHigh();
		}

		bool toggle=GetInput().WaveformAudioInputToggle(target);
		if(toggle)
		{
			AudioInputMode=!AudioInputMode;
			SelectNewVideo();
		}

		bool next = GetInput().WaveformVideoAspectRatioNext(target);
		if(next)
		{
			AspectRatioMode=(AspectRatioMode+1)%3;
		}

		float newBright=GetInput().WaveformVideoBrightness(target);
		if(newBright!=-1.0f)
		{
			if
			(
				VideoBrightness==0.0f &&
				newBright>0.0f
			)
			{
				/*
				if(LGL_VideoDecoder* dec = GetVideo())
				{
					dec->ForcePreload();
				}
				*/
			}
			VideoBrightness=newBright;
		}
		VideoBrightness=LGL_Clamp
		(
			0.0f,
			VideoBrightness+GetInput().WaveformVideoBrightnessDelta(target),
			1.0f
		);

		newBright=GetInput().WaveformSyphonBrightness(target);
		if(newBright!=-1.0f)
		{
			SyphonBrightness=newBright;
		}
		SyphonBrightness=LGL_Clamp
		(
			0.0f,
			SyphonBrightness+GetInput().WaveformSyphonBrightnessDelta(target),
			1.0f
		);

		newBright=GetInput().WaveformOscilloscopeBrightness(target);
		if(newBright!=-1.0f)
		{
			OscilloscopeBrightness=newBright;
		}
		OscilloscopeBrightness=LGL_Clamp
		(
			0.0f,
			OscilloscopeBrightness+GetInput().WaveformOscilloscopeBrightnessDelta(target),
			1.0f
		);

		newBright=GetInput().WaveformFreqSenseBrightness(target);
		if(newBright==-1.0f)
		{
			if(GetInput().WaveformFreqSenseBrightnessDelta(target)!=0.0f)
			{
				newBright=LGL_Clamp
				(
					0.0f,
					FreqSenseBrightness+GetInput().WaveformFreqSenseBrightnessDelta(target),
					1.0f
				);
			}
		}
		if(newBright!=-1.0f)
		{
			if
			(
				FreqSenseBrightness==0.0f &&
				newBright>0.0f
			)
			{
				//VideoLo->ForcePreload();
				//VideoHi->ForcePreload();
			}
			float tmp = FreqSenseBrightness;
			FreqSenseBrightness=newBright;
			if
			(
				tmp==0.0f &&
				FreqSenseBrightness >= 0.0f &&
				(
					VideoLoPath[0]=='\0' ||
					VideoHiPath[0]=='\0'
				)
			)
			{
				SelectNewVideo();
			}
		}

		if
		(
			Focus &&
			GetInputMouse().GetHoverElement()==GUI_ELEMENT_VIDEO_FREQSENSE
		)
		{
			FreqSensePathBrightness=2.0f;
		}

		FreqSensePathBrightness=LGL_Max(0.0f,FreqSensePathBrightness-LGL_SecondsSinceLastFrame());

		//LEDs
		{
			//Group
			float newGroup=GetInput().WaveformFreqSenseLEDGroupFloat(target);
			if(newGroup==-1.0f)
			{
				float delta = GetInput().WaveformFreqSenseLEDGroupFloatDelta(target);
				if(delta!=0.0f)
				{
					newGroup=LGL_Clamp
					(
						0.0f,
						FreqSenseLEDGroupFloat+delta,
						1.0f
					);
				}
			}
			if(newGroup!=-1.0f)
			{
				newGroup=LGL_Clamp
				(
					0.0f,
					newGroup,
					1.0f
				);
				FreqSenseLEDGroupFloat=newGroup;
			}

			//Brightness
			newBright=GetInput().WaveformFreqSenseLEDBrightness(target);
			if(newBright==-1.0f)
			{
				float delta = GetInput().WaveformFreqSenseLEDBrightnessDelta(target);
				if(delta!=0.0f)
				{
					newBright=LGL_Clamp
					(
						0.0f,
						FreqSenseLEDBrightness[GetFreqSenseLEDGroupInt()]+delta,
						1.0f
					);
				}
			}
			if(newBright!=-1.0f)
			{
				newBright=LGL_Clamp
				(
					0.0f,
					newBright,
					1.0f
				);
				FreqSenseLEDBrightness[GetFreqSenseLEDGroupInt()]=newBright;
			}

			//Color Low/High
			for(int f=0;f<2;f++)
			{
				float& targetColor =
					(f==0) ?
					FreqSenseLEDColorScalarLow[GetFreqSenseLEDGroupInt()] :
					FreqSenseLEDColorScalarHigh[GetFreqSenseLEDGroupInt()];
				float newColor=
					(f==0) ?
					GetInput().WaveformFreqSenseLEDColorScalarLow(target) :
					GetInput().WaveformFreqSenseLEDColorScalarHigh(target);
				if(newColor==-1.0f)
				{
					float delta =
						(f==0) ?
						GetInput().WaveformFreqSenseLEDColorScalarLowDelta(target) :
						GetInput().WaveformFreqSenseLEDColorScalarHighDelta(target);
					if(delta!=0.0f)
					{
						newColor=LGL_Clamp
						(
							0.0f,
							targetColor+delta,
							1.0f
						);
					}
				}
				if(newColor!=-1.0f)
				{
					newColor=LGL_Clamp
					(
						0.0f,
						newColor,
						1.0f
					);
					targetColor=newColor;
				}
			}

			//Brightness Wash
			newBright=GetInput().WaveformFreqSenseLEDBrightnessWash(target);
			if(newBright==-1.0f)
			{
				float delta = GetInput().WaveformFreqSenseLEDBrightnessWashDelta(target);
				if(delta!=0.0f)
				{
					newBright=LGL_Clamp
					(
						0.0f,
						FreqSenseLEDBrightnessWash[GetFreqSenseLEDGroupInt()]+delta,
						1.0f
					);
				}
			}
			if(newBright!=-1.0f)
			{
				newBright=LGL_Clamp
				(
					0.0f,
					newBright,
					1.0f
				);
				FreqSenseLEDBrightnessWash[GetFreqSenseLEDGroupInt()]=newBright;
			}
		}

		float newRate=GetInput().WaveformVideoAdvanceRate(target);
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

			FinalSpeed=PauseMultiplier*ReverseMultiplier*(Pitchbend+MixerNudge)+Nudge;
		}

		Sound->SetStickyEndpoints(Channel,endpointsSticky);
		Sound->SetSpeed
		(
			Channel,
			FinalSpeed
		);

		//Pause
		if(GetInput().WaveformPauseToggle(target))
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
				Sound->SetSpeed(Channel,Pitchbend*PauseMultiplier*ReverseMultiplier,true);
			}
		}

		//Reverse
		{
			float reverseMultiplierPrev=ReverseMultiplier;
	 		ReverseMultiplier=GetInput().WaveformReverse(target) ? -1.0f : 1.0f;
			if(MixerNudge!=0.0f)
			{
				ReverseMultiplier=1.0f;
			}

			if(ReverseMultiplier!=reverseMultiplierPrev)
			{
				//Change our speed instantly.
				Sound->SetSpeed(Channel,Pitchbend*PauseMultiplier*ReverseMultiplier,true);

				if(reverseMultiplierPrev==1.0f)
				{
					Diverge();
				}
				else
				{
					Recall();
				}
			}
		}

		//Eject
		int eject = GetInput().WaveformEject(target);
		if(EncodeEveryTrack)
		{
			if(VideoEncoderEndSignal==1)
			{
				eject=2;
				EncodeEveryTrackIndex++;
			}
		}
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
				Sound->Stop(Channel);//PrepareForDelete();
				Channel=-1;
				Mode=3;
				if(LGL_VideoDecoder* dec = GetVideo())
				{
					LGL_DrawLogWrite("!dvj::DeleteVideo|%s\n",dec->GetPath());
				}

				char* update=new char[2048];
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

		if(FindAudioPathThread)
		{
			if(FindAudioPathDone)
			{
				LGL_ThreadWait(FindAudioPathThread);
				FindAudioPathThread=NULL;
			}
		}

		if(LoadAllCachedDataThread)
		{
			if(LoadAllCachedDataDone)
			{
				LGL_ThreadWait(LoadAllCachedDataThread);
				LoadAllCachedDataThread=NULL;
			}
		}

		//Exile any active MetadataFileToRams to Death Row
		if(MetadataFileToRam)
		{
			MetadataFileToRamDeathRow.push_back(MetadataFileToRam);
			MetadataFileToRam=NULL;
		}

		if
		(
			VideoEncoderThread==NULL &&
			LoadAllCachedDataThread==NULL &&
			MetadataFileToRam==NULL /*&&
			(
				Sound==NULL ||
				Sound->ReadyForDelete()
			)
			*/
		)
		{
			Mode=0;
			Mode0Timer.Reset();

			char oldSelection[2048];
			if(GetHighlightedNameDisplayed())
			{
				strncpy
				(
					oldSelection,
					GetHighlightedNameDisplayed(),
					sizeof(oldSelection)-1
				);
			}
			else
			{
				oldSelection[0]='\0';
			}
			oldSelection[sizeof(oldSelection)-1]='\0';

			Channel=-1;
			PauseMultiplier=0;
			DatabaseEntryNow=NULL;
			VideoEncoderPathSrc[0]='\0';
			VideoEncoderUnsupportedCodecTime=0.0f;
			for(int a=0;a<3;a++)
			{
				LGL_VideoDecoder* video = Video;
				if(a==1)
				{
					video=VideoLo;
				}
				else if(a==2)
				{
					video=VideoHi;
				}

				if(video)
				{
					video->InvalidateAllFrameBuffers();
					video->SetVideo(NULL);
					video->SetUserString("NULL");
				}
			}
			FilterTextMostRecent[0]='\0';
			FilterText.SetString();
			FilterTextMostRecentBPM = (int)floorf(BPMMaster+0.5f);
			DatabaseFilter.SetPattern("");

			//Going back to Mode 0 from Mode 3
			if(UpdateFilterListViaThread)
			{
				DatabaseFilter.Assign(UpdateFilterListDatabaseFilterNext);
				UpdateFilterListResetHighlightedRow=false;
				strncpy(UpdateFilterListDesiredSelection,oldSelection,sizeof(UpdateFilterListDesiredSelection)-1);
			}
			else
			{
				GetEntryListFromFilterDance(oldSelection);
			}

			for(int a=0;a<Savepoints.size();a++)
			{
				Savepoints[a].UnsetNoisePercent=0.0f;
				Savepoints[a].UnsetFlashPercent=0.0f;
			}
			SavepointIndex=0;

			AudioInputMode=false;
			Mode0BackspaceTimer.Reset();
			RecordScratch=false;
			LuminScratch=false;

			VideoLoPath[0]='\0';
			VideoHiPath[0]='\0';
			VideoLoPathShort[0]='\0';
			VideoHiPathShort[0]='\0';

			strncpy(DenyPreviewNameDisplayed,SoundSrcNameDisplayed,sizeof(DenyPreviewNameDisplayed)-1);
			DenyPreviewNameDisplayed[sizeof(DenyPreviewNameDisplayed)-1]='\0';
		}
		else
		{
			NoiseFactor=1.0f;
			NoiseFactorVideo=1.0f;
		}
	}

	if(1 || Focus)
	{
		if(Mode==2)
		{
			GlitchPure=
			(
				LGL_GetWiimote(1).ButtonDown(LGL_WIIMOTE_2) &&
				!(LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_MINUS)) &&
				!(LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_PLUS))
			);
			GlitchPureDuo=true;

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
					LGL_LoopCounterAlpha();
					while(GlitchPureVideoNow>(GlitchBegin/44100.0f)+(GlitchLength/44100.0f)*GLITCH_PURE_VIDEO_MULTIPLIER)
					{
						LGL_LoopCounterDelta();
						GlitchPureVideoNow-=(GlitchLength/44100.0f)*GLITCH_PURE_VIDEO_MULTIPLIER;
					}
					LGL_LoopCounterOmega();
				}
			}

			//bool wasScratching = LuminScratch || RecordScratch;
			if(GetInput().WaveformPointerScratch(target)!=-1.0f)
			{
				//Lumin Scratch
				float centerSample=Sound->GetPositionSamples(Channel);
				float leftSample=centerSample-64*512*Pitchbend*2*Sound->GetHz()/44100.0f;
				float rightSample=centerSample+64*512*Pitchbend*2*Sound->GetHz()/44100.0f;

				float gSamplePercent=0.5f+1.05f*(GetInput().WaveformPointerScratch(target)-0.5f);
				gSamplePercent = LGL_Clamp(0,(gSamplePercent-0.2f)*(1.0f/0.6f),1);

				LuminScratch=true;
				LuminScratchSamplePositionDesired=
					(long)(leftSample+gSamplePercent*(rightSample-leftSample));
				LGL_LoopCounterAlpha();
				while(LuminScratchSamplePositionDesired<0)
				{
					LGL_LoopCounterDelta();
					LuminScratchSamplePositionDesired+=Sound->GetLengthSamples();
				}
				LGL_LoopCounterOmega();

				LGL_LoopCounterAlpha();
				while(LuminScratchSamplePositionDesired>=Sound->GetLengthSamples())
				{
					LGL_LoopCounterDelta();
					LuminScratchSamplePositionDesired-=Sound->GetLengthSamples();
				}
				LGL_LoopCounterOmega();

				GlitchBegin=Sound->GetPositionGlitchBeginSamples(Channel)-.005f*64*512*Pitchbend*2;
			}
			else
			{
				LuminScratch=false;
				LuminScratchSamplePositionDesired=-10000;
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
					GrainStreamSourcePoint=GetTimeSeconds();
					GrainStreamPitch=1.0f;
				}
				else
				{
					GrainStreamActiveSeconds+=secondsElapsed;
					if(1)
					{
						LGL_LoopCounterAlpha();
						while(GrainStreamSourcePoint<0.0f)
						{
							LGL_LoopCounterDelta();
							GrainStreamSourcePoint+=Sound->GetLengthSeconds();
						}
						LGL_LoopCounterOmega();

						LGL_LoopCounterAlpha();
						while(GrainStreamSourcePoint>=Sound->GetLengthSeconds())
						{
							LGL_LoopCounterDelta();
							GrainStreamSourcePoint-=Sound->GetLengthSeconds();
						}
						LGL_LoopCounterOmega();
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
		}
	}

	//Update State

	if(Mode==0)
	{
		//File Selection
	}
	else if(Mode==1)
	{
		//Loading...

		if(Sound==NULL)
		{
			AttemptToCreateSound();
		}

		if
		(
			Sound &&
			LoadAllCachedDataThread==NULL
		)
		{
			LoadAllCachedDataDone=false;
			LoadAllCachedDataThread = LGL_ThreadCreate(loadAllCachedDataThread,this);
		}

		if
		(
			Sound &&
			Sound->GetLengthSeconds()+1.0f>GetCurrentSavepointSeconds() &&
			LoadAllCachedDataDone &&
			LGL_WriteFileAsyncQueueCount()==0
		)
		{
			Mode2Timer.Reset();
			Mode=2;

			if(LoadAllCachedDataThread)
			{
				LGL_ThreadWait(LoadAllCachedDataThread);
				LoadAllCachedDataThread=NULL;
			}

			if(Channel==-1)
			{
				Channel=Sound->Play(0,true,0);
				UpdateSoundFreqResponse();
				//Sound->SetPositionSeconds(Channel,GetCurrentSavepointSeconds());
				PauseMultiplier=0;
			}
			else
			{
				//PauseMultiplier=1;
			}
			UpdateSoundFreqResponse();
			Sound->SetSpeedInterpolationFactor
			(
				Channel,
				//16.0/44100.0
				//20.0/44100.0
				32.0f/44100.0f
			);
			
			char* update=new char[2048];
			sprintf(update,"ALPHA: %s",SoundName);
			TrackListFileUpdates.push_back(update);

			if(FileEverOpened==false)
			{
				FileEverOpened=true;
				FileEverOpenedTimer.Reset();

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
			if(GetBPM()==GetBPMAdjusted())
			{
				Pitchbend=1;
			}
			PitchbendLastSetByXponentSlider=false;
			Nudge=0;
			MixerNudge=0;
			ReverseMultiplier=1.0f;
			GlitchPure=false;
			GlitchPureDuo=false;
			GlitchDuo=false;
			GlitchVolume=0;
			GlitchBegin=0;
			GlitchLength=0;
			GlitchPitch=1.0f;
			SetSmoothWaveformScrollingSample(0.0f);
			SmoothWaveformScrollingSampleRate=1.0f;
			SmoothWaveformScrollingSampleRateRemembered=1.0f;
			VideoOffsetSeconds=LGL_RandFloat(0,1000.0f);

			VideoEncoderPercent=-1.0f;
			VideoEncoderEtaSeconds=-1.0f;
			VideoEncoderAudioOnly=false;

			VideoEncoderTerminateSignal=0;
			VideoEncoderBeginSignal=0;
			VideoEncoderEndSignal=0;
			strcpy(VideoEncoderPathSrc,SoundSrcPath);
			VideoEncoderThread=LGL_ThreadCreate(videoEncoderThread,this);

			//Load Video if possible
			/*
			if(Video)
			{
				Video->InvalidateAllFrameBuffers();
				Video->SetVideo(NULL);
			}
			*/
			SelectNewVideo();

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
			Channel=Sound->Play(0,true,0);
			UpdateSoundFreqResponse();
			if(Sound->IsLoaded())
			{
				if(SmoothWaveformScrollingSample/Sound->GetHz() > Sound->GetLengthSeconds()-1)
				{
					SetSmoothWaveformScrollingSample(0);
				}
				else
				{
					Sound->SetPositionSeconds(Channel,SmoothWaveformScrollingSample/Sound->GetHz());
				}
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
				SetSmoothWaveformScrollingSample(Sound->GetPositionSamples(Channel));
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
					GlitchInterpolation=40.0/44100.0;	//Affects scratch responsiveness.
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
			LGL_LoopCounterAlpha();
			while(timeStamp<secondsElapsed)
			{
				LGL_LoopCounterDelta();
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
			LGL_LoopCounterOmega();

			GrainStreamSpawnDelaySeconds=timeStamp-secondsElapsed;
		}
		else
		{
			GrainStreamSpawnDelaySeconds=0.0f;
		}

		if(GetVideo()!=NULL)
		{
			float time = GetTimeSeconds();
			if(LuminScratch)
			{
				time = LuminScratchSamplePositionDesired / 44100.0f;
			}
		}
	}

	if(Sound && Channel>=0)
	{
		//Smooth waveform scrolling

//double smoothWaveformScrollingSamplePrev=SmoothWaveformScrollingSample;
		if
		(
			SmoothWaveformScrollingSampleExactUponDifferent>=0 &&
			SmoothWaveformScrollingSample == SmoothWaveformScrollingSampleExactUponDifferent
		)
		{
			double now=Sound->GetPositionSamples(Channel);
			if(now != SmoothWaveformScrollingSampleExactUponDifferent)
			{
				SetSmoothWaveformScrollingSample(now);
				SmoothWaveformScrollingSampleExactUponDifferent=-1;
			}
		}
		float speed=Sound->GetSpeed(Channel);
		double proposedDelta = SmoothWaveformScrollingSampleRate*(LGL_AudioAvailable()?1:0)*speed*Sound->GetHz()*1.0f/LGL_GetFPSMax();//LGL_SecondsSinceLastFrame();
		double currentSample=Sound->GetPositionSamples(Channel);
		double diff=fabs(currentSample-(SmoothWaveformScrollingSample+proposedDelta));
		double diffMax=LGL_AudioCallbackSamples()*16*LGL_Max(1,fabsf(Sound->GetSpeed(Channel)));
		double deltaFrame = LGL_SecondsSinceLastFrame();
//printf("Proposed: %.2f (%i) (%.2f)\n",(float)proposedDelta,(int)(LGL_SecondsSinceLastFrame()*60.0f),(float)diff);
		/*
		if(deltaFrame <= 5.0f/60.0f)
		{
			deltaFrame = 1.0f/60.0f;
		}
		*/
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

/*
int framesPast=(int)(60.0f/LGL_SecondsSinceLastFrame());

float pctXDiff=0.5f-0.5f*diff/diffMax;
LGL_DrawLineToScreen
(
	pctXDiff,1.0f,
	pctXDiff,0.95f
);

float pctXRate=0.25f+(SmoothWaveformScrollingSampleRate*0.25f);
LGL_DrawLineToScreen
(
	pctXRate,0.80f,
	pctXRate,0.75f,
	0,0,1,1
);

float pctXProposedInitial=0.5f+proposedDelta/diffMax;
LGL_DrawLineToScreen
(
	pctXProposedInitial,0.95f,
	pctXProposedInitial,0.90f
);
*/

		const float rateDelta=2.0f;

		float constantThreashold=1024;//diffMax/2;
		if(0 && diff<constantThreashold)
		{
			//We're accurate enough
//printf("\t\t\t\t\t\tOK!\n");
			SetSmoothWaveformScrollingSample(SmoothWaveformScrollingSample+proposedDelta);
			SmoothWaveformScrollingSampleRate=
				(1.0f-rateDelta*0.1f*LGL_SecondsSinceLastFrame())*SmoothWaveformScrollingSampleRate+
				(0.0f+rateDelta*0.1f*LGL_SecondsSinceLastFrame())*1.0f;
		}
		else if(diff<diffMax)
		{
			//Let's change our scrolling speed to get more accurate
			float deltaFactor=(diff/*-constantThreashold*/)/(diffMax*0.5f/*-constantThreashold*/);
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
			SmoothWaveformScrollingSampleRate=
				(1.0f-rateDelta*LGL_SecondsSinceLastFrame())*SmoothWaveformScrollingSampleRate+
				(0.0f+rateDelta*LGL_SecondsSinceLastFrame())*deltaMultiplier;
			/*
			SmoothWaveformScrollingSampleRate=
				(1.0f-rateDelta*LGL_SecondsSinceLastFrame())*SmoothWaveformScrollingSampleRate+
				(0.0f+rateDelta*LGL_SecondsSinceLastFrame())*
					(
						deltaMultiplier+
						(SmoothWaveformScrollingSampleRate-1.0f)
					);
			*/

			//proposedDelta*=deltaMultiplier;
/*
float pctXProposedRevised=0.5f+proposedDelta/diffMax;
LGL_DrawLineToScreen
(
	pctXProposedRevised,0.95f,
	pctXProposedRevised,0.90f,
	(framesPast>1) ? 1 : 0,(framesPast>1) ? 0 : 1,0,1
);
*/
//printf("\t\t\t\t\t\tDelta! (%.2f) (%.2f)\n",proposedDelta,deltaMultiplier);
			SetSmoothWaveformScrollingSample(SmoothWaveformScrollingSample+proposedDelta);
		}
		else
		{
//printf("\t\t\t\t\t\tFUCK!\n");
			//Fuck, we're really far off. Screw smooth scrolling, just jump to currentSample
			SetSmoothWaveformScrollingSample(currentSample);
		}
		if(RewindFF==false)
		{
			SmoothWaveformScrollingSampleRateRemembered = SmoothWaveformScrollingSampleRate;
		}

/*
float pctXFinalDelta=0.5f+(SmoothWaveformScrollingSample-smoothWaveformScrollingSamplePrev)/diffMax;
LGL_DrawLineToScreen
(
	pctXFinalDelta,0.90f,
	pctXFinalDelta,0.85f,
	(framesPast>1) ? 1 : 0,(framesPast>1) ? 0 : 1,0,1
);
*/

		if(Looping())
		{
			if
			(
				SmoothWaveformScrollingSample>LoopOmegaSeconds*Sound->GetHz() ||
				SmoothWaveformScrollingSample<LoopAlphaSeconds*Sound->GetHz()
			)
			{
				SetSmoothWaveformScrollingSample(LoopAlphaSeconds*Sound->GetHz());
			}
		}

if(LGL_SecondsSinceLastFrame()>=1.5f/60.0f)
{
	SetSmoothWaveformScrollingSample(Sound->GetPositionSamples(Channel));
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
				GrainStreamSourcePoint=GetTimeSeconds();
			}
			else
			{
				GrainStreamSourcePoint=0.0f;
			}
		}
	}

	if
	(
		Sound &&
		Channel!=-1.0f
	)
	{
		float vol = VolumeKill ? 0.0f : 0.5f*VolumeSlider*VolumeMultiplierNow*(1.0f-GrainStreamCrossfader);
		if
		(
			Mode1Timer.SecondsSinceLastReset()>1 &&
			Mode2Timer.SecondsSinceLastReset()>1
		)
		{
			vol *= (1.0f-NoiseFactor);
		}

		float volFront = VolumeInvertBinary ?
			(MixerVolumeFront==0.0f ? 1.0f : 0.0f) :
			MixerVolumeFront;
		float volBack = VolumeInvertBinary ?
			(MixerVolumeBack==0.0f ? 1.0f : 0.0f) :
			MixerVolumeBack;

		volFront*=fastSeekVolumeMultiplier;
		volBack*=fastSeekVolumeMultiplier;

		if(Mode==0)
		{
			volFront=0.0f;
			if(Focus==false)
			{
				volBack=0.0f;
			}
		}

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
	}

	WhiteFactor = LGL_Max(0.0f,WhiteFactor-4.0f*LGL_Min(LGL_SecondsSinceLastFrame(),1.0f/60.0f));
	if(noiseIncreasing==false)
	{
		NoiseFactor = LGL_Max(0.0f,NoiseFactor-2.0f*LGL_Min(LGL_SecondsSinceLastFrame(),1.0f/60.0f));
	}

	FocusPrev=Focus;
}

long lastSampleLeft=0;

void
TurntableObj::
DrawFrame
(
	float	glow,
	bool	visualsQuadrent,
	float	visualizerZoomOutPercent
)
{
	{
		if(Video)
		{
			Video->GetImage();
		}
		if(VideoLo)
		{
			VideoLo->GetImage();
		}
		if(VideoHi)
		{
			VideoHi->GetImage();
		}
	}
	if
	(
		TesterEverEnabled &&
		Which==0
	)
	{
		LGL_DebugPrintf("Saving disabled due to InputTester usage");
	}

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
		
		if(LGL_VideoDecoder* vid = GetVideo())
		{
			if(LGL_Image* image = vid->GetImage())
			{
				if(image->GetFrameNumber()!=-1)
				{
					VideoEncoderBeginSignal=1;
				}
			}
		}

		if(Sound->IsLoaded()==false)
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
				LGL_LoopCounterAlpha();
				while(seconds>=60)
				{
					LGL_LoopCounterDelta();
					minutes++;
					seconds-=60;
				}
				LGL_LoopCounterOmega();

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
				"%.0f%%",
				LGL_Clamp(0.0f,percent,0.99f)*100.0f
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
				LGL_ScopeLock lock(__FILE__,__LINE__,VideoEncoderSemaphore);
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

			if(VideoEncoderAudioOnly==false)
			{
				if(VideoEncoder)
				{
					char str[2048];
					sprintf
					(
						str,
						"%s => mjpeg",
						VideoEncoder->GetCodecName() ? VideoEncoder->GetCodecName() : "Unknown"
					);
					float fontHeight=0.015f;
					float fontWidth=LGL_GetFont().GetWidthString(fontHeight,str);
					float fontWidthMax=width*0.95f;
					fontHeight=LGL_Min(fontHeight,fontHeight*fontWidthMax/fontWidth);
					LGL_GetFont().DrawString
					(
						centerX,bottom+0.75f*height,fontHeight,
						1,1,1,1,
						true,.5f,
						str
					);
				}

				if(GetDebugVideoCaching())
				{
					LGL_ClipRectDisable();
					LGL_DebugPrintf(VideoEncoderReason);
					char videoFileName[1024];
					findVideoPath
					(
						videoFileName,
						SoundSrcPath
					);
				}
			}

			int seconds=(int)VideoEncoderEtaSeconds;
			if(seconds>=0)
			{
				int minutes=0;
				LGL_LoopCounterAlpha();
				while(seconds>=60)
				{
					LGL_LoopCounterDelta();
					minutes++;
					seconds-=60;
				}
				LGL_LoopCounterOmega();

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
				"%.0f%%",
				LGL_Clamp(0.0f,VideoEncoderPercent,0.99f)*100.0f
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
				FreqSensePathBrightness=2.0f;
				if(LGL_MouseStroke(LGL_MOUSE_LEFT))
				{
					if(LGL_KeyDown(LGL_KEY_SHIFT)==false)
					{
						GetInputMouse().SetWaveformVideoAspectRatioNextNext();
					}
					else
					{
						GetInputMouse().SetWaveformVideoSelectLowNext();
					}
				}
				if(LGL_MouseStroke(LGL_MOUSE_RIGHT))
				{
					GetInputMouse().SetWaveformVideoSelectHighNext();
				}
			}

			if(1)
			{
				Visualizer->DrawVideos
				(
					this,
					left,
					right,
					bottom,
					top,
					true
				);
				if(GetFreqSenseLEDBrightnessPreview()>0.0f)
				{
					LGL_Color color = GetVisualizer()->GetLEDColor(GetFreqSenseLEDGroupInt());
					float r=color.GetR();
					float g=color.GetG();
					float b=color.GetB();

					LGL_DrawLineToScreen
					(
						left,bottom,
						right,bottom,
						r,g,b,1.0f,
						3.0f,
						true
					);
					LGL_DrawLineToScreen
					(
						right,bottom,
						right,top,
						r,g,b,1.0f,
						3.0f,
						true
					);
					LGL_DrawLineToScreen
					(
						right,top,
						left,top,
						r,g,b,1.0f,
						3.0f,
						true
					);
					LGL_DrawLineToScreen
					(
						left,top,
						left,bottom,
						r,g,b,1.0f,
						3.0f,
						true
					);
				}
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
		ListSelector.SetBehindImage
		(
			NoiseImage[rand()%NOISE_IMAGE_COUNT_256_64]
		);
		ListSelector.Draw();

		Turntable_DrawSliders
		(
			Which,
			ViewportLeft,
			ViewportRight,
			ViewportBottom,
			ViewportTop,
			EQFinal[0],
			EQFinal[1],
			EQFinal[2],
			GetEQVUL(),
			GetEQVUM(),
			GetEQVUH(),
			GetOtherTT()->GetEQVUL(),
			GetOtherTT()->GetEQVUM(),
			GetOtherTT()->GetEQVUH(),
			GetEQVUPeakL(),
			GetEQVUPeakM(),
			GetEQVUPeakH(),
			GetOtherTT()->GetEQVUPeakL(),
			GetOtherTT()->GetEQVUPeakM(),
			GetOtherTT()->GetEQVUPeakH(),
			GetVU(),
			GetVUPeak(),
			GetOtherTT()->GetVU(),
			GetOtherTT()->GetVUPeak(),
			VolumeMultiplierNow,
			VideoBrightness,
			SyphonBrightness,
			OscilloscopeBrightness,
			FreqSenseBrightness,
			FreqSenseLEDBrightness[GetFreqSenseLEDGroupInt()],
			FreqSenseLEDColorScalarLow[GetFreqSenseLEDGroupInt()],
			FreqSenseLEDColorScalarHigh[GetFreqSenseLEDGroupInt()],
			FreqSenseLEDBrightnessWash[GetFreqSenseLEDGroupInt()],
			FreqSenseLEDGroupFloat,
			GetFreqSenseLEDGroupInt()
		);

		{
			unsigned int savepointBitfield=0;
			unsigned int savepointBPMBitfield=0;
			double savepointSeconds[SAVEPOINT_NUM];
			double savepointBPMs[SAVEPOINT_NUM];
			float savepointUnsetNoisePercent[SAVEPOINT_NUM];
			float savepointUnsetFlashPercent[SAVEPOINT_NUM];
			for(int a=0;a<SAVEPOINT_NUM;a++)
			{
				if(a<Savepoints.size())
				{
					savepointBitfield|=(Savepoints[a].Seconds==-1.0f)?0:(1<<a);
					savepointBPMBitfield|=(Savepoints[a].BPM==BPM_UNDEF)?0:(1<<a);
					savepointSeconds[a]=Savepoints[a].Seconds;
					savepointBPMs[a]=Savepoints[a].BPM;
					savepointUnsetNoisePercent[a]=Savepoints[a].UnsetNoisePercent;
					savepointUnsetFlashPercent[a]=Savepoints[a].UnsetFlashPercent;
				}
				else
				{
					savepointSeconds[a]=-1.0f;
					savepointBPMs[a]=-1.0f;
					savepointUnsetNoisePercent[a]=0.0f;
					savepointUnsetFlashPercent[a]=0.0f;
				}
			}
			Turntable_DrawSavepointSet
			(
				ViewportLeft,
				ViewportRight,
				ViewportBottom,
				ViewportTop,
				LGL_SecondsSinceExecution(),
				savepointSeconds,
				savepointBPMs,
				SavepointIndex,
				SavepointIndex,
				savepointBitfield,
				savepointBPMBitfield,
				savepointUnsetNoisePercent,
				savepointUnsetFlashPercent,
				NoiseImage[rand()%NOISE_IMAGE_COUNT_256_64],
				Which,
				Mode,
				Pitchbend
			);
		}

		if(GetBPMAdjusted()>0)
		{
			char bpmString[512];
			if(GetBPM()==-2.0f)
			{
				strcpy(bpmString,"NONE");
			}
			else if(GetBPM()==-1.0f)
			{
				strcpy(bpmString,"UNDEF");
			}
			else
			{
				sprintf(bpmString,"%.2f",GetBPMAdjusted());
			}
			Turntable_DrawBPMString
			(
				ViewportLeft,
				ViewportRight,
				ViewportBottom,
				ViewportTop,
				bpmString
			);
			Turntable_DrawPitchbendString
			(
				ViewportLeft,
				ViewportRight,
				ViewportBottom,
				ViewportTop,
				Pitchbend,
				Nudge
			);
		}

		{
			float left=	ViewportLeft;
			float right=	0.5f-0.501f*WAVE_WIDTH_PERCENT*ViewportWidth;
			float bottom=	ViewportBottom+0.125*ViewportHeight;
			float top=	ViewportBottom+0.875*ViewportHeight;
			Visualizer->DrawVideos
			(
				this,
				left,
				right,
				bottom,
				top,
				true
			);
		}

		if
		(
			0 &&
			Video->GetPathShort() &&
			GetHighlightedPathShort() &&
			strstr(Video->GetPathShort(),GetHighlightedPathShort())
		)
		{
			if(LGL_Image* image = Video->GetImage())
			{
				if(image->GetFrameNumber()>=0)
				{
					if(strstr(image->GetVideoPath(),Video->GetPathShort()))
					{
						float aspectRatioDst=
							(LGL_WindowResolutionX()*(WaveformLeft-ViewportLeft))/
							(LGL_WindowResolutionY()*(ViewportRight-ViewportBottom));

						float subImgLeft=0.0f;
						float subImgRight=1.0f;
						float subImgBottom=0.0f;
						float subImgTop=1.0f;

						float aspectRatioSrc=
							image->GetWidth()/
							(float)image->GetHeight();

						if(aspectRatioDst>aspectRatioSrc)
						{
							//Too wide, so make it less wide.
							subImgLeft = 0.5f-0.5f*aspectRatioSrc/aspectRatioDst;
							subImgRight = 0.5f+0.5f*aspectRatioSrc/aspectRatioDst;
						}
						/*
						else if(aspectRatioSrc<aspectRatioDst)
						{
							//Too tall, so make it less tall.
							subImgBottom = 0.5f-0.5f*image->GetHeight()*aspectRatioDst;
							subImgTop = 0.5f+0.5f*image->GetHeight()*aspectRatioDst;
						}
						*/

						Video->GetImage()->DrawToScreen
						(
							ViewportLeft,
							WaveformLeft,
							ViewportTop,
							ViewportBottom,
							0,
							1,1,1,1,
							1,
							subImgLeft,
							subImgRight,
							subImgBottom,
							subImgTop
						);
					}
				}
			}
		}

		{
			char drawDirPath[2048];
			const char* title = FilterText.GetString();
			if(title==NULL || title[0]=='\0')
			{
				if(strstr(DatabaseFilter.Dir,LGL_GetHomeDir()))
				{
					sprintf(drawDirPath,"~%s/",&(DatabaseFilter.Dir[strlen(LGL_GetHomeDir())]));
				}
				else
				{
					strcpy(drawDirPath,DatabaseFilter.Dir);
				}
				title=drawDirPath;
			}
			LGL_GetFont().DrawString
			(
				CenterX,ViewportBottom+.875f*ViewportHeight,.025f,
				1,1,1,1,
				true,.5f,
				title
			);
		}
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
		int SoundNameSafeLen=(int)strlen(SoundNameSafe);
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
			Sound ? (Sound->GetPercentLoadedSmooth()*100) : 0.0f
		);

		if(Sound)
		{
			float minutes=0;
			float seconds=ceil(Sound->GetSecondsUntilLoaded());
			minutes = floorf(seconds/60.0f);
			seconds -= minutes*60.0f;
			LGL_LoopCounterAlpha();
			while(seconds>=60)
			{
				LGL_LoopCounterDelta();
				minutes++;
				seconds-=60;
			}
			LGL_LoopCounterOmega();

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

			unsigned int savepointBitfield=0;
			unsigned int savepointBPMBitfield=0;

			float videoSecondsBufferedLeft=0.0f;
			float videoSecondsBufferedRight=0.0f;
			float videoSecondsLoadedLeft=0.0f;
			float videoSecondsLoadedRight=0.0f;
			if(Video)
			{
				videoSecondsBufferedLeft=Video->GetSecondsBufferedLeft(false,true);
				videoSecondsBufferedRight=Video->GetSecondsBufferedRight(false,true);
				videoSecondsLoadedLeft=Video->GetSecondsBufferedLeft(true,true);
				videoSecondsLoadedRight=Video->GetSecondsBufferedRight(true,true);
			}
			if(GetVideoBrightnessPreview()==0.0f)
			{
				videoSecondsBufferedLeft=0;
				videoSecondsBufferedRight=0;
			}
				
			float recallPos=-1.0f;

			if
			(
				Sound->GetWarpPointIsSet(Channel) &&
				Sound->GetWarpPointIsLoop(Channel)==false &&
				Sound->GetWarpPointIsLocked(Channel)
			)
			{
				recallPos=Sound->GetWarpPointSecondsOmega(Channel) +
					(
						GetTimeSeconds() -
						Sound->GetWarpPointSecondsAlpha(Channel)
					);
			}
			if(GetFocus() && GetInputMouse().GetEntireWaveformScrubberDelta())
			{
				recallPos=GetInputMouse().GetEntireWaveformScrubberRecallPercent()*Sound->GetLengthSeconds();
			}


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
				SavepointIndex,						//25
				SavepointIndex,						//26
				savepointBitfield,					//27
				GetBPM(),						//28
				GetBPMAdjusted(),					//29
				GetBPMAnchorMeasureSeconds(),//GetBPMFirstBeatSeconds(),				//30
				EQFinal[0],						//31
				EQFinal[1],						//32
				EQFinal[2],						//33
				LowRez ? 'T' : 'F',					//34
				AudioInputMode ? 'T' : 'F',				//35
				Looping() ? LoopAlphaSeconds : -1.0f,			//36
				Sound->GetWarpPointSecondsAlpha(Channel),		//37
				QuantizePeriodMeasuresExponent,				//38
				QuantizePeriodNoBPMSeconds,				//39
				GetInput().WaveformRecordHold(target) ? 'T' : 'F',	//40
				SoundSrcNameDisplayed,					//41
				videoSecondsBufferedLeft,				//42
				videoSecondsBufferedRight,				//43
				(Which==Master) ? 'T' : 'F',				//44
				(RhythmicVolumeInvert) ? 'T' : 'F',			//45
				GetBeginningOfCurrentMeasureSeconds(),			//46
				VideoBrightness,					//47
				OscilloscopeBrightness,					//48
				FreqSenseBrightness					//49
			);
			
			bool waveArrayFilledBefore=(EntireWaveArrayFillIndex==ENTIRE_WAVE_ARRAY_COUNT);

			LGL_DrawLogPause();
			float freqSensePathActiveMultiplier=1.0f;
			if(strcmp(VideoLo->GetPathShort(),VideoLoPathShort)!=0)
			{
				freqSensePathActiveMultiplier=0.5f;
			}

			double savepointSeconds[SAVEPOINT_NUM];
			double savepointBPMs[SAVEPOINT_NUM];
			float savepointUnsetNoisePercent[SAVEPOINT_NUM];
			float savepointUnsetFlashPercent[SAVEPOINT_NUM];
			for(int a=0;a<SAVEPOINT_NUM;a++)
			{
				if(a<Savepoints.size())
				{
					savepointBitfield|=(Savepoints[a].Seconds==-1.0f)?0:(1<<a);
					savepointBPMBitfield|=(Savepoints[a].BPM==BPM_UNDEF)?0:(1<<a);
					savepointSeconds[a]=Savepoints[a].Seconds;
					savepointBPMs[a]=Savepoints[a].BPM;
					savepointUnsetNoisePercent[a]=Savepoints[a].UnsetNoisePercent;
					savepointUnsetFlashPercent[a]=Savepoints[a].UnsetFlashPercent;
				}
				else
				{
					savepointSeconds[a]=-1.0f;
					savepointBPMs[a]=-1.0f;
					savepointUnsetNoisePercent[a]=0.0f;
					savepointUnsetFlashPercent[a]=0.0f;
				}
			}

			Turntable_DrawWaveform
			(
				Which,
				Sound,							//01
				Sound->IsLoaded(),					//02
				Mode,
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
				savepointSeconds,					//25
				savepointBPMs,
				SavepointIndex,						//26
				SavepointIndex,						//27
				savepointBitfield,					//28
				savepointBPMBitfield,					//28
				savepointUnsetNoisePercent,				//29
				savepointUnsetFlashPercent,				//30
				GetBPM(),						//31
				GetBPMAdjusted(),					//32
				GetInput().WaveformBPMCandidate(target),		//blah
				GetBPMAnchorMeasureSeconds(),//GetBPMFirstBeatSeconds(),				//33
				EQFinal[0],						//34
				EQFinal[1],						//35
				EQFinal[2],						//36
				GetEQVUL(),
				GetEQVUM(),
				GetEQVUH(),
				GetOtherTT()->GetEQVUL(),
				GetOtherTT()->GetEQVUM(),
				GetOtherTT()->GetEQVUH(),
				GetEQVUPeakL(),
				GetEQVUPeakM(),
				GetEQVUPeakH(),
				GetOtherTT()->GetEQVUPeakL(),
				GetOtherTT()->GetEQVUPeakM(),
				GetOtherTT()->GetEQVUPeakH(),
				GetVU(),
				GetVUPeak(),
				GetOtherTT()->GetVU(),
				GetOtherTT()->GetVUPeak(),
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
				GetInput().WaveformRecordHold(target),			//51
				SoundSrcNameDisplayed,					//52
				videoSecondsBufferedLeft,				//53
				videoSecondsBufferedRight,				//54
				videoSecondsLoadedLeft,					//53a
				videoSecondsLoadedRight,				//54a
				Which==Master,						//55
				RhythmicVolumeInvert,					//56
				GetBeginningOfCurrentMeasureSeconds(),			//57
				VideoBrightness,					//58
				SyphonBrightness,					//59
				OscilloscopeBrightness,					//60
				FreqSenseBrightness,					//61
				LGL_Min(FreqSensePathBrightness,1.0f)*freqSensePathActiveMultiplier,
				VideoLoPathShort,
				VideoHiPathShort,
				FreqSenseLEDBrightness[GetFreqSenseLEDGroupInt()],	//62
				FreqSenseLEDColorScalarLow[GetFreqSenseLEDGroupInt()],	//63
				FreqSenseLEDColorScalarHigh[GetFreqSenseLEDGroupInt()],	//64
				FreqSenseLEDBrightnessWash[GetFreqSenseLEDGroupInt()],	//65
				FreqSenseLEDGroupFloat,					//66
				GetFreqSenseLEDGroupInt(),				//67
				Channel,						//68
				recallPos						//69
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
	if(Sound==NULL)
	{
		return;
	}

	float speedHeightScalar = LGL_Clamp(0.0f,FinalSpeed*100.0f,1.0f);
	if(speedHeightScalar == 0.0f)
	{
		return;
	}

	GetVisualizer()->GetProjectorARCoordsFromViewportCoords
	(
		left,
		right,
		bottom,
		top
	);

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
		LGL_Min(FinalSpeed,1.0f)/240.0f,
		0.0f,
		0.0f,
		false
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

TurntableObj*
TurntableObj::
GetOtherTT()
{
	TurntableObj* otherTT = GetMixer().GetTurntable(1-Which);
	return(otherTT);
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

	CenterX=ViewportLeft+0.5f*ViewportWidth;
	CenterY=ViewportBottom+0.5f*ViewportHeight;
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

int
TurntableObj::
GetWhich() const
{
	return(Which);
}

int
TurntableObj::
GetTarget() const
{
	int target = (Focus ? TARGET_FOCUS : 0) |
		((Which==0) ? TARGET_TOP : TARGET_BOTTOM);
	return(target);
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
		if(RhythmicVolumeInvert)
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
		if(Sound->GetRespondToRhythmicSoloInvertCurrentValue(Channel)==0)
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
GetSyphonBrightnessPreview()
{
	return
	(
		GetVisualBrightnessPreview()*
		SyphonBrightness
	);
}

float
TurntableObj::
GetSyphonBrightnessFinal()
{
	return
	(
		GetVisualBrightnessFinal()*
		SyphonBrightness
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

float
TurntableObj::
GetFreqSenseLEDBrightnessPreview()
{
	return
	(
		GetVisualBrightnessPreview()*
		FreqSenseLEDBrightness[GetFreqSenseLEDGroupInt()]
	);
}

float
TurntableObj::
GetFreqSenseLEDBrightnessFinal
(
	int	group
)
{
	group=LGL_Clamp(0,group,LED_GROUP_MAX-1);
	return
	(
		GetVisualBrightnessFinal()*
		FreqSenseLEDBrightness[group]
	);
}

float
TurntableObj::
GetFreqSenseLEDBrightnessWash
(
	int	group
)
{
	group=LGL_Clamp(0,group,LED_GROUP_MAX-1);
	return(FreqSenseLEDBrightnessWash[group]);
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
		return(Video);
	}

	return(NULL);
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

const char*
TurntableObj::
GetVideoLoPath()
{
	return(VideoLoPath);
}

const char*
TurntableObj::
GetVideoHiPath()
{
	return(VideoHiPath);
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
GetTimeSeconds
(
	bool	forceNonSmooth
)
{
	if(Sound==NULL || Channel<0)
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
			if(forceNonSmooth==false && LGL_FPS()>50)
			{
				return(SmoothWaveformScrollingSample/Sound->GetHz());
			}
			else
			{
				return(Sound->GetPositionSeconds(Channel));
			}
		}
	}
}

float
TurntableObj::
GetTimeSecondsPrev()
{
	return(SecondsLast);
}

bool
TurntableObj::
GetFreqMetadata
(
	float&	volAve,
	float&	volMax,
	float&	freqFactor
)
{
	bool ret=false;
	
	if
	(
		AudioInputMode ||
		GetConfigAlwaysUseAudioInForFreqSense()
	)
	{
		ret=LGL_AudioInMetadata(volAve,volMax,freqFactor,GetGain(),GetEQMid()*0.5f);
	}
	else
	{
		if(Sound)
		{
			float timeNow=GetTimeSeconds();
			float timePrev=GetTimeSecondsPrev();
			if(timePrev>timeNow-1.0f/60.0f)
			{
				timePrev=timeNow-1.0f/60.0f;
			}
			if(timeNow>timePrev+1.0f/60.0f)
			{
				timeNow=timePrev+1.0f/60.0f;
			}
			ret=
				Sound->GetMetadata
				(
					timePrev,
					timeNow,
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

void
TurntableObj::
LoadMetadataOld(const char* data)
{
	if(data==NULL)
	{
		return;
	}

	Savepoints.clear();

	FileInterfaceObj fi;
	fi.ReadLine(data);
	if(fi.Size()==0)
	{
		return;
	}
	if
	(
		strcasecmp(fi[0],"HomePoints")==0 ||
		strcasecmp(fi[0],"Savepoints")==0
	)
	{
		if(fi.Size()!=19)
		{
			printf("TurntableObj::LoadMetadataOld('%s'): Warning!\n",SoundName);
			printf("\tSavepoints has strange fi.size() of '%i' (Expecting 11)\n",fi.Size());
		}
		for(unsigned int a=1;a<fi.Size() && a<SAVEPOINT_NUM;a++)
		{
			//Drop 2nd savepoint
			if(a==2)
			{
				continue;
			}
			SavepointObj sp;
			sp.Seconds = atof(fi[a]);
			if
			(
				a==1 &&
				fi.Size()>2
			)
			{
				sp.BPM = GetBPMFromDeltaSeconds(atof(fi[a+1])-atof(fi[a+0]));
			}
			Savepoints.push_back(sp);
		}
	}

	LGL_LoopCounterAlpha();
	while(Savepoints.size()<12)
	{
		LGL_LoopCounterDelta();
		Savepoints.push_back(SavepointObj());
	}
	LGL_LoopCounterOmega();
	
	SortSavepoints();
}

void
TurntableObj::
LoadMetadataNew(const char* data)
{
	if(data==NULL)
	{
		return;
	}

	Savepoints.clear();
	VideoLoPath[0]='\0';
	VideoHiPath[0]='\0';

	FileInterfaceObj fi;
	const char* lineNow=data;
	LGL_LoopCounterAlpha();
	for(;;)
	{
		LGL_LoopCounterDelta();
		if(lineNow[0]=='\0')
		{
			break;
		}

		fi.ReadLine(lineNow);

		if(fi.Size()==2)
		{
			if(strcasecmp(fi[0],"FreqVideoLow")==0)
			{
				strcpy(VideoLoPath,fi[1]);
				if(const char* lastSlash = strrchr(VideoLoPath,'/'))
				{
					strcpy(VideoLoPathShort,&(lastSlash[1]));
				}
				else
				{
					strcpy(VideoLoPathShort,VideoLoPath);
				}
			}
			if(strcasecmp(fi[0],"FreqVideoHigh")==0)
			{
				strcpy(VideoHiPath,fi[1]);
				if(const char* lastSlash = strrchr(VideoHiPath,'/'))
				{
					strcpy(VideoHiPathShort,&(lastSlash[1]));
				}
				else
				{
					strcpy(VideoHiPathShort,VideoHiPath);
				}
			}
		}
		else if(fi.Size()==3)
		{
			if(strcasecmp(fi[0],"Savepoint")==0)
			{
				SavepointObj sp;
				sp.Seconds = atof(fi[1]);
				sp.BPM = atof(fi[2]);
				if(sp.Seconds!=-1.0f)
				{
					Savepoints.push_back(sp);
				}
			}
		}

		lineNow=strchr(lineNow,'\n');
		if(lineNow==NULL)
		{
			break;
		}
		else
		{
			lineNow=&(lineNow[1]);
		}
	}
	LGL_LoopCounterOmega();

	SavepointIndex=0;

	LGL_LoopCounterAlpha();
	while(Savepoints.size()<12)
	{
		LGL_LoopCounterDelta();
		Savepoints.push_back(SavepointObj());
	}
	LGL_LoopCounterOmega();
}

void
TurntableObj::
SaveMetadataOld()
{
	//Currently changing this!!
#if	USE_SAVEPOINTS_OLD
#else
	printf("Attempting to call SaveMetadataOld()! NOOP!\n");
#endif
	return;

#if	USE_SAVEPOINTS_OLD
	if(Sound==NULL)
	{
		return;
	}

	if(DatabaseEntryNow)
	{
		DatabaseEntryNow->BPM = GetBPM();
	}

	char metaDataPath[2048];
	GetMetadataPathOld(metaDataPath);

	char data[4096];
	sprintf
	(
		data,
		"Savepoints|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f|%.5f\n",
		SavepointSeconds[0],
		SavepointSeconds[1],
		SavepointSeconds[2],
		SavepointSeconds[3],
		SavepointSeconds[4],
		SavepointSeconds[5],
		SavepointSeconds[6],
		SavepointSeconds[7],
		SavepointSeconds[8],
		SavepointSeconds[9],
		SavepointSeconds[10],
		SavepointSeconds[11],
		SavepointSeconds[12],
		SavepointSeconds[13],
		SavepointSeconds[14],
		SavepointSeconds[15],
		SavepointSeconds[16],
		SavepointSeconds[17]
	);

	LGL_WriteFileAsync(metaDataPath,data,(int)strlen(data));

	if(MetadataSavedThisFrame)
	{
		delete MetadataSavedThisFrame;
		MetadataSavedThisFrame=NULL;
	}
	MetadataSavedThisFrame = new char[strlen(data)+2];
	strcpy(MetadataSavedThisFrame,data);
#endif
}

void
TurntableObj::
SaveMetadataNew()
{
	if
	(
		TesterEverEnabled ||
		GetInputTester().GetEnable()
	)
	{
		return;
	}

	if(Sound==NULL)
	{
		return;
	}

	if(DatabaseEntryNow)
	{
		bool bpmFound=false;
		for(int a=0;a<Savepoints.size();a++)
		{
			if(Savepoints[a].BPM>0.0f)
			{
				DatabaseEntryNow->BPM = Savepoints[a].BPM;
				bpmFound=true;
				break;
			}
		}

		if(bpmFound==false)
		{
			DatabaseEntryNow->BPM = BPM_UNDEF;
		}
	}

	char metaDataPath[2048];
	GetMetadataPathNew(metaDataPath);

	char data[1024*16];
	data[0]='\0';
	char tmp[2048];
	for(int a=0;a<Savepoints.size();a++)
	{
		if(Savepoints[a].Seconds!=-1.0f)
		{
			sprintf
			(
				tmp,
				"Savepoint|%.5f|%.5f\n",
				Savepoints[a].Seconds,
				Savepoints[a].BPM
			);
			strncat(data,tmp,sizeof(data)-1);
		}
	}

	sprintf
	(
		tmp,
		"FreqVideoLow|%s\n",
		VideoLoPath
	);
	strncat(data,tmp,sizeof(data)-1);
	sprintf
	(
		tmp,
		"FreqVideoHigh|%s\n",
		VideoHiPath
	);
	strncat(data,tmp,sizeof(data)-1);

	LGL_WriteFileAsync(metaDataPath,data,(int)strlen(data));

	if(MetadataSavedThisFrame)
	{
		delete MetadataSavedThisFrame;
		MetadataSavedThisFrame=NULL;
	}
	MetadataSavedThisFrame = new char[strlen(data)+2];
	strcpy(MetadataSavedThisFrame,data);
}

const char*
TurntableObj::
GetMetadataSavedThisFrame()	const
{
	//Allow Metadata to propagate to other turntables
	return(MetadataSavedThisFrame);
}

void
TurntableObj::
ResetAllCachedData()
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
}

void
TurntableObj::
LoadAllCachedData()
{
	LoadWaveArrayData();
	LoadCachedMetadata();

	LoadAllCachedDataDone=true;
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
	if(EntireWaveArrayFillIndex!=ENTIRE_WAVE_ARRAY_COUNT)
	{
		return;
	}

	char waveArrayDataPath[1024];
	sprintf(waveArrayDataPath,"%s/.dvj/cache/waveArrayData/%s.dvj-wavearraydata-%i.bin",LGL_GetHomeDir(),SoundName,ENTIRE_WAVE_ARRAY_COUNT);
	long len = 3*ENTIRE_WAVE_ARRAY_COUNT*sizeof(float);
	char* data = new char[len];
	float* dataFloat = (float*)data;
	memcpy(&dataFloat[ENTIRE_WAVE_ARRAY_COUNT*0],EntireWaveArrayMagnitudeAve,ENTIRE_WAVE_ARRAY_COUNT*sizeof(float));
	memcpy(&dataFloat[ENTIRE_WAVE_ARRAY_COUNT*1],EntireWaveArrayMagnitudeMax,ENTIRE_WAVE_ARRAY_COUNT*sizeof(float));
	memcpy(&dataFloat[ENTIRE_WAVE_ARRAY_COUNT*2],EntireWaveArrayFreqFactor,ENTIRE_WAVE_ARRAY_COUNT*sizeof(float));
	LGL_WriteFileAsync(waveArrayDataPath,data,(int)len);
	delete data;

	/*
	FILE* fd=fopen(waveArrayDataPath,"wb");
	if(fd)
	{
		fwrite(EntireWaveArrayMagnitudeAve,sizeof(float),ENTIRE_WAVE_ARRAY_COUNT,fd);
		fwrite(EntireWaveArrayMagnitudeMax,sizeof(float),ENTIRE_WAVE_ARRAY_COUNT,fd);
		fwrite(EntireWaveArrayFreqFactor,sizeof(float),ENTIRE_WAVE_ARRAY_COUNT,fd);
		EntireWaveArrayFillIndex=ENTIRE_WAVE_ARRAY_COUNT;
		fclose(fd);
	}
	*/
}

void
TurntableObj::
GetMetadataPathOld
(
	char*	dst
)
{
	sprintf(dst,"%s/.dvj/metadata/%s.dvj-metadata.txt",LGL_GetHomeDir(),SoundName);
}

void
TurntableObj::
GetMetadataPathNew
(
	char*	dst
)
{
	sprintf(dst,"%s/.dvj/metadata/%s.savepoints.txt",LGL_GetHomeDir(),SoundSrcNameDisplayed);
}

void
TurntableObj::
GetCacheMetadataPath
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
	GetCacheMetadataPath(cachedPath);

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
	GetCacheMetadataPath(cachedLengthPath);

	char data[2048];
	sprintf(data,"%.3f\n%f\n",CachedLengthSeconds,Sound->GetVolumePeak());

	int len = (int)strlen(data);
	LGL_WriteFileAsync(cachedLengthPath,data,len);

	/*
	FILE* fd=fopen(cachedLengthPath,"w");
	if(fd)
	{
		fprintf(fd,"%.3f\n",CachedLengthSeconds);
		fprintf(fd,"%f\n",Sound->GetVolumePeak());
		fclose(fd);
	}
	*/
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

const
char*
TurntableObj::
GetHighlightedPath()
{
	if(DatabaseFilteredEntries.size()==0)
	{
		return(NULL);
	}

	return
	(
		DatabaseFilteredEntries[ListSelector.GetHighlightedRow()]->PathFull
	);
}

const
char*
TurntableObj::
GetHighlightedPathShort()
{
	if
	(
		DatabaseFilteredEntries.size()==0 ||
		ListSelector.GetHighlightedRow() >= DatabaseFilteredEntries.size()
	)
	{
		return(NULL);
	}

	return
	(
		DatabaseFilteredEntries[ListSelector.GetHighlightedRow()]->PathShort
	);
}

const
char*
TurntableObj::
GetHighlightedNameDisplayed()
{
	if
	(
		DatabaseFilteredEntries.size()==0 ||
		ListSelector.GetHighlightedRow() >= DatabaseFilteredEntries.size()
	)
	{
		return(NULL);
	}

	return
	(
		DatabaseFilteredEntries[ListSelector.GetHighlightedRow()]->NameDisplayed
	);
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

float
TurntableObj::
GetSoundPositionPercent()
{
	if(GetSoundLoaded()==false)
	{
		return(0.0f);
	}

	return(GetTimeSeconds()/Sound->GetLengthSeconds());
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

SavepointObj*
TurntableObj::
GetCurrentSavepoint()
{
	if(SavepointIndex<Savepoints.size())
	{
		return(&(Savepoints[SavepointIndex]));
	}
	else
	{
		return(NULL);
	}
}

float
TurntableObj::
GetCurrentSavepointSeconds()
{
	if(SavepointObj* sp=GetCurrentSavepoint())
	{
		return(sp->Seconds);
	}
	else
	{
		return(-1.0f);
	}
}

float
TurntableObj::
GetCurrentSavepointBPM()
{
	if(SavepointObj* sp=GetCurrentSavepoint())
	{
		return(sp->BPM);
	}
	else
	{
		return(-1.0f);
	}
}

bool savepointSortPredicate(const SavepointObj& s1, const SavepointObj& s2)
{
	float s1Proxy = s1.Seconds;
	float s2Proxy = s2.Seconds;
	if(s1Proxy==-1.0f)
	{
		s1Proxy=999999999.0f;
	}
	if(s2Proxy==-1.0f)
	{
		s2Proxy=999999999.0f;
	}

	return
	(
		s1Proxy < s2Proxy
	);
}

void
TurntableObj::
SortSavepoints()
{
	float oldTime = GetCurrentSavepointSeconds();
	bool plus = SavepointIndexAtPlus();
	std::sort
	(
		Savepoints.begin(),
		Savepoints.end(),
		savepointSortPredicate
	);
	if(oldTime>=0.0f)
	{
		for(int a=0;a<Savepoints.size();a++)
		{
			if(Savepoints[a].Seconds == oldTime)
			{
				SavepointIndex=a;
				break;
			}
		}
	}
	else
	{
		SavepointIndex=0;
		if(plus)
		{
			LGL_LoopCounterAlpha();
			while(SavepointIndexAtPlus()==false)
			{
				LGL_LoopCounterDelta();
				SavepointIndex++;
			}
			LGL_LoopCounterOmega();
		}
	}
}

bool
TurntableObj::
SavepointIndexAtPlus()
{
	return
	(
		Mode==2 &&
		Savepoints[SavepointIndex].Seconds==-1.0f &&
		(
			SavepointIndex==0 ||
			Savepoints[SavepointIndex-1].Seconds!=-1.0f
		)
	);
}

float
TurntableObj::
GetSecondsToSync()
{
	if(LGL_MidiClockBPM()>0)
	{
		return(GetSecondsToSyncToMidiClock());
	}
	else
	{
		return(GetSecondsToSyncToOtherTT());
	}
}

float
TurntableObj::
GetSecondsToSyncToOtherTT()
{
	if
	(
		GetCurrentSavepointSeconds()<0
	)
	{
		return(-1.0f);
	}

	TurntableObj* otherTT = GetOtherTT();
	Pitchbend=1.0f;
	float targetSeconds = GetCurrentSavepointSeconds();
	float myBPM = GetBPM();
	if
	(
		targetSeconds>=0 &&
		myBPM>0 &&
		otherTT->GetMode()==2 &&
		otherTT->GetBPM()>0 &&
		otherTT->GetFinalSpeed()>0.0f
	)
	{
		const int measureGranularity=16;
		float otherBPMAdjusted = otherTT->GetBPMAdjusted();
		float best = otherBPMAdjusted/myBPM;
		for(int a=0;a<3;a++)
		{
			float mult = powf(2.0f,a-1);
			float candidate = otherBPMAdjusted/(mult*GetBPM());
			if(fabsf(1.0f-candidate)<fabsf(1.0f-best))
			{
				best=candidate;
			}
		}
		Pitchbend = best;
		SetBPMAdjusted(otherBPMAdjusted);
		//targetSeconds /= LGL_Max(0.01f,Pitchbend);
		targetSeconds += 
			//(LGL_AudioCallbackSamples()/(1.0f*LGL_AudioRate())) +
			Pitchbend/(LGL_Max(otherTT->GetFinalSpeed(),0.01f)) *
			(
				otherTT->GetTimeSeconds() -
				otherTT->GetBeginningOfCurrentMeasureSeconds(measureGranularity)
			);
	}

	return(targetSeconds);
}

float
TurntableObj::
GetSecondsToSyncToMidiClock()
{
	if
	(
		GetCurrentSavepointSeconds()<0
	)
	{
		return(-1.0f);
	}

	Pitchbend=1.0f;
	float targetSeconds = GetCurrentSavepointSeconds();
	float myBPM = GetBPM();
	if
	(
		targetSeconds>=0 &&
		myBPM>0 &&
		LGL_MidiClockBPM()>0
	)
	{
		const int measureGranularity=16;
		float otherBPMAdjusted = LGL_MidiClockBPM();
		float best = otherBPMAdjusted/myBPM;
		for(int a=0;a<3;a++)
		{
			float mult = powf(2.0f,a-1);
			float candidate = otherBPMAdjusted/(mult*GetBPM());
			if(fabsf(1.0f-candidate)<fabsf(1.0f-best))
			{
				best=candidate;
			}
		}
		Pitchbend = best;
		SetBPMAdjusted(otherBPMAdjusted);
		float otherSecondsIntoMeasure =
			LGL_MidiClockPercentOfCurrentMeasure(measureGranularity) *
			measureGranularity*4*(60.0f/otherBPMAdjusted);
		targetSeconds += 
			//(LGL_AudioCallbackSamples()/(1.0f*LGL_AudioRate())) +
			Pitchbend*
			otherSecondsIntoMeasure;
	}

	return(targetSeconds);
}

void
TurntableObj::
DeriveSoundStrings()
{
	if(GetHighlightedPath())
	{
		strcpy(SoundSrcNameDisplayed,GetHighlightedNameDisplayed());
		strcpy
		(
			 SoundSrcPath,
			 GetHighlightedPath()
		);
		strcpy(SoundSrcDir,SoundSrcPath);
		if(char* lastSlash = strrchr(SoundSrcDir,'/'))
		{
			lastSlash[0]='\0';
		}
		else
		{
			SoundSrcDir[0]='\0';
		}
		strcpy(SoundName,&(strrchr(SoundSrcPath,'/')[1]));
	}
}

void
TurntableObj::
CreateFindAudioPathThread()
{
	if(FindAudioPathThread)
	{
		printf("CreateFindAudioPathThread(): Warning! FindAudioPathThread already active!\n");
		return;
	}

	FindAudioPathDone=false;
	FoundAudioPathIsDir=false;
	DeriveSoundStrings();
	FindAudioPathThread=LGL_ThreadCreate(findAudioPathThread,this);
}

void
TurntableObj::
AttemptToCreateSound()
{
	if
	(
		Sound==NULL &&
		DatabaseFilteredEntries[ListSelector.GetHighlightedRow()]->Loadable
	)
	{
		if(FindAudioPathThread==NULL)
		{
			CreateFindAudioPathThread();
		}

		if(MetadataFileToRam==NULL)
		{
			char metaDataPath[2048];
			GetMetadataPathNew(metaDataPath);
			MetadataFileToRam=new LGL_FileToRam(metaDataPath);
		}

		if
		(
			FindAudioPathDone &&
			MetadataFileToRam->GetStatus()!=0
		)
		{
			if(MetadataFileToRam->GetFailed())
			{
				if(strstr(MetadataFileToRam->GetPath(),"savepoints.txt"))
				{
					//We failed to load the new metadata, so try old metadata path
					delete MetadataFileToRam;
					char metaDataPath[2048];
					GetMetadataPathOld(metaDataPath);
					MetadataFileToRam=new LGL_FileToRam(metaDataPath);
					return;
				}
			}

			if(FindAudioPathThread)
			{
				LGL_ThreadWait(FindAudioPathThread);
				FindAudioPathThread=NULL;
			}

			Savepoints.clear();
			LGL_LoopCounterAlpha();
			while(Savepoints.size()<12)
			{
				LGL_LoopCounterDelta();
				Savepoints.push_back(SavepointObj());
			}
			LGL_LoopCounterOmega();

			if(MetadataFileToRam->GetStatus()==1)
			{
				if(strstr(MetadataFileToRam->GetPath(),"savepoints.txt"))
				{
					LoadMetadataNew(MetadataFileToRam->GetData());
				}
				else
				{
					LoadMetadataOld(MetadataFileToRam->GetData());
				}
			}
			delete MetadataFileToRam;
			MetadataFileToRam=NULL;

			if(FoundAudioPathIsDir)
			{
				FilterTextMostRecent[0]='\0';
				FilterText.SetString("");
				FilterTextMostRecentBPM = (int)floorf(BPMMaster+0.5f);
				char oldDir[2048];
				strcpy(oldDir,&(strrchr(DatabaseFilter.GetDir(),'/')[1]));

				DatabaseFilter.SetDir(FoundAudioPath);
				DatabaseFilter.SetPattern(FilterText.GetString());

				//Just tried to create an audio track
				if(UpdateFilterListViaThread)
				{
					DatabaseFilter.Assign(UpdateFilterListDatabaseFilterNext);
					UpdateFilterListResetHighlightedRow=false;
				}
				else
				{
					GetEntryListFromFilterDance(NULL);
				}

				FilterText.SetString();
				NoiseFactor=1.0f;
				WhiteFactor=1.0f;

				Mode=3;
				return;
			}

			char videoTracksPath[2048];
			sprintf(videoTracksPath,"%s/%s",SoundSrcDir,GetDvjCacheDirName());

			ResetAllCachedData();

			LGL_DrawLogWrite("!dvj::NewSound|%s|%i\n",FoundAudioPath,Which);
			Sound=new LGL_Sound
			(
				FoundAudioPath,
				true,
				2,
				SoundBuffer,
				SoundBufferLength
			);

			Sound->SetVolumePeak(CachedVolumePeak);

			DatabaseEntryNow=DatabaseFilteredEntries[ListSelector.GetHighlightedRow()];
			DecodeTimer.Reset();
			SecondsLast=0.0f;
			SecondsNow=0.0f;
			VolumeInvertBinary=false;
			RhythmicVolumeInvert=false;
			RhythmicSoloInvert=false;
			LoopAlphaSeconds=-1.0;
			//QuantizePeriodMeasuresExponent=QuantizePeriodMeasuresExponent
			QuantizePeriodNoBPMSeconds=1.0;
			LoopActive=false;
			LoopThenRecallActive=false;
			AutoDivergeRecallActive=false;
			if(SavepointIndex>Savepoints.size()-1)
			{
				SavepointIndex=LGL_Max(0,Savepoints.size()-1);
			}

			//BPMRecalculationRequired=true;

			/*
			if(Video)
			{
				Video->InvalidateAllFrameBuffers();
			}
			*/
		}
	}
}

void
TurntableObj::
UpdateDatabaseFilterFn()
{
	/*
	if(UpdateFilterListThread)
	{
		NoiseFactor=LGL_Max(NoiseFactor,UpdateFilterListTimer.SecondsSinceLastReset()*4.0f);
	}
	*/

	//See if any old threads are complete
	if(DatabaseFilteredEntriesNextReady)
	{
		LGL_ThreadWait(UpdateFilterListThread);
		UpdateFilterListThread=NULL;

		if(UpdateFilterListAbortSignal==false)
		{
			DatabaseFilteredEntries=DatabaseFilteredEntriesNext;
			DatabaseFilteredEntriesNext.clear();
			UpdateListSelector();
			if(UpdateFilterListResetHighlightedRow)
			{
				ListSelector.SetHighlightedRow(1);
			}
			else if(UpdateFilterListDesiredSelection[0]!='\0')
			{
				bool ok = ListSelectorToString(UpdateFilterListDesiredSelection);
				if(ok==false)
				{
					ListSelector.SetHighlightedRow(0);
					ListSelector.CenterHighlightedRow();
				}
				UpdateFilterListDesiredSelection[0]='\0';
			}
		}

		UpdateFilterListDatabaseFilterNow.SetBPMCenter(-9999.0f);
		DatabaseFilteredEntriesNextReady=false;
	}

	//See if we should start a new thread
	if(UpdateFilterListDatabaseFilterNext.GetBPMCenter()!=-9999.0f)
	{
		if(UpdateFilterListThread)
		{
			UpdateFilterListAbortSignal=true;
		}
		else
		{
			UpdateFilterListDatabaseFilterNext.Assign(UpdateFilterListDatabaseFilterNow);
			UpdateFilterListDatabaseFilterNext.SetBPMCenter(-9999.0f);

			UpdateFilterListAbortSignal=false;
			UpdateFilterListThread=LGL_ThreadCreate(updateDatabaseFilterThread,this);
			UpdateFilterListTimer.Reset();
		}
	}
}

void
TurntableObj::
GetEntryListFromFilterDance
(
	const char*	oldSelection
)
{
	DatabaseFilteredEntries=Database->GetEntryListFromFilter(&DatabaseFilter);
	UpdateListSelector();

	if(oldSelection)
	{
		bool ok = ListSelectorToString(oldSelection);
		if(ok==false)
		{
			ListSelector.SetHighlightedRow(0);
			ListSelector.CenterHighlightedRow();
		}
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
Diverge()
{
	if(Sound==NULL)
	{
		return;
	}

	Sound->DivergeRecallPush(Channel,Pitchbend*PauseMultiplier);
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
	Sound->SetSpeed(Channel,Pitchbend*PauseMultiplier*ReverseMultiplier,true);
	Sound->DivergeRecallPop(Channel);
	if(Sound->GetDivergeRecallCount(Channel)==0)
	{
		Sound->SetWarpPoint(Channel);
	}

	SetSmoothWaveformScrollingSample(Sound->GetPositionSamples(Channel));
}

bool
TurntableObj::
RecallIsSet()
{
	return(Sound!=NULL && Sound->GetDivergeRecallCount(Channel)>0);
}

double
TurntableObj::GetQuantizePeriodSeconds()
{
	return(GetMeasureLengthSeconds()*pow(2,QuantizePeriodMeasuresExponent));
}

double
TurntableObj::GetBeatLengthSeconds()
{
	double bpm = GetBPM();
	if(bpm<=0)
	{
		printf("Warning! Called GetBeatLengthSeconds() when BPM is %.2f!\n",bpm);
		bpm=120.0f;
		assert(false);
	}
	if(bpm*Pitchbend<100.0f)
	{
		bpm*=2.0f;
	}
	if(bpm*Pitchbend>=200.0f)
	{
		bpm*=0.5f;
	}
	return(60.0/bpm);
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
	const float fudge=2.0f/3.0f;
	for(int a=0;a<LGL_EQ_LOWMID;a++)
	{
		FreqResponse[a]=LGL_Min(2.0f,EQFinal[0]*MixerEQ[0]);
	}
	for(int a=LGL_EQ_LOWMID;a<LGL_EQ_MIDHIGH;a++)
	{
		FreqResponse[a]=fudge*LGL_Min(2.0f,EQFinal[1]*MixerEQ[1]);
	}
	for(int a=LGL_EQ_MIDHIGH;a<513;a++)
	{
		FreqResponse[a]=fudge*LGL_Min(2.0f,EQFinal[2]*MixerEQ[2]);
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

int
TurntableObj::
GetFreqSenseLEDGroupInt()
{
	return
	(
		(int)
		LGL_Clamp
		(
			0,
			(int)floorf
			(
				FreqSenseLEDGroupFloat*
				GetVisualizer()->GetLEDGroupCount()
			),
			LGL_Max(0,GetVisualizer()->GetLEDGroupCount()-1)
		)
	);
}

float
TurntableObj::
GetEjectVisualBrightnessScalar()
{
	return
	(
		1.0f-
		NoiseFactor
	);
}

LGL_Color
TurntableObj::
GetFreqSenseLEDColorLow
(
	int	group
)
{
	group=LGL_Clamp(0,group,LED_GROUP_MAX-1);
	return(GetColorFromScalar(FreqSenseLEDColorScalarLow[group]));
}

LGL_Color
TurntableObj::
GetFreqSenseLEDColorHigh
(
	int	group
)
{
	group=LGL_Clamp(0,group,LED_GROUP_MAX-1);
	return(GetColorFromScalar(FreqSenseLEDColorScalarHigh[group]));
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

float exceedL=0.0f;
float exceedM=0.0f;
float exceedH=0.0f;

float
TurntableObj::
GetEQVUL()
{
	float val=0.0f;
	if
	(
		Sound &&
		Channel>=0
	)
	{
		val=Sound->GetEQVUL(Channel);
	}
	const float multFactor=0.025f;
	val*=multFactor;
	if(val>EQFinal[0]/2.0f)
	{
		if(val>exceedL)
		{
			exceedL=val;
			//printf("EQLow exceeded EQFinal: %.2f > %.2f\n",val,EQFinal[0]/2.0f);
			//printf("EQLow multFactor should be: %.3f\n",multFactor*((EQFinal[0]/2.0f)/val));
		}
		val=EQFinal[0]/2.0f;
	}
	val*=GetGain();
	return(LGL_Min(1.0f,val));
}

float
TurntableObj::
GetEQVUM()
{
	float val=0.0f;
	if
	(
		Sound &&
		Channel>=0
	)
	{
		val=Sound->GetEQVUM(Channel);
	}
	const float multFactor=0.10f;
	val*=multFactor;
	if(val>EQFinal[1]/2.0f)
	{
		if(val>exceedM)
		{
			exceedM=val;
			//printf("EQMid exceeded EQFinal: %.2f > %.2f\n",val,EQFinal[1]/2.0f);
			//printf("EQMid multFactor should be: %.3f\n",multFactor*((EQFinal[1]/2.0f)/val));
		}
		val=EQFinal[1]/2.0f;
	}
	val*=GetGain();
	return(LGL_Min(1.0f,val));
}

float
TurntableObj::
GetEQVUH()
{
	float val=0.0f;
	if
	(
		Sound &&
		Channel>=0
	)
	{
		val=Sound->GetEQVUH(Channel);
	}
	const float multFactor=0.42f;
	val*=multFactor;
	if(val>EQFinal[2]/2.0f)
	{
		if(val>exceedH)
		{
			exceedH=val;
			//printf("EQHi exceeded EQFinal: %.2f > %.2f\n",val,EQFinal[2]/2.0f);
			//printf("EQHi multFactor should be: %.3f\n",multFactor*((EQFinal[2]/2.0f)/val));
		}
		val=EQFinal[2]/2.0f;
	}
	val*=GetGain();
	return(LGL_Min(1.0f,val));
}

float
TurntableObj::
GetEQVUPeakL()
{
	return(EQPeak[0]);
}

float
TurntableObj::
GetEQVUPeakM()
{
	return(EQPeak[1]);
}

float
TurntableObj::
GetEQVUPeakH()
{
	return(EQPeak[2]);
}

float
TurntableObj::
GetVU()
{
	float val=0.0f;
	if
	(
		Sound &&
		Channel>=0
	)
	{
		val = Sound->GetVU(Channel);
	}
	val*=0.5f*GetGain();
	return(LGL_Min(1.0f,val));
}

float
TurntableObj::
GetVUPeak()
{
	return(VUPeak);
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

bool
TurntableObj::
GetFileEverOpened()
{
	return(FileEverOpened);
}

float
TurntableObj::
GetSecondsSinceFileEverOpened()
{
	if(FileEverOpened)
	{
		return(FileEverOpenedTimer.SecondsSinceLastReset());
	}
	else
	{
		return(0.0f);
	}
}

bool
TurntableObj::
GetSurroundMode()
{
	return(SurroundMode);
}

void
TurntableObj::
UpdateListSelector()
{
	ListSelector.Clear();

	int rowNow=0;

	for(int a=0;a<DatabaseFilteredEntries.size();a++)
	{
		if(DatabaseFilteredEntries[a]->IsDir)
		{
			ListSelector.SetCellColRowString
			(
				1,
				rowNow,
				DatabaseFilteredEntries[a]->NameDisplayed
			);
			ListSelector.SetCellColRowStringRGB
			(
				1,
				rowNow,
				0.0f,
				0.0f,
				1.0f
			);
			for(int a=0;a<2;a++)
			{
				ListSelector.GetCellColRow
				(
					a,
					rowNow
				)->UserData = DatabaseFilteredEntries[a];
			}
			rowNow++;
		}
	}

	for(int a=0;a<DatabaseFilteredEntries.size();a++)
	{
		if(DatabaseFilteredEntries[a]->IsDir==false)
		{
			if(DatabaseFilteredEntries[a]->BPM>0)
			{
				ListSelector.SetCellColRowString
				(
					0,
					rowNow,
					"%.0f",
					DatabaseFilteredEntries[a]->BPM
				);
			}
			ListSelector.SetCellColRowString
			(
				1,
				rowNow,
				DatabaseFilteredEntries[a]->NameDisplayed
			);
			for(int c=0;c<2;c++)
			{
				ListSelector.GetCellColRow
				(
					c,
					rowNow
				)->UserData = DatabaseFilteredEntries[a];
			}
			if(DatabaseFilteredEntries[a]->Loadable==false)
			{
				ListSelector.SetCellColRowStringRGB
				(
					1,
					rowNow,
					1.0f,
					0.0f,
					0.0f
				);
			}
			else if(DatabaseFilteredEntries[a]->AlreadyPlayed)
			{
				ListSelector.SetCellColRowStringRGB
				(
					1,
					rowNow,
					0.25f,
					0.25f,
					0.25f
				);
			}
			rowNow++;
		}
	}

	ListSelector.CenterHighlightedRow();
}

int
TurntableObj::
GetAspectRatioMode()
{
	return(AspectRatioMode);
}

void
TurntableObj::
SelectNewVideo()
{
	if(VideoLoPath[0]=='\0')
	{
		SelectNewVideoLow();
	}

	if(VideoHiPath[0]=='\0')
	{
		SelectNewVideoHigh();
	}
	
	if(VideoEncoderPercent==2.0f || VideoEncoderThread==NULL)
	{
		//Change the normal videos
		if(VideoFileExists)
		{
			if
			(
				Video &&
				strcmp(Video->GetPath(),FoundVideoPath)==0
			)
			{
				return;
			}

			Video->SetVideo(FoundVideoPath);
			VideoOffsetSeconds=0;
		}
		LGL_DrawLogWrite("!dvj::NewVideo|%s\n",Video->GetPath());
	}
}

void
TurntableObj::
SelectNewVideoLow()
{
	char path[4096];

	//Change the freq-videos
	Visualizer->GetNextVideoPathRandomLow(path);
	strcpy(VideoLoPath,path);
	if(const char* lastSlash = strrchr(VideoLoPath,'/'))
	{
		strcpy(VideoLoPathShort,&(lastSlash[1]));
	}
	else
	{
		strcpy(VideoLoPathShort,VideoLoPath);
	}

	SaveMetadataNew();
}

void
TurntableObj::
SelectNewVideoHigh()
{
	char path[2048];
	
	//Change the freq-videos

	Visualizer->GetNextVideoPathRandomHigh(path);
	strcpy(VideoHiPath,path);
	if(const char* lastSlash = strrchr(VideoHiPath,'/'))
	{
		strcpy(VideoHiPathShort,&(lastSlash[1]));
	}
	else
	{
		strcpy(VideoHiPathShort,VideoHiPath);
	}

	SaveMetadataNew();
}

bool
TurntableObj::
BPMAvailable()
{
	return(GetBPM()>0);
}

float
TurntableObj::
GetBPM()
{
	float bpm=BPM_UNDEF;

	if
	(
		Sound==NULL //||
		//Channel < 0
	)
	{
		return(BPM_UNDEF);
	}

	float time = GetTimeSeconds();
	bool breakWhenDefined=false;
	if(time-1.0f/Sound->GetHz()<Savepoints[0].Seconds)
	{
		breakWhenDefined=true;
	}
	for(int a=0;a<Savepoints.size();a++)
	{
//printf("GetBPM()[%i]: Alpha (%.2f => %.2f)\n",a,bpm,Savepoints[a].BPM);
		if(Savepoints[a].BPM > 0.0f)
		{
			bpm=Savepoints[a].BPM;
		}
		else if(Savepoints[a].BPM == BPM_NONE)
		{
			bpm=Savepoints[a].BPM;
		}
		else if(Savepoints[a].BPM == BPM_UNDEF)
		{
			//Respect the prior BPM
		}
		else
		{
			printf("Didn't ever expect to execute this line...\n");
			bpm=BPM_NONE;
		}

		float aPlusOne = LGL_Min(a+1,Savepoints.size()-1);

		if
		(
			time+1.0f/Sound->GetHz()>Savepoints[a].Seconds &&
			time-1.0f/Sound->GetHz()<Savepoints[aPlusOne].Seconds
		)
		{
			breakWhenDefined=true;
		}

		if
		(
			bpm!=BPM_UNDEF &&
			breakWhenDefined
		)
		{
//printf("GetBPM()[%i]: break (%.2f)\n",a,bpm);
			break;
		}
	}

	//printf("GetBPM(%.2f): %.2f\n",GetTimeSeconds(),bpm);

	if
	(
		isnan(bpm) ||
		isinf(bpm)
	)
	{
		bpm = BPM_NONE;
	}

	if
	(
		bpm!=BPM_UNDEF &&
		bpm!=BPM_NONE
	)
	{
		LGL_LoopCounterAlpha();
		while(bpm<100.0f)
		{
			LGL_LoopCounterDelta();
			bpm*=2.0f;
		}
		LGL_LoopCounterOmega();

		LGL_LoopCounterAlpha();
		while(bpm>200.0f)
		{
			LGL_LoopCounterDelta();
			bpm*=0.5f;
		}
		LGL_LoopCounterOmega();
	}
//printf("GetBPM(): %.2f\n",bpm);
	return(bpm);
}

float
TurntableObj::
GetBPMAdjusted
(
	bool	normalize
)
{
	float bpm=BPM_UNDEF;

	if(BPMAvailable())
	{
		bpm=GetBPM();
		if
		(
			isnan(bpm) ||
			isinf(bpm)
		)
		{
			return(BPM_NONE);
		}
	}

	if
	(
		normalize &&
		bpm!=BPM_UNDEF &&
		bpm!=BPM_NONE
	)
	{
		bpm*=Pitchbend;
		LGL_LoopCounterAlpha();
		while(bpm<100.0f)
		{
			LGL_LoopCounterDelta();
			bpm*=2.0f;
		}
		LGL_LoopCounterOmega();

		LGL_LoopCounterAlpha();
		while(bpm>200.0f)
		{
			LGL_LoopCounterDelta();
			bpm*=0.5f;
		}
		LGL_LoopCounterOmega();
	}

	return(bpm);
}

float
TurntableObj::
GetBPMFromDeltaSeconds
(
	float	deltaSeconds
)
{
	if(fabsf(deltaSeconds)<1.0f/44100.0f)
	{
		return(BPM_NONE);
	}

	LGL_LoopCounterAlpha();
	for(int a=0;a<200;a++)
	{
		LGL_LoopCounterDelta();
		if(deltaSeconds<10.0f)
		{
			deltaSeconds*=2.0f;
		}
		else
		{
			break;
		}
	}
	LGL_LoopCounterOmega();

	float bpm = -1.0f;
	int bpmMin=100;
	int measuresGuess=1;
	if(deltaSeconds!=0)
	{
		LGL_LoopCounterAlpha();
		for(int a=0;a<200;a++)
		{
			LGL_LoopCounterDelta();
			float bpmGuess=(4*measuresGuess)/(deltaSeconds/60.0f);
			if(bpmGuess>=bpmMin)
			{
				bpm=bpmGuess;
				break;
			}
			measuresGuess*=2;
		}
		LGL_LoopCounterOmega();
	}

	if
	(
		bpm>=100 &&
		bpm<=200
	)
	{
		return(bpm);
	}
	else
	{
		return(BPM_UNDEF);
	}
}

float
TurntableObj::
GetBPMFirstBeatSeconds()
{
	if(Savepoints.size()>0)
	{
		return(Savepoints[0].Seconds);
	}

	return(0.0f);
}

float
TurntableObj::
GetBPMFirstMeasureSeconds()
{
	if(BPMAvailable()==false)
	{
		return(0.0f);
	}

	double firstBeatSeconds = GetBPMFirstBeatSeconds();
	double deltaMeasure = GetMeasureLengthSeconds();
	double candidate = firstBeatSeconds;
	LGL_LoopCounterAlpha();
	while(candidate - deltaMeasure > 0)
	{
		LGL_LoopCounterDelta();
		candidate -= deltaMeasure;
	}
	LGL_LoopCounterOmega();

	return(candidate);
}

float
TurntableObj::
GetBPMLastMeasureSeconds()
{
	if(BPMAvailable()==false)
	{
		return(0.0f);
	}

	double firstBeatSeconds = GetBPMFirstBeatSeconds();
	double deltaMeasure = GetMeasureLengthSeconds();
	double candidate = firstBeatSeconds;

	//Loop Optimization
	double sndLen = Sound->GetLengthSeconds();
	double dist = sndLen - candidate;
	double delta = floorf(dist/deltaMeasure);
	candidate += deltaMeasure * delta;

	LGL_LoopCounterAlpha();
	while(candidate + deltaMeasure < sndLen-0.01f)
	{
		LGL_LoopCounterDelta();
		candidate += deltaMeasure;
	}
	LGL_LoopCounterOmega();

	return(candidate);
}

float
TurntableObj::
GetBPMAnchorMeasureSeconds()
{
	if(BPMAvailable()==false)
	{
		return(0.0f);
	}

	float best=0.0f;
	if(Savepoints.size()>0)
	{
		best=Savepoints[0].Seconds;
	}
	for(int a=0;a<Savepoints.size();a++)
	{
		float candidate=Savepoints[a].Seconds;
		if
		(
			candidate>best &&
			candidate<=GetTimeSeconds()
		)
		{
			best=candidate;
		}
	}

	return(best);
}

void
TurntableObj::
SetBPMAdjusted
(
	float	bpmAdjusted
)
{
	if(GetBPM()>0)
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
		PitchbendLastSetByXponentSlider=false;
	}
}

void
TurntableObj::
SetBPMMaster
(
	float bpmMaster
)
{
	if
	(
		FilterTextMostRecentBPM!=(int)floorf(bpmMaster+0.5f) &&
		GetFocus()==false &&
		strchr(FilterTextMostRecent,'~')
	)
	{
		//Reevaluate filtered entries
		FilterTextMostRecent[0]='\0';
		FilterTextMostRecentBPM = (int)floorf(bpmMaster+0.5f);
	}

	BPMMaster=bpmMaster;
}

void
TurntableObj::
SetPitchbend
(
	float	pitchbend
)
{
	Pitchbend=pitchbend;
}

float
TurntableObj::
GetPitchbend()
{
	return(Pitchbend);
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
	if(Channel<0)
	{
		return(false);
	}
	if(BPMAvailable()==false)
	{
		return(false);
	}

	double startBeat = GetBPMFirstMeasureSeconds();
	double deltaBeat = fractionOfBeat * (60.0/(double)GetBPM());
	if(deltaBeat<=0.0f)
	{
		return(false);
	}
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

	//Loop Optimization
	double dist = windowStart - candidate;
	double delta = floorf(dist/deltaBeat);
	candidate += delta * deltaBeat;

	LGL_LoopCounterAlpha();
	while(candidate < windowEnd)
	{
		LGL_LoopCounterDelta();
		if
		(
			candidate>=windowStart &&
			candidate< windowEnd
		)
		{
			LGL_LoopCounterOmega();
			return(true);
		}
		candidate+=deltaBeat;
	}
	LGL_LoopCounterOmega();

	return(false);
}

double
TurntableObj::
GetPercentOfCurrentMeasure
(
	float	measureMultiplier,
	bool	recalculateSecondsNow
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

	double startMeasure = GetBeginningOfCurrentMeasureSeconds(measureMultiplier,recalculateSecondsNow);
	double deltaMeasure = measureMultiplier*GetMeasureLengthSeconds();

	//LGL_DebugPrintf("Start: %.2f\n",startMeasure);
	//LGL_DebugPrintf("Delta: %.2f\n",deltaMeasure);

	float secondsNow = recalculateSecondsNow ? GetTimeSeconds(true) : SecondsNow;
	return(LGL_Clamp(0,(secondsNow-startMeasure)/deltaMeasure,1));
}

double
TurntableObj::
GetBeginningOfCurrentMeasureSeconds
(
	float	measureMultiplier,
	bool	recalculateSecondsNow
)
{
	float secondsNow = recalculateSecondsNow ? GetTimeSeconds(true) : SecondsNow;
	return(GetBeginningOfArbitraryMeasureSeconds(secondsNow,measureMultiplier));
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
	
	double deltaPeriod = measureMultiplier*GetMeasureLengthSeconds();
	double candidate = GetBPMAnchorMeasureSeconds();

	//Loop Optimization
	float dist = seconds-candidate;
	float delta = floorf(dist/deltaPeriod);
	candidate += delta*deltaPeriod;

	if(candidate==seconds)
	{
		//Huzzah!
	}
	else if(candidate < seconds)
	{
		LGL_LoopCounterAlpha();
		while(candidate+deltaPeriod<seconds+0.001f)
		{
			LGL_LoopCounterDelta();
			candidate+=deltaPeriod;
		}
		LGL_LoopCounterOmega();
	}
	else
	{
		LGL_LoopCounterAlpha();
		while(candidate-0.001f>seconds)
		{
			LGL_LoopCounterDelta();
			candidate-=deltaPeriod;
		}
		LGL_LoopCounterOmega();
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
GetRhythmicSoloInvert()
{
	return(RhythmicSoloInvert);
}

void
TurntableObj::
SetRespondToRhythmicSoloInvert
(
	int	soloChannel
)
{
	if(Sound)
	{
		Sound->SetRespondToRhythmicSoloInvertChannel
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

bool
TurntableObj::
ListSelectorToString
(
	const char*	str
)
{
	if(str==NULL)
	{
		return(false);
	}

	bool ok=false;

	for(unsigned int a=0;a<DatabaseFilteredEntries.size();a++)
	{
		if(strcmp(DatabaseFilteredEntries[a]->NameDisplayed,str)==0)
		{
			ListSelector.SetHighlightedRow(a);
			ListSelector.CenterHighlightedRow();
			ok=true;
			break;
		}
	}

	return(ok);
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

void
TurntableObj::
SetSmoothWaveformScrollingSample
(
	double	sample
)
{
	if(Sound->GetLengthSeconds()<1.0f)
	{
		sample = LGL_Clamp(0.0f,sample,LGL_Max(0,Sound->GetLengthSamples()-1));
	}

	//Allow for a little straying...
	if
	(
		sample < 0 &&
		sample > Sound->GetHz()*-10.0f
	)
	{
		sample=0;
	}
	if
	(
		sample >= Sound->GetLengthSamples() &&
		sample < Sound->GetLengthSamples() + Sound->GetHz()*10
	)
	{
		sample=LGL_Max(0,Sound->GetLengthSamples()-1);
	}

	if
	(
		sample<0 ||
		(
			sample!=0 &&
			sample>=Sound->GetLengthSamples()
		)
	)
	{
		printf
		(
			"Warning! SetSmoothWaveformScrollingSample given invalid input: 0 <= %.2f < %.2f\n",
			sample,
			(double)Sound->GetLengthSamples()
		);
		LGL_AssertIfDebugMode();

		sample = LGL_Clamp(0,sample,Sound->GetLengthSamples()-1);
	}

	SmoothWaveformScrollingSample = sample;
}

