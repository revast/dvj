/*
 *
 * Database.cpp
 *
 */

#include "LGL.h"
#include "FileInterface.h"
#include "Database.h"

//DatabaseFilterObj

DatabaseFilterObj::
DatabaseFilterObj()
{
	Dir=NULL;
	Pattern=NULL;

	BPMCenter=-1;
	BPMRange=10;
}

DatabaseFilterObj::
~DatabaseFilterObj()
{
	if(Dir!=NULL)
	{
		delete Dir;
		Dir=NULL;
	}

	if(Pattern!=NULL)
	{
		delete Pattern;
		Pattern=NULL;
	}
}

void
DatabaseFilterObj::
SetDir
(
	const char*	dir
)
{
	if(Dir)
	{
		delete Dir;
		Dir=NULL;
	}
	if
	(
		dir &&
		dir[0]!='\0'
	)
	{
		Dir=new char[strlen(dir)+1];
		strcpy(Dir,dir);
	}
}

void
DatabaseFilterObj::
SetPattern
(
	const char*	pattern
)
{
	if(Pattern)
	{
		delete Pattern;
		Pattern=NULL;
	}
	if
	(
		pattern &&
		pattern[0]!='\0'
	)
	{
		Pattern=new char[strlen(pattern)+1];
		strcpy(Pattern,pattern);
	}
}

void
DatabaseFilterObj::
SetBPMCenter
(
	float	bpmCenter
)
{
	BPMCenter=bpmCenter;
}

void
DatabaseFilterObj::
SetBPMRange
(
	float	bpmRange
)
{
	BPMRange=bpmRange;
}



//DatabaseEntryObj

DatabaseEntryObj::
DatabaseEntryObj
(
	const char*	pathFromMusicRoot,
	float		bpm
)
{
	PathFull = new char[strlen(pathFromMusicRoot)+1];
	strcpy(PathFull,pathFromMusicRoot);

	char* pathShort = PathFull;
	while(char* nextSlash=strstr(pathShort,"/"))
	{
		pathShort=&(nextSlash[1]);
	}
	PathShort=new char[strlen(pathShort)+1];
	strcpy(PathShort,pathShort);

	char tmp=pathShort[0];
	pathShort[0]='\0';
	PathDir = new char[strlen(PathFull)+1];
	strcpy(PathDir,PathFull);
	PathDir[strlen(PathDir)-1]='\0';
	pathShort[0]=tmp;

	BPM=bpm;
	IsDir=LGL_DirectoryExists(pathFromMusicRoot);
}

DatabaseEntryObj::
~DatabaseEntryObj()
{
	delete PathFull;
	PathFull=NULL;

	delete PathDir;
	PathDir=NULL;

	delete PathShort;
	PathShort=NULL;
}

bool
DatabaseEntryObj::
MatchesFilter
(
	DatabaseFilterObj*	filter
)
{
	//BPM
	if(filter->BPMCenter>=0)
	{
		if(BPM<=0)
		{
			return(false);
		}

		float top = filter->BPMCenter + filter->BPMRange+0.5f;
		float bottom = filter->BPMCenter - filter->BPMRange-0.5f;
		if
		(
			filter->BPMCenter==0 ||
			(
				0.5f*BPM>=bottom &&
				0.5f*BPM<=top
			) ||
			(
				1.0f*BPM>=bottom &&
				1.0f*BPM<=top
			) ||
			(
				2.0f*BPM>=bottom &&
				2.0f*BPM<=top
			)
		)
		{
			//We're within range!
		}
		else
		{
			return(false);
		}
	}

	//Dir
	if(filter->Dir)
	{
		unsigned int filterLen = strlen(filter->Dir);
		unsigned int entryLen = strlen(PathDir);
		if(filterLen!=entryLen)
		{
			return(false);
		}
		for(unsigned int a=0;a<entryLen;a++)
		{
			if(tolower(PathDir[a])!=tolower(filter->Dir[a]))
			{
				return(false);
			}
		}
	}

	//Pattern
	if(filter->Pattern)
	{
		const char* filterText = filter->Pattern;
		char tempText[2048];
		LGL_Assert(strlen(filterText) < 2047);
		strcpy(tempText,filterText);
		std::vector<char*> filterWordList;
		char* ptr=tempText;
		for(;;)
		{
			if(ptr[0]=='\0')
			{
				break;
			}
			else if(ptr[0]==' ')
			{
				ptr=&(ptr[1]);
			}
			else
			{
				char* nextSpace=strchr(ptr,' ');
				bool lastWord=(nextSpace==NULL);

				if(lastWord==false ) nextSpace[0]='\0';
				char* neo=new char[strlen(ptr)+1];
				strcpy(neo,ptr);
				filterWordList.push_back(neo);
				if(lastWord==false) nextSpace[0]=' ';
				ptr=&(nextSpace[1]);

				if(lastWord)
				{
					break;
				}
			}
		}

		bool match=true;
		unsigned int wordCount=filterWordList.size();
		for(unsigned int b=0;b<wordCount;b++)
		{
			if(strcasestr(PathShort,filterWordList[b])==NULL)
			{
				match=false;
				break;
			}
		}

		for(unsigned int a=0;a<filterWordList.size();a++)
		{
			delete filterWordList[a];
		}
		filterWordList.clear();

		return(match);
	}

	return(true);
}



