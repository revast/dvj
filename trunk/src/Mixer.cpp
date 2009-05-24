/*
 *
 * Mixer.cpp
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

#include "Mixer.h"

#include <sys/types.h>
#include <sys/stat.h>

#define	CPU_TEMP_DANGER (70)

MixerObj::
MixerObj() : Database("data/music")
{
	for(int a=0;a<2;a++)
	{
		Turntable[a]=new TurntableObj(0.025,0.975,0.25,0.50,&Database);
		Turntable[a]->SetTurntableNumber(a);
	}

	Focus=0;
	Turntable[Focus]->SetFocus(true);
	Turntable[(Focus+1)%2]->SetFocus(false);

	CPUSpeedHighest=0;

	SetViewPort(0,1,0,0.5f);

	for(int a=0;a<3;a++)
	{
		EQXponent[a]=1.0f;
		EQJP8k[a]=1.0f;
	}

	CrossFadeMiddle=1.0f;
	CrossFadeSliderLeft=0.5f;
	CrossFadeSliderRight=0.5f;

	Visualizer=NULL;

	Recording=false;
	RecordingSecondsSinceExecution=0.0f;
	RecordingTrackListFile=NULL;
	RecordingFailedTimer=0.0f;

	VideoAdvancedLastFrame=0;

	LowRez=false;
	CanDisplayJackWarning=true;

	SetViewPortStatus(0.5f,1.0f,0.5f,1.0f);
}

MixerObj::
~MixerObj()
{
	for(int a=0;a<2;a++)
	{
		delete Turntable[a];
	}

	if(RecordingTrackListFile!=NULL)
	{
		fclose(RecordingTrackListFile);
		RecordingTrackListFile=NULL;
	}
}

void
MixerObj::
NextFrame
(
	float	secondsElapsed
)
{
	float candidate;

	//Process Input

	assert(Focus>=0);

	if(CanDisplayJackWarning)
	{
		for(int a=0;a<2;a++)
		{
			if(Turntable[a]->GetMode()!=0)
			{
				CanDisplayJackWarning=false;
			}
		}

		if(LGL_KeyStroke(SDLK_F4))
		{
			system("firefox http://code.google.com/p/dvj/wiki/JACK &");
			exit(0);
		}
	}

	if(Input.FocusChange())
	{
		Turntable[Focus]->SetFocus(false);
		Focus=((Focus+1)%2);
		Turntable[Focus]->SetFocus(true);
	}

	if(Input.FocusTop())
	{
		Turntable[0]->SetFocus(true);
		Turntable[1]->SetFocus(false);
		Focus=0;
	}
	else if(Input.FocusBottom())
	{
		Turntable[0]->SetFocus(false);
		Turntable[1]->SetFocus(true);
		Focus=1;
	}

	for(int a=0;a<2;a++)
	{
		KillSwitch[a]=false;
		FullSwitch[a]=false;
	}

	int masterToHeadphones=Input.MasterToHeadphones();
	if(masterToHeadphones==0)
	{
		LGL_AudioMasterToHeadphones(false);
	}
	else if(masterToHeadphones==1)
	{
		LGL_AudioMasterToHeadphones(true);
	}

	candidate=Input.XfaderSpeakers();
	if(candidate!=-1.0f)
	{
		CrossFadeSliderLeft=candidate;
		if
		(
			CrossFadeSliderLeft>=63.0f/127.0f &&
			CrossFadeSliderLeft<=65.0f/127.0f
		)
		{
			CrossFadeSliderLeft=0.5f;
		}
		if(LGL_AudioChannels()==2)
		{
			CrossFadeSliderRight=candidate;
			if
			(
				CrossFadeSliderRight>=63.0f/127.0f &&
				CrossFadeSliderRight<=65.0f/127.0f
			)
			{
				CrossFadeSliderRight=0.5f;
			}
		}
	}

	candidate=Input.XfaderHeadphones();
	if(candidate!=-1.0f)
	{
		CrossFadeSliderRight=candidate;
		if
		(
			CrossFadeSliderRight>=63.0f/127.0f &&
			CrossFadeSliderRight<=65.0f/127.0f
		)
		{
			CrossFadeSliderRight=0.5f;
		}
		if(LGL_AudioChannels()==2)
		{
			CrossFadeSliderLeft=candidate;
			if
			(
				CrossFadeSliderLeft>=63.0f/127.0f &&
				CrossFadeSliderLeft<=65.0f/127.0f
			)
			{
				CrossFadeSliderLeft=0.5f;
			}
		}
	}

	int syncTT = -1;
	int target = -1;

	if
	(
		Turntable[0]->GetBPM()>0 &&
		Turntable[1]->GetBPM()>0
	)
	{
		if(Input.SyncTopToBottom())
		{
			syncTT=0;
			target=1;
		}
		if(Input.SyncBottomToTop())
		{
			syncTT=1;
			target=0;
		}
		if(Turntable[Focus]->GetMode()==2)
		{
			if(Input.WaveformSyncBPM(TARGET_FOCUS))
			{
				syncTT=Focus;
				target=Focus?0:1;
			}
		}
	}

	if(syncTT!=-1)
	{
		//Sync BPM
		Turntable[syncTT]->SetBPMAdjusted(Turntable[target]->GetBPMAdjusted());

		//Nudge toward sync
		float bpmScalar = Turntable[target]->GetBPMAdjusted()/Turntable[syncTT]->GetBPMAdjusted();
		float percentOfMeasureSelf = Turntable[syncTT]->GetPercentOfCurrentMeasure(1.0f/bpmScalar);
		float percentOfMeasureTarget = Turntable[target]->GetPercentOfCurrentMeasure();
		if
		(
			percentOfMeasureSelf >= 0 &&
			percentOfMeasureTarget >= 0
		)
		{
			float candidate1 = percentOfMeasureTarget - percentOfMeasureSelf;
			float candidate2 = percentOfMeasureTarget - percentOfMeasureSelf + 1.0f;
			float candidate3 = percentOfMeasureTarget - percentOfMeasureSelf - 1.0f;
			float shortestDirection = (fabsf(candidate1) < fabsf(candidate2)) ? candidate1 : candidate2;
			if(fabsf(candidate3) < fabsf(shortestDirection)) shortestDirection=candidate3;
			if(fabsf(shortestDirection)>0.005f)
			{
				float nudgeAmount = LGL_Sign(shortestDirection)*0.32f;
				if(fabsf(2*shortestDirection)<1.0f/8.0f)
				{
					float factor = fabsf(2*shortestDirection)*8.0f;
					nudgeAmount = LGL_Sign(shortestDirection)*
					(
						powf(factor,2.0f-factor)*0.32f
					);
				}
				Turntable[syncTT]->SetMixerNudge(nudgeAmount);
			}
			else
			{
				Turntable[syncTT]->SetMixerNudge(0.0f);
			}
		}
	}
	else
	{
		Turntable[0]->SetMixerNudge(0.0f);
		Turntable[1]->SetMixerNudge(0.0f);
	}
	
	//Crossfade
	if(LGL_AudioChannels()==2)
	{
		//Since we only have two channels, bind Speakers/Headphones
		float delta=0;
		delta+=Input.XfaderSpeakersDelta();
		delta+=Input.XfaderHeadphonesDelta();
		CrossFadeSliderLeft=LGL_Clamp(0.0f,CrossFadeSliderLeft+delta,1.0f);
		CrossFadeSliderRight=LGL_Clamp(0.0f,CrossFadeSliderRight+delta,1.0f);
	}
	else if(LGL_AudioChannels()==4)
	{
		CrossFadeSliderLeft=LGL_Clamp(0.0f,CrossFadeSliderLeft+Input.XfaderSpeakersDelta(),1.0f);
		CrossFadeSliderRight=LGL_Clamp(0.0f,CrossFadeSliderRight+Input.XfaderHeadphonesDelta(),1.0f);
	}

	int masterTT=-1;
	if
	(
		Turntable[0]->GetMode()==2 &&
		Turntable[1]->GetMode()==2
	)
	{
		if(CrossFadeSliderLeft>0.5f)
		{
			masterTT=0;
		}
		else
		{
			masterTT=1;
		}
	}
	else if(Turntable[0]->GetMode()==2)
	{
		masterTT=0;
	}
	else if(Turntable[1]->GetMode()==2)
	{
		masterTT=1;
	}

	float bpmMaster=-1.0f;
	if(masterTT!=-1)
	{
		float adjusted=Turntable[masterTT]->GetBPMAdjusted();
		if(adjusted>0)
		{
			bpmMaster=adjusted;
		}
	}
	Turntable[0]->SetBPMMaster(bpmMaster);
	Turntable[1]->SetBPMMaster(bpmMaster);

	for(int a=0;a<2;a++)
	{
		Turntable[a]->SetMixerVolumeBack(0);
	}

	bool solo[2];
	solo[0]=Turntable[0]->GetSolo();
	solo[1]=Turntable[1]->GetSolo();
	bool soloActive=(solo[0] || solo[1]);

	//Update Active Turntables
	for(int Which=0;Which<2;Which++)
	{
		float meFront;
		float meBack;

		int soloStatus=0;
		if(soloActive)
		{
			soloStatus = solo[Which] ? 1 : -1;
		}

		if
		(
			KillSwitch[Which]==1 ||
			soloStatus==-1
		)
		{
			//Kill Switch
			meFront=0;
			meBack=0;
		}
		else if
		(
			FullSwitch[Which]==1 ||
			soloStatus==1
		)
		{
			//Full Volume case
			meFront=1;
			meBack=1;
		}
		else
		{
			meFront=(1-Which)*CrossFadeSliderLeft+(Which*(1-CrossFadeSliderLeft));
			meBack=(1-Which)*CrossFadeSliderRight+(Which*(1-CrossFadeSliderRight));
		}

		if(meFront>.5)
		{
			//Crossfader closs to max
			meFront=CrossFadeMiddle+(1.0-CrossFadeMiddle)*2*(meFront-.5);
		}
		else
		{
			//Crossfader far from max
			meFront=CrossFadeMiddle*(2*meFront);
		}
		if(meBack>.5)
		{
			//Crossfader closs to max
			meBack=CrossFadeMiddle+(1.0-CrossFadeMiddle)*2*(meBack-.5);
		}
		else
		{
			//Crossfader far from max
			meBack=CrossFadeMiddle*(2*meBack);
		}

		/*
		//Deal with crossfading and EQ for sequencer vs turntables
		if(LGL_GetXsession())
		{
			LGL_MidiDevice* xsession = LGL_GetXsession();
			float xfader = xsession->GetKnobStatus(LGL_XSESSION_KNOB_XFADER);
			float TTfront = 1.0f;
			float JPfront = 1.0f;

			//Deal with Xfader
			if(xfader!=-1.0f)
			{
				if(xfader<0.5f)
				{
					TTfront = 1.0f + (2.0f*(xfader-0.5f));
				}
				else
				{
					JPfront = 1.0f - (2.0f*(xfader-0.5f));
				}
			}

			//Set Turntable Volumes
			{
				float slider = xsession->GetKnobStatus(LGL_XSESSION_KNOB_LEFT_VOLUME);
				if(slider!=-1.0f)
				{
					TTfront*=slider;
				}
				float back=xsession->GetKnobStatus(LGL_XSESSION_KNOB_LEFT_PITCHBEND);
				if
				(
					back==-1.0f ||
					xsession->GetButtonDown(LGL_XSESSION_BUTTON_LEFT_HEADPHONES)
				)
				{
					back=1.0f;
				}
				meFront*=TTfront;
				meBack*=back;
			}

			//Set JP8k Volumes
			{
				float slider = xsession->GetKnobStatus(LGL_XSESSION_KNOB_RIGHT_VOLUME);
				if(slider!=-1.0f)
				{
					JPfront*=slider;
				}
				float back=xsession->GetKnobStatus(LGL_XSESSION_KNOB_RIGHT_PITCHBEND);
				if
				(
					back==-1.0f ||
					xsession->GetButtonDown(LGL_XSESSION_BUTTON_RIGHT_HEADPHONES)
				)
				{
					back=1.0f;
				}
				Sequencer.SetVolumeFrontBack(JPfront*VolumeMaster,back*VolumeMaster);
			}
			
			//Deal with EQ
			{
				for(int a=0;a<2;a++)
				{
					int low = a==0 ? LGL_XSESSION_KNOB_LEFT_LOW : LGL_XSESSION_KNOB_RIGHT_LOW;
					int mid = a==0 ? LGL_XSESSION_KNOB_LEFT_MID : LGL_XSESSION_KNOB_RIGHT_MID;
					int high = a==0 ? LGL_XSESSION_KNOB_LEFT_HIGH : LGL_XSESSION_KNOB_RIGHT_HIGH;
					float* eq = a==0 ? EQXponent : EQJP8k;

					bool eqChanged=false;
					if(LGL_GetXsession()->GetKnobTweak(low))
					{
						eq[0]=2*LGL_GetXsession()->GetKnobStatus(low);
						eqChanged=true;
					}
					if(LGL_GetXsession()->GetKnobTweak(mid))
					{
						eq[1]=2*LGL_GetXsession()->GetKnobStatus(mid);
						eqChanged=true;
					}
					if(LGL_GetXsession()->GetKnobTweak(high))
					{
						eq[2]=2*LGL_GetXsession()->GetKnobStatus(high);
						eqChanged=true;
					}

					if(eqChanged)
					{
						if(a==0)
						{
							for(int t=0;t<2;t++)
							{
								Turntable[t]->SetMixerEQ(eq[0],eq[1],eq[2]);
							}
						}
						else
						{
							Sequencer.SetEQ(eq[0],eq[1],eq[2]);
						}
					}
				}
			}
		}
		*/

		meFront*=VolumeMaster;
		meBack*=VolumeMaster;

		Turntable[Which]->SetMixerVolumeFront(meFront);
		Turntable[Which]->SetMixerVolumeBack(meBack);

		Turntable[Which]->NextFrame(secondsElapsed);
		if(Turntable[Which]->GetMetaDataSavedThisFrame())
		{
			const char* pathShort=Turntable[Which]->GetSoundPathShort();
			if(pathShort)
			{
				for(int a=0;a<2;a++)
				{
					if
					(
						a!=Which &&
						Turntable[a]->GetSoundPathShort() &&
						strcmp(Turntable[a]->GetSoundPathShort(),pathShort)==0
					)
					{
						Turntable[a]->LoadMetaData();
					}
				}
			}
		}

		if
		(
			RecordingTrackListFile!=NULL &&
			Turntable[Which]->GetTrackListFileUpdates().empty()==false
		)
		{
			int seconds=(int)
				(
					LGL_SecondsSinceExecution()-
					RecordingSecondsSinceExecution
				);

			int minutes=0;
			while(seconds>=60)
			{
				seconds-=60;
				minutes++;
			}
			int hours=0;
			while(minutes>=60)
			{
				minutes-=60;
				hours++;
			}
			char timestamp[1024];
			sprintf
			(
				timestamp,
				"%.2i:%.2i.%.2i",
				hours,minutes,seconds
			);

			std::vector<char*> updates=Turntable[Which]->GetTrackListFileUpdates();
			for(unsigned int a=0;a<updates.size();a++)
			{
				fprintf(RecordingTrackListFile,"%s - Turntable[%i] - %s\n",timestamp,Which,updates[a]);
			}
		}
	}

	//Maybe display the text of the song we're playing
	TurntableObj* tt=NULL;
	if(CrossFadeSliderLeft>0.75f)
	{
		tt=Turntable[0];
	}
	else if(CrossFadeSliderLeft<0.25f)
	{
		tt=Turntable[1];
	}
	if
	(
		tt!=NULL &&
		(
			(
				Visualizer->GetScrollTextEnabled()==false &&
				tt->GetPaused()==false
			) ||
			SongTitleAsScrollTextTimer.SecondsSinceLastReset()>=120
		)
	)
	{
		const char* txt=tt->GetSoundPathShort();
		if(txt!=NULL)
		{
			char* pushme=new char[strlen(txt)+1];
			strcpy(pushme,txt);
			for(int a=strlen(pushme)-1;a>=0;a--)
			{
				if(pushme[a]=='.')
				{
					pushme[a]='\0';
					break;
				}
			}
			Visualizer->QueueScrollText(pushme);
			delete pushme;
			SongTitleAsScrollTextTimer.Reset();
		}
	}

	//Maybe load a track-specific Scroll Text for our song, if it's the only one
	TurntableObj* soloTable=NULL;
	if
	(
		Turntable[0]->GetSoundLoaded() &&
		Turntable[1]->GetSoundLoaded()==false
	)
	{
		soloTable=Turntable[0];
	}
	if
	(
		Turntable[0]->GetSoundLoaded()==false &&
		Turntable[1]->GetSoundLoaded()
	)
	{
		soloTable=Turntable[1];
	}

	if(soloTable)
	{
		//Visualizer->MaybeSetScrollTextTrackFile(soloTable->GetSoundPathShort());
	}

	//Decide which video is highlighted, if any
	int highlighted=-1;
	if(Visualizer)
	{
		for(int a=0;a<2;a++)
		{
			if(FullSwitch[a]==1)
			{
				highlighted = a;
			}
		}
		if
		(
			highlighted==-1 &&
			(
				Turntable[0]->GetMixerVolumeFront() > 0 ||
				Turntable[1]->GetMixerVolumeFront() > 0
			)
		)
		{
			bool pause0 = Turntable[0]->GetPaused() && Turntable[0]->GetRecordScratch()==false;
			bool pause1 = Turntable[1]->GetPaused() && Turntable[1]->GetRecordScratch()==false;
			if
			(
				pause0==false &&
				pause1 
			)
			{
				highlighted=0;
			}
			else if
			(
				pause0 &&
				pause1==false 
			)
			{
				highlighted=1;
			}
			else
			{
				highlighted =
				(
					Turntable[0]->GetMixerVolumeFront() + ((Turntable[0]->GetVideoFront()==NULL) ? -10.0f : 0) > 
					Turntable[1]->GetMixerVolumeFront() + ((Turntable[1]->GetVideoFront()==NULL) ? -10.0f : 0)
				) ? 0 : 1;
			}
		}

		if(highlighted==-1)
		{
			highlighted=(CrossFadeSliderLeft <= 0.5f) ? 0 : 1;
		}

		//Pick a video to advance
		if(highlighted!=-1)
		{
			if(Turntable[highlighted]->GetVideoFront())
			{
				Turntable[highlighted]->GetVideoFront()->SetPrimaryDecoder();
			}
		}

		for(int i=0;i<2;i++)
		{
			if(Turntable[i]->GetVideo())
			{
				const float OFFSET = 2.0f/60.0f;	//Combat lag associated with projector scaling
				Turntable[i]->GetVideo()->SetTime
				(
					Turntable[i]->GetVideoTimeSeconds() + OFFSET
				);
			}
		}

		float soloFactor[2];
		soloFactor[0] = Turntable[0]->GetVideoSolo() && !Turntable[1]->GetVideoSolo();
		soloFactor[1] = Turntable[1]->GetVideoSolo() && !Turntable[0]->GetVideoSolo();

		Visualizer->SetVideos
		(
			Turntable[0]->VideoEncoderPercent==-1.0f ? Turntable[0]->GetVideo() : NULL,
			Turntable[0]->GetMixerVolumeFront()*((Turntable[0]->GetPaused() && Turntable[0]->GetRecordScratch()==false)?0.0f:1.0f)*(LGL_AudioAvailable()?1:0)*(1.0f-soloFactor[1]),
			Turntable[1]->VideoEncoderPercent==-1.0f ? Turntable[1]->GetVideo() : NULL,
			Turntable[1]->GetMixerVolumeFront()*((Turntable[1]->GetPaused() && Turntable[1]->GetRecordScratch()==false)?0.0f:1.0f)*(LGL_AudioAvailable()?1:0)*(1.0f-soloFactor[0])
		);

		float volAve[2];
		float volMax[2];
		float freqFactor[2];
		float peak[2];
		LGL_Video* vidBack[2];
		LGL_Video* vidFront[2];
		bool enable[2];

		for(int a=0;a<2;a++)
		{
			Turntable[a]->GetFreqMetaData(volAve[a],volMax[a],freqFactor[a]);
			peak[a]=Turntable[a]->GetVolumePeak();
			if(peak[a]==0.0f)
			{
				//Guard against divide-by-zero during John Cage - 4'33"
				peak[a]=1.0f;
			}
			vidBack[a]=Turntable[a]->GetVideoLo();
			vidFront[a]=Turntable[a]->GetVideoHi();
			volAve[a]=LGL_Min(1.0f,(1.0f/peak[a])*volAve[a]);
			volMax[a]=LGL_Min(1.0f,(1.0f/peak[a])*volMax[a]);
			enable[a]=Turntable[a]->GetVideoFrequencySensitiveMode();
		}

		Visualizer->SetFrequencySensitiveVideos
		(
			vidBack[0], vidFront[0], volAve[0], volMax[0], freqFactor[0], enable[0],
			vidBack[1], vidFront[1], volAve[1], volMax[1], freqFactor[1], enable[1]
		);
	}
}

