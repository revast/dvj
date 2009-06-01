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

using namespace std;

#define	NOISE_IMAGE_COUNT_128_128 64

class VisualizerObj
{

public:

				VisualizerObj();
				~VisualizerObj();

	void			NextFrame(float secondsElapsed);
	void			DrawVisuals(bool visualizerQuadrent, float visualizerZoomOutPercent);

	void			SetViewPortVisuals
				(
					float	left,	float	right,
					float	bottom,	float	top
				);
	void			ToggleFullScreen();

	void			SetImageSetPrefix(char* prefix);
	void			SetMovieClipPrefix(char* prefix);
	
	void			QueueScrollText(const char* text);
	void			MaybeSetScrollTextTrackFile(const char* trackFileName);
	bool			GetScrollTextEnabled();

	bool			IsVideoAvailable() const;
	void			SetVideos
				(
					LGL_Video* video0,	float videoBrightness0,
					LGL_Video* video1,	float videoBrightness1
				);
	LGL_Video*		GetVideo(int which);
	void			SetFrequencySensitiveVideos
				(
					LGL_Video* video0l, LGL_Video* video0h, float volAve0, float volMax0, float freqFactor0, int mode0,
					LGL_Video* video1l, LGL_Video* video1h, float volAve1, float volMax1, float freqFactor1, int mode1
				);
	void			SetFrequencySensitiveGainEQ
				(
					float	gain0,	float	eqLo0,	float	eqHi0,
					float	gain1,	float	eqLo1,	float	eqHi1
				);

	void			GetNextVideoPathAmbient(char* path);
	void			GetNextVideoPathAmbientMellow(char* path);

private:

	void			LoadNewImageSet();
	void			LoadNewMovieClip();
	
public:
	
	void			LoadNewImageSetThread();
	void			LoadNewMovieClipThread();
	
private:

	int			PickRandomValidMovieClip();
	int			GetClipImageIndexNow(int whichClipInList);
	void			ForceVideoToBackOfAmbientQueue(const char* pathShort);
	void			PopulateCharStarBufferWithScrollTextFile(std::vector<char*>& buffer, const char* path);

	float			ViewPortVisualsLeft;
	float			ViewPortVisualsRight;
	float			ViewPortVisualsBottom;
	float			ViewPortVisualsTop;
	float			ViewPortVisualsWidth;
	float			ViewPortVisualsHeight;

	bool			FullScreen;

	bool			MovieMode;

	LGL_Image*		AccumulationNow;

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
	
	vector<LGL_Image*>	ImageSet;
	vector<LGL_Image*>	ImageSetNext;
	int			ImageSetNextStatus;
	LGL_Timer		ImageSetTimer;
	char			ImageSetName[256];
	int			ImageSetLastWhich;
	float			ImageSetLastPeak;
	float			ImageSetThreashold;
	char			ImageSetPrefix[256];

	int			MovieClipNum;
	int			MovieClipNow[4];
	int			MovieClipSimultaneous;
	vector<LGL_Image*>	MovieClipList[8];
	vector<LGL_Image*>	MovieClipLoading;
	int			MovieClipLoadingStatus;
	char			MovieClipLoadingDir[1024];
	float			MovieClipScratchL;
	float			MovieClipScratchR;
	float			MovieClipGlitch;
	bool			MovieClipSlideDown[8];
	float			MovieClipSlideDownTimeNow[8];
	float			MovieClipSlideDownTimeMax[8];
	int			MovieClipSlideDownNum[8];
	float			MovieClipSlideDownNow[8][128];
	float			MovieClipSlideDownDelta[8][128];
	LGL_Timer		MovieClipTimerGlobal;
	LGL_Timer		MovieClipTimer[8];
	float			MovieClipAlpha[8];
	bool			MovieClipQuad[8];
	char			MovieClipPrefix[256];

	float			JoyAuxTimeLast;
	float			JoyAuxScratch;
	float			JoyAuxScratchTimer;
	float			JoyAuxSlideXNow;
	int			JoyAuxSlideXNum;
	float			JoyAuxSlideXDelta[64];
	float			JoyAuxSlideXMomentum;
	int			JoyAuxStrobeDelay;
	bool			JoyAuxStrobeNow;

	bool			DoNotLoadImages;

	bool			VideoAvailable;

	LGL_Video*		Videos[2];
	float			VideoBrightness[2];

	int			FreqMode[2];
	LGL_Video*		FreqVideos[4];
	float			FreqVolume[2];
	float			FreqFreqFactor[2];
	float			FreqGain[2];
	float			FreqEQLo[2];
	float			FreqEQHi[2];

public:
	void			DrawVideos(int which, float l, float r, float b, float t, bool fullBrightness=false);

private:
	char			VideoAmbientPath[1024];
	std::vector<char*>	VideoAmbientQueue;
	unsigned int		VideoAmbientGetCount;
	
	float			NoiseFactor[2];
static	LGL_Image*		NoiseImage[NOISE_IMAGE_COUNT_128_128];

};

#endif	//_DVJ_VISUALIZER_H_
