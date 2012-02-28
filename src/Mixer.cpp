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

#include "Config.h"
#include "Common.h"
#include "Input.h"

#include <sys/types.h>
#include <sys/stat.h>

#define	CPU_TEMP_DANGER (70)

MixerObj&
GetMixer()
{
	static MixerObj mixer;
	return(mixer);
}

MixerObj::
MixerObj()
{
	for(int a=0;a<2;a++)
	{
		float bottom = 0.25f;
		float top = 0.50f;
		if(a==1)
		{
			bottom=0.0f;
			top=0.25f;
		}
		Turntable[a]=new TurntableObj(0.025f,0.975f,bottom,top,&Database);
		Turntable[a]->SetTurntableNumber(a);
	}

	Focus=0;
	Turntable[Focus]->SetFocus(true);
	Turntable[(Focus+1)%2]->SetFocus(false);

	CPUSpeedHighest=0;

	SetViewport(0,1,0,0.5f);

	for(int a=0;a<3;a++)
	{
		EQXponent[a]=1.0f;
		EQJP8k[a]=1.0f;
	}

	CrossfadeMiddle=1.0f;
	CrossfadeSliderLeft=0.5f;
	CrossfadeSliderRight=0.5f;

	Visualizer=NULL;

	Recording=false;
	RecordingDetermined=false;
	RecordingSecondsSinceExecution=0.0f;
	strcpy(RecordingTrackListPath,GetDVJSessionTracklistPath());
	RecordingTrackListFD=NULL;
	RecordingFailedTimer=0.0f;

	VideoAdvancedLastFrame=0;

	LowRez=false;
	CanDisplayJackWarning=true;

	SetViewportStatus(0.0f,1.0f,0.5f,1.0f);
}

MixerObj::
~MixerObj()
{
	Cleanup();
}

