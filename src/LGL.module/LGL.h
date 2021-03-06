/*
 *
 * LGL.h
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

//Init + ShutDown

#ifndef DEFINE_LGL
#define DEFINE_LGL

#ifdef	LGL_LINUX

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#ifndef	LGL_OSX
#define	LGL_LINUX_WIIMOTE
#include <cwiid.h>
#include <sys/soundcard.h>
#endif	//LGL_OSX

#include <fcntl.h>
#include <unistd.h>
#endif	//LGL_LINUX

#ifdef	LGL_WIN32
#include <windows.h>
#define	strcasecmp	strcmpi
#endif	//WIN32

#include "SDL/SDL_opengl.h"
#ifndef	LGL_OSX
#include <GL/gl.h>
#include <GL/glext.h>
//#define	GL_PIXEL_UNPACK_BUFFER GL_PIXEL_UNPACK_BUFFER_ARB
#endif	//LGL_OSX

#include <SDL/SDL.h>
#include <SDL/SDL_main.h>
#include <SDL/SDL_scancode.h>
#include <SDL/SDL_compat.h>

#include <SDL/SDL_thread.h>
#include <pthread.h>

#include <fftw3.h>

#include "RtMidi.module/RtMidi.h"

//OSC
#include <iostream>
#include "oscpack.module/OscReceivedElements.h"
#include "oscpack.module/OscPacketListener.h"
#include "oscpack.module/OscOutboundPacketStream.h"
#include "oscpack.module/UdpSocket.h"

//Syphon
#include "syphon.module/lgl_syphon.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#include <vector>
#include <deque>
#include <list>
#include <map>
#include <algorithm>
#include <sys/mman.h>

extern "C"
{
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
}

const int LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE = (AVCODEC_MAX_AUDIO_FRAME_SIZE*2);

enum
{
	LGL_KEY_NONE		= 0,
	LGL_KEY_UNKNOWN		= 1,
	LGL_KEY_BACKSPACE	= SDLK_BACKSPACE,
	LGL_KEY_TAB		= SDLK_TAB,
	LGL_KEY_CLEAR		= SDL_SCANCODE_CLEAR,
	LGL_KEY_RETURN		= SDLK_RETURN,
	LGL_KEY_PAUSE		= SDL_SCANCODE_PAUSE,
	LGL_KEY_ESCAPE		= SDLK_ESCAPE,
	LGL_KEY_SPACE		= 32,//SDL_SCANCODE_SPACE,
	LGL_KEY_EXCLAM		= SDL_SCANCODE_KP_EXCLAM,
	LGL_KEY_QUOTEDBL	= SDLK_QUOTEDBL,
	LGL_KEY_HASH		= SDLK_HASH,
	LGL_KEY_DOLLAR		= SDLK_DOLLAR,
	LGL_KEY_AMPERSAND	= SDLK_AMPERSAND,
	LGL_KEY_QUOTE		= SDLK_QUOTE,
	LGL_KEY_LEFTPAREN	= SDLK_LEFTPAREN,
	LGL_KEY_RIGHTPAREN	= SDLK_RIGHTPAREN,
	LGL_KEY_ASTERISK	= SDLK_ASTERISK,
	LGL_KEY_PLUS		= SDLK_PLUS,
	LGL_KEY_COMMA		= 44,//SDL_SCANCODE_COMMA,
	LGL_KEY_MINUS		= 45,//SDL_SCANCODE_MINUS,
	LGL_KEY_PERIOD		= 46,//SDL_SCANCODE_PERIOD,
	LGL_KEY_SLASH		= 47,//SDL_SCANCODE_SLASH,
	LGL_KEY_BACKSLASH	= 92,//SDL_SCANCODE_BACKSLASH,
	LGL_KEY_VERTICALBAR	= SDL_SCANCODE_KP_VERTICALBAR,
	LGL_KEY_0		= SDL_SCANCODE_0,
	LGL_KEY_1		= SDL_SCANCODE_1,
	LGL_KEY_2		= SDL_SCANCODE_2,
	LGL_KEY_3		= SDL_SCANCODE_3,
	LGL_KEY_4		= SDL_SCANCODE_4,
	LGL_KEY_5		= SDL_SCANCODE_5,
	LGL_KEY_6		= SDL_SCANCODE_6,
	LGL_KEY_7		= SDL_SCANCODE_7,
	LGL_KEY_8		= SDL_SCANCODE_8,
	LGL_KEY_9		= SDL_SCANCODE_9,
	LGL_KEY_COLON		= SDLK_COLON,
	LGL_KEY_SEMICOLON	= ';',
	LGL_KEY_LESS		= SDLK_LESS,
	LGL_KEY_EQUALS		= 61,
	LGL_KEY_GREATER		= SDLK_GREATER,
	LGL_KEY_QUESTION	= SDLK_QUESTION,
	LGL_KEY_AT		= SDLK_AT,
	LGL_KEY_LEFTBRACKET	= '[',
	LGL_KEY_RIGHTBRACKET	= ']',
	LGL_KEY_CARET		= SDLK_CARET,
	LGL_KEY_UNDERSCORE	= SDLK_UNDERSCORE,
	LGL_KEY_BACKQUOTE	= SDLK_BACKQUOTE,
	//LGL_KEY_TILDE		= '~',
	LGL_KEY_A		= SDLK_a,
	LGL_KEY_B		= SDLK_b,
	LGL_KEY_C		= SDLK_c,
	LGL_KEY_D		= SDLK_d,
	LGL_KEY_E		= SDLK_e,
	LGL_KEY_F		= SDLK_f,
	LGL_KEY_G		= SDLK_g,
	LGL_KEY_H		= SDLK_h,
	LGL_KEY_I		= SDLK_i,
	LGL_KEY_J		= SDLK_j,
	LGL_KEY_K		= SDLK_k,
	LGL_KEY_L		= SDLK_l,
	LGL_KEY_M		= SDLK_m,
	LGL_KEY_N		= SDLK_n,
	LGL_KEY_O		= SDLK_o,
	LGL_KEY_P		= SDLK_p,
	LGL_KEY_Q		= SDLK_q,
	LGL_KEY_R		= SDLK_r,
	LGL_KEY_S		= SDLK_s,
	LGL_KEY_T		= SDLK_t,
	LGL_KEY_U		= SDLK_u,
	LGL_KEY_V		= SDLK_v,
	LGL_KEY_W		= SDLK_w,
	LGL_KEY_X		= SDLK_x,
	LGL_KEY_Y		= SDLK_y,
	LGL_KEY_Z		= SDLK_z,
	LGL_KEY_DELETE		= SDL_SCANCODE_DELETE,
	LGL_KEY_KP_0		= SDL_SCANCODE_KP_0,
	LGL_KEY_KP_1		= SDL_SCANCODE_KP_1,
	LGL_KEY_KP_2		= SDL_SCANCODE_KP_2,
	LGL_KEY_KP_3		= SDL_SCANCODE_KP_3,
	LGL_KEY_KP_4		= SDL_SCANCODE_KP_4,
	LGL_KEY_KP_5		= SDL_SCANCODE_KP_5,
	LGL_KEY_KP_6		= SDL_SCANCODE_KP_6,
	LGL_KEY_KP_7		= SDL_SCANCODE_KP_7,
	LGL_KEY_KP_8		= SDL_SCANCODE_KP_8,
	LGL_KEY_KP_9		= SDL_SCANCODE_KP_9,
	LGL_KEY_KP_PERIOD	= SDL_SCANCODE_KP_PERIOD,
	LGL_KEY_KP_DIVIDE	= SDL_SCANCODE_KP_DIVIDE,
	LGL_KEY_KP_MULTIPLY	= SDL_SCANCODE_KP_MULTIPLY,
	LGL_KEY_KP_MINUS	= SDL_SCANCODE_KP_MINUS,
	LGL_KEY_KP_PLUS		= SDL_SCANCODE_KP_PLUS,
	LGL_KEY_KP_ENTER	= SDL_SCANCODE_KP_ENTER,
	LGL_KEY_KP_EQUALS	= SDL_SCANCODE_KP_EQUALS,
	LGL_KEY_UP		= 338,
	LGL_KEY_DOWN		= 337,
	LGL_KEY_LEFT		= 336,
	LGL_KEY_RIGHT		= 335,
	LGL_KEY_INSERT		= SDL_SCANCODE_INSERT,
	LGL_KEY_HOME		= 330,
	LGL_KEY_END		= 333,
	LGL_KEY_PAGEUP		= 331,
	LGL_KEY_PAGEDOWN	= 334,
	LGL_KEY_F1		= 314,
	LGL_KEY_F2		= 315,
	LGL_KEY_F3		= 316,
	LGL_KEY_F4		= 317,
	LGL_KEY_F5		= 318,
	LGL_KEY_F6		= 319,
	LGL_KEY_F7		= 320,
	LGL_KEY_F8		= 321,
	LGL_KEY_F9		= 322,
	LGL_KEY_F10		= 323,
	LGL_KEY_F11		= 324,
	LGL_KEY_F12		= 325,
	LGL_KEY_F13		= 326,
	LGL_KEY_F14		= 327,
	LGL_KEY_F15		= 328,
	LGL_KEY_NUMLOCK		= SDLK_NUMLOCK,
	LGL_KEY_CAPSLOCK	= SDL_SCANCODE_CAPSLOCK,
	LGL_KEY_SCROLLLOCK	= SDL_SCANCODE_SCROLLLOCK,
	LGL_KEY_SHIFT		= 500,
	LGL_KEY_LSHIFT		= 481,//SDL_SCANCODE_LSHIFT,
	LGL_KEY_RSHIFT		= 485,//SDL_SCANCODE_RSHIFT,
	LGL_KEY_LCTRL		= 480,//SDL_SCANCODE_LCTRL,
	LGL_KEY_RCTRL		= SDL_SCANCODE_RCTRL,
	LGL_KEY_LALT		= 482,//SDL_SCANCODE_LALT,
	LGL_KEY_RALT		= 486,//SDL_SCANCODE_RALT,
	LGL_KEY_LCOMMAND	= 483,//SDL_SCANCODE_LCMD,
	LGL_KEY_RCOMMAND	= 487,//SDL_SCANCODE_RCMD,
	LGL_KEY_MODE		= SDL_SCANCODE_MODE,
	LGL_KEY_APPLICATION	= SDL_SCANCODE_APPLICATION,
	LGL_KEY_HELP		= SDL_SCANCODE_HELP,
	LGL_KEY_PRINTSCREEN	= SDL_SCANCODE_PRINTSCREEN,
	LGL_KEY_SYSREQ		= SDL_SCANCODE_SYSREQ,
	LGL_KEY_STOP		= SDL_SCANCODE_STOP,
	LGL_KEY_MENU		= SDL_SCANCODE_MENU,
	LGL_KEY_POWER		= SDL_SCANCODE_POWER,
	LGL_KEY_EURO		= SDLK_EURO,
	LGL_KEY_UNDO		= SDL_SCANCODE_UNDO,
	LGL_KEY_MAX		= 512
};

#define LGL_PI	3.1415926535
#define LGL_DISPLAY_MAX	2

#define LGL_EQ_SAMPLES_FFT	(1024*4)

#define	LGL_EQ_LOWMID	(3)
#define	LGL_EQ_MIDHIGH (16)

//Endian macros for dealing with OSX's big-endian-ness (this'll be fun when they move to intel...)
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	#define SWAP16(X)	(X)
	#define SWAP32(X)	(X)
#else
	#define SWAP16(X)	SDL_Swap16(X)
	#define SWAP32(X)	SDL_Swap32(X)
#endif

typedef struct _TCPsocket *TCPsocket;

bool		LGL_Init
		(
			int		inVideoResolutionX,
			int		inVideoResolutionY,
			int		inAudioChannels,
			const char*	inWindowTitle
		);

bool		LGL_Running();
void		LGL_SetUserExit(void(*fn)(void));
void		LGL_Exit();
void		LGL_ExitAlpha();
void		LGL_ExitOmega();

class LGL_Semaphore;



#define	LGL_OBJECT (0)

#if LGL_OBJECT

class LGL_Object
{

public:

			LGL_Object();
			~LGL_Object();

			//RefCounting
	int		GetRetainCount();
	LGL_Semaphore*	GetRetainCountSemaphore();
	void		Retain();
	void		Release();

private:

	int		RetainCount;
	LGL_Semaphore*	RetainCountSemaphore;

};

template<class T>
class LGL_ObjectSP
{

public:

			LGL_ObjectSP(LGL_Object* obj);
			~LGL_ObjectSP();
	
inline	T*		cast();
const	LGL_ObjectSP	operator=(const LGL_ObjectSP<T>& sp);

private:

	void		ReleaseObject();

	LGL_Object*	Object;

};

#endif



class LGL_Vector
{

public:

			LGL_Vector
			(
				float	x=0.0f,
				float	y=0.0f,
				float	z=0.0f
			);
			~LGL_Vector();

	float		GetX() const;
	float		GetY() const;
	float		GetZ() const;
	float		GetAngleXY() const;
	float		GetLength() const;
	float		GetLengthSquared() const;
	float		GetLengthXY() const;
	float		GetLengthXYSquared() const;

	void		SetX(const float& valueX);
	void		SetY(const float& valueY);
	void		SetZ(const float& valueZ);
	void		SetXY
			(
				const
				float&	valueX,
				const
				float&	valueY
			);
	LGL_Vector&	SetXYZ
			(
				const
				float&	valueX,
				const
				float&	valueY,
				const
				float&	valueZ
			);
	void		SetAngleXY(const float&	angle);
	void		SetLength(const float&	length);

	float		GetDistance
			(
				const
				float&	x,
				const
				float&	y,
				const
				float&	z
			);
	float		GetDistanceXY
			(
				const
				float&	x,
				const
				float&	y
			);
	float		GetDistanceSquared
			(
				const
				float&	x,
				const
				float&	y,
				const
				float&	z
			);
	float		GetDistanceSquaredXY
			(
				const
				float&	x,
				const
				float&	y
			);
	
	LGL_Vector	operator+(const LGL_Vector& vector) const;
	LGL_Vector	operator*(const float& scalar) const;

private:

	float		X;
	float		Y;
	float		Z;
};



//Time

void		LGL_Delay();
void		LGL_DelayMS(float ms);
void		LGL_DelaySeconds(float seconds);
double		LGL_SecondsSinceLastFrame();
double		LGL_SecondsSinceThisFrame();
double		LGL_SecondsSinceExecution();
unsigned long	LGL_FramesSinceExecution();
int		LGL_FPS();
int		LGL_GetFPSMax();
void		LGL_SetFPSMax(int fpsMax);
const
char*		LGL_TimeOfDay();
const
char*		LGL_DateAndTimeOfDay();
const
char*		LGL_DateAndTimeOfDayOfExecution();

class LGL_Timer
{

public:

			LGL_Timer();
			~LGL_Timer();

	double		SecondsSinceLastReset() const;
	void		Reset();

private:

	//timeval	TimeAtLastReset;
	//uint64_t	TimeAtLastResetMach;
	double		TimeAtLastResetMicroSeconds;
};



//System Info

unsigned int	LGL_CPUCount();
unsigned int	LGL_CPUSpeed(int which=-1);		//Returns MHz
int		LGL_CPUTemp();				//Returns Celsius
bool		LGL_IsOsxAppBundle();


//Threading

SDL_Thread*	LGL_ThreadCreate(int (*fn)(void*), void* data=NULL);
int		LGL_ThreadWait(SDL_Thread* target);
void		LGL_ThreadKill(SDL_Thread* target);
void		LGL_ThreadSetPriority(float priority, const char* threadName=NULL);	//0 is lowest, 1 is highest
void		LGL_ThreadSetPriorityForeground();
void		LGL_ThreadSetCPUAffinity(int cpu);

class LGL_Semaphore
{

public:

			LGL_Semaphore(const char* name, bool promiscuous=false);
			~LGL_Semaphore();

	bool		Lock
			(
				const char*	file,
				int		line,
				float		timeoutSeconds=-1		//-1=inf, never timeout
			);
	bool		Unlock();
	bool		IsLocked();
	float		SecondsLocked();
	void		PrintLockInfo();
	const char*	GetName();
	const char*	GetLockOwnerFile();
	int		GetLockOwnerLine();
	
	int		Value();


private:

	char		Name[1024];
	char		LockOwnerFile[1024];
	int		LockOwnerLine;
	SDL_sem*	Sem;
	float		TimeOfLock;

	bool		Promiscuous;
};

class LGL_ScopeLock
{

public:

			LGL_ScopeLock
			(
				const char*	file,
				int		line,
				LGL_Semaphore&	semaphore,
				float		timeoutSeconds=-1
			);
			LGL_ScopeLock
			(
				const char*	file,
				int		line,
				LGL_Semaphore*	semaphore,
				float		timeoutSeconds=-1
			);
			~LGL_ScopeLock();

	bool		GetLockObtained();

private:

	void		Init
			(
				const char*	file,
				int		line,
				LGL_Semaphore*	semaphore,
				float		timeoutSeconds=-1
			);

	LGL_Semaphore*	Semaphore;
	bool		LockObtained;
};

//Video

void		LGL_SwapBuffers(bool endFrame=true, bool clearBackBuffer=true);
void		LGL_FullScreenToggle();
void		LGL_FullScreen(bool inFullScreen);
bool		LGL_IsFullScreen();
int		LGL_WindowResolutionX();
int		LGL_WindowResolutionY();
int		LGL_DisplayCount();
int		LGL_DisplayResolutionX(int which=-1);
int		LGL_DisplayResolutionY(int which=-1);
int		LGL_DisplayRefreshRate(int which=-1);
float		LGL_WindowAspectRatio();
float		LGL_DisplayAspectRatio(int which=-1);
int		LGL_GetActiveDisplay();
void		LGL_SetActiveDisplay(int display);
bool		LGL_VSync();
void		LGL_HideProjectorWindows();
void		LGL_AudioSwapOutputStreams();
void		LGL_FakeSecondDisplay();
void		LGL_FakeSecondDisplay3x();

void		LGL_ViewportDisplay
		(
			float left=0,	float right=1,
			float bottom=0,	float top=1
		);
void		LGL_GetViewportDisplay
		(
			float&	left,	float&	right,
			float&	bottom,	float&	top
		);

void		LGL_ClipRectEnable
		(
			float left,	float right,
			float bottom,	float top
		);

void		LGL_ClipRectDisable();

void		LGL_DrawLineToScreen
		(
			float x1,	float y1,
			float x2,	float y2,
			float r=1.0f,	float g=1.0f,	float b=1.0f,	float a=1.0f,
			float thickness=1.0f,
			bool antialias=true
		);
void		LGL_DrawLineStripToScreen
		(
			float*	pointsXY,
			int	pointCount,
			float	r,
			float	g,
			float	b,
			float	a,
			float	thickness=1.0f,
			bool	antialias=true
		);
void		LGL_DrawLineStripToScreen
		(
			float*	pointsXY,
			float*	colorsRGBA,
			int	pointCount,
			float	thickness=1.0f,
			bool	antialias=true
		);
void		LGL_DrawTriStripToScreen
		(
			float*	pointsXY,
			float*	colorsRGBA,
			int	pointCount,
			bool	antialias=true
		);

void		LGL_DrawRectToScreen
		(
			float left,	float right,
			float bottom,	float top,
			float r=1,	float g=1,	float b=1,	float a=1,
			float rotation=0
		);

void		LGL_ViewportWorld
		(
			float EyeX,	float EyeY,	float EyeZ,
			float TargetX,	float TargetY,	float TargetZ,
			float UpX,	float UpY,	float UpZ
		);
//#define DEBUG
void		LGL_DrawLineToWorld
		(
			float x1,	float y1,	float z1,
			float x2,	float y2,	float z2,
			float r,	float g,	float b,	float a,
			float thickness,
			bool antialias=false
		);
void		LGL_DrawTriToWorld
		(
			float x1,	float y1,	float z1,
			float x2,	float y2,	float z2,
			float x3,	float y3,	float z3,
			float r,	float g,	float b,	float a
		);

void		LGL_SmoothLines(bool in=true);
void		LGL_SmoothPolygons(bool in=true);

void		LGL_ClearBackBuffer();	//Automatically called in LGL_SwapBuffers()

float		LGL_TextureUsageMB();

void		LGL_BlockOnGPU();

float		LGL_StatsLinesPerFrame();
float		LGL_StatsRectsPerFrame();
float		LGL_StatsImagesPerFrame();
float		LGL_StatsPixelsPerFrame();



//Shaders: This API isn't quite stablized...
	
bool		LGL_ShaderAvailable();
bool		LGL_ShaderAvailableVert();
bool		LGL_ShaderAvailableFrag();


class LGL_Shader
{

public:

			LGL_Shader(const char* inDescription);
			~LGL_Shader();

	const char*	GetDescription();
	void		SetDescription(const char* inDescription);

	bool		VertCompile(const char* inFileVert);
	bool		FragCompile(const char* inFileFrag);

	bool		VertCompiled();
	bool		FragCompiled();

	bool		Link();
	bool		IsLinked();

	bool		Enable(bool enable=true);
	bool		Disable();
	bool		IsEnabled();
	bool		EnableToggle();

	bool		SetVertAttributeInt
			(
				const char*	name,
				int		value0
			);
	bool		SetVertAttributeInt
			(
				const char*	name,
				int		value0,	int	value1
			);
	bool		SetVertAttributeInt
			(
				const char*	name,
				int		value0,	int	value1,	int	value2
			);
	bool		SetVertAttributeInt
			(
				const char*	name,
				int		value0,	int	value1,	int	value2,	int	value3
			);
	
	bool		SetVertAttributeFloat
			(
				const char*	name,
				float		value0
			);
	bool		SetVertAttributeFloat
			(
				const char*	name,
				float		value0,	float	value1
			);
	bool		SetVertAttributeFloat
			(
				const char*	name,
				float		value0,	float	value1,	float	value2
			);
	bool		SetVertAttributeFloat
			(
				const char*	name,
				float		value0,	float	value1,	float	value2,	float	value3
			);
	
	bool		SetUniformAttributeInt
			(
				const char*	name,
				int		value0
			);
	bool		SetUniformAttributeInt
			(
				const char*	name,
				int		value0,	int	value1
			);
	bool		SetUniformAttributeInt
			(
				const char*	name,
				int		value0,	int	value1,	int	value2
			);
	bool		SetUniformAttributeInt
			(
				const char*	name,
				int		value0,	int	value1,	int	value2,	int	value3
			);
	
	bool		SetUniformAttributeFloat
			(
				const char*	name,
				float		value0
			);
	bool		SetUniformAttributeFloat
			(
				const char*	name,
				float		value0,	float	value1
			);
	bool		SetUniformAttributeFloat
			(
				const char*	name,
				float		value0,	float	value1,	float	value2
			);
	bool		SetUniformAttributeFloat
			(
				const char*	name,
				float		value0,	float	value1,	float	value2,	float	value3
			);

	void		VertDelete();		//Deletes Program as well
	void		FragDelete();		//Deletes Program as well
	void		ProgramDelete();	//Keeps Vert, Frag
	void		OmniDelete();


private:

	bool		InfoLogExists(GLhandleARB obj);
	bool		InfoLogPrint(GLhandleARB obj);

	bool		SetVertAttributeIntPrivate
			(
				const char*	name,		int	num,
				int		value0,		int	value1=0,
				int		value2=0,	int	value3=1
			);
	bool		SetVertAttributeFloatPrivate
			(
				const char*	name,		int	num,
				float		value0,		float	value1=0,
				float		value2=0,	float	value3=1
			);
	bool		SetUniformAttributeIntPrivate
			(
				const char*	name,		int	num,
				int		value0,		int	value1=0,
				int		value2=0,	int	value3=1
			);
	bool		SetUniformAttributeFloatPrivate
			(
				const char*	name,		int	num,
				float		value0,		float	value1=0,
				float		value2=0,	float	value3=1
			);

	char		Description[1024];

	char		FileVert[1024];
	char		FileFrag[1024];

	GLhandleARB	VertObject;
	GLhandleARB	FragObject;
	GLhandleARB	ProgramObject;
};

bool		LGL_VBOAvailable();

bool		LGL_ShaderAvailable();
bool		LGL_ShaderAvailableVert();
bool		LGL_ShaderAvailableFrag();
void		LGL_ShaderUseVert(char* inFileVert);
void		LGL_ShaderUseFrag(char* inFileFrag);
void		LGL_ShaderUseVertFrag(char* inFileVert, char* inFileFrag);
bool		LGL_ShaderInUse();

class LGL_Image
{

//Note: The alpha channel is a purely subtractive element applied to the background

public:

			//Initialize from file
			LGL_Image
			(
				const
				char*	filename,
				bool	inLinearInterpolation=true,
				bool	loadToGLTexture=true,
				int	loadToExistantGLTexture=0,
				int	loadToExistantGLTextureX=0,
				int	loadToExistantGLTextureY=0
			);

			//Initialize from raw array
			LGL_Image
			(
				int x, int y,
				int bytesperpixel,
				unsigned char* data,
				bool inLinearInterpolation=true,
				const char* name="RAM Image"
			);

			//Initialize from glFrameBuffer
			LGL_Image
			(
				float	left,	float	right,
				float	bottom,	float	top,
				bool	inReadFromFrontBuffer=false
			);
			
			~LGL_Image();

	void		DrawToScreen
			(
				float left=0, float right=1,
				float bottom=0, float top=1,
				float rotation=0,
				float r=1, float g=1, float b=1, float a=1,
				float brightnessScalar=1.0f,
				float leftsubimage=0.0f, float rightsubimage=1.0f,
				float bottomsubimage=0.0f, float topsubimage=1.0f
			);
	void		DrawToScreen
			(
				float x[4],
				float y[4],
				float r=1, float g=1, float b=1, float a=1,
				float brightnessScalar=1.0f,
				float leftsubimage=0.0f, float rightsubimage=1.0f,
				float bottomsubimage=0.0f, float topsubimage=1.0f,
				float rgbSpatializerScalar=0.0f
			);
	void		DrawToScreen
			(
				float xDst[4],
				float yDst[4],
				float xSrc[4],
				float ySrc[4],
				float r=1, float g=1, float b=1, float a=1,
				float brightnessScalar=1.0f,
				float rgbSpatializerScalar=0.0f
			);
	void		DrawToScreenAsLine
			(
				float x1, float y1,
				float x2, float y2,
				float thickness,
				float r, float g, float b, float a,
				bool continueLastLine=false
			);

	void		DrawToWorld
			(
				float bl_x, float bl_y, float bl_z,
				float br_x, float br_y, float br_z,
				float tl_x, float tl_y, float tl_z,
				float tr_x, float tr_y, float tr_z,
				float r, float g, float b, float a
			);

	void		LoadSurfaceToTexture
			(
				bool	LinearInterpolation=true,
				int	loadToExistantGLTexture=0,
				int	loadToExistantGLTextureX=0,
				int	loadToExistantGLTextureY=0
			);
	void		UnloadSurfaceFromTexture();
	void		UpdateTexture
			(
				int w,
				int h,
				int bytesperpixel,
				unsigned char* data,
				bool inLinearInterpolation=true,
				const char* name="RAM Image"
			);
	void		UpdatePixelBufferObjects();
	void		DeletePixelBufferObjects();
	void		FrameBufferUpdate();
	void		FrameBufferViewport
			(
				float	left,	float	right,
				float	bottom,	float	top
			);
	void		FrameBufferFront(bool useFront);

	int		GetWidth();
	int		GetHeight();
	
	int		GetTexWidth();
	int		GetTexHeight();
	
const	char*		GetPath() const;
const	char*		GetPathShort() const;

	//The following functions only affect SurfaceSDL.
	//You MUST call LoadSurfaceToTexture() after for your changes to have an effect.

			//DEPRICATED
	float		GetPixelR(int x, int y);
	float		GetPixelG(int x, int y);
	float		GetPixelB(int x, int y);
	float		GetPixelA(int x, int y);
			//DEPRICATED

			//DEPRICATED
	void		SetPixelR(int x, int y, float r);
	void		SetPixelG(int x, int y, float g);
	void		SetPixelB(int x, int y, float b);
	void		SetPixelA(int x, int y, float b);
			//DEPRICATED

			//DEPRICATED (Or used by the constructor?)
	void		FileLoad(const char* inFilename);
			//DEPRICATED (Or used by the constructor?)

	void		FileSave(char* bmpFilename);

	int		GetReferenceCount();
	void		IncrementReferenceCount();
	void		DecrementReferenceCount();

	long		GetFrameNumber();
	void		SetFrameNumber(long framenumber);

	const char*	GetVideoPath();
	void		SetVideoPath(const char* videoPath);

//private:

	int		ImgW;
	int		ImgH;
	int		TexW;
	int		TexH;

	SDL_Surface*	SurfaceSDL;
	GLuint		TextureGL;
	bool		TextureGLMine;
	bool		TextureGLRect;
	GLuint		PixelBufferObjectFrontGL;
	GLuint		PixelBufferObjectBackGL;
	GLsizei		PixelBufferObjectSize;
	bool		PixelBufferVirgin;
	bool		LinearInterpolation;

	char		Path[2048];
	char		PathShort[2048];

	bool		AlphaChannel;
	bool		PixelBufferEnable;

	//glFrameBuffer stuff

	bool		FrameBufferImage;
	bool		InvertY;
	bool		ReadFromFrontBuffer;
	int		LeftInt;
	int		BottomInt;
	int		WidthInt;
	int		HeightInt;

	int		ReferenceCount;
	long		FrameNumber;
	char		VideoPath[2048];

	//YUV

	void		YUV_Construct();
	void		YUV_Destruct();
	bool		YUV_Available();
	void		YUV_ConstructTextures();
	void		YUV_DestructTextures();
	void		YUV_UpdatePixelBufferObjects();
	void		YUV_DeletePixelBufferObjects();
	void		YUV_UpdateTexture
			(
				int		w,
				int		h,
				unsigned char*	dataY,
				unsigned char*	dataU,
				unsigned char*	dataV,
				const char*	name="RAM Image"
			);

	int		YUV_ImgW;
	int		YUV_ImgH;
	int		YUV_TexW;
	int		YUV_TexH;

	GLuint		YUV_TextureGL[3];

	GLuint		YUV_PixelBufferObjectFrontGL[3];
	GLuint		YUV_PixelBufferObjectBackGL[3];
	GLsizei		YUV_PixelBufferObjectSize;

static	LGL_Shader	ImageShader;
static	LGL_Shader	YUV_ImageShader;
};

class LGL_Animation
{

public:

				LGL_Animation(const char* path, bool loadInForkedThread=false);
				~LGL_Animation();

	LGL_Image*		GetImage(unsigned int	index);
	LGL_Image*		GetImage(int		index);
	LGL_Image*		GetImage(float		zeroToOne);
	LGL_Image*		GetImage(double		zeroToOne);
	unsigned int		GetIndexFromFloat(float zeroToOne);
	unsigned int		Size();

	float			GetPercentLoaded();
	bool			IsLoaded();
	
	void			LoadImages();	// !!! Users should NEVER call this.
						// !!! Images load automatically.

const	char*			GetPath() const;
const	char*			GetPathShort() const;

	int			GetReferenceCount();
	void			IncrementReferenceCount();
	void			DecrementReferenceCount();

private:

	void			DeleteImages();

	std::vector<LGL_Image*>	Animation;
	LGL_Semaphore		AnimationSem;
	char			Path[1024];
	char			PathShort[1024];
	unsigned int		ImageCountMax;

	int			ReferenceCount;
};

enum
LGL_SpriteType
{
	LGL_SPRITE_NULL,
	LGL_SPRITE_IMAGE,
	LGL_SPRITE_ANIMATION,
};

//A wrapper for any classes which work with LGL_Images
class LGL_Sprite
{

public:

			LGL_Sprite(const char* path=NULL);
			~LGL_Sprite();

	LGL_SpriteType	GetType() const;
	const char*	GetPath() const;
	const char*	GetPathShort() const;
	LGL_Image*	GetImage() const;
	LGL_Animation*	GetAnimation() const;

	bool		SetNull();
	bool		SetPath(const char* path=NULL);
	bool		SetImage(LGL_Image* image);
	bool		SetAnimation(LGL_Animation* animation);

private:

	LGL_SpriteType	Type;
	LGL_Image*	Image;
	LGL_Animation*	Animation;
};

class lgl_FrameBuffer
{

public:

				lgl_FrameBuffer();
				~lgl_FrameBuffer();

	bool			IsLoaded();
	bool			IsReady();
	void			NullifyBuffer();
	void			SwapInNewBufferRGB
				(
					char*		videoPath,
					unsigned char*&	bufferRGB,
					unsigned int&	bufferRGBBytes,
					int		bufferWidth,
					int		bufferHeight,
					long		frameNumber
				);
	void			SwapInNewBufferYUV
				(
					char*		videoPath,
					unsigned char*&	bufferYUV,
					unsigned int&	bufferYUVBytes,
					int		bufferWidth,
					int		bufferHeight,
					long		frameNumber
				);
	unsigned char*		LockBufferRGB(unsigned int bufferBytes);
	void			UnlockBufferRGB
				(
				const	char*		videoPath,
					int		bufferWidth,
					int		bufferHeight,
					long		frameNumber
				);
	const char*		GetVideoPath() const;
	unsigned char*		GetBufferRGB() const;
	unsigned int		GetBufferRGBBytes() const;
	unsigned char*		GetBufferYUV() const;
	unsigned int		GetBufferYUVBytes() const;
	int			GetBufferWidth() const;
	int			GetBufferHeight() const;
	long			GetFrameNumber() const;
	unsigned char*		GetBufferY() const;
	unsigned int		GetBufferYBytes() const;
	unsigned char*		GetBufferU() const;
	unsigned int		GetBufferUBytes() const;
	unsigned char*		GetBufferV() const;
	unsigned int		GetBufferVBytes() const;
	AVPacket*		LockPacket();
	void			UnlockPacket();
	void			SetPacket
				(
					AVPacket*	packet,
					const char*	path,
					long		frameNumber
				);
	void			Invalidate();

private:

	char			VideoPath[2048];
	bool			Ready;
	unsigned char*		Buffer;
	unsigned int		BufferBytes;
	bool			BufferIsRGB;
	int			BufferWidth;
	int			BufferHeight;
	long			FrameNumber;
	AVPacket*		Packet;
	LGL_Semaphore		PacketSemaphore;

public:
	LGL_Semaphore		BufferSemaphore;

};

class lgl_FrameBufferSortContainer
{

public:
	
	lgl_FrameBufferSortContainer(lgl_FrameBuffer* inFrameBuffer);

	lgl_FrameBuffer*	FrameBuffer;
	long			FrameNumber;
};

class lgl_PathNextInterchange
{

public:

				lgl_PathNextInterchange();
				~lgl_PathNextInterchange();

	bool			BelongsToDecoderThread;
	char			Path[4096];
	long			RequestNum;

};

const long lgl_PathNextInterchangeCount=64;

class LGL_VideoDecoder
{

public:

				LGL_VideoDecoder(const char* path);
				~LGL_VideoDecoder();

	void			Init();
	void			UnloadVideo();
	void			SetVideo(const char* path);
	void			SetVideo(std::vector<const char*> pathAttempts);
	const char*		GetPath();
	const char*		GetPathShort();
	int			GetPathNum();
	void			SetTime(double seconds);
	double			GetTime();
	double			GetLengthSeconds();
	float			GetBitrateMBps();
	double			GetFPS();
	int			GetFPSDisplayed();
	int			GetFPSMissed();
	LGL_Image*		GetImage(bool decodeAllowed=true);
	double			GetSecondsBufferedLeft(bool loaded, bool ready);
	double			GetSecondsBufferedRight(bool loaded, bool ready);
	void			SetFrameBufferAddBackwards(bool addBackwards=true);
	int			GetFrameBufferAddRadius() const;
	void			SetFrameBufferAddRadius(int frames);
	void			InvalidateAllFrameBuffers();
	void			SetNextRequestedDecodeFrame(long frameNum=-1);
	long			GetNextRequestedDecodeFrame();
	long			GetPosBytes();
	float			GetPreloadMaxMB();
	void			SetPreloadMaxMB(float maxMB);
	bool			GetPreloadEnabled();
	void			SetPreloadEnabled(bool enabled=true);
	void			ForcePreload();
	bool			GetPreloadFromCurrentTime();
	void			SetPreloadFromCurrentTime(bool fromCurrentTime=true);
	float			GetPreloadPercent();
	void			SetPreloadPercent(float pct);	//to be called from preload thread
	int			GetReadAheadMB();
	void			SetReadAheadMB(int MB);
	int			GetReadAheadDelayMS();
	void			SetReadAheadDelayMS(int ms);
	bool			GetDecodeInThread();
	void			SetDecodeInThread(bool decodeInThread=true);

	void			SetUserString(const char* str);
	const char*		GetUserString();

	//Thread Functions

	void			MaybeLoadVideo();
	bool			MaybeReadAhead();
	bool			MaybeLoadImage();
	bool			MaybeDecodeImage(long desiredFrameNum=-1);
	void			MaybeInvalidateBuffers();
	bool			GetThreadTerminate();

private:

	char			Path[2048];
	char			PathShort[2048];
	char			PathNext[2048];
	std::vector<const char*>
				PathNextAttempts;
	int			PathNum;

	char			UserString[2048];

	double			FPS;
	double			FPSTimestamp;
	int			FPSDisplayed;
	int			FPSMissed;
	int			FPSDisplayedHitCounter;
	int			FPSDisplayedMissCounter;
	unsigned long		FPSDisplayedHitMissFrameNumber;
	LGL_Timer		FPSDisplayedTimer;
	LGL_Timer		FPSDisplayedConstTimeTimer;
	double			LengthSeconds;
	double			LengthBytes;
	double			TimeSeconds;
	double			TimeSecondsPrev;
	bool			SetTimeHappened;
	long			FrameNumberNext;
	long			FrameNumberDisplayed;
	long			NextRequestedDecodeFrame;
	double			PosBytes;

public:
	std::vector<lgl_FrameBuffer*>
				GetFrameBufferReadyList(bool sorted);
	std::vector<lgl_FrameBuffer*>
				GetFrameBufferLoadedList(bool sorted);

private:
	std::vector<lgl_FrameBuffer*>
				FrameBufferList;

private:
	bool			FrameBufferAddBackwards;
	int			FrameBufferAddRadius;
	int			FrameBufferSubtractRadius;
	float			PreloadMaxMB;
	bool			PreloadEnabled;
	bool			PreloadFromCurrentTime;
	float			PreloadPercent;
	int			ReadAheadMB;
	int			ReadAheadDelayMS;
	bool			DecodeInThread;

	bool			ThreadTerminate;
	SDL_Thread*		ThreadPreload;
	SDL_Thread*		ThreadReadAhead;
	SDL_Thread*		ThreadLoad;
	SDL_Thread*		ThreadDecode;
	LGL_Semaphore		PathSemaphore;

	void			ResolvePathNextInterchange();
	lgl_PathNextInterchange	PathNextInterchangeList[lgl_PathNextInterchangeCount];
	LGL_Semaphore		PathNextSemaphore;

	AVFormatContext*	FormatContext;
	AVCodecContext*		CodecContext;
	AVCodec*		Codec;
	int			VideoStreamIndex;
	AVFrame*		FrameNative;
	AVFrame*		FrameRGB;
	unsigned char*		BufferRGB;
	unsigned int		BufferRGBBytes;
	int			BufferWidth;
	int			BufferHeight;
	SwsContext*		SwsConvertContextBGRA;
	void*			QuicktimeMovie;

	unsigned char*		BufferYUV;
	unsigned int		BufferYUVBytes;

	unsigned char*		BufferYUVAsRGB;
	unsigned int		BufferYUVAsRGBBytes;

	bool			IsImage;
	LGL_Image*		Image;
	bool			VideoOK;
	LGL_Semaphore		VideoOKSemaphore;
	int			VideoOKUserCount;

public:
	float			StoredBrightness[2];	//Hate this!

private:

	//Internal Functions

	double			TimestampToSeconds(long timestamp);
	long			SecondsToTimestamp(double seconds);
	double			FrameNumberToSeconds(long frameNumber);
	long			SecondsToFrameNumber(double seconds);
	long			FrameNumberToTimestamp(long FrameNumber);

	long			GetNextFrameNumberToLoad();
	long			GetNextFrameNumberToLoadPredictNext(std::vector<long>& frameNumList, bool mustNotBeLoaded=true);
	long			GetNextFrameNumberToLoadForwards(std::vector<long>& frameNumList);
	long			GetNextFrameNumberToLoadBackwards(std::vector<long>& frameNumList);

	long			GetNextFrameNumberToDecode();
	long			GetNextFrameNumberToDecodePredictNext(bool mustNotBeDecoded=true);
	long			GetNextFrameNumberToDecodeForwards();
	long			GetNextFrameNumberToDecodeBackwards();
	lgl_FrameBuffer*	GetInvalidFrameBuffer();

	bool			IsYUV420P();
};

class LGL_VideoEncoder
{

public:

				LGL_VideoEncoder
				(
					const char*	src,
					const char*	dstVideo,
					const char*	dstAudio
				);
				~LGL_VideoEncoder();

	bool			IsValid();
	bool			IsUnsupportedCodec();
	void			SetEncodeAudio(bool encode=true);
	void			SetEncodeVideo(bool encode=true);
	bool			GetEncodeAudio();
	bool			GetEncodeVideo();
	void			Encode(int frames);
	void			FlushAudioBuffer(bool force=false);
	float			GetPercentFinished();
	bool			IsFinished();
	const char*		GetCodecName();
	bool			IsMJPEG();
	float			GetBitrateMaxMBps();
static	void			SetBitrateMaxMBps(float max);
	LGL_Image*		GetImage();
	void			DeleteImage();
	const char*		GetSrcPath();

private:

	bool			Valid;
	bool			UnsupportedCodec;
	bool			EncodeAudio;
	bool			EncodeVideo;
static	float			BitrateMaxMBps;

	//Src

	char			SrcPath[2048];
	AVFormatContext*	SrcFormatContext;
	AVCodecContext*		SrcCodecContext;
	char			SrcCodecName[64];
	AVCodecContext*		SrcAudioCodecContext;
	AVCodec*		SrcCodec;
	AVCodec*		SrcAudioCodec;
	AVFrame*		SrcFrame;
	AVPacket		SrcPacket;
	double			SrcPacketPosMax;
	int			SrcAudioStreamIndex;
	int			SrcVideoStreamIndex;
	double			SrcFileBytes;

	unsigned int		SrcBufferBytes;
	unsigned int		SrcBufferWidth;
	unsigned int		SrcBufferHeight;

	long			SrcFrameNow;
	double			SrcSecondsNow;

	//Convert

	SwsContext*		SwsConvertContextYUV;
	SwsContext*		SwsConvertContextBGRA;

	//Dst

	char			DstPath[2048];
	AVOutputFormat*		DstOutputFormat;
	AVFormatContext*	DstFormatContext;
	AVCodecContext*		DstCodecContext;
	RcOverride		DstCodecContextRcOverride;
	AVCodec*		DstCodec;
	AVStream*		DstStream;
	AVFrame*		DstFrameYUV;
	uint8_t*		DstBufferYUV;
	LGL_Semaphore		DstFrameYUVSemaphore;
	AVFrame*		DstFrameBGRA;
	uint8_t*		DstBufferBGRA;
	AVPacket		DstPacket;
	int64_t			DstPacketVideoPts;

	char			DstMp3Path[2048];
	AVOutputFormat*		DstMp3OutputFormat;
	AVFormatContext*	DstMp3FormatContext;
	AVCodecContext*		DstMp3CodecContext;
	AVCodec*		DstMp3Codec;
	AVStream*		DstMp3Stream;
	int16_t*		DstMp3Buffer;
	int16_t*		DstMp3BufferSamples;
	int			DstMp3BufferSamplesIndex;
	long			DstMp3BufferSamplesTotalBytes;
	int64_t			DstMp3BufferSrcPts;
	int16_t*		DstMp3Buffer2;
	AVPacket		DstMp3Packet;

	LGL_Image*		Image;
};

class LGL_AudioEncoder
{

public:

				LGL_AudioEncoder
				(
					const char*	dstPath,
					bool		surroundMode=false
				);
				~LGL_AudioEncoder();

	bool			IsValid();
	void			Encode
				(
					const char*	data,
					long		bytes
				);
	void			Finalize();
	int			GetChannelCount();

	void			ThreadFunc();

private:

	void			FlushBuffer(bool force=false);

	bool			SurroundMode;
	char			DstMp3Path[2048];
	AVOutputFormat*		DstMp3OutputFormat;
	AVFormatContext*	DstMp3FormatContext;
	AVCodecContext*		DstMp3CodecContext;
	AVCodec*		DstMp3Codec;
	AVStream*		DstMp3Stream;
	int16_t*		DstMp3Buffer;
	int16_t*		DstMp3BufferSamples;
	int			DstMp3BufferSamplesIndex;
	int16_t*		DstMp3Buffer2;
	AVPacket		DstMp3Packet;
	long			CircularBufferBytes;
	char*			CircularBuffer;
	long			CircularBufferHead;
	long			CircularBufferTail;

	bool			Valid;
	bool			AVOpened;

	SDL_Thread*		Thread;
	bool			DestructHint;
};

bool
LGL_VideoIsMJPEG
(
	const char*	path
);

void
LGL_SetUseLibJpegTurbo
(
	bool	use=true
);

class LGL_Font
{

public:
			LGL_Font(const char* dirpath);
			~LGL_Font();

	float		DrawString
			(
				float x, float y, float height,
				float r, float g, float b, float a,
				bool centered,
				float shadowAlpha,
				const char* string,
				...
			);

	bool		QueryChar(char in);

const	char*		GetPath() const;
const	char*		GetPathShort() const;
	
	int		GetReferenceCount();
	void		IncrementReferenceCount();
	void		DecrementReferenceCount();

	void		PrintMissingGlyphs(const char* string, ...);

	int		GetHeightPixels();

//private:

	float		GetWidthChar
			(
				float height,
				char in
			);

	float		GetWidthString
			(
				float height,
				const char* in
			);
	
	LGL_Image*	Glyph[256];
	char		Path[1024];
	char		PathShort[1024];
	bool		DrawingString;

	int		ReferenceCount;

	int		GlyphSideLength;
	int		TextureSideLength;
	GLuint		TextureGL;
	int		GlyphTexLeft[256];
	int		GlyphTexRight[256];
	int		GlyphTexBottom[256];
	int		GlyphTexTop[256];
	int		GlyphTexWidth[256];
	int		GlyphTexHeight[256];
	float		GlyphTexWidthHeightRatio[256];
};

LGL_Font&
LGL_GetFont();

void
LGL_DebugPrintf
(
	const char*	string,
	...
);

void
lgl_DebugPrintfInternal
(
	const char*	string
);

void
LGL_SetDebugMode
(
	bool	debugMode=true
);

bool
LGL_GetDebugMode();

FILE*
LGL_fopen
(
	const char*	path,
	const char*	mode
);

int
LGL_fclose
(
	FILE*	fd
);

class LGL_InputBuffer
{

public:

			LGL_InputBuffer();
			~LGL_InputBuffer();
	
	void		ClearBuffer();
	
	void		GrabFocus();
	void		ReleaseFocus();
	bool		HasFocus() const;

	void		AcceptString();
	void		AcceptInt();
	void		AcceptFloat();
	void		AcceptTime();

const	char*		GetString();
	int		GetInt();
	float		GetFloat();
	int		GetHoursComponent();
	int		GetMinutesComponent();
	float		GetSecondsComponent();
	float		GetSecondsTotal();

	void		SetString(const char* in=NULL);
	void		SetInt(int in);
	void		SetFloat(float in);
	void		SetTime(int hours, int minutes, float seconds);

private:

	char		Buffer[1024];
	bool		Focus;
	int		Accept;

public:

	void		LGL_INTERNAL_ProcessInput();	//USERS MAY NOT CALL THIS FN()!!!

//FIXME: LGL_InputBuffer has a public fn() which shouldn't be public
};

//Audio2

class LGL_AudioGrain;

unsigned int
LGL_AudioChannels();

bool
LGL_AudioOutAvailable();

bool
LGL_AudioInAvailable();

void
LGL_SetAudioInPassThru
(
	bool passThru=true
);

bool
LGL_AudioOutDisconnected();

std::vector<LGL_AudioGrain*>&
LGL_AudioInGrainList();

bool
LGL_AudioInMetadata
(
	float&	volAve,
	float&	volMax,
	float&	freqFactor,
	float	gain=1.0f,
	float	freqEQBalance=0.5f
);

void
LGL_DrawWaveform
(
	float	left=0.0f,	float	right=1.0f,
	float	bottom=0.0f,	float	top=1.0f,
	float	r=1.0f,		float	g=1.0f,	float	b=1.0f,	float	a=1.0f,
	float	thickness=1.0f,
	bool	antialias=true
);

void
LGL_DrawAudioInWaveform
(
	float	left=0.0f,	float	right=1.0f,
	float	bottom=0.0f,	float	top=1.0f,
	float	r=1.0f,		float	g=1.0f,	float	b=1.0f,	float	a=1.0f,
	float	thickness=1.0f,
	bool	antialias=true
);

void
LGL_DrawAudioInSpectrum
(
	float	left=0.0f,	float	right=1.0f,
	float	bottom=0.0f,	float	top=1.0f
);

enum
LGL_EnvelopeType
{
	LGL_ENVELOPE_FULL,
	LGL_ENVELOPE_TRIANGLE,
	LGL_ENVELOPE_SINE
};

void
LGL_FFT
(
	float*	real,
	float*	imaginary,
	int	lgSize,
	int	direction,
	int	threadID=0
);

class LGL_AudioStream
{

public:

			LGL_AudioStream();
virtual			~LGL_AudioStream();
	
virtual	void		Update();	//Called by LGL. Don't call this from your application.
virtual	bool		Finished() const;
//virtual	void		PrepareAudioStream(int len);
virtual	int		MixIntoStream
			(
				void*	userdata,
				Uint8*	stream,
				int	len
			);
/*
virtual	void		GetAudioFreq
			(
				float*	samplesLeft,
				float*	samplesRight
				int	samples
			);
*/

