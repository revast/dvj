/*
 *
 * InputNull.cpp - Input abstraction object
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

#include "InputWiimote.h"

InputWiimoteObj::
InputWiimoteObj()
{
	//
}

InputWiimoteObj::
~InputWiimoteObj()
{
	//
}

//Core

void
InputWiimoteObj::
NextFrame()
{
	if(LGL_GetWiimote(0).ButtonStroke(LGL_WIIMOTE_B))
	{
		LGL_Vector acc=LGL_GetWiimote(0).GetAccelRaw();
		ScrollInitialAccel=acc.GetY();
	}

	if
	(
		LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_A) ||
		LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_B)
	)
	{
		if(LGL_GetWiimote(0).GetPointerAvailable())
		{
			LastKnownPointerScratchX=LGL_Clamp
			(
				0,
				LGL_GetWiimote(0).GetPointerX(),
				1
			);
		}
	}
	else
	{
		LastKnownPointerScratchX=0.5f;
	}
	if(LGL_GetWiimote(0).ButtonStroke(LGL_WIIMOTE_HOME))
	{
		HomeDownTimer.Reset();
	}
}

//Global Input

bool
InputWiimoteObj::
FocusChange()	const
{
	bool change=false;
	if(LGL_GetWiimote(0).ButtonStroke(LGL_WIIMOTE_2))
	{
		change=true;
	}
	return(change);
}

bool
InputWiimoteObj::
FocusBottom()	const
{
	bool bottom=false;
	return(bottom);
}

bool
InputWiimoteObj::
FocusTop()	const
{
	bool top=false;
	return(top);
}

float
InputWiimoteObj::
XfaderSpeakers()	const
{
	float xfade=-1.0f;
	LGL_Wiimote& wiimote = LGL_GetWiimote(0);
	if(wiimote.ButtonDown(LGL_WIIMOTE_MINUS))
	{
		if(wiimote.GetPointerAvailable())
		{
			xfade=LGL_Clamp
			(
				0,
				2*wiimote.GetPointerY(),
				1
			);
		}
		else
		{
			printf("Unavailable!\n");
		}
	}
	return(xfade);
}

float
InputWiimoteObj::
XfaderSpeakersDelta()	const
{
	float delta=0.0f;
	return(delta);
}

float
InputWiimoteObj::
XfaderHeadphones()	const
{
	float xfade=-1.0f;
	LGL_Wiimote& wiimote = LGL_GetWiimote(0);
	if(wiimote.ButtonDown(LGL_WIIMOTE_PLUS))
	{
		if(wiimote.GetPointerAvailable())
		{
			xfade=LGL_Clamp
			(
				0,
				2*wiimote.GetPointerY(),
				1
			);
		}
		else
		{
			printf("Unavailable!\n");
		}
	}
	return(xfade);
}

float
InputWiimoteObj::
XfaderHeadphonesDelta()	const
{
	float delta=0.0f;
	return(delta);
}

int
InputWiimoteObj::
MasterToHeadphones()	const
{
	int to=-1;
	return(to);
}

//Mode 0: File Selection

float
InputWiimoteObj::
FileScroll
(
	unsigned int	target
)	const
{
	float scroll=0.0f;

	if(target & TARGET_FOCUS)
	{
		LGL_Wiimote& wiimote = LGL_GetWiimote(0);

		//D-Pad
		if(wiimote.ButtonStroke(LGL_WIIMOTE_DOWN))
		{
			scroll++;
		}
		if(wiimote.ButtonStroke(LGL_WIIMOTE_UP))
		{
			scroll--;
		}

		//Accelerometer
		if(wiimote.ButtonDown(LGL_WIIMOTE_B))
		{
			LGL_Vector acc = wiimote.GetAccelRaw();
			float diff = acc.GetY()-ScrollInitialAccel;
			scroll+=LGL_SecondsSinceLastFrame()*LGL_Sign(diff)*powf(2.0f,10*fabsf(diff));
		}
	}

	return(scroll);
}

int
InputWiimoteObj::
FileSelect
(
	unsigned int	target
)	const
{
	int choose=0;

	if(target & TARGET_FOCUS)
	{
		if(LGL_GetWiimote(0).ButtonStroke(LGL_WIIMOTE_A))
		{
			choose=1;
		}
	}

	return(choose);
}

bool
InputWiimoteObj::
FileMarkUnopened
(
	unsigned int	target
)	const
{
	bool mark=false;
	return(mark);
}

bool
InputWiimoteObj::
FileRefresh
(
	unsigned int	target
)	const
{
	bool refresh=false;
	return(refresh);
}



//Mode 2: Waveform

int
InputWiimoteObj::
WaveformEject
(
	unsigned int	target
)	const
{
	int eject=0;
	if(target & TARGET_FOCUS)
	{
		if(LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_HOME))
		{
			eject=(HomeDownTimer.SecondsSinceLastReset()>0.5f) ? 1 : 0;
		}
	}
	return(eject);
}

bool
InputWiimoteObj::
WaveformPauseToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;
	
	if(target & TARGET_FOCUS)
	{
		if(LGL_GetWiimote(0).ButtonStroke(LGL_WIIMOTE_HOME))
		{
			toggle=true;
		}
	}

	return(toggle);
}

float
InputWiimoteObj::
WaveformNudge
(
	unsigned int	target
)	const
{
	float nudge=0.0f;
	if(target & TARGET_FOCUS)
	{
		if(LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_LEFT))
		{
			nudge-=4;
		}
		if(LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_RIGHT))
		{
			nudge+=4;
		}
	}
	return(nudge);
}

float
InputWiimoteObj::
WaveformPitchbend
(
	unsigned int	target
)	const
{
	float pitchbend=0.0f;
	bool inputReceived=false;
	if(target & TARGET_FOCUS)
	{
		if(LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_UP))
		{
			pitchbend+=LGL_SecondsSinceLastFrame();
			inputReceived=true;
		}
		if(LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_DOWN))
		{
			pitchbend-=LGL_SecondsSinceLastFrame();
			inputReceived=true;
		}
	}
	if(inputReceived==false)
	{
		pitchbend=DVJ_INPUT_NIL;
	}
	return(pitchbend);
}

float
InputWiimoteObj::
WaveformPitchbendDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

float
InputWiimoteObj::
WaveformEQLow
(
	unsigned int	target
)	const
{
	float low=-1.0f;
	return(low);
}

float
InputWiimoteObj::
WaveformEQLowDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputWiimoteObj::
WaveformEQLowKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputWiimoteObj::
WaveformEQMid
(
	unsigned int	target
)	const
{
	float mid=-1.0f;
	return(mid);
}

float
InputWiimoteObj::
WaveformEQMidDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputWiimoteObj::
WaveformEQMidKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputWiimoteObj::
WaveformEQHigh
(
	unsigned int	target
)	const
{
	float high=-1.0f;
	return(high);
}

float
InputWiimoteObj::
WaveformEQHighDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputWiimoteObj::
WaveformEQHighKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputWiimoteObj::
WaveformGain
(
	unsigned int	target
)	const
{
	float gain=-1.0f;
	return(gain);
}

float
InputWiimoteObj::
WaveformGainDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputWiimoteObj::
WaveformGainKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputWiimoteObj::
WaveformVolumeSlider
(
	unsigned int	target
)	const
{
	float volume=-1.0f;
	return(volume);
}

bool
InputWiimoteObj::
WaveformVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	/*
	if(target & TARGET_FOCUS)
	{
		if(LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_B))
		{
			invert=true;
		}
	}
	*/
	return(invert);
}