//DatabaseObj

DatabaseObj::
DatabaseObj
(
	const char*	musicRoot
) :
	EntryDotDot("..")
{
	assert(musicRoot);

	strcpy(MusicRoot,musicRoot);

	Refresh();
}

DatabaseObj::
~DatabaseObj()
{
	//
}

std::vector<DatabaseEntryObj*>
DatabaseObj::
GetEntryListFromFilter
(
	DatabaseFilterObj*	filter
)
{
	std::vector<DatabaseEntryObj*> list;

	if
	(
		filter->Pattern==NULL &&
		strcmp(filter->Dir,MusicRoot)!=0
	)
	{
		list.push_back(&EntryDotDot);
	}

	//First dirs
	for(unsigned int a=0;a<DatabaseEntryList.size();a++)
	{
		if
		(
			DatabaseEntryList[a]->IsDir &&
			DatabaseEntryList[a]->MatchesFilter(filter)
		)
		{
			list.push_back(DatabaseEntryList[a]);
		}
	}

	//Then files
	for(unsigned int a=0;a<DatabaseEntryList.size();a++)
	{
		if
		(
			DatabaseEntryList[a]->IsDir==false &&
			DatabaseEntryList[a]->MatchesFilter(filter)
		)
		{
			list.push_back(DatabaseEntryList[a]);
		}
	}

	return(list);
}

void
DatabaseObj::
Refresh
(
	const char*	subdirPath
)
{
	LGL_DirTree dirTree(subdirPath);
	dirTree.WaitOnWorkerThread();

	//Add files
	for(unsigned int a=0;a<dirTree.GetFileCount();a++)
	{
		char path[2048];
		sprintf(path,"%s/%s",subdirPath,dirTree.GetFileName(a));

		float bpm=0;
		char pathMeta[2048];
		sprintf(pathMeta,"data/metadata/%s.musefuse-metadata.txt",dirTree.GetFileName(a));
		if(FILE* fd=fopen(pathMeta,"r"))
		{
			FileInterfaceObj fi;
			float bpmStart=-1;
			float bpmEnd=-1;
			for(;;)
			{
				fi.ReadLine(fd);
				if(feof(fd))
				{
					break;
				}
				if(fi.Size()==0)
				{
					continue;
				}
				if
				(
					strcasecmp(fi[0],"HomePoints")==0 ||
					strcasecmp(fi[0],"SavePoints")==0
				)
				{
					if(fi.Size()!=19)
					{
						printf("DatbaseObj::LoadMetaData('%s'): Warning!\n",path);
						printf("\tSavePoints has strange fi.size() of '%i' (Expecting 11)\n",fi.Size());
					}
					for(unsigned int a=0;a<fi.Size()-1 && a<2;a++)
					{
						if(a==0)
						{
							bpmStart=atof(fi[a+1]);
						}
						else if(a==1)
						{
							bpmEnd=atof(fi[a+1]);
						}
					}
				}
			}

			if(bpmStart!=-1 && bpmEnd!=-1)
			{
				int bpmMin=100;
				float p0=bpmStart;
				float p1=bpmEnd;
				float dp=p1-p0;
				int measuresGuess=1;
				float bpmGuess;
				if(dp!=0)
				{
					for(int a=0;a<10;a++)
					{
						bpmGuess=(4*measuresGuess)/(dp/60.0f);
						if(bpmGuess>=bpmMin)
						{
							bpm=bpmGuess;
							break;
						}
						measuresGuess*=2;
					}
				}
			}

			fclose(fd);
			fd=NULL;
		}
		DatabaseEntryObj* ent = new DatabaseEntryObj(path,bpm);
		DatabaseEntryList.push_back(ent);
	}

	//Recurse on subdirs
	for(unsigned int a=0;a<dirTree.GetDirCount();a++)
	{
		if(dirTree.GetDirName(a)[0]!='.')
		{
			char path[2048];
			sprintf(path,"%s/%s",subdirPath,dirTree.GetDirName(a));
			DatabaseEntryObj* ent = new DatabaseEntryObj(path);
			DatabaseEntryList.push_back(ent);
			Refresh(path);
		}
	}
}

