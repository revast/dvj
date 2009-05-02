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

#include "Input.h"
#include "InputNull.h"
#include "InputKeyboard.h"
#include "InputMouse.h"
#include "InputXponent.h"
#include "InputWiimote.h"

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

LGL_DirTree dirTreeMusic("data");

void InitializeGlobalsPreLGL()
{
#ifdef	LGL_LINUX
	//Move all old generic data/record files to data/record/old
	if(LGL_DirectoryExists("data/record"))
	{
		LGL_DirTree dirTree("data/record");
		if(LGL_DirectoryExists("data/record/old")==false)
		{
			assert(LGL_DirectoryCreate("data/record/old"));
		}
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
					printf("\ndvj: Please verify that your user is the owner of 'data/record/old'\n\n");
					exit(0);
				}
			}
		}
	}

	dirTreeMusic.SetPath("music");
#endif	//LGL_LINUX
}

void InitializeGlobals()
{
	Input.AddChild(new InputNullObj);
	Input.AddChild(new InputKeyboardObj);
	Input.AddChild(new InputMouseObj);
	Input.AddChild(new InputXponentObj);
	Input.AddChild(new InputWiimoteObj);

	QuadrentSplitX=.5f;
	QuadrentSplitY=.5f;
	ExitPrompt=false;
	VisualizerFullScreen=false;

	ParticleSystemImage=new LGL_Image("data/particle.png");
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
#ifndef	LGL_WIN32
	while(LGL_DirectoryExists("data/music")==false)
	{
		LGL_InputBuffer outputBuffer;
		outputBuffer.SetString("/home/");
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
			if(LGL_KeyStroke(SDLK_RETURN))
			{
				if(dirExists)
				{
					outputBuffer.ReleaseFocus();
					char cmd[1024];
					sprintf(cmd,"ln -s \"%s\" data/music",outputBuffer.GetString());
					system(cmd);
				}
				break;
			}

			if(LGL_KeyStroke(SDLK_ESCAPE)) exit(0);

			LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());	//Limit framerate to 60 fps
			LGL_SwapBuffers();
		}
	}
#endif	//LGL_WIN32
}

