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
#include <string.h>

LGL_Image* VisualizerObj::NoiseImage[NOISE_IMAGE_COUNT_128_128];

VisualizerObj::
VisualizerObj()
{
	NoSound=new LGL_Image("data/nosound.png");
	BlueScreenOfDeath=new LGL_Image("data/bsod.png");

	AccumulationNow=new LGL_Image(0.0f,0.5f,0.5f,1.0f);

	SetViewPortVisuals(0.0f,0.5f,0.5f,1.0f);

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

	ImageSetNextStatus=0;
	ImageSetLastPeak=0;
	ImageSetThreashold=.20;
	ImageSetPrefix[0]='\0';
	MovieClipPrefix[0]='\0';
	MovieClipScratchL=.5;
	MovieClipScratchR=.5;
	MovieClipLoadingStatus=0;
	for(int a=0;a<8;a++)
	{
		MovieClipSlideDown[a]=false;
		MovieClipQuad[a]=true;
	}
	MovieClipNum=4;
	for(int a=0;a<4;a++)
	{
		MovieClipNow[a]=0;
	}
	MovieClipSimultaneous=1;

	JoyAuxTimeLast=999;
	JoyAuxScratch=.5;
	JoyAuxScratchTimer=0;
	JoyAuxSlideXNum=1;
	JoyAuxSlideXMomentum=1;
	JoyAuxStrobeDelay=0;
	JoyAuxStrobeNow=false;

	DoNotLoadImages=true;
	MovieMode=false;

	VideoAvailable=false;

	for(int a=0;a<2;a++)
	{
		Videos[a]=NULL;
		NoiseFactor[a]=1.0f;
	}

	char tmp[2048];

	//Prepare Ambient Video Queue

	LGL_DirTree videoAmbientDirTree;
	if(LGL_DirectoryExists("data/video/ambient"))
	{
		videoAmbientDirTree.SetPath("data/video/ambient");
		VideoAvailable=true;
	}
	else if(LGL_DirectoryExists("data/video"))
	{
		videoAmbientDirTree.SetPath("data/video");
		VideoAvailable=true;
	}
	else
	{
		VideoAvailable=false;
	}
	videoAmbientDirTree.WaitOnWorkerThread();
	videoAmbientDirTree.SetFilterText("avi");
	strcpy(VideoAmbientPath,videoAmbientDirTree.GetPath());

	for(unsigned int a=0;a<videoAmbientDirTree.GetFilteredFileCount();a++)
	{
		sprintf(tmp,"%s",videoAmbientDirTree.GetFilteredFileName(a));
		char* str=new char[strlen(tmp)+1];
		strcpy(str,tmp);
		VideoAmbientQueue.push_back(str);
	}

	random_shuffle(VideoAmbientQueue.rbegin(),VideoAmbientQueue.rend());
	VideoAmbientGetCount=0;
	
	//Prepare Ambient Mellow Video Queue

	LGL_DirTree videoAmbientMellowDirTree;
	if(LGL_DirectoryExists("data/video/ambient/mellow"))
	{
		videoAmbientMellowDirTree.SetPath("data/video/ambient/mellow");
		VideoAvailable=true;
	}
	else if(LGL_DirectoryExists("data/video/ambient"))
	{
		videoAmbientMellowDirTree.SetPath("data/video/ambient");
		VideoAvailable=true;
	}
	else
	{
		VideoAvailable=false;
	}
	videoAmbientMellowDirTree.WaitOnWorkerThread();
	videoAmbientMellowDirTree.SetFilterText("mjpeg");
	strcpy(VideoAmbientMellowPath,videoAmbientMellowDirTree.GetPath());

	for(unsigned int a=0;a<videoAmbientMellowDirTree.GetFilteredFileCount();a++)
	{
		sprintf(tmp,"%s",videoAmbientMellowDirTree.GetFilteredFileName(a));
		char* str=new char[strlen(tmp)+1];
		strcpy(str,tmp);
		VideoAmbientMellowQueue.push_back(str);
	}

	random_shuffle(VideoAmbientMellowQueue.rbegin(),VideoAmbientMellowQueue.rend());
	VideoAmbientMellowGetCount=0;

	if(NoiseImage[0]==NULL)
	{
		for(int a=0;a<NOISE_IMAGE_COUNT_256_64;a++)
		{
			char path[1024];
			sprintf
			(
				path,
				"data/noise/128x128/%02i.png",
				a
			);
			LGL_Assertf(LGL_FileExists(path),("Noise file '%s' doesn't exist\n",path));
			NoiseImage[a] = new LGL_Image(path);
		}
	}
}

VisualizerObj::
~VisualizerObj()
{
	delete	NoSound;
	delete	BlueScreenOfDeath;
	delete	AccumulationNow;
	//TODO: Take care of scroll text buffers
}

