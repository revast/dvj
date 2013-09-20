/*
 *
 * Input.cpp - Input abstraction object
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

#include "Input.h"

InputObj&
GetInput()
{
	static InputObj input;
	return(input);
}

float	InputFocusChange(unsigned int target) { return(GetInput().FocusChange()); }
float	InputFocusBottom(unsigned int target) { return(GetInput().FocusBottom()); }
float	InputFocusTop(unsigned int target) { return(GetInput().FocusTop()); }
float	InputXfaderSpeakers(unsigned int target) { return(GetInput().XfaderSpeakers()); }
float	InputXfaderSpeakersDelta(unsigned int target) { return(GetInput().XfaderSpeakersDelta()); }
float	InputXfaderHeadphones(unsigned int target) { return(GetInput().XfaderHeadphones()); }
float	InputXfaderHeadphonesDelta(unsigned int target) { return(GetInput().XfaderHeadphonesDelta()); }
float	InputMasterToHeadphones(unsigned int target) { return(GetInput().MasterToHeadphones()); }
float	InputFileScroll(unsigned int target) { return(GetInput().FileScroll(target)); }
float	InputFileSelect(unsigned int target) { return(GetInput().FileSelect(target)); }
float	InputFileMarkUnopened(unsigned int target) { return(GetInput().FileMarkUnopened(target)); }
float	InputFileRefresh(unsigned int target) { return(GetInput().FileRefresh(target)); }
float	InputFileIndexHighlight(unsigned int target) { return(GetInput().FileIndexHighlight(target)); }
float	InputWaveformEject(unsigned int target) { return(GetInput().WaveformEject(target)); }
float	InputWaveformPauseToggle(unsigned int target) { return(GetInput().WaveformPauseToggle(target)); }
float	InputWaveformNudge(unsigned int target) { return(GetInput().WaveformNudge(target)); }
float	InputWaveformPitchbend(unsigned int target) { return(GetInput().WaveformPitchbend(target)); }
float	InputWaveformPitchbendDelta(unsigned int target) { return(GetInput().WaveformPitchbendDelta(target)); }
float	InputWaveformEQLow(unsigned int target) { return(GetInput().WaveformEQLow(target)); }
float	InputWaveformEQLowDelta(unsigned int target) { return(GetInput().WaveformEQLowDelta(target)); }
float	InputWaveformEQLowKill(unsigned int target) { return(GetInput().WaveformEQLowKill(target)); }
float	InputWaveformEQMid(unsigned int target) { return(GetInput().WaveformEQMid(target)); }
float	InputWaveformEQMidDelta(unsigned int target) { return(GetInput().WaveformEQMidDelta(target)); }
float	InputWaveformEQMidKill(unsigned int target) { return(GetInput().WaveformEQMidKill(target)); }
float	InputWaveformEQHigh(unsigned int target) { return(GetInput().WaveformEQHigh(target)); }
float	InputWaveformEQHighDelta(unsigned int target) { return(GetInput().WaveformEQHighDelta(target)); }
float	InputWaveformEQHighKill(unsigned int target) { return(GetInput().WaveformEQHighKill(target)); }
float	InputWaveformGain(unsigned int target) { return(GetInput().WaveformGain(target)); }
float	InputWaveformGainDelta(unsigned int target) { return(GetInput().WaveformGainDelta(target)); }
float	InputWaveformGainKill(unsigned int target) { return(GetInput().WaveformGainKill(target)); }
float	InputWaveformVolumeSlider(unsigned int target) { return(GetInput().WaveformVolumeSlider(target)); }
float	InputWaveformVolumeInvert(unsigned int target) { return(GetInput().WaveformVolumeInvert(target)); }
float	InputWaveformRhythmicVolumeInvert(unsigned int target) { return(GetInput().WaveformRhythmicVolumeInvert(target)); }
float	InputWaveformRhythmicVolumeInvertOther(unsigned int target) { return(GetInput().WaveformRhythmicVolumeInvertOther(target)); }
float	InputWaveformVolumeSolo(unsigned int target) { return(GetInput().WaveformVolumeSolo(target)); }
float	InputWaveformRewindFF(unsigned int target) { return(GetInput().WaveformRewindFF(target)); }
float	InputWaveformRecordHold(unsigned int target) { return(GetInput().WaveformRecordHold(target)); }
float	InputWaveformRecordSpeed(unsigned int target) { return(GetInput().WaveformRecordSpeed(target)); }
float	InputWaveformStutter(unsigned int target) { return(GetInput().WaveformStutter(target)); }
float	InputWaveformStutterPitch(unsigned int target) { return(GetInput().WaveformStutterPitch(target)); }
float	InputWaveformStutterSpeed(unsigned int target) { return(GetInput().WaveformStutterSpeed(target)); }
float	InputWaveformSavepointPrev(unsigned int target) { return(GetInput().WaveformSavepointPrev(target)); }
float	InputWaveformSavepointNext(unsigned int target) { return(GetInput().WaveformSavepointNext(target)); }
float	InputWaveformSavepointPick(unsigned int target) { return(GetInput().WaveformSavepointPick(target)); }
float	InputWaveformSavepointSet(unsigned int target) { return(GetInput().WaveformSavepointSet(target)); }
float	InputWaveformSavepointSetBPMAtNeedle(unsigned int target) { return(GetInput().WaveformSavepointSetBPMAtNeedle(target)); }
float	InputWaveformSavepointSetBPMUndef(unsigned int target) { return(GetInput().WaveformSavepointSetBPMUndef(target)); }
float	InputWaveformSavepointSetBPMNone(unsigned int target) { return(GetInput().WaveformSavepointSetBPMNone(target)); }
float	InputWaveformSavepointUnsetPercent(unsigned int target) { return(GetInput().WaveformSavepointUnsetPercent(target)); }
float	InputWaveformSavepointShift(unsigned int target) { return(GetInput().WaveformSavepointShift(target)); }
float	InputWaveformSavepointShiftAll(unsigned int target) { return(GetInput().WaveformSavepointShiftAll(target)); }
float	InputWaveformSavepointShiftAllHere(unsigned int target) { return(GetInput().WaveformSavepointShiftAllHere(target)); }
float	InputWaveformSavepointShiftBPM(unsigned int target) { return(GetInput().WaveformSavepointShiftBPM(target)); }
float	InputWaveformSavepointJumpNow(unsigned int target) { return(GetInput().WaveformSavepointJumpNow(target)); }
float	InputWaveformSavepointJumpAtMeasure(unsigned int target) { return(GetInput().WaveformSavepointJumpAtMeasure(target)); }
float	InputWaveformBPM(unsigned int target) { return(GetInput().WaveformBPM(target)); }
const char*
	InputWaveformBPMCandidate(unsigned int target) { return(GetInput().WaveformBPMCandidate(target)); }
void	InputWaveformClearBPMCandidate(unsigned int target) { GetInput().WaveformClearBPMCandidate(target); }
void	InputWaveformHintBPMCandidate(unsigned int target, float bpm) { GetInput().WaveformHintBPMCandidate(target,bpm); }
float	InputWaveformJumpToPercent(unsigned int target) { return(GetInput().WaveformJumpToPercent(target)); }
float	InputWaveformLoopMeasuresExponent(unsigned int target) { return(GetInput().WaveformLoopMeasuresExponent(target)); }
float	InputWaveformQuantizationPeriodHalf(unsigned int target) { return(GetInput().WaveformQuantizationPeriodHalf(target)); }
float	InputWaveformQuantizationPeriodDouble(unsigned int target) { return(GetInput().WaveformQuantizationPeriodDouble(target)); }
float	InputWaveformLoopSecondsLess(unsigned int target) { return(GetInput().WaveformLoopSecondsLess(target)); }
float	InputWaveformLoopSecondsMore(unsigned int target) { return(GetInput().WaveformLoopSecondsMore(target)); }
float	InputWaveformLoopAll(unsigned int target) { return(GetInput().WaveformLoopAll(target)); }
float	InputWaveformLoopToggle(unsigned int target) { return(GetInput().WaveformLoopToggle(target)); }
float	InputWaveformLoopThenRecallActive(unsigned int target) { return(GetInput().WaveformLoopThenRecallActive(target)); }
float	InputWaveformReverse(unsigned int target) { return(GetInput().WaveformReverse(target)); }
float	InputWaveformAutoDivergeRecall(unsigned int target) { return(GetInput().WaveformAutoDivergeRecall(target)); }
float	InputWaveformVideoSelectLow(unsigned int target) { return(GetInput().WaveformVideoSelectLow(target)); }
float	InputWaveformVideoSelectHigh(unsigned int target) { return(GetInput().WaveformVideoSelectHigh(target)); }
float	InputWaveformVideoBrightness(unsigned int target) { return(GetInput().WaveformVideoBrightness(target)); }
float	InputWaveformVideoBrightnessDelta(unsigned int target) { return(GetInput().WaveformVideoBrightnessDelta(target)); }
float	InputWaveformSyphonBrightness(unsigned int target) { return(GetInput().WaveformSyphonBrightness(target)); }
float	InputWaveformSyphonBrightnessDelta(unsigned int target) { return(GetInput().WaveformSyphonBrightnessDelta(target)); }
float	InputWaveformVideoAdvanceRate(unsigned int target) { return(GetInput().WaveformVideoAdvanceRate(target)); }
float	InputWaveformFreqSenseBrightness(unsigned int target) { return(GetInput().WaveformFreqSenseBrightness(target)); }
float	InputWaveformFreqSenseBrightnessDelta(unsigned int target) { return(GetInput().WaveformFreqSenseBrightnessDelta(target)); }
float	InputWaveformFreqSenseLEDGroupFloat(unsigned int target) { return(GetInput().WaveformFreqSenseLEDGroupFloat(target)); }
float	InputWaveformFreqSenseLEDGroupFloatDelta(unsigned int target) { return(GetInput().WaveformFreqSenseLEDGroupFloatDelta(target)); }
float	InputWaveformFreqSenseLEDColorScalarLow(unsigned int target) { return(GetInput().WaveformFreqSenseLEDColorScalarLow(target)); }
float	InputWaveformFreqSenseLEDColorScalarLowDelta(unsigned int target) { return(GetInput().WaveformFreqSenseLEDColorScalarLowDelta(target)); }
float	InputWaveformFreqSenseLEDColorScalarHigh(unsigned int target) { return(GetInput().WaveformFreqSenseLEDColorScalarHigh(target)); }
float	InputWaveformFreqSenseLEDColorScalarHighDelta(unsigned int target) { return(GetInput().WaveformFreqSenseLEDColorScalarHighDelta(target)); }
float	InputWaveformFreqSenseLEDBrightness(unsigned int target) { return(GetInput().WaveformFreqSenseLEDBrightness(target)); }
float	InputWaveformFreqSenseLEDBrightnessDelta(unsigned int target) { return(GetInput().WaveformFreqSenseLEDBrightnessDelta(target)); }
float	InputWaveformFreqSenseLEDBrightnessWash(unsigned int target) { return(GetInput().WaveformFreqSenseLEDBrightnessWash(target)); }
float	InputWaveformFreqSenseLEDBrightnessWashDelta(unsigned int target) { return(GetInput().WaveformFreqSenseLEDBrightnessWashDelta(target)); }
float	InputWaveformAudioInputToggle(unsigned int target) { return(GetInput().WaveformAudioInputToggle(target)); }
float	InputWaveformVideoAspectRatioNext(unsigned int target) { return(GetInput().WaveformVideoAspectRatioNext(target)); }
float	InputWaveformOscilloscopeBrightness(unsigned int target) { return(GetInput().WaveformOscilloscopeBrightness(target)); }
float	InputWaveformOscilloscopeBrightnessDelta(unsigned int target) { return(GetInput().WaveformOscilloscopeBrightnessDelta(target)); }
float	InputWaveformSync(unsigned int target) { return(GetInput().WaveformSync(target)); }
float	InputWaveformPointerScratch(unsigned int target) { return(GetInput().WaveformPointerScratch(target)); }

InputObj::
InputObj()
{
	//
}

InputObj::
~InputObj()
{
	//
}

//Core

void
InputObj::
AddChild
(
	InputObj*	child
)
{
	Children.push_back(child);
}

void
InputObj::
NextFrame()
{
	for(unsigned int a=0;a<Children.size();a++)
	{
		Children[a]->NextFrame();
	}
}

//Global Input

bool
InputObj::
FocusChange()	const
{
	bool change=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		change|=Children[a]->FocusChange();
	}

	return(change);
}

bool
InputObj::
FocusBottom()	const
{
	bool bottom=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		bottom|=Children[a]->FocusBottom();
	}

	return(bottom);
}

bool
InputObj::
FocusTop()	const
{
	bool top=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		top|=Children[a]->FocusTop();
	}

	return(top);
}

float
InputObj::
XfaderSpeakers()	const
{
	float xfade=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->XfaderSpeakers();
		if(candidate!=-1.0f)
		{
			xfade=candidate;
		}
	}

	return(xfade);
}

float
InputObj::
XfaderSpeakersDelta()	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->XfaderSpeakersDelta();
	}

	return(delta);
}

float
InputObj::
XfaderHeadphones()	const
{
	float xfade=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->XfaderHeadphones();
		if(candidate!=-1.0f)
		{
			xfade=candidate;
		}
	}

	return(xfade);
}

float
InputObj::
XfaderHeadphonesDelta()	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->XfaderHeadphonesDelta();
	}

	return(delta);
}

int
InputObj::
MasterToHeadphones()	const
{
	int to=-1;

	for(unsigned int a=0;a<Children.size();a++)
	{
		int candidate=Children[a]->MasterToHeadphones();
		if(candidate!=-1)
		{
			to=candidate;
		}
	}

	return(to);
}

//Mode 0: File Selection

float
InputObj::
FileScroll
(
	unsigned int	target
)	const
{
	float scroll=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		scroll+=Children[a]->FileScroll(target);
	}

	return(scroll);
}

int
InputObj::
FileSelect
(
	unsigned int	target
)	const
{
	int choose=0;

	for(unsigned int a=0;a<Children.size();a++)
	{
		int val=Children[a]->FileSelect(target);
		if(val != 0)
		{
			choose = val;
		}
	}

	return(choose);
}

bool
InputObj::
FileMarkUnopened
(
	unsigned int	target
)	const
{
	bool mark=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		mark|=Children[a]->FileMarkUnopened(target);
	}

	return(mark);
}

bool
InputObj::
FileRefresh
(
	unsigned int	target
)	const
{
	bool refresh=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		refresh|=Children[a]->FileRefresh(target);
	}

	return(refresh);
}

int
InputObj::
FileIndexHighlight
(
	unsigned int	target
)	const
{
	int highlight=-1;

	for(unsigned int a=0;a<Children.size();a++)
	{
		int val=Children[a]->FileIndexHighlight(target);
		if(val != -1)
		{
			highlight = val;
			break;
		}
	}

	return(highlight);
}



//Mode 2: Waveform

int
InputObj::
WaveformEject
(
	unsigned int	target
)	const
{
	int eject=0;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		int val=Children[a]->WaveformEject(target);
		if(val > eject)
		{
			eject = val;
		}
	}

	return(eject);
}

bool
InputObj::
WaveformPauseToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		toggle|=Children[a]->WaveformPauseToggle(target);
	}

	return(toggle);
}

float
InputObj::
WaveformNudge
(
	unsigned int	target
)	const
{
	float nudge=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		nudge+=Children[a]->WaveformNudge(target);
	}

	return(nudge);
}

float
InputObj::
WaveformPitchbend
(
	unsigned int	target
)	const
{
	float pitchbend=DVJ_INPUT_NIL;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformPitchbend(target);
		if(candidate!=DVJ_INPUT_NIL)
		{
			pitchbend=candidate;
		}
	}

	return(pitchbend);
}

float
InputObj::
WaveformPitchbendDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformPitchbendDelta(target);
	}

	return(delta);
}

float
InputObj::
WaveformEQLow
(
	unsigned int	target
)	const
{
	float low=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformEQLow(target);
		if(candidate!=-1.0f)
		{
			low=candidate;
		}
	}

	return(low);
}

float
InputObj::
WaveformEQLowDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformEQLowDelta(target);
	}

	return(delta);
}

bool
InputObj::
WaveformEQLowKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		kill|=Children[a]->WaveformEQLowKill(target);
	}

	return(kill);
}

float
InputObj::
WaveformEQMid
(
	unsigned int	target
)	const
{
	float mid=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformEQMid(target);
		if(candidate!=-1.0f)
		{
			mid=candidate;
		}
	}

	return(mid);
}

float
InputObj::
WaveformEQMidDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformEQMidDelta(target);
	}

	return(delta);
}

bool
InputObj::
WaveformEQMidKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		kill|=Children[a]->WaveformEQMidKill(target);
	}

	return(kill);
}

float
InputObj::
WaveformEQHigh
(
	unsigned int	target
)	const
{
	float high=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformEQHigh(target);
		if(candidate!=-1.0f)
		{
			high=candidate;
		}
	}

	return(high);
}

float
InputObj::
WaveformEQHighDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformEQHighDelta(target);
	}

	return(delta);
}

bool
InputObj::
WaveformEQHighKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		kill|=Children[a]->WaveformEQHighKill(target);
	}

	return(kill);
}

float
InputObj::
WaveformGain
(
	unsigned int	target
)	const
{
	float gain=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformGain(target);
		if(candidate!=-1.0f)
		{
			gain=candidate;
		}
	}

	return(gain);
}

float
InputObj::
WaveformGainDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformGainDelta(target);
	}

	return(delta);
}

bool
InputObj::
WaveformGainKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		kill|=Children[a]->WaveformGainKill(target);
	}

	return(kill);
}

float
InputObj::
WaveformVolumeSlider
(
	unsigned int	target
)	const
{
	float volume=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformVolumeSlider(target);
		if(candidate!=-1.0f)
		{
			volume=candidate;
		}
	}

	return(volume);
}

bool
InputObj::
WaveformVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		invert|=Children[a]->WaveformVolumeInvert(target);
	}

	return(invert);
}

bool
InputObj::
WaveformRhythmicVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		invert|=Children[a]->WaveformRhythmicVolumeInvert(target);
	}

	return(invert);
}

bool
InputObj::
WaveformRhythmicVolumeInvertOther
(
	unsigned int	target
)	const
{
	bool invert=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		invert|=Children[a]->WaveformRhythmicVolumeInvertOther(target);
	}

	return(invert);
}

bool
InputObj::
WaveformVolumeSolo
(
	unsigned int	target
)	const
{
	bool solo=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		solo|=Children[a]->WaveformVolumeSolo(target);
	}

	return(solo);
}

float
InputObj::
WaveformRewindFF
(
	unsigned int	target
)	const
{
	float rewindff=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		rewindff+=Children[a]->WaveformRewindFF(target);
	}

	return(rewindff);
}

bool
InputObj::
WaveformRecordHold
(
	unsigned int	target
)	const
{
	bool hold=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		hold|=Children[a]->WaveformRecordHold(target);
	}

	return(hold);
}

float
InputObj::
WaveformRecordSpeed
(
	unsigned int	target
)	const
{
	float speed=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		speed+=Children[a]->WaveformRecordSpeed(target);
	}

	return(speed);
}

bool
InputObj::
WaveformStutter
(
	unsigned int	target
)	const
{
	bool stutter=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		stutter|=Children[a]->WaveformStutter(target);
	}

	return(stutter);
}

float
InputObj::
WaveformStutterPitch
(
	unsigned int	target
)	const
{
	float pitch=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformStutterPitch(target);
		if(candidate!=-1.0f)
		{
			pitch=candidate;
		}
	}

	return(pitch);
}

float
InputObj::
WaveformStutterSpeed
(
	unsigned int	target
)	const
{
	float speed=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformStutterSpeed(target);
		if(candidate!=-1.0f)
		{
			speed=candidate;
		}
	}

	return(speed);
}

bool
InputObj::
WaveformSavepointPrev
(
	unsigned int	target
)	const
{
	bool prev=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		prev|=Children[a]->WaveformSavepointPrev(target);
	}

	return(prev);
}

bool
InputObj::
WaveformSavepointNext
(
	unsigned int	target
)	const
{
	bool next=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		next|=Children[a]->WaveformSavepointNext(target);
	}

	return(next);
}

int
InputObj::
WaveformSavepointPick
(
	unsigned int	target
)	const
{
	int pick=-9999;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		int candidate=Children[a]->WaveformSavepointPick(target);
		if(candidate!=-9999)
		{
			pick=candidate;
		}
	}

	return(pick);
}

bool
InputObj::
WaveformSavepointSet
(
	unsigned int	target
)	const
{
	bool set=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		set|=Children[a]->WaveformSavepointSet(target);
	}

	return(set);
}

bool
InputObj::
WaveformSavepointSetBPMAtNeedle
(
	unsigned int	target
)	const
{
	bool set=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		set|=Children[a]->WaveformSavepointSetBPMAtNeedle(target);
	}

	return(set);
}

bool
InputObj::
WaveformSavepointSetBPMUndef
(
	unsigned int	target
)	const
{
	bool set=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		set|=Children[a]->WaveformSavepointSetBPMUndef(target);
	}

	return(set);
}

bool
InputObj::
WaveformSavepointSetBPMNone
(
	unsigned int	target
)	const
{
	bool set=false;

	for(unsigned int a=0;a<Children.size();a++)
	{
		set|=Children[a]->WaveformSavepointSetBPMNone(target);
	}

	return(set);
}

float
InputObj::
WaveformSavepointUnsetPercent
(
	unsigned int	target
)	const
{
	float percent=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformSavepointUnsetPercent(target);
		if(candidate>percent)
		{
			percent=candidate;
		}
	}

	return(percent);
}

float
InputObj::
WaveformSavepointShift
(
	unsigned int	target
)	const
{
	float shift=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		shift+=Children[a]->WaveformSavepointShift(target);
	}

	return(shift);
}

float
InputObj::
WaveformSavepointShiftAll
(
	unsigned int	target
)	const
{
	float shift=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		shift+=Children[a]->WaveformSavepointShiftAll(target);
	}

	return(shift);
}

bool
InputObj::
WaveformSavepointShiftAllHere
(
	unsigned int	target
)	const
{
	bool here=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		here|=Children[a]->WaveformSavepointShiftAllHere(target);
	}

	return(here);
}

float
InputObj::
WaveformSavepointShiftBPM
(
	unsigned int	target
)	const
{
	float shift=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		shift+=Children[a]->WaveformSavepointShiftBPM(target);
	}

	return(shift);
}

bool
InputObj::
WaveformSavepointJumpNow
(
	unsigned int	target
)	const
{
	bool jump=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		jump|=Children[a]->WaveformSavepointJumpNow(target);
	}

	return(jump);
}

bool
InputObj::
WaveformSavepointJumpAtMeasure
(
	unsigned int	target
)	const
{
	bool jump=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		jump|=Children[a]->WaveformSavepointJumpAtMeasure(target);
	}

	return(jump);
}

float
InputObj::
WaveformBPM
(
	unsigned int	target
)	const
{
	float bpm = -1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float val = Children[a]->WaveformBPM(target);
		if(val != -1.0f)
		{
			bpm = val;
			break;
		}
	}

	return(bpm);
}

const char*
InputObj::
WaveformBPMCandidate
(
	unsigned int	target
)	const
{
	const char* str = NULL;

	for(unsigned int a=0;a<Children.size();a++)
	{
		const char* neoStr = Children[a]->WaveformBPMCandidate(target);
		if(neoStr)
		{
			str = neoStr;
			break;
		}
	}

	return(str);
}

void
InputObj::
WaveformClearBPMCandidate
(
	unsigned int	target
)
{
	for(unsigned int a=0;a<Children.size();a++)
	{
		Children[a]->WaveformClearBPMCandidate(target);
	}
}

void
InputObj::
WaveformHintBPMCandidate
(
	unsigned int	target,
	float		bpm
)
{
	for(unsigned int a=0;a<Children.size();a++)
	{
		Children[a]->WaveformHintBPMCandidate(target,bpm);
	}
}

float
InputObj::
WaveformJumpToPercent
(
	unsigned int	target
)	const
{
	float jump=-1.0f;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformJumpToPercent(target);
		if(candidate!=-1.0f)
		{
			jump=candidate;
			break;
		}
	}

	return(jump);
}

int
InputObj::
WaveformLoopMeasuresExponent
(
	unsigned int	target
)	const
{
	int exponent=WAVEFORM_LOOP_MEASURES_EXPONENT_NULL;

	for(unsigned int a=0;a<Children.size();a++)
	{
		int candidate=Children[a]->WaveformLoopMeasuresExponent(target);
		if(candidate!=WAVEFORM_LOOP_MEASURES_EXPONENT_NULL)
		{
			exponent=candidate;
		}
	}

	return(exponent);
}

bool
InputObj::
WaveformQuantizationPeriodHalf
(
	unsigned int	target
)	const
{
	bool half=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		half|=Children[a]->WaveformQuantizationPeriodHalf(target);
	}

	return(half);
}

bool
InputObj::
WaveformQuantizationPeriodDouble
(
	unsigned int	target
)	const
{
	bool twoX=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		twoX|=Children[a]->WaveformQuantizationPeriodDouble(target);
	}

	return(twoX);
}

bool
InputObj::
WaveformLoopSecondsLess
(
	unsigned int	target
)	const
{
	bool less=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		less|=Children[a]->WaveformLoopSecondsLess(target);
	}

	return(less);
}

bool
InputObj::
WaveformLoopSecondsMore
(
	unsigned int	target
)	const
{
	bool more=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		more|=Children[a]->WaveformLoopSecondsMore(target);
	}

	return(more);
}

bool
InputObj::
WaveformLoopAll
(
	unsigned int	target
)	const
{
	bool all=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		all|=Children[a]->WaveformLoopAll(target);
	}

	return(all);
}

bool
InputObj::
WaveformLoopToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		toggle|=Children[a]->WaveformLoopToggle(target);
	}

	return(toggle);
}

bool
InputObj::
WaveformLoopThenRecallActive
(
	unsigned int	target
)	const
{
	bool active=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		active|=Children[a]->WaveformLoopThenRecallActive(target);
	}

	return(active);
}

bool
InputObj::
WaveformReverse
(
	unsigned int	target
)	const
{
	bool reverse=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		reverse|=Children[a]->WaveformReverse(target);
	}

	return(reverse);
}

int
InputObj::
WaveformAutoDivergeRecall
(
	unsigned int	target
)	const
{
	int ret=0;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		int now = Children[a]->WaveformAutoDivergeRecall(target);
		if(now!=0)
		{
			ret=now;
		}
	}

	return(ret);
}

bool
InputObj::
WaveformVideoSelectLow
(
	unsigned int	target
)	const
{
	bool select=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		select|=Children[a]->WaveformVideoSelectLow(target);
	}

	return(select);
}

bool
InputObj::
WaveformVideoSelectHigh
(
	unsigned int	target
)	const
{
	bool select=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		select|=Children[a]->WaveformVideoSelectHigh(target);
	}

	return(select);
}

float
InputObj::
WaveformVideoBrightness
(
	unsigned int	target
)	const
{
	float bright=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformVideoBrightness(target);
		if(candidate!=-1.0f)
		{
			bright=candidate;
		}
	}

	return(bright);
}

float
InputObj::
WaveformVideoBrightnessDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformVideoBrightnessDelta(target);
	}

	return(delta);
}

float
InputObj::
WaveformSyphonBrightness
(
	unsigned int	target
)	const
{
	float bright=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformSyphonBrightness(target);
		if(candidate!=-1.0f)
		{
			bright=candidate;
		}
	}

	return(bright);
}

float
InputObj::
WaveformSyphonBrightnessDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformSyphonBrightnessDelta(target);
	}

	return(delta);
}

float
InputObj::
WaveformVideoAdvanceRate
(
	unsigned int	target
)	const
{
	float rate=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformVideoAdvanceRate(target);
		if(candidate!=-1.0f)
		{
			rate=candidate;
		}
	}

	return(rate);
}

float
InputObj::
WaveformFreqSenseBrightness
(
	unsigned int	target
)	const
{
	float brightness=-1.0f;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformFreqSenseBrightness(target);
		if(candidate!=-1.0f)
		{
			brightness=candidate;
		}
	}

	return(brightness);
}

float
InputObj::
WaveformFreqSenseBrightnessDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformFreqSenseBrightnessDelta(target);
	}

	return(delta);
}

float
InputObj::
WaveformFreqSenseLEDGroupFloat
(
	unsigned int	target
)	const
{
	float group=-1.0f;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformFreqSenseLEDGroupFloat(target);
		if(candidate!=-1.0f)
		{
			group=candidate;
		}
	}

	return(group);
}

float
InputObj::
WaveformFreqSenseLEDGroupFloatDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformFreqSenseLEDGroupFloatDelta(target);
	}

	return(delta);
}

float
InputObj::
WaveformFreqSenseLEDColorScalarLow
(
	unsigned int	target
)	const
{
	float colorScalar=-1.0f;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformFreqSenseLEDColorScalarLow(target);
		if(candidate!=-1.0f)
		{
			colorScalar=candidate;
		}
	}

	return(colorScalar);
}

float
InputObj::
WaveformFreqSenseLEDColorScalarLowDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformFreqSenseLEDColorScalarLowDelta(target);
	}

	return(delta);
}

float
InputObj::
WaveformFreqSenseLEDColorScalarHigh
(
	unsigned int	target
)	const
{
	float colorScalar=-1.0f;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformFreqSenseLEDColorScalarHigh(target);
		if(candidate!=-1.0f)
		{
			colorScalar=candidate;
		}
	}

	return(colorScalar);
}

float
InputObj::
WaveformFreqSenseLEDColorScalarHighDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformFreqSenseLEDColorScalarHighDelta(target);
	}

	return(delta);
}

float
InputObj::
WaveformFreqSenseLEDBrightness
(
	unsigned int	target
)	const
{
	float brightness=-1.0f;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformFreqSenseLEDBrightness(target);
		if(candidate!=-1.0f)
		{
			brightness=candidate;
		}
	}

	return(brightness);
}

float
InputObj::
WaveformFreqSenseLEDBrightnessDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformFreqSenseLEDBrightnessDelta(target);
	}

	return(delta);
}

float
InputObj::
WaveformFreqSenseLEDBrightnessWash
(
	unsigned int	target
)	const
{
	float brightness=-1.0f;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformFreqSenseLEDBrightnessWash(target);
		if(candidate!=-1.0f)
		{
			brightness=candidate;
		}
	}

	return(brightness);
}

float
InputObj::
WaveformFreqSenseLEDBrightnessWashDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformFreqSenseLEDBrightnessWashDelta(target);
	}

	return(delta);
}

bool
InputObj::
WaveformAudioInputToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		toggle|=Children[a]->WaveformAudioInputToggle(target);
	}

	return(toggle);
}

bool
InputObj::
WaveformVideoAspectRatioNext
(
	unsigned int	target
)	const
{
	bool next=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		next|=Children[a]->WaveformVideoAspectRatioNext(target);
	}

	return(next);
}

float
InputObj::
WaveformOscilloscopeBrightness
(
	unsigned int	target
)	const
{
	float bright=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformOscilloscopeBrightness(target);
		if(candidate!=-1.0f)
		{
			bright=candidate;
		}
	}

	return(bright);
}

float
InputObj::
WaveformOscilloscopeBrightnessDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		delta+=Children[a]->WaveformOscilloscopeBrightnessDelta(target);
	}

	return(delta);
}

bool
InputObj::
WaveformSync
(
	unsigned int	target
)	const
{
	bool sync=false;
	
	for(unsigned int a=0;a<Children.size();a++)
	{
		sync|=Children[a]->WaveformSync(target);
	}

	return(sync);
}

float
InputObj::
WaveformPointerScratch
(
	unsigned int	target
)	const
{
	float targetX=-1.0f;

	for(unsigned int a=0;a<Children.size();a++)
	{
		float candidate=Children[a]->WaveformPointerScratch(target);
		if(candidate!=-1.0f)
		{
			targetX=candidate;
		}
	}

	return(targetX);
}


