/*
 *
 * DrumMachine.cpp
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

#include "LGL.h"
#include "DrumMachine.h"

DrumMachineObj::
DrumMachineObj()
{
	Active=false;
	KitIndex=1;
	if(LGL_DirectoryExists("data/drumMachineKits"))
	{
		DirTree.SetPath("data/drumMachineKits");
	}
	else
	{
		printf("DrumMachineObj::DrumMachineObj(): Error! Folder 'data/drumMachineKits' must exist!\n");
		exit(0);
	}

	for(int a=0;a<2;a++)
	{
		for(int b=0;b<4;b++)
		{
			for(int z=0;z<1024;z++)
			{
				Kit[z][a][b]=NULL;
			}
			sprintf
			(
				KitNames[a][b],
				"%s%s",
				(a==0) ? "right" : "left",
				(b==0) ? ".wav" :
					( (b==1) ? "_a.wav" :
						( (b==2) ? "_b.wav" : "_ab.wav" ))
			);
		}
	}

	KitVolume=1.0f;

	LoadAllKits();
	LoadNewKit();
	PrevKit();
}

DrumMachineObj::
~DrumMachineObj()
{
	DeleteAllKits();
}

void
DrumMachineObj::
NextFrame()
{
	/*
	if(LGL_GetWiimote(1).ButtonStroke(LGL_WIIMOTE_HOME))
	{
		Active=!Active;
	}
	*/

	Active=true;

	if(DirTree.GetDirCount()==0)
	{
		Active=false;
	}

	if(Active==false)
	{
		return;
	}

	//Begin Guitar Hero Controller Code

	if(LGL_JoyStroke(0,LGL_JOY_SELECT))
	{
		PrevKit();
	}
	if(LGL_JoyStroke(0,LGL_JOY_START))
	{
		NextKit();
	}

	int guitarNotes[5];
	guitarNotes[0]=LGL_JOY_GUITAR_GREEN;
	guitarNotes[1]=LGL_JOY_GUITAR_RED;
	guitarNotes[2]=LGL_JOY_GUITAR_YELLOW;
	guitarNotes[3]=LGL_JOY_GUITAR_BLUE;
	guitarNotes[4]=LGL_JOY_GUITAR_ORANGE;

	bool strum=
	(
		GuitarStrumPossible &&
		LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_YAXIS)!=0.0f
	);
	
	for(int n=0;n<5;n++)
	{
		if
		(
			(strum && LGL_JoyDown(0,guitarNotes[n])) ||
			(!strum && LGL_JoyStroke(0,guitarNotes[n]) && GuitarStrumLastTimer.SecondsSinceLastReset()<0.1f)
		)
		{
			Kit[KitIndex][0][n]->Play(KitVolume);
			//printf("Drum: %s\n",Kit[KitIndex][0][n]->GetPath());
		}
	}

	if(strum)
	{
		GuitarStrumLastTimer.Reset();
	}
	GuitarStrumPossible=(LGL_JoyAnalogueStatus(0,LGL_JOY_ANALOGUE_L,LGL_JOY_YAXIS)==0.0f);

	Active=false;

	//Begin Wiimote Controller Code

	/*
	if(LGL_GetWiimote(1).ButtonStroke(LGL_WIIMOTE_MINUS))
	{
		PrevKit();
	}
	if(LGL_GetWiimote(1).ButtonStroke(LGL_WIIMOTE_PLUS))
	{
		NextKit();
	}

	for(int a=0;a<2;a++)
	{
		bool flick = (a==0) ? LGL_GetWiimote(a).GetFlickXPositive() : LGL_GetWiimote(a).GetFlickXNegative();
		if(flick)
		{
			if
			(
				LGL_GetWiimote(a).ButtonDown(LGL_WIIMOTE_A) &&
				LGL_GetWiimote(a).ButtonDown(LGL_WIIMOTE_B)
			)
			{
				if(Kit[KitIndex][a][3]!=NULL && Kit[KitIndex][a][3]->IsLoaded())
				{
					Kit[KitIndex][a][3]->Play(KitVolume);
				}
			}
			else if(LGL_GetWiimote(a).ButtonDown(LGL_WIIMOTE_B))
			{
				if(Kit[KitIndex][a][2]!=NULL && Kit[KitIndex][a][2]->IsLoaded())
				{
					Kit[KitIndex][a][2]->Play(KitVolume);
				}
			}
			else if(LGL_GetWiimote(a).ButtonDown(LGL_WIIMOTE_A))
			{
				if(Kit[KitIndex][a][1]!=NULL && Kit[KitIndex][a][1]->IsLoaded())
				{
					Kit[KitIndex][a][1]->Play(KitVolume);
				}
			}
			else
			{
				if(Kit[KitIndex][a][0]!=NULL && Kit[KitIndex][a][0]->IsLoaded())
				{
					Kit[KitIndex][a][0]->Play(KitVolume);
				}
			}
			LGL_GetWiimote(a).SetRumble(true,0.125f);
		}
	}
	*/
}

