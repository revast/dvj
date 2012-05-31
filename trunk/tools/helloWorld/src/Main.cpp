/*
 *
 * Main.cpp
 *
 */

#include "LGL.module/LGL.h"

int
main
(
	int	argc,
	char**	argv
)
{
	LGL_Init
	(
		LGL_DisplayResolutionX()-100,
		LGL_DisplayResolutionY()-100,
		2,
		"testFrameRate"
	);

	for(;;)
	{
		LGL_ProcessInput();

		LGL_DrawFPSGraph();
		LGL_DebugPrintf("Hello World\n");

		LGL_SwapBuffers();
	}
}

