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
GetVideoBufferFrames();

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
int	GetInputKeyboardWaveformVideoFreqSenseModeKey();
int	GetInputKeyboardWaveformSyncBPMKey();
int	GetInputKeyboardRecordingStartKey();
int	GetInputKeyboardFullScreenToggleKey();
int	GetInputKeyboardVisualizerFullScreenToggleKey();
int	GetInputKeyboardScreenshotKey();
//int	GetInputKeyboardWaveformKey();

#endif	//_DVJ_CONFIG_H_