private:

	float		DefaultSineCounter;

};

void
LGL_AddAudioStream
(
	LGL_AudioStream*	stream
);

class LGL_AudioDSP
{

public:

			LGL_AudioDSP();
			~LGL_AudioDSP();
	
	void		ProcessLeft	(const float* input, float* output,
					float&	eq_vu_l,
					float&	eq_vu_m,
					float&	eq_vu_h,
					unsigned long samples);
	void		ProcessRight	(const float* input, float* output,
					float&	eq_vu_l,
					float&	eq_vu_m,
					float&	eq_vu_h,
					unsigned long samples);
	void		ProcessStereo	(const float* input, float* output,
					float&	eq_vu_l,
					float&	eq_vu_m,
					float&	eq_vu_h,
					unsigned long samples);

	void		SetFreqResponse	(float*		freqResponseArrayOf513);

private:

	void		ProcessChannel
			(
				const
				float*		userInput,
				float*		userOutput,
				float*		carryOver,
				float&		eq_vu_l,
				float&		eq_vu_m,
				float&		eq_vu_h,
				unsigned long	samples
			);

/*
	void		ProcessChannelStereo
			(
				const
				float*		userInputL,
				float*		userInputR,
				float*		userOutputL,
				float*		userOutputR,
				float*		carryOverL,
				float*		carryOverR,
				unsigned long	samples
			);
*/

