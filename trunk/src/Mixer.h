/*
 *
 * Mixer.h
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

#ifndef	_DVJ_MIXER_H_
#define	_DVJ_MIXER_H_

#include "LGL.module/LGL.h"
#include "Turntable.h"

#include "Database.h"

#include "Visualizer.h"

class MixerObj
{
	
public:

					MixerObj();
					~MixerObj();

	void				NextFrame(float secondsElapsed);
	void				DrawFrame(bool visualizerQuadrent, float visualizerZoomOutPercent);

	void				SetViewPort
					(
						float	left,	float	right,
						float	bottom,	float	top
					);
	void				SetVisualizer(VisualizerObj* viz);

	void				SetVolumeMaster(float volumeMaster);
	void				SetTurntable(int index);
	
	void				SetRecording(bool inRecording=true);
	bool				GetRecording() const;
	void				SetRecordingFailed();
	
	void				SetViewPortStatus
					(
						float	left,	float	right,
						float	bottom,	float	top
					);

	void				SetLowRez(bool lowRez);
	void				BlankFocusFilterText();

private:

	void				DrawStatus
					(
						float glow,
						bool	visualizerQuadrent,
						float	visualizerZoomOutPercent
					);

	float				ViewPortLeft;
	float				ViewPortRight;
	float				ViewPortBottom;
	float				ViewPortTop;
	float				ViewPortWidth;
	float				ViewPortHeight;
	
	TurntableObj*			Turntable[2];
	int				Focus;
	bool				KillSwitch[2];
	bool				FullSwitch[2];
	float				VolumeMaster;

	float				EQXponent[3];
	float				EQJP8k[3];

	float				CrossFadeMiddle;
	float				CrossFadeSliderLeft;
	float				CrossFadeSliderRight;

	VisualizerObj*			Visualizer;

	float				ViewPortStatusLeft;
	float				ViewPortStatusRight;
	float				ViewPortStatusBottom;
	float				ViewPortStatusTop;
	float				ViewPortStatusWidth;
	float				ViewPortStatusHeight;

	bool				Recording;
	bool				RecordingDetermined;
	float				RecordingSecondsSinceExecution;
	char				RecordingTrackListPath[2048];
	FILE*				RecordingTrackListFD;
	float				RecordingFailedTimer;

	int				VideoAdvancedLastFrame;

	unsigned int			CPUSpeedHighest;
	bool				LowRez;
	bool				CanDisplayJackWarning;

	LGL_Timer			SongTitleAsScrollTextTimer;

	DatabaseObj			Database;
};

#endif	//_DVJ_MIXER_H_
