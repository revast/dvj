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

class MixerObj;

MixerObj& GetMixer();

class MixerObj
{
	
public:

					MixerObj();
					~MixerObj();
	
	void				Cleanup();

	void				NextFrame(float secondsElapsed);
	void				DrawFrame(bool visualizerQuadrent, float visualizerZoomOutPercent);

	void				SetViewport
					(
						float	left,	float	right,
						float	bottom,	float	top
					);
	void				SetVisualizer(VisualizerObj* viz);

	void				SetVolumeMaster(float volumeMaster);
	void				SetTurntable(int index);
	TurntableObj*			GetTurntable(int index);
	TurntableObj**			GetTurntables();
	
	void				SetRecording(bool inRecording=true);
	bool				GetRecording() const;
	void				SetRecordingFailed();
	
	void				SetViewportStatus
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

	float				ViewportLeft;
	float				ViewportRight;
	float				ViewportBottom;
	float				ViewportTop;
	float				ViewportWidth;
	float				ViewportHeight;
	
	TurntableObj*			Turntable[2];
	int				Focus;
	bool				KillSwitch[2];
	bool				FullSwitch[2];
	float				VolumeMaster;

	float				EQXponent[3];
	float				EQJP8k[3];

	float				CrossfadeMiddle;
	float				CrossfadeSliderLeft;
	float				CrossfadeSliderRight;

	VisualizerObj*			Visualizer;

	float				ViewportStatusLeft;
	float				ViewportStatusRight;
	float				ViewportStatusBottom;
	float				ViewportStatusTop;
	float				ViewportStatusWidth;
	float				ViewportStatusHeight;

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
