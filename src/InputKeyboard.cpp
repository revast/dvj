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

#include "Config.h"

//Core

void
InputKeyboardObj::
NextFrame()
{
	if
	(
		LGL_KeyDown(GetInputKeyboardWaveformSavePointUnsetKey())==false ||
		LGL_KeyDown(GetInputKeyboardFocusChangeKey())
	)
	{
		WaveformSavePointUnsetTimer.Reset();
	}

	if
	(
		LGL_KeyDown(GetInputKeyboardWaveformLoopMeasuresHalfKey())==false &&
		LGL_KeyDown(GetInputKeyboardWaveformLoopMeasuresDoubleKey())==false
	)
	{
		WaveformLoopAllDebump=false;
	}
	else if(WaveformLoopAll(TARGET_FOCUS))
	{
		WaveformLoopAllDebump=true;
	}
}

//Global Input

bool
InputKeyboardObj::
FocusChange()		const
{
	return(LGL_KeyStroke(GetInputKeyboardFocusChangeKey()));
}

bool
InputKeyboardObj::
FocusBottom()		const
{
	return(LGL_KeyStroke(GetInputKeyboardFocusBottomKey()));
}

bool
InputKeyboardObj::
FocusTop()		const
{
	return(LGL_KeyStroke(GetInputKeyboardFocusTopKey()));
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
	delta+=(LGL_KeyDown(GetInputKeyboardXfaderSpeakersDeltaUpKey())?1:0)*LGL_SecondsSinceLastFrame();
	delta-=(LGL_KeyDown(GetInputKeyboardXfaderSpeakersDeltaDownKey())?1:0)*LGL_SecondsSinceLastFrame();
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
	delta+=(LGL_KeyDown(GetInputKeyboardXfaderHeadphonesDeltaUpKey())?1:0)*LGL_SecondsSinceLastFrame();
	delta-=(LGL_KeyDown(GetInputKeyboardXfaderHeadphonesDeltaDownKey())?1:0)*LGL_SecondsSinceLastFrame();
	return(delta);
}

bool
InputKeyboardObj::
SyncTopToBottom()	const
{
	return(LGL_KeyDown(GetInputKeyboardSyncTopToBottomKey()));
}

bool
InputKeyboardObj::
SyncBottomToTop()	const
{
	return(LGL_KeyDown(GetInputKeyboardSyncBottomToTopKey()));
}

int
InputKeyboardObj::
MasterToHeadphones()	const
{
	int to=-1;
	if(XfaderHeadphonesDelta()!=0)
	{
		to=0;
	}
	if(XfaderSpeakersDelta()!=0)
	{
		to=1;
	}
	return(to);
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
			(LGL_KeyDown(GetInputKeyboardFileScrollDownManyKey()) ? LGL_SecondsSinceLastFrame()*RATE : 0) +
			(LGL_KeyDown(GetInputKeyboardFileScrollUpManyKey()) ? -LGL_SecondsSinceLastFrame()*RATE : 0)
		);
		scroll+=
		(
			(LGL_KeyStroke(GetInputKeyboardFileScrollDownOneKey()) ? 1 : 0) +
			(LGL_KeyStroke(GetInputKeyboardFileScrollUpOneKey()) ? -1 : 0)
		);
	}

	return(scroll);
}

int
InputKeyboardObj::
FileSelect
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(GetInputKeyboardFileSelectKey()) ? 1 : 0);
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
FileMarkUnopened
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(GetInputKeyboardFileMarkUnopenedKey()));
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
		return(LGL_KeyStroke(GetInputKeyboardFileRefreshKey()));
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
		return(LGL_KeyStroke(GetInputKeyboardDecodeAbortKey()));
	}
	else
	{
		return(false);
	}
}

//Mode 2: Waveform