void
MixerObj::
DrawFrame(bool visualizerQuadrent, float visualizerZoomOutPercent)
{
	float glow = GetGlowFromTime(LGL_SecondsSinceExecution());
	for(int a=0;a<2;a++)
	{
		Turntable[a]->DrawFrame(glow,visualizerQuadrent,visualizerZoomOutPercent);
	}

	LGL_DrawLogWrite("MixF|%.2f|%.2f|%c|%.3f\n",CrossFadeSliderLeft,CrossFadeSliderRight,visualizerQuadrent?'T':'F',visualizerZoomOutPercent);
	LGL_DrawLogPause();
	if(LowRez==false)
	{
		Mixer_DrawGlowLinesTurntables
		(
			LGL_SecondsSinceExecution(),
			CrossFadeSliderLeft,
			CrossFadeSliderRight,
			1.0f,
			visualizerQuadrent,
			visualizerZoomOutPercent
		);
	}
	LGL_DrawLogPause(false);
	
	//Mixer Levels Text
	LGL_DrawLogWrite
	(
		"MixL|%i|%i|%i|%i|%c|%.3f\n",
		(int)LGL_Round(100.0f*Turntable[1]->GetMixerVolumeFront()),
		(int)LGL_Round(100.0f*Turntable[0]->GetMixerVolumeFront()),
		(int)LGL_Round(100.0f*Turntable[1]->GetMixerVolumeBack()),
		(int)LGL_Round(100.0f*Turntable[0]->GetMixerVolumeBack()),
		visualizerQuadrent ? 'T' : 'F',
		visualizerZoomOutPercent
	);
	LGL_DrawLogPause();
	Mixer_DrawLevels
	(
		ViewPortBottom,
		ViewPortTop,
		LGL_Round(100.0f*Turntable[1]->GetMixerVolumeFront()),
		LGL_Round(100.0f*Turntable[0]->GetMixerVolumeFront()),
		LGL_Round(100.0f*Turntable[1]->GetMixerVolumeBack()),
		LGL_Round(100.0f*Turntable[0]->GetMixerVolumeBack()),
		visualizerQuadrent,
		visualizerZoomOutPercent
	);
	LGL_DrawLogPause(false);

	RecordingFailedTimer=LGL_Max(0.0f,RecordingFailedTimer-LGL_SecondsSinceLastFrame());

	DrawStatus(glow,visualizerQuadrent,visualizerZoomOutPercent);
}

