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
	WaveformVideoSelectLowNow=false;
	WaveformVideoSelectLowNext=false;
	WaveformVideoSelectHighNow=false;
	WaveformVideoSelectHighNext=false;
	WaveformLoopToggleNow=false;
	WaveformLoopToggleNext=false;
	HoverOnSelectedSavepointNow=false;
	HoverOnSelectedSavepointNext=false;
	HoverInSavepointsNow=false;
	HoverInSavepointsNext=false;
	EntireWaveformScrubberLength=-1.0f;
	EntireWaveformScrubberPosAlpha=-1.0f;
	EntireWaveformScrubberSpeed=-1.0f;
	EntireWaveformScrubberForceNow=-1.0f;
	EntireWaveformScrubberForceNext=-1.0f;
	HoverElement=GUI_ELEMENT_NULL;
	DragElement=GUI_ELEMENT_NULL;
	DragTarget=0;
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

	WaveformVideoSelectLowNow=WaveformVideoSelectLowNext;
	WaveformVideoSelectLowNext=false;

	WaveformVideoSelectHighNow=WaveformVideoSelectHighNext;
	WaveformVideoSelectHighNext=false;
	
	WaveformLoopToggleNow=WaveformLoopToggleNext;
	WaveformLoopToggleNext=false;

	HoverOnSelectedSavepointNow=HoverOnSelectedSavepointNext;
	HoverOnSelectedSavepointNext=false;

	HoverInSavepointsNow=HoverInSavepointsNext;
	HoverInSavepointsNext=false;

	EntireWaveformScrubberForceNow=EntireWaveformScrubberForceNext;
	EntireWaveformScrubberForceNext=-1.0f;

	HoverElement=HoverElementNext;
	HoverElementNext=GUI_ELEMENT_NULL;

	if(0)
	{
		if(HoverElement == GUI_ELEMENT_NULL)
		{
			//
		}
		else if(HoverElement == GUI_ELEMENT_FILE_SELECT)
		{
			LGL_DebugPrintf("Hover: FILE_SELECT");
		}
		else if(HoverElement == GUI_ELEMENT_WAVEFORM)
		{
			LGL_DebugPrintf("Hover: WAVEFORM");
		}
		else if(HoverElement == GUI_ELEMENT_ENTIRE_WAVEFORM)
		{
			LGL_DebugPrintf("Hover: ENTIRE_WAVEFORM");
		}
		else if(HoverElement == GUI_ELEMENT_XFADER_LEFT)
		{
			LGL_DebugPrintf("Hover: XFADER_LEFT");
		}
		else if(HoverElement == GUI_ELEMENT_XFADER_RIGHT)
		{
			LGL_DebugPrintf("Hover: XFADER_RIGHT");
		}
		else if(HoverElement == GUI_ELEMENT_EQ_LOW)
		{
			LGL_DebugPrintf("Hover: EQ_LOW");
		}
		else if(HoverElement == GUI_ELEMENT_EQ_MID)
		{
			LGL_DebugPrintf("Hover: EQ_MID");
		}
		else if(HoverElement == GUI_ELEMENT_EQ_HIGH)
		{
			LGL_DebugPrintf("Hover: EQ_HIGH");
		}
		else if(HoverElement == GUI_ELEMENT_EQ_GAIN)
		{
			LGL_DebugPrintf("Hover: EQ_GAIN");
		}
		else if(HoverElement == GUI_ELEMENT_VIDEO)
		{
			LGL_DebugPrintf("Hover: VIDEO");
		}
		else if(HoverElement == GUI_ELEMENT_VIDEO_FREQSENSE)
		{
			LGL_DebugPrintf("Hover: VIDEO_FREQSENSE");
		}
		else if(HoverElement == GUI_ELEMENT_SYPHON)
		{
			LGL_DebugPrintf("Hover: SYPHON");
		}
		else if(HoverElement == GUI_ELEMENT_OSCILLOSCOPE)
		{
			LGL_DebugPrintf("Hover: OSCILLOSCOPE");
		}
		else if(HoverElement == GUI_ELEMENT_LED_FREQSENSE)
		{
			LGL_DebugPrintf("Hover: LED_FREQSENSE");
		}
		else if(HoverElement == GUI_ELEMENT_LED_COLOR_LOW)
		{
			LGL_DebugPrintf("Hover: LED_COLOR_LOW");
		}
		else if(HoverElement == GUI_ELEMENT_LED_COLOR_HIGH)
		{
			LGL_DebugPrintf("Hover: LED_COLOR_HIGH");
		}
		else if(HoverElement == GUI_ELEMENT_LED_COLOR_HIGH_WASH)
		{
			LGL_DebugPrintf("Hover: LED_COLOR_HIGH_WASH");
		}
		else if(HoverElement == GUI_ELEMENT_LED_GROUP)
		{
			LGL_DebugPrintf("Hover: LED_GROUP");
		}
		else if(HoverElement == GUI_ELEMENT_SAVEPOINT_0)
		{
			LGL_DebugPrintf("Hover: SAVEPOINT_0");
		}
		else if(HoverElement == GUI_ELEMENT_SAVEPOINT_1)
		{
			LGL_DebugPrintf("Hover: SAVEPOINT_1");
		}
		else if(HoverElement == GUI_ELEMENT_SAVEPOINT_2)
		{
			LGL_DebugPrintf("Hover: SAVEPOINT_2");
		}
		else if(HoverElement == GUI_ELEMENT_SAVEPOINT_3)
		{
			LGL_DebugPrintf("Hover: SAVEPOINT_3");
		}
		else if(HoverElement == GUI_ELEMENT_SAVEPOINT_4)
		{
			LGL_DebugPrintf("Hover: SAVEPOINT_4");
		}
		else if(HoverElement == GUI_ELEMENT_SAVEPOINT_5)
		{
			LGL_DebugPrintf("Hover: SAVEPOINT_5");
		}
		else if(HoverElement == GUI_ELEMENT_SAVEPOINT_6)
		{
			LGL_DebugPrintf("Hover: SAVEPOINT_6");
		}
		else if(HoverElement == GUI_ELEMENT_SAVEPOINT_7)
		{
			LGL_DebugPrintf("Hover: SAVEPOINT_7");
		}
		else if(HoverElement == GUI_ELEMENT_SAVEPOINT_8)
		{
			LGL_DebugPrintf("Hover: SAVEPOINT_8");
		}
		else if(HoverElement == GUI_ELEMENT_SAVEPOINT_9)
		{
			LGL_DebugPrintf("Hover: SAVEPOINT_9");
		}
		else if(HoverElement == GUI_ELEMENT_SAVEPOINT_A)
		{
			LGL_DebugPrintf("Hover: SAVEPOINT_A");
		}
		else if(HoverElement == GUI_ELEMENT_SAVEPOINT_B)
		{
			LGL_DebugPrintf("Hover: SAVEPOINT_B");
		}
		else if(HoverElement == GUI_ELEMENT_SAVEPOINT_POS)
		{
			LGL_DebugPrintf("Hover: SAVEPOINT_POS");
		}
		else if(HoverElement == GUI_ELEMENT_SAVEPOINT_BPM)
		{
			LGL_DebugPrintf("Hover: SAVEPOINT_BPM");
		}
		else if(HoverElement == GUI_ELEMENT_LOOP_MEASURES)
		{
			LGL_DebugPrintf("Hover: LOOP_MEASURES");
		}
		else if(HoverElement == GUI_ELEMENT_BPM)
		{
			LGL_DebugPrintf("Hover: BPM");
		}
		else if(HoverElement == GUI_ELEMENT_PITCH)
		{
			LGL_DebugPrintf("Hover: PITCH");
		}
	}
	
	if(LGL_MouseRelease(LGL_MOUSE_LEFT))
	{
		DragTarget=TARGET_NONE;
		DragElement=GUI_ELEMENT_NULL;
		DragFloatNext=-1.0f;
	}

	DragFloat=DragFloatNext;
	DragFloatNext=-1.0f;

	if(LGL_MouseMotion())
	{
		MouseMotionTimer.Reset();
	}
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
	if(DragElement==GUI_ELEMENT_XFADER_LEFT)
	{
		return(DragFloat);
	}

	return(-1.0f);
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
	if(DragElement==GUI_ELEMENT_XFADER_RIGHT)
	{
		return(DragFloat);
	}

	return(-1.0f);
}