	float		CarryOverLeft[LGL_EQ_SAMPLES_FFT*4];
	float		CarryOverRight[LGL_EQ_SAMPLES_FFT*4];

	float		FreqResponseReal[LGL_EQ_SAMPLES_FFT*2];
	float		FreqResponseImaginary[LGL_EQ_SAMPLES_FFT*2];

	float		FreqResponseNextReal[LGL_EQ_SAMPLES_FFT*2];
	float		FreqResponseNextImaginary[LGL_EQ_SAMPLES_FFT*2];
	bool		FreqResponseNextAvailable;
	LGL_Semaphore	FreqResponseNextSemaphore;
};

class LGL_Sound;

class LGL_AudioGrain : public LGL_AudioStream
{

public:

			LGL_AudioGrain();
			~LGL_AudioGrain();

	void		DrawWaveform
			(
				float	left,	float	right,
				float	bottom,	float	top,
				float	r,	float	g,	float	b,	float	a,
				float	thickness=1.0f,
				bool	antialias=true
			);
	void		DrawSpectrum
			(
				float	left,	float	right,
				float	bottom,	float	top
			);

const	float*		GetWaveform();
const	float*		GetSpectrum();
	void		GetMetadata
			(
				float&	volAve,
				float&	volMax,
				float&	freqFactor,
				float	gain=1.0f,
				float	freqEQBalance=0.5f
			);
	
virtual	void		Update();
	bool		IsPlaying();
virtual	bool		Finished() const;
virtual	int		MixIntoStream
			(
				void*	userdata,
				Uint8*	stream,
				int	len
			);
	
