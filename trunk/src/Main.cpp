/*
 *
 * dvj
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

#include "LGL.module/LGL.h"
#include "Common.h"
#include "Visualizer.h"
#include "Mixer.h"
#include "Particle.h"
#include "Config.h"

#include "Input.h"
#include "InputNull.h"
#include "InputKeyboard.h"
#include "InputMouse.h"
#include "InputOsc.h"
#include "InputXponent.h"
#include "InputXsession.h"
#include "InputWiimote.h"
#include "InputTester.h"

//Global Variables

float				QuadrentSplitX;
float				QuadrentSplitY;
bool				ExitPrompt;

MixerObj*			Mixer;
VisualizerObj*			Visualizer;
bool				VisualizerFullScreen;

LGL_Image*			ParticleSystemImage;
ParticleSystemObj*		ParticleSystem;
bool				ParticleSystemActive=false;
LGL_Timer			MouseMotionTimer;

ParticleSystemObj*		ParticlePointers[8];
bool				ParticlePointersActive[8];

bool				LogVisuals=true;
bool				LogEverything=true;

float				OmniFader=1.0f;
float				OmniFaderDirection=1.0f;

float				VisualizerZoomOutPercent=0.0f;
float				VisualizerZoomOutPercentSmooth=0.0f;
bool				VisualizerZoomOut=false;

bool				eeepc=false;

char				recordPath[2048];
char				recordOldPath[2048];

void InitializeGlobalsPreLGL()
{
	sprintf(recordPath,"%s/.dvj/record",LGL_GetHomeDir());
	sprintf(recordOldPath,"%s/old",recordPath);
#ifdef	LGL_LINUX
	//Move all old generic record files to old folder
	if(LGL_DirectoryExists(recordPath))
	{
		if(LGL_DirectoryExists(recordOldPath)==false)
		{
			bool ok=LGL_DirectoryCreate(recordOldPath);
			assert(ok);
		}
		LGL_DirTree dirTree(recordPath);
		dirTree.WaitOnWorkerThread();
		for(unsigned int a=0;a<dirTree.GetFileCount();a++)
		{
			const char* target=dirTree.GetFileName(a);
			if
			(
				target[0]=='2' &&
				target[1]=='0'
			)
			{
				char oldPath[2048];
				char newPath[2048];
				sprintf(oldPath,"%s/%s",dirTree.GetPath(),target);
				sprintf(newPath,"%s/old/%s",dirTree.GetPath(),target);
				if(LGL_FileDirMove(oldPath,newPath)==false)
				{
					printf("\ndvj: Please verify that your user is the owner of '%s'\n\n",recordOldPath);
					exit(0);
				}
			}
		}
	}
#endif	//LGL_LINUX
}

void InitializeGlobals()
{
	Input.AddChild(new InputNullObj);
	Input.AddChild(new InputKeyboardObj);
	Input.AddChild(&GetInputMouse());
	Input.AddChild(&GetInputMultiTouch());
	Input.AddChild(&GetInputOsc());
	Input.AddChild(&GetInputXponent());
	Input.AddChild(new InputXsessionObj);
	Input.AddChild(new InputWiimoteObj);
	Input.AddChild(new InputTesterObj);

	QuadrentSplitX=.5f;
	QuadrentSplitY=.5f;
	ExitPrompt=false;
	VisualizerFullScreen=false;

	ParticleSystemImage=new LGL_Image("data/image/particle.png");
	for(int a=0;a<8;a++)
	{
		float r=(a==0) ? 0.50f : 0.0f;
		float g=(a==0) ? 0.25f : 0.0f;
		ParticlePointers[a] = new ParticleSystemObj
		(
			0.5f,0.5f,			//Start X/Y
			1.0f,1.0f,			//Scale X/Y
			0.05f,0.0125f,			//Radius Begin / End
			r,g,1.0f,0.0f,			//Begin Color
			0.0f,0.0f,0.0f,0.0f,		//End Color
			0.25f,0.75f,			//Life Min/Max
			0.00f,0.05f,			//Velocity Min/Max
			0.5f,				//Drag
			ParticleSystemImage,		//Image
			POINTER_PARTICLES_PER_SECOND	//Particles Per Second
		);
		ParticlePointersActive[a]=false;
	}

	Visualizer=new VisualizerObj;
	Mixer=new MixerObj;
	Mixer->SetVisualizer(Visualizer);
}

void
VerifyMusicDir()
{
	char musicDir[2048];
	strcpy(musicDir,GetMusicRootPath());
#ifdef LGL_OSX
	if
	(
		LGL_DirectoryExists(musicDir)==false ||
		LGL_FileExists(GetMusicRootConfigFilePath())==false
	)
	{
		char homeDocumentsDvjMusicRoot[2048];
		sprintf(homeDocumentsDvjMusicRoot,"%s/Documents/dvj/MusicRoot",LGL_GetHomeDir());
		SetMusicRootPath(homeDocumentsDvjMusicRoot);
	}

	return;
#endif	//LGL_OSX

	while
	(
		LGL_DirectoryExists(musicDir)==false ||
		LGL_FileExists(GetMusicRootConfigFilePath())==false
	)
	{
		char dir[2048];
		strcpy(dir,LGL_GetHomeDir());
#ifdef	LGL_OSX
		strcat(dir,"/Music/iTunes/iTunes Music/");
#else
		strcat(dir,"/Music/");
#endif	//LGL_OSX

		LGL_InputBuffer outputBuffer;
		outputBuffer.SetString(dir);
		outputBuffer.GrabFocus();
		for(;;)
		{
			LGL_ProcessInput();
			LGL_GetFont().DrawString
			(
				.05,.3,.025,
				1,1,1,1,
				false,
				.75,
				"Select your music folder:"
			);
			bool dirExists = LGL_DirectoryExists(outputBuffer.GetString()) &&
				strcmp(outputBuffer.GetString(),"/home")!=0 &&
				strcmp(outputBuffer.GetString(),"/home/")!=0;
			LGL_GetFont().DrawString
			(
				.05,.20,.025,
				1,dirExists?1:0,dirExists?1:0,1,
				false,
				.75,
				"> %s",
				outputBuffer.GetString()
			);
			if(LGL_KeyStroke(LGL_KEY_RETURN))
			{
				if(dirExists)
				{
					outputBuffer.ReleaseFocus();
					SetMusicRootPath(outputBuffer.GetString());
				}
				LGL_SwapBuffers();
				break;
			}

			if(LGL_KeyStroke(LGL_KEY_ESCAPE)) exit(0);

			LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());	//Limit framerate to 60 fps
			LGL_SwapBuffers();
		}
	}
}

void WarnVsync()
{
	printf("dvj: Error! OpenGL V-Sync must be enabled!\n");
	for(;;)
	{
		LGL_ProcessInput();
		
		if(LGL_KeyStroke(LGL_KEY_ESCAPE))
		{
			exit(-1);
		}

		LGL_GetFont().DrawString
		(
			0.5f,0.9f,0.06f,
			1,0,0,1,
			true,
			1.0f,
			"Error"
		);
		LGL_GetFont().DrawString
		(
			0.5f,0.5f,0.04f,
			1,1,1,1,
			true,
			1.0f,
			"OpenGL V-Sync Must Be Enabled"
		);

		LGL_GetFont().DrawString
		(
			0.5f,0.1f,0.03f,
			1,1,1,1,
			true,
			1.0f,
			"Press [ESC] to exit"
		);

		LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());
		LGL_SwapBuffers();
	}
}

//Core Functions

bool mouseMotionEver=false;

void NextFrame()
{
	LGL_ProcessInput();
	Input.NextFrame();

	if(LGL_SecondsSinceExecution()<5.0f)
	{
		if(LGL_FPS()>100)
		{
			//WarnVsync();
		}
	}

	if
	(
		LGL_AudioAvailable()==false &&
		LGL_AudioWasOnceAvailable()
	)
	{
		LGL_AttemptAudioRevive();
	}

	if
	(
		LGL_GetWiimote(1).ButtonStroke(LGL_WIIMOTE_RIGHT) &&
		LGL_GetWiimote(1).ButtonDown(LGL_WIIMOTE_B)
	)
	{
		VisualizerZoomOut=!VisualizerZoomOut;
	}
	if(VisualizerZoomOut)
	{
		VisualizerZoomOutPercent=LGL_Min(1.0f,VisualizerZoomOutPercent+0.5f*LGL_SecondsSinceLastFrame());
	}
	else
	{
		VisualizerZoomOutPercent=LGL_Max(0.0f,VisualizerZoomOutPercent-0.5f*LGL_SecondsSinceLastFrame());
	}
	VisualizerZoomOutPercentSmooth=(1.0f-cosf(LGL_PI*VisualizerZoomOutPercent))/2.0f;

	mouseMotionEver|=LGL_MouseMotion();

	//Globals

	if(ExitPrompt==false)
	{
		if(LGL_KeyStroke(LGL_KEY_ESCAPE))
		{
			ExitPrompt=true;
		}
	}
	else
	{
		if(LGL_KeyStroke(LGL_KEY_Y))
		{
			delete Visualizer;
			delete Mixer;
			LGL_Exit();
		}
		if
		(
			LGL_KeyStroke(LGL_KEY_N) ||
			LGL_KeyStroke(LGL_KEY_ESCAPE)
		)
		{
			ExitPrompt=false;
			Mixer->BlankFocusFilterText();
		}
	}

	if(LGL_KeyStroke(GetInputKeyboardVisualizerFullScreenToggleKey()))
	{
		VisualizerFullScreen=!VisualizerFullScreen;
		Visualizer->ToggleFullScreen();
	}

	if(LGL_KeyStroke(GetInputKeyboardScreenshotKey()))
	{
		char screenshotDir[2048];
		sprintf(screenshotDir,"%s/.dvj/screenshots",LGL_GetHomeDir());
		if(LGL_DirectoryExists(screenshotDir)==false)
		{
			LGL_DirectoryCreateChain(screenshotDir);
			if(LGL_DirectoryExists(screenshotDir)==false)
			{
				printf("LGL_DirectoryCreateChain() failed!\n");
			}
		}

		char screenshotPath[2048];
		for(int a=0;a<1000;a++)
		{
			sprintf(screenshotPath,"%s/dvj_screenshot_%.3i.bmp",screenshotDir,a);
			if(LGL_FileExists(screenshotPath)==false)
			{
				break;
			}
		}
		LGL_ScreenShot(screenshotPath);
	}
	if(LGL_KeyStroke(GetInputKeyboardFullScreenToggleKey()))
	{
		LGL_FullScreenToggle();
	}

	//Mixer
	Mixer->NextFrame(LGL_SecondsSinceLastFrame());

	//Visuals
	Visualizer->NextFrame(LGL_SecondsSinceLastFrame(),Mixer->GetTurntables());

	if(LGL_MouseMotion())
	{
		MouseMotionTimer.Reset();
	}

	//Wiimote
	if(LGL_MouseMotion() && 0)
	{
		if(ParticlePointersActive[0]==false)
		{
			char str[1024];
			sprintf(str,"!dvj::ParticlePointerActive|%i|%f|%f\n",0,LGL_MouseX(),LGL_MouseY());
			LGL_DrawLogWrite(str);
			ParticlePointers[0]->Pos.SetXY(LGL_MouseX(),LGL_MouseY());
		}
		ParticlePointersActive[0]=true;
	}

	for(int a=0;a<8;a++)
	{
		float seconds = LGL_Max(LGL_SecondsSinceLastFrame(),1.0f/60.0f);
		ParticlePointers[a]->ParticlesMax = 100;
		if(LGL_GetWiimote(a).GetPointerAvailable())
		{
			//Sub-frame pointer motion
			ParticlePointersActive[a]=true;
			ParticlePointers[a]->ParticlesPerSecond = POINTER_PARTICLES_PER_SECOND;
			std::vector<LGL_Vector> motion = LGL_GetWiimote(a).GetPointerMotionThisFrame();
			if(motion.empty()==false)
			{
				for(unsigned int p=0;p<motion.size();p++)
				{
					ParticlePointers[a]->PosPrev=ParticlePointers[a]->Pos;
					ParticlePointers[a]->Pos.SetXY
					(
						motion[p].GetX(),
						motion[p].GetY()
					);
					ParticlePointers[a]->NextFrame(seconds/motion.size());
				}
			}
			else
			{
				//No motion, so no need to update our Pos variables
				ParticlePointers[a]->PosPrev=ParticlePointers[a]->Pos;
				ParticlePointers[a]->ParticlesPerSecond = 0.0f;
				ParticlePointers[a]->NextFrame(seconds);
				ParticlePointers[a]->ParticlesPerSecond = POINTER_PARTICLES_PER_SECOND;
			}
		}
		else if
		(
			0 &&
			a==0 &&
			LGL_GetWiimote(a).Connected()==false &&
			mouseMotionEver
		)
		{
			//Fall back on LGL_Mouse

			ParticlePointersActive[a]=true;
			ParticlePointers[a]->ParticlesPerSecond = (MouseMotionTimer.SecondsSinceLastReset()<5.0f)?POINTER_PARTICLES_PER_SECOND:0.0f;
			ParticlePointers[a]->PosPrev=ParticlePointers[a]->Pos;
			ParticlePointers[a]->Pos.SetXY
			(
				LGL_MouseX(),
				LGL_MouseY()
			);
			ParticlePointers[a]->NextFrame(seconds);
		}
		else
		{
			ParticlePointers[a]->ParticlesPerSecond = 0;
			ParticlePointers[a]->PosPrev=ParticlePointers[a]->Pos;
			ParticlePointers[a]->NextFrame(seconds);
		}
	}

	OmniFader+=0.25f*LGL_SecondsSinceLastFrame()*OmniFaderDirection;
	if(OmniFader>1.0f && OmniFaderDirection==1.0f)
	{
		OmniFader=1.0f;
		OmniFaderDirection=0.0f;
	}
	if(OmniFader<0.0f && OmniFaderDirection==-1.0f)
	{
		OmniFader=0.0f;
		OmniFaderDirection=0.0f;
	}
	LGL_SetRecordDVJToFileVolume(OmniFader);
	Mixer->SetVolumeMaster(OmniFader);
	LGL_DrawLogWrite("dvj::OmniFader|%.3f\n",OmniFader);
}

void DrawFrame(bool visualsQuadrent, float visualizerZoomOutPercent=0.0f)
{
	if(ExitPrompt)
	{
		LGL_DrawLogWrite("NoTTLines\n");
	}
	
	if(visualsQuadrent)
	{
		LGL_DrawLogWrite("vqcre|T\n");
	}

	if(visualsQuadrent)
	{
		LGL_ClipRectEnable
		(
			0.0f,
			0.5f,
			0.5f,
			1.0f
		);
	}
	else
	{
		float left=0.0f;
		float right=1.0f+1.0f*visualizerZoomOutPercent;
		float bottom=0.0f-1.0f*visualizerZoomOutPercent;
		float top=1.0f;
		LGL_ClipRectEnable(left,right,bottom,top);
	}
	
	//Draw Visuals
	if(VisualizerFullScreen)
	{
		if(LogEverything==false && LogVisuals==false) LGL_DrawLogPause();
		//Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent,Mixer->GetTurntables());
		if(LogEverything==false && LogVisuals==false) LGL_DrawLogPause(false);
	}

	Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent,Mixer->GetTurntables());
	if(LGL_DisplayCount()>1 && LGL_IsFullScreen())
	{
		LGL_SetActiveDisplay(1);
		Visualizer->SetViewportVisuals(0.0f,1.0f,0.0f,1.0f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent,Mixer->GetTurntables());
		float left=0.0f;
		float right=LGL_Min(1.0f,GetProjectorQuadrentResX()/(float)LGL_DisplayResolutionX(0));
		float bottom=LGL_Max(0.5f,1.0f-GetProjectorQuadrentResY()/(float)LGL_DisplayResolutionY(0));
		float top=1.0f;
		Visualizer->SetViewportVisuals(left,right,bottom,top);
		LGL_SetActiveDisplay(0);
	}
	/*
	else if(EIGHT_WAY==false)
	{
		Visualizer->SetViewportVisuals(0.0f,0.5f,0.5f,1.0f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent,Mixer->GetTurntables());
	}
	else
	{
		Visualizer->SetViewportVisuals(0.00f,0.25f,0.75f,1.0f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent,Mixer->GetTurntables());
		Visualizer->SetViewportVisuals(0.25f,0.50f,0.75f,1.0f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent,Mixer->GetTurntables());
		Visualizer->SetViewportVisuals(0.50f,0.75f,0.75f,1.0f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent,Mixer->GetTurntables());
		Visualizer->SetViewportVisuals(0.75f,1.0f,0.75f,1.0f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent,Mixer->GetTurntables());

		Visualizer->SetViewportVisuals(0.00f,0.25f,0.50f,0.75f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent,Mixer->GetTurntables());
		Visualizer->SetViewportVisuals(0.25f,0.50f,0.50f,0.75f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent,Mixer->GetTurntables());
		Visualizer->SetViewportVisuals(0.50f,0.75f,0.50f,0.75f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent,Mixer->GetTurntables());
		Visualizer->SetViewportVisuals(0.75f,1.0f,0.50f,0.75f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent,Mixer->GetTurntables());
	}
	*/

	if(OmniFader<1.0f)
	{
		LGL_DrawRectToScreen
		(
			0,1,
			0,1,
			0,0,0,1.0f-OmniFader
		);
	}

	if(visualsQuadrent)
	{
		LGL_DrawLogWrite("vqcre|F\n");
	}

	if(LogEverything==false) LGL_DrawLogPause();
	if(ExitPrompt)
	{
		LGL_GetFont().DrawString
		(
			0.5f,0.26f,0.03f,
			1.0f,1.0f,1.0f,1.0f,
			true,
			0.75f,
			"Exit?"
		);
		LGL_GetFont().DrawString
		(
			0.5f,0.20f,0.03f,
			1.0f,1.0f,1.0f,1.0f,
			true,
			0.75f,
			"[Y] / [N]"
		);
	}
	if(LogEverything==false) LGL_DrawLogPause(false);
	
	if(ExitPrompt==false)
	{
		//Draw Mixer
		if(VisualizerFullScreen==false)
		{
			if(LogEverything==false) LGL_DrawLogPause();
			Mixer->DrawFrame(visualsQuadrent, visualizerZoomOutPercent);
			if(LogEverything==false) LGL_DrawLogPause(false);
		}
	}
	
	//Draw Quadrent Lines
	if(VisualizerFullScreen==false)
	{
		float vizRight=Visualizer->GetViewportRight();
		LGL_DrawLogWrite("dvj::MainDrawGlowLines|%c|%.3f|%f\n",visualsQuadrent?'T':'F',visualizerZoomOutPercent,vizRight);
		LGL_DrawLogPause();
		Main_DrawGlowLines(LGL_SecondsSinceExecution(),1.0f,visualsQuadrent,visualizerZoomOutPercent,vizRight);
		LGL_DrawLogPause(false);
	}

	//Draw Wiimote
	LGL_DrawLogPause();
	for(int a=0;a<8;a++)
	{
		if(ParticlePointersActive[a])
		{
			ParticlePointers[a]->Draw();
		}
	}
	LGL_DrawLogPause(false);
	LGL_ClipRectDisable();
}

