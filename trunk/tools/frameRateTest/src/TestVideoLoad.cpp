/*
 *
 * TestVideoLoad.cpp
 *
 */

#include "TestVideoLoad.h"

#define BUFFER_LENGTH (1024*1024*100)

TestVideoLoad::
TestVideoLoad()
{
	Video=NULL;
	Buffer = new char[BUFFER_LENGTH];
	strcpy(Path,"data/testVideoLoad.mp3.mjpeg.avi");
	strcpy(Path2,"data/testVideoLoad2.mp3.mjpeg.avi");
	PercentComplete=-1.0f;
	strcpy(Name,"VideoLoad");

	Video = new LGL_VideoDecoder(Path);
	SetVideoPos(0.65f,0.85f,0.65f,0.85f);
}

TestVideoLoad::
~TestVideoLoad()
{
	//memleak...
	delete Video;
	Video=NULL;
}

void
TestVideoLoad::
Activate()
{
	Test::Activate();
}

void
TestVideoLoad::
Update()
{
	Test::Update();

	if(Active)
	{
		LGL_Image* img = Video->GetImage();
		{
			img->DrawToScreen(Left,Right,Bottom,Top);
		}
		if(LGL_RandInt(0,10)==0)
		{
			if(strcmp(Video->GetPath(),Path)==0)
			{
				Video->SetVideo(Path2);
			}
			else
			{
				Video->SetVideo(Path);
			}
		}
		float time = LGL_SecondsSinceExecution();
		while(time>60.0f)
		{
			time-=60.0f;
		}
		Video->SetTime(30.0f+time);
	}
}

void
TestVideoLoad::
Deactivate()
{
	Test::Deactivate();
}

void
TestVideoLoad::
SetVideoPos
(
	float	left,
	float	right,
	float	bottom,
	float	top
)
{
	Left=left;
	Right=right;
	Bottom=bottom;
	Top=top;
}

