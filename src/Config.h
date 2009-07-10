/*
 *
 * Config.h
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

#ifndef	_DVJ_CONFIG_H_
#define	_DVJ_CONFIG_H_

void
CreateDefaultDVJRC
(
	const char*	path
);

void
LoadDVJRC();

void
GetColorCool
(
	float&	r,
	float&	g,
	float&	b
);

void
GetColorWarm
(
	float&	r,
	float&	g,
	float&	b
);

#endif	//_DVJ_CONFIG_H_

