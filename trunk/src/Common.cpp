/*
 *
 * Common.cpp
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

#include "Common.h"
#include "Config.h"

#define	NEEDLE_DISTANCE_FROM_EDGES (0.0f)

#define SAMPLE_RADIUS_MULTIPLIER (2.0f)

LGL_Image* logo=NULL;

void
DrawLoadScreen
(
	float		loadScreenPercent,
	const char*	line1,
	const char*	line2,
	const char*	line3,
	float		line3Brightness
)
{
	char loadScreenPath[2048];

	//Try user-specified (defaults to ~/.dvj/data/image/loadscreen.png
	GetLoadScreenPath(loadScreenPath);
	if(LGL_FileExists(loadScreenPath)==false)
	{
		//Fall back on default, which might not exist...
		strcpy(loadScreenPath,"data/image/loadscreen.png");
	}

	if(LGL_FileExists(loadScreenPath))
	{
		if(logo==NULL)
		{
			logo = new LGL_Image(loadScreenPath);
		}
		logo->InvertY=1;	//WHY??
		logo->DrawToScreen();
	}
	else
	{
		if(logo==NULL)
		{
			logo = new LGL_Image("data/image/logo.png");
		}
		float height=0.03f;
		float aspect = LGL_DisplayAspectRatio();
		logo->DrawToScreen
		(
			0.5f-0.5f*height,	0.5f+0.5f*height,
			0.5f-0.5f*height*aspect,0.5f+0.5f*height*aspect
		);
		if(line1)
		{
			LGL_GetFont().DrawString
			(
				.5,.3,.02,
				1,1,1,1,
				true,
				.75,
				line1
			);
		}
	}

	float pct=LGL_Min(1.0f,loadScreenPercent);
	if(line2)
	{
		float fontHeight=0.02f;
		float fontWidth=LGL_GetFont().GetWidthString(fontHeight,line2);
		float fontWidthMax=0.95f;
		fontHeight=LGL_Min(fontHeight,fontHeight*fontWidthMax/fontWidth);
		LGL_GetFont().DrawString
		(
			.5f,.1f,fontHeight,
			1,1,1,1,
			true,
			.75f,
			line2
		);
	}
	if(line3)
	{
		float fontHeight=0.015f;
		float fontWidth=LGL_GetFont().GetWidthString(fontHeight,line3);
		float fontWidthMax=0.95f;
		fontHeight=LGL_Min(fontHeight,fontHeight*fontWidthMax/fontWidth);
		LGL_GetFont().DrawString
		(
			.5f,.1-0.03f,fontHeight,
			line3Brightness,
			line3Brightness,
			line3Brightness,
			line3Brightness,
			true,
			.75,
			line3
		);
	}

	float coolR;
	float coolG;
	float coolB;
	GetColorCool(coolR,coolG,coolB);
	float warmR;
	float warmG;
	float warmB;
	GetColorWarm(warmR,warmG,warmB);

	float glow = 1.0f;	//GetGlowFromTime(LGL_FramesSinceExecution()/60.0f);
	float brW = pct;
	float brC = pct * (1.0f-brW);
	if(loadScreenPercent>0.0f)
	{
		LGL_DrawRectToScreen
		(
			0,pct/2.0f,
			0,0.05f,
			brC*coolR*glow + brW*warmR*glow,
			brC*coolG*glow + brW*warmG*glow,
			brC*coolB*glow + brW*warmB*glow,
			1.0f
		);
		LGL_DrawRectToScreen
		(
			1.0f-pct/2.0f,1.0f,
			0,0.05f,
			brC*coolR*glow + brW*warmR*glow,
			brC*coolG*glow + brW*warmG*glow,
			brC*coolB*glow + brW*warmB*glow,
			1.0f
		);
	}
	if(loadScreenPercent<1.0f)
	{
		LGL_DrawLineToScreen
		(
			0.5f,0.0f,
			0.5f,0.05f,
			0,0,0,1,
			1.0f
		);
	}
	LGL_SwapBuffers();
	LGL_ProcessInput();
}

float
GetGlowFromTime
(
	float	time
)
{
	return((2.5f+sin(LGL_PI*time))/3.5f);
}

void
Main_DrawGlowLines
(
	float	time,
	float	brightness,
	bool	visualizerQuadrent,
	float	visualizerZoomOutPercent,
	float	visualizerRight
)
{
	float coolR;
	float coolG;
	float coolB;
	GetColorCool(coolR,coolG,coolB);

	float glow = GetGlowFromTime(time) * brightness;
	//float quadrentSplitX = visualizerRight;
	float quadrentSplitY = 0.5f;

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

	for(float a=1;a<7;a++)
	{
		float br=glow*a/7.0f;
		LGL_ClipRectEnable
		(
			0.0f,
			1.0f,
			0.0f,
			0.5f
		);
		LGL_DrawLineToScreen
		(
			0,quadrentSplitY,
			1,quadrentSplitY,
			2*coolR*br,2*coolG*br,2*coolB*br,0.5f*br,
			7-a,
			false
		);
		LGL_ClipRectDisable();
		LGL_ClipRectEnable
		(
			0.0f,
			1.0f,
			quadrentSplitY,
			1.0f
		);
		if(LGL_DisplayCount()>1)
		{
			//Draw projector-AR indicator lines
			float myL = GetVisualizer()->GetViewportVisualsL();
			float myR = GetVisualizer()->GetViewportVisualsR();
			float myB = GetVisualizer()->GetViewportVisualsB();
			float myT = GetVisualizer()->GetViewportVisualsT();

			GetVisualizer()->GetProjectorARCoordsFromViewportCoords
			(
				myL,
				myR,
				myB,
				myT
			);

			LGL_DrawLineToScreen
			(
				myL,myB,
				myL,myT,
				2*coolR*br,2*coolG*br,2*coolB*br,0.5f*br,
				7-a,
				false
			);
			LGL_DrawLineToScreen
			(
				myR,myB,
				myR,myT,
				2*coolR*br,2*coolG*br,2*coolB*br,0.5f*br,
				7-a,
				false
			);
		}
		LGL_ClipRectDisable();
	}
	if(visualizerQuadrent)
	{
		LGL_ClipRectDisable();
	}
}

void
Mixer_DrawGlowLinesTurntables
(
	float	time,
	float	crossFadeSliderLeft,
	float	crossFadeSliderRight,
	float	brightness,
	bool	visualizerQuadrent,
	float	visualizerZoomOutPercent
)
{
	float coolR;
	float coolG;
	float coolB;
	GetColorCool(coolR,coolG,coolB);

	float warmR;
	float warmG;
	float warmB;
	GetColorWarm(warmR,warmG,warmB);

	float glow = GetGlowFromTime(time) * brightness;
	float viewportLeft = 0.0f;
	float viewportRight = 1.0f;
	float viewportBottom = 0.0f;
	float viewportTop = 0.5f;
	//float viewportWidth = viewportRight - viewportLeft;
	float viewportHeight = viewportTop - viewportBottom;

	const float crossFadeSliderWidth=0.025f;

	for(int a=0;a<2;a++)
	{
		const float cfLeft = (a==0) ? 0 : (1.0f-crossFadeSliderWidth);
		const float cfRight = (a==0) ? crossFadeSliderWidth : 1.0f;
		const float cfBottom=viewportBottom;
		const float cfTop=viewportTop;
		bool justSet=false;
		if
		(
			LGL_MouseX()>=cfLeft &&
			LGL_MouseX()<=cfRight &&
			LGL_MouseY()>=cfBottom &&
			LGL_MouseY()<=cfTop
		)
		{
			GetInputMouse().SetHoverElement
			(
				(a==0) ?
				GUI_ELEMENT_XFADER_LEFT :
				GUI_ELEMENT_XFADER_RIGHT
			);
			if(LGL_MouseStroke(LGL_MOUSE_LEFT))
			{
				GetInputMouse().SetDragTarget(TARGET_NONE);
				GetInputMouse().SetDragElement
				(
					(a==0) ?
					GUI_ELEMENT_XFADER_LEFT :
					GUI_ELEMENT_XFADER_RIGHT
				);
				justSet=true;
			}
		}

		if
		(
			justSet ||
			(
				LGL_MouseMotion() &&
				GetInputMouse().GetDragElement() ==
				(
					(a==0) ?
					GUI_ELEMENT_XFADER_LEFT :
					GUI_ELEMENT_XFADER_RIGHT
				)
			)
		)
		{
			GetInputMouse().SetDragFloatNext
			(
				LGL_Clamp
				(
					0.0f,
					(LGL_MouseY()-cfBottom)/
					(cfTop-cfBottom),
					1.0f
				)
			);
		}
	}

	for(float a=1;a<7;a++)
	{
		float br=glow*a/7.0f;
		LGL_ClipRectEnable
		(
			visualizerQuadrent?0.0f:viewportLeft,
			visualizerQuadrent?0.5f:viewportRight,
			visualizerQuadrent?0.5f:viewportBottom,
			visualizerQuadrent?1.0f:viewportTop
		);
		LGL_DrawLineToScreen
		(
			0.025,viewportBottom+.5*viewportHeight,
			0.975,viewportBottom+.5*viewportHeight,
			2*coolR*br,2*coolG*br,2*coolB*br,br,
			7-a,
			false
		);
		for(int zz=1;zz<=5;zz++)
		{
			LGL_DrawLineToScreen
			(
				0,crossFadeSliderLeft*viewportTop,
				0+crossFadeSliderWidth,crossFadeSliderLeft*viewportTop,
				warmR*br,warmG*br,warmB*br,br,
				zz*(7-a),
				false
			);
			LGL_DrawLineToScreen
			(
				1.0f-crossFadeSliderWidth,crossFadeSliderRight*viewportTop,
				1,crossFadeSliderRight*viewportTop,
				warmR*br,warmG*br,warmB*br,br,
				zz*(7-a),
				false
			);
		}
		if(visualizerQuadrent==false) LGL_ClipRectDisable();
		LGL_DrawLineToScreen
		(
			0.975,viewportTop,
			0.975,0,
			2*coolR*br,2*coolG*br,2*coolB*br,br,
			7-a,
			false
		);
		LGL_DrawLineToScreen
		(
			0.025,viewportTop,
			0.025,0,
			2*coolR*br,2*coolG*br,2*coolB*br,br,
			7-a,
			false
		);
		if(visualizerQuadrent==true) LGL_ClipRectDisable();
	}
}

void
Mixer_DrawGlowLinesStatus
(
	float	time,
	float	brightness,
	bool	visualizerQuadrent,
	float	visualizerZoomOutPercent
)
{
	float coolR;
	float coolG;
	float coolB;
	GetColorCool(coolR,coolG,coolB);

	float glow = GetGlowFromTime(time) * brightness;
	float l=0.5f;
	float r=1.0f;
	float b=0.5f;
	//float t=1.0f;
	float w=r-l;
	//float h=t-b;

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

	for(float a=1;a<7;a++)
	{
		float br=glow*a/7.0f;

		//Two long horizontal lines
		for(int c=1;c<3;c++)
		{
			LGL_DrawLineToScreen
			(
				l,b+0.075f*c,
				r,b+0.075f*c,
				2*coolR*br,2*coolG*br,2*coolB*br,br,
				7-a,
				false
			);
		}

		//Line to left of VU
		LGL_DrawLineToScreen
		(
			r-0.025f,b,
			r-0.025f,b+0.075f*2,
			2*coolR*br,2*coolG*br,2*coolB*br,br,
			7-a,
			false
		);

		//Line to left of Wave
		LGL_DrawLineToScreen
		(
			r-0.075f,b,
			r-0.075f,b+0.075f*2,
			2*coolR*br,2*coolG*br,2*coolB*br,br,
			7-a,
			false
		);

		//Line to right of Spectrum (Spectrum on far left)
		LGL_DrawLineToScreen
		(
			l+0.05f,b,
			l+0.05f,b+0.075f*2,
			2*coolR*br,2*coolG*br,2*coolB*br,br,
			7-a,
			false
		);
		
		//Line to the Top, splitting wiimotes
		LGL_DrawLineToScreen
		(
			l+0.5f*w,b+0.15f,
			l+0.5f*w,1.0f,
			2*coolR*br,2*coolG*br,2*coolB*br,br,
			7-a,
			false
		);
	}

	/*
	LGL_GetFont().DrawString
	(
		l+.475*w,b+0.1f,0.02f,
		brightness,brightness,brightness,1,
		true,
		1.0f,
		"interim.descriptor at gmail.com"
	);

	LGL_GetFont().DrawString
	(
		l+.475f*w,b+0.025f,0.02f,
		brightness,brightness,brightness,1,
		true,
		1.0f,
		"interimdescriptor.blogspot.org"
	);
	*/
	
	if(visualizerQuadrent)
	{
		LGL_ClipRectDisable();
	}
}

