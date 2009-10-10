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

#include "InputNull.h"

InputNullObj::
InputNullObj()
{
	//
}

InputNullObj::
~InputNullObj()
{
	//
}

//Core

void
InputNullObj::
NextFrame()
{
	//
}

//Global Input

bool
InputNullObj::
FocusChange()	const
{
	bool change=false;
	return(change);
}

bool
InputNullObj::
FocusBottom()	const
{
	bool bottom=false;
	return(bottom);
}

bool
InputNullObj::
FocusTop()	const
{
	bool top=false;
	return(top);
}

float
InputNullObj::
XfaderSpeakers()	const
{
	float xfade=-1.0f;
	return(xfade);
}

float
InputNullObj::
XfaderSpeakersDelta()	const
{
	float delta=0.0f;
	return(delta);
}

float
InputNullObj::
XfaderHeadphones()	const
{
	float xfade=-1.0f;
	return(xfade);
}

float
InputNullObj::
XfaderHeadphonesDelta()	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputNullObj::
SyncTopToBottom()	const
{
	bool sync=false;
	return(sync);
}

int
InputNullObj::
MasterToHeadphones()	const
{
	int to=-1;
	return(to);
}

bool
InputNullObj::
SyncBottomToTop()	const
{
	bool sync=false;
	return(sync);
}

//Mode 0: File Selection

float
InputNullObj::
FileScroll
(
	unsigned int	target
)	const
{
	float scroll=0.0f;
	return(scroll);
}

bool
InputNullObj::
FileSelect
(
	unsigned int	target
)	const
{
	bool choose=false;
	return(choose);
}

bool
InputNullObj::
FileMarkUnopened
(
	unsigned int	target
)	const
{
	bool mark=false;
	return(mark);
}

bool
InputNullObj::
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
InputNullObj::
DecodeAbort
(
	unsigned int	target
)	const
{
	bool abort=false;
	return(abort);
}

//Mode 2: Waveform

bool
InputNullObj::
WaveformEject
(
	unsigned int	target
)	const
{
	bool eject=false;
	return(eject);
}

bool
InputNullObj::
WaveformTogglePause
(
	unsigned int	target
)	const
{
	bool toggle=false;
	return(toggle);
}

float
InputNullObj::
WaveformNudge
(
	unsigned int	target
)	const
{
	float nudge=0.0f;
	return(nudge);
}

float
InputNullObj::
WaveformPitchbend
(
	unsigned int	target
)	const
{
	float pitchbend=0.0f;
	return(pitchbend);
}

float
InputNullObj::
WaveformPitchbendDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

float
InputNullObj::
WaveformEQLow
(
	unsigned int	target
)	const
{
	float low=-1.0f;
	return(low);
}

float
InputNullObj::
WaveformEQLowDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputNullObj::
WaveformEQLowKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputNullObj::
WaveformEQMid
(
	unsigned int	target
)	const
{
	float mid=-1.0f;
	return(mid);
}

float
InputNullObj::
WaveformEQMidDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputNullObj::
WaveformEQMidKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputNullObj::
WaveformEQHigh
(
	unsigned int	target
)	const
{
	float high=-1.0f;
	return(high);
}

float
InputNullObj::
WaveformEQHighDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputNullObj::
WaveformEQHighKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputNullObj::
WaveformGain
(
	unsigned int	target
)	const
{
	float gain=-1.0f;
	return(gain);
}

float
InputNullObj::
WaveformGainDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputNullObj::
WaveformGainKill
(
	unsigned int	target
)	const
{
	bool kill=false;
	return(kill);
}

float
InputNullObj::
WaveformVolumeSlider
(
	unsigned int	target
)	const
{
	float volume=-1.0f;
	return(volume);
}

bool
InputNullObj::
WaveformVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputNullObj::
WaveformVolumeSolo
(
	unsigned int	target
)	const
{
	bool solo=false;
	return(solo);
}

float
InputNullObj::
WaveformRewindFF
(
	unsigned int	target
)	const
{
	float rewindff=0.0f;
	return(rewindff);
}

