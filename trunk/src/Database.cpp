/*
 *
 * Database.cpp
 *
 */

#include "Database.h"

#include "Common.h"
#include "Config.h"
#include "FileInterface.h"

#include "LGL.module/LGL.h"

const char* iTunesMusicStr = "iTunes";

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

const char*
DatabaseFilterObj::
GetDir()
{
	return(Dir ? Dir : "");
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
		const char* dir = GetMusicRootPath();
		Dir=new char[strlen(dir)+1];
		strcpy(Dir,dir);
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

const char*
DatabaseFilterObj::
GetPattern()
{
	return(Pattern ? Pattern : "");
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

float
DatabaseFilterObj::
GetBPMCenter()
{
	return(BPMCenter);
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

float
DatabaseFilterObj::
GetBPMRange()
{
	return(BPMRange);
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

void
DatabaseFilterObj::
Assign
(
	DatabaseFilterObj&	dst
)
{
	dst.SetDir(GetDir());
	dst.SetPattern(GetPattern());
	dst.SetBPMCenter(GetBPMCenter());
	dst.SetBPMRange(GetBPMRange());
}



//DatabaseEntryObj

DatabaseEntryObj::
DatabaseEntryObj
(
	const char*	pathFromMusicRoot,
	float		bpm,
	LGL_FileType	type
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

	NameDisplayed=NULL;
	if(strstr(PathFull,iTunesMusicStr))
	{
		DatabaseObj::GenerateiTunesNameDisplayed(PathFull,NameDisplayed);
	}

	if(NameDisplayed==NULL)
	{
		NameDisplayed = new char[strlen(PathShort)+1];
		strcpy(NameDisplayed,PathShort);
	}

	BPM=bpm;
	if(type!=LGL_FILETYPE_UNDEF)
	{
		IsDir=type==LGL_FILETYPE_DIR;
		Loadable=IsDir || type==LGL_FILETYPE_FILE;
		if(type==LGL_FILETYPE_SYMLINK)
		{
			Loadable=IsDir || LGL_FileExists(pathFromMusicRoot);
		}
	}
	else
	{
		IsDir=LGL_DirectoryExists(pathFromMusicRoot);
		Loadable=IsDir || LGL_FileExists(pathFromMusicRoot);
	}
	AlreadyPlayed=false;
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

	delete NameDisplayed;
	NameDisplayed=NULL;
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
		unsigned int filterLen = (unsigned int)strlen(filter->Dir);
		unsigned int entryLen = (unsigned int)strlen(PathDir);
		if(GetOldFileStructure() || IsDir)
		{
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
		else
		{
			if(filterLen>entryLen)
			{
				return(false);
			}
			for(unsigned int a=0;a<filterLen;a++)
			{
				if(tolower(PathDir[a])!=tolower(filter->Dir[a]))
				{
					return(false);
				}
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
		unsigned int wordCount=(unsigned int)filterWordList.size();
		for(unsigned int b=0;b<wordCount;b++)
		{
			/*
			if(strcasestr(PathShort,filterWordList[b])==NULL)
			{
				match=false;
				break;
			}
			*/
			if(strcasestr(NameDisplayed,filterWordList[b])==NULL)
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

int
DatabaseRefreshThread
(
	void* ptr
);

int
DatabaseRefreshThread
(
	void* ptr
)
{
	DatabaseObj* db = (DatabaseObj*)ptr;

	char refreshTarget[2048];
	for(;;)
	{
		if(db->GetThreadRefreshTarget()[0]!='\0')
		{
			strcpy(refreshTarget,db->GetThreadRefreshTarget());
			db->ClearThreadRefreshTarget();
			db->Refresh_Internal(refreshTarget);
		}
		else
		{
			db->SetThreadCompletionPercent(1.0f);
			break;
		}
	}

	return(0);
}

DatabaseObj::
DatabaseObj() :
	EntryDotDot(".."),
	Semaphore("Database Semaphore")
{
	strcpy(MusicRoot,GetMusicRootPath());

	Thread=NULL;
	ThreadDieHint=false;
	SkipMetadataHint=false;
	ThreadRefreshTarget[0]='\0';
	ThreadCompletionPercent=0.0f;

	Refresh(NULL,true);
}

DatabaseObj::
~DatabaseObj()
{
	ThreadDieHint=true;
	SkipMetadataHint=true;
	LGL_ThreadWait(Thread);
	Thread=NULL;

	ClearDatabaseEntryList();
}

bool databaseEntrySortPredicate(const DatabaseEntryObj* d1, const DatabaseEntryObj* d2)
{
	return
	(
		d1 &&
		d2 &&
		d1->NameDisplayed &&
		d2->NameDisplayed &&
		(
			(
				d1->IsDir &&
				d2->IsDir==false
			) ||
			strcasecmp(d1->NameDisplayed, d2->NameDisplayed) < 0
		)
	);
}

std::vector<DatabaseEntryObj*>
DatabaseObj::
GetEntryListFromFilter
(
	DatabaseFilterObj*	filter,
	bool*			abortSignal
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
		if(abortSignal && (*abortSignal))
		{
			std::vector<DatabaseEntryObj*> empty;
			return(empty);
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
		if(abortSignal && (*abortSignal))
		{
			std::vector<DatabaseEntryObj*> empty;
			return(empty);
		}
	}

	return(list);
}

void
DatabaseObj::
Refresh
(
	const char*	subdirPath,
	bool		drawLoadScreen
)
{
	//Update MetadataEntryList
	{
		for(unsigned int a=0;a<MetadataEntryList.size();a++)
		{
			delete MetadataEntryList[a];
		}
		MetadataEntryList.clear();
		
		char pathMeta[2048];
		sprintf(pathMeta,"%s/.dvj/metadata",LGL_GetHomeDir());
		LGL_DirTree metadataDirtree;
		metadataDirtree.SetPath(pathMeta);
		metadataDirtree.WaitOnWorkerThread();
		for(unsigned int a=0;a<metadataDirtree.GetFileCount();a++)
		{
			char* newb = new char[strlen(metadataDirtree.GetFileName(a))+1];
			strcpy(newb,metadataDirtree.GetFileName(a));
			MetadataEntryList.push_back(newb);
		}
	}
	if(subdirPath==NULL)
	{
		subdirPath=MusicRoot;
	}

	strcpy(ThreadRefreshTarget,subdirPath);

	if(Thread==NULL)
	{
		FilesProcessed=0;
		ExpectedFilesProcessed=-1;
		MostRecentFileScanned[0]='\0';

		ThreadCompletionPercent=0.0f;
		ThreadDieHint=false;
		SkipMetadataHint=false;
		Thread=LGL_ThreadCreate(DatabaseRefreshThread,this);
		char mostRecentFileScanned[2048];
		mostRecentFileScanned[0]='\0';
		char count[2048];
		count[0]='\0';

		for(;;)
		{
			float bright = LGL_Clamp
			(
				0.0f,
				2.0f*(1.0f-MostRecentFileScannedTimer.SecondsSinceLastReset()),
				1.0f
			);
			if(FilesProcessed>0 || ThreadCompletionPercent==1.0f)
			{
				if(drawLoadScreen)
				{
					GetMostRecentFileScanned(mostRecentFileScanned);
					if(ExpectedFilesProcessed==-1)
					{
						//sprintf(count,"%i",FilesProcessed);
						DrawLoadScreen
						(
							-1.0f,
							NULL,//count,
							"Scanning library",
							NULL,//mostRecentFileScanned,
							bright
						);
					}
					else
					{
						DrawLoadScreen
						(
							ThreadCompletionPercent,
							NULL,//count,
							"Scanning library",
							NULL,//mostRecentFileScanned,
							bright
						);
					}
				}
			}

			if(drawLoadScreen)
			{
				if(LGL_KeyStroke(LGL_KEY_ESCAPE))
				{
					bool die=GetEscDuringScanExits();
					if(die)
					{
						ThreadDieHint=true;
					}
					SkipMetadataHint=true;
					if(die)
					{
						LGL_ThreadWait(Thread);
						Thread=NULL;
						exit(0);
					}
				}
			}

			if
			(
				ThreadCompletionPercent==1.0f &&
				bright==0.0f
			)
			{
				LGL_ThreadWait(Thread);
				Thread=NULL;
				break;
			}
		}
	}
}

void
DatabaseObj::
Refresh_Internal
(
	const char*	subdirPath,
	bool		recursing
)
{
	if(ThreadDieHint)
	{
		return;
	}

	LGL_DirTree dirTree(subdirPath);
	dirTree.WaitOnWorkerThread();

	char dbCachePath[2048];
	sprintf(dbCachePath,"%s/.dvj/cache/databaseCache.txt",LGL_GetHomeDir());

	unsigned int volumesCount=0;

	if(recursing==false)
	{
		ThreadCompletionPercent=0.0f;
		FilesProcessed=0;
		ExpectedFilesProcessed=-1;
		ClearDatabaseEntryList();
	
		char dbCachePrevPath[2048];
		dbCachePrevPath[0]='\0';
		int dbCachePrevFileCount=0;
		unsigned int dbCachePrevVolumeCount=0;

		LGL_DirTree volumes("/Volumes");
		volumes.WaitOnWorkerThread();
		volumesCount=volumes.GetDirCount();

		if(FILE* fd=fopen(dbCachePath,"r"))
		{
			FileInterfaceObj fi;
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
				
				if(strcasecmp(fi[0],"Path")==0)
				{
					strcpy(dbCachePrevPath,fi[1]);
				}
				else if(strcasecmp(fi[0],"FileCount")==0)
				{
					dbCachePrevFileCount=atoi(fi[1]);
				}
				else if(strcasecmp(fi[0],"VolumeCount")==0)
				{
					dbCachePrevVolumeCount=atoi(fi[1]);
				}
			}

			fclose(fd);

			if
			(
				strcasecmp(dbCachePrevPath,subdirPath)==0 &&
				dbCachePrevFileCount!=0 &&
				volumesCount==dbCachePrevVolumeCount
			)
			{
				ExpectedFilesProcessed=dbCachePrevFileCount;
			}
		}
	}

	//Add files
	for(unsigned int a=0;a<dirTree.GetFileCount();a++)
	{
		if(ThreadDieHint)
		{
			return;
		}

		char path[2048];
		sprintf(path,"%s/%s",subdirPath,dirTree.GetFileName(a));

		if(a!=0)
		{
			SetMostRecentFileScanned(dirTree.GetFileName(a));
		}

		float bpm=0;
		const char* dirTreeGetFileName = dirTree.GetFileName(a);
		char* nameDisplayediTunes=NULL;
		if(strstr(path,iTunesMusicStr))
		{
			GenerateiTunesNameDisplayed(path,nameDisplayediTunes);
			dirTreeGetFileName = nameDisplayediTunes;
		}

		//Look for new metadata
		{
			char pathMeta[2048];
			char pathMetaShort[2048];
			sprintf(pathMetaShort,"%s.savepoints.txt",dirTreeGetFileName);
			sprintf(pathMeta,"%s/.dvj/metadata/%s",LGL_GetHomeDir(),pathMetaShort);

			bool metaExists=false;
			if(SkipMetadataHint==false)
			{
				//TODO: Make this more efficient
				for(unsigned int m=0;m<MetadataEntryList.size();m++)
				{
					if(strcmp(pathMetaShort,MetadataEntryList[m])==0)
					{
						metaExists=true;
						break;
					}
				}
			}

			if(metaExists)//LGL_FileExists(pathMeta))
			{
				if(FILE* fd=fopen(pathMeta,"r"))
				{
					FileInterfaceObj fi;
					for(;;)
					{
						fi.ReadLine(fd);
						if(feof(fd))
						{
							break;
						}
						if(fi.Size()!=3)
						{
							continue;
						}

						if(strcasecmp(fi[0],"Savepoint")==0)
						{
							float tmpBPM = atof(fi[2]);
							if(tmpBPM>0)
							{
								bpm=tmpBPM;
								break;
							}
						}
					}

					fclose(fd);
					fd=NULL;
				}
			}
		}

		//Look for old metadata
		if(bpm==0)
		{
			char pathMeta[2048];
			char pathMetaShort[2048];
			sprintf(pathMetaShort,"%s.dvj-metadata.txt",dirTreeGetFileName);
			sprintf(pathMeta,"%s/.dvj/metadata/%s",LGL_GetHomeDir(),pathMetaShort);

			bool metaExists=false;
			if(SkipMetadataHint==false)
			{
				//TODO: Make this more efficient
				for(unsigned int m=0;m<MetadataEntryList.size();m++)
				{
					if(strcmp(pathMetaShort,MetadataEntryList[m])==0)
					{
						metaExists=true;
						break;
					}
				}
			}

			if(metaExists)//LGL_FileExists(pathMeta))
			{
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
			}
		}

		DatabaseEntryObj* ent = new DatabaseEntryObj(path,bpm,LGL_FILETYPE_FILE);
		DatabaseEntryList.push_back(ent);

		FilesProcessed++;
		if(ExpectedFilesProcessed!=-1)
		{
			ThreadCompletionPercent=FilesProcessed/(float)ExpectedFilesProcessed;
			if(ThreadCompletionPercent >= 1.0f)
			{
				ThreadCompletionPercent=0.9999f;
			}
		}
		else
		{
			ThreadCompletionPercent=-1.0f;
		}

		if(nameDisplayediTunes)
		{
			delete nameDisplayediTunes;
			nameDisplayediTunes=NULL;
		}
	}

	//Recurse on subdirs
	for(unsigned int a=0;a<dirTree.GetDirCount();a++)
	{
		if
		(
			dirTree.GetDirName(a)[0]!='.' &&
			strcmp(dirTree.GetDirName(a),GetDvjCacheDirName())!=0
		)
		{
			char path[2048];
			sprintf(path,"%s/%s",subdirPath,dirTree.GetDirName(a));
			DatabaseEntryObj* ent = new DatabaseEntryObj(path,-1,LGL_FILETYPE_DIR);
			DatabaseEntryList.push_back(ent);
			Refresh_Internal(path,true);
		}
	}

	if(recursing==false)
	{
		if(FILE* fd = fopen(dbCachePath,"w"))
		{
			fprintf(fd,"Path|%s\n",subdirPath);
			fprintf(fd,"FileCount|%i\n",FilesProcessed);
			fprintf(fd,"VolumeCount|%i\n",volumesCount);
			fclose(fd);
		}

		FilesProcessed=0;

		if(DatabaseEntryList.size()>0)
		{
			std::sort
			(
				DatabaseEntryList.begin(),
				DatabaseEntryList.begin() + DatabaseEntryList.size()-1,
				databaseEntrySortPredicate
			);
		}
	}
}

bool
DatabaseObj::
GetThreadDieHint()
{
	return(ThreadDieHint);
}

const char*
DatabaseObj::
GetThreadRefreshTarget()
{
	return(ThreadRefreshTarget);
}

void
DatabaseObj::
ClearThreadRefreshTarget()
{
	ThreadRefreshTarget[0]='\0';
}

void
DatabaseObj::
SetThreadCompletionPercent(float pct)
{
	ThreadCompletionPercent=pct;
}

void
DatabaseObj::
GetMostRecentFileScanned
(
	char*	dst
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,Semaphore);
	strcpy(dst,MostRecentFileScanned);
}

void
DatabaseObj::
SetMostRecentFileScanned
(
	const char*	src
)
{
	if
	(
		MostRecentFileScannedTimer.SecondsSinceLastReset()>=1.0f ||
		MostRecentFileScanned[0]=='\0'
	)
	{
		LGL_ScopeLock lock(__FILE__,__LINE__,Semaphore);
		strcpy(MostRecentFileScanned,src?src:"");
		MostRecentFileScannedTimer.Reset();
	}
}

void
DatabaseObj::
ClearDatabaseEntryList()
{
	for(unsigned int a=0;a<DatabaseEntryList.size();a++)
	{
		delete DatabaseEntryList[a];
		DatabaseEntryList[a]=NULL;
	}

	DatabaseEntryList.clear();
}

void
DatabaseObj::
GenerateiTunesNameDisplayed
(
	const char* pathFull,
	char*&	nameDisplayed
)
{
	const char* pathShort = pathFull;
	while(const char* slash = strchr(pathShort,'/'))
	{
		if(slash[1]!='\0')
		{
			pathShort=&(slash[1]);
		}
		else
		{
			break;
		}
	}

	if(strstr(pathFull,iTunesMusicStr)==NULL)
	{
		nameDisplayed = new char[strlen(pathShort)+1];
		strcpy(nameDisplayed,pathShort);
		return;
	}

	char* tmpAlpha;
	char* tmpOmega;
	char* artist=NULL;
	char* album=NULL;
	char* track=NULL;
	char* name=NULL;
	if((tmpAlpha = strstr(pathFull,iTunesMusicStr)))
	{
		//Found "iTunes"
		tmpAlpha = strstr(tmpAlpha,"/");
		if(tmpAlpha)
		{
			tmpAlpha = &(tmpAlpha[1]);
			if((tmpOmega = strstr(tmpAlpha,"/")))
			{
				//Extract artist
				tmpOmega[0]='\0';
				artist = new char[strlen(tmpAlpha)+1];
				strcpy(artist,tmpAlpha);
				tmpOmega[0]='/';
				tmpAlpha=&(tmpOmega[1]);

				if(tmpAlpha[0])
				{
					//Extract album
					if((tmpOmega = strstr(tmpAlpha,"/")))
					{
						tmpOmega[0]='\0';
						album = new char[strlen(tmpAlpha)+1];
						strcpy(album,tmpAlpha);
						tmpOmega[0]='/';
						tmpAlpha=&(tmpOmega[1]);

						if(tmpAlpha[0])
						{
							//Extract track
							if
							(
								tmpAlpha[0] >= '0' && tmpAlpha[0]<='9' &&
								tmpAlpha[1] >= '0' && tmpAlpha[1]<='9' &&
								tmpAlpha[2] == ' '
							)
							{
								//"01 name"
								track = new char[3];
								track[0]=tmpAlpha[0];
								track[1]=tmpAlpha[1];
								track[2]='\0';
								tmpAlpha=&(tmpAlpha[3]);
							}
							else if
							(
								tmpAlpha[0] >= '0' && tmpAlpha[0]<='9' &&
								tmpAlpha[1] == '-' &&
								tmpAlpha[2] >= '0' && tmpAlpha[0]<='9' &&
								tmpAlpha[3] >= '0' && tmpAlpha[1]<='9' &&
								tmpAlpha[4] == ' '
							)
							{
								//"3-01 name"
								track = new char[4];
								track[0]=tmpAlpha[0];
								track[1]=tmpAlpha[2];
								track[2]=tmpAlpha[3];
								track[3]='\0';
								tmpAlpha = &(tmpAlpha[5]);
							}
							else
							{
								//track shall remain NULL
							}

							//Extract name
							name = new char[strlen(tmpAlpha)+1];
							strcpy(name,tmpAlpha);
						}
					}
				}
			}
		}
	}

	if
	(
		artist &&
		album &&
		name
	)
	{
		char tmpLong[2048];
		if(track)
		{
			snprintf
			(
				tmpLong,
				sizeof(tmpLong)-1,
				"%s - %s (%s) - %s",
				artist,
				album,
				track,
				name
			);
		}
		else
		{
			snprintf
			(
				tmpLong,
				sizeof(tmpLong)-1,
				"%s - %s - %s",
				artist,
				album,
				name
			);
		}

		nameDisplayed = new char[strlen(tmpLong)+1];
		strcpy(nameDisplayed,tmpLong);
	}

	if(artist)
	{
		delete artist;
		artist=NULL;
	}
	if(album)
	{
		delete album;
		album=NULL;
	}
	if(track)
	{
		delete track;
		track=NULL;
	}
	if(name)
	{
		delete name;
		name=NULL;
	}
}