	void		GetWaveformToMemory
			(
				Uint8*		channelStereo
			);
	void		GetWaveformToMemory
			(
				Sint16*		channelL,
				Sint16*		channelR
			);

	void		SetWaveformFromMemory
			(
				Uint8*		buffer,
				long		lengthSamples
			);
	void		SetWaveformFromAudioGrain
			(
				LGL_AudioGrain*	grain
			);
	void		SetWaveformFromLGLSound
			(
				LGL_Sound*	sound,
				float		centerSeconds,
				float		lengthSeconds,
				float		centerSecondsVariance=0.0f,
				float		lengthSecondsVariance=0.0f,
				bool		lockSound=true
			);
	void		SetWaveformFromLGLSoundSamples
			(
				LGL_Sound*	sound,
				long		startSamples,
				long		lengthSamples
			);

	void		SetStartDelaySeconds(float startDelaySeconds, float variance=0.0f);
	void		SetVolume(float volume, float variance=0.0f);
	void		SetVolumeStereo
			(
				float volumeLeft,		float volumeRight,
				float varianceLeft=0.0f,	float varianceRight=0.0f
			);
	void		SetVolumeSurround
			(
				float volumeFrontLeft,		float volumeFrontRight,
				float volumeBackLeft,		float volumeBackRight,
				float varianceFrontLeft=0.0f,	float varianceFrontRight=0.0f,
				float varianceBackLeft=0.0f,	float varianceBackRight=0.0f
			);
	void		SetPitch(float pitch=1.0f, float variance=0.0f);
	void		SetPitchAlphaOmega
			(
				float pitchAlpha=1.0f,
				float pitchOmega=1.0f,
				float varianceAlpha=0.0f,
				float varianceOmega=0.0f
			);
	
	int		GetStartDelaySamplesRemaining() const;
	void		SetStartDelaySamplesRemaining(int samples);

	void		SetEnvelope(LGL_EnvelopeType envelopeType);

	int		GetLengthSamples() const;
	int		GetSpectrumSamples() const;
	bool		IsSilent();

	float		GetDistanceEuclidean(LGL_AudioGrain* queryGrain, int mapMapLevel=-1, float* distanceMost=NULL);
	bool		GetDistanceLessThanEuclidean(LGL_AudioGrain* queryGrain, float distance);
	bool		GetDistanceGreaterThanEuclidean(LGL_AudioGrain* queryGrain, float distance);

	void		CalculateWaveformDerivatives
			(
				float	gain=1.0f,
				float	freqEQBalance=0.5f
			);
	void		CalculateSpectrum();
	void		CalculateSpectrumMipMaps();
	void		NullifyWaveformAndSpectrum();
	void		NullifyAllButWaveform();

	void		TEST_PitchShift(int steps);
	void		TEST_ApplyThreeChannelEQ(float low, float mid, float high);

private:
	
	Uint8*		Waveform;
	float*		WaveformLeftFloat;
	float*		WaveformRightFloat;
	float*		WaveformMonoFloat;
	float*		SpectrumLeft;
	float*		SpectrumRight;
	float*		SpectrumMono;
	float**		SpectrumMipMaps;
	float**		SpectrumMipMapsError;
	float		VolAve;
	float		VolMax;
	float		FreqFactor;
	float		Gain;
	float		FreqEQBalance;