void
MixerObj::
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

	assert(Turntable[0]);
	assert(Turntable[1]);

	Turntable[0]->SetViewPort
	(
		.025,			.975,
		bottom+.5*(top-bottom),	top
	);
	Turntable[1]->SetViewPort
	(
		.025,			.975,
		0,			bottom+.5*(top-bottom)
	);
}

void
MixerObj::
SetVisualizer
(
	VisualizerObj* viz
)
{
	Visualizer=viz;
	for(int a=0;a<2;a++)
	{
		Turntable[a]->SetVisualizer(viz);
	}
}

void
MixerObj::
SetVolumeMaster
(
	float	volumeMaster
)
{
	VolumeMaster=volumeMaster;
}

void
MixerObj::
SetTurntable
(
	int	index
)
{
	assert(index>=0 && index<2);

	for(int a=0;a<2;a++)
	{
		if(index==a)
		{
			return;
		}
	}

	Turntable[Focus]->SetFocus(false);
	Turntable[Focus]->SetTurntableNumber(-1);

	Turntable[index]->SetFocus(true);
	Turntable[index]->SetTurntableNumber(Focus);

	if(Focus==0)
	{
		Turntable[index]->SetViewPort(0.025,0.975,0.25,0.50);
	}
	else
	{
		Turntable[index]->SetViewPort(0.025,0.975,0.00,0.25);
	}
}

