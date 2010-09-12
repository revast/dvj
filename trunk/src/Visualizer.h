/*
 *
 * Visualizer.h
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

#ifndef	_DVJ_VISUALIZER_H_
#define	_DVJ_VISUALIZER_H_

#include "LGL.module/LGL.h"
#include <vector>

class TurntableObj;

using namespace std;

#define	NOISE_IMAGE_COUNT_128_128 64

class VisualizerObj
{

public:

				VisualizerObj();
				~VisualizerObj();

	void			NextFrame(float secondsElapsed);
	void			DrawVisuals
				(
					bool		visualizerQuadrent,
					float		visualizerZoomOutPercent,
					TurntableObj**	tts
				);

	void			SetViewportVisuals
				(
					float	left,	float	right,
					float	bottom,	float	top
				);
	float			GetViewportVisualsWidth();
	float			GetViewportVisualsHeight();
	float			GetViewportRight();
	void			ToggleFullScreen();

	void			QueueScrollText(const char* text);
	void			MaybeSetScrollTextTrackFile(const char* trackFileName);
	bool			GetScrollTextEnabled();

	bool			IsVideoAvailable() const;
	void			SetVideos
				(
					LGL_VideoDecoder* video0,	float videoBrightness0,
					LGL_VideoDecoder* video1,	float videoBrightness1
				);
	void			SetSoundsLoaded(bool loaded0, bool loaded1);
	LGL_VideoDecoder*	GetVideo(int which);
	void			SetFrequencySensitiveVideos
				(
					LGL_VideoDecoder* video0l, LGL_VideoDecoder* video0h, float volAve0, float volMax0, float freqFactor0, int mode0,
					LGL_VideoDecoder* video1l, LGL_VideoDecoder* video1h, float volAve1, float volMax1, float freqFactor1, int mode1
				);
	void			SetFrequencySensitiveGainEQ
				(
					float	gain0,	float	eqLo0,	float	eqHi0,
					float	gain1,	float	eqLo1,	float	eqHi1
				);

	void			GetNextVideoPathRandom(char* path);
	void			ForceVideoToBackOfRandomQueue
				(
					const
					char*	pathShort
				);

private:

	void			PopulateCharStarBufferWithScrollTextFile(std::vector<char*>& buffer, const char* path);

	float			ViewportVisualsLeft;
	float			ViewportVisualsRight;
	float			ViewportVisualsBottom;
	float			ViewportVisualsTop;
	float			ViewportVisualsWidth;
	float			ViewportVisualsHeight;

	bool			FullScreen;

	//LGL_Image*		AccumulationNow;

	LGL_Image*		NoSound;
	LGL_Image*		BlueScreenOfDeath;

	std::vector<char*>	ScrollTextBuffer;
	char			ScrollTextCurrentAmbientFile[1024];
	std::vector<char*>	ScrollTextCurrentAmbientFileBuffer;
	std::vector<char*>	ScrollTextCurrentTrackBuffer;
	char			ScrollTextCurrentTrackFile[1024];
	float			ScrollTextBottomLeftPosition;
	bool			ScrollTextEnabled;
	std::vector<char*>	ScrollTextAmbientFileQueue;
	std::vector<char*>	ScrollTextAmbientFileQueueUsed;

	bool			SoundsLoaded[2];

	LGL_VideoDecoder*	Videos[2];
	float			VideoBrightness[2];
	float			VideoFPSDisplay;

	int			FreqMode[2];
	LGL_VideoDecoder*	FreqVideos[4];
	float			FreqVolume[2];
	float			FreqFreqFactor[2];
	float			FreqGain[2];
	float			FreqEQLo[2];
	float			FreqEQHi[2];

public:
	void			DrawVideos
				(
					bool		preview,
					int		which,
					TurntableObj*	tt,
					float		l,
					float		r,
					float		b,
					float		t,
					float		overrideBrightness=-1.0f
				);

private:
	char			VideoRandomPath[1024];
	std::vector<char*>	VideoRandomQueue;
	unsigned int		VideoRandomGetCount;
	
	float			NoiseFactor[2];
static	LGL_Image*		NoiseImage[NOISE_IMAGE_COUNT_128_128];

};

#endif	//_DVJ_VISUALIZER_H_

