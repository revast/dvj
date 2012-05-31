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
		640,480,
		2,
		"testFrameRate"
	);

	for(;;)
	{
		LGL_ProcessInput();
		LGL_DrawFPSGraph();
		LGL_SwapBuffers();
	}
}