void
MixerObj::
SetRecording
(
	bool	inRecording
)
{
	if(Recording!=inRecording)
	{
		Recording=inRecording;
		if(Recording)
		{
			RecordingSecondsSinceExecution=LGL_SecondsSinceExecution();
			char file[1024];
			sprintf(file,"data/record/%s.tracklist",LGL_DateAndTimeOfDayOfExecution());
			RecordingTrackListFile=fopen(file,"w");
			LGL_Assert(RecordingTrackListFile);
			chmod
			(       
				 file,
				 S_IRUSR |       //o+r
				 S_IWUSR |       //o+w
				 S_IRGRP |       //g+r
				 S_IWGRP |       //g+w
				 S_IROTH |       //o+r
				 S_IWOTH         //o+w
			);
		}
		else
		{
			if(RecordingTrackListFile!=NULL)
			{
				fclose(RecordingTrackListFile);
				RecordingTrackListFile=NULL;
			}
		}
	}
}

bool
MixerObj::
GetRecording()	const
{
	return(Recording);
}

void
MixerObj::
SetRecordingFailed()
{
	RecordingFailedTimer=3.0f;
}

void
MixerObj::
SetViewPortStatus
(
	float	left,	float	right,
	float	bottom,	float	top
)
{
	ViewPortStatusLeft=left;
	ViewPortStatusRight=right;
	ViewPortStatusBottom=bottom;
	ViewPortStatusTop=top;
	ViewPortStatusWidth=right-left;
	ViewPortStatusHeight=top-bottom;
}

