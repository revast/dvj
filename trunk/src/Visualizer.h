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

	void			NextFrame
				(
					float secondsElapsed,
					TurntableObj**	tts
				);
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
	void			SetVisualBrightness(int which, float brightness);
	void			SetVideoBrightness(int which, float brightness);
	void			SetOscilloscopeBrightness(int which, float brightness);
	void			SetFreqSenseBrightness(int which, float brightness);

	void			GetNextVideoPathRandomLow(char* path);
	void			GetNextVideoPathRandomHigh(char* path);
	void			GetNextVideoPathRandom(char* path, bool low);
	void			ForceVideoToBackOfRandomQueue
				(
					const
					char*	pathShort
				);
	
	bool			GetProjectorClear();
	void			SetProjectorClear(bool clear=true);
	bool			GetProjectorPreviewClear();
	void			SetProjectorPreviewClear(bool clear=true);

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

	float			VideoFPSDisplay;

public:
	void			DrawVideos
				(
					TurntableObj*	tt,
					float		l,
					float		r,
					float		b,
					float		t,
					bool		preview
				);

private:
	char			VideoRandomPath[2048];
	char			VideoRandomLowPath[2048];
	char			VideoRandomHighPath[2048];
	std::vector<char*>	VideoRandomLowQueue;
	std::vector<char*>	VideoRandomHighQueue;
	unsigned int		VideoRandomGetCount;
	float			LowMemoryWarningScalar;
	int			LowMemoryMB;
	LGL_Timer		LowMemoryTimer;

	bool			ProjectorClear;
	bool			ProjectorPreviewClear;
	
static	LGL_Image*		NoiseImage[NOISE_IMAGE_COUNT_128_128];

};

#endif	//_DVJ_VISUALIZER_H_

