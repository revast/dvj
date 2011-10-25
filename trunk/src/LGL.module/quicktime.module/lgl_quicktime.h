/*
 *
 * lgl_quicktime.h
 *
 */

#import <OpenGL/OpenGL.h>

void*
lgl_quicktime_open
(
	const char*	path
);

void
lgl_quicktime_close
(
	void*		qtMovie
);

bool
lgl_quicktime_decode
(
	void*		qtMovie,
	double		time,
	unsigned char*	dstData,
	long		dstLen
);

void
lgl_quicktimeTest();

bool
lgl_quicktime_decode_jpeg
(
	unsigned char*	dstData,
	long		dstLen,
	unsigned char*	srcData,
	long		srcLen
);