void
VisualizerObj::
NextFrame
(
	float	secondsElapsed
)
{
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
#if 0
	if(!DoNotLoadImages)
	{
		if
		(
			(
				LGL_JoyDown(0,LGL_JOY_START) &&
				LGL_JoyStroke(0,LGL_JOY_SELECT)
			) ||
			ImageSetTimer.SecondsSinceLastReset()>ImageSet.size() ||
		 	LGL_KeyStroke(SDLK_i)
			/*
		 	LGL_KeyStroke(SDLK_i) ||
			ImageSet.size()==0
			*/
		)
		{
			LoadNewImageSet();
		}

		int empty=-1;
		for(int a=0;a<MovieClipNum;a++)
		{
			if(MovieClipList[a].empty())
			{
				empty=a;
				break;
			}
		}
		int oldestIndex=-1;
		float oldestTime=-1;
		for(int a=0;a<MovieClipNum;a++)
		{
			if(MovieClipTimer[a].SecondsSinceLastReset()>oldestTime)
			{
				oldestIndex=a;
				oldestTime=MovieClipTimer[a].SecondsSinceLastReset();
			}
		}

		if
		(
			(
				LGL_JoyDown(0,LGL_JOY_START) &&
				LGL_JoyStroke(0,LGL_JOY_SELECT)
			) ||
			MovieClipTimerGlobal.SecondsSinceLastReset()>10 ||
			empty!=-1 ||
		 	LGL_KeyStroke(SDLK_m)
			/*
		 	LGL_KeyStroke(SDLK_m) ||
			MovieClip.size()==0
			*/
		)
		{
			LoadNewMovieClip();
		}
	}

	if(LGL_JoyStroke(0,LGL_JOY_TRIANGLE))
	{
		MovieClipGlitch=.5+.35*(LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS));
		if(MovieClipGlitch>.9)
		{
			MovieClipGlitch=.9;
		}
	}
	if
	(
		(
			LGL_AudioPeakLeft()>=ImageSetThreashold &&
			LGL_RandFloat()<.1 &&
			JoyAuxTimeLast>2
		) ||
		LGL_JoyAnalogueStatus(1,LGL_JOY_ANALOGUE_L,LGL_JOY_YAXIS)==-1
	)
	{
		if(LGL_RandInt(0,3)==0)
		{
			for(int a=0;a<4;a++)
			{
				MovieClipNow[a]=PickRandomValidMovieClip();
			}
			MovieClipSimultaneous=LGL_RandInt(1,4);
			if(MovieClipSimultaneous==3) MovieClipSimultaneous=1;
		}
		if(MovieClipSlideDown[MovieClipNow[0]]==false)
		{
			MovieClipSlideDown[MovieClipNow[0]]=true;
			MovieClipSlideDownNum[MovieClipNow[0]]=(int)floor(pow(2,LGL_RandFloat(3,6)));
			MovieClipSlideDownTimeNow[MovieClipNow[0]]=0;
			MovieClipSlideDownTimeMax[MovieClipNow[0]]=LGL_RandFloat(.25,2);
			for(int a=0;a<MovieClipSlideDownNum[MovieClipNow[0]];a++)
			{
				MovieClipSlideDownNow[MovieClipNow[0]][a]=0;
				MovieClipSlideDownDelta[MovieClipNow[0]][a]=-LGL_RandFloat(0,.15);
			}
		}
	}
	if(MovieClipSlideDown[MovieClipNow[0]])
	{
		for(int a=0;a<MovieClipSlideDownNum[MovieClipNow[0]];a++)
		{
			MovieClipSlideDownNow[MovieClipNow[0]][a]+=
				MovieClipSlideDownDelta[MovieClipNow[0]][a]*
				LGL_SecondsSinceLastFrame();
		}
		MovieClipSlideDownTimeNow[MovieClipNow[0]]+=LGL_SecondsSinceLastFrame();
		if
		(
			(
				MovieClipSlideDownTimeNow[MovieClipNow[0]] >=
				MovieClipSlideDownTimeMax[MovieClipNow[0]]
			) &&
			LGL_AudioPeakLeft()>=ImageSetThreashold
		)
		{
			if(fabs(MovieClipSlideDownDelta[MovieClipNow[0]][0]<.05))
			{
				MovieClipSlideDown[MovieClipNow[0]]=false;
			}
			else
			{
				for(int a=0;a<MovieClipSlideDownNum[MovieClipNow[0]];a++)
				{
					MovieClipSlideDownDelta[MovieClipNow[0]][a]*=-1;
				}
				MovieClipSlideDownTimeNow[MovieClipNow[0]]=0;
			}
		}
	}

	float alpha1=1.0-7.5*LGL_SecondsSinceLastFrame();
	if(alpha1<0) alpha1=0;
	float alpha2=1.0-alpha1;

	//120b/m=2b/s=.5 s/b
	//180b/m=3b/s=.333 s/b
	//This is set for DnB tempo. I know a way to make it variable / autodetect.
	//I must code this later.

	ImageSetLastPeak=LGL_Clamp(0,ImageSetLastPeak-1.5*LGL_SecondsSinceLastFrame()*.333,1);
	ImageSetThreashold=LGL_Interpolate
	(
		.01,
		ImageSetThreashold,
		1.0-LGL_Clamp(0,.5*LGL_SecondsSinceLastFrame(),1)
	);

	if(ImageSetNextStatus==2)
	{
		for(unsigned int a=0;a<ImageSet.size();a++)
		{
			LGL_Image* baka=ImageSet[a];
			delete(baka);
		}
		ImageSet.clear();
		ImageSet=ImageSetNext;
		ImageSetNext.clear();
		ImageSetNextStatus=0;
	}

	MovieClipScratchL=
		alpha1*MovieClipScratchL+
		alpha2*.5*(LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS)+1);
	MovieClipScratchR=
		alpha1*MovieClipScratchR+
		alpha2*.5*(LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_R,LGL_JOY_XAXIS)+1);

	if(MovieClipLoadingStatus==2)
	{
		//Our next loading movie has finished! Put it into the best spot.
		int victim=0;
		float victimAge=0;
		for(int a=0;a<MovieClipNum;a++)
		{
			if(MovieClipTimer[a].SecondsSinceLastReset()>victimAge)
			{
				victim=a;
				victimAge=MovieClipTimer[a].SecondsSinceLastReset();
			}
			if(MovieClipList[a].empty())
			{
				victim=a;
				victimAge=999999;
				break;
			}
		}
		for(unsigned int a=0;a<MovieClipList[victim].size();a++)
		{
			LGL_Image* baka=MovieClipList[victim][a];
			delete(baka);
		}
		MovieClipList[victim].clear();
		MovieClipList[victim]=MovieClipLoading;
		MovieClipLoading.clear();
	
		MovieClipTimer[victim].Reset();
		MovieClipTimerGlobal.Reset();
		float AlphaMin=.1;
		float AlphaMax=1;
		MovieClipQuad[victim]=LGL_RandInt(0,1);

		//Apply hints

		char HintPath[512];
		sprintf(HintPath,"%s/.hints",MovieClipLoadingDir);
		FILE* HintFile=fopen(HintPath,"r");
		if(HintFile)
		{
			char tempstring[256];
			while(!feof(HintFile))
			{
				fgets(tempstring,255,HintFile);
				if(tempstring[0]=='$')
				{
					if(strchr(tempstring,'='))
					{
						char *tempstring2=strchr(tempstring,'=');
						char value[256];
						strcpy(value,&(tempstring2[1]));
						char argument[256];
						tempstring2[0]='\0';
						strcpy(argument,tempstring);
						if(value[strlen(value)-1]=='\n')
						{
							value[strlen(value)-1]='\0';
						}
						//int valueint=atoi(value);
						float valuefloat=atof(value);

						if(strcmp(argument,"$AlphaMin")==0)
						{
							AlphaMin=valuefloat;
						}
						if(strcmp(argument,"$AlphaMax")==0)
						{
							AlphaMax=valuefloat;
						}
						if(0 && strcmp(argument,"$QuadProbability")==0)
						{
							MovieClipQuad[victim]=
								(LGL_RandFloat(0,1)<valuefloat);
						}
					}
				}
			}
			fclose(HintFile);
		}

		MovieClipAlpha[victim]=LGL_RandFloat(AlphaMin,AlphaMax);

		MovieClipLoadingStatus=0;
	}

	//JoyAux Stuff

	JoyAuxTimeLast+=LGL_SecondsSinceLastFrame();

	if
	(
		LGL_JoyAnalogueStatus(1,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS)==-1
	)
	{
		//DDR Pad MovieClip Scratching: Left

		JoyAuxScratch=JoyAuxScratch-LGL_SecondsSinceLastFrame();
		while(JoyAuxScratch<0)
		{
			JoyAuxScratch=0;
		}

		JoyAuxScratchTimer=1;
		JoyAuxTimeLast=0;
		MovieClipSimultaneous=1;
		MovieClipQuad[MovieClipNow[0]]=0;
	}
	else if
	(
		LGL_JoyAnalogueStatus(1,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS)==1
	)
	{
		//DDR Pad MovieClip Scratching: Right

		JoyAuxScratch=JoyAuxScratch+LGL_SecondsSinceLastFrame();
		while(JoyAuxScratch>1)
		{
			JoyAuxScratch=1;
		}

		JoyAuxScratchTimer=1;
		JoyAuxTimeLast=0;
	}
	else
	{
		if(!(LGL_JoyAnalogueStatus(1,LGL_JOY_ANALOGUE_L,LGL_JOY_YAXIS)==1))
		{
			JoyAuxScratch=
				alpha1*JoyAuxScratch+
				alpha2*.5;
		}
		JoyAuxScratchTimer-=LGL_SecondsSinceLastFrame();
		if(JoyAuxScratchTimer<0) JoyAuxScratchTimer=0;
	}
	
	if
	(
		LGL_JoyAnalogueStatus(1,LGL_JOY_ANALOGUE_L,LGL_JOY_YAXIS)==1
	)
	{
		//DDR Pad MovieClip SlideX
		if(JoyAuxSlideXNow==0)
		{
			JoyAuxSlideXNum=(int)floor(pow(2,LGL_RandFloat(3,6)));
			for(int a=0;a<JoyAuxSlideXNum;a++)
			{
				JoyAuxSlideXDelta[a]=.1+LGL_RandFloat(0,.65);
				if(a>0 && LGL_RandInt(0,3)==3)
				{
					JoyAuxSlideXDelta[a]=JoyAuxSlideXDelta[a-1];
				}
			}
			JoyAuxSlideXMomentum=1;
		}
		JoyAuxScratchTimer=1;
		JoyAuxSlideXNow=LGL_Clamp
		(
			0,
			JoyAuxSlideXNow+LGL_SecondsSinceLastFrame()*JoyAuxSlideXMomentum,
			1
		);
		if(JoyAuxSlideXNow==0 || JoyAuxSlideXNow==1)
		{
			JoyAuxSlideXMomentum*=-1;
		}
		JoyAuxTimeLast=0;

		if(!(LGL_JoyAnalogueStatus(1,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS)==-1))
		{
			JoyAuxScratch=JoyAuxScratch+.25*LGL_SecondsSinceLastFrame();
			while(JoyAuxScratch>1)
			{
				JoyAuxScratch--;
			}
		}
	}
	else
	{
		JoyAuxSlideXNow=LGL_Max(0,JoyAuxSlideXNow-LGL_SecondsSinceLastFrame());
	}

	JoyAuxStrobeNow=false;
	JoyAuxStrobeDelay=(int)LGL_Max(0,JoyAuxStrobeDelay-1);
	if(LGL_JoyDown(1,LGL_JOY_CIRCLE) && JoyAuxStrobeDelay==0)
	{
		JoyAuxStrobeNow=true;
		JoyAuxStrobeDelay=2;
	}
	if(LGL_JoyDown(1,LGL_JOY_TRIANGLE) && JoyAuxStrobeDelay==0)
	{
		JoyAuxStrobeNow=true;
		JoyAuxStrobeDelay=4;
	}