bool
InputNullObj::
WaveformRecordHold
(
	unsigned int	target
)	const
{
	bool hold=false;
	return(hold);
}

float
InputNullObj::
WaveformRecordSpeed
(
	unsigned int	target
)	const
{
	float speed=0.0f;
	return(speed);
}

bool
InputNullObj::
WaveformStutter
(
	unsigned int	target
)	const
{
	bool stutter=false;
	return(stutter);
}

float
InputNullObj::
WaveformStutterPitch
(
	unsigned int	target
)	const
{
	float pitch=-1.0f;
	return(pitch);
}

float
InputNullObj::
WaveformStutterSpeed
(
	unsigned int	target
)	const
{
	float speed=-1.0f;
	return(speed);
}

bool
InputNullObj::
WaveformSavePointPrev
(
	unsigned int	target
)	const
{
	bool prev=false;
	return(prev);
}

bool
InputNullObj::
WaveformSavePointNext
(
	unsigned int	target
)	const
{
	bool next=false;
	return(next);
}

bool
InputNullObj::
WaveformSavePointSet
(
	unsigned int	target
)	const
{
	bool set=false;
	return(set);
}

float
InputNullObj::
WaveformSavePointUnsetPercent
(
	unsigned int	target
)	const
{
	float percent=0.0f;
	return(percent);
}

float
InputNullObj::
WaveformSavePointShift
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

float
InputNullObj::
WaveformSavePointShiftAll
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

bool
InputNullObj::
WaveformSavePointShiftAllHere
(
	unsigned int	target
)	const
{
	bool here=false;
	return(here);
}

bool
InputNullObj::
WaveformSavePointJumpNow
(
	unsigned int	target
)	const
{
	bool jump=false;
	return(jump);
}

bool
InputNullObj::
WaveformSavePointJumpAtMeasure
(
	unsigned int	target
)	const
{
	bool jump=false;
	return(jump);
}

int
InputNullObj::
WaveformLoopMeasuresExponent
(
	unsigned int	target
)	const
{
	int exponent=WAVEFORM_LOOP_MEASURES_EXPONENT_NULL;
	return(exponent);
}

bool
InputNullObj::
WaveformLoopMeasuresHalf
(
	unsigned int	target
)	const
{
	bool half=false;
	return(half);
}

bool
InputNullObj::
WaveformLoopMeasuresDouble
(
	unsigned int	target
)	const
{
	bool twoX=false;
	return(twoX);
}

bool
InputNullObj::
WaveformLoopSecondsLess
(
	unsigned int	target
)	const
{
	bool less=false;
	return(less);
}

bool
InputNullObj::
WaveformLoopSecondsMore
(
	unsigned int	target
)	const
{
	bool more=false;
	return(more);
}

bool
InputNullObj::
WaveformLoopToggle
(
	unsigned int	target
)	const
{
	bool toggle=false;
	return(toggle);
}

bool
InputNullObj::
WaveformLoopThenRecallActive
(
	unsigned int	target
)	const
{
	bool active=false;
	return(active);
}

bool
InputNullObj::
WaveformVideoSelect
(
	unsigned int	target
)	const
{
	bool select=false;
	return(select);
}

float
InputNullObj::
WaveformVideoBrightness
(
	unsigned int	target
)	const
{
	float bright=-1.0f;
	return(bright);
}

float
InputNullObj::
WaveformVideoAdvanceRate
(
	unsigned int	target
)	const
{
	float rate=-1.0f;
	return(rate);
}

int
InputNullObj::
WaveformVideoFreqSenseMode
(
	unsigned int	target
)	const
{
	int mode=-1;
	return(mode);
}

bool
InputNullObj::
WaveformSyncBPM
(
	unsigned int	target
)	const
{
	bool sync=false;
	return(sync);
}

float
InputNullObj::
WaveformPointerScratch
(
	unsigned int	target
)	const
{
	float targetX=-1.0f;
	return(targetX);
}

