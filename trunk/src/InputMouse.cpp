/*
 *
 * InputMouse.cpp - Input abstraction object
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

#include "InputMouse.h"

InputMouseObj&
GetInputMouse()
{
	static InputMouseObj inputMouse;
	return(inputMouse);
}

InputMouseObj::
InputMouseObj()
{
	FocusNow=-1;
	FocusNext=-1;
	FileIndexHighlightNow=-1;
	FileIndexHighlightNext=-1;
	FileSelectNow=0;
	FileSelectNext=0;
	WaveformVideoAspectRatioNextNow=false;
	WaveformVideoAspectRatioNextNext=false;
	WaveformVideoSelectNow=false;
	WaveformVideoSelectNext=false;
	HoverTarget=GUI_TARGET_NULL;
	DragTarget=GUI_TARGET_NULL;
	DragFloat=-1.0f;
	DragFloatNext=-1.0f;
}

InputMouseObj::
~InputMouseObj()
{
	//
}

//Core

void
InputMouseObj::
NextFrame()
{
	FocusNow=FocusNext;
	FocusNext=-1;

	FileIndexHighlightNow=FileIndexHighlightNext;
	FileIndexHighlightNext=-1;

	FileSelectNow=FileSelectNext;
	FileSelectNext=0;

	WaveformVideoAspectRatioNextNow=WaveformVideoAspectRatioNextNext;
	WaveformVideoAspectRatioNextNext=false;

	WaveformVideoSelectNow=WaveformVideoSelectNext;
	WaveformVideoSelectNext=false;

	HoverTarget=HoverTargetNext;
	HoverTargetNext=GUI_TARGET_NULL;

	if(LGL_MouseRelease(LGL_MOUSE_LEFT))
	{
		DragTarget=GUI_TARGET_NULL;
		DragFloatNext=-1.0f;
	}

	DragFloat=DragFloatNext;
	DragFloatNext=-1.0f;
}

//Global Input

bool
InputMouseObj::
FocusChange()	const
{
	bool change=false;
	return(change);
}

bool
InputMouseObj::
FocusBottom()	const
{
	bool bottom=(FocusNow==1);
	return(bottom);
}

bool
InputMouseObj::
FocusTop()	const
{
	bool top=(FocusNow==0);
	return(top);
}

float
InputMouseObj::
XfaderSpeakers()	const
{
	if(DragTarget==GUI_TARGET_XFADER_LEFT)
	{
		return(DragFloat);
	}
	else
	{
		return(-1.0f);
	}
}

float
InputMouseObj::
XfaderSpeakersDelta()	const
{
	float delta=0.0f;
	return(delta);
}

float
InputMouseObj::
XfaderHeadphones()	const
{
	if(DragTarget==GUI_TARGET_XFADER_RIGHT)
	{
		return(DragFloat);
	}
	else
	{
		return(-1.0f);
	}
}

float
InputMouseObj::
XfaderHeadphonesDelta()	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputMouseObj::
SyncTopToBottom()	const
{
	bool sync=false;
	return(sync);
}

bool
InputMouseObj::
SyncBottomToTop()	const
{
	bool sync=false;
	return(sync);
}

int
InputMouseObj::
MasterToHeadphones()	const
{
	int to=-1;
	return(to);
}

//Mode 0: File Selection

float
InputMouseObj::
FileScroll
(
	unsigned int	target
)	const
{
	float scroll=0.0f;
	return(scroll);
}

int
InputMouseObj::
FileSelect
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(FileSelectNow);
	}
	else
	{
		return(0);
	}
}

bool
InputMouseObj::
FileMarkUnopened
(
	unsigned int	target
)	const
{
	bool mark=false;
	return(mark);
}

bool
InputMouseObj::
FileRefresh
(
	unsigned int	target
)	const
{
	bool refresh=false;
	return(refresh);
}

int
InputMouseObj::
FileIndexHighlight
(
	unsigned int	target
)	const
{
	//FIXME
	if(target & TARGET_FOCUS)
	{
		return(FileIndexHighlightNow);
	}
	else
	{
		return(-1);
	}
}

//Mode 1: Decoding...

bool
InputMouseObj::
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
InputMouseObj::
WaveformEject
(
	unsigned int	target
)	const
{
	int eject=0;
	return(eject);
}

bool
InputMouseObj::
WaveformTogglePause
(
	unsigned int	target
)	const
{
	bool toggle=false;
	return(toggle);
}

float
InputMouseObj::
WaveformNudge
(
	unsigned int	target
)	const
{
	float nudge=0.0f;
	return(nudge);
}

float
InputMouseObj::
WaveformPitchbend
(
	unsigned int	target
)	const
{
	float pitchbend=0.0f;
	return(pitchbend);
}

float
InputMouseObj::
WaveformPitchbendDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

float
InputMouseObj::
WaveformEQLow
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		if(DragTarget==GUI_TARGET_EQ_LOW)
		{
			return(DragFloat);
		}
	}

	return(-1.0f);
}

float
InputMouseObj::
WaveformEQLowDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputMouseObj::
WaveformEQLowKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputMouseObj::
WaveformEQMid
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		if(DragTarget==GUI_TARGET_EQ_MID)
		{
			return(DragFloat);
		}
	}

	return(-1.0f);
}

float
InputMouseObj::
WaveformEQMidDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputMouseObj::
WaveformEQMidKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputMouseObj::
WaveformEQHigh
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		if(DragTarget==GUI_TARGET_EQ_HIGH)
		{
			return(DragFloat);
		}
	}

	return(-1.0f);
}

float
InputMouseObj::
WaveformEQHighDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputMouseObj::
WaveformEQHighKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputMouseObj::
WaveformGain
(
	unsigned int	target
)	const
{
	float gain=-1.0f;
	return(gain);
}

float
InputMouseObj::
WaveformGainDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputMouseObj::
WaveformGainKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputMouseObj::
WaveformVolumeSlider
(
	unsigned int	target
)	const
{
	float volume=-1.0f;
	return(volume);
}

bool
InputMouseObj::
WaveformVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputMouseObj::
WaveformRapidVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputMouseObj::
WaveformRapidSoloInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputMouseObj::
WaveformVolumeSolo
(
	unsigned int	target
)	const
{
	bool solo=false;
	return(solo);
}

float
InputMouseObj::
WaveformRewindFF
(
	unsigned int	target
)	const
{
	float rewindff=0.0f;
	return(rewindff);
}

bool
InputMouseObj::
WaveformRecordHold
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(DragTarget==GUI_TARGET_WAVEFORM);
	}
	else
	{
		return(false);
	}
}

float
InputMouseObj::
WaveformRecordSpeed
(
	unsigned int	target
)	const
{
	float speed=0.0f;
	if(WaveformRecordHold(target))
	{
		speed=LGL_MouseDX()*-175.0f;
	}
	return(speed);
}

bool
InputMouseObj::
WaveformStutter
(
	unsigned int	target
)	const
{
	bool stutter=false;
	return(stutter);
}

float
InputMouseObj::
WaveformStutterPitch
(
	unsigned int	target
)	const
{
	float pitch=-1.0f;
	return(pitch);
}

float
InputMouseObj::
WaveformStutterSpeed
(
	unsigned int	target
)	const
{
	float speed=-1.0f;
	return(speed);
}

bool
InputMouseObj::
WaveformSavePointPrev
(
	unsigned int	target
)	const
{
	bool prev=false;
	return(prev);
}

bool
InputMouseObj::
WaveformSavePointNext
(
	unsigned int	target
)	const
{
	bool next=false;
	return(next);
}

int
InputMouseObj::
WaveformSavePointPick
(
	unsigned int	target
)	const
{
	int pick=-9999;
	if(target & TARGET_FOCUS)
	{
		if(LGL_MouseStroke(LGL_MOUSE_LEFT))
		{
			if(HoverTarget==GUI_TARGET_SAVEPOINT_BPM_ALPHA)
			{
				pick=-2;
			}
			else if(HoverTarget==GUI_TARGET_SAVEPOINT_BPM_OMEGA)
			{
				pick=-1;
			}
			else if(HoverTarget==GUI_TARGET_SAVEPOINT_0)
			{
				pick=0;
			}
			else if(HoverTarget==GUI_TARGET_SAVEPOINT_1)
			{
				pick=1;
			}
			else if(HoverTarget==GUI_TARGET_SAVEPOINT_2)
			{
				pick=2;
			}
			else if(HoverTarget==GUI_TARGET_SAVEPOINT_3)
			{
				pick=3;
			}
			else if(HoverTarget==GUI_TARGET_SAVEPOINT_4)
			{
				pick=4;
			}
			else if(HoverTarget==GUI_TARGET_SAVEPOINT_5)
			{
				pick=5;
			}
			else if(HoverTarget==GUI_TARGET_SAVEPOINT_6)
			{
				pick=6;
			}
			else if(HoverTarget==GUI_TARGET_SAVEPOINT_7)
			{
				pick=7;
			}
			else if(HoverTarget==GUI_TARGET_SAVEPOINT_8)
			{
				pick=8;
			}
			else if(HoverTarget==GUI_TARGET_SAVEPOINT_9)
			{
				pick=9;
			}
		}
	}
	return(pick);
}

bool
InputMouseObj::
WaveformSavePointSet
(
	unsigned int	target
)	const
{
	bool set=false;
	return(set);
}

float
InputMouseObj::
WaveformSavePointUnsetPercent
(
	unsigned int	target
)	const
{
	float percent=0.0f;
	return(percent);
}

float
InputMouseObj::
WaveformSavePointShift
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

float
InputMouseObj::
WaveformSavePointShiftAll
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

bool
InputMouseObj::
WaveformSavePointShiftAllHere
(
	unsigned int	target
)	const
{
	bool here=false;
	return(here);
}

bool
InputMouseObj::
WaveformSavePointJumpNow
(
	unsigned int	target
)	const
{
	bool jump=false;
	return(jump);
}

bool
InputMouseObj::
WaveformSavePointJumpAtMeasure
(
	unsigned int	target
)	const
{
	bool jump=false;
	return(jump);
}

int
InputMouseObj::
WaveformLoopMeasuresExponent
(
	unsigned int	target
)	const
{
	int exponent=WAVEFORM_LOOP_MEASURES_EXPONENT_NULL;
	return(exponent);
}

bool
InputMouseObj::
WaveformLoopMeasuresHalf
(
	unsigned int	target
)	const
{
	bool half=false;
	return(half);
}

bool
InputMouseObj::
WaveformLoopMeasuresDouble
(
	unsigned int	target
)	const
{
	bool twoX=false;
	return(twoX);
}

bool
InputMouseObj::
WaveformLoopSecondsLess
(
	unsigned int	target
)	const
{
	bool less=false;
	return(less);
}

bool
InputMouseObj::
WaveformLoopSecondsMore
(
	unsigned int	target
)	const
{
	bool more=false;
	return(more);
}

bool
InputMouseObj::
WaveformLoopAll
(
	unsigned int	target
)	const
{
	bool all=false;
	return(all);
}

bool
InputMouseObj::
WaveformLoopToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;
	return(toggle);
}

bool
InputMouseObj::
WaveformLoopThenRecallActive
(
	unsigned int	target
)	const
{
	bool active=false;
	return(active);
}

int
InputMouseObj::
WaveformAutoDivergeRecall
(
	unsigned int	target
)	const
{
	int ret=0;
	return(ret);
}

bool
InputMouseObj::
WaveformVideoSelect
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(WaveformVideoSelectNow);
	}
	else
	{
		return(false);
	}
}

float
InputMouseObj::
WaveformVideoBrightness
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		if(DragTarget==GUI_TARGET_VIS_VIDEO)
		{
			return(DragFloat);
		}
	}

	return(-1.0f);
}

float
InputMouseObj::
WaveformVideoAdvanceRate
(
	unsigned int	target
)	const
{
	float rate=-1.0f;
	return(rate);
}

float
InputMouseObj::
WaveformFreqSenseBrightness
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		if(DragTarget==GUI_TARGET_VIS_FREQSENSE)
		{
			return(DragFloat);
		}
	}

	return(-1.0f);
}

int
InputMouseObj::
WaveformAudioInputMode
(
	unsigned int	target
)	const
{
	int mode=-1;
	return(mode);
}

bool
InputMouseObj::
WaveformVideoAspectRatioNext
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		return(WaveformVideoAspectRatioNextNow);
	}
	else
	{
		return(false);
	}
}

float
InputMouseObj::
WaveformOscilloscopeBrightness
(
	unsigned int	target
)	const
{
	if(target & TARGET_FOCUS)
	{
		if(DragTarget==GUI_TARGET_VIS_OSCILLOSCOPE)
		{
			return(DragFloat);
		}
	}

	return(-1.0f);
}

bool
InputMouseObj::
WaveformSyncBPM
(
	unsigned int	target
)	const
{
	bool sync=false;
	return(sync);
}

float
InputMouseObj::
WaveformPointerScratch
(
	unsigned int	target
)	const
{
	float targetX=-1.0f;

	/*
	if(target & TARGET_FOCUS)
	{
		if(LGL_MouseDown(LGL_MOUSE_LEFT))
		{
			targetX=LGL_MouseX();
		}
	}
	*/

	return(targetX);
}

