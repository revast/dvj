/*
 *
 * InputXsession.cpp - Input abstraction object
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

#include "InputXsession.h"

InputXsessionObj::
InputXsessionObj()
{
	//
}

InputXsessionObj::
~InputXsessionObj()
{
	//
}

//Core

void
InputXsessionObj::
NextFrame()
{
	//
}

//Global Input

bool
InputXsessionObj::
FocusChange()	const
{
	bool change=false;
	return(change);
}

bool
InputXsessionObj::
FocusBottom()	const
{
	if(LGL_MidiDevice* xsession=LGL_GetXsession())
	{
		return
		(
			xsession->GetButtonStroke(LGL_XSESSION_BUTTON_RIGHT_HEADPHONES) ||
			xsession->GetButtonStroke(LGL_XSESSION_BUTTON_RIGHT_LEFT) ||
			xsession->GetButtonStroke(LGL_XSESSION_BUTTON_RIGHT_RIGHT) ||
			xsession->GetButtonStroke(LGL_XSESSION_BUTTON_RIGHT_EJECT) ||
			xsession->GetButtonStroke(LGL_XSESSION_BUTTON_RIGHT_TOGGLE_PAUSE) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_RIGHT_3) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_RIGHT_2) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_RIGHT_1) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_RIGHT_HIGH) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_RIGHT_MID) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_RIGHT_LOW) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_RIGHT_PITCHBEND) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_RIGHT_VOLUME)
		);
	}
	else
	{
		return(false);
	}
}

bool
InputXsessionObj::
FocusTop()	const
{
	if(LGL_MidiDevice* xsession=LGL_GetXsession())
	{
		return
		(
			xsession->GetButtonStroke(LGL_XSESSION_BUTTON_LEFT_HEADPHONES) ||
			xsession->GetButtonStroke(LGL_XSESSION_BUTTON_LEFT_LEFT) ||
			xsession->GetButtonStroke(LGL_XSESSION_BUTTON_LEFT_RIGHT) ||
			xsession->GetButtonStroke(LGL_XSESSION_BUTTON_LEFT_EJECT) ||
			xsession->GetButtonStroke(LGL_XSESSION_BUTTON_LEFT_TOGGLE_PAUSE) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_LEFT_3) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_LEFT_2) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_LEFT_1) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_LEFT_HIGH) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_LEFT_MID) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_LEFT_LOW) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_LEFT_PITCHBEND) ||
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_LEFT_VOLUME)
		);
	}
	else
	{
		return(false);
	}
}

float
InputXsessionObj::
XfaderSpeakers()	const
{
	float xfade=-1.0f;

	if(LGL_MidiDevice* xsession = LGL_GetXsession())
	{
		if(xsession->GetKnobTweak(LGL_XSESSION_KNOB_XFADER))
		{
			xfade=xsession->GetKnobStatus(LGL_XSESSION_KNOB_XFADER);
		}
	}

	return(xfade);
}

float
InputXsessionObj::
XfaderSpeakersDelta()	const
{
	float delta=0.0f;
	return(delta);
}

float
InputXsessionObj::
XfaderHeadphones()	const
{
	float xfade=-1.0f;

	if(LGL_MidiDevice* xsession = LGL_GetXsession())
	{
		if
		(
			xsession->GetButtonDown(LGL_XSESSION_BUTTON_LEFT_HEADPHONES) &&
			xsession->GetButtonDown(LGL_XSESSION_BUTTON_RIGHT_HEADPHONES)
		)
		{
			xfade=0.5f;
		}
		else if(xsession->GetButtonStroke(LGL_XSESSION_BUTTON_LEFT_HEADPHONES))
		{
			xfade=1.0f;
		}
		else if(xsession->GetButtonStroke(LGL_XSESSION_BUTTON_RIGHT_HEADPHONES))
		{
			xfade=0.0f;
		}
	}

	return(xfade);
}

float
InputXsessionObj::
XfaderHeadphonesDelta()	const
{
	float delta=0.0f;
	return(delta);
}

int
InputXsessionObj::
MasterToHeadphones()	const
{
	int to=-1;
	return(to);
}

//Mode 0: File Selection

float
InputXsessionObj::
FileScroll
(
	unsigned int	target
)	const
{
	float scroll=0.0f;
	if
	(
		!(target & TARGET_BOTTOM) &&
		!(target & TARGET_TOP)
	)
	{
		return(scroll);
	}
	else if(LGL_MidiDevice* xsession = LGL_GetXsession())
	{
		if(target & TARGET_TOP)
		{
			scroll-=xsession->GetButtonStroke(LGL_XSESSION_BUTTON_LEFT_LEFT);
			scroll+=xsession->GetButtonStroke(LGL_XSESSION_BUTTON_LEFT_RIGHT);
		}
		else if(target & TARGET_BOTTOM)
		{
			scroll-=xsession->GetButtonStroke(LGL_XSESSION_BUTTON_RIGHT_LEFT);
			scroll+=xsession->GetButtonStroke(LGL_XSESSION_BUTTON_RIGHT_RIGHT);
		}
	}

	return(scroll);
}

int
InputXsessionObj::
FileSelect
(
	unsigned int	target
)	const
{
	int choose=false;

	if(LGL_MidiDevice* xsession = LGL_GetXsession())
	{
		if(target & TARGET_TOP)
		{
			choose=(xsession->GetButtonStroke(LGL_XSESSION_BUTTON_LEFT_TOGGLE_PAUSE)) ? 1 : 0;
		}
		else if(target & TARGET_BOTTOM)
		{
			choose=(xsession->GetButtonStroke(LGL_XSESSION_BUTTON_RIGHT_TOGGLE_PAUSE)) ? 1 : 0;
		}
	}

	return(choose);
}

bool
InputXsessionObj::
FileMarkUnopened
(
	unsigned int	target
)	const
{
	bool mark=false;
	return(mark);
}

bool
InputXsessionObj::
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
InputXsessionObj::
WaveformEject
(
	unsigned int	target
)	const
{
	int eject=false;

	if(LGL_MidiDevice* xsession = LGL_GetXsession())
	{
		if(target & TARGET_TOP)
		{
			eject=(xsession->GetButtonDown(LGL_XSESSION_BUTTON_LEFT_EJECT)) ? 1 : 0;
		}
		else if(target & TARGET_BOTTOM)
		{
			eject=(xsession->GetButtonDown(LGL_XSESSION_BUTTON_RIGHT_EJECT)) ? 1 : 0;
		}
	}

	return(eject);
}

bool
InputXsessionObj::
WaveformPauseToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;
	
	if(LGL_MidiDevice* xsession = LGL_GetXsession())
	{
		if(target & TARGET_TOP)
		{
			toggle=xsession->GetButtonStroke(LGL_XSESSION_BUTTON_LEFT_TOGGLE_PAUSE);
		}
		else if(target & TARGET_BOTTOM)
		{
			toggle=xsession->GetButtonStroke(LGL_XSESSION_BUTTON_RIGHT_TOGGLE_PAUSE);
		}
	}

	return(toggle);
}

float
InputXsessionObj::
WaveformNudge
(
	unsigned int	target
)	const
{
	const float rate = 0.04f;

	float nudge=0.0f;

	if(LGL_MidiDevice* xsession = LGL_GetXsession())
	{
		if(target & TARGET_TOP)
		{
			nudge-=rate*xsession->GetButtonDown(LGL_XSESSION_BUTTON_LEFT_LEFT);
			nudge+=rate*xsession->GetButtonDown(LGL_XSESSION_BUTTON_LEFT_RIGHT);
		}
		else if(target & TARGET_BOTTOM)
		{
			nudge-=rate*xsession->GetButtonDown(LGL_XSESSION_BUTTON_RIGHT_LEFT);
			nudge+=rate*xsession->GetButtonDown(LGL_XSESSION_BUTTON_RIGHT_RIGHT);
		}
	}

	return(nudge);
}

float
InputXsessionObj::
WaveformPitchbend
(
	unsigned int	target
)	const
{
	const float RANGE=0.08f;

	float pitchbend=DVJ_INPUT_NIL;

	if(LGL_MidiDevice* xsession = LGL_GetXsession())
	{
		if
		(
			(target & TARGET_TOP) &&
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_LEFT_PITCHBEND)
		)
		{
			pitchbend = 1.0f+RANGE*-2.0f*(0.5f-xsession->GetKnobStatus(LGL_XSESSION_KNOB_LEFT_PITCHBEND));
		}
		else if
		(
			(target & TARGET_BOTTOM) &&
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_RIGHT_PITCHBEND)
		)
		{
			pitchbend = 1.0f+RANGE*-2.0f*(0.5f-xsession->GetKnobStatus(LGL_XSESSION_KNOB_RIGHT_PITCHBEND));
		}

		if(pitchbend!=DVJ_INPUT_NIL)
		{
			//Quantize a bit
			pitchbend = floorf(pitchbend*1000.0f)/1000.0f;
		}
	}

	return(pitchbend);
}

float
InputXsessionObj::
WaveformPitchbendDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

float
InputXsessionObj::
WaveformEQLow
(
	unsigned int	target
)	const
{
	float low=-1.0f;

	if(LGL_MidiDevice* xsession = LGL_GetXsession())
	{
		if
		(
			(target & TARGET_TOP) &&
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_LEFT_LOW)
		)
		{
			low = xsession->GetKnobStatus(LGL_XSESSION_KNOB_LEFT_LOW);
		}
		else if
		(
			(target & TARGET_BOTTOM) &&
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_RIGHT_LOW)
		)
		{
			low = xsession->GetKnobStatus(LGL_XSESSION_KNOB_RIGHT_LOW);
		}
	}

	return(low);
}

float
InputXsessionObj::
WaveformEQLowDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputXsessionObj::
WaveformEQLowKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputXsessionObj::
WaveformEQMid
(
	unsigned int	target
)	const
{
	float mid=-1.0f;

	if(LGL_MidiDevice* xsession = LGL_GetXsession())
	{
		if
		(
			(target & TARGET_TOP) &&
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_LEFT_MID)
		)
		{
			mid = xsession->GetKnobStatus(LGL_XSESSION_KNOB_LEFT_MID);
		}
		else if
		(
			(target & TARGET_BOTTOM) &&
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_RIGHT_MID)
		)
		{
			mid = xsession->GetKnobStatus(LGL_XSESSION_KNOB_RIGHT_MID);
		}
	}

	return(mid);
}

float
InputXsessionObj::
WaveformEQMidDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputXsessionObj::
WaveformEQMidKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputXsessionObj::
WaveformEQHigh
(
	unsigned int	target
)	const
{
	float high=-1.0f;

	if(LGL_MidiDevice* xsession = LGL_GetXsession())
	{
		if
		(
			(target & TARGET_TOP) &&
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_LEFT_HIGH)
		)
		{
			high = xsession->GetKnobStatus(LGL_XSESSION_KNOB_LEFT_HIGH);
		}
		else if
		(
			(target & TARGET_BOTTOM) &&
			xsession->GetKnobTweak(LGL_XSESSION_KNOB_RIGHT_HIGH)
		)
		{
			high = xsession->GetKnobStatus(LGL_XSESSION_KNOB_RIGHT_HIGH);
		}
	}

	return(high);
}

float
InputXsessionObj::
WaveformEQHighDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputXsessionObj::
WaveformEQHighKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputXsessionObj::
WaveformGain
(
	unsigned int	target
)	const
{
	float gain=-1.0f;
	return(gain);
}

float
InputXsessionObj::
WaveformGainDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputXsessionObj::
WaveformGainKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputXsessionObj::
WaveformVolumeSlider
(
	unsigned int	target
)	const
{
	float volume=-1.0f;
	
	if(LGL_MidiDevice* xsession = LGL_GetXsession())
	{
		if((target & TARGET_BOTTOM))
		{
			if(xsession->GetKnobTweak(LGL_XSESSION_KNOB_RIGHT_VOLUME))
			{
				volume=xsession->GetKnobStatus(LGL_XSESSION_KNOB_RIGHT_VOLUME);
			}
		}
		else if(target & TARGET_TOP)
		{
			if(xsession->GetKnobTweak(LGL_XSESSION_KNOB_LEFT_VOLUME))
			{
				volume=xsession->GetKnobStatus(LGL_XSESSION_KNOB_LEFT_VOLUME);
			}
		}
	}
	return(volume);
}

bool
InputXsessionObj::
WaveformVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputXsessionObj::
WaveformRhythmicVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputXsessionObj::
WaveformRhythmicVolumeInvertOther
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputXsessionObj::
WaveformVolumeSolo
(
	unsigned int	target
)	const
{
	bool solo=false;
	return(solo);
}

float
InputXsessionObj::
WaveformRewindFF
(
	unsigned int	target
)	const
{
	float rate = 32.0f;

	float rewindff=0.0f;

	if(LGL_MidiDevice* xsession = LGL_GetXsession())
	{
		if(target & TARGET_TOP)
		{
			if(xsession->GetButtonDown(LGL_XSESSION_BUTTON_LEFT_TOGGLE_PAUSE))
			{
				if(xsession->GetButtonDown(LGL_XSESSION_BUTTON_LEFT_LEFT))
				{
					rewindff-=rate;
				}
				if(xsession->GetButtonDown(LGL_XSESSION_BUTTON_LEFT_RIGHT))
				{
					rewindff+=rate;
				}
			}
		}
		if(target & TARGET_BOTTOM)
		{
			if(xsession->GetButtonDown(LGL_XSESSION_BUTTON_RIGHT_TOGGLE_PAUSE))
			{
				if(xsession->GetButtonDown(LGL_XSESSION_BUTTON_RIGHT_LEFT))
				{
					rewindff-=rate;
				}
				if(xsession->GetButtonDown(LGL_XSESSION_BUTTON_RIGHT_RIGHT))
				{
					rewindff+=rate;
				}
			}
		}
	}

	return(rewindff);
}

bool
InputXsessionObj::
WaveformRecordHold
(
	unsigned int	target
)	const
{
	bool hold=false;
	return(hold);
}

float
InputXsessionObj::
WaveformRecordSpeed
(
	unsigned int	target
)	const
{
	float speed=0.0f;
	return(speed);
}

bool
InputXsessionObj::
WaveformStutter
(
	unsigned int	target
)	const
{
	bool stutter=false;
	return(stutter);
}

float
InputXsessionObj::
WaveformStutterPitch
(
	unsigned int	target
)	const
{
	float pitch=-1.0f;
	return(pitch);
}

float
InputXsessionObj::
WaveformStutterSpeed
(
	unsigned int	target
)	const
{
	float speed=-1.0f;
	return(speed);
}

bool
InputXsessionObj::
WaveformSavepointPrev
(
	unsigned int	target
)	const
{
	bool prev=false;
	return(prev);
}

bool
InputXsessionObj::
WaveformSavepointNext
(
	unsigned int	target
)	const
{
	bool next=false;
	return(next);
}

int
InputXsessionObj::
WaveformSavepointPick
(
	unsigned int	target
)	const
{
	int pick=-9999;
	return(pick);
}

bool
InputXsessionObj::
WaveformSavepointSet
(
	unsigned int	target
)	const
{
	bool set=false;
	return(set);
}

float
InputXsessionObj::
WaveformSavepointUnsetPercent
(
	unsigned int	target
)	const
{
	float percent=0.0f;
	return(percent);
}

float
InputXsessionObj::
WaveformSavepointShift
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

float
InputXsessionObj::
WaveformSavepointShiftAll
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

bool
InputXsessionObj::
WaveformSavepointShiftAllHere
(
	unsigned int	target
)	const
{
	bool here=false;
	return(here);
}

bool
InputXsessionObj::
WaveformSavepointJumpNow
(
	unsigned int	target
)	const
{
	bool jump=false;
	return(jump);
}

bool
InputXsessionObj::
WaveformSavepointJumpAtMeasure
(
	unsigned int	target
)	const
{
	bool jump=false;
	return(jump);
}

int
InputXsessionObj::
WaveformLoopMeasuresExponent
(
	unsigned int	target
)	const
{
	int exponent=WAVEFORM_LOOP_MEASURES_EXPONENT_NULL;
	return(exponent);
}

bool
InputXsessionObj::
WaveformQuantizationPeriodHalf
(
	unsigned int	target
)	const
{
	bool half=false;
	return(half);
}

bool
InputXsessionObj::
WaveformQuantizationPeriodDouble
(
	unsigned int	target
)	const
{
	bool twoX=false;
	return(twoX);
}

bool
InputXsessionObj::
WaveformLoopSecondsLess
(
	unsigned int	target
)	const
{
	bool less=false;
	return(less);
}

bool
InputXsessionObj::
WaveformLoopSecondsMore
(
	unsigned int	target
)	const
{
	bool more=false;
	return(more);
}

bool
InputXsessionObj::
WaveformLoopAll
(
	unsigned int	target
)	const
{
	bool all=false;
	return(all);
}

bool
InputXsessionObj::
WaveformLoopToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;
	return(toggle);
}

bool
InputXsessionObj::
WaveformLoopThenRecallActive
(
	unsigned int	target
)	const
{
	bool active=false;
	return(active);
}

int
InputXsessionObj::
WaveformAutoDivergeRecall
(
	unsigned int	target
)	const
{
	int ret=0;
	return(ret);
}

bool
InputXsessionObj::
WaveformVideoSelect
(
	unsigned int	target
)	const
{
	bool select=false;
	return(select);
}

float
InputXsessionObj::
WaveformVideoBrightness
(
	unsigned int	target
)	const
{
	float bright=-1.0f;
	return(bright);
}

float
InputXsessionObj::
WaveformVideoAdvanceRate
(
	unsigned int	target
)	const
{
	float rate=-1.0f;
	return(rate);
}

float
InputXsessionObj::
WaveformFreqSenseBrightness
(
	unsigned int	target
)	const
{
	float brightness=-1;
	return(brightness);
}

bool
InputXsessionObj::
WaveformAudioInputToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;
	return(toggle);
}

bool
InputXsessionObj::
WaveformVideoAspectRatioNext
(
	unsigned int	target
)	const
{
	bool next=false;
	return(next);
}

float
InputXsessionObj::
WaveformOscilloscopeBrightness
(
	unsigned int	target
)	const
{
	return(-1.0f);
}

bool
InputXsessionObj::
WaveformSync
(
	unsigned int	target
)	const
{
	bool sync=false;
	return(sync);
}

float
InputXsessionObj::
WaveformPointerScratch
(
	unsigned int	target
)	const
{
	float targetX=-1.0f;
	return(targetX);
}