int
InputKeyboardObj::
WaveformEject
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyDown(GetInputKeyboardWaveformEjectKey()) ? 1 : 0);
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
		return(LGL_KeyStroke(GetInputKeyboardWaveformTogglePauseKey()));
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
			(LGL_KeyDown(GetInputKeyboardWaveformNudgeLeft1Key()) ? -DELTA : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformNudgeRight1Key()) ? DELTA : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformNudgeLeft2Key()) ? -DELTA : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformNudgeRight2Key()) ? DELTA : 0)
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
			(LGL_KeyDown(GetInputKeyboardWaveformPitchbendDeltaDownSlowKey()) ? -RATE_SLOW*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformPitchbendDeltaUpSlowKey()) ? RATE_SLOW*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformPitchbendDeltaDownFastKey()) ? -RATE_FAST*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformPitchbendDeltaUpFastKey()) ? RATE_FAST*LGL_SecondsSinceLastFrame() : 0)
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
			(LGL_KeyDown(GetInputKeyboardWaveformEQLowDeltaDownKey()) ? -EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformEQLowDeltaUpKey()) ? EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0)
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
		return(LGL_KeyDown(GetInputKeyboardWaveformEQLowKillKey()));
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
			(LGL_KeyDown(GetInputKeyboardWaveformEQMidDeltaDownKey()) ? -EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformEQMidDeltaUpKey()) ? EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0)
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
		return(LGL_KeyDown(GetInputKeyboardWaveformEQMidKillKey()));
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
			(LGL_KeyDown(GetInputKeyboardWaveformEQHighDeltaDownKey()) ? -EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformEQHighDeltaUpKey()) ? EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0)
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
		return(LGL_KeyDown(GetInputKeyboardWaveformEQHighKillKey()));
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
			(LGL_KeyDown(GetInputKeyboardWaveformGainDeltaDownKey()) ? -EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformGainDeltaUpKey()) ? EQ_DELTA_RATE*LGL_SecondsSinceLastFrame() : 0)
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
		return(LGL_KeyDown(GetInputKeyboardWaveformVolumeInvertKey()));
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformRapidVolumeInvert
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyDown(GetInputKeyboardWaveformRapidVolumeInvertKey()));
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformRapidSoloInvert
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyDown(GetInputKeyboardWaveformRapidSoloInvertKey()));
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
	return(LGL_KeyDown(GetInputKeyboardWaveformVolumeSoloKey()));
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
		speed-=(LGL_KeyDown(GetInputKeyboardWaveformRewindKey())?1:0)*SPEED_FAST;
		speed+=(LGL_KeyDown(GetInputKeyboardWaveformFFKey())?1:0)*SPEED_FAST;
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
		hold|=(WaveformRecordSpeed(target)!=0.0f);
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
		speed-=(LGL_KeyDown(GetInputKeyboardWaveformRecordSpeedBackKey())?1:0)*SPEED_SLOW;
		speed+=(LGL_KeyDown(GetInputKeyboardWaveformRecordSpeedForwardKey())?1:0)*SPEED_SLOW;
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
	return(LGL_KeyDown(GetInputKeyboardWaveformStutterKey()));
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
		return(LGL_KeyStroke(GetInputKeyboardWaveformSavePointPrevKey()));
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
		return(LGL_KeyStroke(GetInputKeyboardWaveformSavePointNextKey()));
	}
	else
	{
		return(false);
	}
	return(false);
}

int
InputKeyboardObj::
WaveformSavePointPick
(
	unsigned int	target
)	const
{
	return(-9999);
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
		return(LGL_KeyStroke(GetInputKeyboardWaveformSavePointSetKey()));
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
			(LGL_KeyDown(GetInputKeyboardWaveformSavePointShiftLeftKey()) ? -1 : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformSavePointShiftRightKey()) ? 1 : 0)
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
	const float SPEED = 0.025f;
	float percent=0.0f;

	if((target & TARGET_FOCUS))
	{
		percent+=LGL_SecondsSinceLastFrame()*SPEED*
		(
			(LGL_KeyDown(GetInputKeyboardWaveformSavePointShiftAllLeftKey()) ? -1 : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformSavePointShiftAllRightKey()) ? 1 : 0)
		);
	}

	return(percent);
}