void WarnVsync()
{
	printf("dvj: Error! OpenGL V-Sync must be enabled!\n");
	for(;;)
	{
		LGL_ProcessInput();
		
		if(LGL_KeyStroke(SDLK_ESCAPE))
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
		LGL_AudioWasOnceAvailable() &&
		LGL_KeyStroke(SDLK_F9)
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

	if
	(
		LGL_KeyStroke(SDLK_F7) &&
		Mixer->GetRecording()==false
	)
	{
		//Start recording!
		LGL_RecordDVJToFileStart();
		Mixer->SetRecording();

		if(LGL_FileExists("data/record/drawlog.txt"))
		{
			LGL_FileDelete("data/record/drawlog.txt");
		}
		LGL_DrawLogStart("data/record/drawlog.txt");

		LGL_DrawLogWrite
		(
			"!dvj::Record.mp3|data/record/%s.mp3\n",
			LGL_DateAndTimeOfDayOfExecution()
		);
	}

	//Globals

	if(ExitPrompt==false)
	{
		if(LGL_KeyStroke(SDLK_ESCAPE))
		{
			ExitPrompt=true;
		}
	}
	else
	{
		if(LGL_KeyStroke(SDLK_y))
		{
			delete Visualizer;
			delete Mixer;
			exit(0);
		}
		if
		(
			LGL_KeyStroke(SDLK_n) ||
			LGL_KeyStroke(SDLK_ESCAPE)
		)
		{
			ExitPrompt=false;
			Mixer->BlankFocusFilterText();
		}
	}

	if(LGL_KeyStroke(SDLK_F2))
	{
		VisualizerFullScreen=!VisualizerFullScreen;
		Visualizer->ToggleFullScreen();
	}
	
	/*
	if(LGL_KeyStroke(SDLK_F10))
	{
		LGL_ScreenShot();
	}
	*/
	if(LGL_KeyStroke(SDLK_F11))
	{
		LGL_FullScreenToggle();
	}

	//Mixer
	Mixer->NextFrame(LGL_SecondsSinceLastFrame());

	//Visuals
	Visualizer->NextFrame(LGL_SecondsSinceLastFrame());

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
		LGL_DrawLogWrite("dvj::MainDrawGlowLines|%c|%.3f\n",visualsQuadrent?'T':'F',visualizerZoomOutPercent);
		LGL_DrawLogPause();
		Main_DrawGlowLines(LGL_SecondsSinceExecution(),1.0f,visualsQuadrent,visualizerZoomOutPercent);
		LGL_DrawLogPause(false);
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
	if(visualsQuadrent)
	{
		LGL_ClipRectDisable();
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
	//Draw Wiimote for visuals
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
	
	//Draw Visuals
	if(VisualizerFullScreen)
	{
		if(LogEverything==false && LogVisuals==false) LGL_DrawLogPause();
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent);
		if(LogEverything==false && LogVisuals==false) LGL_DrawLogPause(false);
	}
	else if(EIGHT_WAY==false)
	{
		Visualizer->SetViewPortVisuals(0.0f,0.5f,0.5f,1.0f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent);
	}
	else
	{
		Visualizer->SetViewPortVisuals(0.00f,0.25f,0.75f,1.0f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent);
		Visualizer->SetViewPortVisuals(0.25f,0.50f,0.75f,1.0f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent);
		Visualizer->SetViewPortVisuals(0.50f,0.75f,0.75f,1.0f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent);
		Visualizer->SetViewPortVisuals(0.75f,1.0f,0.75f,1.0f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent);

		Visualizer->SetViewPortVisuals(0.00f,0.25f,0.50f,0.75f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent);
		Visualizer->SetViewPortVisuals(0.25f,0.50f,0.50f,0.75f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent);
		Visualizer->SetViewPortVisuals(0.50f,0.75f,0.50f,0.75f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent);
		Visualizer->SetViewPortVisuals(0.75f,1.0f,0.50f,0.75f);
		Visualizer->DrawVisuals(visualsQuadrent,visualizerZoomOutPercent);
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
	LGL_DrawLogPause();
	for(int a=0;a<8;a++)
	{
		if(ParticlePointersActive[a])
		{
			ParticlePointers[a]->Draw();
		}
	}
	LGL_DrawLogPause(false);
	if(visualsQuadrent)
	{
		LGL_ClipRectDisable();
	}

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
}

LGL_Image* logo=NULL;

void
DrawLoadScreen()
{
	if(logo==NULL)
	{
		logo = new LGL_Image("data/logo.png");
	}
	float height=0.03f;
	float aspect = LGL_VideoAspectRatio();
	logo->DrawToScreen
	(
		0.5f-0.5f*height,	0.5f+0.5f*height,
		0.5f-0.5f*height*aspect,0.5f+0.5f*height*aspect
	);
	LGL_GetFont().DrawString
	(
		.5,.3,.02,
		1,1,1,1,
		true,
		.75,
		" loading..."
	);
	LGL_SwapBuffers();
}

int main(int argc, char** argv)
{
	//Initialize LGL
	int channels=4;
	bool fullscreen=true;
	bool drawFPS=false;
	float drawFPSSpike=0.0f;
	int resX=9999;
	int resY=9999;

	for(int a=0;a<argc;a++)
	{
		if(strcasecmp(argv[a],"--channels=2")==0)
		{
			channels=2;
		}
		else if(strcasecmp(argv[a],"--channels=4")==0)
		{
			channels=4;
		}
		else if(strcasecmp(argv[a],"--channels=6")==0)
		{
			channels=6;
		}
		else if(strcasecmp(argv[a],"--720p")==0)
		{
			resX=1280;
			resY=720;
		}
		else if(strcasecmp(argv[a],"--480p")==0)
		{
			resX=853;
			resY=480;
		}
		else if(strcasecmp(argv[a],"--drawFPS")==0)
		{
			drawFPS=true;
		}
	}

	InitializeGlobalsPreLGL();

	LGL_Init
	(
		resX,
		resY,
		channels,
		"dvj"
	);

	LGL_MouseVisible(false);

	if(fullscreen)
	{
		//This seems weird, but it keeps us at fullscreen and lets us rotate our Compiz Cube
		//LGL_FullScreenToggle();
	}

	VerifyMusicDir();

	DrawLoadScreen();

	InitializeGlobals();

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

	DrawLoadScreen();
	DrawLoadScreen();

	for(;;)
	{
		DrawFrame(false);
		if(VisualizerZoomOutPercentSmooth!=0.0f)
		{
			float left=0.0f;
			float right=1.0f+1.0f*VisualizerZoomOutPercentSmooth;
			float bottom=0.0f-1.0f*VisualizerZoomOutPercentSmooth;
			float top=1.0f;
			LGL_DrawRectToScreen(0.0f,0.5f,0.5f,1.0f,0,0,0,1);
			LGL_ViewPortScreen
			(
				left,
				right,
				bottom,
				top
			);
			DrawFrame(true,VisualizerZoomOutPercentSmooth);
			LGL_ViewPortScreen();
		}
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
		if(LGL_FPS()<50 || LGL_KeyStroke(SDLK_g))
		{
			drawFPSSpike=5.0f;
		}
		if
		(
			(
				drawFPS ||
				drawFPSSpike > 0.0f
			) &&
			VisualizerFullScreen==false &&
			LGL_SecondsSinceExecution()>15.0f
		)
		{
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
		LGL_SwapBuffers();
	}
}

