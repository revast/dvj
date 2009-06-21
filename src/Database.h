/*
 *
 * Database.h
 *
 */

#ifndef	_DATABASE_H_
#define	_DATABASE_H_

class DatabaseObj;

class DatabaseFilterObj
{

public:

		DatabaseFilterObj();
		~DatabaseFilterObj();

	void	SetDir(const char* dir);
	void	SetPattern(const char* pattern);

	void	SetBPMCenter(float bpmCenter);
	void	SetBPMRange(float bpmRange);

//private:

	char*	Dir;
	char*	Pattern;

	float	BPMCenter;
	float	BPMRange;
};

class DatabaseEntryObj
{
public:

		DatabaseEntryObj
		(
			const char*	pathFromMusicRoot,
			float		bpm=-1
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

	float	BPM;
	bool	IsDir;
};

class DatabaseObj
{

public:

	DatabaseObj (const char* musicRoot);
	~DatabaseObj();
	
	std::vector<DatabaseEntryObj*> GetEntryListFromFilter(DatabaseFilterObj* filter);
	
	void	Refresh(const char* subdirPath="data/music");

	void	LoadMetadataFile(const char* file);
	void	LoadMetadataDir(const char* dir);

	void	LoadDBFile();
	void	SaveDBFile();

private:

	char	MusicRoot[2048];
	std::vector<DatabaseEntryObj*>
		DatabaseEntryList;
	
	DatabaseEntryObj
		EntryDotDot;
};

#endif	//_DATABASE_H_
