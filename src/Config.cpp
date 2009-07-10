/*
 *
 * Config.cpp
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

#include "Config.h"

void
CreateDefaultDVJRC
(
	const char*	path
)
{
	if(FILE* fd=fopen(path,"w"))
	{
		//TODO
		fprintf(fd,"#dvjrc\n");
		fclose(fd);
	}
}

void
LoadDVJRC()
{
	//Initialize .dvj subtree if necessary
	char dotDvj[2048];
	sprintf(dotDvj,"%s/.dvj/",LGL_GetHomeDir());
	if(LGL_DirectoryExists(dotDvj)==false)
	{
		LGL_DirectoryCreate(dotDvj);
	}
	LGL_DirectoryCreate(dotDvj);

	char dvjrc[2048];
	sprintf(dvjrc,"%s/dvjrc",dotDvj);
	if(LGL_FileExists(dvjrc)==false)
	{
		CreateDefaultDVJRC(dvjrc);
	}

	char dvjCache[2048];
	sprintf(dvjCache,"%s/cache",dotDvj);
	if(LGL_DirectoryExists(dvjCache)==false)
	{
		LGL_DirectoryCreate(dvjCache);
	}

	char dvjCacheFileLength[2048];
	sprintf(dvjCacheFileLength,"%s/fileLength",dvjCache);
	if(LGL_DirectoryExists(dvjCacheFileLength)==false)
	{
		LGL_DirectoryCreate(dvjCacheFileLength);
	}

	char dvjCacheMetadata[2048];
	sprintf(dvjCacheMetadata,"%s/metadata",dvjCache);
	if(LGL_DirectoryExists(dvjCacheMetadata)==false)
	{
		LGL_DirectoryCreate(dvjCacheMetadata);
	}

	char dvjCacheWaveArrayData[2048];
	sprintf(dvjCacheWaveArrayData,"%s/waveArrayData",dvjCache);
	if(LGL_DirectoryExists(dvjCacheWaveArrayData)==false)
	{
		LGL_DirectoryCreate(dvjCacheWaveArrayData);
	}
}

void
GetColorCool
(
	float&	r,
	float&	g,
	float&	b
)
{
	r=0.0f;
	g=0.0f;
	b=0.5f;
}

void
GetColorWarm
(
	float&	r,
	float&	g,
	float&	b
)
{
	r=0.4f;
	g=0.2f;
	b=1.0f;
}


