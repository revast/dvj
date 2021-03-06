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

#define	BPM_INPUT_KEY	LGL_KEY_BACKQUOTE

void
InputKeyboardObj::
NextFrame()
{
	if
	(
		LGL_KeyDown(GetInputKeyboardWaveformSavepointSetKey())==false ||
		LGL_KeyDown(LGL_KEY_SHIFT) ||
		LGL_KeyDown(GetInputKeyboardFocusChangeKey())
	)
	{
		WaveformSavepointUnsetTimer.Reset();
	}

	if
	(
		LGL_KeyDown(GetInputKeyboardWaveformQuantizationPeriodHalfKey())==false &&
		LGL_KeyDown(GetInputKeyboardWaveformQuantizationPeriodDoubleKey())==false
	)
	{
		WaveformLoopAllDebump=false;
	}
	else if(WaveformLoopAll(TARGET_FOCUS))
	{
		WaveformLoopAllDebump=true;
	}

	if(LGL_KeyDown(GetInputKeyboardWaveformPitchbendResetKey()))
	{
		SyncDebump=true;
	}
	if(LGL_KeyDown(GetInputKeyboardWaveformSyncKey())==false)
	{
		SyncDebump=false;
	}

	if(LGL_KeyDown(BPM_INPUT_KEY))
	{
		if(LGL_KeyStroke(BPM_INPUT_KEY))
		{
			BPMInputBuffer.ClearBuffer();
			BPMInputBuffer.GrabFocus();
			BPMInputBuffer.AcceptFloat();
			char tmpstr[1024];
			if(BPMValueHint>0.0f)
			{
				sprintf(tmpstr,"%.2f",BPMValueHint);
			}
			else
			{
				tmpstr[0]='\0';
			}
			BPMInputBuffer.SetString(tmpstr);
		}
		BPMValueCandidate = BPMInputBuffer.GetString();
		if(LGL_KeyStroke(LGL_KEY_RETURN))
		{
			BPMValue = BPMInputBuffer.GetFloat();
		}
		else
		{
			BPMValue = -1.0f;
		}
	}
	else
	{
		BPMValue=-1.0f;
		BPMValueCandidate=NULL;
		BPMInputBuffer.ClearBuffer();
		BPMInputBuffer.ReleaseFocus();
	}

	if(LGL_KeyDown(GetInputKeyboardWaveformSavepointSetBPMAtNeedleKey()))
	{
		if(BPMAtNeedleState==0)
		{
			//Not Pressed => Pressed

			BPMAtNeedleTimer.Reset();
			BPMAtNeedleState++;
		}
		else if (BPMAtNeedleState==1)
		{
			//BPM At Needle (1 Frame)

			//This just works
			BPMAtNeedleState++;
		}
		else if (BPMAtNeedleState==2)
		{
			//Waiting for BPM_UNDEF
			if(BPMAtNeedleTimer.SecondsSinceLastReset()>=1.0f)
			{
				BPMAtNeedleState++;
			}
		}
		else if (BPMAtNeedleState==3)
		{
			//BPM_UNDEF (1 Frame)

			BPMAtNeedleState++;
		}
		else if (BPMAtNeedleState==4)
		{
			//Waiting for BPM_NONE
			if(BPMAtNeedleTimer.SecondsSinceLastReset()>=2.0f)
			{
				BPMAtNeedleState++;
			}
		}
		else if (BPMAtNeedleState==5)
		{
			//BPM_NONE (1 Frame)

			BPMAtNeedleState++;
		}
		else if (BPMAtNeedleState==6)
		{
			//Waiting
		}
	}
	else
	{
		BPMAtNeedleState=0;
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
	const float RATE = 200.0f;

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
			(LGL_KeyStroke(GetInputKeyboardFileScrollNextKey()) ? 1 : 0) +
			(LGL_KeyStroke(GetInputKeyboardFileScrollPrevKey()) ? -1 : 0)
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



//Mode 2: Waveform

int
InputKeyboardObj::
WaveformEject
(
	unsigned int	target
)	const
{
	if(LGL_KeyDown(BPM_INPUT_KEY)) return(false);
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
WaveformPauseToggle
(
	unsigned int	target
)	const
{
	if(LGL_KeyDown(BPM_INPUT_KEY)) return(false);
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(GetInputKeyboardWaveformPauseToggleKey()));
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
			(LGL_KeyDown(GetInputKeyboardWaveformNudgeBackwardKey()) ? -DELTA : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformNudgeForwardKey()) ? DELTA : 0)
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
	if(LGL_KeyStroke(GetInputKeyboardWaveformPitchbendResetKey()))
	{
		if(target & TARGET_FOCUS)
		{
			return(1.0f);
		}
	}
	
	return(DVJ_INPUT_NIL);
}

float
InputKeyboardObj::
WaveformPitchbendDelta
(
	unsigned int	target
)	const
{
	const float RATE_SLOW = 0.005f;
	const float RATE_MID = 0.005f;
	const float RATE_FAST = 0.1f;

	float delta=0.0f;

	if(target & TARGET_FOCUS)
	{
		delta+=
		(
			(LGL_KeyDown(GetInputKeyboardWaveformPitchbendDeltaDownSlowKey()) ? -RATE_SLOW*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformPitchbendDeltaUpSlowKey()) ? RATE_SLOW*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformPitchbendDeltaDownKey()) ? -RATE_MID*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformPitchbendDeltaUpKey()) ? RATE_MID*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformPitchbendDeltaDownFastKey()) ? -RATE_FAST*LGL_SecondsSinceLastFrame() : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformPitchbendDeltaUpFastKey()) ? RATE_FAST*LGL_SecondsSinceLastFrame() : 0)
		);
		if(LGL_KeyDown(LGL_KEY_SHIFT))
		{
			delta*=0.05f;
		}
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
WaveformRhythmicVolumeInvert
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyDown(GetInputKeyboardWaveformRhythmicVolumeInvertKey()));
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformRhythmicVolumeInvertOther
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyDown(GetInputKeyboardWaveformRhythmicVolumeInvertOtherKey()));
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
		speed-=(LGL_KeyDown(GetInputKeyboardWaveformSeekBackwardFastKey())?1:0)*SPEED_FAST;
		speed+=(LGL_KeyDown(GetInputKeyboardWaveformSeekForwardFastKey())?1:0)*SPEED_FAST;
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
		speed-=(LGL_KeyDown(GetInputKeyboardWaveformSeekBackwardSlowKey())?1:0)*SPEED_SLOW;
		speed+=(LGL_KeyDown(GetInputKeyboardWaveformSeekForwardSlowKey())?1:0)*SPEED_SLOW;
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
WaveformSavepointPrev
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return
		(
			LGL_KeyStroke(GetInputKeyboardWaveformSavepointPrevKey()) &&
			LGL_KeyDown(LGL_KEY_SHIFT)==false
		);
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformSavepointNext
(
	unsigned int	target
)	const
{
	if(LGL_KeyDown(BPM_INPUT_KEY)) return(false);
	if(target & TARGET_FOCUS)
	{
		return
		(
			LGL_KeyStroke(GetInputKeyboardWaveformSavepointNextKey()) &&
			LGL_KeyDown(LGL_KEY_SHIFT)==false
		);
	}
	else
	{
		return(false);
	}
	return(false);
}

