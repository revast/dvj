/*
 *
 * InputNull.h - Input abstraction object
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

#ifndef	_INPUT_WIIMOTE_H_
#define	_INPUT_WIIMOTE_H_

#include "Input.h"

class InputWiimoteObj : public InputObj
{

public:

		InputWiimoteObj();
virtual		~InputWiimoteObj();

	//Core

virtual	void	NextFrame();

	//Global Input

virtual bool	FocusChange()						const;	//Change focus
virtual	bool	FocusBottom()						const;	//Change focus to bottom turntable
virtual	bool	FocusTop()						const;	//Change focus to top turntable
virtual float	XfaderSpeakers()					const;	//Crossfader for speakers
virtual float	XfaderSpeakersDelta()					const;	//Crossfader for speakers (change)
virtual float	XfaderHeadphones()					const;	//Crossfader for headphones
virtual float	XfaderHeadphonesDelta()					const;	//Crossfader for headphones (change)
virtual	bool	SyncTopToBottom()					const;	//Sync top turntable to bottom
virtual	bool	SyncBottomToTop()					const;	//Sync bottom turntable to top

	//Mode 0: File Selection

virtual	float	FileScroll			(unsigned int target)	const;	//How many file to scroll this frame
virtual	bool	FileSelect			(unsigned int target)	const;	//Enter the directory or open the file
virtual	bool	FileRefresh			(unsigned int target)	const;	//Rescan current folder

	//Mode 1: Decoding...

virtual bool	DecodeAbort			(unsigned int target)	const;	//Return to File Selection

	//Mode 2: Waveform

virtual bool	WaveformEject			(unsigned int target)	const;	//Return to File Selection
virtual bool	WaveformTogglePause		(unsigned int target)	const;	//Play/Pause the record
virtual	float	WaveformNudge			(unsigned int target)	const;	//Temporary pitchbend for beatmatching
virtual	float	WaveformPitchbend		(unsigned int target)	const;	//Permanent pitchbend for beatmatching
virtual	float	WaveformPitchbendDelta		(unsigned int target)	const;	//Permanent pitchbend for beatmatching
virtual	float	WaveformEQLow			(unsigned int target)	const;	//Respected in all modes
virtual	float	WaveformEQLowDelta		(unsigned int target)	const;	//Respected in all modes
virtual	bool	WaveformEQLowKill		(unsigned int target)	const;	//Respected in all modes
virtual	float	WaveformEQMid			(unsigned int target)	const;	//Respected in all modes
virtual	float	WaveformEQMidDelta		(unsigned int target)	const;	//Respected in all modes
virtual	bool	WaveformEQMidKill		(unsigned int target)	const;	//Respected in all modes
virtual	float	WaveformEQHigh			(unsigned int target)	const;	//Respected in all modes
virtual	float	WaveformEQHighDelta		(unsigned int target)	const;	//Respected in all modes
virtual	bool	WaveformEQHighKill		(unsigned int target)	const;	//Respected in all modes
virtual float	WaveformGain			(unsigned int target)	const;	//Respected in all modes
virtual float	WaveformGainDelta		(unsigned int target)	const;	//Respected in all modes
virtual	bool	WaveformGainKill		(unsigned int target)	const;	//Respected in all modes
virtual	float	WaveformVolumeSlider		(unsigned int target)	const;	//Respected in all modes
virtual	bool	WaveformVolumeInvert		(unsigned int target)	const;	//Audible => Inaudible => Full
virtual	bool	WaveformVolumeSolo		(unsigned int target)	const;	//Full + others muted
virtual	float	WaveformRewindFF		(unsigned int target)	const;	//Velocity of seeking
virtual	bool	WaveformRecordHold		(unsigned int target)	const;	//Finger on Record
virtual	float	WaveformRecordSpeed		(unsigned int target)	const;	//Velocity of scratching
virtual	bool	WaveformStutter			(unsigned int target)	const;	//Repeat a small segment of audio
virtual	float	WaveformStutterPitch		(unsigned int target)	const;	//Pitch of the stutter
virtual	float	WaveformStutterSpeed		(unsigned int target)	const;	//Speed / Length of the stutter
virtual	bool	WaveformSavePointPrev		(unsigned int target)	const;	//Highlight previous save point
virtual	bool	WaveformSavePointNext		(unsigned int target)	const;	//Highlight next save point
virtual	bool	WaveformSavePointSet		(unsigned int target)	const;	//Lock a save point
virtual	float	WaveformSavePointUnsetPercent	(unsigned int target)	const;	//Clear a save point
virtual	float	WaveformSavePointShift		(unsigned int target)	const;	//Shift current save point X seconds
virtual	float	WaveformSavePointShiftAll	(unsigned int target)	const;	//Shift all save points X seconds
virtual	bool	WaveformSavePointShiftAllHere	(unsigned int target)	const;	//Shift all save points so this is beat 1
virtual	bool	WaveformSavePointJumpNow	(unsigned int target)	const;	//Jump to current save point
virtual	bool	WaveformSavePointJumpAtMeasure	(unsigned int target)	const;	//Jump to current save point at the end of this measure
virtual	bool	WaveformLoopBegin		(unsigned int target)	const;	//Mark start loop point as beginning of current measure
virtual	bool	WaveformLoopEnd			(unsigned int target)	const;	//Mark end loop point as end of current measure
virtual	bool	WaveformLoopDisable		(unsigned int target)	const;	//Exit loop mode
virtual	int	WaveformLoopMeasures		(unsigned int target)	const;	//Loop end point = Loop start point + N measures
virtual	bool	WaveformVideoSelect		(unsigned int target)	const;	//Choose a new video
virtual	float	WaveformVideoAdvanceRate	(unsigned int target)	const;	//How quickly to advance the video relative to the audio
virtual	bool	WaveformSyncBPM			(unsigned int target)	const;	//Sync BPM to opposite turntable
virtual	float	WaveformPointerScratch		(unsigned int target)	const;	//Point at the waveform for scratching

	private:

	float	ScrollInitialAccel;
	float	LastKnownPointerScratchX;

};

#endif	//_INPUT_WIIMOTE_H_