bool
InputKeyboardObj::
WaveformSavePointShiftAllHere
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return
		(
			LGL_KeyDown(GetInputKeyboardWaveformSavePointShiftLeftKey()) &&
			LGL_KeyDown(GetInputKeyboardWaveformSavePointShiftRightKey()) &&
			LGL_KeyStroke(GetInputKeyboardWaveformSavePointSetKey())
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
		return(LGL_KeyStroke(GetInputKeyboardWaveformSavePointJumpNowKey()));
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
		return(LGL_KeyStroke(GetInputKeyboardWaveformSavePointJumpAtMeasureKey()));
	}
	else
	{
		return(false);
	}
}

int
InputKeyboardObj::
WaveformLoopMeasuresExponent
(
	unsigned int	target
)	const
{
	return(WAVEFORM_LOOP_MEASURES_EXPONENT_NULL);
}

bool
InputKeyboardObj::
WaveformLoopMeasuresHalf
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return
		(
			WaveformLoopAllDebump==false &&
			LGL_KeyStroke(GetInputKeyboardWaveformLoopMeasuresHalfKey())
		);
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformLoopMeasuresDouble
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return
		(
			WaveformLoopAllDebump==false &&
			LGL_KeyStroke(GetInputKeyboardWaveformLoopMeasuresDoubleKey())
		);
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformLoopSecondsLess
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return
		(
			WaveformLoopAllDebump==false &&
			LGL_KeyDown(GetInputKeyboardWaveformLoopMeasuresHalfKey()) &&
			LGL_KeyDown(GetInputKeyboardWaveformLoopMeasuresDoubleKey())==false
		);
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformLoopSecondsMore
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return
		(
			WaveformLoopAllDebump==false &&
			LGL_KeyDown(GetInputKeyboardWaveformLoopMeasuresDoubleKey()) &&
			LGL_KeyDown(GetInputKeyboardWaveformLoopMeasuresHalfKey())==false
		);
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformLoopAll
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return
		(
			LGL_KeyDown(GetInputKeyboardWaveformLoopMeasuresHalfKey()) &&
			LGL_KeyDown(GetInputKeyboardWaveformLoopMeasuresDoubleKey()) &&
			LGL_KeyTimer(GetInputKeyboardWaveformLoopMeasuresHalfKey()) >= 1.0f &&
			LGL_KeyTimer(GetInputKeyboardWaveformLoopMeasuresDoubleKey()) >= 1.0f
		);
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformLoopToggle
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(GetInputKeyboardWaveformLoopToggleKey()));
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformLoopThenRecallActive
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyDown(GetInputKeyboardWaveformLoopThenRecallKey()));
	}
	else
	{
		return(false);
	}
}

int
InputKeyboardObj::
WaveformAutoDivergeRecall
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(GetInputKeyboardWaveformAutoDivergeRecallKey()) ? -2 : 0);
	}
	else
	{
		return(0);
	}
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
		if(LGL_KeyStroke(GetInputKeyboardWaveformVideoSelectKey()))
		{
			return(true);
		}
	}

	return(false);
}

float
InputKeyboardObj::
WaveformVideoBrightness
(
	unsigned int	target
)	const
{
	return(-1.0f);
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

int
InputKeyboardObj::
WaveformVideoFreqSenseMode
(
	unsigned int	target
)	const
{
	int mode=-1;

	if(target & TARGET_FOCUS)
	{
		if(0 && LGL_KeyStroke(GetInputKeyboardWaveformVideoFreqSenseModeKey()))
		{
			mode=-10;
		}
		if
		(
			LGL_KeyDown(GetInputKeyboardWaveformVideoFreqSenseModeKey()) &&
			LGL_KeyTimer(GetInputKeyboardWaveformVideoFreqSenseModeKey())>=1.0f
		)
		{
			mode=2;
		}
	}

	return(mode);
}

bool
InputKeyboardObj::
WaveformVideoAspectRatioModeNext
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		if(LGL_KeyStroke(GetInputKeyboardWaveformVideoAspectRatioModeNextKey()))
		{
			return(true);
		}
	}

	return(false);
}

float
InputKeyboardObj::
WaveformOscilloscopeBrightness
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
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyDown(GetInputKeyboardWaveformSyncBPMKey()));
	}
	
	return(false);
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

