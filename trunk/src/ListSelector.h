/*
 *
 * ListSelector.h
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

#ifndef	_DVJ_LISTSELECTOR_H_
#define	_DVJ_LISTSELECTOR_H_

#include "LGL.module/LGL.h"

class dvjListSelectorCell
{

public:

			dvjListSelectorCell();
			~dvjListSelectorCell();
	
	void		SetString(const char* str);
	const char*	GetString();

private:

	char*		String;

public:

	float		StringColorR;
	float		StringColorG;
	float		StringColorB;

	void*		UserData;
};

class dvjListSelector
{

public:

			dvjListSelector
			(
				int	colCount=1
			);
			~dvjListSelector();

	void		SetWindowScope
			(
				float	left,
				float	right,
				float	bottom,
				float	top
			);
	float		GetWindowScopeLeft();
	float		GetWindowScopeRight();
	float		GetWindowScopeBottom();
	float		GetWindowScopeTop();
	void		SetColLeftEdge
			(
				int	col,
				float	leftEdge
			);
	void		SetCellColRowString
			(
				int		col,
				int		row,
				const char*	str,
				...
			);
	void		SetCellColRowStringRGB
			(
				int		col,
				int		row,
				float		r,
				float		g,
				float		b
			);
	dvjListSelectorCell*
			GetCellColRow
			(
				int		col,
				int		row
			);
	int		GetHighlightedRow();
	void		SetHighlightedRow(int row);
	void		CenterHighlightedRow();
	void		Clear();
	void		SetBehindImage
			(
				LGL_Image*	img
			);
	void		SetBadRowFlash();

	void		ScrollHighlightedRow(float rows);
	/*
	void		HighlightNearest
			(
				const char*	search,
				int		col
			);
	*/

	int		GetVisibleRowIndexTop();
	int		GetVisibleRowIndexBottom();

	void		NextFrame();
	void		Draw();

private:

	int		GetColCount();
	int		GetRowCount();
	void		AddRow();
	int		GetIndexFromColRow
			(
				int	col,
				int	row
			);
	float		GetCellHeight();
	float		GetFontHeight();
	float		GetColBezel();
	float		GetColLeftEdge
			(
				int	col
			);
	float		GetColRightEdge
			(
				int	col
			);
	float		GetRowsDisplayed();
	bool		GetScrollingBeyondTop();
	bool		GetScrollingBeyondBottom();

	float		Left;
	float		Right;
	float		Bottom;
	float		Top;

	int		ColCount;	//Shall always be >= 1

	std::vector<dvjListSelectorCell*>
			Cells;
	std::vector<float>
			ColLeftEdge;

	float		CellsScrolled;
	float		ScrollMomentum;
	float		HighlightedRow;

	LGL_Image*	BehindImage;
	float		BadRowFlash;
};

#endif	//_DVJ_LISTSELECTOR_H_