void
MixerObj::
Cleanup()
{
	for(int a=0;a<2;a++)
	{
		delete Turntable[a];
		Turntable[a]=NULL;
	}

	if(RecordingTrackListFD!=NULL)
	{
		fclose(RecordingTrackListFD);
		RecordingTrackListFD=NULL;
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
	if(RecordingDetermined==false)
	{
		int result = LGL_GetRecordDVJToFile();
		if(result==1)
		{
			RecordingDetermined=true;
			SetRecording(true);
		}
		else if(result==-1)
		{
			RecordingDetermined=true;
			SetRecordingFailed();
		}
	}

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

		if
		(
			CanDisplayJackWarning &&
			LGL_AudioUsingJack()==false
		)
		{
			if(LGL_KeyStroke(LGL_KEY_F1))
			{
#ifndef	LGL_OSX
				system("firefox http://code.google.com/p/dvj/wiki/JACK &");
				exit(0);
#endif	//LGL_OSX
			}
		}
	}

	if(GetInput().FocusChange())
	{
		Turntable[Focus]->SetFocus(false);
		Focus=((Focus+1)%2);
		Turntable[Focus]->SetFocus(true);
	}

	if(GetInput().FocusTop())
	{
		Turntable[0]->SetFocus(true);
		Turntable[1]->SetFocus(false);
		Focus=0;
	}
	else if(GetInput().FocusBottom())
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

	if(TurntableObj::GetSurroundMode()==false)
	{
		int masterToHeadphones=GetInput().MasterToHeadphones();
		if(masterToHeadphones==0)
		{
			LGL_AudioMasterToHeadphones(false);
		}
		else if(masterToHeadphones==1)
		{
			LGL_AudioMasterToHeadphones(true);
		}
	}

	candidate=GetInput().XfaderSpeakers();
	if(candidate!=-1.0f)
	{
		CrossfadeSliderLeft=candidate;
		if
		(
			CrossfadeSliderLeft>=63.0f/127.0f &&
			CrossfadeSliderLeft<=65.0f/127.0f
		)
		{
			CrossfadeSliderLeft=0.5f;
		}
		if(LGL_AudioChannels()==2)
		{
			CrossfadeSliderRight=candidate;
			if
			(
				CrossfadeSliderRight>=63.0f/127.0f &&
				CrossfadeSliderRight<=65.0f/127.0f
			)
			{
				CrossfadeSliderRight=0.5f;
			}
		}
	}

	if(TurntableObj::GetSurroundMode()==false)
	{
		candidate=GetInput().XfaderHeadphones();
	}
	else
	{
		candidate=GetInput().XfaderSpeakers();
	}
	if(candidate!=-1.0f)
	{
		CrossfadeSliderRight=candidate;
		if
		(
			CrossfadeSliderRight>=63.0f/127.0f &&
			CrossfadeSliderRight<=65.0f/127.0f
		)
		{
			CrossfadeSliderRight=0.5f;
		}
		if(LGL_AudioChannels()==2)
		{
			CrossfadeSliderLeft=candidate;
			if
			(
				CrossfadeSliderLeft>=63.0f/127.0f &&
				CrossfadeSliderLeft<=65.0f/127.0f
			)
			{
				CrossfadeSliderLeft=0.5f;
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
		if(Turntable[Focus]->GetMode()==2)
		{
			if(GetInput().WaveformSync(TARGET_FOCUS | ((Focus==0) ? TARGET_TOP : TARGET_BOTTOM)))
			{
				syncTT=Focus;
				target=Focus?0:1;
			}
		}
	}

	if(syncTT!=-1)
	{
		//Sync BPM

		float midiClockBPM = LGL_MidiClockBPM();
		float midiClockPercentOfCurrentMeasure = LGL_MidiClockPercentOfCurrentMeasure();

		if
		(
			midiClockBPM > 0 &&
			midiClockPercentOfCurrentMeasure >= 0.0f
		)
		{
			for(int t=0;t<2;t++)
			{

				if(!GetInput().WaveformSync(TARGET_FOCUS | ((Focus==0) ? TARGET_TOP : TARGET_BOTTOM)))
				{
					continue;
				}

				Turntable[t]->SetBPMAdjusted(midiClockBPM);

				if(Turntable[t]->GetPaused()==false)
				{
					//Nudge toward sync
					//float bpmScalar = Turntable[target]->GetBPMAdjusted()/Turntable[syncTT]->GetBPMAdjusted();
					//float percentOfMeasureSelf = Turntable[syncTT]->GetPercentOfCurrentMeasure(1.0f/bpmScalar,true);
					float percentOfMeasureSelf = Turntable[t]->GetPercentOfCurrentMeasure(1.0f,true);
					float percentOfMeasureTarget = midiClockPercentOfCurrentMeasure;
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
							Turntable[t]->SetMixerNudge(nudgeAmount);
						}
						else
						{
							Turntable[t]->SetMixerNudge(0.0f);
						}
					}
				}
			}
		}
		else
		{
			Turntable[syncTT]->SetBPMAdjusted(Turntable[target]->GetBPMAdjusted());

			if
			(
				Turntable[0]->GetPaused()==false &&
				Turntable[1]->GetPaused()==false
			)
			{
				//Nudge toward sync
				//float bpmScalar = Turntable[target]->GetBPMAdjusted()/Turntable[syncTT]->GetBPMAdjusted();
				//float percentOfMeasureSelf = Turntable[syncTT]->GetPercentOfCurrentMeasure(1.0f/bpmScalar,true);
				float percentOfMeasureSelf = Turntable[syncTT]->GetPercentOfCurrentMeasure(1.0f,true);
				float percentOfMeasureTarget = Turntable[target]->GetPercentOfCurrentMeasure(1.0f,true);
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
		delta+=GetInput().XfaderSpeakersDelta();
		delta+=GetInput().XfaderHeadphonesDelta();
		CrossfadeSliderLeft=LGL_Clamp(0.0f,CrossfadeSliderLeft+delta,1.0f);
		CrossfadeSliderRight=LGL_Clamp(0.0f,CrossfadeSliderRight+delta,1.0f);
	}
	else if(LGL_AudioChannels()==4)
	{
		CrossfadeSliderLeft=LGL_Clamp(0.0f,CrossfadeSliderLeft+GetInput().XfaderSpeakersDelta(),1.0f);
		CrossfadeSliderRight=LGL_Clamp(0.0f,CrossfadeSliderRight+GetInput().XfaderHeadphonesDelta(),1.0f);
	}

	int masterTT=-1;
	if
	(
		Turntable[0]->GetMode()==2 &&
		Turntable[1]->GetMode()==2
	)
	{
		if(CrossfadeSliderLeft>0.5f)
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
		int b=(a+1)%2;
		Turntable[a]->SetMixerVolumeBack(0);
		Turntable[a]->SetRespondToRhythmicSoloInvert
		(
			Turntable[b]->GetRhythmicSoloInvert() ?
				Turntable[b]->GetSoundChannel() :
				-1
		);
	}

	bool solo[2];
	solo[0]=Turntable[0]->GetSolo();
	solo[1]=Turntable[1]->GetSolo();
	bool soloActive=(solo[0] || solo[1]);

	//Update Active Turntables
	for(int which=0;which<2;which++)
	{
		float meFront;
		float meBack;

		int soloStatus=0;
		if(soloActive)
		{
			soloStatus = solo[which] ? 1 : -1;
		}

		if
		(
			KillSwitch[which]==1 ||
			soloStatus==-1
		)
		{
			//Kill Switch
			meFront=0;
			meBack=0;
		}
		else if
		(
			FullSwitch[which]==1 ||
			soloStatus==1
		)
		{
			//Full Volume case
			meFront=1;
			meBack=1;
		}
		else
		{
			meFront=(1-which)*CrossfadeSliderLeft+(which*(1-CrossfadeSliderLeft));
			meBack=(1-which)*CrossfadeSliderRight+(which*(1-CrossfadeSliderRight));
		}

		if(meFront>.5)
		{
			//Crossfader closs to max
			meFront=CrossfadeMiddle+(1.0-CrossfadeMiddle)*2*(meFront-.5);
		}
		else
		{
			//Crossfader far from max
			meFront=CrossfadeMiddle*(2*meFront);
		}
		if(meBack>.5)
		{
			//Crossfader closs to max
			meBack=CrossfadeMiddle+(1.0-CrossfadeMiddle)*2*(meBack-.5);
		}
		else
		{
			//Crossfader far from max
			meBack=CrossfadeMiddle*(2*meBack);
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

		Turntable[which]->SetMixerVolumeFront(meFront);
		Turntable[which]->SetMixerVolumeBack(meBack);

		Turntable[which]->SetMixerCrossfadeFactorBack
		(
			(which==0) ?
			0.0f+CrossfadeSliderRight :
			1.0f-CrossfadeSliderRight
		);
		Turntable[which]->SetMixerCrossfadeFactorFront
		(
			(which==0) ?
			0.0f+CrossfadeSliderLeft :
			1.0f-CrossfadeSliderLeft
		);

		Turntable[which]->NextFrame(secondsElapsed);
		if(const char* data = Turntable[which]->GetMetaDataSavedThisFrame())
		{
			const char* pathShort=Turntable[which]->GetSoundPathShort();
			if(pathShort)
			{
				for(int a=0;a<2;a++)
				{
					if
					(
						a!=which &&
						Turntable[a]->GetSoundPathShort() &&
						strcmp(Turntable[a]->GetSoundPathShort(),pathShort)==0
					)
					{
						Turntable[a]->LoadMetaData(data);
					}
				}
			}
		}

		if(RecordingTrackListFD!=NULL)
		{
			std::vector<char*> updates=Turntable[which]->GetTrackListFileUpdates();
			if(updates.empty()==false)
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

				for(unsigned int a=0;a<updates.size();a++)
				{
					fprintf(RecordingTrackListFD,"%s - Turntable[%i] - %s\n",timestamp,which,updates[a]);
				}
			}
		}
	}

	//Maybe display the text of the song we're playing
	TurntableObj* tt=NULL;
	if(CrossfadeSliderLeft>0.75f)
	{
		tt=Turntable[0];
	}
	else if(CrossfadeSliderLeft<0.25f)
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
			for(int a=(int)strlen(pushme)-1;a>=0;a--)
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
					Turntable[0]->GetMixerVolumeFront() + ((Turntable[0]->GetVideo()==NULL) ? -10.0f : 0) > 
					Turntable[1]->GetMixerVolumeFront() + ((Turntable[1]->GetVideo()==NULL) ? -10.0f : 0)
				) ? 0 : 1;
			}
		}

		if(highlighted==-1)
		{
			highlighted=(CrossfadeSliderLeft <= 0.5f) ? 0 : 1;
		}

		//Pick a video to advance
		/*
		if(highlighted!=-1)
		{
			if(Turntable[highlighted]->GetVideo())
			{
				Turntable[highlighted]->GetVideo()->SetPrimaryDecoder();
			}
		}
		*/

		//Set FreqSense videos
		for(int a=0;a<2;a++)
		{
			TurntableObj* tt = Turntable[a];

			if(tt->GetVideoLo())
			{
				tt->GetVideoLo()->SetVideo(tt->GetVideoLoPath());
			}
			if(tt->GetVideoHi())
			{
				tt->GetVideoHi()->SetVideo(tt->GetVideoHiPath());
			}
		}

		for(int i=0;i<2;i++)
		{
			if(Turntable[i]->GetVideo())
			{
				const float OFFSET = 2.0f/60.0f;	//Combat lag associated with projector scaling
				if(Turntable[i]->GetVideoBrightnessPreview()>0.0f)
				{
					Turntable[i]->GetVideo()->SetTime
					(
						Turntable[i]->GetVideoTimeSeconds() + OFFSET
					);
				}
			}
		}

		Turntable[1]->SetMixerVideoMute(Turntable[0]->GetVideoSolo() && !Turntable[1]->GetVideoSolo());
		Turntable[0]->SetMixerVideoMute(Turntable[1]->GetVideoSolo() && !Turntable[0]->GetVideoSolo());
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

	LGL_DrawLogWrite("MixF|%.2f|%.2f|%c|%.3f\n",CrossfadeSliderLeft,CrossfadeSliderRight,visualizerQuadrent?'T':'F',visualizerZoomOutPercent);
	LGL_DrawLogPause();
	if(LowRez==false)
	{
		Mixer_DrawGlowLinesTurntables
		(
			LGL_SecondsSinceExecution(),
			CrossfadeSliderLeft,
			CrossfadeSliderRight,
			Turntable[1]->GetBeatThisFrame(),
			Turntable[0]->GetBeatThisFrame(),
			Turntable[1]->GetPaused() ? -1.0f : Turntable[1]->GetPercentOfCurrentMeasure(0.25f,true),
			Turntable[0]->GetPaused() ? -1.0f : Turntable[0]->GetPercentOfCurrentMeasure(0.25f,true),
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
		ViewportBottom,
		ViewportTop,
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

	if(GetDebugRecordHold())
	{
		if(LGL_GetXponent())
		{
			LGL_DebugPrintf("Left Record: %s\n",
				LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_RECORD) ? "Down" : "Up");
			LGL_DebugPrintf("Right Record: %s\n",
				LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_RECORD) ? "Down" : "Up");
		}
	}
}

