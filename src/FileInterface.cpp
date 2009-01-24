/*
 *
 * FileInterface.cpp
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

#include "FileInterface.h"

FileInterfaceObj::
FileInterfaceObj()
{
	Line=NULL;
	RewindPoint=0;
}

FileInterfaceObj::
~FileInterfaceObj()
{
	Clear();
}

void
FileInterfaceObj::
Clear()
{
	//Delete every string in Argv
	for(unsigned int a=0;a<Argv.size();a++)
	{
		delete Argv[a];
	}
	Argv.clear();

	//Delete and NULLify Line, if necessary
	if(Line!=NULL)
	{
		delete Line;
		Line=NULL;
	}
}

void
FileInterfaceObj::
AddArgument
(
	const
	char*	arg,
	...
)
{
	assert(arg!=NULL);

	//Process the formatted part of the string
	char str[1024];
	va_list args;
	va_start(args,arg);
	vsprintf(str,arg,args);
	va_end(args);

	char* neo=new char[strlen(str)+1];
	strcpy(neo,str);

	Argv.push_back(neo);

	BuildLineFromArgv();
}

bool
FileInterfaceObj::
ReadLine
(
	FILE*		file
)
{
	assert(file!=NULL);

	RewindPoint=ftell(file);

	//Grab a line from a file.
	char buffer[1024];
	fgets(buffer,1024,file);

	//If we've read past the EOF, return false.
	if
	(
		strlen(buffer)==0 &&
		feof(file)!=0
	)
	{
		Clear();
		return(false);
	}

	//No need to get rid of the last '\n', since the next fn() will take care of that.
	ReadLine(buffer);

	return(true);
}

void
FileInterfaceObj::
ReadLine
(
	const
	char*		line,
			...
)
{
	assert(line!=NULL);

	Clear();

	//If our input is zero length, we're done.
	if(strlen(line)==0)
	{
		return;
	}

	//Process the formatted part of the string
	char str[1024];
	va_list args;
	va_start(args,line);
	vsprintf(str,line,args);
	va_end(args);

	//The first '\n' we find becomes our new '\0'.
	char* newline=strstr(str,"\n");
	if(newline!=NULL)
	{
		newline[0]='\0';
	}

	//Copy the full line into Line.
	Line=new char[strlen(str)+1];
	strcpy(Line,str);

	//Place each argument seperated by a '|' into Argv
	char* srcBegin=Line;
	for(;;)
	{
		char temp[1024];
		strcpy(temp,srcBegin);
		char* tempEnd=strstr(temp,"|");

		if(tempEnd)
		{
			tempEnd[0]='\0';
		}

		char* pushMe=new char[strlen(temp)+1];
		strcpy(pushMe,temp);

		Argv.push_back(pushMe);

		if(tempEnd==NULL) break;
		else srcBegin=&(strstr(srcBegin,"|")[1]);
	}
}

void
FileInterfaceObj::
WriteLine
(
	FILE*		file
)
{
	fprintf(file,"%s\n",Line);
}

const
char*
FileInterfaceObj::
GetLine() const
{
	return(Line);
}

void
FileInterfaceObj::
PrintLine
(
	FILE*		file
)	const
{
	printf("%s\n",Line);
	for(unsigned int a=0;a<Argv.size();a++)
	{
		printf("\t%s\n",Argv[a]);
	}
}

void
FileInterfaceObj::
RewindLine
(
	FILE*		file
)
{
	fseek(file,RewindPoint,SEEK_SET);
}

unsigned int
FileInterfaceObj::
Size()	const
{
	return(Argv.size());
}

char*
FileInterfaceObj::
operator[]
(
	const
	unsigned int&	index
)	const
{
	assert(index<Argv.size());
	return(Argv[index]);
}

void
FileInterfaceObj::
BuildLineFromArgv()
{
	if(Line!=NULL)
	{
		delete Line;
		Line=NULL;
	}

	char temp[1024];
	temp[0]='\0';
	char* place=temp;
	unsigned int num=0;

	for(unsigned int a=0;a<Argv.size();a++)
	{
		num+=strlen(Argv[a]);
		assert(num<1024);

		if(a!=0)
		{
			place[0]='|';
			num++;
			assert(num<1024);
			place=&(place[1]);
		}
		strcpy(place,Argv[a]);
		place=&(place[strlen(place)]);
	}

	Line=new char[strlen(temp)+1];
	strcpy(Line,temp);
}

