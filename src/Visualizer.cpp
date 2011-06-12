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

VisualizerObj* visualizer=NULL;

VisualizerObj*
GetVisualizer()
{
	if(visualizer==NULL)
	{
		visualizer=new VisualizerObj;
	}

	return(visualizer);
}

VisualizerObj::
VisualizerObj()
{
	BlueScreenOfDeath=new LGL_Image("data/image/bsod.png");

	float resX=LGL_DisplayResolutionX(0);
	float center = resX/2.0f;
	float left=LGL_Max(0.0f,(center-0.5f*resX)/resX);
	float right=LGL_Min(1.0f,(center+0.5f*resX)/resX);
	//float right=LGL_Min(1.0f,GetProjectorQuadrentResX()/(float)LGL_DisplayResolutionX(0));
	float bottom=LGL_Max(0.5f,1.0f-GetProjectorQuadrentResY()/(float)LGL_DisplayResolutionY(0));
	float top=1.0f;
	Accumulation=new LGL_Image(left,right,bottom,top);

	if(LGL_DisplayCount()>1)
	{
		float quadrentSplitY = 0.5f;
		float projAR = LGL_DisplayResolutionX(1)/(float)LGL_DisplayResolutionY(1);
		float targetAR = 1.0f*LGL_DisplayResolutionX(0)/(float)((1.0f-quadrentSplitY)*LGL_DisplayResolutionY(0));

		left = 0.5f - (0.5f * (projAR/targetAR));
		right = 0.5f + (0.5f * (projAR/targetAR));
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

	std::vector<LEDClient> inLEDClientList = GetLEDClientList();
	LEDGroupCount=0;
	for(unsigned int a=0;a<inLEDClientList.size();a++)
	{
		char host[2048];
		inLEDClientList[a].Endpoint.AddressAsString(host);	//FIXME: Causes a hang if no DNS server...
		inLEDClientList[a].Group = LGL_Clamp(0,inLEDClientList[a].Group,LED_GROUP_MAX-1);
		LEDClientList.push_back
		(
			new LGL_LEDClient
			(
				host,
				inLEDClientList[a].Endpoint.port,
				inLEDClientList[a].Channel,
				inLEDClientList[a].Group
			)
		);
		LEDGroupCount=LGL_Max
		(
			LEDGroupCount,
			inLEDClientList[a].Group+1
		);
	}

	{
		float coolR;
		float coolG;
		float coolB;
		GetColorCool(coolR,coolG,coolB);

		float warmR;
		float warmG;
		float warmB;
		GetColorWarm(warmR,warmG,warmB);

		for(int a=0;a<LED_GROUP_MAX;a++)
		{
			LEDColor[a].SetRGB
			(
				0.0f,
				0.0f,
				0.0f
			);
		}
	}

	sprintf(ProjMapCornersPath,"%s/.dvj/projMapCorners.txt",LGL_GetHomeDir());
	for(int a=0;a<4;a++)
	{
		ProjMapOffsetX[a]=0.0f;
		ProjMapOffsetY[a]=0.0f;
		ProjMapOffsetPrevX[a]=0.0f;
		ProjMapOffsetPrevY[a]=0.0f;
	}
	LoadProjMapPrevCorners();
}

VisualizerObj::
~VisualizerObj()
{
	delete	BlueScreenOfDeath;
	delete	Accumulation;

	for(unsigned int a=0;a<LEDClientList.size();a++)
	{
		delete LEDClientList[a];
	}
	LEDClientList.clear();
}

void
VisualizerObj::
NextFrame
(
	float		secondsElapsed,
	TurntableObj**	tts
)
{
	//Frequency-sensitive video SetTime()
	for(int t=0;t<2;t++)
	{
		if
		(
			tts[t]->GetFinalSpeed()!=0.0f &&
			tts[t]->GetFreqSenseBrightnessPreview()>0.0f
		)
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
						GetTestFreqSenseTime()==false &&
						oldFreqBrightness>0.0f &&
						neoFreqBrightness==0.0f
					)
					{
						float min=LGL_Min(10.0f,vid->GetLengthSeconds()-5.0f);
						if(min<0.0f) min=0.0f;
						float max=vid->GetLengthSeconds()-10.0f;
						if(max<0.0f) max=vid->GetLengthSeconds();

						/*
						float cand = LGL_Clamp
						(
							min,
							LGL_RandFloat(-10.0f,10.0f)+tts[t]->GetSoundPositionPercent()*vid->GetLengthSeconds()-10.0f,
							max
						);
						vid->SetTime(cand);
						*/

						vid->SetTime(LGL_RandFloat(min,max));
					}
					else if
					(
						GetTestFreqSenseTime() &&
						oldFreqBrightness==0.0f &&
						neoFreqBrightness>0.0f
					)
					{
						float min=LGL_Min(10.0f,vid->GetLengthSeconds()-5.0f);
						if(min<0.0f) min=0.0f;
						float max=vid->GetLengthSeconds()-10.0f;
						if(max<0.0f) max=vid->GetLengthSeconds();

						vid->SetTime(max*neoFreqBrightness);
					}
					else
					{
						float speedFactor=
							(vid==vidL) ? 1.0f : 4.0f;
						vid->SetTime(vid->GetTime()+speedFactor*(1.0f/60.0f));
						
						/*
						float min=LGL_Min(10.0f,vid->GetLengthSeconds()-5.0f);
						if(min<0.0f) min=0.0f;
						float max=vid->GetLengthSeconds()-10.0f;
						if(max<0.0f) max=vid->GetLengthSeconds();

						vid->SetTime(max*neoFreqBrightness);
						*/
					}
					vid->StoredBrightness=neoFreqBrightness;
				}
			}
		}
	}

	/*
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
	*/

	//Projection Map Offset
	if(LGL_KeyDown(LGL_KEY_RALT))
	{
		if(LGL_KeyStroke(LGL_KEY_Q))
		{
			ApplyProjMapPrevCorners();
		}

		int which=GetWhichProjMapCorner();
		const float rate = 0.1f * (1.0f/60.0f);
		if(LGL_KeyDown(LGL_KEY_A))
		{
			ProjMapOffsetX[which]-=rate;
		}
		if(LGL_KeyDown(LGL_KEY_D))
		{
			ProjMapOffsetX[which]+=rate;
		}
		if(LGL_KeyDown(LGL_KEY_S))
		{
			ProjMapOffsetY[which]-=rate;
		}
		if(LGL_KeyDown(LGL_KEY_W))
		{
			ProjMapOffsetY[which]+=rate;
		}

		/*
		if(LGL_MultiTouchFingerCount()==3)
		{
			ProjMapOffsetX[which]+=LGL_MultiTouchDX()*0.1f;
			ProjMapOffsetY[which]+=LGL_MultiTouchDY()*0.1f;
		}
		*/
	}

	if(LGL_KeyRelease(LGL_KEY_RALT))
	{
		SaveProjMapPrevCorners();
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
				LowMemoryTimer.Reset();
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

	if(LGL_AudioAvailable()==false)
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

	//Frequency-sensitive LEDs

	if(LGL_GetActiveDisplay()==0)
	{
		if(LEDTimer.SecondsSinceLastReset()>=1.0f/30.0f)
		{
			LEDTimer.Reset();

			//Cache some values
			float brLow[2];
			float brHigh[2];
			float freqFactor[2];
			for(int t=0;t<2;t++)
			{
				float volAve;
				float volMax;
				tts[t]->GetFreqMetaData(volAve,volMax,freqFactor[t]);
				brLow[t] = GetFreqBrightness(false,freqFactor[t],volAve);
				brHigh[t] = GetFreqBrightness(true,freqFactor[t],volAve);
			}

			//Get LEDColor for each group.
			for(int g=0;g<LEDGroupCount;g++)
			{
				float ledR=0.0f;
				float ledG=0.0f;
				float ledB=0.0f;
				
				//Each turntable adds to the led* vars
				for(int t=0;t<2;t++)
				{
					float ejectBrightnessScalar=tts[t]->GetEjectVisualBrightnessScalar();
					float freqSenseLEDBright=tts[t]->GetFreqSenseLEDBrightnessFinal(g);
					float freqSenseLEDBrightWash=tts[t]->GetFreqSenseLEDBrightnessWash(g);
					freqSenseLEDBright*=ejectBrightnessScalar;
					float brL=(1.0f-freqSenseLEDBrightWash)*brLow[t]*freqSenseLEDBright;
					float brH=((1.0f-freqSenseLEDBrightWash)*brHigh[t]+freqSenseLEDBrightWash)*freqSenseLEDBright;
					LGL_Color colorL = tts[t]->GetFreqSenseLEDColorLow(g);
					LGL_Color colorH = tts[t]->GetFreqSenseLEDColorHigh(g);

					ledR+=
						brL*colorL.GetR()+
						brH*colorH.GetR();
					
					ledG+=
						brL*colorL.GetG()+
						brH*colorH.GetG();
					
					ledB+=
						brL*colorL.GetB()+
						brH*colorH.GetB();
				}

				//Set the final LEDColors.
				float minR=LGL_Max
					(
						0.0f,
						LEDColor[g].GetR()-0.25f
					);
				float minG=LGL_Max
					(
						0.0f,
						LEDColor[g].GetG()-0.25f
					);
				float minB=LGL_Max
					(
						0.0f,
						LEDColor[g].GetB()-0.25f
					);

				ledR=LGL_Clamp(minR,ledR,1.0f);
				ledG=LGL_Clamp(minG,ledG,1.0f);
				ledB=LGL_Clamp(minB,ledB,1.0f);

				LEDColor[g].SetRGB
				(
					ledR,
					ledG,
					ledB
				);
			}

			//Send out color for each client, as defined by its group
			for(unsigned int c=0;c<LEDClientList.size();c++)
			{
				int group=LEDClientList[c]->GetGroup();
				LEDClientList[c]->SetColor
				(
					LEDColor[group].GetR(),
					LEDColor[group].GetG(),
					LEDColor[group].GetB()
				);
			}
		}
	}

	//Projection Map Corners
	if(LGL_GetActiveDisplay()==0)
	{
		float myL=l;
		float myR=r;
		float myB=b;
		float myT=t;

		GetProjectorARCoordsFromViewportCoords
		(
			myL,
			myR,
			myB,
			myT
		);

		myL-=0.075f*w;
		myR+=0.02f*w;
		myT-=0.10f*h;
		myB+=0.12f*h;

		myL=LGL_Max(l+0.02f*w,myL);
		myR=LGL_Min(r-0.05f*w,myR);

		if(LGL_KeyDown(LGL_KEY_RALT))
		{
			int resX=(int)(ViewportVisualsWidth*LGL_DisplayResolutionX());
			int resY=(int)(ViewportVisualsHeight*LGL_DisplayResolutionY());
			for(int a=0;a<4;a++)
			{
				float x;
				float y;
	
				if(a==0)
				{
					//Bottom Left
					x=myL;
					y=myB;
				}
				else if(a==1)
				{
					//Top Left
					x=myL;
					y=myT;
				}
				else if(a==2)
				{
					//Top Right
					x=myR;
					y=myT;
				}
				else if(a==3)
				{
					//Bottom Right
					x=myR;
					y=myB;
				}

				float bri=(GetWhichProjMapCorner()==a) ? 1.0f : 0.5f;

				LGL_GetFont().DrawString
				(
					x,
					y,
					h*0.04f,
					bri,bri,bri,1.0f,
					false,
					0.75f,
					"x: %i",
					(int)(ProjMapOffsetX[a]*resX)
				);

				LGL_GetFont().DrawString
				(
					x,
					y-2*h*0.04f,
					h*0.04f,
					bri,bri,bri,1.0f,
					false,
					0.75f,
					"y: %i",
					(int)(ProjMapOffsetY[a]*resY)
				);
			}
		}
	}
	
	if(LGL_GetActiveDisplay()==0)
	{
		if(GetSyphonServerEnabled())
		{
			Accumulation->FrameBufferUpdate();
			LGL_SyphonPushImage(Accumulation);
		}
	}
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

	//Accumulation->FrameBufferViewport(left,right,bottom,top);
}

