/*
 *
 * FileInterface.h
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

#ifndef	DEFINE_FILE_INTERFACE
#define	DEFINE_FILE_INTERFACE

#include <stdarg.h>
#include <stdio.h>
#include <vector>

class FileInterfaceObj
{

public:

					FileInterfaceObj();
					~FileInterfaceObj();

	void				Clear();
	void				AddArgument(const char* arg, ...);

	bool				ReadLine(FILE* file);
	void				ReadLine(const char* line, ...);

	void				WriteLine(FILE* file);

const	char*				GetLine() const;
	void				PrintLine(FILE* file) const;

	void				RewindLine(FILE* file);

	unsigned int			Size() const;
	char*				operator[](const unsigned int& index) const;

private:

	void				BuildLineFromArgv();

	std::vector<char*>		Argv;
	char*				Line;
	long				RewindPoint;

};

#endif	//DEFINE_FILE_INTERFACE

