/*
 *
 * InputTester.h - Input abstraction object
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

#ifndef	_INPUT_TESTER_H_
#define	_INPUT_TESTER_H_

#include "Input.h"
#include "Config.h"

class InputTesterObj;

InputTesterObj& GetInputTester();

class InputTesterObj : public InputObj
{

public:

		InputTesterObj();
virtual		~InputTesterObj();

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
virtual int	MasterToHeadphones()					const;	//Headphones hears what's being output to master

	//Mode 0: File Selection

virtual	float	FileScroll			(unsigned int target)	const;	//How many file to scroll this frame
virtual	int	FileSelect			(unsigned int target)	const;	//Enter the directory or open the file
virtual bool	FileMarkUnopened		(unsigned int target)	const;	//Don't display that the current file has been opened
virtual	bool	FileRefresh			(unsigned int target)	const;	//Rescan current folder

	//Mode 2: Waveform

virtual int	WaveformEject			(unsigned int target)	const;	//Return to File Selection
virtual bool	WaveformPauseToggle		(unsigned int target)	const;	//Play/Pause the record
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
virtual	bool	WaveformRhythmicVolumeInvert	(unsigned int target)	const;	//Invert my Turntable's volume so I'm audible on nth notes, as defined by LoopMeasures
virtual	bool	WaveformRhythmicVolumeInvertOther
						(unsigned int target)	const;	//Invert other TT's volume so only I'm audible on nth notes, as defined by LoopMeasures
virtual	bool	WaveformVolumeSolo		(unsigned int target)	const;	//Full + others muted
virtual	float	WaveformRewindFF		(unsigned int target)	const;	//Velocity of seeking
virtual	bool	WaveformRecordHold		(unsigned int target)	const;	//Finger on Record
virtual	float	WaveformRecordSpeed		(unsigned int target)	const;	//Velocity of scratching
virtual	bool	WaveformStutter			(unsigned int target)	const;	//Repeat a small segment of audio
virtual	float	WaveformStutterPitch		(unsigned int target)	const;	//Pitch of the stutter
virtual	float	WaveformStutterSpeed		(unsigned int target)	const;	//Speed / Length of the stutter
virtual	bool	WaveformSavepointPrev		(unsigned int target)	const;	//Highlight previous save point
virtual	bool	WaveformSavepointNext		(unsigned int target)	const;	//Highlight next save point
virtual	int	WaveformSavepointPick		(unsigned int target)	const;	//Highlight a chosen save point
virtual	bool	WaveformSavepointSet		(unsigned int target)	const;	//Lock a save point
virtual	float	WaveformSavepointUnsetPercent	(unsigned int target)	const;	//Clear a save point
virtual	float	WaveformSavepointShift		(unsigned int target)	const;	//Shift current save point X seconds
virtual	float	WaveformSavepointShiftAll	(unsigned int target)	const;	//Shift all save points X seconds
virtual	bool	WaveformSavepointShiftAllHere	(unsigned int target)	const;	//Shift all save points so this is beat 1
virtual	bool	WaveformSavepointJumpNow	(unsigned int target)	const;	//Jump to current save point
virtual	bool	WaveformSavepointJumpAtMeasure	(unsigned int target)	const;	//Jump to current save point at the end of this measure
virtual	int	WaveformLoopMeasuresExponent	(unsigned int target)	const;	//Loop 2^n measures. If disabled, enable. Else, disable if equal.
virtual	bool	WaveformQuantizationPeriodHalf	(unsigned int target)	const;	//Loop half as many measures
virtual	bool	WaveformQuantizationPeriodDouble(unsigned int target)	const;	//Loop twice as many measures
virtual	bool	WaveformLoopSecondsLess		(unsigned int target)	const;	//Loop less seconds (When no BPM is available)
virtual	bool	WaveformLoopSecondsMore		(unsigned int target)	const;	//Loop more seconds (When no BPM is available)
virtual bool	WaveformLoopAll			(unsigned int target)	const;	//Loop all measures (to savepoint [9], or last measure), or all seconds
virtual	bool	WaveformLoopToggle		(unsigned int target)	const;	//Enter/Exit loop mode
virtual	bool	WaveformLoopThenRecallActive	(unsigned int target)	const;	//Loops, but when done, jump to where we would have otherwise been, preserving flow
virtual int	WaveformAutoDivergeRecall	(unsigned int target)	const;	//When done diverging, jump to where we would have otherwise been, had we not diverged
virtual	bool	WaveformVideoSelectLow		(unsigned int target)	const;	//Choose a new low video
virtual	bool	WaveformVideoSelectHigh		(unsigned int target)	const;	//Choose a new high video
virtual	float	WaveformVideoBrightness		(unsigned int target)	const;	//How bright the video is, independent of the crossfader
virtual	float	WaveformSyphonBrightness	(unsigned int target)	const;	//How bright syphon is, independent of the crossfader
virtual	float	WaveformVideoAdvanceRate	(unsigned int target)	const;	//How quickly to advance the video relative to the audio
virtual	float	WaveformFreqSenseBrightness	(unsigned int target)	const;	//Set frequency-sensitive video mixer brightness
virtual	bool	WaveformAudioInputToggle	(unsigned int target)	const;	//Set audio input mode
virtual	bool	WaveformVideoAspectRatioNext	(unsigned int target)	const;	//Advance to next aspect ratio mode
virtual	float	WaveformOscilloscopeBrightness	(unsigned int target)	const;	//How bright the oscolloscope is, independent of the crossfader
virtual	bool	WaveformSync			(unsigned int target)	const;	//Sync BPM to opposite turntable
virtual	float	WaveformPointerScratch		(unsigned int target)	const;	//Point at the waveform for scratching

private:

	LGL_Timer	LastActionTimer;

	int		CurrentActionBottom;
	int		CurrentActionTop;
	bool		Enable;
	LGL_Timer	EnableTimer;

	int		GetCurrentAction(unsigned int target) const;

public:

	bool		GetEnable();
};

#endif	//_INPUT_TESTER_H_

