/*
 *
 * TestSoundLoadAllFiles.cpp
 *
 */

#include "TestSoundLoadAllFiles.h"


TestSoundLoadAllFiles::
TestSoundLoadAllFiles()
{
	assert(DirTree==NULL);
	DirTree=new LGL_DirTree("data/music");
	strcpy(Name,"SoundLoad | Directory");
	CurrentTest.SetLoop(false);
}

TestSoundLoadAllFiles::
~TestSoundLoadAllFiles()
{
	assert(DirTree!=NULL);
	delete DirTree;
	DirTree=NULL;
}

void
TestSoundLoadAllFiles::
Activate()
{
	Test::Activate();
	DirTreeIndexNow=0;
}

void
TestSoundLoadAllFiles::
Update()
{
	Test::Update();

	if(Active)
	{
		if(CurrentTest.IsActive()==false)
		{
			char path[1024];
			sprintf(path,"%s/%s",DirTree->GetPath(),DirTree->GetFileName(DirTreeIndexNow));
printf("Loading '%s'...\n",DirTree->GetFileName(DirTreeIndexNow));
			CurrentTest.SetPath(path);
			CurrentTest.Activate();
		}
		if(CurrentTest.GetPercentComplete()==1.0f)
		{
			CurrentTest.Deactivate();
			DirTreeIndexNow++;
			PercentComplete=DirTreeIndexNow/(float)DirTree->GetFileCount();
		}
		CurrentTest.Update();
	}
}

void
TestSoundLoadAllFiles::
Deactivate()
{
	Test::Deactivate();
}