	long		LengthSamples;
	long		SpectrumSamples;
	long		StartDelaySamples;
	long		CurrentPositionSamplesInt;
	double		CurrentPositionSamplesFloat;
	float		PlaybackPercent;
	float		VolumeFrontLeft;
	float		VolumeFrontRight;
	float		VolumeBackLeft;
	float		VolumeBackRight;
	float		PitchAlpha;
	float		PitchOmega;
	LGL_EnvelopeType
			EnvelopeType;
	int		Silent;
};

class LGL_AudioGrainStream : public LGL_AudioStream
{

public:

			LGL_AudioGrainStream();
			~LGL_AudioGrainStream();

virtual	void		Update();
virtual	bool		Finished() const;
virtual	int		MixIntoStream	//Returns number of samples mixed
			(
				void*	userdata,
				Uint8*	stream,
				int	len
			);

	void		AddNextGrain(LGL_AudioGrain* grain, long offsetDelaySamples=-1);
	void		AddNextGrains(std::vector<LGL_AudioGrain*>& grains);

	long		GetGrainsQueuedCount();

private:

	std::vector<LGL_AudioGrain*>
			AudioGrainsQueued;

	std::vector<LGL_AudioGrain*>
			AudioGrainsActive;

	LGL_Semaphore	GrainListsSemaphore;
};

class LGL_AudioGrainDatabase
{

public:

					LGL_AudioGrainDatabase();
virtual					~LGL_AudioGrainDatabase();
	
virtual	void				Clear();
virtual	bool				Empty();
virtual	long				Size();

virtual void				AddGrain(LGL_AudioGrain* grain);
virtual void				AddGrains(std::vector<LGL_AudioGrain*> grains);

virtual LGL_AudioGrain*			GetNearestNeighbor(LGL_AudioGrain* queryGrain);
virtual std::vector<LGL_AudioGrain*>	GetNearestNeighbors
					(
						LGL_AudioGrain* queryGrain,
						int count,
						float radius
					);

virtual LGL_AudioGrain*			GetApproximateNearestNeighbor(LGL_AudioGrain* queryGrain);
virtual std::vector<LGL_AudioGrain*>	GetApproximateNearestNeighbors
					(
						LGL_AudioGrain* queryGrain,
						int count,
						float radius
					);
};

//Audio1

#define	LGL_SOUND_METADATA_ENTRIES_PER_SECOND	(64)
#define	LGL_SOUND_METADATA_SIZE_MAX		(20*60*LGL_SOUND_METADATA_ENTRIES_PER_SECOND)

class LGL_Sound
{

public:
			LGL_Sound
			(
				const char*		filename,
				bool			LoadInNewThread=false,
				int			channels=2,
				Uint8*			buffer=NULL,
				unsigned long		bufferLength=0
			);
			LGL_Sound
			(
				Uint8*			buffer,
				Uint32			len,
				int			channels=2
			);
			~LGL_Sound();

	void		PrepareForDelete();
	void		PrepareForDeleteThreadFunc();
	bool		PreparingForDelete();
	bool		ReadyForDelete();

	int		Play
			(
				float			volume=1,
				bool			looping=false,
				float			speed=1,
				float			startSeconds=0,
				float			lengthSeconds=-1
			);
	void		Stop(int channel);
	void		SetVolume
			(
				int	channel,
				float	volume
			);
	void		SetVolumeStereo
			(
				int	channel,
				float	left,
				float	right
			);
	void		SetVolumeSurround
			(
				int	channel,
				float	frontLeft,
				float	frontRight,
				float	backLeft,
				float	backRight
			);
	void		TogglePause(int channel);
	bool		IsPlaying(int channel);
	
	void		SetPositionSamples
			(
				int	channel,
				unsigned long samples
			);
	void		SetPositionSeconds
			(
				int	channel,
				float	seconds
			);
	void		SetLooping
			(
				int	channel,
				bool	looping=true
			);
	void		SetStickyEndpoints
			(
				int	channel,
				bool	stickyEndpoints=true
			);
	void		SetSpeed
			(
				int	channel,
				float	speed,
				bool	instant=false
			);
	void		SetSpeedInterpolationFactor
			(
				int	channel,
				float	speedif
			);
	void		SetGlitchAttributes
			(
				int	channel,
				bool	enable,
				long	samplesBegin,
				int	samplesLength,
				float	volume,
				float	speed,
				float	duo,
				float	interpolation,
				bool	luminscratch,
				long	luminscratchPositionDesired
			);
	void		SetGlitchSamplesNow(int channel, long glitchNowSamples);
	long		GetGlitchLuminScratchPositionDesired(int channel);
	void		SetDownMixToMono
			(
				int	channel,
				bool	DownMix
			);
	void		SetFreqResponse
			(
				int	channel,
				float*	freqResponseArrayOf513=NULL
			);

	float		GetSpeed(int channel);
	float		GetSample(int sample);
	float		GetSampleLeft(int sample);
	float		GetSampleRight(int sample);
	void		LockBuffer();
	void		UnlockBuffer();
	void		LockBufferForReading(int id=0);
	void		UnlockBufferForReading(int id=0);
	Uint8*		GetBuffer(bool warnIfNotLocked=true);
	unsigned long	GetBufferLength();
	double		GetPositionSeconds(int channel);
	float		GetPositionPercent(int channel);
	unsigned long	GetPositionSamples(int channel);
	unsigned long	GetPositionGlitchBeginSamples(int channel);
	unsigned long	GetSamplesPlayedBack(int channel);
	//bool		SetDivergeRecallOff(int channel);
	//bool		SetDivergeRecallBegin(int channel, float speed);
	//bool		SetDivergeRecallEnd(int channel);
	bool		DivergeRecallPush(int channel, float speed=-1.0f);
	bool		DivergeRecallPop(int channel, bool recall=true);
	long		GetDivergeRecallSampleCount(int channel);
	int		GetDivergeRecallCount(int channel);
	bool		GetWarpPointIsSet(int channel);
	bool		GetWarpPointIsLoop(int channel);
	bool		GetWarpPointIsLocked(int channel);
	bool		SetWarpPoint(int channel);
	bool		SetWarpPoint(int channel, double alphaSeconds, double omegaSeconds, bool loop=false, bool lock=false);
	float		GetWarpPointSecondsAlpha(int channel);
	float		GetWarpPointSecondsOmega(int channel);
	void		SetRhythmicInvertProperties(int channel, float secondsAlpha, float secondsDelta);
	void		SetRhythmicVolumeInvert(int channel, bool rapidVolumeInvert);
	void		SetRespondToRhythmicSoloInvertChannel(int channel, int soloChannel);
	int		GetRespondToRhythmicSoloInvertCurrentValue(int channel);

	float		GetVU(int channel) const;
	float		GetEQVUL(int channel) const;
	float		GetEQVUM(int channel) const;
	float		GetEQVUH(int channel) const;

	float		GetLengthSeconds();
	long		GetLengthSamples();
	bool		GetHogCPU() const;
	void		SetHogCPU(bool hogCPU=true);
	bool		AnalyzeWaveSegment
			(
				long		sampleFirst,
				long		sampleLast,
				float&		zeroCrossingFactor,
				float&		magnitudeAve,
				float&		magnitudeMax
			);
	bool		GetMetadata
			(
				float	secondsBegin,
				float	secondsEnd,
				float&	zeroCrossingFactor,
				float&	magnitudeAve,
				float&	magnitudeMax
			);
	bool		GetSilent() const;
	float		GetVolumePeak();
	void		SetVolumePeak(float volumePeak);
	void		LoadToMemory();
	bool		IsLoaded();
	bool		IsUnloadable();
	float		GetPercentLoaded();
	float		GetPercentLoadedSmooth();
	float		GetSecondsUntilLoaded();
	void		MaybeSwapBuffers();	//Called from lgl_AudioCallback(). Users: Don't call this!

	int		GetChannelCount();
	int		GetHz();

	int		GetReferenceCount();
	void		IncrementReferenceCount();
	void		DecrementReferenceCount();

const	char*		GetPath() const;
const	char*		GetPathShort() const;
const	char*		GetPathUser() const;
	void		SetPathUser(const char* pathUser);

private:

	char		Path[2048];
	char		PathShort[2048];
	char		PathUser[2048];
	Uint8*		Buffer;
	unsigned long	BufferLength;
	unsigned long	BufferLengthTotal;
	Uint8*		BufferBack;
	unsigned long	BufferBackLength;
	unsigned long	BufferBackLengthTotal;
	bool		BufferBackReadyForWriting;
	bool		BufferBackReadyForReading;
	bool		BufferAllocatedFromElsewhere;
	float		PercentLoaded;
	LGL_Semaphore	BufferSemaphore;
	LGL_Semaphore	BufferReaderCountSemaphore;
	int		BufferReaderCount;
	SDL_Thread*	DecoderThread;
	bool		Loaded;
	float		LoadedMin;
	float		LoadedSmooth;
	float		LoadedSmoothTime;
	int		Channels;
	int		Hz;

	LGL_Timer	LoadTimer;
	LGL_Timer	LoadTimerDeltaTimer;
	float		LoadSecondsUntilLoaded;

	int		ReferenceCount;

	bool		BadFile;
	bool		Silent;

	bool		HogCPU;
	bool		DestructorHint;
	bool		DeleteOK;
	SDL_Thread*	PrepareForDeleteThread;

	float		MetadataVolumePeak;
	float		MetadataVolumeAve[LGL_SOUND_METADATA_SIZE_MAX];
	float		MetadataVolumeMax[LGL_SOUND_METADATA_SIZE_MAX];
	float		MetadataFreqFactor[LGL_SOUND_METADATA_SIZE_MAX];
	int		MetadataFilledSize;

public:

	LGL_Semaphore	DeleteSemaphore;
};

bool		LGL_AudioAvailable();
bool		LGL_AudioWasOnceAvailable();
bool		LGL_AttemptAudioRevive();
bool		LGL_AudioUsingJack();
bool		LGL_AudioJackXrun();
bool		LGL_AudioIsRealtime();
int		LGL_AudioRate();
int		LGL_AudioCallbackSamples();
float		LGL_AudioSampleLeft(int sample);
float*		LGL_AudioSampleLeftArray();
float		LGL_AudioSampleRight(int sample);
float*		LGL_AudioSampleRightArray();
float		LGL_AudioSampleMono(int sample);
float*		LGL_AudioSampleMonoArray();
int		LGL_AudioSampleArraySize();
float		LGL_AudioPeakLeft();
float		LGL_AudioPeakRight();
float		LGL_AudioPeakMono();
float		LGL_FreqL(float freq);
float		LGL_FreqR(float freq);
float		LGL_FreqMono(float freq);
float		LGL_FreqBufferL(int index, int width=0);	//index [0,512)
float		LGL_FreqBufferR(int index, int width=0);
float		LGL_FreqBufferMono(int index, int width=0);
int		LGL_GetRecordDVJToFile();
void		LGL_RecordDVJToFileStart(const char* path, bool surroundMode=false);
void		LGL_RecordDVJToFileStartPaused(const char* path, bool surroundMode=false);
void		LGL_RecordDVJToTileUnpause();
const char*	LGL_RecordDVJToFilePath();
const char*	LGL_RecordDVJToFilePathShort();
void		LGL_SetRecordDVJToFileVolume(float volume);
void		LGL_AudioMasterToHeadphones(bool copy=true);

//Input

void		LGL_ProcessInput();

//Keyboard

bool		LGL_KeyDown(int key);
bool		LGL_KeyStroke(int key);
bool		LGL_KeyRelease(int key);
float		LGL_KeyTimer(int key);
const
char*		LGL_KeyStream();

//Mouse

#define		LGL_MOUSE_LEFT		0
#define		LGL_MOUSE_MIDDLE	1
#define		LGL_MOUSE_RIGHT		2

float		LGL_MouseX();
float		LGL_MouseY();
float		LGL_MouseDX();
float		LGL_MouseDY();
bool		LGL_MouseMotion();
bool		LGL_MouseDown(int button);
bool		LGL_MouseStroke(int button);
bool		LGL_MouseRelease(int button);
float		LGL_MouseTimer(int button);

void		LGL_MouseVisible(bool visible);
void		LGL_MouseWarp(float x, float y);

//MultiTouch

float		LGL_MultiTouchX();
float		LGL_MultiTouchY();
float		LGL_MultiTouchDX();
float		LGL_MultiTouchDY();
float		LGL_MultiTouchDX2();
float		LGL_MultiTouchDY2();
float		LGL_MultiTouchDXTotal();
float		LGL_MultiTouchDYTotal();
float		LGL_MultiTouchRotate();
float		LGL_MultiTouchPinch();
bool		LGL_MultiTouchMotion();
int		LGL_MultiTouchFingerCount();
int		LGL_MultiTouchFingerCountDelta();



//Joystick (Playstation 2)

#define		LGL_JOY_XAXIS		0
#define		LGL_JOY_YAXIS		1
#define		LGL_JOY_DPAD		2

#define		LGL_JOY_ANALOGUE_L	0
#define		LGL_JOY_ANALOGUE_R	1
#define		LGL_JOY_LEFT		2
#define		LGL_JOY_RIGHT		3
#define		LGL_JOY_DOWN		4
#define		LGL_JOY_UP		5
#define		LGL_JOY_L1		6
#define		LGL_JOY_R1		7
#define		LGL_JOY_L2		8
#define		LGL_JOY_R2		9
#define		LGL_JOY_START		10
#define		LGL_JOY_SELECT		11
#define		LGL_JOY_SQUARE		12
#define		LGL_JOY_TRIANGLE	13
#define		LGL_JOY_CROSS		14
#define		LGL_JOY_CIRCLE		15

#define		LGL_JOY_BEATMANIA_BLACK_1	6
#define		LGL_JOY_BEATMANIA_BLACK_2	7
#define		LGL_JOY_BEATMANIA_BLACK_3	8
#define		LGL_JOY_BEATMANIA_WHITE_1	12
#define		LGL_JOY_BEATMANIA_WHITE_2	14
#define		LGL_JOY_BEATMANIA_WHITE_3	15
#define		LGL_JOY_BEATMANIA_WHITE_4	16
#define		LGL_JOY_BEATMANIA_RECORD_CW	17
#define		LGL_JOY_BEATMANIA_RECORD_CCW	18