float
VisualizerObj::
GetViewportVisualsL()
{
	return(ViewportVisualsLeft);
}

float
VisualizerObj::
GetViewportVisualsR()
{
	return(ViewportVisualsRight);
}

float
VisualizerObj::
GetViewportVisualsB()
{
	return(ViewportVisualsBottom);
}

float
VisualizerObj::
GetViewportVisualsT()
{
	return(ViewportVisualsTop);
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
		Accumulation->FrameBufferViewport
		(
			0,1,
			0,1
		);
	}
	else
	{
		Accumulation->FrameBufferViewport
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

bool
VisualizerObj::
GetProjectorClear()
{
	return(ProjectorClear);
}

void
VisualizerObj::
SetProjectorClear
(
	bool	clear
)
{
	ProjectorClear=clear;
}

bool
VisualizerObj::
GetProjectorPreviewClear()
{
	return(ProjectorPreviewClear);
}

void
VisualizerObj::
SetProjectorPreviewClear
(
	bool	clear
)
{
	ProjectorPreviewClear=clear;
}

void
VisualizerObj::
GetProjectorARCoordsFromViewportCoords
(
	float&	l,
	float&	r,
	float&	b,
	float&	t
)
{
	int projW;
	int projH;
	if(LGL_DisplayCount()==1)
	{
		projW = ViewportVisualsWidth * LGL_DisplayResolutionX();
		projH = ViewportVisualsHeight * LGL_DisplayResolutionY();
	}
	else
	{
		int projDisplay = LGL_Max(0,LGL_DisplayCount()-1);
		projW = LGL_DisplayResolutionX(projDisplay);
		projH = LGL_DisplayResolutionY(projDisplay);
	}
	float projAR = projW/(float)projH;

	GetTargetARCoordsFromViewportCoords
	(
		projAR,
		l,
		r,
		b,
		t
	);
}

void
VisualizerObj::
GetImageARCoordsFromViewportCoords
(
	LGL_Image*	image,
	float&		l,
	float&		r,
	float&		b,
	float&		t
)
{
	float imageAR = image->GetWidth()/(float)image->GetHeight();

	GetTargetARCoordsFromViewportCoords
	(
		imageAR,
		l,
		r,
		b,
		t
	);
}

void
VisualizerObj::
GetTargetARCoordsFromViewportCoords
(
	float	targetAR,
	float&	l,
	float&	r,
	float&	b,
	float&	t
)
{
	float w=r-l;
	float h=t-b;
	float midX = 0.5f*(l+r);
	float midY = 0.5f*(b+t);
	float viewportAR = w*LGL_DisplayResolutionX()/(float)(h*LGL_DisplayResolutionY());

	float viewportLimitL = midX - 0.5f * w * (targetAR/viewportAR);
	float viewportLimitR = midX + 0.5f * w * (targetAR/viewportAR);
	float viewportLimitB = midY - 0.5f * h * (viewportAR/targetAR);
	float viewportLimitT = midY + 0.5f * h * (viewportAR/targetAR);

	l=LGL_Max(l,viewportLimitL);
	r=LGL_Min(r,viewportLimitR);
	b=LGL_Max(b,viewportLimitB);
	t=LGL_Min(t,viewportLimitT);

	/*
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
	*/
}

int
VisualizerObj::
GetWhichProjMapCorner()
{
	int which=0;
	float midX=0.5f*(ViewportVisualsLeft+ViewportVisualsRight);
	float midY=0.5f*(ViewportVisualsBottom+ViewportVisualsTop);
	if(LGL_MouseX()<midX)
	{
		if(LGL_MouseY()<midY)
		{
			which=0;
		}
		else
		{
			which=1;
		}
	}
	else
	{
		if(LGL_MouseY()<midY)
		{
			which=3;
		}
		else
		{
			which=2;
		}
	}

	return(which);
}

void
VisualizerObj::
SaveProjMapPrevCorners()
{
	char data[4096];
	sprintf
	(
		data,
		"ProjMapCorners|%f|%f|%f|%f|%f|%f|%f|%f\n",
		ProjMapOffsetX[0],
		ProjMapOffsetY[0],
		ProjMapOffsetX[1],
		ProjMapOffsetY[1],
		ProjMapOffsetX[2],
		ProjMapOffsetY[2],
		ProjMapOffsetX[3],
		ProjMapOffsetY[3]
	);

	LGL_WriteFileAsync(ProjMapCornersPath,data,strlen(data));

	/*
	for(int a=0;a<4;a++)
	{
		ProjMapOffsetPrevX[a]=ProjMapOffsetX[a];
		ProjMapOffsetPrevY[a]=ProjMapOffsetY[a];
	}
	*/
}

void
VisualizerObj::
LoadProjMapPrevCorners()
{
	//Initialize to default values.
	for(int a=0;a<4;a++)
	{
		ProjMapOffsetPrevX[a]=0.0f;
		ProjMapOffsetPrevY[a]=0.0f;
	}

	//Read the data in...
	const int dataLen=2048;
	char data[dataLen];
	if(FILE* fd=fopen(ProjMapCornersPath,"r"))
	{
		fgets(data,dataLen,fd);
		fclose(fd);
	}
	else
	{
		return;
	}

	//Process the data
	FileInterfaceObj fi;
	fi.ReadLine(data);
	if(fi.Size()==0)
	{
		return;
	}
	if(strcasecmp(fi[0],"ProjMapCorners")==0)
	{
		if(fi.Size()!=9)
		{
			printf("VisualizerObj::LoadProjMapCorners('%s'): Warning!\n",ProjMapCornersPath);
			printf("\tStrange fi.size() of '%i' (Expecting 9)\n",fi.Size());
		}
		else
		{
			ProjMapOffsetPrevX[0]=atof(fi[1]);
			ProjMapOffsetPrevY[0]=atof(fi[2]);
			ProjMapOffsetPrevX[1]=atof(fi[3]);
			ProjMapOffsetPrevY[1]=atof(fi[4]);
			ProjMapOffsetPrevX[2]=atof(fi[5]);
			ProjMapOffsetPrevY[2]=atof(fi[6]);
			ProjMapOffsetPrevX[3]=atof(fi[7]);
			ProjMapOffsetPrevY[3]=atof(fi[8]);
		}
	}
}

void
VisualizerObj::
ApplyProjMapPrevCorners()
{
	for(int a=0;a<4;a++)
	{
		ProjMapOffsetX[a]=ProjMapOffsetPrevX[a];
		ProjMapOffsetY[a]=ProjMapOffsetPrevY[a];
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
	if(tt->GetMode()!=2)
	{
		return;
	}

	float lOrig=l;
	float rOrig=r;
	float wOrig=r-l;
	float bOrig=b;
	float tOrig=t;
	float hOrig=t-b;
	//float w=r-l;
	//float h=t-b;

	float videoBright = preview ? tt->GetVideoBrightnessPreview() : tt->GetVideoBrightnessFinal();
	float syphonBright = preview ? tt->GetSyphonBrightnessPreview() : tt->GetSyphonBrightnessFinal();
	float oscilloscopeBright = preview ? tt->GetOscilloscopeBrightnessPreview() : tt->GetOscilloscopeBrightnessFinal();
	float freqSenseBright = preview ? tt->GetFreqSenseBrightnessPreview() : tt->GetFreqSenseBrightnessFinal();

	float ejectBrightnessScalar=tt->GetEjectVisualBrightnessScalar();
	videoBright*=ejectBrightnessScalar;
	syphonBright*=ejectBrightnessScalar;
	oscilloscopeBright*=ejectBrightnessScalar;
	freqSenseBright*=ejectBrightnessScalar;

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
				if(vid==vidL) br*=4;	//FIXME: Ben / Zebbler hack... Shouldn't be this way!!
				br*=vid->StoredBrightness*freqSenseBright;

				LGL_Image* image = vid->GetImage();//EIGHT_WAY ? !preview : preview);
				{
					if
					(
						image &&
						image->GetFrameNumber()!=-1
					)
					{
						float myL = l;
						float myR = r;
						float myB = b;
						float myT = t;

						GetProjectorARCoordsFromViewportCoords
						(
							myL,
							myR,
							myB,
							myT
						);

						float alpha=0.0f;
						if(preview==false)
						{
							if
							(
								LGL_GetActiveDisplay()==0 &&
								ProjectorPreviewClear
							)
							{
								alpha=1.0f;
								ProjectorPreviewClear=false;
							}

							if
							(
								LGL_GetActiveDisplay()==1 &&
								ProjectorClear
							)
							{
								alpha=1.0f;
								ProjectorClear=false;
							}
						}

						float x[4];
						float y[4];

						if(LGL_GetActiveDisplay()==0)
						{
							//LB
							x[0]=myL;
							y[0]=myB;
							//RB
							x[1]=myR;
							y[1]=myB;
							//RT
							x[2]=myR;
							y[2]=myT;
							//LT
							x[3]=myL;
							y[3]=myT;
						}
						else
						{
							//LB
							x[0]=myL+ProjMapOffsetX[0];
							y[0]=myB+ProjMapOffsetY[0];
							//RB
							x[1]=myR+ProjMapOffsetX[3];
							y[1]=myB+ProjMapOffsetY[3];
							//RT
							x[2]=myR+ProjMapOffsetX[2];
							y[2]=myT+ProjMapOffsetY[2];
							//LT
							x[3]=myL+ProjMapOffsetX[1];
							y[3]=myT+ProjMapOffsetY[1];
						}
						image->DrawToScreen
						(
							x,
							y,
							1.0f,
							1.0f,
							1.0f,
							alpha,
							br
						);
//LGL_DebugPrintf("br: %.2f\n",br);
					}
				}
			}
		}
	}

	if(oscilloscopeBright > 0.0f)
	{
		bool drewOscilloscope=false;
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
				drewOscilloscope=true;
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
			drewOscilloscope=true;
		}

		if(drewOscilloscope)
		{
			if(preview==false)
			{
				if
				(
					LGL_GetActiveDisplay()==0 &&
					ProjectorPreviewClear
				)
				{
					ProjectorPreviewClear=false;
				}

				if
				(
					LGL_GetActiveDisplay()==1 &&
					ProjectorClear
				)
				{
					ProjectorClear=false;
				}
			}
		}
	}

	for(int v=0;v<2;v++)
	{
		float vBright = (v==0) ? syphonBright : videoBright;
		if(vBright > 0.0f)
		{
			LGL_Image* image=NULL;
			int vidFPS=-1;
			int vidFPSDisplayed=-1;
			int vidFPSMissed=0;
			if(v==0)
			{
				image=LGL_SyphonImage();
				if(image)
				{
					image->SetFrameNumber(0);
//if(preview==false) LGL_SyphonPushImage(image);
				}
			}
			else
			{
				if(LGL_VideoDecoder* vid = tt->GetVideo())
				{
					image=vid->GetImage();
					ForceVideoToBackOfRandomQueue(vid->GetPathShort());
					if(preview)
					{
						VideoFPSDisplay-=LGL_SecondsSinceLastFrame();
						if
						(
							tt->GetPaused()==false &&
							GetInput().WaveformRecordHold(tt->GetTarget())==false
						)
						{
							if(vid->GetFPSMissed()>0)
							{
								VideoFPSDisplay=5.0f;
							}
							else if(vid->GetFPSDisplayed()<vid->GetFPS()*0.90f)
							{
								VideoFPSDisplay=5.0f;
							}
						}
					}
					vidFPS=(int)ceilf(vid->GetFPS());
					vidFPSDisplayed=vid->GetFPSDisplayed();
					vidFPSMissed=vid->GetFPSMissed();
				}
			}

			if
			(
				image!=NULL &&
				image->GetFrameNumber()!=-1
			)
			{
				float alpha=0.0f;
				if(preview==false)
				{
					if
					(
						LGL_GetActiveDisplay()==0 &&
						ProjectorPreviewClear
					)
					{
						alpha=1.0f;
						ProjectorPreviewClear=false;
					}

					if
					(
						LGL_GetActiveDisplay()==1 &&
						ProjectorClear
					)
					{
						alpha=1.0f;
						ProjectorClear=false;
					}
				}

				float myL = l;
				float myR = r;
				float myB = b;
				float myT = t;

				if(tt->GetAspectRatioMode()!=2)
				{
					if(tt->GetAspectRatioMode()==0)
					{
						//Respect Image AR

						GetProjectorARCoordsFromViewportCoords
						(
							myL,
							myR,
							myB,
							myT
						);

						GetImageARCoordsFromViewportCoords
						(
							image,
							myL,
							myR,
							myB,
							myT
						);
					}
					else if(tt->GetAspectRatioMode()==1)
					{
						//Fill (but respect projector AR)

						GetProjectorARCoordsFromViewportCoords
						(
							myL,
							myR,
							myB,
							myT
						);
					}

					float x[4];
					float y[4];

					if(LGL_GetActiveDisplay()==0)
					{
						//LB
						x[0]=myL;
						y[0]=myB;
						//RB
						x[1]=myR;
						y[1]=myB;
						//RT
						x[2]=myR;
						y[2]=myT;
						//LT
						x[3]=myL;
						y[3]=myT;
					}
					else
					{
						//LB
						x[0]=myL+ProjMapOffsetX[0];
						y[0]=myB+ProjMapOffsetY[0];
						//RB
						x[1]=myR+ProjMapOffsetX[3];
						y[1]=myB+ProjMapOffsetY[3];
						//RT
						x[2]=myR+ProjMapOffsetX[2];
						y[2]=myT+ProjMapOffsetY[2];
						//LT
						x[3]=myL+ProjMapOffsetX[1];
						y[3]=myT+ProjMapOffsetY[1];
					}

					//float rgbSpatializerScalar=0.0f;
					{
						/*
						float volAve;
						float volMax;
						float freqFactor;
						tt->GetFreqMetaData(volAve,volMax,freqFactor);
						volAve = LGL_Min(1.0f,volAve*2.0f);
						LGL_DebugPrintf("volAve: %.2f\n",volAve);
						LGL_DebugPrintf("volMax: %.2f\n",volMax);
						LGL_DebugPrintf("freqFactor: %.2f\n",freqFactor);

						float vol = LGL_Min(1,volAve*tt->GetGain());
						float multFreq = 1.0f;//tt->GetEQLo();
						float myFreqFactor=freqFactor;
						float br = GetFreqBrightness(false,myFreqFactor,vol)*multFreq;
						rgbSpatializerScalar=br*4.0f;
						LGL_DebugPrintf("RGB Spatializer: %.2f\n",rgbSpatializerScalar,myFreqFactor,vol,freqFactor);
						LGL_DebugPrintf("myFF: %.2f\n",myFreqFactor,vol);
						LGL_DebugPrintf("vol: %.2f\n",vol);
						LGL_DebugPrintf("FF: %.2f\n",freqFactor);
						*/
					}

					image->DrawToScreen
					(
						x,
						y,
						vBright,
						vBright,
						vBright,
						alpha,
						1.0f,	//brightnessScalar
						0.0f,
						1.0f,
						0.0f,
						1.0f,
						0.0f//rgbSpatializerScalar
					);
				}
				else //tt->GetAspectRatioMode()==2
				{
					//Zebbler-tiling

					GetProjectorARCoordsFromViewportCoords
					(
						myL,
						myR,
						myB,
						myT
					);

					float myL13rd = myL + (1.0f/3.0f)*(myR-myL);
					float myL23rd = myL + (2.0f/3.0f)*(myR-myL);
					myL=LGL_Max(l,myL);
					myR=LGL_Min(r,myR);
					myB=LGL_Max(b,myB);
					myT=LGL_Min(t,myT);
					image->DrawToScreen
					(
						myL13rd,myL,
						myB,myT,
						0,
						vBright,
						vBright,
						vBright,
						alpha
					);
					image->DrawToScreen
					(
						myL13rd,myL23rd,
						myB,myT,
						0,
						vBright,
						vBright,
						vBright,
						alpha
					);
					image->DrawToScreen
					(
						myR,myL23rd,
						myB,myT,
						0,
						vBright,
						vBright,
						vBright,
						alpha
					);
				}

				if
				(
					preview &&
					VideoFPSDisplay
				)
				{
					if(vidFPS>=0)
					{
						LGL_GetFont().DrawString
						(
							lOrig+0.05f*wOrig,tOrig-0.15f*hOrig,0.1f*hOrig,
							VideoFPSDisplay,VideoFPSDisplay,VideoFPSDisplay,VideoFPSDisplay,
							false,
							0.75f,
							"%i",
							vidFPS
						);
					}

					float br=VideoFPSDisplay;
					if(vidFPSDisplayed>=0)
					{
						LGL_GetFont().DrawString
						(
							lOrig+0.05f*wOrig,bOrig+0.05f*hOrig,0.1f*hOrig,
							br,br,br,br,
							false,
							0.75f,
							"%i",
							vidFPSDisplayed
						);
					}

					/*
					LGL_GetFont().DrawString
					(
						rOrig-0.3f*wOrig,tOrig-0.15f*hOrig,0.1f*hOrig,
						br,br,br,br,
						false,
						0.75f,
						"(%i)",
						LGL_FPS()
					);
					*/

					if(vidFPSMissed>0)
					{
						LGL_GetFont().DrawString
						(
							rOrig-0.3f*wOrig,bOrig+0.05f*hOrig,0.1f*hOrig,
							br,0,0,br,
							false,
							0.75f,
							"(%i)",
							vidFPSMissed
						);
					}
				}
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
					noiseFactorVideo*vBright,
					noiseFactorVideo*vBright,
					noiseFactorVideo*vBright,
					noiseFactorVideo*vBright
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
}

int
VisualizerObj::
GetLEDGroupCount()
{
	return(LEDGroupCount);
}

LGL_Color
VisualizerObj::
GetLEDColor
(
	int	group
)
{
	group=LGL_Clamp
	(
		0,
		group,
		LED_GROUP_MAX
	);
	return(LEDColor[group]);
}

