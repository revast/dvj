/*
 *
 * ProjMapGrid.h
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

#ifndef	_DVJ_PROJMAPGRID_H_
#define	_DVJ_PROJMAPGRID_H_

#include "LGL.module/LGL.h"
#include <vector>

using namespace std;

#define	PROJ_MAP_GRID_DISPLAY_MAX (16)

class ProjMapGridObj
{

public:

				ProjMapGridObj();
				~ProjMapGridObj();

	void			Construct();
	void			Nullify();

	void			SetWidth(int width);
	void			SetHeight(int height);

	void			NextFrame();

	void			DrawSrcGrid();
	void			DrawDstGrids();

//private:

	void			DrawGrid(float* points);

	int			GetProjMapGridIndex(int x, int y);
	float			GetProjMapGridValueX(float* grid, int x, int y);
	float			GetProjMapGridValueY(float* grid, int x, int y);
	void			SetProjMapGridValueX(float* grid, int x, int y, float val);
	void			SetProjMapGridValueY(float* grid, int x, int y, float val);

	void			GetMouseIndexes(float* points, int display, int& inX, int& inY);

	int			GridW;
	int			GridH;
	float*			SrcPoints;
	float*			DstPoints[PROJ_MAP_GRID_DISPLAY_MAX];

	float*			SelectedPoints;
	int			SelectedIndexX;
	int			SelectedIndexY;
	int			SelectedDrag;
};

#endif	//_DVJ_VISUALIZER_H_