float SwapOutOtherProgramsPercent=0.0f;
bool SwapOutOtherProgramsFinished=false;

int
SwapOutOtherPrograms(void* baka)
{
	long int memDelta = 128;
	long int memMax = 2048;
	for(long int size=memDelta;size<=memMax;size+=memDelta)
	{
		LGL_DelayMS(1);
		void* mem = malloc(size*1024*1024);
		free(mem);
		if(SwapOutOtherProgramsFinished)
		{
			break;
		}
		if(mem && size<memMax)
		{
			//Keep on going...
		}
		else
		{
			mem=NULL;
			while(mem==NULL)
			{
				size-=memDelta;
				if(size<=0)
				{
					break;
				}
				mem = malloc(size*1024*1024);
			}
			if(mem==NULL)
			{
				break;
			}
			char* memChar = (char*)mem;
			for(long int a=0;a<size*1024*1024;a+=2048)
			{
				if(a%(1024*1024)==0)
				{
					LGL_DelayMS(1);
				}
				memChar[a]=1;
				SwapOutOtherProgramsPercent=(a/(float)(size*1024*1024));
				if(SwapOutOtherProgramsFinished)
				{
					break;
				}
			}
			free(mem);
			break;
		}
	}
	SwapOutOtherProgramsFinished=true;
	return(0);
}

