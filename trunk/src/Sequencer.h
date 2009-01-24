/*
 *
 * Sequencer.h
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

#ifndef _DVJ_SEQUENCER_H_
#define	_DVJ_SEQUENCER_H_

#include "LGL.module/LGL.h"

#define	SOUND_NUM 44

class SequencerObj
{

public:

			SequencerObj();
			~SequencerObj();
	
	void		NextFrame();
	void		Draw
			(
				bool	visualizerQuadrent,
				float	visualizerZoomOutPercent
			);
	
	void		SetBPMInfo
			(
				float	bpm,
				float	secondsAfterFirstBeat,
				float	speed
			);
	void		SetVolumeFrontBack
			(
				float	volumeFront,
				float	volumeBack
			);
	void		SetEQ
			(
				float	low,
				float	mid,
				float	high
			);

private:

	void		NextFrameBack();
	void		MaybeSwap();
	void		ProcessInput();

	char		ClipFrontPath[SOUND_NUM][1024];
	char		ClipBackPath[SOUND_NUM][1024];
	int		ClipChannel[SOUND_NUM];

	LGL_Sound*	ClipFront[SOUND_NUM];
	LGL_Sound*	ClipBack[SOUND_NUM];
	char*		ClipMem[SOUND_NUM];

	float		BPMInfoBPM;
	float		BPMInfoSecondsAfterFirstBeat;
	float		BPMInfoSpeed;

	float		VolumeFront;
	float		VolumeBack;
	float		EQ[3];
	float		FreqResponse[513];
};

#endif	//_DVJ_SEQUENCER_H_

