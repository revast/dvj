/*
 *
 * TestSoundLoad.h
 *
 */

#ifndef	_TEST_SOUND_LOAD_H
#define	_TEST_SOUND_LOAD_H

#include "Test.h"

#include "LGL.h"

class TestSoundLoad : public Test
{

public:

				TestSoundLoad();
				~TestSoundLoad();

	virtual	void		Activate();
	virtual void		Update();
	virtual	void		Deactivate();

				//

		void		SetLoadFull(bool loadFull=true);
		void		SetLoop(bool loop=true);
		void		SetPath(const char* path);

private:

	LGL_Sound*		Sound;
	bool			LoadFull;
	bool			Loop;
	char*			Buffer;
	char			Path[1024];
};

#endif	//_TEST_SOUND_LOAD_H