void
Mixer_DrawLevels
(
	float viewportBottom,
	float viewportTop,
	float leftBottomLevel,
	float leftTopLevel,
	float rightBottomLevel,
	float rightTopLevel,
	bool  visualizerQuadrent,
	float visualizerZoomOutPercent
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
	float viewportHeight=viewportTop-viewportBottom;
	for(int a=0;a<2;a++)
	{
		/*
		LGL_GetFont().DrawString
		(
			.025*.5,
			viewportBottom+.245*viewportHeight+(1.5)*.5*viewportHeight,
			.010,
			1,1,1,1,
			true,.5,
			"%.0f",
			LGL_Round(100.0*CrossFadeMiddle)
		);
		*/
		LGL_GetFont().DrawString
		(
			.025f*.5f,
			viewportBottom+.245f*viewportHeight+(1-a)*.5f*viewportHeight,
			.010f,
			1,1,1,1,
			true,.5f,
			"%.0f",
			(a==1) ? leftBottomLevel : leftTopLevel
		);
		LGL_GetFont().DrawString
		(
			1.0f-.025f*.5f,
			viewportBottom+.245f*viewportHeight+(1-a)*.5f*viewportHeight,
			.010f,
			1,1,1,1,
			true,.5f,
			"%.0f",
			(a==1) ? rightBottomLevel : rightTopLevel
		);
	}
	if(visualizerQuadrent)
	{
		LGL_ClipRectDisable();
	}
}

void
Turntable_DrawDirTree
(
	int		which,
	float		time,
	const char*	filterText,
	const char*	path,
	const char**	nameArray,
	bool*		isDirBits,
	bool*		loadableBits,
	bool*		alreadyPlayedBits,
	int		fileNum,
	int		fileSelectInt,
	float		viewportBottom,
	float		viewportTop,
	float		badFileFlash,
	float*		inBPMList
)
{
	if(filterText==NULL)
	{
		filterText="";
	}
	if(path==NULL)
	{
		path="";
	}
	if(nameArray==NULL)
	{
		return;
	}

	float coolR;
	float coolG;
	float coolB;
	GetColorCool(coolR,coolG,coolB);

	float glow = GetGlowFromTime(time);
	float centerX = 0.5f;
	float viewportLeft = 0.025f;
	float viewportRight = 0.975f;
	float viewportWidth = viewportRight - viewportLeft;
	float viewportHeight = viewportTop - viewportBottom;
	LGL_GetFont().DrawString
	(
		centerX,viewportBottom+.875*viewportHeight,.025,
		1,1,1,1,
		true,.5,
		filterText[0]=='\0' ?
		path :
		filterText
	);

	for
	(
		int b=0;
		b<5 && b<fileNum;
		b++
	)
	{
		const char* fileNow=nameArray[b];
		if(fileNow==NULL)
		{
			continue;
		}
		
		float R=1.0f;
		float G=1.0f;
		float B=1.0f;
		if(loadableBits[b]==false)
		{
			R=1.0f;
			G=0.0f;
			B=0.0f;
		}
		else if(isDirBits[b])
		{
			R=0.0f;
			G=0.0f;
			B=1.0f;
		}
		else if(alreadyPlayedBits[b])
		{
			R=0.25f;
			G=0.25f;
			B=0.25f;
		}

		if(strlen(fileNow)>0)
		{
			float rectLeft=viewportLeft;
			float rectRight=viewportRight;
			float rectBottom=viewportTop-(.1f+(b+1)/6.25f+.02f)*viewportHeight;
			float rectTop=viewportTop-(.1f+(b+1)/6.25f+.02f)*viewportHeight+viewportHeight/15.0f+.04f*viewportHeight;

			if
			(
				LGL_MouseX()>=rectLeft &&
				LGL_MouseX()<=rectRight &&
				LGL_MouseY()>=rectBottom &&
				LGL_MouseY()<=rectTop
			)
			{
				if(LGL_MouseMotion())
				{
					GetInputMouse().SetFileIndexHighlightNext(b);
				}
			}

			if(b==fileSelectInt)
			{
				float R=
					(0.0f+badFileFlash) +
					(1.0f-badFileFlash) * coolR * 0.6f * glow;
				float G=(1.0f-badFileFlash) * coolG * 0.6f * glow;
				float B=(1.0f-badFileFlash) * coolB * 0.6f * glow;
				float A=.5f;
				LGL_DrawRectToScreen
				(
					rectLeft,rectRight,
					rectBottom,rectTop,
					R,G,B,A
				);
			}

			float stringY=viewportTop-0.1f*viewportHeight-(viewportHeight*((b+1)/6.25f-0.035f));

			float bpm = inBPMList[b];
			if(bpm>0)
			{
				LGL_GetFont().DrawString
				(
					viewportLeft+0.01f*viewportWidth,
					stringY-0.5f*viewportHeight/15.0f,
					viewportHeight/15.0f,
					R,G,B,1,
					false,0,
					"%.0f",
					bpm
				);
			}

			float fontHeight=viewportHeight/15.0f;
			float fontWidth=LGL_GetFont().GetWidthString(fontHeight,fileNow);
			float fontWidthMax=viewportWidth*0.9f;
			fontHeight=LGL_Min(fontHeight,fontHeight*fontWidthMax/fontWidth);

			char fileNowSafe[2048];
			strcpy(fileNowSafe,fileNow);
			const int fileNowSafeLen = strlen(fileNowSafe);
			for(int s=0;s<fileNowSafeLen;s++)
			{
				if(fileNowSafe[s]=='%')
				{
					fileNowSafe[s]=' ';
				}
			}
			LGL_GetFont().DrawString
			(
				viewportLeft+(.025f+0.05f)*viewportWidth,
				stringY-0.5f*fontHeight,
				fontHeight,
				R,G,B,1,
				false,0,
				fileNowSafe
			);
		}
	}
}

void
turntable_DrawBPMLines
(
	float	leftSample,
	float	rightSample,
	float	soundLengthSeconds,
	float	bpmFirstBeatSeconds,
	float	secondsPerBeat,
	float	pointBottom,
	float	pointTop,
	float	wavLeft,
	float	wavWidth
)
{
	float widthSample=rightSample-leftSample;
	float pointHeight = pointTop-pointBottom;

	int whichBeat=1;
	int whichMeasure=1;
	double startSeconds = bpmFirstBeatSeconds;
	if(startSeconds > -secondsPerBeat*4) whichMeasure=-1;
	while(startSeconds > -secondsPerBeat*4)
	{
		startSeconds-=secondsPerBeat*4;
		whichMeasure-=1;
	}

	const float shadowOffset=0.0025f;

	for(double beatSeconds=startSeconds;beatSeconds<soundLengthSeconds;beatSeconds+=secondsPerBeat)
	{
		long bpmPointSamples = (long)(beatSeconds*44100);
		if
		(
			bpmPointSamples >= 0 &&
			bpmPointSamples >= leftSample-(44100/4) &&
			bpmPointSamples <= rightSample+(44100/4)
		)
		{
			float bpmPointPercent = (bpmPointSamples-leftSample)/(float)widthSample;
			bool b1=(whichBeat==1);

			int big=3;
			int small=3;

			float len = b1?0.2f:0.1f;

			LGL_DrawLineToScreen
			(
				wavLeft+wavWidth*bpmPointPercent+shadowOffset,pointBottom+pointHeight*0.0f-shadowOffset,
				wavLeft+wavWidth*bpmPointPercent+shadowOffset,pointBottom+pointHeight*len-shadowOffset,
				0,0,0,0.9f,
				b1?big:small
			);
			LGL_DrawLineToScreen
			(
				wavLeft+wavWidth*bpmPointPercent,pointBottom+pointHeight*0.0f,
				wavLeft+wavWidth*bpmPointPercent,pointBottom+pointHeight*len,
				1,1,1,1,
				b1?big:small
			);

			LGL_DrawLineToScreen
			(
				wavLeft+wavWidth*bpmPointPercent+shadowOffset,pointTop-pointHeight*0.0f-shadowOffset,
				wavLeft+wavWidth*bpmPointPercent+shadowOffset,pointTop-pointHeight*len-shadowOffset,
				0,0,0,0.9f,
				b1?big:small
			);
			LGL_DrawLineToScreen
			(
				wavLeft+wavWidth*bpmPointPercent,pointTop-pointHeight*0.0f,
				wavLeft+wavWidth*bpmPointPercent,pointTop-pointHeight*len,
				1,1,1,1,
				b1?big:small
			);

			float fontHeight=0.10f*pointHeight;
			float lDelta=0.01f;
			//float lrDelta=0.020f;
			//float dashDelta=0.001f;
			if(whichBeat==1)
			{
				LGL_GetFont().DrawString
				(
					wavLeft+wavWidth*bpmPointPercent+lDelta,
					pointBottom+0.1f*pointHeight-0.5f*fontHeight,
					fontHeight,
					1,1,1,1,
					false,
					0.9f,
					"%i",
					whichMeasure
				);
			}
		}
		else if(bpmPointSamples > rightSample)
		{
			break;
		}
		whichBeat++;
		if(whichBeat==5)
		{
			whichBeat=1;
			whichMeasure++;
			if(whichMeasure==-1) whichMeasure=1;
		}
	}
}

void
DrawWarpPointLineOrRect
(
	float		soundPositionSamples,
	float		sampleRadiusMultiplier,
	float		warpPointSecondsStart,
	float		warpPointSecondsTrigger,
	float		pointLeft,
	float		pointWidth,
	float		pointBottom,
	float		pointTop,
	float		warmR,
	float		warmG,
	float		warmB,
	LGL_Image*	noiseImage256x64
)
{
	//Draw warp line (or rect)!
	float centerSample=soundPositionSamples;
	float leftSample=centerSample-(64*512*sampleRadiusMultiplier);
	float rightSample=centerSample+(64*512*sampleRadiusMultiplier);
	float warpSample=44100*warpPointSecondsTrigger;

	float screenCenter=pointLeft + pointWidth*((warpSample-leftSample)/(rightSample-leftSample));
	float screenRadius=0.0025f;
	float screenLeft=screenCenter-screenRadius;
	float screenRight=screenCenter+screenRadius;
	if(warpPointSecondsStart!=-1.0f)
	{
		screenRight = LGL_Clamp(0.0f,screenCenter,1.0f);
		screenLeft = LGL_Clamp(0.0f,pointLeft + pointWidth*((warpPointSecondsStart*44100.0f-leftSample)/(rightSample-leftSample)),1.0f);
	}
	noiseImage256x64->DrawToScreen
	(
		screenLeft,
		screenRight,
		pointBottom,
		pointTop,
		0,
		warmR,warmG,warmB,0.0f
	);
}

const long pointResolutionMax=(1920*2)+1;
float arrayV[pointResolutionMax*2];
float arrayC[pointResolutionMax*4];
bool overdriven[pointResolutionMax];

float arrayVtri[pointResolutionMax*2*2];
float arrayCtri[pointResolutionMax*4*2];
float entireWaveArrayLine1Points[pointResolutionMax*2];
float entireWaveArrayLine2Points[pointResolutionMax*2];
float entireWaveArrayTriPoints[pointResolutionMax*2*2];
float entireWaveArrayLine1Colors[pointResolutionMax*4];
float entireWaveArrayLine2Colors[pointResolutionMax*4];
float entireWaveArrayTriColors[pointResolutionMax*4*2];

