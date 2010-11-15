/*
 *
 * Visualizer.cpp
 *
 * Copyright Chris Nelson (interim.descriptor@gmail.com), 2009
 *
 * This file is part of dvj.
 *
 * dvj is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dvj is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dvj.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "Visualizer.h"

#include "Common.h"
#include "Config.h"

#include <string.h>

bool NOISE_IMAGE_INITIALIZED=false;
LGL_Image* VisualizerObj::NoiseImage[NOISE_IMAGE_COUNT_128_128];

VisualizerObj::
VisualizerObj()
{
	BlueScreenOfDeath=new LGL_Image("data/image/bsod.png");

	//AccumulationNow=new LGL_Image(0.0f,0.5f,0.5f,1.0f);
	float resX=LGL_DisplayResolutionX(0);
	float center = resX/2.0f;
	float left=LGL_Max(0.0f,(center-0.5f*resX)/resX);
	float right=LGL_Min(1.0f,(center+0.5f*resX)/resX);
	//float right=LGL_Min(1.0f,GetProjectorQuadrentResX()/(float)LGL_DisplayResolutionX(0));
	float bottom=LGL_Max(0.5f,1.0f-GetProjectorQuadrentResY()/(float)LGL_DisplayResolutionY(0));
	float top=1.0f;

	if(LGL_DisplayCount()>1)
	{
		float projAspect = LGL_DisplayResolutionX(1)/(float)LGL_DisplayResolutionY(1);
		right = projAspect * (top-bottom);
	}

	SetViewportVisuals(left,right,bottom,top);

	FullScreen=false;

	ScrollTextBottomLeftPosition = 0.55f;
	ScrollTextEnabled=false;
	if(0 && LGL_DirectoryExists("data/text/ambient"))
	{
		LGL_DirTree scrollTextAmbientDirTree;
		scrollTextAmbientDirTree.SetPath("data/text/ambient");
		scrollTextAmbientDirTree.SetFilterText("txt");

		std::vector<char*> random;

		for(unsigned int a=0;a<scrollTextAmbientDirTree.GetFileCount();a++)
		{
			const char* filename=scrollTextAmbientDirTree.GetFileName(a);
			char* neo=new char[strlen(filename)+1];
			strcpy(neo,filename);
			random.push_back(neo);
		}

		random_shuffle(random.rbegin(),random.rend());

		for(unsigned int a=0;a<random.size();a++)
		{
			ScrollTextAmbientFileQueue.push_back(random[a]);
			random[a]=NULL;
		}
		random.clear();
	}

	VideoFPSDisplay=0.0f;
	LowMemoryWarningScalar=0.0f;
	LowMemoryMB=75;

	char tmp[2048];

	//Prepare Random Video Queue

	char videoPath[2048];
	sprintf(videoPath,"%s/.dvj/video",LGL_GetHomeDir());
	sprintf(VideoRandomPath,"%s/random",videoPath);
	sprintf(VideoRandomLowPath,"%s/low",VideoRandomPath);
	sprintf(VideoRandomHighPath,"%s/high",VideoRandomPath);

	for(int q=0;q<2;q++)
	{
		LGL_DirTree videoRandomDirTree;
		videoRandomDirTree.SetPath(NULL);
		char* pathNow = q==0 ? VideoRandomLowPath : VideoRandomHighPath;
		if(LGL_DirectoryExists(pathNow))
		{
			videoRandomDirTree.SetPath(pathNow);
			videoRandomDirTree.WaitOnWorkerThread();
		}
		if(videoRandomDirTree.GetFileCount()==0)
		{
			videoRandomDirTree.SetPath(NULL);
			if(LGL_DirectoryExists(VideoRandomPath))
			{
				strcpy(pathNow,VideoRandomPath);
				videoRandomDirTree.SetPath(VideoRandomPath);
				videoRandomDirTree.WaitOnWorkerThread();
			}
		}
		for(unsigned int a=0;a<videoRandomDirTree.GetFileCount();a++)
		{
			sprintf(tmp,"%s",videoRandomDirTree.GetFileName(a));
			char* str=new char[strlen(tmp)+1];
			strcpy(str,tmp);
			if(q==0)
			{
				VideoRandomLowQueue.push_back(str);
			}
			else
			{
				VideoRandomHighQueue.push_back(str);
			}
		}
	}

	random_shuffle(VideoRandomLowQueue.rbegin(),VideoRandomLowQueue.rend());
	random_shuffle(VideoRandomHighQueue.rbegin(),VideoRandomHighQueue.rend());

	VideoRandomGetCount=0;

	if(NOISE_IMAGE_INITIALIZED==false)
	{
		for(int a=0;a<NOISE_IMAGE_COUNT_256_64;a++)
		{
			char path[1024];
			sprintf
			(
				path,
				"data/image/noise/128x128/%02i.png",
				a
			);
			LGL_Assertf(LGL_FileExists(path),("Noise file '%s' doesn't exist\n",path));
			NoiseImage[a] = new LGL_Image(path);
		}
		NOISE_IMAGE_INITIALIZED=true;
	}
}

VisualizerObj::
~VisualizerObj()
{
	delete	BlueScreenOfDeath;
	//delete	AccumulationNow;
	//TODO: Take care of scroll text buffers
}

void
VisualizerObj::
NextFrame
(
	float		secondsElapsed,
	TurntableObj**	tts
)
{
	//Frequency-sensitive video mixing
	for(int t=0;t<2;t++)
	{
		if(tts[t]->GetFreqSenseBrightnessPreview()>0.0f)
		{
			LGL_VideoDecoder* vidL=tts[t]->GetVideoLo();
			LGL_VideoDecoder* vidH=tts[t]->GetVideoHi();

			float volAve;
			float volMax;
			float freqFactor;
			tts[t]->GetFreqMetaData(volAve,volMax,freqFactor);
			float vol=LGL_Min(1.0f,volAve*2);

			//See if we should time-jump in any of our vids
			for(int a=0;a<2;a++)
			{
				LGL_VideoDecoder* vid = (a==0) ? vidL : vidH;
				if(vid)
				{
					float neoVol=vol;
					float neoFreqFactor=freqFactor;
					float oldFreqBrightness = vid->StoredBrightness;
					float neoFreqBrightness = GetFreqBrightness((vid==vidH),neoFreqFactor,neoVol);
					if
					(
						oldFreqBrightness>0.0f &&
						neoFreqBrightness==0.0f
					)
					{
						float min=LGL_Min(10.0f,vid->GetLengthSeconds()-5.0f);
						if(min<0.0f) min=0.0f;
						float max=vid->GetLengthSeconds()-10.0f;
						if(max<0.0f) max=vid->GetLengthSeconds();
						vid->SetTime(LGL_RandFloat(min,max));
					}
					else
					{
						float speedFactor=
							(tts[t]->GetAudioInputMode() ? 1.0f : tts[t]->GetFinalSpeed()) *
							((vid==vidL) ? 1.0f : 4.0f);
						vid->SetTime(vid->GetTime()+speedFactor*(1.0f/60.0f));
					}
					vid->StoredBrightness=neoFreqBrightness;
				}
			}
		}
	}

	if
	(
		ScrollTextCurrentAmbientFileBuffer.empty() &&
		ScrollTextAmbientFileQueue.empty()==false
	)
	{
		char path[1024];
		sprintf(path,"data/text/ambient/%s",ScrollTextAmbientFileQueue[0]);
		
		ScrollTextAmbientFileQueueUsed.push_back(ScrollTextAmbientFileQueue[0]);
		ScrollTextAmbientFileQueue[0]=NULL;
		ScrollTextAmbientFileQueue.erase((std::vector<char*>::iterator)(&(ScrollTextAmbientFileQueue[0])));
		if(ScrollTextAmbientFileQueue.empty())
		{
			random_shuffle(ScrollTextAmbientFileQueueUsed.rbegin(),ScrollTextAmbientFileQueueUsed.rend());
			for(unsigned int a=0;a<ScrollTextAmbientFileQueueUsed.size();a++)
			{
				ScrollTextAmbientFileQueue.push_back(ScrollTextAmbientFileQueueUsed[a]);
				ScrollTextAmbientFileQueueUsed[a]=NULL;
			}
			ScrollTextAmbientFileQueueUsed.clear();
		}

		if(LGL_FileExists(path))
		{
			PopulateCharStarBufferWithScrollTextFile(ScrollTextCurrentAmbientFileBuffer,path);
			strcpy(ScrollTextCurrentAmbientFile,path);
		}
	}
	if(ScrollTextEnabled)
	{
		if(ScrollTextBuffer.empty()==false)
		{
			ScrollTextBottomLeftPosition-=0.1f*secondsElapsed;
			if(ScrollTextBottomLeftPosition + LGL_GetFont().GetWidthString(0.02f,ScrollTextBuffer[0]) <= -0.25f)
			{
				ScrollTextBottomLeftPosition = 0.55f;
				delete ScrollTextBuffer[0];
				ScrollTextBuffer[0]=NULL;
				ScrollTextBuffer.erase((std::vector<char*>::iterator)(&(ScrollTextBuffer[0])));
			}
		}
		else if(ScrollTextCurrentTrackBuffer.empty()==false)
		{
			//Place the next item from the current track into ScrollTextBuffer
			ScrollTextBuffer.push_back(ScrollTextCurrentTrackBuffer[0]);
			ScrollTextCurrentTrackBuffer[0]=NULL;
			ScrollTextCurrentTrackBuffer.erase((std::vector<char*>::iterator)(&(ScrollTextCurrentTrackBuffer[0])));
		}
		else if(ScrollTextCurrentAmbientFileBuffer.empty()==false)
		{
			//Place the next item from the ambient file into ScrollTextBuffer
			ScrollTextBuffer.push_back(ScrollTextCurrentAmbientFileBuffer[0]);
			ScrollTextCurrentAmbientFileBuffer[0]=NULL;
			ScrollTextCurrentAmbientFileBuffer.erase((std::vector<char*>::iterator)(&(ScrollTextCurrentAmbientFileBuffer[0])));
		}
	}

	return;
}

void
VisualizerObj::
DrawVisuals
(
	bool		visualizerQuadrent,
	float		visualizerZoomOutPercent,
	TurntableObj**	tts
)
{
	float l=ViewportVisualsLeft;
	float r=ViewportVisualsRight;
	float b=ViewportVisualsBottom;
	float t=ViewportVisualsTop;
	float w=ViewportVisualsWidth;
	float h=ViewportVisualsHeight;

	if(FullScreen)
	{
		l=0;
		r=1;
		b=0;
		t=1;
		w=1;
		h=1;
	}

	if(visualizerQuadrent)
	{
		float pct=visualizerZoomOutPercent;
		LGL_ClipRectEnable
		(
			(0.0f+pct)*0.0f+
			(1.0f-pct)*l,
			(0.0f+pct)*0.25f+
			(1.0f-pct)*r,
			(0.0f+pct)*0.75f+
			(1.0f-pct)*b,
			(0.0f+pct)*1.0f+
			(1.0f-pct)*t
		);
	}
	else
	{
		LGL_ClipRectEnable(l,r,b,t);
	}

	//Warn on low memory
	if(LGL_GetActiveDisplay()==0)
	{
		LowMemoryWarningScalar-=1.0f/60.0f;
		if(LowMemoryWarningScalar<0) LowMemoryWarningScalar=0.0f;
		if(LGL_RamFreeMB()<75)
		{
			LowMemoryWarningScalar=5.0f;
		}

		if(LowMemoryWarningScalar>0)
		{
			float bri=LGL_Min(LowMemoryWarningScalar,1.0f);
			if(LowMemoryTimer.SecondsSinceLastReset()>1.0f)
			{
				LowMemoryMB=LGL_RamFreeMB();
			}
			LGL_GetFont().DrawString
			(
				l+0.025f*w,
				b+0.90f*h,
				h*0.04f,
				bri,0,0,bri,
				false,
				0.75f*bri,
				"LOW MEMORY: %iMB",
				LowMemoryMB
			);
			LGL_GetFont().DrawString
			(
				l+0.025f*w,
				b+0.825f*h,
				h*0.04f,
				bri,0,0,bri,
				false,
				0.75f*bri,
				"Audio might skip"
			);
		}
	}

	if(LGL_AudioAvailable())
	{
		if(LGL_VidCamAvailable())
		{
			/*
			LGL_VidCamImageRaw()->DrawToScreen
			(
				l,r,
				b,t,
				0,
				1,1,1,1
			);
			*/
			LGL_VidCamImageProcessed()->DrawToScreen
			(
				l,r,
				b,t,
				0,
				1,1,1,1
			);
		}
		/*
		float s=2.0f*LGL_SecondsSinceLastFrame();
		float y=2.0-(1+0);//LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_YAXIS));
		AccumulationNow->DrawToScreen
		(
			l-y*s*w,	r+y*s*w,
			b-y*s*h,	t+y*s*h,
			2*s*0.0f,//LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS),
			1.0-s,1.0-s,1.0-s,0.0f
		);
		*/
	}
	else
	{
		if(LGL_AudioWasOnceAvailable())
		{
			BlueScreenOfDeath->DrawToScreen
			(
				l,r,
				b,t,
				0,
				1,1,1,1
			);
		}
		else
		{
			/*
			NoSound->DrawToScreen
			(
				l,r,
				b,t,
				0,
				1,1,1,1
			);
			*/
		}
	}

	//The ACTUAL videos we're drawing.
	for(int videoNow=0;videoNow<2;videoNow++)
	{
		DrawVideos
		(
			tts[videoNow],
			l,
			r,
			b,
			t,
			false
		);
	}
	
	/*

	//AccumulationNow->FrameBufferUpdate();

	//Draw Scroll Text
	if(ScrollTextBuffer.empty()==false)
	{
		//Bottom
		LGL_GetFont().DrawString
		(
			2*w*ScrollTextBottomLeftPosition,b+0.01f*2*h,0.02f,
			1,1,1,1,
			false,
			0.75f,
			ScrollTextBuffer[0]
		);

		//Top
		LGL_GetFont().DrawString
		(
			- LGL_GetFont().GetWidthString(0.02f,ScrollTextBuffer[0]) + 2*w*(0.5f - ScrollTextBottomLeftPosition),b+0.95f*h,0.02f,
			1,1,1,1,
			false,
			0.75f,
			ScrollTextBuffer[0]
		);
	}
	*/
	LGL_ClipRectDisable();
}

