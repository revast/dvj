/*
 *
 * lgl_syphon.h
 *
 */

#import <OpenGL/OpenGL.h>

int
lgl_SyphonServerCount();

bool
lgl_SyphonImageInfo
(
	int	serverIndex,
	GLuint&	glID,
	int&	width,
	int&	height
);

void
lgl_SyphonPushImage
(
	GLuint	glID,
	int	imgW,
	int	imgH,
	int	texW,
	int	texH
);

void
lgl_SyphonExit();