#define		LGL_JOY_GUITAR_GREEN		9
#define		LGL_JOY_GUITAR_RED		15
#define		LGL_JOY_GUITAR_YELLOW		13
#define		LGL_JOY_GUITAR_BLUE		14
#define		LGL_JOY_GUITAR_ORANGE		12
#define		LGL_JOY_GUITAR_START		11
#define		LGL_JOY_GUITAR_SELECT		10

int		LGL_JoyNumber();
const char*	LGL_JoyName(int which);

bool		LGL_JoyDown(int Joystick, int Button);
bool		LGL_JoyStroke(int Joystick, int Button);
bool		LGL_JoyRelease(int Joystick, int Button);
float		LGL_JoyAnalogueStatus
		(
			int Joystick,
			int Side,
			int Axis
		);
bool		LGL_JoyReset(int Joystick);

#define		LGL_WIIMOTE_LEFT		0
#define		LGL_WIIMOTE_RIGHT		1
#define		LGL_WIIMOTE_DOWN		2
#define		LGL_WIIMOTE_UP			3
#define		LGL_WIIMOTE_A			4
#define		LGL_WIIMOTE_B			5
#define		LGL_WIIMOTE_MINUS		6
#define		LGL_WIIMOTE_PLUS		7
#define		LGL_WIIMOTE_HOME		8
#define		LGL_WIIMOTE_1			9
#define		LGL_WIIMOTE_2			10

#define		LGL_WIIMOTE_EXTENSION_NONE	0
#define		LGL_WIIMOTE_EXTENSION_NUNCHUK	1
#define		LGL_WIIMOTE_EXTENSION_CLASSIC	2

class		LGL_Wiimote
{

public:

			LGL_Wiimote();
			~LGL_Wiimote();

	void		ListenForConnection(bool listen=true);
	bool		IsListeningForConnection();
	bool		Connected();
	void		Disconnect();

	void		SetRumble(bool status=true, float seconds=-1.0f);
	void		SetLED(int which, bool status=true);
	bool		GetRumble();
	float		GetBattery();

	bool		ButtonDown(int which);
	bool		ButtonStroke(int which);
	bool		ButtonRelease(int which);

	bool		GetPointerAvailable();
	float		GetPointerX();
	float		GetPointerY();
	std::vector<LGL_Vector>
			GetPointerMotionThisFrame();
	void		DrawPointerIRSources(float left=0, float right=1, float bottom=0, float top=1);

	LGL_Vector	GetAccelRaw();
	LGL_Vector	GetAccelRawNormalized();
	void		DrawAccelGraph(float left=0, float right=1, float bottom=0, float top=1);

	bool		GetFlickXPositive();
	bool		GetFlickXNegative();

	int		GetExtension();

	bool		INTERNAL_Connect();					//DON'T CALL THIS FROM USER-LAND! Use ListenForConnection() instead!
#ifdef	LGL_LINUX_WIIMOTE
	void		INTERNAL_Callback(union cwiid_mesg* mesg);		//DON'T CALL THIS!!!
#endif	//LGL_LINUX_WIIMOTE
	void		INTERNAL_ProcessInput();				//DON'T CALL THIS EITHER!!!
	void		INTERNAL_UpdateButton(int which, bool pressed);		//DON'T CALL THIS ESPECIALLY!!!
#ifdef	LGL_LINUX_WIIMOTE
	cwiid_wiimote_t*
			INTERNAL_GetWiimote();					//DON'T CALL THIS, JERKFACE!!
#endif	//LGL_LINUX_WIIMOTE
	LGL_Semaphore*	INTERNAL_GetWiimoteSemaphore();

private:

	void		Reset();

	int		ID;
static	int		IDCounter;

#ifdef	LGL_LINUX_WIIMOTE
	cwiid_wiimote_t*
#else
	void*
#endif	//LGL_LINUX_WIIMOTE
			Wiimote;
	LGL_Semaphore	WiimoteSemaphore;
	SDL_Thread*	ConnecterThread;

	bool		Rumble;
	float		RumbleSeconds;
	bool		LEDState[4];
	float		Battery;

	LGL_Semaphore	ButtonArraySemaphore;
	bool		ButtonDownArrayFront[11];
	bool		ButtonStrokeArrayFront[11];
	bool		ButtonReleaseArrayFront[11];
	bool		ButtonDownArrayBack[11];
	bool		ButtonStrokeArrayBack[11];
	bool		ButtonReleaseArrayBack[11];

	LGL_Vector	PointerFront;
	LGL_Vector	PointerBack;
	LGL_Vector	PointerSingleSourceOffset;
	LGL_Vector	PointerIRSources[4];
	std::vector<LGL_Vector>
			PointerMotionThisFrameFront;
	std::vector<LGL_Vector>
			PointerMotionThisFrameBack;
	LGL_Semaphore	PointerMotionSemaphore;
	float		PointerGreatestIRSourceDistance;

	//unsigned char	AccelCalibration[2][3];
	//unsigned char	AccelCalibrationNunchuk[2][3];
	LGL_Vector	AccelFront;
	LGL_Vector	AccelBack;
	std::vector<LGL_Vector>
			AccelPast;

	bool		FlickXPositiveNow;
	bool		FlickXNegativeNow;
	float		FlickXPositiveDebounce;
	float		FlickXNegativeDebounce;

	int		Extension;
};

LGL_Wiimote& LGL_GetWiimote(int which);



//MIDI

unsigned int	LGL_MidiDeviceCount();
const char*	LGL_MidiDeviceName(unsigned int which);

float		LGL_MidiClockBPM();
float		LGL_MidiClockPercentOfCurrentMeasure(float measureMultiplier=1.0f);



//LGL_MidiDevice

#define	LGL_MIDI_CONTROL_MAX			255

class LGL_MidiDevice
{

public:

			LGL_MidiDevice();
virtual			~LGL_MidiDevice();
	
	bool		GetButtonStroke(int button);
	bool		GetButtonDown(int button);
	bool		GetButtonRelease(int button);
	float		GetButtonTimer(int button);
	float		GetButtonForce(int button);

	bool		GetKnobTweak(int knob);
	float		GetKnobStatus(int knob);

//private:

	int		DeviceID;

	bool		ButtonStrokeFront[LGL_MIDI_CONTROL_MAX];
	bool		ButtonStrokeBack[LGL_MIDI_CONTROL_MAX];
	bool		ButtonDownFront[LGL_MIDI_CONTROL_MAX];
	bool		ButtonDownBack[LGL_MIDI_CONTROL_MAX];
	bool		ButtonReleaseFront[LGL_MIDI_CONTROL_MAX];
	bool		ButtonReleaseBack[LGL_MIDI_CONTROL_MAX];
	LGL_Timer	ButtonTimer[LGL_MIDI_CONTROL_MAX];
	float		ButtonForceFront[LGL_MIDI_CONTROL_MAX];
	float		ButtonForceBack[LGL_MIDI_CONTROL_MAX];

	bool		KnobTweakFront[LGL_MIDI_CONTROL_MAX];
	bool		KnobTweakBack[LGL_MIDI_CONTROL_MAX];
	float		KnobStatusFront[LGL_MIDI_CONTROL_MAX];
	float		KnobStatusBack[LGL_MIDI_CONTROL_MAX];

	LGL_Semaphore	BackBufferSemaphore;
virtual	void		LGL_INTERNAL_SwapBuffers();
};

//Xponent

#define	lgl_turntable_tweak_history_size (8)

class LGL_MidiDeviceXponent : public LGL_MidiDevice
{

public:

			LGL_MidiDeviceXponent();
			~LGL_MidiDeviceXponent();

//private:

	int		TurntableTweakIndex;
	bool		TurntableTweakHistoryL[lgl_turntable_tweak_history_size];
	bool		TurntableTweakHistoryR[lgl_turntable_tweak_history_size];

virtual	void		LGL_INTERNAL_SwapBuffers();

};

LGL_MidiDeviceXponent*
LGL_GetXponent();

#define	LGL_XPONENT_BUTTON_LEFT_RECORD		22
#define	LGL_XPONENT_BUTTON_LEFT_FINGER		21
#define	LGL_XPONENT_BUTTON_LEFT_HEADPHONES	20
#define	LGL_XPONENT_BUTTON_LEFT_X		18
#define	LGL_XPONENT_BUTTON_LEFT_DASH		19
#define	LGL_XPONENT_BUTTON_LEFT_LEFT		16
#define	LGL_XPONENT_BUTTON_LEFT_RIGHT		17
#define	LGL_XPONENT_BUTTON_LEFT_MOD_POWER_1	12
#define	LGL_XPONENT_BUTTON_LEFT_MOD_POWER_2	13
#define	LGL_XPONENT_BUTTON_LEFT_MOD_POWER_3	14
#define	LGL_XPONENT_BUTTON_LEFT_MOD_POWER_4	15
#define	LGL_XPONENT_BUTTON_LEFT_REWIND		33
#define	LGL_XPONENT_BUTTON_LEFT_FAST_FORWARD	34
#define	LGL_XPONENT_BUTTON_LEFT_POINT_1		23
#define	LGL_XPONENT_BUTTON_LEFT_POINT_2		24
#define	LGL_XPONENT_BUTTON_LEFT_POINT_3		25
#define	LGL_XPONENT_BUTTON_LEFT_POINT_4		26
#define	LGL_XPONENT_BUTTON_LEFT_POINT_5		27
#define	LGL_XPONENT_BUTTON_LEFT_PREV		28
#define	LGL_XPONENT_BUTTON_LEFT_NEXT		29
#define	LGL_XPONENT_BUTTON_LEFT_LOCK		30
#define	LGL_XPONENT_BUTTON_LEFT_PLUS		31
#define	LGL_XPONENT_BUTTON_LEFT_MINUS		32
#define	LGL_XPONENT_BUTTON_LEFT_EJECT		35
#define	LGL_XPONENT_BUTTON_LEFT_TOGGLE_PAUSE	36
#define	LGL_XPONENT_BUTTON_LEFT_LOOP_1		37
#define	LGL_XPONENT_BUTTON_LEFT_LOOP_2		38
#define	LGL_XPONENT_BUTTON_LEFT_LOOP_4		39
#define	LGL_XPONENT_BUTTON_LEFT_LOOP_8		40
#define	LGL_XPONENT_BUTTON_LEFT_LOOP_CYCLE	42
#define	LGL_XPONENT_BUTTON_LEFT_LOOP_IN		41
#define	LGL_XPONENT_BUTTON_LEFT_LOOP_OUT	43
#define	LGL_XPONENT_BUTTON_LEFT_UP		44
#define	LGL_XPONENT_BUTTON_LEFT_GAIN		11
#define	LGL_XPONENT_BUTTON_LEFT_HIGH		10
#define	LGL_XPONENT_BUTTON_LEFT_MID		9
#define	LGL_XPONENT_BUTTON_LEFT_LOW		8
#define	LGL_XPONENT_BUTTON_LEFT_SYNC		2
#define	LGL_XPONENT_BUTTON_LEFT_VOLUME		7

#define	LGL_XPONENT_BUTTON_RIGHT_RECORD		122
#define	LGL_XPONENT_BUTTON_RIGHT_FINGER		121
#define	LGL_XPONENT_BUTTON_RIGHT_HEADPHONES	120
#define	LGL_XPONENT_BUTTON_RIGHT_X		118
#define	LGL_XPONENT_BUTTON_RIGHT_DASH		119
#define	LGL_XPONENT_BUTTON_RIGHT_LEFT		116
#define	LGL_XPONENT_BUTTON_RIGHT_RIGHT		117
#define	LGL_XPONENT_BUTTON_RIGHT_MOD_POWER_1	112
#define	LGL_XPONENT_BUTTON_RIGHT_MOD_POWER_2	113
#define	LGL_XPONENT_BUTTON_RIGHT_MOD_POWER_3	114
#define	LGL_XPONENT_BUTTON_RIGHT_MOD_POWER_4	115
#define	LGL_XPONENT_BUTTON_RIGHT_REWIND		133
#define	LGL_XPONENT_BUTTON_RIGHT_FAST_FORWARD	134
#define	LGL_XPONENT_BUTTON_RIGHT_POINT_1	123
#define	LGL_XPONENT_BUTTON_RIGHT_POINT_2	124
#define	LGL_XPONENT_BUTTON_RIGHT_POINT_3	125
#define	LGL_XPONENT_BUTTON_RIGHT_POINT_4	126
#define	LGL_XPONENT_BUTTON_RIGHT_POINT_5	127
#define	LGL_XPONENT_BUTTON_RIGHT_PREV		128
#define	LGL_XPONENT_BUTTON_RIGHT_NEXT		129
#define	LGL_XPONENT_BUTTON_RIGHT_LOCK		130
#define	LGL_XPONENT_BUTTON_RIGHT_PLUS		131
#define	LGL_XPONENT_BUTTON_RIGHT_MINUS		132
#define	LGL_XPONENT_BUTTON_RIGHT_EJECT		135
#define	LGL_XPONENT_BUTTON_RIGHT_TOGGLE_PAUSE	136
#define	LGL_XPONENT_BUTTON_RIGHT_LOOP_1		137
#define	LGL_XPONENT_BUTTON_RIGHT_LOOP_2		138
#define	LGL_XPONENT_BUTTON_RIGHT_LOOP_4		139
#define	LGL_XPONENT_BUTTON_RIGHT_LOOP_8		140
#define	LGL_XPONENT_BUTTON_RIGHT_LOOP_CYCLE	142
#define	LGL_XPONENT_BUTTON_RIGHT_LOOP_IN	141
#define	LGL_XPONENT_BUTTON_RIGHT_LOOP_OUT	143
#define	LGL_XPONENT_BUTTON_RIGHT_UP		144
#define	LGL_XPONENT_BUTTON_RIGHT_GAIN		111
#define	LGL_XPONENT_BUTTON_RIGHT_HIGH		110
#define	LGL_XPONENT_BUTTON_RIGHT_MID		109
#define	LGL_XPONENT_BUTTON_RIGHT_LOW		108
#define	LGL_XPONENT_BUTTON_RIGHT_SYNC		102
#define	LGL_XPONENT_BUTTON_RIGHT_VOLUME		107

