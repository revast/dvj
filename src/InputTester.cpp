/*
 *
 * InputTester.cpp - Input abstraction object
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

#include "InputTester.h"

InputTesterObj::
InputTesterObj()
{
	Enable=false;
}

InputTesterObj::
~InputTesterObj()
{
	//
}

//Core

void
InputTesterObj::
NextFrame()
{
	const float actionDelaySeconds = 0.1f;

	if(LGL_KeyDown(LGL_KEY_TAB))
	{
		if(EnableTimer.SecondsSinceLastReset() > 5.0f)
		{
			Enable = true;
		}
	}
	else
	{
		EnableTimer.Reset();
	}

	if(LGL_KeyStroke(LGL_KEY_TAB))
	{
		Enable = false;
	}
	
	if(Enable && LastActionTimer.SecondsSinceLastReset() >= actionDelaySeconds)
	{
		//Choose a new action
		CurrentActionBottom = rand() % ACTION_LAST;
		CurrentActionTop = rand() % ACTION_LAST;
	}
	else
	{
		//No action this frame
		CurrentActionBottom = NOOP;
		CurrentActionTop = NOOP;
	}
}

//Global Input

bool
InputTesterObj::
FocusChange()	const
{
	bool change=CurrentActionBottom == FOCUS_CHANGE;
	return(change);
}

bool
InputTesterObj::
FocusBottom()	const
{
	bool bottom=CurrentActionBottom == FOCUS_BOTTOM;
	return(bottom);
}

bool
InputTesterObj::
FocusTop()	const
{
	bool top=CurrentActionBottom == FOCUS_TOP;
	return(top);
}

float
InputTesterObj::
XfaderSpeakers()	const
{
	float xfade=-1.0f;
	return(xfade);
}

float
InputTesterObj::
XfaderSpeakersDelta()	const
{
	float delta=
		((CurrentActionBottom == XFADER_SPEAKERS_DELTA_DOWN) ? -0.05f : 0.0f) +
		((CurrentActionBottom == XFADER_SPEAKERS_DELTA_UP) ? 0.05f : 0.0f);
	return(delta);
}

float
InputTesterObj::
XfaderHeadphones()	const
{
	float xfade=-1.0f;
	return(xfade);
}

float
InputTesterObj::
XfaderHeadphonesDelta()	const
{
	float delta=
		((CurrentActionBottom == XFADER_HEADPHONES_DELTA_DOWN) ? -0.05f : 0.0f) +
		((CurrentActionBottom == XFADER_HEADPHONES_DELTA_UP) ? 0.05f : 0.0f);
	return(delta);
}

int
InputTesterObj::
MasterToHeadphones()	const
{
	int to = -1;
	if(CurrentActionBottom == MASTER_TO_HEADPHONES)
	{
		to=LGL_RandInt(0,1);
	}
	return(to);
}

//Mode 0: File Selection

float
InputTesterObj::
FileScroll
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	float scroll=
		((currentAction == FILE_SCROLL_PREV) ? 1.0f : 0.0f) +
		((currentAction == FILE_SCROLL_NEXT) ? -1.0f : 0.0f) +
		((currentAction == FILE_SCROLL_DOWN_MANY) ? -5.0f : 0.0f) +
		((currentAction == FILE_SCROLL_UP_MANY) ? 5.0f : 0.0f);
	return(scroll);
}

int
InputTesterObj::
FileSelect
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	int choose=(currentAction == FILE_SELECT) ? 2 : 0;
	return(choose);
}

bool
InputTesterObj::
FileMarkUnopened
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool mark=currentAction == FILE_MARK_UNOPENED;
	return(mark);
}

bool
InputTesterObj::
FileRefresh
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool refresh=currentAction == FILE_REFRESH;
	return(refresh);
}



//Mode 2: Waveform

int
InputTesterObj::
WaveformEject
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	int eject=0;
	if(currentAction == WAVEFORM_EJECT)
	{
		if((rand()%20)==0)
		{
			eject = 2;
		}
	}
	return(eject);
}

bool
InputTesterObj::
WaveformPauseToggle
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool toggle=currentAction == WAVEFORM_PAUSE_TOGGLE;
	return(toggle);
}

float
InputTesterObj::
WaveformNudge
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	float nudge=
		((currentAction == WAVEFORM_NUDGE_SLOWER) ? -0.02f : 0.0f) +
		((currentAction == WAVEFORM_NUDGE_FASTER) ? 0.02f : 0.0f);
	return(nudge);
}

float
InputTesterObj::
WaveformPitchbend
(
	unsigned int	target
)	const
{
	float pitchbend=0.0f;
	return(pitchbend);
}

float
InputTesterObj::
WaveformPitchbendDelta
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	float delta=
		((currentAction == WAVEFORM_PITCHBEND_DELTA_DOWN_SLOW) ? -0.005f : 0.0f) +
		((currentAction == WAVEFORM_PITCHBEND_DELTA_DOWN_FAST) ? -0.1f : 0.0f) +
		((currentAction == WAVEFORM_PITCHBEND_DELTA_UP_SLOW) ? 0.005f : 0.0f) +
		((currentAction == WAVEFORM_PITCHBEND_DELTA_UP_FAST) ? 0.1f : 0.0f);
	return(delta);
}

float
InputTesterObj::
WaveformEQLow
(
	unsigned int	target
)	const
{
	float low=-1.0f;
	return(low);
}

float
InputTesterObj::
WaveformEQLowDelta
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	float delta=
		((currentAction == WAVEFORM_EQ_LOW_DELTA_DOWN) ? -LGL_SecondsSinceLastFrame() : 0.0f) +
		((currentAction == WAVEFORM_EQ_LOW_DELTA_UP) ? LGL_SecondsSinceLastFrame(): 0.0f);
	return(delta);
}

bool
InputTesterObj::
WaveformEQLowKill
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool kill=currentAction == WAVEFORM_EQ_LOW_KILL;
	return(kill);
}

float
InputTesterObj::
WaveformEQMid
(
	unsigned int	target
)	const
{
	float mid=-1.0f;
	return(mid);
}

float
InputTesterObj::
WaveformEQMidDelta
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	float delta=
		((currentAction == WAVEFORM_EQ_MID_DELTA_DOWN) ? -LGL_SecondsSinceLastFrame() : 0.0f) +
		((currentAction == WAVEFORM_EQ_MID_DELTA_UP) ? LGL_SecondsSinceLastFrame(): 0.0f);
	return(delta);
}

bool
InputTesterObj::
WaveformEQMidKill
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool kill=currentAction == WAVEFORM_EQ_MID_KILL;
	return(kill);
}

float
InputTesterObj::
WaveformEQHigh
(
	unsigned int	target
)	const
{
	float high=-1.0f;
	return(high);
}

float
InputTesterObj::
WaveformEQHighDelta
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	float delta=
		((currentAction == WAVEFORM_EQ_HIGH_DELTA_DOWN) ? -LGL_SecondsSinceLastFrame() : 0.0f) +
		((currentAction == WAVEFORM_EQ_HIGH_DELTA_UP) ? LGL_SecondsSinceLastFrame(): 0.0f);
	return(delta);
}

bool
InputTesterObj::
WaveformEQHighKill
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool kill=currentAction == WAVEFORM_EQ_HIGH_KILL;
	return(kill);
}

float
InputTesterObj::
WaveformGain
(
	unsigned int	target
)	const
{
	float gain=-1.0f;
	return(gain);
}

float
InputTesterObj::
WaveformGainDelta
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	float delta=
		((currentAction == WAVEFORM_GAIN_DELTA_DOWN) ? -LGL_SecondsSinceLastFrame() : 0.0f) +
		((currentAction == WAVEFORM_GAIN_DELTA_UP) ? LGL_SecondsSinceLastFrame(): 0.0f);
	return(delta);
}

bool
InputTesterObj::
WaveformGainKill
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool kill=currentAction == WAVEFORM_GAIN_KILL;
	return(kill);
}

float
InputTesterObj::
WaveformVolumeSlider
(
	unsigned int	target
)	const
{
	float volume=-1.0f;
	return(volume);
}

bool
InputTesterObj::
WaveformVolumeInvert
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool invert=currentAction == WAVEFORM_VOLUME_INVERT;
	return(invert);
}

bool
InputTesterObj::
WaveformRhythmicVolumeInvert
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool invert=currentAction == WAVEFORM_RHYTHMIC_VOLUME_INVERT;
	return(invert);
}

bool
InputTesterObj::
WaveformRhythmicVolumeInvertOther
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool invert=currentAction == WAVEFORM_RHYTHMIC_VOLUME_INVERT_OTHER;
	return(invert);
}

bool
InputTesterObj::
WaveformVolumeSolo
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool solo=currentAction == WAVEFORM_VOLUME_SOLO;
	return(solo);
}

float
InputTesterObj::
WaveformRewindFF
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	float rewindff=
		((currentAction == WAVEFORM_SEEK_BACKWARD_FAST) ? -32.0f : 0.0f) +
		((currentAction == WAVEFORM_SEEK_FORWARD_FAST) ? 32.0f : 0.0f);
	return(rewindff);
}

bool
InputTesterObj::
WaveformRecordHold
(
	unsigned int	target
)	const
{
	bool hold=WaveformRecordSpeed(target)!=0.0f;
	return(hold);
}

float
InputTesterObj::
WaveformRecordSpeed
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	float speed=
		((currentAction == WAVEFORM_SEEK_BACKWARD_SLOW) ? -2.0f : 0.0f) +
		((currentAction == WAVEFORM_SEEK_FORWARD_SLOW) ? 2.0f : 0.0f);
	return(speed);
}

bool
InputTesterObj::
WaveformStutter
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool stutter=currentAction == WAVEFORM_STUTTER;
	return(stutter);
}

float
InputTesterObj::
WaveformStutterPitch
(
	unsigned int	target
)	const
{
	float pitch=-1.0f;
	return(pitch);
}

float
InputTesterObj::
WaveformStutterSpeed
(
	unsigned int	target
)	const
{
	float speed=-1.0f;
	return(speed);
}

bool
InputTesterObj::
WaveformSavePointPrev
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool prev=currentAction == WAVEFORM_SAVEPOINT_PREV;
	return(prev);
}

bool
InputTesterObj::
WaveformSavePointNext
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool next=currentAction == WAVEFORM_SAVEPOINT_NEXT;
	return(next);
}

int
InputTesterObj::
WaveformSavePointPick
(
	unsigned int	target
)	const
{
	int pick=-9999;
	return(pick);
}

bool
InputTesterObj::
WaveformSavePointSet
(
	unsigned int	target
)	const
{
	bool set=false;
	return(set);
}

float
InputTesterObj::
WaveformSavePointUnsetPercent
(
	unsigned int	target
)	const
{
	float percent=0.0f;
	return(percent);
}

float
InputTesterObj::
WaveformSavePointShift
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

float
InputTesterObj::
WaveformSavePointShiftAll
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

bool
InputTesterObj::
WaveformSavePointShiftAllHere
(
	unsigned int	target
)	const
{
	bool here=false;
	return(here);
}

bool
InputTesterObj::
WaveformSavePointJumpNow
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool jump=currentAction == WAVEFORM_SAVEPOINT_JUMP_NOW;
	return(jump);
}

bool
InputTesterObj::
WaveformSavePointJumpAtMeasure
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool jump=currentAction == WAVEFORM_SAVEPOINT_JUMP_AT_MEASURE;
	return(jump);
}

int
InputTesterObj::
WaveformLoopMeasuresExponent
(
	unsigned int	target
)	const
{
	int exponent=WAVEFORM_LOOP_MEASURES_EXPONENT_NULL;
	return(exponent);
}

bool
InputTesterObj::
WaveformQuantizationPeriodHalf
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool half=currentAction == WAVEFORM_QUANTIZATION_PERIOD_HALF;
	return(half);
}

bool
InputTesterObj::
WaveformQuantizationPeriodDouble
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool twoX=currentAction == WAVEFORM_QUANTIZATION_PERIOD_DOUBLE;
	return(twoX);
}

bool
InputTesterObj::
WaveformLoopSecondsLess
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool less=currentAction == WAVEFORM_QUANTIZATION_PERIOD_HALF;
	return(less);
}

bool
InputTesterObj::
WaveformLoopSecondsMore
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool more=currentAction == WAVEFORM_QUANTIZATION_PERIOD_DOUBLE;
	return(more);
}

bool
InputTesterObj::
WaveformLoopAll
(
	unsigned int	target
)	const
{
	bool all=false;
	return(all);
}


bool
InputTesterObj::
WaveformLoopToggle
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool toggle=currentAction == WAVEFORM_LOOP_TOGGLE;
	return(toggle);
}

bool
InputTesterObj::
WaveformLoopThenRecallActive
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool active=currentAction == WAVEFORM_LOOP_THEN_RECALL;
	return(active);
}

int
InputTesterObj::
WaveformAutoDivergeRecall
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	int ret=currentAction == WAVEFORM_AUTO_DIVERGE_THEN_RECALL;
	return(ret);
}

bool
InputTesterObj::
WaveformVideoSelect
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool select=currentAction == WAVEFORM_VIDEO_SELECT;
	return(select);
}

float
InputTesterObj::
WaveformVideoBrightness
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	if(currentAction == NOOP)
	{
		return(-1.0f);
	}
	else
	{
		float brightness = LGL_RandFloat(0.0f,1.0f);
		if(LGL_RandFloat(0.0f,1.0f) < 0.1f)
		{
			brightness=0.0f;
		}
		else if(LGL_RandFloat(0.0f,1.0f) < 0.1f)
		{
			brightness=-1.0f;
		}
		return(brightness);
	}
}

float
InputTesterObj::
WaveformVideoAdvanceRate
(
	unsigned int	target
)	const
{
	float rate=-1.0f;
	return(rate);
}

float
InputTesterObj::
WaveformFreqSenseBrightness
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	if(currentAction == NOOP)
	{
		return(-1.0f);
	}
	else
	{
		float brightness = LGL_RandFloat(0.0f,1.0f);
		if(LGL_RandFloat(0.0f,1.0f) < 0.1f)
		{
			brightness=0.0f;
		}
		else if(LGL_RandFloat(0.0f,1.0f) < 0.1f)
		{
			brightness=-1.0f;
		}
		return(brightness);
	}
}

bool
InputTesterObj::
WaveformAudioInputToggle
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool toggle = false;
	if(currentAction == WAVEFORM_AUDIO_INPUT_TOGGLE)
	{
		toggle = (LGL_RandInt(0,1)==1);
	}
	return(toggle);
}

bool
InputTesterObj::
WaveformVideoAspectRatioNext
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool next=false;
	if(currentAction == WAVEFORM_VIDEO_ASPECT_RATIO_NEXT)
	{
		next=true;
	}
	return(next);
}

float
InputTesterObj::
WaveformOscilloscopeBrightness
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	if(currentAction == NOOP)
	{
		return(-1.0f);
	}
	else
	{
		float brightness = LGL_RandFloat(0.0f,1.0f);
		if(LGL_RandFloat(0.0f,1.0f) < 0.1f)
		{
			brightness=0.0f;
		}
		else if(LGL_RandFloat(0.0f,1.0f) < 0.1f)
		{
			brightness=-1.0f;
		}
		return(brightness);
	}
}

bool
InputTesterObj::
WaveformSync
(
	unsigned int	target
)	const
{
	int currentAction = GetCurrentAction(target);
	bool sync=currentAction == WAVEFORM_SYNC;
	return(sync);
}

float
InputTesterObj::
WaveformPointerScratch
(
	unsigned int	target
)	const
{
	float targetX=-1.0f;
	return(targetX);
}

int
InputTesterObj::
GetCurrentAction
(
	unsigned int	target
)	const
{
	return((target & TARGET_BOTTOM) ? CurrentActionBottom : CurrentActionTop);
}

