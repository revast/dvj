/*
 *
 * ListSelector.cpp
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

#include "ListSelector.h"

#include "LGL.module/LGL.h"

#include "Config.h"



//dvjListSelectorCell

dvjListSelectorCell::
dvjListSelectorCell()
{
	String=NULL;
	StringColorR=1.0f;
	StringColorG=1.0f;
	StringColorB=1.0f;
}

dvjListSelectorCell::
~dvjListSelectorCell()
{
	delete String;
	String=NULL;
}

void
dvjListSelectorCell::
SetString
(
	const char*	str
)
{
	delete String;
	String=NULL;

	if(str)
	{
		String = new char[strlen(str)+1];
		strcpy(String,str);
	}
}

const char*
dvjListSelectorCell::
GetString()
{
	return(String);
}



//dvjListSelector

dvjListSelector::
dvjListSelector
(
	int	colCount
)
{
	Left=0.0f;
	Right=1.0f;
	Bottom=0.0f;
	Top=1.0f;

	ColCount=LGL_Max(1,colCount);
	for(int a=0;a<ColCount;a++)
	{
		ColLeftEdge.push_back(a/(float)ColCount);
	}

	CellsScrolled=0.0f;
	ScrollMomentum=0.0f;
	HighlightedRow=0;
	BehindImage=NULL;
	BadRowFlash=0.0f;
}

dvjListSelector::
~dvjListSelector()
{
	Clear();

	BehindImage=NULL;	//We don't own BehindImage, so don't delete it.
}

void
dvjListSelector::
SetWindowScope
(
	float	left,
	float	right,
	float	bottom,
	float	top
)
{
	if(left>right)
	{
		left=right;
	}
	if(right<left)
	{
		right=left;
	}
	if(bottom>top)
	{
		bottom=top;
	}
	if(top<bottom)
	{
		top=bottom;
	}

	Left=left;
	Right=right;
	Bottom=bottom;
	Top=top;
}

float
dvjListSelector::
GetWindowScopeLeft()
{
	return(Left);
}

float
dvjListSelector::
GetWindowScopeRight()
{
	return(Right);
}

float
dvjListSelector::
GetWindowScopeBottom()
{
	return(Bottom);
}

float
dvjListSelector::
GetWindowScopeTop()
{
	return(Top);
}

void
dvjListSelector::
SetColLeftEdge
(
	int	col,
	float	leftEdge
)
{
	col=LGL_Clamp(0,col,GetColCount());
	if(col>=GetColCount())
	{
		return;
	}

	ColLeftEdge[col]=leftEdge;
}

void
dvjListSelector::
SetCellColRowString
(
	int		col,
	int		row,
	const char*	str,
	...
)
{
	while(GetRowCount()<=row)
	{
		AddRow();
	}

	char* strFinal=NULL;
	if(str)
	{
		char tmpstr[1024*8];
		va_list args;
		va_start(args,str);
		vsprintf(tmpstr,str,args);
		va_end(args);

		strFinal = new char[strlen(tmpstr)+1];
		strcpy(strFinal,tmpstr);
	}

	int index = GetIndexFromColRow(col,row);

	Cells[index]->SetString(strFinal);

	delete strFinal;
}

void
dvjListSelector::
SetCellColRowStringRGB
(
	int		col,
	int		row,
	float		r,
	float		g,
	float		b
)
{
	int index = GetIndexFromColRow(col,row);

	Cells[index]->StringColorR=r;
	Cells[index]->StringColorG=g;
	Cells[index]->StringColorB=b;
}

dvjListSelectorCell*
dvjListSelector::
GetCellColRow
(
	int	col,
	int	row
)
{
	if
	(
		GetColCount()<=col ||
		GetRowCount()<=row
	)
	{
		return(NULL);
	}

	int index = GetIndexFromColRow(col,row);

	return(Cells[index]);
}

int
dvjListSelector::
GetHighlightedRow()
{
	return((int)HighlightedRow);
}

void
dvjListSelector::
SetHighlightedRow
(
	int	row
)
{
	HighlightedRow = LGL_Clamp(0,row,LGL_Max(0,GetRowCount()-1));
}

void
dvjListSelector::
CenterHighlightedRow()
{
	CellsScrolled = LGL_Max(0,HighlightedRow - floorf(0.5f*GetRowsDisplayed()));
	ScrollMomentum=0.0f;
}

void
dvjListSelector::
Clear()
{
	for(int a=0;a<Cells.size();a++)
	{
		if(Cells[a])
		{
			delete Cells[a];
			Cells[a]=NULL;
		}
	}
	Cells.clear();
}

void
dvjListSelector::
SetBehindImage
(
	LGL_Image*	img
)
{
	BehindImage=img;
}

void
dvjListSelector::
SetBadRowFlash()
{
	BadRowFlash=1.0f;
}

void
dvjListSelector::
ScrollHighlightedRow
(
	float	rows
)
{
	HighlightedRow = LGL_Clamp(0,HighlightedRow+rows,LGL_Max(0,GetRowCount()-1));
}

/*
void
dvjListSelector::
HighlightNearest
(
	const char*	search,
	int		col
)
{
	printf("highlightNearest alpha!\n");
	col = LGL_Clamp(0,col,GetColCount());

	if(search==NULL)
	{
		HighlightedRow=0;
		return;
	}

	for(unsigned int row=0;row<GetRowCount();row++)
	{
		int index=GetIndexFromColRow
		(
			col,
			row
		);
		const char* test=Cells[index]->GetString();
		printf("Searching '%s' => '%s'\n",search,test);
		if
		(
			test &&
			strcasecmp(search,test) <= 0
		)
		{
			printf("bingo!\n");
			HighlightedRow=row;
			break;
		}
	}
}
*/