int
InputKeyboardObj::
WaveformSavepointPick
(
	unsigned int	target
)	const
{
	return(-9999);
}

bool
InputKeyboardObj::
WaveformSavepointSet
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return
		(
			LGL_KeyStroke(GetInputKeyboardWaveformSavepointSetKey()) &&
			LGL_KeyDown(LGL_KEY_SHIFT)==false
		);
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformSavepointSetBPMAtNeedle
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyStroke(GetInputKeyboardWaveformSavepointSetBPMAtNeedleKey()));
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformSavepointSetBPMUndef
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(BPMAtNeedleState==3);
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformSavepointSetBPMNone
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(BPMAtNeedleState==5);
	}
	else
	{
		return(false);
	}
}

float
InputKeyboardObj::
WaveformSavepointUnsetPercent
(
	unsigned int	target
)	const
{
	float percent=0.0f;

	if(target & TARGET_FOCUS)
	{
		if(LGL_KeyDown(GetInputKeyboardWaveformSavepointSetKey()))
		{
			percent=LGL_Clamp(0.0f,2.0f*WaveformSavepointUnsetTimer.SecondsSinceLastReset(),1.0f);
		}
	}
	else
	{
		//
	}

	return(percent);
}

float
InputKeyboardObj::
WaveformSavepointShift
(
	unsigned int	target
)	const
{
	/*
	const float SPEED = 0.025f;
	float percent=0.0f;

	if((target & TARGET_FOCUS))
	{
		percent+=LGL_SecondsSinceLastFrame()*SPEED*
		(
			(LGL_KeyDown(GetInputKeyboardWaveformSavepointShiftBackwardKey()) ? -1 : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformSavepointShiftForwardKey()) ? 1 : 0)
		);
	}

	return(percent);
	*/

	float amount=0.0f;
	
	if((target & TARGET_FOCUS))
	{
		amount+=
			(LGL_KeyStroke(GetInputKeyboardWaveformSavepointShiftBackwardKey()) ? -0.01f : 0) +
			(LGL_KeyStroke(GetInputKeyboardWaveformSavepointShiftForwardKey()) ? 0.01f : 0);
	}

	return(amount);
}

float
InputKeyboardObj::
WaveformSavepointShiftAll
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
			(LGL_KeyDown(GetInputKeyboardWaveformSavepointShiftAllBackwardKey()) ? -1 : 0) +
			(LGL_KeyDown(GetInputKeyboardWaveformSavepointShiftAllForwardKey()) ? 1 : 0)
		);
	}

	return(percent);
}