void
InputMouseObj::
SetFocusNext
(
	int	next
)
{
	if
	(
		LGL_MouseDown(LGL_MOUSE_LEFT)==false &&
		LGL_MouseDown(LGL_MOUSE_RIGHT)==false
	)
	{
		FocusNext=next;
	}
}

void
InputMouseObj::
SetFileIndexHighlightNext
(
	int	next
)
{
	FileIndexHighlightNext=next;
}

void
InputMouseObj::
SetFileSelectNext()
{
	FileSelectNext=1;
}

void
InputMouseObj::
SetWaveformVideoAspectRatioNextNext()
{
	WaveformVideoAspectRatioNextNext=true;
}

void
InputMouseObj::
SetWaveformVideoSelectNext()
{
	WaveformVideoSelectNext=true;
}

DVJ_GuiTarget
InputMouseObj::
GetHoverTarget()	const
{
	return(HoverTarget);
}

DVJ_GuiTarget
InputMouseObj::
GetDragTarget()	const
{
	return(DragTarget);
}

void
InputMouseObj::
SetHoverTarget
(
	DVJ_GuiTarget	hoverTarget
)
{
	HoverTargetNext=hoverTarget;
}

void
InputMouseObj::
SetDragTarget
(
	DVJ_GuiTarget	dragTarget
)
{
	DragTarget=dragTarget;
}

void
InputMouseObj::
SetDragFloatNext
(
	float	dragFloat
)
{
	DragFloatNext=dragFloat;
}

