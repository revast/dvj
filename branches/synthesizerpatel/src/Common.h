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

#define	EIGHT_WAY (0)
#define	POINTER_PARTICLES_PER_SECOND (250)
#define	WAVE_WIDTH_PERCENT (0.60f)

#include "LGL.h"

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
	float	visualizerZoomOutPercent
);

void
Mixer_DrawGlowLinesTurntables
(
	float	time,
	float	crossFadeSliderLeft,
	float	crossFadeSliderRight,
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
	float viewPortBottom,
	float viewPortTop,
	float leftBottomLevel,
	float leftTopLevel,
	float rightBottomLevel,
	float rightTopLevel,
	bool  visualizerQuadrent,
	float visualizerZoomOutPercent
);

void
Turntable_DrawDirTree
(
	float		time,
	const char*	filterText,
	const char*	path,
	const char**	nameArray,
	bool*		isDirBits,
	int		fileNum,
	int		fileSelectInt,
	float		viewPortBottom,
	float		viewPortTop,
	float		badFileFlash,
	float*		inBPMList
);

void
Turntable_DrawWaveform
(
	LGL_Sound*	sound,
	bool		loaded,
	const char*	videoPathShort,
	bool		glitch,
	float		glitchBegin,
	float		glitchLength,
	double		soundPositionSamples,
	double		soundLengthSamples,
	float		soundSpeed,
	float		pitchBend,
	float		grainStreamCrossfader,
	float		grainStreamSourcePoint,
	float		grainStreamLength,
	float		grainStreamPitch,
	float		viewPortLeft,
	float		viewPortRight,
	float		viewPortBottom,
	float		viewPortTop,
	float		volumeMultiplierNow,
	float		centerX,
	bool		pause,
	float		nudge,
	float		joyAnalogueStatusLeftX,
	float		time,
	double*		savePointSeconds,
	int		savePointIndex,
	int		savePointIndexActual,
	unsigned int	savePointSetBitfield,
	float*		savePointUnsetNoisePercent,
	float*		savePointUnsetFlashPercent,
	float		bpm,
	float		bpmAdjusted,
	float		bpmFirstBeatSeconds,
	float		eq0,
	float		eq1,
	float		eq2,
	bool		lowRez,
	int&		entireWaveArrayFillIndex,
	int		entireWaveArrayCount,
	float*		entireWaveArrayMagnitudeAve,
	float*		entireWaveArrayMagnitudeMax,
	float*		entireWaveArrayFreqFactor,
	float		cachedLengthSeconds,
	LGL_Image*	noiseImage256x64,
	int		freqSensitiveMode
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
	float	vol
);
