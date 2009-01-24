/*
 *
 * TestSoundLoadAllFiles.h
 *
 */

#ifndef	_TEST_SOUND_LOAD_ALL_FILES_H
#define	_TEST_SOUND_LOAD_ALL_FILES_H

#include "Test.h"
#include "TestSoundLoad.h"

#include "LGL.module/LGL.h"

class TestSoundLoadAllFiles : public Test
{

public:

				TestSoundLoadAllFiles();
				~TestSoundLoadAllFiles();

	virtual	void		Activate();
	virtual void		Update();
	virtual	void		Deactivate();

				//

private:

	TestSoundLoad		CurrentTest;
	LGL_DirTree*		DirTree;
	int			DirTreeIndexNow;

};

#endif	//_TEST_SOUND_LOAD_ALL_FILES_H

