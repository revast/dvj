/*
 *
 * DrumMachine.h
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

#ifndef _DVJ_DRUM_MACHINE_H_
#define	_DVJ_DRUM_MACHINE_H_

#include "LGL.module/LGL.h"

class DrumMachineObj
{

public:

			DrumMachineObj();
			~DrumMachineObj();
	
	void		NextFrame();
	void		Draw
			(
				bool	visualizerQuadrent,
				float	visualizerZoomOutPercent
			);

	void		NextKit();
	void		PrevKit();
	const
	char*		GetKitName();
	
	bool		GetActive();

private:

	void		LoadAllKits();
	bool		LoadNewKit();
	void		DeleteAllKits();

	LGL_DirTree	DirTree;

	bool		Active;
	int		KitIndex;
	LGL_Sound*	Kit[1024][2][4];
	char		KitNames[2][4][128];
	float		KitVolume;

	bool		GuitarStrumPossible;
	LGL_Timer	GuitarStrumLastTimer;
};

#endif	//_DVJ_DRUM_MACHINE_H_