#endif	//0
}

void
VisualizerObj::
DrawVisuals
(
	bool	visualizerQuadrent,
	float	visualizerZoomOutPercent
)
{
	float l=ViewPortVisualsLeft;
	float r=ViewPortVisualsRight;
	float b=ViewPortVisualsBottom;
	float t=ViewPortVisualsTop;
	float w=ViewPortVisualsWidth;
	float h=ViewPortVisualsHeight;

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
		//float s=2.0f*LGL_SecondsSinceLastFrame();
		//float y=2.0-(1+0);//LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_YAXIS));
		
		/*
		AccumulationNow->DrawToScreen
		(
			l-y*s*w,	r+y*s*w,
			b-y*s*h,	t+y*s*h,
			2*s*0.0f,//LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS),
			1.0-s,1.0-s,1.0-s,0.0f
		);
		*/

		/*
		LGL_DrawLineToScreen
		(
			l,b+.5*w+.5*w*LGL_AudioPeakLeft(),
			r,b+.5*w+.5*w*LGL_AudioPeakLeft(),
			.4,.2,1,1,
			1,
			false
		);
		LGL_DrawLineToScreen
		(
			l,b+.5*w-.5*w*LGL_AudioPeakLeft(),
			r,b+.5*w-.5*w*LGL_AudioPeakLeft(),
			.4,.2,1,1,
			1,
			false
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
			NoSound->DrawToScreen
			(
				l,r,
				b,t,
				0,
				1,1,1,1
			);
		}
	}
	//return;

	//Draw Image Stuff
	
	if
	(
		ImageSet.empty()==false &&
		ImageSetLastPeak<LGL_AudioPeakLeft() &&
		LGL_AudioPeakLeft()>=ImageSetThreashold
	)
	{
		ImageSetLastPeak=LGL_AudioPeakLeft()+1.0/9.0;
		ImageSetThreashold=LGL_Interpolate(ImageSetThreashold,LGL_AudioPeakLeft()*.8,.1);

		/*
		ImageSetLastWhich=LGL_RandInt(0,ImageSet.size()-1);
		ImageSet[ImageSetLastWhich]->DrawToScreen
		(
			l,r,
			b,t,
			0,
			1,1,1,1
		);
		*/

		MovieMode=(LGL_RandInt(0,1)==0);
		if(LGL_RandFloat()<.3)
		{
			MovieClipSlideDown[MovieClipNow[0]]=false;
		}
	}

	//Draw Movie Stuff

	if
	(
		(
			MovieMode &&
			MovieClipList[MovieClipNow[0]].empty()==false
		) ||
		(
			MovieClipList[MovieClipNow[0]].empty()==false && false
		)
	)
	{
		int which=GetClipImageIndexNow(MovieClipNow[0]);

		if(MovieClipQuad[MovieClipNow[0]])
		{
			if(MovieClipSlideDown[MovieClipNow[0]])
			{
				for(int z=0;z<4;z++)
				{
					float L=l;
					float R=r;
					float B=b;
					float T=t;
					if(z==0)
					{
						L=l;
						R=l+.5*w;
						B=b+.5*h;
						T=t;
					}
					else if(z==1)
					{
						L=r;
						R=l+.5*w;
						B=b+.5*h;
						T=t;
					}
					else if(z==2)
					{
						L=l;
						R=l+.5*w;
						B=b+.5*h;
						T=b;
					}
					else if(z==3)
					{
						L=r;
						R=l+.5*w;
						B=b+.5*h;
						T=b;
					}
					float dx=R-L;
					//float dy=T-B;

					for(int a=0;a<MovieClipSlideDownNum[MovieClipNow[0]];a++)
					{
						float left=L+(a/(float)MovieClipSlideDownNum[MovieClipNow[0]])*dx;
						float right=L+((a+1)/(float)MovieClipSlideDownNum[MovieClipNow[0]])*dx;
						float bottom=B-MovieClipSlideDownNow[MovieClipNow[0]][a];
						float top=T-MovieClipSlideDownNow[MovieClipNow[0]][a];
						if(z>=2)
						{
							bottom=B+MovieClipSlideDownNow[MovieClipNow[0]][a];
							top=T+MovieClipSlideDownNow[MovieClipNow[0]][a];
						}
						
						int current=0;
						if(MovieClipSimultaneous==2)
						{
							if(z>=2)
							{
								if(MovieClipList[MovieClipNow[1]].empty()==false)
								{
									current=1;
								}
							}
						}
						if(MovieClipSimultaneous==4)
						{
							if(MovieClipList[MovieClipNow[z]].empty()==false)
							{
								current=z;
							}
						}
						which=GetClipImageIndexNow(MovieClipNow[current]);

						MovieClipList[MovieClipNow[current]][which]->DrawToScreen
						(
							left,right,
							bottom,top,
							0,
							1,1,1,MovieClipAlpha[MovieClipNow[0]],
							false,false,0,0,0,
							(a+0)/(float)MovieClipSlideDownNum[MovieClipNow[0]],
							(a+1)/(float)MovieClipSlideDownNum[MovieClipNow[0]],
							0,1
						);	
					}
				}
			}
			else
			{
				//Upper Left
				MovieClipList[MovieClipNow[0]][which]->DrawToScreen
				(
					l,l+.5*w,
					b+.5*h,t,
					0,
					1,1,1,MovieClipAlpha[MovieClipNow[0]]
				);
				//Upper Right
				MovieClipList[MovieClipNow[0]][which]->DrawToScreen
				(
					r,l+.5*w,
					b+.5*h,t,
					0,
					1,1,1,MovieClipAlpha[MovieClipNow[0]]
				);
				//Lower Left
				MovieClipList[MovieClipNow[0]][which]->DrawToScreen
				(
					l,l+.5*w,
					b+.5*h,b,
					0,
					1,1,1,MovieClipAlpha[MovieClipNow[0]]
				);
				//Lower Right
				MovieClipList[MovieClipNow[0]][which]->DrawToScreen
				(
					r,l+.5*w,
					b+.5*h,b,
					0,
					1,1,1,MovieClipAlpha[MovieClipNow[0]]
				);
			}
		}
		else
		{
			if(MovieClipSlideDown[MovieClipNow[0]])
			{
				for(int a=0;a<MovieClipSlideDownNum[MovieClipNow[0]];a++)
				{
					float dx=r-l;
					//float dy=t-b;
					float left=l+(a/(float)MovieClipSlideDownNum[MovieClipNow[0]])*dx;
					float right=l+((a+1)/(float)MovieClipSlideDownNum[MovieClipNow[0]])*dx;
					float bottom=b-MovieClipSlideDownNow[MovieClipNow[0]][a];
					float top=t-MovieClipSlideDownNow[MovieClipNow[0]][a];
					MovieClipList[MovieClipNow[0]][which]->DrawToScreen
					(
						left,right,
						bottom,top,
						0,
						1,1,1,MovieClipAlpha[MovieClipNow[0]],
						false,false,0,0,0,
						(a+0)/(float)MovieClipSlideDownNum[MovieClipNow[0]],
						(a+1)/(float)MovieClipSlideDownNum[MovieClipNow[0]],
						0,1
					);	
				}
			}
			else
			{
				MovieClipList[MovieClipNow[0]][which]->DrawToScreen
				(
					l,r,
					b,t,
					0,
					1,1,1,MovieClipAlpha[MovieClipNow[0]]
				);
			}
		}
	}

	//JoyAux Stuff
	
	if(JoyAuxScratchTimer>0 && MovieClipList[MovieClipNow[0]].empty()==false)
	{
		for(int a=0;a<JoyAuxSlideXNum;a++)
		{
			float dx=r-l;
			//float dy=t-b;
			float left=l+(a/(float)JoyAuxSlideXNum)*dx;
			float right=l+((a+1)/(float)JoyAuxSlideXNum)*dx;
			float bottom=b-JoyAuxSlideXDelta[a]*JoyAuxSlideXNow;
			float top=t-JoyAuxSlideXDelta[a]*JoyAuxSlideXNow;

			int which=(int)floor(JoyAuxScratch*(MovieClipList[MovieClipNow[0]].size()-1));
			MovieClipList[MovieClipNow[0]][which]->DrawToScreen
			(
				left,right,
				bottom,top,
				0,
				JoyAuxScratchTimer,JoyAuxScratchTimer,JoyAuxScratchTimer,sqrt(JoyAuxScratchTimer),
				false,false,0,0,0,
				(a+0)/(float)JoyAuxSlideXNum,
				(a+1)/(float)JoyAuxSlideXNum,
				0,1
			);	
		}
	}

	//The ACTUAL videos we're drawing.
	for(int videoNow=0;videoNow<2;videoNow++)
	{
		if(Videos[videoNow])
		{
			LGL_Image* image=Videos[videoNow]->LockImage();
			if(image!=NULL)
			{
				image->DrawToScreen
				(
					l,r,b,t,
					0,
					VideoBrightness[videoNow],
					VideoBrightness[videoNow],
					VideoBrightness[videoNow],
					0.0f
				);
			}
			Videos[videoNow]->UnlockImage(image);

			bool videoReady = Videos[videoNow]->GetImageDecodedSinceVideoChange();
			if(videoReady)
			{
				NoiseFactor[videoNow]=LGL_Max(0.0f,NoiseFactor[videoNow]-4.0f*LGL_SecondsSinceLastFrame());
			}
			else
			{
				NoiseFactor[videoNow]=1.0f;
			}

			if(NoiseFactor[videoNow]>0.0f)
			{
				int which = LGL_RandInt(0,NOISE_IMAGE_COUNT_128_128-1);
				NoiseImage[which]->DrawToScreen
				(
					l,r,b,t,
					0,
					NoiseFactor[videoNow]*VideoBrightness[videoNow],
					NoiseFactor[videoNow]*VideoBrightness[videoNow],
					NoiseFactor[videoNow]*VideoBrightness[videoNow],
					0.0f
				);
			}

			float secondsSinceVideoChange = Videos[videoNow]->GetSecondsSinceVideoChange();
			if(secondsSinceVideoChange<0.25f)
			{
				float whiteFactor=LGL_Max(0.0f,1.0f-4.0f*secondsSinceVideoChange);
				LGL_DrawRectToScreen
				(
					l,r,b,t,
					whiteFactor*VideoBrightness[videoNow],
					whiteFactor*VideoBrightness[videoNow],
					whiteFactor*VideoBrightness[videoNow],
					0.0f
				);
			}

			ForceVideoToBackOfAmbientQueue(Videos[videoNow]->GetPathShort());
		}
	}

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

	float l=ViewPortStatusLeft;
	float r=ViewPortStatusRight;
	float b=ViewPortStatusBottom;
	//float t=ViewPortStatusTop;
	float w=ViewPortStatusWidth;
	float h=ViewPortStatusHeight;
	
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
SetViewPortVisuals
(
	float	left,	float	right,
	float	bottom,	float	top
)
{
	ViewPortVisualsLeft=left;
	ViewPortVisualsRight=right;
	ViewPortVisualsBottom=bottom;
	ViewPortVisualsTop=top;
	ViewPortVisualsWidth=right-left;
	ViewPortVisualsHeight=top-bottom;

	AccumulationNow->FrameBufferViewPort(left,right,bottom,top);
}