int
dvjListSelector::
GetVisibleRowIndexTop()
{
	const int begin = LGL_Max(0,floorf(CellsScrolled));
	return(begin);
}

int
dvjListSelector::
GetVisibleRowIndexBottom()
{
	const int begin = LGL_Max(0,floorf(CellsScrolled));
	const int end = LGL_Min(GetRowCount()-1,begin + ceilf(GetRowsDisplayed()));
	return(end);
}

void
dvjListSelector::
NextFrame()
{
	const float highlightedRowPrev = HighlightedRow;

	if
	(
		LGL_MultiTouchFingerCount()<2 &&
		LGL_MouseMotion() &&
		LGL_MouseX()>=Left &&
		LGL_MouseX()<=Right &&
		LGL_MouseY()>=Bottom &&
		LGL_MouseY()<=Top
	)
	{
		float offset = GetCellHeight()*0.25f;
		HighlightedRow = CellsScrolled +
			(
				GetRowsDisplayed()*
				((Top-(LGL_MouseY()+offset))/(Top-Bottom))
			);
	}

	if
	(
		LGL_MultiTouchFingerCount()==2 &&
		LGL_MouseX()>=Left &&
		LGL_MouseX()<=Right &&
		LGL_MouseY()>=Bottom &&
		LGL_MouseY()<=Top
	)
	{
		float factor=40.0f;
		if
		(
			GetScrollingBeyondTop() ||
			GetScrollingBeyondBottom()
		)
		{
			factor=10.0f;
		}
		ScrollMomentum=LGL_MultiTouchDY()*factor;
	}
	else
	{
		float momentumDampener=1.5f;
		const float momentumDampenerRubberband=10.0f;

		const float minScroll=0.5f;
		const float rubberbandFactor=15.0f;

		if(GetScrollingBeyondTop())
		{
			momentumDampener=momentumDampenerRubberband;

			const float delta=LGL_Min(CellsScrolled,-1.0f*minScroll)*-1.0f*rubberbandFactor*LGL_SecondsSinceLastFrame();
			CellsScrolled+=delta;

			if(CellsScrolled>0.0f)
			{
				CellsScrolled=0.0f;
			}
		}
		else if(GetScrollingBeyondBottom())
		{
			momentumDampener=momentumDampenerRubberband;

			float furthestRowAtTopEdge = GetRowCount()-GetRowsDisplayed();

			const float delta=LGL_Max(CellsScrolled-furthestRowAtTopEdge,1.0f*minScroll)*1.0f*rubberbandFactor*LGL_SecondsSinceLastFrame();
			CellsScrolled-=delta;

			if(CellsScrolled<0.0f)
			{
				CellsScrolled=0.0f;
			}
		}

		if(ScrollMomentum>0.0f)
		{
			ScrollMomentum=LGL_Max(0.0f,ScrollMomentum-momentumDampener*LGL_SecondsSinceLastFrame());
		}
		else if(ScrollMomentum<0.0f)
		{
			ScrollMomentum=LGL_Min(0.0f,ScrollMomentum+momentumDampener*LGL_SecondsSinceLastFrame());
		}
	}

	const float percentPermittedOffscreen=0.75f;
	float highlightedRowMin = floorf(CellsScrolled+percentPermittedOffscreen)+1;
	float highlightedRowMax = floorf(CellsScrolled-percentPermittedOffscreen+GetRowsDisplayed());

	{
		const float nudgeMagnitude = 0.2f;
		if(ScrollMomentum <= 0.0f)
		{
			if
			(
				CellsScrolled>0.0f &&
				HighlightedRow<highlightedRowMin
			)
			{
				ScrollMomentum=-fabsf(HighlightedRow-highlightedRowMin)*nudgeMagnitude;
			}
		}
		if(ScrollMomentum >= 0.0f)
		{
			if
			(
				CellsScrolled<GetRowCount()-1 &&
				HighlightedRow>highlightedRowMax
			)
			{
				ScrollMomentum=fabsf(HighlightedRow-highlightedRowMax)*nudgeMagnitude;
			}
		}
	}

	CellsScrolled+=ScrollMomentum;

	if
	(
		HighlightedRow < highlightedRowMin &&
		ScrollMomentum > 0.0f
	)
	{
		HighlightedRow = highlightedRowMin;
	}
	if
	(
		HighlightedRow > highlightedRowMax &&
		ScrollMomentum < 0.0f
	)
	{
		HighlightedRow = highlightedRowMax;
	}

	if(highlightedRowPrev != HighlightedRow)
	{
		BadRowFlash=0.0f;
	}
	else
	{
		BadRowFlash=LGL_Max(0.0f,BadRowFlash-2.0f*LGL_SecondsSinceLastFrame());
	}
}

