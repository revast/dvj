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

typedef enum
{
	NOOP = 0,
	FOCUS_CHANGE,
	FOCUS_BOTTOM,
	FOCUS_TOP,
	XFADER_SPEAKERS,
	XFADER_SPEAKERS_DELTA_DOWN,
	XFADER_SPEAKERS_DELTA_UP,
	XFADER_HEADPHONES,
	XFADER_HEADPHONES_DELTA_DOWN,
	XFADER_HEADPHONES_DELTA_UP,
	MASTER_TO_HEADPHONES,
	FILE_SCROLL,
	FILE_SCROLL_DOWN_MANY,
	FILE_SCROLL_UP_MANY,
	FILE_SCROLL_PREV,
	FILE_SCROLL_NEXT,
	FILE_SELECT,
	FILE_MARK_UNOPENED,
	FILE_REFRESH,
	WAVEFORM_EJECT,
	WAVEFORM_PAUSE_TOGGLE,
	WAVEFORM_NUDGE,
	WAVEFORM_NUDGE_BACKWARD,
	WAVEFORM_NUDGE_FORWARD,
	WAVEFORM_PITCHBEND,
	WAVEFORM_PITCHBEND_DELTA_DOWN_SLOW,
	WAVEFORM_PITCHBEND_DELTA_UP_SLOW,
	WAVEFORM_PITCHBEND_DELTA_DOWN,
	WAVEFORM_PITCHBEND_DELTA_UP,
	WAVEFORM_PITCHBEND_DELTA_DOWN_FAST,
	WAVEFORM_PITCHBEND_DELTA_UP_FAST,
	WAVEFORM_EQ_LOW,
	WAVEFORM_EQ_LOW_DELTA_DOWN,
	WAVEFORM_EQ_LOW_DELTA_UP,
	WAVEFORM_EQ_LOW_KILL,
	WAVEFORM_EQ_MID,
	WAVEFORM_EQ_MID_DELTA_DOWN,
	WAVEFORM_EQ_MID_DELTA_UP,
	WAVEFORM_EQ_MID_KILL,
	WAVEFORM_EQ_HIGH,
	WAVEFORM_EQ_HIGH_DELTA_DOWN,
	WAVEFORM_EQ_HIGH_DELTA_UP,
	WAVEFORM_EQ_HIGH_KILL,
	WAVEFORM_GAIN,
	WAVEFORM_GAIN_DELTA_DOWN,
	WAVEFORM_GAIN_DELTA_UP,
	WAVEFORM_GAIN_KILL,
	WAVEFORM_VOLUME,
	WAVEFORM_VOLUME_INVERT,
	WAVEFORM_RHYTHMIC_VOLUME_INVERT,
	WAVEFORM_RHYTHMIC_VOLUME_INVERT_OTHER,
	WAVEFORM_VOLUME_SOLO,
	WAVEFORM_SEEK_BACKWARD_SLOW,
	WAVEFORM_SEEK_BACKWARD_FAST,
	WAVEFORM_SEEK_FORWARD_SLOW,
	WAVEFORM_SEEK_FORWARD_FAST,
	WAVEFORM_SCRATCH_SPEED,
	WAVEFORM_STUTTER,
	WAVEFORM_SAVEPOINT_PREV,
	WAVEFORM_SAVEPOINT_NEXT,
	WAVEFORM_SAVEPOINT_SET,
	WAVEFORM_SAVEPOINT_SHIFT_BACKWARD,
	WAVEFORM_SAVEPOINT_SHIFT_FORWARD,
	WAVEFORM_SAVEPOINT_SHIFT_ALL_BACKWARD,
	WAVEFORM_SAVEPOINT_SHIFT_ALL_FORWARD,
	WAVEFORM_SAVEPOINT_JUMP_NOW,
	WAVEFORM_SAVEPOINT_JUMP_AT_MEASURE,
	WAVEFORM_QUANTIZATION_PERIOD_HALF,
	WAVEFORM_QUANTIZATION_PERIOD_DOUBLE,
	WAVEFORM_LOOP_TOGGLE,
	WAVEFORM_LOOP_THEN_RECALL,
	WAVEFORM_AUTO_DIVERGE_THEN_RECALL,
	WAVEFORM_VIDEO_SELECT,
	WAVEFORM_VIDEO_BRIGHTNESS,
	WAVEFORM_FREQ_SENSE_BRIGHTNESS,
	WAVEFORM_OSCILLOSCOPE_BRIGHTNESS,
	WAVEFORM_AUDIO_INPUT_TOGGLE,
	WAVEFORM_VIDEO_ASPECT_RATIO_NEXT,
	WAVEFORM_SYNC,
	FULL_SCREEN_TOGGLE,
	VISUALIZER_FULL_SCREEN_TOGGLE,
	SCREENSHOT,
	ACTION_LAST
} DVJ_Action;

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