void
DrumMachineObj::
Draw
(
	bool	visualizerQuadrent,
	float	visualizerZoomOutPercent
)
{
	if(visualizerQuadrent)
	{
		LGL_ClipRectEnable
		(
			0.0f,
			0.5f,
			0.5f,
			1.0f
		);
	}
	for(int a=0;a<2;a++)
	{
		LGL_Wiimote& wiimote = LGL_GetWiimote(a);
		if(wiimote.Connected())
		{
			float bottomAnchorDelta=-0.1f;

			wiimote.DrawPointerIRSources
			(
				0.925f,0.975f,
				0.500f+.075f*(1-a),
				0.575f+.075f*(1-a)
			);
			//wiimote.DrawAccelGraph(0.5f,0.55f,0.5f+.075f*(1-a),0.575f+.075f*(1-a));
			wiimote.DrawAccelGraph
			(
				0.525f+.250f*(1-a),
				0.525f+.250f*(1-a)+.2f,
				0.9f+bottomAnchorDelta,
				0.95f
			);
			LGL_GetFont().DrawString
			(
				0.525f+.250f*(1-a)+.1f,
				0.965f,
				0.03f,
				1,1,1,1,
				true,
				0.75f,
				(a==0) ? "Right Wiimote" : "Left Wiimote"
			);

			LGL_GetFont().DrawString
			(
				0.525f+.250f*(1-a)+.1f-.125f*0.525f,
				0.875f+bottomAnchorDelta,
				0.02f,
				1,1,1,1,
				true,
				0.75f,
				"X"
			);
			LGL_GetFont().DrawString
			(
				0.525f+.250f*(1-a)+.1f,
				0.875f+bottomAnchorDelta,
				0.02f,
				1,1,1,1,
				true,
				0.75f,
				"Y"
			);
			LGL_GetFont().DrawString
			(
				0.525f+.250f*(1-a)+.1f+.125f*.575f,
				0.875f+bottomAnchorDelta,
				0.02f,
				1,1,1,1,
				true,
				0.75f,
				"Z"
			);

			float active;
			active=wiimote.ButtonDown(LGL_WIIMOTE_A) ? 1.0f : 0.35f;
			LGL_GetFont().DrawString
			(
				0.525f+.25f*(1-a),
				0.825f+bottomAnchorDelta,
				0.03f,
				active,active,active,1,
				true,
				0.75f,
				"[A]"
			);
			active=wiimote.ButtonDown(LGL_WIIMOTE_B) ? 1.0f : 0.35f;
			LGL_GetFont().DrawString
			(
				0.525f+.25f*(1-a),
				0.775f+bottomAnchorDelta,
				0.03f,
				active,active,active,1,
				true,
				0.75f,
				"[B]"
			);

			active=wiimote.ButtonDown(LGL_WIIMOTE_1) ? 1.0f : 0.35f;
			LGL_GetFont().DrawString
			(
				0.575f+.25f*(1-a),
				0.825f+bottomAnchorDelta,
				0.03f,
				active,active,active,1,
				true,
				0.75f,
				"[1]"
			);
			active=wiimote.ButtonDown(LGL_WIIMOTE_2) ? 1.0f : 0.35f;
			LGL_GetFont().DrawString
			(
				0.575f+.25f*(1-a),
				0.775f+bottomAnchorDelta,
				0.03f,
				active,active,active,1,
				true,
				0.75f,
				"[2]"
			);

			active=wiimote.ButtonDown(LGL_WIIMOTE_PLUS) ? 1.0f : 0.35f;
			LGL_GetFont().DrawString
			(
				0.625f+.25f*(1-a),
				0.835f+bottomAnchorDelta,
				0.02f,
				active,active,active,1,
				true,
				0.75f,
				"[+]"
			);
			active=wiimote.ButtonDown(LGL_WIIMOTE_HOME) ? 1.0f : 0.35f;
			LGL_GetFont().DrawString
			(
				0.625f+.25f*(1-a),
				0.805f+bottomAnchorDelta,
				0.02f,
				active,active,active,1,
				true,
				0.75f,
				"[H]"
			);
			active=wiimote.ButtonDown(LGL_WIIMOTE_MINUS) ? 1.0f : 0.35f;
			LGL_GetFont().DrawString
			(
				0.625f+.25f*(1-a),
				0.775f+bottomAnchorDelta,
				0.02f,
				active,active,active,1,
				true,
				0.75f,
				"[-]"
			);

			active=wiimote.ButtonDown(LGL_WIIMOTE_UP) ? 1.0f : 0.35f;
			LGL_GetFont().DrawString
			(
				0.700f+.25f*(1-a),
				0.835f+bottomAnchorDelta,
				0.02f,
				active,active,active,1,
				true,
				0.75f,
				"[U]"
			);
			active=wiimote.ButtonDown(LGL_WIIMOTE_DOWN) ? 1.0f : 0.35f;
			LGL_GetFont().DrawString
			(
				0.700f+.25f*(1-a),
				0.775f+bottomAnchorDelta,
				0.02f,
				active,active,active,1,
				true,
				0.75f,
				"[D]"
			);

			active=wiimote.ButtonDown(LGL_WIIMOTE_LEFT) ? 1.0f : 0.35f;
			LGL_GetFont().DrawString
			(
				0.675f+.25f*(1-a),
				0.805f+bottomAnchorDelta,
				0.02f,
				active,active,active,1,
				true,
				0.75f,
				"[L]"
			);
			active=wiimote.ButtonDown(LGL_WIIMOTE_RIGHT) ? 1.0f : 0.35f;
			LGL_GetFont().DrawString
			(
				0.725f+.25f*(1-a),
				0.805f+bottomAnchorDelta,
				0.02f,
				active,active,active,1,
				true,
				0.75f,
				"[R]"
			);
		}
		else if(wiimote.IsListeningForConnection())
		{
			//
		}
		else
		{
			//
		}
	}
	if(visualizerQuadrent)
	{
		LGL_ClipRectDisable();
	}
}

