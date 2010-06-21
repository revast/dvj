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
	float quadrentSplitX = visualizerRight;
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
	if(EIGHT_WAY==false)
	{
		float myX = quadrentSplitX + 1.0f/LGL_DisplayResolutionX(0);
		if(myX<=1.0f)
		{
			LGL_DrawLineToScreen
			(
				myX,quadrentSplitY,
				myX,1,
				2*coolR*glow,2*coolG*glow,2*coolB*glow,glow,
				1,
				false
			);
		}
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
		if(0 && EIGHT_WAY)
		{
			LGL_DrawLineToScreen
			(
				0.25f,quadrentSplitY,
				0.25f,1,
				2*coolR*br,2*coolG*br,2*coolB*br,0.5f*br,
				7-a,
				false
			);
			LGL_DrawLineToScreen
			(
				0.75f,quadrentSplitY,
				0.75f,1,
				2*coolR*br,2*coolG*br,2*coolB*br,0.5f*br,
				7-a,
				false
			);
			LGL_DrawLineToScreen
			(
				0,0.75f,
				1,0.75f,
				2*coolR*br,2*coolG*br,2*coolB*br,0.5f*br,
				7-a,
				false
			);
		}
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
	float viewPortLeft = 0.0f;
	float viewPortRight = 1.0f;
	float viewPortBottom = 0.0f;
	float viewPortTop = 0.5f;
	//float viewPortWidth = viewPortRight - viewPortLeft;
	float viewPortHeight = viewPortTop - viewPortBottom;

	for(float a=1;a<7;a++)
	{
		float br=glow*a/7.0f;
		LGL_ClipRectEnable
		(
			visualizerQuadrent?0.0f:viewPortLeft,
			visualizerQuadrent?0.5f:viewPortRight,
			visualizerQuadrent?0.5f:viewPortBottom,
			visualizerQuadrent?1.0f:viewPortTop
		);
		LGL_DrawLineToScreen
		(
			0.025,viewPortBottom+.5*viewPortHeight,
			0.975,viewPortBottom+.5*viewPortHeight,
			2*coolR*br,2*coolG*br,2*coolB*br,br,
			7-a,
			false
		);
		for(int zz=1;zz<=5;zz++)
		{
			LGL_DrawLineToScreen
			(
				0,crossFadeSliderLeft*viewPortTop,
				.025,crossFadeSliderLeft*viewPortTop,
				warmR*br,warmG*br,warmB*br,br,
				zz*(7-a),
				false
			);
			LGL_DrawLineToScreen
			(
				0.975,crossFadeSliderRight*viewPortTop,
				1,crossFadeSliderRight*viewPortTop,
				warmR*br,warmG*br,warmB*br,br,
				zz*(7-a),
				false
			);
		}
		if(visualizerQuadrent==false) LGL_ClipRectDisable();
		LGL_DrawLineToScreen
		(
			0.975,viewPortTop,
			0.975,0,
			2*coolR*br,2*coolG*br,2*coolB*br,br,
			7-a,
			false
		);
		LGL_DrawLineToScreen
		(
			0.025,viewPortTop,
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
	float viewPortBottom,
	float viewPortTop,
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
	float viewPortHeight=viewPortTop-viewPortBottom;
	for(int a=0;a<2;a++)
	{
		/*
		LGL_GetFont().DrawString
		(
			.025*.5,
			viewPortBottom+.245*viewPortHeight+(1.5)*.5*viewPortHeight,
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
			viewPortBottom+.245f*viewPortHeight+(1-a)*.5f*viewPortHeight,
			.010f,
			1,1,1,1,
			true,.5f,
			"%.0f",
			(a==1) ? leftBottomLevel : leftTopLevel
		);
		LGL_GetFont().DrawString
		(
			1.0f-.025f*.5f,
			viewPortBottom+.245f*viewPortHeight+(1-a)*.5f*viewPortHeight,
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
	float		time,
	const char*	filterText,
	const char*	path,
	const char**	nameArray,
	bool*		isDirBits,
	bool*		loadableBits,
	bool*		alreadyPlayedBits,
	int		fileNum,
	int		fileSelectInt,
	float		viewPortBottom,
	float		viewPortTop,
	float		badFileFlash,
	float*		inBPMList
)
{
	float coolR;
	float coolG;
	float coolB;
	GetColorCool(coolR,coolG,coolB);

	float glow = GetGlowFromTime(time);
	float centerX = 0.5f;
	float viewPortLeft = 0.025f;
	float viewPortRight = 0.975f;
	float viewPortWidth = viewPortRight - viewPortLeft;
	float viewPortHeight = viewPortTop - viewPortBottom;
	LGL_GetFont().DrawString
	(
		centerX,viewPortBottom+.875*viewPortHeight,.025,
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
					viewPortLeft,viewPortRight,
					viewPortTop-(.1f+(b+1)/6.25f+.02f)*viewPortHeight,
					viewPortTop-(.1f+(b+1)/6.25f+.02f)*viewPortHeight+viewPortHeight/15.0f+.04f*viewPortHeight,
					R,G,B,A
				);
			}

			float stringY=viewPortTop-0.1f*viewPortHeight-(viewPortHeight*((b+1)/6.25f-0.035f));

			float bpm = inBPMList[b];
			if(bpm>0)
			{
				LGL_GetFont().DrawString
				(
					viewPortLeft+0.01f*viewPortWidth,
					stringY-0.5f*viewPortHeight/15.0f,
					viewPortHeight/15.0f,
					R,G,B,1,
					false,0,
					"%.0f",
					bpm
				);
			}

			float fontHeight=viewPortHeight/15.0f;
			float fontWidth=LGL_GetFont().GetWidthString(fontHeight,fileNow);
			float fontWidthMax=viewPortWidth*0.9f;
			fontHeight=LGL_Min(fontHeight,fontHeight*fontWidthMax/fontWidth);

			LGL_GetFont().DrawString
			(
				viewPortLeft+(.025f+0.05f)*viewPortWidth,
				stringY-0.5f*fontHeight,
				fontHeight,
				R,G,B,1,
				false,0,
				fileNow
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
	float		viewPortLeft,
	float		viewPortRight,
	float		viewPortBottom,
	float		viewPortTop,
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
	int		freqSensitiveMode,
	float		warpPointSecondsStart,
	float		warpPointSecondsTrigger,
	int		loopExponent,
	float		loopSeconds,
	bool		waveformRecordHold,
	const char*	soundName,
	float		videoSecondsBufferedLeft,
	float		videoSecondsBufferedRight,
	bool		isMaster,
	bool		rapidVolumeInvertSelf,
	float		beginningOfCurrentMeasureSeconds
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
	float viewPortWidth = viewPortRight - viewPortLeft;
	float viewPortHeight = viewPortTop - viewPortBottom;
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

	long pos=(long)soundPositionSamples;

	//Smooth Zooming Waveform Renderer

	float zoom=4444.0f;
	if(pitchBend!=0.0f)
	{
		zoom=1.0f/pitchBend;
	}

	long pointResolution=256+1;

	float pointRadiusMin=viewPortRight-(viewPortLeft+0.5f*viewPortWidth);
	float pointRadius=pointRadiusMin*zoom;

	float percentTowardsNextZoomInLevel=(pointRadius-pointRadiusMin)/pointRadiusMin;

	int sampleRadius=(int)(512*64*SAMPLE_RADIUS_MULTIPLIER);	//This number is arbitrary and possibly magical for an unknown reason.
	long sampleLeft=pos-sampleRadius;
	long sampleRight=pos+sampleRadius;
	long sampleWidth=sampleRight-sampleLeft;

	float viewPortCenter=viewPortLeft+0.5f*viewPortWidth;
	float pointLeft=	viewPortCenter-(0.5f+0.5f*percentTowardsNextZoomInLevel)*(WAVE_WIDTH_PERCENT*viewPortWidth);
	float pointRight=	viewPortCenter+(0.5f+0.5f*percentTowardsNextZoomInLevel)*(WAVE_WIDTH_PERCENT*viewPortWidth);
	float pointWidth=pointRight-pointLeft;
	float pointBottom=	viewPortBottom+0.125*viewPortHeight;
	float pointTop=		viewPortBottom+0.875*viewPortHeight;
	float pointHeight=	pointTop-pointBottom;
	float pointYMid=	0.5f*(pointBottom+pointTop);
	
	float wavLeft = viewPortCenter-0.5f*WAVE_WIDTH_PERCENT*viewPortWidth;
	float wavRight=	viewPortCenter+0.5f*WAVE_WIDTH_PERCENT*viewPortWidth;
	float wavWidth= wavRight-wavLeft;

	LGL_ClipRectEnable
	(
		wavLeft,
		wavRight,
		pointBottom,
		pointTop+0.01f
	);

	if(freqSensitiveMode==2)
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
			GetFreqBrightness(false,freqFactor,2*volAve*volumeMultiplierNow) ||
			GetFreqBrightness(true,freqFactor,2*volAve*volumeMultiplierNow)
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
				if(freqSensitiveMode==1)
				{
					magnitudeAve*=(GetFreqBrightness(false,zeroCrossingFactor,magnitudeAve/sound->GetVolumePeak()) + zeroCrossingFactor > 0.0f) ? 1.0f : 0.0f;
				}

				magnitudeAve*=volumeMultiplierNow*0.5f;
				magnitudeMax*=volumeMultiplierNow*0.5f;

				double sampleNowDouble=(sampleLeft+sampleWidth*(z/(double)pointResolution));
				float xOffset=1.0f-(sampleNow-sampleNowDouble)/(float)deltaSample;

				const float xFudgeFactor = 0.0045;
				arrayV[(a*2)+0]=pointLeft+((z-xOffset)/(float)pointResolution)*pointWidth+xFudgeFactor;
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
						arrayV[(a*2)+0] >= centerX+needleDeltaL*viewPortWidth &&
						arrayV[(a*2)+0] <= centerX+needleDeltaR*viewPortWidth
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
		for(int a=0;a<pointsToDrawIndexEnd/*-pointsToDrawIndexStart*/;a++)
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

		//Draw Center Needle Rectangle
		LGL_DrawRectToScreen
		(
			centerX+needleDeltaL*viewPortWidth,
			centerX+needleDeltaR*viewPortWidth,
			pointBottom+NEEDLE_DISTANCE_FROM_EDGES*pointHeight,
			pointTop-NEEDLE_DISTANCE_FROM_EDGES*pointHeight,
			0.25f*warmR,0.25f*warmG,0.25f*warmB,0.0f
		);

		//Draw video buffer
		if
		(
			videoSecondsBufferedLeft != 0 ||
			videoSecondsBufferedRight != 0
		)
		{
			float secondsRadius = (soundPositionSamples - sampleLeft)/44100.0f;
			//float secondsNow = soundPositionSamples/44100.0f;
			float videoBufferedLeft = centerX+(pointLeft-centerX)*(videoSecondsBufferedLeft/secondsRadius);
			float videoBufferedRight = centerX+(pointRight-centerX)*(videoSecondsBufferedRight/secondsRadius);
			LGL_DrawLineToScreen
			(
				centerX,
				pointBottom,
				videoBufferedLeft,
				pointBottom,
				warmR,warmG,warmB,1.0f
			);
			LGL_DrawLineToScreen
			(
				centerX,
				pointBottom,
				videoBufferedRight,
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
			centerX+needleDeltaL*viewPortWidth,pointBottom+NEEDLE_DISTANCE_FROM_EDGES*pointHeight,
			centerX+needleDeltaL*viewPortWidth,pointTop-NEEDLE_DISTANCE_FROM_EDGES*pointHeight,
			warmR,warmG,warmB,1.0f,
			1
		);
		LGL_DrawLineToScreen
		(
			centerX+needleDeltaR*viewPortWidth,pointBottom+NEEDLE_DISTANCE_FROM_EDGES*pointHeight,
			centerX+needleDeltaR*viewPortWidth,pointTop-NEEDLE_DISTANCE_FROM_EDGES*pointHeight,
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

	if(freqSensitiveMode!=2)
	{
		float noiseBottom=viewPortBottom;
		float noiseTop=pointBottom;
		float noiseLeft=viewPortLeft + (warpPointSecondsStart/cachedLengthSeconds)*viewPortWidth;
		float noiseRight=viewPortLeft + (warpPointSecondsTrigger/cachedLengthSeconds)*viewPortWidth;
		float noiseHeight=noiseTop-noiseBottom;
		float noiseWidth=noiseRight-noiseLeft;
		float noiseRelativeWidth=noiseWidth/viewPortWidth;

		//Background loop noise
		if(warpPointSecondsStart!=-1.0f)
		{
			noiseImage256x64->DrawToScreen
			(
				noiseLeft,noiseRight,
				noiseBottom,noiseTop,
				0,
				warmR,warmG,warmB,0.0f,
				false,false,0,0,0,
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
			float waveBottom=viewPortBottom;
			float waveTop=pointBottom;
			float waveHeight=waveTop-waveBottom;

			for(int a=0;a<entireWaveArrayFillIndex;a++)
			{
				float zeroCrossingFactor=entireWaveArrayFreqFactor[a];
				float magnitudeAve=entireWaveArrayMagnitudeAve[a];
				float magnitudeMax=entireWaveArrayMagnitudeMax[a];
				bool overdriven=(magnitudeMax*volumeMultiplierNow)>1.0f;
				float entirePercent = a/(float)(entireWaveArrayCount-1);

				zeroCrossingFactor=GetFreqBrightness(true,zeroCrossingFactor,magnitudeAve/sound->GetVolumePeak());

				entireWaveArrayLine1Points[a*2+0]=viewPortLeft+(a/(float)entireWaveArrayCount)*viewPortWidth;
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

				entireWaveArrayLine2Points[a*2+0]=viewPortLeft+(a/(float)entireWaveArrayCount)*viewPortWidth;
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
							viewPortLeft+viewPortWidth*savePointPercent,waveBottom,
							viewPortLeft+viewPortWidth*savePointPercent,waveTop,
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
				float pos=soundPositionSeconds/cachedLengthSeconds;
				LGL_DrawLineToScreen
				(
					viewPortLeft+pos*viewPortWidth,
					waveBottom,
					viewPortLeft+pos*viewPortWidth,
					waveTop,
					1,1,1,1,
					4.0f,
					false
				);
				LGL_DrawLineToScreen
				(
					viewPortLeft+pos*viewPortWidth,
					waveBottom,
					viewPortLeft+pos*viewPortWidth,
					waveTop,
					warmR,warmG,warmB,1,
					2.0f,
					false
				);
			}
		}
	}

	//Draw Text
	char tmp[2048];
	strcpy(tmp,soundName);
	if(strstr(tmp,".mp3"))
	{
		strstr(tmp,".mp3")[0]='\0';
	}
	if(strstr(tmp,".ogg"))
	{
		strstr(tmp,".ogg")[0]='\0';
	}
	if(freqSensitiveMode==2)
	{
		strcpy(tmp,"Audio Input");
	}

	float txtCenterX=centerX+0.25f*viewPortWidth;
	float fontWidthMax=viewPortWidth*0.45f;
	if(videoPathShort==NULL || 1)
	{
		txtCenterX=centerX;
		fontWidthMax=viewPortWidth*0.9f;
	}

	float fontHeight=0.05f*viewPortHeight;
	float fontWidth=LGL_GetFont().GetWidthString(fontHeight,tmp);
	fontHeight=LGL_Min(fontHeight,fontHeight*fontWidthMax/fontWidth);

	LGL_GetFont().DrawString
	(
		txtCenterX,
		viewPortBottom+.925f*viewPortHeight-0.5f*fontHeight,
		fontHeight,
		1,1,1,1,
		true,.5f,
		tmp
	);

	fontHeight=0.05f*viewPortHeight;
	fontWidth=LGL_GetFont().GetWidthString(fontHeight,tmp);
	fontHeight=LGL_Min(fontHeight,fontHeight*0.45f*viewPortWidth/fontWidth);

	if(videoPathShort && 0)
	{
		strcpy(tmp,videoPathShort);
		if(strstr(tmp,".mp3"))
		{
			strstr(tmp,".mp3")[0]='\0';
		}

		LGL_GetFont().DrawString
		(
			centerX-0.25f*viewPortWidth,
			viewPortBottom+.925f*viewPortHeight-0.5f*fontHeight,
			fontHeight,
			1,1,1,1,
			true,.5f,
			tmp
		);
	}

	char temp[256];
	/*
	if
	(
		pause==0 &&
		grainStreamCrossfader==0.0f
	)
	{
		sprintf(temp,"off");
	}
	else
	{
		float num=
			(1.0f-grainStreamCrossfader) * soundSpeed*100 +
			(0.0f+grainStreamCrossfader) * 2.0f*joyAnalogueStatusLeftX*100;
		sprintf(temp,"%.2f",num);
	}
	LGL_GetFont().DrawString
	(
		viewPortLeft+.02f*viewPortWidth,
		viewPortBottom+.80f*viewPortHeight,
		viewPortHeight/15.0f,
		1,1,1,1,
		false,.5f,
		"Speed:"
	);
	LGL_GetFont().DrawString
	(
		viewPortLeft+.125f*viewPortWidth,
		viewPortBottom+.80f*viewPortHeight,
		viewPortHeight/15.0f,
		1,1,1,1,
		false,.5f,
		temp
	);
	*/

	float eqLeftEdge=0.0f;
	float eqRightEdge=0.0f;
	if(freqSensitiveMode!=2)
	{
		if(bpmAdjusted>0)
		{
			LGL_GetFont().DrawString
			(
				//viewPortLeft+.02f*viewPortWidth,
				0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.009f,
				viewPortBottom+.80f*viewPortHeight,
				0.05f*viewPortHeight,
				1,1,1,1,
				false,.5f,
				"BPM:"
			);
			LGL_GetFont().DrawString
			(
				//viewPortLeft+.125f*viewPortWidth,
				0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.06f-0.0095f,
				viewPortBottom+.80f*viewPortHeight,
				0.05f*viewPortHeight,
				1,1,1,1,
				false,.5f,
				"%.2f",
				bpmAdjusted
			);

			if(0 && isMaster)
			{
				float masterBoxLeft=0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.006f;
				float masterBoxRight=0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.105f;
				float masterBoxBottom=viewPortBottom+.78f*viewPortHeight;
				float masterBoxTop=viewPortBottom+.88f*viewPortHeight;
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
			sprintf(temp,"+%.2f",pbAbs);
		}
		else
		{
			sprintf(temp,"-%.2f",pbAbs);
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
			//viewPortLeft+.02f*viewPortWidth,
			0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.009f,
			viewPortBottom+.70f*viewPortHeight,
			0.05f*viewPortHeight,
			1,1,1,1,
			false,.5f,
			"Pitch:"
		);
		LGL_GetFont().DrawString
		(
			//viewPortLeft+.125f*viewPortWidth,
			0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.06f,
			viewPortBottom+.70f*viewPortHeight,
			0.05f*viewPortHeight,
			1,1,1,1,
			false,.5f,
			"%s%s",
			temp,
			tempNudge
		);

		float lb=warpPointSecondsStart>=0.0f ? 1.0f : 0.25f;

		loopImage->DrawToScreen
		(
			0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.009f+0.115f,
			0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.009f+0.124f,
			viewPortBottom+.80f*viewPortHeight+0.005f*viewPortHeight,
			viewPortBottom+.80f*viewPortHeight+0.06f*viewPortHeight,
			0,
			lb,lb,lb,1.0f
		);

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
			0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.009f+0.1325f,
			viewPortBottom+.80f*viewPortHeight,
			0.05f*viewPortHeight,
			1,1,1,1,
			false,.5f,
			loopStr
		);
		
		/*
		LGL_GetFont().DrawString
		(
			viewPortLeft+.02f*viewPortWidth,
			viewPortBottom+.60f*viewPortHeight,
			viewPortHeight/15.0f,
			1,1,1,1,
			false,.5f,
			"Volume:"
		);
		LGL_GetFont().DrawString
		(
			viewPortLeft+.125f*viewPortWidth,
			viewPortBottom+.60f*viewPortHeight,
			viewPortHeight/15.0f,
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
				sprintf(temp,"0%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(temp,"0%.0f:%.2f",minutes,seconds);
			}
		}
		else
		{
			if(seconds<10)
			{
				sprintf(temp,"%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(temp,"%.0f:%.2f",minutes,seconds);
			}
		}

		LGL_GetFont().DrawString
		(
			viewPortLeft+.02f*viewPortWidth,
			viewPortBottom+.45f*viewPortHeight,
			viewPortHeight/15.0f,
			1,1,1,1,
			false,.5f,
			"Length:"
		);
		LGL_GetFont().DrawString
		(
			viewPortLeft+.125f*viewPortWidth,
			viewPortBottom+.45f*viewPortHeight,
			viewPortHeight/15.0f,
			1,1,1,1,
			false,.5f,
			temp
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
				sprintf(temp,"0%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(temp,"0%.0f:%.2f",minutes,seconds);
			}
		}
		else
		{
			if(seconds<10)
			{
				sprintf(temp,"%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(temp,"%.0f:%.2f",minutes,seconds);
			}
		}
		/*
		LGL_GetFont().DrawString
		(
			//viewPortLeft+.02f*viewPortWidth,
			0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.009f,
			viewPortBottom+.25f*viewPortHeight,
			0.05f*viewPortHeight,
			1,1,1,1,
			false,.5f,
			"Position:"
		);
		*/
		float posElapsedRemainingHeight=viewPortBottom+.15f*viewPortHeight;
		LGL_GetFont().DrawString
		(
			//viewPortLeft+.125f*viewPortWidth,
			//0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.11f,
			0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.009f,
			posElapsedRemainingHeight,
			0.05f*viewPortHeight,
			1,1,1,1,
			false,.5f,
			temp
		);
		
		LGL_GetFont().DrawString
		(
			//viewPortLeft+.125f*viewPortWidth,
			//0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.11f,
			0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.095f,
			posElapsedRemainingHeight,
			0.05f*viewPortHeight,
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
				sprintf(temp,"0%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(temp,"0%.0f:%.2f",minutes,seconds);
			}
		}
		else
		{
			if(seconds<10)
			{
				sprintf(temp,"%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(temp,"%.0f:%.2f",minutes,seconds);
			}
		}
		/*
		LGL_GetFont().DrawString
		(
			//viewPortLeft+.02f*viewPortWidth,
			0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.009f,
			viewPortBottom+.15f*viewPortHeight,
			0.05f*viewPortHeight,
			1,1,1,1,
			false,.5f,
			"Remaining:"
		);
		*/
		LGL_GetFont().DrawString
		(
			//viewPortLeft+.125f*viewPortWidth,
			0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.11f,
			posElapsedRemainingHeight,
			0.05f*viewPortHeight,
			1,1,1,1,
			false,.5f,
			temp
		);

		/*
		float percent=soundPositionSeconds/soundLengthSeconds;
		LGL_GetFont().DrawString
		(
			viewPortLeft+.02f*viewPortWidth,
			viewPortBottom+.25f*viewPortHeight,
			viewPortHeight/15.0f,
			1,1,1,1,
			false,.5f,
			"Percent:"
		);
		LGL_GetFont().DrawString
		(
			viewPortLeft+.125f*viewPortWidth,
			viewPortBottom+.25f*viewPortHeight,
			viewPortHeight/15.0f,
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
				sprintf(temp,"0%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(temp,"0%.0f:%.2f",minutes,seconds);
			}
		}
		else
		{
			if(seconds<10)
			{
				sprintf(temp,"%.0f:0%.2f",minutes,seconds);
			}
			else
			{
				sprintf(temp,"%.0f:%.2f",minutes,seconds);
			}
		}
		LGL_GetFont().DrawString
		(
			//viewPortLeft+.02f*viewPortWidth,
			0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.009f,
			viewPortBottom+.15f*viewPortHeight,
			0.05f*viewPortHeight,
			1,1,1,1,
			false,.5f,
			"Remaining:"
		);
		LGL_GetFont().DrawString
		(
			//viewPortLeft+.125f*viewPortWidth,
			0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.11f,
			viewPortBottom+.15f*viewPortHeight,
			0.05f*viewPortHeight,
			1,1,1,1,
			false,.5f,
			temp
		);
		*/

		/*
		LGL_GetFont().DrawString
		(
			pointLeft+.02f*viewPortWidth,
			viewPortBottom+.02f*viewPortHeight,
			viewPortHeight/15.0f,
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

		//float lft=pointLeft+0.15f*viewPortWidth;
		float lft=0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.0075f;
		float wth=(viewPortWidth*0.25f)/16.0f;
		//float bot=(viewPortBottom+(0.015f*viewPortHeight);
		float bot=viewPortBottom+0.24f*viewPortHeight;
		float top=bot+0.075f*viewPortHeight;
		float spc=wth*0.75f;
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

			//Left
			LGL_DrawLineToScreen
			(
				lft+i*wth,bot,
				lft+i*wth,top,
				r,g,b,1,
				thickness
			);

			if(i==1)
			{
				eqLeftEdge=lft+i*wth;
			}

			//Right
			LGL_DrawLineToScreen
			(
				lft+i*wth+spc,bot,
				lft+i*wth+spc,top,
				r,g,b,1,
				thickness
			);

			if(i==11)
			{
				eqRightEdge=lft+i*wth+spc;
			}

			//Bottom
			LGL_DrawLineToScreen
			(
				lft+i*wth,bot,
				lft+i*wth+spc,bot,
				r,g,b,1,
				thickness
			);

			//Top
			LGL_DrawLineToScreen
			(
				lft+i*wth,top,
				lft+i*wth+spc,top,
				r,g,b,1,
				thickness
			);

			float savePointSetScalar = mySavePointSet[i] ? 1.0f : 0.0f;

			LGL_DrawRectToScreen
			(
				lft+i*wth,lft+i*wth+spc,
				bot,top,
				mySavePointSet[i]*coolR,
				mySavePointSet[i]*coolG,
				mySavePointSet[i]*coolB,
				1
			);
			noiseImage256x64->DrawToScreen
			(
				lft+i*wth,lft+i*wth+spc,
				bot,top,
				0,
				savePointUnsetNoisePercent[i],savePointUnsetNoisePercent[i],savePointUnsetNoisePercent[i],savePointUnsetNoisePercent[i],
				false,false,0,0,0,
				0,1.0f/32.0f,
				0,1.0f/8.0f
			);
			LGL_DrawRectToScreen
			(
				lft+i*wth,lft+i*wth+spc,
				bot,top,
				savePointUnsetFlashPercent[i],savePointUnsetFlashPercent[i],savePointUnsetFlashPercent[i],0
			);

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
					bot+0.25f*0.05f*viewPortHeight,//+0.0275f*viewPortHeight,
					0.05f*viewPortHeight,
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

	//Draw EQ

	float eq[3];
	eq[0]=0.5f*eq0;
	eq[1]=0.5f*eq1;
	eq[2]=0.5f*eq2;
	for(int f=0;f<3;f++)
	{
		//Vert
		/*
		float spc=viewPortWidth*0.0125f;
		float wth=viewPortWidth*0.015f;
		float lft=viewPortLeft+0.925f*viewPortWidth+f*(wth+spc);
		float bot=(viewPortBottom+0.145f*viewPortHeight);
		float top=(viewPortBottom+0.855f*viewPortHeight);
		*/
	
		//Horiz
		float spc=viewPortWidth*0.025f;
		//float lft=viewPortLeft+0.525f*viewPortWidth+f*(wth+spc);
		float lft=eqLeftEdge;//0.5f+0.5f*viewPortWidth*WAVE_WIDTH_PERCENT+0.025;
		float wth=eqRightEdge-lft;//viewPortWidth*0.17f;
		float bot=viewPortBottom+(0.5f-0.075f/2.0f)*viewPortHeight+spc*(f-1);
		float top=viewPortBottom+(0.5f+0.075f/2.0f)*viewPortHeight+spc*(f-1);

		int thickness;
		float r;
		float g;
		float b;

		thickness=1;
		r=1.0f;
		g=1.0f;
		b=1.0f;

		LGL_DrawRectToScreen
		(	
			lft,
			lft+wth,
			bot,top,
			0,0,0,1
		);

		float warmScale=eq[f];
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
			//Vert
			//lft,lft+wth,
			//bot,bot+eq[f]*(top-bot),
			//Horiz
			lft,lft+eq[f]*wth,
			bot,top,
			barR,
			barG,
			barB,
			1
		);

		//Left
		LGL_DrawLineToScreen
		(
			lft,bot,
			lft,top,
			r,g,b,1,
			thickness
		);

		//Right
		LGL_DrawLineToScreen
		(
			lft+wth,bot,
			lft+wth,top,
			r,g,b,1,
			thickness
		);

		//Bottom
		LGL_DrawLineToScreen
		(
			lft,bot,
			lft+wth,bot,
			r,g,b,1,
			thickness
		);

		//Top
		LGL_DrawLineToScreen
		(
			lft,top,
			lft+wth,top,
			r,g,b,1,
			thickness
		);

		char str[8];

		if(f==0)
		{
			strcpy(str,"L");
		}
		else if(f==1)
		{
			strcpy(str,"M");
		}
		else //if(f==2)
		{
			strcpy(str,"H");
		}
		LGL_GetFont().DrawString
		(
			lft-0.0125f,
			bot+0.5f*(top-bot)-0.025f*viewPortHeight,
			0.05f*viewPortHeight,
			1,1,1,1,
			true,
			0.75f,
			str
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
	float	vol
)
{
	float freqMag;
	if(hi==false)
	{
		float minVol=0.2f;
		float maxVol=0.5f;
		float volMag=LGL_Clamp(0,(vol-minVol)/(maxVol-minVol),1);
		float myFreqMag=1.0f-freqFactor;
		float minFreqMag=0.35f;
		freqMag=LGL_Clamp
		(
			0.0f,
			volMag*(myFreqMag-minFreqMag)/(1.0f-minFreqMag),
			1.0f
		);
	}
	else
	{
		float minVol=0.25f;
		float maxVol=0.5f;
		float volMag=LGL_Clamp(0,(vol-minVol)/(maxVol-minVol),1);
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
		freqMag*=volMag;
	}
	float brightFactor = hi ? 4.0f : 0.25f;
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
/*
if(hi)
{
	if(ret>0.0f)
	{
		if(ret>0.25f) printf("\t\t");
		if(ret>0.5f) printf("\t\t");
		if(ret>1.0f) printf("\t\t");
		if(ret>2.0f) printf("\t\t");
		printf("(%.2f,%.2f) => %.2f\n",freqFactor,vol,ret);
	}
	else
	{
		printf("\n");
	}
}
*/
	return(ret);
}

