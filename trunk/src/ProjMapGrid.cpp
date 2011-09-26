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

#include "Config.h"

#include "Visualizer.h"

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
	SetWidth(GetProjMapGridSideLengthX());
	SetHeight(GetProjMapGridSideLengthY());
	SelectedDstRadius=0.0f;

	sprintf(LoadPath,"%s/.dvj/projMapLoad.txt",LGL_GetHomeDir());
	sprintf(SavePath,"%s/.dvj/projMapSave.txt",LGL_GetHomeDir());
	if(LGL_FileExists(SavePath))
	{
		LGL_FileDirMove(SavePath,LoadPath);
	}
	Identity=true;

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
Load()
{
	if(FILE* fd=fopen(LoadPath,"r"))
	{
		const int bufLen=2048;
		char buf[bufLen];

		fgets(buf,bufLen,fd);
		GridW=atoi(buf);
		fgets(buf,bufLen,fd);
		GridH=atoi(buf);
		for(int a=0;a<GridW*GridH*2;a++)
		{
			fgets(buf,bufLen,fd);
			SrcPoints[a]=atof(buf);
			fgets(buf,bufLen,fd);
			DstPoints[a]=atof(buf);
		}
		fclose(fd);
		Identity=false;
	}
}

void
ProjMapGridObj::
Save()
{
	if(Identity)
	{
		return;
	}

	if(FILE* fd=fopen(SavePath,"w"))
	{
		fprintf(fd,"%i\n",GridW);
		fprintf(fd,"%i\n",GridH);
		for(int a=0;a<GridW*GridH*2;a++)
		{
			fprintf(fd,"%f\n",SrcPoints[a]);
			fprintf(fd,"%f\n",DstPoints[a]);
		}
		fclose(fd);
	}
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

float
getLineSlope
(
	float	p1x,
	float	p1y,
	float	p2x,
	float	p2y
);

float
getLineSlope
(
	float	p1x,
	float	p1y,
	float	p2x,
	float	p2y
)
{
	if(p2x==p1x)
	{
		printf("getLineSlope(): Warning! Slope will be infinite\n");
	}
	return
	(
		(p2y-p1y) /
		(p2x-p1x)
	);
}

float
getLineOffset
(
	float	p1x,
	float	p1y,
	float	p2x,
	float	p2y
);

float
getLineOffset
(
	float	p1x,
	float	p1y,
	float	p2x,
	float	p2y
)
{
	float slope = getLineSlope
	(
		p1x,
		p1y,
		p2x,
		p2y
	);

	float offset = p1y - p1x * slope;
	return(offset);
}

bool
getPointRightOfLine
(
	float	x,
	float	y,
	float	slope,
	float	offset
);

bool
getPointRightOfLine
(
	float	x,
	float	y,
	float	slope,
	float	offset
)
{
	if(slope==0)
	{
		printf("getPointRightOfLine: Warning! Dividing by zero...\n");
	}
	float lineValX = (y - offset) / slope;
	return(x>lineValX);
}

bool
getPointAboveLine
(
	float	x,
	float	y,
	float	slope,
	float	offset
);

bool
getPointAboveLine
(
	float	x,
	float	y,
	float	slope,
	float	offset
)
{
	float lineValY = slope * x + offset;
	return(y>lineValY);
}

bool
getPointRightOfLine
(
	float	x,
	float	y,
	float	l1x,
	float	l1y,
	float	l2x,
	float	l2y
);

bool
getPointRightOfLine
(
	float	x,
	float	y,
	float	l1x,
	float	l1y,
	float	l2x,
	float	l2y
)
{
	if(l1x==l2x)
	{
		return(x>l1x);
	}

	float slope = getLineSlope
	(
		l1x,
		l1y,
		l2x,
		l2y
	);
	float offset = getLineOffset
	(
		l1x,
		l1y,
		l2x,
		l2y
	);

	return
	(
		getPointRightOfLine
		(
			x,
			y,
			slope,
			offset
		)
	);
}

bool
getPointAboveLine
(
	float	x,
	float	y,
	float	l1x,
	float	l1y,
	float	l2x,
	float	l2y
);

bool
getPointAboveLine
(
	float	x,
	float	y,
	float	l1x,
	float	l1y,
	float	l2x,
	float	l2y
)
{
	if(l1x==l2x)
	{
		return(y>l1y);
	}

	float slope = getLineSlope
	(
		l1x,
		l1y,
		l2x,
		l2y
	);
	float offset = getLineOffset
	(
		l1x,
		l1y,
		l2x,
		l2y
	);

	return
	(
		getPointAboveLine
		(
			x,
			y,
			slope,
			offset
		)
	);
}

bool
getPointInsideQuad
(
	float	x,
	float	y,
	float	lbx,
	float	lby,
	float	rbx,
	float	rby,
	float	ltx,
	float	lty,
	float	rtx,
	float	rty
);

bool
getPointInsideQuad
(
	float	x,
	float	y,
	float	lbx,
	float	lby,
	float	rbx,
	float	rby,
	float	ltx,
	float	lty,
	float	rtx,
	float	rty
)
{
	//Point is left of left?
	if
	(
		getPointRightOfLine
		(
			x,y,
			lbx,
			lby,
			ltx,
			lty
		)==false
	)
	{
		return(false);
	}
	
	//Point is right of right?
	if
	(
		getPointRightOfLine
		(
			x,y,
			rbx,
			rby,
			rtx,
			rty
		)
	)
	{
		return(false);
	}

	//Point is below bottom?
	if
	(
		getPointAboveLine
		(
			x,y,
			lbx,
			lby,
			rbx,
			rby
		)==false
	)
	{
		return(false);
	}
	
	//Point is above top?
	if
	(
		getPointAboveLine
		(
			x,y,
			ltx,
			lty,
			rtx,
			rty
		)
	)
	{
		return(false);
	}

	return(true);
}

void
ProjMapGridObj::
InsertPoint
(
	float	x,
	float	y
)
{
	//Find index at which to add the point
	int indexX=-1;
	int indexY=-1;

	//See which quad the point is inside.
	for(int a=0;a<GridW-1;a++)
	{
		for(int b=0;b<GridH-1;b++)
		{
			float lbx = GetProjMapGridValueX(SrcPoints,a+0,b+0);
			float lby = GetProjMapGridValueY(SrcPoints,a+0,b+0);

			float rbx = GetProjMapGridValueX(SrcPoints,a+1,b+0);
			float rby = GetProjMapGridValueY(SrcPoints,a+1,b+0);

			float ltx = GetProjMapGridValueX(SrcPoints,a+0,b+1);
			float lty = GetProjMapGridValueY(SrcPoints,a+0,b+1);

			float rtx = GetProjMapGridValueX(SrcPoints,a+1,b+1);
			float rty = GetProjMapGridValueY(SrcPoints,a+1,b+1);

			if
			(
				getPointInsideQuad
				(
					x,
					y,
					lbx,
					lby,
					rbx,
					rby,
					ltx,
					lty,
					rtx,
					rty
				)
			)
			{
				indexX=a+1;
				indexY=b+1;
				break;
			}
		}
	}

	//Fallback method
	if
	(
		indexX==-1 ||
		indexY==-1
	)
	{
		float closestDistSq=9999.0f;
		for(int a=0;a<GridW;a++)
		{
			for(int b=0;b<GridH;b++)
			{
				float nowX = GetProjMapGridValueX(SrcPoints,a,b);
				float nowY = GetProjMapGridValueY(SrcPoints,a,b);
				if
				(
					x<nowX &&
					y<nowY
				)
				{
					float distSq = powf(x-nowX,2) + powf(y-nowY,2);
					if(distSq<closestDistSq)
					{
						indexX=a;
						indexY=b;
						closestDistSq=distSq;
					}
				}
			}
		}
	}

	//Failsafe
	if(indexX==-1) indexX=0;
	if(indexY==-1) indexY=0;

	//Yoink SrcPoints and DstPoints

	float* oldSrcPoints=SrcPoints;
	float* oldDstPoints=DstPoints;
	SrcPoints=NULL;
	DstPoints=NULL;

	//Call SetWidth() and SetHeight()
	
	int gridWOld=GridW;
	int gridHOld=GridH;
	int gridWNew=GridW+1;
	int gridHNew=GridH+1;

	SetWidth(gridWNew);
	SetHeight(gridHNew);



	//Fill in new SrcPoints and DstPoints (1st pass: Copy over prior grid)

	for(int a=0;a<GridW;a++)
	{
		for(int b=0;b<GridH;b++)
		{
			if
			(
				a==indexX &&
				b==indexY
			)
			{
				//The new point we're trying to add
				//Do this on 2nd pass
			}
			else if(a==indexX)
			{
				//A new point in the same row as the point we're adding
				//Do this on 2nd pass
			}
			else if(b==indexY)
			{
				//A new point in the same column as the point we're adding
				//Do this on 2nd pass
			}
			else
			{
				int aSrc=a;
				if(aSrc>=indexX)
				{
					aSrc--;
					if(aSrc<0) aSrc=0;
				}
				int bSrc=b;
				if(bSrc>=indexY)
				{
					bSrc--;
					if(bSrc<0) bSrc=0;
				}
				GridW=gridWOld;
				GridH=gridHOld;
				float xSrcNew = 
					GetProjMapGridValueX
					(
						oldSrcPoints,
						aSrc,
						bSrc
					);
				float ySrcNew =
					GetProjMapGridValueY
					(
						oldSrcPoints,
						aSrc,
						bSrc
					);
/*
				float xDstNew = 
					GetProjMapGridValueX
					(
						oldDstPoints,
						aSrc,
						bSrc
					);
				float yDstNew =
					GetProjMapGridValueY
					(
						oldDstPoints,
						aSrc,
						bSrc
					);
*/

				GridW=gridWNew;
				GridH=gridHNew;
				SetProjMapGridValueX
				(
					SrcPoints,
					a,
					b,
					xSrcNew
				);
				SetProjMapGridValueY
				(
					SrcPoints,
					a,
					b,
					ySrcNew
				);

				/*
				SetProjMapGridValueX
				(
					DstPoints,
					a,
					b,
					xDstNew
				);
				SetProjMapGridValueY
				(
					DstPoints,
					a,
					b,
					yDstNew
				);
				*/
			}
		}
	}
	
	//Fill in new SrcPoints and DstPoints (2nd pass)

	for(int a=0;a<GridW;a++)
	{
		for(int b=0;b<GridH;b++)
		{
			if
			(
				a==indexX &&
				b==indexY
			)
			{
				//The new point we're trying to add
				float xSrcNew=x;
				float ySrcNew=y;
				SetProjMapGridValueX
				(
					SrcPoints,
					a,
					b,
					xSrcNew
				);
				SetProjMapGridValueY
				(
					SrcPoints,
					a,
					b,
					ySrcNew
				);
				/*
				SetProjMapGridValueX
				(
					DstPoints,
					a,
					b,
					x
				);
				SetProjMapGridValueY
				(
					DstPoints,
					a,
					b,
					y
				);
				*/
			}
			else if(a==indexX)
			{
				//A new point in the same column as the point we're adding
				float xSrcNew = 0.5f *
				(
					GetProjMapGridValueX
					(
						SrcPoints,
						((a-1) >= 0) ? (a-1) : (a+1),
						b
					)+
					GetProjMapGridValueX
					(
						SrcPoints,
						((a+1) < GridW) ? (a+1) : (a-1),
						b
					)
				);
				float ySrcNew = 0.5f *
				(
					GetProjMapGridValueY
					(
						SrcPoints,
						((a-1) >= 0) ? (a-1) : (a+1),
						b
					)+
					GetProjMapGridValueY
					(
						SrcPoints,
						((a+1) < GridW) ? (a+1) : (a-1),
						b
					)
				);
				SetProjMapGridValueX
				(
					SrcPoints,
					a,
					b,
					xSrcNew
				);
				SetProjMapGridValueY
				(
					SrcPoints,
					a,
					b,
					ySrcNew
				);

				//TODO: Dst
			}
			else if(b==indexY)
			{
				//A new point in the same row as the point we're adding
				float xSrcNew = 0.5f *
				(
					GetProjMapGridValueX
					(
						SrcPoints,
						a,
						((b-1) >= 0) ? (b-1) : (b+1)
					)+
					GetProjMapGridValueX
					(
						SrcPoints,
						a,
						((b+1) < GridH) ? (b+1) : (b-1)
					)
				);
				float ySrcNew = 0.5f *
				(
					GetProjMapGridValueY
					(
						SrcPoints,
						a,
						((b-1) >= 0) ? (b-1) : (b+1)
					)+
					GetProjMapGridValueY
					(
						SrcPoints,
						a,
						((b+1) < GridH) ? (b+1) : (b-1)
					)
				);
				SetProjMapGridValueX
				(
					SrcPoints,
					a,
					b,
					xSrcNew
				);
				SetProjMapGridValueY
				(
					SrcPoints,
					a,
					b,
					ySrcNew
				);

				//TODO: DstPoints
			}
			else
			{
				//A point from the prior grid
				//We did this in the 1st pass
			}
		}
	}

	//Cleanup yoinked arrays

	delete oldSrcPoints;
	delete oldDstPoints;
}

void
ProjMapGridObj::
NextFrame()
{
return;
	if(LGL_KeyDown(PROJ_MAP_KEY)==false)
	{
		if(LGL_KeyRelease(PROJ_MAP_KEY))
		{
			Save();
		}
		SelectedIndexX=-1;
		SelectedIndexY=-1;
		SelectedDrag=false;
		return;
	}

	if(LGL_KeyStroke(LGL_KEY_C))
	{
		Load();
	}

	if(LGL_KeyStroke(LGL_KEY_X))
	{
		InsertPoint(0.5f,0.5f);
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
		if
		(
			LGL_KeyDown(LGL_KEY_LSHIFT) ||
			SelectedIndexX!=selectedIndexXOld ||
			SelectedIndexY!=selectedIndexYOld
		)
		{
			SelectedDstRadius=0.1f;
		}
		else
		{
			SelectedDstRadius = LGL_Max
			(
				0.005f,
				SelectedDstRadius - LGL_SecondsSinceLastFrame()*0.35f
			);
		}

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

/*
	for(int a=0;a<GridW;a++)
	{
		for(int b=0;b<GridH;b++)
		{
			SetProjMapGridValueX
			(
				DstPoints,
				a,
				b,
				GetProjMapGridValueX
				(
					SrcPoints,
					a,
					b
				)
			);
			SetProjMapGridValueY
			(
				DstPoints,
				a,
				b,
				GetProjMapGridValueY
				(
					SrcPoints,
					a,
					b
				)
			);
		}
	}
*/
}

void
ProjMapGridObj::
DrawSrcGrid()
{
return;
	if(GetProjMapSimple())
	{
		float l=0.0f;
		float r=1.0f;
		float b=0.0f;
		float t=1.0f;
		GetVisualizer()->GetWindowARCoordsFromProjectorCoords
		(
			l,
			b
		);
		GetVisualizer()->GetWindowARCoordsFromProjectorCoords
		(
			r,
			t
		);

		LGL_DrawRectToScreen
		(
			l,
			r,
			b,
			t,
			0.0f,
			0.0f,
			0.0f,
			1.0f
		);
	}

	DrawGrid(SrcPoints);
}

void
ProjMapGridObj::
DrawDstGrid()
{
return;
	if(GetProjMapSimple()==false)
	{
		DrawGrid(DstPoints);
	}
	else
	{
		DrawGrid(SrcPoints);
	}
}

/*
void
ProjMapGridObj::
DrawMappedImage
(
	LGL_Image*	image,
	float		left,
	float		right,
	float		bottom,
	float		top
)
{
	for(int i=0;i<GridW-1;i++)
	{
		for(int j=0;j<GridH-1;j++)
		{
			float xSrc[4];
			float ySrc[4];
			//LB
			xSrc[0]=GetProjMapGridValueX
			(
				SrcPoints,
				i,
				j
			);
			ySrc[0]=GetProjMapGridValueY
			(
				SrcPoints,
				i,
				j
			);
			//RB
			xSrc[1]=GetProjMapGridValueX
			(
				SrcPoints,
				i+1,
				j
			);
			ySrc[1]=GetProjMapGridValueY
			(
				SrcPoints,
				i+1,
				j
			);
			//RT
			xSrc[2]=GetProjMapGridValueX
			(
				SrcPoints,
				i+1,
				j+1
			);
			ySrc[2]=GetProjMapGridValueY
			(
				SrcPoints,
				i+1,
				j+1
			);
			//LT
			xSrc[3]=GetProjMapGridValueX
			(
				SrcPoints,
				i,
				j+1
			);
			ySrc[3]=GetProjMapGridValueY
			(
				SrcPoints,
				i,
				j+1
			);

			float xDst[4];
			float yDst[4];
			//LB
			xDst[0]=GetProjMapGridValueX
			(
				DstPoints,
				i,
				j
			);
			yDst[0]=GetProjMapGridValueY
			(
				DstPoints,
				i,
				j
			);
			//RB
			xDst[1]=GetProjMapGridValueX
			(
				DstPoints,
				i+1,
				j
			);
			yDst[1]=GetProjMapGridValueY
			(
				DstPoints,
				i+1,
				j
			);
			//RT
			xDst[2]=GetProjMapGridValueX
			(
				DstPoints,
				i+1,
				j+1
			);
			yDst[2]=GetProjMapGridValueY
			(
				DstPoints,
				i+1,
				j+1
			);
			//LT
			xDst[3]=GetProjMapGridValueX
			(
				DstPoints,
				i,
				j+1
			);
			yDst[3]=GetProjMapGridValueY
			(
				DstPoints,
				i,
				j+1
			);

			for(int q=0;q<4;q++)
			{
				xDst[q]=myL+myW*xDst[q];
				yDst[q]=myB+myH*yDst[q];
			}

			mmi->DrawToScreen
			(
				xDst,
				yDst,
				xSrc,
				ySrc,
				1.0f,
				1.0f,
				1.0f,
				1.0f,
				1.0f	//brightnessScalar
			);
		}
	}
}
*/

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

	const float pointRad=(LGL_GetActiveDisplay()==0) ? 0.005f : SelectedDstRadius;
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
				float brR = (SelectedDrag || LGL_GetActiveDisplay()!=0) ? LGL_RandFloat(0.5f,1.0f) : 0.5f;
				float brG = (SelectedDrag || LGL_GetActiveDisplay()!=0) ? LGL_RandFloat(0.5f,1.0f) : 0.5f;
				float brB = (SelectedDrag || LGL_GetActiveDisplay()!=0) ? LGL_RandFloat(0.5f,1.0f) : 0.5f;
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

				const float rectBorder=0.002f;

				LGL_DrawRectToScreen
				(
					p1X-pointRadX-rectBorder,
					p1X+pointRadX+rectBorder,
					p1Y-pointRadY-rectBorder*2,
					p1Y+pointRadY+rectBorder*2,
					0.0f,0.0f,0.0f,1.0f
				);

				LGL_DrawRectToScreen
				(
					p1X-pointRadX,
					p1X+pointRadX,
					p1Y-pointRadY,
					p1Y+pointRadY,
					brR,brG,brB,0.0f
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
	Identity=false;
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
	Identity=false;
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