float
InputMouseObj::
XfaderHeadphonesDelta()	const
{
	float delta=0.0f;
	return(delta);
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
WaveformPauseToggle
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
	float pitchbend=DVJ_INPUT_NIL;
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
	if(target & DragTarget)
	{
		if(DragElement==GUI_ELEMENT_EQ_LOW)
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
	if(target & DragTarget)
	{
		if(DragElement==GUI_ELEMENT_EQ_MID)
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
	if(target & DragTarget)
	{
		if(DragElement==GUI_ELEMENT_EQ_HIGH)
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
	if(target & DragTarget)
	{
		if(DragElement==GUI_ELEMENT_EQ_GAIN)
		{
			if(DragFloat!=-1.0f)
			{
				return(DragFloat*2.0f);
			}
		}
	}

	return(-1.0f);
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
WaveformRhythmicVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputMouseObj::
WaveformRhythmicVolumeInvertOther
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
	if(target & DragTarget)
	{
		return(DragElement==GUI_ELEMENT_WAVEFORM);
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
	if(target & DragTarget)
	{
		if(WaveformRecordHold(target))
		{
			speed=LGL_MouseDX()*175.0f;
		}
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
WaveformSavepointPrev
(
	unsigned int	target
)	const
{
	bool prev=false;
	return(prev);
}

bool
InputMouseObj::
WaveformSavepointNext
(
	unsigned int	target
)	const
{
	bool next=false;
	return(next);
}

int
InputMouseObj::
WaveformSavepointPick
(
	unsigned int	target
)	const
{
	int pick=-9999;
	if(target & DragTarget)
	{
		if(LGL_MouseStroke(LGL_MOUSE_LEFT))
		{
			if(HoverElement==GUI_ELEMENT_SAVEPOINT_0)
			{
				pick=0;
			}
			else if(HoverElement==GUI_ELEMENT_SAVEPOINT_1)
			{
				pick=1;
			}
			else if(HoverElement==GUI_ELEMENT_SAVEPOINT_2)
			{
				pick=2;
			}
			else if(HoverElement==GUI_ELEMENT_SAVEPOINT_3)
			{
				pick=3;
			}
			else if(HoverElement==GUI_ELEMENT_SAVEPOINT_4)
			{
				pick=4;
			}
			else if(HoverElement==GUI_ELEMENT_SAVEPOINT_5)
			{
				pick=5;
			}
			else if(HoverElement==GUI_ELEMENT_SAVEPOINT_6)
			{
				pick=6;
			}
			else if(HoverElement==GUI_ELEMENT_SAVEPOINT_7)
			{
				pick=7;
			}
			else if(HoverElement==GUI_ELEMENT_SAVEPOINT_8)
			{
				pick=8;
			}
			else if(HoverElement==GUI_ELEMENT_SAVEPOINT_9)
			{
				pick=9;
			}
			else if(HoverElement==GUI_ELEMENT_SAVEPOINT_A)
			{
				pick=10;
			}
			else if(HoverElement==GUI_ELEMENT_SAVEPOINT_B)
			{
				pick=11;
			}
		}
	}
	return(pick);
}

bool
InputMouseObj::
WaveformSavepointSet
(
	unsigned int	target
)	const
{
	bool set=false;
	if(target & DragTarget)
	{
		if(LGL_KeyDown(LGL_KEY_SHIFT))
		{
			if(LGL_MouseStroke(LGL_MOUSE_LEFT))
			{
				if(HoverOnSelectedSavepointNow)
				{
					set=true;
				}
			}
		}
	}
	return(set);
}

float
InputMouseObj::
WaveformSavepointUnsetPercent
(
	unsigned int	target
)	const
{
	float percent=0.0f;
	if(target & DragTarget)
	{
		if(LGL_KeyDown(LGL_KEY_SHIFT))
		{
			if(LGL_MouseDown(LGL_MOUSE_LEFT))
			{
				if(HoverOnSelectedSavepointNow)
				{
					percent = LGL_Clamp
					(
						0.0f,
						LGL_Min
						(
							LGL_MouseTimer(LGL_MOUSE_LEFT),
							MouseMotionTimer.SecondsSinceLastReset()
						),
						1.0f
					);
				}
			}
		}
	}
	return(percent);
}

float
InputMouseObj::
WaveformSavepointShift
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

float
InputMouseObj::
WaveformSavepointShiftAll
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

bool
InputMouseObj::
WaveformSavepointShiftAllHere
(
	unsigned int	target
)	const
{
	bool here=false;
	return(here);
}

bool
InputMouseObj::
WaveformSavepointJumpNow
(
	unsigned int	target
)	const
{
	bool jump=false;
	return(jump);
}

bool
InputMouseObj::
WaveformSavepointJumpAtMeasure
(
	unsigned int	target
)	const
{
	bool jump=false;
	return(jump);
}

float
InputMouseObj::
WaveformJumpToPercent
(
	unsigned int	target
)	const
{
	float percent=-1.0f;

	if(target & DragTarget)
	{
		if(DragElement==GUI_ELEMENT_ENTIRE_WAVEFORM)
		{
			percent=DragFloat;
		}

		if(EntireWaveformScrubberForceNow!=-1.0f)
		{
			percent=EntireWaveformScrubberForceNow;
		}
	}

	return(percent);
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
WaveformQuantizationPeriodHalf
(
	unsigned int	target
)	const
{
	bool half=false;
	return(half);
}

bool
InputMouseObj::
WaveformQuantizationPeriodDouble
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
	if(target & DragTarget)
	{
		return(WaveformLoopToggleNow);
	}
	else
	{
		return(false);
	}
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
WaveformVideoSelectLow
(
	unsigned int	target
)	const
{
	if(target & DragTarget)
	{
		return(WaveformVideoSelectLowNow);
	}
	else
	{
		return(false);
	}
}

bool
InputMouseObj::
WaveformVideoSelectHigh
(
	unsigned int	target
)	const
{
	if(target & DragTarget)
	{
		return(WaveformVideoSelectHighNow);
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
	if(target & DragTarget)
	{
		if(DragElement==GUI_ELEMENT_VIDEO)
		{
			return(DragFloat);
		}
	}

	return(-1.0f);
}

float
InputMouseObj::
WaveformSyphonBrightness
(
	unsigned int	target
)	const
{
	if(target & DragTarget)
	{
		if(DragElement==GUI_ELEMENT_SYPHON)
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
	if(target & DragTarget)
	{
		if(DragElement==GUI_ELEMENT_VIDEO_FREQSENSE)
		{
			return(DragFloat);
		}
	}

	return(-1.0f);
}

float
InputMouseObj::
WaveformFreqSenseLEDGroupFloat
(
	unsigned int	target
)	const
{
	if(target & DragTarget)
	{
		if(DragElement==GUI_ELEMENT_LED_GROUP)
		{
			return(DragFloat);
		}
	}

	return(-1.0f);
}

float
InputMouseObj::
WaveformFreqSenseLEDColorScalarLow
(
	unsigned int	target
)	const
{
	if(target & DragTarget)
	{
		if(DragElement==GUI_ELEMENT_LED_COLOR_LOW)
		{
			return(DragFloat);
		}
	}

	return(-1.0f);
}

float
InputMouseObj::
WaveformFreqSenseLEDColorScalarHigh
(
	unsigned int	target
)	const
{
	if(target & DragTarget)
	{
		if(DragElement==GUI_ELEMENT_LED_COLOR_HIGH)
		{
			return(DragFloat);
		}
	}

	return(-1.0f);
}

float
InputMouseObj::
WaveformFreqSenseLEDBrightness
(
	unsigned int	target
)	const
{
	if(target & DragTarget)
	{
		if(DragElement==GUI_ELEMENT_LED_FREQSENSE)
		{
			return(DragFloat);
		}
	}

	return(-1.0f);
}

float
InputMouseObj::
WaveformFreqSenseLEDBrightnessWash
(
	unsigned int	target
)	const
{
	if(target & DragTarget)
	{
		if(DragElement==GUI_ELEMENT_LED_COLOR_HIGH_WASH)
		{
			return(DragFloat);
		}
	}

	return(-1.0f);
}

bool
InputMouseObj::
WaveformAudioInputToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;
	return(toggle);
}

bool
InputMouseObj::
WaveformVideoAspectRatioNext
(
	unsigned int	target
)	const
{
	if(target & DragTarget)
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
	if(target & DragTarget)
	{
		if(DragElement==GUI_ELEMENT_OSCILLOSCOPE)
		{
			return(DragFloat);
		}
	}

	return(-1.0f);
}

bool
InputMouseObj::
WaveformSync
(
	unsigned int	target
)	const
{
	bool sync=false;
	if(target & DragTarget)
	{
		if
		(
			HoverElement == GUI_ELEMENT_BPM ||
			HoverElement == GUI_ELEMENT_PITCH
		)
		{
			if(LGL_MouseDown(LGL_MOUSE_LEFT))
			{
				sync=true;
			}
		}
	}
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
	if(target & DragTarget)
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
		LGL_MouseDown(LGL_MOUSE_RIGHT)==false &&
		LGL_KeyDown(LGL_KEY_LCTRL)==false
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
SetWaveformVideoSelectLowNext()
{
	WaveformVideoSelectLowNext=true;
}

void
InputMouseObj::
SetWaveformVideoSelectHighNext()
{
	WaveformVideoSelectHighNext=true;
}

void
InputMouseObj::
SetWaveformLoopToggleNext()
{
	WaveformLoopToggleNext=true;
}

bool
InputMouseObj::
GetHoverOnSelectedSavepoint()
{
	return(HoverOnSelectedSavepointNow);
}

void
InputMouseObj::
SetHoverOnSelectedSavepoint()
{
	HoverOnSelectedSavepointNext=true;;
}

bool
InputMouseObj::
GetHoverInSavepoints()
{
	return(HoverInSavepointsNow);
}

void
InputMouseObj::
SetHoverInSavepoints()
{
	HoverInSavepointsNext=true;;
}

void
InputMouseObj::
EntireWaveformScrubberAlpha
(
	float	length,
	float	posNow,
	float	speed
)
{
	if
	(
		EntireWaveformScrubberLength==-1.0f &&
		EntireWaveformScrubberPosAlpha==-1.0f &&
		EntireWaveformScrubberSpeed==-1.0f
	)
	{
		EntireWaveformScrubberLength=length;
		EntireWaveformScrubberPosAlpha=posNow;
		EntireWaveformScrubberSpeed=speed;
		EntireWaveformScrubberTimer.Reset();
	}
}

void
InputMouseObj::
EntireWaveformScrubberOmega()
{
	if
	(
		EntireWaveformScrubberLength!=-1.0f &&
		EntireWaveformScrubberPosAlpha!=-1.0f &&
		EntireWaveformScrubberSpeed!=-1.0f
	)
	{
		EntireWaveformScrubberForceNext=1.0f+GetEntireWaveformScrubberRecallPercent();
	}

	EntireWaveformScrubberLength=-1.0f;
	EntireWaveformScrubberPosAlpha=-1.0f;
	EntireWaveformScrubberSpeed=-1.0f;
}

bool
InputMouseObj::
GetEntireWaveformScrubberDelta()
{
	return
	(
		EntireWaveformScrubberLength!=-1.0f ||
		EntireWaveformScrubberPosAlpha!=-1.0f ||
		EntireWaveformScrubberSpeed!=-1.0f ||
		EntireWaveformScrubberForceNow!=-1.0f ||
		EntireWaveformScrubberForceNext!=-1.0f
	);
}

float
InputMouseObj::
GetEntireWaveformScrubberRecallPercent()
{
	float ret=
	LGL_Clamp
	(
		0.0f,
		(
			EntireWaveformScrubberPosAlpha+
			(
				EntireWaveformScrubberTimer.SecondsSinceLastReset()*
				EntireWaveformScrubberSpeed
			)
		) / EntireWaveformScrubberLength,
		1.0f
	);
	return(ret);
}

void
InputMouseObj::
SetEntireWaveformScrubberForceNext
(
	float pct
)
{
	if(GetEntireWaveformScrubberDelta())
	{
		EntireWaveformScrubberForceNext=LGL_Clamp(0.0f,pct,1.0f);
	}
}

DVJ_GuiElement
InputMouseObj::
GetHoverElement()	const
{
	return(HoverElement);
}

int
InputMouseObj::
GetDragTarget()		const
{
	return(DragTarget);
}

DVJ_GuiElement
InputMouseObj::
GetDragElement()	const
{
	return(DragElement);
}

void
InputMouseObj::
SetDragTarget
(
	int	target
)
{
	DragTarget=target;
}

void
InputMouseObj::
SetHoverElement
(
	DVJ_GuiElement	hoverElement
)
{
	HoverElementNext=hoverElement;
}

void
InputMouseObj::
SetDragElement
(
	DVJ_GuiElement	dragElement
)
{
	DragElement=dragElement;
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

