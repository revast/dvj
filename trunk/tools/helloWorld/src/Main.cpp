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
		9999,
		9999,
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

