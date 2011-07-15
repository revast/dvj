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
SyphonServer*		syServer=NULL;
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

	for(int s=0;s<[serverList count];s++)
	{
		const char* name = [
				[
					[serverList objectAtIndex:s]
					objectForKey:SyphonServerDescriptionAppNameKey
				]
				cStringUsingEncoding:NSMacOSRomanStringEncoding
			];

		if(strcmp(name,"dvj.osx"))
		{
			serverIndex=s;
			break;
		}
	}

	//Abort if dvj is the only server
	{
		const char* name = [
				[
					[serverList objectAtIndex:serverIndex]
					objectForKey:SyphonServerDescriptionAppNameKey
				]
				cStringUsingEncoding:NSMacOSRomanStringEncoding
			];
		if(strcmp(name,"dvj.osx")==0)
		{
			return(false);
		}
	}

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

	[syImage release];
	syImage=NULL;

	return(true);
}

void
lgl_SyphonPushImage
(
	GLuint	glID,
	int	imgW,
	int	imgH,
	int	texW,
	int	texH
)
{	if(pool==NULL)
	{
		pool = [[NSAutoreleasePool alloc] init];
	}
	
	if(syServer==NULL)
	{
		CGLContextObj ctx = CGLGetCurrentContext();
		syServer = [[SyphonServer alloc] initWithName:@"dvj" context:ctx options:nil];
	}

	if(syServer)
	{
		[syServer publishFrameTexture:glID textureTarget:GL_TEXTURE_RECTANGLE_ARB imageRegion:NSMakeRect(0, 0, imgW, imgH) textureDimensions:NSMakeSize(texW, texH) flipped:NO];
	}
}

void
lgl_SyphonExit()
{
	if(pool==NULL)
	{
		pool = [[NSAutoreleasePool alloc] init];
	}

	if(syServer)
	{
		[syServer stop];
	}

	if(syClient)
	{
		[syClient stop];
		[syClient release];
		syClient = nil;
	}
}

