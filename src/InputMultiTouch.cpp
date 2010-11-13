/*
 *
 * InputMultiTouch.cpp - Input abstraction object
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

#include "InputMultiTouch.h"

#include "InputMouse.h"

#define	KNOB_SCALAR	(4.0f)

InputMultiTouchObj&
GetInputMultiTouch()
{
	static InputMultiTouchObj inputMultiTouch;
	return(inputMultiTouch);
}

InputMultiTouchObj::
InputMultiTouchObj()
{
	//
}

InputMultiTouchObj::
~InputMultiTouchObj()
{
	//
}

//Core

void
InputMultiTouchObj::
NextFrame()
{
	//
}

//Global Input

bool
InputMultiTouchObj::
FocusChange()	const
{
	bool change=false;
	return(change);
}

bool
InputMultiTouchObj::
FocusBottom()	const
{
	bool bottom=false;
	return(bottom);
}

bool
InputMultiTouchObj::
FocusTop()	const
{
	bool top=false;
	return(top);
}

float
InputMultiTouchObj::
XfaderSpeakers()	const
{
	float xfade=-1.0f;
	return(xfade);
}

float
InputMultiTouchObj::
XfaderSpeakersDelta()	const
{
	float delta=0.0f;
	if(GetInputMouse().GetHoverTarget()==GUI_TARGET_XFADER_LEFT)
	{
		delta=LGL_MultiTouchDY()*KNOB_SCALAR;
	}
	return(delta);
}

float
InputMultiTouchObj::
XfaderHeadphones()	const
{
	float xfade=-1.0f;
	return(xfade);
}

float
InputMultiTouchObj::
XfaderHeadphonesDelta()	const
{
	float delta=0.0f;
	if(GetInputMouse().GetHoverTarget()==GUI_TARGET_XFADER_RIGHT)
	{
		delta=LGL_MultiTouchDY()*KNOB_SCALAR;
	}
	return(delta);
}

bool
InputMultiTouchObj::
SyncTopToBottom()	const
{
	bool sync=false;
	return(sync);
}

int
InputMultiTouchObj::
MasterToHeadphones()	const
{
	int to=-1;
	return(to);
}

bool
InputMultiTouchObj::
SyncBottomToTop()	const
{
	bool sync=false;
	return(sync);
}

//Mode 0: File Selection

float
InputMultiTouchObj::
FileScroll
(
	unsigned int	target
)	const
{
	float scroll=0.0f;
	if(target & TARGET_FOCUS)
	{
		if(GetInputMouse().GetHoverTarget()==GUI_TARGET_NULL)
		{
			if(LGL_MultiTouchFingerCount()>=2)
			{
				scroll=LGL_MultiTouchDY()*-250.0f;
			}
		}
	}
	return(scroll);
}

int
InputMultiTouchObj::
FileSelect
(
	unsigned int	target
)	const
{
	int choose=0;
	return(choose);
}

bool
InputMultiTouchObj::
FileMarkUnopened
(
	unsigned int	target
)	const
{
	bool mark=false;
	return(mark);
}

bool
InputMultiTouchObj::
FileRefresh
(
	unsigned int	target
)	const
{
	bool refresh=false;
	return(refresh);
}

//Mode 1: Decoding...

bool
InputMultiTouchObj::
DecodeAbort
(
	unsigned int	target
)	const
{
	bool abort=false;
	return(abort);
}

//Mode 2: Waveform

int
InputMultiTouchObj::
WaveformEject
(
	unsigned int	target
)	const
{
	int eject=0;
	return(eject);
}

bool
InputMultiTouchObj::
WaveformTogglePause
(
	unsigned int	target
)	const
{
	bool toggle=false;
	return(toggle);
}

float
InputMultiTouchObj::
WaveformNudge
(
	unsigned int	target
)	const
{
	float nudge=0.0f;
	if(target & TARGET_FOCUS)
	{
		if(GetInputMouse().GetHoverTarget()==GUI_TARGET_WAVEFORM)
		{
			if(LGL_MultiTouchFingerCount()>=2)
			{
				if(fabsf(LGL_MultiTouchDXTotal())>2*fabsf(LGL_MultiTouchDYTotal()))
				{
					nudge=LGL_MultiTouchDX()*10.0f;
				}
			}
		}
	}
	return(nudge);
}

float
InputMultiTouchObj::
WaveformPitchbend
(
	unsigned int	target
)	const
{
	float pitchbend=0.0f;
	return(pitchbend);
}

float
InputMultiTouchObj::
WaveformPitchbendDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	if(target & TARGET_FOCUS)
	{
		if(GetInputMouse().GetHoverTarget()==GUI_TARGET_WAVEFORM)
		{
			if(LGL_MultiTouchFingerCount()>=2)
			{
				if(fabsf(LGL_MultiTouchDYTotal())>2*fabsf(LGL_MultiTouchDXTotal()))
				{
					delta=LGL_MultiTouchDY()/60.0f;
				}
			}
		}
	}
	return(delta);
}

float
InputMultiTouchObj::
WaveformEQLow
(
	unsigned int	target
)	const
{
	float low=-1.0f;
	return(low);
}

float
InputMultiTouchObj::
WaveformEQLowDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	if(target && TARGET_FOCUS)
	{
		if(GetInputMouse().GetHoverTarget()==GUI_TARGET_EQ_LOW)
		{
			delta=LGL_MultiTouchDY()*KNOB_SCALAR;
		}
	}
	return(delta);
}

bool
InputMultiTouchObj::
WaveformEQLowKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputMultiTouchObj::
WaveformEQMid
(
	unsigned int	target
)	const
{
	float mid=-1.0f;
	return(mid);
}

float
InputMultiTouchObj::
WaveformEQMidDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	if(target && TARGET_FOCUS)
	{
		if(GetInputMouse().GetHoverTarget()==GUI_TARGET_EQ_MID)
		{
			delta=LGL_MultiTouchDY()*KNOB_SCALAR;
		}
	}
	return(delta);
}

bool
InputMultiTouchObj::
WaveformEQMidKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputMultiTouchObj::
WaveformEQHigh
(
	unsigned int	target
)	const
{
	float high=-1.0f;
	return(high);
}

float
InputMultiTouchObj::
WaveformEQHighDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	if(target && TARGET_FOCUS)
	{
		if(GetInputMouse().GetHoverTarget()==GUI_TARGET_EQ_HIGH)
		{
			delta=LGL_MultiTouchDY()*KNOB_SCALAR;
		}
	}
	return(delta);
}

bool
InputMultiTouchObj::
WaveformEQHighKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputMultiTouchObj::
WaveformGain
(
	unsigned int	target
)	const
{
	float gain=-1.0f;
	return(gain);
}

float
InputMultiTouchObj::
WaveformGainDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputMultiTouchObj::
WaveformGainKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputMultiTouchObj::
WaveformVolumeSlider
(
	unsigned int	target
)	const
{
	float volume=-1.0f;
	return(volume);
}

bool
InputMultiTouchObj::
WaveformVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputMultiTouchObj::
WaveformRapidVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputMultiTouchObj::
WaveformRapidSoloInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputMultiTouchObj::
WaveformVolumeSolo
(
	unsigned int	target
)	const
{
	bool solo=false;
	return(solo);
}

float
InputMultiTouchObj::
WaveformRewindFF
(
	unsigned int	target
)	const
{
	float rewindff=0.0f;
	return(rewindff);
}

bool
InputMultiTouchObj::
WaveformRecordHold
(
	unsigned int	target
)	const
{
	bool hold=false;
	if(target & TARGET_FOCUS)
	{
		if(LGL_KeyDown(LGL_KEY_SHIFT))
		{
			if(LGL_MultiTouchFingerCount()>=2)
			{
				hold=true;
			}
		}
	}
	return(hold);
}

float
InputMultiTouchObj::
WaveformRecordSpeed
(
	unsigned int	target
)	const
{
	float speed=0.0f;
	if(target & TARGET_FOCUS)
	{
		if(LGL_KeyDown(LGL_KEY_SHIFT))
		{
			if(LGL_MultiTouchFingerCount()>=2)
			{
				speed=LGL_MultiTouchDX()*50.0f;
			}
		}
	}
	return(speed);
}

bool
InputMultiTouchObj::
WaveformStutter
(
	unsigned int	target
)	const
{
	bool stutter=false;
	return(stutter);
}

float
InputMultiTouchObj::
WaveformStutterPitch
(
	unsigned int	target
)	const
{
	float pitch=-1.0f;
	return(pitch);
}

float
InputMultiTouchObj::
WaveformStutterSpeed
(
	unsigned int	target
)	const
{
	float speed=-1.0f;
	return(speed);
}

bool
InputMultiTouchObj::
WaveformSavePointPrev
(
	unsigned int	target
)	const
{
	bool prev=false;
	return(prev);
}

bool
InputMultiTouchObj::
WaveformSavePointNext
(
	unsigned int	target
)	const
{
	bool next=false;
	return(next);
}

int
InputMultiTouchObj::
WaveformSavePointPick
(
	unsigned int	target
)	const
{
	int pick=-9999;
	return(pick);
}

bool
InputMultiTouchObj::
WaveformSavePointSet
(
	unsigned int	target
)	const
{
	bool set=false;
	return(set);
}

float
InputMultiTouchObj::
WaveformSavePointUnsetPercent
(
	unsigned int	target
)	const
{
	float percent=0.0f;
	return(percent);
}

float
InputMultiTouchObj::
WaveformSavePointShift
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

float
InputMultiTouchObj::
WaveformSavePointShiftAll
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

bool
InputMultiTouchObj::
WaveformSavePointShiftAllHere
(
	unsigned int	target
)	const
{
	bool here=false;
	return(here);
}

bool
InputMultiTouchObj::
WaveformSavePointJumpNow
(
	unsigned int	target
)	const
{
	bool jump=false;
	return(jump);
}

bool
InputMultiTouchObj::
WaveformSavePointJumpAtMeasure
(
	unsigned int	target
)	const
{
	bool jump=false;
	return(jump);
}

int
InputMultiTouchObj::
WaveformLoopMeasuresExponent
(
	unsigned int	target
)	const
{
	int exponent=WAVEFORM_LOOP_MEASURES_EXPONENT_NULL;
	return(exponent);
}

bool
InputMultiTouchObj::
WaveformLoopMeasuresHalf
(
	unsigned int	target
)	const
{
	bool half=false;
	return(half);
}

bool
InputMultiTouchObj::
WaveformLoopMeasuresDouble
(
	unsigned int	target
)	const
{
	bool twoX=false;
	return(twoX);
}

bool
InputMultiTouchObj::
WaveformLoopSecondsLess
(
	unsigned int	target
)	const
{
	bool less=false;
	return(less);
}

bool
InputMultiTouchObj::
WaveformLoopSecondsMore
(
	unsigned int	target
)	const
{
	bool more=false;
	return(more);
}
bool
InputMultiTouchObj::
WaveformLoopAll
(
	unsigned int	target
)	const
{
	bool all=false;
	return(all);
}


bool
InputMultiTouchObj::
WaveformLoopToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;
	return(toggle);
}

bool
InputMultiTouchObj::
WaveformLoopThenRecallActive
(
	unsigned int	target
)	const
{
	bool active=false;
	return(active);
}

int
InputMultiTouchObj::
WaveformAutoDivergeRecall
(
	unsigned int	target
)	const
{
	int ret=0;
	return(ret);
}

bool
InputMultiTouchObj::
WaveformVideoSelect
(
	unsigned int	target
)	const
{
	bool select=false;
	return(select);
}

float
InputMultiTouchObj::
WaveformVideoBrightness
(
	unsigned int	target
)	const
{
	float bright=-1.0f;
	return(bright);
}

float
InputMultiTouchObj::
WaveformVideoBrightnessDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	if(target & TARGET_FOCUS)
	{
		if(GetInputMouse().GetHoverTarget()==GUI_TARGET_VIS_VIDEO)
		{
			delta=LGL_MultiTouchDY()*KNOB_SCALAR;
		}
	}
	return(delta);
}

float
InputMultiTouchObj::
WaveformVideoAdvanceRate
(
	unsigned int	target
)	const
{
	float rate=-1.0f;
	return(rate);
}

float
InputMultiTouchObj::
WaveformFreqSenseBrightness
(
	unsigned int	target
)	const
{
	float brightness=-1;
	return(brightness);
}

float
InputMultiTouchObj::
WaveformFreqSenseBrightnessDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	if(target & TARGET_FOCUS)
	{
		if(GetInputMouse().GetHoverTarget()==GUI_TARGET_VIS_FREQSENSE)
		{
			delta=LGL_MultiTouchDY()*KNOB_SCALAR;
		}
	}
	return(delta);
}

int
InputMultiTouchObj::
WaveformAudioInputMode
(
	unsigned int	target
)	const
{
	int mode=-1;
	return(mode);
}

bool
InputMultiTouchObj::
WaveformVideoAspectRatioNext
(
	unsigned int	target
)	const
{
	bool next=false;
	return(next);
}

float
InputMultiTouchObj::
WaveformOscilloscopeBrightness
(
	unsigned int	target
)	const
{
	float brightness=-1.0f;
	return(brightness);
}

float
InputMultiTouchObj::
WaveformOscilloscopeBrightnessDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	if(target & TARGET_FOCUS)
	{
		if(GetInputMouse().GetHoverTarget()==GUI_TARGET_VIS_OSCILLOSCOPE)
		{
			delta=LGL_MultiTouchDY()*KNOB_SCALAR;
		}
	}
	return(delta);
}

bool
InputMultiTouchObj::
WaveformSyncBPM
(
	unsigned int	target
)	const
{
	bool sync=false;
	return(sync);
}

float
InputMultiTouchObj::
WaveformPointerScratch
(
	unsigned int	target
)	const
{
	float targetX=-1.0f;
	return(targetX);
}