void
MixerObj::
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

	assert(Turntable[0]);
	assert(Turntable[1]);

	Turntable[0]->SetViewport
	(
		.025,			.975,
		bottom+.5*(top-bottom),	top
	);
	Turntable[1]->SetViewport
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
		Turntable[index]->SetViewport(0.025,0.975,0.25,0.50);
	}
	else
	{
		Turntable[index]->SetViewport(0.025,0.975,0.00,0.25);
	}
}

TurntableObj*
MixerObj::
GetTurntable
(
	int	index
)
{
	return(Turntable[index]);
}

TurntableObj**
MixerObj::
GetTurntables()
{
	return(Turntable);
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
			/*
			char file[2048];
			strcpy(file,RecordingTrackListPath);
			RecordingTrackListFD=fopen(file,"w");
			if(RecordingTrackListFD)
			{
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
				//
			}
			*/
		}
		else
		{
			/*
			if(RecordingTrackListFD!=NULL)
			{
				fclose(RecordingTrackListFD);
				RecordingTrackListFD=NULL;
			}
			*/
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
	if(GetDVJSessionFlacPath()[0]!='\0')
	{
		RecordingFailedTimer=3.0f;
	}
}

void
MixerObj::
SetViewportStatus
(
	float	left,	float	right,
	float	bottom,	float	top
)
{
	ViewportStatusLeft=left;
	ViewportStatusRight=right;
	ViewportStatusBottom=bottom;
	ViewportStatusTop=top;
	ViewportStatusWidth=right-left;
	ViewportStatusHeight=top-bottom;
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

int
MixerObj::
GetFocus()
{
	return(Focus);
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

	float l=ViewportStatusLeft;
	float r=ViewportStatusRight;
	float b=ViewportStatusBottom;
	float t=ViewportStatusTop;
	float w=ViewportStatusWidth;
	float h=ViewportStatusHeight;

	if(l>1.0f) return;

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
		float secondsTotal=
			(
				LGL_SecondsSinceExecution()-
				RecordingSecondsSinceExecution
			);
		int seconds=(int)secondsTotal;

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

		char out[2048];
		float bright=1.0f;
		if(secondsTotal<5.0f)
		{
			sprintf(out,"Recording to %s",LGL_RecordDVJToFilePathShort());
			bright=LGL_Min(1.0f,5.0f-secondsTotal);
		}
		else
		{
			sprintf
			(
				out,
				"%.2i:%.2i.%.2i",
				hours,minutes,seconds
			);
			bright=LGL_Min(1.0f,secondsTotal-5.0f);
		}
		LGL_GetFont().DrawString
		(
			l+.025f*w,
			b+.95f*h,
			.015f,
			bright,bright,bright,1.0f,
			false,.5f,
			out
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

#ifndef	LGL_OSX
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
#endif

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
#ifndef	LGL_OSX
		LGL_GetFont().DrawString
		(
			l+.05f*w,
			b+.20f*h,
			.015f,
			1.0f,0,0,1.0f,
			false,.5f,
			"Press [F1] for more info"
		);
#endif	//LGL_OSX
	}

	LGL_ClipRectDisable();
}

