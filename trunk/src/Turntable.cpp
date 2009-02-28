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
#include <string.h>
#include <stdlib.h>

#define	BEATMANIA

#define	GLITCH_PURE_VIDEO_MULTIPLIER (5.0f)

const int lm = 8;	//Low => Mid transition
const int mh = 40;	//Mid => High transition
int ENTIRE_WAVE_ARRAY_COUNT;

VisualizerObj* TurntableObj::Visualizer;
LGL_Image* TurntableObj::NoiseImage[NOISE_IMAGE_COUNT_256_64];

TurntableObj::
TurntableObj
(
	float	left,	float	right,
	float	bottom,	float	top
)
{
	Mode=0;

	SetViewPort(left,right,bottom,top);
	SetFocus(false);
	
	Sound=NULL;
	sprintf(SoundName,"No Track Selected");
	SoundBufferLength=4*44100*60*20;
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

	CenterX=.5f*(ViewPortLeft+ViewPortRight);
	CenterY=.5f*(ViewPortBottom+ViewPortTop);

	LGL_Assertf(LGL_DirectoryExists("data/music"),("Error! Must create directory data/music!\n"));
	DirTree.SetPath("data/music");
	DirTree.WaitOnWorkerThread();
	FilterTextMostRecent[0]='\0';

	FileTop=DirTree.GetFilteredDirCount();
	FileSelectInt=FileTop;
	FileSelectFloat=(float)FileTop;
	UpdateFileBPM();

	BadFileFlash=0.0f;

	Channel=-1;
	PauseMultiplier=0;
	Nudge=0;
	NudgeFromMixer=0;
	FinalSpeed=0.0f;
	RewindFF=false;
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
	LoopStartSeconds=-1.0;
	LoopLengthMeasures=0;
	LoopAtEndOfMeasure=false;
	SavePointIndex=0;

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

	TrackListFileUpdates.clear();

	VideoFront=NULL;
	VideoBack=NULL;
	VideoAdvanceRate=1.0f;

	ENTIRE_WAVE_ARRAY_COUNT=LGL_VideoResolutionX();

	ClearRecallOrigin();

	WhiteFactor=0.0f;
	NoiseFactor=0.0f;
	NoiseFactorVideo=0.0f;
	if(NoiseImage[0]==NULL)
	{
		for(int a=0;a<NOISE_IMAGE_COUNT_256_64;a++)
		{
			char path[1024];
			sprintf
			(
				path,
				"data/noise/256x64/%02i.png",
				a
			);
			assert(LGL_FileExists(path));
			NoiseImage[a] = new LGL_Image(path);
		}
	}

	LowRez=false;
}