#define LGL_XPONENT_KNOB_LEFT_RECORD		22
#define	LGL_XPONENT_KNOB_LEFT_MOD_1		12
#define	LGL_XPONENT_KNOB_LEFT_MOD_2		13
#define	LGL_XPONENT_KNOB_LEFT_MOD_3		14
#define	LGL_XPONENT_KNOB_LEFT_MOD_4		15
#define	LGL_XPONENT_KNOB_LEFT_PITCHBEND		30
#define	LGL_XPONENT_KNOB_LEFT_GAIN		11
#define	LGL_XPONENT_KNOB_LEFT_HIGH		10
#define	LGL_XPONENT_KNOB_LEFT_MID		9
#define	LGL_XPONENT_KNOB_LEFT_LOW		8
#define	LGL_XPONENT_KNOB_LEFT_VOLUME		7

#define LGL_XPONENT_KNOB_RIGHT_RECORD		122
#define	LGL_XPONENT_KNOB_RIGHT_MOD_1		112
#define	LGL_XPONENT_KNOB_RIGHT_MOD_2		113
#define	LGL_XPONENT_KNOB_RIGHT_MOD_3		114
#define	LGL_XPONENT_KNOB_RIGHT_MOD_4		115
#define	LGL_XPONENT_KNOB_RIGHT_PITCHBEND	130
#define	LGL_XPONENT_KNOB_RIGHT_GAIN		111
#define	LGL_XPONENT_KNOB_RIGHT_HIGH		110
#define	LGL_XPONENT_KNOB_RIGHT_MID		109
#define	LGL_XPONENT_KNOB_RIGHT_LOW		108
#define	LGL_XPONENT_KNOB_RIGHT_VOLUME		107

#define	LGL_XPONENT_KNOB_CUE			90
#define	LGL_XPONENT_KNOB_XFADER			91
#define	LGL_XPONENT_KNOB_TOUCHPAD_X		92
#define	LGL_XPONENT_KNOB_TOUCHPAD_Y		93



//Xsession

LGL_MidiDevice*
LGL_GetXsession();

#define	LGL_XSESSION_BUTTON_LEFT_HEADPHONES	44
#define	LGL_XSESSION_BUTTON_LEFT_LEFT		46
#define	LGL_XSESSION_BUTTON_LEFT_RIGHT		43
#define	LGL_XSESSION_BUTTON_LEFT_EJECT		58
#define	LGL_XSESSION_BUTTON_LEFT_TOGGLE_PAUSE	70

#define	LGL_XSESSION_BUTTON_RIGHT_HEADPHONES	45
#define	LGL_XSESSION_BUTTON_RIGHT_LEFT		56
#define	LGL_XSESSION_BUTTON_RIGHT_RIGHT		57
#define	LGL_XSESSION_BUTTON_RIGHT_EJECT		59
#define	LGL_XSESSION_BUTTON_RIGHT_TOGGLE_PAUSE	69

#define	LGL_XSESSION_KNOB_LEFT_3		24
#define	LGL_XSESSION_KNOB_LEFT_2		25
#define	LGL_XSESSION_KNOB_LEFT_1		26
#define	LGL_XSESSION_KNOB_LEFT_HIGH		27
#define	LGL_XSESSION_KNOB_LEFT_MID		28
#define	LGL_XSESSION_KNOB_LEFT_LOW		29
#define	LGL_XSESSION_KNOB_LEFT_PITCHBEND	12
#define	LGL_XSESSION_KNOB_LEFT_VOLUME		11

#define	LGL_XSESSION_KNOB_RIGHT_3		31
#define	LGL_XSESSION_KNOB_RIGHT_2		32
#define	LGL_XSESSION_KNOB_RIGHT_1		33
#define	LGL_XSESSION_KNOB_RIGHT_HIGH		34
#define	LGL_XSESSION_KNOB_RIGHT_MID		35
#define	LGL_XSESSION_KNOB_RIGHT_LOW		36
#define	LGL_XSESSION_KNOB_RIGHT_VOLUME		14
#define	LGL_XSESSION_KNOB_RIGHT_PITCHBEND	15

#define	LGL_XSESSION_KNOB_XFADER		20



//TriggerFinger

LGL_MidiDevice*
LGL_GetTriggerFinger();

#define	LGL_TRIGGERFINGER_BUTTON_P1		36
#define	LGL_TRIGGERFINGER_BUTTON_P2		38
#define	LGL_TRIGGERFINGER_BUTTON_P3		40
#define	LGL_TRIGGERFINGER_BUTTON_P4		37
#define	LGL_TRIGGERFINGER_BUTTON_P5		50
#define	LGL_TRIGGERFINGER_BUTTON_P6		48
#define	LGL_TRIGGERFINGER_BUTTON_P7		45
#define	LGL_TRIGGERFINGER_BUTTON_P8		41
#define	LGL_TRIGGERFINGER_BUTTON_P9		56
#define	LGL_TRIGGERFINGER_BUTTON_P10		39
#define	LGL_TRIGGERFINGER_BUTTON_P11		42
#define	LGL_TRIGGERFINGER_BUTTON_P12		46
#define	LGL_TRIGGERFINGER_BUTTON_P13		49
#define	LGL_TRIGGERFINGER_BUTTON_P14		57
#define	LGL_TRIGGERFINGER_BUTTON_P15		51
#define	LGL_TRIGGERFINGER_BUTTON_P16		53
#define	LGL_TRIGGERFINGER_KNOB_C1		10
#define	LGL_TRIGGERFINGER_KNOB_C2		91
#define	LGL_TRIGGERFINGER_KNOB_C3		12
#define	LGL_TRIGGERFINGER_KNOB_C4		93
#define	LGL_TRIGGERFINGER_KNOB_C5		5
#define	LGL_TRIGGERFINGER_KNOB_C6		71
#define	LGL_TRIGGERFINGER_KNOB_C7		84
#define	LGL_TRIGGERFINGER_KNOB_C8		72
#define	LGL_TRIGGERFINGER_KNOB_F1		7
#define	LGL_TRIGGERFINGER_KNOB_F2		1
#define	LGL_TRIGGERFINGER_KNOB_F3		71
#define	LGL_TRIGGERFINGER_KNOB_F4		74



//JP8k

LGL_MidiDevice*
LGL_GetJP8k();

#define	LGL_JP8K_BUTTON_KEY_0_0			48
#define	LGL_JP8K_BUTTON_KEY_0_1			49
#define	LGL_JP8K_BUTTON_KEY_0_2			50
#define	LGL_JP8K_BUTTON_KEY_0_3			51
#define	LGL_JP8K_BUTTON_KEY_0_4			52
#define	LGL_JP8K_BUTTON_KEY_0_5			53
#define	LGL_JP8K_BUTTON_KEY_0_6			54
#define	LGL_JP8K_BUTTON_KEY_0_7			55
#define	LGL_JP8K_BUTTON_KEY_0_8			56
#define	LGL_JP8K_BUTTON_KEY_0_9			57
#define	LGL_JP8K_BUTTON_KEY_0_10		58
#define	LGL_JP8K_BUTTON_KEY_0_11		59
#define	LGL_JP8K_BUTTON_KEY_1_0			60
#define	LGL_JP8K_BUTTON_KEY_1_1			61
#define	LGL_JP8K_BUTTON_KEY_1_2			62
#define	LGL_JP8K_BUTTON_KEY_1_3			63
#define	LGL_JP8K_BUTTON_KEY_1_4			64
#define	LGL_JP8K_BUTTON_KEY_1_5			65
#define	LGL_JP8K_BUTTON_KEY_1_6			66
#define	LGL_JP8K_BUTTON_KEY_1_7			67
#define	LGL_JP8K_BUTTON_KEY_1_8			68
#define	LGL_JP8K_BUTTON_KEY_1_9			69
#define	LGL_JP8K_BUTTON_KEY_1_10		70
#define	LGL_JP8K_BUTTON_KEY_1_11		71
#define	LGL_JP8K_BUTTON_KEY_2_0			72
#define	LGL_JP8K_BUTTON_KEY_2_1			73
#define	LGL_JP8K_BUTTON_KEY_2_2			74
#define	LGL_JP8K_BUTTON_KEY_2_3			75
#define	LGL_JP8K_BUTTON_KEY_2_4			76
#define	LGL_JP8K_BUTTON_KEY_2_5			77
#define	LGL_JP8K_BUTTON_KEY_2_6			78
#define	LGL_JP8K_BUTTON_KEY_2_7			79
#define	LGL_JP8K_BUTTON_KEY_2_8			80
#define	LGL_JP8K_BUTTON_KEY_2_9			81
#define	LGL_JP8K_BUTTON_KEY_2_10		82
#define	LGL_JP8K_BUTTON_KEY_2_11		83
#define	LGL_JP8K_BUTTON_KEY_3_0			84
#define	LGL_JP8K_BUTTON_KEY_3_1			85
#define	LGL_JP8K_BUTTON_KEY_3_2			86
#define	LGL_JP8K_BUTTON_KEY_3_3			87
#define	LGL_JP8K_BUTTON_KEY_3_4			88
#define	LGL_JP8K_BUTTON_KEY_3_5			89
#define	LGL_JP8K_BUTTON_KEY_3_6			90
#define	LGL_JP8K_BUTTON_KEY_3_7			91
#define	LGL_JP8K_BUTTON_KEY_3_8			92
#define	LGL_JP8K_BUTTON_KEY_3_9			93
#define	LGL_JP8K_BUTTON_KEY_3_10		94
#define	LGL_JP8K_BUTTON_KEY_3_11		95
#define	LGL_JP8K_BUTTON_KEY_4_0			96



//OSC

class LGL_OscServer :
	private osc::OscPacketListener
{

public:

			LGL_OscServer(int port);
			~LGL_OscServer();
	
	void		ThreadFunc();

protected:

	virtual void	ProcessMessage
			(
				const osc::ReceivedMessage&	m,
				const IpEndpointName&		remoteEndpoint
			);

private:

	SDL_Thread*	Thread;
	UdpListeningReceiveSocket
			ListeningReceiveSocket;

};

class LGL_OscClient
{

public:

			LGL_OscClient
			(
				const char*	address,
				int		port
			);
			~LGL_OscClient();

	osc::OutboundPacketStream&
			Stream();
	void		Send();
	
	const char*	GetAddress();

private:

    	UdpTransmitSocket
			TransmitSocket;
	osc::OutboundPacketStream
			PacketStream;
#define			PacketStreamBufferBytes (1024)
	char		PacketStreamBuffer[PacketStreamBufferBytes];

	char		Address[2048];

};



//Syphon

int
LGL_SyphonServerCount();

LGL_Image*
LGL_SyphonImage(int server=0);

void
LGL_SyphonPushImage(LGL_Image* img);

//VidCam

bool		LGL_VidCamAvailable();
int		LGL_VidCamJoyNumber();
int		LGL_VidCamFPS();
LGL_Image*	LGL_VidCamImageRaw();
LGL_Image*	LGL_VidCamImageProcessed();
void		LGL_VidCamCalibrate(LGL_Image* Background, float FadeSeconds);



//Networking

class LGL_Datagram
{

public:

				LGL_Datagram();
				~LGL_Datagram();

	void			ClearData();

	bool			LoadData(FILE* file);
	bool			LoadData
				(
					char*	inMeta,
					Uint16	inDataLength=0,
					Uint8*	inData=NULL
				);

	bool			LoadData
				(
					Uint16	inMetaLength,
					char*	inMeta,
					Uint16	inDataLength,
					Uint8*	inData
				);

	unsigned int		Argc() const;
const	char*			Argv(unsigned int index) const;
	Uint16			GetMetaLength() const;
const	char*			GetMeta() const;
	Uint16			GetDataLength() const;
const	Uint8*			GetData() const;

private:
	std::vector<char*>	argv;		//Easy Access to Metadata

	Uint16			MetaLength;
	char*			Meta;
	Uint16			DataLength;
	Uint8*			Data;

	char*			MetaArgv;	//What does this do...?
};

//FIXME: WARNING!!!! LGL_NetConnections are INSECURE. Call RecvFiles() at your own risk!
class LGL_NetConnection
{

public:

				//Creation

				LGL_NetConnection();					//Server
				LGL_NetConnection(char* inHost);			//Client
				LGL_NetConnection(int ip0, int ip1, int ip2, int ip3);	//Client
				LGL_NetConnection(LGL_NetConnection* parent, TCPsocket inSocket);
				~LGL_NetConnection();
	void			ConstructorGeneric();	//Don't call this, k?

				//Connection

	bool			ConnectTCP(int port);
	bool			ListenTCP(int port);
	LGL_NetConnection*	AcceptTCP();
	void			CloseTCP();

				//Communication

	bool			SendTCP(LGL_Datagram* dg);	//Don't call this one, k?
	bool			SendTCP(char* inMeta, Uint16 inDataLength=0, Uint8* Data=NULL);
	bool			RecvTCP(LGL_Datagram* datagram);
	bool			PeekTCP();

				//File Sync

	bool			SendFile(char* file);
	bool			SendDirectory(char* dir);
	void			RecvFiles(char* requiredPrefix);

	int			SendFileQueuedFiles();
	long			SendFileQueuedBytes();
	int			SendFileCompletedFiles();
	long			SendFileCompletedBytes();
	float			SendFileCompletedPercent();
	int			SendFileTotalFiles();
	long			SendFileTotalBytes();
	void			SendFileResetStatistics();
	char*			SendFileNowName();
	char*			SendFileNowStatus();

	int			RecvFileQueuedFiles();
	long			RecvFileQueuedBytes();
	int			RecvFileCompletedFiles();
	long			RecvFileCompletedBytes();
	float			RecvFileCompletedPercent();
	int			RecvFileTotalFiles();
	long			RecvFileTotalBytes();
	void			RecvFileResetStatistics();
	char*			RecvFileNowName();
	char*			RecvFileNowStatus();

				//Information
	
	int			ConnectionStatus();	//-2 Previous successful connection closed
							//-1 Failed to establish a connection
							// 0 Connection not yet established
							//+1 Connecting...
							//+2 Connected
				
