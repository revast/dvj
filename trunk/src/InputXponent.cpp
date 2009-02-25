/*
 *
 * InputXponent.h
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

#include "InputXponent.h"

#include "LGL.module/LGL.h"

//Core

void
InputXponentObj::
NextFrame()
{
	if(LGL_GetXponent())
	{
		if
		(
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOCK) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_PREV) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_NEXT) ||
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_MINUS) ||
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_PLUS)
		)
		{
			WaveformSavePointUnsetTimerRight.Reset();
		}

		if
		(
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOCK) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_PREV) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_NEXT) ||
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_MINUS) ||
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_PLUS)
		)
		{
			WaveformSavePointUnsetTimerLeft.Reset();
		}
	}
}

//Global Input

bool
InputXponentObj::
FocusChange()		const
{
	return(false);
}

bool
InputXponentObj::
FocusBottom()		const
{
	if(LGL_GetXponent()==NULL)
	{
		return(false);
	}
	else
	{
		return
		(
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_RECORD) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_FINGER) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_HEADPHONES) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_X) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_DASH) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LEFT) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_RIGHT) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_MOD_POWER_1) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_MOD_POWER_2) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_MOD_POWER_3) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_MOD_POWER_4) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_REWIND) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_FAST_FORWARD) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_POINT_1) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_POINT_2) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_POINT_3) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_POINT_4) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_POINT_5) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_PREV) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_NEXT) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOCK) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_PLUS) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_MINUS) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_EJECT) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_TOGGLE_PAUSE) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_1) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_2) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_4) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_8) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_CYCLE) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_IN) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_OUT) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_UP) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_GAIN) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_HIGH) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_MID) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOW) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_SYNC) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_VOLUME) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_RECORD) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_MOD_1) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_MOD_2) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_MOD_3) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_MOD_4) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_PITCHBEND) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_GAIN) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_HIGH) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_MID) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_LOW) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_VOLUME) 
		);
	}
}

bool
InputXponentObj::
FocusTop()		const
{
	if(LGL_GetXponent()==NULL)
	{
		return(false);
	}
	else
	{
		return
		(
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_RECORD) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_FINGER) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_HEADPHONES) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_X) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_DASH) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LEFT) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_RIGHT) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_MOD_POWER_1) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_MOD_POWER_2) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_MOD_POWER_3) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_MOD_POWER_4) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_REWIND) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_FAST_FORWARD) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_POINT_1) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_POINT_2) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_POINT_3) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_POINT_4) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_POINT_5) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_PREV) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_NEXT) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOCK) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_PLUS) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_MINUS) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_EJECT) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_TOGGLE_PAUSE) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_1) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_2) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_4) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_8) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_CYCLE) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_IN) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_OUT) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_UP) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_GAIN) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_HIGH) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_MID) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOW) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_SYNC) ||
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_VOLUME) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_RECORD) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_MOD_1) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_MOD_2) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_MOD_3) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_MOD_4) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_PITCHBEND) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_GAIN) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_HIGH) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_MID) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_LOW) ||
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_VOLUME)
		);
	}
}

float
InputXponentObj::
XfaderSpeakers()	const
{
	float xfade=-1.0f;

	if(LGL_GetXponent())
	{
		if(LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_XFADER))
		{
			xfade=1.0f-LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_XFADER);
		}
	}

	return(xfade);
}
float
InputXponentObj::
XfaderSpeakersDelta()	const
{
	return(0.0f);
}

float
InputXponentObj::
XfaderHeadphones()	const
{
	float xfade=-1.0f;

	if(LGL_GetXponent())
	{
		if(LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_CUE))
		{
			xfade=1.0f-LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_CUE);
		}
	}

	return(xfade);
}

float
InputXponentObj::
XfaderHeadphonesDelta()	const
{
	return(0.0f);
}

bool
InputXponentObj::
SyncTopToBottom()	const
{
	if(LGL_GetXponent())
	{
		if(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_SYNC))
		{
			return(true);
		}
	}

	return(false);
}

bool
InputXponentObj::
SyncBottomToTop()	const
{
	if(LGL_GetXponent())
	{
		if(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_SYNC))
		{
			return(true);
		}
	}

	return(false);
}

//Mode 0: File Selection

float
InputXponentObj::
FileScroll
(
	unsigned int	target
)	const
{
	float scroll = 0.0f;

	if
	(
		!(target & TARGET_BOTTOM) &&
		!(target & TARGET_TOP)
	)
	{
		return(scroll);
	}

	if(LGL_GetXponent())
	{
		//Record-spin scrolling
		{
			float status = LGL_GetXponent()->GetKnobStatus
			(
				(target & TARGET_TOP) ?
				LGL_XPONENT_KNOB_LEFT_RECORD :
				LGL_XPONENT_KNOB_RIGHT_RECORD
			);
			bool tweak = LGL_GetXponent()->GetKnobTweak
			(
				(target & TARGET_TOP) ?
				LGL_XPONENT_KNOB_LEFT_RECORD :
				LGL_XPONENT_KNOB_RIGHT_RECORD
			);
			bool touchDown = LGL_GetXponent()->GetButtonDown
			(
				(target & TARGET_TOP) ?
				LGL_XPONENT_BUTTON_LEFT_RECORD :
				LGL_XPONENT_BUTTON_RIGHT_RECORD
			);
			if
			(
				status!=-1.0f &&
				(
					tweak ||
					touchDown ||
					(
						status!=63.0f/127.0f &&
						status!=65.0f/127.0f
					)
				)
			)
			{
				float magnitude = (1.0f-(2.0f*(status*127.0f/128.0f)));
				magnitude = LGL_Sign(magnitude) * powf(magnitude,2.0f);
				scroll+=LGL_SecondsSinceLastFrame()*60.0f*-500*magnitude;
			}
		}

		//Unitary button scrolling
		{
			if(target & TARGET_BOTTOM)
			{
				scroll+=
				(
					(
						LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_IN) ||
						LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_RIGHT) ||
						LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_NEXT)
					) ? 1 : 0
				);
				scroll+=
				(
					(
						LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_OUT) ||
						LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LEFT) ||
						LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_PREV)
					) ? -1 : 0
				);
			}
			else if(target & TARGET_TOP)
			{
				scroll+=
				(
					(
						LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_IN) ||
						LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_RIGHT) ||
						LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_NEXT)
					) ? 1 : 0
				);
				scroll+=
				(
					(
						LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_OUT) ||
						LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LEFT) ||
						LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_PREV)
					) ? -1 : 0
				);
			}
		}
	}

	return(scroll);
}

bool
InputXponentObj::
FileSelect
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent()==NULL)
	{
		return(false);
	}
	else
	{
		if(target & TARGET_BOTTOM)
		{
			return
			(
				LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_TOGGLE_PAUSE) ||
				LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_DASH)
			);
		}
		else if(target & TARGET_TOP)
		{
			return
			(
				LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_TOGGLE_PAUSE) ||
				LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_DASH)
			);
		}
		else
		{
			return(false);
		}
	}
}

bool
InputXponentObj::
FileRefresh
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent()==NULL)
	{
		return(false);
	}
	else
	{
		if(target & TARGET_BOTTOM)
		{
			return(LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_CYCLE));
		}
		else if(target & TARGET_TOP)
		{
			return(LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_CYCLE));
		}
		else
		{
			return(false);
		}
	}
}

//Mode 1: Decoding...

bool
InputXponentObj::
DecodeAbort
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent())
	{
		if(target & TARGET_BOTTOM)
		{
			return(LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_EJECT));
		}
		else if(target & TARGET_TOP)
		{
			return(LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_EJECT));
		}
	}

	return(false);
}

//Mode 2: Waveform

bool
InputXponentObj::
WaveformEject
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent())
	{
		if(target & TARGET_BOTTOM)
		{
			return(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_EJECT));
		}
		else if(target & TARGET_TOP)
		{
			return(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_EJECT));
		}
	}

	return(false);
}

bool
InputXponentObj::
WaveformTogglePause
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent())
	{
		if(target & TARGET_BOTTOM)
		{
			return(LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_TOGGLE_PAUSE));
		}
		else if(target & TARGET_TOP)
		{
			return(LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_TOGGLE_PAUSE));
		}
	}

	return(false);
}

float
InputXponentObj::
WaveformNudge
(
	unsigned int	target
)	const
{
	const float RATE = 0.04f;

	float nudge=0.0f;

	if(LGL_GetXponent())
	{
		if(target & TARGET_BOTTOM)
		{
			nudge-=RATE*(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_LEFT));
			nudge+=RATE*(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_RIGHT));
		}
		else if(target & TARGET_TOP)
		{
			nudge-=RATE*(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_LEFT));
			nudge+=RATE*(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_RIGHT));
		}
	}

	return(nudge);
}

float
InputXponentObj::
WaveformPitchbend
(
	unsigned int	target
)	const
{
	const float RANGE=0.08f;

	float pitchbend=0.0f;

	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_PITCHBEND)
		)
		{
			pitchbend = 1.0f+RANGE*-2.0f*(0.5f-LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_RIGHT_PITCHBEND));
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_PITCHBEND)
		)
		{
			pitchbend = 1.0f+RANGE*-2.0f*(0.5f-LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_LEFT_PITCHBEND));
		}
	}

	//Quantize a bit
	pitchbend = floorf(pitchbend*1000.0f)/1000.0f;

	return(pitchbend);
}

float
InputXponentObj::
WaveformPitchbendDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	if(LGL_GetXponent())
	{
		unsigned int which=0;
		if(target & TARGET_BOTTOM)
		{
			which=1;
		}
		else if(target & TARGET_TOP)
		{
			which=0;
		}

		if
		(
			LGL_GetXponent()->GetButtonDown
			(
				(which==0) ?
				LGL_XPONENT_BUTTON_LEFT_MOD_POWER_4 :
				LGL_XPONENT_BUTTON_RIGHT_MOD_POWER_4
			) &&
			LGL_GetXponent()->GetKnobStatus
			(
				(which==0) ?
				LGL_XPONENT_KNOB_LEFT_MOD_4 :
				LGL_XPONENT_KNOB_RIGHT_MOD_4
			)!=-1.0f
		)
		{
			float dir = 0.0f;
			float status = LGL_GetXponent()->GetKnobStatus
			(
				(which==0) ?
				LGL_XPONENT_KNOB_LEFT_MOD_4 :
				LGL_XPONENT_KNOB_RIGHT_MOD_4
			);
			if(status>0.55f)
			{
				dir=powf((status-0.55f)/0.45f,3.0f);
			}
			else if(status<0.45f)
			{
				dir=-powf((0.45f-status)/0.45f,3.0f);
			}
			
			float factor=0.05f*dir;
			delta=factor*LGL_SecondsSinceLastFrame();
		}
	}

	return(delta);
}

float
InputXponentObj::
WaveformEQLow
(
	unsigned int	target
)	const
{
	float low=-1.0f;

	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_LOW)
		)
		{
			low = LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_RIGHT_LOW);
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_LOW)
		)
		{
			low = LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_LEFT_LOW);
		}
	}
	return(low);
}

float
InputXponentObj::
WaveformEQLowDelta
(
	unsigned int	target
)	const	
{
	return(0.0f);
}

bool
InputXponentObj::
WaveformEQLowKill
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_LOW)
		)
		{
			return(true);
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_LOW)
		)
		{
			return(true);
		}
	}

	return(false);
}

float
InputXponentObj::
WaveformEQMid
(
	unsigned int	target
)	const
{
	float mid=-1.0f;

	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_MID)
		)
		{
			mid = LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_RIGHT_MID);
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_MID)
		)
		{
			mid = LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_LEFT_MID);
		}
	}

	return(mid);
}

float
InputXponentObj::
WaveformEQMidDelta
(
	unsigned int	target
)	const	
{
	return(0.0f);
}

bool
InputXponentObj::
WaveformEQMidKill
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_MID)
		)
		{
			return(true);
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_MID)
		)
		{
			return(true);
		}
	}

	return(false);
}

float
InputXponentObj::
WaveformEQHigh
(
	unsigned int	target
)	const
{
	float high=-1.0f;

	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_HIGH)
		)
		{
			high = LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_RIGHT_HIGH);
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_HIGH)
		)
		{
			high = LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_LEFT_HIGH);
		}
	}

	return(high);
}

float
InputXponentObj::
WaveformEQHighDelta
(
	unsigned int	target
)	const	
{
	return(0.0f);
}

bool
InputXponentObj::
WaveformEQHighKill
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_HIGH)
		)
		{
			return(true);
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_HIGH)
		)
		{
			return(true);
		}
	}

	return(false);
}

float
InputXponentObj::
WaveformGain
(
	unsigned int	target
)	const
{
	float gain=-1.0f;

	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_GAIN)
		)
		{
			gain = LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_RIGHT_GAIN)*2;
			gain*=gain*gain*gain;
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_GAIN)
		)
		{
			gain = LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_LEFT_GAIN)*2;
			gain*=gain*gain*gain;
		}
	}

	return(gain);
}

float
InputXponentObj::
WaveformGainDelta
(
	unsigned int	target
)	const	
{
	return(0.0f);
}

bool
InputXponentObj::
WaveformGainKill
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_GAIN)
		)
		{
			return(true);
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_GAIN)
		)
		{
			return(true);
		}
	}

	return(false);
}

float
InputXponentObj::
WaveformVolumeSlider
(
	unsigned int	target
)	const
{
	float volume=-1.0f;

	if(LGL_GetXponent())
	{
		if((target & TARGET_BOTTOM))
		{
			if(LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_BUTTON_RIGHT_VOLUME))
			{
				volume=LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_BUTTON_RIGHT_VOLUME);
			}
		}
		else if(target & TARGET_TOP)
		{
			if(LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_BUTTON_LEFT_VOLUME))
			{
				volume=LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_BUTTON_LEFT_VOLUME);
			}
		}
	}

	return(volume);
}

bool
InputXponentObj::
WaveformVolumeInvert
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_VOLUME)
		)
		{
			return(true);
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_VOLUME)
		)
		{
			return(true);
		}
	}

	return(false);
}

bool
InputXponentObj::
WaveformVolumeSolo
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_UP)
		)
		{
			return(true);
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_UP)
		)
		{
			return(true);
		}
	}

	return(false);
}

float
InputXponentObj::
WaveformRewindFF
(
	unsigned int	target
)	const
{
	float rewindff=0.0f;

	if(LGL_GetXponent())
	{
		if((target & TARGET_BOTTOM))
		{
			if(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_REWIND))
			{
				rewindff-=32.0f;
			}
			if(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_FAST_FORWARD))
			{
				rewindff+=32.0f;
			}
		}
		else if(target & TARGET_TOP)
		{
			if(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_REWIND))
			{
				rewindff-=32.0f;
			}
			if(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_FAST_FORWARD))
			{
				rewindff+=32.0f;
			}
		}
	}

	return(rewindff);
}

bool
InputXponentObj::
WaveformRecordHold
(
	unsigned int	target
)	const
{
	bool hold=false;

	if(LGL_GetXponent())
	{
		if((target & TARGET_BOTTOM))
		{
			hold|=LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_RECORD);
		}
		else if((target & TARGET_TOP))
		{
			hold|=LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_RECORD);
		}
	}
	return(hold);
}

bool
InputXponentObj::
WaveformStutter
(
	unsigned int	target
)	const
{
	bool hold=false;

	if(LGL_GetXponent())
	{
		if((target & TARGET_BOTTOM))
		{
			hold|=LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_MOD_POWER_2);
		}
		else if((target & TARGET_TOP))
		{
			hold|=LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_MOD_POWER_2);
		}
	}

	return(hold);
}

float
InputXponentObj::
WaveformStutterPitch
(
	unsigned int	target
)	const
{
	float percent=-1.0f;

	if
	(
		LGL_GetXponent() &&
		WaveformStutter(target)
	)
	{
		percent=LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_TOUCHPAD_Y);
	}

	return(percent);
}

float
InputXponentObj::
WaveformStutterSpeed
(
	unsigned int	target
)	const
{
	float percent=-1.0f;

	if
	(
		LGL_GetXponent() &&
		WaveformStutter(target)
	)
	{
		if(target & TARGET_BOTTOM)
		{
			percent=LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_TOUCHPAD_X);
		}
		else if(target & TARGET_TOP)
		{
			percent=LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_TOUCHPAD_X);
		}
	}

	return(percent);
}

float
InputXponentObj::
WaveformRecordSpeed
(
	unsigned int	target
)	const
{
	float speed=0.0f;

	if(LGL_GetXponent())
	{
		const float neutral=64.0f;
		float deadZone=1.0f;

		if((target & TARGET_BOTTOM))
		{
			speed=LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_RIGHT_RECORD);
		}
		else if(target & TARGET_TOP)
		{
			speed=LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_LEFT_RECORD);
		}

		if
		(
			speed==-1.0f ||
			(
				speed>=(neutral-deadZone)/127.0f &&
				speed<=(neutral+deadZone)/127.0f
			)
		)
		{
			speed=0.5f;	//neutral
		}

		if(speed>0.5f) speed-=deadZone/127.0f;
		else if(speed<0.5f) speed+=deadZone/127.0f;

		if(speed!=0.5f) speed*=127.0f/128.0f;	//Recenter

		//[0,1] => [-1,1]
		speed=2.0f*(speed-0.5f);

		//TODO: Detect which midi value = normal playback speed for a 45 record.
		speed*=16.0f;
	}

	return(speed);
}

bool
InputXponentObj::
WaveformSavePointPrev
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_PREV)
		)
		{
			return(true);
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_PREV)
		)
		{
			return(true);
		}
	}

	return(false);
}

bool
InputXponentObj::
WaveformSavePointNext
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_NEXT)
		)
		{
			return(true);
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_NEXT)
		)
		{
			return(true);
		}
	}

	return(false);
}

bool
InputXponentObj::
WaveformSavePointSet
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent())
	{
		if
		(
			!LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_PREV) ||
			!LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_NEXT)
		)
		{
			if
			(
				(target & TARGET_BOTTOM) &&
				LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOCK)
			)
			{
				return(true);
			}
			else if
			(
				(target & TARGET_TOP) &&
				LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOCK)
			)
			{
				return(true);
			}
		}
	}

	return(false);
}

float
InputXponentObj::
WaveformSavePointUnsetPercent
(
	unsigned int	target
)	const
{
	float percent=0.0f;

	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_LOCK)
		)
		{
			percent=LGL_Min(1.0f,2.0f*WaveformSavePointUnsetTimerRight.SecondsSinceLastReset());
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_LOCK)
		)
		{
			percent=LGL_Min(1.0f,2.0f*WaveformSavePointUnsetTimerLeft.SecondsSinceLastReset());
		}
	}

	return(percent);
}

float
InputXponentObj::
WaveformSavePointShift
(
	unsigned int	target
)	const
{
	const float SPEED = 0.025f;
	float percent=0.0f;

	if(LGL_GetXponent())
	{
		if((target & TARGET_BOTTOM))
		{
			if
			(
				!LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_PREV) &&
				!LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_NEXT)
			)
			{
				percent+=LGL_SecondsSinceLastFrame()*SPEED*
				(
					(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_MINUS) ? 1 : 0) -
					(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_PLUS) ? 1 : 0)
				);
			}
		}
		else if((target & TARGET_TOP))
		{
			if
			(
				!LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_PREV) &&
				!LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_NEXT)
			)
			{
				percent+=LGL_SecondsSinceLastFrame()*SPEED*
				(
					(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_MINUS) ? 1 : 0) -
					(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_PLUS) ? 1 : 0)
				);
			}
		}
	}

	return(percent);
}

float
InputXponentObj::
WaveformSavePointShiftAll
(
	unsigned int	target
)	const
{
	const float SPEED = 0.025f;
	float percent=0.0f;

	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_PREV) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_NEXT)
		)
		{
			percent+=LGL_SecondsSinceLastFrame()*SPEED*
			(
				(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_MINUS) ? 1 : 0) -
				(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_PLUS) ? 1 : 0)
			);
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_PREV) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_NEXT)
		)
		{
			percent+=LGL_SecondsSinceLastFrame()*SPEED*
			(
				(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_MINUS) ? 1 : 0) -
				(LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_PLUS) ? 1 : 0)
			);
		}
	}

	return(percent);
}

bool
InputXponentObj::
WaveformSavePointShiftAllHere
(
	unsigned int	target
)	const
{
	if(LGL_GetXponent())
	{
		if
		(
			(target & TARGET_BOTTOM) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_PREV) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_RIGHT_NEXT) &&
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOCK)
		)
		{
			return(true);
		}
		else if
		(
			(target & TARGET_TOP) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_PREV) &&
			LGL_GetXponent()->GetButtonDown(LGL_XPONENT_BUTTON_LEFT_NEXT) &&
			LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOCK)
		)
		{
			return(true);
		}
	}

	return(false);
}

bool
InputXponentObj::
WaveformSavePointJumpAtMeasure
(
	unsigned int	target
)	const
{
	bool select=false;

	if(LGL_GetXponent())
	{
		if((target & TARGET_BOTTOM))
		{
			select|=LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_X);
		}
		else if((target & TARGET_TOP))
		{
			select|=LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_X);
		}
	}

	return(select);
}

bool
InputXponentObj::
WaveformSavePointJumpNow
(
	unsigned int	target
)	const
{
	bool select=false;

	if(LGL_GetXponent())
	{
		if((target & TARGET_BOTTOM))
		{
			select|=LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_DASH);
		}
		else if((target & TARGET_TOP))
		{
			select|=LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_DASH);
		}
	}

	return(select);
}

bool
InputXponentObj::
WaveformLoopBegin
(
	unsigned int	target
)	const
{
	bool begin=false;

	if(LGL_GetXponent())
	{
		if((target & TARGET_BOTTOM))
		{
			begin|=LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_IN);
		}
		else if((target & TARGET_TOP))
		{
			begin|=LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_IN);
		}
	}

	return(begin);
}

bool
InputXponentObj::
WaveformLoopEnd
(
	unsigned int	target
)	const
{
	bool end=false;

	if(LGL_GetXponent())
	{
		if((target & TARGET_BOTTOM))
		{
			end|=LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_OUT);
		}
		else if((target & TARGET_TOP))
		{
			end|=LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_OUT);
		}
	}

	return(end);
}

bool
InputXponentObj::
WaveformLoopDisable
(
	unsigned int	target
)	const
{
	bool disable=false;

	if(LGL_GetXponent())
	{
		if((target & TARGET_BOTTOM))
		{
			disable|=LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_CYCLE);
		}
		else if((target & TARGET_TOP))
		{
			disable|=LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_CYCLE);
		}
	}

	return(disable);
}

int
InputXponentObj::
WaveformLoopMeasures
(
	unsigned int	target
)	const
{
	int measures=-1;

	if(LGL_GetXponent())
	{
		if((target & TARGET_BOTTOM))
		{
			if(LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_1)) measures=1;
			if(LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_2)) measures=2;
			if(LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_4)) measures=4;
			if(LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_LOOP_8)) measures=8;
		}
		else if((target & TARGET_TOP))
		{
			if(LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_1)) measures=1;
			if(LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_2)) measures=2;
			if(LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_4)) measures=4;
			if(LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_LOOP_8)) measures=8;
		}
	}

	return(measures);
}

bool
InputXponentObj::
WaveformVideoSelect
(
	unsigned int	target
)	const
{
	bool select=false;

	if(LGL_GetXponent())
	{
		if((target & TARGET_BOTTOM))
		{
			select|=
				LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_RECORD) ||
				LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_RIGHT_FINGER);
		}
		else if((target & TARGET_TOP))
		{
			select|=
				LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_RECORD) ||
				LGL_GetXponent()->GetButtonStroke(LGL_XPONENT_BUTTON_LEFT_FINGER);
		}
	}

	return(select);
}

float
InputXponentObj::
WaveformVideoAdvanceRate
(
	unsigned int	target
)	const
{
	float rate=-1.0f;

	if(LGL_GetXponent())
	{
		if((target & TARGET_BOTTOM))
		{
			if(LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_RIGHT_MOD_1))
			{
				rate=powf(2.0f,3.0f*LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_RIGHT_MOD_1));
			}
		}
		else if((target & TARGET_TOP))
		{
			if(LGL_GetXponent()->GetKnobTweak(LGL_XPONENT_KNOB_LEFT_MOD_1))
			{
				rate=powf(2.0f,3.0f*LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_LEFT_MOD_1));
			}
		}
	}

	return(rate);
}

bool
InputXponentObj::
WaveformSyncBPM
(
	unsigned int	target
)	const
{
	return(false);
}

float
InputXponentObj::
WaveformPointerScratch
(
	unsigned int	target
)	const
{
	return(-1.0f);
}
