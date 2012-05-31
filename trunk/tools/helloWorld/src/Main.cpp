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

	LGL_Image img("data/image/fftw_wisdom.png");

	for(;;)
	{
		LGL_ProcessInput();

		img.DrawToScreen();
		LGL_DrawFPSGraph();
		LGL_DebugPrintf("Hello World\n");

		LGL_SwapBuffers();
	}
}