void
dvjListSelector::
Draw()
{
	float width = Right-Left;

	for
	(
		int row=(int)floor(CellsScrolled);
		true;
		row++
	)
	{
		float cellTop=Top-(row-CellsScrolled)*GetCellHeight();
		float cellBottom=cellTop-GetCellHeight();

		if(row<0)
		{
			if
			(
				row==-1 &&
				BehindImage
			)
			{
				LGL_ClipRectEnable
				(
					Left,
					Right,
					cellBottom,
					Top
				);
				BehindImage->DrawToScreen
				(
					Left,
					Right,
					Bottom,
					Top,
					0.0f,
					0.25f,
					0.25f,
					0.25f,
					0.25f
				);
				LGL_ClipRectDisable();
			}
			continue;
		}
		if(row>=GetRowCount())
		{
			break;
		}

		if(cellTop<Bottom)
		{
			break;
		}

		//Draw Background

		float bgR=0.0f;
		float bgG=0.0f;
		float bgB=0.0f;
		if(row%2==0)
		{
			GetColorCool
			(
				bgR,
				bgG,
				bgB
			);
			bgR*=0.1f;
			bgG*=0.1f;
			bgB*=0.1f;
		}

		if(row==(int)HighlightedRow)
		{
			GetColorCool
			(
				bgR,
				bgG,
				bgB
			);
			bgR*=0.5f;
			bgG*=0.5f;
			bgB*=0.5f;

			if(BadRowFlash>0.0f)
			{
				bgR=
					(1.0f-BadRowFlash) * bgR +
					(0.0f+BadRowFlash) * 1.0f;
				bgG=
					(1.0f-BadRowFlash) * bgG +
					(0.0f+BadRowFlash) * 0.0f;
				bgB=
					(1.0f-BadRowFlash) * bgB +
					(0.0f+BadRowFlash) * 0.0f;
			}
		}

		LGL_ClipRectEnable(Left,Right,Bottom,Top);
		LGL_DrawRectToScreen
		(
			Left,Right,
			cellBottom,cellTop,
			bgR,bgG,bgB,0.0f
		);
		LGL_ClipRectDisable();
	
		for(int col=0;col<GetColCount();col++)
		{
			if(dvjListSelectorCell* cell = GetCellColRow(col,row))
			{
				if(const char* str = cell->GetString())
				{
					float strLeft = Left+width*(GetColLeftEdge(col) + GetColBezel());
					float strRightBezel = Left+width*(GetColLeftEdge(col+1) - GetColBezel());
					float strRight = Left+width*(GetColLeftEdge(col+1));

					float fontHeight=GetFontHeight();
					float fontWidth=LGL_GetFont().GetWidthString(fontHeight,str);
					float fontWidthMax=strRightBezel-strLeft;
					fontHeight=LGL_Min(fontHeight,fontHeight*fontWidthMax/fontWidth);

					float strY = cellBottom + GetCellHeight()*0.5f - GetFontHeight()*0.5f;
					LGL_ClipRectEnable(strLeft,strRight,LGL_Max(cellBottom,Bottom),LGL_Min(cellTop,Top));
					LGL_GetFont().DrawString
					(
						strLeft,
						strY,
						fontHeight,
						cell->StringColorR,
						cell->StringColorG,
						cell->StringColorB,
						1.0f,
						false,
						0.75f,
						str
					);
					LGL_ClipRectDisable();
				}
			}
		}
	}
}

