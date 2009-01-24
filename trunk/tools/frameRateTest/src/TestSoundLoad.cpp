/*
 *
 * TestSoundLoad.cpp
 *
 */

#include "TestSoundLoad.h"

#define BUFFER_LENGTH (1024*1024*100)

TestSoundLoad::
TestSoundLoad()
{
	Sound=NULL;
	SetLoadFull();
	SetLoop();
	Buffer = new char[BUFFER_LENGTH];
	strcpy(Path,"data/testSoundLoad.ogg");
}

TestSoundLoad::
~TestSoundLoad()
{
	//memleak...
}

void
TestSoundLoad::
Activate()
{
	Test::Activate();
}

void
TestSoundLoad::
Update()
{
	Test::Update();

	if(Sound)
	{
		if(Sound->ReadyForDelete())
		{
			delete Sound;
			Sound=NULL;
			PercentComplete=0.0f;
		}
	}

	if(Active)
	{
		if(Sound==NULL)
		{
			Sound = new LGL_Sound
			(
				Path,
				true,
				2,
				(Uint8*)Buffer,
				BUFFER_LENGTH
			);
			PercentComplete=0.0f;
		}

		if(Sound)
		{
			if(LGL_KeyStroke(SDLK_RETURN))
			{
				Sound->Play();
			}

			bool prepareForDelete=false;

			if(LoadFull)
			{
				PercentComplete = Sound->GetPercentLoadedSmooth();
				if(Sound->IsUnloadable())
				{
					printf("Sound '%s' is unloadable!\n",Sound->GetPath());
					PercentComplete=1.0f;
				}
				if(PercentComplete==1.0f)
				{
					if(Loop)
					{
						PercentComplete=0.99f;
					}
					prepareForDelete=true;
				}
			}
			else
			{
				if
				(
					Sound->PreparingForDelete()==false &&
					Sound->ReadyForDelete()==false
				)
				{
					Sound->PrepareForDelete();
				}
				if(Loop)
				{
					PercentComplete=-1.0f;
				}
				else
				{
					PercentComplete = Sound->GetPercentLoadedSmooth();
				}
				prepareForDelete=true;
			}

			if
			(
				prepareForDelete &&
				Sound->PreparingForDelete()==false &&
				Sound->ReadyForDelete()==false
			)
			{
				Sound->PrepareForDelete();
			}
		}
	}
}

void
TestSoundLoad::
Deactivate()
{
	Test::Deactivate();
}

void
TestSoundLoad::
SetLoadFull
(
	bool	loadFull
)
{
	LoadFull=loadFull;
	if(LoadFull)
	{
		strcpy(Name,"SoundLoad | Full");
	}
	else
	{
		strcpy(Name,"SoundLoad | Partial");
	}
}

void
TestSoundLoad::
SetLoop
(
	bool	loop
)
{
	Loop=loop;
}

void
TestSoundLoad::
SetPath
(
	const char*	path
)
{
	assert(path);
	strcpy(Path,path);
}