bool
InputWiimoteObj::
WaveformRhythmicVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputWiimoteObj::
WaveformRhythmicVolumeInvertOther
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputWiimoteObj::
WaveformVolumeSolo
(
	unsigned int	target
)	const
{
	bool solo=false;
	return(solo);
}

float
InputWiimoteObj::
WaveformRewindFF
(
	unsigned int	target
)	const
{
	float rewindff=0.0f;
	if(target & TARGET_FOCUS)
	{
		if(LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_B))
		{
			float val=16.0f*(LastKnownPointerScratchX-0.5f);
			rewindff=(powf(2.0f,fabsf(val))-1.0f)*LGL_Sign(val);
		}
	}
	return(rewindff);
}

bool
InputWiimoteObj::
WaveformRecordHold
(
	unsigned int	target
)	const
{
	bool hold=false;
	return(hold);
}

float
InputWiimoteObj::
WaveformRecordSpeed
(
	unsigned int	target
)	const
{
	float speed=0.0f;
	return(speed);
}

bool
InputWiimoteObj::
WaveformStutter
(
	unsigned int	target
)	const
{
	bool stutter=false;
	return(stutter);
}

float
InputWiimoteObj::
WaveformStutterPitch
(
	unsigned int	target
)	const
{
	float pitch=-1.0f;
	return(pitch);
}

float
InputWiimoteObj::
WaveformStutterSpeed
(
	unsigned int	target
)	const
{
	float speed=-1.0f;
	return(speed);
}

