/*
 *
 * Sequencer.cpp
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

#include "Sequencer.h"

#include "LGL.module/LGL.h"

#define	MAX_VOLUME	(0.35f)

SequencerObj::
SequencerObj()
{
	for(int a=0;a<SOUND_NUM;a++)
	{
		ClipFrontPath[a][0]='\0';
		ClipBackPath[a][0]='\0';
		ClipChannel[a]=-1;
		ClipFront[a]=NULL;
		ClipBack[a]=NULL;

		if(a<31)
		{
			int note=a;
			int octave=0;
			while(note>11)
			{
				octave++;
				note-=12;
			}
			char path[1024];
			sprintf(path,"data/sequencer/test/%i_%i.wav",octave,note);
			strcpy(ClipBackPath[a],path);
		}

		ClipMem[a]=(char*)malloc(1024*1024*4);
	}

	BPMInfoBPM = 0.0f;
	BPMInfoSecondsAfterFirstBeat = -1.0f;

	VolumeFront = 1.0f;
	VolumeBack = 1.0f;
	for(int a=0;a<3;a++) EQ[a]=1.0f;
	for(int a=0;a<513;a++) FreqResponse[a]=1.0f;
}

SequencerObj::
~SequencerObj()
{
	//
}

void
SequencerObj::
NextFrame()
{
return;
	NextFrameBack();
	MaybeSwap();
	ProcessInput();
}

void
SequencerObj::
Draw
(
	bool	visualizerQuadrent,
	float	visualizeZoomOutPercent
)
{
	//Nothing to draw yet
}

void
SequencerObj::
SetBPMInfo
(
	float	bpm,
	float	secondsAfterFirstBeat,
	float	speed
)
{
	if(bpm>0.0f && secondsAfterFirstBeat>=0.0f)
	{
		BPMInfoBPM = bpm;
		BPMInfoSecondsAfterFirstBeat = secondsAfterFirstBeat;
		BPMInfoSpeed = speed;
		while(BPMInfoBPM<100.0f)
		{
			BPMInfoBPM*=2.0f;
			BPMInfoSpeed*=0.5;
		}
		while(BPMInfoBPM>200.0f)
		{
			BPMInfoBPM*=0.5f;
			BPMInfoSpeed*=2.0;
		}
	}
	else
	{
		BPMInfoBPM = 0.0f;
		BPMInfoSecondsAfterFirstBeat = -1.0f;
		BPMInfoSpeed = 0.0f;
	}
}

void
SequencerObj::
SetVolumeFrontBack
(
	float	volumeFront,
	float	volumeBack
)
{
	if
	(
		VolumeFront!=volumeFront ||
		VolumeBack!=volumeBack
	)
	{
		VolumeFront=volumeFront;
		VolumeBack=volumeBack;
		for(int a=0;a<SOUND_NUM;a++)
		{
			if(ClipChannel[a]>=0)
			{
				ClipFront[a]->SetVolumeSurround
				(
					ClipChannel[a],
					MAX_VOLUME*VolumeFront,
					MAX_VOLUME*VolumeFront,
					MAX_VOLUME*VolumeBack,
					MAX_VOLUME*VolumeBack
				);
			}
		}
	}
}

void
SequencerObj::
SetEQ
(
	float	low,
	float	mid,
	float	high
)
{
	if
	(
		EQ[0]!=low ||
		EQ[1]!=mid ||
		EQ[2]!=high
	)
	{
		EQ[0]=low;
		EQ[1]=mid;
		EQ[2]=high;

		for(int a=0;a<8;a++)
		{
			FreqResponse[a]=EQ[0];
		}
		for(int a=8;a<40;a++)
		{
			FreqResponse[a]=EQ[1];
		}
		for(int a=40;a<513;a++)
		{
			FreqResponse[a]=EQ[2];
		}

		for(int a=0;a<SOUND_NUM;a++)
		{
			if(ClipChannel[a]>=0)
			{
				ClipFront[a]->SetFreqResponse
				(
					ClipChannel[a],
					FreqResponse
				);
			}
		}
	}
}

void
SequencerObj::
NextFrameBack()
{
	//Are we loading a sound?
	bool loading=false;
	for(int a=0;a<SOUND_NUM;a++)
	{
		if
		(
			ClipBack[a] &&
			ClipBack[a]->IsLoaded()==false
		)
		{
			loading=true;
			break;
		}
	}

	if(loading==false)
	{
		//Try to load a clip into first empty ClipBack[]
		for(int a=0;a<SOUND_NUM;a++)
		{
			if
			(
				ClipBack[a]==NULL &&
				strlen(ClipBackPath[a])>0
			)
			{
				ClipBack[a] = new LGL_Sound
				(
					ClipBackPath[a],
					true,
					2,
					(Uint8*)ClipMem[a],
					1024*1024*4
				);
				break;
			}
		}
	}

	//Are there any sounds that can't load?
	for(int a=0;a<SOUND_NUM;a++)
	{
		if
		(
			ClipBack[a] &&
			ClipBack[a]->IsUnloadable()
		)
		{
			delete ClipBack[a];
			ClipBack[a]=NULL;
		}
	}
}

void
SequencerObj::
MaybeSwap()
{
	for(int a=0;a<SOUND_NUM;a++)
	{
		if
		(
			ClipFront[a]==NULL &&
			ClipBack[a]!=NULL &&
			ClipBack[a]->IsLoaded()
		)
		{
			ClipFront[a]=ClipBack[a];
			ClipBack[a]=NULL;
			strcpy(ClipFrontPath[a],ClipBackPath[a]);
			ClipBackPath[a][0]='\0';
		}
	}
}

void
SequencerObj::
ProcessInput()
{
	LGL_MidiDevice* midi = LGL_GetJP8k();
	if(midi==NULL) return;

	float bpmNow=BPMInfoBPM;
	float timeNow=BPMInfoSecondsAfterFirstBeat+1.0f/60.0f;
	
	for(int key=LGL_JP8K_BUTTON_KEY_0_0;key<=LGL_JP8K_BUTTON_KEY_4_0;key++)
	{
		//Play new sound
		int index=key-LGL_JP8K_BUTTON_KEY_0_0;
		if(midi->GetButtonStroke(key))
		{
			if
			(
				ClipFront[index] &&
				ClipFront[index]->GetLengthSeconds()==0
			)
			{
				printf("Error! Clip with filename '%s' is zero length!\n",ClipFront[index]->GetPath());
			}
			else if
			(
				ClipFront[index] &&
				ClipFront[index]->GetLengthSeconds()>0
			)
			{
				//Stop self
float posBefore = 0.0f;
				if(ClipChannel[index]>=0)
				{
posBefore=ClipFront[index]->GetPositionSeconds(ClipChannel[index]);
					ClipFront[index]->Stop(ClipChannel[index]);
				}
				
				//Play our sound
				float myLen = ClipFront[index]->GetLengthSeconds();
				float myBPM = 60.0f*(4.0f/myLen);
				while(myBPM>200)
				{
					myBPM*=0.5f;
				}
				while(myBPM<100)
				{
					myBPM*=2.0f;
				}
//printf("myBPM: %.2f (%.2f)\n",bpmNow,myBPM);
				float mySpeed=bpmNow/myBPM;
				float myPos=timeNow*mySpeed/BPMInfoSpeed;
				while(myPos>myLen)
				{
					myPos-=myLen;
				}
//printf("MyBPM, mySpeed, myFinalBPM: %.2f, %.2f, %.2f\n",myBPM,mySpeed,myBPM*mySpeed);
//printf("Pos %.3f: %.3f => %.3f (%.3f)\n",timeNow,posBefore,myPos,BPMInfoSpeed);
//printf("Play! %.2f, %.2f, %.2f\n",VolumeFront,VolumeBack,mySpeed);
				ClipChannel[index]=ClipFront[index]->Play
				(
					0.0f,
					true,
					mySpeed,	//Speed
					0
				);
				ClipFront[index]->SetVolumeSurround
				(
					ClipChannel[index],
					MAX_VOLUME*VolumeFront,
					MAX_VOLUME*VolumeFront,
					MAX_VOLUME*VolumeBack,
					MAX_VOLUME*VolumeBack
				);
				ClipFront[index]->SetFreqResponse
				(
					ClipChannel[index],
					FreqResponse
				);
				ClipFront[index]->SetPositionSeconds(ClipChannel[index],myPos);
			}
			
			//Stop all non-protected sounds
			for(int a=0;a<SOUND_NUM;a++)
			{
				int associatedKey=a+LGL_JP8K_BUTTON_KEY_0_0;
				if
				(
					ClipChannel[a]>=0 &&
					midi->GetButtonDown(associatedKey)==false
				)
				{
					ClipFront[a]->Stop(ClipChannel[a]);
					ClipChannel[a]=-1;
				}
			}
		}	
	}
}