				//-1 no		0 not yet	1 yes
	int			HostResolved(bool blockUntilKnown=false);
	int			IPResolved(bool blockUntilKnown=false);
				//-1 no		0 not yet	1 yes

	char*			Hostname(bool blockUntilResolution=false);
	int			IPQuad(int whichQuad,bool blockUntilResolution=false);
	char*			IPString(bool blockUntilResolution=false);

//private:
	
	char			Host[256];
	int			IP[4];
	char			IPstr[256];
	int			Port;
	int			Connection_Status;

	TCPsocket		SocketTCP;

	SDL_Thread*		ResolverThreadHost;
	SDL_Thread*		ResolverThreadIP;
	SDL_Thread*		ConnectTCPThread;
	SDL_Thread*		SocketSendThread;
	SDL_Thread*		SocketRecvThread;

	std::deque
	<LGL_Datagram*>		SendBuffer;

	std::deque
	<LGL_Datagram*>		RecvBuffer;

	std::deque<char*>	FileSendBuffer[2];	//0 checking...		1 to be sent
	bool			FileSendQueryOK;
	FILE*			FileSendNowFD;
	long			FileSendNowLength;
	char			FileSendNowName[1024];
	char			FileSendNowStatus[1024];
	char			FileRecvNowName[1024];
	char			FileRecvNowStatus[1024];

	int			FileSendQueuedFiles;
	long			FileSendQueuedBytes;
	int			FileSendCompletedFiles;
	long			FileSendCompletedBytes;
	int			FileRecvQueuedFiles;
	long			FileRecvQueuedBytes;
	int			FileRecvCompletedFiles;
	long			FileRecvCompletedBytes;

	SDL_mutex*		Mutex;

	char			RecvFileRequiredPrefix[256];
};

bool		LGL_NetAvailable();
bool		LGL_NetHostToIP(char* host, int& a, int& b, int& c, int& d);	//Blocks on net
bool		LGL_NetIPToHost(char* host, int& a, int& b, int& c, int& d);	//Blocks on net

//Memory
/*
int		LGL_MemTotalMB();
int		LGL_MemUsedMB();
float		LGL_MemUsedPercent();
int		LGL_MemAvailableMB();
float		LGL_MemAvailablePercent();

int		LGL_SwapTotalMB();
int		LGL_SwapUsedMB();
float		LGL_SwapUsedPercent();
int		LGL_SwapAvailableMB();
float		LGL_SwapAvailablePercent();
*/

//Filesystem

enum
LGL_FileType
{
	LGL_FILETYPE_UNDEF,
	LGL_FILETYPE_FILE,
	LGL_FILETYPE_DIR,
	LGL_FILETYPE_SYMLINK,
	LGL_FILETYPE_ALIAS
};

class LGL_FileInfo
{

public:

				LGL_FileInfo
				(
					const char*	path,
					LGL_FileType	type=LGL_FILETYPE_UNDEF,
					long		bytes=-1
				);
				~LGL_FileInfo();

	char*			Path;
	const char*		GetPathShort();
	LGL_FileType		Type;
	long			Bytes;
	LGL_FileInfo*		ResolvedFileInfo;
};

class LGL_DirTree
{

public:

				LGL_DirTree(const char* path=NULL);
				~LGL_DirTree();
	
	bool			Ready() const;

	const
	char*			GetPath() const;
	bool			SetPath(const char* path=NULL);

	void			Refresh();
	void			Refresh_INTERNAL();

	unsigned int		GetFileCount() const;
	unsigned int		GetDirCount() const;

	const
	char*			GetFileName(unsigned int index) const;
	const
	char*			GetDirName(unsigned int index) const;

	const
	char*			GetFilterText() const;
	void			SetFilterText(const char* filterText=NULL);

private:
	void			GenerateFilterLists();

public:
	
	unsigned int		GetFilteredFileCount();
	unsigned int		GetFilteredDirCount();

	const
	char*			GetFilteredFileName(unsigned int index);
	const
	char*			GetFilteredDirName(unsigned int index);
	void			WaitOnWorkerThread() const;

private:

	void			ClearLists();

	char			Path[1024];
	std::vector<char*>	FileList;
	std::vector<char*>	DirList;

	char			FilterText[1024];
	std::vector<char*>	FilteredFileList;
	std::vector<char*>	FilteredDirList;

	SDL_Thread*		WorkerThread;
	bool			WorkerThreadDone;
};

class LGL_FileToRam
{

public:

				LGL_FileToRam(const char* path);
				~LGL_FileToRam();

	const char*		GetPath();
	int			GetStatus();
	bool			GetFailed();	//Status == -1
	bool			GetLoading();	//Status == 0
	bool			GetReady();	//Status == 1
const	char*			GetData();
	long			GetByteCount();
	bool			GetThreadTerminateSignal();
	void			SetThreadTerminateSignal();
	bool			GetReadyForNonblockingDelete();

	//Thread funcs

	void			Thread_Load();

private:

	char			Path[2048];
	char*			Data;
	long			ByteCount;
	int			Status;
	SDL_Thread*		ThreadWorker;
	bool			ThreadTerminateSignal;
};

//FIXME: LGL_DirectoryListCreate returns a vector<char*>, which encourages memleaks.

bool		LGL_FileExists(const char* path);
bool		LGL_DirectoryExists(const char* dir);
bool		LGL_DirectoryCreate(const char* dir);
bool		LGL_DirectoryCreateChain(const char* dir);
bool		LGL_FileDelete(const char* file);
bool		LGL_DirectoryDelete(const char* dir);
bool		LGL_FileDirMove(const char* oldLocation, const char* newLocation);
double		LGL_FileLengthBytes(const char* file);
bool		LGL_FirstFileMoreRecentlyModified(const char* firstFile, const char* secondFile);
char*		LGL_MD5sum(const char* file, char* output);	//output must be at least char[32]
bool		LGL_FileExtension(const char* filename, const char* extension);
bool		LGL_FileExtensionIsAudio(const char* filename);
bool		LGL_FileExtensionIsImage(const char* filename);
void		LGL_SimplifyPath(char* simplePath, const char* complexPath);
const char*	LGL_GetUsername();
const char*	LGL_GetHomeDir();
bool		LGL_PathIsAlias(const char* path, bool useCache=true);
bool		LGL_ResolveAlias(char* outPath, int outPathLength, const char* inPath);
bool		LGL_PathIsSymlink(const char* path);
bool		LGL_ResolveSymlink(char* outPath, int outPathLength, const char* inPath);
void		LGL_WriteFileAsync(const char* path, const char* data, int len);
unsigned int	LGL_WriteFileAsyncQueueCount();

class
lgl_PathIsAliasCacher
{

public:

		lgl_PathIsAliasCacher(const char* path=NULL);
		~lgl_PathIsAliasCacher();

	void	Load();
	void	Save();

	void	Add
		(
			const char*	path,
			int		isAlias
		);
	
	int	Check
		(
			const char*	path
		);

private:

	char*	Path;
	std::map<std::string, int>
		Map;

};

//DEPRECATED! Use LGL_DirTree.
std::vector<char*>
		LGL_DirectoryListCreate
		(
			const char*			TargetDir,
			bool				justFiles=true,
			bool				seeHidden=false,
			std::vector<LGL_FileInfo*>*	fileInfoList=NULL
		);
void		LGL_DirectoryListDelete(std::vector<char*>& list);

//Misc

int64_t		LGL_RamFreeB();
int		LGL_RamFreeMB();
int64_t		LGL_MemoryUsedByThisB();
int		LGL_MemoryUsedByThisMB();
float		LGL_MemoryFreePercent();
bool		LGL_BatteryChargeDraining();
float		LGL_BatteryChargePercent();
float		LGL_FilesystemFreeSpaceMB();

void		LGL_DrawFPSGraph
		(
			float left=0.8f,	float right=0.9f,
			float bottom=0.8f,	float top=0.9f,
			float brightness=1.0f,
			float alpha=1.0f
		);
void		LGL_ResetFPSGraph();

void		LGL_ScreenShot(const char* bmpFile="screenshot.bmp");

void		LGL_RecordMovieStart(const char* inPathPrefix, int fps);
void		LGL_RecordMovieStop();

void		LGL_DrawLogStart(const char* outFile);
void		LGL_DrawLogWrite(const char* str,...);
void		LGL_DrawLogWriteForceNow(const char* str,...);
void		LGL_DrawLogPause(bool pause=true);
void		LGL_DrawLogStop();

void		LGL_FrameBufferTextureGlitchFixToggle();

//Debugging

class lgl_tracing_output_debug_string
{
public:
	lgl_tracing_output_debug_string
	(
		bool	test,
		const
		char*	testStr,
		const
		char*	file,
		int	line
	) :
		m_test( test ),
		m_testStr( testStr ),
		m_file( file ),
		m_line( line )
	{
		//
	}

	bool	operator()(const char* format, ... )
	{
		//Process the formatted part of the string
		char tmpstr[1024];
		va_list args;
		va_start(args,format);
		vsprintf(tmpstr,format,args);
		va_end(args);

		//Get rid of one trailing \n, as we already do that.
		if(tmpstr[strlen(tmpstr)-1]=='\n') tmpstr[strlen(tmpstr)-1]='\0';

		//Assert or printf if either necessary
		if(m_test==false)
		{
			printf("%s:%i: LGL_Assertf(%s): Failed! (\"%s\")\n",m_file,m_line,m_testStr,tmpstr);
		}
		else if(m_testStr==NULL)
		{
			printf("%s:%i: %s\n",m_file,m_line,tmpstr);
		}
		return(m_test);
	}

private:
	
	bool		m_test;
const	char*		m_testStr;
const	char*		m_file;
	int		m_line;
};

bool	lgl_assert
	(
		bool	test,
		const
		char*	testStr,
		const
		char*	file,
		int	line
	);

#define lgl_AssertfPayload(a,b) (lgl_tracing_output_debug_string( a, b, __FILE__, __LINE__ ))
#define LGL_Tracef (lgl_tracing_output_debug_string( true, NULL, __FILE__, __LINE__))
#define LGL_Assert(a) if(lgl_assert(a,#a,__FILE__,__LINE__)==false) assert(a);
#define LGL_Assertf(a,b) if((lgl_AssertfPayload(a,#a)b)==false) assert(a);
#define LGL_AssertIfDebugMode() if(LGL_GetDebugMode() && !lgl_assert(false,"Debug Mode Assert",__FILE__,__LINE__)) assert(false);

/*
void		LGL_Tracef(const char* str, ...);
void		LGL_Assert(bool test);
void		LGL_Assertf(bool test, (const char* str, ...));
*/

void		lgl_LoopCounterAlpha
		(
			const char*	file,
			long		line
		);

#define		LGL_LoopCounterAlpha() lgl_LoopCounterAlpha(__FILE__,__LINE__)
void		LGL_LoopCounterDelta();//long warningIterations=90);
void		LGL_LoopCounterOmega(long warningIterations=99);

const
char*		LGL_GetErrorStringGL();

const
char*		LGL_CompileTime();

const
char*		LGL_CompileDate();

//Note on movies:
//
//After you record, you'll have a file called movie.avi in the same dir as your executable.

float		LGL_RandFloat();
float		LGL_RandFloat(float highest);
float		LGL_RandFloat(float lowest, float highest);
float		LGL_RandGaussian();
int		LGL_RandInt(int lowest, int highest);
bool		LGL_RandBool(float trueProbability=.5);

float		LGL_Interpolate(float in0, float in1, float interpolationFactor);
float		LGL_InterpolateModulus
		(
			float	in0,
			float	in1,
			float	interpolationFactor,
			float	modulus
		);
float		LGL_Clamp(float lowerbound, float target, float upperbound);
float		LGL_ClampModulus(float lowerbound, float target, float upperbound, float modulus);
float		LGL_DifferenceModulus
		(
			float	in0,
			float	in1,
			float	modulus
		);
float		LGL_Round(float in);
float		LGL_Round(float in, int decimals);
double		LGL_Min(double in1, double in2);
double		LGL_Max(double in1, double in2);
float		LGL_Sign(float in);

class LGL_Color
{

public:

			LGL_Color
			(
				const
				float&	r=0.0f,
				const
				float&	g=0.0f,
				const
				float&	b=0.0f,
				const
				float&	a=0.0f
			);
			~LGL_Color();
	
const	float		GetR() const;
const	float		GetG() const;
const	float		GetB() const;
const	float		GetA() const;

	void		SetR(const float& r);
	void		SetG(const float& g);
	void		SetB(const float& b);
	void		SetA(const float& a);
	void		SetRGB
			(
				const
				float&	r,
				const
				float&	g,
				const
				float&	b
			);
	void		SetRGBA
			(
				const
				float&	r,
				const
				float&	g,
				const
				float&	b,
				const
				float&	a
			);
	
	LGL_Color	operator+(const LGL_Color& color) const;
	LGL_Color	operator*(const float& scalar) const;

private:

	float		R;
	float		G;
	float		B;
	float		A;

};



//LEDs

class lgl_LEDHost
{

public:

		lgl_LEDHost
		(
			const char*	hostname,
			int		port=6038
		);

	bool	Matches
		(
			const char*	hostname,
			int		port
		);

	void	SetColor
		(
			float	red,
			float	green,
			float	blue,
			int	channel
		);

	void	Send();

private:

	char	Hostname[2048];
	int	Port;
	int	SocketInstance;

	char	Red[128];
	char	Green[128];
	char	Blue[128];
	int	ChannelCount;
};

class LGL_LEDClient
{

public:

		LGL_LEDClient
		(
			const char*	hostname,
			int		port=6038,
			int		channel=0,
			int		group=0
		);
		~LGL_LEDClient();

	void	SetColor(float red, float green, float blue);
	int	GetGroup();

private:

	char	Hostname[2048];
	int	Port;
	int	Channel;
	int	Group;

	lgl_LEDHost*
		LEDHost;
};



void LGL_SpawnMainThreadBlockDetector();








//FIXME: Move the internals section at the bottom of .cpp file into the .h file
//Internals: Game Developers shouldn't use the functions below...

#define SDL_DSP_NOSELECT		1

int		LGL_NextPowerOfTwo(float target);

void		LGL_ShutDown();

#endif //DEFINE_LGL