/*
void
VisualizerObj::
DrawStatus()
{
	if(FullScreen)
	{
		return;
	}

	float l=ViewportStatusLeft;
	float r=ViewportStatusRight;
	float b=ViewportStatusBottom;
	//float t=ViewportStatusTop;
	float w=ViewportStatusWidth;
	float h=ViewportStatusHeight;
	
	//Draw memory usage
	LGL_GetFont().DrawString
	(
		l+.800*w,
		b+.9*h,
		.020,
		1,1,1,1,
		false,.5,
		"Mem:%.2f",
		LGL_TextureUsageMB()
	);

	LGL_GetFont().DrawString
	(
		l+.825f*w,
		b+.95*h,
		.02f,
		1,1,1,1,
		false,.5f,
		LGL_TimeOfDay()
	);

	if(Recording)
	{
		int seconds=(int)
			(
				LGL_SecondsSinceExecution()-
				RecordingSecondsSinceExecution
			);

		int minutes=0;
		while(seconds>=60)
		{
			seconds-=60;
			minutes++;
		}
		int hours=0;
		while(minutes>=60)
		{
			minutes-=60;
			hours++;
		}
		LGL_GetFont().DrawString
		(
			l+.825f*w,
			b+.8875f*h,
			.02f,
			1.0f,0.0f,0.0f,1.0f,
			false,.5f,
			"%.2i:%.2i.%.2i",
			hours,minutes,seconds
		);
	}

	LGL_DrawFPSGraph
	(
		l+.825f*w,r-.025f*w,
		b+.710f*h,b+.860f*h
	);

	if(LGL_VidCamAvailable())
	{
		LGL_GetFont().DrawString
		(
			0.01,
			b+.95*h,
			.020,
			1,1,1,1,
			false,.5,
			"VidCam FPS: %i",
			LGL_VidCamFPS()
		);
	}
	
	for(int a=0;a<511;a++)
	{
		float x1=l+.5*w+.5*w*(a+0)/512.0;
		float x2=l+.5*w+.5*w*(a+1)/512.0;
		int l=(int)(512-floor(a/1));
		float y1=LGL_FreqBufferL(l,0);//512-(l+1));
		
		LGL_DrawRectToScreen
		(
			x1,x2,
			b+.5*h,b+.5*h+.5*h*y1*30,
			.4,.2,1,1
		);
	}

	//Draw ImageSet Stuff
	if(ImageSet.empty()==false)
	{
		//Left
		{
			int which=LGL_RandInt(0,ImageSet.size()-1);
			ImageSet[which]->DrawToScreen
			(
				l+.5*w,l+.75*w,
				b,b+.25*h,
				0,
				1,1,1,1
			);
		}
		//Last
		{
			ImageSet[ImageSetLastWhich]->DrawToScreen
			(
				l+.75*w,r,
				b,b+.25*h,
				0,
				1,1,1,1
			);
		}
	}
	if(ImageSetNext.size()>1)
	{
		//Load
		{
			int which=LGL_RandInt(0,ImageSetNext.size()-2);
			ImageSetNext[which]->DrawToScreen
			(
				l+.5*w,l+.75*w,
				b+.25*h,b+.5*h,
				0,
				1,1,1,1
			);
		}
		//Right
		{
			int which=ImageSetNext.size()-2;
			ImageSetNext[which]->DrawToScreen
			(
				l+.75*w,r,
				b+.25*h,b+.5*h,
				0,
				1,1,1,1
			);
		}
	}
	
	//Draw MovieClip Stuff
	for(int a=0;a<MovieClipNum;a++)
	{
		float DivideBy=LGL_Max(4,MovieClipNum);
		if(MovieClipList[a].empty()==false)
		{
			float s=(MovieClipList[a].size()-1)/30.0;

			//Left
			{
				int which=
					(int)floor(MovieClipScratchL*(MovieClipList[a].size()-1));

				MovieClipList[a][which]->DrawToScreen
				(
					l,l+1/(DivideBy)*w,
					b+(a+0)/(DivideBy)*h,
					b+(a+1)/(DivideBy)*h,
					0,
					1,1,1,1
				);
			}
			//Loop
			{
				int which=(int)floor
				(
					(
						LGL_SecondsSinceExecution()/s-
						floor(LGL_SecondsSinceExecution()/s)
					)*
					(MovieClipList[a].size()-1)
				);
				MovieClipList[a][which]->DrawToScreen
				(
					l+(1)/(DivideBy)*w,
					l+(2)/(DivideBy)*w,
					b+(a+0)/(DivideBy)*h,
					b+(a+1)/(DivideBy)*h,
					0,
					1,1,1,1
				);
			}
		}
		
		int victim=0;
		float victimAge=0;
		for(int q=0;q<MovieClipNum;q++)
		{
			if(MovieClipTimer[q].SecondsSinceLastReset()>victimAge)
			{
				victim=q;
				victimAge=MovieClipTimer[q].SecondsSinceLastReset();
			}
			if(MovieClipList[q].size()==0)
			{
				victim=q;
				victimAge=999999;
				break;
			}
		}
		if(MovieClipLoading.size()>1 && victim==a)
		{
			//Load
			{
				int which=MovieClipLoading.size()-2;
				MovieClipLoading[which]->DrawToScreen
				(
					l+(2)/(DivideBy)*w,
					l+(3)/(DivideBy)*w,
					b+(victim+0)/(DivideBy)*h,
					b+(victim+1)/(DivideBy)*h,
					0,
					1,1,1,1
				);
			}
			//Right
			{
				int which=(int)floor(MovieClipScratchL*(MovieClipLoading.size()-2));
				MovieClipLoading[which]->DrawToScreen
				(
					l+(3)/(DivideBy)*w,
					l+(4)/(DivideBy)*w,
					b+(victim+0)/(DivideBy)*h,
					b+(victim+1)/(DivideBy)*h,
					0,
					1,1,1,1
				);
			}
		}
		else
		{
			char temp[1024];
			sprintf(temp,"%i",a);
			LGL_GetFont().DrawString
			(
				l+(2.5)/(DivideBy)*w,
				b+(a+0.475)/((float)MovieClipNum)*h,
				.020,
				1,1,1,1,
				true,.5,
				temp
			);
		}
	}
}
*/

