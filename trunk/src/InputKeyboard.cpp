/*
 *
 * InputKeyboard.cpp
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

#include "InputKeyboard.h"

#include "LGL.module/LGL.h"

//Core

void
InputKeyboardObj::
NextFrame()
{
	if
	(
		LGL_KeyDown(SDLK_KP5)==false ||
		LGL_KeyDown(SDLK_TAB)
	)
	{
		WaveformSavePointUnsetTimer.Reset();
	}
}

//Global Input

bool
InputKeyboardObj::
FocusChange()		const
{
	return(LGL_KeyStroke(SDLK_TAB));
}

bool
InputKeyboardObj::
FocusBottom()		const
{
	return(false);
}

bool
InputKeyboardObj::
FocusTop()		const
{
	return(false);
}

float
InputKeyboardObj::
XfaderSpeakers()	const
{
	return(-1.0f);
}

float
InputKeyboardObj::
XfaderSpeakersDelta()	const
{
	float delta=0.0f;
	delta+=(LGL_KeyDown(SDLK_INSERT)?1:0)*LGL_SecondsSinceLastFrame();
	delta-=(LGL_KeyDown(SDLK_DELETE)?1:0)*LGL_SecondsSinceLastFrame();
	return(delta);
}

float
InputKeyboardObj::
XfaderHeadphones()	const
{
	return(-1.0f);
}

float
InputKeyboardObj::
XfaderHeadphonesDelta()	const
{
	float delta=0.0f;
	delta+=(LGL_KeyDown(SDLK_HOME)?1:0)*LGL_SecondsSinceLastFrame();
	delta-=(LGL_KeyDown(SDLK_END)?1:0)*LGL_SecondsSinceLastFrame();
	return(delta);
}

bool
InputKeyboardObj::
SyncTopToBottom()	const
{
	return(false);
}

bool
InputKeyboardObj::
SyncBottomToTop()	const
{
	return(false);
}

//Mode 0: File Selection

float
InputKeyboardObj::
FileScroll
(
	unsigned int	target
)	const
{
	const float RATE = 40.0f;

	float scroll = 0.0f;

	if(target & TARGET_FOCUS)
	{
		scroll+=
		(
			(LGL_KeyDown(SDLK_PAGEDOWN) ? LGL_SecondsSinceLastFrame()*RATE : 0) +
			(LGL_KeyDown(SDLK_PAGEUP) ? -LGL_SecondsSinceLastFrame()*RATE : 0)
		);
		scroll+=
		(
			(LGL_KeyStroke(SDLK_DOWN) ? 1 : 0) +
			(LGL_KeyStroke(SDLK_UP) ? -1 : 0)
		);
	}

	return(scroll);
}

bool
InputKeyboardObj::
FileSelect
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(SDLK_RETURN));
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
FileRefresh
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(SDLK_F5));
	}
	else
	{
		return(false);
	}
}

//Mode 1: Decoding...

bool
InputKeyboardObj::
DecodeAbort
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(SDLK_BACKSPACE));
	}
	else
	{
		return(false);
	}
}

//Mode 2: Waveform

bool
InputKeyboardObj::
WaveformEject
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyDown(SDLK_BACKSPACE));
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformTogglePause
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(SDLK_RETURN));
	}
	else
	{
		return(false);
	}
}

float
InputKeyboardObj::
WaveformNudge
(
	unsigned int	target
)	const
{
	const float DELTA = 0.04f;

	if(target & TARGET_FOCUS)
	{
		return
		(
			(LGL_KeyDown(SDLK_LEFT) ? -DELTA : 0) +
			(LGL_KeyDown(SDLK_RIGHT) ? DELTA : 0) +
			(LGL_KeyDown(SDLK_KP7) ? -DELTA : 0) +
			(LGL_KeyDown(SDLK_KP9) ? DELTA : 0)
		);
	}
	else
	{
		return(0.0f);
	}
}

float
InputKeyboardObj::
WaveformPitchbend
(
	unsigned int	target
)	const
{
	return(0.0f);
}

float
InputKeyboardObj::
WaveformPitchbendDelta
(
	unsigned int	target
)	const
{
	const float RATE_SLOW = 0.005f;
	const float RATE_FAST = 0.1f;

	float delta=0.0f;

	if(target & TARGET_FOCUS)
	{
		delta+=
		(
			(LGL_KeyDown(SDLK_DOWN) ? -RATE_SLOW*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(SDLK_UP) ? RATE_SLOW*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(SDLK_PAGEDOWN) ? -RATE_FAST*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(SDLK_PAGEUP) ? RATE_FAST*LGL_SecondsSinceLastFrame() : 0)
		);
	}

	return(delta);
}

#define EQ_DELTA_RATE (1.0f)

float
InputKeyboardObj::
WaveformEQLow
(
	unsigned int	target
)	const
{
	return(-1.0f);
}

float
InputKeyboardObj::
WaveformEQLowDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	if(target & TARGET_FOCUS)
	{
		delta=
		(
			(LGL_KeyDown(SDLK_z) ? -EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(SDLK_x) ? EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0)
		);
	}
	return(delta);
}

bool
InputKeyboardObj::
WaveformEQLowKill
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyDown(SDLK_c));
	}
	else
	{
		return(false);
	}
}

float
InputKeyboardObj::
WaveformEQMid
(
	unsigned int	target
)	const
{
	return(-1.0f);
}

float
InputKeyboardObj::
WaveformEQMidDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	if(target & TARGET_FOCUS)
	{
		delta=
		(
			(LGL_KeyDown(SDLK_a) ? -EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(SDLK_s) ? EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0)
		);
	}
	return(delta);
}

bool
InputKeyboardObj::
WaveformEQMidKill
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyDown(SDLK_d));
	}
	else
	{
		return(false);
	}
}

float
InputKeyboardObj::
WaveformEQHigh
(
	unsigned int	target
)	const
{
	return(-1.0f);
}

float
InputKeyboardObj::
WaveformEQHighDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	if(target & TARGET_FOCUS)
	{
		delta=
		(
			(LGL_KeyDown(SDLK_q) ? -EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(SDLK_w) ? EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0)
		);
	}
	return(delta);
}

bool
InputKeyboardObj::
WaveformEQHighKill
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyDown(SDLK_e));
	}
	else
	{
		return(false);
	}
}

float
InputKeyboardObj::
WaveformGain
(
	unsigned int	target
)	const
{
	return(-1.0f);
}

float
InputKeyboardObj::
WaveformGainDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	if(target & TARGET_FOCUS)
	{
		delta=
		(
			(LGL_KeyDown(SDLK_MINUS) ? -EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(SDLK_EQUALS) ? EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0)
		);
	}
	return(delta);
}

bool
InputKeyboardObj::
WaveformGainKill
(
	unsigned int	target
)	const
{
	return(false);
}

float
InputKeyboardObj::
WaveformVolumeSlider
(
	unsigned int	target
)	const
{
	return(-1.0f);
}

bool
InputKeyboardObj::
WaveformVolumeInvert
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyDown(SDLK_KP0));
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformVolumeSolo
(
	unsigned int	target
)	const
{
	return(false);
}

float
InputKeyboardObj::
WaveformRewindFF
(
	unsigned int	target
)	const
{
	const float SPEED_FAST=32.0f;
	float speed=0.0f;

	if(target & TARGET_FOCUS)
	{
		speed-=(LGL_KeyDown(SDLK_LEFTBRACKET)?1:0)*SPEED_FAST;
		speed+=(LGL_KeyDown(SDLK_RIGHTBRACKET)?1:0)*SPEED_FAST;
	}
	
	return(speed);
}

bool
InputKeyboardObj::
WaveformRecordHold
(
	unsigned int	target
)	const
{
	bool hold=false;

	if(target & TARGET_FOCUS)
	{
		hold|=LGL_KeyDown(SDLK_SEMICOLON);
		hold|=LGL_KeyDown(SDLK_QUOTE);
	}

	return(hold);
}

float
InputKeyboardObj::
WaveformRecordSpeed
(
	unsigned int	target
)	const
{
	const float SPEED_SLOW=2.0f;
	float speed=0.0f;

	if(target & TARGET_FOCUS)
	{
		speed-=(LGL_KeyDown(SDLK_SEMICOLON)?1:0)*SPEED_SLOW;
		speed+=(LGL_KeyDown(SDLK_QUOTE)?1:0)*SPEED_SLOW;
	}

	return(speed);
}

bool
InputKeyboardObj::
WaveformStutter
(
	unsigned int	target
)	const
{
	return(false);
}

float
InputKeyboardObj::
WaveformStutterPitch
(
	unsigned int	target
)	const
{
	return(-1.0f);
}

float
InputKeyboardObj::
WaveformStutterSpeed
(
	unsigned int	target
)	const
{
	return(-1.0f);
}

bool
InputKeyboardObj::
WaveformSavePointPrev
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(SDLK_KP4));
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformSavePointNext
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(SDLK_KP6));
	}
	else
	{
		return(false);
	}
	return(false);
}

bool
InputKeyboardObj::
WaveformSavePointSet
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(SDLK_KP5));
	}
	else
	{
		return(false);
	}
}

float
InputKeyboardObj::
WaveformSavePointUnsetPercent
(
	unsigned int	target
)	const
{
	float percent=0.0f;

	if(target & TARGET_FOCUS)
	{
		percent=LGL_Clamp(0.0f,2.0f*WaveformSavePointUnsetTimer.SecondsSinceLastReset(),1.0f);
	}
	else
	{
		//
	}

	return(percent);
}

float
InputKeyboardObj::
WaveformSavePointShift
(
	unsigned int	target
)	const
{
	const float SPEED = 0.025f;
	float percent=0.0f;

	if((target & TARGET_FOCUS))
	{
		percent+=LGL_SecondsSinceLastFrame()*SPEED*
		(
			(LGL_KeyDown(SDLK_KP3) ? 1 : 0) -
			(LGL_KeyDown(SDLK_KP1) ? 1 : 0)
		);
	}

	return(percent);
}

float
InputKeyboardObj::
WaveformSavePointShiftAll
(
	unsigned int	target
)	const
{
	return(0.0f);
}

bool
InputKeyboardObj::
WaveformSavePointShiftAllHere
(
	unsigned int	target
)	const
{
printf("ABC: %i %i %i\n",
			LGL_KeyDown(SDLK_KP4),
			LGL_KeyDown(SDLK_KP6),
			LGL_KeyStroke(SDLK_KP5));
	if(target & TARGET_FOCUS)
	{
		return
		(
			LGL_KeyDown(SDLK_KP4) &&
			LGL_KeyDown(SDLK_KP6) &&
			LGL_KeyStroke(SDLK_KP5)
		);
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformSavePointJumpNow
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(SDLK_KP2));
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformSavePointJumpAtMeasure
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(SDLK_KP8));
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformLoopBegin
(
	unsigned int	target
)	const
{
	return(false);
}

bool
InputKeyboardObj::
WaveformLoopEnd
(
	unsigned int	target
)	const
{
	return(false);
}

bool
InputKeyboardObj::
WaveformLoopDisable
(
	unsigned int	target
)	const
{
	return(false);
}

int
InputKeyboardObj::
WaveformLoopMeasures
(
	unsigned int	target
)	const
{
	return(-1);
}

bool
InputKeyboardObj::
WaveformVideoSelect
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		if(LGL_KeyStroke(SDLK_SPACE))
		{
			return(true);
		}
	}

	return(false);
}

float
InputKeyboardObj::
WaveformVideoAdvanceRate
(
	unsigned int	target
)	const
{
	return(-1.0f);
}

bool
InputKeyboardObj::
WaveformSyncBPM
(
	unsigned int	target
)	const
{
	return(LGL_KeyDown(SDLK_BACKSLASH));
}

float
InputKeyboardObj::
WaveformPointerScratch
(
	unsigned int	target
)	const
{
	return(-1.0f);
}