int main(int argc, char** argv)
{
	//Set working directory
	char wd[2048];
	strcpy(wd,argv[0]);
	if(char* lastSlash = strrchr(wd,'/'))
	{
		lastSlash[0]='\0';
#ifndef	LGL_OSX
		chdir(wd);
#else	//LGL_OSX
		//Currently wd is /path/to/dvj.app/Contents/MacOS
		chdir(wd);
#endif	//LGL_OSX
	}

	//Load config
	ConfigInit();

	//Purge active memory

#ifdef	LGL_OSX
	if
	(
		LGL_IsOsxAppBundle() &&
		GetPurgeInactiveMemory()
	)
	{
		printf("Purging inactive RAM: Alpha\n");
		system("purge");
		printf("Purging inactive RAM: Omega\n");
	}
#endif	//LGL_OSX

	//Initialize LGL
	int channels=4;
	bool drawFPS=false;
	float drawFPSSpike=0.0f;
	int resX=9999;
	int resY=9999;
	bool wireMemory=GetWireMemory();;

	for(int a=0;a<argc;a++)
	{
		if(strcasecmp(argv[a],"--1080p")==0)
		{
			resX=1920;
			resY=1080;
		}
		else if(strcasecmp(argv[a],"--720p")==0)
		{
			resX=1280;
			resY=720;
		}
		else if(strcasecmp(argv[a],"--480p")==0)
		{
			resX=852;
			resY=480;
		}
		else if(strcasecmp(argv[a],"--drawFPS")==0)
		{
			drawFPS=true;
		}
		else if(strcasecmp(argv[a],"--noWireMemory")==0)
		{
			wireMemory=false;
		}
		else if(strcasecmp(argv[a],"--help")==0)
		{
			printf("dvj, svn pre-release\n\n");
			printf("usage: dvj [--480p] [--720p] [--1080p] [--drawFPS] [--noWireMemory] [--help]\n\n");
			exit(0);
		}
	}

	InitializeGlobalsPreLGL();

	const char* appName = "dvj";
	if(LGL_IsOsxAppBundle())
	{
		appName=argv[0];
		if(const char* lastSlash = strrchr(argv[0],'/'))
		{
			appName = &(lastSlash[1]);
		}
	}

	LGL_Init
	(
		resX,
		resY,
		channels,
		appName
	);

	LGL_MouseVisible(false);
	LGL_SetFPSMax(GetFPSMax());

	VerifyMusicDir();

	float loadScreenPercent=0.0f;

	if(wireMemory)
	{
		const char* line1 = NULL;
		const char* line2 = "Obtaining memory";
		const char* line3 = "[ESC] skips (and risks audio/video skippage)";
		DrawLoadScreen(loadScreenPercent,line1,line2,line3);
		SDL_Thread* thread = LGL_ThreadCreate(SwapOutOtherPrograms);
		for(;;)
		{
			if(LGL_KeyStroke(LGL_KEY_ESCAPE))
			{
				SwapOutOtherProgramsFinished=true;
				break;
			}
			loadScreenPercent=powf(SwapOutOtherProgramsPercent,3);
			DrawLoadScreen(loadScreenPercent,line1,line2,line3);
			LGL_DelaySeconds(1.0f/60.0f);

			if(SwapOutOtherProgramsFinished)
			{
				LGL_ThreadWait(thread);
				loadScreenPercent=3.0f;
				break;
			}
		}
	}

	loadScreenPercent=0.0f;
	DrawLoadScreen(loadScreenPercent,NULL,"Scanning library");

	InitializeGlobals();

	LGL_SetAudioInPassThru(GetAudioInPassThru());

	if(eeepc)
	{
		Mixer->SetLowRez(eeepc);
		LGL_ThreadSetPriority(-0.9f);
		LGL_SmoothLines(false);
		LGL_SmoothPolygons(false);
	}

	LGL_DrawLogWrite
	(
		"!dvj::ParticleMouse|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%s|%i\n",
		0.5f,0.5f,		//Start X/Y
		1.0f,1.0f,		//Scale X/Y
		0.075f,0.025f,		//Radius Begin / End
		0.5f,0.25f,1.0f,0.0f,	//Begin Color
		0.0f,0.0f,0.0f,0.0f,	//End Color
		0.25f,0.5f,		//Life Min/Max
		0.0f,0.1f,		//Velocity Min/Max
		0.5f,			//Drag
		ParticleSystemImage->GetPath(),
		600			//Particles Per Second
	);

	for(;;)
	{
		DrawFrame(false);
		/*
		if(VisualizerZoomOutPercentSmooth!=0.0f)
		{
			float left=0.0f;
			float right=1.0f+1.0f*VisualizerZoomOutPercentSmooth;
			float bottom=0.0f-1.0f*VisualizerZoomOutPercentSmooth;
			float top=1.0f;
			LGL_DrawRectToScreen(0.0f,0.5f,0.5f,1.0f,0,0,0,1);
			LGL_ViewportScreen
			(
				left,
				right,
				bottom,
				top
			);
			DrawFrame(true,VisualizerZoomOutPercentSmooth);
			LGL_ViewportScreen();
		}
		*/
		NextFrame();
		/*
		LGL_DrawAudioOutBufferGraph
		(
			0.8f,0.9f,
			0.65f,0.75f,
			drawFPS ? 1 : drawFPSSpike,
			drawFPS ? 1 : drawFPSSpike
		);
		*/
		drawFPSSpike = LGL_Max(0.0f,drawFPSSpike-LGL_SecondsSinceLastFrame());
		if(LGL_FPS()<50)
		{
			drawFPSSpike=5.0f;
		}
		if
		(
			drawFPS &&
			VisualizerFullScreen==false //&&
			//LGL_SecondsSinceExecution()>15.0f
		)
		{
			drawFPSSpike=5.0f;
			LGL_DrawFPSGraph
			(
				0.875f,0.975f,
				0.875f,0.975f,
				drawFPS ? 1 : drawFPSSpike,
				drawFPS ? 1 : drawFPSSpike
			);
		}

		/*
		if((1.0f/60.0f)>LGL_SecondsSinceThisFrame())
		{
			LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());
		}
		*/
		LGL_MouseVisible(false);
		LGL_SwapBuffers();
	}
}