void
VisualizerObj::
SetViewportVisuals
(
	float	left,	float	right,
	float	bottom,	float	top
)
{
	ViewportVisualsLeft=left;
	ViewportVisualsRight=right;
	ViewportVisualsBottom=bottom;
	ViewportVisualsTop=top;
	ViewportVisualsWidth=right-left;
	ViewportVisualsHeight=top-bottom;

	//AccumulationNow->FrameBufferViewport(left,right,bottom,top);
}

float
VisualizerObj::
GetViewportVisualsWidth()
{
	return(ViewportVisualsRight-ViewportVisualsLeft);
}

float
VisualizerObj::
GetViewportVisualsHeight()
{
	return(ViewportVisualsTop-ViewportVisualsBottom);
}

float
VisualizerObj::
GetViewportRight()
{
	return(ViewportVisualsRight);
}

void
VisualizerObj::
ToggleFullScreen()
{
	FullScreen=!FullScreen;
	/*
	if(FullScreen)
	{
		AccumulationNow->FrameBufferViewport
		(
			0,1,
			0,1
		);
	}
	else
	{
		AccumulationNow->FrameBufferViewport
		(
			ViewportVisualsLeft,	ViewportVisualsRight,
			ViewportVisualsBottom,	ViewportVisualsTop
		);
	}
	*/
}