void
DrumMachineObj::
NextKit()
{
	KitIndex++;
	if(KitIndex>=(int)DirTree.GetDirCount())
	{
		KitIndex=1;
	}
}

void
DrumMachineObj::
PrevKit()
{
	KitIndex--;
	if(KitIndex<1)
	{
		KitIndex=DirTree.GetDirCount()-1;
	}
	if(KitIndex<1)
	{
		KitIndex=1;
	}
}

const
char*
DrumMachineObj::
GetKitName()
{
	if(Active && DirTree.GetDirCount()>1)
	{
		return(DirTree.GetDirName(KitIndex));
	}
	else
	{
		return("[Inactive]");
	}
}

bool
DrumMachineObj::
GetActive()
{
	return(Active);
}

void
DrumMachineObj::
LoadAllKits()
{
	for(unsigned int a=1;a<DirTree.GetDirCount();a++)
	{
		KitIndex=a;
		LoadNewKit();
	}
}

bool
DrumMachineObj::
LoadNewKit()
{
	if(DirTree.GetDirCount()==0)
	{
		Active=false;
		return(false);
	}

	bool allGood=true;

	char target[1024];
	for(int a=0;a<2;a++)
	{
		for(int b=0;b<4;b++)
		{
			sprintf(target,"%s/%s/%s",DirTree.GetPath(),DirTree.GetDirName(KitIndex),KitNames[a][b]);
			if(LGL_FileExists(target))
			{
				Kit[KitIndex][a][b]=new LGL_Sound(target);
			}
			else
			{
				printf("DrumMachineObj::LoadNewKit()[%i][%i]: Warning! Couldn't load '%s'!\n",a,b,target);
				allGood=false;
			}
		}
	}

	return(allGood);
}

void
DrumMachineObj::
DeleteAllKits()
{
	for(int a=0;a<2;a++)
	{
		for(int b=0;b<4;b++)
		{
			for(int z=0;z<1024;z++)
			{
				if(Kit[z][a][b]!=NULL)
				{
					delete Kit[z][a][b];
					Kit[z][a][b]=NULL;
				}
			}
		}
	}
}

