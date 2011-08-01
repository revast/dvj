/*
 *
 * ProjMapGrid.cpp
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

#include "ProjMapGrid.h"

#include <string.h>

ProjMapGridObj::
ProjMapGridObj()
{
	GridW=0;
	GridH=0;
	SrcPoints=NULL;
	DstPoints=NULL;
	SelectedIndexX=-1;
	SelectedIndexY=-1;
	SetWidth(4);
	SetHeight(4);

#if 0
	for(int x=0;x<GridW;x++)
	{
		for(int y=0;y<GridH;y++)
		{
			printf
			(
				"(%i, %i): %.2f, %.2f\n",
				x,
				y,
				GetProjMapGridValueX(SrcPoints,x,y),
				GetProjMapGridValueY(SrcPoints,x,y)
			);
		}
	}
#endif
}

ProjMapGridObj::
~ProjMapGridObj()
{
	Nullify();
}

void
ProjMapGridObj::
Construct()
{
	int oldW=GridW;
	int oldH=GridH;
	Nullify();
	GridW=oldW;
	GridH=oldH;

	SrcPoints = new float[GridW*GridH*2];
	DstPoints = new float[GridW*GridH*2];

	float l=0.0f;
	float r=1.0f;
	float b=0.0f;
	float t=1.0f;
	float w=r-l;
	float h=t-b;

	for(int x=0;x<GridW;x++)
	{
		for(int y=0;y<GridH;y++)
		{
			SetProjMapGridValueX
			(
				SrcPoints,
				x,y,
				l+w*(x/(GridW-1.0f))
			);
			SetProjMapGridValueY
			(
				SrcPoints,
				x,y,
				b+h*(y/(GridH-1.0f))
			);
			SetProjMapGridValueX
			(
				DstPoints,
				x,y,
				x/(GridW-1.0f)
			);
			SetProjMapGridValueY
			(
				DstPoints,
				x,y,
				y/(GridH-1.0f)
			);
		}
	}
}

void
ProjMapGridObj::
Nullify()
{

	if(SrcPoints)
	{
		delete SrcPoints;
		SrcPoints=NULL;
	}
	if(DstPoints)
	{
		delete DstPoints;
		DstPoints=NULL;
	}

	GridW=0;
	GridH=0;
}

void
ProjMapGridObj::
SetWidth
(
	int	width
)
{
	if(width==GridW)
	{
		return;
	}

	GridW=LGL_Clamp(2,width,32);
	Construct();
}

void
ProjMapGridObj::
SetHeight
(
	int	height
)
{
	if(height==GridH)
	{
		return;
	}

	GridH=LGL_Clamp(2,height,32);
	Construct();
}

void
ProjMapGridObj::
NextFrame()
{
	if(LGL_KeyDown(PROJ_MAP_KEY)==false)
	{
		SelectedIndexX=-1;
		SelectedIndexY=-1;
		SelectedDrag=false;
		return;
	}

	int selectedIndexXOld=SelectedIndexX;
	int selectedIndexYOld=SelectedIndexY;

	SelectedIndexX=-1;
	SelectedIndexY=-1;
	SelectedDrag=LGL_MouseDown(LGL_MOUSE_LEFT);

	GetMouseIndexes(SrcPoints,0,SelectedIndexX,SelectedIndexY);

	if
	(
		SelectedDrag &&
		(
			(
				SelectedIndexX==-1 ||
				SelectedIndexY==-1
			) ||
			(
				SelectedIndexX!=-1 &&
				SelectedIndexX!=selectedIndexXOld
			) ||
			(
				SelectedIndexY!=-1 &&
				SelectedIndexY!=selectedIndexYOld
			)
		)
	)
	{
		SelectedIndexX=selectedIndexXOld;
		SelectedIndexY=selectedIndexYOld;
	}

	if
	(
		SelectedIndexX!=-1 &&
		SelectedIndexY!=-1
	)
	{
		if(SelectedDrag)
		{
			float dxMult=1.0f;
			float dyMult=1.0f;
			if(LGL_GetActiveDisplay()==0)
			{
				float l=0.0f;
				float r=1.0f;
				float b=0.0f;
				float t=1.0f;
				GetVisualizer()->GetProjectorARCoordsFromWindowCoords
				(
					l,
					b
				);
				GetVisualizer()->GetProjectorARCoordsFromWindowCoords
				(
					r,
					t
				);

				dxMult=r-l;
				dyMult=t-b;
			}
			SetProjMapGridValueX
			(
				SrcPoints,
				SelectedIndexX,
				SelectedIndexY,
				GetProjMapGridValueX
				(
					SrcPoints,
					SelectedIndexX,
					SelectedIndexY
				)+LGL_MouseDX()*dxMult
			);
			SetProjMapGridValueY
			(
				SrcPoints,
				SelectedIndexX,
				SelectedIndexY,
				GetProjMapGridValueY
				(
					SrcPoints,
					SelectedIndexX,
					SelectedIndexY
				)+LGL_MouseDY()*dyMult
			);
		}
	}

	if
	(
		SelectedIndexX!=-1 &&
		SelectedIndexY!=-1
	)
	{
		float dx=0.0f;
		float dy=0.0f;
		if(LGL_KeyDown(LGL_KEY_A))
		{
			dx-=1.0f;
		}
		if(LGL_KeyDown(LGL_KEY_D))
		{
			dx+=1.0f;
		}
		if(LGL_KeyDown(LGL_KEY_S))
		{
			dy-=1.0f;
		}
		if(LGL_KeyDown(LGL_KEY_W))
		{
			dy+=1.0f;
		}

		const float speed=0.1f;
		if(dx!=0.0f)
		{
			dx*=LGL_SecondsSinceLastFrame()*speed;
			SetProjMapGridValueX
			(
				DstPoints,
				SelectedIndexX,
				SelectedIndexY,
				GetProjMapGridValueX
				(
					DstPoints,
					SelectedIndexX,
					SelectedIndexY
				)+dx
			);
		}
		if(dy!=0.0f)
		{
			dy*=LGL_SecondsSinceLastFrame()*speed;
			SetProjMapGridValueY
			(
				DstPoints,
				SelectedIndexX,
				SelectedIndexY,
				GetProjMapGridValueY
				(
					DstPoints,
					SelectedIndexX,
					SelectedIndexY
				)+dy
			);
		}
	}
}

void
ProjMapGridObj::
DrawSrcGrid()
{
	DrawGrid(SrcPoints);
}

void
ProjMapGridObj::
DrawDstGrid()
{
	DrawGrid(DstPoints);
}

void
ProjMapGridObj::
DrawGrid
(
	float*	points
)
{
	if(points==NULL)
	{
		return;
	}

	for(int l=0;l<2;l++)
	{
		float thickness=1.0f;
		float br=1.0f;
		if(l==0)
		{
			thickness=3.0f;
			br=0.0f;
		}
	
		for(int x=0;x<GridW;x++)
		{
			for(int y=0;y<GridH;y++)
			{
				if(x<GridW-1)
				{
					float p1X=GetProjMapGridValueX(points,x,y);
					float p1Y=GetProjMapGridValueY(points,x,y);
					float p2X=GetProjMapGridValueX(points,x+1,y);
					float p2Y=GetProjMapGridValueY(points,x+1,y);
					if(LGL_GetActiveDisplay()==0)
					{
						GetVisualizer()->GetWindowARCoordsFromProjectorCoords
						(
							p1X,
							p1Y
						);
						GetVisualizer()->GetWindowARCoordsFromProjectorCoords
						(
							p2X,
							p2Y
						);
					}
					LGL_DrawLineToScreen
					(
						p1X,p1Y,
						p2X,p2Y,
						br,br,br,1.0f,
						thickness
					);
				}
				if(y<GridH-1)
				{
					float p1X=GetProjMapGridValueX(points,x,y);
					float p1Y=GetProjMapGridValueY(points,x,y);
					float p2X=GetProjMapGridValueX(points,x,y+1);
					float p2Y=GetProjMapGridValueY(points,x,y+1);
					if(LGL_GetActiveDisplay()==0)
					{
						GetVisualizer()->GetWindowARCoordsFromProjectorCoords
						(
							p1X,
							p1Y
						);
						GetVisualizer()->GetWindowARCoordsFromProjectorCoords
						(
							p2X,
							p2Y
						);
					}
					LGL_DrawLineToScreen
					(
						p1X,p1Y,
						p2X,p2Y,
						br,br,br,1.0f,
						thickness
					);
				}
			}
		}
	}

	const float pointRad=0.005f;
	float pointRadX=pointRad;
	float pointRadY=pointRad*LGL_WindowAspectRatio();

	int mouseIndexX=SelectedIndexX;
	int mouseIndexY=SelectedIndexY;

	for(int x=0;x<GridW;x++)
	{
		for(int y=0;y<GridH;y++)
		{
			if
			(
				mouseIndexX==x &&
				mouseIndexY==y
			)
			{
				float br = SelectedDrag ? 1.0f : 0.5f;
				if(SelectedDrag)
				{
					pointRadX*=1.5f;
					pointRadY*=1.5f;
				}
				float p1X=GetProjMapGridValueX(points,x,y);
				float p1Y=GetProjMapGridValueY(points,x,y);
				if(LGL_GetActiveDisplay()==0)
				{
					GetVisualizer()->GetWindowARCoordsFromProjectorCoords
					(
						p1X,
						p1Y
					);
				}
				LGL_DrawRectToScreen
				(
					p1X-pointRadX,
					p1X+pointRadX,
					p1Y-pointRadY,
					p1Y+pointRadY,
					br,br,br,0.0f
				);
			}
		}
	}
}

int
ProjMapGridObj::
GetProjMapGridIndex
(
	int	x,
	int	y
)
{
	x=LGL_Clamp(0,x,GridW-1);
	y=LGL_Clamp(0,y,GridH-1);
	return(2*(y*GridW+x));
}

float
ProjMapGridObj::
GetProjMapGridValueX
(
	float*	grid,
	int	x,
	int	y
)
{
	if(grid==NULL)
	{
		return(-1.0f);
	}

	return(grid[GetProjMapGridIndex(x,y)+0]);
}

float
ProjMapGridObj::
GetProjMapGridValueY
(
	float*	grid,
	int	x,
	int	y
)
{
	if(grid==NULL)
	{
		return(-1.0f);
	}

	return(grid[GetProjMapGridIndex(x,y)+1]);
}

void
ProjMapGridObj::
SetProjMapGridValueX
(
	float*	grid,
	int	x,
	int	y,
	float	val
)
{
	if(grid==NULL)
	{
		return;
	}

	grid[GetProjMapGridIndex(x,y)+0]=val;
}

void
ProjMapGridObj::
SetProjMapGridValueY
(
	float*	grid,
	int	x,
	int	y,
	float	val
)
{
	if(grid==NULL)
	{
		return;
	}

	grid[GetProjMapGridIndex(x,y)+1]=val;
}

void
ProjMapGridObj::
GetMouseIndexes
(
	float*	points,
	int	display,
	int&	inX,
	int&	inY
)
{
	inX=-1;
	inY=-1;

	if(points==NULL)
	{
		return;
	}

	float mx=LGL_MouseX();
	float my=LGL_MouseY();

	//Transform coords
	GetVisualizer()->GetProjectorARCoordsFromWindowCoords
	(
		mx,
		my
	);

	float closestDistSq=powf(0.05f,2.0f);

	for(int x=0;x<GridW;x++)
	{
		for(int y=0;y<GridH;y++)
		{
			float nowX=GetProjMapGridValueX(points,x,y);
			float nowY=GetProjMapGridValueY(points,x,y);
			float distSq=
				powf(mx-nowX,2.0f)+
				powf(my-nowY,2.0f);
			if(distSq<closestDistSq)
			{
				inX=x;
				inY=y;
				closestDistSq=distSq;
			}
		}
	}
}

