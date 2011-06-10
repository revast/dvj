/*
 *
 * LGL_Syphon.m
 *
 */

#import "lgl_syphon.h" 

#import <Cocoa/Cocoa.h>
#import "stdio.h"

#import <Syphon.h>

SyphonClient*		syClient=NULL;
SyphonImage*		syImage=NULL;
NSAutoreleasePool*	pool=NULL;

int
lgl_SyphonServerCount()
{
	if(pool==NULL)
	{
		pool = [[NSAutoreleasePool alloc] init];
	}

	SyphonServerDirectory* syDirectory = [SyphonServerDirectory sharedDirectory];
	NSArray* serverList = [syDirectory servers];

	int count=0;
	for(NSDictionary* serverDescription in serverList)
	{
		count++;
	}
	return(count);
}

bool
lgl_SyphonImageInfo
(
	int	serverIndex,
	GLuint&	glID,
	int&	width,
	int&	height
)
{
	if(pool==NULL)
	{
		pool = [[NSAutoreleasePool alloc] init];
	}

	SyphonServerDirectory* syDirectory = [SyphonServerDirectory sharedDirectory];
	NSArray* serverList = [syDirectory servers];

	if([serverList count] == 0)
	{
		return(false);
	}

	if(serverIndex>[serverList count]-1)
	{
		serverIndex=[serverList count]-1;
	}
	if(serverIndex<0)
	{
		serverIndex=0;
	}
	serverIndex=0;	//For now, only support server 0 for simplicity.

	NSDictionary* server = [serverList objectAtIndex:serverIndex];

	if(syClient==NULL)
	{
		syClient = [SyphonClient alloc];
		[syClient initWithServerDescription:server options:nil newFrameHandler:nil];
	}

	if([syClient isValid]==false)
	{
		[syClient stop];
		[syClient release];
		syClient = nil;
		return(false);
	}

	CGLContextObj ctx = CGLGetCurrentContext();
	GLuint idPrev=syImage ? syImage.textureName : 0;
	syImage = [syClient newFrameImageForContext:ctx];

	if
	(
		idPrev!=0 &&
		(
			syImage==NULL ||
			syImage.textureName != idPrev
		)
	)
	{
		glDeleteTextures(1,&idPrev);
	}

	if(syImage==NULL)
	{
		return(false);
	}

	glID=syImage.textureName;
	NSSize imageSize = [syImage textureSize];
	width=(int)imageSize.width;
	height=(int)imageSize.height;

	/*
	[syClient stop];
	[syClient release];
	syClient = nil;
	*/

	[syImage release];
	syImage=NULL;

	return(true);
}