bool
GetDVJSessionRecordAudio();

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

int
GetAudioSamplePeriod();

bool
GetWireMemory();

bool
GetEscDuringScanExits();

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
int	GetInputKeyboardFileScrollDownManyKey();
int	GetInputKeyboardFileScrollUpManyKey();
int	GetInputKeyboardFileScrollPrevKey();
int	GetInputKeyboardFileScrollNextKey();
int	GetInputKeyboardFileSelectKey();
int	GetInputKeyboardFileMarkUnopenedKey();
int	GetInputKeyboardFileRefreshKey();
int	GetInputKeyboardWaveformEjectKey();
int	GetInputKeyboardWaveformPauseToggleKey();
int	GetInputKeyboardWaveformNudgeBackwardKey();
int	GetInputKeyboardWaveformNudgeForwardKey();
int	GetInputKeyboardWaveformPitchbendDeltaDownSlowKey();
int	GetInputKeyboardWaveformPitchbendDeltaUpSlowKey();
int	GetInputKeyboardWaveformPitchbendDeltaDownKey();
int	GetInputKeyboardWaveformPitchbendDeltaUpKey();
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
int	GetInputKeyboardWaveformRhythmicVolumeInvertKey();
int	GetInputKeyboardWaveformRhythmicVolumeInvertOtherKey();
int	GetInputKeyboardWaveformVolumeSoloKey();
int	GetInputKeyboardWaveformSeekBackwardFastKey();
int	GetInputKeyboardWaveformSeekForwardFastKey();
int	GetInputKeyboardWaveformSeekBackwardSlowKey();
int	GetInputKeyboardWaveformSeekForwardSlowKey();
int	GetInputKeyboardWaveformSavePointPrevKey();
int	GetInputKeyboardWaveformSavePointNextKey();
int	GetInputKeyboardWaveformSavePointSetKey();
int	GetInputKeyboardWaveformSavePointShiftBackwardKey();
int	GetInputKeyboardWaveformSavePointShiftForwardKey();
int	GetInputKeyboardWaveformSavePointShiftAllBackwardKey();
int	GetInputKeyboardWaveformSavePointShiftAllForwardKey();
int	GetInputKeyboardWaveformSavePointJumpNowKey();
int	GetInputKeyboardWaveformSavePointJumpAtMeasureKey();
int	GetInputKeyboardWaveformQuantizationPeriodHalfKey();
int	GetInputKeyboardWaveformQuantizationPeriodDoubleKey();
int	GetInputKeyboardWaveformStutterKey();
int	GetInputKeyboardWaveformLoopToggleKey();
int	GetInputKeyboardWaveformLoopThenRecallKey();
int	GetInputKeyboardWaveformAutoDivergeRecallKey();
int	GetInputKeyboardWaveformVideoSelectKey();
int	GetInputKeyboardWaveformAudioInputToggleKey();
int	GetInputKeyboardWaveformVideoAspectRatioNextKey();
int	GetInputKeyboardWaveformSyncKey();
int	GetInputKeyboardFullScreenToggleKey();
int	GetInputKeyboardVisualizerFullScreenToggleKey();
int	GetInputKeyboardScreenshotKey();
//int	GetInputKeyboardWaveformKey();

int
GetOscServerPort();

std::vector<IpEndpointName>
GetOscClientList();

std::vector<int>
GetOscClientAutoPortList();

std::vector<const char*>
GetOscAddressPatternList
(
	DVJ_Action	action,
	int		target
);

#endif	//_DVJ_CONFIG_H_