void
VisualizerObj::
QueueScrollText
(
	const
	char*	text
)
{
return;
	if(text==NULL)
	{
		return;
	}

	char* neo=new char[strlen(text)+1];
	strcpy(neo,text);
	ScrollTextBuffer.push_back(neo);
	ScrollTextEnabled=true;
}

void
VisualizerObj::
MaybeSetScrollTextTrackFile
(
	const
	char*	trackFileName
)
{
	bool clearCurrentTrackBuffer=false;
	if(trackFileName==NULL)
	{
		clearCurrentTrackBuffer=true;
	}
	else
	{
		char scrollTextFileNamePath[1024];
		sprintf(scrollTextFileNamePath,"data/text/tracks/%s.txt",trackFileName);
		if(LGL_FileExists(scrollTextFileNamePath))
		{
			char test[1024];
			sprintf(test,"%s.txt",trackFileName);
			if(strcmp(ScrollTextCurrentTrackFile,test)==0)
			{
				return;
			}
			else
			{
				//
			}
			clearCurrentTrackBuffer=true;
		}
	}

	if(clearCurrentTrackBuffer)
	{
		for(unsigned int a=0;a<ScrollTextCurrentTrackBuffer.size();a++)
		{
			delete ScrollTextCurrentTrackBuffer[a];
			ScrollTextCurrentTrackBuffer[a]=NULL;
		}
		ScrollTextCurrentTrackBuffer.clear();
	}
	
	if(trackFileName==NULL)
	{
		return;
	}

	char scrollTextFileNamePath[1024];
	sprintf(scrollTextFileNamePath,"data/text/tracks/%s.txt",trackFileName);
	if(LGL_FileExists(scrollTextFileNamePath))
	{
		PopulateCharStarBufferWithScrollTextFile(ScrollTextCurrentTrackBuffer,scrollTextFileNamePath);
		sprintf(ScrollTextCurrentTrackFile,"%s.txt",trackFileName);
	}
}

