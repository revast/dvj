/*
 *
 * Config.h
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

#ifndef	_DVJ_CONFIG_H_
#define	_DVJ_CONFIG_H_

void
ConfigInit();

const char*
GetMusicRootPath();

const char*
GetMusicRootConfigFilePath();

const char*
GetDvjCacheDirName();

void
SetMusicRootPath(const char* path);

bool
GetPurgeInactiveMemory();

void
GetLoadScreenPath
(
	char*	loadScreenPath
);

const char*
GetDVJSessionFlacPath();

const char*
GetDVJSessionTracklistPath();

const char*
GetDVJSessionDrawLogPath();

int
GetProjectorQuadrentResX();

int
GetProjectorQuadrentResY();

float
GetCachedVideoAveBitrateMBps();

float
GetVisualBrightnessAtCenter();

void
GetColorCool
(
	float&	r,
	float&	g,
	float&	b
);

void
GetColorWarm
(
	float&	r,
	float&	g,
	float&	b
);

int
GetFPSMax();

bool
GetAudioInPassThru();

int
GetVideoBufferFrames();

int
GetVideoBufferFramesFreqSense();

int	GetInputKeyboardFocusChangeKey();
int	GetInputKeyboardFocusBottomKey();
int	GetInputKeyboardFocusTopKey();
int	GetInputKeyboardXfaderSpeakersDeltaDownKey();
int	GetInputKeyboardXfaderSpeakersDeltaUpKey();
int	GetInputKeyboardXfaderHeadphonesDeltaDownKey();
int	GetInputKeyboardXfaderHeadphonesDeltaUpKey();
int	GetInputKeyboardSyncTopToBottomKey();
int	GetInputKeyboardSyncBottomToTopKey();
int	GetInputKeyboardFileScrollDownManyKey();
int	GetInputKeyboardFileScrollUpManyKey();
int	GetInputKeyboardFileScrollDownOneKey();
int	GetInputKeyboardFileScrollUpOneKey();
int	GetInputKeyboardFileSelectKey();
int	GetInputKeyboardFileMarkUnopenedKey();
int	GetInputKeyboardFileRefreshKey();
int	GetInputKeyboardDecodeAbortKey();
int	GetInputKeyboardWaveformEjectKey();
int	GetInputKeyboardWaveformTogglePauseKey();
int	GetInputKeyboardWaveformNudgeLeft1Key();
int	GetInputKeyboardWaveformNudgeRight1Key();
int	GetInputKeyboardWaveformNudgeLeft2Key();
int	GetInputKeyboardWaveformNudgeRight2Key();
int	GetInputKeyboardWaveformPitchbendDeltaDownSlowKey();
int	GetInputKeyboardWaveformPitchbendDeltaUpSlowKey();
int	GetInputKeyboardWaveformPitchbendDeltaDownFastKey();
int	GetInputKeyboardWaveformPitchbendDeltaUpFastKey();
int	GetInputKeyboardWaveformEQLowDeltaDownKey();
int	GetInputKeyboardWaveformEQLowDeltaUpKey();
int	GetInputKeyboardWaveformEQLowKillKey();
int	GetInputKeyboardWaveformEQMidDeltaDownKey();
int	GetInputKeyboardWaveformEQMidDeltaUpKey();
int	GetInputKeyboardWaveformEQMidKillKey();
int	GetInputKeyboardWaveformEQHighDeltaDownKey();
int	GetInputKeyboardWaveformEQHighDeltaUpKey();
int	GetInputKeyboardWaveformEQHighKillKey();
int	GetInputKeyboardWaveformGainDeltaDownKey();
int	GetInputKeyboardWaveformGainDeltaUpKey();
int	GetInputKeyboardWaveformVolumeInvertKey();
int	GetInputKeyboardWaveformRapidVolumeInvertKey();
int	GetInputKeyboardWaveformRapidSoloInvertKey();
int	GetInputKeyboardWaveformVolumeSoloKey();
int	GetInputKeyboardWaveformRewindKey();
int	GetInputKeyboardWaveformFFKey();
int	GetInputKeyboardWaveformRecordSpeedBackKey();
int	GetInputKeyboardWaveformRecordSpeedForwardKey();
int	GetInputKeyboardWaveformStutterKey();
int	GetInputKeyboardWaveformSavePointPrevKey();
int	GetInputKeyboardWaveformSavePointNextKey();
int	GetInputKeyboardWaveformSavePointSetKey();
int	GetInputKeyboardWaveformSavePointUnsetKey();
int	GetInputKeyboardWaveformSavePointShiftLeftKey();
int	GetInputKeyboardWaveformSavePointShiftRightKey();
int	GetInputKeyboardWaveformSavePointShiftAllLeftKey();
int	GetInputKeyboardWaveformSavePointShiftAllRightKey();
int	GetInputKeyboardWaveformSavePointJumpNowKey();
int	GetInputKeyboardWaveformSavePointJumpAtMeasureKey();
int	GetInputKeyboardWaveformLoopMeasuresHalfKey();
int	GetInputKeyboardWaveformLoopMeasuresDoubleKey();
int	GetInputKeyboardWaveformLoopToggleKey();
int	GetInputKeyboardWaveformLoopThenRecallKey();
int	GetInputKeyboardWaveformAutoDivergeRecallKey();
int	GetInputKeyboardWaveformVideoSelectKey();
int	GetInputKeyboardWaveformAudioInputModeKey();
int	GetInputKeyboardWaveformVideoAspectRatioNextKey();
int	GetInputKeyboardWaveformSyncBPMKey();
int	GetInputKeyboardRecordingStartKey();
int	GetInputKeyboardFullScreenToggleKey();
int	GetInputKeyboardVisualizerFullScreenToggleKey();
int	GetInputKeyboardScreenshotKey();
//int	GetInputKeyboardWaveformKey();

typedef enum
{
	NOOP = 0,
	FOCUS_CHANGE,
	FOCUS_BOTTOM,
	FOCUS_TOP,
	XFADER_SPEAKERS_DELTA_DOWN,
	XFADER_SPEAKERS_DELTA_UP,
	XFADER_HEADPHONES_DELTA_DOWN,
	XFADER_HEADPHONES_DELTA_UP,
	SYNC_TOP_TO_BOTTOM,
	SYNC_BOTTOM_TO_TOP,
	MASTER_TO_HEADPHONES,
	FILE_SCROLL_DOWN_MANY,
	FILE_SCROLL_UP_MANY,
	FILE_SCROLL_DOWN_ONE,
	FILE_SCROLL_UP_ONE,
	FILE_SELECT,
	FILE_MARK_UNOPENED,
	FILE_REFRESH,
	DECODE_ABORT,
	WAVEFORM_EJECT,
	WAVEFORM_TOGGLE_PAUSE,
	WAVEFORM_NUDGE_LEFT_1,
	WAVEFORM_NUDGE_RIGHT_1,
	WAVEFORM_NUDGE_LEFT_2,
	WAVEFORM_NUDGE_RIGHT_2,
	WAVEFORM_PITCHBEND_DELTA_DOWN_SLOW,
	WAVEFORM_PITCHBEND_DELTA_UP_SLOW,
	WAVEFORM_PITCHBEND_DELTA_DOWN_FAST,
	WAVEFORM_PITCHBEND_DELTA_UP_FAST,
	WAVEFORM_EQ_LOW_DELTA_DOWN,
	WAVEFORM_EQ_LOW_DELTA_UP,
	WAVEFORM_EQ_LOW_KILL,
	WAVEFORM_EQ_MID_DELTA_DOWN,
	WAVEFORM_EQ_MID_DELTA_UP,
	WAVEFORM_EQ_MID_KILL,
	WAVEFORM_EQ_HIGH_DELTA_DOWN,
	WAVEFORM_EQ_HIGH_DELTA_UP,
	WAVEFORM_EQ_HIGH_KILL,
	WAVEFORM_GAIN_DELTA_DOWN,
	WAVEFORM_GAIN_DELTA_UP,
	WAVEFORM_GAIN_KILL,
	WAVEFORM_VOLUME_INVERT,
	WAVEFORM_RAPID_VOLUME_INVERT,
	WAVEFORM_RAPID_SOLO_INVERT,
	WAVEFORM_VOLUME_SOLO,
	WAVEFORM_REWIND,
	WAVEFORM_FF,
	WAVEFORM_RECORD_SPEED_BACK,
	WAVEFORM_RECORD_SPEED_FORWARD,
	WAVEFORM_STUTTER,
	WAVEFORM_SAVEPOINT_PREV,
	WAVEFORM_SAVEPOINT_NEXT,
	WAVEFORM_SAVEPOINT_SET,
	WAVEFORM_SAVEPOINT_UNSET,
	WAVEFORM_SAVEPOINT_SHIFT_LEFT,
	WAVEFORM_SAVEPOINT_SHIFT_RIGHT,
	WAVEFORM_SAVEPOINT_SHIFT_ALL_LEFT,
	WAVEFORM_SAVEPOINT_SHIFT_ALL_RIGHT,
	WAVEFORM_SAVEPOINT_JUMP_NOW,
	WAVEFORM_SAVEPOINT_JUMP_AT_MEASURE,
	WAVEFORM_LOOP_MEASURES_HALF,
	WAVEFORM_LOOP_MEASURES_DOUBLE,
	WAVEFORM_LOOP_TOGGLE,
	WAVEFORM_LOOP_THEN_RECALL,
	WAVEFORM_AUTO_DIVERGE_THEN_RECALL,
	WAVEFORM_VIDEO_SELECT,
	WAVEFORM_AUDIO_INPUT_MODE,
	WAVEFORM_VIDEO_ASPECT_RATIO_NEXT,
	WAVEFORM_SYNC_BPM,
	RECORDING_START,
	FULL_SCREEN_TOGGLE,
	VISUALIZER_FULL_SCREEN_TOGGLE,
	SCREENSHOT,
	ACTION_LAST
} DVJ_Action;

#endif	//_DVJ_CONFIG_H_