bool
InputKeyboardObj::
WaveformSavepointShiftAllHere
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return
		(
			LGL_KeyDown(GetInputKeyboardWaveformSavepointShiftBackwardKey()) &&
			LGL_KeyDown(GetInputKeyboardWaveformSavepointShiftForwardKey()) &&
			LGL_KeyStroke(GetInputKeyboardWaveformSavepointSetKey())
		);
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformSavepointJumpNow
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return
		(
			LGL_KeyStroke(GetInputKeyboardWaveformSavepointJumpNowKey()) &&
			(
				(
					GetInputKeyboardWaveformSavepointJumpNowKey() !=
					GetInputKeyboardWaveformSavepointJumpAtMeasureKey()
				) ||
				LGL_KeyDown(LGL_KEY_SHIFT)
			)
		);
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformSavepointJumpAtMeasure
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return
		(
			LGL_KeyStroke(GetInputKeyboardWaveformSavepointJumpAtMeasureKey()) &&
			(
				(
					GetInputKeyboardWaveformSavepointJumpNowKey() !=
					GetInputKeyboardWaveformSavepointJumpAtMeasureKey()
				) ||
				LGL_KeyDown(LGL_KEY_SHIFT)==false
			)
		);
	}
	else
	{
		return(false);
	}
}

float
InputKeyboardObj::
WaveformBPM
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(BPMValue);
	}
	else
	{
		return(-1.0f);
	}
}

const char*
InputKeyboardObj::
WaveformBPMCandidate
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		if(BPMInputBuffer.HasFocus())
		{
			return(BPMValueCandidate);
		}
	}

	return(NULL);
}

void
InputKeyboardObj::
WaveformClearBPMCandidate
(
	unsigned int	target
)
{
	if(target & TARGET_FOCUS)
	{
		BPMInputBuffer.ClearBuffer();
		BPMInputBuffer.ReleaseFocus();
	}
}

void
InputKeyboardObj::
WaveformHintBPMCandidate
(
	unsigned int	target,
	float		bpm
)
{
	if(target & TARGET_FOCUS)
	{
		BPMValueHint=bpm;
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
WaveformQuantizationPeriodHalf
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return
		(
			WaveformLoopAllDebump==false &&
			LGL_KeyStroke(GetInputKeyboardWaveformQuantizationPeriodHalfKey())
		);
	}
	else
	{
		return(false);
	}
}

bool
InputKeyboardObj::
WaveformQuantizationPeriodDouble
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return
		(
			WaveformLoopAllDebump==false &&
			LGL_KeyStroke(GetInputKeyboardWaveformQuantizationPeriodDoubleKey())
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
			LGL_KeyDown(GetInputKeyboardWaveformQuantizationPeriodHalfKey()) &&
			LGL_KeyDown(GetInputKeyboardWaveformQuantizationPeriodDoubleKey())==false
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
			LGL_KeyDown(GetInputKeyboardWaveformQuantizationPeriodDoubleKey()) &&
			LGL_KeyDown(GetInputKeyboardWaveformQuantizationPeriodHalfKey())==false
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
			LGL_KeyDown(GetInputKeyboardWaveformQuantizationPeriodHalfKey()) &&
			LGL_KeyDown(GetInputKeyboardWaveformQuantizationPeriodDoubleKey()) &&
			LGL_KeyTimer(GetInputKeyboardWaveformQuantizationPeriodHalfKey()) >= 1.0f &&
			LGL_KeyTimer(GetInputKeyboardWaveformQuantizationPeriodDoubleKey()) >= 1.0f
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

bool
InputKeyboardObj::
WaveformReverse
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(LGL_KeyDown(GetInputKeyboardWaveformReverseKey()));
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
WaveformVideoSelectLow
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		if(LGL_KeyStroke(GetInputKeyboardWaveformVideoSelectLowKey()))
		{
			return(true);
		}
	}

	return(false);
}

bool
InputKeyboardObj::
WaveformVideoSelectHigh
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		if(LGL_KeyStroke(GetInputKeyboardWaveformVideoSelectHighKey()))
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

float
InputKeyboardObj::
WaveformFreqSenseBrightness
(
	unsigned int	target
)	const
{
	return(-1.0f);
}

bool
InputKeyboardObj::
WaveformAudioInputToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;

	if(target & TARGET_FOCUS)
	{
		if(LGL_KeyStroke(GetInputKeyboardWaveformAudioInputToggleKey()))
		{
			toggle=true;
		}
	}

	return(toggle);
}

bool
InputKeyboardObj::
WaveformVideoAspectRatioNext
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		if(LGL_KeyStroke(GetInputKeyboardWaveformVideoAspectRatioNextKey()))
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
WaveformSync
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		bool keyDown = LGL_KeyDown(GetInputKeyboardWaveformSyncKey());
		if
		(
			SyncDebump
		)
		{
			keyDown = false;
		}
		return(keyDown);
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