bool
VisualizerObj::
GetScrollTextEnabled()
{
	return(ScrollTextEnabled);
}

void
VisualizerObj::
GetNextVideoPathRandomLow
(
	char*	path
)
{
	GetNextVideoPathRandom(path,true);
}

void
VisualizerObj::
GetNextVideoPathRandomHigh
(
	char*	path
)
{
	GetNextVideoPathRandom(path,false);
}

void
VisualizerObj::
GetNextVideoPathRandom
(
	char*	path,
	bool	low
)
{
	std::vector<char*>& queue = low ? VideoRandomLowQueue : VideoRandomHighQueue;
	const char* videoPath=low ? VideoRandomLowPath : VideoRandomHighPath;
	if(queue.empty())
	{
		path[0]='\0';
		return;
	}

	sprintf(path,"%s/%s",videoPath,queue[0]);
	char* str=queue[0];
	queue.erase((std::vector<char*>::iterator)(&(queue[0])));
	queue.push_back(str);

	VideoRandomGetCount++;
	if(VideoRandomGetCount==queue.size())
	{
		random_shuffle(queue.rbegin(),queue.rend());
		VideoRandomGetCount=0;
	}
}

//Privates

void
VisualizerObj::
ForceVideoToBackOfRandomQueue
(
	const
	char*	pathShort
)
{
	if(pathShort==NULL)
	{
		return;
	}

	//If we're drawing a video to the screen, then we want to put it at the end of the random list.
	for(int q=0;q<2;q++)
	{
		std::vector<char*>& queue = (q==0) ? VideoRandomLowQueue : VideoRandomHighQueue;
		if
		(
			queue.size()>1 &&
			strcmp(pathShort,queue[queue.size()-1])!=0
		)
		{
			for(int a=queue.size()-1;a>=0;a--)
			{
				if(strcmp(pathShort,queue[a])==0)
				{
					char* tmp=queue[a];
					queue.erase((std::vector<char*>::iterator)(&(queue[a])));
					queue.push_back(tmp);
					break;
				}
			}
		}
	}
}

