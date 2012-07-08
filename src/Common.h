/*
 *
 * Common.h
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

#ifndef	_DVJ_COMMON_H_
#define	_DVJ_COMMON_H_

#define	EIGHT_WAY (0)
#define	POINTER_PARTICLES_PER_SECOND (250)

#include "LGL.module/LGL.h"

void
DrawLoadScreen
(
	float		loadScreenPercent,
	const char*	line1 = NULL,
	const char*	line2 = NULL,
	const char*	line3 = NULL,
	float		line3Brightness = 1.0f
);

float
GetGlowFromTime
(
	float	time
);

void
Main_DrawGlowLines
(
	float	time,
	float	brightness,
	bool	visualizerQuadrent,
	float	visualizerZoomOutPercent,
	float	visualizerRight
);

void
Mixer_DrawGlowLinesTurntables
(
	float	time,
	float	crossFadeSliderLeft,
	float	crossFadeSliderRight,
	bool	beatThisFrameBottom,
	bool	beatThisFrameTop,
	float	percentOfCurrentBeatBottom,
	float	percentOfCurrentBeatTop,
	float	brightness,
	bool	visualizerQuadrent,
	float	visualizerZoomOutPercent
);

void
Mixer_DrawGlowLinesStatus
(
	float	time,
	float	brightness,
	bool	visualizerQuadrent,
	float	visualizerZoomOutPercent
);

void
Mixer_DrawLevels
(
	float	viewportBottom,
	float	viewportTop,
	float	leftBottomLevel,
	float	leftTopLevel,
	float	rightBottomLevel,
	float	rightTopLevel,
	bool	visualizerQuadrent,
	float	visualizerZoomOutPercent
);

void
Turntable_DrawWaveform
(
	int		which,
	LGL_Sound*	sound,
	bool		loaded,
	int		mode,
	const char*	videoPathShort,
	bool		glitch,
	float		glitchBegin,
	float		glitchLength,
	double		soundPositionSamples,
	double		soundLengthSamples,
	float		soundSpeed,
	float		pitchbend,
	float		grainStreamCrossfader,
	float		grainStreamSourcePoint,
	float		grainStreamLength,
	float		grainStreamPitch,
	float		viewportLeft,
	float		viewportRight,
	float		viewportBottom,
	float		viewportTop,
	float		volumeMultiplierNow,
	float		centerX,
	bool		pause,
	float		nudge,
	float		joyAnalogueStatusLeftX,
	float		time,
	double*		savepointSeconds,
	double*		savepointBPMs,
	int		savepointIndex,
	int		savepointIndexActual,
	unsigned int	savepointSetBitfield,
	unsigned int	savepointSetBPMBitfield,
	float*		savepointUnsetNoisePercent,
	float*		savepointUnsetFlashPercent,
	float		bpm,
	float		bpmAdjusted,
	const char*	bpmCandidate,
	float		bpmFirstBeatSeconds,
	float		eq0,
	float		eq1,
	float		eq2,
	float		eqVuMe0,
	float		eqVuMe1,
	float		eqVuMe2,
	float		eqVuOther0,
	float		eqVuOther1,
	float		eqVuOther2,
	float		eqVuMePeak0,
	float		eqVuMePeak1,
	float		eqVuMePeak2,
	float		eqVuOtherPeak0,
	float		eqVuOtherPeak1,
	float		eqVuOtherPeak2,
	float		vu,
	float		vuPeak,
	float		otherVu,
	float		otherVuPeak,
	bool		lowRez,
	int&		entireWaveArrayFillIndex,
	int		entireWaveArrayCount,
	float*		entireWaveArrayMagnitudeAve,
	float*		entireWaveArrayMagnitudeMax,
	float*		entireWaveArrayFreqFactor,
	float		cachedLengthSeconds,
	LGL_Image*	noiseImage256x64,
	LGL_Image*	loopImage,
	bool		audioInputMode,
	float		warpPointSecondsStart,
	float		warpPointSecondsTrigger,
	int		loopExponent,
	float		loopSeconds,
	bool		waveformRecordHold,
	const char*	soundName,
	float		videoSecondsBufferedLeft,
	float		videoSecondsBufferedRight,
	float		videoSecondsLoadedLeft,
	float		videoSecondsLoadedRight,
	bool		isMaster,
	bool		rapidVolumeInvertSelf,
	float		beginningOfCurrentMeasureSeconds,
	float		videoBrightness,
	float		syphonBrightness,
	float		oscilloscopeBrightness,
	float		freqSenseBrightness,
	float		freqSensePathBrightness,
	const char*	freqSenseLowPathShort,
	const char*	freqSenseHighPathShort,
	float		freqSenseLowBitrateMBps,
	float		freqSenseHighBitrateMBps,
	float		freqSenseLEDBrightness,
	float		freqSenseLEDColorScalarLow,
	float		freqSenseLEDColorScalarHigh,
	float		freqSenseLEDBrightnessWash,
	float		freqSenseLEDGroupFloat,
	int		freqSenseLEDGroupInt,
	int		channel,
	float		recallPos
);

void
Turntable_DrawBPMString
(
	float		viewportLeft,
	float		viewportRight,
	float		viewportBottom,
	float		viewportTop,
	const char*	bpmString
);

void
Turntable_DrawPitchbendString
(
	float		viewportLeft,
	float		viewportRight,
	float		viewportBottom,
	float		viewportTop,
	float		pitchbend,
	float		nudge
);

void
Turntable_DrawPitchbendString
(
	float	viewportLeft,
	float	viewportRight,
	float	viewportBottom,
	float	viewportTop,
	float	pitchbend,
	float	nudge
);

void
Turntable_DrawSavepointSet
(
	float		viewportLeft,
	float		viewportRight,
	float		viewportBottom,
	float		viewportTop,
	float		time,
	double*		savepointSeconds,
	double*		savepointBPMs,
	int		savepointIndex,
	int		savepointIndexActual,
	unsigned int	savepointSetBitfield,
	unsigned int	savepointSetBPMBitfield,
	float*		savepointUnsetNoisePercent,
	float*		savepointUnsetFlashPercent,
	LGL_Image*	noiseImage256x64,
	int		which,
	int		mode,
	float		pitchbend
);

void
Turntable_DrawSliders
(
	int		which,
	float		viewportLeft,
	float		viewportRight,
	float		viewportBottom,
	float		viewportTop,
	float		eq0,
	float		eq1,
	float		eq2,
	float		eqVuMe0,
	float		eqVuMe1,
	float		eqVuMe2,
	float		eqVuOther0,
	float		eqVuOther1,
	float		eqVuOther2,
	float		eqVuMePeak0,
	float		eqVuMePeak1,
	float		eqVuMePeak2,
	float		eqVuOtherPeak0,
	float		eqVuOtherPeak1,
	float		eqVuOtherPeak2,
	float		vu,
	float		vuPeak,
	float		otherVu,
	float		otherVuPeak,
	float		gain,
	float		videoBrightness,
	float		syphonBrightness,
	float		oscilloscopeBrightness,
	float		freqSenseBrightness,
	float		freqSenseLEDBrightness,
	float		freqSenseLEDColorScalarLow,
	float		freqSenseLEDColorScalarHigh,
	float		freqSenseLEDBrightnessWash,
	float		freqSenseLEDGroupFloat,
	int		freqSenseLEDGroupInt
);

void
Visualizer_DrawWaveform
(
	float*	waveformSamples,
	int	waveformSamplesCount,
	bool	fullscreen
);

float
GetFreqBrightness
(
	bool	hi,
	float	freqFactor,
	float	vol,
	float	exagertionFactor=0.5f
);

LGL_Color
GetColorFromScalar
(
	float	scalar
);

#endif	//_DVJ_COMMON_H_

