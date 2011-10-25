/*
 *
 * lgl_quicktime.mm
 *
 */

#import "lgl_quicktime.h"

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>

NSString*		moviePathString = @"/Users/id/mp3/test.mov";

void*
lgl_quicktime_open
(
	const char*	path
)
{
	NSAutoreleasePool* local_pool = [[NSAutoreleasePool alloc] init];

	NSError* error;
	QTMovie* movie = [QTMovie movieWithFile:moviePathString error:&error];
	if(movie)
	{
		[movie retain];
	}
	else
	{
		NSLog(@"lgl_quicktime_open(): Error opening file: %s",path);
		NSLog(@"\t%@",[error localizedDescription]);
	}

	[local_pool release];

	return(movie);
}

void
lgl_quicktime_close
(
	void*	qtMovie
)
{
	NSAutoreleasePool* local_pool = [[NSAutoreleasePool alloc] init];

	if(qtMovie)
	{
		QTMovie* movie = (QTMovie*)qtMovie;
		[movie release];
	}

	[local_pool release];
}

bool
lgl_quicktime_decode
(
	void*		qtMovie,
	double		time,
	unsigned char*	dstData,
	long		dstLen
)
{
	if(time<0) time=0;

	//NSBitmapImageRep method: This works but is slower than libjpeg-turbo
	/*
	NSAutoreleasePool* local_pool = [[NSAutoreleasePool alloc] init];

	NSData* data = [NSData dataWithBytesNoCopy:srcData length:srcLen freeWhenDone:false];
	NSBitmapImageRep* imageRep = [NSBitmapImageRep imageRepWithData:data]; 

	memcpy(dstData,[imageRep bitmapData],dstLen);

	[local_pool release];

	return(true);
	*/

	NSAutoreleasePool* local_pool = [[NSAutoreleasePool alloc] init];
	
	bool ret=false;

	QTMovie* movie = (QTMovie*) qtMovie;

	if(movie)
	{
		//printf("Movie!\n");
		//QTTime duration = [movie duration];
		//float durationFloat = duration.timeValue / (float)duration.timeScale;
		//printf("Duration: %.2f\n",durationFloat);
		NSImage* image = [movie frameImageAtTime:QTMakeTime(time/1000.0,1000)];
		if(image)
		{
			//printf("image!\n");
			NSBitmapImageRep *bitmap = [[image representations] objectAtIndex:0];
			if(bitmap)
			{
				//printf("bitmap!\n");
				/*
				if(const unsigned char* bitmapData = [bitmap bitmapData])
				{
					//printf("bitmap data!\n");
					//memcpy(dstData,bitmapData,dstLen);
					bzero(dstData,dstLen);
					ret=true;
				}
				*/
			}
		}
	}
	else
	{
		printf("Movie FAIL\n");
	}

	[local_pool release];
	
	return(ret);
}

/*
void
lgl_quicktimeTest()
{
	NSAutoreleasePool* local_pool = [[NSAutoreleasePool alloc] init];

	NSDictionary* attributes =
	[
		NSDictionary
		dictionaryWithObjectsAndKeys:
		//data, QTMovieDataAttribute,
		//[NSNumber numberWithBool:YES], QTMovieLoopsAttribute,
		//[NSNumber numberWithBool:YES], QTMovieOpenForPlaybackAttribute,
		moviePathString, QTMovieFileNameAttribute,
		nil
	];

	NSError* error;
	QTMovie* movie =
	[
		QTMovie
		movieWithAttributes:attributes
		error:&error
	];

	if(movie)
	{
		printf("qt Movie!\n");
		NSImage* image = [movie frameImageAtTime:QTMakeTime(60000,1000)];
		if(image)
		{
			printf("image!\n");
			NSBitmapImageRep *rep = [[image representations] objectAtIndex: 0];
			if(rep)
			{
				printf("rep!\n");
				NSData* data = [rep representationUsingType:NSPNGFileType
					properties: nil];
				[data writeToFile: @"/Users/id/test.png"
					atomically: NO];
			}
		}

		//Make a window
		{
			NSRect frame = NSMakeRect(0, 0, 200, 200);
			NSWindow* window  = [[[NSWindow alloc] initWithContentRect:frame
				styleMask:NSBorderlessWindowMask
				backing:NSBackingStoreBuffered
				defer:NO] autorelease];
			[window retain];
			[window setBackgroundColor:[NSColor blueColor]];
			[window makeKeyAndOrderFront:NSApp];

			[window setWindowController:
		}
	}
	else
	{
		printf("qt Failed!\n");
	}

	[local_pool release];
}
*/

bool
lgl_quicktime_decode_jpeg
(
	unsigned char*	dstData,
	long		dstLen,
	unsigned char*	srcData,
	long		srcLen
)
{
	return(false);

	//NSBitmapImageRep method: This works but is slower than libjpeg-turbo
	/*
	NSAutoreleasePool* local_pool = [[NSAutoreleasePool alloc] init];

	NSData* data = [NSData dataWithBytesNoCopy:srcData length:srcLen freeWhenDone:false];
	NSBitmapImageRep* imageRep = [NSBitmapImageRep imageRepWithData:data]; 

	memcpy(dstData,[imageRep bitmapData],dstLen);

	[local_pool release];

	return(true);
	*/

	//QTMovie method: This doesn't yet work
	/*
	NSData* data = [NSData dataWithBytesNoCopy:srcData length:srcLen freeWhenDone:false];

	NSNumber *numYes = [NSNumber numberWithBool:YES];
	//NSNumber *numNo = [NSNumber numberWithBool:NO];

	NSDictionary* attributes =
	[
		NSDictionary
		dictionaryWithObjectsAndKeys:
		data, QTMovieDataAttribute,
		//numYes, QTMovieLoopsAttribute,
		numYes, QTMovieOpenForPlaybackAttribute,
		nil
	];

	NSError* error;
	QTMovie* movie =
	[
		[QTMovie alloc]
		initWithAttributes:attributes
		error:&error
	];

	if(movie)
	{
		printf("Movie!\n");
		QTTime duration = [movie duration];
		float durationFloat = duration.timeValue / (float)duration.timeScale;
		printf("Duration: %.2f\n",durationFloat);
		[movie play];
		NSImage* image = [movie currentFrameImage];
		if(image)
		{
			printf("image!\n");
		}
	}
	else
	{
		printf("Movie FAIL\n");
		NSLog(@"\tError: %@",[error localizedDescription]);
	}

	return(false);
	*/
}