void
VisualizerObj::
PopulateCharStarBufferWithScrollTextFile
(
	std::vector<char*>&	buffer,
	const char*		path
)
{
	if(path==NULL)
	{
		return;
	}

	FILE* file = fopen(path,"r");
	if(file)
	{
		char line[2048];
		for(;;)
		{
			fgets(line,2047,file);
			if(feof(file))
			{
				break;
			}

			int percents=0;
			for(unsigned int c=0;c<strlen(line);c++)
			{
				if(line[c]=='%')
				{
					percents++;
				}
			}

			char* neo = new char[strlen(line)+1+percents];
			strcpy(neo,line);

			for(unsigned int c=0;c<strlen(neo);c++)
			{
				//We have to double-up our % signs, so they aren't interpreted by varargs
				if(neo[c]=='%')
				{
					char temp[2048];
					strcpy(temp,&(neo[c+1]));
					strcpy(&(neo[c+2]),temp);
					neo[c+1]='%';
					c++;
				}
			}

			if
			(
				strlen(neo)>0 &&
				neo[strlen(neo)-1]=='\n'
			)
			{
				neo[strlen(neo)-1]='\0';
			}
			buffer.push_back(neo);
		}
		fclose(file);
	}
}

void
VisualizerObj::
DrawVideos
(
	TurntableObj*	tt,
	float		l,
	float		r,
	float		b,
	float		t,
	bool		preview
)
{
	float lOrig=l;
	float rOrig=r;
	float wOrig=r-l;
	float bOrig=b;
	float tOrig=t;
	float hOrig=t-b;
	float w=r-l;
	float h=t-b;

	float videoBright = preview ? tt->GetVideoBrightnessPreview() : tt->GetVideoBrightnessFinal();
	float oscilloscopeBright = preview ? tt->GetOscilloscopeBrightnessPreview() : tt->GetOscilloscopeBrightnessFinal();
	float freqSenseBright = preview ? tt->GetFreqSenseBrightnessPreview() : tt->GetFreqSenseBrightnessFinal();

//if(preview) printf("Brights: %.2f, %.2f, %.2f\n",videoBright,oscilloscopeBright,freqSenseBright);

	if(preview)
	{
		/*
		float projAR=
			(LGL_WindowResolutionX()*(ViewportVisualsRight-ViewportVisualsLeft))/(float)
			(LGL_WindowResolutionY()*(ViewportVisualsTop-ViewportVisualsBottom));

		if(LGL_DisplayCount()>1)
		{
			int oldDisplay=LGL_GetActiveDisplay();
			LGL_SetActiveDisplay(1);
			projAR=LGL_DisplayAspectRatio();
			LGL_SetActiveDisplay(oldDisplay);
		}

		int previewPixelL=l*LGL_WindowResolutionX();
		int previewPixelR=r*LGL_WindowResolutionX();
		int previewPixelB=b*LGL_WindowResolutionY();
		int previewPixelT=t*LGL_WindowResolutionY();

		float previewPixelAR = (previewPixelR-previewPixelL)/(float)(previewPixelT-previewPixelB);

		float yCenter=(b+t)/2.0f;
		float yRadius=yCenter-b;

		b=yCenter-(previewPixelAR/projAR)*yRadius;
		t=yCenter+(previewPixelAR/projAR)*yRadius;
		*/
	}

	//float w=r-l;
	//float h=t-b;

	if(freqSenseBright>0.0f)
	{
		//Frequency-sensitive video mixing

		LGL_VideoDecoder* vidL=tt->GetVideoLo();
		LGL_VideoDecoder* vidH=tt->GetVideoHi();

		float volAve;
		float volMax;
		float freqFactor;
		tt->GetFreqMetaData(volAve,volMax,freqFactor);
		volAve = LGL_Min(1.0f,volAve*2.0f);

		for(int a=0;a<2;a++)
		{
			LGL_VideoDecoder* vid = (a==0) ? vidL : vidH;
			if(vid)
			{
				float vol = LGL_Min(1,volAve*tt->GetGain());
				float multFreq = (vid==vidL) ? tt->GetEQLo() : tt->GetEQHi();
				float myFreqFactor=freqFactor;
				float br = GetFreqBrightness(a,myFreqFactor,vol)*multFreq;
				br*=vid->StoredBrightness*freqSenseBright;

				LGL_Image* image = vid->GetImage();//EIGHT_WAY ? !preview : preview);
				{
					if
					(
						image &&
						image->GetFrameNumber()!=-1
					)
					{
						while(br>0.0f)
						{
							image->DrawToScreen
							(
								l,r,b,t,
								0,
								br,
								br,
								br,
								0.0f
							);
							br-=1.0f;
						}
					}
				}
			}
		}
	}

	if(oscilloscopeBright > 0.0f)
	{
		//FIXME: This should be a single path for uniform oscilloscope rendering
		if(tt->GetAudioInputMode())
		{
			float coolR;
			float coolG;
			float coolB;
			GetColorCool(coolR,coolG,coolB);

			float warmR;
			float warmG;
			float warmB;
			GetColorWarm(warmR,warmG,warmB);

			float volAve;
			float volMax;
			float freqFactor;
			LGL_AudioInMetadata(volAve,volMax,freqFactor);

			float red=
				(1.0f-freqFactor)*coolR+
				(0.0f+freqFactor)*warmR;
			float green=
				(1.0f-freqFactor)*coolG+
				(0.0f+freqFactor)*warmG;
			float blue=
				(1.0f-freqFactor)*coolB+
				(0.0f+freqFactor)*warmB;

			if(volMax>=0.99f)
			{
				red=1.0f;
				green=0.0f;
				blue=0.0f;
			}

			if
			(
				GetFreqBrightness(false,freqFactor,2*volAve*1.0f) ||
				GetFreqBrightness(true,freqFactor,2*volAve*1.0f)
			)
			{
				LGL_DrawAudioInWaveform
				(
					l,r,b,t,
					red*oscilloscopeBright,green*oscilloscopeBright,blue*oscilloscopeBright,0.0f,
					3.0f,
					true
				);
			}
		}
		else
		{
			tt->DrawWave
			(
				l,r,b,t,
				oscilloscopeBright,
				preview
			);
		}
	}
	
	if
	(
		videoBright > 0.0f &&
		1//tt->GetAudioInputMode()==false
	)
	{
		if(LGL_VideoDecoder* vid = tt->GetVideo())
		{
			LGL_Image* image=vid->GetImage();//EIGHT_WAY ? !preview : preview);
			if
			(
				image!=NULL &&
				image->GetFrameNumber()!=-1
			)
			{
				int projDisplay = LGL_Max(0,LGL_DisplayCount()-1);
				int projW;
				int projH;
				if(LGL_DisplayCount()==1)
				{
					projW = ViewportVisualsWidth * LGL_DisplayResolutionX();
					projH = ViewportVisualsHeight * LGL_DisplayResolutionY();
				}
				else
				{
					projW = LGL_DisplayResolutionX(projDisplay);
					projH = LGL_DisplayResolutionY(projDisplay);
				}
				float projAR = projW/(float)projH;
				float imageAR = image->GetWidth()/(float)image->GetHeight();
				if(LGL_DisplayCount()==1 && tt->GetAspectRatioMode()==0) projAR=imageAR;
				float targetAR = w*LGL_DisplayResolutionX()/(float)(h*LGL_DisplayResolutionY());

				float midX = 0.5f*(l+r);
				float midY = 0.5f*(b+t);

				float myL = l;
				float myR = r;
				float myB = b;
				float myT = t;

				if(tt->GetAspectRatioMode()==0)
				{
					//Respect AR

					//Fill as much width-wise as our AR says we should, possibly making it too wide. Span the height.

					myL = midX - 0.5f * w * (imageAR/targetAR);
					myR = midX + 0.5f * w * (imageAR/targetAR);
					myB = midY - 0.5f * h;
					myT = midY + 0.5f * h;

					//Make sure we're not too wide
					float targetLimitL = midX - 0.5f * w * (projAR/targetAR);
					float targetLimitR = midX + 0.5f * w * (projAR/targetAR);
					if(myL<targetLimitL)
					{
						float scaleFactor = (midX-targetLimitL)/(midX-myL);
						myB = midY - 0.5f * h * scaleFactor;
						myT = midY + 0.5f * h * scaleFactor;
						myL = targetLimitL;
						myR = targetLimitR;
					}

					//Make sure we're not too tall
					float targetLimitB = midY - 0.5f * h * (targetAR/projAR);
					float targetLimitT = midY + 0.5f * h * (targetAR/projAR);
					if(myB<targetLimitB)
					{
						float scaleFactor = (midY-targetLimitB)/(midY-myB);
						myL = midX - (midX-myL) * scaleFactor;
						myR = midX + (myR-midX) * scaleFactor;
						myB = targetLimitB;
						myT = targetLimitT;
					}
				}
				else if(tt->GetAspectRatioMode()==1)
				{
					//Fill (but respect projector AR)
					//Fill as much width-wise as our AR says we should, possibly making it too wide. Span the height.
					float targetLimitL = midX - 0.5f * w * (projAR/targetAR);
					float targetLimitR = midX + 0.5f * w * (projAR/targetAR);

					myL = targetLimitL;
					myR = targetLimitR;
					myB = b;
					myT = t;

					//Make sure we're not too wide
					if(myL<l)
					{
						float scaleFactor = (midX-l)/(midX-myL);
						myB = midY - 0.5f * h * scaleFactor;
						myT = midY + 0.5f * h * scaleFactor;
						myL = l;
						myR = r;
					}
				}
				else if(tt->GetAspectRatioMode()==2)
				{
					//Zebbler-tiling
					//Fill as much width-wise as our AR says we should, possibly making it too wide. Span the height.
					float targetLimitL = midX - 0.5f * w * (projAR/targetAR);
					float targetLimitR = midX + 0.5f * w * (projAR/targetAR);

					myL = targetLimitL;
					myR = targetLimitR;
					myB = b;
					myT = t;

					//Make sure we're not too wide
					if(myL<l)
					{
						float scaleFactor = (midX-l)/(midX-myL);
						myB = midY - 0.5f * h * scaleFactor;
						myT = midY + 0.5f * h * scaleFactor;
						myL = l;
						myR = r;
					}

					//At this point we're filling the whole screen, so...
					float myL13rd = myL + (1.0f/3.0f)*(myR-myL);
					float myL23rd = myL + (2.0f/3.0f)*(myR-myL);
					image->DrawToScreen
					(
						myL13rd,myL,
						myB,myT,
						0,
						videoBright,
						videoBright,
						videoBright,
						preview?1.0f:0.0f
					);
					image->DrawToScreen
					(
						myL13rd,myL23rd,
						myB,myT,
						0,
						videoBright,
						videoBright,
						videoBright,
						preview?1.0f:0.0f
					);
					image->DrawToScreen
					(
						myR,myL23rd,
						myB,myT,
						0,
						videoBright,
						videoBright,
						videoBright,
						preview?1.0f:0.0f
					);
				}

				if(tt->GetAspectRatioMode()!=2)
				{
					image->DrawToScreen
					(
						myL,myR,myB,myT,
						0,
						videoBright,
						videoBright,
						videoBright,
						preview?1.0f:0.0f
					);
				}

				if(vid->GetFPSMissed()>0)
				{
					VideoFPSDisplay=5.0f;
				}
				else
				{
					VideoFPSDisplay-=LGL_SecondsSinceLastFrame();
				}

				if
				(
					preview &&
					VideoFPSDisplay
				)
				{
					LGL_GetFont().DrawString
					(
						lOrig+0.05f*wOrig,tOrig-0.15f*hOrig,0.1f*hOrig,
						VideoFPSDisplay,VideoFPSDisplay,VideoFPSDisplay,VideoFPSDisplay,
						false,
						0.75f,
						"%i",
						(int)(ceilf(vid->GetFPS()))
					);

					float br=VideoFPSDisplay;
					LGL_GetFont().DrawString
					(
						lOrig+0.05f*wOrig,bOrig+0.05f*hOrig,0.1f*hOrig,
						br,br,br,br,
						false,
						0.75f,
						"%i",
						vid->GetFPSDisplayed()
					);

					if(vid->GetFPSMissed())
					{
						LGL_GetFont().DrawString
						(
							rOrig-0.3f*wOrig,bOrig+0.05f*hOrig,0.1f*hOrig,
							br,0,0,br,
							false,
							0.75f,
							"(%i)",
							vid->GetFPSMissed()
						);
					}
				}
			}

			ForceVideoToBackOfRandomQueue(vid->GetPathShort());
		}

		const bool videoReady = true;//vid->GetImageDecodedSinceVideoChange();
		float noiseFactorVideo = tt->GetNoiseFactorVideo();
		if(videoReady)
		{
			noiseFactorVideo=LGL_Max(0.0f,noiseFactorVideo-4.0f*LGL_SecondsSinceLastFrame());
		}
		else
		{
			noiseFactorVideo=1.0f;
		}
		tt->SetNoiseFactorVideo(noiseFactorVideo);

noiseFactorVideo=0.0f;
		if(noiseFactorVideo>0.0f)
		{
			int which = LGL_RandInt(0,NOISE_IMAGE_COUNT_128_128-1);
			NoiseImage[which]->DrawToScreen
			(
				l,r,b,t,
				0,
				noiseFactorVideo*videoBright,
				noiseFactorVideo*videoBright,
				noiseFactorVideo*videoBright,
				noiseFactorVideo*videoBright
			);
		}

		/*
		float secondsSinceVideoChange = Videos[videoNow]->GetSecondsSinceVideoChange();
		if(secondsSinceVideoChange<0.25f)
		{
			float whiteFactor=LGL_Max(0.0f,1.0f-4.0f*secondsSinceVideoChange);
			LGL_DrawRectToScreen
			(
				l,r,b,t,
				whiteFactor*bright,
				whiteFactor*bright,
				whiteFactor*bright,
				0.0f
			);
		}
		*/
	}
}

