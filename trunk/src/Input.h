/*
 *
 * Input.h - Input abstraction object
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

#ifndef	_INPUT_H_
#define	_INPUT_H_

#include "LGL.module/LGL.h"

#include <vector>

typedef enum
{
	GUI_ELEMENT_NULL = 0,
	GUI_ELEMENT_WAVEFORM,
	GUI_ELEMENT_ENTIRE_WAVEFORM,
	GUI_ELEMENT_XFADER_LEFT,
	GUI_ELEMENT_XFADER_RIGHT,
	GUI_ELEMENT_EQ_LOW,
	GUI_ELEMENT_EQ_MID,
	GUI_ELEMENT_EQ_HIGH,
	GUI_ELEMENT_EQ_GAIN,
	GUI_ELEMENT_VIS_VIDEO,
	GUI_ELEMENT_VIS_OSCILLOSCOPE,
	GUI_ELEMENT_VIS_FREQSENSE,
	GUI_ELEMENT_SAVEPOINT_BPM_ALPHA,
	GUI_ELEMENT_SAVEPOINT_BPM_OMEGA,
	GUI_ELEMENT_SAVEPOINT_0,
	GUI_ELEMENT_SAVEPOINT_1,
	GUI_ELEMENT_SAVEPOINT_2,
	GUI_ELEMENT_SAVEPOINT_3,
	GUI_ELEMENT_SAVEPOINT_4,
	GUI_ELEMENT_SAVEPOINT_5,
	GUI_ELEMENT_SAVEPOINT_6,
	GUI_ELEMENT_SAVEPOINT_7,
	GUI_ELEMENT_SAVEPOINT_8,
	GUI_ELEMENT_SAVEPOINT_9,
	GUI_ELEMENT_LOOP_MEASURES,
	GUI_ELEMENT_BPM_PITCH,
	GUI_ELEMENT_COUNT
} DVJ_GuiElement;

class InputObj;

extern InputObj Input;

#define	TARGET_NONE (0)
#define	TARGET_FOCUS (1<<0)
#define	TARGET_BOTTOM (1<<1)
#define	TARGET_TOP (1<<2)

#define	WAVEFORM_LOOP_MEASURES_EXPONENT_NULL (-10000)

class InputObj
{

public:

		InputObj();
virtual		~InputObj();

	//Core

	void	AddChild(InputObj*);
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
virtual int	MasterToHeadphones()					const;	//Headphones hears what's being output to master

	//Mode 0: File Selection

virtual	float	FileScroll			(unsigned int target)	const;	//How many file to scroll this frame
virtual	int	FileSelect			(unsigned int target)	const;	//Enter the directory or open the file
virtual bool	FileMarkUnopened		(unsigned int target)	const;	//Don't display that the current file has been opened
virtual	bool	FileRefresh			(unsigned int target)	const;	//Rescan current folder
virtual int	FileIndexHighlight		(unsigned int target)	const;	//Highlight an entry

	//Mode 1: Decoding...

virtual bool	DecodeAbort			(unsigned int target)	const;	//Return to File Selection

	//Mode 2: Waveform

virtual int	WaveformEject			(unsigned int target)	const;	//Return to File Selection
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
virtual	bool	WaveformRapidVolumeInvert	(unsigned int target)	const;	//Invert my Turntable's volume so I'm audible on nth notes, as defined by LoopMeasures
virtual	bool	WaveformRapidSoloInvert		(unsigned int target)	const;	//Invert other TT's volume so only I'm audible on nth notes, as defined by LoopMeasures
virtual	bool	WaveformVolumeSolo		(unsigned int target)	const;	//Full + others muted
virtual	float	WaveformRewindFF		(unsigned int target)	const;	//Velocity of seeking
virtual	bool	WaveformRecordHold		(unsigned int target)	const;	//Finger on Record
virtual	float	WaveformRecordSpeed		(unsigned int target)	const;	//Velocity of scratching
virtual	bool	WaveformStutter			(unsigned int target)	const;	//Repeat a small segment of audio
virtual	float	WaveformStutterPitch		(unsigned int target)	const;	//Pitch of the stutter
virtual	float	WaveformStutterSpeed		(unsigned int target)	const;	//Speed / Length of the stutter
virtual	bool	WaveformSavePointPrev		(unsigned int target)	const;	//Highlight previous save point
virtual	bool	WaveformSavePointNext		(unsigned int target)	const;	//Highlight next save point
virtual	int	WaveformSavePointPick		(unsigned int target)	const;	//Highlight a chosen save point
virtual	bool	WaveformSavePointSet		(unsigned int target)	const;	//Lock a save point
virtual	float	WaveformSavePointUnsetPercent	(unsigned int target)	const;	//Clear a save point
virtual	float	WaveformSavePointShift		(unsigned int target)	const;	//Shift current save point X seconds
virtual	float	WaveformSavePointShiftAll	(unsigned int target)	const;	//Shift all save points X seconds
virtual	bool	WaveformSavePointShiftAllHere	(unsigned int target)	const;	//Shift all save points so this is beat 1
virtual	bool	WaveformSavePointJumpNow	(unsigned int target)	const;	//Jump to current save point
virtual	bool	WaveformSavePointJumpAtMeasure	(unsigned int target)	const;	//Jump to current save point at the end of this measure
virtual float	WaveformJumpToPercent		(unsigned int target)	const;	//Jump to a percent of the track's duration
virtual	int	WaveformLoopMeasuresExponent	(unsigned int target)	const;	//Loop 2^n measures. If disabled, enable. Else, disable if equal.
virtual	bool	WaveformLoopMeasuresHalf	(unsigned int target)	const;	//Loop half as many measures
virtual	bool	WaveformLoopMeasuresDouble	(unsigned int target)	const;	//Loop twice as many measures
virtual	bool	WaveformLoopSecondsLess		(unsigned int target)	const;	//Loop less seconds (When no BPM is available)
virtual	bool	WaveformLoopSecondsMore		(unsigned int target)	const;	//Loop more seconds (When no BPM is available)
virtual bool	WaveformLoopAll			(unsigned int target)	const;	//Loop all measures (to savepoint [9], or last measure), or all seconds
virtual	bool	WaveformLoopToggle		(unsigned int target)	const;	//Enter/Exit loop mode
virtual	bool	WaveformLoopThenRecallActive	(unsigned int target)	const;	//Loops, but when done, jump to where we would have otherwise been, preserving flow
virtual int	WaveformAutoDivergeRecall	(unsigned int target)	const;	//When done diverging, jump to where we would have otherwise been, had we not diverged
virtual	bool	WaveformVideoSelect		(unsigned int target)	const;	//Choose a new video
virtual	float	WaveformVideoBrightness		(unsigned int target)	const;	//How bright the video is, independent of the crossfader
virtual	float	WaveformVideoBrightnessDelta	(unsigned int target)	const;	//How bright the video is, independent of the crossfader
virtual	float	WaveformVideoAdvanceRate	(unsigned int target)	const;	//How quickly to advance the video relative to the audio
virtual	float	WaveformFreqSenseBrightness	(unsigned int target)	const;	//Set frequency-sensitive video mixer brightness
virtual	float	WaveformFreqSenseBrightnessDelta(unsigned int target)	const;	//Set frequency-sensitive video mixer brightness
virtual	int	WaveformAudioInputMode		(unsigned int target)	const;	//Set audio input mode
virtual	bool	WaveformVideoAspectRatioNext	(unsigned int target)	const;	//Advance to next aspect ratio mode
virtual	float	WaveformOscilloscopeBrightness	(unsigned int target)	const;	//How bright the oscolloscope is, independent of the crossfader
virtual	float	WaveformOscilloscopeBrightnessDelta
						(unsigned int target)	const;	//How bright the oscolloscope is, independent of the crossfader
virtual	bool	WaveformSyncBPM			(unsigned int target)	const;	//Sync BPM to opposite turntable
virtual	float	WaveformPointerScratch		(unsigned int target)	const;	//Point at the waveform for scratching

private:

	std::vector<InputObj*>
		Children;

};

#endif	//_INPUT_H_