void
VisualizerObj::
ToggleFullScreen()
{
	FullScreen=!FullScreen;
	if(FullScreen)
	{
		AccumulationNow->FrameBufferViewPort
		(
			0,1,
			0,1
		);
	}
	else
	{
		AccumulationNow->FrameBufferViewPort
		(
			ViewPortVisualsLeft,	ViewPortVisualsRight,
			ViewPortVisualsBottom,	ViewPortVisualsTop
		);
	}
}

void
VisualizerObj::
SetImageSetPrefix
(
	char*	prefix
)
{
	sprintf(ImageSetPrefix,"%s",prefix);
}

void
VisualizerObj::
SetMovieClipPrefix
(
	char*	prefix
)
{
	sprintf(MovieClipPrefix,"%s",prefix);
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
SetVideos
(
	LGL_Video*	video0,
	LGL_Video*	video1,
	float		videoBrightness0,
	float		videoBrightness1
)
{
	Videos[0]=video0;
	Videos[1]=video1;
	VideoBrightness[0]=videoBrightness0;
	VideoBrightness[1]=videoBrightness1;
}

LGL_Video*
VisualizerObj::
GetVideo
(
	int	which
)
{
	return(Videos[which]);
}

void
VisualizerObj::
GetNextVideoPathAmbient(char* path)
{
	if(VideoAmbientQueue.empty())
	{
		path[0]='\0';
		return;
	}

	sprintf(path,"%s/%s",VideoAmbientPath,VideoAmbientQueue[0]);
	char* str=VideoAmbientQueue[0];
	VideoAmbientQueue.erase((std::vector<char*>::iterator)(&(VideoAmbientQueue[0])));
	VideoAmbientQueue.push_back(str);

	VideoAmbientGetCount++;
	if(VideoAmbientGetCount==VideoAmbientQueue.size())
	{
		random_shuffle(VideoAmbientQueue.rbegin(),VideoAmbientQueue.rend());
		VideoAmbientGetCount=0;
	}
}

void
VisualizerObj::
GetNextVideoPathAmbientMellow(char* path)
{
	if(VideoAmbientMellowQueue.empty())
	{
		path[0]='\0';
		return;
	}

	sprintf(path,"%s/%s",VideoAmbientMellowPath,VideoAmbientMellowQueue[0]);
	char* str=VideoAmbientMellowQueue[0];
	VideoAmbientMellowQueue.erase((std::vector<char*>::iterator)(&(VideoAmbientMellowQueue[0])));
	VideoAmbientMellowQueue.push_back(str);

	VideoAmbientMellowGetCount++;
	if(VideoAmbientMellowGetCount==VideoAmbientMellowQueue.size())
	{
		random_shuffle(VideoAmbientMellowQueue.rbegin(),VideoAmbientMellowQueue.rend());
		VideoAmbientMellowGetCount=0;
	}
}

//Privates

int
ImageSetLoader
(
	void*	object
)
{
	VisualizerObj* Viz=(VisualizerObj*)object;
	Viz->LoadNewImageSetThread();
	return(0);
}

int
MovieClipLoader
(
	void*	object
)
{
	VisualizerObj* Viz=(VisualizerObj*)object;
	Viz->LoadNewMovieClipThread();
	return(0);
}

void
VisualizerObj::
LoadNewImageSet()
{
	if(ImageSetNextStatus!=0 || MovieClipLoadingStatus!=0)
	{
		return;
	}
	ImageSetNextStatus=1;
	LGL_ThreadCreate(ImageSetLoader,this);
}

void
VisualizerObj::
LoadNewMovieClip()
{
	if(ImageSetNextStatus!=0 || MovieClipLoadingStatus!=0)
	{
		return;
	}
	MovieClipLoadingStatus=1;
	LGL_ThreadCreate(MovieClipLoader,this);
}

void
VisualizerObj::
LoadNewImageSetThread()
{
	ImageSetNext.clear();
	std::vector<char*> dir=LGL_DirectoryListCreate("data/ImageSets",false,true);

	std::vector<int> acceptableList;
	if(strlen(ImageSetPrefix)>0)
	{
		for(unsigned int x=0;x<dir.size();x++)
		{
			char baka[1024];
			sprintf(baka,"%s",dir[x]);
			baka[strlen(ImageSetPrefix)]='\0';
			if(strcasecmp(baka,ImageSetPrefix)==0)
			{
				acceptableList.push_back(x);
			}
		}
	}

	char myDir[1024];
	if(acceptableList.size()>0)
	{
		int which=LGL_RandInt(0,acceptableList.size()-1);
		sprintf(myDir,"data/ImageSets/%s",dir[acceptableList[which]]);
		strcpy(ImageSetName,dir[acceptableList[which]]);
		LGL_DirectoryListDelete(dir);
	}
	else
	{
		LGL_DirectoryListDelete(dir);
		dir=LGL_DirectoryListCreate("data/ImageSets",false,false);
		int which=LGL_RandInt(0,dir.size()-1);
		sprintf(myDir,"data/ImageSets/%s",dir[which]);
		strcpy(ImageSetName,dir[which]);
		LGL_DirectoryListDelete(dir);
	}

	dir=LGL_DirectoryListCreate(myDir);
	for(unsigned int a=0;a<dir.size();a++)
	{
		LGL_Image* NewImage;
		char path[1024];
		sprintf(path,"%s/%s",myDir,dir[a]);
		NewImage=new LGL_Image(path,true,false);
		ImageSetNext.push_back(NewImage);
	}
	LGL_DirectoryListDelete(dir);

	ImageSetTimer.Reset();
	ImageSetLastWhich=0;
	ImageSetNextStatus=2;
}

void
VisualizerObj::
LoadNewMovieClipThread()
{
	if(MovieClipLoading.size()!=0)
	{
		printf("Visualizer.cpp::LoadMovieClipThread(): Warning! Memory leak!\n");
	}

	MovieClipLoading.clear();
	std::vector<char*> dir=LGL_DirectoryListCreate("data/MovieClips",false,true);
	
	std::vector<int> acceptableList;
	if(strlen(MovieClipPrefix)>0)
	{
		for(unsigned int x=0;x<dir.size();x++)
		{
			char baka[1024];
			sprintf(baka,"%s",dir[x]);
			baka[strlen(MovieClipPrefix)]='\0';
			if(strcasecmp(baka,MovieClipPrefix)==0)
			{
				acceptableList.push_back(x);
			}
		}
	}

	char myDir[1024];
	if(acceptableList.size()>0)
	{
		int which=LGL_RandInt(0,acceptableList.size()-1);
		sprintf(myDir,"data/MovieClips/%s",dir[acceptableList[which]]);
		LGL_DirectoryListDelete(dir);
	}
	else
	{
		LGL_DirectoryListDelete(dir);
		dir=LGL_DirectoryListCreate("data/MovieClips",false,false);
		if(dir.size()<=0)
		{
			printf("Visualizer.cpp:LoadNewMovieClipThread(): Error! No Movie Clips!\n");
			exit(0);
		}
		int which=LGL_RandInt(0,dir.size()-1);
		sprintf(myDir,"data/MovieClips/%s",dir[which]);
		LGL_DirectoryListDelete(dir);
	}
	
	dir=LGL_DirectoryListCreate(myDir);
	
	for(unsigned int a=0;a<dir.size();a++)
	{
		LGL_Image* NewImage;
		char path[1024];
		sprintf(path,"%s/%s",myDir,dir[a]);
		NewImage=new LGL_Image(path,true,false);
		MovieClipLoading.push_back(NewImage);
	}
	LGL_DirectoryListDelete(dir);

	MovieClipLoadingStatus=2;

	sprintf(MovieClipLoadingDir,"%s",myDir);
}

int
VisualizerObj::
PickRandomValidMovieClip()
{
	bool none=true;
	for(int a=0;a<MovieClipNum;a++)
	{
		if(MovieClipList[a].empty()==false)
		{
			none=false;
		}
	}
	if(none)
	{
		return(0);
	}
	else
	{
		for(int a=0;a<100;a++)
		{
			int guess=LGL_RandInt(0,MovieClipNum-1);
			if(MovieClipList[guess].empty()==false)
			{
				return(guess);
			}
		}
		return(0);
	}
}

int VisualizerObj::
GetClipImageIndexNow
(
	int	whichClipInList
)
{
	int w=whichClipInList;
	if(MovieClipList[w].empty())
	{
		printf("Visualizer.cpp::GetClipImageIndexNow(%i): Error! Clip is Empty!\n",w);
		exit(0);
	}

	int which=
		((int)(LGL_SecondsSinceExecution()*30))%
		MovieClipList[w].size();
	/*
	if
	(
		LGL_JoyDown(0,LGL_JOY_TRIANGLE) ||
		LGL_JoyDown(0,LGL_JOY_CROSS) ||
		LGL_JoyDown(0,LGL_JOY_CIRCLE)
	)
	{
		which=(int)floor
		(
			MovieClipScratchL*(MovieClipList[w].size()-1)
		);
		if(LGL_JoyDown(0,LGL_JOY_TRIANGLE))
		{
			float x=LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS);
			float y=LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_YAXIS);
			x=.5+.5*x;
			y=.5+.35*y;
			float pos=
				.25*(
					(15*LGL_SecondsSinceExecution()*x)-
					floor(15*LGL_SecondsSinceExecution()*x)
				);
			if(pos>y)
			{
				pos=y-pos;
			}
			if(pos<0)
			{
				pos=0;
			}
			float scratch=1+2*(MovieClipGlitch-1);
			scratch=.5+.4*scratch;

			which=
				(int)floor((MovieClipList[w].size()-1)*
				(scratch+pos));
			
			if
			(
				which>0 &&
				(unsigned int)which>MovieClipList[w].size()-1
			)
			{
				which-=MovieClipList[w].size()-1;
			}
			while(which<0)
			{
				which+=MovieClipList[w].size()-1;
			}
		}
	}
	*/
	return(which);
}

void
VisualizerObj::
ForceVideoToBackOfAmbientQueue
(
	const
	char*	pathShort
)
{
	if(pathShort==NULL)
	{
		return;
	}

	//If we're drawing a video to the screen, then we want to put it at the end of the ambient list.
	if
	(
		VideoAmbientQueue.size()>1 &&
		strcmp(pathShort,VideoAmbientQueue[VideoAmbientQueue.size()-1])!=0
	)
	{
		for(int a=VideoAmbientQueue.size()-1;a>=0;a--)
		{
			if(strcmp(pathShort,VideoAmbientQueue[a])==0)
			{
				char* tmp=VideoAmbientQueue[a];
				VideoAmbientQueue.erase((std::vector<char*>::iterator)(&(VideoAmbientQueue[a])));
				VideoAmbientQueue.push_back(tmp);
				break;
			}
		}
	}
	
	//If we're drawing a video to the screen, then we want to put it at the end of the ambient mellow list.
	if
	(
		VideoAmbientMellowQueue.size()>1 &&
		strcmp(pathShort,VideoAmbientMellowQueue[VideoAmbientMellowQueue.size()-1])!=0
	)
	{
		for(int a=VideoAmbientMellowQueue.size()-1;a>=0;a--)
		{
			if(strcmp(pathShort,VideoAmbientMellowQueue[a])==0)
			{
				char* tmp=VideoAmbientMellowQueue[a];
				VideoAmbientMellowQueue.erase((std::vector<char*>::iterator)(&(VideoAmbientMellowQueue[a])));
				VideoAmbientMellowQueue.push_back(tmp);
				break;
			}
		}
	}
}

void
VisualizerObj::
PopulateCharStarBufferWithScrollTextFile
(
	std::vector<char*>&	buffer,
	const
	char*			path
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

