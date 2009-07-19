/*
 *
 * diskWriter
 *
 * Copyright Chris Nelson, 2003
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "LGL.module/LGL.h"

int main(int argc, char** argv)
{
	if(argc<2)
	{
		printf("Usage: diskWriter path/to/savefile --lame\n");
		printf("NOTE! There is no reason diskWriter should be executed by the user.\n");
		printf("NOTE! Only dvj should invoke diskWriter.\n");
		exit(0);
	}

	bool recordToLame=false;
	bool gzip=false;
	int bufferSize=1024*16;//1024;
	int lameFreq=44100;
	for(int a=0;a<argc;a++)
	{
		if(strcasecmp(argv[a],"--lame")==0)
		{
			recordToLame=true;
		}
		if(strcasecmp(argv[a],"--freq")==0)
		{
			if(a+1<argc)
			{
				lameFreq=atoi(argv[a+1]);
				a++;
			}
			else
			{
				printf("Warning! --freq requires an argument (--freq 48000)!\n");
			}
		}
		if(strcasecmp(argv[a],"--gzip")==0)
		{
			gzip=true;
		}
	}

	char* buffer=new char[bufferSize];

	FILE* fd;
	if(recordToLame)
	{
		const char* lamePath;
		if(LGL_FileExists("lame"))
		{
			lamePath = "./lame";
		}
		else
		{
			lamePath = "lame";
		}
		char cmd[1024];
		sprintf(cmd,"%s -r %s -h -b 320 - \"%s\"",lamePath,lameFreq==48000?"-s 48":"-s 44.1",argv[1]);
//printf("Lame command line:\n");
//printf("\t'%s'\n",cmd);
		fd=popen(cmd,"w");
	}
	else if(gzip)
	{
		char cmd[1024];
		sprintf
		(
			cmd,"nice -n 10 gzip -c - > \"%s.gz\"",argv[1]
		);
		fd=popen(cmd,"w");
	}
	else
	{
		fd=fopen64(argv[1],"w");
	}
	if(fd==NULL)
	{
		exit(-1);
	}

	for(;;)
	{
		fread(buffer,1,bufferSize,stdin);
		if(feof(stdin))
		{
			fflush(NULL);
			fclose(fd);
			chmod
			(
				argv[1],
				S_IRUSR |	//o+r
				S_IWUSR |	//o+w
				S_IRGRP |	//g+r
				S_IWGRP |	//g+w
				S_IROTH |	//o+r
				S_IWOTH		//o+w
			);
			exit(0);
		}
		fwrite(buffer,1,bufferSize,fd);
		fflush(NULL);
	}
}

