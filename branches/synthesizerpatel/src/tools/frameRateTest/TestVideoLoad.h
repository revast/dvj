/*
 *
 * TestVideoLoad.h
 *
 */

#ifndef	_TEST_VIDEO_LOAD_H
#define	_TEST_VIDEO_LOAD_H

#include "Test.h"

#include "LGL.h"

class TestVideoLoad : public Test
{

public:

				TestVideoLoad();
				~TestVideoLoad();

	virtual	void		Activate();
	virtual void		Update();
	virtual	void		Deactivate();

				//

		void		SetVideoPos(float left, float right, float bottom, float top);

private:

	LGL_Video*		Video;
	char*			Buffer;
	char			Path[1024];
	char			Path2[1024];
	float			Left;
	float			Right;
	float			Bottom;
	float			Top;
};

#endif	//_TEST_VIDEO_LOAD_H