TurntableObj::
~TurntableObj()
{
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
				LGL_DelayMS(1);
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
	unsigned int target =
		(Focus ? TARGET_FOCUS : 0) |
		((Which==0) ? TARGET_TOP : TARGET_BOTTOM);
	float candidate = 0.0f;

	for(unsigned int a=0;a<TrackListFileUpdates.size();a++)
	{
		delete TrackListFileUpdates[a];
		TrackListFileUpdates[a]=NULL;
	}
	TrackListFileUpdates.clear();
	MetaDataSavedThisFrame=false;

	bool volumeFull=false;
	bool endpointsSticky=true;
	float localVolumeMultiplier=1.0f;
	bool noiseIncreasing=false;
	bool glitchPurePrev=GlitchPure;

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

	if(Mode==0)
	{
		int fileTopOld = FileTop;
		bool filterDelta = false;
		//File Select

		BadFileFlash-=2.0f*LGL_SecondsSinceLastFrame();
		if(BadFileFlash<0.0f)
		{
			BadFileFlash=0.0f;
		}

		if(Focus)
		{
			if(LGL_KeyStroke(SDLK_ESCAPE))
			{
				FilterText.SetString();
			}

			FilterText.GrabFocus();
			if(strcmp(FilterText.GetString(),FilterTextMostRecent)!=0)
			{
				filterDelta=true;
				strcpy(FilterTextMostRecent,FilterText.GetString());

				//First, we must make note of our currently selected file.
				char* oldSelection=NULL;
				if(GetCurrentFileString()!=NULL)
				{
					char* oldSelection=new char[strlen(GetCurrentFileString())+1];
					strcpy(oldSelection,GetCurrentFileString());
				}
				//Now we update our DirTree
				DirTree.SetFilterText(FilterText.GetString());
				LGL_DrawLogWrite("!DirTreeFilter|%i|%s\n",Which,DirTree.GetFilterText());

				bool oldSelectionFound=false;

				if(oldSelection!=NULL)
				{
					//Finally, we try to seek to our previously highlighted file
					unsigned int fileNum=DirTree.GetFilteredDirCount()+DirTree.GetFilteredFileCount();
					if(fileNum> 0)
					{
						for(unsigned int z=0;z<fileNum;z++)
						{
							const char* fileNow;
							fileNow=(z<DirTree.GetFilteredDirCount()) ?
								DirTree.GetFilteredDirName(z) :
								DirTree.GetFilteredFileName(z - DirTree.GetFilteredDirCount());
							if(strcmp(fileNow,oldSelection)==0)
							{
								FileTop=z-4;
								if(FileTop<0) FileTop=0;
								FileSelectInt=z;
								FileSelectFloat=z;
								oldSelectionFound=true;
							}
						}
					}
					delete oldSelection;
				}
				
				if(oldSelectionFound==false)
				{
					FileTop=DirTree.GetFilteredDirCount();
					FileSelectInt=FileTop;
					FileSelectFloat=(float)FileTop;
				}
			}
		}
		else
		{
			FilterText.ReleaseFocus();
		}

		if(Input.FileSelect(target))
		{
			if
			(
				FileSelectInt>=(int)DirTree.GetFilteredDirCount() &&
				DirTree.GetFilteredFileCount()>0
			)
			{
				sprintf
				(
					 SoundName,
					 "%s",
					 (FileSelectInt<(int)DirTree.GetFilteredDirCount()) ?
					 DirTree.GetFilteredDirName(FileSelectInt) :
					 DirTree.GetFilteredFileName(FileSelectInt - DirTree.GetFilteredDirCount())
				);
				char filename[512];
				sprintf
				(
					 filename,
					 "%s/%s",
					 DirTree.GetPath(),
					 SoundName
				);
				if(LGL_FileExists(filename))
				{
					LGL_DrawLogWrite("!dvj::NewSound|%s|%i\n",filename,Which);
					Sound=new LGL_Sound
					(
						 filename,
						 true,
						 2,
						 SoundBuffer,
						 SoundBufferLength
					);
					if(Sound->IsUnloadable())
					{
						LGL_DrawLogWrite("!dvj::DeleteSound|%i\n",Which);
						Sound->PrepareForDelete();
						Mode=3;
						BadFileFlash=1.0f;
						if(GetVideo()!=NULL)
						{
							LGL_DrawLogWrite("!dvj::DeleteVideo|%s\n",GetVideo()->GetPath());
						}
						return;
					}
					else
					{
						Mode=1;
						DecodeTimer.Reset();
						SecondsLast=0.0f;
						SecondsNow=0.0f;
						VolumeInvertBinary=false;
						LoopStartSeconds=-1.0;
						LoopLengthMeasures=0;
						LoopAtEndOfMeasure=false;
						SavePointIndex=0;
						for(int a=0;a<18;a++)
						{
							SavePointSeconds[a]=-1.0f;
							SavePointUnsetNoisePercent[a]=0.0f;
							SavePointUnsetFlashPercent[a]=0.0f;
						}
						LoadMetaData();
						FilterText.ReleaseFocus();
						ClearRecallOrigin();

						char hintpath[512];
						sprintf
						(
							 hintpath,
							 "%s/.hints/%s.hint",
							 DirTree.GetPath(),
							 SoundName
						);

						ProcessHintFile(hintpath);

						char* update=new char[1024];
						sprintf(update,"ALPHA: %s",SoundName);
						TrackListFileUpdates.push_back(update);

						WhiteFactor=1.0f;
						NoiseFactor=1.0f;
						NoiseFactorVideo=1.0f;
					}
				}
			}
			else if(DirTree.GetFilteredDirCount()>0)
			{
				DirTree.SetPath(DirTree.GetFilteredDirName(FileSelectInt));
				DirTree.SetFilterText();
				FilterTextMostRecent[0]='\0';
				FilterText.SetString("");
				LGL_DrawLogWrite("!DirTreePath|%i|%s\n",Which,DirTree.GetPath());
				FileTop=0;
				FileSelectInt=0;
				FileSelectFloat=0.0f;
				FilterText.SetString();
				UpdateFileBPM();
				NoiseFactor=1.0f;
				WhiteFactor=1.0f;
			}
		}

		if(Input.FileRefresh(target))
		{
			DirTree.Refresh();
			if((unsigned int)FileSelectInt >= DirTree.GetFilteredFileCount() + DirTree.GetFilteredDirCount())
			{
				FileTop=0;
				FileSelectInt=0;
				FileSelectFloat=0.0f;
			}
		}

		if(DirTree.Ready())
		{
			FileSelectFloat=
				FileSelectInt +
				Input.FileScroll(target);

			if(FileSelectFloat<0)
			{
				FileSelectFloat=0;
			}
			if(FileSelectFloat+0.5f >= DirTree.GetFilteredDirCount()+DirTree.GetFilteredFileCount())
			{
				FileSelectFloat=LGL_Max(0,((int)(DirTree.GetFilteredDirCount()))+((int)(DirTree.GetFilteredFileCount()))-1);
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

			if
			(
				FileTop!=fileTopOld ||
				filterDelta
			)
			{
				UpdateFileBPM();
			}
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
			if(GetVideo()!=NULL)
			{
				LGL_DrawLogWrite("!dvj::DeleteVideo|%s\n",GetVideo()->GetPath());
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
		if(recordHold) RecordHoldReleaseTimer.Reset();

		//Scratch / Queue
		{
			if(rewindFFFactor!=0.0f)
			{
				//Rewind
				endpointsSticky=true;
				recordSpeed=rewindFFFactor;
				float normalVolFactor=(fabsf(rewindFFFactor)<2.0f) ? 1.0f : LGL_Max(0.0f,1.0f-fabsf(fabsf(rewindFFFactor)-2.0f)/2.0f);
				localVolumeMultiplier=
					(0.0f+normalVolFactor)*1.00f+
					(1.0f-normalVolFactor)*0.25f;
				LoopStartSeconds=-1.0;
				if
				(
					(long)Sound->GetPositionSamples(Channel) == 0 &&
					RewindFF==false &&
					rewindFFFactor<0.0f
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
				RecordScratch=true;
				LoopStartSeconds=-1.0;

				float driveFactor=LGL_Min(RecordHoldReleaseTimer.SecondsSinceLastReset()*4.0f,1.0f);

				float driveSpeed = 
					(1.0f-driveFactor) * recordSpeed +
					(0.0f+driveFactor) * (Pitchbend*PauseMultiplier) * (RewindFF ? 0.0f : 1.0f);

				FinalSpeed=
					driveSpeed+
					recordSpeed+
					Nudge+
					NudgeFromMixer;
			}
			else
			{
				RecordScratch=false;
			}
		}

		//Volume
		VolumeInvertBinary=Input.WaveformVolumeInvert(target);

		//Save Points
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
					LGL_Min(1,Input.WaveformSavePointUnsetPercent(target)*2.0f)
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
			LoopStartSeconds=-1.0;
			double savePointSecondsOrig=SavePointSeconds[SavePointIndex];
			double savePointSeconds=savePointSecondsOrig;
			if(savePointSeconds < 0.0f)
			{
				if(Sound->IsLoaded()==false)
				{
					savePointSeconds=0.0f;
				}
				else
				{
					savePointSeconds += Sound->GetLengthSeconds();
				}
			}

			if(savePointSecondsOrig>=0)
			{
				if(Sound->GetLengthSeconds()>savePointSeconds)
				{
					Sound->SetPositionSeconds(Channel,savePointSeconds);
				}
			}
			else
			{
				if(Sound->IsLoaded())
				{
					Sound->SetPositionSeconds(Channel,Sound->GetLengthSeconds()+savePointSecondsOrig);
				}
				else
				{
					Sound->SetPositionSeconds(Channel,0.0f);
				}
			}
		}
		if
		(
			Input.WaveformSavePointJumpAtMeasure(target) &&
			SavePointSeconds[SavePointIndex]!=-1.0f &&
			GetBPM()!=0 &&
			PauseMultiplier!=0 &&
			SecondsNow>=GetBPMFirstBeatSeconds()
		)
		{
			LoopStartSeconds=-1.0;
			double savePointSecondsOrig=SavePointSeconds[SavePointIndex];
			double savePointSeconds=savePointSecondsOrig;
			if(savePointSeconds < 0.0f)
			{
				if(Sound->IsLoaded()==false)
				{
					savePointSeconds=0.0f;
				}
				else
				{
					savePointSeconds += Sound->GetLengthSeconds();
				}
			}
	
			//Jump at start of next measure
			double beatStart=GetBPMFirstBeatSeconds();
			double measureLength=(1.0f/GetBPM())*60.0f*(4.0f);
			double savePointSecondsQuantized=beatStart;
			double closest=99999.0;
			for(double test=beatStart;test<savePointSeconds+2*measureLength;test+=0.25*measureLength)
			{
				if(fabsf(test-savePointSecondsOrig)<closest)
				{
					closest=fabsf(test-savePointSecondsOrig);
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
			if
			(
				Sound->IsLoaded() &&
				beatStart<0
			)
			{
				savePointSecondsQuantized = Sound->GetLengthSeconds()+beatStart;
			}
			Sound->SetWarpPoint(Channel,timeToWarp,savePointSecondsQuantized);
			Sound->SetDivergeRecallBegin(Channel,Pitchbend);
		}

		//Pitch
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

		//Glitch
		if(Input.WaveformStutter(target))
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

		//Loop
		if(BPMAvailable())
		{
			if(Input.WaveformLoopBegin(target))
			{
				LoopStartSeconds=GetBeginningOfCurrentMeasureSeconds();
				LoopLengthMeasures=0;
				Sound->SetWarpPoint(Channel);
			}

			if(Input.WaveformLoopEnd(target))
			{
				if(LoopStartSeconds>=0.0f)
				{
					float deltaMeasure = 4*(60.0f/GetBPM());
					float test=LoopStartSeconds;
					LoopLengthMeasures=0;
					while(test<Sound->GetPositionSeconds(Channel))
					{
						LoopLengthMeasures++;
						test+=deltaMeasure;
					}
				}
			}

			if(Input.WaveformLoopDisable(target))
			{
				LoopStartSeconds=-1.0f;
				LoopLengthMeasures=0;
				Sound->SetWarpPoint(Channel);
			}

			candidate=Input.WaveformLoopMeasures(target);
			if(candidate!=-1.0f)
			{
				LoopLengthMeasures=(int)candidate;
				if(LoopStartSeconds<0) LoopStartSeconds=GetBeginningOfCurrentMeasureSeconds();
			}
		}

		if(LoopActive())
		{
			if(Sound->GetWarpPointIsSet(Channel)==false)
			{
				//Clear after we've looped
				LoopAtEndOfMeasure=false;
			}

			double deltaMeasure = 4*(60.0/(double)GetBPM());
			int measures = LoopLengthMeasures;
			if(LoopAtEndOfMeasure)
			{
				const double epsilon = 0.01;
				double endSeconds = GetBeginningOfCurrentMeasureSeconds()+deltaMeasure-epsilon;
				measures=1;
				while(LoopStartSeconds+measures*deltaMeasure < endSeconds)
				{
					measures++;
				}
			}
			if(measures>0)
			{
				while(LoopStartSeconds + deltaMeasure*measures < SecondsNow)
				{
					measures++;
				}
				Sound->SetWarpPoint
				(
					Channel,
					LoopStartSeconds + deltaMeasure*measures,
					LoopStartSeconds
				);
			}
		}

		//Video
		if(Input.WaveformVideoSelect(target))
		{
			SelectNewVideo();
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

			FinalSpeed=PauseMultiplier*(Pitchbend+NudgeFromMixer)+Nudge;
		}

		Sound->SetStickyEndpoints(Channel,endpointsSticky);
		Sound->SetSpeed
		(
			Channel,
			FinalSpeed
		);
			
		//Pause
		if(Input.WaveformTogglePause(target))
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
		if(Input.WaveformEject(target))
		{
			if(EjectTimer.SecondsSinceLastReset()>=1.0f)
			{
				//Select New Track
				LGL_DrawLogWrite("!dvj::DeleteSound|%i\n",Which);
				Sound->PrepareForDelete();
				Channel=-1;
				Mode=3;
				if(GetVideo()!=NULL)
				{
					LGL_DrawLogWrite("!dvj::DeleteVideo|%s\n",GetVideo()->GetPath());
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
		if(Sound->ReadyForDelete())
		{
			delete Sound;
			Sound=NULL;
			UpdateFileBPM();
			Mode=0;
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
				float leftSample=centerSample-64*512*Pitchbend;
				float rightSample=centerSample+64*512*Pitchbend;

				float gSamplePercent=0.5f+(Input.WaveformPointerScratch(target)-0.5f)*(1.0f/WAVE_WIDTH_PERCENT);

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
				GlitchBegin=Sound->GetPositionGlitchBeginSamples(Channel)-.005f*64*512*Pitchbend;
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
				Sound!=NULL &&
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
						double measureLength=(1.0f/GetBPM())*60.0f*(4.0f);
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

			if(Visualizer!=NULL && MixerVolumeFront>.5f)
			{
				Visualizer->SetImageSetPrefix(ImageSetPrefix);
				Visualizer->SetMovieClipPrefix(MovieClipPrefix);
			}
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
		
		if(Sound->GetLengthSamples()>0)//Sound->IsLoaded())
		//if(Sound->IsLoaded())
		{
			Mode=2;

			BPMRecalculationRequired=true;

			//Load Video if possible
			SelectNewVideo();

			Channel=Sound->Play(1,true,0);
			Sound->SetSpeedInterpolationFactor
			(
				Channel,
				//16.0/44100.0
				20.0/44100.0
			);
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
			NudgeFromMixer=0;
			GlitchPure=false;
			GlitchPureDuo=false;
			GlitchDuo=false;
			GlitchVolume=0;
			GlitchBegin=0;
			GlitchLength=0;
			GlitchPitch=1.0f;
			SmoothWaveformScrollingSample=0.0f;
			VideoOffsetSeconds=LGL_RandFloat(0,1000.0f);

			LoadAllCachedData();

			char videoFileName[1024];
			sprintf(videoFileName,"data/video/tracks/%s.mjpeg.avi",SoundName);
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
				Sound->SetPositionSeconds(Channel,0);
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
			}
			Sound->SetSpeedInterpolationFactor
			(
				Channel,
				16.0/44100.0
			);
		}

		if
		(
			CachedLengthSeconds!=0.0f &&
			Sound->GetLengthSeconds()>CachedLengthSeconds
		)
		{
			//
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
		vol *= (1.0f-NoiseFactor);
		float volFront = VolumeInvertBinary ?
			(MixerVolumeFront==0.0f ? 1.0f : 0.0f) :
			MixerVolumeFront;
		float volBack = VolumeInvertBinary ?
			(MixerVolumeBack==0.0f ? 1.0f : MixerVolumeBack) :
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

	WhiteFactor = LGL_Max(0.0f,WhiteFactor-4.0f*LGL_SecondsSinceLastFrame());
	if(noiseIncreasing==false)
	{
		if
		(
			Mode!=0 ||
			DirTree.Ready()
		)
		{
			NoiseFactor = LGL_Max(0.0f,NoiseFactor-2.0f*LGL_SecondsSinceLastFrame());
		}
	}
}

long lastSampleLeft=0;
float badFlash=0.0f;

void
TurntableObj::
DrawFrame
(
	float	glow,
	bool	visualsQuadrent,
	float	visualizerZoomOutPercent
)
{
	if(visualsQuadrent==false)
	{
		LGL_ClipRectEnable(ViewPortLeft,ViewPortRight,ViewPortBottom,ViewPortTop);
	}
	else
	{
		LGL_ClipRectEnable(0.0f,0.5f,0.5f,1.0f);
	}

	float rectAlpha=1.0f;
	if(GetVideo()!=NULL)
	{
		//On Left
		float left=	ViewPortLeft;
		float right=	0.5f-0.501f*WAVE_WIDTH_PERCENT*ViewPortWidth;

		float bottom=	ViewPortBottom+0.125*ViewPortHeight;
		float top=	ViewPortBottom+0.875*ViewPortHeight;

		LGL_Image* image=GetVideo()->LockImage();
		if(image!=NULL)
		{
			rectAlpha=0.0f;
			image->DrawToScreen
			(
				left,
				right,
				bottom,
				top
			);
		}
		GetVideo()->UnlockImage(image);

		bool videoReady = VideoFront->GetImageDecodedSinceVideoChange();
		if(videoReady)
		{
			NoiseFactorVideo=LGL_Max(0.0f,NoiseFactorVideo-4.0f*LGL_SecondsSinceLastFrame());
		}
		else
		{
			NoiseFactorVideo=1.0f;
		}

		if(NoiseFactorVideo>0.0f)
		{
			int which = LGL_RandInt(0,NOISE_IMAGE_COUNT_256_64-1);
			NoiseImage[which]->DrawToScreen
			(
				left,
				right,
				bottom,
				top,
				0,
				NoiseFactorVideo,
				NoiseFactorVideo,
				NoiseFactorVideo,
				NoiseFactorVideo,
				false,false,0,0,0,
				0.0f,
				0.25f,
				0.0f,
				1.0f
			);
		}

		float secondsSinceVideoChange = VideoFront->GetSecondsSinceVideoChange();
		float whiteFactor=LGL_Max(0.0f,1.0f-4.0f*secondsSinceVideoChange);
		if(whiteFactor>0.0f)
		{
			LGL_DrawRectToScreen
			(
				left,
				right,
				bottom,
				top,
				whiteFactor,
				whiteFactor,
				whiteFactor,
				0.0f
			);
		}

		rectAlpha=0.0f;
	}

	if(Focus)
	{
		LGL_DrawRectToScreen
		(
			ViewPortLeft,ViewPortRight,
			ViewPortBottom,ViewPortTop,
			0,0,.15*glow,rectAlpha
		);
	}
	
	if(Mode==0)
	{
		//File Selection

		if(DirTree.Ready())
		{
			LGL_DrawLogPause();
			Turntable_DrawDirTree
			(
				LGL_SecondsSinceExecution()*Focus,
				&DirTree,
				FileTop,
				FileSelectInt,
				ViewPortBottom,
				ViewPortTop,
				BadFileFlash,
				FileBPM
			);
			LGL_DrawLogPause(false);
			LGL_DrawLogWrite("DirTreeDraw|%i|%i|%i|%.2f\n",Which,FileTop,FileSelectInt,BadFileFlash);
		}
	}
	if(Mode==1)
	{
		//Loading...
/*
		LGL_DrawRectToScreen
		(
			ViewPortLeft,ViewPortLeft+ViewPortWidth*Sound->GetPercentLoadedSmooth(),
			ViewPortBottom+ViewPortHeight*.25,ViewPortBottom+ViewPortHeight*.5,
			0,0,.25+.25*glow,.5
		);
*/
		
		char temp[1024];
		sprintf
		(
			temp,
			"Decoding '%s'...",
			SoundName
		);
		LGL_GetFont().DrawString
		(
			CenterX,ViewPortBottom+ViewPortHeight*.66,.015,
			1,1,1,1,
			true,.5,
			temp
		);
		sprintf
		(
			temp,
			"%.2f",
			Sound->GetPercentLoadedSmooth()*100
		);
		LGL_GetFont().DrawString
		(
			CenterX,ViewPortBottom+ViewPortHeight*.36,.015,
			1,1,1,1,
			true,.5,
			temp
		);

		float minutes=0;
		float seconds=ceil(Sound->GetSecondsUntilLoaded());
		while(seconds>=60)
		{
			minutes++;
			seconds-=60;
		}
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
			CenterX,ViewPortBottom+ViewPortHeight*.10f,.015f,
			1,1,1,1,
			true,.5f,
			temp
		);
	}
	if(Mode==2)
	{
		//Smooth waveform scrolling

		double proposedDelta = (LGL_AudioAvailable()?1:0)*Sound->GetSpeed(Channel)*44100*(LGL_SecondsSinceLastFrame())*1.0f;
		double currentSample=Sound->GetPositionSamples(Channel);
		double diff=fabsf(currentSample-(SmoothWaveformScrollingSample+proposedDelta));
		double diffMax=1024*2;
		if
		(
			PauseMultiplier==0 ||
			(
				diff>=1024 &&
				(
					FinalSpeed!=PauseMultiplier*(Pitchbend+NudgeFromMixer)+Nudge ||
					FinalSpeed!=Sound->GetSpeed(Channel)
				)
			)
		)
		{
			//A chance to seek to the exact sample!
			diff=99999;
			badFlash=1.0f;
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
LGL_ClipRectEnable(ViewPortLeft,ViewPortRight,ViewPortBottom,ViewPortTop);
#endif

		if(diff<diffMax/2)
		{
			//We're accurate enough
			SmoothWaveformScrollingSample+=proposedDelta;
			currentSample=SmoothWaveformScrollingSample;
		}
		else if(diff<diffMax)
		{
			//Let's change our scrolling speed to get more accurate
			float deltaFactor=(diff/2048.0f)*0.5f;
			SmoothWaveformScrollingSample+=proposedDelta *
				((currentSample>SmoothWaveformScrollingSample+proposedDelta) ? (1.0f + deltaFactor) : (1.0f-deltaFactor) );
			currentSample=SmoothWaveformScrollingSample;
		}
		else
		{
			//Fuck, we're really far off. Screw smooth scrolling, just jump to currentSample
			SmoothWaveformScrollingSample=currentSample;
		}

		unsigned int savePointBitfield=0;
		for(int a=0;a<18;a++)
		{
			savePointBitfield|=(SavePointSeconds[a]==-1.0f)?0:(1<<a);
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
			"dtt|%i|%c|%c|%.0f|%.0f|%.0f|%.0f|%.5f|%.5f|%.3f|%.0f|%.3f|%.3f|%.4f|%.4f|%.4f|%.4f|%.3f|%.4f|%c|%.3f|%.3f|%.3f|%i|%i|%i|%.4f|%.3f|%.3f|%.3f|%.3f|%.3f\n",
			Which,
			Sound->IsLoaded() ? 'T' : 'F',
			(GlitchDuo || LuminScratch || GlitchPure) ? 'T' : 'F',
			GlitchBegin,
			GlitchLength,
			currentSample,//(double)Sound->GetPositionSamples(Channel),
			(double)Sound->GetLengthSamples(),
			Sound->GetSpeed(Channel),
			Pitchbend,
			GrainStreamCrossfader,
			GrainStreamSourcePoint,
			GrainStreamLength,
			GrainStreamPitch,
			ViewPortLeft,
			ViewPortRight,
			ViewPortBottom,
			ViewPortTop,
			VolumeMultiplierNow,
			CenterX,
			PauseMultiplier ? 'T' : 'F',
			Nudge,
			0.0f,//LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS),
			SavePointSeconds[SavePointIndex],
			SavePointIndex,
			SavePointIndex,
			savePointBitfield,
			GetBPM(),
			GetBPMAdjusted(),
			GetBPMFirstBeatSeconds(),
			EQFinal[0],
			EQFinal[1],
			EQFinal[2]
		);
		
		bool waveArrayFilledBefore=(EntireWaveArrayFillIndex==ENTIRE_WAVE_ARRAY_COUNT);

		LGL_DrawLogPause();
		Turntable_DrawWaveform
		(
			Sound,
			Sound->IsLoaded(),
			GetVideo() ? GetVideo()->GetPathShort() : NULL,
			GlitchDuo || LuminScratch || GlitchPure,
			GlitchBegin,
			GlitchLength,
			currentSample,//Sound->GetPositionSamples(Channel),
			Sound->GetLengthSamples(),
			Sound->GetSpeed(Channel),
			Pitchbend,
			GrainStreamCrossfader,
			GrainStreamSourcePoint,
			GrainStreamLength,
			GrainStreamPitch,
			ViewPortLeft,
			ViewPortRight,
			ViewPortBottom,
			ViewPortTop,
			VolumeMultiplierNow,
			CenterX,
			PauseMultiplier,
			Nudge,
			0.0f,//LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS),
			LGL_SecondsSinceExecution(),
			SavePointSeconds,
			SavePointIndex,
			SavePointIndex,
			savePointBitfield,
			SavePointUnsetNoisePercent,
			SavePointUnsetFlashPercent,
			GetBPM(),
			GetBPMAdjusted(),
			GetBPMFirstBeatSeconds(),
			EQFinal[0],
			EQFinal[1],
			EQFinal[2],
			LowRez,
			EntireWaveArrayFillIndex,
			ENTIRE_WAVE_ARRAY_COUNT,
			EntireWaveArrayMagnitudeAve,
			EntireWaveArrayMagnitudeMax,
			EntireWaveArrayFreqFactor,
			CachedLengthSeconds,
			NoiseImage[rand()%NOISE_IMAGE_COUNT_256_64]
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

	if(NoiseFactor>0.0f)
	{
		int which = LGL_RandInt(0,NOISE_IMAGE_COUNT_256_64-1);
		float factor = NoiseFactor;
		NoiseImage[which]->DrawToScreen
		(
			ViewPortLeft,
			ViewPortRight,
			ViewPortBottom,
			ViewPortTop,
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
			ViewPortLeft,
			ViewPortRight,
			ViewPortBottom,
			ViewPortTop,
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
	float	glow
)
{
	//
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
SetViewPort
(
	float	left,	float	right,
	float	bottom,	float	top
)
{
	ViewPortLeft=left;
	ViewPortRight=right;
	ViewPortBottom=bottom;
	ViewPortTop=top;
	ViewPortWidth=right-left;
	ViewPortHeight=top-bottom;

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
			return(1.01f);
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
	NudgeFromMixer=mixerNudge;
}

std::vector<char*>
TurntableObj::
GetTrackListFileUpdates()
{
	return(TrackListFileUpdates);
}

LGL_Video*
TurntableObj::
GetVideo()
{
	if(Sound==NULL)
	{
		return(NULL);
	}

	if(VideoFront!=NULL)
	{
		return(VideoFront);
	}
	else
	{
		return(VideoBack);
	}
}

LGL_Video*
TurntableObj::
GetVideoFront()
{
	return(VideoFront);
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
			return(GetTimeSeconds());
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
	return((RecordScratch || LuminScratch) && (volFront>0.0f));
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
			return(Sound->GetPositionSeconds(Channel));
		}
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

	char metaDataPath[1024];
	sprintf(metaDataPath,"data/metadata/%s.musefuse-metadata.txt",Sound->GetPathShort());
	FILE* fd=fopen(metaDataPath,"r");
	if(fd)
	{
		FileInterfaceObj fi;
		for(;;)
		{
			fi.ReadLine(fd);
			if(feof(fd))
			{
				break;
			}
			if(fi.Size()==0)
			{
				continue;
			}
			if
			(
				strcasecmp(fi[0],"HomePoints")==0 ||
				strcasecmp(fi[0],"SavePoints")==0
			)
			{
				if(fi.Size()!=19)
				{
					printf("TurntableObj::LoadMetaData('%s'): Warning!\n",Sound->GetPathShort());
					printf("\tSavePoints has strange fi.size() of '%i' (Expecting 11)\n",fi.Size());
				}
				for(unsigned int a=0;a<fi.Size()-1 && a<18;a++)
				{
					SavePointSeconds[a]=atof(fi[a+1]);
				}
			}
		}

		fclose(fd);
	}
	/*
	else
	{
		printf("TurntableObj::LoadMetaData('%s'): Warning!\n",Sound->GetPathShort());
		printf("\tCouldn't load metadata: '%s'\n",metaDataPath);
	}
	*/
}

void
TurntableObj::
SaveMetaData()
{
	if(Sound==NULL)
	{
		return;
	}

	char metaDataPath[1024];
	sprintf(metaDataPath,"data/metadata/%s.musefuse-metadata.txt",Sound->GetPathShort());
	FILE* fd=fopen(metaDataPath,"w");
	if(fd)
	{
		fprintf
		(
			fd,
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
		fclose(fd);

		MetaDataSavedThisFrame=true;
	}
	else
	{
		printf("TurntableObj::SaveMetaData('%s'): Warning!\n",Sound->GetPathShort());
		printf("\tCould not open '%s' for writing\n",metaDataPath);
	}
}

bool
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
	CachedLengthSeconds=0.0f;

	//Check to see if our file has changed
	bool cachedDataValid = LoadCachedFileLength();

	if(cachedDataValid)
	{
		LoadWaveArrayData();
		LoadCachedLength();
	}
	else
	{
		//TODO: Delete stale cache files
		if(Sound!=NULL)
		{
			char waveArrayDataPath[1024];
			char cachedLengthPath[1024];
			char cachedFileLengthPath[1024];

			sprintf(waveArrayDataPath,"data/cache/waveArrayData/%s.dvj-wavearraydata-%i.bin",Sound->GetPathShort(),ENTIRE_WAVE_ARRAY_COUNT);
			sprintf(cachedLengthPath,"data/cache/length/%s.dvj-length.txt",Sound->GetPathShort());
			sprintf(cachedFileLengthPath,"data/cache/filelength/%s.dvj-filelength.txt",Sound->GetPathShort());

			if(LGL_FileExists(waveArrayDataPath))
			{
				LGL_FileDelete(waveArrayDataPath);
			}
			if(LGL_FileExists(cachedLengthPath))
			{
				LGL_FileDelete(cachedLengthPath);
			}
			if(LGL_FileExists(cachedFileLengthPath))
			{
				LGL_FileDelete(cachedFileLengthPath);
			}
		}
	}
}

void
TurntableObj::
SaveAllCachedData()
{
	SaveWaveArrayData();
	SaveCachedLength();
	SaveCachedFileLength();
}

void
TurntableObj::
LoadWaveArrayData()
{
	if(Sound==NULL)
	{
		return;
	}

	char waveArrayDataPath[1024];
	sprintf(waveArrayDataPath,"data/cache/waveArrayData/%s.dvj-wavearraydata-%i.bin",Sound->GetPathShort(),ENTIRE_WAVE_ARRAY_COUNT);

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

	FILE* fd=fopen(waveArrayDataPath,"rb");
	if(fd)
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
	if(Sound==NULL)
	{
		return;
	}
	
	if(LGL_DirectoryExists("data/cache")==false)
	{
		LGL_DirectoryCreate("data/cache");
	}

	if(LGL_DirectoryExists("data/cache/waveArrayData")==false)
	{
		LGL_DirectoryCreate("data/cache/waveArrayData");
	}

	char waveArrayDataPath[1024];
	sprintf(waveArrayDataPath,"data/cache/waveArrayData/%s.dvj-wavearraydata-%i.bin",Sound->GetPathShort(),ENTIRE_WAVE_ARRAY_COUNT);
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
LoadCachedLength()
{
	if(Sound==NULL)
	{
		return;
	}

	char cachedLengthPath[1024];
	sprintf(cachedLengthPath,"data/cache/length/%s.dvj-length.txt",Sound->GetPathShort());

	FILE* fd=fopen(cachedLengthPath,"r");
	if(fd)
	{
		char buf[1024];
		fgets(buf,1024,fd);
		CachedLengthSeconds=atof(buf);
		fclose(fd);
	}
}

void
TurntableObj::
SaveCachedLength()
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

	if(LGL_DirectoryExists("data/cache")==false)
	{
		LGL_DirectoryCreate("data/cache");
	}

	if(LGL_DirectoryExists("data/cache/length")==false)
	{
		LGL_DirectoryCreate("data/cache/length");
	}

	char cachedLengthPath[1024];
	sprintf(cachedLengthPath,"data/cache/length/%s.dvj-length.txt",Sound->GetPathShort());
	FILE* fd=fopen(cachedLengthPath,"w");
	if(fd)
	{
		fprintf(fd,"%.3f\n",CachedLengthSeconds);
		fclose(fd);
	}
}

bool
TurntableObj::
LoadCachedFileLength()
{
	if(Sound==NULL)
	{
		return(false);
	}

	char cachedFileLengthPath[1024];
	sprintf(cachedFileLengthPath,"data/cache/filelength/%s.dvj-filelength.txt",Sound->GetPathShort());

	FILE* fd=fopen(cachedFileLengthPath,"r");
	if(fd)
	{
		char buf[1024];
		fgets(buf,1024,fd);
		fclose(fd);

		long cachedFileLength=atol(buf);
		long actualFileLength=LGL_FileLengthBytes(Sound->GetPath());
		return(cachedFileLength==actualFileLength);
	}
	else
	{
		return(false);
	}
}

void
TurntableObj::
SaveCachedFileLength()
{
	if(Sound==NULL)
	{
		return;
	}

	long actualFileLength=LGL_FileLengthBytes(Sound->GetPath());

	if(LGL_DirectoryExists("data/cache")==false)
	{
		LGL_DirectoryCreate("data/cache");
	}

	if(LGL_DirectoryExists("data/cache/filelength")==false)
	{
		LGL_DirectoryCreate("data/cache/filelength");
	}

	char cachedFileLengthPath[1024];
	sprintf(cachedFileLengthPath,"data/cache/filelength/%s.dvj-filelength.txt",Sound->GetPathShort());
	FILE* fd=fopen(cachedFileLengthPath,"w");
	if(fd)
	{
		fprintf(fd,"%li\n",actualFileLength);
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
		return(Sound->GetPath());
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
		return(Sound->GetPathShort());
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

const
char*
TurntableObj::
GetCurrentFileString()
{
	int fileNum=DirTree.GetFilteredDirCount()+DirTree.GetFilteredFileCount();
	if(fileNum==0) return(NULL);

	for(int b=0;b<5 && b+FileTop<fileNum && b+FileTop<10000;b++)
	{
		const char* fileNow;

		unsigned int num=b+FileTop;
		fileNow=(num<DirTree.GetFilteredDirCount()) ?
			DirTree.GetFilteredDirName(num) :
			DirTree.GetFilteredFileName(num - DirTree.GetFilteredDirCount());

		if
		(
			strlen(fileNow)>0 &&
			b+FileTop==FileSelectInt
		)
		{
			return(fileNow);
		}
	}

	return(NULL);
}

bool
TurntableObj::
LoopActive()
{
	return
	(
		LoopStartSeconds>=0.0 &&
		(
			LoopLengthMeasures>0 ||
			LoopAtEndOfMeasure
		)
	);
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
	if(Channel>=0) Sound->SetDivergeRecallOff(Channel);
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

	if(PauseMultiplier==0)
	{
		ClearRecallOrigin();
		return;
	}

	//Change our speed instantly.
	Sound->SetSpeed(Channel,Pitchbend*PauseMultiplier,true);
	Sound->SetDivergeRecallEnd(Channel);
	
	ClearRecallOrigin();
}

void
TurntableObj::
UpdateFileBPM()
{
	if(DirTree.Ready()==false)
	{
		return;
	}

	int fileNum=DirTree.GetFilteredDirCount()+DirTree.GetFilteredFileCount();

	for
	(
		int b=0;
		(
			b<5 &&
			b+FileTop<fileNum &&
			b+FileTop<10000
		);
		b++
	)
	{
		unsigned int num=b+FileTop;
		if(num<DirTree.GetFilteredDirCount())
		{
			FileBPM[b]=0;
			continue;
		}
		const char* fileNow = DirTree.GetFilteredFileName(num - DirTree.GetFilteredDirCount());

		char metaDataPath[1024];
		sprintf(metaDataPath,"data/metadata/%s.musefuse-metadata.txt",fileNow);
		FILE* fd=fopen(metaDataPath,"r");
		float bpm=0;
		float savePointSeconds[20];
		for(int a=0;a<20;a++)
		{
			savePointSeconds[a]=-999;
		}
		if(fd)
		{
			FileInterfaceObj fi;
			for(;;)
			{
				fi.ReadLine(fd);
				if(feof(fd))
				{
					break;
				}
				if(fi.Size()==0)
				{
					continue;
				}
				if
				(
					strcasecmp(fi[0],"HomePoints")==0 ||
					strcasecmp(fi[0],"SavePoints")==0
				)
				{
					for(unsigned int c=0;c<fi.Size()-1 && c<18;c++)
					{
						savePointSeconds[c]=atof(fi[c+1]);
					}
				}
			}

			fclose(fd);
			fd=NULL;
		}

		if
		(
			savePointSeconds[0]!=-999 &&
			savePointSeconds[1]!=-999
		)
		{
			int bpmMin=100;
			float p0=savePointSeconds[0];
			float p1=savePointSeconds[1];
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
						bpm=bpmGuess;
						break;
					}
					measuresGuess*=2;
				}
			}
		}

		FileBPM[b] = bpm;
	}
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

	if(Sound)
	{
		Sound->SetFreqResponse(Channel,FreqResponse);
	}
}

void
TurntableObj::
SwapVideos()
{
	LGL_Video* temp=VideoFront;
	VideoFront=VideoBack;
	VideoBack=temp;
}

void
TurntableObj::
SelectNewVideo
(
	bool	forceAmbient,
	bool	forceMellow
)
{
	char videoFileName[1024];
	sprintf(videoFileName,"data/video/tracks/%s.mjpeg.avi",SoundName);
	if
	(
		forceAmbient ||
		VideoFileExists==false
	)
	{
		//Get next ambient video from Visualizer.
		char path[2048];
		if(forceMellow)
		{
			Visualizer->GetNextVideoPathAmbientMellow(path);
		}
		else
		{
			Visualizer->GetNextVideoPathAmbient(path);
		}

		if(path[0]!='\0')
		{
			if(VideoBack==NULL)
			{
				VideoBack=new LGL_Video(path);
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
		VideoIsMellow=forceMellow;

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

		if(VideoBack==NULL)
		{
			VideoBack=new LGL_Video(videoFileName);
		}
		else
		{
			VideoBack->SetVideo(videoFileName);
		}
		VideoIsMellow=false;
		VideoOffsetSeconds=0;
	}
	assert(VideoBack);
	SwapVideos();
	assert(VideoFront);
	LGL_DrawLogWrite("!dvj::NewVideo|%s\n",VideoFront->GetPath());
}

bool
TurntableObj::
BPMAvailable()
{
	return
	(
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

	double startBeat = GetBPMFirstBeatSeconds();
	double deltaBeat = fractionOfBeat * (60.0/(double)GetBPM());
	double windowStart = SecondsLast;
	double windowEnd = SecondsNow;
	double candidate = startBeat;

	if
	(
		LoopActive() &&
		fabsf(SecondsNow-LoopStartSeconds) < 0.1 &&
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
	if(BPMAvailable()==false)
	{
		return(-1.0);
	}

	double startMeasure = GetBeginningOfCurrentMeasureSeconds(measureMultiplier);
	double deltaMeasure = measureMultiplier*4*(60.0/(double)GetBPM());

	if
	(
		startMeasure<0 ||
		Channel<0 ||
		GetBPMFirstBeatSeconds() > Sound->GetPositionSeconds(Channel)
	)
	{
		return(-1.0);
	}

	return((SecondsNow-startMeasure)/deltaMeasure);
}

double
TurntableObj::
GetBeginningOfCurrentMeasureSeconds
(
	float	measureMultiplier
)
{
	if
	(
		BPMAvailable()==false ||
		Channel<0 ||
		GetBPMFirstBeatSeconds() > Sound->GetPositionSeconds(Channel)
	)
	{
		return(-1.0);
	}

	double firstBeat = GetBPMFirstBeatSeconds();
	double deltaMeasure = measureMultiplier*4*(60.0/(double)GetBPM());
	double candidate = firstBeat;
	while(candidate+deltaMeasure<SecondsNow)
	{
		candidate+=deltaMeasure;
	}
	return(candidate);
}

bool
TurntableObj::
GetSolo()
{
	return(VolumeSolo);
}