int
dvjListSelector::
GetColCount()
{
	return(ColCount);
}

int
dvjListSelector::
GetRowCount()
{
	return(Cells.size()/ColCount);
}

void
dvjListSelector::
AddRow()
{
	for(int a=0;a<GetColCount();a++)
	{
		Cells.push_back(new dvjListSelectorCell);
	}
}

int
dvjListSelector::
GetIndexFromColRow
(
	int	col,
	int	row
)
{
	//Sanity check, possibly enlarging Cells to be big enough for requested index.
	{
		if(col>=GetColCount())
		{
			printf
			(
				"dvjListSelector::GetIndexFromColRow(): Warning! col >= GetColCount() (%i >= %i)\n",
				col,
				GetColCount()
			);
			col=LGL_Max(0,GetColCount()-1);
		}
		if(row>=GetRowCount())
		{
			/*
			printf
			(
				"dvjListSelector::GetIndexFromColRow(): Warning! row >= GetRowCount() (%i >= %i)\n",
				row,
				GetRowCount()
			);
			*/
			row=LGL_Max(0,GetRowCount()-1);
			while(GetRowCount()<row)
			{
				AddRow();
			}
		}
	}

	return(row*ColCount+col);
}

float
dvjListSelector::
GetCellHeight()
{
	return(GetFontHeight()*2.0f);
}

float
dvjListSelector::
GetFontHeight()
{
	return(1.5f*LGL_GetFont().GetHeightPixels()/(float)LGL_WindowResolutionY());
	//return(LGL_GetFont().GetHeightPixels()/(float)LGL_WindowResolutionY());
}

float
dvjListSelector::
GetColBezel()
{
	return(0.015f);
}

float
dvjListSelector::
GetColLeftEdge
(
	int	col
)
{
	col=LGL_Clamp(0,col,GetColCount());
	if(col>=GetColCount())
	{
		return(1.0f);
	}
	return(ColLeftEdge[col]);
}

float
dvjListSelector::
GetColRightEdge
(
	int	col
)
{
	return(GetColLeftEdge(col+1));
}

float
dvjListSelector::
GetRowsDisplayed()
{
	return((Top-Bottom)/GetCellHeight());
}

bool
dvjListSelector::
GetScrollingBeyondTop()
{
	return(CellsScrolled<0.0f);
}

bool
dvjListSelector::
GetScrollingBeyondBottom()
{
	bool ret=
	(
		CellsScrolled > 0 &&
		GetRowCount()-CellsScrolled < GetRowsDisplayed()
	);
	return(ret);
}

