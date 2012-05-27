/*
 *
 * Database.h
 *
 */

#ifndef	_DATABASE_H_
#define	_DATABASE_H_

#include <vector>
#include "LGL.module/LGL.h"

class DatabaseObj;

class DatabaseFilterObj
{

public:

			DatabaseFilterObj();
			~DatabaseFilterObj();

	const char*	GetDir();
	void		SetDir(const char* dir);
	void		SetPattern(const char* pattern);

	void		SetBPMCenter(float bpmCenter);
	void		SetBPMRange(float bpmRange);

//private:

	char*		Dir;
	char*		Pattern;

	float		BPMCenter;
	float		BPMRange;
};

class DatabaseEntryObj
{
public:

		DatabaseEntryObj
		(
			const char*	pathFromMusicRoot,
			float		bpm=-1,
			LGL_FileType	fileType=LGL_FILETYPE_UNDEF
		);
		~DatabaseEntryObj();

	bool	MatchesFilter
		(
			DatabaseFilterObj* filter
		);

//private:

	char*	PathFull;
	char*	PathDir;
	char*	PathShort;
	char*	NameDisplayed;

	float	BPM;
	bool	IsDir;
	bool	Loadable;
	bool	AlreadyPlayed;
};

class DatabaseObj
{

public:

		DatabaseObj();
		~DatabaseObj();
	
	std::vector<DatabaseEntryObj*>
		GetEntryListFromFilter(DatabaseFilterObj* filter);
	
	void	Refresh(const char* subdirPath=NULL, bool drawLoadScreen=false);
	void	Refresh_Internal(const char* subdirPath=NULL, bool recursing=false);

	void	LoadMetadataFile(const char* file);
	void	LoadMetadataDir(const char* dir);

	void	LoadDBFile();
	void	SaveDBFile();

	bool	GetThreadDieHint();
	const char*
		GetThreadRefreshTarget();
	void	ClearThreadRefreshTarget();
	void	SetThreadCompletionPercent(float pct);
	void	GetMostRecentFileScanned(char* dst);
	void	SetMostRecentFileScanned(const char* src);
	
static	void	GenerateiTunesNameDisplayed
		(
			const char*	pathFull,
			char*&		nameDisplayed
		);

private:
	
	void	ClearDatabaseEntryList();

	char	MusicRoot[2048];
	std::vector<DatabaseEntryObj*>
		DatabaseEntryList;

	std::vector<char*>
		MetadataEntryList;
	
	DatabaseEntryObj
		EntryDotDot;

	SDL_Thread*
		Thread;
	bool	ThreadDieHint;
	bool	SkipMetadataHint;
	char	ThreadRefreshTarget[2048];
	float	ThreadCompletionPercent;
	
	int	FilesProcessed;
	int	ExpectedFilesProcessed;
	char	MostRecentFileScanned[2048];
	LGL_Timer
		MostRecentFileScannedTimer;

	LGL_Semaphore
		Semaphore;
};

#endif	//_DATABASE_H_
