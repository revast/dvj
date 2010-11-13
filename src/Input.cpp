/*
 *
 * Input.cpp - Input abstraction object
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

#include "Input.h"

InputObj Input;

InputObj::
InputObj()
{
	//
}

InputObj::
~InputObj()
{
	//
}

//Core

void
InputObj::
AddChild
(
	InputObj*	child
)
{
	Children.push_back(child);
}

void
InputObj::
NextFrame()
{
	for(unsigned int a=0;a<Children.size();a++)
	{
		Children[a]->NextFrame();
	}
}

//Global Input

bool
InputObj::
FocusChange()	const
{
	bool change=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		change|=Children[a]->FocusChange();
	}

	return(change);
}

bool
InputObj::
FocusBottom()	const
{
	bool bottom=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		bottom|=Children[a]->FocusBottom();
	}

	return(bottom);
}

bool
InputObj::
FocusTop()	const
{
	bool top=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		top|=Children[a]->FocusTop();
	}

	return(top);
}

float
InputObj::
XfaderSpeakers()	const
{
	float xfade=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->XfaderSpeakers();
		if(candidate!=-1.0f)
		{
			xfade=candidate;
		}
	}

	return(xfade);
}

float
InputObj::
XfaderSpeakersDelta()	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->XfaderSpeakersDelta();
	}

	return(delta);
}

float
InputObj::
XfaderHeadphones()	const
{
	float xfade=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->XfaderHeadphones();
		if(candidate!=-1.0f)
		{
			xfade=candidate;
		}
	}

	return(xfade);
}

float
InputObj::
XfaderHeadphonesDelta()	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->XfaderHeadphonesDelta();
	}

	return(delta);
}

bool
InputObj::
SyncTopToBottom()	const
{
	bool sync=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		sync|=Children[a]->SyncTopToBottom();
	}

	return(sync);
}

bool
InputObj::
SyncBottomToTop()	const
{
	bool sync=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		sync|=Children[a]->SyncBottomToTop();
	}

	return(sync);
}

int
InputObj::
MasterToHeadphones()	const
{
	int to=-1;

	for(unsigned int a=0;a<Children.size();a++)
	{
		int candidate=Children[a]->MasterToHeadphones();
		if(candidate!=-1)
		{
			to=candidate;
		}
	}

	return(to);
}

//Mode 0: File Selection

float
InputObj::
FileScroll
(
	unsigned int	target
)	const
{
	float scroll=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		scroll+=Children[a]->FileScroll(target);
	}

	return(scroll);
}

int
InputObj::
FileSelect
(
	unsigned int	target
)	const
{
	int choose=0;

	for(unsigned int a=0;a<Children.size();a++)
	{
		int val=Children[a]->FileSelect(target);
		if(val != 0)
		{
			choose = val;
		}
	}

	return(choose);
}

bool
InputObj::
FileMarkUnopened
(
	unsigned int	target
)	const
{
	bool mark=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		mark|=Children[a]->FileMarkUnopened(target);
	}

	return(mark);
}

bool
InputObj::
FileRefresh
(
	unsigned int	target
)	const
{
	bool refresh=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		refresh|=Children[a]->FileRefresh(target);
	}

	return(refresh);
}

int
InputObj::
FileIndexHighlight
(
	unsigned int	target
)	const
{
	int highlight=-1;

	for(unsigned int a=0;a<Children.size();a++)
	{
		int val=Children[a]->FileIndexHighlight(target);
		if(val != -1)
		{
			highlight = val;
			break;
		}
	}

	return(highlight);
}



//Mode 1: Decoding...

bool
InputObj::
DecodeAbort
(
	unsigned int	target
)	const
{
	bool abort=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		abort|=Children[a]->DecodeAbort(target);
	}

	return(abort);
}

//Mode 2: Waveform

int
InputObj::
WaveformEject
(
	unsigned int	target
)	const
{
	int eject=0;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		int val=Children[a]->WaveformEject(target);
		if(val > eject)
		{
			eject = val;
		}
	}

	return(eject);
}

bool
InputObj::
WaveformTogglePause
(
	unsigned int	target
)	const
{
	bool toggle=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		toggle|=Children[a]->WaveformTogglePause(target);
	}

	return(toggle);
}

float
InputObj::
WaveformNudge
(
	unsigned int	target
)	const
{
	float nudge=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		nudge+=Children[a]->WaveformNudge(target);
	}

	return(nudge);
}

float
InputObj::
WaveformPitchbend
(
	unsigned int	target
)	const
{
	float pitchbend=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformPitchbend(target);
		if(candidate!=0.0f)
		{
			pitchbend=candidate;
		}
	}

	return(pitchbend);
}

float
InputObj::
WaveformPitchbendDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformPitchbendDelta(target);
	}

	return(delta);
}

float
InputObj::
WaveformEQLow
(
	unsigned int	target
)	const
{
	float low=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformEQLow(target);
		if(candidate!=-1.0f)
		{
			low=candidate;
		}
	}

	return(low);
}

float
InputObj::
WaveformEQLowDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformEQLowDelta(target);
	}

	return(delta);
}

bool
InputObj::
WaveformEQLowKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		kill|=Children[a]->WaveformEQLowKill(target);
	}

	return(kill);
}

float
InputObj::
WaveformEQMid
(
	unsigned int	target
)	const
{
	float mid=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformEQMid(target);
		if(candidate!=-1.0f)
		{
			mid=candidate;
		}
	}

	return(mid);
}

float
InputObj::
WaveformEQMidDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformEQMidDelta(target);
	}

	return(delta);
}

bool
InputObj::
WaveformEQMidKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		kill|=Children[a]->WaveformEQMidKill(target);
	}

	return(kill);
}

float
InputObj::
WaveformEQHigh
(
	unsigned int	target
)	const
{
	float high=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformEQHigh(target);
		if(candidate!=-1.0f)
		{
			high=candidate;
		}
	}

	return(high);
}

float
InputObj::
WaveformEQHighDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformEQHighDelta(target);
	}

	return(delta);
}

bool
InputObj::
WaveformEQHighKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		kill|=Children[a]->WaveformEQHighKill(target);
	}

	return(kill);
}

float
InputObj::
WaveformGain
(
	unsigned int	target
)	const
{
	float gain=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformGain(target);
		if(candidate!=-1.0f)
		{
			gain=candidate;
		}
	}

	return(gain);
}

float
InputObj::
WaveformGainDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformGainDelta(target);
	}

	return(delta);
}

bool
InputObj::
WaveformGainKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		kill|=Children[a]->WaveformGainKill(target);
	}

	return(kill);
}

float
InputObj::
WaveformVolumeSlider
(
	unsigned int	target
)	const
{
	float volume=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformVolumeSlider(target);
		if(candidate!=-1.0f)
		{
			volume=candidate;
		}
	}

	return(volume);
}

bool
InputObj::
WaveformVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		invert|=Children[a]->WaveformVolumeInvert(target);
	}

	return(invert);
}

bool
InputObj::
WaveformRapidVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		invert|=Children[a]->WaveformRapidVolumeInvert(target);
	}

	return(invert);
}

bool
InputObj::
WaveformRapidSoloInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		invert|=Children[a]->WaveformRapidSoloInvert(target);
	}

	return(invert);
}

bool
InputObj::
WaveformVolumeSolo
(
	unsigned int	target
)	const
{
	bool solo=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		solo|=Children[a]->WaveformVolumeSolo(target);
	}

	return(solo);
}

float
InputObj::
WaveformRewindFF
(
	unsigned int	target
)	const
{
	float rewindff=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		rewindff+=Children[a]->WaveformRewindFF(target);
	}

	return(rewindff);
}

bool
InputObj::
WaveformRecordHold
(
	unsigned int	target
)	const
{
	bool hold=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		hold|=Children[a]->WaveformRecordHold(target);
	}

	return(hold);
}

float
InputObj::
WaveformRecordSpeed
(
	unsigned int	target
)	const
{
	float speed=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		speed+=Children[a]->WaveformRecordSpeed(target);
	}

	return(speed);
}

bool
InputObj::
WaveformStutter
(
	unsigned int	target
)	const
{
	bool stutter=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		stutter|=Children[a]->WaveformStutter(target);
	}

	return(stutter);
}

float
InputObj::
WaveformStutterPitch
(
	unsigned int	target
)	const
{
	float pitch=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformStutterPitch(target);
		if(candidate!=-1.0f)
		{
			pitch=candidate;
		}
	}

	return(pitch);
}

float
InputObj::
WaveformStutterSpeed
(
	unsigned int	target
)	const
{
	float speed=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformStutterSpeed(target);
		if(candidate!=-1.0f)
		{
			speed=candidate;
		}
	}

	return(speed);
}

bool
InputObj::
WaveformSavePointPrev
(
	unsigned int	target
)	const
{
	bool prev=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		prev|=Children[a]->WaveformSavePointPrev(target);
	}

	return(prev);
}

bool
InputObj::
WaveformSavePointNext
(
	unsigned int	target
)	const
{
	bool next=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		next|=Children[a]->WaveformSavePointNext(target);
	}

	return(next);
}

int
InputObj::
WaveformSavePointPick
(
	unsigned int	target
)	const
{
	int pick=-9999;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		int candidate=Children[a]->WaveformSavePointPick(target);
		if(candidate!=-9999)
		{
			pick=candidate;
		}
	}

	return(pick);
}

bool
InputObj::
WaveformSavePointSet
(
	unsigned int	target
)	const
{
	bool set=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		set|=Children[a]->WaveformSavePointSet(target);
	}

	return(set);
}

float
InputObj::
WaveformSavePointUnsetPercent
(
	unsigned int	target
)	const
{
	float percent=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformSavePointUnsetPercent(target);
		if(candidate>percent)
		{
			percent=candidate;
		}
	}

	return(percent);
}

float
InputObj::
WaveformSavePointShift
(
	unsigned int	target
)	const
{
	float shift=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		shift+=Children[a]->WaveformSavePointShift(target);
	}

	return(shift);
}

float
InputObj::
WaveformSavePointShiftAll
(
	unsigned int	target
)	const
{
	float shift=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		shift+=Children[a]->WaveformSavePointShiftAll(target);
	}

	return(shift);
}

bool
InputObj::
WaveformSavePointShiftAllHere
(
	unsigned int	target
)	const
{
	bool here=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		here|=Children[a]->WaveformSavePointShiftAllHere(target);
	}

	return(here);
}

bool
InputObj::
WaveformSavePointJumpNow
(
	unsigned int	target
)	const
{
	bool jump=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		jump|=Children[a]->WaveformSavePointJumpNow(target);
	}

	return(jump);
}

bool
InputObj::
WaveformSavePointJumpAtMeasure
(
	unsigned int	target
)	const
{
	bool jump=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		jump|=Children[a]->WaveformSavePointJumpAtMeasure(target);
	}

	return(jump);
}

int
InputObj::
WaveformLoopMeasuresExponent
(
	unsigned int	target
)	const
{
	int exponent=WAVEFORM_LOOP_MEASURES_EXPONENT_NULL;

	for(unsigned int a=0;a<Children.size();a++)
	{
		int candidate=Children[a]->WaveformLoopMeasuresExponent(target);
		if(candidate!=WAVEFORM_LOOP_MEASURES_EXPONENT_NULL)
		{
			exponent=candidate;
		}
	}

	return(exponent);
}

bool
InputObj::
WaveformLoopMeasuresHalf
(
	unsigned int	target
)	const
{
	bool half=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		half|=Children[a]->WaveformLoopMeasuresHalf(target);
	}

	return(half);
}

bool
InputObj::
WaveformLoopMeasuresDouble
(
	unsigned int	target
)	const
{
	bool twoX=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		twoX|=Children[a]->WaveformLoopMeasuresDouble(target);
	}

	return(twoX);
}

bool
InputObj::
WaveformLoopSecondsLess
(
	unsigned int	target
)	const
{
	bool less=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		less|=Children[a]->WaveformLoopSecondsLess(target);
	}

	return(less);
}

bool
InputObj::
WaveformLoopSecondsMore
(
	unsigned int	target
)	const
{
	bool more=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		more|=Children[a]->WaveformLoopSecondsMore(target);
	}

	return(more);
}

bool
InputObj::
WaveformLoopAll
(
	unsigned int	target
)	const
{
	bool all=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		all|=Children[a]->WaveformLoopAll(target);
	}

	return(all);
}

bool
InputObj::
WaveformLoopToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		toggle|=Children[a]->WaveformLoopToggle(target);
	}

	return(toggle);
}

bool
InputObj::
WaveformLoopThenRecallActive
(
	unsigned int	target
)	const
{
	bool active=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		active|=Children[a]->WaveformLoopThenRecallActive(target);
	}

	return(active);
}

int
InputObj::
WaveformAutoDivergeRecall
(
	unsigned int	target
)	const
{
	int ret=0;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		int now = Children[a]->WaveformAutoDivergeRecall(target);
		if(now!=0)
		{
			ret=now;
		}
	}

	return(ret);
}

bool
InputObj::
WaveformVideoSelect
(
	unsigned int	target
)	const
{
	bool select=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		select|=Children[a]->WaveformVideoSelect(target);
	}

	return(select);
}

float
InputObj::
WaveformVideoBrightness
(
	unsigned int	target
)	const
{
	float bright=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformVideoBrightness(target);
		if(candidate!=-1.0f)
		{
			bright=candidate;
		}
	}

	return(bright);
}

float
InputObj::
WaveformVideoBrightnessDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformVideoBrightnessDelta(target);
	}

	return(delta);
}

float
InputObj::
WaveformVideoAdvanceRate
(
	unsigned int	target
)	const
{
	float rate=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformVideoAdvanceRate(target);
		if(candidate!=-1.0f)
		{
			rate=candidate;
		}
	}

	return(rate);
}

float
InputObj::
WaveformFreqSenseBrightness
(
	unsigned int	target
)	const
{
	float brightness=-1.0f;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformFreqSenseBrightness(target);
		if(candidate!=-1.0f)
		{
			brightness=candidate;
		}
	}

	return(brightness);
}

float
InputObj::
WaveformFreqSenseBrightnessDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformFreqSenseBrightnessDelta(target);
	}

	return(delta);
}

int
InputObj::
WaveformAudioInputMode
(
	unsigned int	target
)	const
{
	int mode=-1;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		int candidate=Children[a]->WaveformAudioInputMode(target);
		if(candidate!=-1)
		{
			mode=candidate;
		}
	}

	return(mode);
}

bool
InputObj::
WaveformVideoAspectRatioNext
(
	unsigned int	target
)	const
{
	bool next=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		next|=Children[a]->WaveformVideoAspectRatioNext(target);
	}

	return(next);
}

float
InputObj::
WaveformOscilloscopeBrightness
(
	unsigned int	target
)	const
{
	float bright=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformOscilloscopeBrightness(target);
		if(candidate!=-1.0f)
		{
			bright=candidate;
		}
	}

	return(bright);
}

float
InputObj::
WaveformOscilloscopeBrightnessDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformOscilloscopeBrightnessDelta(target);
	}

	return(delta);
}

bool
InputObj::
WaveformSyncBPM
(
	unsigned int	target
)	const
{
	bool sync=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		sync|=Children[a]->WaveformSyncBPM(target);
	}

	return(sync);
}

float
InputObj::
WaveformPointerScratch
(
	unsigned int	target
)	const
{
	float targetX=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformPointerScratch(target);
		if(candidate!=-1.0f)
		{
			targetX=candidate;
		}
	}

	return(targetX);
}