void
Turntable_DrawWaveform
(
	int		which,
	LGL_Sound*	sound,
	bool		loaded,
	const char*	videoPathShort,
	bool		glitch,
	float		glitchBegin,
	float		glitchLength,
	double		soundPositionSamples,
	double		soundLengthSamples,
	float		soundSpeed,
	float		pitchBend,
	float		grainStreamCrossfader,
	float		grainStreamSourcePoint,
	float		grainStreamLength,
	float		grainStreamPitch,
	float		viewportLeft,
	float		viewportRight,
	float		viewportBottom,
	float		viewportTop,
	float		volumeMultiplierNow,
	float		centerX,
	bool		pause,
	float		nudge,
	float		joyAnalogueStatusLeftX,
	float		time,
	double*		savePointSeconds,
	int		savePointIndex,
	int		savePointIndexActual,
	unsigned int	savePointSetBitfield,
	float*		savePointUnsetNoisePercent,
	float*		savePointUnsetFlashPercent,
	float		bpm,
	float		bpmAdjusted,
	float		bpmFirstBeatSeconds,
	float		eq0,
	float		eq1,
	float		eq2,
	bool		lowRez,
	int&		entireWaveArrayFillIndex,
	int		entireWaveArrayCount,
	float*		entireWaveArrayMagnitudeAve,
	float*		entireWaveArrayMagnitudeMax,
	float*		entireWaveArrayFreqFactor,
	float		cachedLengthSeconds,
	LGL_Image*	noiseImage256x64,
	LGL_Image*	loopImage,
	bool		audioInputMode,
	float		warpPointSecondsStart,
	float		warpPointSecondsTrigger,
	int		loopExponent,
	float		loopSeconds,
	bool		waveformRecordHold,
	const char*	soundName,
	float		videoSecondsBufferedLeft,
	float		videoSecondsBufferedRight,
	float		videoSecondsLoadedLeft,
	float		videoSecondsLoadedRight,
	bool		isMaster,
	bool		rapidVolumeInvertSelf,
	float		beginningOfCurrentMeasureSeconds,
	float		videoBrightness,
	float		oscilloscopeBrightness,
	float		freqSenseBrightness,
	float		freqSenseLEDBrightness,
	float		freqSenseLEDColorScalarLow,
	float		freqSenseLEDColorScalarHigh,
	float		freqSenseLEDGroupFloat,
	int		freqSenseLEDGroupInt,
	int		channel,
	float		recallPos
)
{
	float coolR;
	float coolG;
	float coolB;
	GetColorCool(coolR,coolG,coolB);

	float warmR;
	float warmG;
	float warmB;
	GetColorWarm(warmR,warmG,warmB);

	float glow = GetGlowFromTime(time);

	//Ensure our sound is sufficiently loaded
	while(sound->GetLengthSamples() < soundLengthSamples)
	{
		//This can only trigger in logDrawer, so it's safe
		LGL_DelayMS(50);
		printf("waiting for sound to load... %li vs %li\n",(long int)sound->GetLengthSamples(), (long int)soundLengthSamples);
	}

	//Prepare some derived varibales
	float viewportWidth = viewportRight - viewportLeft;
	float viewportHeight = viewportTop - viewportBottom;
	float soundLengthSamplesHalf = 0.5f*soundLengthSamples;
	float soundLengthSeconds = soundLengthSamples / sound->GetHz();
	float soundPositionSeconds = soundPositionSamples / sound->GetHz();
	soundPositionSamples = soundPositionSamples * 44100.0f/sound->GetHz();
	glitchBegin=glitchBegin*44100.0f/sound->GetHz();
	glitchLength=glitchLength*44100.0f/sound->GetHz();

	//Draw Waveform

	float needleDeltaL=
		(0.0f+grainStreamCrossfader) * -(0.0f/4.0f)*grainStreamLength +
		(1.0f-grainStreamCrossfader) * -0.005f;
	float needleDeltaR=
		(0.0f+grainStreamCrossfader) * (3.0f/4.0f)*grainStreamLength +
		(1.0f-grainStreamCrossfader) * 0.005f;

	float sampleRadiusMultiplier=SAMPLE_RADIUS_MULTIPLIER;
	
	needleDeltaL/=sampleRadiusMultiplier;
	needleDeltaR/=sampleRadiusMultiplier;

	if(loaded)
	{
		while(sound->IsLoaded() == false)
		{
			LGL_DelayMS(50);
		}
	}

	const long xFudgeFactor = (long)(0.0175f*44100);
	long pos=(long)soundPositionSamples - xFudgeFactor;

	//Smooth Zooming Waveform Renderer

	float zoom=4444.0f;
	if(pitchBend!=0.0f)
	{
		zoom=1.0f/pitchBend;
	}

	long pointResolution=256+1;

	float pointRadiusMin=viewportRight-(viewportLeft+0.5f*viewportWidth);
	float pointRadius=pointRadiusMin*zoom;

	float percentTowardsNextZoomInLevel=(pointRadius-pointRadiusMin)/pointRadiusMin;

	int sampleRadius=(int)(512*64*SAMPLE_RADIUS_MULTIPLIER);	//This number is arbitrary and possibly magical for an unknown reason.
	long sampleLeft=pos-sampleRadius;
	long sampleRight=pos+sampleRadius;
	long sampleWidth=sampleRight-sampleLeft;

	float viewportCenter=viewportLeft+0.5f*viewportWidth;
	float pointLeft=	viewportCenter-(0.5f+0.5f*percentTowardsNextZoomInLevel)*(WAVE_WIDTH_PERCENT*viewportWidth);
	float pointRight=	viewportCenter+(0.5f+0.5f*percentTowardsNextZoomInLevel)*(WAVE_WIDTH_PERCENT*viewportWidth);
	float pointWidth=	pointRight-pointLeft;
	float pointBottom=	viewportBottom+0.125*viewportHeight;
	float pointTop=		viewportBottom+0.875*viewportHeight;
	float pointHeight=	pointTop-pointBottom;
	float pointYMid=	0.5f*(pointBottom+pointTop);
	
	float wavLeft = viewportCenter-0.5f*WAVE_WIDTH_PERCENT*viewportWidth;
	float wavRight=	viewportCenter+0.5f*WAVE_WIDTH_PERCENT*viewportWidth;
	float wavWidth= wavRight-wavLeft;

	LGL_ClipRectEnable
	(
		wavLeft,
		wavRight,
		pointBottom,
		pointTop+0.01f
	);

	if
	(
		LGL_MouseX()>=wavLeft &&
		LGL_MouseX()<=wavRight &&
		LGL_MouseY()>=pointBottom &&
		LGL_MouseY()<=pointTop
	)
	{
		GetInputMouse().SetHoverElement(GUI_ELEMENT_WAVEFORM);
		if(LGL_MouseStroke(LGL_MOUSE_LEFT))
		{
			GetInputMouse().SetDragTarget((which==0) ? TARGET_TOP : TARGET_BOTTOM);
			GetInputMouse().SetDragElement(GUI_ELEMENT_WAVEFORM);
		}
	}

	if(audioInputMode)
	{
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
			GetFreqBrightness(false,freqFactor,2*volAve*volumeMultiplierNow,eq0) ||
			GetFreqBrightness(true,freqFactor,2*volAve*volumeMultiplierNow,eq2)
		)
		{
			LGL_DrawAudioInWaveform
			(
				pointLeft,
				pointRight,
				pointYMid-0.5f*volumeMultiplierNow*pointHeight,
				pointYMid+0.5f*volumeMultiplierNow*pointHeight,
				red,green,blue,0.0f,
				3.0f,
				true
			);
		}
	}
	else
	{
		//Figure out Glitch Lines
		bool glitchLines=false;
		float glitchLinesLeft=-1;
		float glitchLinesRight=-1;
		float glitchSampleLeft=-1;
		float glitchSampleRight=-1;
		if
		(
			glitchBegin >= 0 &&
			glitch
		)
		{
			float gleftSample=glitchBegin-glitchLength;
			float grightSample=glitchBegin+glitchLength;

			float centerSample=soundPositionSamples;
			float leftSample=centerSample-(64*512*pitchBend*sampleRadiusMultiplier);
			float rightSample=centerSample+(64*512*pitchBend*sampleRadiusMultiplier);

			if
			(
				leftSample<0 ||
				rightSample>soundLengthSamples
			)
			{
				//Deal with wrap around

				if(centerSample<soundLengthSamplesHalf)
				{
					if(gleftSample>soundLengthSamplesHalf)
					{
						gleftSample-=soundLengthSamples;
					}
					if(grightSample>soundLengthSamplesHalf)
					{
						grightSample-=soundLengthSamples;
					}
				}
				else
				{
					if(gleftSample<soundLengthSamplesHalf)
					{
						gleftSample+=soundLengthSamples;
					}
					if(grightSample<soundLengthSamplesHalf)
					{
						grightSample+=soundLengthSamples;
					}
				}
			}

			float gLeftPercent=1.0f-(rightSample-gleftSample)/(rightSample-leftSample);
			float gRightPercent=1.0f-(rightSample-grightSample)/(rightSample-leftSample);

			glitchLines=true;
			glitchLinesLeft=	pointLeft+gLeftPercent*pointWidth;
			glitchLinesRight=	pointLeft+gRightPercent*pointWidth;
			glitchSampleLeft=gleftSample;
			glitchSampleRight=grightSample;
		}

		long sampleLeftExact=sampleLeft;
		double deltaSample=sampleWidth/(double)pointResolution;
		deltaSample*=1024;
		deltaSample=LGL_NextPowerOfTwo(deltaSample);
		deltaSample/=1024;
		long deltaSampleLong=(long)deltaSample;
		if(deltaSampleLong<1) deltaSampleLong=1;

		long sampleLeftBase=sampleLeftExact;
		sampleLeftBase/=(deltaSampleLong*2);
		sampleLeftBase*=(deltaSampleLong*2);
		//bool sampleLeftBaseIsOdd=(sampleLeftBase%(deltaSampleLong*2))!=0;

		int pointsToDrawIndexStart=0;
		int pointsToDrawIndexEnd=pointResolution;

		float vertLeft = wavLeft-0.05f;
		float vertRight= wavRight+0.05f;

		if
		(
			warpPointSecondsTrigger>0.0f &&
			warpPointSecondsStart!=-1.0f
		)
		{
			DrawWarpPointLineOrRect
			(
				soundPositionSamples,
				sampleRadiusMultiplier,
				warpPointSecondsStart,
				warpPointSecondsTrigger,
				pointLeft,
				pointWidth,
				pointBottom,
				pointTop,
				warmR*1.0f,
				warmG*1.0f,
				warmB*1.0f,
				noiseImage256x64
			);
		}

		int localExponent = loopExponent;
		if(localExponent<-7)
		{
			localExponent=-7;
		}
		double secondsPerBeat = (bpm!=0) ? (60.0/bpm) : 0;
		double secondsPerLoopPeriod = pow(2,localExponent)*(secondsPerBeat*4);
		long bpmFirstBeatCurrentMeasureSamples = beginningOfCurrentMeasureSeconds*44100.0;
		long samplesPerLoopPeriod = secondsPerLoopPeriod*44100.0;

		//Experimental frequency-sensitive renderer
if(1)//LGL_KeyDown(LGL_KEY_RALT)==false)
{
		for(int z=-pointResolution;z<pointResolution*2;z++)
		{
			int a=z+pointResolution;
			float xPreview = pointLeft + z/(float)(pointResolution-2)*pointWidth;
			if
			(
				xPreview >= vertLeft &&
				xPreview <= vertRight
			)
			{
				long sampleNow=(long)(sampleLeftBase+deltaSample*z);
				const long sampleLast=(long)(sampleNow+deltaSample);
				float zeroCrossingFactor;
				float magnitudeAve;
				float magnitudeMax;

				sound->GetMetadata
				(
					sampleNow/44100.0f-(0.5f/LGL_SOUND_METADATA_ENTRIES_PER_SECOND),
					sampleLast/44100.0f-(0.5f/LGL_SOUND_METADATA_ENTRIES_PER_SECOND),
					zeroCrossingFactor,
					magnitudeAve,
					magnitudeMax
				);

				zeroCrossingFactor=GetFreqBrightness(true,zeroCrossingFactor,magnitudeAve/sound->GetVolumePeak());
				if(0)
				{
					magnitudeAve*=(GetFreqBrightness(false,zeroCrossingFactor,magnitudeAve/sound->GetVolumePeak()) + zeroCrossingFactor > 0.0f) ? 1.0f : 0.0f;
				}

				magnitudeAve*=volumeMultiplierNow*0.5f;
				magnitudeMax*=volumeMultiplierNow*0.5f;

				double sampleNowDouble=(sampleLeft+sampleWidth*(z/(double)pointResolution));
				float xOffset=1.0f-(sampleNow-sampleNowDouble)/(float)deltaSample;

				arrayV[(a*2)+0]=pointLeft+((z-xOffset)/(float)pointResolution)*pointWidth;
				arrayV[(a*2)+1]=LGL_Clamp(pointBottom,pointBottom+(0.5f+0.5f*magnitudeAve)*pointHeight,pointTop);

				float glitchDelta = -0.35f*(glitchSampleRight-glitchSampleLeft);

				bool active=
				(
					(
						glitchLines &&
						sampleNow>glitchSampleLeft-glitchDelta &&
						sampleNow<glitchSampleRight-glitchDelta
					) ||
					(
						arrayV[(a*2)+0] >= centerX+needleDeltaL*viewportWidth &&
						arrayV[(a*2)+0] <= centerX+needleDeltaR*viewportWidth
					)
				);

				bool muted=false;
				if(rapidVolumeInvertSelf)
				{
					muted=(((long)fabs(sampleNow-bpmFirstBeatCurrentMeasureSamples)/samplesPerLoopPeriod)%2)==1;
					if(sampleNow<bpmFirstBeatCurrentMeasureSamples)
					{
						muted=!muted;
					}
				}
				if(muted)
				{
					for(int m=0;m<4;m++)
					{
						arrayC[a*4+m]=0;
					}
				}
				else
				{
					arrayC[(a*4)+0]=
						active ?
						1.0f :
						(
							(1.0f-zeroCrossingFactor)*coolR +
							(0.0f+zeroCrossingFactor)*warmR
						);
					arrayC[(a*4)+1]=
						active ?
						1.0f :
						(
							(1.0f-zeroCrossingFactor)*coolG +
							(0.0f+zeroCrossingFactor)*warmG
						);
					arrayC[(a*4)+2]=
						active ?
						1.0f :
						(
							(1.0f-zeroCrossingFactor)*coolB +
							(0.0f+zeroCrossingFactor)*warmB
						);
					arrayC[(a*4)+3]=1.0f;
				}

				//Tristrip!

				//Top
				arrayVtri[(a*4)+0]=arrayV[(a*2)+0];
				arrayVtri[(a*4)+1]=arrayV[(a*2)+1];

				//Bottom
				arrayVtri[(a*4)+2]=arrayV[(a*2)+0];
				arrayVtri[(a*4)+3]=(pointBottom+0.5f*pointHeight)-(arrayV[a*2+1]-(pointBottom+0.5f*pointHeight));

				//Top
				arrayCtri[(a*8)+0]=arrayC[a*4+0]*(0.5f+0.5f*zeroCrossingFactor);
				arrayCtri[(a*8)+1]=arrayC[a*4+1]*(0.5f+0.5f*zeroCrossingFactor);
				arrayCtri[(a*8)+2]=arrayC[a*4+2]*(0.5f+0.5f*zeroCrossingFactor);
				arrayCtri[(a*8)+3]=arrayC[a*4+3];

				//Bottom
				arrayCtri[(a*8)+4]=arrayC[a*4+0]*(0.5f+0.5f*zeroCrossingFactor);
				arrayCtri[(a*8)+5]=arrayC[a*4+1]*(0.5f+0.5f*zeroCrossingFactor);
				arrayCtri[(a*8)+6]=arrayC[a*4+2]*(0.5f+0.5f*zeroCrossingFactor);
				arrayCtri[(a*8)+7]=arrayC[a*4+3];

				//Silhouette
				arrayC[(a*4)+0]+=0.5f*zeroCrossingFactor;
				arrayC[(a*4)+1]+=0.5f*zeroCrossingFactor;
				arrayC[(a*4)+2]+=0.5f*zeroCrossingFactor;

				if(magnitudeMax>1.0f)
				{
					arrayC[(a*4)+0]=1.0f;
					arrayC[(a*4)+1]=0.0f;
					arrayC[(a*4)+2]=0.0f;
				}
			}
			else if(xPreview < vertLeft)
			{
				pointsToDrawIndexStart=a+2;
			}
			else if(xPreview > vertRight)
			{
				pointsToDrawIndexEnd=a-1;
				break;
			}
		}
		//Tristrip!
		if(!lowRez)
		{
			LGL_DrawTriStripToScreen
			(
				&(arrayVtri[pointsToDrawIndexStart*4]),
				&(arrayCtri[pointsToDrawIndexStart*8]),
				2*(pointsToDrawIndexEnd-pointsToDrawIndexStart),
				!lowRez
			);
		}

		//Linestrip top!
		for(int a=0;a<pointsToDrawIndexEnd;a++)
		{
			if(overdriven[a])
			{
				arrayC[(a*4)+0]=1.0f;
				arrayC[(a*4)+1]=0.0f;
				arrayC[(a*4)+2]=0.0f;
				//arrayC[(a*4)+3]=1.0f;
			}
			else
			{
				arrayC[(a*4)+0]*=1.25f;
				arrayC[(a*4)+1]*=1.25f;
				arrayC[(a*4)+2]*=1.25f;
				//arrayC[(a*4)+3]=1.0f;
			}
		}
		LGL_DrawLineStripToScreen
		(
			&(arrayV[pointsToDrawIndexStart*2]),
			&(arrayC[pointsToDrawIndexStart*4]),
			pointsToDrawIndexEnd-pointsToDrawIndexStart,
			3.5f,
			!lowRez
		);

		//Linestrip bottom!
		for(int a=pointsToDrawIndexStart;a<pointsToDrawIndexEnd;a++)
		{
			arrayV[(a*2)+1]=(pointBottom+0.5f*pointHeight)-(arrayV[(a*2)+1]-(pointBottom+0.5f*pointHeight));
		}
		LGL_DrawLineStripToScreen
		(
			&(arrayV[pointsToDrawIndexStart*2]),
			&(arrayC[pointsToDrawIndexStart*4]),
			pointsToDrawIndexEnd-pointsToDrawIndexStart,
			3.5f,
			!lowRez
		);
}

		//Draw Center Needle Rectangle
		LGL_DrawRectToScreen
		(
			centerX+needleDeltaL*viewportWidth,
			centerX+needleDeltaR*viewportWidth,
			pointBottom+NEEDLE_DISTANCE_FROM_EDGES*pointHeight,
			pointTop-NEEDLE_DISTANCE_FROM_EDGES*pointHeight,
			0.25f*warmR,0.25f*warmG,0.25f*warmB,0.0f
		);

		//Draw video buffer
		if
		(
			0 &&
			(
				videoSecondsBufferedLeft != 0 ||
				videoSecondsBufferedRight != 0
			)
		)
		{
			float secondsRadius = (soundPositionSamples - sampleLeft)/44100.0f;
			//float secondsNow = soundPositionSamples/44100.0f;
			float videoBufferedLeft = centerX+(pointLeft-centerX)*(videoSecondsBufferedLeft/secondsRadius);
			float videoBufferedRight = centerX+(pointRight-centerX)*(videoSecondsBufferedRight/secondsRadius);
			LGL_DrawLineToScreen
			(
				centerX,
				pointTop,
				videoBufferedLeft,
				pointTop,
				warmR,warmG,warmB,1.0f
			);
			LGL_DrawLineToScreen
			(
				centerX,
				pointTop,
				videoBufferedRight,
				pointTop,
				warmR,warmG,warmB,1.0f
			);
		}
		if
		(
			videoSecondsLoadedLeft != 0 ||
			videoSecondsLoadedRight != 0
		)
		{
			float secondsRadius = (soundPositionSamples - sampleLeft)/44100.0f;
			//float secondsNow = soundPositionSamples/44100.0f;
			float videoLoadedLeft = centerX+(pointLeft-centerX)*(videoSecondsLoadedLeft/secondsRadius);
			float videoLoadedRight = centerX+(pointRight-centerX)*(videoSecondsLoadedRight/secondsRadius);
			LGL_DrawLineToScreen
			(
				centerX,
				pointBottom,
				videoLoadedLeft,
				pointBottom,
				warmR,warmG,warmB,1.0f
			);
			LGL_DrawLineToScreen
			(
				centerX,
				pointBottom,
				videoLoadedRight,
				pointBottom,
				warmR,warmG,warmB,1.0f
			);
		}

		if
		(
			warpPointSecondsTrigger>0.0f &&
			warpPointSecondsStart==-1.0f
		)
		{
			DrawWarpPointLineOrRect
			(
				soundPositionSamples,
				sampleRadiusMultiplier,
				warpPointSecondsStart,
				warpPointSecondsTrigger,
				pointLeft,
				pointWidth,
				pointBottom,
				pointTop,
				warmR*1.0f,
				warmG*1.0f,
				warmB*1.0f,
				noiseImage256x64
			);
		}

		//Draw Glitch Rectangle
		if
		(
			glitchBegin >= 0 &&
			glitch
		)
		{
			float gleftSample=glitchBegin-glitchLength;
			float grightSample=glitchBegin+glitchLength;
			
			float centerSample=soundPositionSamples;
			float leftSample=centerSample-(64*512*pitchBend*sampleRadiusMultiplier);
			float rightSample=centerSample+(64*512*pitchBend*sampleRadiusMultiplier);

			if(leftSample<0 || rightSample>soundLengthSamples)
			{
				//Deal with wrap around

				if(centerSample<soundLengthSamplesHalf)
				{
					if(gleftSample>soundLengthSamplesHalf)
					{
						gleftSample-=soundLengthSamples;
					}
					if(grightSample>soundLengthSamplesHalf)
					{
						grightSample-=soundLengthSamples;
					}
				}
				else
				{
					if(gleftSample<soundLengthSamplesHalf)
					{
						gleftSample+=soundLengthSamples;
					}
					if(grightSample<soundLengthSamplesHalf)
					{
						grightSample+=soundLengthSamples;
					}
				}
			}

			float gLeftPercent=1.0-(rightSample-gleftSample)/(rightSample-leftSample);
			float gRightPercent=1.0-(rightSample-grightSample)/(rightSample-leftSample);

			if
			(
				glitchSampleRight>leftSample &&
				glitchSampleLeft<rightSample
			)
			{
				LGL_DrawRectToScreen
				(
					pointLeft+gLeftPercent*pointWidth,
					pointLeft+gRightPercent*pointWidth,
					pointBottom,
					pointTop,
					0.25f*warmR,0.25f*warmG,0.25f*warmB,0.0f
				);
			}
		}

		//Center Needle frame
		LGL_DrawLineToScreen
		(
			centerX+needleDeltaL*viewportWidth,pointBottom+NEEDLE_DISTANCE_FROM_EDGES*pointHeight,
			centerX+needleDeltaL*viewportWidth,pointTop-NEEDLE_DISTANCE_FROM_EDGES*pointHeight,
			warmR,warmG,warmB,1.0f,
			1
		);
		LGL_DrawLineToScreen
		(
			centerX+needleDeltaR*viewportWidth,pointBottom+NEEDLE_DISTANCE_FROM_EDGES*pointHeight,
			centerX+needleDeltaR*viewportWidth,pointTop-NEEDLE_DISTANCE_FROM_EDGES*pointHeight,
			warmR,warmG,warmB,1.0f,
			1
		);

		if(glitchLines)
		{
			LGL_DrawLineToScreen
			(
				glitchLinesLeft,pointBottom,
				glitchLinesLeft,pointTop,
				warmR,warmG,warmB,1.0f,
				1
			);
			LGL_DrawLineToScreen
			(
				glitchLinesRight,pointBottom,
				glitchLinesRight,pointTop,
				warmR,warmG,warmB,1.0f,
				1
			);
		}
		
		//Draw BPM Lines
		if(bpm>0)
		{
			float centerSample=soundPositionSamples;
			float leftSample=centerSample-64*512*pitchBend*SAMPLE_RADIUS_MULTIPLIER;
			float rightSample=centerSample+64*512*pitchBend*SAMPLE_RADIUS_MULTIPLIER;
			double secondsPerBeat=(60.0/bpm);

			if(leftSample<0)
			{
				//We can see both the start and end, center is at start
				float localLeftSample = soundLengthSamples+leftSample;
				float localRightSample = soundLengthSamples-1;
				if(loaded)
				{
					turntable_DrawBPMLines
					(
						localLeftSample,
						localRightSample,
						soundLengthSeconds,
						bpmFirstBeatSeconds,
						secondsPerBeat,
						pointBottom,
						pointTop,
						wavLeft,
						wavWidth*(localRightSample-localLeftSample)/(rightSample-leftSample)
					);
				}

				float prevWavWidth=wavWidth*(localRightSample-localLeftSample)/(rightSample-leftSample);
				localLeftSample=0;
				localRightSample=rightSample;
				turntable_DrawBPMLines
				(
					localLeftSample,
					localRightSample,
					soundLengthSeconds,
					bpmFirstBeatSeconds,
					secondsPerBeat,
					pointBottom,
					pointTop,
					wavLeft+prevWavWidth,
					wavWidth-prevWavWidth
				);
			}
			else if(rightSample>=soundLengthSamples)
			{
				//We can see both the start and end, center is at end
				float localLeftSample = leftSample;
				float localRightSample = soundLengthSamples-1;
				if(loaded)
				{
					turntable_DrawBPMLines
					(
						localLeftSample,
						localRightSample,
						soundLengthSeconds,
						bpmFirstBeatSeconds,
						secondsPerBeat,
						pointBottom,
						pointTop,
						wavLeft,
						wavWidth*(localRightSample-localLeftSample)/(rightSample-leftSample)
					);
				}

				float prevWavWidth=wavWidth*(localRightSample-localLeftSample)/(rightSample-leftSample);
				localLeftSample=0;
				localRightSample=rightSample-soundLengthSamples;
				turntable_DrawBPMLines
				(
					localLeftSample,
					localRightSample,
					soundLengthSeconds,
					bpmFirstBeatSeconds,
					secondsPerBeat,
					pointBottom,
					pointTop,
					wavLeft+prevWavWidth,
					wavWidth-prevWavWidth
				);
			}
			else
			{
				//We can see a continuous section
				turntable_DrawBPMLines
				(
					leftSample,
					rightSample,
					soundLengthSeconds,
					bpmFirstBeatSeconds,
					secondsPerBeat,
					pointBottom,
					pointTop,
					wavLeft,
					wavWidth
				);
			}
		}

		//Draw Save Points to large wave
		for(int a=0;a<18;a++)
		{
			if(savePointSeconds[a]>=0.0f)
			{
				float centerSample=soundPositionSamples;
				float leftSample=centerSample-64*512*pitchBend*SAMPLE_RADIUS_MULTIPLIER;
				float rightSample=centerSample+64*512*pitchBend*SAMPLE_RADIUS_MULTIPLIER;
				float widthSample=rightSample-leftSample;

				long savePointSamples = (long)(savePointSeconds[a]*44100);
				if
				(
					savePointSamples >= leftSample-(44100/4) &&
					savePointSamples <= rightSample+(44100/4)
				)
				{
					float savePointPercent = (savePointSamples-leftSample)/(float)widthSample;
					LGL_DrawLineToScreen
					(
						wavLeft+wavWidth*savePointPercent,pointBottom+0.4f*pointHeight,
						wavLeft+wavWidth*savePointPercent,pointBottom+0.6f*pointHeight,
						1.0f,1.0f,1.0f,1.0f,
						3
					);
					char str[4];
					if(a==0)
					{
						strcpy(str,"[");
					}
					else if(a==1)
					{
						strcpy(str,"]");
					}
					else if(a<=11)
					{
						sprintf(str,"%i",a-2);
					}
					else
					{
						sprintf(str,"%C",'A'+(((unsigned char)a)-12));
					}
					float fontHeight=0.10f*pointHeight;
					float lDelta=0.02f;
					LGL_GetFont().DrawString
					(
						wavLeft+wavWidth*savePointPercent-lDelta,
						pointBottom+0.5f*pointHeight-0.5f*fontHeight,
						fontHeight,
						1,1,1,1,
						true,
						0.75f,
						str
					);
				}
			}
		}
	}

	LGL_ClipRectDisable();

	float percentLoaded = 1.0f;
	if(cachedLengthSeconds!=0.0f)
	{
		percentLoaded = soundLengthSeconds/cachedLengthSeconds;
	}

	if(audioInputMode==false)
	{
		float noiseBottom=viewportBottom;
		float noiseTop=pointBottom;
		float noiseLeft=viewportLeft + (warpPointSecondsStart/cachedLengthSeconds)*viewportWidth;
		float noiseRight=viewportLeft + (warpPointSecondsTrigger/cachedLengthSeconds)*viewportWidth;
		float noiseHeight=noiseTop-noiseBottom;
		float noiseWidth=noiseRight-noiseLeft;
		float noiseRelativeWidth=noiseWidth/viewportWidth;

		//Background loop noise
		if(warpPointSecondsStart!=-1.0f)
		{
			noiseImage256x64->DrawToScreen
			(
				noiseLeft,noiseRight,
				noiseBottom,noiseTop,
				0,
				warmR,warmG,warmB,0.0f,
				1.0f,
				0,1.0f,
				0,LGL_Min(1,4*(noiseHeight/noiseWidth)/noiseRelativeWidth)
			);
		}

		//Entire Wave Array
		if(loaded)
		{
			//Waveform
			for(;;)
			{
				if(entireWaveArrayFillIndex<entireWaveArrayCount)
				{
					float zeroCrossingFactor;
					float magnitudeAve;
					float magnitudeMax;

					long sampleNow=(long)(entireWaveArrayFillIndex*(2*soundLengthSeconds*44100.0f/(double)(entireWaveArrayCount*2)));
					long sampleLast=sampleNow+(2*soundLengthSeconds*44100.0f/entireWaveArrayCount)-1;

					bool ret=sound->GetMetadata
					(
						sampleNow/44100.0f,
						sampleLast/44100.0f,
						zeroCrossingFactor,
						magnitudeAve,
						magnitudeMax
					);

					magnitudeAve*=0.5f;
					magnitudeMax*=0.5f;

					if(ret==false)
					{
						break;
					}

					entireWaveArrayMagnitudeAve[entireWaveArrayFillIndex]=magnitudeAve;
					entireWaveArrayMagnitudeMax[entireWaveArrayFillIndex]=magnitudeMax;
					entireWaveArrayFreqFactor[entireWaveArrayFillIndex]=zeroCrossingFactor;

					entireWaveArrayFillIndex++;
				}
				else
				{
					break;
				}
			}	
		}

		if(entireWaveArrayFillIndex>0)
		{
			float waveBottom=viewportBottom;
			float waveTop=pointBottom;
			float waveHeight=waveTop-waveBottom;
			float waveLeft=viewportLeft;
			float waveRight=viewportRight;

			if
			(
				LGL_MouseX()>=waveLeft &&
				LGL_MouseX()<=waveRight &&
				LGL_MouseY()>=waveBottom &&
				LGL_MouseY()<=waveTop
			)
			{
				GetInputMouse().SetHoverElement(GUI_ELEMENT_ENTIRE_WAVEFORM);
				if(LGL_MouseStroke(LGL_MOUSE_LEFT))
				{
					GetInputMouse().SetDragTarget((which==0) ? TARGET_TOP : TARGET_BOTTOM);
					GetInputMouse().SetDragElement(GUI_ELEMENT_ENTIRE_WAVEFORM);
				}
			}
			if(GetInputMouse().GetDragElement()==GUI_ELEMENT_ENTIRE_WAVEFORM)
			{
				if
				(
					LGL_MouseMotion() ||
					LGL_MouseStroke(LGL_MOUSE_LEFT)
				)
				{
					if(GetInputMouse().GetDragTarget()==(which==0) ? TARGET_TOP : TARGET_BOTTOM)
					{
						GetInputMouse().SetDragFloatNext
						(
							(LGL_MouseX()-waveLeft)/(waveRight-waveLeft)
						);
					}
				}
			}

			if(LGL_KeyDown(LGL_KEY_LCTRL))
			{
				bool alphaDone=false;
				if(warpPointSecondsTrigger<0.0f)
				{
					if(GetInputMouse().GetEntireWaveformScrubberDelta()==false)
					{
						if(sound->GetWarpPointIsLocked(channel)==false)
						{
							GetInputMouse().EntireWaveformScrubberAlpha(soundLengthSeconds,sound->GetPositionSeconds(channel),soundSpeed);
							alphaDone=true;
						}
					}
				}
				if(LGL_MouseMotion() || alphaDone)
				{
					float next = LGL_Clamp
					(
						0.0f,
						(LGL_MouseX()-waveLeft)/(waveRight-waveLeft),
						1.0f
					);
					if(GetInputMouse().GetEntireWaveformScrubberDelta())
					{
						GetInputMouse().SetEntireWaveformScrubberForceNext
						(
							next
						);
					}
				}
			}
			if(LGL_KeyRelease(LGL_KEY_LCTRL))
			{
				GetInputMouse().EntireWaveformScrubberOmega();
			}

			for(int a=0;a<entireWaveArrayFillIndex;a++)
			{
				float zeroCrossingFactor=entireWaveArrayFreqFactor[a];
				float magnitudeAve=entireWaveArrayMagnitudeAve[a];
				float magnitudeMax=entireWaveArrayMagnitudeMax[a];
				bool overdriven=(magnitudeMax*volumeMultiplierNow)>1.0f;
				float entirePercent = a/(float)(entireWaveArrayCount-1);

				zeroCrossingFactor=GetFreqBrightness(true,zeroCrossingFactor,magnitudeAve/sound->GetVolumePeak());

				entireWaveArrayLine1Points[a*2+0]=viewportLeft+(a/(float)entireWaveArrayCount)*viewportWidth;
				entireWaveArrayLine1Points[a*2+1]=LGL_Clamp
				(
					waveBottom,
					waveBottom+(0.5f+0.5f*magnitudeAve*volumeMultiplierNow)*waveHeight,
					waveTop
				);

				entireWaveArrayLine1Colors[a*4+0]=2.0f*
				(
					(1.0f-zeroCrossingFactor) * coolR +
					(0.0f+zeroCrossingFactor) * warmR
				);
				entireWaveArrayLine1Colors[a*4+1]=2.0f*
				(
					(1.0f-zeroCrossingFactor) * coolG +
					(0.0f+zeroCrossingFactor) * warmG
				);
				entireWaveArrayLine1Colors[a*4+2]=1.25f*
				(
					(1.0f-zeroCrossingFactor) * coolB +
					(0.0f+zeroCrossingFactor) * warmB
				);
				entireWaveArrayLine1Colors[a*4+3]=1.0f;

				entireWaveArrayLine2Points[a*2+0]=viewportLeft+(a/(float)entireWaveArrayCount)*viewportWidth;
				entireWaveArrayLine2Points[a*2+1]=LGL_Clamp
				(
					waveBottom,
					waveBottom+(0.5f-0.5f*magnitudeAve*volumeMultiplierNow)*waveHeight,
					waveTop
				);

				entireWaveArrayLine2Colors[a*4+0]=entireWaveArrayLine1Colors[a*4+0];
				entireWaveArrayLine2Colors[a*4+1]=entireWaveArrayLine1Colors[a*4+1];
				entireWaveArrayLine2Colors[a*4+2]=entireWaveArrayLine1Colors[a*4+2];
				entireWaveArrayLine2Colors[a*4+3]=1.0f;

				entireWaveArrayTriPoints[a*4+0]=entireWaveArrayLine1Points[a*2+0];
				entireWaveArrayTriPoints[a*4+1]=entireWaveArrayLine1Points[a*2+1];
				entireWaveArrayTriPoints[a*4+2]=entireWaveArrayLine2Points[a*2+0];
				entireWaveArrayTriPoints[a*4+3]=entireWaveArrayLine2Points[a*2+1];

				float loadedScalar = (entirePercent<=percentLoaded) ? 1.0f : 0.0f;

				entireWaveArrayTriColors[a*8+0]=loadedScalar*(0.5f+0.5f*zeroCrossingFactor)*entireWaveArrayLine1Colors[a*4+0];
				entireWaveArrayTriColors[a*8+1]=loadedScalar*(0.5f+0.5f*zeroCrossingFactor)*entireWaveArrayLine1Colors[a*4+1];
				entireWaveArrayTriColors[a*8+2]=loadedScalar*(0.5f+0.5f*zeroCrossingFactor)*entireWaveArrayLine1Colors[a*4+2];
				entireWaveArrayTriColors[a*8+3]=entireWaveArrayLine1Colors[a*4+3];

				entireWaveArrayTriColors[a*8+4]=loadedScalar*(0.5f+0.5f*zeroCrossingFactor)*entireWaveArrayLine2Colors[a*4+0];
				entireWaveArrayTriColors[a*8+5]=loadedScalar*(0.5f+0.5f*zeroCrossingFactor)*entireWaveArrayLine2Colors[a*4+1];
				entireWaveArrayTriColors[a*8+6]=loadedScalar*(0.5f+0.5f*zeroCrossingFactor)*entireWaveArrayLine2Colors[a*4+2];
				entireWaveArrayTriColors[a*8+7]=0.5f*entireWaveArrayLine2Colors[a*4+3];

				if(overdriven)
				{
					entireWaveArrayLine1Colors[a*4+0]=1.0f;
					entireWaveArrayLine1Colors[a*4+1]=0.0f;
					entireWaveArrayLine1Colors[a*4+2]=0.0f;;

					entireWaveArrayLine2Colors[a*4+0]=1.0f;
					entireWaveArrayLine2Colors[a*4+1]=0.0f;
					entireWaveArrayLine2Colors[a*4+2]=0.0f;;
				}
			}
		
			if(!lowRez)
			{
				LGL_DrawTriStripToScreen
				(
					entireWaveArrayTriPoints,//arrayVtri,
					entireWaveArrayTriColors,
					entireWaveArrayFillIndex*2
				);
			}

			LGL_DrawLineStripToScreen
			(
				entireWaveArrayLine1Points,
				entireWaveArrayLine1Colors,
				entireWaveArrayFillIndex,
				2.0f,
				true
			);
			LGL_DrawLineStripToScreen
			(
				entireWaveArrayLine2Points,
				entireWaveArrayLine2Colors,
				entireWaveArrayFillIndex,
				2.0f,
				true
			);

			if(cachedLengthSeconds!=0.0f)
			{
				//Draw Save Points to entire wave array
				for(int a=0;a<18;a++)
				{
					if(savePointSeconds[a]>=0.0f)
					{
						long savePointSamples = (long)(savePointSeconds[a]*44100);
						float savePointPercent = savePointSamples/(double)(cachedLengthSeconds*44100);
						float bright=(a==savePointIndex)?1.0f:0.5f;
						LGL_DrawLineToScreen
						(
							viewportLeft+viewportWidth*savePointPercent,waveBottom,
							viewportLeft+viewportWidth*savePointPercent,waveTop,
							bright,bright,bright,1.0f,
							(a==savePointIndex)?3.0f:1.0f,
							false
						);
					}
				}
			}

			if(warpPointSecondsStart!=-1.0f)
			{
				LGL_DrawLineToScreen
				(
					noiseLeft,noiseBottom,
					noiseLeft,noiseTop,
					0.0f,0.0f,0.0f,1.0f,
					4.5f,
					true
				);
				LGL_DrawLineToScreen
				(
					noiseRight,noiseBottom,
					noiseRight,noiseTop,
					0.0f,0.0f,0.0f,1.0f,
					4.5f,
					true
				);
				LGL_DrawLineToScreen
				(
					noiseLeft,noiseBottom,
					noiseLeft,noiseTop,
					warmR,warmG,warmB,0.0f,
					1.0f
				);
				LGL_DrawLineToScreen
				(
					noiseRight,noiseBottom,
					noiseRight,noiseTop,
					warmR,warmG,warmB,0.0f,
					1.0f
				);
			}

			if(cachedLengthSeconds!=0)
			{
				float r=1.0f;
				float g=1.0f;
				float b=1.0f;
				float a=1.0f;

				if(recallPos!=-1.0f)
				{
					float pctRecall=recallPos/cachedLengthSeconds;
					LGL_DrawLineToScreen
					(
						viewportLeft+pctRecall*viewportWidth,
						waveBottom,
						viewportLeft+pctRecall*viewportWidth,
						waveTop,
						r,g,b,a,
						4.0f,
						false
					);
					LGL_DrawLineToScreen
					(
						viewportLeft+pctRecall*viewportWidth,
						waveBottom,
						viewportLeft+pctRecall*viewportWidth,
						waveTop,
						warmR,warmG,warmB,1,
						2.0f,
						false
					);

					r=0.5f;
					g=0.5f;
					b=0.5f;
					a=0.5f;
				}
				float pos=soundPositionSeconds/cachedLengthSeconds;
				LGL_DrawLineToScreen
				(
					viewportLeft+pos*viewportWidth,
					waveBottom,
					viewportLeft+pos*viewportWidth,
					waveTop,
					r,g,b,a,
					4.0f,
					false
				);
				LGL_DrawLineToScreen
				(
					viewportLeft+pos*viewportWidth,
					waveBottom,
					viewportLeft+pos*viewportWidth,
					waveTop,
					warmR*r,warmG*g,warmB*b,1,
					2.0f,
					false
				);
			}
		}
	}

	//Draw Text

	char soundNameSafe[2048];
	strcpy(soundNameSafe,soundName);
	int soundNameSafeLen=strlen(soundNameSafe);
	for(int s=0;s<soundNameSafeLen;s++)
	{
		if(soundNameSafe[s]=='%')
		{
			soundNameSafe[s]=' ';
		}
	}

	char tmpStr[2048];
	strcpy(tmpStr,soundNameSafe);
	if(strstr(tmpStr,".mp3"))
	{
		strstr(tmpStr,".mp3")[0]='\0';
	}
	if(strstr(tmpStr,".ogg"))
	{
		strstr(tmpStr,".ogg")[0]='\0';
	}
	if(audioInputMode)
	{
		strcpy
		(
			tmpStr,
			LGL_AudioInAvailable() ?
			"Audio Input" :
			"Audio Input Unavailable"
		);
	}

	float txtCenterX=centerX+0.25f*viewportWidth;
	float fontWidthMax=viewportWidth*0.45f;
	if(videoPathShort==NULL || 1)
	{
		txtCenterX=centerX;
		fontWidthMax=viewportWidth*0.9f;
	}

	float fontHeight=0.05f*viewportHeight;
	float fontWidth=LGL_GetFont().GetWidthString(fontHeight,tmpStr);
	fontHeight=LGL_Min(fontHeight,fontHeight*fontWidthMax/fontWidth);

	LGL_GetFont().DrawString
	(
		txtCenterX,
		viewportBottom+.925f*viewportHeight-0.5f*fontHeight,
		fontHeight,
		1,1,1,1,
		true,.5f,
		tmpStr
	);

	fontHeight=0.05f*viewportHeight;
	fontWidth=LGL_GetFont().GetWidthString(fontHeight,tmpStr);
	fontHeight=LGL_Min(fontHeight,fontHeight*0.45f*viewportWidth/fontWidth);

	if(videoPathShort && 0)
	{
		strcpy(tmpStr,videoPathShort);
		if(char* mp3 = strstr(tmpStr,".mp3"))
		{
			mp3[0]='\0';
		}

		LGL_GetFont().DrawString
		(
			centerX-0.25f*viewportWidth,
			viewportBottom+.925f*viewportHeight-0.5f*fontHeight,
			fontHeight,
			1,1,1,1,
			true,.5f,
			tmpStr
		);
	}

	/*
	if
	(
		pause==0 &&
		grainStreamCrossfader==0.0f
	)
	{
		sprintf(tmpStr,"off");
	}
	else
	{
		float num=
			(1.0f-grainStreamCrossfader) * soundSpeed*100 +
			(0.0f+grainStreamCrossfader) * 2.0f*joyAnalogueStatusLeftX*100;
		sprintf(tmpStr,"%.2f",num);
	}
	LGL_GetFont().DrawString
	(
		viewportLeft+.02f*viewportWidth,
		viewportBottom+.80f*viewportHeight,
		viewportHeight/15.0f,
		1,1,1,1,
		false,.5f,
		"Speed:"
	);
	LGL_GetFont().DrawString
	(
		viewportLeft+.125f*viewportWidth,
		viewportBottom+.80f*viewportHeight,
		viewportHeight/15.0f,
		1,1,1,1,
		false,.5f,
		tmpStr
	);
	*/

	float lft=0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.0075f;
	float wth=(viewportWidth*0.25f)/16.0f;
	float bot=viewportBottom+0.24f*viewportHeight;
	float top=bot+0.075f*viewportHeight;
	float spc=wth*0.75f;

	if(audioInputMode==false)
	{
		float bpmPitchL = 0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.009f;
		float bpmPitchR = 0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.009f+0.100f;
		float bpmPitchB = viewportBottom+.70f*viewportHeight;
		float bpmPitchT = viewportBottom+0.85f*viewportHeight;

		if
		(
			LGL_MouseX()>=bpmPitchL &&
			LGL_MouseX()<=bpmPitchR &&
			LGL_MouseY()>=bpmPitchB-0.01f &&
			LGL_MouseY()<=bpmPitchT+0.01f
		)
		{
			GetInputMouse().SetHoverElement(GUI_ELEMENT_BPM_PITCH);
		}

		if(bpmAdjusted>0)
		{
			LGL_GetFont().DrawString
			(
				//viewportLeft+.02f*viewportWidth,
				bpmPitchL,
				viewportBottom+.80f*viewportHeight,
				0.05f*viewportHeight,
				1,1,1,1,
				false,.5f,
				"BPM:"
			);
			LGL_GetFont().DrawString
			(
				//viewportLeft+.125f*viewportWidth,
				0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.06f-0.0095f,
				viewportBottom+.80f*viewportHeight,
				0.05f*viewportHeight,
				1,1,1,1,
				false,.5f,
				"%.2f",
				bpmAdjusted
			);

			if(0 && isMaster)
			{
				float masterBoxLeft=0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.006f;
				float masterBoxRight=0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.105f;
				float masterBoxBottom=viewportBottom+.78f*viewportHeight;
				float masterBoxTop=viewportBottom+.88f*viewportHeight;
				LGL_DrawLineToScreen
				(
					masterBoxLeft,
					masterBoxTop,
					masterBoxRight,
					masterBoxTop,
					1,1,1,1
				);
				LGL_DrawLineToScreen
				(
					masterBoxRight,
					masterBoxTop,
					masterBoxRight,
					masterBoxBottom,
					1,1,1,1
				);
				LGL_DrawLineToScreen
				(
					masterBoxRight,
					masterBoxBottom,
					masterBoxLeft,
					masterBoxBottom,
					1,1,1,1
				);
				LGL_DrawLineToScreen
				(
					masterBoxLeft,
					masterBoxBottom,
					masterBoxLeft,
					masterBoxTop,
					1,1,1,1
				);
			}
		}
		
		float pbFloat=
			(1.0f-grainStreamCrossfader) * pitchBend +
			(0.0f+grainStreamCrossfader) * grainStreamPitch;
			
		float pbAbs=fabs((pbFloat-1)*100);
		if(pbFloat>=1)
		{
			sprintf(tmpStr,"+%.2f",pbAbs);
		}
		else
		{
			sprintf(tmpStr,"-%.2f",pbAbs);
		}
		if(strstr(tmpStr,"0.00"))
		{
			strcpy(tmpStr,"+0.00");
		}

		char tempNudge[1024];
		if(nudge>0)
		{
			sprintf(tempNudge,"+%.1f",fabs((nudge)*100));
		}
		else if(nudge<0)
		{
			sprintf(tempNudge,"-%.1f",fabs((nudge)*100));
		}
		else
		{
			tempNudge[0]='\0';
		}
		LGL_GetFont().DrawString
		(
			//viewportLeft+.02f*viewportWidth,
			bpmPitchL,
			bpmPitchB,
			0.05f*viewportHeight,
			1,1,1,1,
			false,.5f,
			"Pitch:"
		);
		LGL_GetFont().DrawString
		(
			//viewportLeft+.125f*viewportWidth,
			0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.06f,
			bpmPitchB,
			0.05f*viewportHeight,
			1,1,1,1,
			false,.5f,
			"%s%s",
			tmpStr,
			tempNudge
		);

		float lb=warpPointSecondsStart>=0.0f ? 1.0f : 0.25f;

		//Looping
		{
			float loopL=0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.009f+0.112f;
			float loopR=0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.009f+0.127f;
			float loopB=viewportBottom+.80f*viewportHeight-0.010f*viewportHeight;
			float loopT=viewportBottom+.80f*viewportHeight+0.075f*viewportHeight;
			loopImage->DrawToScreen
			(
				loopL,loopR,
				loopB,loopT,
				0,
				lb,lb,lb,1.0f
			);

			if
			(
				LGL_MouseX()>=loopL &&
				LGL_MouseX()<=viewportRight &&
				LGL_MouseY()>=loopB &&
				LGL_MouseY()<=loopT
			)
			{
				GetInputMouse().SetHoverElement(GUI_ELEMENT_LOOP_MEASURES);
			}

			if
			(
				LGL_MouseX()>=loopL &&
				LGL_MouseX()<=loopR &&
				LGL_MouseY()>=loopB &&
				LGL_MouseY()<=loopT
			)
			{
				if(LGL_MouseStroke(LGL_MOUSE_LEFT))
				{
					GetInputMouse().SetWaveformLoopToggleNext();
				}
			}
		}

		char loopStr[2048];
		if(bpmAdjusted>0)
		{
			if(loopExponent>1000)
			{
				strcpy(loopStr,"all");
			}
			else
			{
				sprintf
				(
					loopStr,
					loopExponent>=0 ? "%i" : "1/%i",
					loopExponent>=0 ? (int)(powf(2,loopExponent)) : (int)(powf(2,-loopExponent))
				);
			}
		}
		else
		{
			if
			(
				warpPointSecondsStart==0.0f &&
				warpPointSecondsTrigger>=soundLengthSeconds
			)
			{
				strcpy(loopStr,"all");
			}
			else
			{
				sprintf
				(
					loopStr,
					loopSeconds < 10.0 ? "%.3f" : "%.2f",
					loopSeconds
				);
			}
		}
		LGL_GetFont().DrawString
		(
			0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.009f+0.1325f,
			viewportBottom+.80f*viewportHeight,
			0.05f*viewportHeight,
			1,1,1,1,
			false,.5f,
			loopStr
		);
		
		/*
		LGL_GetFont().DrawString
		(
			viewportLeft+.02f*viewportWidth,
			viewportBottom+.60f*viewportHeight,
			viewportHeight/15.0f,
			1,1,1,1,
			false,.5f,
			"Volume:"
		);
		LGL_GetFont().DrawString
		(
			viewportLeft+.125f*viewportWidth,
			viewportBottom+.60f*viewportHeight,
			viewportHeight/15.0f,
			1,1,1,1,
			false,.5f,
			"%.2f",fabs((volumeMultiplierNow)*2.0f)
		);
		*/

		float minutes=0.0f;
		float seconds=0.0f;
		/*
		if(pitchBend>0.0f)
		{
			seconds=soundLengthSeconds/pitchBend;
			if(seconds>60*999) seconds=999;
		}
		while(seconds>60)
		{
			seconds-=60;
			minutes+=1;
		}
		if(minutes<10)
		{
			if(seconds<10)
			{
				sprintf(tmpStr,"0%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(tmpStr,"0%.0f:%.2f",minutes,seconds);
			}
		}
		else
		{
			if(seconds<10)
			{
				sprintf(tmpStr,"%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(tmpStr,"%.0f:%.2f",minutes,seconds);
			}
		}

		LGL_GetFont().DrawString
		(
			viewportLeft+.02f*viewportWidth,
			viewportBottom+.45f*viewportHeight,
			viewportHeight/15.0f,
			1,1,1,1,
			false,.5f,
			"Length:"
		);
		LGL_GetFont().DrawString
		(
			viewportLeft+.125f*viewportWidth,
			viewportBottom+.45f*viewportHeight,
			viewportHeight/15.0f,
			1,1,1,1,
			false,.5f,
			tmpStr
		);
		*/
		
		minutes=0;
		seconds=soundPositionSeconds/pitchBend;
		if(seconds>60*999) seconds=999;
		while(seconds>60)
		{
			seconds-=60;
			minutes+=1;
		}
		if(minutes<10)
		{
			if(seconds<10)
			{
				sprintf(tmpStr,"0%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(tmpStr,"0%.0f:%.2f",minutes,seconds);
			}
		}
		else
		{
			if(seconds<10)
			{
				sprintf(tmpStr,"%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(tmpStr,"%.0f:%.2f",minutes,seconds);
			}
		}
		/*
		LGL_GetFont().DrawString
		(
			//viewportLeft+.02f*viewportWidth,
			0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.009f,
			viewportBottom+.25f*viewportHeight,
			0.05f*viewportHeight,
			1,1,1,1,
			false,.5f,
			"Position:"
		);
		*/
		float posElapsedRemainingHeight=viewportBottom+.15f*viewportHeight;
		LGL_GetFont().DrawString
		(
			//viewportLeft+.125f*viewportWidth,
			//0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.11f,
			0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.009f,
			posElapsedRemainingHeight,
			0.05f*viewportHeight,
			1,1,1,1,
			false,.5f,
			tmpStr
		);
		
		LGL_GetFont().DrawString
		(
			//viewportLeft+.125f*viewportWidth,
			//0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.11f,
			0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.095f,
			posElapsedRemainingHeight,
			0.05f*viewportHeight,
			1,1,1,1,
			false,.5f,
			"/"
		);

		minutes=0;
		seconds=((cachedLengthSeconds!=0.0f) ? cachedLengthSeconds : soundLengthSeconds) - soundPositionSeconds;
		seconds/=pitchBend;
		while(seconds>60)
		{
			seconds-=60;
			minutes+=1;
		}
		if(minutes<10)
		{
			if(seconds<10)
			{
				sprintf(tmpStr,"0%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(tmpStr,"0%.0f:%.2f",minutes,seconds);
			}
		}
		else
		{
			if(seconds<10)
			{
				sprintf(tmpStr,"%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(tmpStr,"%.0f:%.2f",minutes,seconds);
			}
		}
		/*
		LGL_GetFont().DrawString
		(
			//viewportLeft+.02f*viewportWidth,
			0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.009f,
			viewportBottom+.15f*viewportHeight,
			0.05f*viewportHeight,
			1,1,1,1,
			false,.5f,
			"Remaining:"
		);
		*/
		LGL_GetFont().DrawString
		(
			//viewportLeft+.125f*viewportWidth,
			0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.11f,
			posElapsedRemainingHeight,
			0.05f*viewportHeight,
			1,1,1,1,
			false,.5f,
			tmpStr
		);

		/*
		float percent=soundPositionSeconds/soundLengthSeconds;
		LGL_GetFont().DrawString
		(
			viewportLeft+.02f*viewportWidth,
			viewportBottom+.25f*viewportHeight,
			viewportHeight/15.0f,
			1,1,1,1,
			false,.5f,
			"Percent:"
		);
		LGL_GetFont().DrawString
		(
			viewportLeft+.125f*viewportWidth,
			viewportBottom+.25f*viewportHeight,
			viewportHeight/15.0f,
			1,1,1,1,
			false,.5f,
			"%.2f",
			percent*100
		);
		*/

		/*
		minutes=0;
		seconds=soundLengthSeconds - soundPositionSeconds;
		seconds/=pitchBend;
		while(seconds>60)
		{
			seconds-=60;
			minutes+=1;
		}
		if(minutes<10)
		{
			if(seconds<10)
			{
				sprintf(tmpStr,"0%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(tmpStr,"0%.0f:%.2f",minutes,seconds);
			}
		}
		else
		{
			if(seconds<10)
			{
				sprintf(tmpStr,"%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(tmpStr,"%.0f:%.2f",minutes,seconds);
			}
		}
		LGL_GetFont().DrawString
		(
			//viewportLeft+.02f*viewportWidth,
			0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.009f,
			viewportBottom+.15f*viewportHeight,
			0.05f*viewportHeight,
			1,1,1,1,
			false,.5f,
			"Remaining:"
		);
		LGL_GetFont().DrawString
		(
			//viewportLeft+.125f*viewportWidth,
			0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.11f,
			viewportBottom+.15f*viewportHeight,
			0.05f*viewportHeight,
			1,1,1,1,
			false,.5f,
			tmpStr
		);
		*/

		/*
		LGL_GetFont().DrawString
		(
			pointLeft+.02f*viewportWidth,
			viewportBottom+.02f*viewportHeight,
			viewportHeight/15.0f,
			1,1,1,1,
			false,.5f,
			"Save Points:"
		);
		*/

		bool mySavePointSet[12];
		for(int a=0;a<12;a++)
		{
			mySavePointSet[a]=savePointSetBitfield & (1<<a);
		}

		if
		(
			LGL_MouseX()>=lft &&
			LGL_MouseX()<=lft+11*wth+spc &&
			LGL_MouseY()>=bot &&
			LGL_MouseY()<=top
		)
		{
			GetInputMouse().SetHoverInSavePoints();
		}

		for(int i=0;i<12;i++)
		{
			int thickness;
			float r;
			float g;
			float b;
			if(i==savePointIndex)// || i==savePointIndexActual+1)
			{
				thickness=6;
				r=LGL_Min(1.0f,warmR+0.1f*glow);
				g=LGL_Min(1.0f,warmG+0.1f*glow);
				b=LGL_Min(1.0f,warmB+0.1f*glow);
			}
			else
			{
				thickness=2;
				r=coolR;
				g=coolG;
				b=coolB;
			}

			float myL=lft+i*wth;
			float myR=lft+i*wth+spc;
			float myB=bot;
			float myT=top;

			if
			(
				LGL_MouseX()>=myL &&
				LGL_MouseX()<=myR &&
				LGL_MouseY()>=myB-0.005f &&
				LGL_MouseY()<=myT
			)
			{
				DVJ_GuiElement target=GUI_ELEMENT_NULL;
				if(i==0)
				{
					target=GUI_ELEMENT_SAVEPOINT_BPM_ALPHA;
				}
				else if(i==1)
				{
					target=GUI_ELEMENT_SAVEPOINT_BPM_OMEGA;
				}
				else if(i==2)
				{
					target=GUI_ELEMENT_SAVEPOINT_0;
				}
				else if(i==3)
				{
					target=GUI_ELEMENT_SAVEPOINT_1;
				}
				else if(i==4)
				{
					target=GUI_ELEMENT_SAVEPOINT_2;
				}
				else if(i==5)
				{
					target=GUI_ELEMENT_SAVEPOINT_3;
				}
				else if(i==6)
				{
					target=GUI_ELEMENT_SAVEPOINT_4;
				}
				else if(i==7)
				{
					target=GUI_ELEMENT_SAVEPOINT_5;
				}
				else if(i==8)
				{
					target=GUI_ELEMENT_SAVEPOINT_6;
				}
				else if(i==9)
				{
					target=GUI_ELEMENT_SAVEPOINT_7;
				}
				else if(i==10)
				{
					target=GUI_ELEMENT_SAVEPOINT_8;
				}
				else if(i==11)
				{
					target=GUI_ELEMENT_SAVEPOINT_9;
				}
				GetInputMouse().SetHoverElement(target);
				if(i==savePointIndex)
				{
					GetInputMouse().SetHoverOnSelectedSavePoint();
				}
			}

			//Left
			LGL_DrawLineToScreen
			(
				myL,myB,
				myL,myT,
				r,g,b,1,
				thickness
			);

			//Right
			LGL_DrawLineToScreen
			(
				myR,myB,
				myR,myT,
				r,g,b,1,
				thickness
			);

			//Bottom
			LGL_DrawLineToScreen
			(
				myL,myB,
				myR,myB,
				r,g,b,1,
				thickness
			);

			//Top
			LGL_DrawLineToScreen
			(
				myL,myT,
				myR,myT,
				r,g,b,1,
				thickness
			);

			float savePointSetScalar = mySavePointSet[i] ? 1.0f : 0.0f;

			LGL_DrawRectToScreen
			(
				myL,myR,
				myB,myT,
				mySavePointSet[i]*coolR,
				mySavePointSet[i]*coolG,
				mySavePointSet[i]*coolB,
				1
			);
			if(savePointUnsetNoisePercent[i]>0.0f)
			{
				noiseImage256x64->DrawToScreen
				(
					lft+i*wth,lft+i*wth+spc,
					bot,top,
					0,
					savePointUnsetNoisePercent[i],savePointUnsetNoisePercent[i],savePointUnsetNoisePercent[i],savePointUnsetNoisePercent[i],
					1.0f,
					0,1.0f/32.0f,
					0,1.0f/8.0f
				);
			}
			if(savePointUnsetFlashPercent[i]>0.0f)
			{
				LGL_DrawRectToScreen
				(
					lft+i*wth,lft+i*wth+spc,
					bot,top,
					savePointUnsetFlashPercent[i],savePointUnsetFlashPercent[i],savePointUnsetFlashPercent[i],0
				);
			}

			if(1)//i!=0)
			{
				char str[4];

				if(i==0)
				{
					strcpy(str,"[");
				}
				else if(i==1)
				{
					strcpy(str,"]");
				}
				/*
				else if(i>=19)
				{
					strcpy(str,(i==19)?"?":"!");
				}
				*/
				else if(i<=12)
				{
					sprintf(str,"%i",i-2);
				}
				else
				{
					sprintf(str,"%C",'A'+(((unsigned char)i)-13));
				}

				LGL_GetFont().DrawString
				(
					lft+i*wth+.6f*spc,
					bot+0.25f*0.05f*viewportHeight,//+0.0275f*viewportHeight,
					0.05f*viewportHeight,
					0.25f+0.75f*savePointSetScalar,
					0.25f+0.75f*savePointSetScalar,
					0.25f+0.75f*savePointSetScalar,
					1,
					true,
					0.75f,
					str
				);
			}
		}	
	}

	Turntable_DrawSliders
	(
		which,
		viewportLeft,
		viewportRight,
		viewportBottom,
		viewportTop,
		eq0,
		eq1,
		eq2,
		volumeMultiplierNow,
		videoBrightness,
		oscilloscopeBrightness,
		freqSenseBrightness,
		freqSenseLEDBrightness,
		freqSenseLEDColorScalarLow,
		freqSenseLEDColorScalarHigh,
		freqSenseLEDGroupFloat,
		freqSenseLEDGroupInt
	);
}

void
Turntable_DrawSliders
(
	int		which,
	float		viewportLeft,
	float		viewportRight,
	float		viewportBottom,
	float		viewportTop,
	float		eq0,
	float		eq1,
	float		eq2,
	float		gain,
	float		videoBrightness,
	float		oscilloscopeBrightness,
	float		freqSenseBrightness,
	float		freqSenseLEDBrightness,
	float		freqSenseLEDColorScalarLow,
	float		freqSenseLEDColorScalarHigh,
	float		freqSenseLEDGroupFloat,
	int		freqSenseLEDGroupInt
)
{
	float viewportWidth = viewportRight - viewportLeft;
	float viewportHeight = viewportTop - viewportBottom;

	float coolR;
	float coolG;
	float coolB;
	GetColorCool(coolR,coolG,coolB);

	float warmR;
	float warmG;
	float warmB;
	GetColorWarm(warmR,warmG,warmB);

	float lft=0.5f+0.5f*viewportWidth*WAVE_WIDTH_PERCENT+0.0075f;
	float wth=(viewportWidth*0.25f)/16.0f;
	//float bot=viewportBottom+0.24f*viewportHeight;
	//float top=bot+0.075f*viewportHeight;
	float spc=wth*0.75f;
	float eq[3];
	
	float sliderB=viewportBottom+(0.5f-0.075f/2.0f)*viewportHeight+spc*(-1);
	float sliderT=viewportBottom+(0.5f+0.075f/2.0f)*viewportHeight+spc*(2.5f);

	float textB=sliderB-0.075f*viewportHeight;
	
	eq[0]=0.5f*eq0;
	eq[1]=0.5f*eq1;
	eq[2]=0.5f*eq2;

	char letters[12];
	for(int l=0;l<12;l++)
	{
		letters[l]='\0';
	}
	letters[0]='L';
	letters[1]='M';
	letters[2]='H';
	letters[3]='G';
	letters[5]='V';
	letters[6]='O';
	letters[7]='F';
	letters[8]='L';
	letters[9]='c';
	letters[10]='C';
	letters[11]='0'+freqSenseLEDGroupInt;

	LGL_Color letterColors[12];
	for(int l=0;l<12;l++)
	{
		letterColors[l].SetRGBA(1.0f,1.0f,1.0f,1.0f);
	}
	letterColors[9]=GetColorFromScalar(freqSenseLEDColorScalarLow);
	letterColors[10]=GetColorFromScalar(freqSenseLEDColorScalarHigh);

	DVJ_GuiElement guiElements[12];
	for(int l=0;l<12;l++)
	{
		guiElements[l]=GUI_ELEMENT_NULL;
	}
	guiElements[0]=GUI_ELEMENT_EQ_LOW;
	guiElements[1]=GUI_ELEMENT_EQ_MID;
	guiElements[2]=GUI_ELEMENT_EQ_HIGH;
	guiElements[3]=GUI_ELEMENT_EQ_GAIN;
	guiElements[5]=GUI_ELEMENT_VIS_VIDEO;
	guiElements[6]=GUI_ELEMENT_VIS_OSCILLOSCOPE;
	guiElements[7]=GUI_ELEMENT_VIS_FREQSENSE;
	guiElements[8]=GUI_ELEMENT_VIS_FREQSENSE_LED_BRIGHTNESS;
	guiElements[9]=GUI_ELEMENT_VIS_FREQSENSE_LED_COLOR_SCALAR_LOW;
	guiElements[10]=GUI_ELEMENT_VIS_FREQSENSE_LED_COLOR_SCALAR_HIGH;
	guiElements[11]=GUI_ELEMENT_VIS_FREQSENSE_LED_GROUP_FLOAT;

	float levels[12];
	for(int l=0;l<12;l++)
	{
		levels[l]=0.0f;
	}
	levels[0]=eq[0];
	levels[1]=eq[1];
	levels[2]=eq[2];
	levels[3]=LGL_Clamp(0.0f,gain*0.5f,1.0f);
	levels[5]=videoBrightness;
	levels[6]=oscilloscopeBrightness;
	levels[7]=freqSenseBrightness;
	levels[8]=freqSenseLEDBrightness;
	levels[9]=freqSenseLEDColorScalarLow;
	levels[10]=freqSenseLEDColorScalarHigh;
	levels[11]=freqSenseLEDGroupFloat;

	for(int f=0;f<12;f++)
	{
		if(guiElements[f]==GUI_ELEMENT_NULL)
		{
			continue;
		}

		float sliderL=lft+f*wth;
		float sliderR=sliderL+spc;

		//Mouse Input
		{
			DVJ_GuiElement guiElementNow=guiElements[f];

			float dragFloat=LGL_Clamp
			(
				0.0f,
				(LGL_MouseY()-sliderB)/(sliderT-sliderB),
				1.0f
			);
			
			if
			(
				LGL_MouseX()>=sliderL &&
				LGL_MouseX()<=sliderR &&
				LGL_MouseY()>=textB &&
				LGL_MouseY()<=sliderT
			)
			{
				GetInputMouse().SetHoverElement(guiElementNow);
			}

			if
			(
				LGL_MouseX()>=sliderL &&
				LGL_MouseX()<=sliderR &&
				LGL_MouseY()>=sliderB &&
				LGL_MouseY()<=sliderT
			)
			{
				if(LGL_MouseStroke(LGL_MOUSE_LEFT))
				{
					GetInputMouse().SetDragTarget((which==0) ? TARGET_TOP : TARGET_BOTTOM);
					GetInputMouse().SetDragElement(guiElementNow);
					GetInputMouse().SetDragFloatNext(dragFloat);
				}
			}

			if(GetInputMouse().GetDragElement()==guiElementNow)
			{
				if(LGL_MouseMotion())
				{
					if(GetInputMouse().GetDragTarget() == ((which==0) ? TARGET_TOP : TARGET_BOTTOM))
					{
						GetInputMouse().SetDragFloatNext(dragFloat);
					}
				}
			}
		}

		int thickness;
		float r;
		float g;
		float b;

		thickness=1;
		r=coolR*1.0f;
		g=coolG*1.0f;
		b=coolB*1.0f;

		LGL_DrawRectToScreen
		(	
			sliderL,
			sliderR,
			sliderB,
			sliderT,
			0,0,0,1
		);

		float warmScale=levels[f];
		float barR=
			(1.0f-warmScale)*coolR+
			(0.0f+warmScale)*warmR;
		float barG=
			(1.0f-warmScale)*coolG+
			(0.0f+warmScale)*warmG;
		float barB=
			(1.0f-warmScale)*coolB+
			(0.0f+warmScale)*warmB;

		LGL_DrawRectToScreen
		(
			sliderL,
			sliderR,
			sliderB,
			sliderB+levels[f]*(sliderT-sliderB),
			barR,
			barG,
			barB,
			1
		);

		//Left
		LGL_DrawLineToScreen
		(
			sliderL,sliderB,
			sliderL,sliderT,
			r,g,b,1,
			thickness
		);

		//Right
		LGL_DrawLineToScreen
		(
			sliderR,sliderB,
			sliderR,sliderT,
			r,g,b,1,
			thickness
		);

		//Bottom
		LGL_DrawLineToScreen
		(
			sliderL,sliderB,
			sliderR,sliderB,
			r,g,b,1,
			thickness
		);

		//Top
		LGL_DrawLineToScreen
		(
			sliderL,sliderT,
			sliderR,sliderT,
			r,g,b,1,
			thickness
		);

		LGL_GetFont().DrawString
		(
			sliderL+0.5f*spc,
			textB,
			0.05f*viewportHeight,
			letterColors[f].GetR(),
			letterColors[f].GetG(),
			letterColors[f].GetB(),
			1.0f,
			true,
			0.75f,
			"%c",
			letters[f]
		);
	}
}

void
Visualizer_DrawWaveform
(
	float*	waveformSamples,
	int	waveformSamplesCount,
	bool	fullscreen
)
{
	float l=0.0f;
	float r=0.5f;
	float b=0.5f;
	float t=1.0f;
	float w=r-l;
	float h=t-b;
	if(fullscreen)
	{
		l=0;
		r=1;
		b=0;
		t=1;
		w=1;
		h=1;
	}

	float warmR;
	float warmG;
	float warmB;
	GetColorWarm(warmR,warmG,warmB);

	float R=warmR;
	float G=warmG;
	float B=warmB;
	float A=0.5f;

	/*
	if(LGL_RandBool())
	{
		R+=LGL_RandFloat(.25,75);
	}
	else
	{
		G+=LGL_RandFloat(.25,.75);
	}
	if(LGL_RandBool())
	{
		G+=LGL_RandFloat(.25,.75);
	}
	else
	{
		B+=LGL_RandFloat(.25,.75);
	}
	if(LGL_RandBool())
	{
		B+=LGL_RandFloat(.25,.75);
	}
	else
	{
		A+=LGL_RandFloat(.25,.75);
	}
	*/

	float* arraySrc=waveformSamples;
	float* arrayDst=new float[waveformSamplesCount*2];

	for(int a=0;a<waveformSamplesCount;a++)
	{
		arrayDst[(a*2)+0]=l+w*(a/(float)waveformSamplesCount);
		arrayDst[(a*2)+1]=b+0.5f*h+0.5f*h*(arraySrc[a]-0.5f)*2;
	}

	LGL_DrawLineStripToScreen
	(
		arrayDst,
		waveformSamplesCount,
		R,G,B,A,
		4.5f,
		true
	);

	delete arrayDst;
}

float
GetFreqBrightness
(
	bool	hi,
	float	freqFactor,
	float	vol,
	float	exagerationFactor
)
{
	float freqMag;
	if(hi==false)
	{
		float minVol=0.2f;
		float maxVol=0.5f;
		float volMagNormalized=LGL_Clamp(0,(vol-minVol)/(maxVol-minVol),1);
		float myFreqMag=1.0f-freqFactor;
		float minFreqMag=0.35f;
		freqMag=LGL_Clamp
		(
			0.0f,
			volMagNormalized*(myFreqMag-minFreqMag)/(1.0f-minFreqMag),
			1.0f
		);
	}
	else
	{
		float minVol=0.25f;
		float maxVol=0.5f;
		float volMagNormalized=LGL_Clamp(0,(vol-minVol)/(maxVol-minVol),1);
		float myFreqMag=0.0f+freqFactor;
		float minFreqMag=0.2f;
		freqMag=LGL_Clamp
		(
			0.0f,
			(myFreqMag-minFreqMag)/(1.0f-minFreqMag),
			1.0f
		);
		freqMag*=2.0f;
		if(freqMag>1.0f)
		{
			freqMag*=freqMag;
		}
		freqMag/=2.0f;
		freqMag*=volMagNormalized;
	}
	
	float brightFactor=1.0f;
	//EQ Affects Analysis
	if(exagerationFactor!=-1.0f)
	{
		if(hi)
		{
			//0 => 0
			//.5 => 1
			//1 => 4
			brightFactor=exagerationFactor*powf(2.0f,(exagerationFactor*4.0f));
		}
		else
		{
			brightFactor=exagerationFactor;
			//0 => 0
			//.5 => 1
			//1 => 2
		}
	}
	/*
	if(hi)
	{
		brightFactor = 4.0f;
		if(LGL_GetXponent())
		{
			float knobHigh = LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_LEFT_HIGH);
			if(knobHigh!=-1.0f)
			{
				brightFactor = brightFactor*2.0f*knobHigh;
			}
		}
	}
	else
	{
		brightFactor = 0.5f;
		if(LGL_GetXponent())
		{
			float knobLow = LGL_GetXponent()->GetKnobStatus(LGL_XPONENT_KNOB_LEFT_LOW);
			if(knobLow!=-1.0f)
			{
				brightFactor = brightFactor*4.0f*knobLow;
			}
		}
	}
	*/
	float ret=freqMag*brightFactor;

	if(hi)
	{
		if(ret<0)
		{
			ret*=ret;
		}
		else
		{
			ret=sqrtf(ret);
		}
	}

	if(ret>2.0f)
	{
		ret=2.0f;
	}

	return(ret);
}

LGL_Color
GetColorFromScalar
(
	float	scalar
)
{
	scalar=LGL_Clamp(0.0f,scalar,1.0f);

	const float rad=1.0f/3.0f;

	LGL_Color ret;
	ret.SetR
	(
		LGL_Clamp
		(
			0.0f,
			LGL_Clamp
			(
				0.0f,
				2.0f*(rad-fabsf(scalar-0*rad))/rad,
				1.0f
			) +
			LGL_Clamp
			(
				0.0f,
				(rad-fabsf(scalar-3*rad))/rad,
				1.0f
			),
			1.0f
		)
	);
	ret.SetG
	(
		LGL_Clamp
		(
			0.0f,
			2.0f*(rad-fabsf(scalar-1*rad))/rad,
			1.0f
		)
	);
	ret.SetB
	(
		LGL_Clamp
		(
			0.0f,
			2.0f*(rad-fabsf(scalar-2*rad))/rad,
			1.0f
		)
	);

	return(ret);
}

