/*
 *
 * Main.cpp
 *
 */

#include "LGL.module/LGL.h"

#include "Test.h"

#include "TestSoundLoad.h"
#include "TestSoundLoadAllFiles.h"
#include "TestVideoLoad.h"

int testKeystrokes[] =
{
	SDLK_0,
	SDLK_1,
	SDLK_2,
	SDLK_3,
	SDLK_4
};

int
main
(
	int	argc,
	char**	argv
)
{
	LGL_Init
	(
		640,480,
		2,
		"testFrameRate"
	);

	std::vector<Test*> tests;
	
	//Sound Load Full
	TestSoundLoad* tsl = new TestSoundLoad;
	tests.push_back(tsl);

	//Sound Load Partial
	tsl = new TestSoundLoad;
	tsl->SetLoadFull(false);
	tests.push_back(tsl);

	//Sound Load All Files
	tests.push_back(new TestSoundLoadAllFiles);
	
	//Video Load
	tests.push_back(new TestVideoLoad);

	//Video Load
	TestVideoLoad* tvl = new TestVideoLoad;
	tvl->SetVideoPos(0.65f,0.85f,0.40f,0.60f);
	tests.push_back(tvl);

	for(;;)
	{
		//Process Input

		LGL_ProcessInput();

		for(unsigned int a=0;a<tests.size();a++)
		{
			if(LGL_KeyStroke(testKeystrokes[a]))
			{
				tests[a]->ToggleActive();
			}
		}
		if(LGL_KeyStroke(SDLK_ESCAPE))
		{
			exit(0);
		}

		

		//Update Tests

		for(unsigned int a=0;a<tests.size();a++)
		{
			if(tests[a]->IsActive())
			{
				tests[a]->Update();
				if(tests[a]->GetPercentComplete()==1.0f)
				{
					tests[a]->Deactivate();
				}
			}
		}



		//Draw Interface

		LGL_GetFont().DrawString
		(
			0.5f,0.9f,0.06f,
			1,1,1,1,
			true,
			1.0f,
			"Frame Rate Test"
		);

		for(unsigned int a=0;a<tests.size();a++)
		{
			char pctStr[32];
			if(tests[a]->IsActive())
			{
				float pctComplete = tests[a]->GetPercentComplete();
				if(pctComplete>=0.0f)
				{
					sprintf(pctStr,"%.0f%%",tests[a]->GetPercentComplete()*100.0f);
				}
				else
				{
					int time = LGL_SecondsSinceExecution()*10;
					if((time%8) == 0)
					{
						strcpy(pctStr,"|");
					}
					if((time%8) == 1)
					{
						strcpy(pctStr,"/");
					}
					if((time%8) == 2)
					{
						strcpy(pctStr,"-");
					}
					if((time%8) == 3)
					{
						strcpy(pctStr,"\\");
					}
					if((time%8) == 4)
					{
						strcpy(pctStr,"|");
					}
					if((time%8) == 5)
					{
						strcpy(pctStr,"/");
					}
					if((time%8) == 6)
					{
						strcpy(pctStr,"-");
					}
					if((time%8) == 7)
					{
						strcpy(pctStr,"\\");
					}
				}
				LGL_GetFont().DrawString
				(
					0.05f,0.8f-0.05f*a,0.03f,
					1,1,1,1,
					true,
					1.0f,
					"%s",
					pctStr
				);
			}
			LGL_GetFont().DrawString
			(
				0.125f,0.8f-0.05f*a,0.03f,
				1,1,1,1,
				false,
				1.0f,
				"[%i] %s",
				a,
				tests[a]->GetName()
			);
		}

		LGL_DrawFPSGraph
		(
			0.0f,0.125f,
			0.875f,1.0f
		);

		if((1.0f/60.0f)>LGL_SecondsSinceThisFrame())
		{
			LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());
		}
		LGL_SwapBuffers();
	}
}