void
MixerObj::
SetLowRez
(
	bool	lowRez
)
{
	LowRez=lowRez;
	for(int a=0;a<2;a++)
	{
		Turntable[a]->SetLowRez(lowRez);
	}
}

void
MixerObj::
BlankFocusFilterText()
{
	Turntable[Focus]->BlankFilterTextIfMode0();
}

void
MixerObj::
DrawStatus
(
	float	glow,
	bool	visualizerQuadrent,
	float	visualizerZoomOutPercent
)
{
	bool drawEverything=false;

	float l=ViewPortStatusLeft;
	float r=ViewPortStatusRight;
	float b=ViewPortStatusBottom;
	float t=ViewPortStatusTop;
	float w=ViewPortStatusWidth;
	float h=ViewPortStatusHeight;

	LGL_ClipRectEnable
	(
		visualizerQuadrent?0.0f:l,
		visualizerQuadrent?0.5f:r,
		visualizerQuadrent?0.5f:b,
		visualizerQuadrent?1.0f:t
	);

	/*
	LGL_GetFont().DrawString
	(
		l+.825f*w,
		b+.95*h,
		.02f,
		1,1,1,1,
		false,.5f,
		LGL_TimeOfDay()
	);
	*/

	if(Recording || drawEverything)
	{
		int seconds=(int)
			(
				LGL_SecondsSinceExecution()-
				RecordingSecondsSinceExecution
			);

		int minutes=0;
		while(seconds>=60)
		{
			seconds-=60;
			minutes++;
		}
		int hours=0;
		while(minutes>=60)
		{
			minutes-=60;
			hours++;
		}
		LGL_GetFont().DrawString
		(
			l+.025f*w,
			b+.95f*h,
			.015f,
			1.0f,1.0f,1.0f,1.0f,
			false,.5f,
			"%.2i:%.2i.%.2i",
			hours,minutes,seconds
		);
	}
	else if(RecordingFailedTimer>0.0f)
	{
		LGL_GetFont().DrawString
		(
			l+.025f*w,
			b+.95f*h,
			.015f,
			RecordingFailedTimer,0.0f,0.0f,RecordingFailedTimer,
			false,.5f,
			"Recording failed"
		);
	}

	float freeMemPercent = LGL_MemoryFreePercent();
	if(freeMemPercent<0.05f || drawEverything)
	{
		LGL_GetFont().DrawString
		(
			l+.025f*w,
			b+.025f*h,
			.015f,
			1.0f,0.0f,0.0f,1.0f,
			false,.5f,
			"Low Free Memory: %.1f%%",
			freeMemPercent*100.0f
		);
	}

	/*
	LGL_DrawLogWrite
	(
		"MixS|%.3f|%.3f\n",
		visualizerQuadrent,
		visualizerZoomOutPercent
	);
	LGL_DrawLogPause();
	Mixer_DrawGlowLinesStatus
	(
		LGL_SecondsSinceExecution(),
		1.0f,
		visualizerQuadrent,
		visualizerZoomOutPercent
	);
	LGL_DrawLogPause(false);
	*/

	CPUSpeedHighest=(int)LGL_Max(LGL_CPUSpeed(),CPUSpeedHighest);

	int cpuTemp=999;//LGL_CPUTemp();

	if
	(
		CPUSpeedHighest>LGL_CPUSpeed() ||
		(
			cpuTemp>=CPU_TEMP_DANGER &&
			cpuTemp!=999
		) ||
		drawEverything
	)
	{
		float R,G,B;
		if(CPUSpeedHighest>LGL_CPUSpeed())
		{
			R=1.0f;
			G=0.0f;
			B=0.0f;
		}
		else
		{
			R=1.0f;
			G=1.0f;
			B=1.0f;
		}
		LGL_GetFont().DrawString
		(
			l+.025f*w,
			b+.90f*h,
			.02f,
			R,G,B,1.0f,
			false,.5f,
			"%.2f                                                                    GHz",	//Goofy hack to disable monospace
			LGL_CPUSpeed()/1000.0f
		);
		LGL_GetFont().DrawString
		(
			l+.125f*w,
			b+.90f*h,
			.02f,
			R,G,B,1.0f,
			false,.5f,
			"GHz"
		);

		if(cpuTemp!=999)
		{
			if(cpuTemp>=CPU_TEMP_DANGER)
			{
				R=1.0f;
				G=0.0f;
				B=0.0f;
			}
			else
			{
				R=1.0f;
				G=1.0f;
				B=1.0f;
			}
			LGL_GetFont().DrawString
			(
				l+.025f*w,
				b+.85f*h,
				.02f,
				R,G,B,1.0f,
				false,.5f,
				"%i                                                                      C",	//Goofy hack to disable monospace
				cpuTemp
			);
			LGL_GetFont().DrawString
			(
				l+.125f*w,
				b+.85f*h,
				.02f,
				R,G,B,1.0f,
				false,.5f,
				"C"
			);
		}
	}

	if(LGL_BatteryChargeDraining() || drawEverything)
	{
		LGL_GetFont().DrawString
		(
			l+.025f*w,
			b+.80f*h,
			.02f,
			1.0f,0,0,1.0f,
			false,.5f,
			"Battery Discharging"
		);
		LGL_GetFont().DrawString
		(
			l+.025f*w,
			b+.75f*h,
			.02f,
			1.0f,0,0,1.0f,
			false,.5f,
			"(%.2f%% left)",
			LGL_BatteryChargePercent()*100.0f
		);
	}

	float freeMB = LGL_FilesystemFreeSpaceMB();
	if(freeMB<1000 || drawEverything)
	{
		LGL_GetFont().DrawString
		(
			l+.025f*w,
			b+.70f*h,
			.02f,
			1.0f,0,0,1.0f,
			false,.5f,
			"Low Disk Space Remaining"
		);
		LGL_GetFont().DrawString
		(
			l+.025f*w,
			b+.65f*h,
			.02f,
			1.0f,0,0,1.0f,
			false,.5f,
			"(%.0fMB left)",
			freeMB
		);
	}

	//TODO: Get VUs looking better
	/*
	Turntable[0]->DrawVU
	(
		r-0.025f,r,
		b+0.075f,b+0.075f*2,
		glow
	);
	Turntable[1]->DrawVU
	(
		r-0.025f,r,
		b,b+0.075f,
		glow
	);
	*/

	if
	(
		(
			CanDisplayJackWarning &&
			LGL_AudioUsingJack()==false
		) ||
		drawEverything
	)
	{
		LGL_GetFont().DrawString
		(
			l+.05f*w,
			b+.45f*h,
			.03f,
			1.0f,0,0,1.0f,
			false,.5f,
			"WARNING: Not using JACK"
		);
		LGL_GetFont().DrawString
		(
			l+.05f*w,
			b+.25f*h,
			.015f,
			1.0f,0,0,1.0f,
			false,.5f,
			"Audio can lag & skip"
		);
		LGL_GetFont().DrawString
		(
			l+.05f*w,
			b+.20f*h,
			.015f,
			1.0f,0,0,1.0f,
			false,.5f,
			"Press [F4] for more info"
		);
	}

	LGL_ClipRectDisable();
}

