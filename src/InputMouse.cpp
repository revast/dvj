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

InputMouseObj::
InputMouseObj()
{
	//
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
	//
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
	bool bottom=false;
	return(bottom);
}

bool
InputMouseObj::
FocusTop()	const
{
	bool top=false;
	return(top);
}

float
InputMouseObj::
XfaderSpeakers()	const
{
	float xfade=-1.0f;
	return(xfade);
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
	float xfade=-1.0f;
	return(xfade);
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

bool
InputMouseObj::
FileSelect
(
	unsigned int	target
)	const
{
	bool choose=false;
	return(choose);
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

bool
InputMouseObj::
WaveformEject
(
	unsigned int	target
)	const
{
	bool eject=false;
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
	float low=-1.0f;
	return(low);
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
	float mid=-1.0f;
	return(mid);
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
	float high=-1.0f;
	return(high);
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
	
	if(target & TARGET_FOCUS)
	{
		if(LGL_MouseDown(LGL_MOUSE_RIGHT))
		{
			float val = 16.0f*(LGL_MouseX()-0.5f);
			rewindff=(powf(2.0f,fabsf(val))-1.0f)*LGL_Sign(val);
		}
	}

	return(rewindff);
}

bool
InputMouseObj::
WaveformRecordHold
(
	unsigned int	target
)	const
{
	bool hold=false;
	return(hold);
}

float
InputMouseObj::
WaveformRecordSpeed
(
	unsigned int	target
)	const
{
	float speed=0.0f;
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

bool
InputMouseObj::
WaveformLoopBegin
(
	unsigned int	target
)	const
{
	bool begin=false;
	return(begin);
}

bool
InputMouseObj::
WaveformLoopEnd
(
	unsigned int	target
)	const
{
	bool end=false;
	return(end);
}

bool
InputMouseObj::
WaveformLoopDisable
(
	unsigned int	target
)	const
{
	bool disable=false;
	return(disable);
}

int
InputMouseObj::
WaveformLoopMeasures
(
	unsigned int	target
)	const
{
	int measures=-1;
	return(measures);
}

bool
InputMouseObj::
WaveformVideoSelect
(
	unsigned int	target
)	const
{
	bool select=false;
	return(select);
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

int
InputMouseObj::
WaveformVideoFreqSenseMode
(
	unsigned int	target
)	const
{
	int mode=-1;
	return(mode);
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
	
	if(target & TARGET_FOCUS)
	{
		if(LGL_MouseDown(LGL_MOUSE_LEFT))
		{
			targetX=LGL_MouseX();
		}
	}

	return(targetX);
}