bool
InputWiimoteObj::
WaveformSavepointPrev
(
	unsigned int	target
)	const
{
	bool prev=false;
	return(prev);
}

bool
InputWiimoteObj::
WaveformSavepointNext
(
	unsigned int	target
)	const
{
	bool next=false;
	return(next);
}

int
InputWiimoteObj::
WaveformSavepointPick
(
	unsigned int	target
)	const
{
	int pick=-9999;
	return(pick);
}

bool
InputWiimoteObj::
WaveformSavepointSet
(
	unsigned int	target
)	const
{
	bool set=false;
	return(set);
}

float
InputWiimoteObj::
WaveformSavepointUnsetPercent
(
	unsigned int	target
)	const
{
	float percent=0.0f;
	return(percent);
}

float
InputWiimoteObj::
WaveformSavepointShift
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

float
InputWiimoteObj::
WaveformSavepointShiftAll
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

bool
InputWiimoteObj::
WaveformSavepointShiftAllHere
(
	unsigned int	target
)	const
{
	bool here=false;
	return(here);
}

bool
InputWiimoteObj::
WaveformSavepointJumpNow
(
	unsigned int	target
)	const
{
	bool jump=false;
	return(jump);
}

bool
InputWiimoteObj::
WaveformSavepointJumpAtMeasure
(
	unsigned int	target
)	const
{
	bool jump=false;
	return(jump);
}

int
InputWiimoteObj::
WaveformLoopMeasuresExponent
(
	unsigned int	target
)	const
{
	int exponent=WAVEFORM_LOOP_MEASURES_EXPONENT_NULL;
	return(exponent);
}

bool
InputWiimoteObj::
WaveformQuantizationPeriodHalf
(
	unsigned int	target
)	const
{
	bool half=false;
	return(half);
}

bool
InputWiimoteObj::
WaveformQuantizationPeriodDouble
(
	unsigned int	target
)	const
{
	bool twoX=false;
	return(twoX);
}

bool
InputWiimoteObj::
WaveformLoopSecondsLess
(
	unsigned int	target
)	const
{
	bool less=false;
	return(less);
}

bool
InputWiimoteObj::
WaveformLoopSecondsMore
(
	unsigned int	target
)	const
{
	bool more=false;
	return(more);
}

bool
InputWiimoteObj::
WaveformLoopAll
(
	unsigned int	target
)	const
{
	bool all=false;
	return(all);
}

bool
InputWiimoteObj::
WaveformLoopToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;
	return(toggle);
}

bool
InputWiimoteObj::
WaveformLoopThenRecallActive
(
	unsigned int	target
)	const
{
	bool active=false;
	return(active);
}

int
InputWiimoteObj::
WaveformAutoDivergeRecall
(
	unsigned int	target
)	const
{
	int ret=0;
	return(ret);
}

bool
InputWiimoteObj::
WaveformVideoSelect
(
	unsigned int	target
)	const
{
	bool select=false;
	return(select);
}

float
InputWiimoteObj::
WaveformVideoBrightness
(
	unsigned int	target
)	const
{
	float bright=-1.0f;
	return(bright);
}

float
InputWiimoteObj::
WaveformVideoAdvanceRate
(
	unsigned int	target
)	const
{
	float rate=-1.0f;
	return(rate);
}

float
InputWiimoteObj::
WaveformFreqSenseBrightness
(
	unsigned int	target
)	const
{
	float brightness=-1.0f;
	return(brightness);
}

bool
InputWiimoteObj::
WaveformAudioInputToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;
	return(toggle);
}

bool
InputWiimoteObj::
WaveformVideoAspectRatioNext
(
	unsigned int	target
)	const
{
	bool next=false;
	return(next);
}

float
InputWiimoteObj::
WaveformOscilloscopeBrightness
(
	unsigned int	target
)	const
{
	return(-1.0f);
}

bool
InputWiimoteObj::
WaveformSync
(
	unsigned int	target
)	const
{
	bool sync=false;

	if(target & TARGET_FOCUS)
	{
		if(LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_1))
		{
			sync=true;
		}
	}

	return(sync);
}

float
InputWiimoteObj::
WaveformPointerScratch
(
	unsigned int	target
)	const
{
	float targetX=-1.0f;

	if(target & TARGET_FOCUS)
	{
		if(LGL_GetWiimote(0).ButtonDown(LGL_WIIMOTE_A))
		{
			targetX=LastKnownPointerScratchX;
		}
	}

	return(targetX);
}

