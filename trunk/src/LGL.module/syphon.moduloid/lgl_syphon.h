/*
 *
 * LGL_Syphon.h
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

