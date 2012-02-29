/*
 *
 * LGL.cpp
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

//#define	LGL_NO_GRAPHICS

#include "LGL.h"
#ifdef	LGL_OSX
#include "quicktime.module/lgl_quicktime.h"
#endif

#include <stdlib.h>
#include <math.h>
#include <time.h>	//Purely for random# seeding
#include <string.h>
#include <ctype.h>	//isapha(), ispunct(), etc

#include <SDL_video.h>
#include <SDL_image.h>
#include <SDL_net.h>
#include <SDL_endian.h>

#include <stdlib.h>		//malloc()
#include <unistd.h>		//read(), LGL_Memory*
#include <sys/types.h>
#define _DARWIN_USE_64_BIT_INODE
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>

#ifdef	LGL_LINUX

#include <sys/ioctl.h>
#include <sys/time.h>		//LGL_Memory*
#include <sys/resource.h>	//LGL_Memory*
#include <dirent.h>		//Directory Searching
#include <errno.h>		//Error indentification
#include <sys/mman.h>		//mmap()
#include <sys/statvfs.h>	//Free disk space
#include <sys/param.h>		//for cpu count
#include <sys/sysctl.h>		//for cpu count

#include <sched.h>

#endif	//LGL_LINUX

#include <samplerate.h>

#ifdef LGL_LINUX_VIDCAM
//<V4L>
#include <stdlib.h>		//malloc()
#include <sys/ioctl.h>
#include <sys/time.h>		//LGL_Memory*
#include <sys/resource.h>	//LGL_Memory*
#include <dirent.h>		//Directory Searching
#include <linux/videodev.h>	//v4l

#define	LGL_VIDCAM_FPS_MIN	10.0
//<V4L>
#endif //LGL_LINUX_VIDCAM

#ifdef	LGL_OSX

#include <CoreServices/CoreServices.h>
#include <Aliases.h>
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
#include <mach/thread_act.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h> 
#include <mach/mach_host.h>
#include <mach/task.h>
#include <mach/task_info.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

#endif	//LGL_OSX

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef	LGL_OSX
#define	LGL_PRIORITY_AUDIO_OUT		(1.0f)
#define	LGL_PRIORITY_MAIN		(0.85f)
#define	LGL_PRIORITY_VIDEO_DECODE	(0.5f)
#define	LGL_PRIORITY_VIDEO_PRELOAD	(0.6f)
#define	LGL_PRIORITY_VIDEO_READAHEAD	(0.65f)
#define	LGL_PRIORITY_VIDEO_LOAD		(0.8f)
#define	LGL_PRIORITY_AUDIO_DECODE	(0.7f)
#define	LGL_PRIORITY_AUDIO_ENCODE	(0.75f)
#define	LGL_PRIORITY_OSC		(0.85f)
#else
#define	LGL_PRIORITY_AUDIO_OUT		(1.0f)
#define	LGL_PRIORITY_MAIN		(0.9f)
#define	LGL_PRIORITY_VIDEO_DECODE	(0.5f)
#define	LGL_PRIORITY_VIDEO_LOAD		(0.8f)
#define	LGL_PRIORITY_AUDIO_DECODE	(0.8f)
#define	LGL_PRIORITY_AUDIO_ENCODE	(0.75f)
#define	LGL_PRIORITY_OSC		(0.85f)
#endif

#define LGL_EQ_SAMPLES_FFT	(512)
#define LGL_SAMPLESIZE		(256)
unsigned int LGL_SAMPLESIZE_SDL;

bool lgl_sdl_initialized=false;
bool lgl_lgl_initialized=false;

void lgl_AudioOutCallback(void* userdata, Uint8* stream, int len8);
void lgl_AudioInCallback(void *udata, Uint8 *stream, int len8);

void lgl_fftw_init();

fftwf_complex* fft_array_main_forward = NULL;
fftwf_complex* fft_array_main_backward = NULL;
fftwf_complex* fft_array_callback_forward = NULL;
fftwf_complex* fft_array_callback_backward = NULL;
fftwf_plan plan_main_forward;
fftwf_plan plan_main_backward;
fftwf_plan plan_callback_forward;
fftwf_plan plan_callback_backward;
int fft_elementCount=LGL_EQ_SAMPLES_FFT;

#define SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES (1)
#define SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES_EDGE (0)

typedef struct
{
	bool			Occupied;
	bool			ClearMe;
	bool			Paused;
	double			PositionSamplesStart;
	double			PositionSamplesPrev;
	double			PositionSamplesNow;
	double			PositionSamplesNowOutwards;
	double			FuturePositionSamplesPrev;
	double			FuturePositionSamplesNow;
	double			PositionSamplesNowLastReported;
	double			PositionSamplesEnd;
	LGL_Timer		PositionSamplesDeltaLastTime;
	float			VolumeFrontLeftDesired;
	float			VolumeFrontRightDesired;
	float			VolumeBackLeftDesired;
	float			VolumeBackRightDesired;
	float			VolumeFrontLeft;
	float			VolumeFrontRight;
	float			VolumeBackLeft;
	float			VolumeBackRight;
	int			Channels;
	int			Hz;
	bool			ToMono;
	float			SpeedNow;
	float			SpeedDesired;
	float			SpeedInterpolationFactor;
	float			SpeedVolumeFactor;
	bool			Glitch;
	float			GlitchVolume;
	float			GlitchSpeedNow;
	float			GlitchSpeedDesired;
	float			GlitchSpeedInterpolationFactor;
	float			GlitchDuo;
	bool			GlitchLuminScratch;
	long			GlitchLuminScratchPositionDesired;
	double			GlitchSamplesNow;
	double			GlitchLast;
	double			GlitchBegin;
	double			GlitchLength;

	bool			FutureGlitchSettingsAvailable;
	float			FutureGlitchSpeedDesired;
	long			FutureGlitchLuminScratchPositionDesired;

	long			FutureGlitchSamplesNow;	//Not affected by FutureGlitchSettingsAvailable. Meh.

	long			RhythmicInvertAlphaSamples;
	long			RhythmicInvertDeltaSamples;
	bool			RhythmicVolumeInvert;

	int			RespondToRhythmicSoloInvertChannel;
	int			RespondToRhythmicSoloInvertCurrentValue;

	float			VU;

	bool			Loop;
	bool			StickyEndpoints;
	double			LengthSamples;
	unsigned long 		BufferLength;
	Uint8*			Buffer;
	LGL_Semaphore*		BufferSemaphore;
	LGL_Sound*		LGLSound;
	LGL_AudioDSP*		LGLAudioDSP[2];

	int			DivergeCount;
	double			DivergeSamples;
	float			DivergeSpeed;
	bool			DivergeRecallNow;
	double			DivergeRecallSamples;
	double			WarpPointSecondsAlpha;
	double			WarpPointSecondsOmega;
	bool			WarpPointLoop;
	bool			WarpPointLock;

	SRC_STATE*		SampleRateConverter[4];
	float			SampleRateConverterBuffer[4][SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES];
	float			SampleRateConverterBufferSpeed;
	long			SampleRateConverterBufferStartSamples;
	int			SampleRateConverterBufferValidSamples;
	long			SampleRateConverterBufferConsumedSamples;
	int			SampleRateConverterBufferCurrentSamplesIndex;
} LGL_SoundChannel;

#define	LGL_SOUND_CHANNEL_NUMBER	32

class lgl_WriteFileAsyncWorkItem
{
public:
	lgl_WriteFileAsyncWorkItem
	(
		const char*	path,
		const char*	data,
		int		len
	);

	~lgl_WriteFileAsyncWorkItem();

	void		Write();
	const char*	GetPath();

private:

	char*		Path;
	char*		Data;
	int		Len;
};

bool lgl_HideProjectorWindows=false;
void
LGL_HideProjectorWindows()
{
	lgl_HideProjectorWindows=true;
}

bool lgl_AudioSwapOutputStreams=false;
void
LGL_AudioSwapOutputStreams()
{
	lgl_AudioSwapOutputStreams=true;
}

bool lgl_FakeSecondDisplay=false;
void
LGL_FakeSecondDisplay()
{
	lgl_FakeSecondDisplay=true;
}

bool lgl_FakeSecondDisplay3x=false;
void
LGL_FakeSecondDisplay3x()
{
	lgl_FakeSecondDisplay=true;
	lgl_FakeSecondDisplay3x=true;
}

typedef struct
{
	//Video
	
	int			DisplayCount;
	int			DisplayNow;
	int			DisplayResolutionX[LGL_DISPLAY_MAX];
	int			DisplayResolutionY[LGL_DISPLAY_MAX];
	int			DisplayRefreshRate[LGL_DISPLAY_MAX];
	int			WindowResolutionX[LGL_DISPLAY_MAX];
	int			WindowResolutionY[LGL_DISPLAY_MAX];
	bool			WindowFullscreen;
	bool			VSync;

	SDL_WindowID		WindowID[LGL_DISPLAY_MAX];
	SDL_GLContext		GLContext;
	SDL_WindowID		MasterWindowID;
	int			MasterWindowResolutionX;
	int			MasterWindowResolutionY;

	SDL_threadID		ThreadIDMain;
	SDL_threadID		ThreadIDWatch;

	float			DisplayViewportLeft[LGL_DISPLAY_MAX];
	float			DisplayViewportRight[LGL_DISPLAY_MAX];
	float			DisplayViewportBottom[LGL_DISPLAY_MAX];
	float			DisplayViewportTop[LGL_DISPLAY_MAX];

	float			ViewportLeft;
	float			ViewportRight;
	float			ViewportBottom;
	float			ViewportTop;

	float			ViewportEyeX;
	float			ViewportEyeY;
	float			ViewportEyeZ;
	float			ViewportTargetX;
	float			ViewportTargetY;
	float			ViewportTargetZ;
	float			ViewportUpX;
	float			ViewportUpY;
	float			ViewportUpZ;

	int			GPUSpeed;
	int			GPUTemp;
	char			GPURenderer[1024];
	SDL_Thread*		GPUSpeedTempUpdateThread;

	GLhandleARB		ShaderProgramCurrent;

	float			StatsLinesPerFrame;
	float			StatsRectsPerFrame;
	float			StatsImagesPerFrame;
	float			StatsPixelsPerFrame;

	char			ErrorStringGL[128];

	LGL_Image*		SyphonImage;

	//Audio

	bool			AudioAvailable;
	bool			AudioWasOnceAvailable;
	bool			AudioQuitting;

	bool			AudioUsingJack;
	bool			AudioJackXrunBack;
	bool			AudioJackXrunFront;

	SDL_AudioSpec*		AudioSpec;
	LGL_SoundChannel	SoundChannel[LGL_SOUND_CHANNEL_NUMBER];
	int			AudioBufferPos;
	float			AudioBufferLFront[1024];
	float			AudioBufferLBack[1024];
	float			AudioBufferRFront[1024];
	float			AudioBufferRBack[1024];
	float			AudioPeakLeft;
	float			AudioPeakRight;
	float			AudioPeakMono;
	float			FreqBufferL[512];
	float			FreqBufferR[512];
	unsigned long		RecordSamplesWritten;
	Uint8			RecordBuffer[LGL_SAMPLESIZE*4*4];
	float			RecordVolume;
	std::vector<LGL_AudioStream*>
				AudioStreamList;
	LGL_Semaphore*		AudioStreamListSemaphore;
	LGL_Semaphore*		AVOpenCloseSemaphore;
	bool			AudioMasterToHeadphones;
	char			AudioEncoderPath[2048];
	LGL_AudioEncoder*	AudioEncoder;
	LGL_Semaphore*		AudioEncoderSemaphore;
	LGL_Timer		AudioOutCallbackTimer;
	LGL_Timer		AudioOutReconnectTimer;

	//AudioIn

	bool			AudioInAvailable;
	bool			AudioInPassThru;
	bool			AudioOutDisconnected;
	std::vector<LGL_AudioGrain*>
				AudioInGrainListFixedSize;
	std::vector<LGL_AudioGrain*>
				AudioInGrainListFront;
	std::vector<LGL_AudioGrain*>
				AudioInGrainListBack;
	LGL_Semaphore*		AudioInSemaphore;
	LGL_AudioGrain*		AudioInFallbackGrain;

	//Keyboard
	
	bool			KeyDown[LGL_KEY_MAX];
	bool			KeyStroke[LGL_KEY_MAX];
	bool			KeyRelease[LGL_KEY_MAX];
	LGL_Timer		KeyTimer[LGL_KEY_MAX];
	char			KeyStream[256];
	LGL_InputBuffer*	InputBufferFocus;

	//Mouse
	
	float			MouseX;
	float			MouseY;
	float			MouseDX;
	float			MouseDY;
	bool			MouseDown[3];
	bool			MouseStroke[3];
	bool			MouseRelease[3];
	LGL_Timer		MouseTimer[LGL_KEY_MAX];

	//MultiTouch

	int			MultiTouchID;
	float			MultiTouchX;
	float			MultiTouchY;
	float			MultiTouchDX;
	float			MultiTouchDY;
	float			MultiTouchXFirst;
	float			MultiTouchYFirst;
	float			MultiTouchRotate;
	float			MultiTouchPinch;
	int			MultiTouchFingerCount;
	int			MultiTouchFingerCountDelta;

	//Joystick
	
	int			JoyNumber;
	SDL_Joystick*		Joystick[4];
	char			JoystickName[4][128];
	bool			JoyDown[4][32];
	bool			JoyStroke[4][32];
	bool			JoyRelease[4][32];
	float			JoyAnalogue[4][2][2];
	bool			JoyDuality[2];

	//Wiimotes

	LGL_Wiimote		Wiimote[8];
	LGL_Semaphore*		WiimoteSemaphore;

	//MIDI

	RtMidiIn*		MidiRtIn;
	std::vector<RtMidiIn*>	MidiRtInDevice;
	std::vector<char*>	MidiRtInDeviceNames;

	//M-Audio MIDI Devices

	LGL_MidiDeviceXponent*	Xponent;
	LGL_MidiDevice*		Xsession;
	LGL_MidiDevice*		TriggerFinger;
	LGL_MidiDevice*		JP8k;

	//VidCam
	
	bool			VidCamAvailable;
	char			VidCamName[128];
	int			VidCamWidthMin;
	int			VidCamHeightMin;
	int			VidCamWidthMax;
	int			VidCamHeightMax;
	int			VidCamWidthNow;
	int			VidCamHeightNow;
	unsigned char*		VidCamBufferRaw;
	unsigned char*		VidCamBufferProcessed;
	LGL_Image*		VidCamImageRaw;
	LGL_Image*		VidCamImageProcessed;
	int			VidCamFD;
#ifdef	LGL_LINUX_VIDCAM
	struct video_mmap	VidCamMMap;
#endif	//LGL_LINUX_VIDCAM
	float			VidCamCaptureDelay;
	LGL_Timer		VidCamCaptureDelayTimer;
	float			VidCamAxisXLast;
	float			VidCamAxisYLast;
	float			VidCamAxisXNext;
	float			VidCamAxisYNext;
	int			VidCamDistanceThumb;
	int			VidCamDistancePointer;
	LGL_Timer		VidCamDPadStrokeDelayTimerLeft;
	LGL_Timer		VidCamDPadStrokeDelayTimerRight;
	LGL_Timer		VidCamDPadStrokeDelayTimerDown;
	LGL_Timer		VidCamDPadStrokeDelayTimerUp;

	//Time
	
	int			FPS;
	int			FPSMax;
	int			FPSCounter;
	LGL_Timer		FPSTimer;
	float			FPSGraph[60];
	LGL_Timer		FPSGraphTimer;
	float			FrameTimeMin;
	float			FrameTimeAve;
	float			FrameTimeMax;
	int			FrameTimeGoodCount;
	int			FrameTimeMediumCount;
	int			FrameTimeBadCount;
	int			FrameTimeGoodTotal;
	int			FrameTimeMediumTotal;
	int			FrameTimeBadTotal;
	float			FrameTimeGraph[60];
	LGL_Font*		Font;
	char			TimeOfDay[1024];
	char			DateAndTimeOfDay[1024];
	char			DateAndTimeOfDayOfExecution[1024];

	//Net

	bool			NetAvailable;

	//LEDs

	std::vector<lgl_LEDHost*>
				ledHostList;

	//Misc

	bool			Running;
	void			(*UserExit)(void);
	
	double			SecondsSinceLastFrame;
	LGL_Timer		SecondsSinceLastFrameTimer;
	double			SecondsSinceExecution;	//FIXME: Hello drift. This should be an LGL_Timer.
	LGL_Timer		SecondsSinceExecutionTimer;
	unsigned long		FramesSinceExecution;

	bool			RecordMovie;
	int			RecordMovieFPS;
	char			RecordMovieFileNamePrefix[1024];
	double			RecordMovieSecondsSinceExecutionInitial;

	FILE*			DrawLogFD;
	char			DrawLogFileName[1024];
	bool			DrawLogPause;
	std::vector<char*>	DrawLog;
	std::vector<char*>	DrawLogData;
	std::vector<long>	DrawLogDataLength;
	float			DrawLogTimeOfNextDrawnFrame;

	std::vector<char*>	DebugPrintfBuffer;
	float			DebugPrintfY;
	LGL_Semaphore*		DebugPrintfSemaphore;

	float			LastImageDrawAsLineLeftX;
	float			LastImageDrawAsLineLeftY;
	float			LastImageDrawAsLineRightX;
	float			LastImageDrawAsLineRightY;

	int			TexturePixels;
	bool			FrameBufferTextureGlitchFix;

	char			HomeDir[2048];
	char			Username[2048];

	std::vector<lgl_WriteFileAsyncWorkItem*>
				WriteFileAsyncWorkItemList;
	int			WriteFileAsyncWorkItemListSize;
	std::vector<lgl_WriteFileAsyncWorkItem*>
				WriteFileAsyncWorkItemListNew;
	int			WriteFileAsyncWorkItemListNewSize;
	LGL_Semaphore*		WriteFileAsyncWorkItemListNewSemaphore;
	SDL_Thread*		WriteFileAsyncThread;
	
	//lgl_PathIsAliasCacher	PathIsAliasCacher;
} LGL_State;

LGL_State LGL;



#if LGL_OBJECT

LGL_Object::
LGL_Object() :
	RetainCount(0)
{
	RetainCountSemaphore=new LGL_Semaphore("RetainCountSemaphore");
}

LGL_Object::
~LGL_Object()
{
	{
		LGL_ScopeLock retainCountLock(__FILE__,__LINE__,RetainCountSemaphore);
		if(int rc=GetRetainCount())
		{
			LGL_Assertf
			(
				rc==0,
				("LGL_Object destroyed with RetainCount=%i!",rc)
			);
		}
	}

	delete RetainCountSemaphore;
	RetainCountSemaphore=NULL;
}

int
LGL_Object::
GetRetainCount()
{
	return(RetainCount);
}

LGL_Semaphore*
LGL_Object::
GetRetainCountSemaphore()
{
	return(RetainCountSemaphore);
}

void
LGL_Object::
Retain()
{
	LGL_ScopeLock retainCountLock(__FILE__,__LINE__,RetainCountSemaphore);
	RetainCount++;
}

void
LGL_Object::
Release()
{
	LGL_ScopeLock retainCountLock(__FILE__,__LINE__,RetainCountSemaphore);
	RetainCount--;
}


template<class T>
LGL_ObjectSP<T>::
LGL_ObjectSP
(
	LGL_Object*	obj
) :
	Object(obj)
{
	RetainObject();
}

template<class T>
LGL_ObjectSP<T>::
~LGL_ObjectSP()
{
	ReleaseObject();
}

template<class T>
T*
LGL_ObjectSP<T>::
cast()
{
	return(Object);
}

void
LGL_ObjectSP<T>::
RetainObject()
{
	if(Object)
	{
		Object->Retain();
	}
}

void
LGL_ObjectSP<T>::
ReleaseObject()
{
	if(Object)
	{
		Object->Release();
		{
			LGL_ScopeLock retainCountLock(__FILE__,__LINE__,Object->GetRetainCountSemaphore());
			if(Object->GetRetainCount()==0)
			{
				delete Object;
			}
		}
		Object=NULL;
	}
}

#endif



#include <jack/jack.h>
#include <jack/statistics.h>
#include <jack/thread.h>

jack_port_t *jack_output_port_fl=NULL;
jack_port_t *jack_output_port_fr=NULL;
jack_port_t *jack_output_port_bl=NULL;
jack_port_t *jack_output_port_br=NULL;
jack_port_t *jack_input_port_l=NULL;
jack_port_t *jack_input_port_r=NULL;
jack_client_t *jack_client=NULL;

Uint8 jack_output_buffer8[1024*64];
Sint16* jack_output_buffer16=(Sint16*)jack_output_buffer8;

Uint8 jack_input_buffer8[1024*64];
Sint16* jack_input_buffer16=(Sint16*)jack_input_buffer8;

int
lgl_AudioOutCallbackJackXrun
(
	void*	baka
)
{
	double delayedUsecs=jack_get_xrun_delayed_usecs(jack_client);
	if(delayedUsecs>0)
	{
		printf("[%.2f] Jack xrun... %.5fms\n",LGL_SecondsSinceExecution(),delayedUsecs/1000.0f);
	}
	LGL.AudioJackXrunBack=true;
	return(0);
}

Uint8 lgl_AudioInCallbackJackBuf[4096];
int lgl_AudioInCallbackJackBufPos=0;

int lgl_AudioInCallbackJack(void *udata, Uint8 *stream, int len8)
{
	if(LGL.AudioAvailable==false) return(0);

	if(len8==2048)
	{
		lgl_AudioInCallback(NULL,stream,len8);
		return(0);
	}
	else if(len8<2048)
	{
		memcpy(&(lgl_AudioInCallbackJackBuf[lgl_AudioInCallbackJackBufPos]),stream,len8);
		lgl_AudioInCallbackJackBufPos+=len8;
		if(lgl_AudioInCallbackJackBufPos>=2048)
		{
			lgl_AudioInCallback(NULL,lgl_AudioInCallbackJackBuf,2048);
			int remain = lgl_AudioInCallbackJackBufPos-2048;
			if(remain>0)
			{
				remain = LGL_Min(remain,2048);
				memcpy(lgl_AudioInCallbackJackBuf,&(lgl_AudioInCallbackJackBuf[2048]),remain);
			}
			lgl_AudioInCallbackJackBufPos=0;
		}
	}
	else
	{
		assert(false);
	}

	return(0);
}

int
lgl_AudioOutCallbackJack
(
	jack_nframes_t	nframes,
	void*		arg
)
{
	if(LGL.AudioAvailable==false) return(0);
	lgl_AudioOutCallback(NULL,jack_output_buffer8,nframes*2*LGL.AudioSpec->channels);

	if(LGL.AudioSpec->channels>=2)
	{
		jack_default_audio_sample_t* out_fl = (jack_default_audio_sample_t*)jack_port_get_buffer(jack_output_port_fl,nframes);
		jack_default_audio_sample_t* out_fr = (jack_default_audio_sample_t*)jack_port_get_buffer(jack_output_port_fr,nframes);
		assert(out_fl);
		assert(out_fr);
		for(unsigned int a=0;a<nframes;a++)
		{
			out_fl[a]=jack_output_buffer16[a*LGL.AudioSpec->channels+0]/(float)((1<<16)-1);
			out_fr[a]=jack_output_buffer16[a*LGL.AudioSpec->channels+1]/(float)((1<<16)-1);
		}
	}
	if(LGL.AudioSpec->channels==4)
	{
		jack_default_audio_sample_t* out_bl = (jack_default_audio_sample_t*)jack_port_get_buffer(jack_output_port_bl,nframes);
		jack_default_audio_sample_t* out_br = (jack_default_audio_sample_t*)jack_port_get_buffer(jack_output_port_br,nframes);
		assert(out_bl);
		assert(out_br);
		for(unsigned int a=0;a<nframes;a++)
		{
			out_bl[a]=jack_output_buffer16[a*LGL.AudioSpec->channels+2]/(float)((1<<16)-1);
			out_br[a]=jack_output_buffer16[a*LGL.AudioSpec->channels+3]/(float)((1<<16)-1);
		}
	}

	if
	(
		jack_input_port_l &&
		jack_input_port_r
	)
	{
		jack_default_audio_sample_t* in_l = (jack_default_audio_sample_t*)jack_port_get_buffer(jack_input_port_l,nframes);
		jack_default_audio_sample_t* in_r = (jack_default_audio_sample_t*)jack_port_get_buffer(jack_input_port_r,nframes);
		assert(in_l);
		assert(in_r);
		for(unsigned int a=0;a<nframes;a++)
		{
			jack_input_buffer16[a*2+0] = (Sint16)(in_l[a]*((1<<16)-1));
			jack_input_buffer16[a*2+1] = (Sint16)(in_r[a]*((1<<16)-1));
			if(LGL.AudioInPassThru)
			{
				jack_default_audio_sample_t* out_fl = (jack_default_audio_sample_t*)jack_port_get_buffer(jack_output_port_fl,nframes);
				jack_default_audio_sample_t* out_fr = (jack_default_audio_sample_t*)jack_port_get_buffer(jack_output_port_fr,nframes);
				assert(out_fl);
				assert(out_fr);
				out_fl[a]+=in_l[a];
				out_fr[a]+=in_r[a];
				if(LGL.AudioSpec->channels==4)
				{
					jack_default_audio_sample_t* out_bl = (jack_default_audio_sample_t*)jack_port_get_buffer(jack_output_port_bl,nframes);
					jack_default_audio_sample_t* out_br = (jack_default_audio_sample_t*)jack_port_get_buffer(jack_output_port_br,nframes);
					out_bl[a]+=in_l[a];
					out_br[a]+=in_r[a];
				}
			}
		}
		lgl_AudioInCallbackJack(NULL,jack_input_buffer8,nframes*2*2);
	}

	return(0);
}

void
lgl_AudioShutdownCallbackJack
(
	void*		arg
)
{
	printf("lgl_AudioShutdownCallbackJack(): Dropped by JACK! Did it crash...?\n");
	LGL.AudioAvailable=false;
}

void lgl_ClearAudioChannel
(
	int a,
	LGL_Sound* requiredPtr=NULL
)
{
	if
	(
		requiredPtr==NULL ||
		requiredPtr==LGL.SoundChannel[a].LGLSound
	)
	{
		LGL.SoundChannel[a].ClearMe=true;
	}
}

void lgl_ClearAudioChannelNow
(
	int a
)
{
	LGL.SoundChannel[a].ClearMe=false;
	LGL.SoundChannel[a].Paused=false;
	LGL.SoundChannel[a].PositionSamplesStart=0;
	LGL.SoundChannel[a].PositionSamplesPrev=0;
	LGL.SoundChannel[a].PositionSamplesNow=0;
	LGL.SoundChannel[a].PositionSamplesNowOutwards=0;
	LGL.SoundChannel[a].FuturePositionSamplesPrev=-1;
	LGL.SoundChannel[a].FuturePositionSamplesNow=-1;
	LGL.SoundChannel[a].PositionSamplesNowLastReported=0;
	LGL.SoundChannel[a].PositionSamplesEnd=-1;
	LGL.SoundChannel[a].PositionSamplesDeltaLastTime.Reset();
	LGL.SoundChannel[a].DivergeCount=0;
	LGL.SoundChannel[a].DivergeSamples=0;
	LGL.SoundChannel[a].DivergeSpeed=1.0f;
	LGL.SoundChannel[a].DivergeRecallNow=false;
	LGL.SoundChannel[a].DivergeRecallSamples=0.0f;
	LGL.SoundChannel[a].WarpPointSecondsAlpha=-1.0f;
	LGL.SoundChannel[a].WarpPointSecondsOmega=-1.0f;
	LGL.SoundChannel[a].WarpPointLoop=false;
	LGL.SoundChannel[a].WarpPointLock=false;
	LGL.SoundChannel[a].SampleRateConverterBufferStartSamples=0;
	for(int b=0;b<4;b++)
	{
		if(LGL.SoundChannel[a].SampleRateConverter[b])
		{
			src_reset(LGL.SoundChannel[a].SampleRateConverter[b]);
		}
		else
		{
			int error=0;
			LGL.SoundChannel[a].SampleRateConverter[b] = src_new(SRC_SINC_FASTEST,1,&error);
			assert(LGL.SoundChannel[a].SampleRateConverter[b]);
		}
	}
	LGL.SoundChannel[a].VolumeFrontLeftDesired=1.0f;
	LGL.SoundChannel[a].VolumeFrontRightDesired=1.0f;
	LGL.SoundChannel[a].VolumeBackLeftDesired=1.0f;
	LGL.SoundChannel[a].VolumeBackRightDesired=1.0f;
	LGL.SoundChannel[a].VolumeFrontLeft=1.0f;
	LGL.SoundChannel[a].VolumeFrontRight=1.0f;
	LGL.SoundChannel[a].VolumeBackLeft=1.0f;
	LGL.SoundChannel[a].VolumeBackRight=1.0f;
	LGL.SoundChannel[a].Channels=2;
	LGL.SoundChannel[a].Hz=44100;
	LGL.SoundChannel[a].ToMono=false;
	LGL.SoundChannel[a].SpeedNow=1;
	LGL.SoundChannel[a].SpeedDesired=1;
	LGL.SoundChannel[a].SpeedInterpolationFactor=1;
	LGL.SoundChannel[a].SpeedVolumeFactor=1;
	LGL.SoundChannel[a].Glitch=false;
	LGL.SoundChannel[a].FutureGlitchSettingsAvailable=false;
	LGL.SoundChannel[a].GlitchVolume=0;
	LGL.SoundChannel[a].GlitchSpeedNow=1;
	LGL.SoundChannel[a].GlitchSpeedDesired=1;
	LGL.SoundChannel[a].GlitchSpeedInterpolationFactor=1;
	LGL.SoundChannel[a].GlitchDuo=0;
	LGL.SoundChannel[a].GlitchLuminScratch=false;
	LGL.SoundChannel[a].GlitchLuminScratchPositionDesired=-10000;
	LGL.SoundChannel[a].FutureGlitchSamplesNow=-10000;
	LGL.SoundChannel[a].RhythmicInvertAlphaSamples=0;
	LGL.SoundChannel[a].RhythmicInvertDeltaSamples=0;
	LGL.SoundChannel[a].RhythmicVolumeInvert=false;
	LGL.SoundChannel[a].RespondToRhythmicSoloInvertChannel=-1;
	LGL.SoundChannel[a].RespondToRhythmicSoloInvertCurrentValue=1;
	LGL.SoundChannel[a].GlitchSamplesNow=0;
	LGL.SoundChannel[a].GlitchLast=0;
	LGL.SoundChannel[a].GlitchBegin=0;
	LGL.SoundChannel[a].GlitchLength=0;
	LGL.SoundChannel[a].Loop=false;
	LGL.SoundChannel[a].StickyEndpoints=false;
	LGL.SoundChannel[a].LengthSamples=0;
	LGL.SoundChannel[a].BufferLength=0;
	LGL.SoundChannel[a].Buffer=NULL;
	LGL.SoundChannel[a].BufferSemaphore=NULL;
	LGL.SoundChannel[a].LGLSound=NULL;
	for(int b=0;b<2;b++)
	{
		if(LGL.SoundChannel[a].LGLAudioDSP[b]!=NULL)
		{
			delete LGL.SoundChannel[a].LGLAudioDSP[b];
			LGL.SoundChannel[a].LGLAudioDSP[b]=NULL;
		}
	}
	LGL.SoundChannel[a].Occupied=false;
}

double myH[1024];
double myZ[1024];
int myPstate;

//#define	GL_TEXTURE_LGL (GL_TEXTURE_2D)
#define	GL_TEXTURE_LGL (GL_TEXTURE_RECTANGLE_ARB)

//GL2 Function Pointer Mapping

typedef void (*glShaderSourceARB_Func)
(
	GLhandleARB	shader,
	GLsizei		nstrings,
const	GLcharARB**	strings,
const	GLint*		lengths
);
glShaderSourceARB_Func gl2ShaderSource=NULL;

typedef GLhandleARB (*glCreateShaderObjectARB_Func)
(
	GLenum		shaderType
);
glCreateShaderObjectARB_Func gl2CreateShaderObject=NULL;

typedef void (*glCompileShaderARB_Func)
(
	GLhandleARB	shader
);
glCompileShaderARB_Func gl2CompileShader=NULL;

typedef void (*glGetObjectParameterivARB_Func)
(
	GLhandleARB	object,
	GLenum		pname,
	GLint*		params
);
glGetObjectParameterivARB_Func gl2GetObjectParameteriv=NULL;

typedef void (*glGetInfoLogARB_Func)
(
	GLhandleARB	object,
	GLsizei		maxLength,
	GLsizei*	length,
	GLcharARB*	infoLog
);
glGetInfoLogARB_Func gl2GetInfoLog=NULL;

typedef GLhandleARB (*glCreateProgramObjectARB_Func)
(
	void
);
glCreateProgramObjectARB_Func gl2CreateProgramObject=NULL;

typedef void (*glAttachObjectARB_Func)
(
	GLhandleARB	shader,
	GLhandleARB	program
);
glAttachObjectARB_Func gl2AttachObject=NULL;

typedef void (*glLinkProgramARB_Func)
(
	GLhandleARB	program
);
glLinkProgramARB_Func gl2LinkProgram=NULL;

typedef void (*glValidateProgramARB_Func)
(
	GLhandleARB	program
);
glValidateProgramARB_Func gl2ValidateProgram=NULL;

typedef void (*glUseProgramObjectARB_Func)
(
	GLhandleARB	program
);
glUseProgramObjectARB_Func gl2UseProgramObject=NULL;

typedef void (*glDeleteObjectARB_Func)
(
	GLhandleARB	object
);
glDeleteObjectARB_Func gl2DeleteObject=NULL;

typedef void (*glGetShaderSourceARB_Func)
(
	GLhandleARB	shader,
	GLsizei		maxLength,
	GLsizei*	length,
	GLcharARB*	source
);
glGetShaderSourceARB_Func gl2GetShaderSource=NULL;

typedef GLint (*glGetAttribLocationARB_Func)
(
	GLhandleARB		program,
const	GLcharARB*		name
);
glGetAttribLocationARB_Func gl2GetAttribLocation=NULL;

typedef GLint (*glGetActiveAttribARB_Func)
(
	GLhandleARB		program,
	GLuint			index,
	GLsizei			maxLength,
	GLsizei*		length,
	GLint*			size,
	GLenum*			type,
	GLcharARB*		name
);
glGetActiveAttribARB_Func gl2GetActiveAttrib=NULL;

typedef void (*glVertexAttrib4ivARB_Func)
(
	GLuint			index,
const	int*			v
);
glVertexAttrib4ivARB_Func gl2VertexAttrib4iv=NULL;

typedef void (*glVertexAttrib4fvARB_Func)
(
	GLuint			index,
const	float*			v
);
glVertexAttrib4fvARB_Func gl2VertexAttrib4fv=NULL;

typedef GLint (*glGetUniformLocationARB_Func)
(
	GLhandleARB		program,
const	GLcharARB*		name
);
glGetUniformLocationARB_Func gl2GetUniformLocation=NULL;

typedef void (*glUniform1iARB_Func)
(
	GLuint			index,
	int			arg0
);
glUniform1iARB_Func gl2Uniform1i=NULL;

typedef void (*glUniform2iARB_Func)
(
	GLuint			index,
	int			arg0,
	int			arg1
);
glUniform2iARB_Func gl2Uniform2i=NULL;

typedef void (*glUniform3iARB_Func)
(
	GLuint			index,
	int			arg0,
	int			arg1,
	int			arg2
);
glUniform3iARB_Func gl2Uniform3i=NULL;

typedef void (*glUniform4iARB_Func)
(
	GLuint			index,
	int			arg0,
	int			arg1,
	int			arg2,
	int			arg3
);
glUniform4iARB_Func gl2Uniform4i=NULL;

typedef void (*glUniform1fARB_Func)
(
	GLuint			index,
	float			arg0
);
glUniform1fARB_Func gl2Uniform1f=NULL;

typedef void (*glUniform2fARB_Func)
(
	GLuint			index,
	float			arg0,
	float			arg1
);
glUniform2fARB_Func gl2Uniform2f=NULL;

typedef void (*glUniform3fARB_Func)
(
	GLuint			index,
	float			arg0,
	float			arg1,
	float			arg2
);
glUniform3fARB_Func gl2Uniform3f=NULL;

typedef void (*glUniform4fARB_Func)
(
	GLuint			index,
	float			arg0,
	float			arg1,
	float			arg2,
	float			arg3
);
glUniform4fARB_Func gl2Uniform4f=NULL;

typedef void (*gl2GenBuffers_Func)
(
	GLsizei			n,
	GLuint*			buffers
);
gl2GenBuffers_Func gl2GenBuffers=NULL;

typedef bool (*gl2IsBuffer_Func)
(
	GLenum			target
);
gl2IsBuffer_Func gl2IsBuffer=NULL;

typedef void (*gl2BindBuffer_Func)
(
	GLenum			target,
	GLuint			buffer
);
gl2BindBuffer_Func gl2BindBuffer=NULL;

typedef void (*gl2BufferData_Func)
(
	GLenum			target,
	GLsizeiptr		size,
	const GLvoid*		data,
	GLenum			usage
);
gl2BufferData_Func gl2BufferData=NULL;

typedef void* (*gl2MapBuffer_Func)
(
	GLenum			target,
	GLenum			access
);
gl2MapBuffer_Func gl2MapBuffer=NULL;

typedef bool (*gl2UnmapBuffer_Func)
(
	GLenum			target
);
gl2UnmapBuffer_Func gl2UnmapBuffer=NULL;

typedef void (*gl2DeleteBuffers_Func)
(
	GLsizei			n,
	GLuint*			buffers
);
gl2DeleteBuffers_Func gl2DeleteBuffers=NULL;

typedef void (*gl2ActiveTexture_Func)
(
	GLenum			texture
);
gl2ActiveTexture_Func gl2ActiveTexture=NULL;

typedef void (*gl2MultiTexCoord2f_Func)
(
	GLenum			target,
	GLfloat			s,
	GLfloat			t
);
gl2MultiTexCoord2f_Func gl2MultiTexCoord2f=NULL;

typedef void (*gl2MultiTexCoord2d_Func)
(
	GLenum			target,
	GLfloat			s,
	GLfloat			t
);
gl2MultiTexCoord2d_Func gl2MultiTexCoord2d=NULL;

bool
LGL_JackInit()
{
	//Kill other instances of jack
	{
		char cmd[2048];
		strcpy(cmd,"killall -9 jackd");
		system(cmd);
	}

	setenv("JACK_DRIVER_DIR","./lib/jack",1);

	LGL.AudioSpec->silence=0;
	jack_options_t jack_options = JackNullOption;
	jack_status_t status;
	bzero(&status,sizeof(jack_status_t));

	//Setup environment
	if(LGL_IsOsxAppBundle())
	{
		//PATH must be supplemented so jack_client_init() finds jackd
		char path[2048];
		sprintf(path,"%s:./",getenv("PATH"));
		setenv("PATH",path,1);

		//Our JACK drivers live inside our App Bundle
		setenv("JACK_DRIVER_DIR","lib/jack",1);
	}

	//Open a client connection to the JACK server
	const char* client_name = "dvj";
	printf("LGL_JackInit(): JACK server starting...\n\n");
	jack_client=jack_client_open(client_name,jack_options,&status);
	printf("\n");
	if(jack_client==NULL)
	{
		printf("LGL_JackInit(): Error! jack_client_open() failed! status = 0x%.2x\n",status);
		char dotJackdrcPath[2048];
		sprintf(dotJackdrcPath,"%s/.jackdrc",LGL_GetHomeDir());
		if(FILE* fd = fopen(dotJackdrcPath,"r"))
		{
			const int bufLen=2048;
			char buf[bufLen];
			fgets(buf,bufLen,fd);
			buf[strlen(buf)-1]='\0';	//Get rid of '\n'
			printf("LGL_JackInit(): ~/.jackdrc:\n");
			printf("\t%s\n",buf);
			/*
			strcat(buf," > /tmp/jack.fail 2>&1");
			printf("Generating /tmp/jack.fail: Alpha\n");
			printf("\t%s\n",buf);
			system(buf);
			printf("Generating /tmp/jack.fail: Omega\n");
			*/
			fclose(fd);
		}
		return(false);
	}
	if(status & JackServerStarted)
	{
		printf("LGL_JackInit(): JACK server started...\n");
	}
	if(status & JackNameNotUnique)
	{
		client_name = jack_get_client_name(jack_client);
		printf("LGL_JackInit(): Unique name '%s' assigned...\n",client_name);
	}

	if(jack_get_sample_rate(jack_client)==0)
	{
		printf("LG_JackInit(): JACK init failed (Sample rate is zero)...\n");
		jack_client_close(jack_client);
		return(false);
	}

	//Tell JACK about our main callback
	jack_set_process_callback(jack_client,lgl_AudioOutCallbackJack,0);

	//Tell JACK about our xrun callback
	jack_set_xrun_callback(jack_client,lgl_AudioOutCallbackJackXrun,0);

	//Tell JACK about our callback if JACK ever quits or drops us
	jack_on_shutdown(jack_client,lgl_AudioShutdownCallbackJack,0);

	printf("LGL_JackInit(): Sample Rate: %i\n",jack_get_sample_rate(jack_client));

	jack_output_port_fl = jack_port_register(jack_client,"output_fl",JACK_DEFAULT_AUDIO_TYPE,JackPortIsOutput,0);
	jack_output_port_fr = jack_port_register(jack_client,"output_fr",JACK_DEFAULT_AUDIO_TYPE,JackPortIsOutput,0);
	jack_output_port_bl = jack_port_register(jack_client,"output_bl",JACK_DEFAULT_AUDIO_TYPE,JackPortIsOutput,0);
	jack_output_port_br = jack_port_register(jack_client,"output_br",JACK_DEFAULT_AUDIO_TYPE,JackPortIsOutput,0);
	jack_input_port_l = jack_port_register(jack_client,"input_l",JACK_DEFAULT_AUDIO_TYPE,JackPortIsInput,0);
	jack_input_port_r = jack_port_register(jack_client,"input_r",JACK_DEFAULT_AUDIO_TYPE,JackPortIsInput,0);

	if
	(
		jack_output_port_fl==NULL ||
		jack_output_port_fr==NULL
	)
	{
		printf("LGL_JackInit(): Error! Couldn't acquire JACK port(s)!\n");
		return(false);
	}

	LGL_SAMPLESIZE_SDL = jack_get_buffer_size(jack_client);

	LGL.AudioAvailable=false;

	//Fire it up!
	if(jack_activate(jack_client)!=0)
	{
		printf("LGL_JackInit(): Error! jack_activate() failed!\n");
		return(false);
	}

	//LGL_SAMPLESIZE_SDL=512;
	//jack_set_buffer_size(jack_client,LGL_SAMPLESIZE_SDL);

	//AudioIn
	LGL.AudioInAvailable=false;
	LGL.AudioInPassThru=false;
	const char** jack_ports_out = jack_get_ports(jack_client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput);
	int portCountOut=0;
	if(jack_ports_out)
	{
		while(jack_ports_out[portCountOut]!=NULL)
		{
			printf("LGL_JackInit(): Audio In found '%s'\n",jack_ports_out[portCountOut]);
			portCountOut++;
			if(portCountOut==2)
			{
				//We don't care to have > 2 channels
				break;
			}
		}
		for(int a=0;a<portCountOut;a++)
		{
			jack_port_t* whichPort;
			if(a==0)	whichPort=jack_input_port_l;
			else if(a==1)	whichPort=jack_input_port_r;
			else break;
			if(jack_connect(jack_client,jack_ports_out[a],jack_port_name(whichPort))!=0)
			{
				printf("LGL_JackInit(): Warning! Cannot connect to input port %i!\n",a);
				break;
			}
			if(a==1)
			{
				LGL.AudioInAvailable=true;
			}
		}
	}
	else
	{
		printf("jack_get_ports() for jack_ports_out failed!\n");
		jack_input_port_l=NULL;
		jack_input_port_r=NULL;
	}

	const char** jack_ports_in = jack_get_ports(jack_client, NULL, NULL, JackPortIsPhysical|JackPortIsInput);
	if(jack_ports_in==NULL)
	{
		printf("LGL_JackInit(): Error! No physical playback ports!\n");
		return(false);
	}
	LGL.AudioSpec->channels=0;
	int portCountIn=0;
	while(jack_ports_in[portCountIn]!=NULL)
	{
		portCountIn++;
		if(portCountIn==4)
		{
			//We don't care to have > 4 channels
			break;
		}
	}
	for(int a=0;a<portCountIn;a++)
	{
		jack_port_t* whichPort;
		if(a==0)	whichPort=jack_output_port_fl;
		else if(a==1)	whichPort=jack_output_port_fr;
		else if(a==2)	whichPort=jack_output_port_bl;
		else if(a==3)	whichPort=jack_output_port_br;
		else break;
		if(jack_connect(jack_client,jack_port_name(whichPort),jack_ports_in[a])!=0)
		{
			printf("LGL_JackInit(): Error! Cannot connect to output port %i!\n",a);
			return(false);
		}
		LGL.AudioSpec->channels=a+1;
	}
	if(LGL.AudioSpec->channels==0)
	{
		printf("LGL_JackInit(): Error! Cannot connect to any output port!\n");
		return(false);
	}

	LGL.AudioAvailable=true;
	LGL.AudioWasOnceAvailable=true;
	LGL.AudioOutDisconnected=false;
	LGL.AudioSpec->freq = jack_get_sample_rate(jack_client);

	//MEMLEAK: if we return early, these calls to free() don't happen. Meh.
	free(jack_ports_in);
	free(jack_ports_out);

	printf("LGL_JackInit(): Success! (%i channels) (Input: %s)\n",LGL.AudioSpec->channels,LGL.AudioInAvailable ? "Present" : "Absent");

	return(true);
}

int lgl_MidiInit2();

LGL_Semaphore&
lgl_get_av_semaphore()
{
	static LGL_Semaphore sem("av_semaphore",true);	//Promiscuous?!
	return(sem);
}

void
lgl_av_register_all()
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	av_register_all();
}

void
lgl_avcodec_init()
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	avcodec_init();
}

void
lgl_avcodec_register_all()
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	avcodec_register_all();
}

void
lgl_av_log_set_level(int level)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	av_log_set_level(level);
}

int
lgl_av_open_input_file
(
	AVFormatContext**	ic_ptr,
	const char*		filename,
	AVInputFormat*		fmt,
	int			buf_size,
	AVFormatParameters*	ap
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	//LGL_ScopeLock avOpenCloseLock(__FILE__,__LINE__,LGL.AVOpenCloseSemaphore);
	return
	(
		av_open_input_file
		(
			ic_ptr,
			filename,
			fmt,
			buf_size,
			ap
		)
	);
}

void
lgl_av_close_input_file(AVFormatContext* fc)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	//LGL_ScopeLock avOpenCloseLock(__FILE__,__LINE__,LGL.AVOpenCloseSemaphore);
	av_close_input_file(fc);
}

int
lgl_av_find_stream_info
(
	AVFormatContext*	fc
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	LGL_ScopeLock avOpenCloseLock(__FILE__,__LINE__,LGL.AVOpenCloseSemaphore);
	return(av_find_stream_info(fc));
}

AVCodec*
lgl_avcodec_find_decoder
(
	enum CodecID	id
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	return(avcodec_find_decoder(id));
}

int
lgl_avcodec_open
(
	AVCodecContext*	cc,
	AVCodec*	c
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	LGL_ScopeLock avOpenCloseLock(__FILE__,__LINE__,LGL.AVOpenCloseSemaphore);
	return(avcodec_open(cc,c));
}

int
lgl_avcodec_close
(
	AVCodecContext*	cc
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	LGL_ScopeLock avOpenCloseLock(__FILE__,__LINE__,LGL.AVOpenCloseSemaphore);
	return(avcodec_close(cc));
}


int
lgl_url_fopen
(
	ByteIOContext**	s,
	const char*	url,
	int		flags
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	LGL_ScopeLock avOpenCloseLock(__FILE__,__LINE__,LGL.AVOpenCloseSemaphore);
	return
	(
		url_fopen
		(
			s,
			url,
			flags
		)
	);
}

int
lgl_url_fclose
(
	ByteIOContext*	s
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	LGL_ScopeLock avOpenCloseLock(__FILE__,__LINE__,LGL.AVOpenCloseSemaphore);
	return
	(
		url_fclose(s)
	);
}

AVFrame*
lgl_avcodec_alloc_frame()
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	return(avcodec_alloc_frame());
}

int
lgl_av_seek_frame
(
	AVFormatContext*	s,
	int			stream_index,
	int64_t			timestamp,
	int			flags
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	static LGL_Semaphore localSem("lgl_av_seek_frame");
	LGL_ScopeLock localLock(__FILE__,__LINE__,localSem);
	return
	(
		av_seek_frame
		(
			s,
			stream_index,
			timestamp,
			flags
		)
	);
}

void
lgl_av_init_packet
(
	AVPacket*	pkt
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	av_init_packet(pkt);
}

int
lgl_av_read_frame
(
	AVFormatContext*	fc,
	AVPacket*		pkt
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	return(av_read_frame(fc,pkt));
}

int
lgl_avcodec_decode_video2
(
	AVCodecContext*	avctx,
	AVFrame*	picture,
	int*		got_picture_ptr,
	AVPacket*	avpkt
)
{
	//LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	static LGL_Semaphore localSem("lgl_av_decode_video2");
	LGL_ScopeLock localLock(__FILE__,__LINE__,localSem);
	return
	(
		avcodec_decode_video2
		(
			avctx,
			picture,
			got_picture_ptr,
			avpkt
		)
	);
}

AVCodec*
lgl_avcodec_find_encoder
(
	enum CodecID	id
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	AVCodec* codec = avcodec_find_encoder(id);
	return(codec);
}

AVCodec*
lgl_avcodec_find_encoder_by_name
(
	const char*	name
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	return(avcodec_find_encoder_by_name(name));
}

void
lgl_av_free_packet
(
	AVPacket*	pkt
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	av_free_packet(pkt);
}

void*
lgl_av_mallocz
(
	unsigned int size
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	return(av_mallocz(size));
}

AVStream*
lgl_av_new_stream
(
	AVFormatContext*	fc,
	int			id
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	return(av_new_stream(fc,id));
}

void
lgl_avcodec_get_context_defaults
(
	AVCodecContext*	cc
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	avcodec_get_context_defaults(cc);
}

int
lgl_av_write_header
(
	AVFormatContext*	fc
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	return(av_write_header(fc));
}

AVFormatContext*
lgl_avformat_alloc_context()
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	return(avformat_alloc_context());
}

void
lgl_av_freep
(
	void*	ptr
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	av_freep(ptr);
}

int
lgl_avcodec_encode_video
(
	AVCodecContext*	avctx,
	uint8_t*	buf,
	int		buf_size,
const	AVFrame*	pict
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	return
	(
		avcodec_encode_video
		(
			avctx,
			buf,
			buf_size,
			pict
		)
	);
}

int
lgl_av_write_frame
(
	AVFormatContext*	s,
	AVPacket*		pkt
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	return(av_write_frame(s,pkt));
}

int
lgl_avcodec_decode_audio3
(
	AVCodecContext*	avctx,
	int16_t*	samples,
	int*		frame_size_ptr,
	AVPacket*	avpkt
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	return
	(
		avcodec_decode_audio3
		(
			avctx,
			samples,
			frame_size_ptr,
			avpkt
		)
	);
}

int
lgl_av_write_trailer
(
	AVFormatContext*	s
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	return(av_write_trailer(s));
}

int
lgl_avcodec_encode_audio
(
	AVCodecContext*	avctx,
	uint8_t*	buf,
	int		buf_size,
	const short*	samples
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,lgl_get_av_semaphore());
	return
	(
		avcodec_encode_audio
		(
			avctx,
			buf,
			buf_size,
			samples
		)
	);
}

int
lgl_sws_scale
(
	struct SwsContext*	context,
	const uint8_t* const	srcSlice[],
	const int		srcStride[],
	int			srcSliceY,
	int			srcSliceH,
	uint8_t* const		dst[],
	const int		dstStride[]
)
{
	//static LGL_Semaphore localSem("lgl_sws_scale");
	//LGL_ScopeLock localLock(__FILE__,__LINE__,localSem);

	return
	(
		sws_scale
		(
			context,
			srcSlice,
			srcStride,
			srcSliceY,
			srcSliceH,
			dst,
			dstStride
		)
	);
}

struct SwsContext*
lgl_sws_getContext
(
	int			srcW,
	int			srcH,
	enum PixelFormat	srcFormat,
	int			dstW,
	int			dstH,
	enum PixelFormat	dstFormat,
	int			flags,
	SwsFilter*		srcFilter,
	SwsFilter*		dstFilter,
	const double*		param
)
{
	//static LGL_Semaphore localSem("lgl_sws_scale");
	//LGL_ScopeLock localLock(__FILE__,__LINE__,localSem);

	return
	(
		sws_getContext
		(
			srcW,
			srcH,
			srcFormat,
			dstW,
			dstH,
			dstFormat,
			flags,
			srcFilter,
			dstFilter,
			param
		)
	);
}

bool
LGL_Init
(
	int		inWindowResolutionX,
	int		inWindowResolutionY,
	int		inAudioChannels,
	const char*	inWindowTitle
)
{
	if(inAudioChannels!=0)
	{
		inAudioChannels=4;
	}

	//Setup initial RT priorities
#ifdef	LGL_LINUX
#ifndef	LGL_OSX
	mlockall(MCL_CURRENT | MCL_FUTURE);
#endif	//LGL_OSX
#endif	//LGL_LINUX

	for(int a=0;a<1024;a++)
	{
		myH[a]=0;
		myZ[a]=0;
	}
	myPstate=0;
	//Initialize LGL_State

	srand((unsigned)time(NULL));
	
	LGL.HomeDir[0]='\0';
	LGL.Username[0]='\0';
	
	LGL_ViewportDisplay(0,1,0,1);
	LGL_ViewportWorld
	(
		1,1,1,
		0,0,0,
		0,0,1
	);

	LGL.AudioAvailable=false;
	LGL.AudioWasOnceAvailable=false;
	LGL.AudioQuitting=false;
	LGL.AudioUsingJack=true;
	LGL.AudioJackXrunBack=false;
	LGL.AudioJackXrunFront=false;

	int a;
	for(a=0;a<512;a++)
	{
		LGL.KeyDown[a]=false;
		LGL.KeyStroke[a]=false;
		LGL.KeyRelease[a]=false;
		LGL.KeyTimer[a].Reset();
	}
	for(a=0;a<256;a++)
	{
		LGL.KeyStream[a]='\0';
	}
	LGL.InputBufferFocus=NULL;
	LGL.Font=NULL;

	LGL.MouseX=.5;
	LGL.MouseY=.5;
	LGL.MouseDX=0;
	LGL.MouseDY=0;
	for(a=0;a<3;a++)
	{
		LGL.MouseDown[a]=false;
		LGL.MouseStroke[a]=false;
		LGL.MouseRelease[a]=false;
		LGL.MouseTimer[a].Reset();
	}

	LGL.MultiTouchID=0;
	LGL.MultiTouchX=-1.0f;
	LGL.MultiTouchY=-1.0f;
	LGL.MultiTouchDX=0.0f;
	LGL.MultiTouchDY=0.0f;
	LGL.MultiTouchXFirst=-1.0f;
	LGL.MultiTouchYFirst=-1.0f;
	LGL.MultiTouchRotate=0.0f;
	LGL.MultiTouchPinch=0.0f;
	LGL.MultiTouchFingerCount=0;
	LGL.MultiTouchFingerCountDelta=0;

	LGL.RecordMovie=false;
	LGL.RecordMovieFPS=60;
	sprintf(LGL.RecordMovieFileNamePrefix,"./");
	LGL.RecordMovieSecondsSinceExecutionInitial=0;

	LGL.DrawLogFD=0;
	LGL.DrawLogPause=false;
	LGL.DrawLogFileName[0]='\0';
	LGL.DrawLogTimeOfNextDrawnFrame=0.0f;

	LGL.DebugPrintfY=1.0f;
	LGL.DebugPrintfSemaphore=new LGL_Semaphore("Debug Printf");

	LGL.AudioSpec=(SDL_AudioSpec*)malloc(sizeof(SDL_AudioSpec));

	atexit(LGL_ShutDown);

	//Initialize SDL
	if
	(
		SDL_Init
		(
#ifndef	LGL_NO_GRAPHICS
			//SDL_INIT_VIDEO |
#endif	//LGL_NO_GRAPHICS
			SDL_INIT_AUDIO |
			SDL_INIT_TIMER |
			SDL_INIT_JOYSTICK
		) < 0
	)
	{
		printf
		(
			"LGL_Init(): SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK) failed... %s\n",
			SDL_GetError()
		);
		assert(false);
		LGL_Exit();
	}
	else
	{
		SDL_VideoInit(NULL,0);
		lgl_sdl_initialized=true;
		//printf("LGL: SDL_Init() Success!\n");
	}

#ifndef	LGL_NO_GRAPHICS
	LGL.DisplayCount=SDL_GetNumVideoDisplays();
	if(LGL.DisplayCount>1)
	{
		lgl_FakeSecondDisplay=0;
	}
	if(lgl_FakeSecondDisplay)
	{
		LGL.DisplayCount=2;
	}
	LGL.DisplayNow=0;

printf("%i displays!\n",LGL.DisplayCount);

	for(int d=0;d<LGL.DisplayCount;d++)
	{
		if(lgl_FakeSecondDisplay)
		{
			SDL_SelectVideoDisplay(0);
		}
		{
			SDL_SelectVideoDisplay(d);
		}
		SDL_DisplayMode mode;
		SDL_GetDesktopDisplayMode(&mode);

		LGL.DisplayResolutionX[d] = mode.w;
		LGL.DisplayResolutionY[d] = mode.h;
		if(lgl_FakeSecondDisplay3x && d==1)
		{
			LGL.DisplayResolutionX[d] = LGL.DisplayResolutionX[0];
			LGL.DisplayResolutionY[d] = LGL.DisplayResolutionY[0]/3;
		}
printf("\t[%i]: %i x %i\n",
	d,
	LGL.DisplayResolutionX[d],
	LGL.DisplayResolutionY[d]);
		LGL.DisplayRefreshRate[d] = mode.refresh_rate ? mode.refresh_rate : 60;

		SDL_DisplayMode displayMode;
		SDL_GetWindowDisplayMode(LGL.WindowID[d],&displayMode);
		displayMode.w=LGL.DisplayResolutionX[d];
		displayMode.h=LGL.DisplayResolutionY[d];
		displayMode.refresh_rate=60;
		SDL_SetWindowDisplayMode(LGL.WindowID[d],&displayMode);
	}
printf("\n");

	SDL_SelectVideoDisplay(0);

	if(lgl_FakeSecondDisplay)
	{
		if(inWindowResolutionX==9999)
		{
			inWindowResolutionX=LGL.DisplayResolutionX[0]/2;
			inWindowResolutionY=LGL.DisplayResolutionY[0]/2;
		}
	}

	//Determine WindowResolutions
	LGL.MasterWindowResolutionX=0;
	LGL.MasterWindowResolutionY=0;
	for(int d=0;d<LGL.DisplayCount;d++)
	{
		if(d==0 || lgl_FakeSecondDisplay)
		{
			if
			(
				inWindowResolutionX==9999 &&
				inWindowResolutionY==9999
			)
			{
				LGL.WindowResolutionX[d]=LGL_DisplayResolutionX()-100;
				LGL.WindowResolutionY[d]=LGL_DisplayResolutionY()-100;
			}
			else
			{
				LGL.WindowFullscreen=false;
				LGL.WindowResolutionX[d]=inWindowResolutionX;
				LGL.WindowResolutionY[d]=inWindowResolutionY;
			}
		}
		else
		{
			LGL.WindowResolutionX[d]=LGL.DisplayResolutionX[d];
			LGL.WindowResolutionY[d]=LGL.DisplayResolutionY[d];
		}
		
		if(lgl_FakeSecondDisplay3x)
		{
			if(d==0)
			{
				LGL.WindowResolutionX[d]=(LGL_DisplayResolutionX()-100)/2;
				LGL.WindowResolutionY[d]=(LGL_DisplayResolutionY()-100)/2;
			}
			else if(d==1)
			{
				LGL.WindowResolutionX[d]=LGL_DisplayResolutionX()-100;
				LGL.WindowResolutionY[d]=(LGL_DisplayResolutionY()-100)/2.25f;
			}
		}

		if(LGL.MasterWindowResolutionX<LGL.WindowResolutionX[d])
		{
			LGL.MasterWindowResolutionX=LGL.WindowResolutionX[d];
		}
		if(LGL.MasterWindowResolutionY<LGL.WindowResolutionY[d])
		{
			LGL.MasterWindowResolutionY=LGL.WindowResolutionY[d];
		}
	}

	for(int d=0;d<LGL.DisplayCount;d++)
	{
		LGL.DisplayViewportLeft[d]=0.0f;
		LGL.DisplayViewportRight[d]=LGL.WindowResolutionX[d]/(float)LGL.MasterWindowResolutionX;
		LGL.DisplayViewportBottom[d]=0.0f;
		LGL.DisplayViewportTop[d]=LGL.WindowResolutionY[d]/(float)LGL.MasterWindowResolutionY;
	}

#endif	//LGL_NO_GRAPHICS
	
	//Time

	LGL_ResetFPSGraph();
	LGL.Font=NULL;

	LGL.SecondsSinceLastFrame=0;
	LGL.SecondsSinceLastFrameTimer.Reset();
	LGL.SecondsSinceExecution=0;
	LGL.SecondsSinceExecutionTimer.Reset();
	LGL.FramesSinceExecution=0;

	time_t tim=time(NULL);
	struct tm TM=*localtime(&tim);
	int seconds=TM.tm_sec;
	int minutes=TM.tm_min;
	int hours=TM.tm_hour;
	int date=TM.tm_mday;
	int month=TM.tm_mon+1;
	int year=1900+TM.tm_year;
	sprintf
	(
		LGL.DateAndTimeOfDayOfExecution,
		"%04i.%02i.%02i - %02i:%02i.%02i",
		year,month,date,
		hours,minutes,seconds
	);

	//Audio

	LGL.AudioEncoderPath[0]='\0';
	LGL.AudioEncoder=NULL;
	LGL.AudioEncoderSemaphore=NULL;

	LGL.AudioSpec->freq=44100;
	LGL.AudioSpec->format=AUDIO_S16;
	LGL.AudioSpec->channels=inAudioChannels;
	LGL.AudioSpec->samples=LGL_SAMPLESIZE_SDL;
	LGL.AudioSpec->callback=lgl_AudioOutCallback;
	LGL.AudioSpec->userdata=malloc(LGL.AudioSpec->samples);
	LGL.RecordVolume=1.0f;
	LGL.RecordSamplesWritten=0;
	LGL.AudioMasterToHeadphones=false;
	
	char dspPath[1024];
	dspPath[0]='\0';
	char audioDriver[1024];
	audioDriver[0]='\0';

	for(int a=0;a<LGL_SOUND_CHANNEL_NUMBER;a++)
	{
		lgl_ClearAudioChannelNow(a);
	}
	LGL.AudioStreamListSemaphore=new LGL_Semaphore("Audio Stream List");
	LGL.AudioInSemaphore=new LGL_Semaphore("AudioIn");
	LGL.AudioInFallbackGrain=new LGL_AudioGrain;
	Uint8 zero=0;
	LGL.AudioInFallbackGrain->SetWaveformFromMemory(&zero,1);
	LGL.AudioBufferPos=0;

	for(int a=0;a<LGL_SOUND_CHANNEL_NUMBER;a++)
	{
		for(int b=0;b<4;b++)
		{
			LGL.SoundChannel[a].SampleRateConverter[b] = NULL;
		}
	}
	LGL.SoundChannel[a].SampleRateConverterBufferStartSamples=-1;
	LGL.SoundChannel[a].SampleRateConverterBufferValidSamples=-1;

	if(inAudioChannels>0)
	{
		bool pulserunning = false;
#ifdef	LGL_LINUX
#ifndef	LGL_OSX
		pulserunning=!system("pgrep pulseaudio > /dev/null");
#endif	//LGL_OSX
#endif	//LGL_LINUX

		/*
		if(1 || LGL.AudioUsingJack==false)
		{
			LGL_ThreadSetPriority(LGL_PRIORITY_AUDIO_OUT,"AudioOut / JACK");
		}
		*/
		printf("\n\nLGL JACK Initialization: ALPHA\n");
		printf("---\n");
		bool jackInitOK = LGL_JackInit();
		printf("---\n");
		printf("LGL JACK Initialization: OMEGA\n\n\n");
		if
		(
			pulserunning==false &&
			jackInitOK
		)
		{
			//Huzzah!
		}
		else
		{
			if(pulserunning) printf("Post-JackInit() error: Pulse running!\n");
			LGL.AudioUsingJack=false;

			//strcpy(audioDriver, "pulse");
			//strcpy(audioDriver, "alsa");
			//strcpy(audioDriver, "oss");
			//strcpy(audioDriver, "esd");

			/*
			if(inAudioChannels>=4)
			{
				strcpy(audioDriver, "dsp");
				//setenv("SDL_AUDIODRIVER", "alsa", 1);
				if(LGL_FileExists("/dev/dsp1"))
				{
					strcpy(dspPath, "/dev/dsp1");
				}
				else if(LGL_FileExists("/dev/dsp2"))
				{
					strcpy(dspPath, "/dev/dsp2");
				}
				else if(LGL_FileExists("/dev/dsp3"))
				{
					strcpy(dspPath, "/dev/dsp3");
				}
				else if(LGL_FileExists("/dev/dsp4"))
				{
					strcpy(dspPath, "/dev/dsp4");
				}
				else if(LGL_FileExists("/dev/dsp5"))
				{
					strcpy(dspPath, "/dev/dsp5");
				}
				else if(LGL_FileExists("/dev/dsp6"))
				{
					strcpy(dspPath, "/dev/dsp6");
				}
				else if(LGL_FileExists("/dev/dsp7"))
				{
					strcpy(dspPath, "/dev/dsp7");
				}
				else if(LGL_FileExists("/dev/dsp8"))
				{
					strcpy(dspPath, "/dev/dsp8");
				}
				else
				{
					printf("LGL_Init(): Warning! /dev/dsp1+ not found! Falling back to stereo on /dev/dsp...\n");
					strcpy(dspPath, "/dev/dsp");
				}

				setenv("SDL_PATH_DSP", dspPath, 1);
			}
			else if(inAudioChannels==2)
			{
				strcpy(audioDriver, "pulse");
			}
			*/

			//setenv("SDL_AUDIODRIVER", audioDriver, 1);

			if(pulserunning && inAudioChannels==4)
			{
				//For pulse, channels 3 and 4 are Center and LFE, so we need 6 channels to get to rear left and rear right.
				//6 channels isn't working... Not clear why. Downmix to 2, for now.
				inAudioChannels=2;
				LGL.AudioSpec->channels=inAudioChannels;
			}

			LGL_SAMPLESIZE_SDL = 1024;
			SDL_AudioSpec* AudioObtained=(SDL_AudioSpec*)malloc(sizeof(SDL_AudioSpec));;
			if(SDL_OpenAudio(LGL.AudioSpec,AudioObtained)==0)
			{
				LGL.AudioAvailable=true;

				assert(LGL.AudioSpec->format==AUDIO_S16);
				assert(LGL.AudioSpec->freq==AudioObtained->freq);
				if(LGL.AudioSpec->samples!=LGL_SAMPLESIZE_SDL)
				{
					printf("\nLGL_SAMPLESIZE_SDL not respected. Got %i. Wanted %i.\n\n",LGL.AudioSpec->samples,LGL_SAMPLESIZE_SDL);
					//assert(LGL.AudioSpec->samples==LGL_SAMPLESIZE);
					//LGL.AudioAvailable=false;
				}

				assert(AudioObtained->format==AUDIO_S16);
				if(AudioObtained->channels!=inAudioChannels)
				{
					if(AudioObtained->channels==4 && inAudioChannels==6)
					{
						printf("6to4 audio channel conversion active...\n");
						inAudioChannels=4;
						LGL.AudioSpec->channels=inAudioChannels;
					}
					else
					{
						if(AudioObtained->channels!=inAudioChannels)
						{
							printf("LGL_Init(): Error! Requested %i audio channels, got %i\n",inAudioChannels,AudioObtained->channels);
							assert(AudioObtained->channels==inAudioChannels);
						}
					}
				}
				//assert(AudioObtained->samples==LGL_SAMPLESIZE);
				assert(LGL.AudioStreamListSemaphore);

				SDL_PauseAudio(0);
/*
				char cmd[2048];
				sprintf(cmd,"chrt -p %i `pgrep pulseaudio`",(int)(LGL_PRIORITY_AUDIO_OUT*99));
				system(cmd);
*/
			}
			else
			{
				LGL.AudioAvailable=false;
			}
		}
		LGL_ThreadSetPriority(LGL_PRIORITY_MAIN,"Main");
	}

	SDL_WM_SetCaption(inWindowTitle,inWindowTitle);
	SDL_EnableUNICODE(1);

	//Initialize Video

#ifndef	LGL_NO_GRAPHICS

	for(int d=LGL.DisplayCount-1;d>=0;d--)
	{
		if(lgl_FakeSecondDisplay)
		{
			SDL_SelectVideoDisplay(0);
		}
		{
			SDL_SelectVideoDisplay(d);
		}

		int windowFlags = SDL_WINDOW_OPENGL;// | SDL_WINDOW_SHOWN;
		if(d>0)
		{
			if(lgl_FakeSecondDisplay)
			{
				windowFlags |= SDL_WINDOW_SHOWN;
			}
			else
			{
				windowFlags |= SDL_WINDOW_BORDERLESS;
			}
			/*
			if(lgl_HideProjectorWindows)
			{
				windowFlags |= SDL_WINDOW_HIDDEN;
			}
			*/
		}
		/*
		if(LGL.WindowFullscreen)
		{
			windowFlags |= SDL_WINDOW_FULLSCREEN;
			windowFlags |= SDL_WINDOW_BORDERLESS;
		}
		*/

		int posX=SDL_WINDOWPOS_CENTERED;
		int posY=SDL_WINDOWPOS_CENTERED;

		if(lgl_FakeSecondDisplay)
		{
			posX=0;
			for(int e=0;e<d;e++)
			{
				posX+=LGL.WindowResolutionX[e];
			}
			posY=0;
		}
		if(lgl_FakeSecondDisplay3x)
		{
			posX=0;
			posY=0;
			if(d==1)
			{
				posY=LGL.WindowResolutionY[0]+200;
			}
		}

		LGL.WindowID[d] = SDL_CreateWindow
		(
			inWindowTitle,
			posX,
			posY,
			LGL.WindowResolutionX[d],
			LGL.WindowResolutionY[d],
			windowFlags
		);
		//printf("WindowID[%i]: %x\n",d,LGL.WindowID[d]);

		if(LGL.WindowID[d]==0)
		{
			printf
			(
				"LGL_Init(): SDL_SetVideoMode \
				(%i,%i) failed for display %i... %s\n",
				LGL.WindowResolutionX[d],
				LGL.WindowResolutionY[d],
				d,
				SDL_GetError()
			);
			return(false);
		}
	}

	SDL_SelectVideoDisplay(0);
	LGL.MasterWindowID = SDL_CreateWindow
	(
		inWindowTitle,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		LGL.MasterWindowResolutionX,
		LGL.MasterWindowResolutionY,
		SDL_WINDOW_OPENGL// | SDL_WINDOW_SHOWN
	);
	LGL.GLContext = SDL_GL_CreateContext(LGL.MasterWindowID);
	SDL_GL_MakeCurrent(LGL.WindowID[0], LGL.GLContext);

	LGL.ThreadIDMain = SDL_ThreadID();
	LGL.ThreadIDWatch = NULL;

	//GL Settings

	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_FRONT);
	glClearColor(0,0,0,0);
	glDisable(GL_DEPTH_TEST);
	//glEnable(GL_LINE_SMOOTH);
	//glEnable(GL_POLYGON_SMOOTH);

	LGL.TexturePixels=0;
	LGL.FrameBufferTextureGlitchFix=true;
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

	LGL_ClearBackBuffer();
	LGL_SwapBuffers();
	LGL_SwapBuffers();

	LGL_SetActiveDisplay(0);

	LGL.GPUSpeed=0;
	LGL.GPUTemp=0;

	LGL.ShaderProgramCurrent=0;
	
	LGL.StatsLinesPerFrame=0;
	LGL.StatsRectsPerFrame=0;
	LGL.StatsImagesPerFrame=0;
	LGL.StatsPixelsPerFrame=0;

	LGL.SyphonImage=NULL;

	//GLSL Function Pointer Assignments

	gl2ShaderSource=		(glShaderSourceARB_Func)SDL_GL_GetProcAddress
					("glShaderSourceARB");
	gl2CreateShaderObject=		(glCreateShaderObjectARB_Func)SDL_GL_GetProcAddress
					("glCreateShaderObjectARB");
	gl2CompileShader=		(glCompileShaderARB_Func)SDL_GL_GetProcAddress
					("glCompileShaderARB");
	gl2GetObjectParameteriv=	(glGetObjectParameterivARB_Func)SDL_GL_GetProcAddress
					("glGetObjectParameterivARB");
	gl2GetInfoLog=			(glGetInfoLogARB_Func)SDL_GL_GetProcAddress
					("glGetInfoLogARB");
	gl2CreateProgramObject=		(glCreateProgramObjectARB_Func)SDL_GL_GetProcAddress
					("glCreateProgramObjectARB");
	gl2AttachObject=		(glAttachObjectARB_Func)SDL_GL_GetProcAddress
					("glAttachObjectARB");
	gl2LinkProgram=			(glLinkProgramARB_Func)SDL_GL_GetProcAddress
					("glLinkProgramARB");
	gl2ValidateProgram=		(glValidateProgramARB_Func)SDL_GL_GetProcAddress
					("glValidateProgramARB");
	gl2UseProgramObject=		(glUseProgramObjectARB_Func)SDL_GL_GetProcAddress
					("glUseProgramObjectARB");
	gl2DeleteObject=		(glDeleteObjectARB_Func)SDL_GL_GetProcAddress
					("glDeleteObjectARB");
	gl2GetShaderSource=		(glGetShaderSourceARB_Func)SDL_GL_GetProcAddress
					("glGetShaderSourceARB");
	gl2GetAttribLocation=		(glGetAttribLocationARB_Func)SDL_GL_GetProcAddress
					("glGetAttribLocationARB");
	gl2GetActiveAttrib=		(glGetActiveAttribARB_Func)SDL_GL_GetProcAddress
					("glGetActiveAttribARB");
	gl2VertexAttrib4iv=		(glVertexAttrib4ivARB_Func)SDL_GL_GetProcAddress
					("glVertexAttrib4ivARB");
	gl2VertexAttrib4fv=		(glVertexAttrib4fvARB_Func)SDL_GL_GetProcAddress
					("glVertexAttrib4fvARB");
	gl2GetUniformLocation=		(glGetUniformLocationARB_Func)SDL_GL_GetProcAddress
					("glGetUniformLocationARB");
	gl2Uniform1i=			(glUniform1iARB_Func)SDL_GL_GetProcAddress
					("glUniform1iARB");
	gl2Uniform2i=			(glUniform2iARB_Func)SDL_GL_GetProcAddress
					("glUniform2iARB");
	gl2Uniform3i=			(glUniform3iARB_Func)SDL_GL_GetProcAddress
					("glUniform3iARB");
	gl2Uniform4i=			(glUniform4iARB_Func)SDL_GL_GetProcAddress
					("glUniform4iARB");
	gl2Uniform1f=			(glUniform1fARB_Func)SDL_GL_GetProcAddress
					("glUniform1fARB");
	gl2Uniform2f=			(glUniform2fARB_Func)SDL_GL_GetProcAddress
					("glUniform2fARB");
	gl2Uniform3f=			(glUniform3fARB_Func)SDL_GL_GetProcAddress
					("glUniform3fARB");
	gl2Uniform4f=			(glUniform4fARB_Func)SDL_GL_GetProcAddress
					("glUniform4fARB");
	gl2GenBuffers=			(gl2GenBuffers_Func)SDL_GL_GetProcAddress
					("glGenBuffers");
	gl2IsBuffer=			(gl2IsBuffer_Func)SDL_GL_GetProcAddress
					("glIsBuffer");
	gl2BindBuffer=			(gl2BindBuffer_Func)SDL_GL_GetProcAddress
					("glBindBuffer");
	gl2BufferData=			(gl2BufferData_Func)SDL_GL_GetProcAddress
					("glBufferData");
	gl2MapBuffer=			(gl2MapBuffer_Func)SDL_GL_GetProcAddress
					("glMapBuffer");
	gl2UnmapBuffer=			(gl2UnmapBuffer_Func)SDL_GL_GetProcAddress
					("glUnmapBuffer");
	gl2DeleteBuffers=		(gl2DeleteBuffers_Func)SDL_GL_GetProcAddress
					("glDeleteBuffers");
	gl2ActiveTexture=		(gl2ActiveTexture_Func)SDL_GL_GetProcAddress
					("glActiveTexture");
	gl2MultiTexCoord2f=		(gl2MultiTexCoord2f_Func)SDL_GL_GetProcAddress
					("glMultiTexCoord2f");
	gl2MultiTexCoord2d=		(gl2MultiTexCoord2d_Func)SDL_GL_GetProcAddress
					("glMultiTexCoord2d");
#endif	//LGL_NO_GRAPHICS

	for(int d=LGL.DisplayCount-1;d>=0;d--)
	{
		LGL_SetActiveDisplay(d);
		LGL_SwapBuffers();
		LGL_SwapBuffers();
		if
		(
			d==0 ||
			lgl_HideProjectorWindows==false
		)
		{
			SDL_ShowWindow(LGL.WindowID[d]);
		}
	}

	//Video Decoding via avcodec
	lgl_av_register_all();
	lgl_avcodec_init(); 
	lgl_avcodec_register_all();
	lgl_av_log_set_level(AV_LOG_WARNING);	//QUIET, ERROR, WARNING, INFO, VERBOSE, DEBUG

	//bool audioIn=inAudioChannels < 0;
	inAudioChannels=abs(inAudioChannels);

	//LGL_Assert(inAudioChannels==0 || inAudioChannels==2 || inAudioChannels==4);

	LGL.AVOpenCloseSemaphore=new LGL_Semaphore("AV Codec Open/Close",false);	//Totally shouldn't be promiscuous, but our lgl_av* wrapper functions take care of that.

	//LGL.AudioSpec=AudioObtained;
	//delete AudioObtained;

	//Joysticks

	//SDL_JoystickEventState(SDL_ENABLE);
	for (int a=0;a<SDL_NumJoysticks();a++)
	{
		LGL.Joystick[a]=SDL_JoystickOpen(a);
		strcpy(LGL.JoystickName[a],SDL_JoystickName(a));
	}
	for(a=0;a<4;a++)
	{
		for(int b=0;b<32;b++)
		{
			LGL.JoyDown[a][b]=false;
			LGL.JoyStroke[a][b]=false;
			LGL.JoyRelease[a][b]=false;
		}
	}

	LGL.JoyNumber=SDL_NumJoysticks();
#ifdef	LGL_OSX
	//LGL.JoyNumber=(int)floor(SDL_JoystickNumButtons(LGL.Joystick[0])/12.0);
#endif	//LGL_OSX

	LGL.WiimoteSemaphore = new LGL_Semaphore("Wiimote");
	LGL.WriteFileAsyncWorkItemListNewSemaphore = new LGL_Semaphore("WriteFileAsyncWorkItemListNewSemaphore");
	LGL.WriteFileAsyncThread = NULL;

#ifdef	LGL_LINUX_VIDCAM
	int tempfd1=open("/dev/video0",O_RDWR);

	if(tempfd1>0)
	{
		close(tempfd1);
		if(LGL.JoyNumber<4)
		{
			LGL.VidCamAvailable=true;
			LGL.JoyNumber++;
		}
	}
#endif	//LGL_LINUX_VIDCAM

	//MIDI

	//M-Audio MIDI Devices

	lgl_MidiInit2();
	
	//VidCam

	//First, set reasonable defaults for no camera

	LGL.VidCamAvailable=false;
	sprintf(LGL.VidCamName,"No VidCam");
	LGL.VidCamWidthMin=1;
	LGL.VidCamHeightMin=1;
	LGL.VidCamWidthMax=1;
	LGL.VidCamHeightMax=1;
	LGL.VidCamWidthNow=1;
	LGL.VidCamHeightNow=1;
	LGL.VidCamBufferRaw=(unsigned char*)malloc(3);
	LGL.VidCamBufferProcessed=(unsigned char*)malloc(3);
	LGL.VidCamImageRaw=NULL;
	/*
	new LGL_Image
	(
		LGL.VidCamWidthNow,LGL.VidCamHeightNow,
		3,
		LGL.VidCamBufferRaw
	);
	*/
	LGL.VidCamImageProcessed=NULL;
	/*
	new LGL_Image
	(
		LGL.VidCamWidthNow,LGL.VidCamHeightNow,
		3,
		LGL.VidCamBufferProcessed
	);
	*/
	LGL.VidCamFD=-1;
	LGL.VidCamCaptureDelay=(2.0/30.0);
	LGL.VidCamCaptureDelayTimer.Reset();
	LGL.VidCamAxisXLast=0;
	LGL.VidCamAxisYLast=0;
	LGL.VidCamAxisXNext=0;
	LGL.VidCamAxisYNext=0;
	LGL.VidCamDistanceThumb=-40;
	LGL.VidCamDistancePointer=-50;
	
#ifdef LGL_LINUX_VIDCAM

	LGL.VidCamFD=open("/dev/video0",O_RDWR);
	
	if(LGL.VidCamFD<0)
	{
		//
	}
	else
	{
		struct video_capability	VidCamCapabilities;
		struct video_window	VidCamWindow;
		struct video_picture	VidCamPicture;
		struct video_buffer	VidCamBufferStruct;
		struct video_mbuf	VidCamMBuf;

		ioctl(LGL.VidCamFD, VIDIOCGCAP, &VidCamCapabilities);
		ioctl(LGL.VidCamFD, VIDIOCGWIN, &VidCamWindow);
		ioctl(LGL.VidCamFD, VIDIOCGPICT, &VidCamPicture);
		ioctl(LGL.VidCamFD, VIDIOCGFBUF, &VidCamBufferStruct);
		
		VidCamWindow.width=VidCamCapabilities.minwidth;
		VidCamWindow.height=VidCamCapabilities.minheight;
		
		ioctl(LGL.VidCamFD, VIDIOCSWIN, &VidCamWindow);
		ioctl(LGL.VidCamFD, VIDIOCGWIN, &VidCamWindow);

		if
		(
			(VidCamCapabilities.type & VID_TYPE_CAPTURE) == false ||
			VidCamPicture.palette != VIDEO_PALETTE_RGB24
		)
		{
			return(false);
		}

		LGL.VidCamAvailable=true;
		sprintf(LGL.VidCamName,"%s",VidCamCapabilities.name);
		LGL.VidCamWidthMin=VidCamCapabilities.minwidth;
		LGL.VidCamHeightMin=VidCamCapabilities.minheight;
		LGL.VidCamWidthMax=VidCamCapabilities.maxwidth;
		LGL.VidCamHeightMax=VidCamCapabilities.maxheight;
		LGL.VidCamWidthNow=VidCamWindow.width;
		LGL.VidCamHeightNow=VidCamWindow.height;

		LGL.VidCamMMap.format=VIDEO_PALETTE_RGB24;
		LGL.VidCamMMap.frame=0;
		LGL.VidCamMMap.width=VidCamWindow.width;
		LGL.VidCamMMap.height=VidCamWindow.height;

		ioctl(LGL.VidCamFD, VIDIOCGMBUF, &VidCamMBuf);

		LGL.VidCamBufferRaw=(unsigned char*)mmap
		(
			0,
			VidCamMBuf.size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			LGL.VidCamFD,
			0
		);
		LGL.VidCamBufferProcessed=(unsigned char*)malloc(VidCamMBuf.size);

		ioctl(LGL.VidCamFD, VIDIOCMCAPTURE, &(LGL.VidCamMMap));

		int p=0;

		ioctl(LGL.VidCamFD, VIDIOCSYNC, &p);
		ioctl(LGL.VidCamFD, VIDIOCMCAPTURE, &(LGL.VidCamMMap));
		
		LGL.VidCamImageRaw->LoadSurfaceToTexture();
		LGL.VidCamImageProcessed->LoadSurfaceToTexture();

		FILE* tempfd=fopen(".lgl-vidcam-calibration","r");
		if(tempfd!=NULL)
		{
			char readstuff[128];
			
			fgets(readstuff,128,tempfd);
			LGL.VidCamDistanceThumb=atoi(readstuff);
			
			fgets(readstuff,128,tempfd);
			LGL.VidCamDistancePointer=atoi(readstuff);
			
			fclose(tempfd);
		}
		else
		{
			LGL_VidCamCalibrate(NULL,.5);
		}
	}

#endif //LGL_LINUX_VIDCAM
	
	//Reset Joysticks
		
	for(int a=0;a<4;a++)
	{
		for(int b=0;b<32;b++)
		{
			LGL.JoyStroke[a][b]=false;
			LGL.JoyDown[a][b]=false;
			LGL.JoyRelease[a][b]=false;
		}
		for(int b=0;b<2;b++)
		{
			for(int c=0;c<2;c++)
			{
				LGL.JoyAnalogue[a][b][c]=0;
			}
		}
	}

	//Clear Bogus Events
	
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		//
	}
	
	//Net

	LGL.NetAvailable=(SDLNet_Init()==-1)?false:true;

	//Misc

	LGL.Running=true;
	LGL.UserExit=NULL;
	
	//All done! Print Results
	
	printf("\nLGL Initialization\n");
	printf("---\n");
#ifdef	LGL_LINUX
#ifdef	LGL_OSX
	printf("OS\t\t\tOSX\n");
#else
	printf("OS\t\t\tLinux\n");
#endif	//LGL_OSX
#endif	//LGL_LINUX
#ifdef	LGL_WIN32
	printf("OS\t\t\tWin32\n");
#endif	//LGL_WIN32
	printf("CPUs\t\t\t%i\n",LGL_CPUCount());
	printf
	(
		"SDL\t\t\t%i.%i.%i\n",
		SDL_Linked_Version()->major,
		SDL_Linked_Version()->minor,
		SDL_Linked_Version()->patch
	);

	if
	(
		LGL_AudioAvailable() &&
		LGL.AudioUsingJack
	)
	{
		printf
		(
			"JACK Output\t\t%i channels. %ihz. %.1fms latency.\n",// RT Priority: %i\n",
			LGL.AudioSpec->channels,
			jack_get_sample_rate(jack_client),
			1000.0f*LGL_SAMPLESIZE_SDL/(44100.0f)
			//jack_client_real_time_priority(jack_client)
		);
		printf
		(
			"JACK Input\t\t%s\n",
			LGL.AudioInAvailable ? "Present" : "Absent"
		);
		if(LGL.AudioInAvailable==false)
		{
#ifdef	LGL_OSX
			printf("\t\t\t\tIn System Preferences => Sound, verify Input and Output devices are correct.\n");
#endif	//LGL_OSX
		}
	}
	else
	{
		printf("JACK\t\t\tInit FAILED\n");
		printf("\t\t\tAre your ~/.jackrc and ~/.asoundrc correct?\n");
		printf("\t\t\tDid you remember to killall pulseaudio?\n");
		printf("\t\t\tAre any other programs using your soundcard?\n");
	}

	if
	(
		LGL_AudioAvailable() &&
		LGL.AudioUsingJack==false
	)
	{
		SDL_AudioDriverName(audioDriver,1024);
		if(dspPath[0]=='\0')
		{
			printf
			(
				"SDL_AudioOut\t\t%i channels. %i samples. %s.\n",
				LGL.AudioSpec->channels,
				LGL.AudioSpec->samples,
				audioDriver
			);
		}
		else
		{
			printf
			(
				"SDL_AudioOut\t\t%i channels. %i samples. %s. (%s)\n",
				LGL.AudioSpec->channels,
				LGL.AudioSpec->samples,
				audioDriver,
				dspPath
			);
		}
	}

	if(LGL_AudioAvailable()==false)
	{
		printf("SDL_AudioOut\t\tInit FAILED\n");
		printf("\t\t\t%s (Driver: %s)\n",SDL_GetError(),SDL_AudioDriverName(audioDriver,1024));
	}

	printf("MIDI Devices\t\t");
	if(LGL_MidiDeviceCount())
	{
		printf("Present\n");
		for(unsigned int a=0;a<LGL_MidiDeviceCount();a++)
		{
			printf("\t\t\t[%i] %s\n",a,LGL_MidiDeviceName(a));
		}
	}
	else
	{
		printf("Absent\n");
	}
	printf("SDL_net\t\t\t%s\n",LGL.NetAvailable?"Present":SDLNet_GetError());
#ifndef	LGL_NO_GRAPHICS
	printf("OpenGL Renderer\t\t%s\n",(char*)(glGetString(GL_RENDERER)));
	GLint rectTexSize=0;
	glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB,&rectTexSize);
	printf("GL_RECT_TEX Size\t%i\n",rectTexSize);
	printf("OpenGL VBO\t\t%s\n",LGL_VBOAvailable()?"Present":"Absent");
	printf("GLSL Vert Shader\t%s\n",LGL_ShaderAvailableVert()?"Present":"Absent");
	printf("GLSL Frag Shader\t%s\n",LGL_ShaderAvailableFrag()?"Present":"Absent");
#endif	//LGL_NO_GRAPHICS
#ifdef	LGL_LINUX_VIDCAM
	printf("Video4Linux Device\t%s\n",LGL_VidCamAvailable()?"Present":"Absent");
#endif	//LGL_LINUX_VIDCAM

	//LGL_MouseWarp(1,0);
	
	if(LGL.JoyNumber!=0)
	{
		printf("%i joystick(s) found:\n",LGL.JoyNumber);
		for(a=0;a<LGL.JoyNumber;a++)
		{
			printf("\t%s\n",LGL_JoyName(a));
		}
	}
	printf("---\n\n");

	lgl_fftw_init();
#ifndef	LGL_NO_GRAPHICS
	strcpy(LGL.GPURenderer,(char*)(glGetString(GL_RENDERER)));
#endif	//LGL_NO_GRAPHICS

	LGL_ThreadSetPriority(LGL_PRIORITY_MAIN,"Main");

	lgl_lgl_initialized=true;

	return(true);
}

bool
LGL_Running()
{
	return(LGL.Running);
}

void
LGL_SetUserExit(void(*fn)(void))
{
	LGL.UserExit = fn;
}

void
LGL_ExitAlpha()
{
	if(LGL.Running)
	{
		LGL.Running=false;
		if(LGL.UserExit)
		{
			LGL.UserExit();
		}
	}
}

void
LGL_ExitOmega()
{
	LGL_ExitAlpha();

	lgl_SyphonExit();

	//LGL.PathIsAliasCacher.Save();

	if(LGL.WriteFileAsyncThread)
	{
		LGL_ThreadWait(LGL.WriteFileAsyncThread);
	}
	if(LGL.AudioEncoder)
	{
		LGL.AudioQuitting=true;
		LGL_DelayMS(100);	//HACK
		delete LGL.AudioEncoder;
		LGL.AudioEncoder=NULL;
	}
	exit(0);
}

void
LGL_Exit()
{
	LGL_ExitOmega();
}

//Time

void
LGL_Delay()
{
	LGL_DelayMS(1);
}

void
LGL_DelayMS
(
	float	ms
)
{
	if(ms<=0)
	{
		LGL_Delay();
	}
	else
	{
		SDL_Delay((int)ms);
	}
}

void
LGL_DelaySeconds
(
	float	seconds
)
{
	if(seconds<=0)
	{
		LGL_Delay();
	}
	else
	{
		LGL_DelayMS((int)(seconds*1000.0));
	}
}

double
LGL_SecondsSinceLastFrame()
{
	return(LGL.SecondsSinceLastFrame);
}

double
LGL_SecondsSinceThisFrame()
{
	return(LGL.SecondsSinceLastFrameTimer.SecondsSinceLastReset());
}

double
LGL_SecondsSinceExecution()
{
	return(LGL.SecondsSinceExecution);
}

unsigned long
LGL_FramesSinceExecution()
{
	return(LGL.FramesSinceExecution);
}
	
int
LGL_FPS()
{
	return(LGL.FPS);
}

int
LGL_GetFPSMax()
{
	return(LGL.FPSMax);
}

void
LGL_SetFPSMax
(
	int	fpsMax
)
{
	LGL.FPSMax=LGL_Clamp(1,fpsMax,60);
}

const
char*
LGL_TimeOfDay()
{
	time_t tim=time(NULL);
	struct tm TM=*localtime(&tim);
	int seconds=TM.tm_sec;
	int minutes=TM.tm_min;
	int hours=TM.tm_hour;
	if(hours>12)
	{
		hours-=12;
	}
	if(hours==0)
	{
		hours=12;
	}
	sprintf(LGL.TimeOfDay,"%02i:%02i.%02i",hours,minutes,seconds);
	return(LGL.TimeOfDay);
}

const
char*
LGL_DateAndTimeOfDay()
{
	time_t tim=time(NULL);
	struct tm TM=*localtime(&tim);
	int seconds=TM.tm_sec;
	int minutes=TM.tm_min;
	int hours=TM.tm_hour;
	/*
	if(hours>12)
	{
		hours-=12;
	}
	if(hours==0)
	{
		hours=12;
	}
	*/
	int date=TM.tm_mday;
	int month=TM.tm_mon+1;
	int year=1900+TM.tm_year;
	sprintf
	(
		LGL.DateAndTimeOfDay,
		"%04i.%02i.%02i - %02i:%02i.%02i",
		year,month,date,
		hours,minutes,seconds
	);
	return(LGL.DateAndTimeOfDay);
}

const
char*
LGL_DateAndTimeOfDayOfExecution()
{
	return(LGL.DateAndTimeOfDayOfExecution);
}

LGL_Timer::
LGL_Timer()
{
	Reset();
}

LGL_Timer::
~LGL_Timer()
{
	//
}

double ConvertMicrosecondsToDouble(UnsignedWidePtr microsecondsValue)
{ 
	double twoPower32 = 4294967296.0; 
	double doubleValue; 

	double upperHalf = (double)microsecondsValue->hi; 
	double lowerHalf = (double)microsecondsValue->lo; 

	doubleValue = (upperHalf * twoPower32) + lowerHalf; 
	return doubleValue;
}

double
LGL_Timer::
SecondsSinceLastReset() const
{
	/*
	timeval timeNow;
	gettimeofday(&timeNow,NULL);

	double ret=timeNow.tv_sec-TimeAtLastReset.tv_sec;
	ret+=(timeNow.tv_usec-TimeAtLastReset.tv_usec)/(1000.0*1000.0);
	if(ret<0) ret=0;
	return(ret);
	*/

	/*
	int64_t timeNowMach=mach_absolute_time();
	int64_t deltaMach = timeNowMach - TimeAtLastResetMach;
	Nanoseconds deltaNano = AbsoluteToNanoseconds( *(AbsoluteTime *) &deltaMach );

	return((*((uint64_t *) (&deltaNano)))/(1000.0*1000.0*1000.0));
	*/

	UnsignedWide currentTime;
	Microseconds(&currentTime);
	double currentTimeAsDouble = ConvertMicrosecondsToDouble(&currentTime);

	double oneMicrosecond = .000001;
	double elapsedTime = currentTimeAsDouble - TimeAtLastResetMicroSeconds;

	// Convert elapsed time to seconds.
	double elapsedTimeInSeconds = elapsedTime * oneMicrosecond;
	return(elapsedTimeInSeconds);
}

void
LGL_Timer::
Reset()
{
	//gettimeofday(&TimeAtLastReset,NULL);

	//TimeAtLastResetMach=mach_absolute_time();

	UnsignedWide currentTime; 
	Microseconds(&currentTime); 
	TimeAtLastResetMicroSeconds = ConvertMicrosecondsToDouble(&currentTime); 
}

unsigned int
LGL_AudioChannels()
{
	return(LGL.AudioSpec->channels);
}

bool
LGL_AudioOutAvailable()
{
	return(LGL.AudioInAvailable);
}

bool
LGL_AudioInAvailable()
{
	return(LGL.AudioInAvailable);
}

void
LGL_SetAudioInPassThru
(
	bool passThru
)
{
	LGL.AudioInPassThru=passThru;
}

bool
LGL_AudioOutDisconnected()
{
	return(LGL.AudioOutDisconnected);
}

std::vector<LGL_AudioGrain*>&
LGL_AudioInGrainList()
{
	return(LGL.AudioInGrainListFront);
}

Uint8 audioInMetadataBuffer[1024*4*4];

bool
LGL_AudioInMetadata
(
	float&	volAve,
	float&	volMax,
	float&	freqFactor,
	float	gain,
	float	freqEQBalance
)
{
	volAve=0.0f;
	volMax=0.0f;
	freqFactor=0.0f;

	if(LGL.AudioInAvailable==false)
	{
		return(false);
	}

	std::vector<LGL_AudioGrain*> inGrains=LGL.AudioInGrainListFixedSize;
	if(inGrains.size()==0)
	{
		return(false);
	}
	//float weightTotal=0.0f;
	//float tmpWeight;
	//float tmpVolAve;
	//float tmpVolMax;
	//float tmpFreqFactor;
	Uint8* bufNow=audioInMetadataBuffer;
	long bufSamples=0;
	for(unsigned int a=0;a<inGrains.size();a++)
	{
		inGrains[a]->GetWaveformToMemory(bufNow);
		bufNow+=4*inGrains[a]->GetLengthSamples();
		bufSamples+=inGrains[a]->GetLengthSamples();

		/*
		tmpWeight=inGrains[a]->GetLengthSamples();
		inGrains[a]->GetMetadata
		(
			tmpVolAve,
			tmpVolMax,
			tmpFreqFactor
		);
		weightTotal+=tmpWeight;
		volAve+=tmpVolAve*tmpWeight;
		volMax=LGL_Max(volMax,tmpVolMax);
		freqFactor+=tmpFreqFactor*tmpWeight;
		*/
	}
	LGL_AudioGrain bigGrain;
	bigGrain.SetWaveformFromMemory
	(
		audioInMetadataBuffer,
		bufSamples
	);
	bigGrain.GetMetadata
	(
		volAve,
		volMax,
		freqFactor,
		gain,
		freqEQBalance
	);
	/*
	volAve/=weightTotal;
	freqFactor/=weightTotal;
	*/

	return(true);
}

void
LGL_DrawWaveform
(
	float	left,	float	right,
	float	bottom,	float	top,
	float	r,	float	g,	float	b,	float	a,
	float	thickness,
	bool	antialias
)
{
	if(LGL_AudioAvailable()==false)
	{
		//TODO: Draw silent waveform
		return;
	}

	//Prepare arguments
	float width=right-left;
	float height=top-bottom;
	float* pointsXY=new float[1024*2];
	for(int z=0;z<1024;z++)
	{
		pointsXY[2*z+0] = left+width*(z/1024.0f);
		pointsXY[2*z+1] = bottom+height*0.5f*(LGL.AudioBufferLFront[z]+LGL.AudioBufferRFront[z]);
	}

	//Draw
	LGL_DrawLineStripToScreen
	(
		pointsXY,
		1024,
		r,g,b,a,
		thickness,
		antialias
	);

	//Cleanup
	delete pointsXY;
}

void
LGL_DrawAudioInWaveform
(
	float	left,	float	right,
	float	bottom,	float	top,
	float	r,	float	g,	float	b,	float	a,
	float	thickness,
	bool	antialias
)
{
	if(LGL_AudioInAvailable()==false)
	{
		//TODO: Draw silent waveform
		return;
	}
	LGL_AudioGrain* grain=LGL.AudioInFallbackGrain;
	if(LGL.AudioInGrainListFront.empty()==false)
	{
		grain=LGL.AudioInGrainListFront[LGL.AudioInGrainListFront.size()-1];
	}
	if(grain)
	{
		grain->DrawWaveform
		(
			left,right,
			bottom,top,
			r,g,b,a,
			thickness,
			antialias
		);
	}
}

void
LGL_DrawAudioInSpectrum
(
	float	left,	float	right,
	float	bottom,	float	top
)
{
	if(LGL_AudioInAvailable()==false)
	{
		//TODO: Draw silent spectrum
		return;
	}
	LGL_AudioGrain* grain=LGL.AudioInFallbackGrain;
	if(LGL.AudioInGrainListFront.empty()==false)
	{
		grain=LGL.AudioInGrainListFront[LGL.AudioInGrainListFront.size()-1];
	}
	grain->DrawSpectrum
	(
		left,right,
		bottom,top
	);
}
		
//Frequency Analysis
#define	LGL_FFTW
#ifdef	LGL_FFTW

float lgl_fftw_wisdom_creation_percent_complete=0.0f;

void
lgl_fftw_init_draw(LGL_Image* img)
{
	//Warn user that we're calculating optimal FFTs
	LGL_GetFont().DrawString
	(
		.5,.925,.05,
		1,1,1,1,
		true,1,
		"Optimizing FFTs"
	);
	LGL_GetFont().DrawString
	(
		.5,.85,.05,
		1,1,1,1,
		true,1,
		"For Your CPU"
	);
	if(img)
	{
		img->DrawToScreen(0.25f,0.75f,0.25f,0.75f);
	}
	LGL_GetFont().DrawString
	(
		.5,.15,.03,
		1,1,1,1,
		true,1,
		"ETA: Less than 1 minute"
	);
	LGL_GetFont().DrawString
	(
		.5,.1,.03,
		1,1,1,1,
		true,1,
		"(This only happens the first time you run dvj)"
	);

	float& pct=lgl_fftw_wisdom_creation_percent_complete;
	LGL_DrawRectToScreen
	(
		0,pct,
		0,0.05f,
		.4f*pct,.2f*pct,0.5f+0.5f*pct,1.0f
	);
	LGL_GetFont().DrawString
	(
		0.49f,0.015f,0.02f,
		1,1,1,1,
		false,
		.75f,
		"%.1f%%",
		pct*100.0f
	);

	LGL_SwapBuffers();
}

int lgl_fftw_wisdom_creation_thread_status=0;

int
lgl_fftw_wisdom_creation_thread
(
	void*	meh
)
{
	int creationFlag = FFTW_EXHAUSTIVE;
	plan_main_forward =		fftwf_plan_dft_1d(fft_elementCount, fft_array_main_forward,		fft_array_main_forward,		FFTW_FORWARD,	creationFlag);
	lgl_fftw_wisdom_creation_percent_complete=0.5f;
	plan_main_backward =		fftwf_plan_dft_1d(fft_elementCount, fft_array_main_backward,		fft_array_main_backward,	FFTW_BACKWARD,	creationFlag);
	plan_callback_forward =		fftwf_plan_dft_1d(fft_elementCount, fft_array_callback_forward,		fft_array_callback_forward,	FFTW_FORWARD,	creationFlag);
	plan_callback_backward =	fftwf_plan_dft_1d(fft_elementCount, fft_array_callback_backward, 	fft_array_callback_backward,	FFTW_BACKWARD,	creationFlag);
	lgl_fftw_wisdom_creation_percent_complete=0.99f;
	lgl_fftw_wisdom_creation_thread_status=-1;
	return(0);
}

void
lgl_fftw_init()
{
	bool wisdomLoaded=false;
	int creationFlag = FFTW_EXHAUSTIVE;

	char fftwWisdomDir[2048];
	sprintf(fftwWisdomDir,"%s/.dvj",LGL_GetHomeDir());
	if(LGL_DirectoryExists(fftwWisdomDir)==false)
	{
		LGL_DirectoryCreate(fftwWisdomDir);
	}

	char fftwWisdomPath[2048];
	sprintf(fftwWisdomPath,"%s/.fftw_wisdom",fftwWisdomDir);

	if(FILE* fd=fopen(fftwWisdomPath,"r"))
	{
		wisdomLoaded=fftwf_import_wisdom_from_file(fd);
		if(wisdomLoaded)
		{
			creationFlag = FFTW_ESTIMATE;
		}
		fclose(fd);
	}

	fft_array_main_forward =	(fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fft_elementCount);
	fft_array_main_backward =	(fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fft_elementCount);
	fft_array_callback_forward =	(fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fft_elementCount);
	fft_array_callback_backward =	(fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * fft_elementCount);

//wisdomLoaded=false;
	if(wisdomLoaded==false)
	{
		LGL_Image* fft_img=NULL;
		const char* fftwWisdomImagePath="data/image/fftw_wisdom.png";
		if(0 && LGL_FileExists(fftwWisdomImagePath))
		{
			fft_img = new LGL_Image(fftwWisdomImagePath);
		}
		LGL_Image fftImage(fftwWisdomImagePath);	//FIXME: This loads the image anew each frame... ouch!
		LGL_SwapBuffers();
		lgl_fftw_init_draw(fft_img);
		LGL_ThreadCreate(lgl_fftw_wisdom_creation_thread);
		while(lgl_fftw_wisdom_creation_thread_status!=-1)
		{
			lgl_fftw_init_draw(fft_img);
			const float addMe=0.00005f;
			if
			(
				(
					lgl_fftw_wisdom_creation_percent_complete+addMe<0.495f ||
					lgl_fftw_wisdom_creation_percent_complete+addMe>0.5f
				) &&
				lgl_fftw_wisdom_creation_percent_complete+addMe<0.99f
			)
			{
				lgl_fftw_wisdom_creation_percent_complete+=addMe;
			}
		}
		lgl_fftw_wisdom_creation_thread_status=0;

		if(FILE* fd=fopen(fftwWisdomPath,"w"))
		{
			fftwf_export_wisdom_to_file(fd);
			fclose(fd);
		}
		lgl_fftw_wisdom_creation_percent_complete=1.0f;
		lgl_fftw_init_draw(fft_img);

		//Finish warning user that we're calculating optimal FFTs
		LGL_DelaySeconds(0.25f);
		LGL_SwapBuffers();

		if(fft_img)
		{
			delete fft_img;
			fft_img=NULL;
		}
//exit(0);
	}
	else
	{
		plan_main_forward =		fftwf_plan_dft_1d(fft_elementCount, fft_array_main_forward,		fft_array_main_forward,		FFTW_FORWARD,	creationFlag);
		plan_main_backward =		fftwf_plan_dft_1d(fft_elementCount, fft_array_main_backward,		fft_array_main_backward,	FFTW_BACKWARD,	creationFlag);
		plan_callback_forward =		fftwf_plan_dft_1d(fft_elementCount, fft_array_callback_forward,		fft_array_callback_forward,	FFTW_FORWARD,	creationFlag);
		plan_callback_backward =	fftwf_plan_dft_1d(fft_elementCount, fft_array_callback_backward, 	fft_array_callback_backward,	FFTW_BACKWARD,	creationFlag);
	}
}

void
LGL_FFT
(
	float*	real,
	float*	imaginary,
	int	lgSize,
	int	direction,
	int	threadID
)
{
	int elementCount = (int)powl(2,lgSize);
	if(fft_elementCount!=elementCount)
	{
		printf("HEY!!! %i vs %i\n",fft_elementCount,elementCount);
		assert(fft_elementCount==elementCount);
	}
	if(direction==1)
	{
		fftwf_complex* fft_array_forward = (threadID==0) ? fft_array_main_forward : fft_array_callback_forward;
		fftwf_plan& plan_forward = (threadID==0) ? plan_main_forward : plan_callback_forward;

		for(int a=0;a<fft_elementCount;a++)
		{
			fft_array_forward[a][0]=real[a];
			fft_array_forward[a][1]=imaginary[a];
		}
		fftwf_execute(plan_forward);
		for(int a=0;a<fft_elementCount;a++)
		{
			real[a]=fft_array_forward[a][0];
			imaginary[a]=fft_array_forward[a][1];
		}
	}
	else
	{
		fftwf_complex* fft_array_backward = (threadID==0) ? fft_array_main_backward : fft_array_callback_backward;
		fftwf_plan& plan_backward = (threadID==0) ? plan_main_backward : plan_callback_backward;

		for(int a=0;a<fft_elementCount;a++)
		{
			fft_array_backward[a][0]=real[a];
			fft_array_backward[a][1]=imaginary[a];
		}
		fftwf_execute(plan_backward);
		for(int a=0;a<fft_elementCount;a++)
		{
			real[a]=fft_array_backward[a][0]/fft_elementCount;
			imaginary[a]=fft_array_backward[a][1]/fft_elementCount;
		}
	}
	//fftwf_destroy_plan(plan);  
	//fftwf_free(fft_array);
}

#else	//LGL_FFTW

//FIXME: Get permission for LGL_FFT code from http://astronomy.swin.edu.au/~pbourke/analysis/dft/
void
LGL_FFT
(
	float*	real,
	float*	imaginary,
	int	lgSize,
	int	direction
)
{
	int dir=direction;
	float* x=real;
	float* y=imaginary;
	int m=lgSize;
	
	long n,i,i1,j,k,i2,l,l1,l2;
	double c1,c2,tx,ty,t1,t2,u1,u2,z;

	/* Calculate the number of points */
	n=(long)pow(2,lgSize);

	/* Do the bit reversal */
	i2 = n >> 1;
	j = 0;
	for (i=0;i<n-1;i++)
	{
		if (i < j)
		{
			tx = x[i];
			ty = y[i];
			x[i] = x[j];
			y[i] = y[j];
			x[j] = tx;
			y[j] = ty;
		}
		k = i2;
		while (k <= j)
		{
			j -= k;
			k >>= 1;
		}
		j += k;
	}

	/* Compute the FFT */
	c1 = -1.0; 
	c2 = 0.0;
	l2 = 1;
	for (l=0;l<m;l++)
	{
		l1 = l2;
		l2 <<= 1;
		u1 = 1.0; 
		u2 = 0.0;
		for (j=0;j<l1;j++)
		{
			for (i=j;i<n;i+=l2)
			{
				i1 = i + l1;
				t1 = u1 * x[i1] - u2 * y[i1];
				t2 = u1 * y[i1] + u2 * x[i1];
				x[i1] = x[i] - t1; 
				y[i1] = y[i] - t2;
				x[i] += t1;
				y[i] += t2;
			}
			z =  u1 * c1 - u2 * c2;
			u2 = u1 * c2 + u2 * c1;
			u1 = z;
		}
		c2 = sqrt((1.0 - c1) / 2.0);
		if (dir == 1) 
			c2 = -c2;
		c1 = sqrt((1.0 + c1) / 2.0);
	}

	/* Scaling for forward transform */
	if (dir == 1)
	{
		for (i=0;i<n;i++)
		{
			x[i] /= n;
			y[i] /= n;
		}
	}
}

#endif	//LGL_FFTW

//Video

float*	audioBufferBackSilence=NULL;
int	lgl_MidiUpdate();	//Internal LGL function

#define	FONT_BUFFER_SIZE (1024*1024)	//4MB for fonts... that sould be more than enough, right?
float lgl_font_gl_buffer[FONT_BUFFER_SIZE];
float* lgl_font_gl_buffer_ptr;

int lgl_font_gl_buffer_int[FONT_BUFFER_SIZE];
int* lgl_font_gl_buffer_int_ptr;

void
lgl_EndFrame()
{
	if(lgl_lgl_initialized && LGL.AudioOutCallbackTimer.SecondsSinceLastReset()>1.0f)
	{
		if(LGL.AudioWasOnceAvailable)
		{
			LGL.AudioOutDisconnected=true;
			if(LGL.AudioUsingJack)
			{
				if(LGL.AudioOutReconnectTimer.SecondsSinceLastReset()>1.0f)
				{
					printf("Trying to revive JACK! (Alpha)\n");
					//jack_client_close(jack_client);
					printf("\tRevive: Killing prior JACK!\n");
					system("killall -9 jackd");
					printf("\tRevive: Waiting half a second!\n");
					LGL_DelayMS(500);
					printf("\tRevive: LGL_JackInit(): Alpha!\n");
					LGL_JackInit();
					printf("\tRevive: LGL_JackInit(): Alpha!\n");
					LGL.AudioOutReconnectTimer.Reset();
					printf("Trying to revive JACK! (Omega)\n");
				}
			}
		}
	}
	else
	{
		LGL.AudioOutDisconnected=false;
	}

	if(LGL.AudioJackXrunBack)
	{
		LGL.AudioJackXrunFront=true;
		LGL.AudioJackXrunBack=false;
	}
	else
	{
		LGL.AudioJackXrunFront=false;
	}

	if(LGL.AudioBufferPos>=1024)
	{
		memcpy
		(
			LGL.AudioBufferLFront,
			LGL.AudioBufferLBack,
			sizeof(LGL.AudioBufferLBack)
		);
		memcpy
		(
			LGL.AudioBufferRFront,
			LGL.AudioBufferRBack,
			sizeof(LGL.AudioBufferRBack)
		);

		if(audioBufferBackSilence==NULL)
		{
			audioBufferBackSilence=new float[1024];
			for(int a=0;a<1024;a++)
			{
				audioBufferBackSilence[a]=0.5f;
			}
		}

		memcpy
		(
			LGL.AudioBufferLBack,
			audioBufferBackSilence,
			sizeof(LGL.AudioBufferLBack)
		);
		memcpy
		(
			LGL.AudioBufferRBack,
			audioBufferBackSilence,
			sizeof(LGL.AudioBufferRBack)
		);

		LGL.AudioPeakLeft=0;
		LGL.AudioPeakRight=0;
		LGL.AudioPeakMono=0;

		for(unsigned int a=0;a<sizeof(LGL.AudioBufferLFront);a++)
		{
			if(fabsf(LGL.AudioBufferLFront[a]-.5f)>LGL.AudioPeakLeft)
			{
				LGL.AudioPeakLeft=fabsf(LGL.AudioBufferLFront[a]-.5f);
				LGL.AudioPeakMono=fabsf(LGL.AudioBufferLFront[a]-.5f);
			}
			if(fabsf(LGL.AudioBufferRFront[a]-.5f)>LGL.AudioPeakRight)
			{
				LGL.AudioPeakRight=fabsf(LGL.AudioBufferRFront[a]-.5f);
				LGL.AudioPeakMono=fabsf(LGL.AudioBufferRFront[a]-.5f);
			}
		}

		LGL.AudioBufferPos=0;

		/*
		float x[1024];
		float y[1024];

		for(int a=0;a<1024;a++)
		{
			x[a]=LGL.AudioBufferLFront[a];
			y[a]=0;
		}

		LGL_FFT(x,y,10,1);

		for(int a=0;a<512;a++)
		{
			LGL.FreqBufferL[a]=100.0f*sqrtf(x[a]*x[a]+y[a]*y[a]);
		}
		*/
	}
	
	if(LGL.AudioStreamListSemaphore!=NULL)
	{
		if(LGL.AudioStreamList.size()>0)
		{
			LGL_ScopeLock AudioStreamListSemaphoreLock(__FILE__,__LINE__,LGL.AudioStreamListSemaphore);
			for(unsigned int a=0;a<LGL.AudioStreamList.size();a++)
			{
				LGL.AudioStreamList[a]->Update();
			}
		}
	}

	if(LGL.AudioInAvailable)
	{
		for(unsigned int a=0;a<LGL.AudioInGrainListFront.size();a++)
		{
			delete LGL.AudioInGrainListFront[a];
		}
		LGL.AudioInGrainListFront.clear();

		{
			LGL_ScopeLock audioInSemaphoreLock(__FILE__,__LINE__,LGL.AudioInSemaphore,0.0f);
			if(audioInSemaphoreLock.GetLockObtained())
			{
				LGL.AudioInGrainListFront=LGL.AudioInGrainListBack;
				LGL.AudioInGrainListBack.clear();
			}
		}

		for(unsigned int a=0;a<LGL.AudioInGrainListFront.size();a++)
		{
			LGL_AudioGrain* neo = new LGL_AudioGrain;
			neo->SetWaveformFromAudioGrain(LGL.AudioInGrainListFront[a]);
			LGL.AudioInGrainListFixedSize.insert(LGL.AudioInGrainListFixedSize.begin(),neo);
		}
		while(LGL.AudioInGrainListFixedSize.size()>4)
		{
			LGL_AudioGrain* del = LGL.AudioInGrainListFixedSize[LGL.AudioInGrainListFixedSize.size()-1];
			delete del;
			LGL.AudioInGrainListFixedSize.pop_back();
		}
		while(LGL.AudioInGrainListFixedSize.size()<4)
		{
			LGL_AudioGrain* neo = new LGL_AudioGrain;
			neo->SetWaveformFromAudioGrain(LGL.AudioInFallbackGrain);
			LGL.AudioInGrainListFixedSize.insert(LGL.AudioInGrainListFixedSize.begin(),neo);
		}
	}

	LGL.DebugPrintfY=1.0f;

	if(LGL.DrawLogFD)
	{
		if(LGL_SecondsSinceExecution() < LGL.DrawLogTimeOfNextDrawnFrame)
		{
			//We must only write critical commands preceded by '!'. Delete all others.
			for(int a=0;a<(int)LGL.DrawLog.size();a++)
			{
				if
				(
					strlen(LGL.DrawLog[a]) < 1 ||
					LGL.DrawLog[a][0]!='!'
				)
				{
					//This is non-critical. We delete.

					//But first: does it have an associated data block?

					if
					(
						strlen(LGL.DrawLog[a]) > 1 &&
						LGL.DrawLog[a][0]=='D' &&
						LGL.DrawLog[a][1]=='|'
					)
					{
						delete LGL.DrawLogData[0];
						LGL.DrawLogData[0]=NULL;
						LGL.DrawLogData.erase(LGL.DrawLogData.begin());
						LGL.DrawLogDataLength.erase(LGL.DrawLogDataLength.begin());
					}

					delete LGL.DrawLog[a];
					LGL.DrawLog[a]=NULL;
					LGL.DrawLog.erase((std::vector<char*>::iterator)(&(LGL.DrawLog[a])));
					a--;
				}
			}
		}
		else
		{
			LGL.DrawLogTimeOfNextDrawnFrame += 1.0f/60.0f;

			char* neo=new char[1024];
			sprintf(neo,"LGL_SwapBuffers|%f\n",LGL.RecordSamplesWritten/(double)LGL.AudioSpec->freq);
			LGL.DrawLog.push_back(neo);
			
			neo=new char[1024];
			sprintf(neo,"LGL_MouseCoords|%f|%f\n",LGL_MouseX(),LGL_MouseY());
			LGL.DrawLog.push_back(neo);
		}

		//Write Wiimote Motion each frame
		char* neo;
		for(int a=0;a<8;a++)
		{
			LGL_GetWiimote(a).INTERNAL_ProcessInput();
			if(LGL_GetWiimote(a).GetPointerAvailable())
			{
				std::vector<LGL_Vector> motion = LGL_GetWiimote(a).GetPointerMotionThisFrame();
				if(motion.empty()==false)
				{
					neo = new char[1024];
					sprintf
					(
						neo,
						"LGL_WiimoteMotion|%i|%.4f",
						a,
						LGL_SecondsSinceLastFrame()
					);
					for(unsigned int p=0;p<motion.size();p++)
					{
						sprintf
						(
							&(neo[strlen(neo)]),
							"|%.3f|%.3f",
							motion[p].GetX(),
							motion[p].GetY()
						);
					}
					sprintf
					(
						&(neo[strlen(neo)]),
						"\n"
					);
					LGL.DrawLog.push_back(neo);
				}
			}
		}

		neo=new char[1024];
		sprintf
		(
			neo,
			"LGL_WiimotePointers|%i|%i|%i|%i|%i|%i|%i|%i\n",
			LGL_GetWiimote(0).GetPointerAvailable(),
			LGL_GetWiimote(1).GetPointerAvailable(),
			LGL_GetWiimote(2).GetPointerAvailable(),
			LGL_GetWiimote(3).GetPointerAvailable(),
			LGL_GetWiimote(4).GetPointerAvailable(),
			LGL_GetWiimote(5).GetPointerAvailable(),
			LGL_GetWiimote(6).GetPointerAvailable(),
			LGL_GetWiimote(7).GetPointerAvailable()
		);
		LGL.DrawLog.push_back(neo);

		//Write contents of LGL.DrawLog to file in a single fwrite()
		long bytes=0;
		long dataBytes=0;
/*
char typeNames[1024][1024];
long typeBytes[1024];
long typeCount[1024];
for(int a=0;a<1024;a++)
{
	typeNames[a][0]='\0';
	typeBytes[a]=0;
	typeCount[a]=0;
}
*/
		for(unsigned int a=0;a<LGL.DrawLog.size();a++)
		{
/*
char type[2048];
assert(strlen(LGL.DrawLog[a])<2048);
strcpy(type,LGL.DrawLog[a]);
if(strstr(type,"|"))
{
	strstr(type,"|")[0]='\0';
}
else if(strlen(type)>0 && type[strlen(type)-1]=='\n')
{
	type[strlen(type)-1]='\0';
}
for(int z=0;z<1024;z++)
{
	if(typeNames[z][0]=='\0')
	{
		strcpy(typeNames[z],type);
	}
	if(strcasecmp(typeNames[z],type)==0)
	{
		typeBytes[z]+=strlen(LGL.DrawLog[a]);
		typeCount[z]++;
		break;
	}
}
*/
			bytes+=strlen(LGL.DrawLog[a]);
			//printf("LGL.DrawLog[%i]: '%s'\n",a,LGL.DrawLog[a]);
		}
		for(unsigned int a=0;a<LGL.DrawLogDataLength.size();a++)
		{
			dataBytes+=LGL.DrawLogDataLength[a]+1;	//+1 is for the trailing \n
		}
		bytes+=dataBytes;
/*
printf("LGL_DrawLog.bytes(): (%li Bytes per frame) (%.3f KB per second) (%.3f MB per Minute) (%.3f GB per Hour)\n",bytes,((double)bytes*60)/1024.0f,(((double)bytes)*60*60)/(1024.0f*1024.0f),(((double)bytes)*60*60*60)/(1024.0f*1024.0f*1024.0f));
int biggestType=-1;
for(int z=0;z<1024;z++)
{
	if(typeNames[z][0]=='\0')
	{
		break;
	}
	if(biggestType==-1 || (typeBytes[z] > typeBytes[biggestType]))
	{
		biggestType=z;
	}
	
	if(0 && strcasecmp("f",typeNames[z])==0)
	{
		biggestType=z;
		break;
	}
}
if(biggestType>=0) printf("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tLGL_DrawLog.biggestType(): '%s' (Count: %li) (Bytes / Count: %.2f) (Pct Bytes: %.2f)\n",typeNames[biggestType],typeCount[biggestType],typeBytes[biggestType]/(float)typeCount[biggestType],typeBytes[biggestType]/(float)(bytes-dataBytes));
*/

		char* data=new char[bytes];
		long bytesRunningTotal=0;
		long bytesNow=0;
		for(unsigned int a=0;a<LGL.DrawLog.size();a++)
		{
			bytesNow=strlen(LGL.DrawLog[a]);
			memcpy(&(data[bytesRunningTotal]),LGL.DrawLog[a],bytesNow);
			bytesRunningTotal+=bytesNow;

			if
			(
				strlen(LGL.DrawLog[a]) > 1 &&
				LGL.DrawLog[a][0]=='D' &&
				LGL.DrawLog[a][1]=='|'
			)
			{
				//We must write the referenced data entry

				bytesNow=LGL.DrawLogDataLength[0];
				memcpy(&(data[bytesRunningTotal]),LGL.DrawLogData[0],bytesNow);
				bytesRunningTotal+=bytesNow;

				//Add a final \n for good measure.

				data[bytesRunningTotal]='\n';
				bytesRunningTotal++;

				delete LGL.DrawLogData[0];
				LGL.DrawLogData[0]=NULL;
				LGL.DrawLogData.erase(LGL.DrawLogData.begin());
				LGL.DrawLogDataLength.erase(LGL.DrawLogDataLength.begin());
			}
		}

		assert(bytesRunningTotal==bytes);

		fwrite(data,bytes,1,LGL.DrawLogFD);
		fflush(LGL.DrawLogFD);
		delete data;

		for(unsigned int a=0;a<LGL.DrawLog.size();a++)
		{
			if(LGL.DrawLog[a]!=NULL)
			{
				delete LGL.DrawLog[a];
				LGL.DrawLog[a]=NULL;
			}
		}
		for(unsigned int a=0;a<LGL.DrawLogData.size();a++)
		{
			if(LGL.DrawLogData[a]!=NULL)
			{
				delete LGL.DrawLogData[a];
				LGL.DrawLogData[a]=NULL;
			}
		}
		LGL.DrawLog.clear();
		LGL.DrawLogData.clear();
		LGL.DrawLogDataLength.clear();
	}

	lgl_MidiUpdate();

	if(LGL_GetXponent())
	{
		LGL_GetXponent()->LGL_INTERNAL_SwapBuffers();
	}
	if(LGL_GetXsession())
	{
		LGL_GetXsession()->LGL_INTERNAL_SwapBuffers();
	}
	if(LGL_GetTriggerFinger())
	{
		LGL_GetTriggerFinger()->LGL_INTERNAL_SwapBuffers();
	}
	if(LGL_GetJP8k())
	{
		LGL_GetJP8k()->LGL_INTERNAL_SwapBuffers();
	}

	lgl_font_gl_buffer_ptr = &(lgl_font_gl_buffer[0]);
	lgl_font_gl_buffer_int_ptr = &(lgl_font_gl_buffer_int[0]);

	if(LGL.RecordMovie)
	{
		int hours;
		int minutes;
		int seconds;
		int ms;

		ms=(int)
			(
				1000*LGL_SecondsSinceExecution()-
				1000*LGL.RecordMovieSecondsSinceExecutionInitial
			);

		seconds=(int)((ms-(ms%1000))/1000.0);
		ms=ms%1000;

		minutes=(int)((seconds-(seconds%60))/60.0);
		seconds=seconds%60;

		hours=(int)((minutes-(minutes%60))/60.0);
		minutes=minutes%60;

		printf("LGL_RecordMovie: %.2ih:%2im:%.2is.%.3ims\n",hours,minutes,seconds,ms);

		char filename[2048];
		sprintf
		(
			filename,
			"%s%.2ih %.2im %.2is %.3ims.bmp",
			LGL.RecordMovieFileNamePrefix,
			hours,minutes,seconds,ms
		);
		char filename2[2048];
		sprintf
		(
			filename2,
			"%s%.2ih %.2im %.2is %.3ims.png",
			LGL.RecordMovieFileNamePrefix,
			hours,minutes,seconds,ms
		);

		LGL_ScreenShot(filename);
#ifdef	LGL_LINUX
		char command[2048];
		sprintf(command,"convert \"%s\" \"%s\" && rm \"%s\"",filename,filename2,filename);
		system(command);
#endif	//LGL_LINUX
	}

	//LGL_ClearBackBuffer();

	if(LGL.RecordMovie)
	{
		LGL.FPS=LGL.RecordMovieFPS;
		LGL.FPSCounter=0;
		LGL.FPSTimer.Reset();

		//LGL.SecondsSinceLastFrame=1.0/LGL.RecordMovieFPS;
		//LGL.SecondsSinceLastFrameTimer.Reset();
	}
	LGL.SecondsSinceExecution=LGL.SecondsSinceExecutionTimer.SecondsSinceLastReset();
}

void
LGL_SwapBuffers(bool endFrame, bool clearBackBuffer)
{
	if(LGL.DisplayNow==0)
	{
		LGL_ScopeLock lock(__FILE__,__LINE__,LGL.DebugPrintfSemaphore,0.0f);
		if(lock.GetLockObtained())
		{
			for(unsigned int a=0;a<LGL.DebugPrintfBuffer.size();a++)
			{
				lgl_DebugPrintfInternal(LGL.DebugPrintfBuffer[a]);
				delete LGL.DebugPrintfBuffer[a];
				LGL.DebugPrintfBuffer[a]=NULL;
			}
			LGL.DebugPrintfBuffer.clear();
		}
	}

	if(endFrame)
	{
		for(unsigned int a=0;a<LGL.ledHostList.size();a++)
		{
			LGL.ledHostList[a]->Send();
		}
	}

#ifdef	LGL_NO_GRAPHICS
	LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());
#else
	{
		bool vsync=endFrame;//true;
		if
		(
			lgl_HideProjectorWindows &&
			LGL.DisplayNow==0 &&
			lgl_FakeSecondDisplay==false
		)
		{
			vsync=true;
		}
		/*
		if(LGL.DisplayNow==0 && LGL.DisplayCount>0)
		{
			vsync=false;
		}
		*/
		SDL_GL_SetSwapInterval(vsync);	//VSYNC
		if(vsync)
		{
			float frameTime=LGL.SecondsSinceLastFrameTimer.SecondsSinceLastReset();
			for(int a=0;a<59;a++)
			{
				LGL.FrameTimeGraph[a]=LGL.FrameTimeGraph[a+1];
			}
			LGL.FrameTimeGraph[59]=frameTime;
		}
		//Enforce sub60fps
		if(LGL.FPSMax<=30 && vsync)
		{
			for(;;)
			{
				float frameTime=LGL.SecondsSinceLastFrameTimer.SecondsSinceLastReset();
				if(frameTime<1.5f/60.0f)
				{
					LGL_DelayMS(1);
				}
				else
				{
					break;
				}
			}
		}
		SDL_GL_SwapWindow(LGL.WindowID[LGL.DisplayNow]);
		if(vsync)
		{
			glFlush();
			LGL.FramesSinceExecution++;
			LGL.SecondsSinceLastFrame=LGL.SecondsSinceLastFrameTimer.SecondsSinceLastReset();
			//Quantize to n/60.0f
			for(int a=0;a<10;a++)
			{
				//if(LGL.SecondsSinceLastFrame<0.5f*((a+1.0f)/60.0f+(a+2.0f)/60.0f))
				if(LGL.SecondsSinceLastFrame<(a+2.0f)/60.0f)
				{
					LGL.SecondsSinceLastFrame=(a+1.0f)/60.0f;
					break;
				}
			}
			LGL.SecondsSinceLastFrameTimer.Reset();

			for(int a=0;a<59;a++)
			{
				LGL.FPSGraph[a]=LGL.FPSGraph[a+1];
			}
			LGL.FPSGraph[59]=1.0/LGL.SecondsSinceLastFrame;
			if(LGL.SecondsSinceLastFrame<=1.5f/60.0f)
			{
				LGL.FrameTimeGoodCount++;
				LGL.FrameTimeGraph[59]=LGL_Min(LGL.FrameTimeGraph[59],1.0f/60.0f);
			}
			else if(LGL.SecondsSinceLastFrame<=2.5f/60.0f)
			{
				LGL.FrameTimeMediumCount++;
				LGL.FrameTimeGraph[59]=LGL_Min(LGL.FrameTimeGraph[59],2.0f/60.0f);
			}
			else
			{
				LGL.FrameTimeBadCount++;
			}
			LGL.FPSGraphTimer.Reset();

			LGL.FPSCounter++;
			if(LGL.FPSTimer.SecondsSinceLastReset()>=1)
			{
				LGL.FPS=LGL.FPSCounter;
				LGL.FPSCounter=0;
				LGL.FPSTimer.Reset();

				
				LGL.FrameTimeMin=1.0f;
				LGL.FrameTimeAve=0.0f;
				LGL.FrameTimeMax=0.0f;
				for(int a=0;a<60;a++)
				{
					LGL.FrameTimeMin=LGL_Min(LGL.FrameTimeGraph[a],LGL.FrameTimeMin);
					LGL.FrameTimeAve+=LGL.FrameTimeGraph[a];
					LGL.FrameTimeMax=LGL_Max(LGL.FrameTimeGraph[a],LGL.FrameTimeMax);
				}
				LGL.FrameTimeAve/=60.0f;
				LGL.FrameTimeGoodTotal=LGL.FrameTimeGoodCount;
				LGL.FrameTimeMediumTotal=LGL.FrameTimeMediumCount;
				LGL.FrameTimeBadTotal=LGL.FrameTimeBadCount;
				LGL.FrameTimeGoodCount=0;
				LGL.FrameTimeMediumCount=0;
				LGL.FrameTimeBadCount=0;
			}
		}
		if(clearBackBuffer)
		{
			LGL_ClearBackBuffer();
		}
		/*
		int activeDisplayPrev=LGL_GetActiveDisplay();
		for(int d=0;d<LGL.DisplayCount;d++)
		{
			LGL_SetActiveDisplay(d);
			bool vsync=true;
			if(d==0 && LGL.DisplayCount>0)
			{
				vsync=false;
			}
			SDL_GL_SetSwapInterval(vsync);	//VSYNC
			SDL_GL_SwapWindow(LGL.WindowID[d]);
			LGL_ClearBackBuffer();
		}
		LGL_SetActiveDisplay(activeDisplayPrev);
		*/
	}

#endif	//LGL_NO_GRAPHICS

	if(endFrame)
	{
		int activeDisplayPrev=LGL_GetActiveDisplay();
		LGL.DisplayNow=0;	//Fake out
		lgl_EndFrame();
		LGL.DisplayNow=activeDisplayPrev;
	}
}

void
LGL_FullScreenToggle()
{
#ifdef	LGL_OSX
	//Doesn't work in OSX...
	return;
#endif	//LGL_OSX
	SDL_Surface *S;

	S = SDL_GetVideoSurface();

	if
	(
		S==NULL ||
		(SDL_WM_ToggleFullScreen(S)!=1)
	)
	{
		printf
		(
			"LGL_FullScreenToggle(): Error. %s\n",
			SDL_GetError()
		);
	}
	LGL.WindowFullscreen=!LGL.WindowFullscreen;
}

void
LGL_FullScreen
(
	bool	inFullScreen
)
{
	if(inFullScreen!=LGL.WindowFullscreen)
	{
		LGL.WindowFullscreen=!LGL.WindowFullscreen;
		LGL_FullScreenToggle();
	}
}

bool
LGL_IsFullScreen()
{
	return(LGL.WindowFullscreen);
}

int
LGL_WindowResolutionX()
{
	return(LGL.WindowResolutionX[LGL.DisplayNow]);
}

int
LGL_WindowResolutionY()
{
	return(LGL.WindowResolutionY[LGL.DisplayNow]);
}

int
LGL_DisplayCount()
{
	return(LGL.DisplayCount);
}

int
LGL_DisplayResolutionX(int which)
{
	if(which<0 || which>=LGL_DisplayCount())
	{
		which=LGL_GetActiveDisplay();
	}
	return(LGL.DisplayResolutionX[which]);
}

int
LGL_DisplayResolutionY(int which)
{
	if(which<0 || which>=LGL_DisplayCount())
	{
		which=LGL_GetActiveDisplay();
	}
	return(LGL.DisplayResolutionY[which]);
}

int
LGL_DisplayRefreshRate(int which)
{
	if(which<0 || which>=LGL_DisplayCount())
	{
		which=LGL_GetActiveDisplay();
	}
	return(LGL.DisplayRefreshRate[which]);
}

float
LGL_WindowAspectRatio()
{
	return
	(
		LGL.WindowResolutionX[LGL.DisplayNow]/(float)
		LGL.WindowResolutionY[LGL.DisplayNow]
	);
}

float
LGL_DisplayAspectRatio(int which)
{
	if(which<0 || which>=LGL_DisplayCount())
	{
		which=LGL_GetActiveDisplay();
	}
	return
	(
		LGL.DisplayResolutionX[which]/(float)
		LGL.DisplayResolutionY[which]
	);
}

int
LGL_GetActiveDisplay()
{
	return(LGL.DisplayNow);
}

void
LGL_SetActiveDisplay(int display)
{
	display=(int)(LGL_Clamp(0,display,LGL.DisplayCount-1));
	LGL.DisplayNow=display;

	SDL_GL_MakeCurrent(LGL.WindowID[LGL.DisplayNow], LGL.GLContext);

	float left=	(0.0f-LGL.DisplayViewportLeft[display])*(1.0f/(LGL.DisplayViewportRight[display]-LGL.DisplayViewportLeft[display]));
	float right=	(1.0f-LGL.DisplayViewportLeft[display])*(1.0f/(LGL.DisplayViewportRight[display]-LGL.DisplayViewportLeft[display]));
	float bottom=	(0.0f-LGL.DisplayViewportBottom[display])*(1.0f/(LGL.DisplayViewportTop[display]-LGL.DisplayViewportBottom[display]));
	float top=	(1.0f-LGL.DisplayViewportBottom[display])*(1.0f/(LGL.DisplayViewportTop[display]-LGL.DisplayViewportBottom[display]));

	LGL_ViewportDisplay
	(
		left,
		right,
		bottom,
		top
	);
}

bool
LGL_VSync()
{
	return(LGL.VSync);
}

inline
void
lgl_glScreenify2D()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(LGL.ViewportLeft,LGL.ViewportRight,LGL.ViewportBottom,LGL.ViewportTop,0,1);
}

void
lgl_glScreenify3D()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 20.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
#ifdef	LGL_LINUX
	/*
	gluLookAt
	(
		LGL.ViewportEyeX,LGL.ViewportEyeY,LGL.ViewportEyeZ,
		LGL.ViewportTargetX,LGL.ViewportTargetY,LGL.ViewportTargetZ,
		LGL.ViewportUpX,LGL.ViewportUpY,LGL.ViewportUpZ
	);
	*/
	printf("lgl_glScreenify3D(): Warning! Function not yet implemented in linux...\n");
#else	//LGL_LINUX
//FIXME: Windows port canot use lgl_glScreenify3D(), due to a libglu32.a mingw32 linker error
	printf("lgl_glScreenify3D(): Warning! Function not yet implemented in win32...\n");
#endif	//LGL_LINUX
}

void
LGL_ViewportDisplay
(
	float	left,
	float	right,
	float	bottom,
	float	top
)
{
	if(LGL.DrawLogFD && !LGL.DrawLogPause)
	{
		LGL_DrawLogWrite
		(
			"LGL_ViewportDisplay|%.3f|%.3f|%.3f|%.3f\n",
			left,
			right,
			bottom,
			top
		);
	}
	LGL.ViewportLeft=left;
	LGL.ViewportRight=right;
	LGL.ViewportBottom=bottom;
	LGL.ViewportTop=top;
}

void
LGL_GetViewportDisplay
(
	float&	left,
	float&	right,
	float&	bottom,
	float&	top
)
{
	left=LGL.ViewportLeft;
	right=LGL.ViewportRight;
	bottom=LGL.ViewportBottom;
	top=LGL.ViewportTop;
}

void
LGL_ClipRectEnable
(
	float	left,
	float	right,
	float	bottom,
	float	top
)
{
	if(LGL.DrawLogFD && !LGL.DrawLogPause)
	{
		char* neo=new char[512];
		sprintf
		(
			neo,
			"ce|%.4f|%.4f|%.4f|%.4f\n",
			left,right,
			bottom,top
		);
		LGL.DrawLog.push_back(neo);
	}

	int sLeft = LGL.WindowResolutionX[LGL.DisplayNow]*(LGL.DisplayViewportLeft[LGL.DisplayNow] + left *
		(
			LGL.DisplayViewportRight[LGL.DisplayNow] -
			LGL.DisplayViewportLeft[LGL.DisplayNow]
		));
	int sRight = LGL.WindowResolutionX[LGL.DisplayNow]*(LGL.DisplayViewportLeft[LGL.DisplayNow] + right *
		(
			LGL.DisplayViewportRight[LGL.DisplayNow] -
			LGL.DisplayViewportLeft[LGL.DisplayNow]
		));
	int sBottom = LGL.WindowResolutionY[LGL.DisplayNow]*(LGL.DisplayViewportBottom[LGL.DisplayNow] + bottom *
		(
			LGL.DisplayViewportTop[LGL.DisplayNow] -
			LGL.DisplayViewportBottom[LGL.DisplayNow]
		));
	int sTop = LGL.WindowResolutionY[LGL.DisplayNow]*(LGL.DisplayViewportBottom[LGL.DisplayNow] + top *
		(
			LGL.DisplayViewportTop[LGL.DisplayNow] -
			LGL.DisplayViewportBottom[LGL.DisplayNow]
		));

	glEnable(GL_SCISSOR_TEST);
	if(LGL.WindowFullscreen)
	{
		glScissor
		(
			sLeft,
			sBottom,
			sRight-sLeft,
			sTop-sBottom
		);
	}
	else
	{
		glScissor
		(
			(int)(left*LGL.WindowResolutionX[LGL.DisplayNow]),
			(int)(bottom*LGL.WindowResolutionY[LGL.DisplayNow]),
			(int)((right-left)*LGL.WindowResolutionX[LGL.DisplayNow]),
			(int)((top-bottom)*LGL.WindowResolutionY[LGL.DisplayNow])
		);
	}
}

void
LGL_ClipRectDisable()
{
	if(LGL.DrawLogFD && !LGL.DrawLogPause)
	{
		char* neo=new char[1024];
		sprintf
		(
			neo,
			"cd\n"
		);
		LGL.DrawLog.push_back(neo);
	}
	glDisable(GL_SCISSOR_TEST);
}

void
LGL_DrawLineToScreen
(
	float	x1,	float	y1,
	float	x2,	float	y2,
	float	r,	float	g,	float	b,	float	a,
	float	thickness,
	bool	antialias
)
{
#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS

#if 1
	float pointsXY[4];
	pointsXY[0]=x1;
	pointsXY[1]=y1;
	pointsXY[2]=x2;
	pointsXY[3]=y2;
	LGL_DrawLineStripToScreen
	(
		pointsXY,
		2,
		r,
		g,
		b,
		a,
		thickness,
		antialias
	);
#else
	lgl_glScreenify2D();
	
	if(LGL.DrawLogFD && !LGL.DrawLogPause)
	{
		char* neo=new char[1024];
		sprintf
		(
			neo,
			"LGL_DrawLineToScreen|%.3f|%.3f|%.3f|%.3f|%.2f|%.2f|%.2f|%.2f|%.1f|%i\n",
			x1,y1,
			x2,y2,
			r,g,b,a,
			thickness,
			antialias?1:0
		);
		LGL.DrawLog.push_back(neo);
	}

	//Draw

	if(antialias)
	{
		glEnable(GL_LINE_SMOOTH);
	}
	else
	{
		glDisable(GL_LINE_SMOOTH);
	}

	glColor4f(r,g,b,a);
	glLineWidth(thickness);
	if(a<1)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_LINES);
	{
		glNormal3f(0,0,-1);
		glVertex2f(x1,y1);
		glVertex2f(x2,y2);
	}
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
#endif
}

void
LGL_DrawLineStripToScreen
(
	float*	pointsXY,
	int	pointCount,
	float	r,
	float	g,
	float	b,
	float	a,
	float	thickness,
	bool	antialias
)
{
#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS

	/*
	float* p2 = new float[pointCount*2*2];
	float* c2 = new float[pointCount*2*4];
	for(int z=0;z<pointCount*2;z+=2)
	{
		//Bottom
		p2[2*z+0] = pointsXY[z+0];//-(thickness/LGL_WindowResolutionX());
		p2[2*z+1] = pointsXY[z+1]-(0.5f*thickness/LGL_WindowResolutionY());

		c2[4*z+0] = r;
		c2[4*z+1] = g;
		c2[4*z+2] = b;
		c2[4*z+3] = a;
		
		//Top
		p2[2*z+2] = pointsXY[z+0];//+(thickness/LGL_WindowResolutionX());
		p2[2*z+3] = pointsXY[z+1]+(0.5f*thickness/LGL_WindowResolutionY());

		c2[4*z+4] = r;
		c2[4*z+5] = g;
		c2[4*z+6] = b;
		c2[4*z+7] = a;
	}

	LGL_DrawTriStripToScreen
	(
		p2,
		c2,
		pointCount*2,
		antialias
	);
	delete p2;
	delete c2;
	return;
	*/

	if(LGL.DrawLogFD && !LGL.DrawLogPause)
	{
		long datalen=sizeof(float)*2*pointCount;

		char* neo=new char[1024];
		sprintf
		(
			neo,
			"LGL_DrawLineStripToScreen|%i|%f|%f|%f|%f|%f|%i\n",
			pointCount,
			r,g,b,a,
			thickness,
			antialias?1:0
		);
		LGL.DrawLog.push_back(neo);
		
		neo=new char[64];
		sprintf(neo,"D|%li\n",datalen);
		LGL.DrawLog.push_back(neo);

		char* data=new char[datalen];
		memcpy(data,pointsXY,datalen);
		LGL.DrawLogData.push_back(data);
		LGL.DrawLogDataLength.push_back(datalen);
	}

	//Slow
	/*
	for(int z=0;z<pointCount-1;z++)
	{
		LGL_DrawLineToScreen
		(
			pointsXY[((z+0)*2)+0],pointsXY[((z+0)*2)+1],
			pointsXY[((z+1)*2)+0],pointsXY[((z+1)*2)+1],
			r,g,b,a,
			thickness,
			antialias
		);
	}
	*/

	//Fast
	lgl_glScreenify2D();

	if(antialias)
	{
		glEnable(GL_LINE_SMOOTH);
	}
	else
	{
		glDisable(GL_LINE_SMOOTH);
	}

	glColor4f(r,g,b,a);
	glLineWidth(thickness);
	if(a<1)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glNormal3f(0,0,-1);

	glVertexPointer
	(
		2,
		GL_FLOAT,
		0,
		pointsXY
	);
	glDrawArrays
	(
		GL_LINE_STRIP,
		0,
		pointCount
	);

	glDisableClientState(GL_VERTEX_ARRAY);

	glDisable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
}

void
LGL_DrawLineStripToScreen
(
	float*	pointsXY,
	float*	colorsRGBA,
	int	pointCount,
	float	thickness,
	bool	antialias
)
{
#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS

	/*
	float* p2 = new float[pointCount*2*2];
	float* c2 = new float[pointCount*2*4];
	for(int a=0;a<pointCount*2;a+=2)
	{
		//Bottom
		p2[2*a+0] = pointsXY[a+0];//-(thickness/LGL_WindowResolutionX());
		p2[2*a+1] = pointsXY[a+1]-(thickness/LGL_WindowResolutionY());

		c2[4*a+0] = colorsRGBA[2*a+0];
		c2[4*a+1] = colorsRGBA[2*a+1];
		c2[4*a+2] = colorsRGBA[2*a+2];
		c2[4*a+3] = colorsRGBA[2*a+3];
		
		//Top
		p2[2*a+2] = pointsXY[a+0];//+(thickness/LGL_WindowResolutionX());
		p2[2*a+3] = pointsXY[a+1]+(thickness/LGL_WindowResolutionY());

		c2[4*a+4] = colorsRGBA[2*a+0];
		c2[4*a+5] = colorsRGBA[2*a+1];
		c2[4*a+6] = colorsRGBA[2*a+2];
		c2[4*a+7] = colorsRGBA[2*a+3];

	}
	LGL_DrawTriStripToScreen
	(
		p2,
		c2,
		pointCount*2,
		antialias
	);
	delete p2;
	delete c2;
	return;
	*/

	if(LGL.DrawLogFD && !LGL.DrawLogPause)
	{
		long datalenPoints=sizeof(float)*2*pointCount;
		long datalenColors=sizeof(float)*4*pointCount;

		char* neo=new char[1024];
		sprintf
		(
			neo,
			"LGL_DrawLineStripToScreenColors|%i|%f|%i\n",
			pointCount,
			thickness,
			antialias?1:0
		);
		LGL.DrawLog.push_back(neo);

		neo=new char[64];
		sprintf(neo,"D|%li\n",datalenPoints);
		LGL.DrawLog.push_back(neo);

		char* data=new char[datalenPoints];
		memcpy(data,pointsXY,datalenPoints);
		LGL.DrawLogData.push_back(data);
		LGL.DrawLogDataLength.push_back(datalenPoints);

		neo=new char[64];
		sprintf(neo,"D|%li\n",datalenColors);
		LGL.DrawLog.push_back(neo);

		data=new char[datalenColors];
		memcpy(data,colorsRGBA,datalenColors);
		LGL.DrawLogData.push_back(data);
		LGL.DrawLogDataLength.push_back(datalenColors);
	}

	lgl_glScreenify2D();

	if(antialias)
	{
		glEnable(GL_LINE_SMOOTH);
	}
	else
	{
		glDisable(GL_LINE_SMOOTH);
	}

	glLineWidth(thickness);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glNormal3f(0,0,-1);

	glColorPointer
	(
		4,
		GL_FLOAT,
		0,
		colorsRGBA
	);
	glVertexPointer
	(
		2,
		GL_FLOAT,
		0,
		pointsXY
	);
	glDrawArrays
	(
		GL_LINE_STRIP,
		0,
		pointCount
	);
	
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glDisable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
}

void
LGL_DrawTriStripToScreen
(
	float*	pointsXY,
	float*	colorsRGBA,
	int	pointCount,
	bool	antialias
)
{
#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS
	if(LGL.DrawLogFD && !LGL.DrawLogPause)
	{
		long datalenPoints=sizeof(float)*2*pointCount;
		long datalenColors=sizeof(float)*4*pointCount;

		char* neo=new char[1024];
		sprintf
		(
			neo,
			"LGL_DrawTriStripToScreenColors|%i|%i\n",
			pointCount,
			antialias?1:0
		);
		LGL.DrawLog.push_back(neo);

		neo=new char[64];
		sprintf(neo,"D|%li\n",datalenPoints);
		LGL.DrawLog.push_back(neo);

		char* data=new char[datalenPoints];
		memcpy(data,pointsXY,datalenPoints);
		LGL.DrawLogData.push_back(data);
		LGL.DrawLogDataLength.push_back(datalenPoints);

		neo=new char[64];
		sprintf(neo,"D|%li\n",datalenColors);
		LGL.DrawLog.push_back(neo);

		data=new char[datalenColors];
		memcpy(data,colorsRGBA,datalenColors);
		LGL.DrawLogData.push_back(data);
		LGL.DrawLogDataLength.push_back(datalenColors);
	}

	lgl_glScreenify2D();

	if(antialias)
	{
		glEnable(GL_LINE_SMOOTH);
	}
	else
	{
		glDisable(GL_LINE_SMOOTH);
	}

	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glNormal3f(0,0,-1);

	glColorPointer
	(
		4,
		GL_FLOAT,
		0,
		colorsRGBA
	);
	glVertexPointer
	(
		2,
		GL_FLOAT,
		0,
		pointsXY
	);
	glDrawArrays
	(
		GL_TRIANGLE_STRIP,
		0,
		pointCount
	);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glDisable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
}

void
LGL_DrawRectToScreen
(
	float left, float right,
	float bottom, float top,
	float r, float g, float b, float a,
	float rotation
)
{
#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS
	lgl_glScreenify2D();

	if(LGL.DrawLogFD && !LGL.DrawLogPause)
	{
		char* neo=new char[1024];
		sprintf
		(
			neo,
			"dr|%.4f|%.4f|%.4f|%.4f|%.2f|%.2f|%.2f|%.2f|%.2f\n",
			left,right,
			bottom,top,
			r,g,b,a,
			rotation
		);
		LGL.DrawLog.push_back(neo);
	}

	float width=right-left;
	float height=top-bottom;
	
	glTranslatef(left+width*.5,bottom+height*.5,0);
	glRotatef(360.0*(rotation/(LGL_PI*2)),0,0,1);
	
	//Draw
	
	glColor4f(r,g,b,a);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
	{
		glNormal3f(0,0,-1);
		glVertex2f(.5*-width,-.5*height);
		glVertex2f(.5*width,-.5*height);
		glVertex2f(.5*width,.5*height);
		glVertex2f(-.5*width,.5*height);
	}
	glEnd();

	glDisable(GL_BLEND);
}

void
LGL_ViewportWorld
(
	float EyeX, float EyeY, float EyeZ,
	float TargetX, float TargetY, float TargetZ,
	float UpX, float UpY, float UpZ
)
{
	LGL.ViewportEyeX=EyeX;
	LGL.ViewportEyeY=EyeY;
	LGL.ViewportEyeZ=EyeZ;
	
	LGL.ViewportTargetX=TargetX;
	LGL.ViewportTargetY=TargetY;
	LGL.ViewportTargetZ=TargetZ;
	
	LGL.ViewportUpX=UpX;
	LGL.ViewportUpY=UpY;
	LGL.ViewportUpZ=UpZ;
}

void
LGL_DrawLineToWorld
(
	float	x1,	float	y1,	float	z1,
	float	x2,	float	y2,	float	z2,
	float	r,	float	g,	float	b,	float	a,
	float	thickness,
	bool	antialias
)
{
	lgl_glScreenify3D();

	if(LGL.DrawLogFD && !LGL.DrawLogPause)
	{
		char* neo=new char[1024];
		sprintf
		(
			neo,
			"LGL_DrawLineToWorld|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%i\n",
			x1,y1,z1,
			x2,y2,z2,
			r,g,b,a,
			thickness,
			antialias?1:0
		);
		LGL.DrawLog.push_back(neo);
	}

	//Draw
	
	if(antialias)
	{
		glEnable(GL_LINE_SMOOTH);
	}
	else
	{
		glDisable(GL_LINE_SMOOTH);
	}

	glColor4f(r,g,b,a);
	glLineWidth(thickness);
	if(a<1)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_LINES);
	{
		glNormal3f(0,0,-1);
		glVertex3f(x1,y1,z1);
		glVertex3f(x2,y2,z2);
	}
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
}

void
LGL_DrawTriToWorld
(
	float	x1,	float	y1,	float z1,
	float	x2,	float	y2,	float z2,
	float	x3,	float	y3,	float z3,
	float	r,	float	g,	float	b,	float	a
)
{
	lgl_glScreenify3D();
	
	if(LGL.DrawLogFD && !LGL.DrawLogPause)
	{
		char* neo=new char[1024];
		sprintf
		(
			neo,
			"LGL_DrawTriToWorld|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f\n",
			x1,y1,z1,
			x2,y2,z2,
			x3,y3,z3,
			r,g,b,a
		);
		LGL.DrawLog.push_back(neo);
	}

	//Draw

	glColor4f(r,g,b,a);
	if(a<1)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_LINE_LOOP);
	{
		glNormal3f(0,0,-1);
		glVertex3f(x1,y1,z1);
		glVertex3f(x2,y2,z2);
		glVertex3f(x3,y3,z3);
	}
	glEnd();

	glDisable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
}

void
LGL_SmoothLines
(
	bool	in
)
{
	if(in)
	{
		glEnable(GL_LINE_SMOOTH);
	}
	else
	{
		glDisable(GL_LINE_SMOOTH);
	}
}

void
LGL_SmoothPolygons
(
	bool	in
)
{
	if(in)
	{
		glEnable(GL_POLYGON_SMOOTH);
	}
	else
	{
		glDisable(GL_POLYGON_SMOOTH);
	}
}

void
LGL_ClearBackBuffer()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

float
LGL_TextureUsageMB()
{
	return(LGL.TexturePixels/(float)(4*32*1024));
}

void
LGL_BlockOnGPU()
{
	glFinish();
}

float
LGL_StatsLinesPerFrame()
{
	return(LGL.StatsLinesPerFrame);
}

float
LGL_StatsRectsPerFrame()
{
	return(LGL.StatsRectsPerFrame);
}

float
LGL_StatsImagesPerFrame()
{
	return(LGL.StatsImagesPerFrame);
}

float
LGL_StatsPixelsPerFrame()
{
	return(LGL.StatsPixelsPerFrame);
}

bool
LGL_VBOAvailable()
{
#ifdef	LGL_NO_GRAPHICS
	return(false);
#endif	//LGL_NO_GRAPHICS
	return
	(
		strstr
		(
			(char*)glGetString(GL_EXTENSIONS),
			"GL_ARB_vertex_buffer_object"
		)!=NULL
	);
}

bool
LGL_ShaderAvailable()
{
#ifdef	LGL_NO_GRAPHICS
	return(false);
#endif	//LGL_NO_GRAPHICS
	return
	(
		LGL_ShaderAvailableVert() ||
		LGL_ShaderAvailableFrag()
	);
}

bool
LGL_ShaderAvailableVert()
{
#ifdef	LGL_NO_GRAPHICS
	return(false);
#endif	//LGL_NO_GRAPHICS
	return
	(
		strstr
		(
			(char*)glGetString(GL_EXTENSIONS),
			"GL_ARB_vertex_shader"
		)!=NULL
	);
}

bool
LGL_ShaderAvailableFrag()
{
#ifdef	LGL_NO_GRAPHICS
	return(false);
#endif	//LGL_NO_GRAPHICS
	return
	(
		strstr
		(
			(char*)glGetString(GL_EXTENSIONS),
			"GL_ARB_fragment_shader"
		)!=NULL
	);
}

LGL_Shader::
LGL_Shader
(
	const char*	inDescription
)
{
	SetDescription(inDescription);
	VertObject=0;
	FragObject=0;
	ProgramObject=0;
}

LGL_Shader::
~LGL_Shader()
{
	OmniDelete();
}

const char*
LGL_Shader::
GetDescription()
{
	return(Description);
}

void
LGL_Shader::
SetDescription
(
	const char*	inDescription
)
{
	strcpy(Description,inDescription);
}

bool
LGL_Shader::
VertCompile
(
	const char*	inFileVert
)
{
	if(LGL_ShaderAvailableVert()==false)
	{
		return(false);
	}

	if(VertObject!=0)
	{
		Disable();
		ProgramDelete();
		VertDelete();
	}

	FILE* file=fopen(inFileVert,"rb");
	if(file==NULL)
	{
		printf
		(
			"LGL_Shader('%s')::VertCompile(): Error! Couldn't open file '%s'...\n",
			Description,
			inFileVert
		);
		LGL_Exit();
	}
	sprintf(FileVert,"%s",inFileVert);

	VertObject=gl2CreateShaderObject(GL_VERTEX_SHADER_ARB);

	fseek(file,0,SEEK_END);
	int length=ftell(file);
	fseek(file,0,SEEK_SET);

	char* buf=new char[length+1];

	fread(buf,1,length,file);
	buf[length]='\0';

	gl2ShaderSource(VertObject,1,(const char**)(&buf),NULL);

	//These lines should NOT exist, but must for win32
	int length2=0;
	length2=length;
	//Why??
	
	gl2CompileShader(VertObject);
	GLint result=2;
	
	gl2GetObjectParameteriv(VertObject,GL_OBJECT_COMPILE_STATUS_ARB,&result);
	if(InfoLogExists(VertObject))
	{
		//
	}
	
	if(result==0)
	{
		
		printf
		(
			"LGL_Shader('%s')::VertCompile(): Error! '%s' failed to compile:\n\n",
			Description,
			inFileVert
		);
		
		printf("---\n\n");
		InfoLogPrint(VertObject);
		printf("---\n");

		LGL_Exit();
	}
	
	fclose(file);
	return(true);
}

bool
LGL_Shader::
FragCompile
(
	const char*	inFileFrag
)
{
	if(LGL_ShaderAvailableFrag()==false)
	{
		return(false);
	}

	if(FragObject!=0)
	{
		Disable();
		ProgramDelete();
		FragDelete();
	}

	FILE* file=fopen(inFileFrag,"rb");
	if(file==NULL)
	{
		printf
		(
			"LGL_Shader('%s')::FragCompile(): Error! Couldn't open file '%s'...\n",
			Description,
			inFileFrag
		);
		LGL_Exit();
	}
	sprintf(FileFrag,"%s",inFileFrag);

	FragObject=gl2CreateShaderObject(GL_FRAGMENT_SHADER_ARB);

	fseek(file,0,SEEK_END);
	int length=ftell(file);
	fseek(file,0,SEEK_SET);

	char* buf=new char[length+1];

	fread(buf,1,length,file);
	buf[length]='\0';

	gl2ShaderSource(FragObject,1,(const char**)(&buf),NULL);

	//These lines should NOT exist, but must for win32
	int length2=0;
	length2=length;
	//Why??
	
	gl2CompileShader(FragObject);
	
	GLint result=2;
	
	gl2GetObjectParameteriv(FragObject,GL_OBJECT_COMPILE_STATUS_ARB,&result);
	if(InfoLogExists(FragObject))
	{
		//
	}
	
	if(result==0)
	{
		
		printf
		(
			"LGL_Shader('%s')::FragCompile(): Error! '%s' failed to compile:\n\n",
			Description,
			inFileFrag
		);
		
		printf("---\n\n");
		InfoLogPrint(FragObject);
		printf("---\n");

		LGL_Exit();
	}

	fclose(file);
	return(true);
}

bool
LGL_Shader::
VertCompiled()
{
	return(VertObject!=0);
}

bool
LGL_Shader::
FragCompiled()
{
	return(FragObject!=0);
}

bool
LGL_Shader::
Link()
{
	if(ProgramObject!=0)
	{
		return(true);
	}

	if
	(
		VertObject==0 &&
		FragObject==0
	)
	{
		return(false);
	}

	ProgramObject=gl2CreateProgramObject();
	if(VertObject!=0) gl2AttachObject(ProgramObject,VertObject);
	if(FragObject!=0) gl2AttachObject(ProgramObject,FragObject);
	gl2LinkProgram(ProgramObject);
	gl2ValidateProgram(ProgramObject);
	GLint valid;
	gl2GetObjectParameteriv(ProgramObject,GL_OBJECT_VALIDATE_STATUS_ARB,&valid);
	if(valid==GL_FALSE)
	{
		printf
		(
			"LGL_Shader('%s')::Link(): Error! Could link program... Source files:\n",
			Description
		);
		if(strlen(FileVert)>0)
		{
			printf
			(
				"\tVert Shader: '%s'\n",
				FileVert
			);
		}
		if(strlen(FileFrag)>0)
		{
			printf
			(
				"\tFrag Shader: '%s'\n",
				FileFrag
			);
		}
		LGL_Exit();
	}
	return(true);
}

bool
LGL_Shader::
IsLinked()
{
	return(ProgramObject!=0);
}

bool
LGL_Shader::
Enable
(
	bool	enable
)
{
	if
	(
		strcasestr
		(
			(char*)glGetString(GL_EXTENSIONS),
			"GL_ARB_SHADING_LANGUAGE_100"
		)==NULL
	)
	{
		printf("LGL_Shader::Enable(): Warning! Your graphics driver compiles but cannot run GLSL programs!\n");
		printf("\tMore Info: Cannot find string 'GL_ARB_SHADING_LANGUAGE_100' in extensions:\n");
		printf("***\n%s\n***\n",(char*)glGetString(GL_EXTENSIONS));
		return(false);
	}
	if(enable==false)
	{
		return(Disable());
	}
	if(ProgramObject==0)
	{
		if(Link()==false)
		{
			return(false);
		}
	}
	if(IsEnabled()==false)
	{
		gl2UseProgramObject(ProgramObject);
	}
	LGL.ShaderProgramCurrent=ProgramObject;
	return(true);
}

bool
LGL_Shader::
Disable()
{
	if(IsEnabled()==false)
	{
		return(false);
	}
		
	gl2UseProgramObject(0);
	LGL.ShaderProgramCurrent=0;
	return(true);
}

bool
LGL_Shader::
IsEnabled()
{
	return
	(
		ProgramObject!=0 &&
		ProgramObject==LGL.ShaderProgramCurrent
	);
}

bool
LGL_Shader::
EnableToggle()
{
	if(IsEnabled())
	{
		Disable();
		return(false);
	}
	else
	{
		Enable();
		return(true);
	}
}

bool
LGL_Shader::
SetVertAttributeInt
(
	const char*	name,
	int		value0
)
{
	return
	(
		SetVertAttributeIntPrivate
		(
			name, 1,
			value0
		)
	);
}
bool
LGL_Shader::
SetVertAttributeInt
(
	const char*	name,
	int		value0,	int	value1
)
{
	return
	(
		SetVertAttributeIntPrivate
		(
			name, 2,
			value0,	value1
		)
	);
}
bool
LGL_Shader::
SetVertAttributeInt
(
	const char*	name,
	int		value0,	int	value1,	int	value2
)
{
	return
	(
		SetVertAttributeIntPrivate
		(
			name, 3,
			value0,	value1,	value2
		)
	);
}
bool
LGL_Shader::
SetVertAttributeInt
(
	const char*	name,
	int		value0,	int	value1,	int	value2,	int	value3
)
{
	return
	(
		SetVertAttributeIntPrivate
		(
			name, 4,
			value0,	value1,	value2,	value3
		)
	);
}

bool
LGL_Shader::
SetVertAttributeFloat
(
	const char*	name,
	float		value0
)
{
	return
	(
		SetVertAttributeFloatPrivate
		(
			name, 1,
			value0
		)
	);
}
bool
LGL_Shader::
SetVertAttributeFloat
(
	const char*	name,
	float		value0,	float	value1
)
{
	return
	(
		SetVertAttributeFloatPrivate
		(
			name, 2,
			value0,	value1
		)
	);
}
bool
LGL_Shader::
SetVertAttributeFloat
(
	const char*	name,
	float		value0,	float	value1,	float	value2
)
{
	return
	(
		SetVertAttributeFloatPrivate
		(
			name, 3,
			value0,	value1,	value2
		)
	);
}
bool
LGL_Shader::
SetVertAttributeFloat
(
	const char*	name,
	float		value0,	float	value1,	float	value2,	float	value3
)
{
	return
	(
		SetVertAttributeFloatPrivate
		(
			name, 4,
			value0,	value1,	value2,	value3
		)
	);
}

bool
LGL_Shader::
SetUniformAttributeInt
(
	const char*	name,
	int		value0
)
{
	return
	(
		SetUniformAttributeIntPrivate
		(
			name, 1,
			value0
		)
	);
}
bool
LGL_Shader::
SetUniformAttributeInt
(
	const char*	name,
	int		value0,	int	value1
)
{
	return
	(
		SetUniformAttributeIntPrivate
		(
			name, 2,
			value0,	value1
		)
	);
}
bool
LGL_Shader::
SetUniformAttributeInt
(
	const char*	name,
	int		value0,	int	value1,	int	value2
)
{
	return
	(
		SetUniformAttributeIntPrivate
		(
			name, 3,
			value0,	value1,	value2
		)
	);
}
bool
LGL_Shader::
SetUniformAttributeInt
(
	const char*	name,
	int		value0,	int	value1,	int	value2,	int	value3
)
{
	return
	(
		SetUniformAttributeIntPrivate
		(
			name, 4,
			value0,	value1,	value2,	value3
		)
	);
}

bool
LGL_Shader::
SetUniformAttributeFloat
(
	const char*	name,
	float		value0
)
{
	return
	(
		SetUniformAttributeFloatPrivate
		(
			name, 1,
			value0
		)
	);
}
bool
LGL_Shader::
SetUniformAttributeFloat
(
	const char*	name,
	float		value0,	float	value1
)
{
	return
	(
		SetUniformAttributeFloatPrivate
		(
			name, 2,
			value0,	value1
		)
	);
}
bool
LGL_Shader::
SetUniformAttributeFloat
(
	const char*	name,
	float		value0,	float	value1,	float	value2
)
{
	return
	(
		SetUniformAttributeFloatPrivate
		(
			name, 3,
			value0,	value1,	value2
		)
	);
}
bool
LGL_Shader::
SetUniformAttributeFloat
(
	const char*	name,
	float		value0,	float	value1,	float	value2,	float	value3
)
{
	return
	(
		SetUniformAttributeFloatPrivate
		(
			name, 4,
			value0,	value1,	value2,	value3
		)
	);
}

void
LGL_Shader::
VertDelete()
{
	Disable();
	ProgramDelete();
	if(VertObject!=0)
	{
		gl2DeleteObject(VertObject);
		VertObject=0;
		FileVert[0]='\0';
	}
}

void
LGL_Shader::
FragDelete()
{
	Disable();
	ProgramDelete();
	if(FragObject!=0)
	{
		gl2DeleteObject(FragObject);
		FragObject=0;
		FileFrag[0]='\0';
	}
}

void
LGL_Shader::
ProgramDelete()
{
	Disable();
	if(ProgramObject!=0)
	{
		gl2DeleteObject(ProgramObject);
		ProgramObject=0;
	}
}

void
LGL_Shader::
OmniDelete()
{
	ProgramDelete();
	FragDelete();
	VertDelete();
}

bool
LGL_Shader::
InfoLogExists
(
	GLhandleARB	obj
)
{
	GLint length=-1;

	gl2GetObjectParameteriv
	(
		obj,
		GL_OBJECT_INFO_LOG_LENGTH_ARB,
		&length
	);

	return(length>1);
}

bool
LGL_Shader::
InfoLogPrint
(
	GLhandleARB	obj
)
{
	GLint length=-1;

	gl2GetObjectParameteriv
	(
		obj,
		GL_OBJECT_INFO_LOG_LENGTH_ARB,
		&length
	);

	if(length<=1)
	{
		return(false);
	}

	char* log=(char*)malloc(length);
	GLsizei written=-777;

	gl2GetInfoLog(obj,length,&written,log);

	if(written<=0)
	{
		printf
		(
			"LGL_Shader('%s')::InfoLogPrint(): Error! Couldn't read log (expected %i, got %i)!\n",
			Description,
			(int)length,
			(int)written
		);
		LGL_Exit();
	}

	printf("%s",log);

	free(log);

	return(true);
}

bool
LGL_Shader::
SetVertAttributeIntPrivate
(
	const char*	name,	int	num,
	int		value0,	int	value1,
	int		value2,	int	value3
)
{
	if(IsEnabled()==false)
	{
		return(false);
	}

	int arglist[4];
	arglist[0]=value0;
	arglist[1]=value1;
	arglist[2]=value2;
	arglist[3]=value3;

	GLint index=gl2GetAttribLocation(ProgramObject,name);

	if(index==-1)
	{
		printf
		(
			"LGL_Shader('%s')::SetVertAttributeInt(): Error! "
			"Could not resolve Attribute '%s'...\n",
			Description,
			name
		);
		for(GLuint a=0;a<16;a++)
		{
			char name[1024];
			gl2GetActiveAttrib
			(
				ProgramObject,
				a,
				1024,NULL,NULL,NULL,
				name
			);
			printf("\tActiveAttrib[%i]: '%s'\n",(int)a,name);
		}
		LGL_Exit();
	}

	gl2VertexAttrib4iv(index,arglist);
	return(true);
}

bool
LGL_Shader::
SetVertAttributeFloatPrivate
(
	const char*	name,	int	num,
	float		value0,	float	value1,
	float		value2,	float	value3
)
{
	if(IsEnabled()==false)
	{
		return(false);
	}

	float arglist[4];
	arglist[0]=value0;
	arglist[1]=value1;
	arglist[2]=value2;
	arglist[3]=value3;

	GLint index=gl2GetAttribLocation(ProgramObject,name);

	if(index==-1)
	{
		printf
		(
			"LGL_Shader('%s')::SetVertAttributeFloat(): Error! "
			"Could not resolve Attribute '%s' for program %i...\n",
			Description,
			name,
			(int)ProgramObject
		);
		for(GLuint a=0;a<16;a++)
		{
			char name[1024];
			gl2GetActiveAttrib
			(
				ProgramObject,
				a,
				1024,NULL,NULL,NULL,
				name
			);
			printf("\tActiveAttrib[%i]: '%s'\n",(int)a,name);
		}
		LGL_Exit();
	}

	gl2VertexAttrib4fv(index,arglist);
	return(true);
}

bool
LGL_Shader::
SetUniformAttributeIntPrivate
(
	const char*	name,		int	num,
	int		value0,		int	value1,
	int		value2,		int	value3
)
{
	if(IsEnabled()==false)
	{
		return(false);
	}
	
	int location=gl2GetUniformLocation(ProgramObject,name);

	if(location==-1)
	{
		printf
		(
			"LGL_Shader('%s')::SetUniformAttributeInt(): Error! "
			"Could not resolve uniform variable '%s'...\n",
			Description,
			name
		);
		LGL_Exit();
	}

	if(num==1)
	{
		gl2Uniform1i(location,value0);
	}
	else if(num==2)
	{
		gl2Uniform2i(location,value0,value1);
	}
	else if(num==3)
	{
		gl2Uniform3i(location,value0,value1,value2);
	}
	else if(num==4)
	{
		gl2Uniform4i(location,value0,value1,value2,value3);
	}
	else
	{
		printf
		(
			"LGL_Shader('%s')::SetUniformAttributeInt(): Error! "
			"Num is '%i', but must be [1,4]...\n",
			Description,
			num
		);
		LGL_Exit();
	}

	return(true);
}

bool
LGL_Shader::
SetUniformAttributeFloatPrivate
(
	const char*	name,		int	num,
	float		value0,		float	value1,
	float		value2,		float	value3
)
{
	if(IsEnabled()==false)
	{
		return(false);
	}
	
	int location=gl2GetUniformLocation(ProgramObject,name);

	if(location==-1)
	{
		printf
		(
			"LGL_Shader('%s')::SetUniformAttributeFloat(): Error! "
			"Could not resolve uniform variable '%s'...\n",
			Description,
			name
		);
		LGL_Exit();
	}

	if(num==1)
	{
		gl2Uniform1f(location,value0);
	}
	else if(num==2)
	{
		gl2Uniform2f(location,value0,value1);
	}
	else if(num==3)
	{
		gl2Uniform3f(location,value0,value1,value2);
	}
	else if(num==4)
	{
		gl2Uniform4f(location,value0,value1,value2,value3);
	}
	else
	{
		printf
		(
			"LGL_Shader('%s')::SetUniformAttributeFloat(): Error! "
			"Num is '%i', but must be [1,4]...\n",
			Description,
			num
		);
		LGL_Exit();
	}
	return(true);
}

LGL_Shader LGL_Image::ImageShader("Image Shader");
LGL_Shader LGL_Image::YUV_ImageShader("YUV Image Shader");

LGL_Image::
LGL_Image
(
	const
	char*	filename,
	bool	inLinearInterpolation,
	bool	loadToGLTexture,
	int	loadToExistantGLTexture,
	int	loadToExistantGLTextureX,
	int	loadToExistantGLTextureY
)
{
	ReferenceCount=0;
	TextureGL=0;
	TextureGLRect=(GL_TEXTURE_LGL==GL_TEXTURE_RECTANGLE_ARB);

#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS

	FileLoad(filename);

	LinearInterpolation=inLinearInterpolation;
	PixelBufferEnable=true;
	PixelBufferObjectFrontGL=0;
	PixelBufferObjectBackGL=0;
	PixelBufferObjectSize=0;

	FrameBufferImage=false;
	InvertY=false;

	if(loadToGLTexture)
	{
		LoadSurfaceToTexture
		(
			LinearInterpolation,
			loadToExistantGLTexture,
			loadToExistantGLTextureX,
			loadToExistantGLTextureY
		);
		DeletePixelBufferObjects();
	}

	FrameNumber=-1;
	VideoPath[0]='\0';

	YUV_Construct();
}

LGL_Image::
LGL_Image
(
	int		width,
	int		height,
	int		bytesperpixel,
	unsigned char*	data,
	bool		inLinearInterpolation,
	const char*	name
)
{
	ReferenceCount=0;
	TextureGL=0;
	TextureGLRect=(GL_TEXTURE_LGL==GL_TEXTURE_RECTANGLE_ARB);

#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS

	assert(name[0]!='!');

	LinearInterpolation=inLinearInterpolation;
	sprintf(Path,"!%s",name);
	sprintf(PathShort,"!%s",name);

	//Load via SDL_CreateRGBSurfaceFrom to an SDL_Surface

	SDL_Surface* mySDL_Surface1=NULL;
	if(data)
	{
		mySDL_Surface1=SDL_CreateRGBSurfaceFrom
		(
			data,
			width,height,8*bytesperpixel,
			width*bytesperpixel,
			0x000000FF,
			0x0000FF00,
			0x00FF0000,
			0x00000000
		);
		if(mySDL_Surface1==NULL)
		{
			printf
			(
				"LGL_Image::LGL_Image(): Error loading '%s' to an SDL_Surface: %s\n",
				Path,
				IMG_GetError()
			);
			char wd[2048];
			getcwd(wd,2048);
			printf("Working Directory: %s\n",wd);
			LGL_Exit();
		}
	}

	AlphaChannel=bytesperpixel==4;//mySDL_Surface1->flags & SDL_SRCALPHA;
	PixelBufferEnable=true;
	PixelBufferObjectFrontGL=0;
	PixelBufferObjectBackGL=0;
	PixelBufferObjectSize=0;

	//Convert mySDL_Surface1 to the desired format for SDL_Surface2

	if(mySDL_Surface1)
	{
		SDL_PixelFormat DesiredFormat;
		DesiredFormat.palette=NULL;
		DesiredFormat.BitsPerPixel=32;
		DesiredFormat.BytesPerPixel=4;
		DesiredFormat.Rmask=0x000000FF;
		DesiredFormat.Gmask=0x0000FF00;
		DesiredFormat.Bmask=0x00FF0000;
		DesiredFormat.Amask=0xFF000000;
		DesiredFormat.Rshift=0;
		DesiredFormat.Gshift=0;
		DesiredFormat.Bshift=0;
		DesiredFormat.Ashift=0;
		DesiredFormat.Rloss=0;
		DesiredFormat.Gloss=0;
		DesiredFormat.Bloss=0;
		DesiredFormat.Aloss=0;
		//DesiredFormat.colorkey=0;
		//DesiredFormat.alpha=255;

		SurfaceSDL=SDL_ConvertSurface(mySDL_Surface1,&DesiredFormat,SDL_SWSURFACE);
		if(SurfaceSDL==NULL)
		{
			printf
			(
				"LGL_Image::LGL_Image(): Error converting '%s' to our desired format (1)...\n",
				Path
			);
			printf("SDL_Error: '%s'\n",SDL_GetError());
			assert(SurfaceSDL);
		}
	}
	else
	{
		SurfaceSDL=NULL;
	}

	ImgW=width;//SurfaceSDL->w;
	ImgH=height;//SurfaceSDL->h;
	TexW=LGL_NextPowerOfTwo(ImgW);
	TexH=LGL_NextPowerOfTwo(ImgH);

	TextureGL=0;
	TextureGLRect=(GL_TEXTURE_LGL==GL_TEXTURE_RECTANGLE_ARB);
	if(TextureGLRect)
	{
		TexW=ImgW;
		TexH=ImgH;
	}
	TextureGLMine=true;
	FrameBufferImage=false;
	InvertY=false;
	if(data)
	{
		LoadSurfaceToTexture(LinearInterpolation);
		UpdateTexture
		(
			ImgW,
			ImgH,
			bytesperpixel,
			data,
			LinearInterpolation,
			PathShort
		);
		DeletePixelBufferObjects();
	}

	//Delete temporary SDL_Surface

	if(mySDL_Surface1)
	{
		SDL_FreeSurface(mySDL_Surface1);
	}

	FrameNumber=-1;
	VideoPath[0]='\0';

	YUV_Construct();
}

LGL_Image::
LGL_Image
(
	float	left,	float	right,
	float	bottom,	float	top,
	bool	inReadFromFrontBuffer
)
{
	ReferenceCount=0;
	TextureGL=0;
	TextureGLRect=(GL_TEXTURE_LGL==GL_TEXTURE_RECTANGLE_ARB);

#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS

	LinearInterpolation=true;
	//Baka bounds checking... Never trust users.
	
	if
	(
		!(left>=0	&& left<=1) ||
		!(right>=0	&& right<=1) ||
		!(bottom>=0	&& bottom<=1) ||
		!(top>=0	&& top<=1) ||
		left>=right	|| bottom>=top
	)
	{
		printf("LGL_Image::LGL_Image(): glFrameBuffer bounds error.\n");
		printf
		(
			"(left, right), (bottom ,top) = (%.2f, %.2f), (%.2f, %.2f)\n",
			left,right,bottom,top
		);
		LGL_Assertf(false,("Bad image size!"));
		LGL_Exit();
	}
	
	sprintf(Path,"FrameBuffer Image");
	sprintf(PathShort,"FrameBuffer Image");

	AlphaChannel=false;
	PixelBufferEnable=true;
	PixelBufferObjectFrontGL=0;
	PixelBufferObjectBackGL=0;
	PixelBufferObjectSize=0;

	SurfaceSDL=NULL;

	FrameBufferImage=true;
	InvertY=false;
	ReadFromFrontBuffer=inReadFromFrontBuffer;
	FrameBufferViewport(left,right,bottom,top);

	if(TextureGLRect)
	{
		TexW=LGL.WindowResolutionX[LGL.DisplayNow];
		TexH=LGL.WindowResolutionY[LGL.DisplayNow];
	}
	else
	{
		TexW=LGL_NextPowerOfTwo(LGL.WindowResolutionX[LGL.DisplayNow]);
		TexH=LGL_NextPowerOfTwo(LGL.WindowResolutionY[LGL.DisplayNow]);
	}
	TextureGL=0;
	TextureGLMine=true;

	glGenTextures(1,&(TextureGL));
	glBindTexture(GL_TEXTURE_LGL,TextureGL);
	glTexImage2D
	(
		GL_TEXTURE_LGL,
		0,			//Level of Detail=0
		GL_RGB,			//Internal Format
		TexW,TexH,
		0,			//Boarder=0
		GL_RGB,
		GL_UNSIGNED_BYTE,
		NULL
	);
	glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri
	(
		GL_TEXTURE_LGL,
		GL_TEXTURE_MIN_FILTER,
		GL_NEAREST
	);
	glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	FrameBufferUpdate();

	FrameNumber=-1;
	VideoPath[0]='\0';

	YUV_Construct();
}

LGL_Image::
~LGL_Image()
{
#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS

	if(ReferenceCount!=0)
	{
		printf("LGL_Image::~LGL_Image(): Error! Reference Count for Image '%s' is %i, not 0!\n",Path,ReferenceCount);
		assert(ReferenceCount==0);
	}
	UnloadSurfaceFromTexture();

	YUV_Destruct();
}

void
lgl_NearestPointOnLine
(
	float	x1,	float	y1,
	float	x2,	float	y2,
	float	x3,	float	y3,
	float*	retX,	float*	retY,
	float*	u
)
{
	(*u)=	((x3-x1)*(x2-x1)+(y3-y1)*(y2-y1))/
		((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));

	(*retX)=x1+(*u)*(x2-x1);
	(*retY)=y1+(*u)*(y2-y1);
}

void
LGL_Image::
DrawToScreen
(
	float left, float right,
	float bottom, float top,
	float rotation,
	float r, float g, float b, float a,
	float brightnessScalar,
	float leftsubimage,
	float rightsubimage,
	float bottomsubimage,
	float topsubimage
)
{
#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS

	if(LGL.DrawLogFD && !LGL.DrawLogPause)
	{
		char* neo=new char[1024];
		if(FrameBufferImage)
		{
			sprintf
			(
				neo,
				"fbd|%i|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f\n",
				ReadFromFrontBuffer?1:0,
				LeftInt/(float)LGL.WindowResolutionX[LGL.DisplayNow],BottomInt/(float)LGL.WindowResolutionY[LGL.DisplayNow],
				WidthInt/(float)LGL.WindowResolutionX[LGL.DisplayNow],HeightInt/(float)LGL.WindowResolutionY[LGL.DisplayNow],
				left,right,
				bottom,top,
				rotation,
				r,g,b,a,
				leftsubimage, rightsubimage, bottomsubimage, topsubimage
			);
		}
		else
		{
			sprintf
			(
				neo,
				"LGL_Image::DrawToScreen|%s|%i|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%li\n",
				Path,
				LinearInterpolation?1:0,
				left,right,
				bottom,top,
				rotation,
				r,g,b,a,
				leftsubimage, rightsubimage, bottomsubimage, topsubimage,
				FrameNumber
			);
		}
		LGL.DrawLog.push_back(neo);
	}

	//glTranslatef(left+width*.5,bottom+height*.5,0);
	//glRotatef(360.0*(rotation/(LGL_PI*2)),0,0,1);

	float x[4];
	float y[4];

	float d=0;
//#ifdef	LGL_LINUX
//FIXME: FrameBufferImage UGLY fudge factor, due to lousy nVidia Drivers
	if(LGL.FrameBufferTextureGlitchFix)
	{
		d=-0.05/LGL.WindowResolutionX[LGL.DisplayNow];
	}
//#endif	//LGL_LINUX
	x[0]=left+d;
	y[0]=top;
	x[1]=right+d;
	y[1]=top;
	x[2]=right;
	y[2]=bottom;
	x[3]=left;
	y[3]=bottom;

	DrawToScreen
	(
		x,
		y,
		r,
		g,
		b,
		a,
		brightnessScalar,
		leftsubimage,
		rightsubimage,
		bottomsubimage,
		topsubimage
	);
}

void
LGL_Image::
DrawToScreen
(
	float	x[4],
	float	y[4],
	float	r,
	float	g,
	float	b,
	float	a,
	float	brightnessScalar,
	float	leftsubimage,
	float	rightsubimage,
	float	bottomsubimage,
	float	topsubimage,
	float	rgbSpatializerScalar
)
{
#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS

	if(InvertY)
	{
		float tmp=bottomsubimage;
		bottomsubimage=topsubimage;
		topsubimage=tmp;
	}

	//xy[4]
	//0: LT
	//1: RT
	//2: RB
	//3: LB

	//xy[9]
	//0: CC
	//1: LT
	//2: CT
	//3: RT
	//4: RC
	//5: RB
	//6: CB
	//7: LB
	//8: LC
	float x2[9];
	float y2[9];
	float x2t[9];
	float y2t[9];

	int imgW=ImgW;
	int imgH=ImgH;
	int texW=TexW;
	int texH=TexH;

	if(YUV_Available())
	{
		imgW=YUV_ImgW;
		imgH=YUV_ImgH;
		texW=YUV_TexW;
		texH=YUV_TexH;
	}

	float imgTexW=(float)imgW/(float)texW;
	float imgTexH=(float)imgH/(float)texH;

	if(TextureGLRect)
	{
		imgTexW=imgW;
		imgTexH=imgH;
	}

	//0: CC
	x2[0]=0.25f*(x[0]+x[1]+x[2]+x[3]);
	y2[0]=0.25f*(y[0]+y[1]+y[2]+y[3]);
	x2t[0]=0.5f*(leftsubimage+rightsubimage)*imgTexW;
	y2t[0]=0.5f*(bottomsubimage+topsubimage)*imgTexH;
	//1: LT
	x2[1]=x[0];
	y2[1]=y[0];
	x2t[1]=leftsubimage*imgTexW+1.0f/imgW;
	y2t[1]=topsubimage*imgTexH-1.0f/imgH;
	//2: CT
	x2[2]=0.5f*(x[0]+x[1]);
	y2[2]=0.5f*(y[0]+y[1]);
	x2t[2]=x2t[0];
	y2t[2]=topsubimage*imgTexH-1.0f/imgH;
	//3: RT
	x2[3]=x[1];
	y2[3]=y[1];
	x2t[3]=rightsubimage*imgTexW-1.0f/imgW;
	y2t[3]=topsubimage*imgTexH-1.0f/imgH;
	//4: RC
	x2[4]=0.5f*(x[1]+x[2]);
	y2[4]=0.5f*(y[1]+y[2]);
	x2t[4]=rightsubimage*imgTexW-1.0f/imgW;
	y2t[4]=y2t[0];
	//5: RB
	x2[5]=x[2];
	y2[5]=y[2];
	x2t[5]=rightsubimage*imgTexW-1.0f/imgW;
	y2t[5]=bottomsubimage*imgTexH+1.0f/imgH;
	//6: CB
	x2[6]=0.5f*(x[2]+x[3]);
	y2[6]=0.5f*(y[2]+y[3]);
	x2t[6]=x2t[0];
	y2t[6]=bottomsubimage*imgTexH+1.0f/imgH;
	//7: LB
	x2[7]=x[3];
	y2[7]=y[3];
	x2t[7]=leftsubimage*imgTexW+1.0f/imgW;
	y2t[7]=bottomsubimage*imgTexH+1.0f/imgH;
	//8: LC
	x2[8]=0.5f*(x[3]+x[0]);
	y2[8]=0.5f*(y[3]+y[0]);
	x2t[8]=leftsubimage*imgTexW+1.0f/imgW;
	y2t[8]=y2t[0];

	lgl_glScreenify2D();

	if(r<0) r=0;
	if(g<0) g=0;
	if(b<0) b=0;
	if(a<0) a=0;

	if(r>1) r=1;
	if(g>1) g=1;
	if(b>1) b=1;
	if(a>1) a=1;

	//Prepare RGB vs YUV (Alpha)

	GLuint textureGL=TextureGL;
	bool alphaChannel=AlphaChannel;
	LGL_Shader* shader=&ImageShader;
	bool enableShader=true;//brightnessScalar!=1.0f;
	if(YUV_Available())
	{
		textureGL=YUV_TextureGL[0];
		alphaChannel=false;
		imgW=YUV_ImgW;
		imgH=YUV_ImgH;
		texW=YUV_TexW;
		texH=YUV_TexH;
		shader=&YUV_ImageShader;
		enableShader=true;
	}

	//Prepare RGB vs YUV (Omega)

	/*
	if(TextureGLRect)
	{
		enableShader=false;
	}
	*/

	if(enableShader)
	{
		shader->Enable();
		shader->SetUniformAttributeFloat
		(
			"brightnessScalar",
			brightnessScalar
		);
		shader->SetUniformAttributeFloat
		(
			"rgbSpatializerScalar",
			rgbSpatializerScalar
		);
	}

	if(textureGL==0)
	{
		LoadSurfaceToTexture(LinearInterpolation);
	}

	//Draw

	glColor4f(r,g,b,a);
	if(alphaChannel==true || a<1)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	//Activate appropriate textures
	if(YUV_Available())
	{
		gl2ActiveTexture(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_LGL);
		glBindTexture(GL_TEXTURE_LGL,YUV_TextureGL[0]);

		gl2ActiveTexture(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_LGL);
		glBindTexture(GL_TEXTURE_LGL,YUV_TextureGL[1]);

		gl2ActiveTexture(GL_TEXTURE2_ARB);
		glEnable(GL_TEXTURE_LGL);
		glBindTexture(GL_TEXTURE_LGL,YUV_TextureGL[2]);
		
		shader->SetUniformAttributeInt
		(
			"myTextureY",
			0
		);
		shader->SetUniformAttributeInt
		(
			"myTextureU",
			1
		);
		shader->SetUniformAttributeInt
		(
			"myTextureV",
			2
		);
	}
	else
	{
		if(TextureGLRect)
		{
			//gl2ActiveTexture(GL_TEXTURE0_ARB);
			glDisable(GL_TEXTURE_LGL);
			glEnable(GL_TEXTURE_RECTANGLE_ARB);
			glBindTexture(GL_TEXTURE_RECTANGLE_ARB,textureGL);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			gl2ActiveTexture(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_LGL);
			glBindTexture(GL_TEXTURE_LGL,textureGL);
		}
	}

	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_TRIANGLE_FAN);
	{
		glNormal3f(0,0,-1);
		float d=0;
//#ifdef	LGL_LINUX
//FIXME: FrameBufferImage UGLY fudge factor, due to lousy nVidia Drivers
		if(LGL.FrameBufferTextureGlitchFix)
		{
			d=-0.05/LGL.WindowResolutionX[LGL.DisplayNow];
		}
//#endif	//LGL_LINUX

		for(int v=0;v<10;v++)
		{
			bool last=(v==9);
			if(last)
			{
				v=1;
			}
			if(YUV_Available())
			{
				for(int c=0;c<3;c++)
				{
					GLenum target;
					if(c==0) target = GL_TEXTURE0_ARB;
					else if(c==1) target = GL_TEXTURE1_ARB;
					else/*if(c==2)*/ target = GL_TEXTURE2_ARB;
					glMultiTexCoord2d
					(
						target,
						x2t[v],
						y2t[v]
					);
				}
			}
			else
			{
				glTexCoord2d
				(
					x2t[v],
					y2t[v]
				);
			}
			glVertex2d(x2[v],y2[v]);
			if(last) break;
		}
	}
	glEnd();

	if(YUV_Available())
	{
		gl2ActiveTexture(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_LGL);

		gl2ActiveTexture(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_LGL);

		gl2ActiveTexture(GL_TEXTURE2_ARB);
		glDisable(GL_TEXTURE_LGL);

		gl2ActiveTexture(GL_TEXTURE0_ARB);
	}
	else
	{
		if(TextureGLRect)
		{
			glDisable(GL_TEXTURE_RECTANGLE_ARB);
		}
		else
		{
			glDisable(GL_TEXTURE_LGL);
		}
	}

	glDisable(GL_BLEND);

	if(enableShader)
	{
		shader->Disable();
	}
}

#if 0
void
LGL_Image::
DrawToScreen
(
	float	xDst[4],
	float	yDst[4],
	float	xSrc[4],
	float	ySrc[4],
	float	r,
	float	g,
	float	b,
	float	a,
	float	brightnessScalar,
	float	rgbSpatializerScalar
)
{
#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS

	if(InvertY)
	{
		//TODO
		//printf("DrawToScreen(src,dst): InvertY not respected\n");
	}

	//xy[4]
	//0: LT
	//1: RT
	//2: RB
	//3: LB

	//Swizzle 2/3
	{
		float xSrc2=xSrc[2];
		float ySrc2=ySrc[2];

		float xSrc3=xSrc[3];
		float ySrc3=ySrc[3];

		float xDst2=xDst[2];
		float yDst2=yDst[2];

		float xDst3=xDst[3];
		float yDst3=yDst[3];

		xSrc[2]=xSrc3;
		ySrc[2]=ySrc3;

		xSrc[3]=xSrc2;
		ySrc[3]=ySrc2;

		xDst[2]=xDst3;
		yDst[2]=yDst3;

		xDst[3]=xDst2;
		yDst[3]=yDst2;
	}

	int imgW=ImgW;
	int imgH=ImgH;
	int texW=TexW;
	int texH=TexH;

	if(YUV_Available())
	{
		imgW=YUV_ImgW;
		imgH=YUV_ImgH;
		texW=YUV_TexW;
		texH=YUV_TexH;
	}

	float imgTexW=(float)imgW/(float)texW;
	float imgTexH=(float)imgH/(float)texH;

	if(TextureGLRect)
	{
		imgTexW=imgW;
		imgTexH=imgH;
	}

	lgl_glScreenify2D();

	if(r<0) r=0;
	if(g<0) g=0;
	if(b<0) b=0;
	if(a<0) a=0;

	if(r>1) r=1;
	if(g>1) g=1;
	if(b>1) b=1;
	if(a>1) a=1;

	//Prepare RGB vs YUV (Alpha)

	GLuint textureGL=TextureGL;
	bool alphaChannel=AlphaChannel;
	LGL_Shader* shader=&ImageShader;
	bool enableShader=true;//brightnessScalar!=1.0f;
	if(YUV_Available())
	{
		textureGL=YUV_TextureGL[0];
		alphaChannel=false;
		imgW=YUV_ImgW;
		imgH=YUV_ImgH;
		texW=YUV_TexW;
		texH=YUV_TexH;
		shader=&YUV_ImageShader;
		enableShader=true;
	}

	//Prepare RGB vs YUV (Omega)

	/*
	if(TextureGLRect)
	{
		enableShader=false;
	}
	*/

	if(enableShader)
	{
		shader->Enable();
		shader->SetUniformAttributeFloat
		(
			"brightnessScalar",
			brightnessScalar
		);
		shader->SetUniformAttributeFloat
		(
			"rgbSpatializerScalar",
			rgbSpatializerScalar
		);
	}

	if(textureGL==0)
	{
		LoadSurfaceToTexture(LinearInterpolation);
	}

	//Draw

	glColor4f(r,g,b,a);
	if(alphaChannel==true || a<1)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	//Activate appropriate textures
	if(YUV_Available())
	{
		gl2ActiveTexture(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_LGL);
		glBindTexture(GL_TEXTURE_LGL,YUV_TextureGL[0]);

		gl2ActiveTexture(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_LGL);
		glBindTexture(GL_TEXTURE_LGL,YUV_TextureGL[1]);

		gl2ActiveTexture(GL_TEXTURE2_ARB);
		glEnable(GL_TEXTURE_LGL);
		glBindTexture(GL_TEXTURE_LGL,YUV_TextureGL[2]);
		
		shader->SetUniformAttributeInt
		(
			"myTextureY",
			0
		);
		shader->SetUniformAttributeInt
		(
			"myTextureU",
			1
		);
		shader->SetUniformAttributeInt
		(
			"myTextureV",
			2
		);
	}
	else
	{
		if(TextureGLRect)
		{
			//gl2ActiveTexture(GL_TEXTURE0_ARB);
			glDisable(GL_TEXTURE_LGL);
			glEnable(GL_TEXTURE_RECTANGLE_ARB);
			glBindTexture(GL_TEXTURE_RECTANGLE_ARB,textureGL);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			gl2ActiveTexture(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_LGL);
			glBindTexture(GL_TEXTURE_LGL,textureGL);
		}
	}

	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_TRIANGLE_STRIP);
	{
		glNormal3f(0,0,-1);

		for(int v=0;v<4;v++)
		{
			if(YUV_Available())
			{
				for(int c=0;c<3;c++)
				{
					GLenum target;
					if(c==0) target = GL_TEXTURE0_ARB;
					else if(c==1) target = GL_TEXTURE1_ARB;
					else/*if(c==2)*/ target = GL_TEXTURE2_ARB;
					glMultiTexCoord2d
					(
						target,
						xSrc[v]*imgTexW,
						ySrc[v]*imgTexH
					);
				}
			}
			else
			{
				glTexCoord2d
				(
					xSrc[v]*(imgTexW-1),
					ySrc[v]*(imgTexH-1)
				);
			}
			glVertex2d(xDst[v],yDst[v]);

/*
			LGL_DebugPrintf("Vert %i: (%.2f, %.2f) => (%.2f, %.2f)\n",
				v,
				xSrc[v],
				ySrc[v],
				xDst[v],
				yDst[v]
			);
*/
		}
	}
	glEnd();

	if(YUV_Available())
	{
		gl2ActiveTexture(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_LGL);

		gl2ActiveTexture(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_LGL);

		gl2ActiveTexture(GL_TEXTURE2_ARB);
		glDisable(GL_TEXTURE_LGL);

		gl2ActiveTexture(GL_TEXTURE0_ARB);
	}
	else
	{
		if(TextureGLRect)
		{
			glDisable(GL_TEXTURE_RECTANGLE_ARB);
		}
		else
		{
			glDisable(GL_TEXTURE_LGL);
		}
	}

	glDisable(GL_BLEND);

	if(enableShader)
	{
		shader->Disable();
	}
}
#endif

void
LGL_Image::
DrawToScreen
(
	float	xDst[4],
	float	yDst[4],
	float	xSrc[4],
	float	ySrc[4],
	float	r,
	float	g,
	float	b,
	float	a,
	float	brightnessScalar,
	float	rgbSpatializerScalar
)
{
#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS

	if(InvertY)
	{
		//TODO
		//printf("DrawToScreen(src,dst): InvertY not respected\n");
	}

	//xy[4]
	//0: LT
	//1: RT
	//2: RB
	//3: LB

	//xy[9]
	//0: CC
	//1: LT
	//2: CT
	//3: RT
	//4: RC
	//5: RB
	//6: CB
	//7: LB
	//8: LC

	float x2Dst[9];
	float y2Dst[9];
	float x2Src[9];
	float y2Src[9];

	int imgW=ImgW;
	int imgH=ImgH;
	int texW=TexW;
	int texH=TexH;

	if(YUV_Available())
	{
		imgW=YUV_ImgW;
		imgH=YUV_ImgH;
		texW=YUV_TexW;
		texH=YUV_TexH;
	}

	float imgTexW=(float)imgW/(float)texW;
	float imgTexH=(float)imgH/(float)texH;

	if(TextureGLRect)
	{
		imgTexW=imgW;
		imgTexH=imgH;
	}

	//0: CC
	x2Dst[0]=0.25f*(xDst[0]+xDst[1]+xDst[2]+xDst[3]);
	y2Dst[0]=0.25f*(yDst[0]+yDst[1]+yDst[2]+yDst[3]);
	x2Src[0]=0.25f*(xSrc[0]+xSrc[1]+xSrc[2]+xSrc[3]);
	y2Src[0]=0.25f*(ySrc[0]+ySrc[1]+ySrc[2]+ySrc[3]);
	//1: LT
	x2Dst[1]=xDst[0];
	y2Dst[1]=yDst[0];
	x2Src[1]=xSrc[0];
	y2Src[1]=ySrc[0];
	//2: CT
	x2Dst[2]=0.5f*(xDst[0]+xDst[1]);
	y2Dst[2]=0.5f*(yDst[0]+yDst[1]);
	x2Src[2]=0.5f*(xSrc[0]+xSrc[1]);
	y2Src[2]=0.5f*(ySrc[0]+ySrc[1]);
	//3: RT
	x2Dst[3]=xDst[1];
	y2Dst[3]=yDst[1];
	x2Src[3]=xSrc[1];
	y2Src[3]=ySrc[1];
	//4: RC
	x2Dst[4]=0.5f*(xDst[1]+xDst[2]);
	y2Dst[4]=0.5f*(yDst[1]+yDst[2]);
	x2Src[4]=0.5f*(xSrc[1]+xSrc[2]);
	y2Src[4]=0.5f*(ySrc[1]+ySrc[2]);
	//5: RB
	x2Dst[5]=xDst[2];
	y2Dst[5]=yDst[2];
	x2Src[5]=xSrc[2];
	y2Src[5]=ySrc[2];
	//6: CB
	x2Dst[6]=0.5f*(xDst[2]+xDst[3]);
	y2Dst[6]=0.5f*(yDst[2]+yDst[3]);
	x2Src[6]=0.5f*(xSrc[2]+xSrc[3]);
	y2Src[6]=0.5f*(ySrc[2]+ySrc[3]);
	//7: LB
	x2Dst[7]=xDst[3];
	y2Dst[7]=yDst[3];
	x2Src[7]=xSrc[3];
	y2Src[7]=ySrc[3];
	//8: LC
	x2Dst[8]=0.5f*(xDst[3]+xDst[0]);
	y2Dst[8]=0.5f*(yDst[3]+yDst[0]);
	x2Src[8]=0.5f*(xSrc[3]+xSrc[0]);
	y2Src[8]=0.5f*(ySrc[3]+ySrc[0]);

	lgl_glScreenify2D();

	if(r<0) r=0;
	if(g<0) g=0;
	if(b<0) b=0;
	if(a<0) a=0;

	if(r>1) r=1;
	if(g>1) g=1;
	if(b>1) b=1;
	if(a>1) a=1;

	//Prepare RGB vs YUV (Alpha)

	GLuint textureGL=TextureGL;
	bool alphaChannel=AlphaChannel;
	LGL_Shader* shader=&ImageShader;
	bool enableShader=true;//brightnessScalar!=1.0f;
	if(YUV_Available())
	{
		textureGL=YUV_TextureGL[0];
		alphaChannel=false;
		imgW=YUV_ImgW;
		imgH=YUV_ImgH;
		texW=YUV_TexW;
		texH=YUV_TexH;
		shader=&YUV_ImageShader;
		enableShader=true;
	}

	//Prepare RGB vs YUV (Omega)

	/*
	if(TextureGLRect)
	{
		enableShader=false;
	}
	*/

	if(enableShader)
	{
		shader->Enable();
		shader->SetUniformAttributeFloat
		(
			"brightnessScalar",
			brightnessScalar
		);
		shader->SetUniformAttributeFloat
		(
			"rgbSpatializerScalar",
			rgbSpatializerScalar
		);
	}

	if(textureGL==0)
	{
		LoadSurfaceToTexture(LinearInterpolation);
	}

	//Draw

	glColor4f(r,g,b,a);
	if(alphaChannel==true || a<1)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	//Activate appropriate textures
	if(YUV_Available())
	{
		gl2ActiveTexture(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_LGL);
		glBindTexture(GL_TEXTURE_LGL,YUV_TextureGL[0]);

		gl2ActiveTexture(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_LGL);
		glBindTexture(GL_TEXTURE_LGL,YUV_TextureGL[1]);

		gl2ActiveTexture(GL_TEXTURE2_ARB);
		glEnable(GL_TEXTURE_LGL);
		glBindTexture(GL_TEXTURE_LGL,YUV_TextureGL[2]);
		
		shader->SetUniformAttributeInt
		(
			"myTextureY",
			0
		);
		shader->SetUniformAttributeInt
		(
			"myTextureU",
			1
		);
		shader->SetUniformAttributeInt
		(
			"myTextureV",
			2
		);
	}
	else
	{
		if(TextureGLRect)
		{
			//gl2ActiveTexture(GL_TEXTURE0_ARB);
			glDisable(GL_TEXTURE_LGL);
			glEnable(GL_TEXTURE_RECTANGLE_ARB);
			glBindTexture(GL_TEXTURE_RECTANGLE_ARB,textureGL);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			gl2ActiveTexture(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_LGL);
			glBindTexture(GL_TEXTURE_LGL,textureGL);
		}
	}

	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_TRIANGLE_FAN);
	{
		glNormal3f(0,0,-1);
		float d=0;
//#ifdef	LGL_LINUX
//FIXME: FrameBufferImage UGLY fudge factor, due to lousy nVidia Drivers
		if(LGL.FrameBufferTextureGlitchFix)
		{
			d=-0.05/LGL.WindowResolutionX[LGL.DisplayNow];
		}
//#endif	//LGL_LINUX

		for(int v=0;v<10;v++)
		{
			bool last=(v==9);
			if(last)
			{
				v=1;
			}
			if(YUV_Available())
			{
				for(int c=0;c<3;c++)
				{
					GLenum target;
					if(c==0) target = GL_TEXTURE0_ARB;
					else if(c==1) target = GL_TEXTURE1_ARB;
					else/*if(c==2)*/ target = GL_TEXTURE2_ARB;
					glMultiTexCoord2d
					(
						target,
						x2Src[v]*imgTexW,
						y2Src[v]*imgTexH
					);
				}
			}
			else
			{
				glTexCoord2d
				(
					x2Src[v]*imgTexW,
					y2Src[v]*imgTexH
				);
			}
			glVertex2d(x2Dst[v],y2Dst[v]);
			if(last) break;
		}
	}
	glEnd();

	if(YUV_Available())
	{
		gl2ActiveTexture(GL_TEXTURE0_ARB);
		glDisable(GL_TEXTURE_LGL);

		gl2ActiveTexture(GL_TEXTURE1_ARB);
		glDisable(GL_TEXTURE_LGL);

		gl2ActiveTexture(GL_TEXTURE2_ARB);
		glDisable(GL_TEXTURE_LGL);

		gl2ActiveTexture(GL_TEXTURE0_ARB);
	}
	else
	{
		if(TextureGLRect)
		{
			glDisable(GL_TEXTURE_RECTANGLE_ARB);
		}
		else
		{
			glDisable(GL_TEXTURE_LGL);
		}
	}

	glDisable(GL_BLEND);

	if(enableShader)
	{
		shader->Disable();
	}
}

void
LGL_Image::
DrawToScreenAsLine
(
	float x1, float y1,
	float x2, float y2,
	float thickness,
	float r, float g, float b, float a,
	bool continueLastLine
)
{
	lgl_glScreenify2D();

	if(LGL.DrawLogFD && !LGL.DrawLogPause)
	{
		char* neo=new char[1024];
		sprintf
		(
			neo,
			"LGL_Image::DrawToScreenAsLine|%s|%i|%i|%i|%i|%i|%i|%i|%f|%f|%f|%f|%f|%f|%f|%f|%f|%i\n",
			Path,
			LinearInterpolation?1:0,
			FrameBufferImage?1:0,
			ReadFromFrontBuffer?1:0,
			LeftInt,BottomInt,WidthInt,HeightInt,
			x1,y1,
			x2,y2,
			thickness,
			r,g,b,a,
			continueLastLine?1:0
		);
		LGL.DrawLog.push_back(neo);
	}

	if(TextureGL==0)
	{
		LoadSurfaceToTexture(LinearInterpolation);
	}

	//Figure out normal vector, and scale it
	
	float NormX=-(y2-y1);
	float NormY=x2-x1;
	float NormMag=sqrtf(NormX*NormX+NormY*NormY);
	float Scalar=.5*thickness/NormMag;
	NormX*=Scalar;
	NormY*=Scalar;

	//Draw

	glColor4f(r,g,b,a);
	if(AlphaChannel==true || a<1)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}
	glEnable(GL_TEXTURE_LGL);
	glBindTexture(GL_TEXTURE_LGL,TextureGL);
	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
	{
		glNormal3f(0,0,-1);
		//SDL Images are upside-down. Compensate.
	
		if(continueLastLine)
		{
			//Begin Right
			glTexCoord2f
			(
				0*(float)ImgW/(float)TexW,
				1*(float)ImgH/(float)TexH
			);
			glVertex2f(LGL.LastImageDrawAsLineRightX,LGL.LastImageDrawAsLineRightY);
			
			//Begin Left
			glTexCoord2f
			(
				1*(float)ImgW/(float)TexW,
				1*(float)ImgH/(float)TexH
			);
			glVertex2f(LGL.LastImageDrawAsLineLeftX,LGL.LastImageDrawAsLineLeftY);
		}
		else
		{
			//Begin Right
			glTexCoord2f
			(
				0*(float)ImgW/(float)TexW,
				1*(float)ImgH/(float)TexH
			);
			glVertex2f(x1+NormX,y1+NormY);
			
			//Begin Left
			glTexCoord2f
			(
				1*(float)ImgW/(float)TexW,
				1*(float)ImgH/(float)TexH
			);
			glVertex2f(x1-NormX,y1-NormY);
		}
		
		//End Left
		glTexCoord2f
		(
			1*(float)ImgW/(float)TexW,
			0*(float)ImgH/(float)TexH
		);
		glVertex2f(x2-NormX,y2-NormY);
		LGL.LastImageDrawAsLineLeftX=x2-NormX;
		LGL.LastImageDrawAsLineLeftY=y2-NormY;
		
		//End Right
		glTexCoord2f
		(
			0*(float)ImgW/(float)TexW,
			0*(float)ImgH/(float)TexH
		);
		glVertex2f(x2+NormX,y2+NormY);
		LGL.LastImageDrawAsLineRightX=x2+NormX;
		LGL.LastImageDrawAsLineRightY=y2+NormY;
	}
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_LGL);
}

void
LGL_Image::
LoadSurfaceToTexture
(
	bool	LinearInterpolation,
	int	loadToExistantGLTexture,
	int	loadToExistantGLTextureX,
	int	loadToExistantGLTextureY
)
{
	//Check to see if we already have a texture loaded...

	if(TextureGL!=0)
	{
		//We already have a texture loaded. Axe it.
		glDeleteTextures(1,&(TextureGL));
	}

	//Load from SurfaceSDL to TextureGL
	if(loadToExistantGLTexture<=0)
	{
		TextureGLMine=true;
		glGenTextures(1,&(TextureGL));
		glBindTexture(GL_TEXTURE_LGL,TextureGL);
		glTexImage2D
		(
			GL_TEXTURE_LGL,
			0,					//Level of Detail=0
			AlphaChannel ? GL_RGBA : GL_RGB,	//Internal Format
			TexW,TexH,
			0,					//Boarder=0
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			NULL
		);

		if(SurfaceSDL)
		{
			glTexSubImage2D
			(
				GL_TEXTURE_LGL,
				0,					//Level of Detail=0
				0,					//X-Offset
				0,					//Y-Offset
				ImgW,
				ImgH,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				SurfaceSDL->pixels
			);
		}

		if(LinearInterpolation)
		{
			/*
			glTexParameteri
			(
				GL_TEXTURE_LGL,
				GL_TEXTURE_MIN_FILTER,
				GL_LINEAR
			);
			*/
			glTexParameteri
			(
				GL_TEXTURE_LGL,
				GL_TEXTURE_MIN_FILTER,
				GL_NEAREST
			);
			glTexParameteri
			(
				GL_TEXTURE_LGL,
				GL_TEXTURE_MAG_FILTER,
				GL_LINEAR
			);
		}
		else
		{
			glTexParameteri
			(
				GL_TEXTURE_LGL,
				GL_TEXTURE_MIN_FILTER,
				GL_NEAREST
			);
			glTexParameteri
			(
				GL_TEXTURE_LGL,
				GL_TEXTURE_MAG_FILTER,
				GL_NEAREST
			);
		}
		
		glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_WRAP_T, GL_CLAMP);

		/*
		gluBuild2DMipmapLevels
		(
			TextureGL,
			GL_RGBA,
			TexW,TexH,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			0,
			0,
			16,
			SurfaceSDL->pixels
		);
		*/

		LGL.TexturePixels+=TexW*TexH;
	}
	else
	{
		TextureGL=loadToExistantGLTexture;
		TextureGLMine=false;
		glBindTexture(GL_TEXTURE_LGL,TextureGL);
		if(SurfaceSDL)
		{
			glTexSubImage2D
			(
				GL_TEXTURE_LGL,
				0,				//Level of Detail=0
				loadToExistantGLTextureX,	//X-Offset
				loadToExistantGLTextureY,	//Y-Offset
				ImgW,
				ImgH,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				SurfaceSDL->pixels
			);
		}
	}

	if(PixelBufferEnable)
	{
		UpdatePixelBufferObjects();	
	}
	else
	{
		DeletePixelBufferObjects();
	}
}

void
LGL_Image::
UnloadSurfaceFromTexture()
{
	if(TextureGL==0)
	{
		return;
	}

	if(TextureGLMine==false)
	{
		TextureGL=0;
		return;
	}
	else
	{
		if(TextureGL!=0)
		{
			glDeleteTextures(1,&(TextureGL));
			TextureGL=0;
			LGL.TexturePixels-=TexW*TexH;
		}
	}

	DeletePixelBufferObjects();
}

void
LGL_Image::
UpdateTexture
(
	int		w,
	int		h,
	int		bytesperpixel,
	unsigned char*	data,
	bool		inLinearInterpolation,
	const char*	name
)
{
	if(YUV_Available())
	{
		YUV_Destruct();
		YUV_Construct();
	}

	if
	(
		w>TexW ||
		w<TexW/2 ||
		h>TexH ||
		h<TexH/2
	)
	{
		UnloadSurfaceFromTexture();
		ImgW=w;
		ImgH=h;
		TexW=LGL_NextPowerOfTwo(ImgW);
		TexH=LGL_NextPowerOfTwo(ImgH);

		TextureGLMine=true;

		glGenTextures(1,&(TextureGL));
		glBindTexture(GL_TEXTURE_LGL,TextureGL);
		glTexImage2D
		(
			GL_TEXTURE_LGL,
			0,			//Level of Detail=0
			GL_RGB,			//Internal Format
			TexW,TexH,
			0,			//Boarder=0
			GL_RGB,
			GL_UNSIGNED_BYTE,
			NULL
		);
		glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_WRAP_T, GL_CLAMP);
		//glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri
		(
			GL_TEXTURE_LGL,
			GL_TEXTURE_MIN_FILTER,
			GL_NEAREST
		);
		glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	if(w!=ImgW)
	{
		ImgW=LGL_Min(w,TexW);
	}
	if(h!=ImgH)
	{
		ImgH=LGL_Min(h,TexH);
	}
	assert(bytesperpixel==3 || bytesperpixel==4);
	if(data==NULL)
	{
		//LGL_Assertf(data!=NULL,("LGL_Image::UpdateTexture(): NULL data! WTF!\n"));
		printf("LGL_Image::UpdateTexture(): NULL data! WTF!\n");
		return;
	}
	LinearInterpolation=inLinearInterpolation;
	if(name==NULL) name = "NULL Name";
	if(name[0]=='!')
	{
		sprintf(Path,"%s",name);
		sprintf(PathShort,"%s",name);
	}
	else
	{
		sprintf(Path,"!%s",name);
		sprintf(PathShort,"!%s",name);
	}

	UpdatePixelBufferObjects();

	if(TextureGL==0)
	{
		//Though surface might be quite undefined at this point... Hmm...
		LoadSurfaceToTexture(LinearInterpolation);
	}

	glBindTexture(GL_TEXTURE_LGL,TextureGL);

	bool pboReady=false;

	//Swap
	/*
	GLuint tmp=PixelBufferObjectBackGL;
	PixelBufferObjectBackGL=PixelBufferObjectFrontGL;
	PixelBufferObjectFrontGL=tmp;
	gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,PixelBufferObjectFrontGL);

	pboReady = gl2IsBuffer(PixelBufferObjectFrontGL);

	if(PixelBufferVirgin)
	{
		pboReady=false;
		gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
		PixelBufferVirgin=false;
	}
	*/

#ifdef	LGL_OSX
	gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
	glTexParameteri
	(
		GL_TEXTURE_LGL,
		GL_TEXTURE_STORAGE_HINT_APPLE,
		GL_STORAGE_CACHED_APPLE
	);
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
	pboReady=false;
#endif

	if(TextureGLRect)
	{
		glTexImage2D
		(
			GL_TEXTURE_LGL,
			0,
			GL_RGBA,
			ImgW,
			ImgH,
			0,
			GL_BGRA,
			GL_UNSIGNED_INT_8_8_8_8_REV,
			pboReady?0:data
		);
	}
	else
	{
		glTexSubImage2D
		(
			GL_TEXTURE_LGL,
			0,			//Level of Detail=0
			0,			//X-Offset
			0,			//Y-Offset
			ImgW,
			ImgH,
			bytesperpixel==3?GL_RGB:GL_BGRA,
			GL_UNSIGNED_INT_8_8_8_8_REV,
			pboReady?0:data
		);
	}

	/*
	pboReady = gl2IsBuffer(PixelBufferObjectBackGL);
	if(pboReady)
	{
		gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,PixelBufferObjectBackGL);
		gl2BufferData(GL_PIXEL_UNPACK_BUFFER, ImgW*ImgH*bytesperpixel, 0, GL_STREAM_DRAW);
		GLubyte* pbo = (GLubyte*)gl2MapBuffer(GL_PIXEL_UNPACK_BUFFER,GL_WRITE_ONLY);
		if(pbo)
		{
			memcpy(pbo,data,ImgW*ImgH*bytesperpixel);
			gl2UnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		}
	}

	gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
	*/

	InvertY=false;
	glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
}

void
LGL_Image::
UpdatePixelBufferObjects()
{
	if
	(
		ImgW<0 ||
		ImgH<0
	)
	{
		printf("Cannot create pixel buffer object with negative dimensions (%i x %i)!\n",ImgW,ImgH);
		return;
	}

	GLsizei size = ImgW*ImgH*(AlphaChannel?4:3);

	if(size>PixelBufferObjectSize)
	{
		DeletePixelBufferObjects();
	}

	if(PixelBufferObjectFrontGL==0)
	{
		//Make a Pixel Buffer Object
		gl2GenBuffers(1,&(PixelBufferObjectFrontGL));
		gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,PixelBufferObjectFrontGL);
		gl2BufferData
		(
			GL_PIXEL_UNPACK_BUFFER,
			(GLsizeiptr)(&size),
			NULL,
			GL_STREAM_DRAW
		);
		gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,0);

		//Make a Pixel Buffer Object
		gl2GenBuffers(1,&(PixelBufferObjectBackGL));
		gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,PixelBufferObjectBackGL);
		gl2BufferData
		(
			GL_PIXEL_UNPACK_BUFFER,
			(GLsizeiptr)(&size),
			NULL,
			GL_STREAM_DRAW
		);
		gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,0);

		PixelBufferObjectSize=size;
		PixelBufferVirgin=true;
	}
}

void
LGL_Image::
DeletePixelBufferObjects()
{
	if(PixelBufferObjectFrontGL>0)
	{
		gl2DeleteBuffers(1,&(PixelBufferObjectFrontGL));
		gl2DeleteBuffers(1,&(PixelBufferObjectBackGL));
		PixelBufferObjectFrontGL=0;
		PixelBufferObjectBackGL=0;
		PixelBufferObjectSize=0;
	}
}

void
LGL_Image::
FrameBufferUpdate()
{
	if(LGL.DrawLogFD && !LGL.DrawLogPause)
	{
		char* neo=new char[1024];
		sprintf
		(
			neo,
			"fbu\n"
		);
		LGL.DrawLog.push_back(neo);
	}

	if(!FrameBufferImage)
	{
		printf("LGL_Image::FrameBufferUpdate(): Error! Not a FrameBufferImage!\n");
		LGL_Exit();
	}

	if(ReadFromFrontBuffer)
	{
		glReadBuffer(GL_FRONT_LEFT);
	}
	else
	{
		glReadBuffer(GL_BACK_LEFT);
	}
	glBindTexture(GL_TEXTURE_LGL,TextureGL);
	glCopyTexSubImage2D
	(
		GL_TEXTURE_LGL,	//target
		0,		//level
		0,		//xoffset
		0,		//yoffset
		LeftInt,	//x
		BottomInt,	//y
		WidthInt,	//width
		HeightInt	//height
	);
}

void
LGL_Image::
FrameBufferViewport
(
	float	left,	float	right,
	float	bottom,	float	top
)
{
#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS
	if(!FrameBufferImage)
	{
		printf("LGL_Image::FrameBufferViewport(): Error! ");
		printf("'%s' is not a FrameBufferImage!\n",Path);
		LGL_Exit();
	}

	LeftInt=	(int)floor(left*LGL.WindowResolutionX[LGL.DisplayNow]);
	BottomInt=	(int)floor(bottom*LGL.WindowResolutionY[LGL.DisplayNow]);
	WidthInt=	((int)ceil(right*LGL.WindowResolutionX[LGL.DisplayNow]))-LeftInt;
	HeightInt=	((int)ceil(top*LGL.WindowResolutionY[LGL.DisplayNow]))-BottomInt;
	ImgW=WidthInt;
	ImgH=HeightInt;
}

void
LGL_Image::
FrameBufferFront
(
	bool	useFront
)
{
	ReadFromFrontBuffer=useFront;
}

int
LGL_Image::
GetWidth()
{
	return(YUV_Available() ? YUV_ImgW : ImgW);
}

int
LGL_Image::
GetHeight()
{
	return(YUV_Available() ? YUV_ImgH : ImgH);
}

int
LGL_Image::
GetTexWidth()
{
	return(TexW);
}

int
LGL_Image::
GetTexHeight()
{
	return(TexH);
}

const
char*
LGL_Image::
GetPath()	const
{
	return(Path);
}

const
char*
LGL_Image::
GetPathShort()	const
{
	return(PathShort);
}

float
LGL_Image::
GetPixelR
(
	int x,
	int y
)
{
	if(SurfaceSDL==NULL)
	{
		printf
		(
			"LGL_Image::LGL_Image(): '%s' can't yet be treated as an SDL_Surface...\n",
			Path
		);
		assert(false);
	}
	if
	(
		x>=0 && x<ImgW &&
		y>=0 && y<ImgH
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		return
		(
			((char*)SurfaceSDL->pixels)[(ImgH-y)*ImgW*Bpp + x*Bpp]/255.0
		);
	}
	else
	{
		printf("LGL_Image::GetPixelR(): (%i,%i) out of range!\n",x,y);
		return(0);
	}
}
float
LGL_Image::
GetPixelG
(
	int x,
	int y
)
{
	if(SurfaceSDL==NULL)
	{
		printf
		(
			"LGL_Image::LGL_Image(): '%s' can't yet be treated as an SDL_Surface...\n",
			Path
		);
		assert(false);
	}
	if
	(
		x>=0 && x<ImgW &&
		y>=0 && y<ImgH
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		return
		(
			((char*)SurfaceSDL->pixels)[(ImgH-y)*ImgW*Bpp + x*Bpp + 1]/255.0
		);
	}
	else
	{
		printf("LGL_Image::GetPixelG(): (%i,%i) out of range!\n",x,y);
		return(0);
	}
}
float
LGL_Image::
GetPixelB
(
	int x,
	int y
)
{
	if(SurfaceSDL==NULL)
	{
		printf
		(
			"LGL_Image::LGL_Image(): '%s' can't yet be treated as an SDL_Surface...\n",
			Path
		);
		assert(false);
	}
	if
	(
		x>=0 && x<ImgW &&
		y>=0 && y<ImgH
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		return
		(
			((char*)SurfaceSDL->pixels)[(ImgH-y)*ImgW*Bpp + x*Bpp + 2]/255.0
		);
	}
	else
	{
		printf("LGL_Image::GetPixelB(): (%i,%i) out of range!\n",x,y);
		return(0);
	}
}
float
LGL_Image::
GetPixelA
(
	int x,
	int y
)
{
	if(SurfaceSDL==NULL)
	{
		printf
		(
			"LGL_Image::LGL_Image(): '%s' can't yet be treated as an SDL_Surface...\n",
			Path
		);
		assert(false);
	}
	if
	(
		x>=0 && x<ImgW &&
		y>=0 && y<ImgH
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		return
		(
			((char*)SurfaceSDL->pixels)[(ImgH-y)*ImgW*Bpp + x*Bpp + 3]/255.0
		);
	}
	else
	{
		printf("LGL_Image::GetPixelA(): (%i,%i) out of range!\n",x,y);
		return(0);
	}
}

void
LGL_Image::
SetPixelR
(
	int x,
	int y,
	float r
)
{
	if(SurfaceSDL==NULL)
	{
		printf
		(
			"LGL_Image::LGL_Image(): '%s' can't yet be treated as an SDL_Surface...\n",
			Path
		);
		assert(false);
	}
	if
	(
		x>=0 && x<ImgW &&
		y>=0 && y<ImgH
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		((char*)SurfaceSDL->pixels)[(ImgH-y)*ImgW*Bpp + x*Bpp]=(int)floor(r*255.0);
	}
	else
	{
		printf("LGL_Image::SetPixelR(): (%i,%i) out of range!\n",x,y);
	}
}
void
LGL_Image::
SetPixelG
(
	int x,
	int y,
	float g
)
{
	if(SurfaceSDL==NULL)
	{
		printf
		(
			"LGL_Image::LGL_Image(): '%s' can't yet be treated as an SDL_Surface...\n",
			Path
		);
		assert(false);
	}
	if
	(
		x>=0 && x<ImgW &&
		y>=0 && y<ImgH
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		((char*)SurfaceSDL->pixels)[(ImgH-y)*ImgW*Bpp + x*Bpp + 1]=(int)floor(g*255.0);
	}
	else
	{
		printf("LGL_Image::SetPixelG(): (%i,%i) out of range!\n",x,y);
	}
}
void
LGL_Image::
SetPixelB
(
	int x,
	int y,
	float b
)
{
	if(SurfaceSDL==NULL)
	{
		printf
		(
			"LGL_Image::LGL_Image(): '%s' can't yet be treated as an SDL_Surface...\n",
			Path
		);
		assert(false);
	}
	if
	(
		x>=0 && x<ImgW &&
		y>=0 && y<ImgH
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		((char*)SurfaceSDL->pixels)[(ImgH-y)*ImgW*Bpp + x*Bpp + 2]=(int)floor(b*255.0);
	}
	else
	{
		printf("LGL_Image::SetPixelB(): (%i,%i) out of range!\n",x,y);
	}
}
void
LGL_Image::
SetPixelA
(
	int x,
	int y,
	float a
)
{
	if(SurfaceSDL==NULL)
	{
		printf
		(
			"LGL_Image::LGL_Image(): '%s' can't yet be treated as an SDL_Surface...\n",
			Path
		);
		assert(false);
	}
	if
	(
		x>=0 && x<ImgW &&
		y>=0 && y<ImgH
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		((char*)SurfaceSDL->pixels)[(ImgH-y)*ImgW*Bpp + x*Bpp + 3]=(int)floor(a*255.0);
	}
	else
	{
		printf("LGL_Image::SetPixelA(): (%i,%i) out of range!\n",x,y);
	}
}

void
LGL_Image::
FileLoad
(
	const
	char*	inFilename
)
{
	if(strlen(inFilename)>2040)
	{
		printf
		(
			"LGL_Image::LGL_Image(): filename '%s' is too long (2040 max)\n",
			inFilename
		);
		return;
	}

	LeftInt=WidthInt=BottomInt=HeightInt=0;
	strcpy(Path,inFilename);
#ifdef	LGL_WIN32
	for(unsigned int a=0;a<strlen(Path);a++)
	{
		if(Path[a]=='/')
		{
			Path[a]='\\';
		}
	}
#endif	//LGL_WIN32

	char temp[1024];
	strcpy(temp,Path);
	char* ptr=&(temp[0]);
	while(strstr(ptr,"/"))
	{
		ptr=&(strstr(ptr,"/")[1]);
	}
	strcpy(PathShort,ptr);

	//Load via SDL_Image to an SDL_Surface

	SDL_Surface *mySDL_Surface1=IMG_Load(Path);
	if(mySDL_Surface1==NULL)
	{
		printf
		(
			"LGL_Image::LGL_Image(): Error loading '%s' to an SDL_Surface: %s\n",
			Path,
			IMG_GetError()
		);
		char wd[2048];
		getcwd(wd,2048);
		printf("Working Directory: %s\n",wd);
		return;
	}

	AlphaChannel=mySDL_Surface1->flags & SDL_SRCALPHA;

	//Convert mySDL_Surface1 to the desired format for SDL_Surface2

	SDL_PixelFormat DesiredFormat;
	DesiredFormat.palette=NULL;
	DesiredFormat.BitsPerPixel=32;
	DesiredFormat.BytesPerPixel=4;
#ifdef	LGL_OSX_PPC
	DesiredFormat.Rmask=0xFF000000;
	DesiredFormat.Gmask=0x00FF0000;
	DesiredFormat.Bmask=0x0000FF00;
	DesiredFormat.Amask=0x000000FF;
#else	//LGL_OSX_PPC
	DesiredFormat.Rmask=0x000000FF;
	DesiredFormat.Gmask=0x0000FF00;
	DesiredFormat.Bmask=0x00FF0000;
	DesiredFormat.Amask=0xFF000000;
#endif	//LGL_OSX_PPC
	DesiredFormat.Rshift=0;
	DesiredFormat.Gshift=0;
	DesiredFormat.Bshift=0;
	DesiredFormat.Ashift=0;
	DesiredFormat.Rloss=0;
	DesiredFormat.Gloss=0;
	DesiredFormat.Bloss=0;
	DesiredFormat.Aloss=0;
	//DesiredFormat.colorkey=0;
	//DesiredFormat.alpha=255;

	SurfaceSDL=SDL_ConvertSurface(mySDL_Surface1,&DesiredFormat,SDL_SWSURFACE);
	if(SurfaceSDL==NULL)
	{
		printf
		(
			"LGL_Image::LGL_Image(): Error converting '%s' to our desired format (2)...\n",
			Path
		);
		printf("SDL_Error: '%s'\n",SDL_GetError());
		assert(SurfaceSDL);
	}

	ImgW=SurfaceSDL->w;
	ImgH=SurfaceSDL->h;
	TexW=LGL_NextPowerOfTwo(ImgW);
	TexH=LGL_NextPowerOfTwo(ImgH);

	TextureGL=0;
	FrameBufferImage=false;
	
	//Delete temporary SDL_Surface

	SDL_FreeSurface(mySDL_Surface1);
}

void
LGL_Image::
FileSave
(
	char*	bmpFile
)
{
	if(FrameBufferImage==false)
	{
		if(SurfaceSDL)
		{
			SDL_SaveBMP(SurfaceSDL, bmpFile);
			return;
		}
	}

//FIXME: LGL_Image::FileSave(): WidthInt must currently be a multiple of 4, for some stupid reason.
	int WidthInt=(int)((this->WidthInt/4)*4);

	//At this point, we have a glFrameBuffer image

	SDL_Surface* temp=NULL;
	unsigned char* pixels;
	int i;

	temp=SDL_CreateRGBSurface
	(
		SDL_SWSURFACE,
		WidthInt, HeightInt, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		0x000000FF, 0x0000FF00, 0x00FF0000, 0
#else
		0x00FF0000, 0x0000FF00, 0x000000FF, 0
#endif
	);
	if(temp==NULL)
	{
		printf("LGL_Image::Save(): Error! Unable to SDL_CreateRGBSurface().\n");
		LGL_Exit();
	}

	pixels=(unsigned char*)malloc(3*WidthInt*HeightInt);
	if(pixels==NULL)
	{
		printf("LGL_Image::Save(): Error! Unable to malloc() pixel buffer.\n");
		LGL_Exit();
	}

	if(FrameBufferImage)
	{
		if(ReadFromFrontBuffer)
		{
			glReadBuffer(GL_FRONT_LEFT);
		}
		else
		{
			glReadBuffer(GL_BACK_LEFT);
		}
		glReadPixels
		(
			LeftInt, BottomInt,
			WidthInt, HeightInt,
			GL_RGB, GL_UNSIGNED_BYTE,
			pixels
		);
	}
	else
	{
		//TODO
		printf("LGL_Image::FileSave(): Failed! Must implement GL Texture path\n");
	}

	for(i=0;i<HeightInt;i++)
	{
		memcpy
		(
			((char*)temp->pixels)+temp->pitch*i,
			pixels+3*WidthInt*(HeightInt-1-i),
			WidthInt*3
		);
	}

	free(pixels);
	SDL_SaveBMP(temp, bmpFile);

	SDL_FreeSurface(temp);
}

int
LGL_Image::
GetReferenceCount()
{
	return(ReferenceCount);
}

void
LGL_Image::
IncrementReferenceCount()
{
	ReferenceCount++;
}

void
LGL_Image::
DecrementReferenceCount()
{
	ReferenceCount--;
	assert(ReferenceCount>=0);
}

long
LGL_Image::
GetFrameNumber()
{
	return(FrameNumber);
}

void
LGL_Image::
SetFrameNumber
(
	long	frameNumber
)
{
	FrameNumber=frameNumber;
}

const char*
LGL_Image::
GetVideoPath()
{
	return(VideoPath);
}

void
LGL_Image::
SetVideoPath
(
	const char*	videoPath
)
{
	strcpy(VideoPath,videoPath);
}

//YUV

void
LGL_Image::
YUV_Construct()
{
	YUV_ImgW=0;
	YUV_ImgH=0;
	YUV_TexW=0;
	YUV_TexH=0;

	for(int c=0;c<3;c++)
	{
		YUV_TextureGL[c]=0;
		YUV_PixelBufferObjectFrontGL[c]=0;
		YUV_PixelBufferObjectBackGL[c]=0;
	}

	if(ImageShader.IsLinked()==false)
	{
		if(ImageShader.VertCompiled()==false)
		{
			ImageShader.VertCompile("data/glsl/image_rgb.vert.glsl");
		}
		if(ImageShader.FragCompiled()==false)
		{
			ImageShader.FragCompile("data/glsl/image_rgb.frag.glsl");
		}
		if(ImageShader.IsLinked()==false)
		{
			ImageShader.Link();
		}
	}

	if(YUV_ImageShader.IsLinked()==false)
	{
		if(YUV_ImageShader.VertCompiled()==false)
		{
			YUV_ImageShader.VertCompile("data/glsl/image_yuv.vert.glsl");
		}
		if(YUV_ImageShader.FragCompiled()==false)
		{
			YUV_ImageShader.FragCompile("data/glsl/image_yuv.frag.glsl");
		}
		if(YUV_ImageShader.IsLinked()==false)
		{
			YUV_ImageShader.Link();
		}
	}
}

void
LGL_Image::
YUV_Destruct()
{
	YUV_DestructTextures();
	YUV_DeletePixelBufferObjects();
}

bool
LGL_Image::
YUV_Available()
{
	return(YUV_ImgW!=0);
}

void
LGL_Image::
YUV_ConstructTextures()
{
	for(int c=0;c<3;c++)
	{
		int w = (c==0) ? YUV_TexW : (YUV_TexW/2);
		int h = (c==0) ? YUV_TexH : (YUV_TexH/2);
		glGenTextures(1,&YUV_TextureGL[c]);
		glBindTexture(GL_TEXTURE_LGL,YUV_TextureGL[c]);
		glTexImage2D
		(
			GL_TEXTURE_LGL,
			0,			//Level of Detail=0
			GL_LUMINANCE,		//Internal Format
			w,
			h,
			0,			//Boarder=0
			GL_LUMINANCE,
			GL_UNSIGNED_BYTE,
			NULL
		);
		glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_WRAP_T, GL_CLAMP);
		//glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri
		(
			GL_TEXTURE_LGL,
			GL_TEXTURE_MIN_FILTER,
			GL_NEAREST
		);
		glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
}

void
LGL_Image::
YUV_DestructTextures()
{
	for(int c=0;c<3;c++)
	{
		if(YUV_TextureGL[c]!=0)
		{
			glDeleteTextures(1,&(YUV_TextureGL[c]));
			YUV_TextureGL[c]=0;
		}
	}

	YUV_DeletePixelBufferObjects();
}

void
LGL_Image::
YUV_UpdatePixelBufferObjects()
{
	if
	(
		YUV_ImgW<0 ||
		YUV_ImgH<0
	)
	{
		printf("LGL_Image::YUV_UpdatePixelBufferObjects(): Cannot create pixel buffer object with negative dimensions (%i x %i)!\n",YUV_ImgW,YUV_ImgH);
		return;
	}

	GLsizei sizeY = YUV_ImgW*YUV_ImgH;
	GLsizei sizeUV = sizeY/4;

	if(sizeY>YUV_PixelBufferObjectSize)
	{
		YUV_DeletePixelBufferObjects();
	}

	for(int c=0;c<3;c++)
	{
		if(YUV_PixelBufferObjectFrontGL[c]==0)
		{
			//Make a Pixel Buffer Object
			gl2GenBuffers(1,&(YUV_PixelBufferObjectFrontGL[c]));
			gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,YUV_PixelBufferObjectFrontGL[c]);
			gl2BufferData
			(
				GL_PIXEL_UNPACK_BUFFER,
				(GLsizeiptr)(&((c==0) ? sizeY : sizeUV)),
				NULL,
				GL_STREAM_DRAW
			);
			gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,0);

			//Make a Pixel Buffer Object
			gl2GenBuffers(1,&(YUV_PixelBufferObjectBackGL[c]));
			gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,YUV_PixelBufferObjectBackGL[c]);
			gl2BufferData
			(
				GL_PIXEL_UNPACK_BUFFER,
				(GLsizeiptr)(&((c==0) ? sizeY : sizeUV)),
				NULL,
				GL_STREAM_DRAW
			);
			gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,0);

			PixelBufferObjectSize=sizeY;
		}
	}
}

void
LGL_Image::
YUV_DeletePixelBufferObjects()
{
	for(int c=0;c<3;c++)
	{
		if(YUV_PixelBufferObjectFrontGL[c]>0)
		{
			gl2DeleteBuffers(1,&(YUV_PixelBufferObjectFrontGL[c]));
			gl2DeleteBuffers(1,&(YUV_PixelBufferObjectBackGL[c]));
			YUV_PixelBufferObjectFrontGL[c]=0;
			YUV_PixelBufferObjectBackGL[c]=0;
			YUV_PixelBufferObjectSize=0;
		}
	}
}

void
LGL_Image::
YUV_UpdateTexture
(
	int		w,
	int		h,
	unsigned char*	dataY,
	unsigned char*	dataU,
	unsigned char*	dataV,
	const char*	name
)
{
	if
	(
		w>YUV_TexW ||
		w<YUV_TexW/2 ||
		h>YUV_TexH ||
		h<YUV_TexH/2
	)
	{
		YUV_DestructTextures();

		YUV_ImgW=w;
		YUV_ImgH=h;
		YUV_TexW=LGL_NextPowerOfTwo(YUV_ImgW);
		YUV_TexH=LGL_NextPowerOfTwo(YUV_ImgH);

		YUV_ConstructTextures();
	}

	if(w!=YUV_ImgW)
	{
		ImgW=LGL_Min(w,TexW);
	}
	if(h!=YUV_ImgH)
	{
		ImgH=LGL_Min(h,TexH);
	}

	if
	(
		dataY==NULL ||
		dataU==NULL ||
		dataV==NULL
	)
	{
printf("LGL_Image::YUV_UpdateTexture(): NULL data! WTF!\n");
		return;
	}

	if(name==NULL) name = "NULL Name";
	if(name[0]=='!')
	{
		sprintf(Path,"%s",name);
		sprintf(PathShort,"%s",name);
	}
	else
	{
		sprintf(Path,"!%s",name);
		sprintf(PathShort,"!%s",name);
	}

	YUV_UpdatePixelBufferObjects();

	bool pboReady =
		gl2IsBuffer(YUV_PixelBufferObjectFrontGL[0]) &&
		gl2IsBuffer(YUV_PixelBufferObjectFrontGL[1]) &&
		gl2IsBuffer(YUV_PixelBufferObjectFrontGL[2]);

	if
	(
		YUV_TextureGL[0]==0 ||
		YUV_TextureGL[1]==0 ||
		YUV_TextureGL[2]==0
	)
	{
		YUV_ConstructTextures();
	}

	unsigned char* dataYUV[3];
	dataYUV[0]=dataY;
	dataYUV[1]=dataU;
	dataYUV[2]=dataV;
	for(int c=0;c<3;c++)
	{
		int imgWNow = (c==0) ? YUV_ImgW : (YUV_ImgW/2);
		int imgHNow = (c==0) ? YUV_ImgH : (YUV_ImgH/2);

		glBindTexture(GL_TEXTURE_LGL,YUV_TextureGL[c]);
		if(pboReady)
		{
			gl2BindBuffer
			(
				GL_PIXEL_UNPACK_BUFFER,
				YUV_PixelBufferObjectFrontGL[c]
			);
			gl2BufferData
			(
				GL_PIXEL_UNPACK_BUFFER,
				imgWNow*imgHNow,
				dataYUV[c],
				GL_STREAM_DRAW
			);
		}
		else
		{
			gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
		}

		glTexSubImage2D
		(
			GL_TEXTURE_LGL,
			0,			//Level of Detail=0
			0,			//X-Offset
			0,			//Y-Offset
			imgWNow,
			imgHNow,
			GL_LUMINANCE,
			GL_UNSIGNED_BYTE,
			pboReady?0:dataYUV[c]
		);

		/*
		if(pboReady)
		{
			gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,PixelBufferObjectBackGL);
			gl2BufferData(GL_PIXEL_UNPACK_BUFFER, ImgW*ImgH*bytesperpixel, 0, GL_STREAM_DRAW);
			GLubyte* pbo = (GLubyte*)gl2MapBuffer(GL_PIXEL_UNPACK_BUFFER,GL_WRITE_ONLY);
			if(pbo)
			{
				memcpy(pbo,data,ImgW*ImgH*bytesperpixel);
				gl2UnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
			}
		}
		*/

		gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,0);

		//Swap
		/*
		GLuint tmp=PixelBufferObjectBackGL;
		PixelBufferObjectBackGL=PixelBufferObjectFrontGL;
		PixelBufferObjectFrontGL=tmp;
		*/
	}

	LinearInterpolation=true;
}



int
ThreadAnimationLoader
(
	void* object
)
{
	LGL_Animation* animation=(LGL_Animation*)object;
	animation->LoadImages();
	return(0);
}

LGL_Animation::
LGL_Animation
(
	const
	char*	path,
	bool	loadInForkedThread
) :
	AnimationSem("Animation")
{
	sprintf(Path,"%s",path);

	char temp[1024];
	strcpy(temp,Path);
	char* ptr=&(temp[0]);
	while(strstr(ptr,"/"))
	{
		ptr=&(strstr(ptr,"/")[1]);
	}
	strcpy(PathShort,ptr);
	
	if(loadInForkedThread)
	{
		LGL_ThreadCreate(ThreadAnimationLoader,this);
	}
	else
	{
		LoadImages();
	}

	ReferenceCount=0;
}

LGL_Animation::
~LGL_Animation()
{
	if(ReferenceCount!=0)
	{
		printf("LGL_Animation::~LGL_Animation(): Error! Reference Count for Animation '%s' is %i, not 0!\n",Path,ReferenceCount);
		assert(ReferenceCount==0);
	}
	DeleteImages();
}

LGL_Image*
LGL_Animation::
GetImage
(
	unsigned int	index
)
{
	int indexmax=0;
	LGL_Image* ret=NULL;

	{
		LGL_ScopeLock AnimationSemLock(__FILE__,__LINE__,AnimationSem);
		indexmax=Animation.size()-1;
		ret=Animation[(int)LGL_Clamp(0,index,indexmax)];
	}

	return(ret);
}

LGL_Image*
LGL_Animation::
GetImage
(
	int	index
)
{
	return(GetImage((unsigned int)(index)));
}

LGL_Image*
LGL_Animation::
GetImage
(
	float	zeroToOne
)
{
	return(GetImage((int)GetIndexFromFloat(zeroToOne)));
}

LGL_Image*
LGL_Animation::
GetImage
(
	double	zeroToOne
)
{
	return(GetImage((int)GetIndexFromFloat(zeroToOne)));
}

unsigned int
LGL_Animation::
GetIndexFromFloat
(
	float	zeroToOne
)
{
	return((unsigned int)floor(zeroToOne*(Animation.size()-1+.5)));
}

unsigned int
LGL_Animation::
Size()
{
	int ret=0;
	
	{
		LGL_ScopeLock AnimationSemLock(__FILE__,__LINE__,AnimationSem);
		ret=Animation.size();
	}
	
	return(ret);
}

float
LGL_Animation::
GetPercentLoaded()
{
	return((float)Animation.size()/(float)ImageCountMax);
}

bool
LGL_Animation::
IsLoaded()
{
	return
	(
		Animation.size()==ImageCountMax &&
		ImageCountMax!=0
	);
}

void
LGL_Animation::
LoadImages()
{
	std::vector<char*> DirList=LGL_DirectoryListCreate(Path);

	ImageCountMax=DirList.size();
	if(ImageCountMax==0)
	{
		printf
		(
			"LGL_Animation::LoadImage(): Error! No Images in '%s'\n",
			Path
		);
		LGL_Exit();
	}
	char target[1024];
	for(unsigned int a=0;a<ImageCountMax;a++)
	{
		sprintf(target,"%s/%s",Path,DirList[a]);
		LGL_Image* newbie=new LGL_Image(target);
		{
			LGL_ScopeLock AnimationSemLock(__FILE__,__LINE__,AnimationSem);
			Animation.push_back(newbie);
		}
		delete DirList[a];
		LGL_DelayMS(1);
	}

	//Check to see if images are of uniform dimensions

	int w=Animation[0]->GetWidth();
	int h=Animation[0]->GetHeight();

	for(unsigned int a=0;a<Animation.size();a++)
	{
		if
		(
			Animation[a]->GetWidth()!=w ||
			Animation[a]->GetHeight()!=h
		)
		{
			printf
			(
				"LGL_Animation::LoadImages(): Warning! Animation '%s' (size %i) has inconsistent dimensions!\n",
				Path,
				(int)Animation.size()
			);
			break;
		}
	}
}

const
char*
LGL_Animation::
GetPath()	const
{
	return(Path);
}

const
char*
LGL_Animation::
GetPathShort()	const
{
	return(PathShort);
}

int
LGL_Animation::
GetReferenceCount()
{
	return(ReferenceCount);
}

void
LGL_Animation::
IncrementReferenceCount()
{
	ReferenceCount++;
}

void
LGL_Animation::
DecrementReferenceCount()
{
	ReferenceCount--;
	assert(ReferenceCount>=0);
}

void
LGL_Animation::
DeleteImages()
{
	ImageCountMax=0;
	for(unsigned int a=0;a<Animation.size();a++)
	{
		delete Animation[a];
	}
}

LGL_Sprite::
LGL_Sprite
(
	const
	char*	path
)
{
	Type=LGL_SPRITE_NULL;
	Image=NULL;
	Animation=NULL;

	SetPath(path);
}

LGL_Sprite::
~LGL_Sprite()
{
	SetNull();
}

LGL_SpriteType
LGL_Sprite::
GetType()	const
{
	return(Type);
}

const
char*
LGL_Sprite::
GetPath()	const
{
	if(Type==LGL_SPRITE_NULL)
	{
		return(NULL);
	}
	else if(Type==LGL_SPRITE_IMAGE)
	{
		assert(Image!=NULL);
		return(Image->GetPath());
	}
	else if(Type==LGL_SPRITE_ANIMATION)
	{
		assert(Animation!=NULL);
		return(Animation->GetPath());
	}
	else
	{
		assert(false);
	}
}

const
char*
LGL_Sprite::
GetPathShort()	const
{
	if(Type==LGL_SPRITE_NULL)
	{
		return(NULL);
	}
	else if(Type==LGL_SPRITE_IMAGE)
	{
		assert(Image!=NULL);
		return(Image->GetPathShort());
	}
	else if(Type==LGL_SPRITE_ANIMATION)
	{
		assert(Animation!=NULL);
		return(Animation->GetPathShort());
	}
	else
	{
		assert(false);
	}
}

LGL_Image*
LGL_Sprite::
GetImage()	const
{
	assert(Type==LGL_SPRITE_IMAGE);
	assert(Image!=NULL);

	return(Image);
}

LGL_Animation*
LGL_Sprite::
GetAnimation()	const
{
	assert(Type==LGL_SPRITE_ANIMATION);
	assert(Animation!=NULL);

	return(Animation);
}

bool
LGL_Sprite::
SetNull()
{
	if(Type==LGL_SPRITE_NULL)
	{
		assert(Image==NULL);
		assert(Animation==NULL);
	}
	else if(Type==LGL_SPRITE_IMAGE)
	{
		assert(Animation==NULL);
		
		/*
		Image->DecrementReferenceCount();
		if(Image->GetReferenceCount()==0)
		{
			delete Image;
		}
		*/

		Image=NULL;
		Type=LGL_SPRITE_NULL;
	}
	else if(Type==LGL_SPRITE_ANIMATION)
	{
		assert(Image==NULL);
		
		/*
		Animation->DecrementReferenceCount();
		if(Animation->GetReferenceCount()==0)
		{
			delete Animation;
		}
		*/

		Animation=NULL;
		Type=LGL_SPRITE_NULL;
	}
	else
	{
		assert(false);
	}

	return(true);
}

bool
LGL_Sprite::
SetPath
(
	const
	char*	path
)
{
	if
	(
		path==NULL ||
		path[0]=='\0'
	)
	{
		return(SetNull());
	}
	else
	{
		if(LGL_DirectoryExists(path))
		{
			SetNull();
			Animation=new LGL_Animation(path);
			//Animation->IncrementReferenceCount();
			Type=LGL_SPRITE_ANIMATION;
			return(true);
		}
		else if(LGL_FileExists(path))
		{
			SetNull();
			Image=new LGL_Image(path);
			//Image->IncrementReferenceCount();
			Type=LGL_SPRITE_IMAGE;
			return(true);
		}
		else
		{
			SetNull();
			return(false);
		}
	}
}

bool
LGL_Sprite::
SetImage
(
	LGL_Image*	image
)
{
	SetNull();
	if(image!=NULL)
	{
		Image=image;
		Type=LGL_SPRITE_IMAGE;
		//Image->IncrementReferenceCount();
	}

	return(true);
}

bool
LGL_Sprite::
SetAnimation
(
	LGL_Animation*	animation
)
{
	SetNull();
	if(animation!=NULL)
	{
		Animation=animation;
		Type=LGL_SPRITE_ANIMATION;
		//Animation->IncrementReferenceCount();
	}

	return(true);
}


bool lgl_FrameBufferSortContainerSortPredicate(const lgl_FrameBufferSortContainer* d1, const lgl_FrameBufferSortContainer* d2)
{
	return(d1->FrameNumber < d2->FrameNumber);
}

bool lgl_LongSortPredicate(const long d1, const long d2)
{
	return(d1 < d2);
}

lgl_FrameBuffer::
lgl_FrameBuffer() :
	PacketSemaphore("Packet Semaphore"),
	BufferSemaphore("Buffer Semaphore")
{
	Ready=false;
	Buffer=NULL;
	BufferBytes=0;
	BufferIsRGB=true;
	BufferWidth=0;
	BufferHeight=0;
	FrameNumber=-1;
	Packet=NULL;
}

lgl_FrameBuffer::
~lgl_FrameBuffer()
{
	NullifyBuffer();

	SetPacket
	(
		NULL,
		NULL,
		-1
	);
}

bool
lgl_FrameBuffer::
IsLoaded()
{
	return
	(
		FrameNumber!=-1 &&
		Packet
	);
}

bool
lgl_FrameBuffer::
IsReady()
{
	return
	(
		FrameNumber!=-1 &&
		Packet &&
		Buffer &&
		Ready
	);
}

void
lgl_FrameBuffer::
NullifyBuffer()
{
	LGL_ScopeLock bufferLock(__FILE__,__LINE__,BufferSemaphore);
	if(Buffer)
	{
		delete Buffer;
		Buffer=NULL;
	}
	BufferBytes=0;
	Invalidate();
}

void
lgl_FrameBuffer::
SwapInNewBufferRGB
(
	char*		videoPath,
	unsigned char*&	bufferRGB,
	unsigned int&	bufferRGBBytes,
	int		bufferWidth,
	int		bufferHeight,
	long		frameNumber
)
{
	LGL_ScopeLock bufferLock(__FILE__,__LINE__,BufferSemaphore);

	//unsigned char* bufferOld=Buffer;
	//unsigned int bufferBytesOld=BufferBytes;

	if(BufferBytes<bufferRGBBytes)
	{
		if(Buffer)
		{
			delete Buffer;
		}
		Buffer = new unsigned char[bufferRGBBytes];
	}
	//Buffer=bufferRGB;
	memcpy(Buffer,bufferRGB,bufferRGBBytes);
	BufferBytes=bufferRGBBytes;
	BufferWidth=bufferWidth;
	BufferHeight=bufferHeight;
	FrameNumber=frameNumber;
	strcpy(VideoPath,videoPath?videoPath:"");
	BufferIsRGB=true;
	Ready=true;

	//bufferRGB=bufferOld;
	//bufferRGBBytes=bufferBytesOld;
}

void
lgl_FrameBuffer::
SwapInNewBufferYUV
(
	char*		videoPath,
	unsigned char*&	bufferYUV,
	unsigned int&	bufferYUVBytes,
	int		bufferWidth,
	int		bufferHeight,
	long		frameNumber
)
{
	LGL_ScopeLock bufferLock(__FILE__,__LINE__,BufferSemaphore);

	//unsigned char* bufferOld=Buffer;
	//unsigned int bufferBytesOld=BufferBytes;

	if(BufferBytes<bufferYUVBytes)
	{
		if(Buffer)
		{
			delete Buffer;
		}
		Buffer = new unsigned char[bufferYUVBytes];
	}
	//Buffer=bufferYUV;
	memcpy(Buffer,bufferYUV,bufferYUVBytes);
	BufferBytes=bufferYUVBytes;
	BufferWidth=bufferWidth;
	BufferHeight=bufferHeight;
	FrameNumber=frameNumber;
	strcpy(VideoPath,videoPath?videoPath:"");
	BufferIsRGB=false;
	Ready=true;

	//bufferYUV=bufferOld;
	//bufferYUVBytes=bufferBytesOld;
}

unsigned char*
lgl_FrameBuffer::
LockBufferRGB
(
	unsigned int	bufferRGBBytes
)
{
	bool obtained=BufferSemaphore.Lock(__FILE__,__LINE__,0.0f);

	if(obtained==false)
	{
		return(NULL);
	}

	if(BufferBytes<bufferRGBBytes)
	{
		if(Buffer)
		{
			delete Buffer;
		}
		Buffer = new unsigned char[bufferRGBBytes];
		BufferBytes=bufferRGBBytes;
	}

	return(Buffer);
}

void
lgl_FrameBuffer::
UnlockBufferRGB
(
const	char*		videoPath,
	int		bufferWidth,
	int		bufferHeight,
	long		frameNumber
)
{
	BufferWidth=bufferWidth;
	BufferHeight=bufferHeight;
	FrameNumber=frameNumber;
	strcpy(VideoPath,videoPath?videoPath:"");
	BufferIsRGB=true;
	Ready=true;

	BufferSemaphore.Unlock();
}

const char*
lgl_FrameBuffer::
GetVideoPath()	const
{
	return(VideoPath);
}

unsigned char*
lgl_FrameBuffer::
GetBufferRGB()	const
{
	return(BufferIsRGB ? Buffer : NULL);
}

unsigned int
lgl_FrameBuffer::
GetBufferRGBBytes()	const
{
	return(BufferIsRGB ? BufferBytes : 0);
}

unsigned char*
lgl_FrameBuffer::
GetBufferYUV()	const
{
	return(BufferIsRGB ? NULL : Buffer);
}

unsigned int
lgl_FrameBuffer::
GetBufferYUVBytes()	const
{
	return(BufferIsRGB ? 0 : BufferBytes);
}

int
lgl_FrameBuffer::
GetBufferWidth()	const
{
	return(BufferWidth);
}

int
lgl_FrameBuffer::
GetBufferHeight()	const
{
	return(BufferHeight);
}

long
lgl_FrameBuffer::
GetFrameNumber()	const
{
	return(FrameNumber);
}

unsigned char*
lgl_FrameBuffer::
GetBufferY()	const
{
	return(BufferIsRGB ? NULL : Buffer);
}

unsigned int
lgl_FrameBuffer::
GetBufferYBytes()	const
{
	return(BufferWidth*BufferHeight);
}

unsigned char*
lgl_FrameBuffer::
GetBufferU()	const
{
	return
	(
		BufferIsRGB ?
		NULL :
		&Buffer[GetBufferYBytes()]
	);
}

unsigned int
lgl_FrameBuffer::
GetBufferUBytes()	const
{
	return((BufferWidth*BufferHeight)/4);
}

unsigned char*
lgl_FrameBuffer::
GetBufferV()	const
{
	return
	(
		BufferIsRGB ?
		NULL :
		&Buffer[GetBufferYBytes()+GetBufferUBytes()]
	);
}

unsigned int
lgl_FrameBuffer::
GetBufferVBytes()	const
{
	return((BufferWidth*BufferHeight)/4);
}

AVPacket*
lgl_FrameBuffer::
LockPacket()
{
	bool locked=PacketSemaphore.Lock(__FILE__,__LINE__,0.0f);
	if(!locked)
	{
		return(NULL);
	}
	return(Packet);
}

void
lgl_FrameBuffer::
UnlockPacket()
{
	PacketSemaphore.Unlock();
}

void
lgl_FrameBuffer::
SetPacket
(
	AVPacket*	packet,
	const char*	path,
	long		frameNumber
)
{
	LGL_ScopeLock packetLock(__FILE__,__LINE__,PacketSemaphore);
	if(Packet)
	{
		lgl_av_free_packet(Packet);
		delete Packet;
	}
	Packet=packet;

	strcpy(VideoPath,path?path:"");
	FrameNumber=frameNumber;
}

void
lgl_FrameBuffer::
Invalidate()
{
	Ready=false;
	FrameNumber=-1;
}

lgl_FrameBufferSortContainer::
lgl_FrameBufferSortContainer
(
	lgl_FrameBuffer*	inFrameBuffer
)
{
	FrameBuffer=inFrameBuffer;
	FrameNumber=FrameBuffer->GetFrameNumber();
}



bool
lgl_debug_process_in_thread()
{
	//Returning true results in a better framerate for the main thread.
	//return(true);
	//Returning false results in a better framerate for the videos we draw to the projection screen.
	return(false);
}

int
lgl_video_decoder_decode_thread
(
	void* ptr
)
{
	LGL_ThreadSetPriority(LGL_PRIORITY_VIDEO_DECODE,"LGL_VideoDecoder");
	LGL_VideoDecoder* dec = (LGL_VideoDecoder*)ptr;

	for(;;)
	{
		if(dec->GetThreadTerminate())
		{
			break;
		}
		bool imageDecoded=false;
		if
		(
			LGL_FPS()>=50 &&
			dec->GetDecodeInThread()
		)
		{
			imageDecoded=dec->MaybeDecodeImage();
		}
		LGL_DelayMS(imageDecoded ? 5 : 10);
	}

	return(0);
}

int
lgl_video_decoder_preload_thread
(
	void* ptr
)
{
	LGL_ThreadSetPriority(LGL_PRIORITY_VIDEO_PRELOAD,"LGL_VideoPreloader");
	LGL_VideoDecoder* dec = (LGL_VideoDecoder*)ptr;

	static LGL_Semaphore sem("video_preload",false);

	int pathNum=-1;

	for(;;)
	{
		if
		(
			pathNum!=dec->GetPathNum() &&
			dec->GetPath() &&
			dec->GetPath()[0]!='\0' &&
			strcmp(dec->GetPath(),"NULL")!=0
		)
		{
			pathNum=dec->GetPathNum();
			double bytesRead=0;
			double bytesTotal=LGL_FileLengthBytes(dec->GetPath());
			bool preload=
				(
					dec->GetPreloadMaxMB() >
					(LGL_FileLengthBytes(dec->GetPath())/(1024.0f*1024.0f))
				);
/*
printf("preload '%s'? %s: %.2f vs %.2f\n",
	dec->GetPath(),
	preload ? "YES" : "NO",
	dec->GetPreloadMaxMB(),
	LGL_FileLengthBytes(dec->GetPath())/(1024.0f*1024.0f));
*/
			dec->SetPreloadPercent(0.0f);
			dec->SetPreloadEnabled(preload);
			if(preload)
			{
				if(FILE* fd=fopen(dec->GetPath(),"rb"))
				{
					const int readSize=1024*1024*8;
					char* readBuf=new char[readSize];

					long posBytes=dec->GetPosBytes();
					if(dec->GetPreloadFromCurrentTime()==false)
					{
						posBytes=0;
					}

					//Read from posBytes => EOF
					fseek(fd,posBytes,SEEK_SET);
					for(;;)
					{
						LGL_ScopeLock lock(__FILE__,__LINE__,sem);
						if(dec->GetThreadTerminate())
						{
							break;
						}
						if(feof(fd))
						{
							break;
						}
						if(pathNum!=dec->GetPathNum())
						{
							break;
						}
						fread(readBuf,readSize,1,fd);
						bytesRead+=readSize;
						dec->SetPreloadPercent(LGL_Clamp(0.0f,bytesRead/bytesTotal,1.0f));
						LGL_DelayMS(1);
					}
					
					//Read from 0 => posBytes
					if(posBytes!=0)
					{
						fseek(fd,0,SEEK_SET);
						for(;;)
						{
							LGL_ScopeLock lock(__FILE__,__LINE__,sem);
							if(dec->GetThreadTerminate())
							{
								break;
							}
							if(feof(fd))
							{
								break;
							}
							if(pathNum!=dec->GetPathNum())
							{
								break;
							}
							if(ftell(fd)>=posBytes)
							{
								break;
							}
							fread(readBuf,readSize,1,fd);
							bytesRead+=readSize;
							dec->SetPreloadPercent(LGL_Clamp(0.0f,bytesRead/bytesTotal,1.0f));
						}
					}

					delete readBuf;

					fclose(fd);
				}
			}
		}
		else
		{
			LGL_DelayMS(50);
		}

		if(dec->GetThreadTerminate())
		{
			break;
		}
	}

	return(0);
}

int
lgl_video_decoder_readahead_thread
(
	void* ptr
)
{
	LGL_ThreadSetPriority(LGL_PRIORITY_VIDEO_READAHEAD,"LGL_VideoDecoder");
	LGL_VideoDecoder* dec = (LGL_VideoDecoder*)ptr;

	for(;;)
	{
		dec->MaybeReadAhead();
		if(dec->GetThreadTerminate())
		{
			break;
		}
		LGL_DelayMS(dec->GetReadAheadDelayMS());
	}

	return(0);
}

int
lgl_video_decoder_load_thread
(
	void* ptr
)
{
	LGL_ThreadSetPriority(LGL_PRIORITY_VIDEO_LOAD,"LGL_VideoLoad");
	LGL_VideoDecoder* dec = (LGL_VideoDecoder*)ptr;

	static LGL_Semaphore sem("video_load",false);

	for(;;)
	{
		{
			LGL_ScopeLock lock(__FILE__,__LINE__,sem);
			for(int a=0;a<30;a++)
			{
				if(dec->GetThreadTerminate())
				{
					break;
				}
				dec->MaybeLoadVideo();
				dec->MaybeLoadImage();
			}
		}
		if(dec->GetThreadTerminate())
		{
			break;
		}
		LGL_DelayMS(1);
	}

	return(0);
}

LGL_VideoDecoder::
LGL_VideoDecoder
(
	const char* path
) :
	PathSemaphore("Path Semaphore"),
	PathNextSemaphore("Path Next Semaphore"),
	VideoOKSemaphore("VideoOK Semaphore")
{
	Init();
	SetVideo(path);
}

LGL_VideoDecoder::
~LGL_VideoDecoder()
{
	ThreadTerminate=true;
	if(ThreadPreload)
	{
		LGL_ThreadWait(ThreadPreload);
		ThreadPreload=NULL;
	}
	if(ThreadReadAhead)
	{
		LGL_ThreadWait(ThreadReadAhead);
		ThreadReadAhead=NULL;
	}
	if(ThreadLoad)
	{
		LGL_ThreadWait(ThreadLoad);
		ThreadLoad=NULL;
	}
	if(ThreadDecode)
	{
		LGL_ThreadWait(ThreadDecode);
		ThreadDecode=NULL;
	}

	UnloadVideo();

	for(unsigned int a=0;a<FrameBufferList.size();a++)
	{
		delete FrameBufferList[a];
	}

	for(int a=0;a<PathNextAttempts.size();a++)
	{
		delete PathNextAttempts[a];
	}
	PathNextAttempts.clear();

	//TODO: Doesn't this leak BufferRGB / BufferYUV...? YES, but only upon quitting dvj.
}

void
LGL_VideoDecoder::
Init()
{
	Path[0]='\0';
	PathShort[0]='\0';
	PathNext[0]='\0';
	PathNum=0;

	UserString[0]='\0';

	FPS=0.0f;
	FPSTimestamp=0.0;
	FPSDisplayed=0;
	FPSMissed=0;
	FPSDisplayedHitCounter=0;
	FPSDisplayedMissCounter=0;
	FPSDisplayedHitMissFrameNumber=0;
	LengthSeconds=0;
	TimeSeconds=0;
	TimeSecondsPrev=0;
	FrameNumberNext=-1;
	FrameNumberDisplayed=-1;
	NextRequestedDecodeFrame=-1;
	PosBytes=0;

	for(int a=0;a<500;a++)
	{
		lgl_FrameBuffer* fb = new lgl_FrameBuffer;
		FrameBufferList.push_back(fb);
	}
	SetFrameBufferAddBackwards(true);
	SetFrameBufferAddRadius(2);	//Also initializes FrameBufferSubtractRadius
	PreloadMaxMB=0;
	PreloadEnabled=false;
	PreloadFromCurrentTime=true;
	PreloadPercent=0.0f;
	ReadAheadMB=16;
	ReadAheadDelayMS=500;
	DecodeInThread=true;

	FormatContext=NULL;
	CodecContext=NULL;
	Codec=NULL;
	VideoStreamIndex=-1;
	FrameNative=NULL;	//Memleak?
	FrameRGB=NULL;		//Memleak?
	BufferRGB=NULL;
	BufferRGBBytes=0;
	BufferWidth=-1;
	BufferHeight=-1;
	SwsConvertContextBGRA=NULL;
	QuicktimeMovie=NULL;

	BufferYUV=NULL;
	BufferYUVBytes=0;

	BufferYUVAsRGB=NULL;
	BufferYUVAsRGBBytes=0;

	IsImage=false;
	Image=NULL;
	VideoOK=false;
	VideoOKUserCount=0;
	for(int a=0;a<2;a++)
	{
		StoredBrightness[a]=0.0f;
	}

	//Preallocate lgl_FrameBuffers
#if 0
	const int width = 1920;
	const int height = 1080;
	unsigned int bufferRGBBytes=4*width*height;//GetProjectorQuadrentResX()*GetProjectorQuadrentResY();
	for(long int a=0;a<FrameBufferAddRadius+FrameBufferSubtractRadius;a++)
	{
		try
		{
			unsigned char* buffer=new uint8_t[bufferRGBBytes];
			lgl_FrameBuffer* frameBuffer = GetRecycledFrameBuffer();
			unsigned char* oldie = frameBuffer->SwapInNewBufferRGB
			(
				NULL,
				buffer,
				bufferRGBBytes,
				width,
				height,
				-9999-a
			);
			if(oldie)
			{
				delete oldie;
				oldie=NULL;
			}
		}
		catch(std::bad_alloc)
		{
			printf("LGL_VideoDecoder(): Caught bad_alloc exception\n");
			break;
		}
		catch(std::exception)
		{
			printf("LGL_VideoDecoder(): Caught unknown exception\n");
			break;
		}
	}
#endif

	ThreadTerminate=false;
	ThreadPreload=NULL;
	ThreadReadAhead=NULL;
	ThreadLoad=NULL;
	ThreadDecode=NULL;
	ThreadPreload=LGL_ThreadCreate(lgl_video_decoder_preload_thread,this);
	ThreadReadAhead=LGL_ThreadCreate(lgl_video_decoder_readahead_thread,this);
	ThreadLoad=LGL_ThreadCreate(lgl_video_decoder_load_thread,this);
	if(1)//LGL_CPUCount()>=4)
	{
		ThreadDecode=LGL_ThreadCreate(lgl_video_decoder_decode_thread,this);
	}
}

void
LGL_VideoDecoder::
UnloadVideo()
{
	if(CodecContext)
	{
		lgl_avcodec_close(CodecContext);
		CodecContext=NULL;
	}
	
	if(SwsConvertContextBGRA)
	{
		sws_freeContext(SwsConvertContextBGRA);
		SwsConvertContextBGRA=NULL;
	}
	
	if(FormatContext)
	{
		lgl_av_close_input_file(FormatContext);
		//NOT necessary: lgl_av_freep(FormatContext);
		FormatContext=NULL;
	}

	if(QuicktimeMovie)
	{
#ifdef	LGL_OSX
		lgl_quicktime_close(QuicktimeMovie);
#endif
		QuicktimeMovie=NULL;
	}

	NextRequestedDecodeFrame=-1;
}

void
LGL_VideoDecoder::
SetVideo
(
	const char*	path
)
{
	if
	(
		path==NULL ||
		path[0]=='\0'
	)
	{
		path="NULL";
	}

	if(strcmp(Path,path)!=0)
	{
		LGL_ScopeLock pathNextLock(__FILE__,__LINE__,PathNextSemaphore);
		if(Image)
		{
			Image->SetFrameNumber(-1);
		}
		strcpy(PathNext,path);

		for(int a=0;a<PathNextAttempts.size();a++)
		{
			delete PathNextAttempts[a];
		}
		PathNextAttempts.clear();
	}
}

void
LGL_VideoDecoder::
SetVideo
(
	std::vector<const char*>	pathAttempts
)
{
	if(pathAttempts.size()==0)
	{
		SetVideo(NULL);
		return;
	}
	else if(pathAttempts.size()==1)
	{
		SetVideo(pathAttempts[0]);
		return;
	}
	else
	{
		if(strcmp(Path,pathAttempts[0])==0)
		{
			return;
		}

		LGL_ScopeLock pathNextLock(__FILE__,__LINE__,PathNextSemaphore);
		if(Image)
		{
			Image->SetFrameNumber(-1);
		}

		for(int a=0;a<PathNextAttempts.size();a++)
		{
			delete PathNextAttempts[a];
		}
		PathNextAttempts.clear();

		for(int a=0;a<pathAttempts.size();a++)
		{
			if
			(
				pathAttempts[a] &&
				pathAttempts[a][0] != '\0'
			)
			{
				const int neoSize = strlen(pathAttempts[a])+1;
				char* neo = new char[neoSize];
				strncpy(neo,pathAttempts[a],neoSize-1);
				neo[neoSize-1]='\0';
				PathNextAttempts.push_back(neo);
			}
		}
		char* neo = new char[16];
		strcpy(neo,"NULL");
		PathNextAttempts.push_back(neo);
	}
}

const char*
LGL_VideoDecoder::
GetPath()
{
	return(Path);
}

const char*
LGL_VideoDecoder::
GetPathShort()
{
	return(PathShort);
}

int
LGL_VideoDecoder::
GetPathNum()
{
	return(PathNum);
}

void
LGL_VideoDecoder::
SetTime
(
	double	seconds
)
{
	long ms = seconds*1000;
	if(LengthSeconds>0)
	{
		ms = ms % (long)(LengthSeconds*1000);
	}
	seconds = ms/1000.0f;

	TimeSecondsPrev=TimeSeconds;
	TimeSeconds=seconds;
}

double
LGL_VideoDecoder::
GetTime()
{
	return(TimeSeconds);
}

double
LGL_VideoDecoder::
GetLengthSeconds()
{
	return(LengthSeconds);
}

double
LGL_VideoDecoder::
GetFPS()
{
	return(FPS);
}

int
LGL_VideoDecoder::
GetFPSDisplayed()
{
	return(FPSDisplayed);
}
int
LGL_VideoDecoder::
GetFPSMissed()
{
	return(FPSMissed);
}

LGL_Image*
LGL_VideoDecoder::
GetImage
(
	bool	decodeAllowed
)
{
	MaybeInvalidateBuffers();

	if(IsImage)
	{
		if(Image)
		{
			if(strcmp(Image->GetPath(),Path)!=0)
			{
				delete Image;
				Image=NULL;
			}
		}

		if(Image==NULL)
		{
			if(LGL_FileExists(Path))
			{
				Image=new LGL_Image(Path);
				Image->SetFrameNumber(0);
			}
			else
			{
				Image = new LGL_Image
				(
					512,//1920,
					512,//1024,
					4,
					NULL,
					true,
					"Empty LGL_VideoDecoder"
				);
				Image->SetFrameNumber(-1);
			}
		}

		Image->InvertY=false;
		return(Image);
	}

	//Ensure Image Exists
	if(Image==NULL)
	{
		Image = new LGL_Image
		(
			512,//1920,
			512,//1024,
			4,
			NULL,
			true,
			"Empty LGL_VideoDecoder"
		);
		Image->SetFrameNumber(-1);
	}

	if(strcmp(Path,"NULL")==0)
	{
		Image->SetFrameNumber(-1);
		return(Image);
	}

	/*
	LGL_ScopeLock videoOKLock(__FILE__,__LINE__,VideoOKSemaphore,0.0f);
	if(videoOKLock.GetLockObtained()==false)
	{
		return(Image);
	}
	*/

	char path[2048];
	{
		LGL_ScopeLock pathLock(__FILE__,__LINE__,PathSemaphore,0.0f);
		if(pathLock.GetLockObtained()==false)
		{
			return(Image);
		}
		strcpy(path,Path);
	}
	if(strcmp(Image->GetVideoPath(),path)!=0)
	{
		Image->SetFrameNumber(-1);
	}

	long frameNumber = SecondsToFrameNumber(TimeSeconds);
	if(lgl_debug_process_in_thread())
	{
		SetNextRequestedDecodeFrame(frameNumber);
	}
	else
	{
		if(decodeAllowed)
		{
			MaybeDecodeImage(frameNumber);
		}
	}
	lgl_FrameBuffer* frameBuffer=NULL;

	std::vector<lgl_FrameBuffer*> frameBufferReady = GetFrameBufferReadyList(true);

	//Early out if nothing decoded...
	if(frameBufferReady.size()==0)
	{
		return(Image);
	}

	//Find the nearest framebuffer
	if(frameNumber<frameBufferReady[0]->GetFrameNumber())
	{
		if(FPSDisplayedHitMissFrameNumber!=LGL_FramesSinceExecution())
		{
			FPSDisplayedHitMissFrameNumber=LGL_FramesSinceExecution();
			FPSDisplayedMissCounter++;
		}
		frameBuffer=frameBufferReady[0];
	}
	else if(frameNumber>frameBufferReady[frameBufferReady.size()-1]->GetFrameNumber())
	{
		if(FPSDisplayedHitMissFrameNumber!=LGL_FramesSinceExecution())
		{
			FPSDisplayedHitMissFrameNumber=LGL_FramesSinceExecution();
			FPSDisplayedMissCounter++;
		}
		frameBuffer=frameBufferReady[frameBufferReady.size()-1];
	}
	else
	{
		long nearestDistance=99999999;
		for(unsigned int a=0;a<frameBufferReady.size();a++)
		{
			if((long)fabsf(frameNumber-frameBufferReady[a]->GetFrameNumber())<nearestDistance)
			{
				frameBuffer=frameBufferReady[a];
				nearestDistance=(long)fabsf(frameNumber-frameBufferReady[a]->GetFrameNumber());
			}
		}	
	}

	if(frameBuffer==NULL)
	{
		//FIXME: If we just made the imade, it contains junk.
		return(Image);
	}

	//Is our image already up to date?
	if(Image->GetFrameNumber()==frameBuffer->GetFrameNumber())
	{
		FrameNumberDisplayed=Image->GetFrameNumber();
		return(Image);
	}

	//Update Image
	char name[1024];
	sprintf(name,"%s",Path);
	Image->SetVideoPath(path);

	if
	(
		BufferWidth<0 ||
		BufferHeight<0
	)
	{
		Image->SetFrameNumber(-1);
		return(Image);
	}

	{
		LGL_ScopeLock bufferLock(__FILE__,__LINE__,frameBuffer->BufferSemaphore,
			(SDL_ThreadID()==LGL.ThreadIDMain) ? 0.0f : -1.0f
		);
		if(bufferLock.GetLockObtained()==false)
		{
			return(Image);
		}

		if(frameBuffer->GetBufferRGB())
		{
			if(IsYUV420P()==false)
			{
				Image->UpdateTexture
				(
					frameBuffer->GetBufferWidth(),
					frameBuffer->GetBufferHeight(),
					4,
					frameBuffer->GetBufferRGB(),
					true,
					name
				);
			}
			else
			{
				Image->YUV_UpdateTexture
				(
					BufferWidth,
					BufferHeight,
					frameBuffer->GetBufferY(),
					frameBuffer->GetBufferU(),
					frameBuffer->GetBufferV(),
					name
				);
			/*
			if(BufferYUVAsRGB==NULL)
			{
				BufferYUVAsRGBBytes=BufferWidth*BufferHeight*4;
				BufferYUVAsRGB=new unsigned char[BufferYUVAsRGBBytes];
			}

			if(unsigned char* bufChosen = frameBuffer->GetBufferV())
			{
				int width=(bufChosen == frameBuffer->GetBufferY()) ? frameBuffer->GetBufferWidth() : (frameBuffer->GetBufferWidth()/2);
				int height=(bufChosen == frameBuffer->GetBufferY()) ? frameBuffer->GetBufferHeight() : (frameBuffer->GetBufferHeight()/2);
				for(int h=0;h<height;h++)
				{
					for(int w=0;w<width;w++)
					{
						for(int c=0;c<3;c++)
						{
							BufferYUVAsRGB[c+4*(w+h*width)]=
								bufChosen[(w+h*width)];
						}
					}
				}

				Image->UpdateTexture
				(
					width,
					height,
					4,
					BufferYUVAsRGB,
					true,
					name
				);
			}
			*/
			}
		}
		else
		{
			return(Image);
		}
	}
	Image->SetFrameNumber(frameBuffer->GetFrameNumber());
	FrameNumberDisplayed=frameBuffer->GetFrameNumber();

	if(FPSDisplayedHitMissFrameNumber!=LGL_FramesSinceExecution())
	{
		FPSDisplayedHitMissFrameNumber=LGL_FramesSinceExecution();
		if(frameNumber==Image->GetFrameNumber())
		{
			FPSDisplayedHitCounter++;
		}
		else
		{
			FPSDisplayedMissCounter++;
		}
	}

	if(FPSDisplayedTimer.SecondsSinceLastReset()>=1.0f)
	{
		FPSDisplayed=FPSDisplayedHitCounter;
		FPSMissed=FPSDisplayedMissCounter;
		FPSDisplayedHitCounter=0;
		FPSDisplayedMissCounter=0;
		FPSDisplayedTimer.Reset();
	}

	return(Image);
}

double
LGL_VideoDecoder::
GetSecondsBufferedLeft
(
	bool	loaded,
	bool	ready
)
{
	ready|=loaded;
	std::vector<long> frameNumList;
	if(loaded)
	{
		std::vector<lgl_FrameBuffer*> frameBufferLoaded = GetFrameBufferLoadedList(false);
		for(unsigned int a=0;a<frameBufferLoaded.size();a++)
		{
			frameNumList.push_back(frameBufferLoaded[a]->GetFrameNumber());
		}
	}
	if(ready)
	{
		std::vector<lgl_FrameBuffer*> frameBufferReady = GetFrameBufferReadyList(false);
		for(unsigned int a=0;a<frameBufferReady.size();a++)
		{
			frameNumList.push_back(frameBufferReady[a]->GetFrameNumber());
		}
	}

	if(frameNumList.size()==0)
	{
		return(0.0f);
	}

	if(ready && loaded)
	{
		std::sort
		(
			frameNumList.begin(),
			frameNumList.end(),
			lgl_LongSortPredicate
		);
	}

	long frameNumberNow = SecondsToFrameNumber(TimeSeconds);
	long frameNumberLength = SecondsToFrameNumber(LengthSeconds);
	if(frameNumberLength==0)
	{
		return(0.0f);
	}
	if(frameNumberNow>frameNumberLength-1) frameNumberNow=frameNumberLength-1;
	int currentIndex=-1;
	for(unsigned int a=0;a<frameNumList.size();a++)
	{
		if(frameNumList[a]==frameNumberNow)
		{
			currentIndex=a;
			break;
		}
	}

	if(currentIndex==-1)
	{
		return(0.0f);
	}

	double seconds=0.0f;

	bool wrap=false;
	for(int a=currentIndex;;a--)
	{
		if(a==-1)
		{
			a=frameNumList.size()-1;
			wrap=true;
		}

		if(wrap && a==currentIndex)
		{
			break;
		}

		int next=a-1;
		if(next==-1)
		{
			next=frameNumList.size()-1;
		}

		if
		(
			frameNumList[a]-frameNumList[next] == 1 ||
			(
				next==(int)frameNumList.size()-1 &&
				fabsf(frameNumList[next] - SecondsToFrameNumber(LengthSeconds))<=1.0f &&
				frameNumList[a] == 0
			)
		)
		{
			seconds+=1.0f/FPS;
		}
		else
		{
			break;
		}
	}

	return(seconds);
}

double
LGL_VideoDecoder::
GetSecondsBufferedRight
(
	bool	loaded,
	bool	ready
)
{
	ready|=loaded;
	std::vector<long> frameNumList;
	if(loaded)
	{
		std::vector<lgl_FrameBuffer*> frameBufferLoaded = GetFrameBufferLoadedList(false);
		for(unsigned int a=0;a<frameBufferLoaded.size();a++)
		{
			frameNumList.push_back(frameBufferLoaded[a]->GetFrameNumber());
		}
	}
	if(ready)
	{
		std::vector<lgl_FrameBuffer*> frameBufferReady = GetFrameBufferReadyList(false);
		for(unsigned int a=0;a<frameBufferReady.size();a++)
		{
			frameNumList.push_back(frameBufferReady[a]->GetFrameNumber());
		}
	}

	if(frameNumList.size()==0)
	{
		return(0.0f);
	}

	if(ready && loaded)
	{
		std::sort
		(
			frameNumList.begin(),
			frameNumList.end(),
			lgl_LongSortPredicate
		);
	}

	long frameNumberNow = SecondsToFrameNumber(TimeSeconds);
	long frameNumberLength = SecondsToFrameNumber(LengthSeconds);
	if(frameNumberLength==0)
	{
		return(0.0f);
	}
	if(frameNumberNow>frameNumberLength-1) frameNumberNow=frameNumberLength-1;
	int currentIndex=-1;
	for(unsigned int a=0;a<frameNumList.size();a++)
	{
		if(frameNumList[a]==frameNumberNow)
		{
			currentIndex=a;
			break;
		}
	}

	if(currentIndex==-1)
	{
		return(0.0f);
	}
	
	double seconds=0.0f;
	bool wrap=false;
	for(int a=currentIndex;;a++)
	{
		if(a==(int)frameNumList.size())
		{
			a=0;
			wrap=true;
		}

		if(wrap && a==currentIndex)
		{
			break;
		}

		int next=a+1;
		if(next==(int)frameNumList.size())
		{
			next=0;
		}

		if
		(
			frameNumList[next]-frameNumList[a] == 1 ||
			(
				next==0 &&
				fabsf(frameNumList[a] - SecondsToFrameNumber(LengthSeconds))<=1.0f &&
				frameNumList[next] == 0
			)
		)
		{
			seconds+=1.0f/FPS;
		}
		else
		{
			break;
		}
	}

	return(seconds);
}

void
LGL_VideoDecoder::
SetFrameBufferAddBackwards
(
	bool	addBackwards
)
{
	FrameBufferAddBackwards=addBackwards;
}

int
LGL_VideoDecoder::
GetFrameBufferAddRadius()	const
{
	return(FrameBufferAddRadius);
}

void
LGL_VideoDecoder::
SetFrameBufferAddRadius
(
	int	frames
)
{
	FrameBufferAddRadius=LGL_Max(2,frames);
	FrameBufferSubtractRadius=(int)(FrameBufferAddRadius*1.0f);;
}

void
LGL_VideoDecoder::
InvalidateAllFrameBuffers()
{
	if(Image)
	{
		Image->SetFrameNumber(-1);
	}
	for(unsigned int a=0;a<FrameBufferList.size();a++)
	{
		FrameBufferList[a]->Invalidate();
	}
}

void
LGL_VideoDecoder::
SetNextRequestedDecodeFrame
(
	long	frameNum
)
{
	NextRequestedDecodeFrame=frameNum;
}

long
LGL_VideoDecoder::
GetNextRequestedDecodeFrame()
{
	//What is supposed to happen here??
	/*
	std::vector<lgl_FrameBuffer*> frameBufferLoaded = GetFrameBufferLoadedList(false);
	for(unsigned int a=0;a<frameBufferLoaded.size();a++)
	{
		frameBufferLoaded[0]->GetFrameNumber();
	}
	*/
	return(NextRequestedDecodeFrame);
}

long
LGL_VideoDecoder::
GetPosBytes()
{
	return((long)PosBytes);
}

float
LGL_VideoDecoder::
GetPreloadMaxMB()
{
	return(PreloadMaxMB);
}

void
LGL_VideoDecoder::
SetPreloadMaxMB(float maxMB)
{
	PreloadMaxMB=maxMB;
}

bool
LGL_VideoDecoder::
GetPreloadEnabled()
{
	return(PreloadEnabled);
}

void
LGL_VideoDecoder::
SetPreloadEnabled
(
	bool	enabled
)
{
	PreloadEnabled=enabled;
}

void
LGL_VideoDecoder::
ForcePreload()
{
	PathNum++;
}

bool
LGL_VideoDecoder::
GetPreloadFromCurrentTime()
{
	return(PreloadFromCurrentTime);
}

void
LGL_VideoDecoder::
SetPreloadFromCurrentTime
(
	bool fromCurrentTime
)
{
	PreloadFromCurrentTime=fromCurrentTime;
}

float
LGL_VideoDecoder::
GetPreloadPercent()
{
	return(PreloadPercent);
}

void
LGL_VideoDecoder::
SetPreloadPercent
(
	float	pct
)
{
	PreloadPercent=pct;
}

int
LGL_VideoDecoder::
GetReadAheadMB()
{
	return(ReadAheadMB);
}

void
LGL_VideoDecoder::
SetReadAheadMB
(
	int	mb
)
{
	ReadAheadMB=mb;
}

int
LGL_VideoDecoder::
GetReadAheadDelayMS()
{
	return(ReadAheadDelayMS);
}

void
LGL_VideoDecoder::
SetReadAheadDelayMS
(
	int	delayMS
)
{
	ReadAheadDelayMS=delayMS;
}

bool
LGL_VideoDecoder::
GetDecodeInThread()
{
	return(DecodeInThread);
}

void
LGL_VideoDecoder::
SetDecodeInThread
(
	bool	decodeInThread
)
{
	DecodeInThread=decodeInThread;
}

void
LGL_VideoDecoder::
SetUserString
(
	const char*	str
)
{
	if(str)
	{
		strncpy(UserString,str,sizeof(UserString)-1);
		UserString[sizeof(UserString)-1]='\0';
	}
}

const char*
LGL_VideoDecoder::
GetUserString()
{
	return(UserString);
}

void
LGL_VideoDecoder::
MaybeLoadVideo()
{
	if(PathNext[0]=='\0')
	{
		LGL_ScopeLock pathNextLock(__FILE__,__LINE__,PathNextSemaphore);
		if(PathNextAttempts.size()!=0)
		{
			strncpy(PathNext,PathNextAttempts[0],sizeof(PathNext)-1);
			PathNext[sizeof(PathNext)-1]='\0';

			PathNextAttempts.erase
			(
				(std::vector<const char*>::iterator)
				(&PathNextAttempts[0])
			);
		}

		return;
	}

	char pathNextLocal[2048];
	{
		LGL_ScopeLock pathNextLock(__FILE__,__LINE__,PathNextSemaphore);
		strcpy(pathNextLocal,PathNext);
		PathNext[0]='\0';
	}

	FPSDisplayed=0;
	FPSMissed=0;
	FPSDisplayedHitCounter=0;
	FPSDisplayedMissCounter=0;

	if(strcmp(pathNextLocal,"NULL")==0)
	{
		strcpy(Path,pathNextLocal);
		return;
	}

	//pathNextLocal => Path
	{
		LGL_ScopeLock pathLock(__FILE__,__LINE__,PathSemaphore);

		if(LGL_FileExists(pathNextLocal)==false)
		{
			return;
		}

		strcpy(Path,pathNextLocal);
		if(const char* lastSlash = strrchr(Path,'/'))
		{
			strcpy(PathShort,&(lastSlash[1]));
		}
		else
		{
			strcpy(PathShort,Path);
		}

		PathNum++;
	}

	//Go for it!!

	int delayCount=0;
	for(;;)
	{
		if(delayCount>0) LGL_DelayMS(1);
		if(VideoOKUserCount>0)
		{
			delayCount++;
			continue;
		}
		LGL_ScopeLock videoOkLock(__FILE__,__LINE__,VideoOKSemaphore,0.0f);
		if(videoOkLock.GetLockObtained()==false)
		{
			delayCount++;
			continue;
		}

		VideoOK=false;
		IsImage=false;
		UnloadVideo();

		if(LGL_FileExtensionIsImage(Path))
		{
			IsImage=true;
			return;
		}

		//Open file
		AVFormatContext* fc=NULL;
		{
			if(lgl_av_open_input_file(&fc, Path, NULL, 0, NULL)!=0)
			{
				printf("LGL_VideoDecoder::MaybeLoadVideo(): Couldn't open '%s'\n",Path);
				return;
			}
		}


		//Find streams
		if(lgl_av_find_stream_info(fc)<0)
		{
			printf("LGL_VideoDecoder::MaybeLoadVideo(): Couldn't find streams for '%s'\n",Path);
			return;
		}


		// Find the first video stream
		for(unsigned int i=0; i<fc->nb_streams; i++)
		{
			if(fc->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO)
			{
				VideoStreamIndex=i;
				break;
			}
		}
		if(VideoStreamIndex==-1)
		{
			printf("LGL_VideoDecoder::MaybeLoadVideo(): Couldn't find video stream for '%s' in %i streams\n",Path,fc->nb_streams);
			return;
		}

		// Get a pointer to the codec context for the video stream
		CodecContext=fc->streams[VideoStreamIndex]->codec;
		if
		(
			!CodecContext->codec_id == CODEC_ID_MJPEG &&
			!CodecContext->codec_id == CODEC_ID_MJPEGB &&
			!CodecContext->codec_id == CODEC_ID_LJPEG
		)
		{
			printf("LGL_VideoDecoder::MaybeLoadVideo(): Video isn't mjpeg: '%s'\n",Path);
			return;
		}

		FormatContext=fc;	//Only set this once fc is fully initialized

		// Find the decoder for the video stream
		Codec=lgl_avcodec_find_decoder(CodecContext->codec_id);
		if(Codec==NULL)
		{
			printf("LGL_VideoDecoder::MaybeLoadVideo(): Couldn't find codec for '%s'. Codec = '%s'\n",Path,CodecContext->codec_name);
			UnloadVideo();
			return;
		}

		// Inform the codec that we can handle truncated bitstreams -- i.e.,
		// bitstreams where frame boundaries can fall in the middle of packets
		if(Codec->capabilities & CODEC_CAP_TRUNCATED)
		{
			CodecContext->flags|=CODEC_FLAG_TRUNCATED;
		}

		// Open codec
		{
			if(lgl_avcodec_open(CodecContext, Codec)<0)
			{
				printf("LGL_VideoDecoder::MaybeLoadVideo(): Couldn't open codec for '%s'. Codec = '%s'\n",Path,CodecContext->codec_name);
				UnloadVideo();
				return;
			}
		}

		LengthSeconds=FormatContext->duration/(double)(AV_TIME_BASE);
		FPS=
			FormatContext->streams[VideoStreamIndex]->r_frame_rate.num/(double)
			FormatContext->streams[VideoStreamIndex]->r_frame_rate.den;
	/*
	printf("FPS A = %i / %i (%.2f)\n",
			FormatContext->streams[VideoStreamIndex]->r_frame_rate.num,
			FormatContext->streams[VideoStreamIndex]->r_frame_rate.den,
			FormatContext->streams[VideoStreamIndex]->r_frame_rate.num/(float)
			FormatContext->streams[VideoStreamIndex]->r_frame_rate.den);
	printf("FPS B = %i / %i (%.2f)\n",
			FormatContext->streams[VideoStreamIndex]->time_base.num,
			FormatContext->streams[VideoStreamIndex]->time_base.den,
			FormatContext->streams[VideoStreamIndex]->time_base.num/(float)
			FormatContext->streams[VideoStreamIndex]->time_base.den);
	printf("FPS C = %i / %i (%.2f)\n",
			FormatContext->streams[VideoStreamIndex]->codec->time_base.num,
			FormatContext->streams[VideoStreamIndex]->codec->time_base.den,
			FormatContext->streams[VideoStreamIndex]->codec->time_base.num/(float)
			FormatContext->streams[VideoStreamIndex]->codec->time_base.den);
	printf("VidStream->r_frame_rate = %i / %i (%.2f)\n",
		FormatContext->streams[VideoStreamIndex]->r_frame_rate.num,
		FormatContext->streams[VideoStreamIndex]->r_frame_rate.den,
		FormatContext->streams[VideoStreamIndex]->r_frame_rate.num/(float)
		FormatContext->streams[VideoStreamIndex]->r_frame_rate.den);
	printf("VidStream->time_base = %i / %i (%.2f) (%.2f)\n",
		FormatContext->streams[VideoStreamIndex]->time_base.num,
		FormatContext->streams[VideoStreamIndex]->time_base.den,
		FormatContext->streams[VideoStreamIndex]->time_base.num/(float)
		FormatContext->streams[VideoStreamIndex]->time_base.den,
		FormatContext->streams[VideoStreamIndex]->time_base.den/(float)
		FormatContext->streams[VideoStreamIndex]->time_base.num);
	printf("VidStream->codec->time_base = %i / %i (%.2f) (%.2f)\n",
		FormatContext->streams[VideoStreamIndex]->codec->time_base.num,
		FormatContext->streams[VideoStreamIndex]->codec->time_base.den,
		FormatContext->streams[VideoStreamIndex]->codec->time_base.num/(float)
		FormatContext->streams[VideoStreamIndex]->codec->time_base.den,
		FormatContext->streams[VideoStreamIndex]->codec->time_base.den/(float)
		FormatContext->streams[VideoStreamIndex]->codec->time_base.num);
	printf("ticks_per_frame = %i\n",CodecContext->ticks_per_frame);
	*/
		FPSTimestamp=CodecContext->time_base.den/(double)CodecContext->time_base.num;
		FPS=FormatContext->streams[VideoStreamIndex]->nb_frames/LengthSeconds;

		if(FrameNative==NULL)
		{
			FrameNative=lgl_avcodec_alloc_frame();
		}
		if(FrameRGB==NULL)
		{
			FrameRGB=lgl_avcodec_alloc_frame();
		}

		if(FrameNative==NULL || FrameRGB==NULL)
		{
			printf("LGL_Video::MaybeChangeVideo(): Couldn't open frames for '%s'\n",Path);
			return;
		}

		// Determine required buffer size and pseudo-allocate buffer
		BufferWidth=CodecContext->width;
		BufferHeight=CodecContext->height;

		SwsConvertContextBGRA = lgl_sws_getContext
		(
			//src
			BufferWidth,
			BufferHeight, 
			CodecContext->pix_fmt, 
			//dst
			BufferWidth,
			BufferHeight,
			PIX_FMT_BGRA,
			SWS_FAST_BILINEAR,
			NULL,
			NULL,
			NULL
		);
		if(SwsConvertContextBGRA==NULL)
		{
			printf("LGL_VideoDecoder::MaybeLoadVideo(): NULL SwsConvertContextBGRA for '%s'\n",Path);
			return;
		}

		InvalidateAllFrameBuffers();

#ifdef	LGL_OSX
		QuicktimeMovie = lgl_quicktime_open
		(
			Path
		);
#endif

		//This can invalidate calls to SetVideo() that occur while loading video... Hmm.
		{
			LGL_ScopeLock pathNextLock(__FILE__,__LINE__,PathNextSemaphore);
			for(int a=0;a<PathNextAttempts.size();a++)
			{
				delete PathNextAttempts[a];
			}
			PathNextAttempts.clear();
		}

		VideoOK=true;
		break;
	}
}

bool
LGL_VideoDecoder::
MaybeReadAhead()
{
	if
	(
		FormatContext==NULL ||
		CodecContext==NULL ||
		FrameNative==NULL ||
		FrameRGB==NULL ||
		strcmp(Path,"NULL")==0 ||
		VideoOK==false ||
		ReadAheadMB<=0
	)
	{
		return(false);
	}

	static LGL_Semaphore sem("video_readahead",false);
	LGL_ScopeLock lock(__FILE__,__LINE__,sem);

	//Is it goofy not to keep this fd?
	if(FILE* fd=fopen(Path,"rb"))
	{
		char* buf = new char[ReadAheadMB*1024*1024];
		fseek(fd,PosBytes,SEEK_SET);
		fread(buf,ReadAheadMB*1024l*1024l,1,fd);
		fclose(fd);
		delete buf;
	}

	return(true);
}

bool
LGL_VideoDecoder::
MaybeLoadImage()
{
	if
	(
		FormatContext==NULL ||
		CodecContext==NULL ||
		FrameNative==NULL ||
		FrameRGB==NULL ||
		strcmp(Path,"NULL")==0 ||
		VideoOK==false
	)
	{
		return(false);
	}

	//Return early if we have enough frames loaded
	std::vector<lgl_FrameBuffer*> frameBufferLoaded = GetFrameBufferLoadedList(false);
	if((int)frameBufferLoaded.size()>=FrameBufferAddRadius*(FrameBufferAddBackwards ? 2 : 1))
	{
		return(false);
	}

	//Find frameNumber of image to add
	long frameNumberTarget=GetNextFrameNumberToLoad();
	if(frameNumberTarget==-1)
	{
		return(false);
	}

	if(frameNumberTarget >= SecondsToFrameNumber(GetLengthSeconds())-1)
	{
		return(false);
	}

	//Lock the video
	{
		LGL_ScopeLock videoOKLock(__FILE__,__LINE__,VideoOKSemaphore);
		VideoOKUserCount++;
	}

	//Seek to the appropriate frame...
	if(FrameNumberNext!=frameNumberTarget)
	{
		long timestampTarget=FrameNumberToTimestamp(frameNumberTarget);
		lgl_av_seek_frame
		(
			FormatContext,
			VideoStreamIndex,
			timestampTarget,
			AVSEEK_FLAG_ANY
		);
		FrameNumberNext=frameNumberTarget+1;
	}

	AVPacket* packet=new AVPacket;
	lgl_av_init_packet(packet);
	bool frameRead=false;
	for(;;)
	{
		int result=0;
		{
			result = lgl_av_read_frame(FormatContext, packet);
		}
		if(result>=0)
		{
			PosBytes=packet->pos;
			//Is this a packet from the video stream?
			if(packet->stream_index==VideoStreamIndex)
			{
				frameRead=true;
				break;
			}
			else
			{
				lgl_av_free_packet(packet);
			}
		}
		else
		{
			LengthSeconds=LGL_Max(0,FrameNumberToSeconds(frameNumberTarget));
			break;
		}
	}

	//If we read a frame, put it into an lgl_FrameBuffer
	if(frameRead)
	{
		char path[2048];
		{
			LGL_ScopeLock pathLock(__FILE__,__LINE__,PathSemaphore);
			strcpy(path,Path);
		}
		//Prepare a framebuffer, and swap its buffer with BufferRGB
		lgl_FrameBuffer* frameBuffer = GetInvalidFrameBuffer();

		if(frameBuffer)
		{
			frameBuffer->SetPacket
			(
				packet,
				Path,
				frameNumberTarget
			);
		}
		else
		{
			printf("Couldn't obtain an invalid framebuffer (A)\n");
		}
	}
	else
	{
		//Free the packet that was allocated by lgl_av_read_frame
		//(Is this right?)
		lgl_av_free_packet(packet);
		delete packet;
		packet=NULL;
	}

	//Unlock the video
	{
		LGL_ScopeLock videoOKLock(__FILE__,__LINE__,VideoOKSemaphore);
		VideoOKUserCount--;
	}
	/*
	if(frameRead)
	{
		printf("Loaded frame %li\n",frameNumberTarget);
	}
	*/
	return(frameRead);
}

#include </opt/libjpeg-turbo/include/jpeglib.h>	//Using an explicit path to libjpeg-turbo
#include <setjmp.h>

struct my_error_mgr
{
	struct jpeg_error_mgr pub;	/* "public" fields */

	jmp_buf setjmp_buffer;	/* for return to caller */
};
typedef struct my_error_mgr* my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}

bool
lgl_decode_jpeg
(
	unsigned char*	dstData,
	long		dstLen,
	unsigned char*	srcData,
	long		srcLen
)
{
	/*
	if(0)
	{
		bool ret = lgl_quicktime_decode_jpeg
		(
			dstData,
			dstLen,
			srcData,
			srcLen
		);
		if(ret)
		{
			return(true);
		}
	}
	*/

	//Modified from example.c in libjpeg-turbo-1.1.1
	
	/* This struct contains the JPEG decompression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	*/
	struct jpeg_decompress_struct cinfo;
	/* We use our private extension JPEG error handler.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct my_error_mgr jerr;
	/* More stuff */
	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */



	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer))
	{
		/* If we get here, the JPEG code has signaled an error.
		* We need to clean up the JPEG object, close the input file, and return.
		*/
		jpeg_destroy_decompress(&cinfo);
		return(false);
	}
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);



	/* Step 2: specify data source (eg, a file) */

	//jpeg_stdio_src(&cinfo, infile);
	jpeg_mem_src(&cinfo,srcData,srcLen);



	/* Step 3: read file parameters with jpeg_read_header() */

	jpeg_read_header(&cinfo, TRUE);
	/* We can ignore the return value from jpeg_read_header since
	*   (a) suspension is not possible with the stdio data source, and
	*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	* See libjpeg.txt for more info.
	*/



	/* Step 4: set parameters for decompression */

	/* In this example, we don't need to change any of the defaults set by
	* jpeg_read_header(), so we do nothing here.
	*/



	/* Step 5: Start decompressor */

	cinfo.out_color_space = JCS_EXT_BGRX;
	cinfo.output_components = 4;
	cinfo.do_fancy_upsampling=false;
	cinfo.dct_method=JDCT_FASTEST;
	jpeg_start_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	/* We may need to do some setup of our own at this point before reading
	* the data.  After jpeg_start_decompress() we have the correct scaled
	* output image dimensions available, as well as the output colormap
	* if we asked for color quantization.
	* In this example, we need to make an output work buffer of the right size.
	*/ 
	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;
	/* Make a one-row-high sample array that will go away when done with image */
	/*
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	*/
	long baka=0;
	buffer=(JSAMPARRAY)(&baka);



	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */

	/* Here we use the library's state variable cinfo.output_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	*/
	while(cinfo.output_scanline < cinfo.output_height)
	{
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could ask for
		* more than one scanline at a time if that's more convenient.
		*/
		buffer[0] = &(dstData[row_stride*(cinfo.output_scanline)]);
		jpeg_read_scanlines(&cinfo, buffer, 1);
		/* Assume put_scanline_someplace wants a pointer and sample count. */
		//put_scanline_someplace(buffer[0], row_stride);
		//memcpy(&(dstData[row_stride*(cinfo.output_scanline-1)]),buffer[0],row_stride);
	}




	/* Step 7: Finish decompression */

	jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/



	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	buffer=NULL;
	jpeg_destroy_decompress(&cinfo);



	return(true);
}

bool
LGL_VideoDecoder::
MaybeDecodeImage
(
	long	desiredFrameNum
)
{
	//We can early out before grabbing the lock
	if
	(
		FormatContext==NULL ||
		CodecContext==NULL ||
		FrameNative==NULL ||
		FrameRGB==NULL ||
		strcmp(Path,"NULL")==0 ||
		VideoOK==false
	)
	{
		return(false);
	}

	//Find the next lgl_FrameBuffer
	lgl_FrameBuffer* frameBuffer=NULL;
	if(desiredFrameNum!=-1)
	{
		std::vector<lgl_FrameBuffer*> frameBufferReady = GetFrameBufferReadyList(false);
		for(unsigned int a=0;a<frameBufferReady.size();a++)
		{
			if(frameBufferReady[a]->GetFrameNumber()==desiredFrameNum)
			{
				//Our desired frame is already ready!
				return(false);
			}
		}
	}

	char path[2048];
	{
		LGL_ScopeLock pathLock(__FILE__,__LINE__,PathSemaphore,
			(SDL_ThreadID()==LGL.ThreadIDMain) ? 0.0f : -1.0f
		);
		if(pathLock.GetLockObtained()==false)
		{
			return(false);
		}
		strcpy(path,Path);
	}
	
	bool mainThread=(SDL_ThreadID()==LGL.ThreadIDMain);

	if(mainThread)
	{
		bool lockObtained=VideoOKSemaphore.Lock(__FILE__,__LINE__,0.0f);
		if(lockObtained==false)
		{
			return(false);
		}
	}
	//Lock the video
	{
		if(mainThread==false)
		{
			LGL_ScopeLock videoOKLock(__FILE__,__LINE__,VideoOKSemaphore);
			VideoOKUserCount++;
		}
		else
		{
			VideoOKUserCount++;
		}
	}

	if
	(
		FormatContext==NULL ||
		CodecContext==NULL ||
		FrameNative==NULL ||
		FrameRGB==NULL ||
		strcmp(Path,"NULL")==0 ||
		VideoOK==false
	)
	{
		//Unlock the video
		if(mainThread==false)
		{
			LGL_ScopeLock videoOKLock(__FILE__,__LINE__,VideoOKSemaphore);
			VideoOKUserCount--;
		}
		else
		{
			VideoOKUserCount--;
			VideoOKSemaphore.Unlock();
		}
		return(false);
	}

	if(desiredFrameNum!=-1)
	{
		//std::vector<lgl_FrameBuffer*> frameBufferLoaded = GetFrameBufferLoadedList(false);
		std::vector<lgl_FrameBuffer*> frameBufferLoaded = GetFrameBufferLoadedList(false);
		for(unsigned int a=0;a<frameBufferLoaded.size();a++)
		{
			if(frameBufferLoaded[a]->GetFrameNumber()==desiredFrameNum)
			{
				frameBuffer=frameBufferLoaded[a];
			}
		}
	}

	if(frameBuffer==NULL)
	{
		{
			std::vector<lgl_FrameBuffer*> frameBufferLoaded = GetFrameBufferLoadedList(false);
			long desiredFrame = GetNextFrameNumberToDecode();
			for(unsigned int a=0;a<frameBufferLoaded.size();a++)
			{
				if(frameBufferLoaded[a]->GetFrameNumber()==desiredFrame)
				{
					frameBuffer=frameBufferLoaded[a];
					break;
				}
			}
			/*
			if(frameBuffer)
			{
				printf("Desired found: %li (%i)\n",desiredFrame,FrameBufferAddRadius);
			}
			*/
		}
		if(frameBuffer==NULL)
		{
			//Can't find our desired frame, so just decode any frame.
			std::vector<lgl_FrameBuffer*> frameBufferLoaded = GetFrameBufferLoadedList(true);
			if(frameBufferLoaded.empty()==false)
			{
				frameBuffer=frameBufferLoaded[0];
			}
		}
	}

	if(frameBuffer==NULL)
	{
		//Unlock the video
		if(mainThread==false)
		{
			LGL_ScopeLock videoOKLock(__FILE__,__LINE__,VideoOKSemaphore);
			VideoOKUserCount--;
		}
		else
		{
			VideoOKUserCount--;
			VideoOKSemaphore.Unlock();
		}
		return(false);
	}

	//Setup YUV FrameNative
	{
		unsigned int bufferYUVBytesNow=avpicture_get_size
		(
			CodecContext->pix_fmt, 
			BufferWidth,
			BufferHeight
		);
		if
		(
			BufferYUV==NULL ||
			BufferYUVBytes<bufferYUVBytesNow
		)
		{
			BufferYUVBytes=bufferYUVBytesNow;
			delete BufferYUV;
			BufferYUV=new uint8_t[BufferYUVBytes];
		}
	}

	//Get the AVPacket
	AVPacket* packet = frameBuffer->LockPacket();
	if(packet==NULL)
	{
		//Unlock the video
		if(mainThread==false)
		{
			LGL_ScopeLock videoOKLock(__FILE__,__LINE__,VideoOKSemaphore);
			VideoOKUserCount--;
		}
		else
		{
			VideoOKUserCount--;
			VideoOKSemaphore.Unlock();
		}
		return(false);
	}

/*
	if(FILE* fd=fopen("packet_test.jpg","wb"))
	{
		fwrite(packet->data,packet->size,1,fd);
		fclose(fd);
		exit(0);
	}
*/

	//Obtain address in which to copy

	unsigned int bufferBytesNow=avpicture_get_size
	(
		PIX_FMT_BGRA,
		BufferWidth,
		BufferHeight
	);
	unsigned char* dst=frameBuffer->LockBufferRGB
	(
		bufferBytesNow
	);

	if(dst==NULL)
	{
		if(mainThread==false)
		{
			LGL_ScopeLock videoOKLock(__FILE__,__LINE__,VideoOKSemaphore);
			VideoOKUserCount--;
		}
		else
		{
			VideoOKUserCount--;
			VideoOKSemaphore.Unlock();
		}
		frameBuffer->UnlockPacket();
		return(false);
	}

	//Decode video frame
	int frameFinished=0;
	const int useLibJpegTurbo=true;//(SDL_ThreadID()!=LGL.ThreadIDMain);
	if(useLibJpegTurbo)
	{
		if(0 && QuicktimeMovie)
		{
			/*
			frameFinished=lgl_quicktime_decode
			(
				QuicktimeMovie,
				FrameNumberToSeconds(desiredFrameNum),
				dst,
				bufferBytesNow
			);
			*/
		}
		else
		{
			frameFinished=lgl_decode_jpeg(dst,bufferBytesNow,packet->data,(long)packet->size);
		}
	}
	else
	{
		lgl_avcodec_decode_video2
		(
			CodecContext,
			FrameNative,
			&frameFinished, 
			packet
		);

		if(IsYUV420P())
		{
			//Do a line-by line memcpy... Laaaame!
			unsigned char* bufYUVNow=BufferYUV;
			for(int c=0;c<3;c++)
			{
				int width = (c==0) ? BufferWidth : (BufferWidth/2);
				int height = (c==0) ? BufferHeight : (BufferHeight/2);
				for(int h=0;h<height;h++)
				{
					memcpy(bufYUVNow,&(FrameNative->data[c][h*FrameNative->linesize[c]]),width);
					bufYUVNow+=width;
				}
			}
		}
	}

	if(frameFinished==false)
	{
		printf("Frame FAIL!!\n");
	}

	//Did we get a video frame?
	bool frameRead=false;
	if(frameFinished)
	{
		if
		(
			useLibJpegTurbo==false &&
			IsYUV420P()==false
		)
		{
			/*
			if
			(
				BufferRGB==NULL ||
				BufferRGBBytes<bufferBytesNow
			)
			{
				BufferRGBBytes=bufferBytesNow;
				delete BufferRGB;
				BufferRGB=new uint8_t[BufferRGBBytes];
			}
			//Update FrameRGB to point to BufferRGB.
			avpicture_fill
			(
				(AVPicture*)FrameRGB,
				BufferRGB,
				PIX_FMT_BGRA,
				BufferWidth,
				BufferHeight
			);
			*/
			//Update FrameRGB to point to dst.
			avpicture_fill
			(
				(AVPicture*)FrameRGB,
				dst,
				PIX_FMT_BGRA,
				BufferWidth,
				BufferHeight
			);
			
			if(SwsConvertContextBGRA==NULL)
			{
				printf("NULL SwsConvertContextBGRA!\n");
			}
			if(FrameNative==NULL)
			{
				printf("NULL FrameNative!\n");
			}
			else if(FrameNative->data==NULL)
			{
				printf("NULL FrameNative->data!\n");
			}
			if(FrameRGB==NULL)
			{
				printf("NULL FrameRGB!\n");
			}
			else if(FrameRGB->data==NULL)
			{
				printf("NULL FrameRGB->data!\n");
			}
			lgl_sws_scale
			(
				SwsConvertContextBGRA,
				FrameNative->data,
				FrameNative->linesize,
				0, 
				BufferHeight,
				FrameRGB->data,
				FrameRGB->linesize
			);
		}
		frameRead=true;
	}

	//If we read a frame, put it into our lgl_FrameBuffer
	if(frameRead)
	{
		/*
		if(SDL_ThreadID()==LGL.ThreadIDMain)
		{
			LGL_DebugPrintf("MAIN\n");
		}
		else
		{
			LGL_DebugPrintf("THREAD\n");
		}
		*/

		if(frameBuffer)
		{
			if(IsYUV420P()==false)
			{
				/*
				frameBuffer->SwapInNewBufferRGB
				(
					path,
					BufferRGB,	//Changes...
					BufferRGBBytes,	//Changes...
					BufferWidth,
					BufferHeight,
					frameBuffer->GetFrameNumber()
				);
				*/
			}
			else
			{
				frameBuffer->SwapInNewBufferYUV
				(
					path,
					BufferYUV,	//Changes...
					BufferYUVBytes,	//Changes...
					BufferWidth,
					BufferHeight,
					frameBuffer->GetFrameNumber()
				);
			}
		}
		else
		{
			printf("Couldn't obtain an invalid framebuffer (B)\n");
		}
	}
	else
	{
		printf("Frame FAIL 2!!\n");
	}

	frameBuffer->UnlockBufferRGB
	(
		path,
		BufferWidth,
		BufferHeight,
		frameBuffer->GetFrameNumber()
	);
	dst=NULL;

	//Free the packet that was allocated by lgl_av_read_frame
	{
		if(frameRead)
		{
			/*
			char tmp[2048];
			sprintf(tmp,frameBuffer->GetVideoPath() ? frameBuffer->GetVideoPath() : "");
			frameBuffer->SetPacket
			(
				NULL,
				tmp,
				frameBuffer->GetFrameNumber()
			);
			*/
		}
		else
		{
			frameBuffer->Invalidate();
		}
	}
	//Unlock the video
	if(mainThread==false)
	{
		LGL_ScopeLock videoOKLock(__FILE__,__LINE__,VideoOKSemaphore);
		VideoOKUserCount--;
	}
	else
	{
		VideoOKUserCount--;
		VideoOKSemaphore.Unlock();
	}
	frameBuffer->UnlockPacket();
	return(frameRead);
}

void
LGL_VideoDecoder::
MaybeInvalidateBuffers()
{
	if(VideoOK==false)
	{
		return;
	}

	char path[2048];
	{
		LGL_ScopeLock pathLock(__FILE__,__LINE__,PathSemaphore,0.0f);
		if(pathLock.GetLockObtained()==false)
		{
			return;
		}
		strcpy(path,Path);
	}

	long frameNumberNow = SecondsToFrameNumber(TimeSeconds);
	long frameNumberLength = SecondsToFrameNumber(LengthSeconds);
	if(frameNumberLength==0)
	{
		return;
	}

	long frameNumberPrev = SecondsToFrameNumber(TimeSecondsPrev);
	long frameNumberTarget = frameNumberNow + (frameNumberNow-frameNumberPrev);

	//Handle wrap-around
	long frameNumberPredict=frameNumberTarget;
	frameNumberPredict = frameNumberPredict % frameNumberLength;

	std::vector<lgl_FrameBuffer*>& list = FrameBufferList;
	int subtractRadius = FrameBufferSubtractRadius;
	long nextDecodeFrameNumber = GetNextFrameNumberToDecodePredictNext(false);
	for(unsigned int a=0;a<list.size();a++)
	{
		if
		(
			//list[a]->GetFrameNumber()==-1 ||
			strcmp(list[a]->GetVideoPath(),path)!=0 ||
			(
				fabsf(frameNumberNow-list[a]->GetFrameNumber())				> subtractRadius &&
				fabsf(frameNumberPredict-list[a]->GetFrameNumber())			> subtractRadius &&
				fabsf((frameNumberNow-frameNumberLength)-list[a]->GetFrameNumber())	> subtractRadius &&
				fabsf((frameNumberNow+frameNumberLength)-list[a]->GetFrameNumber())	> subtractRadius &&
				nextDecodeFrameNumber != list[a]->GetFrameNumber()
			)
		)
		{
			//list[a]->NullifyBuffer();
			list[a]->Invalidate();
		}
	}
}

bool
LGL_VideoDecoder::
GetThreadTerminate()
{
	return
	(
		ThreadTerminate ||
		(LGL.Running==false)
	);
}

std::vector<lgl_FrameBuffer*>
LGL_VideoDecoder::
GetFrameBufferReadyList
(
	bool	sorted
)
{
	std::vector<lgl_FrameBuffer*> ret;
	for(unsigned int a=0;a<FrameBufferList.size();a++)
	{
		if(FrameBufferList[a]==NULL)
		{
			printf("NULL entry in FrameBufferList... WTF??\n");
		}
		else if(FrameBufferList[a]->IsReady())
		{
			ret.push_back(FrameBufferList[a]);
		}
	}

	if(sorted && ret.size()>0)
	{
		std::vector<lgl_FrameBufferSortContainer*> sortContainerList;
		for(unsigned int a=0;a<ret.size();a++)
		{
			sortContainerList.push_back(new lgl_FrameBufferSortContainer(ret[a]));
		}
		std::sort
		(
			sortContainerList.begin(),
			sortContainerList.end(),
			lgl_FrameBufferSortContainerSortPredicate
		);
		ret.clear();
		for(unsigned int a=0;a<sortContainerList.size();a++)
		{
			ret.push_back(sortContainerList[a]->FrameBuffer);
			delete sortContainerList[a];
		}
	}

	return(ret);
}

std::vector<lgl_FrameBuffer*>
LGL_VideoDecoder::
GetFrameBufferLoadedList
(
	bool	sorted
)
{
	std::vector<lgl_FrameBuffer*> ret;
	for(unsigned int a=0;a<FrameBufferList.size();a++)
	{
		if(FrameBufferList[a]==NULL)
		{
			printf("NULL entry in FrameBufferList... WTF??\n");
		}
		if
		(
			FrameBufferList[a]->IsReady()==false &&
			FrameBufferList[a]->IsLoaded()
		)
		{
			ret.push_back(FrameBufferList[a]);
		}
	}

	if(sorted && ret.size()>0)
	{
		std::vector<lgl_FrameBufferSortContainer*> sortContainerList;
		for(unsigned int a=0;a<ret.size();a++)
		{
			sortContainerList.push_back(new lgl_FrameBufferSortContainer(ret[a]));
		}
		std::sort
		(
			sortContainerList.begin(),
			sortContainerList.end(),
			lgl_FrameBufferSortContainerSortPredicate
		);
		ret.clear();
		for(unsigned int a=0;a<sortContainerList.size();a++)
		{
			ret.push_back(sortContainerList[a]->FrameBuffer);
			delete sortContainerList[a];
		}
	}

	return(ret);
}

double
LGL_VideoDecoder::
TimestampToSeconds(long timestamp)
{
	return(timestamp/FPSTimestamp);
}

long
LGL_VideoDecoder::
SecondsToTimestamp
(
	double	seconds
)
{
	return(seconds*FPSTimestamp);
}

double
LGL_VideoDecoder::
FrameNumberToSeconds(long frameNumber)
{
	//frameNumber = frameNumber % (long)(LengthSeconds * FPS);
	return(frameNumber/FPS);
}

long
LGL_VideoDecoder::
SecondsToFrameNumber
(
	double	seconds
)
{
	long ret = seconds * FPS;
	//ret = ret % (long)(LengthSeconds * FPS);
	return(ret);
}

long
LGL_VideoDecoder::
FrameNumberToTimestamp
(
	long	frameNumber
)
{
	return
	(
		SecondsToTimestamp
		(
			FrameNumberToSeconds
			(
				frameNumber
			)
		)
	);
}

long
LGL_VideoDecoder::
GetNextFrameNumberToLoad()
{
	std::vector<long> frameNumList;
	std::vector<lgl_FrameBuffer*> frameBufferLoaded = GetFrameBufferLoadedList(false);
	for(unsigned int a=0;a<frameBufferLoaded.size();a++)
	{
		if(frameBufferLoaded[a]->GetFrameNumber()!=-1)
		{
			frameNumList.push_back(frameBufferLoaded[a]->GetFrameNumber());
		}
	}
	std::vector<lgl_FrameBuffer*> frameBufferReady = GetFrameBufferReadyList(false);
	for(unsigned int a=0;a<frameBufferReady.size();a++)
	{
		if(frameBufferReady[a]->GetFrameNumber()!=-1)
		{
			frameNumList.push_back(frameBufferReady[a]->GetFrameNumber());
		}
	}
	std::sort
	(
		frameNumList.begin(),
		frameNumList.end(),
		lgl_LongSortPredicate
	);

	long ret=-1;

	ret = GetNextFrameNumberToLoadPredictNext(frameNumList);
	if(ret!=-1)
	{
		return(ret);
	}

	if(TimeSeconds>=TimeSecondsPrev)
	{
		ret = GetNextFrameNumberToLoadForwards(frameNumList);
		if(ret!=-1)
		{
			return(ret);
		}
		ret = GetNextFrameNumberToLoadBackwards(frameNumList);
		if(ret!=-1)
		{
			return(ret);
		}
	}
	else
	{
		ret = GetNextFrameNumberToLoadBackwards(frameNumList);
		if(ret!=-1)
		{
			return(ret);
		}
		ret = GetNextFrameNumberToLoadForwards(frameNumList);
		if(ret!=-1)
		{
			return(ret);
		}
	}

	return(-1);
}

long
LGL_VideoDecoder::
GetNextFrameNumberToLoadPredictNext
(
	std::vector<long>&	frameNumList,
	bool			mustNotBeLoaded
)
{
	long frameNumberNow = SecondsToFrameNumber(TimeSeconds);
	long frameNumberLength = SecondsToFrameNumber(LengthSeconds);
	if(frameNumberLength==0)
	{
		return(0);
	}

	long frameNumberPrev = SecondsToFrameNumber(TimeSecondsPrev);
	long frameNumberFind = frameNumberNow + (frameNumberNow-frameNumberPrev);

	//Handle wrap-around
	if(frameNumberFind<0) frameNumberFind=0;
	frameNumberFind = frameNumberFind % frameNumberLength;

	bool found=false;
	for(unsigned int b=0;b<frameNumList.size();b++)
	{
		long frameNumberNow=frameNumList[b];
		if(frameNumberFind==frameNumberNow)
		{
			found=true;
			break;
		}
	}

	if
	(
		found==false ||
		mustNotBeLoaded==false
	)
	{
		return(frameNumberFind);
	}

	return(-1);
}

long
LGL_VideoDecoder::
GetNextFrameNumberToLoadForwards
(
	std::vector<long>&	frameNumList
)
{
	long frameNumberNow = SecondsToFrameNumber(TimeSeconds);
	long frameNumberLength = SecondsToFrameNumber(LengthSeconds);
	if(frameNumberLength==0)
	{
		return(0);
	}

	long frameNumberFinal = frameNumberNow+FrameBufferAddRadius;
	for(long a=frameNumberNow;a<frameNumberFinal;a++)
	{
		//Handle wrap-around
		long frameNumberFind=a;
		while(frameNumberFind>=frameNumberLength)
		{
			frameNumberFind-=frameNumberLength;
		}

		bool found=false;
		for(unsigned int b=0;b<frameNumList.size();b++)
		{
			long frameNumberNow=frameNumList[b];
			if(frameNumberFind<frameNumberNow)
			{
				return(frameNumberFind);
			}
			else if(frameNumberFind==frameNumberNow)
			{
				found=true;
				break;
			}
			else //frameNumberFind>frameNumberNow
			{
				continue;
			}
		}

		if(found==false)
		{
			return(frameNumberFind);
		}
	}

	return(-1);
}

long
LGL_VideoDecoder::
GetNextFrameNumberToLoadBackwards
(
	std::vector<long>&	frameNumList
)
{
	if(FrameBufferAddBackwards==false)
	{
		return(-1);
	}

	long frameNumberNow = SecondsToFrameNumber(TimeSeconds);
	long frameNumberLength = SecondsToFrameNumber(LengthSeconds);
	if(frameNumberLength==0)
	{
		return(0);
	}

	long frameNumberFinal = frameNumberNow-FrameBufferAddRadius;
	for(long a=frameNumberNow;a>frameNumberFinal;a--)
	{
		//Handle wrap-around
		long frameNumberFind=a;
		while(frameNumberFind>=frameNumberLength)
		{
			frameNumberFind-=frameNumberLength;
		}
		while(frameNumberFind<0)
		{
			frameNumberFind+=frameNumberLength;
		}

		bool found=false;
		for(int b=(int)frameNumList.size()-1;b>=0;b--)
		{
			long frameNumberNow=frameNumList[b];
			if(frameNumberFind>frameNumberNow)
			{
				return(frameNumberFind);
			}
			else if(frameNumberFind==frameNumberNow)
			{
				found=true;
				break;
			}
			else //frameNumberFind<frameNumberNow
			{
				continue;
			}
		}

		if(found==false)
		{
			return(frameNumberFind);
		}
	}

	return(-1);
}

long
LGL_VideoDecoder::
GetNextFrameNumberToDecode()
{
	long ret=-1;

	ret = GetNextFrameNumberToDecodePredictNext();
	if(ret!=-1)
	{
		return(ret);
	}

	if(TimeSeconds>=TimeSecondsPrev)
	{
		ret = GetNextFrameNumberToDecodeForwards();
		if(ret!=-1)
		{
			return(ret);
		}
		ret = GetNextFrameNumberToDecodeBackwards();
		if(ret!=-1)
		{
			return(ret);
		}
	}
	else
	{
		ret = GetNextFrameNumberToDecodeBackwards();
		if(ret!=-1)
		{
			return(ret);
		}
		ret = GetNextFrameNumberToDecodeForwards();
		if(ret!=-1)
		{
			return(ret);
		}
	}

	return(-1);
}

long
LGL_VideoDecoder::
GetNextFrameNumberToDecodePredictNext(bool mustNotBeDecoded)
{
	long frameNumberNow = SecondsToFrameNumber(TimeSeconds);
	long frameNumberLength = SecondsToFrameNumber(LengthSeconds);
	if(frameNumberLength==0)
	{
		return(0);
	}

	long frameNumberPrev = SecondsToFrameNumber(TimeSecondsPrev);
	long frameNumberTarget = frameNumberNow + (frameNumberNow-frameNumberPrev);

	//Handle wrap-around
	long frameNumberFind=frameNumberTarget;
	frameNumberFind = frameNumberFind % frameNumberLength;

	if(frameNumberFind<0) frameNumberFind=0;
	if(frameNumberFind>frameNumberLength) frameNumberFind=frameNumberLength;

	bool found=false;
	std::vector<lgl_FrameBuffer*> frameBufferReady = GetFrameBufferReadyList(true);
	for(unsigned int b=0;b<frameBufferReady.size();b++)
	{
		long frameNumberImage=frameBufferReady[b]->GetFrameNumber();
		if(frameNumberFind<frameNumberImage)
		{
			return(frameNumberFind);
		}
		else if(frameNumberFind==frameNumberImage)
		{
			found=true;
			break;
		}
		else //frameNumberFind>frameNumberImage
		{
			continue;
		}
	}

	if
	(
		found==false ||
		mustNotBeDecoded==false
	)
	{
		return(frameNumberFind);
	}

	return(-1);
}

long
LGL_VideoDecoder::
GetNextFrameNumberToDecodeForwards()
{
	long frameNumberNow = SecondsToFrameNumber(TimeSeconds);
	long frameNumberLength = SecondsToFrameNumber(LengthSeconds);
	if(frameNumberLength==0)
	{
		return(0);
	}

	int frameBufferIndex=0;
	long frameNumberFinal = frameNumberNow+FrameBufferAddRadius;
	for(long a=frameNumberNow;a<frameNumberFinal;a++)
	{
		//Handle wrap-around
		long frameNumberFind=a;
		while(frameNumberFind>=frameNumberLength)
		{
			frameNumberFind-=frameNumberLength;
			frameBufferIndex=0;
		}

		bool found=false;
		std::vector<lgl_FrameBuffer*> frameBufferReady = GetFrameBufferReadyList(true);
		for(unsigned int b=frameBufferIndex;b<frameBufferReady.size();b++)
		{
			frameBufferIndex++;
			long frameNumberImage=frameBufferReady[b]->GetFrameNumber();
			if(frameNumberFind<frameNumberImage)
			{
				return(frameNumberFind);
			}
			else if(frameNumberFind==frameNumberImage)
			{
				found=true;
				break;
			}
			else //frameNumberFind>frameNumberImage
			{
				continue;
			}
		}

		if(found==false)
		{
			return(frameNumberFind);
		}
	}

	return(-1);
}

long
LGL_VideoDecoder::
GetNextFrameNumberToDecodeBackwards()
{
	if(FrameBufferAddBackwards==false)
	{
		return(-1);
	}

	long frameNumberNow = SecondsToFrameNumber(TimeSeconds);
	long frameNumberLength = SecondsToFrameNumber(LengthSeconds);
	if(frameNumberLength==0)
	{
		return(0);
	}
	if(frameNumberNow>frameNumberLength) frameNumberNow=frameNumberLength;
	
	std::vector<lgl_FrameBuffer*> frameBufferReady = GetFrameBufferReadyList(true);
	int frameBufferIndex=frameBufferReady.size()-1;
	long frameNumberFinal = frameNumberNow-FrameBufferAddRadius;
	for(long a=frameNumberNow-1;a>frameNumberFinal;a--)
	{
		//Handle wrap-around
		long frameNumberFind=a;
		while(frameNumberFind<0)
		{
			frameNumberFind+=frameNumberLength;
			frameBufferIndex=frameBufferReady.size()-1;
		}

		bool found=false;
		for(int b=frameBufferIndex;b>=0;b--)
		{
			frameBufferIndex--;
			long frameNumberImage=frameBufferReady[b]->GetFrameNumber();
			if
			(
				frameNumberFind>frameNumberImage &&
				frameNumberFind-frameNumberImage < frameNumberLength/2
			)
			{
				return(frameNumberFind);
			}
			else if(frameNumberFind==frameNumberImage)
			{
				found=true;
				break;
			}
			else //frameNumberFind<frameNumberImage
			{
				continue;
			}
		}

		if(found==false)
		{
			return(frameNumberFind);
		}
	}

	return(-1);
}

lgl_FrameBuffer*
LGL_VideoDecoder::
GetInvalidFrameBuffer()
{
	for(unsigned int a=0;a<FrameBufferList.size();a++)
	{
		if
		(
			FrameBufferList[a]->IsLoaded()==false &&
			FrameBufferList[a]->IsReady()==false
		)
		{
			return(FrameBufferList[a]);
		}
	}

	printf("Couldn't find invalid framebuffer!!\n");

	return(NULL);
}

bool
LGL_VideoDecoder::
IsYUV420P()
{
	return
	(
		LGL_KeyDown(LGL_KEY_F4) &&
		CodecContext &&
		CodecContext->pix_fmt==PIX_FMT_YUV420P
	);
}



float LGL_VideoEncoder::BitrateMaxMBps=3.2f;

LGL_VideoEncoder::
LGL_VideoEncoder
(
	const char*	src,
	const char*	dstVideo,
	const char*	dstAudio
) : 	DstFrameYUVSemaphore("DstFrameYUVSemaphore")
{
	strcpy(SrcPath,src);
	strcpy(DstPath,dstVideo);
	strcpy(DstMp3Path,dstAudio);

	Valid=0;
	UnsupportedCodec=false;
	EncodeAudio=true;
	EncodeVideo=true;

	SrcFormatContext=NULL;
	SrcCodecContext=NULL;
	strcpy(SrcCodecName,"Unknown");
	SrcAudioCodecContext=NULL;
	SrcCodec=NULL;
	SrcAudioCodec=NULL;
	SrcFrame=NULL;
	SrcPacketPosMax=0;
	SrcPacket.pos=0;

	SwsConvertContextYUV=NULL;
	SwsConvertContextBGRA=NULL;

	DstOutputFormat=NULL;
	DstFormatContext=NULL;
	DstCodecContext=NULL;
	DstCodec=NULL;
	DstStream=NULL;
	DstFrameYUV=NULL;
	DstBufferYUV=NULL;
	DstFrameBGRA=NULL;
	DstBufferBGRA=NULL;

	DstMp3OutputFormat=NULL;
	DstMp3FormatContext=NULL;
	DstMp3CodecContext=NULL;
	DstMp3Codec=NULL;
	DstMp3Stream=NULL;
	DstMp3Buffer=NULL;
	DstMp3BufferSamples=NULL;
	DstMp3BufferSamplesIndex=0;
	DstMp3BufferSamplesTotalBytes=0;
	DstMp3Buffer2=NULL;

	Image=NULL;

	{
		lgl_av_init_packet(&DstPacket);
		DstPacketVideoPts=0;
		lgl_av_init_packet(&DstMp3Packet);
		DstMp3Packet.dts=0;
		DstMp3Packet.pts=1;

		//Open file
		AVFormatContext* fc=NULL;
		{
			if(lgl_av_open_input_file(&fc, src, NULL, 0, NULL)!=0)
			{
				printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't open '%s'\n",src);
				return;
			}
			SrcFormatContext=fc;
		}

		//Find streams
		if(lgl_av_find_stream_info(fc)<0)
		{
			//printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't find streams for '%s'\n",src);
			return;
		}

		// Find the first video stream
		SrcVideoStreamIndex=-1;
		for(unsigned int i=0; i<fc->nb_streams; i++)
		{
			if(fc->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO)
			{
				SrcVideoStreamIndex=i;
				break;
			}
		}
		if(SrcVideoStreamIndex==-1)
		{
			//printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't find video stream for '%s' in %i streams\n",src,fc->nb_streams);
			return;
		}

		// Find the first audio stream
		SrcAudioStreamIndex=-1;
		for(unsigned int i=0; i<fc->nb_streams; i++)
		{
			if(fc->streams[i]->codec->codec_type==CODEC_TYPE_AUDIO)
			{
				SrcAudioStreamIndex=i;
				break;
			}
		}

		// Get a pointer to the codec context for the video stream
		SrcCodecContext=fc->streams[SrcVideoStreamIndex]->codec;
		strcpy(SrcCodecName,SrcCodecContext->codec_name);
		if(strlen(SrcCodecName)==0)
		{
			strcpy(SrcCodecName,"Unknown Codec");
		}
		AVCodecContext* srcAudioCodecContext=NULL;
		if(SrcAudioStreamIndex!=-1)
		{
			srcAudioCodecContext=fc->streams[SrcAudioStreamIndex]->codec;
		}
		else
		{
			EncodeAudio=false;
		}

		// Find the decoder for the video stream
		SrcCodec=lgl_avcodec_find_decoder(SrcCodecContext->codec_id);
		if(SrcCodec==NULL)
		{
			printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't find codec for '%s'. Codec = '%s'\n",src,SrcCodecContext->codec_name);
			SrcCodecContext=NULL;
			UnsupportedCodec=true;
			return;
		}

		// Find the decoder for the audio stream
		if(srcAudioCodecContext)
		{
			SrcAudioCodec=lgl_avcodec_find_decoder(srcAudioCodecContext->codec_id);
			if(SrcAudioCodec==NULL)
			{
				printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't find audio codec for '%s'. Codec = '%s'\n",src,srcAudioCodecContext->codec_name);
				return;
			}
		}

		// Inform the codec that we can handle truncated bitstreams -- i.e.,
		// bitstreams where frame boundaries can fall in the middle of packets
		/*
		if(SrcCodec->capabilities & CODEC_CAP_TRUNCATED)
		{
			SrcCodecContext->flags|=CODEC_FLAG_TRUNCATED;
		}
		*/

		// Open codec
		{
			if(lgl_avcodec_open(SrcCodecContext, SrcCodec)<0)
			{
				printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't open codec for '%s'\n",src);
				SrcCodecContext=NULL;
				return;
			}
		}
		
		// Open audio codec
		if(srcAudioCodecContext)
		{
			if(lgl_avcodec_open(srcAudioCodecContext, SrcAudioCodec)<0)
			{
				printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't open audio codec for '%s'\n",src);
				SrcAudioCodecContext=NULL;
				return;
			}
			else
			{
				SrcAudioCodecContext=srcAudioCodecContext;
			}
		}

		//assert(SrcFrame==NULL);
		SrcFrame=lgl_avcodec_alloc_frame();

		if(SrcFrame==NULL)
		{
			printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't open frames for '%s'\n",src);
			return;
		}

		// Determine required buffer size and pseudo-allocate buffer
		SrcBufferWidth=SrcCodecContext->width;
		SrcBufferHeight=SrcCodecContext->height;
		SrcBufferBytes=avpicture_get_size
		(
			PIX_FMT_RGB24,
			SrcBufferWidth,
			SrcBufferHeight
		);

		SrcFrameNow=0;
		SrcSecondsNow=0;
	}

	//Prepare dst video

	{
		// find the video encoder
/*
	Candidates:
        CODEC_ID_MJPEG		[OK]
        CODEC_ID_MJPEGB		[NO]
        CODEC_ID_LJPEG
        CODEC_ID_RAWVIDEO
        CODEC_ID_DVVIDEO
        CODEC_ID_HUFFYUV
        CODEC_ID_FFVHUFF	[RGB, FPS?]
        CODEC_ID_ASV1
        CODEC_ID_ASV2		[CRASH_RGB]
        CODEC_ID_VCR1
        CODEC_ID_DNXHD
        CODEC_ID_JPEG2000	[NO]
        CODEC_ID_H264		[NO]
        CODEC_ID_FFH264		[NO]
        CODEC_ID_MPEG4		[NO]
*/
		DstCodec = lgl_avcodec_find_encoder(CODEC_ID_MJPEG);
		if(DstCodec==NULL)
		{
			printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't find encoder codec for '%s'\n",src);
			return;
		}

		// prepare the header

		// Set AVI format
		DstOutputFormat = av_guess_format("avi", NULL, NULL);
		DstOutputFormat->audio_codec = CODEC_ID_NONE;
		DstOutputFormat->video_codec = DstCodec->id;

		// FormatContext
		DstFormatContext = (AVFormatContext*)lgl_av_mallocz(sizeof(AVFormatContext));
		DstFormatContext->oformat = DstOutputFormat;
		strcpy(DstFormatContext->filename, DstPath);

		// Video stream
		DstStream = lgl_av_new_stream(DstFormatContext,0);
		DstStream->r_frame_rate=SrcFormatContext->streams[SrcVideoStreamIndex]->r_frame_rate;
		DstStream->sample_aspect_ratio=SrcFormatContext->streams[SrcVideoStreamIndex]->sample_aspect_ratio;
		DstStream->time_base=SrcFormatContext->streams[SrcVideoStreamIndex]->time_base;
		DstStream->quality=1.0f;	//It's unclear whether that actually affects mjpeg encoding...
		DstFormatContext->streams[0] = DstStream;
		DstFormatContext->nb_streams = 1;

		DstFrameYUV = lgl_avcodec_alloc_frame();
		DstFrameBGRA = lgl_avcodec_alloc_frame();

		DstCodecContext = DstStream->codec;
		lgl_avcodec_get_context_defaults(DstCodecContext);
		DstCodecContext->codec_id=DstCodec->id;
		DstCodecContext->codec_type=CODEC_TYPE_VIDEO;
		DstCodecContext->bit_rate=BitrateMaxMBps*1024*1024*8;
		DstCodecContext->bit_rate_tolerance=DstCodecContext->bit_rate/8;	//Allow for some variance
		DstCodecContext->flags|=0;	//No flags seems helpful to add.
		DstCodecContext->me_method = 1;
		DstCodecContext->sample_aspect_ratio=DstStream->sample_aspect_ratio;
		DstCodecContext->width = SrcBufferWidth;
		DstCodecContext->height = SrcBufferHeight;
		//The next line appears goofy and incorrect, but is necessary to ensure a proper framerate.
		//The SrcCodecContext's time_base can be incorrect, for reasons not entirely clear. ffmpeg can be like that.
		//DstCodecContext->time_base = DstStream->time_base;//SrcCodecContext->time_base;
		DstCodecContext->time_base.den=SrcFormatContext->streams[SrcVideoStreamIndex]->r_frame_rate.num;
		DstCodecContext->time_base.num=SrcFormatContext->streams[SrcVideoStreamIndex]->r_frame_rate.den;
		//DstCodecContext->pix_fmt = PIX_FMT_RGB32;
		DstCodecContext->pix_fmt = PIX_FMT_YUVJ422P;
		//DstCodecContext->pix_fmt = PIX_FMT_YUV420P;

		//These don't seem to help...
		DstCodecContext->qmin = 1;
		//DstCodecContext->qmax = 100;
		DstCodecContext->max_qdiff = 100;

		DstCodecContext->strict_std_compliance=-1;
DstCodecContext->keyint_min=1;
		
		//This doesn't work either
		DstCodecContext->rc_override=&DstCodecContextRcOverride;
		DstCodecContext->rc_override_count=1;
		DstCodecContextRcOverride.start_frame=0;
		DstCodecContextRcOverride.end_frame=INT_MAX;
		DstCodecContextRcOverride.qscale=1;
		DstCodecContextRcOverride.quality_factor=1.0;

		//DstCodecContext->rc_min_rate = DstCodecContext->bit_rate - DstCodecContext->bit_rate_tolerance;
		//DstCodecContext->rc_max_rate = DstCodecContext->bit_rate;
		DstCodecContext->global_quality = 100;

		printf("Attempting to open DstCodec '%s' (%i)\n",DstCodec->name,DstCodecContext->pix_fmt);
		{
			if(lgl_avcodec_open(DstCodecContext, DstCodec) < 0)
			{
				printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't open MJPEG codec for '%s'\n",src);
				DstCodecContext=NULL;
				return;
			}
		}

		dump_format(DstFormatContext,0,DstFormatContext->filename,1);

		//DstStream->time_base.den=SrcFormatContext->streams[SrcVideoStreamIndex]->r_frame_rate.num;
		//DstStream->time_base.num=SrcFormatContext->streams[SrcVideoStreamIndex]->r_frame_rate.den;
		DstStream->codec->time_base.den=SrcFormatContext->streams[SrcVideoStreamIndex]->r_frame_rate.num;
		DstStream->codec->time_base.num=SrcFormatContext->streams[SrcVideoStreamIndex]->r_frame_rate.den;
		//DstCodecContext->ticks_per_frame = DstStream->codec->time_base.num;

		int result = lgl_url_fopen(&(DstFormatContext->pb), DstFormatContext->filename, URL_WRONLY);
		if(result<0)
		{
			printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't url_fopen() output file '%s' (%i)\n",DstPath,result);
			DstCodecContext=NULL;
			return;
		}

		SwsConvertContextYUV = lgl_sws_getContext
		(
			//src
			SrcBufferWidth,
			SrcBufferHeight, 
			SrcCodecContext->pix_fmt, 
			//dst
			DstCodecContext->width,
			DstCodecContext->height,
			DstCodecContext->pix_fmt,
			SWS_BILINEAR,//SWS_FAST_BILINEAR,
			NULL,
			NULL,
			NULL
		);
		if(SwsConvertContextYUV==NULL)
		{
			printf("LGL_VideoEncoder::LGL_VideoEncoder(): NULL SwsConvertContextYUV for '%s'\n",src);
			printf("\tSrc Width/Height: %ix%i\n",SrcCodecContext->width,SrcCodecContext->height);
			printf("\tSrc Format: %i\n",SrcCodecContext->pix_fmt);
			return;
		}

		SwsConvertContextBGRA = lgl_sws_getContext
		(
			//src
			DstCodecContext->width,
			DstCodecContext->height,
			DstCodecContext->pix_fmt,
			//dst
			DstCodecContext->width,
			DstCodecContext->height,
			PIX_FMT_BGRA,
			SWS_FAST_BILINEAR,
			NULL,
			NULL,
			NULL
		);
		if(SwsConvertContextBGRA==NULL)
		{
			printf("LGL_VideoEncoder::LGL_VideoEncoder(): NULL SwsConvertContextBGRA for '%s'\n",src);
			printf("\tSrc Width/Height: %ix%i\n",SrcCodecContext->width,SrcCodecContext->height);
			printf("\tSrc Format: %i\n",SrcCodecContext->pix_fmt);
			return;
		}

		SrcFileBytes=LGL_FileLengthBytes(src);

		lgl_av_write_header(DstFormatContext);

		avpicture_alloc
		(
			(AVPicture*)DstFrameYUV,
			DstCodecContext->pix_fmt,
			SrcBufferWidth,
			SrcBufferHeight
		);
		avpicture_alloc
		(
			(AVPicture*)DstFrameBGRA,
			PIX_FMT_BGRA,
			SrcBufferWidth,
			SrcBufferHeight
		);

		DstBufferYUV = (uint8_t*)lgl_av_mallocz(SrcBufferWidth*SrcBufferHeight*4);
		DstBufferBGRA = (uint8_t*)lgl_av_mallocz(SrcBufferWidth*SrcBufferHeight*4);
		avpicture_fill
		(
			(AVPicture*)DstFrameBGRA,
			DstBufferBGRA,
			PIX_FMT_BGRA,
			SrcBufferWidth,
			SrcBufferHeight
		);

		//Mp3 output
		if(SrcAudioStreamIndex!=-1)
		{
			CodecID acodec = CODEC_ID_FLAC;

			// find the audio encoder
			DstMp3Codec = lgl_avcodec_find_encoder(acodec);
			if(DstMp3Codec==NULL)
			{
				printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't find audio encoding codec for '%s'\n",src);
				return;
			}

			// Set format
			DstMp3OutputFormat = av_guess_format("flac", NULL, NULL);
			if(DstMp3OutputFormat==NULL)
			{
				printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't find audio encoding format for '%s'\n",src);
				return;
			}
			DstMp3OutputFormat->audio_codec = acodec;	//FIXME: mp3?? vorbis?? Pick one and name appropriately!
			DstMp3OutputFormat->video_codec = CODEC_ID_NONE;

			// FormatContext
			DstMp3FormatContext = lgl_avformat_alloc_context();	//FIXME: We never explicitly deallocate this... memleak?
			DstMp3FormatContext->oformat = DstMp3OutputFormat;
			sprintf(DstMp3FormatContext->filename, "%s",DstMp3Path);

			// audio stream
			DstMp3Stream = lgl_av_new_stream(DstMp3FormatContext,0);
			DstMp3Stream->r_frame_rate=SrcFormatContext->streams[SrcAudioStreamIndex]->r_frame_rate;
			//Next line doesn't seem to be accurate. So commented out.
			//DstMp3Stream->avg_frame_rate=SrcFormatContext->streams[SrcAudioStreamIndex]->avg_frame_rate;
			DstMp3Stream->quality=FF_QP2LAMBDA * 100;
			DstMp3FormatContext->streams[0] = DstMp3Stream;
			DstMp3FormatContext->nb_streams = 1;

			DstMp3CodecContext = DstMp3Stream->codec;
			lgl_avcodec_get_context_defaults(DstMp3CodecContext);
			DstMp3CodecContext->codec_id=acodec;
			DstMp3CodecContext->codec_type=CODEC_TYPE_AUDIO;
			if(DstOutputFormat->flags & AVFMT_GLOBALHEADER)
			{
				DstMp3CodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
			}
			DstMp3CodecContext->bit_rate=256*1024;	//This doesn't seem to matter much
			DstMp3CodecContext->bit_rate_tolerance=DstMp3CodecContext->bit_rate/4;
			DstMp3CodecContext->global_quality=DstMp3Stream->quality;
			DstMp3CodecContext->flags |= CODEC_FLAG_QSCALE;
			DstMp3CodecContext->sample_rate=SrcAudioCodecContext->sample_rate;
			DstMp3CodecContext->time_base = (AVRational){1,DstMp3CodecContext->sample_rate};
			DstMp3CodecContext->channels=
				(SrcAudioCodecContext->channels>=2) ?
				SrcAudioCodecContext->channels :
				2;
			DstMp3CodecContext->sample_fmt=SAMPLE_FMT_S16;

			lgl_av_log_set_level(AV_LOG_WARNING);	//QUIET, ERROR, WARNING, INFO, VERBOSE, DEBUG
			int openResult=-1;
			{
				openResult = lgl_avcodec_open(DstMp3CodecContext, DstMp3Codec);
			}
			if(openResult < 0)
			{
				printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't open audio encoder codec for '%s'\n",src);
				EncodeAudio=false;
			}
			else
			{
				//dump_format(DstMp3FormatContext,0,DstMp3FormatContext->filename,1);
				
				result = lgl_url_fopen(&(DstMp3FormatContext->pb), DstMp3FormatContext->filename, URL_WRONLY);
				if(result<0)
				{
					printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't url_fopen() audio output file '%s' (%i)\n",DstMp3FormatContext->filename,result);
					return;
				}

				lgl_av_write_header(DstMp3FormatContext);

				DstMp3Buffer = (int16_t*)lgl_av_mallocz(LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE);
				DstMp3BufferSamples = (int16_t*)lgl_av_mallocz(LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE);
				DstMp3Buffer2 = (int16_t*)lgl_av_mallocz(LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE);
			}
		}
	}

	Valid=1;
}

LGL_VideoEncoder::
~LGL_VideoEncoder()
{
	//Src
	if(SrcCodecContext)
	{
		lgl_avcodec_close(SrcCodecContext);
		SrcCodecContext=NULL;
	}
	if(SrcAudioCodecContext)
	{
		lgl_avcodec_close(SrcAudioCodecContext);
		SrcAudioCodecContext=NULL;
	}
	if(SrcFormatContext)
	{
		lgl_av_close_input_file(SrcFormatContext);
		SrcFormatContext=NULL;
	}
	if(SrcCodec)
	{
		//Don't think this needs to be freed
	}
	if(SrcAudioCodec)
	{
		//Don't think this needs to be freed
	}
	if(SrcFrame)
	{
		//FIXME: The docs say I need to free this like this, but it triggers malloc_error_break... Commenting out for now, and possibly leaking...
		//lgl_av_freep(SrcFrame);
		SrcFrame=NULL;
	}

	if(SwsConvertContextYUV)
	{
		sws_freeContext(SwsConvertContextYUV);
		SwsConvertContextYUV=NULL;
	}
	if(SwsConvertContextBGRA)
	{
		sws_freeContext(SwsConvertContextBGRA);
		SwsConvertContextBGRA=NULL;
	}

	//Dst
	if(DstOutputFormat)
	{
		//Don't free this...
	}
	if(DstCodecContext)
	{
		lgl_avcodec_close(DstCodecContext);
	}
	if(DstCodec)
	{
		//Don't think this needs to be freed
	}
	if(DstFormatContext)
	{
		if(DstFormatContext->pb)
		{
			lgl_url_fclose(DstFormatContext->pb);
			DstFormatContext->pb=NULL;
		}
		lgl_av_freep(&DstFormatContext);
	}

	if(DstMp3FormatContext)
	{
		if(DstMp3FormatContext->pb)
		{
			lgl_url_fclose(DstMp3FormatContext->pb);
			DstMp3FormatContext->pb=NULL;
		}
		lgl_av_freep(&DstMp3FormatContext);
	}
	if(DstMp3CodecContext)
	{
		lgl_avcodec_close(DstMp3CodecContext);
		DstMp3CodecContext=NULL;
	}
	if(DstMp3Codec)
	{
		//Don't think this needs to be freed
	}
	if(DstStream)
	{
		lgl_av_freep(DstStream);
	}
	if(DstFrameYUV)
	{
		lgl_av_freep(DstFrameYUV);
	}
	if(DstFrameBGRA)
	{
		lgl_av_freep(DstFrameBGRA);
	}
	if(DstBufferYUV)
	{
		lgl_av_freep(&DstBufferYUV);
	}
	if(DstBufferBGRA)
	{
		//Causes malloc_error_break() to be called... But why?!
		//lgl_av_freep(&DstBufferBGRA);
	}
	if(DstMp3Buffer)
	{
		lgl_av_freep(&DstMp3Buffer);
	}
	if(DstMp3BufferSamples)
	{
		lgl_av_freep(&DstMp3BufferSamples);
	}
	if(DstMp3Buffer2)
	{
		lgl_av_freep(&DstMp3Buffer2);
	}

	if(Image)
	{
		delete Image;
		Image=NULL;
	}
}

bool
LGL_VideoEncoder::
IsValid()
{
	return(Valid==1);
}

bool
LGL_VideoEncoder::
IsUnsupportedCodec()
{
	return(UnsupportedCodec);
}

void
LGL_VideoEncoder::
SetEncodeAudio(bool encode)
{
	EncodeAudio=EncodeAudio && encode;
}

void
LGL_VideoEncoder::
SetEncodeVideo(bool encode)
{
	EncodeVideo=encode;
}

bool
LGL_VideoEncoder::
GetEncodeAudio()
{
	return(EncodeAudio);
}

bool
LGL_VideoEncoder::
GetEncodeVideo()
{
	return(EncodeVideo);
}

void
LGL_VideoEncoder::
Encode
(
	int frames
)
{
	if(Valid==false)
	{
		return;
	}
	if(IsFinished())
	{
		return;
	}

	int result=0;

	for(int a=0;a<frames;a++)
	{
		//Decode a src frame
		{
			result = lgl_av_read_frame(SrcFormatContext, &SrcPacket);
			if(SrcPacket.pos==-1)
			{
				SrcPacketPosMax+=SrcPacket.size;
			}
		}

		int srcPacketSizeOrig=SrcPacket.size;
		uint8_t* srcPacketDataOrig=SrcPacket.data;

		bool eof=false;
		if(result<0)
		{
			eof=true;
			SrcPacket.size=0;
			SrcPacket.data=NULL;
		}

		// Is this a packet from the video stream?
		if
		(
			EncodeVideo &&
			(
				SrcPacket.stream_index==SrcVideoStreamIndex ||
				eof
			)
		)
		{
			int frameFinished=0;
			{
				lgl_avcodec_decode_video2
				(
					SrcCodecContext,
					SrcFrame,
					&frameFinished, 
					&SrcPacket
				);
			}

			// Did we get a video frame?
			if(frameFinished)
			{
				// Convert the image from src format to dst
				{
					//Is this sws_scale line actually necessary...?
					{
						LGL_ScopeLock dstFrameYUVLock(__FILE__,__LINE__,DstFrameYUVSemaphore);
						lgl_sws_scale
						(
							SwsConvertContextYUV,
							SrcFrame->data,
							SrcFrame->linesize,
							0, 
							SrcBufferHeight,
							DstFrameYUV->data,
							DstFrameYUV->linesize
						);
					}
					DstFrameYUV->quality=1;	//Best
					DstPacket.size = lgl_avcodec_encode_video(DstCodecContext, DstBufferYUV, SrcBufferWidth*SrcBufferHeight*4, DstFrameYUV);
				}

				int64_t conversionFactorNum =
						SrcFormatContext->streams[SrcVideoStreamIndex]->time_base.num *
						DstStream->time_base.den;
				int64_t conversionFactorDen = SrcFormatContext->streams[SrcVideoStreamIndex]->time_base.den *
						DstStream->time_base.num;
				//float conversionFactorVal = (float)(conversionFactorNum/(float)conversionFactorDen);

				int64_t dtsPrev=DstPacket.dts;
				int64_t ptsPrev=DstPacket.pts;
				int64_t candidatePTS = (SrcPacket.pts * conversionFactorNum)/conversionFactorDen;
				int64_t candidateDTS = (SrcPacket.dts * conversionFactorNum)/conversionFactorDen;
				if(candidatePTS>ptsPrev)
				{
					DstPacket.pts=candidatePTS;
				}
				else
				{
					DstPacket.pts++;
				}
				if(candidateDTS>dtsPrev)
				{
					DstPacket.dts=candidateDTS;
				}
				else
				{
					DstPacket.dts++;
				}

				DstPacket.flags |= PKT_FLAG_KEY;
				DstPacket.stream_index = 0;
				DstPacket.data=DstBufferYUV;
				DstPacket.duration=SrcPacket.duration;
				{
					result = lgl_av_write_frame(DstFormatContext, &DstPacket);
				}
				SrcFrameNow++;
			}
		}
		
		// Is this a packet from the audio stream?
		if
		(
			EncodeAudio &&
			(
				SrcPacket.stream_index==SrcAudioStreamIndex ||
				eof
			)
		)
		{
			// Convert the audio from src format to dst
			{
				int outbufsize = LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE;
	
				while(SrcPacket.size>0)
				{
					//FIXME: We don't flush this decoder sufficiently.
					result = lgl_avcodec_decode_audio3
					(
						SrcAudioCodecContext,
						DstMp3Buffer,
						&outbufsize,
						&SrcPacket
					);
					if(result<0)
					{
						//printf("lgl_avcodec_decode_audio3(): Error!\n");
						break;
					}
					SrcPacket.size-=result;
					SrcPacket.data+=result;
					if(DstMp3BufferSrcPts==0)
					{
						DstMp3BufferSrcPts = SrcPacket.pts;
					}
					if(SrcAudioCodecContext->channels>=2)
					{
						memcpy(&(DstMp3BufferSamples[DstMp3BufferSamplesIndex]),DstMp3Buffer,outbufsize);
						DstMp3BufferSamplesIndex+=outbufsize/2;
						DstMp3BufferSamplesTotalBytes+=outbufsize/2;
					}
					else
					{
						int16_t* outbuf16 = (int16_t*)DstMp3Buffer;
						int16_t* buf16 = (int16_t*)(&(DstMp3BufferSamples[DstMp3BufferSamplesIndex])); 
						int samples=outbufsize/2;

						for(int a=0;a<samples;a++)
						{
							buf16[2*a+0]=outbuf16[a];
							buf16[2*a+1]=outbuf16[a];
						}

						DstMp3BufferSamplesIndex+=samples*2;
						DstMp3BufferSamplesTotalBytes+=outbufsize;
					}
				}
			}

			FlushAudioBuffer();
		}

		SrcPacket.size=srcPacketSizeOrig;
		SrcPacket.data=srcPacketDataOrig;
		{
			lgl_av_free_packet(&SrcPacket);
		}

		if(eof)
		{
printf("Src->r_frame_rate = %i / %i (%.2f)\n",
	SrcFormatContext->streams[SrcVideoStreamIndex]->r_frame_rate.num,
	SrcFormatContext->streams[SrcVideoStreamIndex]->r_frame_rate.den,
	SrcFormatContext->streams[SrcVideoStreamIndex]->r_frame_rate.num/(float)
	SrcFormatContext->streams[SrcVideoStreamIndex]->r_frame_rate.den);
printf("Src->time_base = %i / %i (%.2f) (%.2f)\n",
	SrcFormatContext->streams[SrcVideoStreamIndex]->time_base.num,
	SrcFormatContext->streams[SrcVideoStreamIndex]->time_base.den,
	SrcFormatContext->streams[SrcVideoStreamIndex]->time_base.num/(float)
	SrcFormatContext->streams[SrcVideoStreamIndex]->time_base.den,
	SrcFormatContext->streams[SrcVideoStreamIndex]->time_base.den/(float)
	SrcFormatContext->streams[SrcVideoStreamIndex]->time_base.num);
printf("Src->codec->time_base = %i / %i (%.2f) (%.2f)\n",
	SrcFormatContext->streams[SrcVideoStreamIndex]->codec->time_base.num,
	SrcFormatContext->streams[SrcVideoStreamIndex]->codec->time_base.den,
	SrcFormatContext->streams[SrcVideoStreamIndex]->codec->time_base.num/(float)
	SrcFormatContext->streams[SrcVideoStreamIndex]->codec->time_base.den,
	SrcFormatContext->streams[SrcVideoStreamIndex]->codec->time_base.den/(float)
	SrcFormatContext->streams[SrcVideoStreamIndex]->codec->time_base.num);
printf("Src ticks_per_frame: %i\n",SrcCodecContext->ticks_per_frame);

printf("Dst->r_frame_rate = %i / %i (%.2f)\n",
	DstStream->r_frame_rate.num,
	DstStream->r_frame_rate.den,
	DstStream->r_frame_rate.num/(float)
	DstStream->r_frame_rate.den);
printf("Dst->time_base = %i / %i (%.2f) (%.2f)\n",
	DstStream->time_base.num,
	DstStream->time_base.den,
	DstStream->time_base.num/(float)
	DstStream->time_base.den,
	DstStream->time_base.den/(float)
	DstStream->time_base.num);
printf("Dst->codec->time_base = %i / %i (%.2f) (%.2f)\n",
	DstStream->codec->time_base.num,
	DstStream->codec->time_base.den,
	DstStream->codec->time_base.num/(float)
	DstStream->codec->time_base.den,
	DstStream->codec->time_base.den/(float)
	DstStream->codec->time_base.num);
printf("Dst ticks_per_frame: %i\n",DstCodecContext->ticks_per_frame);

			//We're done!
			if(EncodeVideo)
			{
				lgl_av_write_trailer(DstFormatContext);
				lgl_url_fclose(DstFormatContext->pb);	//FIXME: Memleak
			}
			DstFormatContext->pb=NULL;

			if(EncodeAudio)
			{
				FlushAudioBuffer(true);

				//Write silence to match video duration, if necessary
				float lengthSeconds=SrcFormatContext->duration/(float)(AV_TIME_BASE);
				long bufferLengthProper=lengthSeconds*SrcAudioCodecContext->sample_rate*2;
				bzero(DstMp3BufferSamples,LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE);
				while(DstMp3BufferSamplesTotalBytes<bufferLengthProper)
				{
					DstMp3BufferSamplesIndex=LGL_Min
					(
						bufferLengthProper-DstMp3BufferSamplesTotalBytes,
						DstMp3CodecContext->frame_size*DstMp3CodecContext->channels
					);
					DstMp3BufferSamplesTotalBytes+=DstMp3BufferSamplesIndex;
					FlushAudioBuffer(true);
				}

				lgl_av_write_trailer(DstMp3FormatContext);
				lgl_url_fclose(DstMp3FormatContext->pb);	//FIXME: Memleak
			}
			if(DstMp3FormatContext)
			{
				DstMp3FormatContext->pb=NULL;
			}
			break;
		}
	}
}

void
LGL_VideoEncoder::
FlushAudioBuffer
(
	bool	force
)
{
	if
	(
		force &&
		DstMp3BufferSamplesIndex!=0
	)
	{
		//Add silence
		bzero
		(
			DstMp3BufferSamples+DstMp3BufferSamplesIndex,
			DstMp3CodecContext->frame_size*DstMp3CodecContext->channels -
			DstMp3BufferSamplesIndex
		);
		DstMp3BufferSamplesIndex=DstMp3CodecContext->frame_size*DstMp3CodecContext->channels;
	}

	while(DstMp3BufferSamplesIndex>=DstMp3CodecContext->frame_size*DstMp3CodecContext->channels)
	{
		DstMp3Packet.size = lgl_avcodec_encode_audio
		(
			DstMp3CodecContext,
			(uint8_t*)DstMp3Buffer2,
			LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE,
			DstMp3BufferSamples
		);

		int remainderIndex=DstMp3CodecContext->frame_size*DstMp3CodecContext->channels;
		int remainderSize=DstMp3BufferSamplesIndex-remainderIndex;
		memcpy(DstMp3Buffer,&(DstMp3BufferSamples[remainderIndex]),remainderSize*2);
		memcpy(DstMp3BufferSamples,DstMp3Buffer,remainderSize*2);
		DstMp3BufferSamplesIndex=remainderSize;

		//DstMp3Packet.pts = lgl_av_rescale_q(DstMp3CodecContext->coded_frame->pts,DstMp3CodecContext->time_base,DstMp3Stream->time_base);
		//DstMp3Packet.dts = DstMp3Packet.pts;
		//DstPacket.dts = SrcPacket.dts;
		//DstPacket.pts = SrcPacket.pts;//DstCodecContext->coded_frame->pts;
		int64_t conversionFactorNum =
				SrcFormatContext->streams[SrcAudioStreamIndex]->time_base.num *
				DstStream->time_base.den;
		int64_t conversionFactorDen = SrcFormatContext->streams[SrcAudioStreamIndex]->time_base.den *
				DstStream->time_base.num;
		//float conversionFactorVal = (float)(conversionFactorNum/(float)conversionFactorDen);

		DstMp3Packet.pts = (DstMp3BufferSrcPts * conversionFactorNum)/conversionFactorDen;
		DstMp3Packet.dts = DstMp3Packet.pts - 1;
				int64_t dtsPrev=DstMp3Packet.dts;
				int64_t ptsPrev=DstMp3Packet.pts;
				int64_t candidatePTS = (DstMp3BufferSrcPts * conversionFactorNum)/conversionFactorDen;
				int64_t candidateDTS = ((DstMp3BufferSrcPts * conversionFactorNum)/conversionFactorDen) - 1;
				if(candidatePTS>ptsPrev)
				{
					DstMp3Packet.pts=candidatePTS;
				}
				else
				{
					DstMp3Packet.pts++;
				}
				if(candidateDTS>dtsPrev)
				{
					DstMp3Packet.dts=candidateDTS;
				}
				else
				{
					DstMp3Packet.dts++;
				}

		DstMp3BufferSrcPts=0;

		DstMp3Packet.flags |= PKT_FLAG_KEY;
		DstMp3Packet.stream_index = 0;
		DstMp3Packet.data=(uint8_t*)DstMp3Buffer2;
		//DstMp3Packet.duration=0;
		lgl_av_write_frame(DstMp3FormatContext, &DstMp3Packet);
	}
}

float
LGL_VideoEncoder::
GetPercentFinished()
{
	if(Valid==false)
	{
		return(0.0f);
	}
	if(IsFinished())
	{
		return(1.0f);
	}

	SrcPacketPosMax=LGL_Max(SrcPacket.pos,SrcPacketPosMax);
	double bytePos = SrcPacketPosMax;
	double byteLen = SrcFileBytes;
	return(LGL_Clamp(0,bytePos/byteLen,1));
}

bool
LGL_VideoEncoder::
IsFinished()
{
	if(Valid==false)
	{
		return(true);
	}

	return(DstFormatContext->pb==NULL);
}

const char*
LGL_VideoEncoder::
GetCodecName()
{
	return(SrcCodecName);
}

bool
LGL_VideoEncoder::
IsMJPEG()
{
	if(Valid==false)
	{
		return(false);
	}

	int result=false;
	if(SrcCodecContext)
	{
		result = 
		(
			SrcCodecContext->codec_id == CODEC_ID_MJPEG ||
			SrcCodecContext->codec_id == CODEC_ID_MJPEGB ||
			SrcCodecContext->codec_id == CODEC_ID_LJPEG
		);
	}
	return(result);
}

float
LGL_VideoEncoder::
GetBitrateMaxMBps()
{
	return(BitrateMaxMBps);
}

void
LGL_VideoEncoder::
SetBitrateMaxMBps
(
	float max
)
{
	BitrateMaxMBps=LGL_Clamp(0.1f,max,100.0f);
}

LGL_Image*
LGL_VideoEncoder::
GetImage()
{
	//Ensure Image Exists
	if(Image==NULL)
	{
		Image = new LGL_Image
		(
			512,//1920,
			512,//1024,
			4,
			NULL,
			true,
			"Empty LGL_VideoEncoder"
		);
		Image->SetVideoPath(SrcPath);
	}

	{
		LGL_ScopeLock lock(__FILE__,__LINE__,DstFrameYUVSemaphore,0.0f);
		if(lock.GetLockObtained()==false)
		{
			return(Image);
		}
		if(DstFrameYUV->quality==1)
		{
			lgl_sws_scale
			(
				SwsConvertContextBGRA,
				DstFrameYUV->data,
				DstFrameYUV->linesize,
				0, 
				SrcBufferHeight,
				DstFrameBGRA->data,
				DstFrameBGRA->linesize
			);
		}
	}

	//Update Image
	if
	(
		SrcBufferWidth<=0 ||
		SrcBufferHeight<=0
	)
	{
		Image->SetFrameNumber(-1);
		return(Image);
	}

	Image->UpdateTexture
	(
		SrcBufferWidth,
		SrcBufferHeight,
		4,
		DstBufferBGRA,
		true,
		SrcPath
	);
	Image->SetFrameNumber(1);
	Image->InvertY=true;

	return(Image);
}

const char*
LGL_VideoEncoder::
GetSrcPath()
{
	return(SrcPath);
}

//LGL_AudioEncoder

int
lgl_AudioEncoderThread
(
	void* object
)
{
	//LGL_ThreadSetCPUAffinity(0);
	LGL_ThreadSetPriority(LGL_PRIORITY_AUDIO_ENCODE,"AudioEncode");
	LGL_AudioEncoder* encoder=(LGL_AudioEncoder*)object;
	encoder->ThreadFunc();
	return(0);
}

LGL_AudioEncoder::
LGL_AudioEncoder
(
	const char*	dstPath,
	bool		surroundMode
)
{
	strcpy(DstMp3Path,dstPath);
	SurroundMode=surroundMode;

	DstMp3OutputFormat=NULL;
	DstMp3FormatContext=NULL;
	DstMp3CodecContext=NULL;
	DstMp3Codec=NULL;
	DstMp3Stream=NULL;
	DstMp3Buffer=NULL;
	DstMp3BufferSamples=NULL;
	DstMp3BufferSamplesIndex=0;
	DstMp3Buffer2=NULL;

	CircularBufferBytes=LGL_Max(5*1024*1024,LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE);
	CircularBuffer=NULL;
	CircularBufferHead=0;
	CircularBufferTail=0;

	Valid=false;
	AVOpened=false;
	Thread=NULL;
	DestructHint=false;

	CircularBuffer = new char[CircularBufferBytes];

	Valid=true;
	Thread=LGL_ThreadCreate(lgl_AudioEncoderThread,this);
}

LGL_AudioEncoder::
~LGL_AudioEncoder()
{
	DestructHint=true;
	LGL_ThreadWait(Thread);

	Finalize();

	if(CircularBuffer)
	{
		delete CircularBuffer;
		CircularBuffer=NULL;
	}

	{
		if(DstMp3Buffer)
		{
			lgl_av_freep(&DstMp3Buffer);
		}
		if(DstMp3BufferSamples)
		{
			lgl_av_freep(&DstMp3BufferSamples);
		}
		if(DstMp3Buffer2)
		{
			lgl_av_freep(&DstMp3Buffer2);
		}
	}
}

bool
LGL_AudioEncoder::
IsValid()
{
	return(Valid);
}

void
LGL_AudioEncoder::
Encode
(
	const char*	data,
	long		bytes
)
{
	LGL_ScopeLock lock(__FILE__,__LINE__,LGL.AudioEncoderSemaphore);

	if
	(
		Valid==false ||
		DestructHint
	)
	{
		return;
	}

	while(bytes>0)
	{
		long copyLen = bytes;
		if(CircularBufferHead + copyLen > CircularBufferBytes)
		{
			copyLen=CircularBufferBytes-CircularBufferHead;
		}

		memcpy(&(CircularBuffer[CircularBufferHead]),data,copyLen);

		if(CircularBufferHead + copyLen == CircularBufferBytes)
		{
			CircularBufferHead=0;
		}
		else
		{
			if
			(
				CircularBufferHead < CircularBufferTail &&
				CircularBufferHead+copyLen > CircularBufferTail
			)
			{
				printf("LGL_AudioEncoder::Encode(): Buffer Overflow!\n");
			}
			CircularBufferHead+=copyLen;
		}
		data+=copyLen;
		bytes-=copyLen;
	}
}

void
LGL_AudioEncoder::
Finalize()
{
	if(Valid==false)
	{
		return;
	}

	FlushBuffer(true);
	/*
	for(int a=0;a<5;a++)
	{
		DstMp3BufferSamples[0]=0;
		DstMp3BufferSamples[1]=0;
		DstMp3BufferSamplesIndex=2;
		FlushBuffer(true);
	}
	*/

	{
		lgl_av_write_trailer(DstMp3FormatContext);
		lgl_url_fclose(DstMp3FormatContext->pb);	//FIXME: Memleak
	}

	if(DstMp3FormatContext)
	{
		DstMp3FormatContext->pb=NULL;
	}
}

int
LGL_AudioEncoder::
GetChannelCount()
{
	if(DstMp3CodecContext==NULL)
	{
		return(0);
	}
	else
	{
		return(DstMp3CodecContext->channels);
	}
}

void
LGL_AudioEncoder::
ThreadFunc()
{
	for(;;)
	{
		FlushBuffer();
		LGL_DelayMS(5);
		if
		(
			DestructHint ||
			LGL.AudioQuitting
		)
		{
			return;
		}
	}
}

void
LGL_AudioEncoder::
FlushBuffer
(
	bool	force
)
{
	LGL_ScopeLock(__FILE__,__LINE__,LGL.AudioEncoderSemaphore);

	if(AVOpened==false)
	{
		AVOpened=true;
		Valid=false;
		if(FILE* fd=fopen(DstMp3Path,"w"))
		{
			fclose(fd);
			fd=NULL;
			LGL_FileDelete(DstMp3Path);
		}
		else
		{
			return;
		}

		CodecID acodec = CODEC_ID_FLAC;

		// find the audio encoder
		DstMp3Codec = lgl_avcodec_find_encoder(acodec);
		if(DstMp3Codec==NULL)
		{
			printf("LGL_AudioEncoder::LGL_AudioEncoder(): Couldn't find audio encoding codec\n");
			return;
		}

		// Set format
		DstMp3OutputFormat = av_guess_format("flac", NULL, NULL);
		if(DstMp3OutputFormat==NULL)
		{
			printf("LGL_AudioEncoder::LGL_AudioEncoder(): Couldn't find audio encoding format\n");
			return;
		}
		DstMp3OutputFormat->audio_codec = acodec;	//FIXME: mp3?? vorbis?? Pick one and name appropriately!
		DstMp3OutputFormat->video_codec = CODEC_ID_NONE;

		// FormatContext
		DstMp3FormatContext = lgl_avformat_alloc_context();	//FIXME: We never explicitly deallocate this... memleak?
		DstMp3FormatContext->oformat = DstMp3OutputFormat;
		sprintf(DstMp3FormatContext->filename, "%s",DstMp3Path);

		// audio stream
		DstMp3Stream = lgl_av_new_stream(DstMp3FormatContext,0);
		DstMp3Stream->quality=FF_QP2LAMBDA * 100;
		DstMp3FormatContext->streams[0] = DstMp3Stream;
		DstMp3FormatContext->nb_streams = 1;

		DstMp3CodecContext = DstMp3Stream->codec;
		lgl_avcodec_get_context_defaults(DstMp3CodecContext);
		DstMp3CodecContext->codec_id=acodec;
		DstMp3CodecContext->codec_type=CODEC_TYPE_AUDIO;
		DstMp3CodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
		DstMp3CodecContext->bit_rate=256*1024;	//This doesn't seem to matter much
		DstMp3CodecContext->bit_rate_tolerance=DstMp3CodecContext->bit_rate/4;
		DstMp3CodecContext->global_quality=DstMp3Stream->quality;
		DstMp3CodecContext->flags |= CODEC_FLAG_QSCALE;
		DstMp3CodecContext->sample_rate=LGL.AudioSpec->freq;
		DstMp3CodecContext->time_base = (AVRational){1,DstMp3CodecContext->sample_rate};
		DstMp3CodecContext->channels=SurroundMode ? 4 : 2;
		DstMp3CodecContext->sample_fmt=SAMPLE_FMT_S16;

		int openResult=-1;
		{
			openResult = lgl_avcodec_open(DstMp3CodecContext, DstMp3Codec);
		}
		if(openResult < 0)
		{
			printf("LGL_AudioEncoder::LGL_AudioEncoder(): Couldn't open audio encoder codec\n");
			return;
		}
		else
		{
			//dump_format(DstMp3FormatContext,0,DstMp3FormatContext->filename,1);
			
			int result = lgl_url_fopen(&(DstMp3FormatContext->pb), DstMp3FormatContext->filename, URL_WRONLY);
			if(result<0)
			{
				printf("LGL_AudioEncoder::LGL_AudioEncoder(): Couldn't url_fopen() audio output file '%s' (%i)\n",DstMp3FormatContext->filename,result);
				return;
			}

			lgl_av_write_header(DstMp3FormatContext);

			DstMp3Buffer = (int16_t*)lgl_av_mallocz(CircularBufferBytes);
			DstMp3BufferSamples = (int16_t*)lgl_av_mallocz(CircularBufferBytes*2);
			DstMp3Buffer2 = (int16_t*)lgl_av_mallocz(CircularBufferBytes);

			lgl_av_init_packet(&DstMp3Packet);
		}

		Valid=true;
	}

	if(Valid==false)
	{
		return;
	}

	//Circular buffer to fixed buffer
	long circularBufferHead=CircularBufferHead;
	while(CircularBufferTail!=circularBufferHead)
	{
		long circularBufferTarget=circularBufferHead;
		if
		(
			circularBufferTarget==CircularBufferTail &&
			force==false
		)
		{
			return;
		}
		else if(circularBufferTarget<CircularBufferTail)
		{
			//Just flush 'till the end of the circular buffer
			circularBufferTarget=CircularBufferBytes;
		}
		
		memcpy(&(DstMp3BufferSamples[DstMp3BufferSamplesIndex]),&(CircularBuffer[CircularBufferTail]),circularBufferTarget-CircularBufferTail);
		
		DstMp3BufferSamplesIndex+=(circularBufferTarget-CircularBufferTail)/2;
		CircularBufferTail+=(circularBufferTarget-CircularBufferTail);
		if(CircularBufferTail==CircularBufferBytes)
		{
			CircularBufferTail=0;
		}
	}

	if
	(
		0 &&
		force &&
		DstMp3BufferSamplesIndex!=0
	)
	{
		//Add silence
		bzero
		(
			&(DstMp3BufferSamples[DstMp3BufferSamplesIndex]),
			DstMp3CodecContext->frame_size*DstMp3CodecContext->channels -
			DstMp3BufferSamplesIndex*2
		);
		DstMp3BufferSamplesIndex=DstMp3CodecContext->frame_size*DstMp3CodecContext->channels;
	}

	while(DstMp3BufferSamplesIndex>=DstMp3CodecContext->frame_size*DstMp3CodecContext->channels)
	{
		DstMp3Packet.size = lgl_avcodec_encode_audio
		(
			DstMp3CodecContext,
			(uint8_t*)DstMp3Buffer2,
			LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE,
			DstMp3BufferSamples
		);

		int remainderIndex=DstMp3CodecContext->frame_size*DstMp3CodecContext->channels;
		int remainderSize=DstMp3BufferSamplesIndex-remainderIndex;
		memcpy(DstMp3Buffer,&(DstMp3BufferSamples[remainderIndex]),remainderSize*2);
		memcpy(DstMp3BufferSamples,DstMp3Buffer,remainderSize*2);
		DstMp3BufferSamplesIndex=remainderSize;

		DstMp3Packet.flags |= PKT_FLAG_KEY;
		DstMp3Packet.stream_index = 0;
		DstMp3Packet.data=(uint8_t*)DstMp3Buffer2;
		lgl_av_write_frame(DstMp3FormatContext, &DstMp3Packet);
		DstMp3Packet.dts++;
		DstMp3Packet.pts++;
	}
}

bool
LGL_VideoIsMJPEG
(
	const char*	path
)
{
	//FIXME: Framerate hiccup like mad here...

	if(LGL_FileExists(path)==false)
	{
		return(false);
	}

	if
	(
		LGL_FileExtensionIsAudio(path) ||
		LGL_FileExtensionIsImage(path)
	)
	{
		return(false);
	}
#if 0
	LGL_VideoEncoder* ve = new LGL_VideoEncoder
	(
		path,
		"/dev/null",
		"/dev/null"
	);
	bool ret=ve->IsMJPEG();
	delete ve;
	return(ret);
#endif

	bool ret=false;

	{
		//Open file
		AVFormatContext* fc=NULL;
		if(lgl_av_open_input_file(&fc, path, NULL, 0, NULL)==0)
		{
			//Find streams
			if(lgl_av_find_stream_info(fc)>=0)
			{
				// Find the first video stream
				int srcVideoStreamIndex=-1;
				for(unsigned int i=0; i<fc->nb_streams; i++)
				{
					if(fc->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO)
					{
						srcVideoStreamIndex=i;
						break;
					}
				}
				if(srcVideoStreamIndex!=-1)
				{
					AVCodecContext* srcCodecContext=fc->streams[srcVideoStreamIndex]->codec;
					ret =
						(
							srcCodecContext->codec_id == CODEC_ID_MJPEG ||
							srcCodecContext->codec_id == CODEC_ID_MJPEGB ||
							srcCodecContext->codec_id == CODEC_ID_LJPEG
						);
				}
			}

			lgl_av_close_input_file(fc);
		}
	}

	return(ret);
}

//LGL_Font

LGL_Font::
LGL_Font
(
	const
	char*	dirpath
)
{
	for(int a=0;a<256;a++)
	{
		Glyph[a]=NULL;
	}
	TextureGL=0;
	ReferenceCount=0;

#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS
	
	LGL_Assertf(dirpath!=NULL,("LGL_Font::LGL_Font(): Error! NULL dirpath!"));
	LGL_SimplifyPath(Path,dirpath);
	if(Path[strlen(Path)-1]=='/')
	{
		Path[strlen(Path)-1]='\0';
	}

	char temp[1024];
	strcpy(temp,Path);
	char* ptr=&(temp[0]);
	while(strstr(ptr,"/"))
	{
		ptr=&(strstr(ptr,"/")[1]);
	}
	strcpy(PathShort,ptr);

	char path[2048];

	int num=0;
	int GlyphSideLength=0;

	//First pass: discover how large a glyph can be.

	for(int a=0;a<256;a++)
	{
		sprintf(path,"%s/%i.png",Path,a);

		if(LGL_FileExists(path))
		{
			Glyph[a]=new LGL_Image(path,false);
			GlyphSideLength=(int)LGL_Max(GlyphSideLength,Glyph[a]->GetWidth());
			GlyphSideLength=(int)LGL_Max(GlyphSideLength,Glyph[a]->GetHeight());
			num++;
			delete Glyph[a];
			Glyph[a]=NULL;
		}
		else
		{
			Glyph[a]=NULL;
			LGL_Assertf(a!=32,("LGL_Font::LGL_Font(): Error. Font '%s' is missing critical glyph #32 (space)",Path));
		}
	}

	LGL_Assertf(GlyphSideLength>0,("LGL_Font::LGL_Font(): Error! No glyphs detected for font '%s'",Path));

//printf("Glyph Max Size: %i x %i (%i)\n",GlyphSideLength,GlyphSideLength,num);

	TextureSideLength = LGL_NextPowerOfTwo((int)(ceil(sqrtf(num))*GlyphSideLength));

//printf("Texture Size: %i x %i\n",TextureSideLength,TextureSideLength);

	//Create our Font texture.
	glGenTextures(1,&(TextureGL));
	glBindTexture(GL_TEXTURE_LGL,TextureGL);
	glTexImage2D
	(
		GL_TEXTURE_LGL,
		0,			//Level of Detail=0
		GL_RGBA,		//Internal Format
		TextureSideLength,
		TextureSideLength,
		0,			//Boarder=0
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		NULL
	);
	glTexParameteri
	(
		GL_TEXTURE_LGL,
		GL_TEXTURE_MIN_FILTER,
		GL_NEAREST
	);
	glTexParameteri
	(
		GL_TEXTURE_LGL,
		GL_TEXTURE_MAG_FILTER,
		GL_NEAREST
	);

	glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_LGL, GL_TEXTURE_WRAP_T, GL_CLAMP);

	LGL.TexturePixels+=TextureSideLength*TextureSideLength;

	//Second pass: load all glyphs to a single large texture, using a special LGL_Image constructor

	int x=0;
	int y=0;

	for(int a=0;a<256;a++)
	{
		sprintf(path,"%s/%i.png",Path,a);
		
		if(LGL_FileExists(path))
		{
			Glyph[a]=new LGL_Image(path,false,true,TextureGL,x,y);
			GlyphTexLeft[a]=x;
			GlyphTexRight[a]=x+Glyph[a]->GetWidth();
			GlyphTexBottom[a]=y+Glyph[a]->GetHeight();
			GlyphTexTop[a]=y;
			GlyphTexWidth[a]=GlyphTexRight[a]-GlyphTexLeft[a];
			GlyphTexHeight[a]=GlyphTexBottom[a]-GlyphTexTop[a];
			GlyphTexWidthHeightRatio[a]=(float)GlyphTexWidth[a]/(float)GlyphTexHeight[a];

			x+=GlyphSideLength;
			if(x+GlyphSideLength>TextureSideLength)
			{
				x=0;
				y+=GlyphSideLength;
				LGL_Assert(y+GlyphSideLength<=TextureSideLength);
			}
		}
		else
		{
			Glyph[a]=NULL;
			LGL_Assertf(a!=32,("LGL_Font::LGL_Font(): Error. Font '%s' is missing critical glyph #32 (space)",Path));
		}
	}

	DrawingString=false;
}

LGL_Font::
~LGL_Font()
{
	if(ReferenceCount!=0)
	{
		printf("LGL_Font::~LGL_Font(): Error! Reference Count for Font '%s' is %i, not 0!\n",Path,ReferenceCount);
		assert(ReferenceCount==0);
	}

	for(int a=0;a<256;a++)
	{
		if(Glyph[a]!=NULL)
		{
			delete Glyph[a];
		}
	}
}

float*
lgl_GetFontBuffer(int floatCount)
{
	float* ret = lgl_font_gl_buffer_ptr;
	lgl_font_gl_buffer_ptr = &(lgl_font_gl_buffer_ptr[floatCount]);
	assert(lgl_font_gl_buffer_ptr < lgl_font_gl_buffer + FONT_BUFFER_SIZE);
	return(ret);
}

int*
lgl_GetFontBufferInt(int intCount)
{
	int* ret = lgl_font_gl_buffer_int_ptr;
	lgl_font_gl_buffer_int_ptr = &(lgl_font_gl_buffer_int_ptr[intCount]);
	assert(lgl_font_gl_buffer_int_ptr < lgl_font_gl_buffer_int + FONT_BUFFER_SIZE);
	return(ret);
}

float
LGL_Font::
DrawString
(
	float	x,	float	y,	float	height,
	float	r,	float	g,	float	b,	float a,
	bool	centered,
	float	shadowAlpha,
const	char	*string,
	...
)
{
#ifdef	LGL_NO_GRAPHICS
	return(0.0f);
#endif	//LGL_NO_GRAPHICS

	if(string==NULL)
	{
		return(0.0f);
	}

	//Process the formatted part of the string
	char tmpstr[1024*8];
	va_list args;
	va_start(args,string);
	vsprintf(tmpstr,string,args);
	va_end(args);

	PrintMissingGlyphs(tmpstr);

	lgl_glScreenify2D();

	if(LGL.DrawLogFD && !LGL.DrawLogPause)
	{
		char* neo=new char[1024];
		if
		(
			r==1.0f &&
			g==1.0f &&
			b==1.0f &&
			a==1.0f
		)
		{
			sprintf
			(
				neo,
				"f|%.3f|%.3f|%.3f|%i|%.2f|%s\n",
				x,y,height,
				centered?1:0,
				shadowAlpha,
				tmpstr
			);
		}
		else
		{
			sprintf
			(
				neo,
				"f|%.3f|%.3f|%.3f|%.2f|%.2f|%.2f|%.2f|%i|%.2f|%s\n",
				x,y,height,
				r,g,b,a,
				centered?1:0,
				shadowAlpha,
				tmpstr
			);
		}
		LGL.DrawLog.push_back(neo);
	}

	float myA=a;
	if(myA>1) myA=1;
	float xNow=x;

	if(centered==true)
	{
		xNow-=.5f*GetWidthString(height,tmpstr);
	}

	//Okay. Let's do this efficiently using glArrays.

	unsigned int len=strlen(tmpstr);

	int vertexArraySize=2*4*len*2;	//XY * 4 Points * WordLength * Shadows
	int textureArraySize=vertexArraySize;
	int colorArraySize=vertexArraySize*2;	//XY => RGBA

	float* vertexArray=lgl_GetFontBuffer(vertexArraySize);
	float* textureArray=lgl_GetFontBuffer(textureArraySize);
	float* colorArray=lgl_GetFontBuffer(colorArraySize);

	assert(Glyph[32]);
	float glyph32h = Glyph[32]->GetHeight();
	float& alpha = a;

	bool fixedWidthOn=true;
	float fixedWidth=0;
	for(unsigned int a=0;a<len;a++)
	{
		if(isalpha(tmpstr[a]))
		{
			fixedWidthOn=false;
			break;
		}
	}
	if(fixedWidthOn)
	{
		for(unsigned int a='0';a<='9';a++)
		{
			float candidate=GetWidthChar(height,a);
			if(candidate>fixedWidth)
			{
				fixedWidth=candidate;
			}
		}
	}

	for(unsigned int a=0;a<len;a++)
	{
		unsigned char ch=tmpstr[a];
		if(QueryChar(ch)==false) ch=63;	//?
		float width=GetWidthChar(height,ch);
		float glyphTexHeight=GlyphTexHeight[ch];
		float glyphWidthOverHeight=GlyphTexWidthHeightRatio[ch];
		float shadowX = xNow+height/10.0f;
		float shadowY = y-height/10.0f;

		float yBaseActual = y - height*((glyphTexHeight - glyph32h)/glyph32h);
		float extra = y - yBaseActual;

		float shadowYBaseActual = shadowY - height*((glyphTexHeight - glyph32h)/glyph32h);
		float shadowExtra = shadowY - shadowYBaseActual;

		float shadowLeft = shadowX;
		float shadowRight = shadowX+height*glyphWidthOverHeight;
		float shadowBottom = shadowYBaseActual;
		float shadowTop = shadowYBaseActual+height+shadowExtra;

		float left = xNow;
		float right = xNow+height*glyphWidthOverHeight;
		float bottom = yBaseActual;
		float top = yBaseActual+height+extra;

		vertexArray[(a*16)+0] = shadowLeft;		//S.LB.x
		vertexArray[(a*16)+1] = shadowBottom;		//S.LB.y
		vertexArray[(a*16)+2] = shadowLeft;		//S.LT.x
		vertexArray[(a*16)+3] = shadowTop;		//S.LT.y
		vertexArray[(a*16)+4] = shadowRight;		//S.RT.x
		vertexArray[(a*16)+5] = shadowTop;		//S.RT.y
		vertexArray[(a*16)+6] = shadowRight;		//S.RB.x
		vertexArray[(a*16)+7] = shadowBottom;		//S.RB.y

		vertexArray[(a*16)+8] = left;			//LB.x
		vertexArray[(a*16)+9] = bottom;			//LB.y
		vertexArray[(a*16)+10] = left;			//LT.x
		vertexArray[(a*16)+11] = top;			//LT.y
		vertexArray[(a*16)+12] = right;			//RT.x
		vertexArray[(a*16)+13] = top;			//RT.y
		vertexArray[(a*16)+14] = right;			//RB.x
		vertexArray[(a*16)+15] = bottom;		//RB.y

		/*
		int texLeft = GlyphTexLeft[ch];
		int texRight = GlyphTexRight[ch];
		int texBottom = GlyphTexBottom[ch];
		int texTop = GlyphTexTop[ch];
		*/

		const float safety=0.01f;
		
		float texLeft = (GlyphTexLeft[ch]+safety)/(float)TextureSideLength;
		float texRight = (GlyphTexRight[ch]-safety)/(float)TextureSideLength;
		float texBottom = (GlyphTexBottom[ch]-safety)/(float)TextureSideLength;	//Note: bottom is numerically greater than top, here.
		float texTop = (GlyphTexTop[ch]+safety)/(float)TextureSideLength;

		if(GL_TEXTURE_LGL==GL_TEXTURE_RECTANGLE_ARB)
		{
			texLeft = (GlyphTexLeft[ch]+safety);
			texRight = (GlyphTexRight[ch]-safety);
			texBottom = (GlyphTexBottom[ch]-safety);	//Note: bottom is numerically greater than top, here.
			texTop = (GlyphTexTop[ch]+safety);
		}

		textureArray[(a*16)+0] = texLeft;	//LB.x
		textureArray[(a*16)+1] = texBottom;	//LB.y
		textureArray[(a*16)+2] = texLeft;	//LT.x
		textureArray[(a*16)+3] = texTop;	//LT.y
		textureArray[(a*16)+4] = texRight;	//RT.x
		textureArray[(a*16)+5] = texTop;	//RT.y
		textureArray[(a*16)+6] = texRight;	//RB.x
		textureArray[(a*16)+7] = texBottom;	//RB.y

		textureArray[(a*16)+8] = texLeft;	//LB.x
		textureArray[(a*16)+9] = texBottom;	//LB.y
		textureArray[(a*16)+10] = texLeft;	//LT.x
		textureArray[(a*16)+11] = texTop;	//LT.y
		textureArray[(a*16)+12] = texRight;	//RT.x
		textureArray[(a*16)+13] = texTop;	//RT.y
		textureArray[(a*16)+14] = texRight;	//RB.x
		textureArray[(a*16)+15] = texBottom;	//RB.y

		for(int z=0;z<2;z++)
		{
			for(int p=0;p<4;p++)
			{
				colorArray[(a*32)+(z*16)+(p*4)+0] = z*r;
				colorArray[(a*32)+(z*16)+(p*4)+1] = z*g;
				colorArray[(a*32)+(z*16)+(p*4)+2] = z*b;
				colorArray[(a*32)+(z*16)+(p*4)+3] = alpha*(z+(1-z)*shadowAlpha);
			}
		}

		xNow+=fixedWidthOn ? ((fixedWidth>width) ? fixedWidth : width) : width;
	}

	glDisable(GL_POLYGON_SMOOTH);

	//glMatrixMode(GL_TEXTURE);
	//glPushMatrix();
	//glLoadIdentity();
	//glScalef(1.0f/TextureSideLength, 1.0f/TextureSideLength, 1);

	LGL_Shader* shader=&LGL_Image::ImageShader;
	shader->Disable();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_LGL);
	glBindTexture(GL_TEXTURE_LGL,TextureGL);

	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glNormal3f(0,0,-1);
	glColor4f(1.0f,1.0f,1.0f,1.0f);

	glVertexPointer
	(
		2,
		GL_FLOAT,
		0,
		vertexArray
	);
	glTexCoordPointer
	(
		2,
		GL_FLOAT,
		0,
		textureArray
	);
	glColorPointer
	(
		4,
		GL_FLOAT,
		0,
		colorArray
	);

	glDrawArrays
	(
		GL_QUADS,
		0,
		len*8
	);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glDisable(GL_TEXTURE_LGL);

	//glPopMatrix();

	glEnable(GL_POLYGON_SMOOTH);

	return(xNow);
}

bool
LGL_Font::
QueryChar
(
	char	in
)
{
	if(in<0)
	{
		return(false);
	}

	if(Glyph[(unsigned int)in]!=NULL)
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

const
char*
LGL_Font::
GetPath()	const
{
	return(Path);
}

const
char*
LGL_Font::
GetPathShort()	const
{
	return(PathShort);
}

int
LGL_Font::
GetReferenceCount()
{
	return(ReferenceCount);
}

void
LGL_Font::
IncrementReferenceCount()
{
	ReferenceCount++;
}

void
LGL_Font::
DecrementReferenceCount()
{
	ReferenceCount--;
	assert(ReferenceCount>=0);
}

void
LGL_Font::
PrintMissingGlyphs
(
	const
	char*	string,
	...
)
{
return;	//Actually, don't do this...

	LGL_Assert(string!=NULL);

	//Process the formatted part of the string
	char tmpstr[1024];
	va_list args;
	va_start(args,string);
	vsprintf(tmpstr,string,args);
	va_end(args);

	unsigned int len=strlen(tmpstr);

	for(unsigned int a=0;a<len;a++)
	{
		if(tmpstr[a]<0)
		{
			printf("LGL_Font::PrintMissingGlyphs('%s'): '%c' (ASCII %i) IS LESS THAN ZERO.\n",tmpstr,tmpstr[a],tmpstr[a]);
		}
		else if(Glyph[(int)tmpstr[a]]==NULL)
		{
			printf("LGL_Font::PrintMissingGlyphs('%s'): '%c' (ASCII %i)\n",tmpstr,tmpstr[a],tmpstr[a]);
		}
	}
}

int
LGL_Font::
GetHeightPixels()
{
	return(Glyph[32]->GetHeight());
}

float
LGL_Font::
GetWidthChar
(
	float	height,
	char	in
)
{
	if(in<0)
	{
		//printf("LGL_Font::GetWidthChar(): Warning! Character '%c' is < 0 (%i). WTF.\n",in,in);
		return(GetWidthChar(height,' '));
	}

	if(Glyph[(unsigned int)in]==NULL)
	{
		return(GetWidthChar(height,' '));
	}
	if(QueryChar(in)==false)
	{
		//printf("LGL_Font::GetWidthChar(): Font doesn't support '%c'\n",in);
		return(0);
	}
	return
	(
		height*(float)Glyph[(unsigned int)in]->GetWidth()/(float)Glyph[(unsigned int)in]->GetHeight() +
		height*(float)1/(float)Glyph[(unsigned int)in]->GetHeight()
	);
}

float
LGL_Font::
GetWidthString
(
	float	height,
const	char*	in
)
{
	float xNow=0;

	bool fixedWidthOn=true;
	float fixedWidth=0;
	unsigned int len=strlen(in);
	for(unsigned int a=0;a<len;a++)
	{
		if(isalpha(in[a]))
		{
			fixedWidthOn=false;
			break;
		}
	}
	if(fixedWidthOn)
	{
		for(unsigned int a='0';a<='9';a++)
		{
			float candidate=GetWidthChar(height,a);
			if(candidate>fixedWidth)
			{
				fixedWidth=candidate;
			}
		}
	}

	for(unsigned int i=0;i<strlen(in);i++)
	{
		xNow+=fixedWidthOn ?
			fixedWidth :
			GetWidthChar
			(
				height,
				in[i]
			);
	}

	return(xNow);
}

LGL_Font&
LGL_GetFont()
{
	if(LGL.Font==NULL)
	{
		char fontDir[1024];
		sprintf(fontDir,"data/font/default");
		if(!LGL_DirectoryExists(fontDir))
		{
			sprintf(fontDir,"/usr/local/share/lgl/font");
			if(!LGL_DirectoryExists(fontDir))
			{
				printf("LGL_GetFont(): Error! A font directory must live at 'data/font/default'\n");
				LGL_Exit();
			}
		}
		LGL.Font=new LGL_Font(fontDir);
	}
	return(*(LGL.Font));
}

void
LGL_DebugPrintf
(
	const char*	string
	...
)
{
	{
		LGL_ScopeLock lock(__FILE__,__LINE__,LGL.DebugPrintfSemaphore,
			(SDL_ThreadID()==LGL.ThreadIDMain) ? 0.0f : -1.0f
		);
		if(lock.GetLockObtained()==false)
		{
			return;
		}
		if(LGL.DebugPrintfBuffer.size()>32)
		{
			return;
		}
	}

	//Process the formatted part of the string
	char tmpstr[2048];
	va_list args;
	va_start(args,string);
	vsprintf(tmpstr,string,args);
	va_end(args);
	
	if(char* newline=strchr(tmpstr,'\n'))
	{
		newline[0]='\0';
	}

	char* target = new char[strlen(tmpstr)+2];
	strcpy(target,tmpstr);
	{
		LGL_ScopeLock lock(__FILE__,__LINE__,LGL.DebugPrintfSemaphore,
			(SDL_ThreadID()==LGL.ThreadIDMain) ? 0.0f : -1.0f
		);
		if(lock.GetLockObtained()==false)
		{
			delete target;
			return;
		}
		else
		{
			LGL.DebugPrintfBuffer.push_back(target);
		}
	}
}

void
lgl_DebugPrintfInternal
(
	const char*	string
)
{
	if(lgl_lgl_initialized==false)
	{
		return;
	}

	const float height=0.015f;
	LGL.DebugPrintfY-=height*2.0f;

	LGL_GetFont().DrawString
	(
		0.025f,
		LGL.DebugPrintfY,
		height,
		1.0f,1.0f,1.0f,1.0f,
		false,
		1.0f,
		string
	);
}

LGL_InputBuffer::
LGL_InputBuffer()
{
	ClearBuffer();
	Focus=false;
	Accept=0;
}

LGL_InputBuffer::
~LGL_InputBuffer()
{
	ReleaseFocus();
}

void
LGL_InputBuffer::
ClearBuffer()
{
	bzero(Buffer,1024);
}

void
LGL_InputBuffer::
GrabFocus()
{
	if(LGL.InputBufferFocus!=NULL)
	{
		LGL.InputBufferFocus->ReleaseFocus();
	}
	LGL.InputBufferFocus=this;
	Focus=true;
}

void
LGL_InputBuffer::
ReleaseFocus()
{
	if(LGL.InputBufferFocus==this)
	{
		LGL.InputBufferFocus=NULL;
	}
	Focus=false;
}

bool
LGL_InputBuffer::
HasFocus()	const
{
	return(Focus);
}

void
LGL_InputBuffer::
AcceptString()
{
	ClearBuffer();
	Accept=0;
}

void
LGL_InputBuffer::
AcceptInt()
{
	ClearBuffer();
	Accept=1;
}

void
LGL_InputBuffer::
AcceptFloat()
{
	ClearBuffer();
	Accept=2;
}

void
LGL_InputBuffer::
AcceptTime()
{
	ClearBuffer();
	Accept=3;
}

const char*
LGL_InputBuffer::
GetString()
{
	return(Buffer);
}

int
LGL_InputBuffer::
GetInt()
{
	if(Accept==3)
	{
		printf("LGL_InputBuffer::GetInt(): Error! Cannot call GetInt() after calling AcceptTime()...\n");
		LGL_Exit();
	}
	return(atoi(Buffer));
}

float
LGL_InputBuffer::
GetFloat()
{
	if(Accept==3)
	{
		printf("LGL_InputBuffer::GetFloat(): Error! Cannot call GetFloat() after calling AcceptTime()...\n");
		LGL_Exit();
	}
	return(atof(Buffer));
}

int
LGL_InputBuffer::
GetHoursComponent()
{
	if(Accept!=3)
	{
		printf("LGL_InputBuffer::GetHoursComponent() Error! You must call AcceptTime() before calling GetHours()...\n");
		LGL_Exit();
	}
	float seconds=GetSecondsTotal();
	return((int)floor(seconds/(60.0*60.0)));
}

int
LGL_InputBuffer::
GetMinutesComponent()
{
	if(Accept!=3)
	{
		printf("LGL_InputBuffer::GetMinutesComponent() Error! You must call AcceptTime() before calling GetHours()...\n");
		LGL_Exit();
	}
	float seconds=GetSecondsTotal();
	return(((int)floor(seconds/(60.0)))%60);
}

float
LGL_InputBuffer::
GetSecondsComponent()
{
	if(Accept!=3)
	{
		printf("LGL_InputBuffer::GetSecondsComponent() Error! You must call AcceptTime() before calling GetHours()...\n");
		LGL_Exit();
	}
	float seconds=GetSecondsTotal();
	return(((int)floor(seconds/(60.0)))%60);
}

float
LGL_InputBuffer::
GetSecondsTotal()
{
	char temp[1024];
	int hours=0;
	int minutes=0;
	float seconds=0;
	
	sprintf(temp,"%s",Buffer);
	char* h=strstr(temp,"h");
	char* next=temp;
	if(h!=NULL)
	{
		h[0]='\0';
		hours=atoi(next);
		h[0]='h';
		next=&(h[1]);
	}
	char* m=strstr(next,"m");
	if(m!=NULL)
	{
		m[0]='\0';
		minutes=atoi(next);
		m[0]='m';
		next=&(m[1]);
	}
	char* s=strstr(next,"s");
	if(s!=NULL)
	{
		s[0]='\0';
	}
	seconds=atof(next);
	
	return(hours*60*60 + minutes*60 + seconds);
}

void
LGL_InputBuffer::
SetString
(
	const
	char*	in
)
{
	if(in==NULL)
	{
		Buffer[0]='\0';
	}
	else
	{
		sprintf(Buffer,"%s",in);
	}
}

void
LGL_InputBuffer::
SetInt
(
	int	in
)
{
	sprintf(Buffer,"%i",in);
}

void
LGL_InputBuffer::
SetFloat
(
	float	in
)
{
	sprintf(Buffer,"%f",in);
	for(;;)
	{
		int end=strlen(Buffer)-1;
		if(Buffer[end]=='0')
		{
			Buffer[end]='\0';
		}
		else if(Buffer[end]=='.')
		{
			Buffer[end+1]='0';
			Buffer[end+2]='\0';
			break;
		}
		else
		{
			break;
		}
	}
}

void
LGL_InputBuffer::
SetTime
(
	int	hours,
	int	minutes,
	float	seconds
)
{
	if(hours!=0)
	{
		sprintf(Buffer,"%ih%im%.2fs",hours,minutes,seconds);
	}
	else if(minutes!=0)
	{
		sprintf(Buffer,"%im%.2fs",minutes,seconds);
	}
	else
	{
		sprintf(Buffer,"%.2fs",seconds);
	}
}

void
LGL_InputBuffer::
LGL_INTERNAL_ProcessInput()
{
	for(unsigned int a=0;a<strlen(LGL_KeyStream());a++)
	{
		int length=strlen(Buffer);
		if
		(
			(
				LGL_KeyStream()[a]=='\b' ||
				LGL_KeyStream()[a]==127	//OSX Delete
			) &&
			length>0
		)
		{
			Buffer[length-1]='\0';
		}
		else if(LGL_KeyStream()[a]=='\n' || LGL_KeyStream()[a]=='\r')
		{
			//
		}
		else if(LGL_GetFont().QueryChar(LGL_KeyStream()[a]) && length<1023)
		{
			if
			(
				(Accept==0) ||
				(
					Accept==1 &&
					(
						isdigit(LGL_KeyStream()[a]) ||
						(
							LGL_KeyStream()[a]=='-' &&
							Buffer[0]=='\0'
						)
					)
				) ||
				(
					Accept==2 &&
					(
						isdigit(LGL_KeyStream()[a]) ||
						(
							LGL_KeyStream()[a]=='.' &&
							strstr(Buffer,".")==NULL
						) ||
						(
							LGL_KeyStream()[a]=='-' &&
							Buffer[0]=='\0'
						)
					)
				) ||
				(
					Accept==3 &&
					(
						(
							isdigit(LGL_KeyStream()[a]) &&
							strstr(Buffer,"s")==NULL
						) ||
						(
							LGL_KeyStream()[a]=='h' &&
							strstr(Buffer,"h")==NULL &&
							strstr(Buffer,"m")==NULL &&
							strstr(Buffer,"s")==NULL &&
							strstr(Buffer,".")==NULL
						) ||
						(
							LGL_KeyStream()[a]=='m' &&
							strstr(Buffer,"m")==NULL &&
							strstr(Buffer,"s")==NULL &&
							strstr(Buffer,".")==NULL &&
							(
								length==0 ||
								Buffer[length-1]!='h'
							)
						) ||
						(
							LGL_KeyStream()[a]=='s' &&
							strstr(Buffer,"s")==NULL &&
							strstr(Buffer,".")==NULL &&
							(
								length==0 ||
								(
									Buffer[length-1]!='h' &&
									Buffer[length-1]!='m'
								)
							)
						) ||
						(
							LGL_KeyStream()[a]=='.' &&
							strstr(Buffer,".")==NULL &&
							(
								length==0 ||
								(
									Buffer[length-1]!='h' &&
									Buffer[length-1]!='m' &&
									Buffer[length-1]!='s'
								)
							)
						)
					)
				)
			)
			{
				Buffer[length]=LGL_KeyStream()[a];
				Buffer[length+1]='\0';
			}
		}
		else if
		(
			LGL_GetFont().QueryChar(LGL_KeyStream()[a])==false &&
			LGL_KeyStream()[a]!=27 &&	//ESC
			LGL_KeyStream()[a]!=9 &&	//Tab
			LGL_KeyStream()[a]!=127 &&	//Backspace
			LGL_KeyStream()[a] >= 0 &&
			(
				Buffer[0]!='\0' ||
				LGL_KeyStream()[a]!=8	//Backspace at end
			)
		)
		{
			printf("LGL_InputBuffer::ProcessInput(): Warning! User-entered character '%c' dropped, as it's not in LGL_Font... (%i)\n",LGL_KeyStream()[a],LGL_KeyStream()[a]);
		}
	}
}

//Audio2

LGL_AudioStream::
LGL_AudioStream()
{
	DefaultSineCounter=0;
}

LGL_AudioStream::
~LGL_AudioStream()
{
	//
}

void
LGL_AudioStream::
Update()
{
	//
}

bool
LGL_AudioStream::
Finished()	const
{
	return(false);
}

int
LGL_AudioStream::
MixIntoStream
(
	void*	userdata,
	Uint8*	stream,
	int	len
)
{
	//Default sine wave

	Sint16* stream16=(Sint16*)stream;

	int len16=len/2;

	if(LGL_AudioChannels()==2)
	{
		for(int a=0;a<len16;a++)
		{
			stream16[a]+=SWAP16((Sint16)(0.01f*32767*sinf(DefaultSineCounter/20.0f)));
			DefaultSineCounter++;
		}

		return(len16/2);
	}
	else if(LGL_AudioChannels()==4)
	{
		for(int a=0;a<len16/2;a++)
		{
			//Front
			stream16[2*a+0]+=SWAP16((Sint16)(0.01f*32767*sinf(DefaultSineCounter/20.0f)));
			//Back
			stream16[2*a+1]+=SWAP16((Sint16)(0.01f*32767*sinf(DefaultSineCounter/10.0f)));
			DefaultSineCounter++;
		}
		return(len16/4);
	}
	else
	{
		LGL_Assert(LGL_AudioChannels()==2 || LGL_AudioChannels()==4);
		return(0);
	}
}

void
LGL_AddAudioStream
(
	LGL_AudioStream*	stream
)
{
	assert(false);
	{
		LGL_ScopeLock lock(__FILE__,__LINE__,LGL.AudioStreamListSemaphore);
		LGL.AudioStreamList.push_back(stream);
	}
}

LGL_AudioDSP::
LGL_AudioDSP() : FreqResponseNextSemaphore("AudioDSP FreqResponseNext")
{
	for(int a=0;a<2048;a++)
	{
		CarryOverLeft[a]=0;
		CarryOverRight[a]=0;
	}

	float initialFreqResponse[513];
	for(int a=0;a<=512;a++)
	{
		initialFreqResponse[a]=1.0f;
	}
	SetFreqResponse(initialFreqResponse);

	FreqResponseNextAvailable=false;
}

LGL_AudioDSP::
~LGL_AudioDSP()
{
	//
}

void
LGL_AudioDSP::
ProcessLeft
(
	const
	float*		input,
	float*		output,
	unsigned long	samples
)
{
	if(samples==0)
	{
		return;
	}
	ProcessChannel
	(
		input,
		output,
		CarryOverLeft,
		samples
	);
}

void
LGL_AudioDSP::
ProcessRight
(
	const
	float*		input,
	float*		output,
	unsigned long	samples
)
{
	if(samples==0)
	{
		return;
	}
	ProcessChannel
	(
		input,
		output,
		CarryOverRight,
		samples
	);
}

float inputL[LGL_SAMPLESIZE];
float inputR[LGL_SAMPLESIZE];
float outputL[LGL_SAMPLESIZE];
float outputR[LGL_SAMPLESIZE];

void
LGL_AudioDSP::
ProcessStereo
(
	const
	float*		input,
	float*		output,
	unsigned long	samples
)
{
	assert(samples==LGL_SAMPLESIZE);
	if(samples/2==0)
	{
		return;
	}

	for(unsigned long a=0;a<samples*2;a+=2)
	{
		inputL[a/2]=input[a];
		inputR[a/2]=input[a+1];
	}

#if 1
	ProcessLeft(inputL,outputL,samples);
	ProcessRight(inputR,outputR,samples);
#else
	ProcessChannelStereo
	(
		inputL,
		inputR,
		outputL,
		outputR,
		CarryOverLeft,
		CarryOverRight,
		samples
	);
#endif

	for(unsigned long a=0;a<samples*2;a+=2)
	{
		output[a]=outputL[a/2];
		output[a+1]=outputR[a/2];
	}
}

void
LGL_AudioDSP::
SetFreqResponse
(
	float*	freqResponseArrayOf513
)
{
	//Validate freqResponseArrayOf513
	float helperArray[513];
	if(freqResponseArrayOf513==NULL)
	{
		//Reset EQ
		for(int a=0;a<513;a++)
		{
			helperArray[a]=1.0f;
		}
		freqResponseArrayOf513=helperArray;
	}

	//Convert our Polar freqResponseArrayOf513 to Rectangular
	unsigned int samplesMaxFFT = LGL_EQ_SAMPLES_FFT;
	const unsigned int samplesMaxFFTHalf=samplesMaxFFT/2;
	const unsigned int lgSamplesMaxFFT=(unsigned int)(logf(samplesMaxFFT)/logf(2.0f));

	float freqResponseDesiredReal[samplesMaxFFT];
	float freqResponseDesiredImaginary[samplesMaxFFT];
	bzero(freqResponseDesiredReal,samplesMaxFFT*sizeof(float));
	bzero(freqResponseDesiredImaginary,samplesMaxFFT*sizeof(float));
	for(unsigned int a=0;a<513;a++)
	{
		freqResponseDesiredReal[(int)(a*(samplesMaxFFTHalf/512.0f))]+=freqResponseArrayOf513[a];
		freqResponseDesiredImaginary[(int)(a*(samplesMaxFFTHalf/512.0f))]+=freqResponseArrayOf513[a];
	}
	for(unsigned int a=0;a<=samplesMaxFFTHalf;a++)
	{
		freqResponseDesiredReal[a]*=samplesMaxFFTHalf/512.0f;
		freqResponseDesiredImaginary[a]*=samplesMaxFFTHalf/512.0f;
	}
	/*
	for(unsigned int a=0;a<=samplesMaxFFTHalf;a++)
	{
		freqResponseDesiredReal[a]=freqResponseArrayOf513[a];
		freqResponseDesiredImaginary[a]=freqResponseArrayOf513[a];
	}
	*/

	//Enforce symmetry for negative frequencies
	for(unsigned int a=1;a<=samplesMaxFFTHalf;a++)
	{
		freqResponseDesiredReal[samplesMaxFFTHalf+a]=freqResponseDesiredReal[samplesMaxFFTHalf-a];
		freqResponseDesiredImaginary[samplesMaxFFTHalf+a]=-freqResponseDesiredImaginary[samplesMaxFFTHalf-a];
	}

	//FFT to the time domain
	float impulseResponseAliasedReal[samplesMaxFFT];
	float impulseResponseAliasedImaginary[samplesMaxFFT];
	memcpy(impulseResponseAliasedReal,freqResponseDesiredReal,samplesMaxFFT*sizeof(float));
	memcpy(impulseResponseAliasedImaginary,freqResponseDesiredImaginary,samplesMaxFFT*sizeof(float));
	LGL_FFT
	(
		impulseResponseAliasedReal,
		impulseResponseAliasedImaginary,
		lgSamplesMaxFFT,
		-1
	);
	float impulseResponseReal[samplesMaxFFT];
	float impulseResponseImaginary[samplesMaxFFT];
	memcpy(impulseResponseReal,impulseResponseAliasedReal,samplesMaxFFT*sizeof(float));
	memcpy(impulseResponseImaginary,impulseResponseAliasedImaginary,samplesMaxFFT*sizeof(float));

	//Right-Shift by kernelSamplesHalf, apply Blackman window, add trailing zeros
	const unsigned int kernelSamples=samplesMaxFFTHalf;
	const unsigned int kernelSamplesHalf=kernelSamples/2;
	float temp[samplesMaxFFT];
	for(unsigned int a=0;a<samplesMaxFFT;a++)
	{
		int index=a-kernelSamplesHalf;
		if(index<0)
		{
			index+=samplesMaxFFT;
		}
		temp[a]=impulseResponseReal[index]*
			(
				(a<kernelSamples) ?
				(0.42f - 0.5f*cosf(2*LGL_PI*a/(float)(kernelSamples-1)) + .08f*cosf(4*LGL_PI*a/(float)(kernelSamples-1))) :
				0
			);
	}
	memcpy(impulseResponseReal,temp,samplesMaxFFT*sizeof(float));
	for(unsigned int a=0;a<samplesMaxFFT;a++)
	{
		impulseResponseImaginary[a]=0;
	}

	//FFT back to freq domain
	float freqResponseActualReal[samplesMaxFFT];
	float freqResponseActualImaginary[samplesMaxFFT];
	memcpy(freqResponseActualReal,impulseResponseReal,samplesMaxFFT*sizeof(float));
	memcpy(freqResponseActualImaginary,impulseResponseImaginary,samplesMaxFFT*sizeof(float));

	LGL_FFT
	(
		freqResponseActualReal,
		freqResponseActualImaginary,
		lgSamplesMaxFFT,
		1
	);

	{
		LGL_ScopeLock lock(__FILE__,__LINE__,FreqResponseNextSemaphore);
		memcpy(FreqResponseNextReal,freqResponseActualReal,1024*sizeof(float));
		memcpy(FreqResponseNextImaginary,freqResponseActualImaginary,1024*sizeof(float));
		FreqResponseNextAvailable=true;
	}
}

void
LGL_AudioDSP::
ProcessChannel
(
	const
	float*		userInput,
	float*		userOutput,
	float*		carryOver,
	unsigned long	samples
)
{
	if(FreqResponseNextAvailable)
	{
		{
			memcpy(FreqResponseReal,FreqResponseNextReal,1024*sizeof(float));
			memcpy(FreqResponseImaginary,FreqResponseNextImaginary,1024*sizeof(float));
			FreqResponseNextAvailable=false;
		}
	}

	const unsigned int samplesMaxFFT = LGL_EQ_SAMPLES_FFT;
	const unsigned int samplesMaxFFTHalf=samplesMaxFFT/2;
	const unsigned int lgSamplesMaxFFT=(unsigned int)(logf(samplesMaxFFT)/logf(2.0f));

	//Prepare our filter kernel (The EQ filter multiplied by userInput in the frequency domain)

	//Pick our desired frequency array
	float filterKernelReal[samplesMaxFFT];
	float filterKernelImaginary[samplesMaxFFT];
	memcpy(filterKernelReal,FreqResponseReal,samplesMaxFFT*sizeof(float));
	memcpy(filterKernelImaginary,FreqResponseImaginary,samplesMaxFFT*sizeof(float));

	for(unsigned long sampleStart=0;sampleStart<samples;sampleStart+=samplesMaxFFTHalf)
	{
		//Combine EQ'd userInput with carryOver, output results to userOutput

		const unsigned int sampleCount=(unsigned int)LGL_Min(samples-sampleStart,samplesMaxFFTHalf);

		float fftInputReal[samplesMaxFFT];
		float fftInputImaginary[samplesMaxFFT];

		for(unsigned int a=0;a<sampleCount;a++)
		{
			fftInputReal[a]=userInput[sampleStart+a];
			fftInputImaginary[a]=0;
		}
		for(unsigned int a=sampleCount;a<samplesMaxFFT;a++)
		{
			fftInputReal[a]=0;
			fftInputImaginary[a]=0;
		}

		//To the frequency domain!!
		LGL_FFT
		(
			fftInputReal,
			fftInputImaginary,
			lgSamplesMaxFFT,
			1,
			1
		);

		//Apply EQ
		for(unsigned int a=0;a<samplesMaxFFT;a++)
		{
			//Multiplication is a little complex in the complex frequency domain...
			float real=
				fftInputReal[a]*filterKernelReal[a]-
				fftInputImaginary[a]*filterKernelImaginary[a];
			float imaginary=
				fftInputImaginary[a]*filterKernelReal[a]+
				fftInputReal[a]*filterKernelImaginary[a];
			fftInputReal[a]=real;
			fftInputImaginary[a]=imaginary;
		}

		//(back) To the time domain!!
		LGL_FFT
		(
			fftInputReal,
			fftInputImaginary,
			lgSamplesMaxFFT,
			-1,
			1
		);

		//Determine DC so we may cancel it out
		float dc=fftInputReal[0];

		//Write to userOutput
		for(unsigned int a=0;a<sampleCount;a++)
		{
			userOutput[sampleStart+a]=
			(
				fftInputReal[a]+
				carryOver[a]-
				dc
			);
		}

		//Write to carryOver
		float carryOverNew[2048];
		for(unsigned int a=0;a<2048;a++)
		{
			carryOverNew[a]=
				(
					(a+sampleCount<2048) ?
					carryOver[a+sampleCount] :
					0
				) +
				(
					(a+sampleCount<samplesMaxFFT) ?
					fftInputReal[a+sampleCount]-dc :
					0
				);
		}
		memcpy(carryOver,carryOverNew,2048*sizeof(float));
	}
}

void
LGL_AudioDSP::
ProcessChannelStereo
(
	const
	float*		userInputL,
	float*		userInputR,
	float*		userOutputL,
	float*		userOutputR,
	float*		carryOverL,
	float*		carryOverR,
	unsigned long	samples
)
{
	if(FreqResponseNextAvailable)
	{
		{
			LGL_ScopeLock lock(__FILE__,__LINE__,FreqResponseNextSemaphore);
			memcpy(FreqResponseReal,FreqResponseNextReal,1024*sizeof(float));
			memcpy(FreqResponseImaginary,FreqResponseNextImaginary,1024*sizeof(float));
			FreqResponseNextAvailable=false;
		}
	}

	const unsigned int samplesMaxFFT = LGL_EQ_SAMPLES_FFT;
	const unsigned int samplesMaxFFTHalf=samplesMaxFFT/2;
	const unsigned int lgSamplesMaxFFT=(unsigned int)(logf(samplesMaxFFT)/logf(2.0f));

	//Prepare our filter kernel (The EQ filter multiplied by userInput in the frequency domain)

	//Pick our desired frequency array
	float filterKernelReal[samplesMaxFFT];
	float filterKernelImaginary[samplesMaxFFT];
	memcpy(filterKernelReal,FreqResponseReal,samplesMaxFFT*sizeof(float));
	memcpy(filterKernelImaginary,FreqResponseImaginary,samplesMaxFFT*sizeof(float));

	for(unsigned long sampleStart=0;sampleStart<samples;sampleStart+=samplesMaxFFTHalf)
	{
		//Combine EQ'd userInput with carryOver, output results to userOutput

		unsigned long sampleCount=(unsigned long)LGL_Min(samples-sampleStart,samplesMaxFFTHalf);

		float fftInputReal[samplesMaxFFT];
		float fftInputImaginary[samplesMaxFFT];

		for(unsigned int a=0;a<sampleCount;a++)
		{
			fftInputReal[a]=userInputL[sampleStart+a];
			fftInputImaginary[a]=userInputR[sampleStart+a];
		}
		for(unsigned int a=sampleCount;a<samplesMaxFFT;a++)
		{
			fftInputReal[a]=0;
			fftInputImaginary[a]=0;
		}

		//To the frequency domain!!
		LGL_FFT
		(
			fftInputReal,
			fftInputImaginary,
			lgSamplesMaxFFT,
			1,
			1
		);

		//Apply EQ
		for(unsigned int a=0;a<=samplesMaxFFT;a++)
		{
			//Multiplication is a little complex in the complex frequency domain...
			float real=
				fftInputReal[a]*filterKernelReal[a]-
				fftInputImaginary[a]*filterKernelImaginary[a];
			float imaginary=
				(fftInputImaginary[a]-fftInputImaginary[samplesMaxFFT-1-a])*filterKernelReal[a]+
				fftInputReal[a]*filterKernelImaginary[a];
			fftInputReal[a]=real;
			fftInputImaginary[a]=imaginary;
		}

		//(back) To the time domain!!
		LGL_FFT
		(
			fftInputReal,
			fftInputImaginary,
			lgSamplesMaxFFT,
			-1,
			1
		);

		//Determine DC so we may cancel it out
		float dcL=fftInputReal[0];
		float dcR=fftInputImaginary[0];

		//Write to userOutput
		for(unsigned int a=0;a<sampleCount;a++)
		{
			userOutputL[sampleStart+a]=
			(
				fftInputReal[a]+
				carryOverL[a]-
				dcL
			);
			userOutputR[sampleStart+a]=
			(
				fftInputImaginary[a]+
				carryOverR[a]-
				dcR
			);
		}

		//Write to carryOver
		for(int a=0;a<2;a++)
		{
			float carryOverNew[2048];
			float* carryOverOld = (a==0) ? carryOverL : carryOverR;
			float dc = (a==0) ? dcL : dcR;
			for(unsigned int a=0;a<2048;a++)
			{
				carryOverNew[a]=
					(
						(a+sampleCount<2048) ?
						carryOverOld[a+sampleCount] :
						0
					) +
					(
						(a+sampleCount<samplesMaxFFT) ?
						fftInputReal[a+sampleCount]-dc :
						0
					);
			}
			memcpy(carryOverOld,carryOverNew,2048*sizeof(float));
		}
	}
}

LGL_AudioGrain::
LGL_AudioGrain()
{
	Waveform=NULL;
	WaveformLeftFloat=NULL;
	WaveformRightFloat=NULL;
	WaveformMonoFloat=NULL;
	SpectrumLeft=NULL;
	SpectrumRight=NULL;
	SpectrumMono=NULL;
	SpectrumMipMaps=NULL;
	SpectrumMipMapsError=NULL;
	VolAve=0.0f;
	VolMax=0.0f;
	FreqFactor=0.0f;
	Gain=1.0f;
	FreqEQBalance=0.5f;

	LengthSamples=0;
	SpectrumSamples=512;
	StartDelaySamples=0;
	CurrentPositionSamplesInt=0;
	CurrentPositionSamplesFloat=0.0f;
	PlaybackPercent=0;
	VolumeFrontLeft=1.0f;
	VolumeFrontRight=1.0f;
	VolumeBackLeft=1.0f;
	VolumeBackRight=1.0f;
	PitchAlpha=1.0f;
	PitchOmega=1.0f;
	EnvelopeType=LGL_ENVELOPE_FULL;
	Silent=0;
}

LGL_AudioGrain::
~LGL_AudioGrain()
{
	NullifyWaveformAndSpectrum();
}

void
LGL_AudioGrain::
DrawWaveform
(
	float	left,	float	right,
	float	bottom,	float	top,
	float	r,	float	g,	float	b,	float	a,
	float	thickness,
	bool	antialias
)
{
	CalculateWaveformDerivatives();

	//Prepare arguments
	float width=right-left;
	float height=top-bottom;
	float* pointsXY=new float[LengthSamples*2];
	float peak=0.0f;

	int samplesToDraw =
		LGL_Min
		(
			LengthSamples,
			256*((right-left)*1920.0f)/(LGL_DisplayResolutionX())
		);
	for(int z=0;z<samplesToDraw;z++)
	{
		pointsXY[2*z+0] = left+width*(z/(float)(samplesToDraw-1));
		pointsXY[2*z+1] = bottom+height*WaveformMonoFloat[z];
		if(pointsXY[2*z+1]>peak)
		{
			peak=pointsXY[2*z+1];
		}
	}

	//Draw
	LGL_DrawLineStripToScreen
	(
		pointsXY,
		samplesToDraw,
		r,g,b,a,
		thickness,
		antialias
	);

	//Cleanup
	delete pointsXY;
}

void
LGL_AudioGrain::
DrawSpectrum
(
	float	left,	float	right,
	float	bottom,	float	top
)
{
	CalculateSpectrum();

	float width=right-left;
	float height=top-bottom;
	float* pointsXY=new float[SpectrumSamples*2];
	for(int z=0;z<SpectrumSamples;z++)
	{
		pointsXY[2*z+0] = left+width*(z/(0.25f*SpectrumSamples));
		pointsXY[2*z+1] = LGL_Min(top,bottom+height*SpectrumMono[z]);
	}

	//Draw
	LGL_DrawLineStripToScreen
	(
		pointsXY,
		SpectrumSamples/4,
		1,1,1,1,
		1,
		true
	);

	//Cleanup
	delete pointsXY;
}

const
float*
LGL_AudioGrain::
GetWaveform()
{
	CalculateWaveformDerivatives();

	return(WaveformMonoFloat);
}

const
float*
LGL_AudioGrain::
GetSpectrum()
{
	CalculateSpectrum();

	return(SpectrumMono);
}

void
LGL_AudioGrain::
GetMetadata
(
	float&	volAve,
	float&	volMax,
	float&	freqFactor,
	float	gain,
	float	freqEQBalance
)
{
	CalculateWaveformDerivatives(gain,freqEQBalance);

	volAve=VolAve;
	volMax=VolMax;
	freqFactor=FreqFactor;
}

void
LGL_AudioGrain::
Update()
{
	//The LGL_AudioGrain class doesn't need to update
}

bool
LGL_AudioGrain::
IsPlaying()
{
	return(CurrentPositionSamplesInt>0);
}

bool
LGL_AudioGrain::
Finished()	const
{
	return
	(
		Waveform==NULL ||
		PlaybackPercent>=1.0f
	);
}

int
LGL_AudioGrain::
MixIntoStream
(
	void*	userdata,
	Uint8*	stream8,
	int	len
)
{
	if(Waveform==NULL)
	{
		return(0);
	}

	if(CurrentPositionSamplesInt<0)
	{
		CurrentPositionSamplesInt=0;
		CurrentPositionSamplesFloat=0.0f;
	}
	if(CurrentPositionSamplesFloat<0.0f)
	{
		CurrentPositionSamplesInt=0;
		CurrentPositionSamplesFloat=0.0f;
	}

	int	samplesMixed=0;
	int	len16=len/2;
	Sint16* stream16=(Sint16*)stream8;
	Sint16* waveform16=(Sint16*)Waveform;

	Sint16 intSampleNow;

	if(PitchAlpha==1.0f && PitchOmega==1.0f)
	{
		//No Pitchbend, so we can use Ints
		for(int a=0;a<len16;a++)
		{
			if(CurrentPositionSamplesInt>=LengthSamples)
			{
				PlaybackPercent=1.0f;
				break;
			}

			if(StartDelaySamples>0)
			{
				if((a%2)==0)
				{
					StartDelaySamples--;
				}
			}
			else
			{
				PlaybackPercent=CurrentPositionSamplesInt/(float)LengthSamples;
//printf("LengthSamples, CurrentPositionSamplesInt, Result: %li, %li, %f\n",LengthSamples,CurrentPositionSamplesInt,PlaybackPercent);
				intSampleNow=waveform16[2*CurrentPositionSamplesInt+(a%2)];

				float volumeNow=1.0f;	//EnvelopeType==LGL_ENVELOPE_FULL
				if(EnvelopeType==LGL_ENVELOPE_TRIANGLE)
				{
					if(LGL.AudioSpec->channels==2)
					{
						if(a%2==0)
						{
							volumeNow=VolumeFrontLeft*(1.0f-2.0f*fabsf(PlaybackPercent-0.5f));
						}
						else
						{
							volumeNow=VolumeFrontRight*(1.0f-2.0f*fabsf(PlaybackPercent-0.5f));
						}
					}
					else
					{
						LGL_Assert(LGL.AudioSpec->channels==4)
						
						if(a%4==0)
						{
							volumeNow=VolumeFrontLeft*(1.0f-2.0f*fabsf(PlaybackPercent-0.5f));
						}
						else if(a%4==1)
						{
							volumeNow=VolumeFrontRight*(1.0f-2.0f*fabsf(PlaybackPercent-0.5f));
						}
						else if(a%4==2)
						{
							volumeNow=VolumeBackLeft*(1.0f-2.0f*fabsf(PlaybackPercent-0.5f));
						}
						else// if(a%4==3)
						{
							volumeNow=VolumeBackRight*(1.0f-2.0f*fabsf(PlaybackPercent-0.5f));
						}
					}
				}

				stream16[a]+=SWAP16((Sint16)(volumeNow*intSampleNow));

				if((a%LGL.AudioSpec->channels)==LGL.AudioSpec->channels-1)
				{
					CurrentPositionSamplesInt++;
				}
			}

			if(a%LGL_AudioChannels()==LGL_AudioChannels()-1)
			{
				samplesMixed++;
			}
		}
	}
	else
	{
		//Pitchbend, so we must use floats
		Sint16 intSamplePrev;
		Sint16 intSampleNext;
		for(int a=0;a<len16;a++)
		{
			if(CurrentPositionSamplesFloat>=LengthSamples)
			{
				PlaybackPercent=1.0f;
				break;
			}

			if(StartDelaySamples>0)
			{
				StartDelaySamples--;
			}
			else
			{
				PlaybackPercent=1.0f-(LengthSamples-CurrentPositionSamplesFloat)/(float)LengthSamples;
				float volumeNow;
				if(LGL.AudioSpec->channels==2)
				{
					if(a%2==0)
					{
						volumeNow=VolumeFrontLeft*(1.0f-2.0f*fabsf(PlaybackPercent-0.5f));	//Triangle envelope
					}
					else
					{
						volumeNow=VolumeFrontRight*(1.0f-2.0f*fabsf(PlaybackPercent-0.5f));	//Triangle envelope
					}
				}
				else
				{
					LGL_Assert(LGL.AudioSpec->channels==4)
					
					if(a%4==0)
					{
						volumeNow=VolumeFrontLeft*(1.0f-2.0f*fabsf(PlaybackPercent-0.5f));	//Triangle envelope
					}
					else if(a%4==1)
					{
						volumeNow=VolumeFrontRight*(1.0f-2.0f*fabsf(PlaybackPercent-0.5f));	//Triangle envelope
					}
					else if(a%4==2)
					{
						volumeNow=VolumeBackLeft*(1.0f-2.0f*fabsf(PlaybackPercent-0.5f));	//Triangle envelope
					}
					else// if(a%4==3)
					{
						volumeNow=VolumeBackRight*(1.0f-2.0f*fabsf(PlaybackPercent-0.5f));	//Triangle envelope
					}
				}

				intSamplePrev=waveform16[2*((unsigned long)floor(CurrentPositionSamplesFloat))+(a%2)];
				intSampleNext=waveform16[2*((unsigned long)ceil(CurrentPositionSamplesFloat))+(a%2)];
				float delta=CurrentPositionSamplesFloat-floor(CurrentPositionSamplesFloat);
				intSampleNow=(Sint16)
				(
					(1.0f-delta) * intSamplePrev +
					(0.0f+delta) * intSampleNext
				);

				stream16[a]+=SWAP16((Sint16)(volumeNow*intSampleNow));

				if((a%LGL.AudioSpec->channels)==LGL.AudioSpec->channels-1)
				{
					float pitch=
						(1.0f-PlaybackPercent) * PitchAlpha +
						(0.0f+PlaybackPercent) * PitchOmega;
					CurrentPositionSamplesFloat+=pitch;
				}
			}

			if(a%LGL_AudioChannels()==LGL_AudioChannels()-1)
			{
				samplesMixed++;
			}
		}
	}

	PlaybackPercent=1.0f-(LengthSamples-CurrentPositionSamplesInt)/(float)LengthSamples;

	return(samplesMixed);
}

void
LGL_AudioGrain::
GetWaveformToMemory
(
	Uint8*		buffer
)
{
	memcpy(buffer,Waveform,LengthSamples*4);
}

void
LGL_AudioGrain::
GetWaveformToMemory
(
	Sint16*	channelL,
	Sint16*	channelR
)
{
	Sint16* wave16=(Sint16*)Waveform;
	for(int a=0;a<LengthSamples*2;a++)
	{
		if((a%2)==0)
		{
			if(channelL)
			{
				channelL[a/2]=wave16[a];
			}
		}
		else
		{
			if(channelR)
			{
				channelR[a/2]=wave16[a];
			}
		}
	}
}

void
LGL_AudioGrain::
SetWaveformFromMemory
(
	Uint8*		buffer,
	long		lengthSamples
)
{
	NullifyWaveformAndSpectrum();

	if(buffer==NULL || lengthSamples<=0)
	{
		return;
	}

	Waveform=new Uint8[lengthSamples*4];
	memcpy(Waveform,buffer,lengthSamples*4);
	LengthSamples=lengthSamples;
}

void
LGL_AudioGrain::
SetWaveformFromAudioGrain
(
	LGL_AudioGrain*	grain
)
{
	assert(grain);
	NullifyWaveformAndSpectrum();

	if(grain->Waveform!=NULL)
	{
		LengthSamples=grain->GetLengthSamples();
		Waveform=new Uint8[LengthSamples*4];
		memcpy(Waveform,grain->Waveform,LengthSamples*4);
	}
	else
	{
		printf("LGL_AduioGrain::SetWaveformFromAudioGrain(): ERROR! Input grain has no Waveform!\n");
		assert(false);
	}

	if(grain->WaveformLeftFloat!=NULL)
	{
		WaveformLeftFloat=new float[LengthSamples];
		memcpy(WaveformLeftFloat,grain->WaveformLeftFloat,sizeof(float)*LengthSamples);
	}
	if(grain->WaveformRightFloat!=NULL)
	{
		WaveformRightFloat=new float[LengthSamples];
		memcpy(WaveformRightFloat,grain->WaveformRightFloat,sizeof(float)*LengthSamples);
	}
	if(grain->WaveformMonoFloat!=NULL)
	{
		WaveformMonoFloat=new float[LengthSamples];
		memcpy(WaveformMonoFloat,grain->WaveformMonoFloat,sizeof(float)*LengthSamples);
	}

	if(grain->SpectrumLeft!=NULL)
	{
		SpectrumLeft=new float[SpectrumSamples];
		memcpy(SpectrumLeft,grain->SpectrumLeft,sizeof(float)*(SpectrumSamples));
	}
	if(grain->SpectrumRight!=NULL)
	{
		SpectrumRight=new float[SpectrumSamples];
		memcpy(SpectrumRight,grain->SpectrumRight,sizeof(float)*(SpectrumSamples));
	}
	if(grain->SpectrumMono!=NULL)
	{
		SpectrumMono=new float[SpectrumSamples];
		memcpy(SpectrumMono,grain->SpectrumMono,sizeof(float)*(SpectrumSamples));
	}
	if(grain->SpectrumMipMaps!=NULL)
	{
		SpectrumMipMaps = new float*[10];
		SpectrumMipMapsError = new float*[10];
		for(int a=0;a<10;a++)
		{
			SpectrumMipMaps[a]=new float[(int)powf(2,a)];
			SpectrumMipMapsError[a]=new float[(int)powf(2,a)];
			memcpy(SpectrumMipMaps[a],grain->SpectrumMipMaps[a],sizeof(float)*(int)powf(2,a));
			memcpy(SpectrumMipMapsError[a],grain->SpectrumMipMapsError[a],sizeof(float)*(int)powf(2,a));
		}
	}
}

void
LGL_AudioGrain::
SetWaveformFromLGLSound
(
	LGL_Sound*	sound,
	float		centerSeconds,
	float		lengthSeconds,
	float		centerSecondsVariance,
	float		lengthSecondsVariance
)
{
	NullifyWaveformAndSpectrum();

	if(sound==NULL || lengthSeconds<=0)
	{
		printf("LGL_AudioGrain::SetWaveformFromLGLSound(): Warning! NULL LGL_Sound or negative lengthSeconds!\n");
		return;
	}
	
	if(sound->GetChannelCount()!=2)
	{
		//printf("LGL_AudioGrain::SetWaveformFromLGLSound(): FIXME: Only works with stereo sounds.\n");
		return;
	}

	long centerPositionSamples=(int)LGL_Max(0,(centerSeconds+LGL_RandFloat(-centerSecondsVariance,centerSecondsVariance))*sound->GetHz());
	LengthSamples=(int)LGL_Max(0,(lengthSeconds+LGL_RandFloat(-lengthSecondsVariance,lengthSecondsVariance))*sound->GetHz());

	long startPositionSamples=centerPositionSamples-(LengthSamples/2);
	if(startPositionSamples<0) startPositionSamples=0;
	
	long endPositionSamples=startPositionSamples+LengthSamples;
	if(endPositionSamples >= sound->GetLengthSamples())
	{
		endPositionSamples=sound->GetLengthSamples()-1;
		LengthSamples=LGL_Max(0,endPositionSamples-startPositionSamples);
	}

	Waveform=new Uint8[LengthSamples*4];
	sound->LockBufferForReading(2);
	{
		if(Uint8* soundBuffer=sound->GetBuffer())
		{
			memcpy
			(
				Waveform,
				&(soundBuffer[startPositionSamples*4]),LengthSamples*4
			);
		}
		else
		{
			bzero(Waveform,LengthSamples*4);
		}
	}
	sound->UnlockBufferForReading();
}

void
LGL_AudioGrain::
SetWaveformFromLGLSoundSamples
(
	LGL_Sound*	sound,
	long		startSamples,
	long		lengthSamples
)
{
	NullifyWaveformAndSpectrum();

	if(sound==NULL || lengthSamples<=0 || startSamples<0)
	{
		return;
	}

	if(sound->GetChannelCount()!=2)
	{
		printf("LGL_AudioGrain::SetWaveformFromLGLSound(): FIXME: Only works with stereo sounds.\n");
		return;
	}

	LengthSamples=lengthSamples;
	long endPositionSamples=startSamples+LengthSamples;

	if(endPositionSamples >= sound->GetLengthSamples())
	{
		endPositionSamples=sound->GetLengthSamples()-1;
		LengthSamples=endPositionSamples-startSamples;
	}

	Waveform=new Uint8[LengthSamples*4];
	sound->LockBufferForReading(3);
	{
		Uint8* soundBuffer=sound->GetBuffer();
		memcpy(Waveform,&(soundBuffer[startSamples*4]),LengthSamples*4);
	}
	sound->UnlockBufferForReading();
}

void
LGL_AudioGrain::
SetStartDelaySeconds
(
	float	startDelaySeconds,
	float	variance
)
{
	StartDelaySamples=(int)LGL_Max(0,(startDelaySeconds+LGL_RandFloat(-variance,variance))*44100);
}

void
LGL_AudioGrain::
SetVolume
(
	float	volume,
	float	variance
)
{
	VolumeFrontLeft=LGL_Max(0,volume+LGL_RandFloat(-variance,variance));
	VolumeFrontRight=LGL_Max(0,volume+LGL_RandFloat(-variance,variance));
	VolumeBackLeft=LGL_Max(0,volume+LGL_RandFloat(-variance,variance));
	VolumeBackRight=LGL_Max(0,volume+LGL_RandFloat(-variance,variance));
}

void
LGL_AudioGrain::
SetVolumeStereo
(
	float	volumeLeft,
	float	volumeRight,
	float	varianceLeft,
	float	varianceRight
)
{
	VolumeFrontLeft=LGL_Max(0,volumeLeft+LGL_RandFloat(-varianceLeft,varianceLeft));
	VolumeFrontRight=LGL_Max(0,volumeRight+LGL_RandFloat(-varianceRight,varianceRight));
	VolumeBackLeft=LGL_Max(0,volumeLeft+LGL_RandFloat(-varianceLeft,varianceLeft));
	VolumeBackRight=LGL_Max(0,volumeRight+LGL_RandFloat(-varianceRight,varianceRight));
}

void
LGL_AudioGrain::
SetVolumeSurround
(
	float	volumeFrontLeft,
	float	volumeFrontRight,
	float	volumeBackLeft,
	float	volumeBackRight,
	float	varianceFrontLeft,
	float	varianceFrontRight,
	float	varianceBackLeft,
	float	varianceBackRight
)
{
	VolumeFrontLeft=LGL_Max(0,volumeFrontLeft+LGL_RandFloat(-varianceFrontLeft,varianceFrontLeft));
	VolumeFrontRight=LGL_Max(0,volumeFrontRight+LGL_RandFloat(-varianceFrontRight,varianceFrontRight));
	VolumeBackLeft=LGL_Max(0,volumeBackLeft+LGL_RandFloat(-varianceBackLeft,varianceBackLeft));
	VolumeBackRight=LGL_Max(0,volumeBackRight+LGL_RandFloat(-varianceBackRight,varianceBackRight));
}

void
LGL_AudioGrain::
SetPitch
(
	float	pitch,
	float	variance
)
{
	PitchAlpha=LGL_Max(0,pitch+LGL_RandFloat(-variance,variance));
	PitchOmega=PitchAlpha;
}

void
LGL_AudioGrain::
SetPitchAlphaOmega
(
	float	pitchAlpha,
	float	pitchOmega,
	float	varianceAlpha,
	float	varianceOmega
)
{
	PitchAlpha=LGL_Max(0,pitchAlpha+LGL_RandFloat(-varianceAlpha,varianceAlpha));
	PitchOmega=LGL_Max(0,pitchOmega+LGL_RandFloat(-varianceOmega,varianceOmega));
}

int
LGL_AudioGrain::
GetStartDelaySamplesRemaining() const
{
	return(StartDelaySamples);
}

void
LGL_AudioGrain::
SetStartDelaySamplesRemaining
(
	int	samples
)
{
	StartDelaySamples=samples;
}

void
LGL_AudioGrain::
SetEnvelope
(
	LGL_EnvelopeType	envelopeType
)
{
	EnvelopeType=envelopeType;
}

int
LGL_AudioGrain::
GetLengthSamples()	const
{
	return((int)LengthSamples);
}

int
LGL_AudioGrain::
GetSpectrumSamples()	const
{
	return((int)SpectrumSamples);
}

bool
LGL_AudioGrain::
IsSilent()
{
	if(Silent==0)
	{
		Sint16* waveform16=(Sint16*)Waveform;
		for(int a=0;a<LengthSamples;a++)
		{
			if(fabsf(waveform16[a]) > 512)
			{
				Silent=-1;
				break;
			}
		}
		if(Silent==0) Silent=1;
	}

	if(Silent<0) return(false);
	if(Silent>0) return(true);
	return(false);
}

float
LGL_AudioGrain::
GetDistanceEuclidean
(
	LGL_AudioGrain*	queryGrain,
	int		mipMapLevel,
	float*		distanceMost
)
{
	if(queryGrain==NULL)
	{
		return(-1.0f);
	}

	CalculateSpectrum();
	queryGrain->CalculateSpectrum();

	float* myArray = SpectrumMono;
	float* queryArray = queryGrain->SpectrumMono;

	float* myArrayError = NULL;
	float* queryArrayError = NULL;

	int arraySize = SpectrumSamples;
	float multiplier=1.0f;	//Without this, low mipMapLevels appear artificially close

	if(mipMapLevel>=0 && mipMapLevel<10)
	{
		CalculateSpectrumMipMaps();
		queryGrain->CalculateSpectrumMipMaps();

		myArray = SpectrumMipMaps[mipMapLevel];
		queryArray = queryGrain->SpectrumMipMaps[mipMapLevel];
		arraySize = (int)powf(2,mipMapLevel);
		multiplier = sqrt(powf(2,9-mipMapLevel));

		myArrayError = SpectrumMipMapsError[mipMapLevel];
		queryArrayError = queryGrain->SpectrumMipMapsError[mipMapLevel];
	}

	float distLeastSq=0.0f;
	float distMostSq=0.0f;

	assert(myArray!=NULL);
	assert(queryArray!=NULL);

	if(myArrayError!=NULL)
	{
		for(int a=0;a<arraySize;a++)
		{
			distLeastSq+=
				(myArray[a]-queryArray[a])*
				(myArray[a]-queryArray[a]);
			distMostSq+=
				(myArrayError[a]+queryArrayError[a]+fabsf(myArray[a]-queryArray[a]))*
				(myArrayError[a]+queryArrayError[a]+fabsf(myArray[a]-queryArray[a]));
		}
	}
	else
	{
		for(int a=0;a<arraySize;a++)
		{
			distLeastSq+=
				(myArray[a]-queryArray[a])*
				(myArray[a]-queryArray[a]);
		}
		distMostSq=distLeastSq;
	}

	float distLeast = multiplier * sqrtf(distLeastSq);
	float distMost = multiplier * sqrtf(distMostSq);

	if(distanceMost!=NULL)
	{
		(*distanceMost) = distMost;
	}
	return(distLeast);
}

bool
LGL_AudioGrain::
GetDistanceLessThanEuclidean
(
	LGL_AudioGrain*	queryGrain,
	float		distance
)
{
	if
	(
		queryGrain==NULL ||
		distance < 0
	)
	{
		return(false);
	}

	//Search the lowest level mipmaps first, to potentially reduce overall computation
	for(int a=0;a<10;a++)
	{
		float testDistMax;
		float testDistMin = GetDistanceEuclidean(queryGrain,a,&testDistMax);
		if(testDistMax < distance)
		{
			return(true);
		}
		if(testDistMin >= distance)
		{
			return(false);
		}
	}
	assert(false);
	return(false);
}

bool
LGL_AudioGrain::
GetDistanceGreaterThanEuclidean
(
	LGL_AudioGrain*	queryGrain,
	float		distance
)
{
	if
	(
		queryGrain==NULL ||
		distance < 0
	)
	{
		return(false);
	}

	//Search the lowest level mipmaps first, to potentially reduce overall computation
	for(int a=0;a<10;a++)
	{
		float testDistMax;
		float testDistMin = GetDistanceEuclidean(queryGrain,a,&testDistMax);
		if(testDistMin > distance)
		{
			return(true);
		}
		if(testDistMax <= distance)
		{
			return(false);
		}
	}
	assert(false);
	return(false);
}

bool
lgl_analyze_wave_segment
(
	long		sampleFirst,
	long		sampleLast,
	float&		zeroCrossingFactor,
	float&		magnitudeAve,
	float&		magnitudeMax,
	const Sint16*	buf16,
	unsigned long	len16,
	bool		loaded,
	int		hz,
	int		channels,
	float		gain=1.0f,
	float		freqEQBalance=0.5f
)
{
	assert(sampleLast>=sampleFirst);
	zeroCrossingFactor=0.0f;
	magnitudeAve=0.0f;
	magnitudeMax=0.0f;

	if(buf16==NULL)
	{
		return(false);
	}

	float magnitudeTotal=0.0f;
	int zeroCrossings=0;

	for(int a=0;a<2;a++)
	{
		long myIndex = sampleFirst+a;
		if((unsigned long)myIndex>len16-1) myIndex=len16-1;
		int zeroCrossingSign=(int)LGL_Sign(buf16[myIndex]);
		for(long b=sampleFirst*channels+a;b<sampleLast*channels;b+=channels)
		{
			long index=b;
			if
			(
				loaded ||
				(
					index>=0 &&
					(unsigned long)index<len16
				)
			)
			{
				while(index<0) index+=len16;
				Sint16 sampleMag=SWAP16(buf16[index%len16]);
				float sampleMagAbs = fabsf(sampleMag);
				magnitudeTotal+=sampleMagAbs;
				if(sampleMagAbs>magnitudeMax)
				{
					magnitudeMax=sampleMagAbs;
				}
				if(zeroCrossingSign*sampleMag<0)
				{
					zeroCrossingSign*=-1;
					zeroCrossings++;
				}
			}
			else
			{
				return(false);
			}
		}
	}

	int samplesScanned=(int)(sampleLast-sampleFirst);

	//EQ Affects Analysis
	{
		magnitudeTotal*=gain;

		float val=1.0f;
		if(freqEQBalance<0.5f)
		{
			val=freqEQBalance*2.0f;
		}
		else if(freqEQBalance==0.5f)
		{
			val=1.0f;
		}
		else if(freqEQBalance>0.5f)
		{
			val=powf(2.0f,8.0f*(freqEQBalance-0.5f));
		}
		zeroCrossings*=val;
	}

	magnitudeAve=(magnitudeTotal/samplesScanned)/(1<<15);
	if(magnitudeAve>1.0f)
	{
		magnitudeAve=1.0f;
	}
	magnitudeMax/=(1<<15);
	if(magnitudeMax>1.0f)
	{
		magnitudeMax=1.0f;
	}

	float minZeroCrossings=samplesScanned/8;

	if(zeroCrossings<minZeroCrossings)
	{
		zeroCrossings=0;
	}
	zeroCrossingFactor=LGL_Clamp
	(
		0.0f,
		4.0f*(zeroCrossings-minZeroCrossings)/(2.0f*(samplesScanned-minZeroCrossings)),
		1.0f
	);
	zeroCrossingFactor=powf(zeroCrossingFactor,0.5f);

	return(true);
}

void
LGL_AudioGrain::
CalculateWaveformDerivatives
(
	float	gain,
	float	freqEQBalance
)
{
	if
	(
		WaveformMonoFloat!=NULL &&
		Gain==gain &&
		FreqEQBalance==freqEQBalance
	)
	{
		//We've already calculated this.
		assert(WaveformLeftFloat!=NULL);
		assert(WaveformRightFloat!=NULL);
		return;
	}

	//Seperate left and right channels
	Sint16* waveform16=(Sint16*)Waveform;
	WaveformLeftFloat=new float[LengthSamples];
	WaveformRightFloat=new float[LengthSamples];
	WaveformMonoFloat=new float[LengthSamples];

	Gain=gain;
	FreqEQBalance=freqEQBalance;

	for(int a=0;a<LengthSamples;a++)
	{
		WaveformLeftFloat[a]=(waveform16[a*2]+32768)/65536.0f;
		WaveformRightFloat[a]=(waveform16[a*2+1]+32768)/65536.0f;
		WaveformMonoFloat[a]=0.5f*(WaveformLeftFloat[a]+WaveformRightFloat[a]);
	}
	lgl_analyze_wave_segment
	(
		0,
		LengthSamples,
		FreqFactor,
		VolAve,
		VolMax,
		waveform16,
		LengthSamples,
		true,
		44100,	//HACK,
		2,
		Gain,
		FreqEQBalance
	);
}

void
LGL_AudioGrain::
CalculateSpectrum()
{
	if(SpectrumMono!=NULL)
	{
		//Spectrum is already calculated
		assert(SpectrumLeft!=NULL);
		assert(SpectrumRight!=NULL);
		return;
	}

	CalculateWaveformDerivatives();

	//Prepare Spectrum Arrays

	SpectrumLeft = new float[SpectrumSamples];
	SpectrumRight = new float[SpectrumSamples];
	SpectrumMono = new float[SpectrumSamples];

	for(int a=0;a<SpectrumSamples;a++)
	{
		SpectrumLeft[a]=0.0f;
		SpectrumRight[a]=0.0f;
		SpectrumMono[a]=0.0f;
	}

	float realLeft[512];
	float imaginaryLeft[512];
	float realRight[512];
	float imaginaryRight[512];

	int cycles = (int)floorf(LengthSamples/512.0f);
	float cyclesFloat = LengthSamples/512.0f;
	int samplesRemaining = LengthSamples;

	for(int c=0;c<cycles;c++)
	{
		assert(samplesRemaining > 0);

		int copySize = samplesRemaining;
		if(copySize > 512)
		{
			copySize = 512;
		}

		memcpy(realLeft,&(WaveformLeftFloat[c*512]),copySize*sizeof(float));
		memcpy(realRight,&(WaveformRightFloat[c*512]),copySize*sizeof(float));

		//Zero out the rest of real, all of imaginary

		for(int a=copySize;a<512;a++)
		{
			realLeft[a]=0.0f;
			realRight[a]=0.0f;
		}

		for(int a=0;a<512;a++)
		{
			imaginaryLeft[a]=0;
			imaginaryRight[a]=0;
		}

		//fft

		LGL_FFT
		(
			realLeft,
			imaginaryLeft,
			9,
			1
		);
		
		LGL_FFT
		(
			realRight,
			imaginaryRight,
			9,
			1
		);

		//Fill in Spectrum data

		for(int a=0;a<SpectrumSamples;a++)
		{
			SpectrumLeft[a]+=fabsf(realLeft[a+1])+fabsf(imaginaryLeft[a+1]);
			SpectrumRight[a]+=fabsf(realRight[a+1])+fabsf(imaginaryRight[a+1]);
			SpectrumMono[a]+=0.5f*(SpectrumLeft[a]+SpectrumRight[a]);
			assert(SpectrumMono[a]>=0);
		}

		samplesRemaining -= copySize;
	}

	//Normalize about cyclesFloat

	if(LengthSamples != 512)
	{
		float multiplier = 1.0f/cyclesFloat;
		for(int a=0;a<SpectrumSamples;a++)
		{
			SpectrumLeft[a]*=multiplier;
			SpectrumRight[a]*=multiplier;
			SpectrumMono[a]*=multiplier;
		}
	}
}

void
LGL_AudioGrain::
CalculateSpectrumMipMaps()
{
	if(SpectrumMipMaps!=NULL)
	{
		//SpectrumMipMaps are already calculated
		assert(SpectrumMipMapsError!=NULL);
		return;
	}

	CalculateSpectrum();

	SpectrumMipMaps=new float*[10];
	SpectrumMipMapsError=new float*[10];
	SpectrumMipMaps[9]=new float[512];
	SpectrumMipMapsError[9]=new float[512];
	memcpy(SpectrumMipMaps[9],SpectrumMono,sizeof(float)*512);
	for(int a=0;a<512;a++)
	{
		assert(SpectrumMono[a]>=0);
		SpectrumMipMapsError[9][a]=0.0f;
	}
	for(int a=8;a>=0;a--)
	{
		int size=(int)powf(2,a);
		SpectrumMipMaps[a]=new float[size];
		SpectrumMipMapsError[a]=new float[size];
		for(int b=0;b<size;b++)
		{
			SpectrumMipMaps[a][b]=0.5f*
				(
					SpectrumMipMaps[a+1][b*2+0]+
					SpectrumMipMaps[a+1][b*2+1]
				);
			assert(SpectrumMipMaps[a][b]>=0.0f);
			float error = 0.5f*fabsf(SpectrumMipMaps[a+1][b*2+0]-SpectrumMipMaps[a+1][b*2+1]);
			SpectrumMipMapsError[a][b]=
				0.5f*
				(
					SpectrumMipMapsError[a+1][b*2+0]+
					SpectrumMipMapsError[a+1][b*2+1]
				)+
				error;
		}
	}
}

void
LGL_AudioGrain::
NullifyWaveformAndSpectrum()
{
	if(Waveform!=NULL)
	{
		delete Waveform;
		Waveform=NULL;
	}
	NullifyAllButWaveform();
	LengthSamples=0;
}

void
LGL_AudioGrain::
NullifyAllButWaveform()
{
	if(WaveformLeftFloat!=NULL)
	{
		delete WaveformLeftFloat;
		WaveformLeftFloat=NULL;
	}
	if(WaveformRightFloat!=NULL)
	{
		delete WaveformRightFloat;
		WaveformRightFloat=NULL;
	}
	if(WaveformMonoFloat!=NULL)
	{
		delete WaveformMonoFloat;
		WaveformMonoFloat=NULL;
	}
	
	if(SpectrumLeft!=NULL)
	{
		delete SpectrumLeft;
		SpectrumLeft=NULL;
	}
	if(SpectrumRight!=NULL)
	{
		delete SpectrumRight;
		SpectrumRight=NULL;
	}
	if(SpectrumMono!=NULL)
	{
		delete SpectrumMono;
		SpectrumMono=NULL;
	}
	if(SpectrumMipMaps!=NULL)
	{
		assert(SpectrumMipMapsError!=NULL);
		for(int a=0;a<10;a++)
		{
			delete SpectrumMipMaps[a];
			delete SpectrumMipMapsError[a];
		}
		delete SpectrumMipMaps;
		delete SpectrumMipMapsError;
		SpectrumMipMaps=NULL;
		SpectrumMipMapsError=NULL;
	}

	Silent=0;
}

void
LGL_AudioGrain::
TEST_PitchShift
(
	int	steps
)
{
	CalculateWaveformDerivatives();

	int cycles = (int)floorf(LengthSamples/1024.0f);
	int samplesRemaining = LengthSamples;
	Uint8* targetWaveform = new Uint8[LengthSamples*4];

	float realLeft[1024];
	float imaginaryLeft[1024];
	float realRight[1024];
	float imaginaryRight[1024];

	for(int c=0;c<cycles;c++)
	{
		assert(samplesRemaining > 0);

		int copySize = samplesRemaining;
		if(copySize > 1024)
		{
			copySize = 1024;
		}

		memcpy(realLeft,&(WaveformLeftFloat[c*1024]),copySize*sizeof(float));
		memcpy(realRight,&(WaveformRightFloat[c*1024]),copySize*sizeof(float));
		
		//Zero out the rest of real, all of imaginary

		for(int a=copySize;a<1024;a++)
		{
			realLeft[a]=0.0f;
			realRight[a]=0.0f;
		}

		for(int a=0;a<1024;a++)
		{
			imaginaryLeft[a]=0;
			imaginaryRight[a]=0;
		}

		//fft => freq domain

		LGL_FFT
		(
			realLeft,
			imaginaryLeft,
			10,
			1
		);

		LGL_FFT
		(
			realRight,
			imaginaryRight,
			10,
			1
		);

		//Randomize Phase Data (cos vs sin)
#if 0
		for(int a=0;a<=512;a++)
		{
			float myRealLeft=realLeft[a];
			float myRealRight=realRight[a];
			float myImaginaryLeft=imaginaryLeft[a];
			float myImaginaryRight=imaginaryRight[a];

			float realPercent;
			realPercent=LGL_RandFloat();
			realLeft[a]=
				(0.0f+realPercent)*myRealLeft +
				(1.0f-realPercent)*myImaginaryLeft;
			imaginaryLeft[a]=
				(1.0f-realPercent)*myRealLeft +
				(0.0f+realPercent)*myImaginaryLeft;
			
			realRight[a]=
				(0.0f+realPercent)*myRealRight +
				(1.0f-realPercent)*myImaginaryRight;
			imaginaryRight[a]=
				(1.0f-realPercent)*myRealRight +
				(0.0f+realPercent)*myImaginaryRight;
		}
#endif	//0

		//Shift spectrum

		float shiftRealLeft[1024];
		float shiftImaginaryLeft[1024];
		float shiftRealRight[1024];
		float shiftImaginaryRight[1024];
		for(int a=0;a<=512;a++)
		{
			if
			(
				a-steps > 0 &&
				a-steps <= 512
			)
			{
				shiftRealLeft[a]=realLeft[a-steps];
				shiftImaginaryLeft[a]=imaginaryLeft[a-steps];
				shiftRealRight[a]=realRight[a-steps];
				shiftImaginaryRight[a]=imaginaryRight[a-steps];
			}
			else
			{
				shiftRealLeft[a]=0;
				shiftImaginaryLeft[a]=0;
				shiftRealRight[a]=0;
				shiftImaginaryRight[a]=0;
			}
		}

		//enforce symmetry for negative frequencies

		for(int a=1;a<512;a++)
		{
			shiftRealLeft[512+a]=shiftRealLeft[512-a];
			shiftImaginaryLeft[512+a]=-shiftImaginaryLeft[512-a];
			shiftRealRight[512+a]=shiftRealRight[512-a];
			shiftImaginaryRight[512+a]=-shiftImaginaryRight[512-a];
		}

		//fft => time domain

		LGL_FFT
		(
			shiftRealLeft,
			shiftImaginaryLeft,
			10,
			-1
		);

		LGL_FFT
		(
			shiftRealRight,
			shiftImaginaryRight,
			10,
			-1
		);

		//Construct new waveform data
		Sint16* targetWaveform16 = (Sint16*)(&(targetWaveform[1024*4*c]));
		for(int a=0;a<copySize;a++)
		{
			targetWaveform16[2*a+0]=(Sint16)(shiftRealLeft[a] * 32767.0f);
			targetWaveform16[2*a+1]=(Sint16)(shiftRealRight[a] * 32767.0f);
		}

		samplesRemaining -= copySize;
	}

	//Reset everything

	long oldLengthSamples = LengthSamples;
	NullifyWaveformAndSpectrum();
	LengthSamples=oldLengthSamples;

	Waveform=new Uint8[LengthSamples*4];
	/*
	//FIXME: This isn't right. Sint16 vs ZeroToOne
	memcpy(Waveform,targetWaveform,LengthSamples*4);
	WaveformLeftFloat=new float[LengthSamples];
	WaveformRightFloat=new float[LengthSamples];
	WaveformMonoFloat=new float[LengthSamples];

	Sint16* targetWaveform16 = (Sint16*)targetWaveform;
	for(int a=0;a<LengthSamples;a++)
	{
		WaveformLeftFloat[a]=targetWaveform16[2*a+0];
		WaveformRightFloat[a]=targetWaveform16[2*a+1];
		WaveformMonoFloat[a]=0.5f*(WaveformLeftFloat[a]+WaveformRightFloat[a]);
	}
	*/

	//Clean up

	delete targetWaveform;
}

void
LGL_AudioGrain::
TEST_ApplyThreeChannelEQ
(
	float	low,
	float	mid,
	float	high
)
{
	int cycles = (int)floorf(LengthSamples/1024.0f);
	if(cycles==0)
	{
		printf("TEST_ApplyThreeChannelEQ(): Warning! Zero cycles!\n");
		return;
	}
if(cycles>1)
{
	printf("TEST_ApplyThreeChannelEQ(): Warning! >1 cycles!\n");
}

	CalculateWaveformDerivatives();

	int samplesRemaining = LengthSamples;
	Uint8* targetWaveform = new Uint8[LengthSamples*4];

	float realLeft[1024];
	float imaginaryLeft[1024];
	float realRight[1024];
	float imaginaryRight[1024];

	for(int c=0;c<cycles;c++)
	{
		assert(samplesRemaining > 0);

		int copySize = samplesRemaining;
		if(copySize > 1024)
		{
			copySize = 1024;
		}

		memcpy(realLeft,&(WaveformLeftFloat[c*1024]),copySize*sizeof(float));
		memcpy(realRight,&(WaveformRightFloat[c*1024]),copySize*sizeof(float));
		
		//Zero out the rest of real, all of imaginary

		for(int a=copySize;a<1024;a++)
		{
			realLeft[a]=0.0f;
			realRight[a]=0.0f;
		}

		for(int a=0;a<1024;a++)
		{
			imaginaryLeft[a]=0;
			imaginaryRight[a]=0;
		}

		//fft => freq domain

		LGL_FFT
		(
			realLeft,
			imaginaryLeft,
			10,
			1
		);

		LGL_FFT
		(
			realRight,
			imaginaryRight,
			10,
			1
		);

		//Apply EQ

		float eqRealLeft[1024];
		float eqImaginaryLeft[1024];
		float eqRealRight[1024];
		float eqImaginaryRight[1024];
		for(int a=0;a<=512;a++)
		{
			//FIXME: This is just a lowpass, for testing purposes
			if(a<25)
			{
				eqRealLeft[a]=realLeft[a];
				eqImaginaryLeft[a]=imaginaryLeft[a];
				eqRealRight[a]=realRight[a];
				eqImaginaryRight[a]=imaginaryRight[a];
			}
			else
			{
				eqRealLeft[a]=0;
				eqImaginaryLeft[a]=0;
				eqRealRight[a]=0;
				eqImaginaryRight[a]=0;
			}
		}

		//enforce symmetry for negative frequencies

		for(int a=1;a<512;a++)
		{
			eqRealLeft[512+a]=eqRealLeft[512-a];
			eqImaginaryLeft[512+a]=-eqImaginaryLeft[512-a];
			eqRealRight[512+a]=eqRealRight[512-a];
			eqImaginaryRight[512+a]=-eqImaginaryRight[512-a];
		}

		//fft => time domain

		LGL_FFT
		(
			eqRealLeft,
			eqImaginaryLeft,
			10,
			-1
		);

		LGL_FFT
		(
			eqRealRight,
			eqImaginaryRight,
			10,
			-1
		);

		//Construct new waveform data
		Sint16* targetWaveform16 = (Sint16*)(&(targetWaveform[1024*4*c]));
		for(int a=0;a<copySize;a++)
		{
			targetWaveform16[2*a+0]=(Sint16)((eqRealLeft[a]-.5f) * (32767.0f*2));
			targetWaveform16[2*a+1]=(Sint16)((eqRealRight[a]-.5f) * (32767.0f*2));
		}

		samplesRemaining -= copySize;
	}

	//Reset everything

	long oldLengthSamples = LengthSamples;
	NullifyWaveformAndSpectrum();
	LengthSamples=oldLengthSamples;

	Waveform=new Uint8[LengthSamples*4];
	memcpy(Waveform,targetWaveform,LengthSamples*4);
	/*
	//FIXME: This isn't right. Sint16 vs ZeroToOne
	WaveformLeftFloat=new float[LengthSamples];
	WaveformRightFloat=new float[LengthSamples];
	WaveformMonoFloat=new float[LengthSamples];

	Sint16* targetWaveform16 = (Sint16*)targetWaveform;
	for(int a=0;a<LengthSamples;a++)
	{
		WaveformLeftFloat[a]=targetWaveform16[2*a+0];
		WaveformRightFloat[a]=targetWaveform16[2*a+1];
		WaveformMonoFloat[a]=0.5f*(WaveformLeftFloat[a]+WaveformRightFloat[a]);
	}
	*/

	//Clean up

	delete targetWaveform;
}

LGL_AudioGrainStream::
LGL_AudioGrainStream() : GrainListsSemaphore("AudioGrainStream GrainLists")
{
	//
}

LGL_AudioGrainStream::
~LGL_AudioGrainStream()
{
	//FIXME: Delete my grains!
}

void
LGL_AudioGrainStream::
Update()
{
	/*
	for(unsigned int a=0;a<AudioGrainsActive;a++)
	{
		//
	}
	*/
}

bool
LGL_AudioGrainStream::
Finished()	const
{
	return(false);
}

int
LGL_AudioGrainStream::
MixIntoStream
(
	void*	userdata,
	Uint8*	stream8,
	int	len
)
{
	int len16=len/2;
	int len32=len/4;

	int samplesMixed=0;

	{
		LGL_ScopeLock lock(__FILE__,__LINE__,GrainListsSemaphore);
		for(unsigned int a=0;a<AudioGrainsQueued.size();a++)
		{
			bool stereo=true;	//FIXME: AudioGrains must be stereo, for now.
			int myLen=stereo?len32:len16;

			if(AudioGrainsQueued[a]->GetStartDelaySamplesRemaining()<=myLen)
			{
				AudioGrainsActive.push_back(AudioGrainsQueued[a]);
				AudioGrainsQueued.erase
				(
					(std::vector<LGL_AudioGrain*>::iterator)
					(&AudioGrainsQueued[a])
				);
				a--;
			}
		}
		for(unsigned int a=0;a<AudioGrainsQueued.size();a++)
		{
			bool stereo=true;	//FIXME: AudioGrains must be stereo, for now.
			int myLen=stereo?len32:len16;

			assert(AudioGrainsQueued[a]->GetStartDelaySamplesRemaining()>myLen);

			AudioGrainsQueued[a]->SetStartDelaySamplesRemaining
			(
				AudioGrainsQueued[a]->GetStartDelaySamplesRemaining()-myLen
			);
		}
	
		for(unsigned int a=0;a<AudioGrainsActive.size();a++)
		{
			int mixed=AudioGrainsActive[a]->MixIntoStream
			(
				userdata,
				stream8,
				len
			);
			if(mixed>samplesMixed)
			{
				samplesMixed=mixed;
			}
			if(AudioGrainsActive[a]->Finished())
			{
				delete AudioGrainsActive[a];
				AudioGrainsActive.erase
				(
					(std::vector<LGL_AudioGrain*>::iterator)
					(&AudioGrainsActive[a])
				);
				a--;
			}
		}
	}

	return(samplesMixed);
}

void
LGL_AudioGrainStream::
AddNextGrain
(
	LGL_AudioGrain*	grain,
	long offsetDelaySamples
)
{
	{
		LGL_ScopeLock lock(__FILE__,__LINE__,GrainListsSemaphore);
		if(offsetDelaySamples>=0)
		{
			if(AudioGrainsQueued.empty()==false)
			{
				grain->SetStartDelaySamplesRemaining
				(
					AudioGrainsQueued[AudioGrainsQueued.size()-1]->GetStartDelaySamplesRemaining()+
					offsetDelaySamples
				);
			}
			else
			{
				grain->SetStartDelaySamplesRemaining(offsetDelaySamples);
			}
		}
		AudioGrainsQueued.push_back(grain);
	}
}

void
LGL_AudioGrainStream::
AddNextGrains
(
	std::vector<LGL_AudioGrain*>&	grains
)
{
	{
		LGL_ScopeLock lock(__FILE__,__LINE__,GrainListsSemaphore);
		for(unsigned int a=0;a<grains.size();a++)
		{
			LGL_AudioGrain* grain=grains[a];
			AudioGrainsQueued.push_back(grain);
		}
	}
}

long
LGL_AudioGrainStream::
GetGrainsQueuedCount()
{
	return(AudioGrainsQueued.size());
}

//Audio1

int
lgl_sound_decoder_thread
(
	void* object
)
{
	//LGL_ThreadSetCPUAffinity(0);
	LGL_ThreadSetPriority(LGL_PRIORITY_AUDIO_DECODE,"AudioDecode");
	LGL_Sound* sound=(LGL_Sound*)object;
	sound->LoadToMemory();
	sound->DeleteSemaphore.Unlock();
	return(0);
}

LGL_Sound::
LGL_Sound
(
	const char*		filename,
	bool			LoadInNewThread,
	int			channels,
	Uint8*			buffer,
	unsigned long		bufferLength
) :
	BufferSemaphore("Sound Buffer"),
	BufferReaderCountSemaphore("Sound BufferReaderCount"),
	DeleteSemaphore("Sound Delete")
{
	Buffer=buffer;
	BufferLength=0;
	BufferLengthTotal=buffer?bufferLength:0;
	BufferBack=NULL;
	BufferBackLength=0;
	BufferBackLengthTotal=0;
	BufferBackReadyForWriting=true;
	BufferBackReadyForReading=false;
	BufferAllocatedFromElsewhere=(buffer!=NULL);

	if(buffer==NULL)
	{
		BufferLengthTotal=4*44100*60*20;
		Buffer=(Uint8*)lgl_av_mallocz(BufferLengthTotal);
	}

	BufferReaderCount=0;
	HogCPU=false;
	DestructorHint=false;
	DeleteOK=false;
	PrepareForDeleteThread=NULL;
	assert(strlen(filename)<1024);
	//if(LGL.AudioAvailable==false) return;
	Channels=channels;
	Hz=44100;

	LoadSecondsUntilLoaded=9999.0f;

	BadFile=false;
	Silent=false;

	PercentLoaded=0.0f;
	LoadedSmooth=0.0f;
	LoadedMin=0;

	ReferenceCount=0;

	strcpy(Path,filename);

	char temp[1024];
	strcpy(temp,Path);
	char* ptr=&(temp[0]);
	while(strstr(ptr,"/"))
	{
		ptr=&(strstr(ptr,"/")[1]);
	}
	strcpy(PathShort,ptr);

	Loaded=false;
	MetadataVolumePeak=0.0f;
	MetadataFilledSize=0;

	if(LoadInNewThread)
	{
		DeleteSemaphore.Lock(__FILE__,__LINE__);
		DecoderThread=LGL_ThreadCreate(lgl_sound_decoder_thread,this);
		if(DecoderThread==NULL)
		{
			printf("Wow couldn't create an LGL_Sound DecoderThread! Crashy, crashy!\n");	//FIXME: CRASH!!!
		}
	}
	else
	{
		DecoderThread=NULL;
		SetHogCPU(true);
		LoadToMemory();
	}
}

LGL_Sound::
LGL_Sound
(
	Uint8*	buffer,
	Uint32	len,
	int	channels
) :
	BufferSemaphore("Sound Buffer"),
	BufferReaderCountSemaphore("Sound BufferReaderCount"),
	DeleteSemaphore("Sound Delete")
{
	PercentLoaded=0.0f;
	
	Buffer=new Uint8[len];
	memcpy(Buffer,buffer,len);
	BufferLength=len;
	DecoderThread=NULL;
	HogCPU=false;
	DestructorHint=false;
	DeleteOK=false;
	PrepareForDeleteThread=NULL;

	Channels=channels;
	Hz=44100;

	MetadataVolumePeak=0.0f;
	MetadataFilledSize=0;

	strcpy(Path,"Memory Buffer");
	strcpy(PathShort,Path);

	Loaded=true;
	LoadedSmooth=1;
	LoadedSmoothTime=LGL_SecondsSinceExecution();

	ReferenceCount=0;
}

LGL_Sound::
~LGL_Sound()
{
	assert(ReadyForDelete());
	if(DecoderThread)
	{
		LGL_ThreadWait(DecoderThread);
		DecoderThread=NULL;
	}
	if(PrepareForDeleteThread)
	{
		LGL_ThreadWait(PrepareForDeleteThread);
		PrepareForDeleteThread=NULL;
	}
}

int
lgl_Sound_PrepareForDeleteThread
(
	void*	lgl_sound
)
{
	LGL_Sound* snd = (LGL_Sound*)lgl_sound;
	snd->PrepareForDeleteThreadFunc();
	return(0);
}

void
LGL_Sound::
PrepareForDelete()
{
	if(DestructorHint)
	{
		return;
	}

	DestructorHint=true;

	if(ReferenceCount!=0)
	{
		printf("LGL_Sound::~LGL_Sound(): Error! Reference Count for Sound '%s' is %i, not 0!\n",Path,ReferenceCount);
		assert(ReferenceCount==0);
	}

	PrepareForDeleteThread = LGL_ThreadCreate(lgl_Sound_PrepareForDeleteThread,this);
}

bool
LGL_Sound::
PreparingForDelete()
{
	return
	(
		DestructorHint==true &&
		DeleteOK==false
	);
}

bool
LGL_Sound::
ReadyForDelete()
{
	return(DeleteOK);
}

void
LGL_Sound::
PrepareForDeleteThreadFunc()
{
	//Signal we want the sound cleared in all active channels
	for(int a=0;a<LGL_SOUND_CHANNEL_NUMBER;a++)
	{
		if
		(
			LGL.SoundChannel[a].Occupied==true &&
			LGL.SoundChannel[a].Buffer==Buffer
		)
		{
			lgl_ClearAudioChannel(a,this);
		}
	}

	if(LGL.AudioAvailable)
	{
		//Wait until the sound is actually stopped in all active channels
		for(;;)
		{
			bool ok=true;
			for(int a=0;a<LGL_SOUND_CHANNEL_NUMBER;a++)
			{
				if
				(
					LGL.SoundChannel[a].Occupied==true &&
					LGL.SoundChannel[a].Buffer==Buffer
				)
				{
					ok=false;
					break;
				}
			}
			if(!ok)
			{
				if(LGL.AudioOutCallbackTimer.SecondsSinceLastReset()>1.0f)
				{
					ok=true;
				}
			}
			if(ok)
			{
				break;
			}
			else
			{
				LGL_DelayMS(5);
			}
		}
	}

	{
		LGL_ScopeLock deleteLock(__FILE__,__LINE__,DeleteSemaphore);
		if(DecoderThread!=NULL)
		{
			LGL_ThreadWait(DecoderThread);
			DecoderThread=NULL;
		}

		if(Buffer!=NULL)
		{
			if(BufferAllocatedFromElsewhere==false)
			{
				LGL_ScopeLock bufferLock(__FILE__,__LINE__,BufferSemaphore);
				free(Buffer);
				Buffer=NULL;
			}
			else
			{
				Buffer=NULL;
			}
		}
		if(BufferBack!=NULL)
		{
			LGL_ScopeLock bufferLock(__FILE__,__LINE__,BufferSemaphore);
			free(BufferBack);
			BufferBack=NULL;
		}
	}

	DeleteOK=true;
}

int
LGL_Sound::
Play
(
	float			volume,
	bool			looping,
	float			speed,
	float			startSeconds,
	float			lengthSeconds
)
{
	//if(LGL.AudioAvailable==false) return(-1);
	if(false)//IsLoaded()==false)
	{
		printf("LGL_Sound::Play(): Error! '%s' not yet loaded\n",Path);
		LGL_Exit();
	}

	if(lengthSeconds==-1)
	{
		if(IsLoaded())
		{
			lengthSeconds=GetLengthSeconds();
		}
		else
		{
			//We don't know how long it's going to be...
			//So just make it really long, and hope nothing breaks.
			lengthSeconds=55555.0f;
		}
	}

	int available=-1;
	for(int a=0;a<LGL_SOUND_CHANNEL_NUMBER;a++)
	{
		if(LGL.SoundChannel[a].Occupied==false)
		{
			available=a;
			break;
		}
	}
	if(available!=-1)
	{
		lgl_ClearAudioChannelNow(available);
		LGL.SoundChannel[available].Paused=false;
		LGL.SoundChannel[available].PositionSamplesStart=startSeconds*Hz;
		LGL.SoundChannel[available].PositionSamplesPrev=startSeconds*Hz;
		LGL.SoundChannel[available].PositionSamplesNow=startSeconds*Hz;
		LGL.SoundChannel[available].PositionSamplesNowOutwards=LGL.SoundChannel[available].PositionSamplesNow;
		LGL.SoundChannel[available].PositionSamplesEnd=startSeconds*Hz+
			lengthSeconds*Hz;
		LGL.SoundChannel[available].VolumeFrontLeftDesired=volume;
		LGL.SoundChannel[available].VolumeFrontRightDesired=volume;
		LGL.SoundChannel[available].VolumeBackLeftDesired=volume;
		LGL.SoundChannel[available].VolumeBackRightDesired=volume;
		LGL.SoundChannel[available].VolumeFrontLeft=volume;
		LGL.SoundChannel[available].VolumeFrontRight=volume;
		LGL.SoundChannel[available].VolumeBackLeft=volume;
		LGL.SoundChannel[available].VolumeBackRight=volume;
		LGL.SoundChannel[available].Channels=Channels;
		LGL.SoundChannel[available].Hz=Hz;
		LGL.SoundChannel[available].SpeedNow=speed;
		LGL.SoundChannel[available].SpeedDesired=speed;
		LGL.SoundChannel[available].LengthSamples=lengthSeconds*Hz;
		LGL.SoundChannel[available].BufferLength=BufferLength;
		LGL.SoundChannel[available].Buffer=Buffer;
		LGL.SoundChannel[available].BufferSemaphore=&BufferSemaphore;
		LGL.SoundChannel[available].LGLSound=this;
		LGL.SoundChannel[available].Occupied=true;
	}
	else
	{
		printf("LGL_Sound.Play(): Unable to play '%s'. No free channels.\n",Path);
	}

	return(available);
}
	
void
LGL_Sound::
SetVolume
(
	int	channel,
	float	volume
)
{
if(channel<0)
{
	printf("LGL_Sound::SetVolume(): WARNING! channel < 0\n");
	return;
}
	//if(LGL.AudioAvailable==false) return;
	LGL.SoundChannel[channel].VolumeFrontLeftDesired=volume;
	LGL.SoundChannel[channel].VolumeFrontRightDesired=volume;
	LGL.SoundChannel[channel].VolumeBackLeftDesired=volume;
	LGL.SoundChannel[channel].VolumeBackRightDesired=volume;
}

void
LGL_Sound::
Stop
(
	int	channel
)
{
if(channel<0)
{
	printf("LGL_Sound::Stop(): WARNING! channel < 0\n");
	return;
}
	//if(LGL.AudioAvailable==false) return;

	lgl_ClearAudioChannel(channel,this);
}

void
LGL_Sound::
SetVolumeStereo
(
	int	channel,
	float	left,
	float	right
)
{
if(channel<0)
{
	printf("LGL_Sound::SetVolumeStereo(): WARNING! channel < 0\n");
	return;
}
	//if(LGL.AudioAvailable==false) return;

	LGL.SoundChannel[channel].VolumeFrontLeftDesired=left;
	LGL.SoundChannel[channel].VolumeFrontRightDesired=right;
	LGL.SoundChannel[channel].VolumeBackLeftDesired=left;
	LGL.SoundChannel[channel].VolumeBackRightDesired=right;
}

void
LGL_Sound::
SetVolumeSurround
(
	int	channel,
	float	frontLeft,
	float	frontRight,
	float	backLeft,
	float	backRight
)
{
if(channel<0)
{
	printf("LGL_Sound::SetVolumeSurround(): WARNING! channel < 0\n");
	return;
}
	//if(LGL.AudioAvailable==false) return;

	LGL.SoundChannel[channel].VolumeFrontLeftDesired=frontLeft;
	LGL.SoundChannel[channel].VolumeFrontRightDesired=frontRight;

	LGL.SoundChannel[channel].VolumeBackLeftDesired=backLeft;
	LGL.SoundChannel[channel].VolumeBackRightDesired=backRight;
}

void
LGL_Sound::
TogglePause
(
	int	channel
)
{
if(channel<0)
{
	printf("LGL_Sound::TogglePause(): WARNING! channel < 0\n");
	return;
}
	//if(LGL.AudioAvailable==false) return;
	LGL.SoundChannel[channel].Paused=!LGL.SoundChannel[channel].Paused;
}

bool
LGL_Sound::
IsPlaying
(
	int	channel
)
{
	//if(LGL.AudioAvailable==false) return(false);
	return
	(
		LGL.SoundChannel[channel].Occupied &&
		!LGL.SoundChannel[channel].Paused
	);
}

void
LGL_Sound::
SetPositionSamples
(
	int	channel,
	unsigned long samples
)
{
if(channel<0)
{
	printf("LGL_Sound::SetPositionSampless(): WARNING! channel < 0\n");
	return;
}
	//if(LGL.AudioAvailable==false) return;
	LGL.SoundChannel[channel].FuturePositionSamplesPrev=samples;
	LGL.SoundChannel[channel].FuturePositionSamplesNow=samples;
	LGL.SoundChannel[channel].PositionSamplesNowLastReported=samples;
}

void
LGL_Sound::
SetPositionSeconds
(
	int	channel,
	float	seconds
)
{
if(channel<0)
{
	printf("LGL_Sound::SetPositionSeconds(): WARNING! channel < 0\n");
	return;
}
	//if(LGL.AudioAvailable==false) return;

	LGL.SoundChannel[channel].FuturePositionSamplesPrev=seconds*Hz;
	LGL.SoundChannel[channel].FuturePositionSamplesNow=seconds*Hz;
	LGL.SoundChannel[channel].PositionSamplesNowLastReported=seconds*Hz;
}

void
LGL_Sound::
SetLooping
(
	int	channel,
	bool	looping
)
{
if(channel<0)
{
	printf("LGL_Sound::SetLooping(): WARNING! channel < 0\n");
	return;
}
	//if(LGL.AudioAvailable==false) return;

	LGL.SoundChannel[channel].Loop=looping;
}

void
LGL_Sound::
SetStickyEndpoints
(
	int	channel,
	bool	stickyEndpoints
)
{
if(channel<0)
{
	printf("LGL_Sound::SetStickyEndpoints(): WARNING! channel < 0\n");
	return;
}
	//if(LGL.AudioAvailable==false) return;

	LGL.SoundChannel[channel].StickyEndpoints=stickyEndpoints;
}

void
LGL_Sound::
SetSpeed
(
	int	channel,
	float	speed,
	bool	instant
)
{
if(channel<0)
{
	printf("LGL_Sound::SetSpeed(): WARNING! channel < 0\n");
	return;
}
	//if(LGL.AudioAvailable==false) return;

	LGL.SoundChannel[channel].SpeedDesired=speed;
	if(instant)
	{
		LGL.SoundChannel[channel].SpeedNow=speed;
	}
}

void
LGL_Sound::
SetSpeedInterpolationFactor
(
	int	channel,
	float	speedif
)
{
if(channel<0)
{
	printf("LGL_Sound::SetSpeedInterpolationFactor(): WARNING! channel < 0\n");
	return;
}
	//if(LGL.AudioAvailable==false) return;

	if(LGL.SoundChannel[channel].SpeedInterpolationFactor!=speedif)
	{
		LGL.SoundChannel[channel].SpeedInterpolationFactor=speedif;
	}
}

void
LGL_Sound::
SetGlitchAttributes
(
	int	channel,
	bool	enable,
	long	samplesBegin,
	int	samplesLength,
	float	volume,
	float	speed,
	float	duo, float	interpolation,
	bool	luminscratch,
	long	luminscratchPositionDesired
)
{
	if(channel<0)
	{
		printf("LGL_Sound::SetGlitchAttributes(): WARNING! channel < 0\n");
		return;
	}
	//if(LGL.AudioAvailable==false) return;

	if(channel<0 || channel>=LGL_SOUND_CHANNEL_NUMBER)
	{
		printf
		(
			"LGL_Sound::SetGlitchAttributes(): Channel '%i' isn't between 0 and %i\n",
			channel,
			LGL_SOUND_CHANNEL_NUMBER
		);
		LGL_Exit();
	}
	
	//SDL_LockAudio();
	{
		if(enable)
		{
			if(LGL.SoundChannel[channel].Glitch==false)
			{
				LGL.SoundChannel[channel].Glitch=true;
				LGL.SoundChannel[channel].FutureGlitchSettingsAvailable=false;
				LGL.SoundChannel[channel].GlitchVolume=volume;
				LGL.SoundChannel[channel].GlitchSpeedNow=speed;
				if(luminscratch)
				{
					LGL.SoundChannel[channel].GlitchSpeedNow=
						LGL.SoundChannel[channel].SpeedNow;
				}
				if(luminscratch)
				{
					LGL.SoundChannel[channel].GlitchSpeedDesired=speed;
				}
				LGL.SoundChannel[channel].GlitchSpeedInterpolationFactor=interpolation;
				LGL.SoundChannel[channel].GlitchDuo=duo;
				LGL.SoundChannel[channel].GlitchLuminScratch=luminscratch;
				if(luminscratchPositionDesired==-10000)
				{
					LGL.SoundChannel[channel].GlitchLuminScratchPositionDesired=luminscratchPositionDesired;
					LGL.SoundChannel[channel].GlitchSamplesNow=GetPositionSamples(channel);
				}
				else
				{
					LGL.SoundChannel[channel].GlitchLuminScratchPositionDesired=luminscratchPositionDesired%GetLengthSamples();
					LGL.SoundChannel[channel].GlitchSamplesNow=LGL.SoundChannel[channel].GlitchLuminScratchPositionDesired;
				}
				LGL.SoundChannel[channel].GlitchLast=GetPositionSamples(channel);
				LGL.SoundChannel[channel].GlitchBegin=(samplesBegin >= 0) ? samplesBegin : GetPositionSamples(channel);
				LGL.SoundChannel[channel].GlitchLength=samplesLength;
			}
			else
			{
				LGL.SoundChannel[channel].FutureGlitchSettingsAvailable=true;
				LGL.SoundChannel[channel].GlitchVolume=volume;
				if(luminscratchPositionDesired==-10000 || luminscratch==false)
				{
					LGL.SoundChannel[channel].FutureGlitchSpeedDesired=speed;
				}
				LGL.SoundChannel[channel].GlitchSpeedInterpolationFactor=interpolation;
				LGL.SoundChannel[channel].GlitchDuo=duo;
				LGL.SoundChannel[channel].GlitchLuminScratch=luminscratch;
				if(luminscratchPositionDesired==-10000)
				{
					LGL.SoundChannel[channel].FutureGlitchLuminScratchPositionDesired=luminscratchPositionDesired;
				}
				else
				{
					LGL.SoundChannel[channel].FutureGlitchLuminScratchPositionDesired=luminscratchPositionDesired%GetLengthSamples();
				}
				if(samplesBegin >= 0) LGL.SoundChannel[channel].GlitchBegin=samplesBegin;
				LGL.SoundChannel[channel].GlitchLength=samplesLength;
			}
		}
		else
		{
			LGL.SoundChannel[channel].Glitch=false;
			LGL.SoundChannel[channel].FutureGlitchSettingsAvailable=false;
			LGL.SoundChannel[channel].GlitchVolume=0;
			LGL.SoundChannel[channel].GlitchSpeedNow=1;
			LGL.SoundChannel[channel].GlitchSpeedDesired=1;
			LGL.SoundChannel[channel].GlitchSpeedInterpolationFactor=1;
			LGL.SoundChannel[channel].GlitchDuo=0;
			LGL.SoundChannel[channel].GlitchLuminScratch=false;
			LGL.SoundChannel[channel].GlitchLuminScratchPositionDesired=-10000;
			LGL.SoundChannel[channel].GlitchSamplesNow=0;
			LGL.SoundChannel[channel].GlitchLast=0;
			LGL.SoundChannel[channel].GlitchBegin=0;
			LGL.SoundChannel[channel].GlitchLength=0;
		}
	}
	//SDL_UnlockAudio();
}

void
LGL_Sound::
SetGlitchSamplesNow
(
	int	channel,
	long	glitchNowSamples
)
{
if(channel<0)
{
	printf("LGL_Sound::SetGlitchSamplesNow(): WARNING! channel < 0\n");
	return;
}
	LGL.SoundChannel[channel].FutureGlitchSamplesNow=glitchNowSamples;
}

long
LGL_Sound::
GetGlitchLuminScratchPositionDesired
(
	int	channel
)
{
if(channel<0)
{
	printf("LGL_Sound::SetGlitchLuminScratchPositionDesired(): WARNING! channel < 0\n");
	return(0);
}
	return(LGL.SoundChannel[channel].GlitchLuminScratchPositionDesired);
}

void
LGL_Sound::
SetDownMixToMono
(
	int	channel,
	bool	DownMix
)
{
if(channel<0)
{
	printf("LGL_Sound::SetDownMixToMono(): WARNING! channel < 0\n");
	return;
}
	//if(LGL.AudioAvailable==false) return;

	if(channel<0 || channel>=LGL_SOUND_CHANNEL_NUMBER)
	{
		printf
		(
			"LGL_Sound::SetDownMixToMono(): Channel '%i' isn't between 0 and %i\n",
			channel,
			LGL_SOUND_CHANNEL_NUMBER
		);
		LGL_Exit();
	}
	
	LGL.SoundChannel[channel].ToMono=DownMix;
}

void
LGL_Sound::
SetFreqResponse
(
	int	channel,
	float*	freqResponseArrayOf513
)
{
if(channel<0)
{
	printf("LGL_Sound::SetFreqResponse(): WARNING! channel < 0\n");
	return;
}
	//Validate freqResponseArrayOf513
	float helperArray[513];
	if(freqResponseArrayOf513==NULL)
	{
		//Reset EQ
		for(int a=0;a<513;a++)
		{
			helperArray[a]=1.0f;
		}
		freqResponseArrayOf513=helperArray;
	}

	//Set DSP's FreqResponse
	if
	(
		LGL.SoundChannel[channel].Occupied==false ||
		LGL.SoundChannel[channel].LGLSound!=this
	)
	{
		return;
	}

	for(int a=0;a<Channels/2;a++)
	{
		LGL_AudioDSP* dsp=LGL.SoundChannel[channel].LGLAudioDSP[a];
		if(dsp==NULL)
		{
			dsp=new LGL_AudioDSP;
		}
		dsp->SetFreqResponse(freqResponseArrayOf513);
		LGL.SoundChannel[channel].LGLAudioDSP[a]=dsp;
	}
}

float
LGL_Sound::
GetSpeed
(
	int	channel
)
{
if(channel<0)
{
	printf("LGL_Sound::GetSpeed(): WARNING! channel < 0\n");
	return(1.0f);
}
	//if(LGL.AudioAvailable==false) return(1);
	return(LGL.SoundChannel[channel].SpeedNow);
}

double
LGL_Sound::
GetPositionSeconds
(
	int	channel
)
{
if(channel<0)
{
	printf("LGL_Sound::GetPositionSeconds(): WARNING! channel < 0\n");
	return(0.0f);
}
	//if(LGL.AudioAvailable==false) return(0);

	double ret = GetPositionSamples(channel)/(double)Hz;
	if(ret > GetLengthSeconds())
	{
		ret=0.0;
	}
	return(ret);
}

float
LGL_Sound::
GetPositionPercent
(
	int	channel
)
{
if(channel<0)
{
	printf("LGL_Sound::GetPositionPercent(): WARNING! channel < 0\n");
	return(0.0f);
}
	//if(LGL.AudioAvailable==false) return(0);
	if(GetLengthSeconds()==0)
	{
		return(0);
	}
	else
	{
		return(GetPositionSeconds(channel)/GetLengthSeconds());
	}
}

unsigned long
LGL_Sound::
GetPositionSamples
(
	int	channel
)
{
	if(channel<0)
	{
		printf("LGL_Sound::GetPositionSamples(): WARNING! channel < 0\n");
		return(0);
	}
	//if(LGL.AudioAvailable==false) return(0);

	signed long ret=(signed long)((LGL.SoundChannel[channel].PositionSamplesNowOutwards));
	if(LGL.SoundChannel[channel].FuturePositionSamplesNow>=0.0f)
	{
		ret = (signed long)LGL.SoundChannel[channel].FuturePositionSamplesNow;
	}
	if(LGL.SoundChannel[channel].DivergeRecallNow)
	{
		//Recall!
		ret=(signed long)LGL.SoundChannel[channel].DivergeRecallSamples;
	}

	if(ret<0)
	{
		return(0);
	}

	if(ret>=(signed long)BufferLength)
	{
		ret=BufferLength-1;
	}

	LGL.SoundChannel[channel].PositionSamplesNowLastReported = ret;

	return((unsigned long)ret);

}

unsigned long
LGL_Sound::
GetPositionGlitchBeginSamples
(
	int	channel
)
{
if(channel<0)
{
	printf("LGL_Sound::GetPositionGlitchBeginSamples(): WARNING! channel < 0\n");
	return(0);
}
	return((unsigned long)(LGL.SoundChannel[channel].GlitchSamplesNow));	//FIXME: This isn't right.
}

/*
bool
LGL_Sound::
SetDivergeRecallOff
(
	int	channel
)
{
	if(channel<0)
	{
		printf("SetDivergeRecallOff(): WARNING! channel < 0!\n");
		return(false);
	}

	if(LGL.SoundChannel[channel].DivergeState==1)
	{
		LGL.SoundChannel[channel].DivergeState=0;
	}
	return(true);
}

bool
LGL_Sound::
SetDivergeRecallBegin
(
	int	channel,
	float	speed
)
{
	if(channel<0)
	{
		printf("LGL_Sound::SetDivergeRecallBegin(): WARNING! channel < 0\n");
		return(false);
	}

	LGL.SoundChannel[channel].DivergeSpeed=speed;
	LGL.SoundChannel[channel].DivergeState=1;
	LGL.SoundChannel[channel].DivergeSamples=LGL.SoundChannel[channel].PositionSamplesNow;

	return(true);
}

bool
LGL_Sound::
SetDivergeRecallEnd
(
	int	channel
)
{
	if(channel<0)
	{
		printf("LGL_Sound::SetDivergeRecallEnd(): WARNING! channel < 0\n");
		return(false);
	}

	LGL.SoundChannel[channel].DivergeState=-1;

	return(true);
}
*/

bool
LGL_Sound::
DivergeRecallPush
(
	int	channel,
	float	speed
)
{
	if(channel<0)
	{
		printf("LGL_Sound::DivergeRecallPush(): WARNING! channel < 0\n");
		return(false);
	}

	if(LGL.SoundChannel[channel].DivergeCount==0)
	{
		LGL.SoundChannel[channel].DivergeSpeed=(speed==-1.0f) ? LGL.SoundChannel[channel].SpeedDesired : speed;
		LGL.SoundChannel[channel].DivergeSamples=LGL.SoundChannel[channel].PositionSamplesNow;
	}
	LGL.SoundChannel[channel].DivergeCount++;

	return(true);
}

bool
LGL_Sound::
DivergeRecallPop
(
	int	channel,
	bool	recall
)
{
	if(channel<0)
	{
		printf("LGL_Sound::DivergeRecallPop(): WARNING! channel < 0\n");
		return(false);
	}

	if(LGL.SoundChannel[channel].DivergeCount==0)
	{
		printf("LGL_Sound::DivergeRecallPop(): WARNING! Attempting to pop more than you pushed!\n");
		return(false);
	}

	LGL.SoundChannel[channel].DivergeCount--;

	if
	(
		recall &&
		LGL.SoundChannel[channel].DivergeCount==0
	)
	{
		LGL.SoundChannel[channel].DivergeRecallSamples=LGL.SoundChannel[channel].DivergeSamples;
		LGL.SoundChannel[channel].DivergeRecallNow=true;
	}

	return(true);
}

int
LGL_Sound::
GetDivergeRecallCount
(
	int	channel
)
{
	if(channel<0)
	{
		printf("LGL_Sound::GetDivergeRecallCount(): WARNING! channel < 0\n");
		return(false);
	}
	return(LGL.SoundChannel[channel].DivergeCount);
}

bool
LGL_Sound::
GetWarpPointIsSet
(
	int	channel
)
{
	if(channel<0)
	{
		printf("LGL_Sound::GetWarpPointIsSet(): WARNING! channel < 0\n");
		return(false);
	}
	return(LGL.SoundChannel[channel].WarpPointSecondsAlpha>=0);
}

bool
LGL_Sound::
GetWarpPointIsLoop
(
	int	channel
)
{
if(channel<0)
{
	printf("LGL_Sound::GetWarpPointIsLoop(): WARNING! channel < 0\n");
	return(false);
}
	return(LGL.SoundChannel[channel].WarpPointLoop);
}

bool
LGL_Sound::
GetWarpPointIsLocked
(
	int	channel
)
{
if(channel<0)
{
	printf("LGL_Sound::GetWarpPointIsLocked(): WARNING! channel < 0\n");
	return(false);
}
	return(LGL.SoundChannel[channel].WarpPointLock);
}

bool
LGL_Sound::
SetWarpPoint
(
	int	channel
)
{
if(channel<0)
{
	printf("LGL_Sound::SetWarpPoint(1): WARNING! channel < 0\n");
	return(false);
}

	if(LGL.SoundChannel[channel].WarpPointLock==false)
	{
		LGL.SoundChannel[channel].WarpPointSecondsAlpha=-1.0f;
		LGL.SoundChannel[channel].WarpPointSecondsOmega=-1.0f;
		LGL.SoundChannel[channel].WarpPointLoop=false;
	}

	return(true);
}

bool
LGL_Sound::
SetWarpPoint
(
	int	channel,
	double	alphaSeconds,
	double	omegaSeconds,
	bool	loop,
	bool	lock
)
{
if(channel<0)
{
	printf("LGL_Sound::SetWarpPoint(3): WARNING! channel < 0\n");
	return(false);
}
	if(LGL.SoundChannel[channel].WarpPointLock==false)
	{
		LGL.SoundChannel[channel].WarpPointSecondsAlpha=alphaSeconds;
		LGL.SoundChannel[channel].WarpPointSecondsOmega=omegaSeconds;
		LGL.SoundChannel[channel].WarpPointLoop=loop;
		LGL.SoundChannel[channel].WarpPointLock=lock;
	}

	return(true);
}

float
LGL_Sound::
GetWarpPointSecondsAlpha
(
	int	channel
)
{
	return(LGL.SoundChannel[channel].WarpPointSecondsAlpha);
}

float
LGL_Sound::
GetWarpPointSecondsOmega
(
	int	channel
)
{
	return(LGL.SoundChannel[channel].WarpPointSecondsOmega);
}

void
LGL_Sound::
SetRhythmicInvertProperties
(
	int	channel,
	float	secondsAlpha,
	float	secondsDelta
)
{
	LGL.SoundChannel[channel].RhythmicInvertAlphaSamples=secondsAlpha*Hz;
	LGL.SoundChannel[channel].RhythmicInvertDeltaSamples=secondsDelta*Hz;
}

void
LGL_Sound::
SetRhythmicVolumeInvert
(
	int	channel,
	bool	rapidVolumeInvert
)
{
	LGL.SoundChannel[channel].RhythmicVolumeInvert=rapidVolumeInvert;
}

void
LGL_Sound::
SetRespondToRhythmicSoloInvertChannel
(
	int	channel,
	int	soloChannel
)
{
	LGL.SoundChannel[channel].RespondToRhythmicSoloInvertChannel=soloChannel;
}

int
LGL_Sound::
GetRespondToRhythmicSoloInvertCurrentValue
(
	int	channel
)
{
	return(LGL.SoundChannel[channel].RespondToRhythmicSoloInvertCurrentValue);
}

float
LGL_Sound::
GetVU
(
	int	channel
)	const
{
if(channel<0)
{
	printf("LGL_Sound::GetVU(): WARNING! channel < 0\n");
	return(0);
}
	return(LGL.SoundChannel[channel].VU);
}

float
LGL_Sound::
GetSample
(
	int	sample
)
{
	LockBufferForReading(4);

	if
	(
		//LGL.AudioAvailable==false ||
		//IsLoaded()==false
		Buffer==NULL ||
		BufferLength==0
	)
	{
		UnlockBufferForReading();
		return(.5);
	}

	int BPS=Channels*2;
	int r2offset=3%BPS;

	while((float)sample*BPS+r2offset>=(float)BufferLength) sample-=BufferLength/BPS;
	while(sample<0) sample+=BufferLength/BPS;

	Sint16* myBuffer=(Sint16*)Buffer;
	Sint16 myL=SWAP16(myBuffer[sample*BPS/2+0]);
	Sint16 myR=SWAP16(myBuffer[sample*BPS/2+1]);

	float myLf=(myL+32767)/65536.0;
	float myRf=(myR+32767)/65536.0;

	float ret=(myLf+myRf)/2.0;

	UnlockBufferForReading();

	return(ret);
}

float
LGL_Sound::
GetSampleLeft
(
	int	sample
)
{
	LockBufferForReading(5);

	if
	(
		//LGL.AudioAvailable==false ||
		//IsLoaded()==false
		Buffer==NULL ||
		BufferLength==0
	)
	{
		UnlockBufferForReading();
		return(.5);
	}

	int BPS=Channels*2;
	int r2offset=3%BPS;

	while((float)sample*BPS+r2offset>=(float)BufferLength) sample-=BufferLength/BPS;
	while(sample<0) sample+=BufferLength/BPS;

	Sint16* myBuffer=(Sint16*)Buffer;
	Sint16 myL=SWAP16(myBuffer[sample*BPS/2+0]);

	float myLf=(myL+32767)/65536.0;

	UnlockBufferForReading();

	return(myLf);
}

float
LGL_Sound::
GetSampleRight
(
	int	sample
)
{
	LockBufferForReading(6);

	if
	(
		//LGL.AudioAvailable==false ||
		//IsLoaded()==false
		Buffer==NULL ||
		BufferLength==0
	)
	{
		UnlockBufferForReading();
		return(.5);
	}

	int BPS=Channels*2;
	int r2offset=3%BPS;

	while((float)sample*BPS+r2offset>=(float)BufferLength) sample-=BufferLength/BPS;
	while(sample<0) sample+=BufferLength/BPS;

	Sint16* myBuffer=(Sint16*)Buffer;
	Sint16 myR=SWAP16(myBuffer[sample*BPS/2+1]);

	float myRf=(myR+32767)/65536.0;

	UnlockBufferForReading();

	return(myRf);
}

void
LGL_Sound::
LockBuffer()
{
	BufferSemaphore.Lock(__FILE__,__LINE__);
}

void
LGL_Sound::
UnlockBuffer()
{
	BufferSemaphore.Unlock();
}

void
LGL_Sound::
LockBufferForReading(int id)
{
	BufferReaderCountSemaphore.Lock(__FILE__,__LINE__);
	if(BufferReaderCount==0)
	{
		BufferSemaphore.Lock(__FILE__,__LINE__);
	}
	assert(BufferReaderCount>=0);
	BufferReaderCount++;
	BufferReaderCountSemaphore.Unlock();
}

void
LGL_Sound::
UnlockBufferForReading(int id)
{
	BufferReaderCountSemaphore.Lock(__FILE__,__LINE__);
	BufferReaderCount--;
	if(BufferReaderCount==0)
	{
		BufferSemaphore.Unlock();
	}
	//assert(BufferReaderCount>=0);
	if(BufferReaderCount<0)
	{
printf("LGL_Sound::UnlockBufferForReading(): Warning! Buffer wasn't locked in the first place!\n");
		BufferReaderCount=0;
	}
	BufferReaderCountSemaphore.Unlock();
}

Uint8*
LGL_Sound::
GetBuffer()
{
	assert(IsLoaded()==true || BufferSemaphore.IsLocked());
	return(Buffer);
}

unsigned long
LGL_Sound::
GetBufferLength()
{
	return(BufferLength);
}

float
LGL_Sound::
GetLengthSeconds()
{
	//if(LGL.AudioAvailable==false) return(0);
	return(BufferLength/(2.0f*Channels*Hz));
}

long
LGL_Sound::
GetLengthSamples()
{
	//FIXME: This fails for Flashbulb / Flight 404...
	//if(LGL.AudioAvailable==false) return(0);
	int BPS=Channels*2;
	return(BufferLength/BPS);
}

bool
LGL_Sound::
GetHogCPU()	const
{
	return(HogCPU);
}

void
LGL_Sound::
SetHogCPU
(
	bool	hogCPU
)
{
	HogCPU=hogCPU;
}

bool
LGL_Sound::
AnalyzeWaveSegment
(
	long		sampleFirst,
	long		sampleLast,
	float&		zeroCrossingFactor,
	float&		magnitudeAve,
	float&		magnitudeMax
)
{
	bool ret=false;
	LockBufferForReading(10);
	{
		const Sint16* buf16=(Sint16*)GetBuffer();
		unsigned long len16=(GetBufferLength()/2);
		bool loaded=IsLoaded();
		int hz=Hz;

		ret = lgl_analyze_wave_segment
		(
			sampleFirst,
			sampleLast,
			zeroCrossingFactor,
			magnitudeAve,
			magnitudeMax,
			buf16,
			len16,
			loaded,
			hz,
			Channels
		);
	}
	UnlockBufferForReading(10);

	return(ret);
}
	
bool
LGL_Sound::
GetMetadata
(
	float	secondsBegin,
	float	secondsEnd,
	float&	zeroCrossingFactor,
	float&	magnitudeAve,
	float&	magnitudeMax
)
{
	zeroCrossingFactor=0.0f;
	magnitudeAve=0.0f;
	magnitudeMax=0.0f;

	if(MetadataVolumePeak==0.0f)
	{
		return(false);
	}

	float lengthSeconds=GetLengthSeconds();
	float secondsDelta = secondsEnd-secondsBegin;
	if(IsLoaded())
	{
		if(secondsBegin<0)
		{
			//Protect against infini-looping
			if(secondsBegin+100*lengthSeconds>0)
			{
				while(secondsBegin<0)
				{
					secondsBegin+=lengthSeconds;
				}
				secondsEnd=secondsBegin+secondsDelta;
			}
		}
		else if(secondsBegin>lengthSeconds)
		{
			if(secondsBegin-100*lengthSeconds<0)
			{
				while(secondsBegin>lengthSeconds)
				{
					secondsBegin-=lengthSeconds;
				}
				secondsEnd=secondsBegin+secondsDelta;
			}
		}
	}

	secondsBegin = LGL_Clamp(0.0f,secondsBegin,lengthSeconds);
	secondsEnd = LGL_Clamp(0.0f,secondsEnd,lengthSeconds);
	if(secondsBegin==secondsEnd)
	{
		return(false);
	}

	float secondsAnalyzed=MetadataFilledSize/(float)LGL_SOUND_METADATA_ENTRIES_PER_SECOND;
	if(secondsEnd>secondsAnalyzed)
	{
/*
		AnalyzeWaveSegment
		(
			secondsBegin*Hz,
			secondsEnd*Hz,
			zeroCrossingFactor,
			magnitudeAve,
			magnitudeMax
//,entireWaveArray
		);
		return(true);
*/
	}

	float entriesUsed=0.0f;
	float entryDelta=1.0f/LGL_SOUND_METADATA_ENTRIES_PER_SECOND;
	for(float entryBegin=floorf(secondsBegin);entryBegin<secondsEnd;entryBegin+=entryDelta)
	{
		float entryEnd=entryBegin+entryDelta;
		float pctOverlap=1.0f;
		if(entryEnd<secondsBegin)
		{
			continue;
		}
		else if(entryBegin<secondsBegin)
		{
			pctOverlap=(entryEnd-secondsBegin)/entryDelta;
		}

		int entryIndex=(int)(entryBegin*LGL_SOUND_METADATA_ENTRIES_PER_SECOND);

		zeroCrossingFactor+=pctOverlap*MetadataFreqFactor[entryIndex];
		magnitudeAve+=pctOverlap*MetadataVolumeAve[entryIndex];

		if(magnitudeMax<MetadataVolumeMax[entryIndex])
		{
			magnitudeMax=MetadataVolumeMax[entryIndex];
		}

		entriesUsed+=pctOverlap;
	}

	if(entriesUsed!=0.0f)
	{
		zeroCrossingFactor/=entriesUsed;
		magnitudeAve/=entriesUsed;
	}

	return(true);
}

bool
LGL_Sound::
GetSilent()	const
{
	return(Silent);
}

float
LGL_Sound::
GetVolumePeak()
{
	if(Loaded || 1)
	{
		//Guard against divide-by-zero during John Cage - 4'33"
		return((MetadataVolumePeak>0) ? MetadataVolumePeak : 1.0f);
	}
	else
	{
		return(1.0f);
	}
}

void
LGL_Sound::
SetVolumePeak(float volumePeak)
{
	//Guard against stupid users
	MetadataVolumePeak=LGL_Clamp(0.0f,volumePeak,1.0f);
}

void
LGL_Sound::
LoadToMemory()
{
	//ffmpeg implementation

	if(LGL_FileExists(Path)==false)
	{
		printf("LGL_Sound::LoadToMemory(): Warning! File '%s' doesn't exist!\n",Path);
		BadFile=true;
		return;
	}

	if(LGL_FileLengthBytes(Path)<1024.0*1024.0*200)
	{
		//Preload!
		if(FILE* fd=fopen(Path,"rb"))
		{
			const int readSize=1024*1024*8;
			char* readBuf=new char[readSize];
			for(;;)
			{
				if(DestructorHint)
				{
					break;
				}
				if(feof(fd))
				{
					break;
				}
				fread(readBuf,readSize,1,fd);
			}

			delete readBuf;

			fclose(fd);
		}
		if(DestructorHint)
		{
			return;
		}
	}

	int totalFileBytes=-1;
	int totalFileBytesUsed=0;
	if(FILE* fd = fopen(Path,"rb"))
	{
		fseek(fd,0,SEEK_END);
		totalFileBytes=ftell(fd);
		fclose(fd);
	}
	else
	{
		printf("LGL_Sound::LoadToMemory(): Error! Couldn't fopen('%s')!\n",Path);
		BadFile=true;
		return;
	}

	AVFormatContext*	formatContext=NULL;
	AVCodecContext*		codecContext=NULL;
	AVCodec*		codec=NULL;

	int			audioStreamIndex=-1;

	{
		//Open file
		{
			if(lgl_av_open_input_file(&formatContext, Path, NULL, 0, NULL)!=0)
			{
				printf("LGL_Sound::LoadToMemory(): lgl_av_open_input_file() couldn't open '%s'\n",Path);
				BadFile=true;
				return;
			}
		}

		//Find streams
		if(lgl_av_find_stream_info(formatContext)<0)
		{
			printf("LGL_Sound::LoadToMemory(): Couldn't find streams for '%s'\n",Path);
			BadFile=true;
			lgl_av_close_input_file(formatContext);
			return;
		}

		// Find the first audio stream
		for(unsigned int i=0; i<formatContext->nb_streams; i++)
		{
			if(formatContext->streams[i]->codec->codec_type==CODEC_TYPE_AUDIO)
			{
				audioStreamIndex=i;
				break;
			}
		}

		/*
		if(audioStreamIndex==-1)
		{
			printf("LGL_Sound::LoadToMemory(): Couldn't find audio stream for '%s' in %i streams\n",Path,formatContext->nb_streams);
			BadFile=true;
			return;
		}
		*/

		if(audioStreamIndex!=-1)
		{
			// Get a pointer to the codec context for the audio stream
			codecContext=formatContext->streams[audioStreamIndex]->codec;

			// Find the decoder for the audio stream
			codec=lgl_avcodec_find_decoder(codecContext->codec_id);
			if(codec==NULL)
			{
				printf("LGL_Sound::LoadToMemory: Couldn't find audio codec for '%s'. Codec = '%s'\n",Path,codecContext->codec_name);
				BadFile=true;
				lgl_av_close_input_file(formatContext);
				return;
			}

			Channels=codecContext->channels;
			if
			(
				Channels!=1 &&
				Channels!=2 &&
				Channels!=4
			)
			{
				printf("LGL_Sound::LoadToMemory(): Invalid channel count found for '%s': %i\n",Path,Channels);
				BadFile=true;
				lgl_av_close_input_file(formatContext);
				return;
			}

			Hz=codecContext->sample_rate;

			// Open codec
			{
				if(lgl_avcodec_open(codecContext,codec)<0)
				{
					printf("LGL_Sound::LoadToMemory(): Couldn't open codec for '%s'\n",Path);
					BadFile=true;
					lgl_av_close_input_file(formatContext);
					return;
				}
			}
		}
	}
	if(audioStreamIndex==-1)
	{
		Hz=44100;
		float lengthSeconds=formatContext->duration/(float)(AV_TIME_BASE);
		BufferLength=lengthSeconds*Hz*2*2;
		if(BufferLength>BufferLengthTotal) BufferLength=BufferLengthTotal;
		bzero(Buffer,BufferLength);
		MetadataVolumePeak=0.0f;
		Silent=true;
	}
	else
	{
		// Inform the codec that we can handle truncated bitstreams -- i.e.,
		// bitstreams where frame boundaries can fall in the middle of packets
		if(codec->capabilities & CODEC_CAP_TRUNCATED)
		{
			codecContext->flags|=CODEC_FLAG_TRUNCATED;
		}
	    
		int cyclesNow=0;
		int cyclesMax=256;//64;
		int delayMS=1;

		//Don't be tempted to make this a member variable... Alignment issues!
		int16_t* outbuf=new int16_t[LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE];

		for(;;)
		{
			if(DestructorHint)
			{
				break;
			}
		
			AVPacket packet;
			lgl_av_init_packet(&packet);

			int result=0;
			{
				result = lgl_av_read_frame(formatContext, &packet);
			}

			int packetOrigSize=packet.size;
			uint8_t* packetOrigData=packet.data;

			bool eof=false;
			if(result<0)
			{
				eof=true;
				packet.size=0;
				packet.data=NULL;
			}

			bool freePacket=result>=0;

			while
			(
				(
					result>=0 &&
					packet.size>0
				) ||
				eof
			)
			{
				/*
				//chill
				while
				(
					LGL.MainThreadVsyncWait==false &&
					LGL.Running &&
					DestructorHint==false
				)
				{
					//Meh. I don't mind audio decoding to happen always.
					//LGL_DelayMS(1);
				}
				*/
				totalFileBytesUsed+=packet.size;
				// Is this a packet from the audio stream?
				if(packet.stream_index==audioStreamIndex)
				{
					// Decode audio frame
					int outbufsize = LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE;
					{
						int ret = lgl_avcodec_decode_audio3	//FIXME: This undercounts totalFileBytesUsed...
						(
							codecContext,
							(int16_t*)outbuf,
							&outbufsize,
							&packet
						);
						if(outbufsize==0)
						{
							eof=false;
						}

						if(ret<0)
						{
							//printf("lgl_avcodec_decode_audio3(): Error!\n");
							//result=-1;
							//BadFile=true;
							break;
						}
						packet.size-=ret;
						packet.data+=ret;
					}

					PercentLoaded = totalFileBytesUsed/(float)totalFileBytes;

					if(BufferLength+outbufsize < BufferLengthTotal)
					{
						//There's enough room at the end of Buffer

						//Copy Extra Bits To Buffer
						/*if(Channels==1)
						{
							int16_t* outbuf16 = (int16_t*)outbuf; 
							int16_t* buf16 = (int16_t*)Buffer;
							int samples=outbufsize/2;

							for(int a=0;a<samples;a++)
							{
								buf16[BufferLength/2+2*a+0]=outbuf16[a];
								buf16[BufferLength/2+2*a+1]=outbuf16[a];
							}
							BufferLength+=samples*2*2;
						}
						else*/ if
						(
							Channels==1 ||
							Channels==2 ||
							Channels==4
						)
						{
							memcpy(Buffer+BufferLength,(char*)outbuf,outbufsize);
							BufferLength+=outbufsize;
						}

						for(int b=0;b<LGL_SOUND_CHANNEL_NUMBER;b++)
						{
							LGL_SoundChannel* sc=&LGL.SoundChannel[b];
							if
							(
								sc->Occupied &&
								Buffer!=NULL &&
								sc->Buffer==Buffer
							)
							{
								sc->BufferLength=BufferLength;
								sc->LengthSamples=BufferLength/4;	//This assumes stereo... Bad!
							}
						}
					}
					else
					{
						//Too big!
						result=-1;
						break;
					}

					float secondsLoaded=GetLengthSeconds();
					float secondsMetadataAlpha=MetadataFilledSize/(float)LGL_SOUND_METADATA_ENTRIES_PER_SECOND;
					float secondsMetadataDelta=1.0f/(float)LGL_SOUND_METADATA_ENTRIES_PER_SECOND;
					while(secondsMetadataAlpha+secondsMetadataDelta<secondsLoaded)
					{
						long sampleFirst=secondsMetadataAlpha*Hz;
						long sampleLast=(secondsMetadataAlpha+secondsMetadataDelta)*Hz;
						bool ret = AnalyzeWaveSegment
						(
							sampleFirst,
							sampleLast,
							MetadataFreqFactor[MetadataFilledSize],
							MetadataVolumeAve[MetadataFilledSize],
							MetadataVolumeMax[MetadataFilledSize]
						);
						if(ret==false)
						{
							break;
						}
						if(MetadataVolumeMax[MetadataFilledSize]>MetadataVolumePeak)
						{
							MetadataVolumePeak=MetadataVolumeMax[MetadataFilledSize];
						}
						MetadataFilledSize++;
						secondsMetadataAlpha=MetadataFilledSize/(float)LGL_SOUND_METADATA_ENTRIES_PER_SECOND;
					}
				}
				else
				{
					//Thread priority only applies to CPU... Don't hog the HDD!
					LGL_DelayMS(delayMS);
					break;
				}

			}

			if(freePacket)
			{
				// Free the data in the packet that was allocated by lgl_av_read_frame (but not the packet object itself)
				packet.size=packetOrigSize;
				packet.data=packetOrigData;
				lgl_av_free_packet(&packet);
			}

			if(result<0)
			{
				break;
			}

			cyclesNow++;
			if(cyclesNow>=cyclesMax)
			{
				cyclesNow=0;
				if(HogCPU==false)
				{
					//This is a hack. I don't want to delay. But the scheduler won't preempt me for higher priority processes, for some reason....
					LGL_DelayMS(delayMS);
				}
			}
		}

		delete outbuf;
		outbuf=NULL;
	}

	Loaded=true;
	LoadedMin=1;
	PercentLoaded=1.0f;
	
	{
		if(codecContext) lgl_avcodec_close(codecContext);
		lgl_av_close_input_file(formatContext);
	}
}

bool
LGL_Sound::
IsLoaded()
{
	//if(LGL.AudioAvailable==false) return(true);
	return(Loaded);
}

bool
LGL_Sound::
IsUnloadable()
{
	return(BadFile);
}

float
LGL_Sound::
GetPercentLoaded()
{
	if(Loaded) return(1.0f);
	return(PercentLoaded);
}

float
LGL_Sound::
GetPercentLoadedSmooth()
{
	if(Loaded) return(1.0f);
	if(BadFile)
	{
		return(0.0f);
	}
	//if(LGL.AudioAvailable==false) return(1);
#ifdef	LGL_WIN32
	return(.5);
#endif	//LGL_WIN32
	if(LoadedSmoothTime!=LGL_SecondsSinceExecution())
	{
		LoadedSmoothTime=LGL_SecondsSinceExecution();
		float s=LGL_SecondsSinceLastFrame()*10;
		if(s>1) s=1;
		LoadedSmooth=(1.0-s)*LoadedSmooth+s*GetPercentLoaded();
	}
	if(IsLoaded())
	{
		return(1);
	}
	else
	{
		return(LoadedSmooth);
	}
}

float
LGL_Sound::
GetSecondsUntilLoaded()
{
	float pct=GetPercentLoaded();
	if
	(
		BadFile ||
		pct<0.0001f ||
		LoadTimerDeltaTimer.SecondsSinceLastReset()<1.0f
	)
	{
		return(LoadSecondsUntilLoaded);
	}
	LoadTimerDeltaTimer.Reset();
	//if(LGL.AudioAvailable==false) return(0);
#ifdef	LGL_WIN32
	return(20);
#endif	//LGL_WIN32
	float r=LoadTimer.SecondsSinceLastReset();
	float t=r/pct-r;
	if(r<1)
	{
		return(t);
	}
	else
	{
		LoadSecondsUntilLoaded=t;
		return(LoadSecondsUntilLoaded);
	}
}

void
LGL_Sound::
MaybeSwapBuffers()
{
	if(BufferBackReadyForReading)
	{
		Uint8* tmpBuf=Buffer;
		Buffer=BufferBack;
		BufferBack=tmpBuf;

		unsigned long tmpLen=BufferLength;
		BufferLength=BufferBackLength;
		BufferBackLength=tmpLen;

		tmpLen=BufferLengthTotal;
		BufferLengthTotal=BufferBackLengthTotal;
		BufferBackLengthTotal=tmpLen;

		//Ensure active channels point to correct buffer
		for(int b=0;b<LGL_SOUND_CHANNEL_NUMBER;b++)
		{
			LGL_SoundChannel* sc=&LGL.SoundChannel[b];
			if(sc->Buffer==BufferBack)
			{
				sc->Buffer=Buffer;
				sc->BufferLength=BufferLength;
			}
		}

		if(BufferAllocatedFromElsewhere)
		{
			BufferBack=NULL;
			BufferBackLength=0;
			BufferBackLengthTotal=0;
			BufferAllocatedFromElsewhere=false;	//This looks suspect...
		}

		BufferBackReadyForReading=false;
		BufferBackReadyForWriting=true;
	}
}

int
LGL_Sound::
GetChannelCount()
{
	return(Channels);
}

int
LGL_Sound::
GetHz()
{
	return(Hz);
}

int
LGL_Sound::
GetReferenceCount()
{
	return(ReferenceCount);
}

void
LGL_Sound::
IncrementReferenceCount()
{
	ReferenceCount++;
}

void
LGL_Sound::
DecrementReferenceCount()
{
	ReferenceCount--;
	assert(ReferenceCount>=0);
}

const
char*
LGL_Sound::
GetPath()	const
{
	return(Path);
}

const
char*
LGL_Sound::
GetPathShort()	const
{
	return(PathShort);
}

//

bool
LGL_AudioAvailable()
{
	return(LGL.AudioAvailable);
}

bool
LGL_AudioWasOnceAvailable()
{
	return(LGL.AudioWasOnceAvailable);
}

bool
LGL_AttemptAudioRevive()
{
	if(LGL_AudioAvailable()) return(true);
	if(LGL.AudioUsingJack)
	{
		return(LGL_JackInit());
	}
	else
	{
		return(false);
	}
}

bool
LGL_AudioUsingJack()
{
	return(LGL.AudioUsingJack);
}

bool
LGL_AudioJackXrun()
{
	return(LGL.AudioJackXrunFront);
}

bool
LGL_AudioIsRealtime()
{
	return(LGL.AudioUsingJack && jack_client && jack_is_realtime(jack_client));
}

int
LGL_AudioRate()
{
	return(LGL.AudioSpec->freq);
}

int
LGL_AudioCallbackSamples()
{
	return(LGL_SAMPLESIZE_SDL);
}

float
LGL_AudioSampleLeft
(
	int	sample
)
{
	if(LGL.AudioAvailable==false)
	{
		return(.5);
	}

	while(sample<0)
	{
		printf("LGL_AudioSampleLeft(): Warning, you asked for a sample < 0 (%i). Baka.\n",sample);	
		sample+=1024;
	}
	if(sample>1024)
	{
		sample=sample%1024;;
	}

	return(LGL.AudioBufferLFront[sample]);
}

float*
LGL_AudioSampleLeftArray()
{
	return(LGL.AudioBufferLFront);
}

float
LGL_AudioSampleRight
(
	int	sample
)
{
	if(LGL.AudioAvailable==false)
	{
		return(.5);
	}

	while(sample<0)
	{
		printf("LGL_AudioSampleRight(): Warning, you asked for a sample < 0 (%i)\n",sample);	
		sample+=1024;
	}
	if(sample>1024)
	{
		sample=sample%1024;;
	}

	return(LGL.AudioBufferRFront[sample]);
}

float*
LGL_AudioSampleRightArray()
{
	return(LGL.AudioBufferRFront);
}

float
LGL_AudioSampleMono
(
	int	sample
)
{
	//return((LGL_AudioSampleLeft(sample)+LGL_AudioSampleRight(sample))/2.0);
	if(sample%2==0)
	{
		return(LGL_AudioSampleLeft(sample));
	}
	else
	{
		return(LGL_AudioSampleRight(sample));
	}
}

float*
LGL_AudioSampleMonoArray()
{
	return(LGL.AudioBufferLFront);
}

int
LGL_AudioSampleArraySize()
{
	return(1024);
}

float
LGL_AudioPeakLeft()
{
	return(LGL.AudioPeakLeft);
}

float
LGL_AudioPeakRight()
{
	return(LGL.AudioPeakRight);
}

float
LGL_AudioPeakMono()
{
	return(LGL.AudioPeakMono);
}

float
LGL_FreqL
(
	float	freq
)
{
	float indexF=512-freq*1024.0/44100.0f;
	int indexL=(int)floor(indexF)-1;
	int indexH=(int)ceil(indexF)-1;
	float valL=(indexL>=2 && indexL<512)?LGL.FreqBufferL[indexL]:0;
	float valH=(indexH>=2 && indexH<512)?LGL.FreqBufferL[indexH]:0;
	return(LGL_Interpolate(valL,valH,indexF-floor(indexF)));
}

float
LGL_FreqR
(
	float	freq
)
{
	float indexF=512-freq*1024.0/44100.0f;
	int indexL=(int)floor(indexF)-1;
	int indexH=(int)ceil(indexF)-1;
	float valL=(indexL>=0 && indexL<512)?LGL.FreqBufferR[indexL]:0;
	float valH=(indexH>=0 && indexH<512)?LGL.FreqBufferR[indexH]:0;
	return(LGL_Interpolate(valL,valH,indexF-floor(indexF)));
}

float
LGL_FreqMono
(
	float	freq
)
{
	return(.5*(LGL_FreqL(freq)+LGL_FreqR(freq)));
}

float
LGL_FreqBufferL
(
	int	index,
	int	width
)
{
	index=(int)LGL_Clamp(0,index,511);
	if(width==0)
	{
		return(LGL.FreqBufferL[index]);
	}
	else
	{
		float num=1+2*width;
		int low=(int)LGL_Clamp(0,index-width,511);
		int high=(int)LGL_Clamp(0,index+width,511);
		float total=0;
		for(int a=low;a<=high;a++)
		{
			total+=LGL_FreqBufferL(a);
		}
		total/=num;
		return(LGL_Max(total,LGL.FreqBufferL[index]));
	}
}

float
LGL_FreqBufferR
(
	int	index,
	int	width
)
{
	index=(int)LGL_Clamp(0,index,511);
	if(width==0)
	{
		return(LGL.FreqBufferR[index]);
	}
	else
	{
		float num=1+2*width;
		int low=(int)LGL_Clamp(0,index-width,511);
		int high=(int)LGL_Clamp(0,index+width,511);
		float total=0;
		for(int a=low;a<=high;a++)
		{
			total+=LGL_FreqBufferR(a);
		}
		total/=num;
		return(total);//LGL_Max(total,LGL.FreqBufferR[index]));
	}
}
float
LGL_FreqBufferMono
(
	int	index,
	int	width
)
{
	return(.5*(LGL_FreqBufferL(index,width)+LGL_FreqBufferR(index,width)));
}

int
LGL_GetRecordDVJToFile()
{
	if(LGL.AudioEncoder)
	{
		if(LGL.AudioEncoder->IsValid())
		{
			return(1);
		}
		else
		{
			return(-1);
		}
	}
	else
	{
		return(0);
	}
}

void
LGL_RecordDVJToFileStart
(
	const char*	path,
	bool		surroundMode
)
{	
	if(LGL.AudioEncoder==NULL)
	{
		//Actually start recording
		strcpy(LGL.AudioEncoderPath,path);
		LGL.AudioEncoderSemaphore = new LGL_Semaphore("AudioEncoderSemaphore");
		LGL.AudioEncoder = new LGL_AudioEncoder
		(
			LGL.AudioEncoderPath,
			surroundMode
		);

		char* neo=new char[1024];
		sprintf(neo,"!dvj::Record.mp3|%s\n",LGL.AudioEncoderPath);
		LGL.DrawLog.push_back(neo);
	}
}

const char*
LGL_RecordDVJToFilePath()
{
	return(LGL.AudioEncoderPath);
}

const char*
LGL_RecordDVJToFilePathShort()
{
	if(char* slash = strrchr(LGL.AudioEncoderPath,'/'))
	{
		return(&(slash[1]));
	}

	return(LGL_RecordDVJToFilePath());
}

void
LGL_SetRecordDVJToFileVolume
(
	float	volume
)
{
	LGL.RecordVolume = LGL_Clamp(0.0f,volume,1.0f);
}

void
LGL_AudioMasterToHeadphones
(
	bool	copy
)
{
	LGL.AudioMasterToHeadphones=copy;
}

//Input

void
LGL_ProcessInput()
{
	//Handle Wiimotes
	bool listenerFound=false;
	{
		LGL_ScopeLock lock(__FILE__,__LINE__,LGL.WiimoteSemaphore);
		for(int a=0;a<2;a++)	//Limit Wiimotes to two, for now.
		{
			if(LGL.Wiimote[a].Connected())
			{
				LGL.Wiimote[a].INTERNAL_ProcessInput();
			}
			else if(listenerFound==false)
			{
				if(LGL.Wiimote[a].IsListeningForConnection()==false)
				{
					LGL.Wiimote[a].ListenForConnection();
				}
				listenerFound=true;
			}
		}
	}

	//Record a few things for later VidCam processing

	int a;
	
	bool OldThumbCross=false;
	bool OldThumbCircle=false;
	if(LGL_VidCamAvailable())
	{
		OldThumbCross=LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_CROSS];
		OldThumbCircle=LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_CIRCLE];
	}
	if(OldThumbCross && OldThumbCircle)
	{
		if(LGL_RandFloat()<=.5)
		{
			OldThumbCross=true;
			OldThumbCircle=false;
		}
		else
		{
			OldThumbCross=false;
			OldThumbCircle=true;
		}
	}

	//Reset Keyboard State
	
	for(a=0;a<512;a++)
	{
		LGL.KeyStroke[a]=false;
		LGL.KeyRelease[a]=false;
	}

	//Reset mouse State
	
	LGL.MouseDX=0.0f;
	LGL.MouseDY=0.0f;
	for(a=0;a<3;a++)
	{
		LGL.MouseStroke[a]=false;
		LGL.MouseRelease[a]=false;
	}
	for(int a=0;a<8;a++)
	{
		if(LGL_GetWiimote(a).Connected())
		{
			float pointerX = LGL_Clamp(0.0f,LGL_GetWiimote(a).GetPointerX(),1.0f);
			float pointerY = LGL_Clamp(0.0f,LGL_GetWiimote(a).GetPointerY(),1.0f);
			LGL.MouseDX = pointerX - LGL.MouseX;
			LGL.MouseDY = pointerY - LGL.MouseY;
			LGL.MouseX = pointerX;
			LGL.MouseY = pointerY;
			break;
		}
	}

	//Reset MultiTouch State

	float multiTouchXPrev=LGL.MultiTouchX;
	float multiTouchYPrev=LGL.MultiTouchY;
	int multiTouchFingerCountPrev=LGL.MultiTouchFingerCount;

	LGL.MultiTouchDX=0.0f;
	LGL.MultiTouchDY=0.0f;
	LGL.MultiTouchRotate=0.0f;
	LGL.MultiTouchPinch=0.0f;
	LGL.MultiTouchFingerCountDelta=0;
	/*
	//This never seems to happen
	if(SDL_Touch* touch = SDL_GetTouch(LGL.MultiTouchID))
	{
printf("touch focus: %x\n",touch->focus);
		LGL.MultiTouchFingerCount=touch->num_fingers;
		LGL.MultiTouchFingerCountDelta=LGL.MultiTouchFingerCount-multiTouchFingerCountPrev;
		if
		(
			multiTouchFingerCountPrev>=2 &&
			LGL.MultiTouchFingerCount<2
		)
		{
			LGL.MultiTouchX=-1.0f;
			LGL.MultiTouchY=-1.0f;
			LGL.MultiTouchXFirst=LGL.MultiTouchX;
			LGL.MultiTouchYFirst=LGL.MultiTouchY;
		}
	}
	*/



	//Reset Joystick State
	
	for(a=0;a<4;a++)
	{
		for(int b=0;b<16;b++)
		{
			LGL.JoyStroke[a][b]=false;
			LGL.JoyRelease[a][b]=false;
		}
	}
	
	SDL_Event event;

	for(a=0;a<256;a++)
	{
		LGL.KeyStream[a]='\0';
	}
	int StreamCounter=0;

	while(SDL_PollEvent(&event))
	{
		if(event.type==SDL_QUIT)
		{
			LGL_Exit();
		}

		if(event.type==SDL_WINDOWEVENT)
		{
			if(event.window.event == SDL_WINDOWEVENT_CLOSE)
			{
				LGL_Exit();
			}
		}
		
		//Keyboard
		if(event.type==SDL_KEYDOWN)
		{
			if(event.key.windowID!=SDL_GetWindowID(LGL.WindowID[0]))
			{
				//Only care about events for interface window
				continue;
			}

			if(event.key.keysym.sym > 256)
			{
				event.key.keysym.sym = 256 + (event.key.keysym.sym % LGL_KEY_MAX);
			}
			if(event.key.keysym.sym > 512)
			{
				event.key.keysym.sym=0;
			}

			if(event.key.keysym.sym != 0)
			{
//printf("KeyDown: %i (%i, %i)\n",event.key.keysym.sym, LGL_KEY_RALT);
				LGL.KeyStroke[event.key.keysym.sym]=LGL.KeyDown[event.key.keysym.sym]==false;
				LGL.KeyDown[event.key.keysym.sym]=true;
				if(LGL.KeyStroke[event.key.keysym.sym])
				{
					LGL.KeyTimer[event.key.keysym.sym].Reset();
				}

				if(event.key.keysym.sym==LGL_KEY_BACKSPACE)
				{
					//OSX Backspace ("delete" as they call it...)
					event.key.keysym.unicode=event.key.keysym.sym;
				}


				if
				(
					StreamCounter<255 &&
					event.key.keysym.unicode<0x80 &&
					event.key.keysym.unicode>0
				)
				{
					if(isalpha((char)event.key.keysym.unicode))
					{
						if(SDL_GetModState() & KMOD_SHIFT)
						{
							LGL.KeyStream[StreamCounter]=toupper((char)event.key.keysym.unicode);
						}
						else
						{
							LGL.KeyStream[StreamCounter]=tolower((char)event.key.keysym.unicode);
						}
					}
					else
					{
						LGL.KeyStream[StreamCounter]=(char)event.key.keysym.unicode;
					}
					
					StreamCounter++;
					LGL.KeyStream[StreamCounter]='\0';
				}
			}
		}
		if(event.type==SDL_KEYUP)
		{
			if(event.key.windowID!=SDL_GetWindowID(LGL.WindowID[0]))
			{
				//Only care about events for interface window
				continue;
			}

			if(event.key.keysym.sym > 256)
			{
				event.key.keysym.sym = 256 + (event.key.keysym.sym % LGL_KEY_MAX);
			}
			LGL.KeyDown[event.key.keysym.sym]=false;
			LGL.KeyRelease[event.key.keysym.sym]=true;
			LGL.KeyTimer[event.key.keysym.sym].Reset();
		}
		if(event.type==SDL_TEXTINPUT)
		{
			SDL_TextInputEvent* input = (SDL_TextInputEvent*)(&(event));
			LGL.KeyStream[StreamCounter]=input->text[0];
			StreamCounter++;
			LGL.KeyStream[StreamCounter]='\0';
		}

		//Mouse

		if(event.type==SDL_MOUSEMOTION)
		{
			if(event.motion.windowID!=SDL_GetWindowID(LGL.WindowID[0]))
			{
				//Only care about events for interface window
				continue;
			}

			bool wiimotePointerOverride=false;
			for(int a=0;a<8;a++)
			{
				if(LGL_GetWiimote(a).GetPointerAvailable())
				{
					LGL.MouseDX = LGL_GetWiimote(a).GetPointerX() - LGL.MouseX;
					LGL.MouseDY = LGL_GetWiimote(a).GetPointerY() - LGL.MouseY;
					LGL.MouseX = LGL_GetWiimote(a).GetPointerX();
					LGL.MouseY = LGL_GetWiimote(a).GetPointerY();
					wiimotePointerOverride=true;
					break;
				}
			}
			if(wiimotePointerOverride==false)
			{
				LGL.MouseDX=event.motion.x/(float)LGL.WindowResolutionX[LGL.DisplayNow]-LGL.MouseX;
				LGL.MouseDY=(1.0-event.motion.y/(float)LGL.WindowResolutionY[LGL.DisplayNow])-LGL.MouseY;
				LGL.MouseX=event.motion.x/(float)LGL.WindowResolutionX[LGL.DisplayNow];
				LGL.MouseY=(1.0-event.motion.y/(float)LGL.WindowResolutionY[LGL.DisplayNow]);
//printf("Mouse! %.2f, %.2f (%i)\n",LGL.MouseX,LGL.MouseY,SDL_GetMouseFocus());
			}
		}

		if(event.type==SDL_MOUSEBUTTONDOWN)
		{
			if(event.button.windowID!=SDL_GetWindowID(LGL.WindowID[0]))
			{
				//Only care about events for interface window
				continue;
			}

			if(event.button.button==SDL_BUTTON_LEFT)
			{
				LGL.MouseDown[LGL_MOUSE_LEFT]=true;
				LGL.MouseStroke[LGL_MOUSE_LEFT]=true;
				LGL.MouseTimer[LGL_MOUSE_LEFT].Reset();
			}
			if(event.button.button==SDL_BUTTON_MIDDLE)
			{
				LGL.MouseDown[LGL_MOUSE_MIDDLE]=true;
				LGL.MouseStroke[LGL_MOUSE_MIDDLE]=true;
				LGL.MouseTimer[LGL_MOUSE_MIDDLE].Reset();
			}
			if(event.button.button==SDL_BUTTON_RIGHT)
			{
				LGL.MouseDown[LGL_MOUSE_RIGHT]=true;
				LGL.MouseStroke[LGL_MOUSE_RIGHT]=true;
				LGL.MouseTimer[LGL_MOUSE_RIGHT].Reset();
			}
		}
		
		if(event.type==SDL_MOUSEBUTTONUP)
		{
			if(event.button.windowID!=SDL_GetWindowID(LGL.WindowID[0]))
			{
				//Only care about events for interface window
				continue;
			}

			if(event.button.button==SDL_BUTTON_LEFT)
			{
				LGL.MouseDown[LGL_MOUSE_LEFT]=false;
				LGL.MouseRelease[LGL_MOUSE_LEFT]=true;
				LGL.MouseTimer[LGL_MOUSE_LEFT].Reset();
			}
			if(event.button.button==SDL_BUTTON_MIDDLE)
			{
				LGL.MouseDown[LGL_MOUSE_MIDDLE]=false;
				LGL.MouseRelease[LGL_MOUSE_MIDDLE]=true;
				LGL.MouseTimer[LGL_MOUSE_MIDDLE].Reset();
			}
			if(event.button.button==SDL_BUTTON_RIGHT)
			{
				LGL.MouseDown[LGL_MOUSE_RIGHT]=false;
				LGL.MouseRelease[LGL_MOUSE_RIGHT]=true;
				LGL.MouseTimer[LGL_MOUSE_RIGHT].Reset();
			}
		}

		//MultiTouch

		//These should be unnecessary, as SDL_MULTIGESTURE should handle them.
		bool deltaFinger=false;
		if(event.type==SDL_FINGERDOWN)
		{
			LGL.MultiTouchFingerCount++;
			deltaFinger=true;
		}
		if(event.type==SDL_FINGERUP)
		{
			LGL.MultiTouchFingerCount--;
			deltaFinger=true;
		}
		if(deltaFinger)
		{
			LGL.MultiTouchX=-1.0f;
			LGL.MultiTouchY=-1.0f;
			multiTouchXPrev=-1.0f;
			multiTouchYPrev=-1.0f;
		}

		if(event.type==SDL_MULTIGESTURE)
		{
			#if 0
			if(SDL_GetWindowID((SDL_WindowID)event.mgesture.windowID)!=SDL_GetWindowID(LGL.WindowID[0]))
			{
printf("Multitouch OUT: %x vs %x\n",SDL_GetWindowID((SDL_WindowID)event.mgesture.windowID),SDL_GetWindowID(LGL.WindowID[0]));
				//Only care about events for interface window
				continue;
			}
			#endif

			LGL.MultiTouchID=event.mgesture.touchId;
			if(multiTouchXPrev!=-1.0f)
			{
				LGL.MultiTouchDX=event.mgesture.x-multiTouchXPrev;
				LGL.MultiTouchDY=(1.0f-event.mgesture.y)-multiTouchYPrev;
			}
			LGL.MultiTouchX=event.mgesture.x;
			LGL.MultiTouchY=(1.0f-event.mgesture.y);
			if(multiTouchXPrev==-1.0f)
			{
				LGL.MultiTouchXFirst=LGL.MultiTouchX;
				LGL.MultiTouchYFirst=LGL.MultiTouchY;
			}
			LGL.MultiTouchRotate=event.mgesture.dTheta;
			LGL.MultiTouchPinch=event.mgesture.dDist;
			LGL.MultiTouchFingerCount=event.mgesture.numFingers;
			LGL.MultiTouchFingerCountDelta=event.mgesture.numFingers-multiTouchFingerCountPrev;
		}

		//Joysticks
		if(event.type==SDL_JOYAXISMOTION)
		{
			int w=event.jaxis.which;
			int stick=-1;
			char stickstr[64];
			int axis=-1;
			char axisstr[64];

			//Set Stick, Axis
#ifdef	LGL_OSX
			if(event.jaxis.axis>=4)
			{
				w++;
			}
			if(event.jaxis.axis>=8)
			{
				w++;
			}
			if(event.jaxis.axis>=12)
			{
				w++;
			}
			
			if(event.jaxis.axis%4<=1)
			{
				stick=LGL_JOY_ANALOGUE_L;
				sprintf(stickstr,"LGL_JOY_ANALOGUE_L");
			}
			else
			{
				stick=LGL_JOY_ANALOGUE_R;
				sprintf(stickstr,"LGL_JOY_ANALOGUE_R");
			}

			if(event.jaxis.axis%2==0)
			{
				axis=LGL_JOY_XAXIS;
				sprintf(axisstr,"LGL_JOY_XAXIS");
			}
			else
			{
				axis=LGL_JOY_YAXIS;
				sprintf(axisstr,"LGL_JOY_YAXIS");
			}
#else	//LGL_OSX
			if
			(
				event.jaxis.axis==0 ||
				event.jaxis.axis==1
			)
			{
				stick=LGL_JOY_ANALOGUE_L;
				sprintf(stickstr,"LGL_JOY_ANALOGUE_L");
			}

			if
			(
				event.jaxis.axis==2 ||
				event.jaxis.axis==3
			)
			{
				stick=LGL_JOY_ANALOGUE_R;
				sprintf(stickstr,"LGL_JOY_ANALOGUE_R");
			}

			if
			(
				event.jaxis.axis==4 ||
				event.jaxis.axis==5
			)
			{
				stick=LGL_JOY_DPAD;
				sprintf(stickstr,"LGL_JOY_DPAD");
			}

			if
			(
				event.jaxis.axis==0 ||
				event.jaxis.axis==2 ||
				event.jaxis.axis==4
			)
			{
				axis=LGL_JOY_XAXIS;
				sprintf(axisstr,"LGL_JOY_XAXIS");
			}
			else
			{
				axis=LGL_JOY_YAXIS;
				sprintf(axisstr,"LGL_JOY_YAXIS");
			}
#endif	//LGL_OSX

			//Set val

			float v=event.jaxis.value;

			float JOY_ANALOGUE_THREASHOLD=3000;
			if(fabs(v)<JOY_ANALOGUE_THREASHOLD)
			{
				v=0;
			}
			else
			{
				if(v<0)
				{
					v=
					(
						32767*(v+JOY_ANALOGUE_THREASHOLD)/(32767.0-JOY_ANALOGUE_THREASHOLD)
					);
				}
				else
				{
					v=
					(
						32767*(v-JOY_ANALOGUE_THREASHOLD)/(32767.0-JOY_ANALOGUE_THREASHOLD)
					);
				}
			}

			v=v/32767.0;
			if(axis==LGL_JOY_YAXIS)
			{
				v*=-1;
			}
				
			if(stick!=LGL_JOY_DPAD)
			{
				LGL.JoyAnalogue[w][stick][axis]=v;
			}
			else
			{
				int Neg=-1;
				int Pos=-1;
				if(axis==LGL_JOY_XAXIS)
				{
					Neg=LGL_JOY_LEFT;
					Pos=LGL_JOY_RIGHT;
				}
				if(axis==LGL_JOY_YAXIS)
				{
					Neg=LGL_JOY_DOWN;
					Pos=LGL_JOY_UP;
				}

				if(v<0)
				{
					LGL.JoyDown[w][Neg]=true;
					LGL.JoyStroke[w][Neg]=true;

					if(LGL.JoyDown[w][Pos])
					{
						LGL.JoyRelease[w][Pos]=true;
					}
					LGL.JoyDown[w][Pos]=false;
				}
				if(v==0)
				{
					if(LGL.JoyDown[w][Neg])
					{
						LGL.JoyRelease[w][Neg]=true;
					}
					LGL.JoyDown[w][Neg]=false;
					
					if(LGL.JoyDown[w][Pos])
					{
						LGL.JoyRelease[w][Pos]=true;
					}
					LGL.JoyDown[w][Pos]=false;
				}
				if(v>0)
				{
					if(LGL.JoyDown[w][Neg])
					{
						LGL.JoyRelease[w][Neg]=true;
					}
					LGL.JoyDown[w][Neg]=false;
					
					LGL.JoyDown[w][Pos]=true;
					LGL.JoyStroke[w][Pos]=true;
				}
			}
/*
			if(event.jaxis.value!=0)
			{
				printf
				(
					"AXIS: (%i, %i) => C-%i.%s.%s\n",
					event.jaxis.which,
					event.jaxis.axis,
					w,
					stickstr,
					axisstr
				);
			}
*/
		}
		if(event.type==SDL_JOYHATMOTION)
		{
			/*
			printf
			(
				"Holy bats, hatman! %i, %i, %i\n",
				event.jhat.which,
				event.jhat.hat,
				event.jhat.value
			);
			*/

			int w=event.jhat.which;
			int v=event.jhat.value;

#ifdef	LGL_OSX
			w=event.jhat.hat;
#endif	//LGL_OSX

			if(v & SDL_HAT_LEFT)
			{
				LGL.JoyStroke[w][LGL_JOY_LEFT]=true;
				LGL.JoyDown[w][LGL_JOY_LEFT]=true;
			}
			else if(LGL.JoyDown[w][LGL_JOY_LEFT])
			{
				LGL.JoyRelease[w][LGL_JOY_LEFT]=true;
				LGL.JoyDown[w][LGL_JOY_LEFT]=false;
			}
			
			if(v & SDL_HAT_RIGHT)
			{
				LGL.JoyStroke[w][LGL_JOY_RIGHT]=true;
				LGL.JoyDown[w][LGL_JOY_RIGHT]=true;
			}
			else if(LGL.JoyDown[w][LGL_JOY_RIGHT])
			{
				LGL.JoyRelease[w][LGL_JOY_RIGHT]=true;
				LGL.JoyDown[w][LGL_JOY_RIGHT]=false;
			}
			
			if(v & SDL_HAT_DOWN)
			{
				LGL.JoyStroke[w][LGL_JOY_DOWN]=true;
				LGL.JoyDown[w][LGL_JOY_DOWN]=true;
			}
			else if(LGL.JoyDown[w][LGL_JOY_DOWN])
			{
				LGL.JoyRelease[w][LGL_JOY_DOWN]=true;
				LGL.JoyDown[w][LGL_JOY_DOWN]=false;
			}
			
			if(v & SDL_HAT_UP)
			{
				LGL.JoyStroke[w][LGL_JOY_UP]=true;
				LGL.JoyDown[w][LGL_JOY_UP]=true;
			}
			else if(LGL.JoyDown[w][LGL_JOY_UP])
			{
				LGL.JoyRelease[w][LGL_JOY_UP]=true;
				LGL.JoyDown[w][LGL_JOY_UP]=false;
			}
		}
		if
		(
			event.type==SDL_JOYBUTTONDOWN ||
			event.type==SDL_JOYBUTTONUP
		)
		{
			int w=event.jbutton.which;
#ifdef	LGL_OSX
			if(event.jbutton.button>=12)
			{
				w++;
			}
			if(event.jbutton.button>=24)
			{
				w++;
			}
			if(event.jbutton.button>=36)
			{
				w++;
			}
#endif	//LGL_OSX
			char buttonstr[64];

			bool down=event.type==SDL_JOYBUTTONDOWN;
			bool up=!down;
			
			if(event.jbutton.button%12==0)
			{
				//Triangle

				LGL.JoyDown[w][LGL_JOY_TRIANGLE]=down;
				LGL.JoyStroke[w][LGL_JOY_TRIANGLE]=down;
				LGL.JoyRelease[w][LGL_JOY_TRIANGLE]=up;

				sprintf(buttonstr,"Triangle");
			}
			if(event.jbutton.button%12==1)
			{
				//Circle
				
				LGL.JoyDown[w][LGL_JOY_CIRCLE]=down;
				LGL.JoyStroke[w][LGL_JOY_CIRCLE]=down;
				LGL.JoyRelease[w][LGL_JOY_CIRCLE]=up;

				sprintf(buttonstr,"Circle");
			}
			if(event.jbutton.button%12==2)
			{
				//Cross
				
				LGL.JoyDown[w][LGL_JOY_CROSS]=down;
				LGL.JoyStroke[w][LGL_JOY_CROSS]=down;
				LGL.JoyRelease[w][LGL_JOY_CROSS]=up;

				sprintf(buttonstr,"Cross");
			}
			if(event.jbutton.button%12==3)
			{
				//Square
				
				LGL.JoyDown[w][LGL_JOY_SQUARE]=down;
				LGL.JoyStroke[w][LGL_JOY_SQUARE]=down;
				LGL.JoyRelease[w][LGL_JOY_SQUARE]=up;

				sprintf(buttonstr,"Square");
			}
			if(event.jbutton.button%12==4)
			{
				//L2
				
				LGL.JoyDown[w][LGL_JOY_L2]=down;
				LGL.JoyStroke[w][LGL_JOY_L2]=down;
				LGL.JoyRelease[w][LGL_JOY_L2]=up;
				
				sprintf(buttonstr,"L2");
			}
			if(event.jbutton.button%12==5)
			{
				//R2
				
				LGL.JoyDown[w][LGL_JOY_R2]=down;
				LGL.JoyStroke[w][LGL_JOY_R2]=down;
				LGL.JoyRelease[w][LGL_JOY_R2]=up;

				sprintf(buttonstr,"R2");
			}
			if(event.jbutton.button%12==6)
			{
				//L1
				
				LGL.JoyDown[w][LGL_JOY_L1]=down;
				LGL.JoyStroke[w][LGL_JOY_L1]=down;
				LGL.JoyRelease[w][LGL_JOY_L1]=up;

				sprintf(buttonstr,"L1");
			}
			if(event.jbutton.button%12==7)
			{
				//R1
				
				LGL.JoyDown[w][LGL_JOY_R1]=down;
				LGL.JoyStroke[w][LGL_JOY_R1]=down;
				LGL.JoyRelease[w][LGL_JOY_R1]=up;

				sprintf(buttonstr,"R1");
			}
			if(event.jbutton.button%12==8)
			{
				//Start
				
				LGL.JoyDown[w][LGL_JOY_START]=down;
				LGL.JoyStroke[w][LGL_JOY_START]=down;
				LGL.JoyRelease[w][LGL_JOY_START]=up;
				
				sprintf(buttonstr,"Start");
			}
			if(event.jbutton.button%12==9)
			{
				//Select
				
				LGL.JoyDown[w][LGL_JOY_SELECT]=down;
				LGL.JoyStroke[w][LGL_JOY_SELECT]=down;
				LGL.JoyRelease[w][LGL_JOY_SELECT]=up;
				
				sprintf(buttonstr,"Select");
			}
			if(event.jbutton.button%12==10)
			{
				//Analogue L

				LGL.JoyDown[w][LGL_JOY_ANALOGUE_L]=down;
				LGL.JoyStroke[w][LGL_JOY_ANALOGUE_L]=down;
				LGL.JoyRelease[w][LGL_JOY_ANALOGUE_L]=up;
				
				sprintf(buttonstr,"AnalogueL");
			}
			if(event.jbutton.button%12==11)
			{
				//Analogue R
				LGL.JoyDown[w][LGL_JOY_ANALOGUE_R]=down;
				LGL.JoyStroke[w][LGL_JOY_ANALOGUE_R]=down;
				LGL.JoyRelease[w][LGL_JOY_ANALOGUE_R]=up;
				
				sprintf(buttonstr,"AnalogueR");
			}
/*
			printf
			(
				"BUTTON.%s: (%i, %i) => C-%i.%s\n",
				down?"down":"up",
				event.jbutton.which,
				event.jbutton.button,
				w,
				buttonstr
			);
*/
		}
	}

	//Handle Beatmania-unique "buttons"
	for(int a=0;a<4;a++)
	{
		//White 4
		bool white4DownNow=(LGL_JoyAnalogueStatus(a,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS)==-1.0f);
		bool white4DownPrev=LGL.JoyDown[a][LGL_JOY_BEATMANIA_WHITE_4];
		LGL.JoyDown[a][LGL_JOY_BEATMANIA_WHITE_4]=white4DownNow;
		LGL.JoyStroke[a][LGL_JOY_BEATMANIA_WHITE_4]=(!white4DownPrev) && white4DownNow;
		LGL.JoyRelease[a][LGL_JOY_BEATMANIA_WHITE_4]=white4DownPrev && (!white4DownNow);

		//Record_CW
		bool recordCWNow=(LGL_JoyAnalogueStatus(a,LGL_JOY_ANALOGUE_L,LGL_JOY_YAXIS)==1.0f);
		bool recordCWPrev=LGL.JoyDown[a][LGL_JOY_BEATMANIA_RECORD_CW];
		LGL.JoyDown[a][LGL_JOY_BEATMANIA_RECORD_CW]=recordCWNow;
		LGL.JoyStroke[a][LGL_JOY_BEATMANIA_RECORD_CW]=(!recordCWPrev) && recordCWNow;
		LGL.JoyRelease[a][LGL_JOY_BEATMANIA_RECORD_CW]=recordCWPrev && (!recordCWNow);

		//Record_CCW
		bool recordCCWNow=(LGL_JoyAnalogueStatus(a,LGL_JOY_ANALOGUE_L,LGL_JOY_YAXIS)==-1.0f);
		bool recordCCWPrev=LGL.JoyDown[a][LGL_JOY_BEATMANIA_RECORD_CCW];
		LGL.JoyDown[a][LGL_JOY_BEATMANIA_RECORD_CCW]=recordCCWNow;
		LGL.JoyStroke[a][LGL_JOY_BEATMANIA_RECORD_CCW]=(!recordCCWPrev) && recordCCWNow;
		LGL.JoyRelease[a][LGL_JOY_BEATMANIA_RECORD_CCW]=recordCCWPrev && (!recordCCWNow);
	}

	if(LGL.InputBufferFocus!=NULL)
	{
		LGL.InputBufferFocus->LGL_INTERNAL_ProcessInput();
	}

#ifdef LGL_LINUX_VIDCAM
	
	//VidCam
	
	if
	(
		LGL_VidCamAvailable() &&
		LGL.VidCamCaptureDelay<LGL.VidCamCaptureDelayTimer.SecondsSinceLastReset()
	)
	{
		LGL_Timer VidCamTimerLocal;
		VidCamTimerLocal.Reset();
		
		int zero=0;
		ioctl(LGL.VidCamFD,VIDIOCSYNC,&zero);
		if(LGL.VidCamImageRaw!=NULL) delete LGL.VidCamImageRaw;
		if(LGL.VidCamImageProcessed!=NULL) delete LGL.VidCamImageProcessed;
		ioctl(LGL.VidCamFD,VIDIOCMCAPTURE,&LGL.VidCamMMap);

		//Commence Center Detection

		int pixelcount=0;
		int x=0;
		int y=0;
		int xmax=0;
		int ymax=0;
		int xmin=LGL.VidCamWidthNow;
		int ymin=LGL.VidCamHeightNow;
				
		int w=LGL.VidCamWidthNow;
		int h=LGL.VidCamHeightNow;

		for(int j=0;j<h;j++)
		{
			float smooth=0;
			int first=-1;
			int last=9999;
			if(pixelcount<3000)
			{
				for(int i=0;i<w;i++)
				{
					float good=0;
					unsigned char r=LGL.VidCamBufferRaw[j*w*3 + i*3 + 2];
					unsigned char g=LGL.VidCamBufferRaw[j*w*3 + i*3 + 1];
					unsigned char b=LGL.VidCamBufferRaw[j*w*3 + i*3 + 0];
					if(r>75)
					{
						good+=1.25;
					}
					if(g>75)
					{
						good+=1.25;
					}
					if(r>100 && g>100)
					{
						good+=1.0;
					}
					if(b<225 && b>75)
					{
						good+=.75;
					}
					if(r<50 && g<50 && b<50)
					{
						good=0;
						smooth=0;
					}
					smooth=.85*smooth+.15*good;
					if(smooth<.25 || pixelcount>=3000)
					{
						//
					}
					else
					{
						if(first==-1) first=i;
						if(last==9999 || i>last) last=i;
					}
				}
				if(first!=-1 && last!=9999)
				{
					for(int a=0;a<w;a++)
					{
						if(a<first || a>last)
						{
							if
							(
								a>w*.375 &&
								a<w*.625 &&
								j>h*.375 &&
								j<h*.625
							)
							{
								//DPad Center Color
								LGL.VidCamBufferProcessed
									[j*w*3 + a*3 + 2]=128;
								LGL.VidCamBufferProcessed
									[j*w*3 + a*3 + 1]=128;
								LGL.VidCamBufferProcessed
									[j*w*3 + a*3 + 0]=128;
							}
							else if
							(
								a>w*.25 &&
								a<w*.75 &&
								j>h*.25 &&
								j<h*.75
							)
							{
								//DPad Center Color
								LGL.VidCamBufferProcessed
									[j*w*3 + a*3 + 2]=64;
								LGL.VidCamBufferProcessed
									[j*w*3 + a*3 + 1]=64;
								LGL.VidCamBufferProcessed
									[j*w*3 + a*3 + 0]=64;
							}
							else
							{
								LGL.VidCamBufferProcessed
									[j*w*3 + a*3 + 2]=0;
								LGL.VidCamBufferProcessed
									[j*w*3 + a*3 + 1]=0;
								LGL.VidCamBufferProcessed
									[j*w*3 + a*3 + 0]=0;
							}
						}
						else
						{
							unsigned char r=LGL.VidCamBufferRaw
								[j*w*3+a*3+2];
							unsigned char g=LGL.VidCamBufferRaw
								[j*w*3+a*3+1];
							unsigned char b=LGL.VidCamBufferRaw
								[j*w*3+a*3+0];
							LGL.VidCamBufferProcessed
								[j*w*3+a*3+2]=r;
							LGL.VidCamBufferProcessed
								[j*w*3+a*3+1]=g;
							LGL.VidCamBufferProcessed
								[j*w*3+a*3+0]=b;
	
							pixelcount++;
							x+=a;
							y+=j;
							if(a<xmin) xmin=a;
							if(a>xmax) xmax=a;
							if(j<ymin) ymin=j;
							if(j>ymax) ymax=j;
						}

						//Make hand glow blue

						for(int dx=0;dx<3 && first+dx<w &&last-dx>=0;dx++)
						{
							LGL.VidCamBufferProcessed
								[j*w*3+(first+dx)*3 + 0]=255;
							LGL.VidCamBufferProcessed
								[j*w*3+(last-dx)*3 + 0]=255;
						}
					}
				}
				else
				{
					for(int a=0;a<w;a++)
					{
						if
						(
							a>w*.375 &&
							a<w*.625 &&
							j>h*.375 &&
							j<h*.625
						)
						{
							//DPad Center Color
							LGL.VidCamBufferProcessed
								[j*w*3 + a*3 + 2]=128;
							LGL.VidCamBufferProcessed
								[j*w*3 + a*3 + 1]=128;
							LGL.VidCamBufferProcessed
								[j*w*3 + a*3 + 0]=128;
						}
						else if
						(
							a>w*.25 &&
							a<w*.75 &&
							j>h*.25 &&
							j<h*.75
						)
						{
							//DPad Center Color
							LGL.VidCamBufferProcessed
								[j*w*3 + a*3 + 2]=64;
							LGL.VidCamBufferProcessed
								[j*w*3 + a*3 + 1]=64;
							LGL.VidCamBufferProcessed
								[j*w*3 + a*3 + 0]=64;
						}
						else
						{
							LGL.VidCamBufferProcessed
								[j*w*3 + a*3 + 2]=0;
							LGL.VidCamBufferProcessed
								[j*w*3 + a*3 + 1]=0;
							LGL.VidCamBufferProcessed
								[j*w*3 + a*3 + 0]=0;
						}
					}
				}
			}
			else
			{
				for(int a=0;a<w;a++)
				{
					if
					(
						a>w*.375 &&
						a<w*.625 &&
						j>h*.375 &&
						j<h*.625
					)
					{
						//DPad Center Color
						LGL.VidCamBufferProcessed[j*w*3 + a*3 + 2]=128;
						LGL.VidCamBufferProcessed[j*w*3 + a*3 + 1]=128;
						LGL.VidCamBufferProcessed[j*w*3 + a*3 + 0]=128;
					}
					else if
					(
						a>w*.25 &&
						a<w*.75 &&
						j>h*.25 &&
						j<h*.75
					)
					{
						//DPad Center Color
						LGL.VidCamBufferProcessed[j*w*3 + a*3 + 2]=64;
						LGL.VidCamBufferProcessed[j*w*3 + a*3 + 1]=64;
						LGL.VidCamBufferProcessed[j*w*3 + a*3 + 0]=64;
					}
					else
					{
						LGL.VidCamBufferProcessed[j*w*3 + a*3 + 2]=0;
						LGL.VidCamBufferProcessed[j*w*3 + a*3 + 1]=0;
						LGL.VidCamBufferProcessed[j*w*3 + a*3 + 0]=0;
					}
				}
			}
		}

		if(pixelcount<400)
		{
			x=w/2;
			y=h/2;
			pixelcount=1;
		}
		else
		{
			x/=pixelcount;
			y/=pixelcount;
		}

		for(int j=0;j<3;j++)
		{
			for(int i=0;i<8;i++)
			{
				if
				(
					x+j<w &&
					y+i<h &&
					x-j>0 &&
					y-j>0
				)
				{
					LGL.VidCamBufferProcessed[(y+i)*w*3 + (x+j)*3 + 2]=0;
					LGL.VidCamBufferProcessed[(y+i)*w*3 + (x+j)*3 + 1]=255;
					LGL.VidCamBufferProcessed[(y+i)*w*3 + (x+j)*3 + 0]=0;
				
					LGL.VidCamBufferProcessed[(y-i)*w*3 + (x+j)*3 + 2]=0;
					LGL.VidCamBufferProcessed[(y-i)*w*3 + (x+j)*3 + 1]=255;
					LGL.VidCamBufferProcessed[(y-i)*w*3 + (x+j)*3 + 0]=0;
				
					LGL.VidCamBufferProcessed[(y+j)*w*3 + (x+i)*3 + 2]=0;
					LGL.VidCamBufferProcessed[(y+j)*w*3 + (x+i)*3 + 1]=255;
					LGL.VidCamBufferProcessed[(y+j)*w*3 + (x+i)*3 + 0]=0;
				
					LGL.VidCamBufferProcessed[(y+j)*w*3 + (x-i)*3 + 2]=0;
					LGL.VidCamBufferProcessed[(y+j)*w*3 + (x-i)*3 + 1]=255;
					LGL.VidCamBufferProcessed[(y+j)*w*3 + (x-i)*3 + 0]=0;
				}
			}
		}

		//Commence Finger Detection

		bool ThumbCross=false;
		bool ThumbCircle=false;
		bool PointerSquare=false;
		bool PointerTriangle=false;

		int ThumbCrossCount=0;
		int ThumbCircleCount=0;

		//Thumb Detect (Cross)
		int dt=LGL.VidCamDistanceThumb;
		for(int t=0;t<3;t++)
		{
			for(int a=0;a<75;a++)
			{
				int c1=(y+35-a)*w*3 + (x+dt-t)*3;
				int c2=(y+35-a)*w*3 + (x+dt+t)*3;
				if
				(
					(
						x+dt-t>0 &&
						x+dt+t<w &&
						y+35-a>0 &&
						y+35-a<h
					) &&
					(
						!(
						 	LGL.VidCamBufferProcessed[c1+0]==0 &&
						 	LGL.VidCamBufferProcessed[c1+1]==0 &&
						 	LGL.VidCamBufferProcessed[c1+2]==0
						) &&
						!(
						 	LGL.VidCamBufferProcessed[c1+0]==64 &&
						 	LGL.VidCamBufferProcessed[c1+1]==64 &&
						 	LGL.VidCamBufferProcessed[c1+2]==64
						) &&
						!(
						 	LGL.VidCamBufferProcessed[c1+0]==128 &&
						 	LGL.VidCamBufferProcessed[c1+1]==128 &&
						 	LGL.VidCamBufferProcessed[c1+2]==128
						)
					) ||
					(
						!(
						 	LGL.VidCamBufferProcessed[c2+0]==0 &&
						 	LGL.VidCamBufferProcessed[c2+1]==0 &&
						 	LGL.VidCamBufferProcessed[c2+2]==0
						) &&
						!(
						 	LGL.VidCamBufferProcessed[c2+0]==64 &&
						 	LGL.VidCamBufferProcessed[c2+1]==64 &&
						 	LGL.VidCamBufferProcessed[c2+2]==64
							) &&
						!(
					 		LGL.VidCamBufferProcessed[c2+0]==128 &&
						 	LGL.VidCamBufferProcessed[c2+1]==128 &&
						 	LGL.VidCamBufferProcessed[c2+2]==128
						)
					)
				)
				{
					ThumbCrossCount++;
				}
			}
		}
		if(ThumbCrossCount>2*3) ThumbCross=true;
		
		//Thumb Detect (Circle)
		dt=-LGL.VidCamDistanceThumb;
		for(int t=0;t<3;t++)
		{
			for(int a=0;a<75;a++)
			{
				int c1=(y+35-a)*w*3 + (x+dt-t)*3;
				int c2=(y+35-a)*w*3 + (x+dt+t)*3;
				if
				(
					(
						x+dt-t>0 &&
						x+dt+t<w &&
						y+35-a>0 &&
						y+35-a<h
					) &&
					(
						!(
						 	LGL.VidCamBufferProcessed[c1+0]==0 &&
						 	LGL.VidCamBufferProcessed[c1+1]==0 &&
						 	LGL.VidCamBufferProcessed[c1+2]==0
						) &&
						!(
						 	LGL.VidCamBufferProcessed[c1+0]==64 &&
						 	LGL.VidCamBufferProcessed[c1+1]==64 &&
						 	LGL.VidCamBufferProcessed[c1+2]==64
						) &&
						!(
						 	LGL.VidCamBufferProcessed[c1+0]==128 &&
						 	LGL.VidCamBufferProcessed[c1+1]==128 &&
						 	LGL.VidCamBufferProcessed[c1+2]==128
						)
					) ||
					(
						!(
						 	LGL.VidCamBufferProcessed[c2+0]==0 &&
						 	LGL.VidCamBufferProcessed[c2+1]==0 &&
						 	LGL.VidCamBufferProcessed[c2+2]==0
						) &&
						!(
						 	LGL.VidCamBufferProcessed[c2+0]==64 &&
						 	LGL.VidCamBufferProcessed[c2+1]==64 &&
						 	LGL.VidCamBufferProcessed[c2+2]==64
							) &&
						!(
					 		LGL.VidCamBufferProcessed[c2+0]==128 &&
						 	LGL.VidCamBufferProcessed[c2+1]==128 &&
						 	LGL.VidCamBufferProcessed[c2+2]==128
						)
					)
				)
				{
					ThumbCircleCount++;
				}
			}
		}
		if(ThumbCircleCount>2*3) ThumbCircle=true;
		
		//Pointer Detect (Square)
		int dp=LGL.VidCamDistancePointer;
		for(int t=0;t<3;t++)
		{
			for(int a=0;a<35;a++)
			{
				int c1=(y+dp-t+a/4)*w*3 + (x+0-a)*3;
				int c2=(y+dp+t+a/4)*w*3 + (x+0-a)*3;
				if
				(
					(
						x+0-a>0 &&
						x+0-a<w &&
						y+dp-t+a/4>0 &&
						y+dp+t+a/4<h
					) &&
					(
						!(
						 	LGL.VidCamBufferProcessed[c1+0]==0 &&
						 	LGL.VidCamBufferProcessed[c1+1]==0 &&
						 	LGL.VidCamBufferProcessed[c1+2]==0
						) &&
						!(
						 	LGL.VidCamBufferProcessed[c1+0]==64 &&
						 	LGL.VidCamBufferProcessed[c1+1]==64 &&
						 	LGL.VidCamBufferProcessed[c1+2]==64
						) &&
						!(
						 	LGL.VidCamBufferProcessed[c1+0]==128 &&
						 	LGL.VidCamBufferProcessed[c1+1]==128 &&
						 	LGL.VidCamBufferProcessed[c1+2]==128
						)
					) ||
					(
						!(
						 	LGL.VidCamBufferProcessed[c2+0]==0 &&
						 	LGL.VidCamBufferProcessed[c2+1]==0 &&
						 	LGL.VidCamBufferProcessed[c2+2]==0
						) &&
						!(
						 	LGL.VidCamBufferProcessed[c2+0]==64 &&
						 	LGL.VidCamBufferProcessed[c2+1]==64 &&
						 	LGL.VidCamBufferProcessed[c2+2]==64
							) &&
						!(
					 		LGL.VidCamBufferProcessed[c2+0]==128 &&
						 	LGL.VidCamBufferProcessed[c2+1]==128 &&
						 	LGL.VidCamBufferProcessed[c2+2]==128
						)
					)
				)
				{
					PointerSquare=true;
					break;
				}
			}
		}
		
		//Pointer Detect (Triangle)
		dp=LGL.VidCamDistancePointer;
		for(int t=0;t<3;t++)
		{
			for(int a=0;a<35;a++)
			{
				int c1=(y+dp-t+(35/4-a/4))*w*3 + (x+40-a)*3;
				int c2=(y+dp+t+(35/4-a/4))*w*3 + (x+40-a)*3;
				if
				(
					(
						x+40-a>0 &&
						x+40-a<w &&
						y+dp-t+(35/4-a/4)>0 &&
						y+dp+t+(35/4-a/4)<h
					) &&
					(
						!(
						 	LGL.VidCamBufferProcessed[c1+0]==0 &&
						 	LGL.VidCamBufferProcessed[c1+1]==0 &&
						 	LGL.VidCamBufferProcessed[c1+2]==0
						) &&
						!(
						 	LGL.VidCamBufferProcessed[c1+0]==64 &&
						 	LGL.VidCamBufferProcessed[c1+1]==64 &&
						 	LGL.VidCamBufferProcessed[c1+2]==64
						) &&
						!(
						 	LGL.VidCamBufferProcessed[c1+0]==128 &&
						 	LGL.VidCamBufferProcessed[c1+1]==128 &&
						 	LGL.VidCamBufferProcessed[c1+2]==128
						)
					) ||
					(
						!(
						 	LGL.VidCamBufferProcessed[c2+0]==0 &&
						 	LGL.VidCamBufferProcessed[c2+1]==0 &&
						 	LGL.VidCamBufferProcessed[c2+2]==0
						) &&
						!(
						 	LGL.VidCamBufferProcessed[c2+0]==64 &&
						 	LGL.VidCamBufferProcessed[c2+1]==64 &&
						 	LGL.VidCamBufferProcessed[c2+2]==64
							) &&
						!(
					 		LGL.VidCamBufferProcessed[c2+0]==128 &&
						 	LGL.VidCamBufferProcessed[c2+1]==128 &&
						 	LGL.VidCamBufferProcessed[c2+2]==128
						)
					)
				)
				{
					PointerTriangle=true;
					break;
				}
			}
		}

		if(ThumbCross && ThumbCircle)
		{
			if(ThumbCrossCount>=ThumbCircleCount)
			{
				ThumbCross=true;
				ThumbCircle=false;
			}
			else
			{
				ThumbCross=false;
				ThumbCircle=true;
			}
		}
		
		//Thumb Color (Cross)
		dt=LGL.VidCamDistanceThumb;
		int color=2;
		if(ThumbCross==true && OldThumbCircle==false) color=1;
		for(int t=0;t<3;t++)
		{
			for(int a=0;a<75;a++)
			{
				if
				(
					x+dt-t>0 &&
					x+dt+t<w &&
					y+35-a>0 &&
					y+35-a<h
				)
				{
					LGL.VidCamBufferProcessed
						[(y+35-a)*w*3 + (x+dt-t)*3 + color]=255;
					LGL.VidCamBufferProcessed
						[(y+35-a)*w*3 + (x+dt+t)*3 + color]=255;
				}
			}
		}

		//ThumbColor (Circle)
		dt=-LGL.VidCamDistanceThumb;
		color=2;
		if(ThumbCircle==true && OldThumbCross==false) color=1;
		for(int t=0;t<3;t++)
		{
			for(int a=0;a<75;a++)
			{
				if
				(
					x+dt-t>0 &&
					x+dt+t<w &&
					y+35-a>0 &&
					y+35-a<h
				)
				{
					LGL.VidCamBufferProcessed
						[(y+35-a)*w*3 + (x+dt-t)*3 + color]=255;
					LGL.VidCamBufferProcessed
						[(y+35-a)*w*3 + (x+dt+t)*3 + color]=255;
				}
			}
		}
		
		//Pointer Color (Square)
		color=2;
		if(PointerSquare) color=1;
		for(int t=0;t<3;t++)
		{
			for(int a=0;a<35;a++)
			{
				if
				(
					x+0-a>0 &&
					x+0-a<w &&
					y+dp-t+a/4>0 &&
					y+dp+t+a/4<h
				)
				{
					LGL.VidCamBufferProcessed
					[(y+dp-t+a/4)*w*3 + (x+0-a)*3 + color]=255;
					LGL.VidCamBufferProcessed
					[(y+dp+t+a/4)*w*3 + (x+0-a)*3 + color]=255;
				}
			}
		}
		
		//Pointer Color (Triangle)
		color=2;
		if(PointerTriangle) color=1;
		for(int t=0;t<3;t++)
		{
			for(int a=0;a<35;a++)
			{
				if
				(
					x+40-a>0 &&
					x+40-a<w &&
					y+dp-t+(35/4-a/4)>0 &&
					y+dp+t+(35/4-a/4)<h
				)
				{
					LGL.VidCamBufferProcessed
					[(y+dp-t+(35/4-a/4))*w*3 + (x+40-a)*3 + color]=255;
					LGL.VidCamBufferProcessed
					[(y+dp+t+(35/4-a/4))*w*3 + (x+40-a)*3 + color]=255;
				}
			}
		}

		//Draw Button Symbols

		//Cross (Thumb Left)
		dt=LGL.VidCamDistanceThumb-20;
		for(int t=0;t<2;t++)
		{
			for(int a=-5;a<5;a++)
			{
				if
				(
					x+dt+a-t>0 &&
					x+dt+a+t<w &&
					x+dt-a-t>0 &&
					x+dt-a+t<w &&
					y+a-t>0 &&
					y+a+t<h &&
					y-a-t>0 &&
					y-a+t<h
				)
				{
					LGL.VidCamBufferProcessed
						[(y+a+t)*w*3 + (x+dt+a+t)*3 + 0]=255;
					LGL.VidCamBufferProcessed
						[(y-a+t)*w*3 + (x+dt+a+t)*3 + 0]=255;
					LGL.VidCamBufferProcessed
						[(y+a+t)*w*3 + (x+dt-a+t)*3 + 0]=255;
					LGL.VidCamBufferProcessed
						[(y-a+t)*w*3 + (x+dt-a+t)*3 + 0]=255;
				}
			}
		}
		
		//Square (Pointer Left)
		dp=LGL.VidCamDistancePointer-20;
		for(int t=0;t<2;t++)
		{
			for(int a=-5;a<5;a++)
			{
				if
				(
					x-20+a>0 &&
					x-20+a<w &&
					x-20+t-5>0 &&
					x-20+t+5<w &&
					y+5+dp+t-5>0 &&
					y+5+dp+t-5>0 &&
					y+5+dp+a>0 &&
					y+5+dp+a<h
				)
				{
					LGL.VidCamBufferProcessed
					[(y+5+dp-t+5)*w*3 + (x-20+a)*3 + 2]=255;
					LGL.VidCamBufferProcessed
					[(y+5+dp+t-5)*w*3 + (x-20+a)*3 + 2]=255;
					
					LGL.VidCamBufferProcessed
					[(y+5+dp+a)*w*3 + (x-20+t+5)*3 + 2]=255;
					LGL.VidCamBufferProcessed
					[(y+5+dp+a)*w*3 + (x-20+t-5)*3 + 2]=255;
					
					LGL.VidCamBufferProcessed
					[(y+5+dp-t+5)*w*3 + (x-20+a)*3 + 1]=55;
					LGL.VidCamBufferProcessed
					[(y+5+dp+t-5)*w*3 + (x-20+a)*3 + 1]=55;
					
					LGL.VidCamBufferProcessed
					[(y+5+dp+a)*w*3 + (x-20+t+5)*3 + 1]=55;
					LGL.VidCamBufferProcessed
					[(y+5+dp+a)*w*3 + (x-20+t-5)*3 + 1]=55;
					
					LGL.VidCamBufferProcessed
					[(y+5+dp-t+5)*w*3 + (x-20+a)*3 + 0]=155;
					LGL.VidCamBufferProcessed
					[(y+5+dp+t-5)*w*3 + (x-20+a)*3 + 0]=155;
					
					LGL.VidCamBufferProcessed
					[(y+5+dp+a)*w*3 + (x-20+t+5)*3 + 0]=155;
					LGL.VidCamBufferProcessed
					[(y+5+dp+a)*w*3 + (x-20+t-5)*3 + 0]=155;
				}
			}
		}
		
		//Triangle (Pointer Right)
		dp=LGL.VidCamDistancePointer-20;
		for(int t=0;t<2;t++)
		{
			for(int a=-5;a<6;a++)
			{
				int c1=(int)(ceil(a/2.0));
				if
				(
					x+25+a>0 &&
					x+25+a<w &&
					x+25+t-5>0 &&
					x+25+t+2+a/2<w &&
					y+5+dp+t-5>0 &&
					y+5+dp+t-5>0 &&
					y+5+dp+a>0 &&
					y+5+dp+a<h
				)
				{
					LGL.VidCamBufferProcessed
					[(y+5+dp-t+5)*w*3 + (x+25+a)*3 + 2]=0;
					LGL.VidCamBufferProcessed
					[(y+5+dp+a)*w*3 + (x+25+t+2+c1)*3 + 2]=0;
					LGL.VidCamBufferProcessed
					[(y+5+dp+a)*w*3 + (x+25+t-5-c1)*3 + 2]=0;
					
					LGL.VidCamBufferProcessed
					[(y+5+dp-t+5)*w*3 + (x+25+a)*3 + 1]=255;
					LGL.VidCamBufferProcessed
					[(y+5+dp+a)*w*3 + (x+25+t+2+c1)*3 + 1]=255;
					LGL.VidCamBufferProcessed
					[(y+5+dp+a)*w*3 + (x+25+t-5-c1)*3 + 1]=255;
					
					LGL.VidCamBufferProcessed
					[(y+5+dp-t+5)*w*3 + (x+25+a)*3 + 0]=55;
					LGL.VidCamBufferProcessed
					[(y+5+dp+a)*w*3 + (x+25+t+2+c1)*3 + 0]=55;
					LGL.VidCamBufferProcessed
					[(y+5+dp+a)*w*3 + (x+25+t-5-c1)*3 + 0]=55;
				}
			}
		}

		//Circle (Thumb Right)
		dt=LGL.VidCamDistanceThumb-20;
		for(int t=0;t<2;t++)
		{
			for(float a=0;a<1;a+=.05)
			{
				int cdx=(int)floor(5*cos(2*LGL_PI*a));
				int cdy=(int)floor(5*sin(2*LGL_PI*a));
				if
				(
					x-dt+cdx-t>0 &&
					x-dt+cdx+t<w &&
					x-dt-cdx-t>0 &&
					x-dt-cdx+t<w &&
					y+cdy-t>0 &&
					y+cdy+t<h &&
					y-cdy-t>0 &&
					y-cdy+t<h 
				)
				{
					LGL.VidCamBufferProcessed
						[(y+cdy+t)*w*3 + (x-dt-cdx+t)*3 + 2]=255;
					LGL.VidCamBufferProcessed
						[(y+cdy-t)*w*3 + (x-dt-cdx+t)*3 + 2]=255;
					LGL.VidCamBufferProcessed
						[(y+cdy+t)*w*3 + (x-dt-cdx-t)*3 + 2]=255;
					LGL.VidCamBufferProcessed
						[(y+cdy-t)*w*3 + (x-dt-cdx-t)*3 + 2]=255;
					
					LGL.VidCamBufferProcessed
						[(y+cdy+t)*w*3 + (x-dt-cdx+t)*3 + 1]=128;
					LGL.VidCamBufferProcessed
						[(y+cdy-t)*w*3 + (x-dt-cdx+t)*3 + 1]=128;
					LGL.VidCamBufferProcessed
						[(y+cdy+t)*w*3 + (x-dt-cdx-t)*3 + 1]=128;
					LGL.VidCamBufferProcessed
						[(y+cdy-t)*w*3 + (x-dt-cdx-t)*3 + 1]=128;
					
					LGL.VidCamBufferProcessed
						[(y+cdy+t)*w*3 + (x-dt-cdx+t)*3 + 0]=128;
					LGL.VidCamBufferProcessed
						[(y+cdy-t)*w*3 + (x-dt-cdx+t)*3 + 0]=128;
					LGL.VidCamBufferProcessed
						[(y+cdy+t)*w*3 + (x-dt-cdx-t)*3 + 0]=128;
					LGL.VidCamBufferProcessed
						[(y+cdy-t)*w*3 + (x-dt-cdx-t)*3 + 0]=128;
				}
			}
		}

		//Map Processed Buffer To Last Joystick

		LGL.VidCamAxisXLast=LGL.VidCamAxisXNext;
		LGL.VidCamAxisYLast=LGL.VidCamAxisYNext;
		
		LGL.VidCamAxisXNext=(x-(w/2.0))/(w*.25);
		LGL.VidCamAxisYNext=-(y-(h/2.0))/(h*.25);

		if(LGL.VidCamAxisXNext>1)
		{
			LGL.VidCamAxisXNext=1;
		}
		if(LGL.VidCamAxisXNext<-1)
		{
			LGL.VidCamAxisXNext=-1;
		}
		if(LGL.VidCamAxisYNext>1)
		{
			LGL.VidCamAxisYNext=1;
		}
		if(LGL.VidCamAxisYNext<-1)
		{
			LGL.VidCamAxisYNext=-1;
		}

		if(PointerSquare)
		{
			if(LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_SQUARE]==false)
			{
				LGL.JoyStroke[LGL_VidCamJoyNumber()][LGL_JOY_SQUARE]=true;
			}
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_SQUARE]=true;
		}
		else
		{
			if(LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_SQUARE]==true)
			{
				LGL.JoyRelease[LGL_VidCamJoyNumber()][LGL_JOY_SQUARE]=true;
			}
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_SQUARE]=false;
		}
		
		if(PointerTriangle)
		{
			if(LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_TRIANGLE]==false)
			{
				LGL.JoyStroke[LGL_VidCamJoyNumber()][LGL_JOY_TRIANGLE]=true;
			}
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_TRIANGLE]=true;
		}
		else
		{
			if(LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_TRIANGLE]==true)
			{
				LGL.JoyRelease[LGL_VidCamJoyNumber()][LGL_JOY_TRIANGLE]=true;
			}
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_TRIANGLE]=false;
		}

		if(ThumbCross && !OldThumbCircle)
		{
			if(LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_CROSS]==false)
			{
				LGL.JoyStroke[LGL_VidCamJoyNumber()][LGL_JOY_CROSS]=true;
			}
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_CROSS]=true;
		}
		else
		{
			if(LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_CROSS]==true)
			{
				LGL.JoyRelease[LGL_VidCamJoyNumber()][LGL_JOY_CROSS]=true;
			}
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_CROSS]=false;
		}
		if(ThumbCircle && !OldThumbCross)
		{
			if(LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_CIRCLE]==false)
			{
				LGL.JoyStroke[LGL_VidCamJoyNumber()][LGL_JOY_CIRCLE]=true;
			}
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_CIRCLE]=true;
		}
		else
		{
			if(LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_CIRCLE]==true)
			{
				LGL.JoyRelease[LGL_VidCamJoyNumber()][LGL_JOY_CIRCLE]=true;
			}
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_CIRCLE]=false;
		}

		LGL.VidCamImageRaw=new LGL_Image
		(
			LGL.VidCamWidthNow,
			LGL.VidCamHeightNow,
			3,
			LGL.VidCamBufferRaw
		);

		LGL.VidCamImageProcessed=new LGL_Image
		(
			LGL.VidCamWidthNow,
			LGL.VidCamHeightNow,
			3,
			LGL.VidCamBufferProcessed
		);

		if(LGL.FPS<60)
		{
			LGL.VidCamCaptureDelay+=(60-LGL.FPS)/(5*60.0*60.0);
		}
		else if(LGL.VidCamCaptureDelay>(1.0/15.0))
		{
			LGL.VidCamCaptureDelay+=(60-LGL.FPS)/(5*60.0*60.0);
		}

		if(LGL.VidCamCaptureDelay<1.0/15.0)
		{
			LGL.VidCamCaptureDelay=1.0/15.0;
		}
		if(LGL.VidCamCaptureDelay>1.0/LGL_VIDCAM_FPS_MIN)
		{
			LGL.VidCamCaptureDelay=1.0/LGL_VIDCAM_FPS_MIN;
		}

		LGL.VidCamCaptureDelayTimer.Reset();
	}
	else if(LGL_VidCamAvailable())
	{
		float I=1.0-
			(
				LGL.VidCamCaptureDelay-
				LGL.VidCamCaptureDelayTimer.SecondsSinceLastReset()
			) / LGL.VidCamCaptureDelay;
		if(I<0) I=0;
		if(I>1) I=1;

		I=sin(LGL_PI*.5*I);

		LGL.JoyAnalogue[LGL_VidCamJoyNumber()][LGL_JOY_ANALOGUE_L][LGL_JOY_XAXIS]=
			I*LGL.VidCamAxisXNext+(1.0-I)*LGL.VidCamAxisXLast;
		LGL.JoyAnalogue[LGL_VidCamJoyNumber()][LGL_JOY_ANALOGUE_L][LGL_JOY_YAXIS]=
			I*LGL.VidCamAxisYNext+(1.0-I)*LGL.VidCamAxisYLast;

		if(LGL.JoyAnalogue[LGL_VidCamJoyNumber()][LGL_JOY_ANALOGUE_L][LGL_JOY_XAXIS]>1)
		{
			LGL.JoyAnalogue[LGL_VidCamJoyNumber()][LGL_JOY_ANALOGUE_L][LGL_JOY_XAXIS]=
				1;
		}
		if(LGL.JoyAnalogue[LGL_VidCamJoyNumber()][LGL_JOY_ANALOGUE_L][LGL_JOY_XAXIS]<-1)
		{
			LGL.JoyAnalogue[LGL_VidCamJoyNumber()][LGL_JOY_ANALOGUE_L][LGL_JOY_XAXIS]=
				-1;
		}
		if(LGL.JoyAnalogue[LGL_VidCamJoyNumber()][LGL_JOY_ANALOGUE_L][LGL_JOY_YAXIS]>1)
		{
			LGL.JoyAnalogue[LGL_VidCamJoyNumber()][LGL_JOY_ANALOGUE_L][LGL_JOY_YAXIS]=
				1;
		}
		if(LGL.JoyAnalogue[LGL_VidCamJoyNumber()][LGL_JOY_ANALOGUE_L][LGL_JOY_YAXIS]<-1)
		{
			LGL.JoyAnalogue[LGL_VidCamJoyNumber()][LGL_JOY_ANALOGUE_L][LGL_JOY_YAXIS]=
				-1;
		}

		if(LGL.JoyAnalogue[LGL_VidCamJoyNumber()][LGL_JOY_ANALOGUE_L][LGL_JOY_XAXIS]<-.5)
		{
			if
			(
				LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_LEFT]==false &&
				LGL.VidCamDPadStrokeDelayTimerLeft.SecondsSinceLastReset()>.25
			)
			{
				LGL.JoyStroke[LGL_VidCamJoyNumber()][LGL_JOY_LEFT]=true;
			}
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_LEFT]=true;
			LGL.VidCamDPadStrokeDelayTimerLeft.Reset();
		}
		else
		{
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_LEFT]=false;
		}
		if(LGL.JoyAnalogue[LGL_VidCamJoyNumber()][LGL_JOY_ANALOGUE_L][LGL_JOY_XAXIS]>.5)
		{
			if
			(
				LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_RIGHT]==false &&
				LGL.VidCamDPadStrokeDelayTimerRight.SecondsSinceLastReset()>.25
			)
			{
				LGL.JoyStroke[LGL_VidCamJoyNumber()][LGL_JOY_RIGHT]=true;
			}
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_RIGHT]=true;
			LGL.VidCamDPadStrokeDelayTimerRight.Reset();
		}
		else
		{
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_RIGHT]=false;
		}
		if(LGL.JoyAnalogue[LGL_VidCamJoyNumber()][LGL_JOY_ANALOGUE_L][LGL_JOY_YAXIS]<-.5)
		{
			if
			(
				LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_DOWN]==false &&
				LGL.VidCamDPadStrokeDelayTimerDown.SecondsSinceLastReset()>.25
			)
			{
				LGL.JoyStroke[LGL_VidCamJoyNumber()][LGL_JOY_DOWN]=true;
			}
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_DOWN]=true;
			LGL.VidCamDPadStrokeDelayTimerDown.Reset();
		}
		else
		{
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_DOWN]=false;
		}
		if(LGL.JoyAnalogue[LGL_VidCamJoyNumber()][LGL_JOY_ANALOGUE_L][LGL_JOY_YAXIS]>.5)
		{
			if
			(
				LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_UP]==false &&
				LGL.VidCamDPadStrokeDelayTimerUp.SecondsSinceLastReset()>.25
			)
			{
				LGL.JoyStroke[LGL_VidCamJoyNumber()][LGL_JOY_UP]=true;
			}
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_UP]=true;
			LGL.VidCamDPadStrokeDelayTimerUp.Reset();
		}
		else
		{
			LGL.JoyDown[LGL_VidCamJoyNumber()][LGL_JOY_UP]=false;
		}
	}
#endif //LGL_LINUX_VIDCAM
}

//Keyboard

bool
lgl_KeySanityCheck
(
	int	key
)
{
	if(key<0 || key>=512)
	{
		printf
		(
			"lgl_KeySanityCheck(): Invalid parameter (%i)\n",
			key
		);
		assert(false);
		return(false);
	}
	else
	{
		return(true);
	}
}

bool
LGL_KeyDown
(
	int	key
)
{
	if(lgl_KeySanityCheck(key)==false) return(false);

	if(key==LGL_KEY_SHIFT)
	{
		return
		(
			LGL_KeyDown(LGL_KEY_LSHIFT) ||
			LGL_KeyDown(LGL_KEY_RSHIFT)
		);
	}

	return(LGL.KeyDown[key]);
}

bool
LGL_KeyStroke
(
	int	key
)
{
	if(lgl_KeySanityCheck(key)==false) return(false);
	
	if(key==LGL_KEY_SHIFT)
	{
		return
		(
			LGL_KeyStroke(LGL_KEY_LSHIFT) ||
			LGL_KeyStroke(LGL_KEY_RSHIFT)
		);
	}

	return(LGL.KeyStroke[key]);
}

bool
LGL_KeyRelease
(
	int	key
)
{
	if(lgl_KeySanityCheck(key)==false) return(false);
	
	if(key==LGL_KEY_SHIFT)
	{
		return
		(
			LGL_KeyRelease(LGL_KEY_LSHIFT) ||
			LGL_KeyRelease(LGL_KEY_RSHIFT)
		);
	}

	return(LGL.KeyRelease[key]);
}

float
LGL_KeyTimer
(
	int	key
)
{
	if(lgl_KeySanityCheck(key)==false) return(0.0f);
	
	if(key==LGL_KEY_SHIFT)
	{
		return
		(
			LGL_Max
			(
				LGL_KeyTimer(LGL_KEY_LSHIFT),
				LGL_KeyTimer(LGL_KEY_RSHIFT)
			)
		);
	}

	return(LGL.KeyTimer[key].SecondsSinceLastReset());
}

const
char*
LGL_KeyStream()
{
	return(LGL.KeyStream);
}

//Mouse

void
lgl_MouseSanityCheck
(
	int&	button
)
{
	if(button<0 || button>=3)
	{
		printf
		(
			"lgl_MouseSanityCheck(): Invalid parameter (%i)\n",
			button
		);
		button=0;
	}
}

float
LGL_MouseX()
{
	return(LGL.MouseX);
}

float
LGL_MouseY()
{
	return(LGL.MouseY);
}

float
LGL_MouseDX()
{
	return(LGL.MouseDX);
}

float
LGL_MouseDY()
{
	return(LGL.MouseDY);
}

bool
LGL_MouseMotion()
{
	return
	(
		fabs(LGL_MouseDX())>0.00001 ||
		fabs(LGL_MouseDY())>0.00001
	);
}

bool
LGL_MouseDown
(
	int	button
)
{
	lgl_MouseSanityCheck(button);
	return(LGL.MouseDown[button]);
}

bool
LGL_MouseStroke
(
	int	button
)
{
	lgl_MouseSanityCheck(button);
	return(LGL.MouseStroke[button]);
}

bool
LGL_MouseRelease
(
	int	button
)
{
	lgl_MouseSanityCheck(button);
	return(LGL.MouseRelease[button]);
}

float
LGL_MouseTimer
(
	int	button
)
{
	lgl_MouseSanityCheck(button);
	return(LGL.MouseTimer[button].SecondsSinceLastReset());
}

void
LGL_MouseVisible
(
	bool	visible
)
{
	if(visible)
	{
		SDL_ShowCursor(SDL_ENABLE);
	}
	else
	{
		SDL_ShowCursor(SDL_DISABLE);
	}
}

void
LGL_MouseWarp
(
	float	x,
	float	y
)
{
	SDL_WarpMouse
	(
		(int)(x*LGL.WindowResolutionX[LGL.DisplayNow]),
		(int)((1.0-y)*LGL.WindowResolutionY[LGL.DisplayNow])
	);
	LGL.MouseDX=x-LGL.MouseX;
	LGL.MouseDY=x-LGL.MouseY;
	LGL.MouseX=x;
	LGL.MouseY=y;
}

//MultiTouch

float
LGL_MultiTouchX()
{
	return(LGL.MultiTouchX);
}

float
LGL_MultiTouchY()
{
	return(LGL.MultiTouchY);
}

float
LGL_MultiTouchDX()
{
	return(LGL.MultiTouchDX);
}

float
LGL_MultiTouchDX2()
{
	float x = LGL_MultiTouchDX();
	float y = LGL_MultiTouchDY();
	if(fabsf(y)>fabsf(x))
	{
		y*=x/y;
	}
	float result=sqrtf
	(
		powf(x,2.0f)+
		powf(y,2.0f)
	);
	if(LGL_MultiTouchDX()<0.0f)
	{
		result*=-1.0f;
	}

	return(result);
}

float
LGL_MultiTouchDY()
{
	return(LGL.MultiTouchDY);
}

float
LGL_MultiTouchDY2()
{
	float x = LGL_MultiTouchDX();
	float y = LGL_MultiTouchDY();
	if(fabsf(x)>fabsf(y))
	{
		x*=y/x;
	}
	float result=sqrtf
	(
		powf(x,2.0f)+
		powf(y,2.0f)
	);
	if(LGL_MultiTouchDY()<0.0f)
	{
		result*=-1.0f;
	}

	return(result);
}

float
LGL_MultiTouchDXTotal()
{
	return(LGL.MultiTouchX-LGL.MultiTouchXFirst);
}

float
LGL_MultiTouchDYTotal()
{
	return(LGL.MultiTouchY-LGL.MultiTouchYFirst);
}

float
LGL_MultiTouchRotate()
{
	return(LGL.MultiTouchRotate);
}

float
LGL_MultiTouchPinch()
{
	return(LGL.MultiTouchPinch);
}

bool
LGL_MultiTouchMotion()
{
	return
	(
		LGL_MultiTouchDX()!=0.0f ||
		LGL_MultiTouchDY()!=0.0f ||
		LGL_MultiTouchRotate()!=0.0f ||
		LGL_MultiTouchPinch()!=0.0f
	);
}

int
LGL_MultiTouchFingerCount()
{
	return(LGL.MultiTouchFingerCount);
}

int
LGL_MultiTouchFingerCountDelta()
{
	return(LGL.MultiTouchFingerCountDelta);
}

//Joystick

int
LGL_JoyNumber()
{
	return(LGL.JoyNumber);
}

const char*
LGL_JoyName
(
	int	which
)
{
	char* returnme;
	returnme=new char[256];

	sprintf(returnme,"LGL_Joystick[%i]: %s",which,LGL.JoystickName[which]);
	return(returnme);

	if
	(
		LGL_VidCamAvailable() &&
		which==LGL_VidCamJoyNumber()
	)
	{
		sprintf
		(
			returnme,
			"%s (%ix%i) @ (%ix%i)",
			LGL.VidCamName,
			LGL.VidCamWidthMax,
			LGL.VidCamHeightMax,
			LGL.VidCamWidthNow,
			LGL.VidCamHeightNow
		);
		return(returnme);
	}

	if(which==1000)
	{
		LGL_Exit();
	}
	
	int Major=0;
	int Minor=0;
	if(LGL.JoyDuality[0]==false && LGL.JoyDuality[1]==false)
	{
		sprintf
		(
			returnme,
			"LGL_Joystick[%i]: %s",
			which,LGL.JoystickName[which]
		);
		return(returnme);
	}
	if(LGL.JoyDuality[0]==true && LGL.JoyDuality[1]==false)
	{
		if(which==0 || which==1)
		{
			Major=0;
		}
		else
		{
			sprintf
			(
				returnme,
				"LGL_Joystick(%i): %s",
				which,LGL.JoystickName[which]
			);
			return(returnme);
		}
		if(which==0)
		{
			Minor=0;
		}
		if(which==1)
		{
			Minor=1;
		}
		sprintf
		(
			returnme,
			"LGL_Joystick(%i): %s (%i of 2)",
			which,LGL.JoystickName[Major],Minor+1
		);
		return(returnme);
	}
	if(LGL.JoyDuality[0]==true && LGL.JoyDuality[1]==true)
	{
		if(which==0 || which==1)
		{
			Major=0;
		}
		else
		{
			Major=1;
		}
		if(which==0 || which==2)
		{
			Minor=0;
		}
		if(which==1 || which==3)
		{
			Minor=1;
		}
		sprintf
		(
			returnme,
			"LGL_Joystick(%i): %s (%i of 2)",
			which,LGL.JoystickName[Major],Minor+1
		);
		return(returnme);
	}
	if(LGL.JoyDuality[0]==false && LGL.JoyDuality[1]==true)
	{
		if(which==0)
		{
			sprintf
			(
				returnme,
				"LGL_Joystick(%i): %s",
				which,LGL.JoystickName[which]
			);
			return(returnme);
		}
		else
		{
			Major=1;
		}
		if(which==1)
		{
			Minor=0;
		}
		if(which==2)
		{
			Minor=1;
		}
		sprintf
		(
			returnme,
			"LGL_Joystick(%i): %s (%i of 2)",
			which,LGL.JoystickName[Major],Minor+1
		);
		return(returnme);
	}
	printf("LGL_JoyName(): Error, unable to find name of Joystick #%i\n",which);
	LGL_Exit();
}

void
lgl_JoySanityCheck
(
	int	Joystick,
	int	Button,
	int	Side,
	int	Axis
)
{
	if(Joystick>=4)
	{
		printf("lgl_JoySanityCheck(): LGL only supports 4 joysticks! (You requested #%i)\n",Joystick);
		LGL_Exit();
	}
	if(Joystick<-1)
	{
		printf("lgl_JoySanityCheck(): Joysticks %i is invalid! You can't have a negative joystick (except -1, which means any)!\n",Joystick);
		LGL_Exit();
	}
	if(Button>=32 || Button<0)
	{
		printf("lgl_JoySanityCheck(): There is no button %i!\n",Button);
		LGL_Exit();
	}
	if(Side<0 || Side>1)
	{
		printf("lgl_JoySanityCheck(): Invalid Side parameter (%i)\n",Side);
		LGL_Exit();
	}
	if(Axis<0 || Axis>1)
	{
		printf("lgl_JoySanityCheck(): Invalid Axis parameter (%i)\n",Axis);
		LGL_Exit();
	}
}

bool
LGL_JoyDown
(
	int	Joystick,
	int	Button
)
{
	lgl_JoySanityCheck
	(
		Joystick,Button,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS
	);
	if(Joystick==-1)
	{
		return
		(
			LGL.JoyDown[0][Button] ||
			LGL.JoyDown[1][Button] ||
			LGL.JoyDown[2][Button] ||
			LGL.JoyDown[3][Button]
		);
	}
	else
	{
		return(LGL.JoyDown[Joystick][Button]);
	}
}

bool
LGL_JoyStroke
(
	int	Joystick,
	int	Button
)
{
	lgl_JoySanityCheck
	(
		Joystick,Button,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS
	);
	if(Joystick==-1)
	{
		return
		(
			LGL.JoyStroke[0][Button] ||
			LGL.JoyStroke[1][Button] ||
			LGL.JoyStroke[2][Button] ||
			LGL.JoyStroke[3][Button]
		);
	}
	else
	{
		return(LGL.JoyStroke[Joystick][Button]);
	}
}

bool
LGL_JoyRelease
(
	int	Joystick,
	int	Button
)
{
	lgl_JoySanityCheck
	(
		Joystick,Button,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS
	);
	if(Joystick==-1)
	{
		return
		(
			LGL.JoyRelease[0][Button] ||
			LGL.JoyRelease[1][Button] ||
			LGL.JoyRelease[2][Button] ||
			LGL.JoyRelease[3][Button]
		);
	}
	else
	{
		return(LGL.JoyRelease[Joystick][Button]);
	}
}

float
LGL_JoyAnalogueStatus
(
	int	Joystick,
	int	Side,
	int	Axis
)
{
	lgl_JoySanityCheck
	(
		Joystick,LGL_JOY_SQUARE,Side,Axis
	);
	if(Joystick==-1)
	{
		for(unsigned int a=0;a<4;a++)
		{
			if(LGL.JoyAnalogue[a][Side][Axis]!=0)
			{
				return(LGL.JoyAnalogue[a][Side][Axis]);
			}
		}

		return(0);
	}
	return(LGL.JoyAnalogue[Joystick][Side][Axis]);
}

bool
LGL_JoyReset
(
	int	Joystick
)
{
	lgl_JoySanityCheck
	(
		Joystick,LGL_JOY_SQUARE,LGL_JOY_ANALOGUE_L,LGL_JOY_XAXIS
	);
	if(Joystick==-1)
	{
		return
		(
			LGL_JoyReset(0) ||
			LGL_JoyReset(1) ||
			LGL_JoyReset(2) ||
			LGL_JoyReset(3)
		);
	}
	else
	{
		return
		(
			LGL_JoyDown(Joystick,LGL_JOY_L1) &&
			LGL_JoyDown(Joystick,LGL_JOY_R1) &&
			LGL_JoyDown(Joystick,LGL_JOY_L2) &&
			LGL_JoyDown(Joystick,LGL_JOY_R2) &&
			LGL_JoyDown(Joystick,LGL_JOY_START) &&
			LGL_JoyDown(Joystick,LGL_JOY_SELECT)
		);
	}
}

//Wiimote

#ifdef	LGL_LINUX_WIIMOTE

void
lgl_WiimoteCallbackGeneric
(
	int			which,
	int			count,
	union cwiid_mesg*	mesg
)
{
	if(LGL.Running)
	{
		for(int a=0;a<count;a++)
		{
			LGL.Wiimote[which].INTERNAL_Callback(&(mesg[a]));
		}
	}
}

void lgl_WiimoteCallback0(cwiid_wiimote_t* wiimote, int count, union cwiid_mesg *mesg, timespec* ts) { lgl_WiimoteCallbackGeneric(0,count,mesg); }
void lgl_WiimoteCallback1(cwiid_wiimote_t* wiimote, int count, union cwiid_mesg *mesg, timespec* ts) { lgl_WiimoteCallbackGeneric(1,count,mesg); }
void lgl_WiimoteCallback2(cwiid_wiimote_t* wiimote, int count, union cwiid_mesg *mesg, timespec* ts) { lgl_WiimoteCallbackGeneric(2,count,mesg); }
void lgl_WiimoteCallback3(cwiid_wiimote_t* wiimote, int count, union cwiid_mesg *mesg, timespec* ts) { lgl_WiimoteCallbackGeneric(3,count,mesg); }
void lgl_WiimoteCallback4(cwiid_wiimote_t* wiimote, int count, union cwiid_mesg *mesg, timespec* ts) { lgl_WiimoteCallbackGeneric(4,count,mesg); }
void lgl_WiimoteCallback5(cwiid_wiimote_t* wiimote, int count, union cwiid_mesg *mesg, timespec* ts) { lgl_WiimoteCallbackGeneric(5,count,mesg); }
void lgl_WiimoteCallback6(cwiid_wiimote_t* wiimote, int count, union cwiid_mesg *mesg, timespec* ts) { lgl_WiimoteCallbackGeneric(6,count,mesg); }
void lgl_WiimoteCallback7(cwiid_wiimote_t* wiimote, int count, union cwiid_mesg *mesg, timespec* ts) { lgl_WiimoteCallbackGeneric(7,count,mesg); }

#endif	//LGL_LINUX_WIIMOTE

int
lgl_WiimoteListenThread
(
	void* wiimotePtr
)
{
	LGL_Wiimote* wiimote = (LGL_Wiimote*)wiimotePtr;
	while(wiimote->Connected()==false)
	{
		//The user has a minute to connect Wiimotes.
		//Why only a minute? Cell phones with bluetooth can crash this bit of the code inside libbluetooth. FUCK.
		if(LGL_SecondsSinceExecution()<60.0f)
		{
			wiimote->INTERNAL_Connect();
		}
		else
		{
			return(0);
		}
		LGL_DelayMS(50);
	}
	return(0);
}

int LGL_Wiimote::IDCounter = 0;

LGL_Wiimote::
LGL_Wiimote() :
	WiimoteSemaphore("Wiimote Wiimote"),
	ButtonArraySemaphore("Wiimote ButtonArray"),
	PointerMotionSemaphore("Wiimote PointerMotion")
{
	assert(IDCounter>=0 && IDCounter < 8);
	ID=IDCounter;
	IDCounter++;
	Rumble=false;
	for(int a=0;a<4;a++)
	{
		LEDState[a]=false;
	}
	PointerFront.SetXYZ(.5f,.5f,0);
	PointerBack.SetXYZ(.5f,.5f,0);
	PointerGreatestIRSourceDistance=0;
	Battery=0.0f;
	ConnecterThread=NULL;
	Wiimote=NULL;
	Extension=LGL_WIIMOTE_EXTENSION_NONE;
	Reset();
}

LGL_Wiimote::
~LGL_Wiimote()
{
	//
}

void
LGL_Wiimote::
ListenForConnection(bool listen)
{
return;
	if(Connected())
	{
		return;
	}

	if
	(
		ConnecterThread==NULL &&
		listen
	)
	{
		ConnecterThread=LGL_ThreadCreate(lgl_WiimoteListenThread,this);
		if(ConnecterThread==NULL)
		{
			printf("Wow!\n");
		}
	}
	else if
	(
		ConnecterThread!=NULL &&
		listen==false
	)
	{
		LGL_ThreadKill(ConnecterThread);
	}
}

bool
LGL_Wiimote::
IsListeningForConnection()
{
	if
	(
		Connected() ||
		ConnecterThread==NULL
	)
	{
		return(false);
	}
	else
	{
		return(true);
	}
}

bool
LGL_Wiimote::
Connected()
{
	return(Wiimote!=NULL);
}

void
LGL_Wiimote::
Disconnect()
{
	if(Wiimote==NULL)
	{
		return;
	}

#ifdef	LGL_LINUX_WIIMOTE
	{
		LGL_ScopeLock lock(__FILE__,__LINE__,WiimoteSemaphore);
		cwiid_close(Wiimote);
		Wiimote=NULL;
	}
#endif	//LGL_LINUX_WIIMOTE

}

int
lgl_Wiimote_SetRumble
(
	void* wiimotePtr
)
{
#ifdef	LGL_LINUX_WIIMOTE
	LGL_Wiimote* wiimote = (LGL_Wiimote*)wiimotePtr;
	if(LGL_Semaphore* semaphore = wiimote->INTERNAL_GetWiimoteSemaphore())
	{
		LGL_ScopeLock lock(__FILE__,__LINE__,semaphore);
		{
			cwiid_command(wiimote->INTERNAL_GetWiimote(), CWIID_CMD_RUMBLE, wiimote->GetRumble());
		}
	}
#endif	//LGL_LINUX_WIIMOTE

	return(0);
}

void
LGL_Wiimote::
SetRumble
(
	bool	status,
	float	seconds
)
{
	Rumble=status;
	RumbleSeconds=seconds;
	if(Wiimote)
	{
#ifdef	LGL_LINUX_WIIMOTE
		{
			LGL_ScopeLock lock(__FILE__,__LINE__,WiimoteSemaphore);
			cwiid_command(Wiimote, CWIID_CMD_RUMBLE, Rumble);
		}
#endif	//LGL_LINUX_WIIMOTE
	}
}

void
LGL_Wiimote::
SetLED
(
	int	which,
	bool	status
)
{
	LEDState[which] = status;

#ifdef	LGL_LINUX_WIIMOTE
	if(Wiimote)
	{
		unsigned char ledState=0;
		if(LEDState[0]) ledState |= CWIID_LED1_ON;
		if(LEDState[1]) ledState |= CWIID_LED2_ON;
		if(LEDState[2]) ledState |= CWIID_LED3_ON;
		if(LEDState[3]) ledState |= CWIID_LED4_ON;

		{
			LGL_ScopeLock lock(__FILE__,__LINE__,WiimoteSemaphore);
			cwiid_command(Wiimote, CWIID_CMD_LED, ledState);
		}
	}
#endif	//LGL_LINUX_WIIMOTE
}

bool
LGL_Wiimote::
GetRumble()
{
	return(Rumble);
}

float
LGL_Wiimote::
GetBattery()
{
	if(Wiimote==NULL)
	{
		return(0.0f);
	}
	else
	{
		return(Battery);
	}
}

bool
LGL_Wiimote::
ButtonDown(int which)
{
	return(Wiimote && ButtonDownArrayFront[which]);
}

bool
LGL_Wiimote::
ButtonStroke(int which)
{
	return(Wiimote && ButtonStrokeArrayFront[which]);
}

bool
LGL_Wiimote::
ButtonRelease(int which)
{
	return(Wiimote && ButtonReleaseArrayFront[which]);
}

bool
LGL_Wiimote::
GetPointerAvailable()
{
	if(Wiimote==NULL) return(false);

	return(PointerFront.GetZ()>0);
}

float
LGL_Wiimote::
GetPointerX()
{
	return(PointerFront.GetX());
}

float
LGL_Wiimote::
GetPointerY()
{
	return(PointerFront.GetY());
}

std::vector<LGL_Vector>
LGL_Wiimote::
GetPointerMotionThisFrame()
{
	return(PointerMotionThisFrameFront);
}

void
LGL_Wiimote::
DrawPointerIRSources
(
	float	left,	float	right,
	float	bottom,	float	top
)
{
	if(Wiimote==NULL) return;

	float width = right-left;
	float height = top-bottom;

	float dotRadius = 0.01f*.5f*(width+height);
	if(dotRadius < 0.01f*0.25f)
	{
		dotRadius = 0.01f*0.25f;
	}

	for(int a=0;a<4;a++)
	{
		if(PointerIRSources[a].GetZ()==1)
		{
			LGL_DrawRectToScreen
			(
				left+PointerIRSources[a].GetX()*width-dotRadius,
				left+PointerIRSources[a].GetX()*width+dotRadius,
				bottom+PointerIRSources[a].GetY()*height-dotRadius,
				bottom+PointerIRSources[a].GetY()*height+dotRadius
			);
		}
	}
}

LGL_Vector
LGL_Wiimote::
GetAccelRaw()
{
	return(AccelFront);
}

LGL_Vector
LGL_Wiimote::
GetAccelRawNormalized()
{
	LGL_Vector ret=AccelFront;
	ret.SetLength(ret.GetLength()/8.0f);
	return(ret);
}

void
LGL_Wiimote::
DrawAccelGraph
(
	float	left,	float	right,
	float	bottom,	float	top
)
{
	if(Wiimote==NULL) return;

	float width = right-left;
	float height = top-bottom;

	float vertexData[120];

	for(int c=0;c<3;c++)
	{
		float localLeft=left+(c)*width*(1.0f/3.0f);
		float localRight=left+(1.0f/3.0f)*width+(c)*width*(1.0f/3.0f);
		float localWidth=localRight-localLeft;
		for(unsigned int v=0;v<60;v++)
		{
			float acc = 0.0f;
			if(AccelPast.size() > v)
			{
				if(c==0) acc = AccelPast[v].GetX()/8.0f;
				if(c==1) acc = AccelPast[v].GetY()/8.0f;
				if(c==2) acc = AccelPast[v].GetZ()/8.0f;
			}
			vertexData[v*2+0] = localLeft+localWidth*.5f+localWidth*.5f*acc;
			vertexData[v*2+1] = bottom+height*(v/59.0f);
		}
		LGL_DrawLineStripToScreen
		(
			vertexData,
			60,
			0.4f,//(c==0) ? 1 : 0,
			0.2f,//(c==1) ? 1 : 0,
			1.0f,//(c==2) ? 1 : 0,
			1,
			2.0f,
			true
		);
	}
}

bool
LGL_Wiimote::
GetFlickXPositive()
{
	return(FlickXPositiveNow);
}

bool
LGL_Wiimote::
GetFlickXNegative()
{
	return(FlickXNegativeNow);
}

int
LGL_Wiimote::
GetExtension()
{
	return(Wiimote ? Extension : 0);
}

bool
LGL_Wiimote::
INTERNAL_Connect()
{
#ifdef	LGL_LINUX_WIIMOTE_OFF
	bdaddr_t bdany;
	for(int a=0;a<6;a++) bdany.b[a]=0;	//Want to use BDADDR_ANY, but that results in a compiler warning... Hmm...

	Wiimote = NULL;
	Wiimote = cwiid_open(&bdany,0);

	if(Wiimote)
	{
		Reset();
		
		if(ID==0)
		{
			printf("Callback set\n");
			cwiid_set_mesg_callback(Wiimote,lgl_WiimoteCallback0);
		}
		if(ID==1) cwiid_set_mesg_callback(Wiimote,lgl_WiimoteCallback1);
		if(ID==2) cwiid_set_mesg_callback(Wiimote,lgl_WiimoteCallback2);
		if(ID==3) cwiid_set_mesg_callback(Wiimote,lgl_WiimoteCallback3);
		if(ID==4) cwiid_set_mesg_callback(Wiimote,lgl_WiimoteCallback4);
		if(ID==5) cwiid_set_mesg_callback(Wiimote,lgl_WiimoteCallback5);
		if(ID==6) cwiid_set_mesg_callback(Wiimote,lgl_WiimoteCallback6);
		if(ID==7) cwiid_set_mesg_callback(Wiimote,lgl_WiimoteCallback7);

		unsigned char reportMode =
			CWIID_RPT_STATUS |
			CWIID_RPT_BTN |
			CWIID_RPT_IR |
			CWIID_RPT_ACC |
			CWIID_RPT_EXT;
		cwiid_command(Wiimote, CWIID_CMD_RPT_MODE, reportMode);

		cwiid_enable(Wiimote,CWIID_FLAG_MESG_IFC);

		cwiid_command(Wiimote, CWIID_CMD_STATUS, 0);	//Why do we do this?

		for(int a=0;a<4;a++)
		{
			SetLED(a,a==ID);
		}
	}
#endif

	return(Wiimote!=NULL);
}

#ifdef	LGL_LINUX_WIIMOTE

void
LGL_Wiimote::
INTERNAL_Callback
(
	union cwiid_mesg* mesg
)
{
	if(mesg->type==CWIID_MESG_STATUS)
	{
		struct cwiid_status_mesg status_mesg = mesg->status_mesg;
		Battery = floorf(100.0f*status_mesg.battery/(float)CWIID_BATTERY_MAX)/100.0f;
		if(status_mesg.ext_type==CWIID_EXT_NONE)
		{
			Extension = LGL_WIIMOTE_EXTENSION_NONE;
		}
		else if(status_mesg.ext_type==CWIID_EXT_NUNCHUK)
		{
			if(Extension != LGL_WIIMOTE_EXTENSION_NUNCHUK)
			{
				unsigned char accelCalibration[7];
				if(cwiid_read(Wiimote, CWIID_RW_REG | CWIID_RW_DECODE, 0xA40020, 7, accelCalibration))
				{
					for(int a=0;a<2;a++)
					{
						for(int b=0;b<3;b++)
						{
							AccelCalibrationNunchuk[a][b]=accelCalibration[a*3+b];
						}
					}
				}
			}
			Extension = LGL_WIIMOTE_EXTENSION_NUNCHUK;
		}
		else if(status_mesg.ext_type==CWIID_EXT_CLASSIC)
		{
			Extension = LGL_WIIMOTE_EXTENSION_CLASSIC;
		}
	}
	else if(mesg->type==CWIID_MESG_BTN)
	{
		struct cwiid_btn_mesg btn_mesg = mesg->btn_mesg;
		unsigned short buttons = btn_mesg.buttons;
		{
			LGL_ScopeLock lock(__FILE__,__LINE__,ButtonArraySemaphore);
			INTERNAL_UpdateButton(LGL_WIIMOTE_LEFT,(buttons & CWIID_BTN_LEFT));
			INTERNAL_UpdateButton(LGL_WIIMOTE_RIGHT,(buttons & CWIID_BTN_RIGHT));
			INTERNAL_UpdateButton(LGL_WIIMOTE_DOWN,(buttons & CWIID_BTN_DOWN));
			INTERNAL_UpdateButton(LGL_WIIMOTE_UP,(buttons & CWIID_BTN_UP));
			INTERNAL_UpdateButton(LGL_WIIMOTE_A,(buttons & CWIID_BTN_A));
			INTERNAL_UpdateButton(LGL_WIIMOTE_B,(buttons & CWIID_BTN_B));
			INTERNAL_UpdateButton(LGL_WIIMOTE_MINUS,(buttons & CWIID_BTN_MINUS));
			INTERNAL_UpdateButton(LGL_WIIMOTE_PLUS,(buttons & CWIID_BTN_PLUS));
			INTERNAL_UpdateButton(LGL_WIIMOTE_HOME,(buttons & CWIID_BTN_HOME));
			INTERNAL_UpdateButton(LGL_WIIMOTE_1,(buttons & CWIID_BTN_1));
			INTERNAL_UpdateButton(LGL_WIIMOTE_2,(buttons & CWIID_BTN_2));
		}
	}
	else if(mesg->type==CWIID_MESG_ACC)
	{
		struct cwiid_acc_mesg acc_mesg = mesg->acc_mesg;
		AccelBack.SetXYZ
		(
			(acc_mesg.acc[0]-134.25f)/(128.0f*.215),
			(acc_mesg.acc[1]-134.25f)/(128.0f*.215),
			(acc_mesg.acc[2]-134.25f)/(128.0f*.215)
		);
	}
	else if(mesg->type==CWIID_MESG_IR)
	{
		struct cwiid_ir_mesg ir_mesg = mesg->ir_mesg;
		int validSources=0;
		float x=0;
		float y=0;

		//Calculate Sources
		for(int a=0;a<CWIID_IR_SRC_COUNT;a++)
		{
			cwiid_ir_src src = ir_mesg.src[a];
			if(src.valid)
			{
				PointerIRSources[a].SetXYZ
				(
					src.pos[0]/(float)CWIID_IR_X_MAX,
					src.pos[1]/(float)CWIID_IR_Y_MAX,
					1
				);
				validSources++;
				x+=PointerIRSources[a].GetX();
				y+=PointerIRSources[a].GetY();
			}
			else
			{
				PointerIRSources[a].SetZ(-1);
			}
		}

		//Calculate Greatest Distance
		float nextPointerGreatestIRSourceDistance=0.0f;
		for(int a=0;a<validSources-1;a++)
		{
			for(int b=a+1;b<validSources;b++)
			{
				float dist=sqrtf
				(
					powf((PointerIRSources[a].GetX()-PointerIRSources[b].GetX()),2)+
					powf((PointerIRSources[a].GetY()-PointerIRSources[b].GetY()),2)
				);
				if(dist>nextPointerGreatestIRSourceDistance)
				{
					nextPointerGreatestIRSourceDistance=dist;
				}
			}
		}

		//Calculate Pointer
		if
		(
			validSources==4 ||
			(
				validSources==2 &&
				(
					PointerGreatestIRSourceDistance==0 ||
					fabsf(nextPointerGreatestIRSourceDistance-PointerGreatestIRSourceDistance)<0.1f
				)
			)
		)
		{
			PointerBack.SetXYZ
			(
				.5f+(-2*((x/validSources)-0.5f)),
				.5f+(-2*((y/validSources)-0.5f)),
				validSources
			);
			
			{
				LGL_ScopeLock lock(__FILE__,__LINE__,PointerMotionSemaphore);
				PointerMotionThisFrameBack.push_back(PointerBack);
			}
			
			PointerGreatestIRSourceDistance=nextPointerGreatestIRSourceDistance;
		}
		else if(validSources==0)
		{
			PointerBack.SetZ(0);
			PointerGreatestIRSourceDistance=0;
		}
	}
	else if(mesg->type==CWIID_MESG_NUNCHUK)
	{
		printf("Nunchuk!\n");
	}
	else if(mesg->type==CWIID_MESG_CLASSIC)
	{
		printf("Classic!\n");
	}
	else
	{
		printf("WTF?\n");
	}
}

#endif	//LGL_LINUX_WIIMOTE

void
LGL_Wiimote::
INTERNAL_ProcessInput()
{
	if(Connected()==false)
	{
		return;
	}

	for(int a=0;a<11;a++)
	{
		ButtonDownArrayFront[a] = ButtonDownArrayBack[a];
		ButtonStrokeArrayFront[a] = ButtonStrokeArrayBack[a];
		ButtonReleaseArrayFront[a] = ButtonReleaseArrayBack[a];

		ButtonDownArrayBack[a] = ButtonDownArrayBack[a] && !ButtonReleaseArrayBack[a];
		ButtonStrokeArrayBack[a] = false;
		ButtonReleaseArrayBack[a] = false;
	}

	PointerFront=PointerBack;

	{
		LGL_ScopeLock lock(__FILE__,__LINE__,PointerMotionSemaphore);
		PointerMotionThisFrameFront.clear();
		PointerMotionThisFrameFront=PointerMotionThisFrameBack;
		PointerMotionThisFrameBack.clear();
	}

	AccelFront=AccelBack;
	AccelPast.push_back(AccelBack);
	if(AccelPast.size()>60)
	{
		AccelPast.erase((std::vector<LGL_Vector>::iterator)(&(AccelPast[0])));
	}

	FlickXNegativeNow = (GetAccelRaw().GetX() < -2.0f && FlickXNegativeDebounce > -0.25f);
	FlickXPositiveNow = (GetAccelRaw().GetX() >  2.0f && FlickXPositiveDebounce <  0.25f);
	if(FlickXPositiveNow) FlickXPositiveDebounce = GetAccelRaw().GetX();
	if(FlickXNegativeNow) FlickXNegativeDebounce = GetAccelRaw().GetX();
	if(FlickXPositiveDebounce > GetAccelRaw().GetX())
	{
		FlickXPositiveDebounce = GetAccelRaw().GetX();
	}
	if(FlickXNegativeDebounce < GetAccelRaw().GetX())
	{
		FlickXNegativeDebounce = GetAccelRaw().GetX();
	}

	if(RumbleSeconds>0)
	{
		RumbleSeconds-=LGL_SecondsSinceLastFrame();
		if(RumbleSeconds<=0)
		{
			RumbleSeconds=0;
			SetRumble(false);
		}
	}
}

void
LGL_Wiimote::Reset()
{
	Rumble=false;
	for(int a=0;a<4;a++)
	{
		LEDState[a]=false;
	}

	if(LGL.WiimoteSemaphore)
	{
		LGL_ScopeLock lock(__FILE__,__LINE__,LGL.WiimoteSemaphore);
		for(int a=0;a<11;a++)
		{
			ButtonDownArrayFront[a]=false;
			ButtonStrokeArrayFront[a]=false;
			ButtonReleaseArrayFront[a]=false;

			ButtonDownArrayBack[a]=false;
			ButtonStrokeArrayBack[a]=false;
			ButtonReleaseArrayBack[a]=false;
		}
	}

#ifdef	LGL_LINUX_WIIMOTE
	if(Wiimote)
	{
		unsigned char accelCalibration[7];
		if(cwiid_read(Wiimote, CWIID_RW_EEPROM, 0x16, 7, accelCalibration))
		{
			for(int a=0;a<2;a++)
			{
				for(int b=0;b<3;b++)
				{
					AccelCalibration[a][b]=accelCalibration[a*3+b];
				}
			}
		}
	}
#endif	//LGL_LINUX_WIIMOTE
}

void
LGL_Wiimote::
INTERNAL_UpdateButton
(
	int	which,
	bool	pressed
)
{
	bool justStroked = ButtonDownArrayFront[which]==false && pressed==true;
	ButtonStrokeArrayBack[which] = ButtonStrokeArrayBack[which] || justStroked;
	
	bool justReleased = ButtonDownArrayFront[which]==true && pressed==false;
	ButtonReleaseArrayBack[which] = ButtonReleaseArrayBack[which] || justReleased;

	ButtonDownArrayBack[which] = (ButtonDownArrayBack[which] || pressed) && !justReleased;
}

#ifdef	LGL_LINUX_WIIMOTE

cwiid_wiimote_t*
LGL_Wiimote::
INTERNAL_GetWiimote()
{
	return(Wiimote);
}

#endif	//LGL_LINUX_WIIMOTE

LGL_Semaphore*
LGL_Wiimote::
INTERNAL_GetWiimoteSemaphore()
{
	return(&WiimoteSemaphore);
}

LGL_Wiimote&
LGL_GetWiimote
(
	int	which
)
{
	assert(which>=0 && which<8);
	return(LGL.Wiimote[which]);
}



//MIDI

int lgl_MidiInit2()
{
	LGL.Xponent=NULL;
	LGL.Xsession=NULL;
	LGL.TriggerFinger=NULL;
	LGL.JP8k=NULL;

	try
	{
		LGL.MidiRtIn = new RtMidiIn();
	}
	catch(RtError& error)
	{
		printf("lgl_MidiInit2(): Error!\n");
		error.printMessage();
		return(-1);
	}

	for(unsigned int a=0; a<LGL.MidiRtIn->getPortCount(); a++)
	{
		try
		{
			//Make a new RtMidiIn
			RtMidiIn* midiRtIn = new RtMidiIn();

			//Get the name
			std::string str = midiRtIn->getPortName(a);
			char* name = new char[2048];
			strcpy(name,str.c_str());
			LGL.MidiRtInDeviceNames.push_back(name);

			//Open the port
			midiRtIn->openPort(a);

			LGL.MidiRtInDevice.push_back(midiRtIn);
		}
		catch(RtError &error)
		{
			printf("lgl_MidiInit2(): Error!\n");
			error.printMessage();
			return(-1);
		}
	}

	//Create corresponding LGL_MidiDevices
	for(unsigned int a=0;a<LGL_MidiDeviceCount();a++)
	{
		if
		(
			strcmp(LGL_MidiDeviceName(a),"Xponent:0")==0 ||
			strcmp(LGL_MidiDeviceName(a),"Xponent Port 1")==0
		)
		{
			//We have an Xponent!
			LGL.Xponent = new LGL_MidiDeviceXponent;
			LGL.Xponent->DeviceID = a;
		}
		else if
		(
			strcmp(LGL_MidiDeviceName(a),"Xponent:1")==0 ||
			strcmp(LGL_MidiDeviceName(a),"Xponent Port 2")==0
		)
		{
			//Meh
		}
		else if
		(
			strcmp(LGL_MidiDeviceName(a),"USB X-Session:0")==0 ||
			strcmp(LGL_MidiDeviceName(a),"USB X-Session Port 1")==0
		)
		{
			//We have an Xsession!
			LGL.Xsession = new LGL_MidiDevice;
			LGL.Xsession->DeviceID = a;
		}
		else if
		(

			strcmp(LGL_MidiDeviceName(a),"USB X-Session:1")==0 ||
			strcmp(LGL_MidiDeviceName(a),"USB X-Session Port 2")==0
		)
		{
			//Meh
		}
		else if(strcmp(LGL_MidiDeviceName(a),"USB Trigger Finger MIDI 1")==0)
		{
			//We have a TriggerFinger!
			LGL.TriggerFinger = new LGL_MidiDevice;
			LGL.TriggerFinger->DeviceID = a;
		}
		else if(strcmp(LGL_MidiDeviceName(a),"USB Uno MIDI Interface MIDI 1")==0)
		{
			//We have a JP8k!
			LGL.JP8k = new LGL_MidiDevice;
			LGL.JP8k->DeviceID = a;
		}
		else
		{
			printf("Unsupported MIDI Device: '%s'\n",LGL_MidiDeviceName(a));
		};
	}

	return(0);
}

bool done;
static void finish(int meh)
{
	done=true;
}

int lgl_MidiUpdate()
{
	if(LGL.MidiRtIn==NULL)
	{
		return(0);
	}

	for(unsigned int a=0;a<LGL.MidiRtInDevice.size();a++)
	{
		RtMidiIn* midiRtIn = LGL.MidiRtInDevice[a];
		LGL_MidiDevice* device=NULL;
		if
		(
			strcmp(LGL_MidiDeviceName(a),"Xponent:0")==0 ||
			strcmp(LGL_MidiDeviceName(a),"Xponent Port 1")==0
		)
		{
			device = LGL.Xponent;
		}
		if
		(
			strcmp(LGL_MidiDeviceName(a),"USB X-Session:0")==0 ||
			strcmp(LGL_MidiDeviceName(a),"USB X-Session Port 1")==0
		)
		{
			device = LGL.Xsession;
		}

		done=false;
		typedef void (*sighandler)(int);
		sighandler oldie = signal(SIGINT, finish);
		while(done==false)
		{
			std::vector<unsigned char> message;
			midiRtIn->getMessage(&message);

			if(message.size()==0)
			{
				break;
			}
			if(device==NULL)
			{
				continue;
			}

			/*
			for(unsigned int a=0;a<message.size();a++)
			{
				printf("\tByte[%i] = %i\n",a,message[a]);
			}
			*/
			
			if(device==LGL.Xponent)
			{
				//Xponent!
				if(message[0]==176)
				{
					//Left Knob
					unsigned char knobWhich=message[1];
					unsigned char knobValue=message[2];

					{
						LGL_ScopeLock lock(__FILE__,__LINE__,device->BackBufferSemaphore);
						device->KnobTweakBack[knobWhich]=true;
						device->KnobStatusBack[knobWhich]=knobValue;
					}
				}
				else if(message[0]==177)
				{
					//Right Knob
					int knobWhich=message[1]+100;
					unsigned char knobValue=message[2];
					{
						LGL_ScopeLock lock(__FILE__,__LINE__,device->BackBufferSemaphore);
						device->KnobTweakBack[knobWhich]=true;
						device->KnobStatusBack[knobWhich]=knobValue;
					}
				}
				else if(message[0]==224)
				{
					//Left Pitchbend
					unsigned char knobWhich=30;	//o_0
					unsigned char knobValue=message[2];
					{
						LGL_ScopeLock lock(__FILE__,__LINE__,device->BackBufferSemaphore);
						device->KnobTweakBack[knobWhich]=true;
						device->KnobStatusBack[knobWhich]=knobValue;
					}
				}
				else if(message[0]==225)
				{
					//Right Pitchbend
					int knobWhich=30+100;	//o_0
					unsigned char knobValue=message[2];
					{
						LGL_ScopeLock lock(__FILE__,__LINE__,device->BackBufferSemaphore);
						device->KnobTweakBack[knobWhich]=true;
						device->KnobStatusBack[knobWhich]=knobValue;
					}
				}
				else if(message[0]==178)
				{
					//Xfader or touchpad or Cue
					if(message[1]==7)
					{
						//Xfader
						unsigned char knobWhich=91;
						unsigned char knobValue=message[2];
						{
							LGL_ScopeLock lock(__FILE__,__LINE__,device->BackBufferSemaphore);
							device->KnobTweakBack[knobWhich]=true;
							device->KnobStatusBack[knobWhich]=knobValue;
						}
					}
					else if(message[1]==8)
					{
						//Touchpad X
						unsigned char knobWhich=92;
						unsigned char knobValue=message[2];
						{
							LGL_ScopeLock lock(__FILE__,__LINE__,device->BackBufferSemaphore);
							device->KnobTweakBack[knobWhich]=true;
							device->KnobStatusBack[knobWhich]=knobValue;
						}
					}
					else if(message[1]==9)
					{
						//Touchpad Y
						unsigned char knobWhich=93;
						unsigned char knobValue=message[2];
						{
							LGL_ScopeLock lock(__FILE__,__LINE__,device->BackBufferSemaphore);
							device->KnobTweakBack[knobWhich]=true;
							device->KnobStatusBack[knobWhich]=knobValue;
						}
					}
					else if(message[1]==13)
					{
						unsigned char knobWhich=90;
						unsigned char knobValue=message[2];
						{
							LGL_ScopeLock lock(__FILE__,__LINE__,device->BackBufferSemaphore);
							device->KnobTweakBack[knobWhich]=true;
							device->KnobStatusBack[knobWhich]=knobValue;
						}
					}
				}
				else if(message[0]==144)
				{
					//Left Button Stroke
					int button = message[1];
					{
						LGL_ScopeLock lock(__FILE__,__LINE__,device->BackBufferSemaphore);
						device->ButtonStrokeBack[button]=true;
						device->ButtonForceBack[button]=1.0f;
						device->ButtonDownBack[button]=true;
						device->ButtonReleaseBack[button]=false;
					}
				}
				else if(message[0]==128)
				{
					//Left Button Release
					int button = message[1];
					{
						LGL_ScopeLock lock(__FILE__,__LINE__,device->BackBufferSemaphore);
						device->ButtonStrokeBack[button]=false;
						device->ButtonForceBack[button]=0.0f;
						device->ButtonDownBack[button]=false;
						device->ButtonReleaseBack[button]=true;
					}
				}
				else if(message[0]==145)
				{
					//Right Button Stroke
					int button = message[1] + 100;
					{
						LGL_ScopeLock lock(__FILE__,__LINE__,device->BackBufferSemaphore);
						device->ButtonStrokeBack[button]=true;
						device->ButtonForceBack[button]=1.0f;
						device->ButtonDownBack[button]=true;
						device->ButtonReleaseBack[button]=false;
					}
				}
				else if(message[0]==129)
				{
					//Right Button Release
					int button = message[1] + 100;
					{
						LGL_ScopeLock lock(__FILE__,__LINE__,device->BackBufferSemaphore);
						device->ButtonStrokeBack[button]=false;
						device->ButtonForceBack[button]=0.0f;
						device->ButtonDownBack[button]=false;
						device->ButtonReleaseBack[button]=true;
					}
				}
			}

			if(device==LGL.Xsession)
			{
				//Xsession!
				if(message[0]==176)
				{
					//Knob
					unsigned char knobWhich=message[1];
					unsigned char knobValue=message[2];
					{
						LGL_ScopeLock lock(__FILE__,__LINE__,LGL_GetXsession()->BackBufferSemaphore);
						LGL_GetXsession()->KnobTweakBack[knobWhich]=true;
						LGL_GetXsession()->KnobStatusBack[knobWhich]=knobValue;
					}
				}
				else if(message[0]==144)
				{
					//Button
					int button = message[1];
					bool down = (message[2]==127);
					{
						LGL_ScopeLock lock(__FILE__,__LINE__,LGL_GetXsession()->BackBufferSemaphore);
						bool wasDown=device->ButtonDownBack[button];
						if
						(
							wasDown==false &&
							down
						)
						{
							//Stroke!
							LGL_GetXsession()->ButtonStrokeBack[button]=true;
							LGL_GetXsession()->ButtonForceBack[button]=1.0f;
							LGL_GetXsession()->ButtonDownBack[button]=true;
							LGL_GetXsession()->ButtonReleaseBack[button]=false;
						}
						else if
						(
							wasDown==true &&
							down==false
						)
						{
							//Release
							LGL_GetXsession()->ButtonStrokeBack[button]=false;
							LGL_GetXsession()->ButtonForceBack[button]=0.0f;
							LGL_GetXsession()->ButtonDownBack[button]=false;
							LGL_GetXsession()->ButtonReleaseBack[button]=true;
						}
						else
						{
							printf("Strange button stroke...\n");
						}
					}
				}
			}
		}

		signal(SIGINT,oldie);
	}

	return(0);
}

unsigned int
LGL_MidiDeviceCount()
{
	return(LGL.MidiRtInDeviceNames.size());
}

const char*
LGL_MidiDeviceName
(
	unsigned int	which
)
{
	if(which >= LGL_MidiDeviceCount())
	{
		printf("LGL_MidiDeviceName(%i): Error! [0 < arg < %i] violated!\n",which,LGL_MidiDeviceCount());
		assert(which<LGL_MidiDeviceCount());
	}
	return(LGL.MidiRtInDeviceNames[which]);
}



float
LGL_MidiClockBPM()
{
	float ret=-1;
	ret=120.0f;
	return(ret);
}

float
LGL_MidiClockPercentOfCurrentMeasure()
{
	float ret=-1;
	float sec=LGL_SecondsSinceExecution();
	while(sec>=2.0f)	//This is horrid
	{
		sec-=2.0f;
	}
	ret=sec/2.0f;
	return(ret);
}



//LGL_MidiDevice

LGL_MidiDevice::
LGL_MidiDevice() : BackBufferSemaphore("MidiDevice BackBuffer")
{
	DeviceID=-1;

	{
		LGL_ScopeLock lock(__FILE__,__LINE__,BackBufferSemaphore);
		for(int a=0;a<LGL_MIDI_CONTROL_MAX;a++)
		{
			ButtonStrokeFront[a]=false;
			ButtonStrokeBack[a]=false;
			ButtonDownFront[a]=false;
			ButtonDownBack[a]=false;
			ButtonReleaseFront[a]=false;
			ButtonReleaseBack[a]=false;
			ButtonTimer[a].Reset();
			ButtonForceFront[a]=0.0f;
			ButtonForceBack[a]=0.0f;
			KnobTweakFront[a]=false;
			KnobTweakBack[a]=false;
			KnobStatusFront[a]=-127.0f;
			KnobStatusBack[a]=-127.0f;
		}
	}
}

LGL_MidiDevice::
~LGL_MidiDevice()
{
	//
}

bool
LGL_MidiDevice::GetButtonStroke
(
	int	button
)
{
	return(ButtonStrokeFront[button]);
}

bool
LGL_MidiDevice::GetButtonDown
(
	int	button
)
{
	return(ButtonDownFront[button]);
}

bool
LGL_MidiDevice::GetButtonRelease
(
	int	button
)
{
	return(ButtonReleaseFront[button]);
}

float
LGL_MidiDevice::GetButtonTimer
(
	int	button
)
{
	return(ButtonTimer[button].SecondsSinceLastReset());
}

float
LGL_MidiDevice::GetButtonForce
(
	int	button
)
{
	return(ButtonForceFront[button]/127.0f);
}

bool
LGL_MidiDevice::GetKnobTweak
(
	int	knob
)
{
	return(KnobTweakFront[knob]);
}

float
LGL_MidiDevice::GetKnobStatus
(
	int	knob
)
{
	return(KnobStatusFront[knob]/127.0f);
}

void
LGL_MidiDevice::
LGL_INTERNAL_SwapBuffers()
{
	{
		LGL_ScopeLock lock(__FILE__,__LINE__,BackBufferSemaphore);
		for(int a=0;a<LGL_MIDI_CONTROL_MAX;a++)
		{
			if
			(
				ButtonStrokeBack[a] ||
				ButtonReleaseBack[a]
			)
			{
				ButtonTimer[a].Reset();
			}
			ButtonStrokeFront[a]=ButtonStrokeBack[a];
			ButtonStrokeBack[a]=false;
			ButtonForceFront[a]=ButtonForceBack[a];
			ButtonForceBack[a]=0.0f;
			ButtonDownFront[a]=ButtonDownBack[a];
			ButtonReleaseFront[a]=ButtonReleaseBack[a];
			ButtonReleaseBack[a]=false;
			KnobTweakFront[a]=KnobTweakBack[a];
			KnobTweakBack[a]=false;
			KnobStatusFront[a]=KnobStatusBack[a];
		}
	}
}

LGL_MidiDeviceXponent::
LGL_MidiDeviceXponent()
{
	TurntableTweakIndex=0;
	for(int a=0;a<lgl_turntable_tweak_history_size;a++)
	{
		TurntableTweakHistoryL[a]=false;
		TurntableTweakHistoryR[a]=false;
	}
}

LGL_MidiDeviceXponent::
~LGL_MidiDeviceXponent()
{
	//
}

void
LGL_MidiDeviceXponent::
LGL_INTERNAL_SwapBuffers()
{
	//Right Turntable
	TurntableTweakHistoryL[TurntableTweakIndex]=KnobTweakBack[22];
	TurntableTweakHistoryR[TurntableTweakIndex]=KnobTweakBack[122];
	TurntableTweakIndex++;
	if(TurntableTweakIndex==lgl_turntable_tweak_history_size)
	{
		TurntableTweakIndex=0;
	}

	if
	(
		KnobStatusBack[22]>66 ||
		KnobStatusBack[22]<62
	)
	{
		for(int a=0;a<lgl_turntable_tweak_history_size;a++)
		{
			TurntableTweakHistoryL[a]=true;
		}
	}
	if
	(
		KnobStatusBack[122]>66 ||
		KnobStatusBack[122]<62
	)
	{
		for(int a=0;a<lgl_turntable_tweak_history_size;a++)
		{
			TurntableTweakHistoryR[a]=true;
		}
	}

	int tweakCountL=0;
	int tweakCountR=0;
	for(int a=0;a<lgl_turntable_tweak_history_size;a++)
	{
		if(TurntableTweakHistoryL[a])
		{
			tweakCountL++;
		}
		if(TurntableTweakHistoryR[a])
		{
			tweakCountR++;
		}
	}
	float tweakPercentL=tweakCountL/(float)(lgl_turntable_tweak_history_size);
	float tweakPercentR=tweakCountR/(float)(lgl_turntable_tweak_history_size);

	if
	(
		KnobStatusBack[22]>=62 &&
		KnobStatusBack[22]<=63
	)
	{
		KnobStatusBack[22]=63-tweakPercentL;
	}
	if
	(
		KnobStatusBack[22]>=65 &&
		KnobStatusBack[22]<=66
	)
	{
		KnobStatusBack[22]=65+tweakPercentL;
	}

	if
	(
		KnobStatusBack[122]>=62 &&
		KnobStatusBack[122]<=63
	)
	{
		KnobStatusBack[122]=63-tweakPercentR;
	}
	if
	(
		KnobStatusBack[122]>=65 &&
		KnobStatusBack[122]<=66
	)
	{
		KnobStatusBack[122]=65+tweakPercentR;
	}

	LGL_MidiDevice::LGL_INTERNAL_SwapBuffers();
}

LGL_MidiDeviceXponent*
LGL_GetXponent()
{
	return(LGL.Xponent);
}

LGL_MidiDevice*
LGL_GetXsession()
{
	return(LGL.Xsession);
}

LGL_MidiDevice*
LGL_GetTriggerFinger()
{
	return(LGL.TriggerFinger);
}

LGL_MidiDevice*
LGL_GetJP8k()
{
	return(LGL.JP8k);
}



//OSC

int
lgl_OscServer_thread
(
	void*	object
)
{
	LGL_ThreadSetPriority(LGL_PRIORITY_OSC,"Osc");
	LGL_OscServer* oscServer = (LGL_OscServer*)object;
	oscServer->ThreadFunc();
	return(0);
}

LGL_OscServer::
LGL_OscServer
(
	int	port
) :
	ListeningReceiveSocket
	(
		IpEndpointName
		(
			IpEndpointName::ANY_ADDRESS,
			port
		),
		this
	)
{
	Thread=LGL_ThreadCreate(lgl_OscServer_thread,this);
}

LGL_OscServer::
~LGL_OscServer()
{
	//Cleanup Thread
	{
		ListeningReceiveSocket.AsynchronousBreak();
		LGL_ThreadWait(Thread);
		Thread=NULL;
	}
}

void
LGL_OscServer::
ThreadFunc()
{
	ListeningReceiveSocket.Run();
}

void
LGL_OscServer::
ProcessMessage
(
	const osc::ReceivedMessage&	m,
	const IpEndpointName&		remoteEndpoint
)
{
	try
	{
		printf("OSC Message: %s\n",m.AddressPattern());
	}
	catch(osc::Exception& e)
	{
		// any parsing errors such as unexpected argument types, or 
		// missing arguments get thrown as exceptions.
		std::cout << "error while parsing message: "
			<< m.AddressPattern() << ": " << e.what() << "\n";
	}
}

//OSC Client

LGL_OscClient::
LGL_OscClient
(
	const char*	address,
	int		port
) :	TransmitSocket
	(
		IpEndpointName
		(
			address,
			port
		)
	),
	PacketStream
	(
		PacketStreamBuffer,
		PacketStreamBufferBytes
	)
{
	IpEndpointName
	(
		address,
		port
	).AddressAsString(Address);
}

LGL_OscClient::
~LGL_OscClient()
{
	//
}

osc::OutboundPacketStream&
LGL_OscClient::
Stream()
{
	return(PacketStream);
}

void
LGL_OscClient::
Send()
{
	TransmitSocket.Send(PacketStreamBuffer,PacketStreamBufferBytes);

	osc::OutboundPacketStream freshStream(PacketStreamBuffer,PacketStreamBufferBytes);
	PacketStream=freshStream;
}

const char*
LGL_OscClient::
GetAddress()
{
	return(Address);
}



//Syphon
int
LGL_SyphonServerCount()
{
#ifdef LGL_OSX
	return(lgl_SyphonServerCount());
#else
	return(0);
#endif
}

LGL_Image*
LGL_SyphonImage
(
	int	serverIndex
)
{
#ifdef LGL_OSX
	if(LGL.SyphonImage==NULL)
	{
		LGL.SyphonImage = new LGL_Image("data/image/logo.png");
	}

	GLuint id;
	int w;
	int h;
	bool ok=lgl_SyphonImageInfo(serverIndex,id,w,h);
	if(ok==false)
	{
		return(NULL);
	}

	LGL.SyphonImage->TextureGLRect=true;
	LGL.SyphonImage->TexW=w;
	LGL.SyphonImage->TexH=h;
	LGL.SyphonImage->ImgW=w;
	LGL.SyphonImage->ImgH=h;
	LGL.SyphonImage->TextureGL=id;
	LGL.SyphonImage->InvertY=true;

	return(LGL.SyphonImage);
#else
	return(NULL);
#endif
}

void
LGL_SyphonPushImage(LGL_Image* img)
{
#ifdef LGL_OSX
	lgl_SyphonPushImage
	(
		img->TextureGL,
		img->GetWidth(),
		img->GetHeight(),
		img->GetTexWidth(),
		img->GetTexHeight()
	);
#else
	//Do nothing
#endif
}



//VidCam

bool
LGL_VidCamAvailable()
{
	return(LGL.VidCamAvailable);
}

int
LGL_VidCamJoyNumber()
{
	if(LGL_VidCamAvailable()==false)
	{
		printf("LGL_VidCamJoyNumber(): Warning! No VidCam attached!\n");
		printf("LGL_VidCamJoyNumber(): Don't call this fn unless there is one!\n");
		printf("LGL_VidCamJoyNumber(): (Use LGL_VidCamAvailable()...)!\n");
	}	
	return(LGL_JoyNumber()-1);
}

int
LGL_VidCamFPS()
{
	if(LGL_VidCamAvailable())
	{
		return((int)(floor(1.0/LGL.VidCamCaptureDelay)));
	}
	else
	{
		printf("LGL_VidCamFPS(): Warning! No VidCam attached!\n");
		printf("LGL_VidCamFPS(): Don't call this fn unless there is one!\n");
		printf("LGL_VidCamJoyNumber(): (Use LGL_VidCamAvailable()...)!\n");
		return(0);
	}
}

LGL_Image*
LGL_VidCamImageRaw()
{
	if(LGL.VidCamAvailable==false)
	{
		printf("LGL_VidCamImageRaw(): Warning! No VidCam attached!\n");
		printf("LGL_VidCamImageRaw(): Don't call this function unless there is one!\n");
		printf("LGL_VidCamJoyNumber(): (Use LGL_VidCamAvailable()...)!\n");
	}
	if(LGL.VidCamImageRaw==NULL)
	{
		LGL.VidCamImageRaw=new LGL_Image
		(
			LGL.VidCamWidthNow,LGL.VidCamHeightNow,
			3,
			LGL.VidCamBufferRaw
		);
	}
	return(LGL.VidCamImageRaw);
}

LGL_Image*
LGL_VidCamImageProcessed()
{
	if(LGL.VidCamAvailable==false)
	{
		printf("LGL_VidCamImageRaw(): Warning! No VidCam attached!\n");
		printf("LGL_VidCamImageRaw(): Don't call this function unless there is one!\n");
		printf("LGL_VidCamJoyNumber(): (Use LGL_VidCamAvailable()...)!\n");
	}
	if(LGL.VidCamImageProcessed==NULL)
	{
		LGL.VidCamImageProcessed=new LGL_Image
		(
			LGL.VidCamWidthNow,LGL.VidCamHeightNow,
			3,
			LGL.VidCamBufferProcessed
		);
	}

	return(LGL.VidCamImageProcessed);
}

void
LGL_VidCamCalibrate
(
	LGL_Image*	Background,
	float		FadeSeconds
)
{
	LGL_Font& Font=LGL_GetFont();
	float b=0;

	if(LGL_VidCamAvailable()==false)
	{
		printf("LGL_VidCamCalibrate(): Unable to find /dev/video0...\n");
		printf("Calibration aborting...\n");
		return;
	}
	
	int Phase=1;

	LGL.VidCamDistanceThumb=999;
	LGL.VidCamDistancePointer=999;

	int DistThumb=-70;
	int DistPointer=-80;
	LGL_Timer DistTimer;
	LGL_Timer IncreasingTimer;

	bool Exiting=false;

	for(;;)
	{
		//Process Input

		LGL_ProcessInput();

		if(LGL_KeyStroke(SDLK_f))
		{
			//LGL_FullScreenToggle();
		}
		if(LGL_KeyStroke(SDLK_SPACE))
		{
			if(Phase!=3) Phase++;
			if(Phase==3)
			{
				DistTimer.Reset();
			}
			if(Phase==5)
			{
				IncreasingTimer.Reset();
				Exiting=true;
			}
		}
		if(Phase==4)
		{
			if(LGL_KeyStroke(SDLK_r))
			{
				Phase=1;
				LGL.VidCamDistanceThumb=999;
				LGL.VidCamDistancePointer=999;
				DistThumb=-70;
				DistPointer=-80;
			}
		}
		
		//Do stuff below here

		if(Exiting==false)
		{
			b=IncreasingTimer.SecondsSinceLastReset()/FadeSeconds;
			if(b>1) b=1;
		}
		else
		{
			b=1.0-IncreasingTimer.SecondsSinceLastReset()/FadeSeconds;
			if(b<-.25) return;
			if(b<0) b=0;
		}

		if(Background!=NULL)
		{
			Background->DrawToScreen
			(
				0,1,
				0,1,
				0,
				b,b,b,b
			);
		}

		Font.DrawString
		(
			.5,.925,.05,
			b,b,b,1,
			true,1,
			"VidCam Calibration"
		);

		LGL_DrawRectToScreen
		(
			.04,.46,
			.04,.46,
			0,0,.5*b,b
		);
		LGL_DrawRectToScreen
		(
			.54,.96,
			.04,.46,
			0,0,.5*b,b
		);

		if(LGL_VidCamAvailable())
		{
			
			LGL_VidCamImageRaw()->DrawToScreen
			(
				.05,.45,
				.05,.44,
				0,
				b,b,b,b
			);
			LGL_VidCamImageProcessed()->DrawToScreen
			(
				.55,.95,
				.05,.45,
				0,
				b,b,b,b
			);
		}

		//Do stuff above here

		Font.DrawString
		(
			.25,.455,.035,
			b,b,b,b,
			true,1,
			"Raw Input"
		);
		Font.DrawString
		(
			.75,.455,.035,
			b,b,b,b,
			true,1,
			"Processed Input"
		);

		//Phase 1: Instruct User To Set Up VidCam
	
		if(Phase==1)
		{	
			Font.DrawString
			(
				.05,.825,.040,
				b,b,b,b,
				false,1,
				"Phase 1 of 3: (spacebar procedes)"
			);
			Font.DrawString
			(
				.05,.75,.035,
				b,b,b,b,
				false,1,
				"* Position your VidCam approx 1.25 feet"
			);
			Font.DrawString
			(
				.05,.70,.035,
				b,b,b,b,
				false,1,
				"  above the plane of movement."
			);
			Font.DrawString
			(
				.05,.65,.035,
				b,b,b,b,
				false,1,
				"* The room should be well lit."
			);
			Font.DrawString
			(
				.05,.60,.035,
				b,b,b,b,
				false,1,
				"* The green cross should be in the middle."
			);
			Font.DrawString
			(
				.05,.55,.035,
				b,b,b,b,
				false,1,
				"  of the empty 'Processed Input' box."
			);
		}
		if(Phase==2)
		{	
			Font.DrawString
			(
				.05,.825,.040,
				b,b,b,b,
				false,1,
				"Phase 2 of 3: (spacebar procedes)"
			);
			Font.DrawString
			(
				.05,.75,.035,
				b,b,b,b,
				false,1,
				"* With either hand, make an 'L' with your"
			);
			Font.DrawString
			(
				.05,.70,.035,
				b,b,b,b,
				false,1,
				"  pointer and thumb, palm up."
			);
			Font.DrawString
			(
				.05,.65,.035,
				b,b,b,b,
				false,1,
				"* The green cross should be more or less"
			);
			Font.DrawString
			(
				.05,.60,.035,
				b,b,b,b,
				false,1,
				"  in the middle."
			);
			Font.DrawString
			(
				.05,.55,.035,
				b,b,b,b,
				false,1,
				"* Keep your hand like this for phase 3."
			);
		}
		if(Phase==3)
		{	
			Font.DrawString
			(
				.05,.825,.040,
				b,b,b,b,
				false,1,
				"Phase 3 of 3: (auto-procede)"
			);
			Font.DrawString
			(
				.05,.75,.035,
				b,b,b,b,
				false,1,
				"* Keep you hand more or less still"
			);
			Font.DrawString
			(
				.05,.70,.035,
				b,b,b,b,
				false,1,
				"  as the VidCam is calibrated."
			);
			Font.DrawString
			(
				.05,.65,.035,
				b,b,b,b,
				false,1,
				"* Your hand should still be shaped"
			);
			Font.DrawString
			(
				.05,.60,.035,
				b,b,b,b,
				false,1,
				"  like an 'L', palm up"
			);
			Font.DrawString
			(
				.05,.55,.035,
				b,b,b,b,
				false,1,
				"* This should take about 5 seconds."
			);

			if(DistTimer.SecondsSinceLastReset()>.1)
			{				
				bool ResetMe=false;

				if
				(
					LGL_JoyDown(LGL_VidCamJoyNumber(),LGL_JOY_CROSS)==false &&
					LGL_JoyDown(LGL_VidCamJoyNumber(),LGL_JOY_CIRCLE)==false
				)
				{
					DistThumb++;
					ResetMe=true;
				}
				if
				(
					LGL_JoyDown(LGL_VidCamJoyNumber(),LGL_JOY_SQUARE)==false &&
					LGL_JoyDown(LGL_VidCamJoyNumber(),LGL_JOY_TRIANGLE)==false
				)
				{
					DistPointer++;
					ResetMe=true;
				}

				LGL.VidCamDistanceThumb=DistThumb;
				LGL.VidCamDistancePointer=DistPointer;

				if
				(
					(
						LGL_JoyDown(LGL_VidCamJoyNumber(),LGL_JOY_SQUARE) ||
						LGL_JoyDown(LGL_VidCamJoyNumber(),LGL_JOY_TRIANGLE)
					) &&
					(
						LGL_JoyDown(LGL_VidCamJoyNumber(),LGL_JOY_CIRCLE)
						||
						LGL_JoyDown(LGL_VidCamJoyNumber(),LGL_JOY_CROSS) 
					) &&
					DistTimer.SecondsSinceLastReset()>1
				)
				{
					DistThumb+=1;
					DistPointer+=1;

					LGL.VidCamDistanceThumb=DistThumb;
					LGL.VidCamDistancePointer=DistPointer;
					Phase++;

					FILE* writeme=fopen(".lgl-vidcam-calibration","w");
					if(writeme!=NULL)
					{
						char tempstr[64];
						sprintf(tempstr,"%i\n",DistThumb);
						fputs(tempstr,writeme);
						sprintf(tempstr,"%i\n",DistPointer);
						fputs(tempstr,writeme);
						fclose(writeme);
					}
					else
					{
						printf("fopen(./.lgl-vidcam-calibration) failed\n");
					}
				}
				if(ResetMe) DistTimer.Reset();
			}
		}
		if(Phase>=4)
		{	
			Font.DrawString
			(
				.05,.825,.040,
				b,b,b,b,
				false,1,
				"Success! (spacebar exits)"
			);
			Font.DrawString
			(
				.05,.75,.035,
				b,b,b,b,
				false,1,
				"* Try movie your hand and"
			);
			Font.DrawString
			(
				.05,.70,.035,
				b,b,b,b,
				false,1,
				"  activating buttons."
			);
			Font.DrawString
			(
				.05,.65,.035,
				b,b,b,b,
				false,1,
				"* If you're happy, press spacebar"
			);
			Font.DrawString
			(
				.05,.60,.035,
				b,b,b,b,
				false,1,
				"* To recalibrate, press 'r'"
			);
		}

		LGL_SwapBuffers();
	}
}

//Networking

LGL_Datagram::
LGL_Datagram()
{
	Meta=NULL;
	Data=NULL;
	MetaArgv=NULL;
	ClearData();
}

LGL_Datagram::
~LGL_Datagram()
{
	if(Meta!=NULL) delete Meta;
	if(Data!=NULL) delete Data;
	if(MetaArgv!=NULL) delete MetaArgv;
}

void
LGL_Datagram::
ClearData()
{
	argv.clear();
	if(Meta!=NULL) delete Meta;
	if(Data!=NULL) delete Data;
	if(MetaArgv!=NULL) delete MetaArgv;
	MetaLength=0;
	Meta=NULL;
	DataLength=0;
	Data=NULL;
	MetaArgv=NULL;
}

bool
LGL_Datagram::
LoadData
(
	FILE*	file
)
{
	printf("LGL_Datagram::LoadData(): Implement me!\n");
	return(false);
}

bool
LGL_Datagram::
LoadData
(
	char*	inMeta,
	Uint16	inDataLength,
	Uint8*	inData
)
{
	ClearData();

	if
	(
		inMeta!=NULL &&
		strlen(inMeta)>0
	)
	{
		Meta=new char[strlen(inMeta)+1];
	}
	else
	{
		MetaLength=0;
		Meta=NULL;
	}
	if(inMeta!=NULL && Meta!=NULL)
	{
		MetaLength=strlen(inMeta)+1;
		memcpy(Meta,inMeta,strlen(inMeta)+1);
	}

	DataLength=inDataLength;
	if(DataLength>0)
	{
		Data=new Uint8[DataLength];
	}
	else
	{
		Data=NULL;
	}
	if(inData!=NULL && Data!=NULL)
	{
		memcpy(Data,inData,DataLength);
	}

	MetaArgv=new char[MetaLength];
	memcpy(MetaArgv,inMeta,MetaLength);
	argv.push_back(MetaArgv);
	for(;;)
	{
		char* n=strstr(argv[argv.size()-1],"|");
		if(n)
		{
			argv.push_back(&n[1]);
			n[0]='\0';
		}
		else
		{
			break;
		}
	}

	return(true);
}

bool
LGL_Datagram::
LoadData
(
	Uint16	inMetaLength,
	char*	inMeta,
	Uint16	inDataLength,
	Uint8*	inData
)
{
	ClearData();

	MetaLength=inMetaLength;
	if
	(
		inMeta!=NULL
	)
	{
		Meta=new char[MetaLength];
		memcpy(Meta,inMeta,MetaLength);
	}
	else
	{
		Meta=NULL;
	}

	DataLength=inDataLength;
	if(DataLength>0)
	{
		Data=new Uint8[DataLength];
	}
	else
	{
		Data=NULL;
	}
	if(inData!=NULL && Data!=NULL)
	{
		memcpy(Data,inData,DataLength);
	}

	MetaArgv=new char[MetaLength];
	memcpy(MetaArgv,inMeta,MetaLength);
	argv.push_back(MetaArgv);
	for(;;)
	{
		char* n=strstr(argv[argv.size()-1],"|");
		if(n)
		{
			argv.push_back(&n[1]);
			n[0]='\0';
		}
		else
		{
			break;
		}
	}

	return(true);
}

unsigned int
LGL_Datagram::
Argc()	const
{
	return(argv.size());
}

const
char*
LGL_Datagram::
Argv
(
	unsigned int	index
)	const
{
	assert(index<argv.size());
	return(argv[index]);
}

Uint16
LGL_Datagram::
GetMetaLength()	const
{
	return(MetaLength);
}

const
char*
LGL_Datagram::
GetMeta() const
{
	return(Meta);
}

Uint16
LGL_Datagram::
GetDataLength()	const
{
	return(DataLength);
}

const
Uint8*
LGL_Datagram::
GetData() const
{
	return(Data);
}

int lgl_NetConnectionResolverHost(void* object);

int
lgl_NetConnectionResolverIP
(
	void*	object
)
{
	LGL_NetConnection* nc=(LGL_NetConnection*)object;
	if
	(
		LGL_NetHostToIP
		(
			nc->Host,
			nc->IP[0],
			nc->IP[1],
			nc->IP[2],
			nc->IP[3]
		)==false
	)
	{
		for(int a=0;a<4;a++) nc->IP[a]=-1;
		sprintf
		(
			nc->IPstr,
			"IP Unresolvable"
		);
	}
	else
	{
		sprintf
		(
			nc->IPstr,
			"%i.%i.%i.%i",
			nc->IP[0],
			nc->IP[1],
			nc->IP[2],
			nc->IP[3]
		);
	}
	if(strcmp(nc->Host,nc->IPstr)==0)
	{
		nc->Host[0]='\0';
		lgl_NetConnectionResolverHost(object);
	}
	
	nc->ResolverThreadIP=NULL;
	return(0);
}

int
lgl_NetConnectionResolverHost
(
	void*	object
)
{
	LGL_NetConnection* nc=(LGL_NetConnection*)object;
	
	LGL_NetIPToHost
	(
		nc->Host,
		nc->IP[0],
		nc->IP[1],
		nc->IP[2],
		nc->IP[3]
	);

	nc->ResolverThreadHost=NULL;
	return(0);
}

int lgl_NetConnectionSocketSendThread(void* object);
int lgl_NetConnectionSocketRecvThread(void* object);

int
lgl_NetConnectionConnectTCP
(
	void*	object
)
{
	LGL_NetConnection* nc=(LGL_NetConnection*)object;

	IPaddress ip;
	Uint8* ip4=(Uint8*)&ip.host;
	for(int a=0;a<4;a++) ip4[a]=nc->IP[a];
	ip.port=nc->Port;

	nc->SocketTCP=SDLNet_TCP_Open(&ip);
	nc->Connection_Status=(nc->SocketTCP==NULL)?-1:2;
	if(nc->Connection_Status==2)
	{
		nc->SocketSendThread=LGL_ThreadCreate(lgl_NetConnectionSocketSendThread,object);
		nc->SocketRecvThread=LGL_ThreadCreate(lgl_NetConnectionSocketRecvThread,object);
	}
	nc->ConnectTCPThread=NULL;
	return(0);
}

void
lgl_NetConnectionEvilPathScan
(
	char*	path
)
{
	//Detects attempts to break out of our (non)chroot jail

	if(path==NULL)
	{
		return;
	}
	if
	(
		strstr(path,"../")!=NULL ||
		strstr(path,"~")!=NULL ||
		strstr(path,"/")==path ||
#ifdef	LGL_WIN32
		strstr(path,":\\")!=NULL ||
		strstr(path,"..\\")!=NULL ||
#endif	//LGL_WIN32
		false
	)
	{
		printf("lgl_NetConnectionEvilPathScan('%s'): Error!! Path is evil. Remote system is hostile. Exiting...\n",path);
		LGL_Exit();
	}
}

int
lgl_NetConnectionSocketSendThread
(
	void*	object
)
{
	LGL_NetConnection* nc=(LGL_NetConnection*)object;

	while(nc->Connection_Status==2)
	{
		bool boring=true;
		if(nc->SendBuffer.empty()==false)
		{
			//send user datagram
			SDL_mutexP(nc->Mutex);
			{
				nc->SendTCP(nc->SendBuffer[0]);
				delete nc->SendBuffer[0];
				nc->SendBuffer.pop_front();
			}
			SDL_mutexV(nc->Mutex);
			boring=false;
		}
		if(nc->FileSendNowFD!=NULL)
		{
			//send file+= datagram
			Uint8* data=new Uint8[1024];
			Uint16 read=fread(data,1,1024,nc->FileSendNowFD);
			bool eof=false;
			
			SDL_mutexP(nc->Mutex);
			{
				nc->FileSendQueuedBytes=(long)LGL_Max
				(
					0,
					nc->FileSendQueuedBytes-read
				);
				nc->FileSendCompletedBytes+=read;
			}
			SDL_mutexV(nc->Mutex);
			
			if(read<1024 || feof(nc->FileSendNowFD))
			{
				eof=true;
				
				SDL_mutexP(nc->Mutex);
				{
					nc->FileSendQueuedFiles=(int)LGL_Max
					(
						0,
						nc->FileSendQueuedFiles-1
					);
					nc->FileSendCompletedFiles++;
				}
				SDL_mutexV(nc->Mutex);
			}
			if(read>0)
			{
				char meta[1024];
				meta[0]='\0';

				SDL_mutexP(nc->Mutex);
				{
					sprintf
					(
						&meta[1],
						"file+=|%s|%li|%li|%i|%li",
						nc->FileSendNowName,		//name
						nc->FileSendNowLength,		//bytes in this file
						nc->FileSendQueuedBytes,	//bytes to go total
						nc->FileSendQueuedFiles,	//files to go total
						nc->FileSendCompletedBytes
					);
				}
				SDL_mutexV(nc->Mutex);

				LGL_Datagram* dg=new LGL_Datagram;
				dg->LoadData
				(
					strlen(&meta[1])+2,
					meta,
					read,
					data
				);
				nc->SendTCP(dg);
				delete dg;
			}
			else
			{
				//perhaps a fileeof datagram?
			}
			delete data;

			if(eof)
			{
				fclose(nc->FileSendNowFD);
				nc->FileSendNowFD=NULL;
			}
			else
			{
				sprintf
				(
					nc->FileSendNowStatus,
					"Sending (%.1f%%)",
					100*ftell(nc->FileSendNowFD)/(float)nc->FileSendNowLength
				);
			}
			boring=false;
		}
		else if(nc->FileSendBuffer[1].empty()==false && nc->FileSendNowFD==NULL)
		{
			//send file= datagram
			nc->FileSendNowFD=fopen(nc->FileSendBuffer[1][0],"r");
			sprintf(nc->FileSendNowName,"%s",nc->FileSendBuffer[1][0]);
			nc->FileSendNowLength=LGL_FileLengthBytes(nc->FileSendNowName);

			SDL_mutexP(nc->Mutex);
			{
				delete nc->FileSendBuffer[1][0];
				nc->FileSendBuffer[1].pop_front();
			}
			SDL_mutexV(nc->Mutex);

			Uint8* data=new Uint8[1024];
			Uint16 read=fread(data,1,1024,nc->FileSendNowFD);

			SDL_mutexP(nc->Mutex);
			{
				nc->FileSendQueuedBytes=(long)LGL_Max
				(
					0,
					nc->FileSendQueuedBytes-read
				);
				nc->FileSendCompletedBytes+=read;
			}
			SDL_mutexV(nc->Mutex);
			
			bool eof=false;
			if(read<1024 || feof(nc->FileSendNowFD))
			{
				eof=true;
				
				SDL_mutexP(nc->Mutex);
				{
					nc->FileSendQueuedFiles=(int)LGL_Max
					(
						0,
						nc->FileSendQueuedFiles-1
					);
					nc->FileSendCompletedFiles++;
				}
				SDL_mutexV(nc->Mutex);
			}

			char meta[1024];
			meta[0]='\0';
			
			SDL_mutexP(nc->Mutex);
			{
				sprintf
				(
					&meta[1],
					"file=|%s|%li|%li|%i|%li",
					nc->FileSendNowName,		//file name
					nc->FileSendNowLength,		//file length
					nc->FileSendQueuedBytes,	//bytes to go total
					nc->FileSendQueuedFiles,	//files to go total
					nc->FileSendCompletedBytes
				);
			}
			SDL_mutexV(nc->Mutex);
			
			LGL_Datagram* dg=new LGL_Datagram;
			dg->LoadData
			(
				strlen(&meta[1])+2,
				meta,
				read,
				data
			);
			nc->SendTCP(dg);
			delete dg;
			delete data;

			if(eof)
			{
				fclose(nc->FileSendNowFD);
				nc->FileSendNowFD=NULL;
			}
			else
			{
				sprintf
				(
					nc->FileSendNowStatus,
					"Sending (%.1f%%)",
					100*ftell(nc->FileSendNowFD)/(float)nc->FileSendNowLength
				);
			}
			boring=false;
		}
		else if(nc->FileSendBuffer[0].empty()==false && nc->FileSendQueryOK)
		{
			//send file? datagram
			sprintf(nc->FileSendNowName,"%s",nc->FileSendBuffer[0][0]);
			nc->FileSendQueryOK=false;
			char* file=nc->FileSendBuffer[0][0];
			char meta[1024];
			meta[0]='\0';
			sprintf(&(meta[1]),"file?|%s|%.0lf",file,LGL_FileLengthBytes(file));
			Uint16 metaLength=strlen(&(meta[1]))+2;
			LGL_Datagram* dg=new LGL_Datagram;
			dg->LoadData
			(
				metaLength,
				meta,
				0,
				NULL
			);
			nc->SendTCP(dg);
			delete dg;
			delete file;
			nc->FileSendBuffer[0].pop_front();
			boring=false;
				
			sprintf(nc->FileSendNowStatus,"Comparing Length / MD5sum");
		}
		if(boring) LGL_DelayMS(10);
	}
	nc->SocketSendThread=NULL;
	return(0);
}

int
lgl_NetConnectionSocketRecvThread
(
	void*	object
)
{
	LGL_NetConnection* nc=(LGL_NetConnection*)object;

	while(nc->Connection_Status==2)
	{
		//Recv MetaLength
		
		Uint16 metalength;
		Uint8* metalengthnow=(Uint8*)&metalength;
		int bytesRead=0;
		int bytesLeft=2;
		while(bytesLeft!=0)
		{
			bytesRead=SDLNet_TCP_Recv(nc->SocketTCP,metalengthnow,bytesLeft);
			bytesLeft-=bytesRead;
			if(bytesRead==0)
			{
				//We lost our connection

				SDL_mutexP(nc->Mutex);
				{
					SDLNet_TCP_Close(nc->SocketTCP);
					nc->Connection_Status=-2;
				}
				SDL_mutexV(nc->Mutex);
			}
		}

		//Recv Meta
			
		char* meta=NULL;
		char* control=NULL;
		if(metalength>0)
		{
			meta=new char[metalength];
			char* metanow=meta;
			Uint16 metaleft=metalength;
			while(metaleft!=0)
			{
				bytesRead=SDLNet_TCP_Recv
				(
					nc->SocketTCP,
					metanow,
					metaleft
				);
				metanow=&metanow[bytesRead];
				metaleft-=bytesRead;
				
				if(bytesRead==0)
				{
					//We lost our connection

					SDL_mutexP(nc->Mutex);
					{
						SDLNet_TCP_Close(nc->SocketTCP);
						nc->Connection_Status=-2;
					}
					SDL_mutexV(nc->Mutex);

					if(meta!=NULL) delete meta;
				}
			}
			bytesRead=metalength;

			if(metalength>1 && meta[0]=='\0')
			{
				//Special LGL_NetConnection Control Datagram

				control=&(meta[1]);	
			}
		}

		//Recv DataLength
	
		Uint16 datalength;
		Uint8* datalengthnow=(Uint8*)&datalength;
		bytesRead=0;
		bytesLeft=2;
		while(bytesLeft!=0)
		{
			bytesRead=SDLNet_TCP_Recv
			(
				nc->SocketTCP,
				datalengthnow,
				bytesLeft
			);
			datalengthnow=&datalengthnow[bytesRead];
			bytesLeft-=bytesRead;
				
			if(bytesRead==0)
			{
				//We lost our connection

				SDL_mutexP(nc->Mutex);
				{
					SDLNet_TCP_Close(nc->SocketTCP);
					nc->Connection_Status=-2;
				}
				SDL_mutexV(nc->Mutex);

				if(meta!=NULL) delete meta;
			}
		}

		//Recv Data

		Uint8* data=NULL;
		if(datalength>0)
		{
			data=new Uint8[datalength];
			Uint8* datanow=data;
			Uint16 dataleft=datalength;
			while(dataleft!=0)
			{
				bytesRead=SDLNet_TCP_Recv
				(
					nc->SocketTCP,
					datanow,
					dataleft
				);
				datanow=&datanow[bytesRead];
				dataleft-=bytesRead;
				
				if(bytesRead==0)
				{
					//We lost our connection

					SDL_mutexP(nc->Mutex);
					{
						SDLNet_TCP_Close(nc->SocketTCP);
						nc->Connection_Status=-2;
					}
					SDL_mutexV(nc->Mutex);

					if(meta!=NULL) delete meta;
					if(data!=NULL) delete data;
				}
			}
			bytesRead=datalength;
		}

		//Process Datagram

		if(control!=NULL)
		{
			//Special LGL_NetConnection Control Packet
			std::vector<char*> argv;
			char* now=control;
			char* next;
			for(;;)
			{
				next=strstr(now,"|");
				if(next!=NULL)
				{
					next[0]='\0';
					now=&(next[1]);

					char* next2=strstr(now,"|");
					if(next2!=NULL) next2[0]='\0';

					char* neo=new char[strlen(now)+1];
					sprintf(neo,"%s",now);
					argv.push_back(neo);
					
					if(next2!=NULL) next2[0]='|';
				}
				else
				{
					break;
				}
			}
			char* cmd=control;
			if(strstr(cmd,"|")!=NULL)
			{
				strstr(cmd,"|")[0]='\0';
			}

			if(strcmp(cmd,"close")==0)
			{
				//Recv Connection is to be closed.
				SDL_mutexP(nc->Mutex);
				{
					SDLNet_TCP_Close(nc->SocketTCP);
					nc->Connection_Status=-2;
				}
				SDL_mutexV(nc->Mutex);

				if(meta!=NULL) delete meta;
				if(data!=NULL) delete data;
			}
			else if
			(
				strcmp(cmd,"mkdir")==0 &&
				nc->RecvFileRequiredPrefix[0]!='/' &&
				strstr
				(
					argv[0],
					nc->RecvFileRequiredPrefix
				)==argv[0]
			)
			{
				//Recv Make a directory

				lgl_NetConnectionEvilPathScan(argv[0]);

				if(LGL_DirectoryExists(argv[0])==false)
				{
					LGL_DirectoryCreate(argv[0]);
				}
			}
			else if
			(
				strcmp(cmd,"file?")==0 &&
				LGL_DirectoryExists(argv[0])==false
			)
			{
				//Recv Do we need an updated version of file?
				//Respond accordingly

				lgl_NetConnectionEvilPathScan(argv[0]);

				if
				(
					LGL_FileExists(argv[0])==false ||
					atol(argv[1])!=LGL_FileLengthBytes
					(
						argv[0]
					)
				)
				{
					//Recv Either the file doesn't exist,
					//Or the length doesn't match.
					//Regardless, we certainly need
					//a new version.
//printf("recv: file? need a new '%s' for sure! sending 'file!' datagram\n",argv[0]);
					char* nmeta=new char[strlen(argv[0])+strlen(argv[1])+16];
					nmeta[0]='\0';
					sprintf
					(
						&nmeta[1],
						"file!|%s",
						argv[0]
					);
					Uint16 nmetalength=strlen
					(
						&nmeta[1]
					)+2;
					LGL_Datagram* dg=new LGL_Datagram;
					dg->LoadData
					(
						nmetalength,
						nmeta,
						0,
						NULL
					);

					nc->SendTCP(dg);
					delete dg;
					delete(nmeta);

					sprintf
					(
						nc->FileRecvNowName,
						"%s",
						argv[0]
					);
				}
				else
				{
					//Recv: The file exists, and the
					//length matches. Annoying.
					//Calculate md5sum, send back
//printf("recv: file? '%s' length matches... need to compare md5sums!\n",argv[0]);
					char md5[32];
					char nmeta[1024];
					nmeta[0]='\0';
					sprintf
					(
						nc->FileRecvNowName,
						"%s",
						argv[0]
					);
					sprintf
					(
						nc->FileRecvNowStatus,
						"MD5sum Calculation"
					);
					sprintf
					(
						&nmeta[1],
						"file!|%s|%s",
						argv[0],
						LGL_MD5sum(argv[0],md5)
					);
					Uint16 nmetalength=strlen
					(
						&nmeta[1]
					)+2;
					LGL_Datagram* dg=new LGL_Datagram;
					dg->LoadData
					(
						nmetalength,
						nmeta,
						0,
						NULL
					);

					nc->SendTCP(dg);
					delete(dg);

					sprintf
					(
						nc->FileRecvNowName,
						"%s",
						argv[0]
					);
					sprintf
					(
						nc->FileRecvNowStatus,
						"MD5sum Comparison"
					);
				}
			}
			else if
			(
				strcmp(cmd,"file!")==0 &&
				LGL_FileExists(argv[0])
			)
			{
				//We might send a file.
				//We certainly will if argv.size()<2
				//Otherwise, we compare the given md5sum,
				//And send only if it doesn't match
				lgl_NetConnectionEvilPathScan(argv[0]);
				sprintf(nc->FileSendNowStatus,"Comparing Length / MD5sum");
				nc->FileSendQueryOK=true;
				char md5[32];
				if
				(
					argv.size()<2 ||
					strcmp
					(
						argv[1],
						LGL_MD5sum(argv[0],md5)
					)!=0
				)
				{
//printf("recv: file! %s: md5sums don't match, or one wasn't supplied (%i)! Sending!\n",argv[0],argv.size());
					char* neo=new char[strlen(argv[0])+1];
					sprintf(neo,"%s",argv[0]);
					SDL_mutexP(nc->Mutex);
					nc->FileSendBuffer[1].push_back
					(
						neo
					);
					SDL_mutexV(nc->Mutex);
				}
				else
				{
//printf("recv: file! '%s': md5sums match, no need to send\n",argv[0]);
					long len;
					char nmeta[1024];
					nmeta[0]='\0';

					SDL_mutexP(nc->Mutex);
					{
						nc->FileSendQueuedFiles=(int)LGL_Max
						(
							0,
							nc->FileSendQueuedFiles-1
						);
						nc->FileSendCompletedFiles++;
						len=LGL_FileLengthBytes(argv[0]);
						nc->FileSendQueuedBytes=(long)LGL_Max
						(
							0,
							nc->FileSendQueuedBytes-len
						);
						nc->FileSendCompletedBytes+=len;
						sprintf
						(
							&nmeta[1],
							"fileskip|%li|%li|%i|%s|%li",
							len,			//bytes in file
							nc->FileSendQueuedBytes,//bytes to go total
							nc->FileSendQueuedFiles,//files to go total
							argv[0],		//name
							nc->FileSendCompletedBytes
						);
					}
					SDL_mutexV(nc->Mutex);

					LGL_Datagram* nd=new LGL_Datagram;
					nd->LoadData
					(
						strlen(&nmeta[1])+2,
						nmeta,
						0,
						NULL
					);
					nc->SendTCP(nd);
					delete nd;
					SDL_mutexV(nc->Mutex);
				}
			}
			else if
			(
				strcmp(cmd,"file=")==0 &&
				nc->RecvFileRequiredPrefix[0]!='/'
			)
			{
				//Recv Erase current file if it exists
				//Dump data into new file

				lgl_NetConnectionEvilPathScan(argv[0]);

				FILE* file=fopen(argv[0],"w");
				if(file!=NULL)
				{
					fwrite(data,1,datalength,file);
					fclose(file);

					SDL_mutexP(nc->Mutex);
					{
						nc->FileRecvQueuedBytes=atol(argv[2]);
						nc->FileRecvCompletedBytes=atol(argv[4]);
						nc->FileRecvQueuedFiles=atoi(argv[3]);
						if(atol(argv[1])==LGL_FileLengthBytes(argv[0]))
						{
							nc->FileRecvCompletedFiles++;
						}
						sprintf
						(
							nc->FileRecvNowName,
							"%s",
							argv[0]
						);
						sprintf
						(
							nc->FileRecvNowStatus,
							"Recving (%.1f%%)",
							100*datalength/(float)atol(argv[1])
						);
					}
					SDL_mutexV(nc->Mutex);
				}
				else
				{
					//Better report the error
printf("Hey0! Couldn't fopen(%s)\n",argv[0]);
				}

			}
			else if
			(
				strcmp(cmd,"file+=")==0 &&
				nc->RecvFileRequiredPrefix[0]!='/' &&
				LGL_FileExists(argv[0])
			)
			{
				//Create new file, if necessary
				//Append data to file.

				lgl_NetConnectionEvilPathScan(argv[0]);

				FILE* file=fopen(argv[0],"a");
				if(file!=NULL)
				{
					fwrite(data,1,datalength,file);
					
					SDL_mutexP(nc->Mutex);
					{
						nc->FileRecvQueuedBytes=atol(argv[2]);
						nc->FileRecvCompletedBytes=atol(argv[4]);
						nc->FileRecvQueuedFiles=atoi(argv[3]);
						if(atol(argv[1])==LGL_FileLengthBytes(argv[0]))
						{
							nc->FileRecvCompletedFiles++;
						}
						sprintf
						(
							nc->FileRecvNowName,
							"%s",
							argv[0]
						);
						sprintf
						(
							nc->FileRecvNowStatus,
							"Recving (%.1f%%)",
							100*ftell(file)/(float)atol(argv[1])
						);
					}
					SDL_mutexV(nc->Mutex);
					
					fclose(file);
				}
				else
				{
					//Better report the error
printf("Hey1! Couldn't fopen(%s)\n",argv[0]);
				}
			}
			else if
			(
				strcmp(cmd,"fileskip")==0
			)
			{
//printf("Recv: fileskip! (%li)\n",atol(argv[0]));
				SDL_mutexP(nc->Mutex);
				{
					nc->FileRecvQueuedBytes=atol(argv[1]);
					nc->FileRecvQueuedFiles=atoi(argv[2]);
					nc->FileRecvCompletedBytes=atol(argv[4]);
					nc->FileRecvCompletedFiles++;
					sprintf
					(
						nc->FileRecvNowName,
						"%s",
						argv[3]
					);
				}
				SDL_mutexV(nc->Mutex);
			}
			else
			{
printf("lgl_NetConnectionSocketRecvThread(): Unknown command: '%s'\n",cmd);
			}

			while(argv.empty()==false)
			{
				delete(argv[argv.size()-1]);
				argv.pop_back();
			}
		}
		else
		{
			//User SendTCP Datagram
//printf("recv: user datagram: '%s' (%i,%i)\n",meta,metalength,datalength);

			SDL_mutexP(nc->Mutex);
			{
				LGL_Datagram* dg=new LGL_Datagram;
				dg->LoadData
				(
					meta,
					datalength,data
				);
				nc->RecvBuffer.push_back(dg);
			}
			SDL_mutexV(nc->Mutex);
		}
		
		SDL_mutexP(nc->Mutex);
		{
			if
			(
				nc->FileRecvQueuedBytes==0 &&
				nc->FileRecvCompletedBytes!=0
			)
			{
				nc->FileRecvNowName[0]='\0';
				sprintf(nc->FileRecvNowStatus,"Complete");
			}
			if
			(
				nc->FileSendQueuedBytes==0 &&
				nc->FileSendCompletedBytes!=0
			)
			{
				nc->FileSendNowName[0]='\0';
				sprintf(nc->FileSendNowStatus,"Complete");
			}
		}
		SDL_mutexV(nc->Mutex);
	}
	nc->SocketRecvThread=NULL;
	return(0);
}

LGL_NetConnection::
LGL_NetConnection()
{
	ConstructorGeneric();
	sprintf(Host,"Server");
	for(int a=0;a<4;a++) IP[a]=255;
	sprintf(IPstr,"255.255.255.255");
}

LGL_NetConnection::
LGL_NetConnection
(
	char*	inHost
)
{
	ConstructorGeneric();

	sprintf(Host,"%s",inHost);
	ResolverThreadIP=LGL_ThreadCreate(lgl_NetConnectionResolverIP,this);
}

LGL_NetConnection::
LGL_NetConnection
(
	int	ip0,
	int	ip1,
	int	ip2,
	int	ip3
)
{
	if
	(
		ip0<0 || ip0>255 ||
		ip1<0 || ip1>255 ||
		ip2<0 || ip2>255 ||
		ip3<0 || ip3>255
	)
	{
		printf
		(
			"LGL_NetConnection(%i,%i,%i,%i): Error! Invalid IP Address.\n",
			ip0,ip1,ip2,ip3
		);
		LGL_Exit();
	}

	ConstructorGeneric();
	
	IP[0]=ip0;
	IP[1]=ip1;
	IP[2]=ip2;
	IP[3]=ip3;
	sprintf(IPstr,"%i.%i.%i.%i",ip0,ip1,ip2,ip3);
	ResolverThreadHost=LGL_ThreadCreate(lgl_NetConnectionResolverHost,this);
}

LGL_NetConnection::
LGL_NetConnection
(
	LGL_NetConnection*	parent,
	TCPsocket		inSocket
)
{
	ConstructorGeneric();
	
	IPaddress* ip=SDLNet_TCP_GetPeerAddress(inSocket);
	Uint8* ip4=(Uint8*)&ip->host;
	for(int a=0;a<4;a++) IP[a]=ip4[a];
	
	sprintf(IPstr,"%i.%i.%i.%i",IP[0],IP[1],IP[2],IP[3]);
	sprintf(Host,"%s",parent->Host);
	Port=parent->Port;
	Connection_Status=2;
	
	SocketTCP=inSocket;

	sprintf(RecvFileRequiredPrefix,"%s",parent->RecvFileRequiredPrefix);

	SocketSendThread=LGL_ThreadCreate(lgl_NetConnectionSocketSendThread,this);
	SocketRecvThread=LGL_ThreadCreate(lgl_NetConnectionSocketRecvThread,this);
}

LGL_NetConnection::
~LGL_NetConnection()
{
	if(Connection_Status>0)
	{
		printf("LGL_NetConnection::~(): Warning! Deleting an open connection... Use CloseTCP() first.\n");
		CloseTCP();
	}

	SDL_mutexP(Mutex);
	{
		if(ResolverThreadHost!=NULL)
		{
			LGL_ThreadKill(ResolverThreadHost);
		}
		if(ResolverThreadIP!=NULL)
		{
			LGL_ThreadKill(ResolverThreadIP);
		}
		if(ConnectTCPThread!=NULL)
		{
			LGL_ThreadKill(ConnectTCPThread);
		}
		if(SocketSendThread!=NULL)
		{
			LGL_ThreadKill(SocketSendThread);
		}
		if(SocketRecvThread!=NULL)
		{
			LGL_ThreadKill(SocketRecvThread);
		}
	}
	SDL_mutexV(Mutex);

	while(SendBuffer.empty()==false)
	{
		delete SendBuffer[0];
		SendBuffer.pop_front();
	}
	while(RecvBuffer.empty()==false)
	{
		delete RecvBuffer[0];
		RecvBuffer.pop_front();
	}
	
	for(int a=0;a<2;a++)
	{
		while(FileSendBuffer[a].empty()==false)
		{
			delete FileSendBuffer[a][0];
			FileSendBuffer[a].pop_front();
		}
	}

	SDL_DestroyMutex(Mutex);
}

void
LGL_NetConnection::
ConstructorGeneric()
{
	if(LGL_NetAvailable()==false)
	{
		printf("LGL_NetConnection::ConstructorGeneric(): Error! Network unavailable.\n");
		LGL_Exit();
	}
	Host[0]='\0';
	for(int a=0;a<4;a++) IP[a]=0;
	IPstr[0]='\0';
	Port=-1;
	Connection_Status=0;
	SocketTCP=NULL;
	
	ResolverThreadHost=NULL;
	ResolverThreadIP=NULL;
	ConnectTCPThread=NULL;
	SocketSendThread=NULL;
	SocketRecvThread=NULL;
	
	FileSendQueryOK=true;
	FileSendNowFD=NULL;
	FileSendNowName[0]='\0';
	FileSendNowLength=0;
	FileSendNowName[0]='\0';
	sprintf(FileSendNowStatus,"No Activity");
	FileRecvNowName[0]='\0';
	sprintf(FileRecvNowStatus,"No Activity");
	
	FileSendQueuedFiles=0;
	FileSendQueuedBytes=0;
	FileSendCompletedFiles=0;
	FileSendCompletedBytes=0;
	FileRecvQueuedFiles=0;
	FileRecvQueuedBytes=0;
	FileRecvCompletedFiles=0;
	FileRecvCompletedBytes=0;
	
	Mutex=SDL_CreateMutex();
	sprintf(RecvFileRequiredPrefix,"/");
}

bool
LGL_NetConnection::
ConnectTCP
(
	int	port
)
{
	if(Connection_Status>0)
	{
		printf("LGL_NetConnection::ConnectTCP(%i): Warning! Already connected...\n",port);
		return(false);
	}
	if(port<=0 || port>32767)
	{
		printf("LGL_NetConnection::ConnectTCP(%i): Error! Port is out of range...\n",port);
		LGL_Exit();
	}
	if(IP[0]==-1)
	{
		Connection_Status=-1;
		return(false);
	}
	if(strcmp(Host,"Server")==0)
	{
		printf("LGL_NetConnection::ConnectTCP(%i): Error! Servers can't establish connections!\n",port);
		LGL_Exit();
	}
	
	Port=port;
	Connection_Status=1;
	ConnectTCPThread=LGL_ThreadCreate(lgl_NetConnectionConnectTCP,this);
	return(true);
}

bool
LGL_NetConnection::
ListenTCP
(
	int	port
)
{
	if(Connection_Status==1)
	{
		printf("LGL_NetConnection::ListenTCP(%i): Warning! Already listening...\n",port);
		return(false);
	}
	if(Connection_Status==2)
	{
		printf("LGL_NetConnection::ListenTCP(%i): Warning! Already connected...\n",port);
		return(false);
	}
	if(port<=0 || port>32767)
	{
		printf("LGL_NetConnection::ListenTCP(%i): Error! Port is invalid\n",port);
		LGL_Exit();
	}
	if(strcmp(Host,"Server")!=0)
	{
		sprintf(Host,"Server");
		for(int a=0;a<4;a++) IP[a]=255;
		sprintf(IPstr,"255.255.255.255");
	}

	Port=port;
	IPaddress ip;
	ip.host=INADDR_ANY;
	ip.port=Port;

	Connection_Status=1;
	SocketTCP=SDLNet_TCP_Open(&ip);
	if(SocketTCP==NULL)
	{
		SocketTCP=NULL;
		Connection_Status=-1;
		return(false);
	}
	else
	{
		return(true);
	}
}

LGL_NetConnection*
LGL_NetConnection::
AcceptTCP()
{
	if(Connection_Status!=1) return(NULL);
	TCPsocket NewSock=SDLNet_TCP_Accept(SocketTCP);
	if(NewSock==NULL) return(NULL);
	
	return(new LGL_NetConnection(this,NewSock));
}

void
LGL_NetConnection::
CloseTCP()
{
	if(SocketTCP!=NULL && Connection_Status==2)
	{
		char closeConnection[8];
		closeConnection[0]='\0';
		sprintf(&(closeConnection[1]),"close");
		Uint16 length=strlen(&(closeConnection[1]))+2;

		SDL_mutexP(Mutex);
		{
			SDLNet_TCP_Send(SocketTCP,&length,2);
			SDLNet_TCP_Send(SocketTCP,&closeConnection,length);
			length=0;
			SDLNet_TCP_Send(SocketTCP,&length,2);
			SDLNet_TCP_Close(SocketTCP);
			SocketTCP=NULL;
			Connection_Status=-2;
			if(SocketRecvThread!=NULL)
			{
				SDL_KillThread(SocketRecvThread);
				SocketRecvThread=NULL;
			}
			if(SocketSendThread!=NULL)
			{
				SDL_KillThread(SocketSendThread);
				SocketSendThread=NULL;
			}
		}
		SDL_mutexV(Mutex);
	}
	else if
	(
		Connection_Status==1
	)
	{
		SDLNet_TCP_Close(SocketTCP);
		if(ConnectTCPThread!=NULL)
		{
			LGL_ThreadKill(ConnectTCPThread);
			ConnectTCPThread=NULL;
		}
		Connection_Status=-1;
	}
	else
	{
		printf("LGL_NetConnection::CloseTCP(): Warning! Closing a closed connection...\n");
	}
		
	SendFileResetStatistics();
	RecvFileResetStatistics();
		
	while(SendBuffer.empty()==false)
	{
		delete SendBuffer[0];
		SendBuffer.pop_front();
	}
	while(RecvBuffer.empty()==false)
	{
		delete RecvBuffer[0];
		RecvBuffer.pop_front();
	}
	
	for(int a=0;a<2;a++)
	{
		while(FileSendBuffer[a].empty()==false)
		{
			delete FileSendBuffer[a][0];
			FileSendBuffer[a].pop_front();
		}
	}

	FileSendNowName[0]='\0';
	FileRecvNowName[0]='\0';
	sprintf(FileSendNowStatus,"No Activity");
	sprintf(FileRecvNowStatus,"No Activity");
}

bool
LGL_NetConnection::
SendTCP
(
	LGL_Datagram* dg
)
{
	SDL_mutexP(Mutex);
	if(Connection_Status!=2)
	{
		SDL_mutexV(Mutex);
		return(false);
	}

	Uint16 metaLength=dg->GetMeta()==NULL?0:dg->GetMetaLength();
	int sent=SDLNet_TCP_Send(SocketTCP,&metaLength,2);
	if(sent!=2)
	{
		Connection_Status=-2;
		SDL_mutexV(Mutex);
		return(false);
	}
	
	if(metaLength>0)
	{
		sent=SDLNet_TCP_Send(SocketTCP,(void*)dg->GetMeta(),metaLength);
		if(sent!=metaLength)
		{
			Connection_Status=-2;
			SDL_mutexV(Mutex);
			return(false);
		}
	}

	Uint16 length=dg->GetDataLength();
	sent=SDLNet_TCP_Send(SocketTCP,&(length),2);
	if(sent!=2)
	{
		Connection_Status=-2;
		SDL_mutexV(Mutex);
		return(false);
	}
	
	if(dg->GetDataLength()>0)
	{
		sent=SDLNet_TCP_Send(SocketTCP,(void*)dg->GetData(),dg->GetDataLength());
		if(sent!=dg->GetDataLength())
		{
			Connection_Status=-2;
			SDL_mutexV(Mutex);
			return(false);
		}
	}

	SDL_mutexV(Mutex);
	return(true);
}

bool
LGL_NetConnection::
SendTCP
(
	char*	inMeta,
	Uint16	inDataLength,
	Uint8*	inData
)
{
	if(Connection_Status<=0)
	{
		return(false);
	}
	LGL_Datagram* dg=new LGL_Datagram;
	dg->LoadData
	(
		inMeta,
		inDataLength,
		inData
	);
	
	SDL_mutexP(Mutex);
	{
		SendBuffer.push_back(dg);
	}
	SDL_mutexV(Mutex);
	
	return(true);
}

bool
LGL_NetConnection::
RecvTCP
(
	LGL_Datagram*	datagram
)
{
	if(RecvBuffer.empty()) return(false);
	datagram->ClearData();
	LGL_Datagram* dg=NULL;

	SDL_mutexP(Mutex);
	{
		dg=RecvBuffer[0];
		RecvBuffer.pop_front();
	}
	SDL_mutexV(Mutex);

	if(dg!=NULL)
	{
		*datagram=*dg;
		return(true);
	}
	else
	{
		return(false);
	}
}

bool
LGL_NetConnection::
PeekTCP()
{
	return(RecvBuffer.empty()==false);
}

bool
LGL_NetConnection::
SendFile
(
	char*	file
)
{
	if
	(
		Connection_Status!=2 ||
		LGL_FileExists(file)==false
	)
	{
		return(false);
	}

	char* neo=new char[strlen(file)+1];
	sprintf(neo,"%s",file);

	SDL_mutexP(Mutex);
	{
		FileSendBuffer[0].push_back(neo);
		FileSendQueuedFiles++;
		FileSendQueuedBytes+=LGL_FileLengthBytes(file);
	}	
	SDL_mutexV(Mutex);

	return(true);
}

bool
LGL_NetConnection::
SendDirectory
(
	char*	dir
)
{
	if(dir==NULL)
	{
		printf("LGL_NetConnection::SendDirectory(dir): Error! dir cannot be NULL\n");
		LGL_Exit();
	}
	if(Connection_Status!=2)
	{
		return(false);
	}

	//Create the remote Subdir (if necessary)

	char cmd[1024];
	cmd[0]='\0';
	sprintf(&(cmd[1]),"mkdir|%s",dir);
	LGL_Datagram* dg=new LGL_Datagram;
	dg->LoadData
	(
		strlen(&(cmd[1]))+2,
		cmd,
		0,
		NULL
	);
	
	SDL_mutexP(Mutex);
	{
		SendBuffer.push_back(dg);
	}
	SDL_mutexV(Mutex);

	//Send All Files

	std::vector<char*> fileList=LGL_DirectoryListCreate(dir,true,true);
	for(unsigned int a=0;a<fileList.size();a++)
	{
		char src[1024];
		sprintf(src,"%s/%s",dir,fileList[a]);
		SendFile(src);
		delete fileList[a];
	}

	//Send All Subdirs
	
	std::vector<char*> dirList=LGL_DirectoryListCreate(dir,false,true);
	for(unsigned int a=0;a<dirList.size();a++)
	{
		char src[1024];
		sprintf(src,"%s/%s",dir,dirList[a]);
		if
		(
			LGL_DirectoryExists(src) &&
			strcmp(dirList[a],".")!=0 &&
			strcmp(dirList[a],"..")!=0
		)
		{
			SendDirectory(src);
		}
		delete dirList[a];
	}

	return(true);
}

void
LGL_NetConnection::
RecvFiles
(
	char*	requiredPrefix
)
{
	if(requiredPrefix==NULL)
	{
		sprintf(RecvFileRequiredPrefix,"/");;
	}
	else
	{
		sprintf(RecvFileRequiredPrefix,"%s",requiredPrefix);
	}
}

int
LGL_NetConnection::
SendFileQueuedFiles()
{
	return(FileSendQueuedFiles);
}

long
LGL_NetConnection::
SendFileQueuedBytes()
{
	return(FileSendQueuedBytes);
}

int
LGL_NetConnection::
SendFileCompletedFiles()
{
	return(FileSendCompletedFiles);
}

long
LGL_NetConnection::
SendFileCompletedBytes()
{
	return(FileSendCompletedBytes);
}

float
LGL_NetConnection::
SendFileCompletedPercent()
{
	float ret;

	SDL_mutexP(Mutex);
	{
		float bot=FileSendCompletedBytes+FileSendQueuedBytes;
		if(bot==0)
		{
			ret=0;
		}
		else
		{
			ret=FileSendCompletedBytes/bot;
		}
	
		if(FileSendQueuedBytes==0 && FileSendCompletedBytes!=0)
		{
			ret=1;
		}
	}
	SDL_mutexV(Mutex);

	return(ret);
}

int
LGL_NetConnection::
SendFileTotalFiles()
{
	int ret;

	SDL_mutexP(Mutex);
	{
		ret=FileSendCompletedFiles+FileSendQueuedFiles;
	}
	SDL_mutexV(Mutex);

	return(ret);
}

long
LGL_NetConnection::
SendFileTotalBytes()
{
	long ret;

	SDL_mutexP(Mutex);
	{
		ret=FileSendCompletedBytes+FileSendQueuedBytes;
	}
	SDL_mutexV(Mutex);

	return(ret);
}

void
LGL_NetConnection::
SendFileResetStatistics()
{
	FileSendCompletedFiles=0;
	FileSendCompletedBytes=0;
}

char*
LGL_NetConnection::
SendFileNowName()
{
	return(FileSendNowName);
}

char*
LGL_NetConnection::
SendFileNowStatus()
{
	return(FileSendNowStatus);
}

int
LGL_NetConnection::
RecvFileQueuedFiles()
{
	return(FileRecvQueuedFiles);
}

long
LGL_NetConnection::
RecvFileQueuedBytes()
{
	return(FileRecvQueuedBytes);
}

int
LGL_NetConnection::
RecvFileCompletedFiles()
{
	return(FileRecvCompletedFiles);
}

int
LGL_NetConnection::
RecvFileTotalFiles()
{
	int ret;

	SDL_mutexP(Mutex);
	{
		ret=FileRecvCompletedFiles+FileRecvQueuedFiles;
	}
	SDL_mutexV(Mutex);

	return(ret);
}

long
LGL_NetConnection::
RecvFileTotalBytes()
{
	long ret;

	SDL_mutexP(Mutex);
	{
		ret=FileRecvCompletedBytes+FileRecvQueuedBytes;
	}
	SDL_mutexV(Mutex);

	return(ret);
}

void
LGL_NetConnection::
RecvFileResetStatistics()
{
	FileRecvCompletedFiles=0;
	FileRecvCompletedBytes=0;
	FileRecvNowName[0]='\0';
	sprintf(FileRecvNowStatus,"No File Transfer Activity");
}


long
LGL_NetConnection::
RecvFileCompletedBytes()
{
	return(FileRecvCompletedBytes);
}

float
LGL_NetConnection::
RecvFileCompletedPercent()
{
	float ret;

	SDL_mutexP(Mutex);
	{
		float bot=FileRecvQueuedBytes+FileRecvCompletedBytes;
		if(bot==0)
		{
			ret=0;
		}
		else
		{
			ret=FileRecvCompletedBytes/bot;
		}
	}
	SDL_mutexV(Mutex);

	return(ret);
}

char*
LGL_NetConnection::
RecvFileNowName()
{
	return(FileRecvNowName);
}

char*
LGL_NetConnection::
RecvFileNowStatus()
{
	return(FileRecvNowStatus);
}

int
LGL_NetConnection::
ConnectionStatus()
{
	return(Connection_Status);
}

int
LGL_NetConnection::
HostResolved
(
	bool	blockUntilKnown
)
{
	if(blockUntilKnown)
	{
		while(Host[0]=='\0')
		{
			LGL_DelayMS(10);
		}
	}
	if(Host[0]=='\0') return(0);
	else if(strcmp(Host,IPstr)==0) return(-1);
	else return(1);
}

int
LGL_NetConnection::
IPResolved
(
	bool	blockUntilKnown
)
{
	if(blockUntilKnown)
	{
		while(IP[0]==0)
		{
			LGL_DelayMS(10);
		}
	}
	if(IP[0]==-1) return(-1);
	else if(IP[0]==0) return(0);
	else return(1);
}

char*
LGL_NetConnection::
Hostname
(
	bool	blockUntilResolution
)
{
	if(blockUntilResolution)
	{
		while(Host[0]=='\0')
		{
			LGL_DelayMS(10);
		}
	}
	return(Host);
}

int
LGL_NetConnection::
IPQuad
(
	int	whichQuad,
	bool	blockUntilResolution
)
{
	if(whichQuad<0 || whichQuad>3)
	{
		printf("LGL_NetConnection::IPQuad(%i): Error! Argument must be [0,3]\n",whichQuad);
		LGL_Exit();
	}
	if(blockUntilResolution)
	{
		while(IP[0]==0)
		{
			LGL_DelayMS(10);
		}
	}
	return(IP[whichQuad]);
}

char*
LGL_NetConnection::
IPString
(
	bool	blockUntilResolution
)
{
	if(blockUntilResolution)
	{
		while(strcmp(IPstr,"0.0.0.0")==0)
		{
			LGL_DelayMS(10);
		}
	}

	return(IPstr);
}

bool
LGL_NetAvailable()
{
	return(LGL.NetAvailable);
}

bool
LGL_NetHostToIP
(
	char*	host,
	int&	ip0,	int&	ip1,	int&	ip2,	int&	ip3
)
{
	if(LGL_NetAvailable()==false)
	{
		printf("LGL_NetHostToIP(): Warning! Network unavailable. So don't call this...\n");
		ip0=-1;
		ip1=-1;
		ip2=-1;
		ip3=-1;
		return(false);
	}

	IPaddress ip;
	if(SDLNet_ResolveHost(&ip,host,0)!=-1)
	{
		//Success

		Uint8* ip4=(Uint8*)&ip.host;
		ip0=ip4[0];
		ip1=ip4[1];
		ip2=ip4[2];
		ip3=ip4[3];
		return(true);
	}
	else
	{
		//Failure
		ip0=-1;
		ip1=-1;
		ip2=-1;
		ip3=-1;
		return(false);
	}
}

bool
LGL_NetIPToHost
(
	char*	host,
	int&	ip0,	int&	ip1,	int&	ip2,	int&	ip3
)
{
	if(LGL_NetAvailable()==false)
	{
		printf("LGL_NetIPToHost(): Warning! Network unavailable. So don't call this...\n");
		host[0]='\0';
		return(false);
	}

	IPaddress ip;
	Uint8* ip4=(Uint8*)&ip.host;
	ip4[0]=ip0;
	ip4[1]=ip1;
	ip4[2]=ip2;
	ip4[3]=ip3;
	sprintf(host,"%s",SDLNet_ResolveIP(&ip));
	bool ret=strcmp(host,"(null)")!=0;
	if(ret==false)
	{
		sprintf(host,"%i.%i.%i.%i",ip0,ip1,ip2,ip3);
	}
	return(ret);
}

bool lgl_FileInfoSortPredicate(const LGL_FileInfo* fi1, const LGL_FileInfo* fi2)
{
	return(strcmp(fi1->Path,fi2->Path)<0);
}

LGL_FileInfo::
LGL_FileInfo
(
	const char*	path,
	LGL_FileType	type,
	long		bytes
)
{
	Path=NULL;
	Type=LGL_FILETYPE_UNDEF;
	Bytes=0;
	ResolvedFileInfo=NULL;

	if(path)
	{
		Path = new char[strlen(path)+1];
		strcpy(Path,path);
	}
	else
	{
		Path = new char[1];
		Path[0]='\0';
		Type=LGL_FILETYPE_UNDEF;
		return;
	}

	Type=type;
	if(Type==LGL_FILETYPE_UNDEF)
	{
		struct stat buf;
		int ok=lstat(Path,&buf);
		//int ok=stat(Path,&buf);

		if(ok==-1)
		{
			Type=LGL_FILETYPE_UNDEF;
		}
		else if(S_ISREG(buf.st_mode))
		{
			//Aliases show up as files... grr...
			Type=LGL_FILETYPE_FILE;
		}
		else if(S_ISDIR(buf.st_mode))
		{
			Type=LGL_FILETYPE_DIR;
		}
		else if(S_ISLNK(buf.st_mode))
		{
			Type=LGL_FILETYPE_SYMLINK;
			if(0 && strstr(Path,".symlink"))
			{
				Type=LGL_FILETYPE_DIR;
			}
			else
			{
				struct stat buf2;
				int ok2=stat(Path,&buf2);
				if(ok2==-1)
				{
					Type=LGL_FILETYPE_UNDEF;
				}
				else if(S_ISREG(buf2.st_mode))
				{
					//Aliases show up as files... grr...
					Type=LGL_FILETYPE_FILE;
				}
				else if(S_ISDIR(buf2.st_mode))
				{
					Type=LGL_FILETYPE_DIR;
				}
			}
		}
		else if(LGL_PathIsAlias(Path,true))
		{
			Type=LGL_FILETYPE_ALIAS;
		}
		else
		{
			Type=LGL_FILETYPE_UNDEF;
		}
	}

	Bytes=bytes;
	if(Bytes==-1)
	{
		Bytes=LGL_FileLengthBytes(Path);
	}

	/*
	if
	(
		Type==LGL_FILETYPE_SYMLINK ||
		Type==LGL_FILETYPE_ALIAS
	)
	{
		char resolvedPath[2048];
		if(Type==LGL_FILETYPE_SYMLINK)
		{
			LGL_ResolveSymlink(resolvedPath,2048,Path);
		}
		else if(Type==LGL_FILETYPE_ALIAS)
		{
			LGL_ResolveAlias(resolvedPath,2048,Path);
		}

		printf("Resolved '%s' => '%s'\n",Path,resolvedPath);
		ResolvedFileInfo=new LGL_FileInfo(resolvedPath);
	}
	*/
}

LGL_FileInfo::
~LGL_FileInfo()
{
	if(Path)
	{
		delete Path;
		Path=NULL;
	}
	Type=LGL_FILETYPE_UNDEF;
	if(ResolvedFileInfo)
	{
		delete ResolvedFileInfo;
		ResolvedFileInfo=NULL;
	}
}

const char*
LGL_FileInfo::
GetPathShort()
{
	if(const char* lastSlash = strrchr(Path,'/'))
	{
		return(&(lastSlash[1]));
	}
	else
	{
		return(Path);
	}
}

int
lgl_dirTreeRefreshThreadFunc
(
	void*	obj
)
{
	LGL_DirTree* dirTree = (LGL_DirTree*)obj;

	dirTree->Refresh_INTERNAL();

	return(0);
}

LGL_DirTree::
LGL_DirTree
(
	const
	char*	path
)
{
	WorkerThread=NULL;
	WorkerThreadDone=true;

	Path[0]='\0';
	FilterText[0]='\0';
	if(SetPath(path)==false)
	{
		bool ret=SetPath(".");
		assert(ret);
	}
}

LGL_DirTree::
~LGL_DirTree()
{
	WaitOnWorkerThread();
	ClearLists();
}

bool
LGL_DirTree::
Ready() const
{
	return(WorkerThreadDone);
}

const
char*
LGL_DirTree::
GetPath()	const
{
	return(Path);
}

bool
LGL_DirTree::
SetPath
(
	const
	char*	path
)
{
	WaitOnWorkerThread();

	//Verify our path is a valid string
	if
	(
		path==NULL ||
		path[0]=='\0'
	)
	{
		//Null-like input, Null-like behavior
		Path[0]='\0';
		ClearLists();
		return(true);
	}

	if(strlen(path)>1024)
	{
		return(false);
	}

	char neopath[1024];
	strcpy(neopath,path);

	//Get rid of trailing slashs
	while(neopath[strlen(neopath)-1]=='/')
	{
		neopath[strlen(neopath)-1]='\0';
	}

	//Verify our new path, put in absPath
	char absPath[1024];
	
	if
	(
		neopath[0]=='.' &&
		neopath[1]!='/'
	)
	{
		if
		(
			strlen(neopath)>1 &&
			neopath[1]=='.'
		)
		{
			//cd ..
			
			char* lastSlash=strstr(Path,"/");
			for(;;)
			{
				if(lastSlash!=NULL)
				{
					if(strlen(lastSlash)>1)
					{
						char* nextSlash=strstr(&lastSlash[1],"/");
						if(nextSlash==NULL)
						{
							break;
						}
						else
						{
							lastSlash=nextSlash;
						}
					}
					else
					{
						lastSlash=NULL;
						break;
					}
				}
				else
				{
					break;
				}
			}

			if(lastSlash==NULL)
			{
				//We're a single dir deep. Set the vanilla path.

				absPath[0]='.';
				absPath[1]='\0';
			}
			else
			{
				//Chop off the last subdir to cd ..
				lastSlash[0]='\0';
				strcpy(absPath,Path);
				lastSlash[0]='/';
			}
		}
		else
		{
			//. shall refer to the top directory, from which the program runs.
			strcpy(absPath,".");
		}
	}
	else
	{
		if
		(
			strlen(neopath) + strlen(Path) < 1024
		)
		{
			if(Path[0]=='\0')
			{
				strcpy(absPath,neopath);
			}
			else
			{
				sprintf(absPath,"%s/%s",Path,neopath);
			}
		}
		else
		{
			//Our path is too big. Our /path/ is too /big/! ...OUR PATH IS TOO BIG!!! (I am a baNAna!)
			return(false);
		}
	}

	if(LGL_DirectoryExists(absPath)==false)
	{
		//LGL_Assertf(false,("LGL_DirTree::SetPath(): Error! For path '%s', absPath '%s' isn't a directory!",path,absPath));
		printf("LGL_DirTree::SetPath(): Warning! For path '%s', absPath '%s' isn't a directory!\n",path,absPath);
		return(false);
	}

	//At this point we have a valid path in absPath.

	strcpy(Path,absPath);

	if
	(
		Path[0]=='.' &&
		Path[1]=='/'
	)
	{
		char temp[1024];
		strcpy(temp,&(Path[2]));
		strcpy(Path,temp);
	}

	WorkerThreadDone=false;
	WorkerThread = LGL_ThreadCreate(lgl_dirTreeRefreshThreadFunc,this);

	return(true);
}

void
LGL_DirTree::
Refresh()
{
	char path[2048];
	strcpy(path,GetPath());
	SetPath(path);
}

void
LGL_DirTree::
Refresh_INTERNAL()
{
	ClearLists();

	if
	(
		(
			Path[0]=='.' &&
			Path[1]=='\0'
		)==false
	)
	{
		//Supply ".." as the zeroth directory

		char* dotdot=new char[3];
		dotdot[0]='.';
		dotdot[1]='.';
		dotdot[2]='\0';
		DirList.push_back(dotdot);
	}

	std::vector<LGL_FileInfo*> fileInfoList;
	std::vector<char*> everything=LGL_DirectoryListCreate(Path,false,false,&fileInfoList);

	for(unsigned int a=0;a<fileInfoList.size();a++)
	{
		char check[4096];

		sprintf(check,"%s/%s",Path,fileInfoList[a]->GetPathShort());

		bool isAlias = false;
		if(0 && LGL_KeyDown(LGL_KEY_LSHIFT))
		{
			//isAlias=LGL_PathIsAlias(check);
		}
		else
		{
			if
			(
				fileInfoList[a]->Type==LGL_FILETYPE_FILE &&
				1//strchr(fileInfoList[a]->GetPathShort(),'.')==NULL
			)
			{
				//printf("Checking for alias-dir: %s\n",check);
				isAlias=LGL_PathIsAlias(check,true);
			}
		}

		if(isAlias)
		{
			const int len=4096;
			char aliasPath[len];
			if
			(
				LGL_ResolveAlias
				(
					aliasPath,
					len,
					check
				)
			)
			{
				char checkDotSymlink[4096];
				sprintf(checkDotSymlink,"%s.symlink",check);
				if
				(
					LGL_FileExists(checkDotSymlink)==false &&
					LGL_DirectoryExists(checkDotSymlink)==false
				)
				{
					LGL_FileDelete(checkDotSymlink);
					symlink(aliasPath,checkDotSymlink);
				}
				strcpy(check,checkDotSymlink);
				char* checkDotSymlinkShort = strrchr(checkDotSymlink,'/');
				if(checkDotSymlinkShort)
				{
					checkDotSymlinkShort=&(checkDotSymlinkShort[1]);
					bool alreadyExisted=false;
					for(unsigned int b=0;b<fileInfoList.size();b++)
					{
						if(strcmp(checkDotSymlinkShort,fileInfoList[b]->GetPathShort())==0)
						{
							alreadyExisted=true;
							break;
						}
					}
					if(alreadyExisted==false)
					{
						/*
						char* neo=new char[strlen(checkDotSymlinkShort)+1];
						strcpy(neo,checkDotSymlinkShort);
						everything.push_back(neo);
						*/

						LGL_FileInfo* neo=new LGL_FileInfo(checkDotSymlinkShort,LGL_FILETYPE_DIR);
						fileInfoList.push_back(neo);
					}
				}
			}
		}

		if(fileInfoList[a]->Type==LGL_FILETYPE_SYMLINK)
		{
			printf("SYMLINK!! DOING NOTHING!! %s\n",check);
		}

		if(isAlias)
		{
			//It's an alias
			//delete everything[a];
		}
		else if(fileInfoList[a]->Type==LGL_FILETYPE_DIR)
		{
			//It's a directory
			//DirList.push_back(everything[a]);
			char* neo = new char[strlen(fileInfoList[a]->GetPathShort())+1];
			strcpy(neo,fileInfoList[a]->GetPathShort());
			DirList.push_back(neo);
		}
		else if(fileInfoList[a]->Type==LGL_FILETYPE_FILE)
		{
			//It's a file
			//FileList.push_back(everything[a]);
			char* neo = new char[strlen(fileInfoList[a]->GetPathShort())+1];
			strcpy(neo,fileInfoList[a]->GetPathShort());
			FileList.push_back(neo);
		}
	}

	for(unsigned int a=0;a<everything.size();a++)
	{
		delete everything[a];
		everything[a]=NULL;
	}
	everything.clear();

	for(unsigned int a=0;a<fileInfoList.size();a++)
	{
		delete fileInfoList[a];
		fileInfoList[a]=NULL;
	}
	fileInfoList.clear();

	GenerateFilterLists();

	WorkerThreadDone=true;
}

unsigned int
LGL_DirTree::
GetFileCount()	const
{
	WaitOnWorkerThread();
	return(FileList.size());
}

unsigned int
LGL_DirTree::
GetDirCount()	const
{
	WaitOnWorkerThread();
	return(DirList.size());
}

const
char*
LGL_DirTree::
GetFileName
(
	unsigned int	index
)	const
{
	WaitOnWorkerThread();
	assert(index>=0 && index<FileList.size());

	return(FileList[index]);
}

const
char*
LGL_DirTree::
GetDirName
(
	unsigned int	index
)	const
{
	WaitOnWorkerThread();
	assert(index>=0 && index<DirList.size());

	return(DirList[index]);
}

const
char*
LGL_DirTree::
GetFilterText()	const
{
	return(FilterText);
}

void
LGL_DirTree::
SetFilterText
(
	const
	char*	path
)
{
	if(path==NULL)
	{
		FilterText[0]='\0';
	}
	else
	{
		strcpy(FilterText,path);
	}
	if(Ready())
	{
		GenerateFilterLists();
	}
}

void
LGL_DirTree::
GenerateFilterLists()
{
	for(unsigned int a=0;a<FilteredFileList.size();a++)
	{
		delete FilteredFileList[a];
	}
	FilteredFileList.clear();
	
	for(unsigned int a=0;a<FilteredDirList.size();a++)
	{
		delete FilteredDirList[a];
	}
	FilteredDirList.clear();

	char tempText[1024];
	LGL_Assert(strlen(FilterText) < 1023);
	strcpy(tempText,FilterText);
	std::vector<char*> filterWordList;
	char* ptr=tempText;
	for(;;)
	{
		if(ptr[0]=='\0')
		{
			break;
		}
		else if(ptr[0]==' ')
		{
			ptr=&(ptr[1]);
		}
		else
		{
			char* nextSpace=strchr(ptr,' ');
			bool lastWord=(nextSpace==NULL);
			
			if(lastWord==false ) nextSpace[0]='\0';
			char* neo=new char[strlen(ptr)+1];
			strcpy(neo,ptr);
			filterWordList.push_back(neo);
			if(lastWord==false) nextSpace[0]=' ';
			ptr=&(nextSpace[1]);
			
			if(lastWord)
			{
				break;
			}
		}
	}

	for(unsigned int a=0;a<FileList.size();a++)
	{
		bool addThisName=true;
		
		for(unsigned int b=0;b<filterWordList.size();b++)
		{
#ifdef	LGL_WIN32
			char* fileListTemp=new char[strlen(FileList[a])+1];
			char* filterWordListTemp=new char[strlen(filterWordList[b])+1];
			strcpy(fileListTemp,FileList[a]);
			strcpy(filterWordListTemp,filterWordList[b]);
			unsigned int zMax=strlen(fileListTemp);
			for(unsigned int z=0;z<zMax;z++)
			{
				fileListTemp[z]=tolower(fileListTemp[z]);
			}
			zMax=strlen(filterWordListTemp);
			for(unsigned int z=0;z<zMax;z++)
			{
				filterWordListTemp[z]=tolower(filterWordListTemp[z]);
			}

			if(strstr(fileListTemp,filterWordListTemp)==NULL)
			{
				addThisName=false;
				delete fileListTemp;
				delete filterWordListTemp;
				break;
			}

			delete fileListTemp;
			delete filterWordListTemp;
#else
			if(strcasestr(FileList[a],filterWordList[b])==NULL)
			{
				addThisName=false;
				break;
			}
#endif	//LGL_WIN32
		}
		if(addThisName)
		{
			char* neo=new char[strlen(FileList[a])+1];
			strcpy(neo,FileList[a]);
			FilteredFileList.push_back(neo);
		}
	}
	
	for(unsigned int a=0;a<DirList.size();a++)
	{
		bool addThisName=true;
		for(unsigned int b=0;b<filterWordList.size();b++)
		{
#ifdef	LGL_WIN32
			char* dirListTemp=new char[strlen(DirList[a])+1];
			char* filterWordListTemp=new char[strlen(filterWordList[b])+1];
			strcpy(dirListTemp,DirList[a]);
			strcpy(filterWordListTemp,filterWordList[b]);
			unsigned int zMax=strlen(dirListTemp);
			for(unsigned int z=0;z<zMax;z++)
			{
				dirListTemp[z]=tolower(dirListTemp[z]);
			}
			zMax=strlen(filterWordListTemp);
			for(unsigned int z=0;z<zMax;z++)
			{
				filterWordListTemp[z]=tolower(filterWordListTemp[z]);
			}

			if(strstr(dirListTemp,filterWordListTemp)==NULL)
			{
				addThisName=false;
				delete dirListTemp;
				delete filterWordListTemp;
				break;
			}

			delete dirListTemp;
			delete filterWordListTemp;
#else
			if(strcasestr(DirList[a],filterWordList[b])==NULL)
			{
				addThisName=false;
				break;
			}
#endif	//LGL_WIN32
		}
		if(addThisName)
		{
			char* neo=new char[strlen(DirList[a])+1];
			strcpy(neo,DirList[a]);
			FilteredDirList.push_back(neo);
		}
	}
	
	for(unsigned int a=0;a<filterWordList.size();a++)
	{
		delete filterWordList[a];
	}
	filterWordList.clear();
}

unsigned int
LGL_DirTree::
GetFilteredFileCount()
{
	WaitOnWorkerThread();
	return(FilteredFileList.size());
}

unsigned int
LGL_DirTree::
GetFilteredDirCount()
{
	WaitOnWorkerThread();
	return(FilteredDirList.size());
}

const
char*
LGL_DirTree::
GetFilteredFileName
(
	unsigned int	index
)
{
	WaitOnWorkerThread();
	assert(index>=0 && index<FilteredFileList.size());

	return(FilteredFileList[index]);
}

const
char*
LGL_DirTree::
GetFilteredDirName
(
	unsigned int	index
)
{
	WaitOnWorkerThread();
	assert(index>=0 && index<FilteredDirList.size());

	return(FilteredDirList[index]);
}

void
LGL_DirTree::
WaitOnWorkerThread() const
{
	if(WorkerThread)
	{
		LGL_ThreadWait(WorkerThread);
		SDL_Thread** workerThread = const_cast<SDL_Thread**>(&WorkerThread);
		*workerThread=NULL;
	}
}

void
LGL_DirTree::
ClearLists()
{
	for(unsigned int a=0;a<FileList.size();a++)
	{
		delete FileList[a];
	}
	FileList.clear();
	
	for(unsigned int a=0;a<DirList.size();a++)
	{
		delete DirList[a];
	}
	DirList.clear();
	
	for(unsigned int a=0;a<FilteredFileList.size();a++)
	{
		delete FilteredFileList[a];
	}
	FilteredFileList.clear();
	
	for(unsigned int a=0;a<FilteredDirList.size();a++)
	{
		delete FilteredDirList[a];
	}
	FilteredDirList.clear();
}

bool
lgl_CharStarSortPredicate
(
	const char*	s1,
	const char*	s2
)
{
	if
	(
		s1==NULL ||
		s2==NULL
	)
	{
		return(false);
	}

	return(strcmp(s1,s2)<0);
}

void
lgl_SortDirectoryList
(
	std::vector<char*>*	list
)
{
	if
	(
		list &&
		list->empty()==false
	)
	{
		std::sort
		(
			list->begin(),
			list->end(),
			lgl_CharStarSortPredicate
		);
	}
}

int
lgl_FileToRam_WorkerThread
(
	void*	ptr
)
{
	LGL_FileToRam* obj = (LGL_FileToRam*)ptr;
	obj->Thread_Load();
	return(0);
}

LGL_FileToRam::
LGL_FileToRam
(
	const char*	path
)
{
	if(path)
	{
		strcpy(Path,path);
	}
	else
	{
		Path[0]='\0';
	}
	Data=NULL;
	ByteCount=0;
	Status=0;
	//ThreadWorker=NULL;
	//Thread_Load();
	ThreadWorker = LGL_ThreadCreate(lgl_FileToRam_WorkerThread,this);
	ThreadTerminateSignal=false;
}

LGL_FileToRam::
~LGL_FileToRam()
{
	SetThreadTerminateSignal();
	if(ThreadWorker)
	{
		LGL_ThreadWait(ThreadWorker);
		ThreadWorker=NULL;
	}

	if(Data)
	{
		delete Data;
	}
}

const char*
LGL_FileToRam::
GetPath()
{
	return(Path);
}

int
LGL_FileToRam::
GetStatus()
{
	return(Status);
}

bool
LGL_FileToRam::
GetFailed()
{
	return(GetStatus()==-1);
}

bool
LGL_FileToRam::
GetLoading()
{
	return(GetStatus()==0);
}

bool
LGL_FileToRam::
GetReady()
{
	return(GetStatus()==1);
}

const char*
LGL_FileToRam::
GetData()
{
	if(GetReady())
	{
		return(Data);
	}
	else
	{
		return(NULL);
	}
}

long
LGL_FileToRam::
GetByteCount()
{
	if(GetReady())
	{
		return(ByteCount);
	}
	else
	{
		return(0);
	}
}

bool
LGL_FileToRam::
GetThreadTerminateSignal()
{
	return(ThreadTerminateSignal);
}

void
LGL_FileToRam::
SetThreadTerminateSignal()
{
	ThreadTerminateSignal=true;
}

bool
LGL_FileToRam::
GetReadyForNonblockingDelete()
{
	return(Status!=0);
}

void
LGL_FileToRam::
Thread_Load()
{
	if(GetPath()==NULL)
	{
		//printf("FTR: NULL Path\n");
		Status=-1;
		return;
	}

	if(FILE* fd = fopen(GetPath(),"r"))
	{
		fseek(fd,0,SEEK_END);
		long len=ftell(fd);
		fseek(fd,0,SEEK_SET);

		char* buf=new char[len+1];

		fread(buf,len,1,fd);
		buf[len]='\0';

		fclose(fd);
		
		Data=buf;
		ByteCount=len;

		Status=1;
		return;
	}
	else
	{
		//printf("FTR: Failed to open path '%s' (%s)\n",GetPath(),LGL_FileExists(Path) ? "File exists" : "File doesn't exist");
		Status=-1;
		return;
	}
}

bool
LGL_FileExists
(
	const
	char*	path
)
{
	if
	(
		path==NULL ||
		path[0]=='\0'
	)
	{
		return(false);
	}
#ifndef	LGL_WIN32
	struct stat buf;
	int ret=stat(path,&buf);

	if
	(
		ret==0 &&
		S_ISDIR(buf.st_mode)==false
	)
	{
		return(true);
	}
	else
	{
		return(false);
	}
#else	//LGL_WIN32
	WIN32_FIND_DATA	findData;
	HANDLE		handle;
	handle=FindFirstFile(path, &findData);
	FindClose(handle);
	return(handle!=INVALID_HANDLE_VALUE);
#endif	//LGL_WIN32
}

bool
LGL_DirectoryExists
(
	const
	char*	dir
)
{
#ifdef	LGL_LINUX

	struct stat buf;
	int ret=stat(dir,&buf);

	if
	(
		ret==0 &&
		S_ISDIR(buf.st_mode)==true
	)
	{
		return(true);
	}
	else
	{
		return(false);
	}

#endif	//LGL_LINUX

#ifdef	LGL_WIN32

	WIN32_FIND_DATA	findData;
	HANDLE		handle;
	handle=FindFirstFile(dir, &findData);
	FindClose(handle);
	return
	(
		handle!=INVALID_HANDLE_VALUE &&
		(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	);

#endif	//LGL_WIN32
}

bool
LGL_DirectoryCreate
(
	const
	char*	dir
)
{
#ifdef	LGL_LINUX

	return(mkdir(dir,0777)==0);

#endif	//LGL_LINUX

#ifdef	LGL_WIN32

	char cmd[1024];
	sprintf(cmd,"mkdir \"%s\"",dir);
printf("LGL_DirectoryCreate('%s')\n",dir);
	system(cmd);

	return(LGL_DirectoryExists(dir));

#endif	//LGL_WIN32
}

bool
LGL_DirectoryCreateChain
(
	const
	char*	dir
)
{
	char path[2048];
	LGL_SimplifyPath(path,dir);
	if(path[strlen(path)-1]!='/')
	{
		strcat(path,"/");
	}

	char* next=path;
	for(;;)
	{
		if(char* slash = strchr(next,'/'))
		{
			slash[0]='\0';
			if(LGL_DirectoryExists(path)==false)
			{
				LGL_DirectoryCreate(path);
			}
			slash[0]='/';
			next=&(slash[1]);
		}
		else
		{
			break;
		}
	}
	if(LGL_DirectoryExists(path)==false)
	{
		LGL_DirectoryCreate(path);
	}

	return(LGL_DirectoryExists(path));
}

bool
LGL_FileDelete
(
	const
	char*	file
)
{
#ifdef	LGL_LINUX
	
	return(unlink(file)==0);

#endif	//LGL_LINUX

#ifdef	LGL_WIN32

	char* my_file=new char[strlen(file)+1];
	strcpy(my_file,file);
	for(unsigned int a=0;a<strlen(my_file);a++)
	{
		if(my_file[a]=='/')
		{
			my_file[a]='\\';
		}
	}
	bool ret=unlink(my_file)==0;	//0 on success, -1 on failure.
	delete my_file;
	return(ret);

#endif	//LGL_WIN32
}

bool
LGL_DirectoryDelete
(
	const
	char*	dir
)
{
	if(LGL_DirectoryExists(dir)==false)
	{
		printf("LGL_DirectoryDelete('%s'): Warning! Deleting non-existant Directory...\n",dir);
		return(true);
	}
	
	//Delete all files in target directory
	std::vector<char*> deadFiles=LGL_DirectoryListCreate(dir,true,true);
	for(unsigned int a=0;a<deadFiles.size();a++)
	{
		char target[1024];
		sprintf(target,"%s/%s",dir,deadFiles[a]);
		LGL_FileDelete(target);
		delete deadFiles[a];
	}
	
	//Delete all directories in target directory
	std::vector<char*> deadDirs=LGL_DirectoryListCreate(dir,false,true);
	for(unsigned int a=0;a<deadDirs.size();a++)
	{
		char target[1024];
		sprintf(target,"%s/%s",dir,deadDirs[a]);
		LGL_DirectoryDelete(target);
		delete deadDirs[a];
	}

#ifdef	LGL_LINUX

	return(rmdir(dir)==0);

#endif	//LGL_LINUX

#ifdef	LGL_WIN32
	char cmd[1024];
	sprintf(cmd,"rmdir /s /q %s",dir);
printf("LGL_DirectoryDelete('%s'\n",dir);
	system(cmd);
	printf("LGL_DirectoryDelete('%s'): Warning! LGL_WIN32 implementation always returns true...\n",dir);
	return(true);

#endif	//LGL_WIN32
}



lgl_PathIsAliasCacher::
lgl_PathIsAliasCacher
(
	const char*	path
)
{
	if(path==NULL)
	{
		Path = new char[2048];
		sprintf(Path,"%s/.dvj/cache/aliasCache.txt",LGL_GetHomeDir());
	}
	else
	{
		Path = new char[strlen(path)+1];
		strcpy(Path,path);
	}

	Load();
}

lgl_PathIsAliasCacher::
~lgl_PathIsAliasCacher()
{
	delete Path;
	Path=NULL;
}

void
lgl_PathIsAliasCacher::
Load()
{
	if(FILE* fd=fopen(Path,"r"))
	{
		const int bufSize=2048;
		char buf[bufSize];
		while(feof(fd)==false)
		{
			//Get path
			fgets(buf,bufSize,fd);
			char path[bufSize];
			strcpy(path,buf);
			if(char* newline=strrchr(path,'\n'))
			{
				newline[0]='\0';
			}

			//Get status
			fgets(buf,bufSize,fd);
			int status = atoi(buf);

			//Add to map
			Add(path,status);
		}
	}
}

void
lgl_PathIsAliasCacher::
Save()
{
	if(FILE* fd=fopen(Path,"w"))
	{
		for
		(
			std::map<std::string,int>::iterator it = Map.begin();
			it != Map.end();
			it++
		)
		{
			fprintf(fd,"%s\n%i\n",it->first.c_str(),it->second);
		}
		fclose(fd);
	}
}

void
lgl_PathIsAliasCacher::
Add
(
	const char*	path,
	int		isAlias
)
{
	if
	(
		path==NULL ||
		path[0]=='\0' ||
		path[1]=='\0' ||
		path[2]=='\0'
	)
	{
		return;
	}

	if(isAlias==-1)
	{
		isAlias = LGL_PathIsAlias(path,false) ? 1 : 0;
	}
	std::string pathStr(path);
	Map[pathStr] = (isAlias ? 1 : 0);
}

int
lgl_PathIsAliasCacher::
Check
(
	const char*	path
)
{
	std::string pathStr(path);
	std::map<std::string,int>::iterator it = Map.find(pathStr);
	if(it==Map.end())
	{
		//printf("Check(): %i %s\n",-1,path);
		return(-1);
	}
	else
	{
		//printf("Check(): %i %s\n",it->second,path);
		return(it->second);
	}
}



std::vector<char*>
LGL_DirectoryListCreate
(
	const char*			targetDir,
	bool				justFiles,
	bool				seeHidden,
	std::vector<LGL_FileInfo*>*	fileInfoList
)
{
	if
	(
		fileInfoList &&
		fileInfoList->empty()==false
	)
	{
		printf("LGL_DirectoryListCreate(): Warning! fileInfoList isn't empty!\n");
	}

	std::vector<char*> returnMe;

#ifdef	LGL_LINUX
	if(targetDir==NULL)
	{
		return(returnMe);
	}
	DIR* myDir=opendir(targetDir);
	if(myDir==NULL)
	{
		printf("LGL_DirectoryListCreate(): Error! opendir(%s) failed...\n",targetDir);
		return(returnMe);
	}
	
	int totallength=0;
	for(;;)
	{
		dirent* myEnt=readdir(myDir);
		if(myEnt==NULL)
		{
			break;
		}
		else
		{
			char temp[1024];
			struct stat buf;
			
			sprintf(temp,"%s/%s",targetDir,myEnt->d_name);
			int ret=lstat(temp,&buf);
			//int ret=stat(temp,&buf);

			if(ret==-1)
			{
				printf("LGL_DirectoryListCreate(): Warning! stat(%s) returns NULL or -1.\n",temp);
				continue;
			}
			if
			(
				(
					seeHidden ||
					myEnt->d_name[0]!='.'
				)
				&&
				(
					S_ISREG(buf.st_mode) ||
					S_ISLNK(buf.st_mode) ||
					(
						(!justFiles) &&
						S_ISDIR(buf.st_mode)==true
					)
				)
				&&
				!(
					myEnt->d_name[0]=='.' &&
					myEnt->d_name[1]=='/'
				)
				&&
				!(
					myEnt->d_name[0]=='.' &&
					myEnt->d_name[1]=='.'
				)
				&&
				!(
					myEnt->d_name[0]=='.' &&
					myEnt->d_name[1]=='\0'
				)
			)
			{
				int length=strlen(myEnt->d_name)+1;
				totallength+=length;
				char* temp2=new char[length];
				sprintf(temp2,"%s",myEnt->d_name);
				returnMe.push_back(temp2);

				if(fileInfoList)
				{
					/*
					if(S_ISLNK(buf.st_mode))
					{
						stat(temp,&buf);
					}
					*/

					LGL_FileType type=LGL_FILETYPE_UNDEF;
					if(S_ISREG(buf.st_mode))
					{
						/*
						if(LGL_PathIsAlias(temp))
						{
							printf("Alias showing up as a file... (%s)\n",temp);
						}
						*/
						type=LGL_FILETYPE_FILE;
					}
					else if(S_ISDIR(buf.st_mode))
					{
						/*
						if(LGL_PathIsAlias(temp))
						{
							printf("Alias showing up as a dir... (%s)\n",temp);
						}
						*/
						type=LGL_FILETYPE_DIR;
					}
					else if(S_ISLNK(buf.st_mode))
					{
						/*
						if(LGL_PathIsAlias(temp))
						{
							printf("Alias showing up as a symlink(?!)... (%s)\n",temp);
						}
						*/
						type=LGL_FILETYPE_UNDEF;
						//type=LGL_FILETYPE_SYMLINK;
					}
					else
					{
						type=LGL_FILETYPE_UNDEF;
					}

					LGL_FileInfo* neo = new LGL_FileInfo
					(
						temp,
						type,
						(long)buf.st_size
					);
					fileInfoList->push_back(neo);
				}
			}
		}
	}
	closedir(myDir);

	lgl_SortDirectoryList(&returnMe);
	if
	(
		fileInfoList &&
		fileInfoList->empty()==false
	)
	{
		std::sort
		(
			fileInfoList->begin(),
			fileInfoList->end(),
			lgl_FileInfoSortPredicate
		);
	}

#endif	//LGL_LINUX
#ifdef	LGL_WIN32

	char targetDirStar[1024];
	assert(strlen(targetDir)<1020);
	sprintf(targetDirStar,"%s/*",targetDir);

	WIN32_FIND_DATA	findData;
	HANDLE		handle;
	handle=FindFirstFile(targetDirStar, &findData);
	while(handle!=INVALID_HANDLE_VALUE)
	{
		if
		(
			(
				findData.cFileName[0]!='.' ||
				(
					strlen(findData.cFileName)>=2 &&
					findData.cFileName[1]!='.'
				)
			) &&
			(
				seeHidden ||
				findData.cFileName[0]!='.' ||
				(findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			)
			&&
			(
				(!justFiles) ||
				(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==false
			)

		)
		{
			/*
			printf
			(
				"Found '%s'. Dir? '%c'. Hidden? '%c'\n",
				findData.cFileName,
				(findData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)?'y':'n',
				(findData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)?'y':'n'
			);
			*/

			char* neo=new char[strlen(findData.cFileName)+1];
			strcpy(neo,findData.cFileName);
			returnMe.push_back(neo);
		}
		if(FindNextFile(handle,&findData)==0)
		{
			break;
		}
	}
	FindClose(handle);
	lgl_SortDirectoryList(&returnMe);
#endif	//LGL_WIN32
	return(returnMe);
}

void
LGL_DirectoryListDelete
(
	std::vector<char*>&	list
)
{
	for(unsigned int a=0;a<list.size();a++)
	{
		if(list[a]!=NULL)
		{
			delete list[a];
		}
	}
	list.clear();
}

bool
LGL_FileDirMove
(
	const
	char*	oldLocation,
	const
	char*	newLocation
)
{
	bool result = rename(oldLocation,newLocation)==0;

	if(result==false)
	{
		printf("LGL_FileDirMove(): Warning! Could not move '%s' => '%s'!\n",oldLocation,newLocation);
	}
	
	return(result);
}

double
LGL_FileLengthBytes
(
	const
	char*	file
)
{
	struct stat stbuf;
	if(stat(file, &stbuf) < 0)
	{
		printf("Can't stat() '%s'\n", file ? file : "(NULL)");
		return(-1);
	}
	double ret = stbuf.st_size;
	return(ret);

/*
	int fd=open(file,O_RDONLY);
	if(fd<0)
	{
		printf("LGL_FileLengthBytes('%s'): Warning! Can't open file!\n",file);
		printf("errno: %i\n",errno);
		return(-1);
	}
	long ret = lseek(fd,0,SEEK_END);
	if(ret==-1)
	{
		printf("lseek error for '%s': %i (%i)\n",file,errno,EOVERFLOW);
	}
	close(fd);
	return(-ret);
*/
}

bool
LGL_FirstFileMoreRecentlyModified
(
	const char*	firstFile,
	const char*	secondFile
)
{
	struct stat stbuf1;
	if(stat(firstFile, &stbuf1) < 0)
	{
		printf("Can't stat() %s\n",firstFile);
		return(false);
	}

	struct stat stbuf2;
	if(stat(secondFile, &stbuf2) < 0)
	{
		printf("Can't stat() %s\n",secondFile);
		return(false);
	}

	return(stbuf1.st_mtime > stbuf2.st_mtime);
}

int LGL_CompareTimeSpecs(struct timespec& ts1, struct timespec& ts2)
{
	if(ts1.tv_sec < ts2.tv_sec)
	{
		return(-1);
	}
	else if(ts1.tv_sec > ts2.tv_sec)
	{
		return(1);
	}
	else if(ts1.tv_nsec < ts2.tv_nsec)
	{
		return(-1);
	}
	else if(ts1.tv_nsec > ts2.tv_nsec)
	{
		return(1);
	}
	else
	{
		return(0);
	}
}

//Begin RSA Code

/*
 *	LGL_MD5sum():
 *
 *	Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
 *	rights reserved.
 *
 *	License to copy and use this software is granted provided that it
 *	is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 *	Algorithm" in all material mentioning or referencing this software
 *	or this function.
 *
 *	License is also granted to make and use derivative works provided
 *	that such works are identified as "derived from the RSA Data
 *	Security, Inc. MD5 Message-Digest Algorithm" in all material
 *	mentioning or referencing the derived work.
 *
 *	RSA Data Security, Inc. makes no representations concerning either
 *	the merchantability of this software or the suitability of this
 *	software forany particular purpose. It is provided "as is"
 *	without express or implied warranty of any kind.
 *	These notices must be retained in any copies of any part of this
 *	documentation and/or software.
 */

typedef unsigned int uint;
typedef unsigned char byte;
extern int enc64(char*,byte*,int);

enum
{
	S11=	7,
	S12=	12,
	S13=	17,
	S14=	22,

	S21=	5,
	S22=	9,
	S23=	14,
	S24=	20,

	S31=	4,
	S32=	11,
	S33=	16,
	S34=	23,

	S41=	6,
	S42=	10,
	S43=	15,
	S44=	21
};

typedef struct Table
{
	uint	sin;	/* integer part of 4294967296 times abs(sin(i)) */
	byte	x;	/* index into data block */
	byte	rot;	/* amount to rotate left by */
}Table;

typedef struct MD5state
{
	uint len;
	uint state[4];
}MD5state;

void encode(byte*, uint*, uint);
void decode(uint*, byte*, uint);
MD5state* md5(byte*, uint, byte*, MD5state*, MD5state*, Table*);

void
sum(FILE *fd, char* output)
{
	byte *buf;
	byte digest[16];
	int i, n;
	MD5state *s;
	MD5state *myNil=NULL;

	Table tab[] =
	{
		/* round 1 */
		{ 0xd76aa478, 0, S11},	
		{ 0xe8c7b756, 1, S12},	
		{ 0x242070db, 2, S13},	
		{ 0xc1bdceee, 3, S14},	
		{ 0xf57c0faf, 4, S11},	
		{ 0x4787c62a, 5, S12},	
		{ 0xa8304613, 6, S13},	
		{ 0xfd469501, 7, S14},	
		{ 0x698098d8, 8, S11},	
		{ 0x8b44f7af, 9, S12},	
		{ 0xffff5bb1, 10, S13},	
		{ 0x895cd7be, 11, S14},	
		{ 0x6b901122, 12, S11},	
		{ 0xfd987193, 13, S12},	
		{ 0xa679438e, 14, S13},	
		{ 0x49b40821, 15, S14},

		/* round 2 */
		{ 0xf61e2562, 1, S21},	
		{ 0xc040b340, 6, S22},	
		{ 0x265e5a51, 11, S23},	
		{ 0xe9b6c7aa, 0, S24},	
		{ 0xd62f105d, 5, S21},	
		{  0x2441453, 10, S22},	
		{ 0xd8a1e681, 15, S23},	
		{ 0xe7d3fbc8, 4, S24},	
		{ 0x21e1cde6, 9, S21},	
		{ 0xc33707d6, 14, S22},	
		{ 0xf4d50d87, 3, S23},	
		{ 0x455a14ed, 8, S24},	
		{ 0xa9e3e905, 13, S21},	
		{ 0xfcefa3f8, 2, S22},	
		{ 0x676f02d9, 7, S23},	
		{ 0x8d2a4c8a, 12, S24},

		/* round 3 */
		{ 0xfffa3942, 5, S31},	
		{ 0x8771f681, 8, S32},	
		{ 0x6d9d6122, 11, S33},	
		{ 0xfde5380c, 14, S34},	
		{ 0xa4beea44, 1, S31},	
		{ 0x4bdecfa9, 4, S32},	
		{ 0xf6bb4b60, 7, S33},	
		{ 0xbebfbc70, 10, S34},	
		{ 0x289b7ec6, 13, S31},	
		{ 0xeaa127fa, 0, S32},	
		{ 0xd4ef3085, 3, S33},	
		{  0x4881d05, 6, S34},	
		{ 0xd9d4d039, 9, S31},	
		{ 0xe6db99e5, 12, S32},	
		{ 0x1fa27cf8, 15, S33},	
		{ 0xc4ac5665, 2, S34},	

		/* round 4 */
		{ 0xf4292244, 0, S41},	
		{ 0x432aff97, 7, S42},	
		{ 0xab9423a7, 14, S43},	
		{ 0xfc93a039, 5, S44},	
		{ 0x655b59c3, 12, S41},	
		{ 0x8f0ccc92, 3, S42},	
		{ 0xffeff47d, 10, S43},	
		{ 0x85845dd1, 1, S44},	
		{ 0x6fa87e4f, 8, S41},	
		{ 0xfe2ce6e0, 15, S42},	
		{ 0xa3014314, 6, S43},	
		{ 0x4e0811a1, 13, S44},	
		{ 0xf7537e82, 4, S41},	
		{ 0xbd3af235, 11, S42},	
		{ 0x2ad7d2bb, 2, S43},	
		{ 0xeb86d391, 9, S44},	
	};

	s = myNil;
	n = 0;
	buf = (byte*)calloc(256,64);
	for(;;){
		i = fread(buf+n, 1, 128*64-n, fd);
		if(i <= 0)
			break;
		n += i;
		if(n & 0x3f)
			continue;
		s = md5(buf, n, 0, s, myNil, tab);
		n = 0;
	}
	md5(buf, n, digest, s, myNil, tab);

	sprintf
	(
		output,
		"%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X",
		digest[0],digest[1],digest[2],digest[3],
		digest[4],digest[5],digest[6],digest[7],
		digest[8],digest[9],digest[10],digest[11],
		digest[12],digest[13],digest[14],digest[15]
	);
	for(int a=0;a<32;a++)
	{
		output[a]=tolower(output[a]);
	}
	
	free(buf);
}

/*
 *  I require len to be a multiple of 64 for all but
 *  the last call
 */
MD5state*
md5(byte *p, uint len, byte *digest, MD5state *s, MD5state *myNil, Table* tab)
{
	uint a, b, c, d, tmp;
	uint i, done;
	Table *t;
	byte *end;
	uint x[16];

	if(s == myNil){
		s = (MD5state*)calloc(sizeof(*s),1);
		if(s == myNil)
			return myNil;

		/* seed the state, these constants would look nicer big-endian */
		s->state[0] = 0x67452301;
		s->state[1] = 0xefcdab89;
		s->state[2] = 0x98badcfe;
		s->state[3] = 0x10325476;
	}
	s->len += len;

	i = len & 0x3f;
	if(i || len == 0){
		done = 1;

		/* pad the input, assume there's room */
		if(i < 56)
			i = 56 - i;
		else
			i = 120 - i;
		if(i > 0){
			memset(p + len, 0, i);
			p[len] = 0x80;
		}
		len += i;

		/* append the count */
		x[0] = s->len<<3;
		x[1] = s->len>>29;
		encode(p+len, x, 8);
	} else
		done = 0;

	for(end = p+len; p < end; p += 64){
		a = s->state[0];
		b = s->state[1];
		c = s->state[2];
		d = s->state[3];

		decode(x, p, 64);
	
		for(i = 0; i < 64; i++){
			t = tab + i;
			switch(i>>4){
			case 0:
				a += (b & c) | (~b & d);
				break;
			case 1:
				a += (b & d) | (c & ~d);
				break;
			case 2:
				a += b ^ c ^ d;
				break;
			case 3:
				a += c ^ (b | ~d);
				break;
			}
			a += x[t->x] + t->sin;
			a = (a << t->rot) | (a >> (32 - t->rot));
			a += b;
	
			/* rotate variables */
			tmp = d;
			d = c;
			c = b;
			b = a;
			a = tmp;
		}

		s->state[0] += a;
		s->state[1] += b;
		s->state[2] += c;
		s->state[3] += d;
	}

	/* return result */
	if(done){
		encode(digest, s->state, 16);
		free(s);
		return myNil;
	}
	return s;
}

/*
 *	encodes input (uint) into output (byte). Assumes len is
 *	a multiple of 4.
 */
void
encode(byte *output, uint *input, uint len)
{
	uint x;
	byte *e;

	for(e = output + len; output < e;) {
		x = *input++;
		*output++ = x;
		*output++ = x >> 8;
		*output++ = x >> 16;
		*output++ = x >> 24;
	}
}

/*
 *	decodes input (byte) into output (uint). Assumes len is
 *	a multiple of 4.
 */
void
decode(uint *output, byte *input, uint len)
{
	byte *e;

	for(e = input+len; input < e; input += 4)
		*output++ = input[0] | (input[1] << 8) |
			(input[2] << 16) | (input[3] << 24);
}




typedef unsigned long ulong;
typedef unsigned char uchar;

static uchar t64d[256];
static char t64e[64];

static void
init64(void)
{
	int c, i;

	memset(t64d, 255, 256);
	memset(t64e, '=', 64);
	i = 0;
	for(c = 'A'; c <= 'Z'; c++){
		t64e[i] = c;
		t64d[c] = i++;
	}
	for(c = 'a'; c <= 'z'; c++){
		t64e[i] = c;
		t64d[c] = i++;
	}
	for(c = '0'; c <= '9'; c++){
		t64e[i] = c;
		t64d[c] = i++;
	}
	t64e[i] = '+';
	t64d[(int)'+'] = i++;
	t64e[i] = '/';
	t64d[(int)'/'] = i;
}

int
dec64(uchar *out, char *in, int n)
{
	ulong b24;
	uchar *start = out;
	int i, c;

	if(t64e[0] == 0)
		init64();

	b24 = 0;
	i = 0;
	while(n-- > 0){
		c = t64d[(int)*in++];
		if(c == 255)
			continue;
		switch(i){
		case 0:
			b24 = c<<18;
			break;
		case 1:
			b24 |= c<<12;
			break;
		case 2:
			b24 |= c<<6;
			break;
		case 3:
			b24 |= c;
			*out++ = b24>>16;
			*out++ = b24>>8;
			*out++ = b24;
			i = -1;
			break;
		}
		i++;
	}
	switch(i){
	case 2:
		*out++ = b24>>16;
		break;
	case 3:
		*out++ = b24>>16;
		*out++ = b24>>8;
		break;
	}
	*out = 0;
	return out - start;
}

int
enc64(char *out, uchar *in, int n)
{
	int i;
	ulong b24;
	char *start = out;

	if(t64e[0] == 0)
		init64();
	for(i = n/3; i > 0; i--){
		b24 = (*in++)<<16;
		b24 |= (*in++)<<8;
		b24 |= *in++;
		*out++ = t64e[(b24>>18)];
		*out++ = t64e[(b24>>12)&0x3f];
		*out++ = t64e[(b24>>6)&0x3f];
		*out++ = t64e[(b24)&0x3f];
	}

	switch(n%3){
	case 2:
		b24 = (*in++)<<16;
		b24 |= (*in)<<8;
		*out++ = t64e[(b24>>18)];
		*out++ = t64e[(b24>>12)&0x3f];
		*out++ = t64e[(b24>>6)&0x3f];
		break;
	case 1:
		b24 = (*in)<<16;
		*out++ = t64e[(b24>>18)];
		*out++ = t64e[(b24>>12)&0x3f];
		*out++ = '=';
		break;
	}
	*out++ = '=';
	*out = 0;
	return out - start;
}

//End RSA Code

char*
LGL_MD5sum
(
	const
	char*	file,
	char*	output
)
{
	FILE *fd=fopen(file,"r");
	if(fd==NULL)
	{
		printf("LGL_MD5sum('%s'): Warning! Can't open file!\n",file);
		printf("errno: %i\n",errno);
		return(output);
	}
	
	sum(fd,output);

	fclose(fd);

	return(output);
}

bool
LGL_FileExtension
(
	const
	char*	filename,
	const
	char*	extension
)
{
	assert(filename!=NULL);
	assert(extension!=NULL);

	if
	(
		extension[0]!='\0' &&
		extension[0]!='.'
	)
	{
		if(filename[strlen(filename)-(strlen(extension)+1)]!='.')
		{
			return(false);
		}
	}

	for(unsigned int a=0;a<strlen(extension);a++)
	{
		if
		(
			tolower(extension[a])!=
			tolower(filename[strlen(filename)-(strlen(extension)-a)])
		)
		{
			return(false);
		}
	}

	return(true);
}

bool
LGL_FileExtensionIsAudio
(
	const char* path
)
{
	return
	(
		LGL_FileExtension(path,"mp3") ||
		LGL_FileExtension(path,"ogg") ||
		LGL_FileExtension(path,"flac") ||
		LGL_FileExtension(path,"wav")
	);
}

bool
LGL_FileExtensionIsImage
(
	const char*	path
)
{
	return
	(
		LGL_FileExtension(path,"jpg") ||
		LGL_FileExtension(path,"jpeg") || 
		LGL_FileExtension(path,"png")  ||
		LGL_FileExtension(path,"bmp") ||
		LGL_FileExtension(path,"gif")
	);
}

void
LGL_SimplifyPath
(
	char*	simplePath,
	const
	char*	complexPath
)
{
	strcpy(simplePath,complexPath);
	for(unsigned int a=0;a<strlen(simplePath);a++)
	{
		if(simplePath[a]=='\\')
		{
			simplePath[a]='/';
		}
	}

	bool leadingSlash = (simplePath[0]=='/');

	std::vector<char*> elements;
	unsigned int start=0;
	for(unsigned int a=0;a<strlen(simplePath)+1;a++)
	{
		if
		(
			simplePath[a]=='/' ||
			simplePath[a]=='\0'
		)
		{
			if(a==start)
			{
				start=a+1;
			}
			else
			{
				char backup=simplePath[a];
				simplePath[a]='\0';
				char* element=new char[strlen(&(simplePath[start]))+1];
				strcpy(element,&(simplePath[start]));
				if(strcasecmp(element,"..")==0)
				{
					if(elements.size()>0)
					{
						char* popper=elements[elements.size()-1];
						delete popper;
						elements.pop_back();
					}
					delete element;
				}
				else if(strcasecmp(element,".")==0)
				{
					delete element;
				}
				else
				{
					elements.push_back(element);
				}

				simplePath[a]=backup;
				start=a+1;
			}
		}
	}

	simplePath[0]='\0';

	for(unsigned int a=0;a<elements.size();a++)
	{
		if(simplePath[0]=='\0')
		{
			sprintf(simplePath,"%s",elements[a]);
		}
		else
		{
			sprintf(simplePath,"%s/%s",simplePath,elements[a]);
		}
	}

	for(unsigned int a=0;a<elements.size();a++)
	{
		delete elements[a];
	}
	
	if(leadingSlash)
	{
		char tmp[2048];
		sprintf(tmp,"/%s",simplePath);
		strcpy(simplePath,tmp);
	}
}

const char*
LGL_GetUsername()
{
	if(LGL.Username[0]=='\0')
	{
		struct passwd *userinfo = getpwuid(getuid() );
		char *username = userinfo -> pw_name;
		strcpy(LGL.Username,username);
	}
	return(LGL.Username);
}

const char*
LGL_GetHomeDir()
{
	if(LGL.HomeDir[0]=='\0')
	{
		struct passwd *userinfo = getpwuid(getuid());
		char *homeDir = userinfo -> pw_dir;
		strcpy(LGL.HomeDir,homeDir);
	}
	return(LGL.HomeDir);
}

bool
LGL_PathIsAlias
(
	const char*	path,
	bool		useCache
)
{
#ifdef	LGL_OSX
	int cacherResult = -1;
	useCache=false;
	if(useCache)
	{
		//cacherResult=LGL.PathIsAliasCacher.Check(path);
	}
	if(cacherResult!=-1)
	{
		//if(useCache) printf("PathIsAlias(): Cache hit!! %i %s\n",cacherResult,path);
		return(cacherResult);
	}
	else
	{
		//if(useCache) printf("PathIsAlias(): Cache miss!   %s\n",path);
		bool result=true;

		OSStatus err = noErr;
		FSRef fsRef;
		Boolean fileFlag;
		Boolean folderFlag;

		err = FSPathMakeRef
		(
			(UInt8*)path,
			&fsRef,
			&fileFlag
		);

		if(err != noErr)
		{
			result=false;
		}
		else
		{
			err = FSIsAliasFile
			(
				&fsRef,
				&fileFlag,
				&folderFlag
			);

			if(err != noErr)
			{
				result=false;
			}
			else
			{
				result = 
				(
					err == noErr &&
					fileFlag
				);
			}
		}

		/*
		LGL.PathIsAliasCacher.Add
		(
			path,
			result ? 1 : 0
		);
		*/
		return(result);
	}
#endif	//LGL_OSX

	return(false);
}

bool
LGL_ResolveAlias
(
	char*	outPath,
	int	outPathLength,
	const
	char*	inPath
)
{
	outPath[0]='\0';

#ifdef	LGL_OSX
	OSStatus err = noErr;
	FSRef fsRef;
	Boolean fileFlag;
	Boolean folderFlag;

	err = FSPathMakeRef
	(
		(UInt8*)inPath,
		&fsRef,
		&fileFlag
	);

	if(err != noErr)
	{
		return(false);
	}

	err = FSIsAliasFile
	(
		&fsRef,
		&fileFlag,
		&folderFlag
	);

	if(err != noErr)
	{
		return(false);
	}

	if(err == noErr && fileFlag)
	{
		//We're an alias
		Boolean wasAliased;
		err = FSResolveAliasFileWithMountFlags
		(
			&fsRef,
			true,	//Resolve chains
			&folderFlag,
			&wasAliased,
			kResolveAliasFileNoUI
		);

		if(err == noErr)
		{
			err = FSRefMakePath
			(
				&fsRef,
				(UInt8*)outPath,
				outPathLength
			);

			if(err == noErr)
			{
				return(true);
			}
		}
	}
#endif	//LGL_OSX

	return(false);
}

bool
LGL_PathIsSymlink
(
	const char*	path
)
{
	struct stat buf;
	int ret=lstat(path,&buf);
	if(ret==-1)
	{
		return(false);
	}
	else
	{
		return(S_ISLNK(buf.st_mode));
	}

	/*
	const int outPathLength=2048;
	char outPath[outPathLength];
	int num = readlink(path,outPath,outPathLength);
	return(num>=0);
	*/
}

bool
LGL_ResolveSymlink
(
	char*		outPath,
	int		outPathLength,
	const char*	inPath
)
{
	int num = readlink(inPath,outPath,outPathLength);
	if(num==-1)
	{
		outPath[0]='\0';
		return(false);
	}

	if(num<outPathLength)
	{
		outPath[num]='\0';
	}

	return(true);
}

lgl_WriteFileAsyncWorkItem::
lgl_WriteFileAsyncWorkItem
(
	const char*	path,
	const char*	data,
	int		len
)
{
	Path = new char[strlen(path)+2];
	strcpy(Path,path);

	Data = new char[len];
	memcpy(Data,data,len);

	Len = len;
}

lgl_WriteFileAsyncWorkItem::
~lgl_WriteFileAsyncWorkItem()
{
	delete Path;
	Path=NULL;

	delete Data;
	Data=NULL;

	Len = 0;
}

void
lgl_WriteFileAsyncWorkItem::
Write()
{
	char pathTmp[2048];
	sprintf(pathTmp,"%s.tmp",Path);
	if(FILE* fd = fopen(pathTmp,"w"))
	{
		fwrite(Data,Len,1,fd);
		fclose(fd);
		rename(pathTmp,Path);
	}
}

const char*
lgl_WriteFileAsyncWorkItem::
GetPath()
{
	return(Path);
}

int
lgl_WriteFileAsyncThread
(
	void*	baka
)
{
	for(;;)
	{
		unsigned int workItemSize = 0;
		lgl_WriteFileAsyncWorkItem* wi = NULL;

		{
			if(LGL.WriteFileAsyncWorkItemListNewSize>0)
			{
				std::vector<lgl_WriteFileAsyncWorkItem*> listNew;
				{
					LGL_ScopeLock lock(__FILE__,__LINE__,LGL.WriteFileAsyncWorkItemListNewSemaphore);
					listNew=LGL.WriteFileAsyncWorkItemListNew;
					LGL.WriteFileAsyncWorkItemListNew.clear();
					LGL.WriteFileAsyncWorkItemListSize=listNew.size() + LGL.WriteFileAsyncWorkItemList.size();
					LGL.WriteFileAsyncWorkItemListNewSize=0;
				}

				for(unsigned int a=0;a<listNew.size();a++)
				{
					LGL.WriteFileAsyncWorkItemList.push_back
					(
						listNew[a]
					);
				}
			}

			workItemSize = LGL.WriteFileAsyncWorkItemList.size();
			wi=NULL;
			if(workItemSize>0)
			{
				wi = LGL.WriteFileAsyncWorkItemList[0];
				LGL.WriteFileAsyncWorkItemList.erase(LGL.WriteFileAsyncWorkItemList.begin());
				workItemSize = LGL.WriteFileAsyncWorkItemList.size();
				for(unsigned int a=0;a<workItemSize;a++)
				{
					lgl_WriteFileAsyncWorkItem* wiNow = LGL.WriteFileAsyncWorkItemList[a];
					if(strcasecmp(wi->GetPath(),wiNow->GetPath())==0)
					{
						delete wi;
						wi=NULL;
						break;
					}
				}
			}
		}

		if(wi)
		{
			wi->Write();
			delete wi;
		}
		
		LGL.WriteFileAsyncWorkItemListSize=workItemSize;
		if(LGL.Running==false && workItemSize==0)
		{
			return(0);
		}

		LGL_DelayMS((workItemSize>0) ? 1 : 100);
	}
	return(0);
}

void
LGL_WriteFileAsync
(
	const char*	path,
	const char*	data,
	int		len
)
{
	if(LGL.WriteFileAsyncThread == NULL)
	{
		LGL.WriteFileAsyncThread = LGL_ThreadCreate(lgl_WriteFileAsyncThread);
	}

	lgl_WriteFileAsyncWorkItem* wi = new lgl_WriteFileAsyncWorkItem
	(
		path,
		data,
		len
	);

	{
		LGL_ScopeLock lock(__FILE__,__LINE__,LGL.WriteFileAsyncWorkItemListNewSemaphore);
		LGL.WriteFileAsyncWorkItemListNew.push_back(wi);
		LGL.WriteFileAsyncWorkItemListNewSize++;
	}
}

unsigned int
LGL_WriteFileAsyncQueueCount()
{
	return(LGL.WriteFileAsyncWorkItemListSize);
}

//Memory

int64_t
LGL_RamTotalB()
{
#ifdef	LGL_OSX
	vm_size_t page_size;
	mach_port_t mach_port;
	mach_msg_type_number_t count;
	vm_statistics_data_t vm_stats;

	mach_port = mach_host_self();
	count = sizeof(vm_stats) / sizeof(natural_t);
	if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
	    KERN_SUCCESS == host_statistics(mach_port, HOST_VM_INFO, 
					    (host_info_t)&vm_stats, &count))
	{
		int64_t myTotalMemory =
		(
			(
				(int64_t)vm_stats.free_count +
				(int64_t)vm_stats.inactive_count +
				(int64_t)vm_stats.active_count +
				(int64_t)vm_stats.wire_count
			) * (int64_t)page_size
		);
		return(myTotalMemory);

		/*
		used_memory = ((int64_t)vm_stats.active_count + 
			   (int64_t)vm_stats.inactive_count + 
			   (int64_t)vm_stats.wire_count) *  (int64_t)page_size;
*/
	}
#endif	//LGL_OSX
	return(1024*1024*1024);
}

int
LGL_RamTotalMB()
{
	return((int)(LGL_RamTotalB()/((int64_t)(1024*1024))));
}

int64_t
LGL_RamFreeB()
{
#ifdef	LGL_OSX
	vm_size_t page_size;
	mach_port_t mach_port;
	mach_msg_type_number_t count;
	vm_statistics_data_t vm_stats;

	mach_port = mach_host_self();
	count = sizeof(vm_stats) / sizeof(natural_t);
	if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
	    KERN_SUCCESS == host_statistics(mach_port, HOST_VM_INFO, 
					    (host_info_t)&vm_stats, &count))
	{
		int64_t myFreeMemory =
		(
			(
				(int64_t)vm_stats.free_count + 
				(int64_t)vm_stats.inactive_count
			) * (int64_t)page_size
		);
		return(myFreeMemory);

		/*
		used_memory = ((int64_t)vm_stats.active_count + 
			   (int64_t)vm_stats.inactive_count + 
			   (int64_t)vm_stats.wire_count) *  (int64_t)page_size;
*/
	}
#endif	//LGL_OSX
	return(1024*1024*1024);
}

int
LGL_RamFreeMB()
{
	return((int)(LGL_RamFreeB()/((int64_t)(1024*1024))));
}

int64_t
LGL_MemoryUsedByThisB()
{
#ifdef	LGL_OSX
	struct task_basic_info t_info;
	mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

	if (KERN_SUCCESS != task_info(mach_task_self(),
				      TASK_BASIC_INFO, (task_info_t)&t_info, 
				      &t_info_count))
	{
	    return(1024*1024*1024);
	}

	// resident size is in t_info.resident_size;
	return(t_info.virtual_size);
#endif
	return(1024*1024*1024);
}

int
LGL_MemoryUsedByThisMB()
{
	return((int)(LGL_MemoryUsedByThisB()/((int64_t)(1024*1024))));
}

//Misc

float
LGL_MemoryFreePercent()
{
	return(LGL_RamFreeMB()/(float)LGL_RamTotalMB());
}

bool
LGL_BatteryChargeDraining()
{
#ifndef	LGL_OSX
	if(FILE* fd=fopen("/proc/acpi/battery/BAT0/state","r"))
	{
		char buf[2048];
		while(feof(fd)==false)
		{
			fgets(buf,2048,fd);
			if(strstr(buf,"charging state:"))
			{
				fclose(fd);
				return(strstr(buf,"discharging")!=NULL);
			}
		}
		fclose(fd);
	}
	
	return(false);
#else	//LGL_LINUX
	return(false);
#endif	//LGL_LINUX
}

float
LGL_BatteryChargePercent()
{
#ifndef	LGL_OSX
	float capacity=-1.0f;
	float charge=-1.0f;

	if(FILE* fd=fopen("/proc/acpi/battery/BAT0/state","r"))
	{
		char buf[2048];
		while(feof(fd)==false)
		{
			fgets(buf,2048,fd);
			if(strstr(buf,"remaining capacity:"))
			{
				charge=atof(&(strchr(buf,':')[1]));
			}
		}
		fclose(fd);
	}
	if(FILE* fd=fopen("/proc/acpi/battery/BAT0/info","r"))
	{
		char buf[2048];
		while(feof(fd)==false)
		{
			fgets(buf,2048,fd);
			if(strstr(buf,"design capacity:"))
			{
				capacity=atof(&(strchr(buf,':')[1]));
			}
		}
		fclose(fd);
	}

	if
	(
		capacity!=-1.0f &&
		charge!=-1.0f
	)
	{
		return(charge/capacity);
	}
	else
	{
		return(1.0f);
	}
#else	//LGL_LINUX
	return(1.0f);
#endif	//LGL_LINUX
}

float
LGL_FilesystemFreeSpaceMB()
{
#ifdef	LGL_LINUX
	struct statvfs s;
	if(statvfs(".",&s)>=0)
	{
		float blockSize = s.f_frsize;
		float blockAvail = s.f_bavail;
		return(blockAvail*blockSize/(1024.0f*1024.0f));
	}
	else
	{
		return(9999.0f);
	}
#endif	//LGL_LINUX
	return(9999.0f);
}

void
LGL_DrawFPSGraph
(
	float	left,	float	right,
	float	bottom,	float	top,
	float	brightness,
	float	alpha
)
{
	const float width = right-left;
	const float height = top-bottom;
	brightness = LGL_Clamp(0.0f,brightness,1.0f);
	alpha = LGL_Clamp(0.0f,alpha,1.0f);
	LGL_DrawRectToScreen
	(
		left,right,
		bottom,top,
		0,0,.25*brightness,.5*alpha
	);

	float r=0;
	float g=0;
	float b=0;
	float yellowPct=2.0f/3.0f;
	float redPct=1.0f/3.0f;
	for(int a=0;a<60;a++)
	{
		float h=1.0f-LGL_Min(1.0f,LGL.FrameTimeGraph[a]/(3.0f/60.0f));
		if(h<redPct-0.01f)
		{
			r=.5*brightness;
			g=0;
			b=0;
		}
		else if(h<yellowPct-0.01f)
		{
			r=.5*brightness;
			g=.5*brightness;
			b=0;
		}
		else
		{
			r=0;
			g=.5*brightness;
			b=0;
		}

		LGL_DrawRectToScreen
		(
			left+width*(a+0)/60.0,
			left+width*(a+1)/60.0,
			bottom,
			bottom+height*h,
			r,g,b,alpha
		);
	}
	LGL_DrawLineToScreen
	(
		left,
		bottom+height*redPct,
		right,
		bottom+height*redPct,
		brightness,0.0f,0.0f,.5f*alpha
	);
	LGL_DrawLineToScreen
	(
		left,
		bottom+height*yellowPct,
		right,
		bottom+height*yellowPct,
		brightness,brightness,0.0f,.5f*alpha
	);

	float textHeight=.10f*height+.10f*width;

	/*
	LGL_GetFont().DrawString
	(
		.5f*(left+right),
		bottom+height*66.7f/100.0f + textHeight*0.25f,
		textHeight,
		.75f*brightness,.75f*brightness,.75f*brightness,.75f*alpha ,
		true,.5f*alpha,
		"%.0f%%",
		LGL.FrameTimeMax/(1.0f/60.0f)*100.0f
	);
	LGL_GetFont().DrawString
	(
		.5f*(left+right),
		bottom+height*33.3f/100.0f + textHeight*0.25f,
		textHeight,
		.75f*brightness,.75f*brightness,.75f*brightness,.75f*alpha,
		true,.5f*alpha,
		"%.0f%%",
		LGL.FrameTimeAve/(1.0f/60.0f)*100.0f
	);
	LGL_GetFont().DrawString
	(
		.5f*(left+right),
		bottom+height*0.0f/100.0f + textHeight*0.25f,
		textHeight,
		.75f*brightness,.75f*brightness,.75f*brightness,.75f*alpha,
		true,.5f*alpha,
		"%.0f%%",
		LGL.FrameTimeMin/(1.0f/60.0f)*100.0f
	);
	*/

	LGL_GetFont().DrawString
	(
		right+0.05f*width,
		bottom+height*66.7f/100.0f + textHeight*0.25f,
		textHeight,
		.75f*brightness,.75f*brightness,.75f*brightness,.75f*alpha,
		false,.5f*alpha,
		"%i",
		LGL.FrameTimeGoodTotal
	);
	LGL_GetFont().DrawString
	(
		right+0.05f*width,
		bottom+height*33.3f/100.0f + textHeight*0.25f,
		textHeight,
		.75f*brightness,.75f*brightness,.75f*brightness,.75f*alpha,
		false,.5f*alpha,
		"%i",
		LGL.FrameTimeMediumTotal
	);
	LGL_GetFont().DrawString
	(
		right+0.05f*width,
		bottom+height*0.0f/100.0f + textHeight*0.25f,
		textHeight,
		.75f*brightness,.75f*brightness,.75f*brightness,.75f*alpha ,
		false,.5f*alpha,
		"%i",
		LGL.FrameTimeBadTotal
	);

	LGL_GetFont().DrawString
	(
		left+0.375f*width,
		bottom-height*33.3f/100.0f + textHeight*0.25f,
		textHeight,
		.75f*brightness,.75f*brightness,.75f*brightness,.75f*alpha ,
		false,.5f*alpha,
		"%i",
		LGL.FrameTimeBadTotal +
		LGL.FrameTimeMediumTotal +
		LGL.FrameTimeGoodTotal
	);

	LGL_DrawLineToScreen
	(
		left,bottom,
		left,top,
		brightness*.4,brightness*.2,brightness,.5*alpha
	);
	LGL_DrawLineToScreen
	(
		left,top,
		right,top,
		brightness*.4,brightness*.2,brightness,.5*alpha
	);
	LGL_DrawLineToScreen
	(
		right,top,
		right,bottom,
		brightness*.4,brightness*.2,brightness,.5*alpha
	);
	LGL_DrawLineToScreen
	(
		right,bottom,
		left,bottom,
		brightness*.4,brightness*.2,brightness,.5*alpha
	);
}

void
LGL_ResetFPSGraph()
{
	LGL.FPS=0;
	LGL.FPSMax=60;
	LGL.FPSTimer.Reset();
	for(int a=0;a<60;a++)
	{
		LGL.FPSGraph[a]=0.0f;
		LGL.FrameTimeGraph[a]=0.0f;
	}
	LGL.FrameTimeMin=0.0f;
	LGL.FrameTimeAve=0.0f;
	LGL.FrameTimeMax=0.0f;
	LGL.FrameTimeGoodCount=0;
	LGL.FrameTimeMediumCount=0;
	LGL.FrameTimeBadCount=0;
	LGL.FrameTimeGoodTotal=0;
	LGL.FrameTimeMediumTotal=0;
	LGL.FrameTimeBadTotal=0;
	LGL.FPSGraphTimer.Reset();
}

void
LGL_ScreenShot
(
	const char*	bmpFile
)
{
	SDL_Surface* temp=NULL;
	unsigned char* pixels;

	temp=SDL_CreateRGBSurface
	(
		SDL_SWSURFACE,
		LGL.WindowResolutionX[LGL.DisplayNow], LGL.WindowResolutionY[LGL.DisplayNow], 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		0x000000FF, 0x0000FF00, 0x00FF0000, 0
#else
		0x00FF0000, 0x0000FF00, 0x000000FF, 0
#endif
	);
	if(temp==NULL)
	{
		printf("LGL_ScreenShot(): Error! Unable to SDL_CreateGFBSurface().\n");
		LGL_Exit();
	}

	pixels=(unsigned char*)malloc(3*LGL.WindowResolutionX[LGL.DisplayNow]*LGL.WindowResolutionY[LGL.DisplayNow]);
	if(pixels==NULL)
	{
		printf("LGL_ScreenShot(): Error! Unable to malloc() pixel buffer.\n");
		LGL_Exit();
	}

	glReadBuffer(GL_FRONT_LEFT);
	glReadPixels
	(
		0, 0,
		LGL.WindowResolutionX[LGL.DisplayNow], LGL.WindowResolutionY[LGL.DisplayNow],
		GL_RGB, GL_UNSIGNED_BYTE,
		pixels
	);

	for(int i=0;i<LGL.WindowResolutionY[LGL.DisplayNow];i++)
	{
		memcpy
		(
			((char*)temp->pixels)+temp->pitch*i,
			pixels+3*LGL.WindowResolutionX[LGL.DisplayNow]*(LGL.WindowResolutionY[LGL.DisplayNow]-i-1),
			LGL.WindowResolutionX[LGL.DisplayNow]*3
		);
	}

	free(pixels);
	SDL_SaveBMP(temp, bmpFile);
	SDL_FreeSurface(temp);
}

void
LGL_RecordMovieStart
(
	const char* inPathPrefix,
	int fps
)
{
	printf("LGL_RecordMovieStart(): Warning! Mencoder and imagemagick must be installed for this to work...\n");

	char command[2048];
	sprintf(command,"rm -f %s*.png",inPathPrefix);
	system(command);

	LGL.RecordMovie=true;
	LGL.RecordMovieFPS=fps;
	LGL.RecordMovieSecondsSinceExecutionInitial=LGL_SecondsSinceExecution();
	sprintf(LGL.RecordMovieFileNamePrefix,"%s",inPathPrefix);
}

void
LGL_RecordMovieStop()
{
	LGL.RecordMovie=false;
}

void
lgl_DrawLogPlayback
(
	char* inFile
)
{
	if(inFile==NULL)
	{
		printf("lgl_DrawLogPlayback(NULL): Error! Can't fopen NULL filename!\n");
		LGL_Exit();
	}
		
	FILE* f=fopen(inFile,"r");
	if(f==NULL)
	{
		printf("lgl_DrawLogPlayback(%s): Error! Couldn't fopen file!\n",inFile);
		LGL_Exit();
	}
	fclose(f);
}

void
LGL_DrawLogStart
(
	const char*	outFile
)
{
	if(LGL.DrawLogFD!=0)
	{
		printf
		(
			"LGL_DrawLogStart('%s'): Error! Already logging (%s)\n",
			outFile,
			LGL.DrawLogFileName
		);
		assert(LGL.DrawLogFD==0);
	}

	if(outFile!=NULL)
	{
		sprintf(LGL.DrawLogFileName,"%s",outFile);
		const char* diskWriterPath = "./diskWriter";
#ifdef	LGL_LINUX
		diskWriterPath = "./diskWriter.lin";
#endif	//LGL_LINUX
#ifdef	LGL_OSX
		diskWriterPath = "./diskWriter.osx";
#endif	//LGL_OSX
		char command[1024];
		sprintf(command,"%s \"%s\" --gzip",diskWriterPath,outFile);
		if(LGL_FileExists(diskWriterPath)==false)
		{
			printf("LGL_DrawLogStart('%s'): Error! Couldn't open diskWriter '%s'...\n",outFile,diskWriterPath);
			LGL.DrawLogFD=0;
			return;
		}
		LGL.DrawLogFD=popen
		(
			command,
			"w"
		);
		if(LGL.DrawLogFD<=0)
		{
			printf("LGL_DrawLogStart('%s'): Error! Couldn't open file...\n",outFile);
			LGL.DrawLogFD=0;
			return;
		}

		char intro[1024];
		sprintf(intro,"LGL_DrawLogStart|%f\n",(LGL.AudioEncoder==NULL)?0.0f:(LGL.RecordSamplesWritten/(double)LGL.AudioSpec->freq));
		fwrite(intro,strlen(intro),1,LGL.DrawLogFD);
		
		sprintf(intro,"LGL_MouseCoords|%f|%f\n",LGL_MouseX(),LGL_MouseY());
		fwrite(intro,strlen(intro),1,LGL.DrawLogFD);
	}
	else
	{
		printf("LGL_DrawLogStart(NULL): Error! Must specify a non-NULL filename...\n");
		assert(outFile);
	}
}

void
LGL_DrawLogWrite
(
	const
	char*	str,
	...
)
{
	if
	(
		LGL.DrawLogFD==NULL &&
		str[0]!='!'
	)
	{
		return;
	}

	//Process the formatted part of the string
	char tmpstr[1024];
	va_list args;
	va_start(args,str);
	vsprintf(tmpstr,str,args);
	va_end(args);

	char* target = new char[strlen(tmpstr)+1];
	strcpy(target,tmpstr);

	LGL.DrawLog.push_back(target);
}

void
LGL_DrawLogWriteForceNow
(
	const
	char*	str,
	...
)
{
	if(LGL.DrawLogFD==NULL)
	{
		return;
	}

	//Process the formatted part of the string
	char tmpstr[1024];
	va_list args;
	va_start(args,str);
	vsprintf(tmpstr,str,args);
	va_end(args);

	assert(LGL.DrawLogFD);
	fwrite(tmpstr,strlen(tmpstr),1,LGL.DrawLogFD);
}

void
LGL_DrawLogPause
(
	bool	pause
)
{
	LGL.DrawLogPause=pause;
}

void
LGL_DrawLogStop()
{
	if(LGL.DrawLogFD!=0)
	{
		fclose(LGL.DrawLogFD);
		LGL.DrawLogFD=0;
	}
	LGL.DrawLogFileName[0]='\0';
	for(unsigned int a=0;a<LGL.DrawLog.size();a++)
	{
		delete LGL.DrawLog[a];
	}
	LGL.DrawLog.clear();
}

void
LGL_FrameBufferTextureGlitchFixToggle()
{
	LGL.FrameBufferTextureGlitchFix=!LGL.FrameBufferTextureGlitchFix;
}

bool
lgl_assert
(
	bool	test,
	const
	char*	testStr,
	const
	char*	file,
	int	line
)
{
	//Note: The actual call to assert() is done in the LGL_Assert() macro.
	if(test==false)
	{
		printf("%s:%i: LGL_Assert(%s): Failed!\n",file,line,testStr);
	}

	return(test);
}

const
char*
LGL_GetErrorStringGL()
{
	int err=glGetError();
	char* estr=LGL.ErrorStringGL;
	strcpy(estr,"Error not found");
	if(err==GL_NO_ERROR) strcpy(estr,"GL_NO_ERROR");
	else if(err==GL_INVALID_ENUM) strcpy(estr,"GL_INVALID_ENUM");
	else if(err==GL_INVALID_VALUE) strcpy(estr,"GL_INVALID_VALUE");
	else if(err==GL_INVALID_OPERATION) strcpy(estr,"GL_INVALID_OPERATION");
	else if(err==GL_STACK_OVERFLOW) strcpy(estr,"GL_STACK_OVERFLOW");
	else if(err==GL_STACK_UNDERFLOW) strcpy(estr,"GL_STACK_UNDERFLOW");
	else if(err==GL_OUT_OF_MEMORY) strcpy(estr,"GL_OUT_OF_MEMORY");
	else if(err==GL_TABLE_TOO_LARGE) strcpy(estr,"GL_TABLE_TOO_LARGE");

	return(estr);
}

const
char*
LGL_CompileTime()
{
	return(__TIME__);
}

const
char*
LGL_CompileDate()
{
	return(__DATE__);
}

float
LGL_RandFloat()
{
	return((rand()%32767)/32767.0);
}

float
LGL_RandFloat
(
	float	highest
)
{
	return(highest*LGL_RandFloat());
}

float
LGL_RandFloat
(
	float	lowest,
	float	highest
)
{
	return(lowest+LGL_RandFloat()*(highest-lowest));
}

float
LGL_RandGaussian()
{
	//FIXME: This needs to be re-examined
	return
	(
		sqrtf(-2*log(LGL_RandFloat()))*
		cosf(2*LGL_PI*LGL_RandFloat())
	);
}

int
LGL_RandInt
(
	int	lowest,
	int	highest
)
{
	int r=highest-lowest;
	return((rand()%(r+1))+lowest);
}

bool
LGL_RandBool
(
	float	trueProbability
)
{
	return(LGL_RandFloat()<trueProbability);
}

float
LGL_Interpolate
(
	float	in0,	float	in1,
	float	interpolationFactor
)
{
	float f=LGL_Clamp(0,interpolationFactor,1);
	return
	(
		(1.0-f)*in0+
		(0.0+f)*in1
	);
}

float
LGL_InterpolateModulus
(
	float	in0,	float	in1,
	float	interpolationFactor,
	float	modulus
)
{
	in0=in0-modulus*floor(in0/modulus);
	in1=in1-modulus*floor(in1/modulus);

	float i=LGL_Clamp(0,interpolationFactor,1);

	float lesser=in0;
	float greater=in1;

	if(in0>in1)
	{
		lesser=in1;
		greater=in0;
		i=1.0-i;
	}

	if(lesser+.5*modulus<greater)
	{
		float gPrime=greater-lesser;
		float mPrime=modulus-gPrime;
		float ret=lesser-i*mPrime;
		while(ret<0)
		{
			ret+=modulus;
		}
		while(ret>modulus)
		{
			ret-=modulus;
		}
		return(ret);
	}
	else
	{
		float d=greater-lesser;
		return(lesser+i*d);
	}
}

float
LGL_Clamp
(
	float lowerbound,
	float target,
	float upperbound
)
{
	if(lowerbound>upperbound)
	{
		printf("LGL_Clamp(): lower > upper: %.2f > %.2f\n",lowerbound,upperbound);
	}
	if(target>upperbound) target=upperbound;
	if(target<lowerbound) target=lowerbound;
	return(target);
}

float
LGL_ClampModulus
(
	float lowerbound,
	float target,
	float upperbound,
	float modulus
)
{
	lowerbound=lowerbound-modulus*floor(lowerbound/modulus);
	upperbound=upperbound-modulus*floor(upperbound/modulus);

	if(lowerbound>upperbound)
	{
		if(fabs(lowerbound-target)<fabs(upperbound-target))
		{
			if(target<lowerbound) target=lowerbound;
		}
		else
		{
			if(target>upperbound) target=upperbound;
		}
	}
	else
	{
		if(target>upperbound) target=upperbound;
		if(target<lowerbound) target=lowerbound;
	}
	return(target);
}

float
LGL_DifferenceModulus
(
	float	in0,	float	in1,
	float	modulus
)
{
	in0=in0-modulus*floor(in0/modulus);
	in1=in1-modulus*floor(in1/modulus);

	float try1=fabs(in0-in1);
	float try2=fabs((in0+modulus)-in1);
	return(LGL_Min(try1,try2));
}

float
LGL_Round
(
	float	in
)
{
	if(in-floor(in)>=.5)
	{
		return(ceil(in));
	}
	else
	{
		return(floor(in));
	}
}
float
LGL_Round
(
	float	in,
	int	decimals
)
{
	float temp=in/pow(10,decimals);
	if(temp-floor(in)>=.5)
	{
		return(ceil(temp)*pow(10,decimals));
	}
	else
	{
		return(floor(temp)*pow(10,decimals));
	}
}

double
LGL_Min
(
	double	in1,
	double	in2
)
{
	if(in1<in2)
	{
		return(in1);
	}
	else
	{
		return(in2);
	}
}
double
LGL_Max
(
	double	in1,
	double	in2
)
{
	if(in1>in2)
	{
		return(in1);
	}
	else
	{
		return(in2);
	}
}

float
LGL_Sign
(
	float in
)
{
	float ret=0;
	if(in>0) ret=1;
	else if(in<0) ret=-1;
	return(ret);
}

LGL_Vector::
LGL_Vector
(
	float	x,
	float	y,
	float	z
)
{
	X=x;
	Y=y;
	Z=z;
}

LGL_Vector::
~LGL_Vector()
{
	//
}

float
LGL_Vector::
GetX()	const
{
	return(X);
}

float
LGL_Vector::
GetY()	const
{
	return(Y);
}

float
LGL_Vector::
GetZ()	const
{
	return(Z);
}

float
LGL_Vector::
GetAngleXY()	const
{
	const float &x=X;
	const float &y=Y;

	if(x==0 || y==0)
	{
		if(x> 0 && y==0) return(0);
		if(x==0 && y> 0) return(float(LGL_PI/2.0f));
		if(x< 0 && y==0) return((float)LGL_PI);
		if(x==0 && y< 0) return(float(3.0f*LGL_PI/2.0f));
	}

	if(x>0  && y> 0)	//Quadrent 1
	{
		return(atanf(y/x));
	}
	if(x<0  && y> 0)	//Quadrent 2
	{
		return(atanf((y/x))+LGL_PI);
	}
	if(x<0  && y< 0)	//Quadrent 3
	{
		return(atanf((y/x))+LGL_PI);
	}
	if(x>0  && y< 0)	//Quadrent 4
	{
		return(atanf((y/x))+2*LGL_PI);
	}
	return(0);
}

float
LGL_Vector::
GetLength()	const
{
	return
	(
		sqrtf
		(
			X*X+
			Y*Y+
			Z*Z
		)
	);
}

float
LGL_Vector::
GetLengthSquared()	const
{
	return
	(
		X*X+
		Y*Y+
		Z*Z
	);
}

float
LGL_Vector::
GetLengthXY()	const
{
	return
	(
		sqrtf
		(
			X*X+
			Y*Y
		)
	);
}

float
LGL_Vector::
GetLengthXYSquared()	const
{
	return
	(
		X*X+
		Y*Y
	);
}

void
LGL_Vector::
SetX
(
	const
	float&	valueX
)
{
	X=valueX;
}

void
LGL_Vector::
SetY
(
	const
	float&	valueY
)
{
	Y=valueY;
}

void
LGL_Vector::
SetZ
(
	const
	float&	valueZ
)
{
	Z=valueZ;
}

void
LGL_Vector::
SetXY
(
	const
	float&	valueX,
	const
	float&	valueY
)
{
	X=valueX;
	Y=valueY;
}

LGL_Vector&
LGL_Vector::
SetXYZ
(
	const
	float&	valueX,
	const
	float&	valueY,
	const
	float&	valueZ
)
{
	X=valueX;
	Y=valueY;
	Z=valueZ;

	return(*this);
}

void
LGL_Vector::
SetAngleXY
(
	const
	float&	angle
)
{
	float oldLengthXY=GetLengthXY();

	X=cosf(angle)*oldLengthXY;
	Y=sinf(angle)*oldLengthXY;
}

void
LGL_Vector::
SetLength
(
	const
	float&	length
)
{
	float oldLength=GetLength();

	X*=length/oldLength;
	Y*=length/oldLength;
	Z*=length/oldLength;
}

float
LGL_Vector::
GetDistance
(
	const
	float&	x,
	const
	float&	y,
	const
	float&	z
)
{
	return
	(
		sqrtf
		(
			(X-x)*(X-x)+
			(Y-y)*(Y-y)+
			(Z-z)*(Z-z)
		)
	);
}

float
LGL_Vector::
GetDistanceXY
(
	const
	float&	x,
	const
	float&	y
)
{
	return
	(
		sqrtf
		(
			(X-x)*(X-x)+
			(Y-y)*(Y-y)
		)
	);
}

float
LGL_Vector::
GetDistanceSquared
(
	const
	float&	x,
	const
	float&	y,
	const
	float&	z
)
{
	return
	(
		(X-x)*(X-x)+
		(Y-y)*(Y-y)+
		(Z-z)*(Z-z)
	);
}

float
LGL_Vector::
GetDistanceSquaredXY
(
	const
	float&	x,
	const
	float&	y
)
{
	return
	(
		(X-x)*(X-x)+
		(Y-y)*(Y-y)
	);
}

LGL_Vector
LGL_Vector::
operator+
(
	const
	LGL_Vector&	vector
)	const
{
	return
	(
		LGL_Vector
		(
			X+vector.GetX(),
			Y+vector.GetY(),
			Z+vector.GetZ()
		)
	);
}

LGL_Vector
LGL_Vector::
operator*
(
	const
	float&	scalar
)	const
{
	return
	(
		LGL_Vector
		(
			X*scalar,
			Y*scalar,
			Z*scalar
		)
	);
}

LGL_Color::
LGL_Color
(
	const
	float&	r,
	const
	float&	g,
	const
	float&	b,
	const
	float&	a
)
{
	R=LGL_Clamp(0.0f,r,1.0f);
	G=LGL_Clamp(0.0f,g,1.0f);
	B=LGL_Clamp(0.0f,b,1.0f);
	A=LGL_Clamp(0.0f,a,1.0f);
}

LGL_Color::
~LGL_Color()
{
	//
}

const
float
LGL_Color::
GetR()	const
{
	return(R);
}

const
float
LGL_Color::
GetG()	const
{
	return(G);
}

const
float
LGL_Color::
GetB()	const
{
	return(B);
}

const
float
LGL_Color::
GetA()	const
{
	return(A);
}

void
LGL_Color::
SetR
(
	const
	float&	r
)
{
	R=LGL_Clamp(0.0f,r,1.0f);
}

void
LGL_Color::
SetG
(
	const
	float&	g
)
{
	G=LGL_Clamp(0.0f,g,1.0f);
}

void
LGL_Color::
SetB
(
	const
	float&	b
)
{
	B=LGL_Clamp(0.0f,b,1.0f);
}

void
LGL_Color::
SetA
(
	const
	float&	a
)
{
	A=LGL_Clamp(0.0f,a,1.0f);
}

void
LGL_Color::
SetRGB
(
	const
	float&	r,
	const
	float&	g,
	const
	float&	b
)
{
	R=LGL_Clamp(0.0f,r,1.0f);
	G=LGL_Clamp(0.0f,g,1.0f);
	B=LGL_Clamp(0.0f,b,1.0f);
}

void
LGL_Color::
SetRGBA
(
	const
	float&	r,
	const
	float&	g,
	const
	float&	b,
	const
	float&	a
)
{
	R=LGL_Clamp(0.0f,r,1.0f);
	G=LGL_Clamp(0.0f,g,1.0f);
	B=LGL_Clamp(0.0f,b,1.0f);
	A=LGL_Clamp(0.0f,a,1.0f);
}

LGL_Color
LGL_Color::
operator+
(
	const
	LGL_Color&	color
)	const
{
	return
	(
		LGL_Color
		(
			R+color.GetR(),
			G+color.GetG(),
			B+color.GetB(),
			A+color.GetA()
		)
	);
}

LGL_Color
LGL_Color::
operator*
(
	const
	float&	scalar
)	const
{
	return
	(
		LGL_Color
		(
			R*scalar,
			G*scalar,
			B*scalar,
			A*scalar
		)
	);
}



//System Info

std::vector<int>
lgl_read_proc_cpuinfo()
{
	std::vector<int> cpuSpeeds;
#ifndef	LGL_OSX
	if(FILE* fd = fopen("/proc/cpuinfo","r"))
	{
		while(!feof(fd))
		{
			char buf[2048];
			fgets(buf,2047,fd);
			if(strstr(buf,"cpu MHz")==buf)
			{
				char* ptr=strstr(buf,":");
				assert(ptr);
				ptr=&(ptr[2]);
				float cpuSpeed=atof(ptr);
				cpuSpeeds.push_back((int)cpuSpeed);
			}
		}
		fclose(fd);
	}
#else	//LGL_LINUX
#endif	//LGL_LINUX
	return(cpuSpeeds);
}

unsigned int
LGL_CPUCount()
{
	int mib[2];
	size_t len;
	int cpus = 1;

	mib[0] = CTL_HW;
	mib[1] = HW_NCPU;
	len = sizeof(cpus);
	if
	(
		sysctl
		(
			mib,
			2,
			&cpus,
			&len,
			NULL,
			NULL
		) == -1
	)
	{
		printf("could not determine number of cpus available");
		cpus=1;
	}

	return cpus;
}

unsigned int
LGL_CPUSpeed(int which)
{
	assert(which<(int)LGL_CPUCount());
	if(which>=(int)LGL_CPUCount())
	{
		printf("LGL_CPUSpeed(): Warning! Invalid argument '%i' (you have %i CPUs) (A)\n",which, LGL_CPUCount());
	}
#ifdef	LGL_LINUX
	std::vector<int> cpuSpeeds = lgl_read_proc_cpuinfo();
	assert((int)cpuSpeeds.size()>which);
	if((int)cpuSpeeds.size()<=which)
	{
		printf("LGL_CPUSpeed(): Warning! Invalid argument '%i' (you have %i CPUs) (B)\n",which, LGL_CPUCount());
		return(9999);
	}
	if(which==-1)
	{
		//Return the average speed of all CPUs
		unsigned int total=0;
		for(unsigned int a=0;a<cpuSpeeds.size();a++)
		{
			total+=cpuSpeeds[a];
		}
		if(cpuSpeeds.size()!=0) total/=cpuSpeeds.size();
		return(total);
	}
	else
	{
		return(cpuSpeeds[which]);
	}
#else	//LGL_LINUX
	printf("LGL_CPUSpeed(): Warning! This function not yet implemented outside of linux port!\n");
	return(9999);
#endif	//LGL_LINUX
}

int
LGL_CPUTemp()
{
#ifdef	LGL_LINUX
	int cpuTemp=999;
	FILE* fd = fopen("/proc/acpi/thermal_zone/THM/temperature","r");
	if(fd)
	{
		while(!feof(fd))
		{
			char buf[2048];
			fgets(buf,2047,fd);
			if(strstr(buf,"temperature:")==buf)
			{
				char* ptr=strstr(buf,":");
				assert(ptr);
				ptr=&(ptr[1]);
				cpuTemp=atoi(ptr);
			}
		}
		fclose(fd);
	}
	return(cpuTemp);
#else	//LGL_LINUX
	printf("LGL_CPUTemp(): Warning! This function not yet implemented outside of linux port!\n");
	return(999);
#endif	//LGL_LINUX
}

int isOsxAppBundleCached=0;

bool
LGL_IsOsxAppBundle()
{
	if(isOsxAppBundleCached==0)
	{
		char wd[2048];
		getcwd(wd,20480);
		isOsxAppBundleCached = (strstr(wd,"Contents/MacOS")) ? 1 : -1;
	}

	return(isOsxAppBundleCached==1);
}



//Multithreading

int lgl_thread_count=1;

SDL_Thread*
LGL_ThreadCreate
(
	int (*fn)(void *),
	void *data
)
{
	lgl_thread_count++;
	if(lgl_thread_count>100)
	{
		printf("LGL_ThreadCreate(): Warning!! %i threads exist! You're probably not calling LGL_ThreadWait()! Keep this up, and the program will crash!!\n",lgl_thread_count);
	}
	SDL_Thread* ret = SDL_CreateThread(fn,data);
	return(ret);
}

int
LGL_ThreadWait
(
	SDL_Thread*	target
)
{
	int result;
	SDL_WaitThread(target,&result);
	lgl_thread_count--;
	return(result);
}

void
LGL_ThreadKill
(
	SDL_Thread*	target
)
{
	SDL_KillThread(target);
	lgl_thread_count--;
}

void
LGL_ThreadSetPriority
(
	float		priority,
	const char*	threadName
)
{
#ifndef	LGL_OSX
	//Linux version currently runs better without priorities... FIXME
	//return;
#endif

	priority = LGL_Clamp(-1.0f,priority,1.0f);

#ifdef	LGL_OSX

	/*
	struct thread_time_constraint_policy ttcpolicy;
	ttcpolicy.period=period; // HZ/160
	ttcpolicy.computation=computation; // HZ/3300;
	ttcpolicy.constraint=constraint; // HZ/2200;
	ttcpolicy.preemptible=1;
	*/

	thread_precedence_policy_data_t prepolicy;
	prepolicy.importance=(int)(31+priority*40);

	int result=thread_policy_set
	(
		mach_thread_self(),
		THREAD_PRECEDENCE_POLICY,
		&(prepolicy.importance),
		THREAD_PRECEDENCE_POLICY_COUNT
	);

	if(result!=KERN_SUCCESS)
	{
		printf("Couldn't set thread priority '%i' for '%s' (%.2f)\n",prepolicy.importance,threadName,priority);
	}
	else
	{
		//printf("Set thread priority '%i' for '%s' (%.2f)\n",prepolicy.importance,threadName,priority);
	}

	/*
	int policy;
	pthread_t thread = pthread_self();
	struct sched_param schedParam;
	pthread_getschedparam(thread, &policy, &schedParam);
	if(priority<0.0f)
	{
		policy=SCHED_RR;//OTHER;
	}
	else if(priority<1.0f)
	{
		policy=SCHED_RR;
	}
	else
	{
		policy=SCHED_FIFO;
	}
	float priorityScalar = 0.5f*(priority+1.0f);
	schedParam.sched_priority =  (int)(sched_get_priority_min(policy) + priorityScalar*(sched_get_priority_max(policy)-sched_get_priority_min(policy)));
printf("sched_priority: %i (%i - %i)\n",schedParam.sched_priority,sched_get_priority_min(policy),sched_get_priority_max(policy));
	if(pthread_setschedparam(thread, policy, &schedParam)!=0)
	{
		printf("Priority FAIL for '%s' (%i)\n",threadName,schedParam.sched_priority);
	}
	*/
#else
	struct sched_param schedParam;
	if(priority<=0.0f && priority!=-1.0f)
	{
		//Vanilla Process Priority
		int p=(int)(-20+(-priority*20));
		if(setpriority(PRIO_PROCESS,0,p)!=0)
		{
			printf("setpriority(): Error! (%s => %i)\n",threadName,p);
		}
		//printf("LGL_ThreadSetPriority(%.2f, '%s'): setpriority(%i) (A)\n",priority,threadName?threadName:"NULL",p);
	}
	else
	{
		int p=(int)(priority*-20);
		if(setpriority(PRIO_PROCESS,0,p)!=0)
		{
			printf("setpriority(): Error! (%s => %i)\n",threadName,p);
		}
		//printf("LGL_ThreadSetPriority(%.2f, '%s'): setpriority(%i) (B)\n",priority,threadName?threadName:"NULL",-20);
	}

	if(priority<=0.0f)
	{
		//Not realtime
		schedParam.sched_priority = 0;
		//printf("LGL_ThreadSetPriority(%.2f, '%s'): sched_setscheduler(%i) (OTHER)\n",priority,threadName?threadName:"NULL",schedParam.sched_priority);
		sched_setscheduler(0,SCHED_OTHER,&schedParam);
	}
	else if(priority<1.0f)
	{
		//Realtime
		int pmin = sched_get_priority_min(SCHED_RR);
		int pmax = sched_get_priority_max(SCHED_RR);
		int pint = (int)(pmin + priority*(pmax-pmin));
		schedParam.sched_priority = pint;
		//printf("LGL_ThreadSetPriority(%.2f, '%s'): sched_setscheduler(%i) (RR)\n",priority,threadName?threadName:"NULL",schedParam.sched_priority);
		sched_setscheduler(0,SCHED_RR,&schedParam);
	}
	else
	{
		//Realtime, maximum priority
		int pmax = sched_get_priority_max(SCHED_FIFO);
		schedParam.sched_priority = pmax;
		//printf("LGL_ThreadSetPriority(%.2f, '%s'): sched_setscheduler(%i) (FIFO)\n",priority,threadName?threadName:"NULL",schedParam.sched_priority);
		sched_setscheduler(0,SCHED_FIFO,&schedParam);
	}
#endif	//LGL_OSX
}

void
LGL_ThreadSetCPUAffinity
(
	int	cpu
)
{
#ifdef	LGL_OSX
	printf("LGL_ThreadSetCPUAffinity(): Not implemented in OSX!\n");
#else
	cpu_set_t cpuSet;
	CPU_ZERO(&cpuSet);
	CPU_SET(cpu,&cpuSet);
	sched_setaffinity(0,sizeof(cpu_set_t),&cpuSet);
#endif	//LGL_OSX
}

LGL_Semaphore::
LGL_Semaphore
(
	const char*	name,
	bool		promiscuous
)
{
	strcpy(Name,name);
	Sem=SDL_CreateSemaphore(1);
	if(Sem==NULL)
	{
		printf("LGL_Semaphore::LGL_Semaphore(): Error! Returned NULL.\n");
		LGL_Exit();
	}
	Promiscuous=promiscuous;
}

LGL_Semaphore::
~LGL_Semaphore()
{
	SDL_DestroySemaphore(Sem);
}

bool
LGL_Semaphore::
Lock
(
	const char*	file,
	int		line,
	float		timeoutSeconds
)
{
	if(Promiscuous) return(true);

	LGL_Timer timer;
	char name[1024];
	strcpy(name,GetName());
	char lockOwnerFile[1024];
	strcpy(lockOwnerFile,GetLockOwnerFile());
	int lockOwnerLine;
	lockOwnerLine=GetLockOwnerLine();
	bool mainBlocked=false;
	if
	(
		SDL_ThreadID() == LGL.ThreadIDMain &&
		IsLocked() &&
		timeoutSeconds!=0.0f
	)
	{
		mainBlocked=true;
	}

	bool ret=false;
	if(timeoutSeconds==0.0f)
	{
		//Despite its name, SDL_SemTryWait() doesn't actually wait... It's nonblocking.
		ret = (SDL_SemTryWait(Sem)==0);
	}
	else if(timeoutSeconds<0)
	{
		//This suspends the thread until the sem is obtained
		ret = (SDL_SemWait(Sem)==0);
	}
	else
	{
		//This waits a given amount of time, and then gives up.
		ret = (SDL_SemWaitTimeout(Sem,(Uint32)(timeoutSeconds*1000.0))==0);
	}

	if(ret)
	{
		strcpy(LockOwnerFile,file);
		LockOwnerLine=line;
		TimeOfLock = LGL_SecondsSinceExecution();
	}

	bool printMore=false;
	if(mainBlocked)
	{
		printf("Main thread blocks on semaphore!\n");
		printMore=true;
	}
	else if(timer.SecondsSinceLastReset()>1.0f)
	{
		printf("Long lock!\n");
		printMore=true;
	}
	if(printMore)
	{
		printf("\tName: %s\n",name);
		printf("\tTime: %.4f\n",timer.SecondsSinceLastReset());
		printf("\tLock Waiter File: %s\n",file);
		printf("\tLock Waiter Line: %i\n",line);
		printf("\tLock Waiter Thread: %s\n",
			(
				(int)SDL_ThreadID() == 
				(int)LGL.ThreadIDMain
			) ? "Main" : "Not Main"
		);
		printf("\tLock Owner File: %s\n",lockOwnerFile);
		printf("\tLock Owner Line: %i\n",lockOwnerLine);
		printf("\n");
	}
	return(ret);
}

bool
LGL_Semaphore::
Unlock()
{
	if(Promiscuous) return(true);

	LGL_Assertf(Value()==0,("Value = %i",Value()));
	LockOwnerFile[0]='\0';
	LockOwnerLine=-1;
	bool ret=SDL_SemPost(Sem);
	return(ret);
}

bool
LGL_Semaphore::
IsLocked()
{
	if(Promiscuous) return(false);

	return(Value()==0);
}

float
LGL_Semaphore::
SecondsLocked()
{
	if(IsLocked()==false) return(0.0f);
	else return(LGL_SecondsSinceExecution()-TimeOfLock);
}

void
LGL_Semaphore::
PrintLockInfo()
{
	printf("\tSem = %s\n",Name);
	printf("\tLocking file = %s\n",LockOwnerFile);
	printf("\tLocking line = %i\n",LockOwnerLine);
	printf("\n");
}

const char*
LGL_Semaphore::
GetName()
{
	return(Name);
}

const char*
LGL_Semaphore::
GetLockOwnerFile()
{
	return(LockOwnerFile);
}

int
LGL_Semaphore::
GetLockOwnerLine()
{
	return(LockOwnerLine);
}

int
LGL_Semaphore::
Value()
{
	if(Promiscuous) return(1);

	return(SDL_SemValue(Sem));
}



LGL_ScopeLock::
LGL_ScopeLock
(
	const char*	file,
	int		line,
	LGL_Semaphore&	semaphore,
	float		timeoutSeconds
)
{
	Init
	(
		file,
		line,
		&semaphore,
		timeoutSeconds
	);
}

LGL_ScopeLock::
LGL_ScopeLock
(
	const char*	file,
	int		line,
	LGL_Semaphore*	semaphore,
	float		timeoutSeconds
)
{
	Init
	(
		file,
		line,
		semaphore,
		timeoutSeconds
	);
}

void
LGL_ScopeLock::
Init
(
	const char*	file,
	int		line,
	LGL_Semaphore*	semaphore,
	float		timeoutSeconds
)
{
	Semaphore=semaphore;
	LockObtained=false;
	if(Semaphore)
	{
		/*
		LGL_Timer timer;
		char name[1024];
		strcpy(name,Semaphore->GetName());
		char lockOwnerFile[1024];
		strcpy(lockOwnerFile,Semaphore->GetLockOwnerFile());
		int lockOwnerLine;
		lockOwnerLine=Semaphore->GetLockOwnerLine();
		bool mainBlocked=false;
		if
		(
			SDL_ThreadID() == LGL.ThreadIDMain &&
			Semaphore->IsLocked() &&
			timeoutSeconds!=0.0f
		)
		{
			mainBlocked=true;
		}
		*/
		LockObtained=Semaphore->Lock
		(
			file,
			line,
			timeoutSeconds
		);
		/*
		if(mainBlocked)
		{
			printf("Main thread blocks on semaphore!\n");
			printf("\tName: %s\n",name);
			printf("\tTime: %.4f\n",timer.SecondsSinceLastReset());
			printf("\tLock Owner File: %s\n",lockOwnerFile);
			printf("\tLock Owner Line: %i\n",lockOwnerLine);
			printf("\tLock Waiter File: %s\n",file);
			printf("\tLock Waiter Line: %i\n",line);
			printf("\tLock Waiter Thread: %i (%i)\n",
				(int)SDL_ThreadID(),
				(int)LGL.ThreadIDMain);
			printf("\n");
		}
		if(timer.SecondsSinceLastReset()>1.0f)
		{
			printf("Long lock!\n");
			printf("\tName: %s\n",name);
			printf("\tTime: %.4f\n",timer.SecondsSinceLastReset());
			printf("\tLock Owner File: %s\n",lockOwnerFile);
			printf("\tLock Owner Line: %i\n",lockOwnerLine);
			printf("\tLock Waiter File: %s\n",file);
			printf("\tLock Waiter Line: %i\n",line);
			printf("\tLock Waiter Thread: %i (%i)\n",
				(int)SDL_ThreadID(),
				(int)LGL.ThreadIDMain);
			printf("\n");
		}
		*/
	}
	if(LockObtained==false)
	{
		//printf("Lock not obtained!! (timeout = %.2f) (sem = %s)\n",timeoutSeconds,Semaphore ? "OK" : "NULL");
	}
}

LGL_ScopeLock::
~LGL_ScopeLock()
{
	if(Semaphore && LockObtained)
	{
		Semaphore->Unlock();
	}

	Semaphore=NULL;
	LockObtained=false;
}

bool
LGL_ScopeLock::
GetLockObtained()
{
	return(LockObtained);
}



//LEDs

void lgl_set_leds
(
	const char*	host,
	int		port,
	int		numLights,
	int&		socketInstance,
	char		red[],
	char		green[],
	char		blue[]
)
{

#if 0
printf("lgl_set_leds(): '%s' %i\n",host,port);
for(int a=0;a<numLights;a++)
{
	printf("\t[%i]:\t%i\t%i\t%i\n",
		a,
		red[a],
		green[a],
		blue[a]
	);
}
printf("\n");
#endif

	int idx, call;
	struct sockaddr_in sock;
	struct hostent *hp;

	char* red_ptr = red;
	char* green_ptr = green;
	char* blue_ptr = blue; 

	if
	(
		 host==NULL ||
		 host[0]=='\0' ||
		 port==0
	)
	{
		return;
	}

	bzero(&sock,sizeof(sock));

	const int packet_size = 536;
	const int header_size = 21;
	const unsigned char header[] = {4,1,220,74,1,0,1,1,0,0,0,0,0,0,0,0,255,255,255,255,0};
	static unsigned char buffer[packet_size + 1];


	if(socketInstance==-1)
	{
		//Initialize...
		socketInstance = socket(AF_INET, SOCK_DGRAM, 0);
		memcpy(buffer,header,header_size);
	}

	if (socketInstance < 0)
	{
		fprintf(stderr, "lgl_set_leds(): Error creating socket\n");
		return;
	}

	hp = gethostbyname(host);
	if (hp==0)
	{
		fprintf(stderr, "lgl_set_leds(): Unknown host\n");
		return;
	}

	sock.sin_family = hp->h_addrtype;
	sock.sin_port = htons(port);

	memcpy((char *)&sock.sin_addr, (char *)hp->h_addr, hp->h_length);

	for (idx = header_size; idx < header_size + (numLights * 3); idx++)
	{

		switch(idx % 3)
		{
			case 0:
				buffer[idx] = *red_ptr;
				red_ptr++;
				break;
			case 1:
				buffer[idx] = *green_ptr;
				green_ptr++;
				break;
			case 2:
				buffer[idx] = *blue_ptr;
				blue_ptr++;
				break;
		}
	}

	call=sendto
	(
		socketInstance,
		buffer,
		packet_size,
		0,
		(struct sockaddr*) &sock,
		sizeof(struct sockaddr_in)
	);
	
	//close(socketInstance);
	/*
	if (call < 0)
	{
		fprintf(stderr, "lgl_set_leds(): Failed to send (%i) (%i) (%i)\n",packet_size,hp->h_length,call);
	}
	*/
}


//Old
/*
void
lgl_set_leds
(
	const char*	host,
	int		port,
	int&		socketInstance,
	char		red,
	char		green,
	char		blue
)
{
	int idx, call;
	struct sockaddr_in sock;
	struct hostent *hp;

	if
	(
		host==NULL ||
		host[0]=='\0' ||
		port==0
	)
	{
		return;
	}

	bzero(&sock,sizeof(sock));

	const int packet_size = 536;
	const int header_size = 21;
	const unsigned char header[] = {4,1,220,74,1,0,1,1,0,0,0,0,0,0,0,0,255,255,255,255,0};
	static unsigned char buffer[packet_size + 1];


	if(socketInstance==-1)
	{
		//Initialize...
		socketInstance = socket(AF_INET, SOCK_DGRAM, 0);
		memcpy(buffer,header,header_size);
	}

	if (socketInstance < 0)
	{
		fprintf(stderr, "lgl_set_leds(): Error creating socket\n");
		return;
	}

	hp = gethostbyname(host);
	if (hp==0)
	{
		fprintf(stderr, "lgl_set_leds(): Unknown host\n");
		return;
	}

	sock.sin_family = hp->h_addrtype;
	sock.sin_port = htons(port);

	memcpy((char *)&sock.sin_addr, (char *)hp->h_addr, hp->h_length);

	for (idx = header_size; idx < packet_size; idx++) {
		switch(idx % 3) {
			case 0:
				buffer[idx] = red;
				break;
			case 1:
				buffer[idx] = green;
				break;
			case 2:
				buffer[idx] = blue;
				break;
		}
	}
	call=sendto(socketInstance, buffer, packet_size, 0, (struct sockaddr*) &sock, sizeof(struct sockaddr_in));
	//close(socketInstance);
	if (call < 0)
	{
		//fprintf(stderr, "lgl_set_leds(): Failed to send (%i) (%i) (%i)\n",packet_size,hp->h_length,call);
	}
}

void
LGL_SetLEDs
(
	const char*	hostname,
	int		port,
	int		channel,
	int&		socketInstance,
	float		r,
	float		g,
	float		b
)
{
	lgl_set_leds
	(
		hostname,
		port,
		socketInstance,
		(char)(LGL_Clamp(0.0f,r,1.0f)*255),
		(char)(LGL_Clamp(0.0f,g,1.0f)*255),
		(char)(LGL_Clamp(0.0f,b,1.0f)*255)
	);
}
*/

lgl_LEDHost*
lgl_GetLEDHost
(
	const char*	hostname,
	int		port=6038
)
{
	if
	(
		hostname==NULL ||
		port==0
	)
	{
		return(NULL);
	}

	lgl_LEDHost* ret=NULL;

	for(unsigned int a=0;a<LGL.ledHostList.size();a++)
	{
		lgl_LEDHost* now = LGL.ledHostList[a];
		if(now->Matches(hostname,port))
		{
			ret=now;
			break;
		}
	}

	if(ret==NULL)
	{
		ret = new lgl_LEDHost
		(
			hostname,
			port
		);
		LGL.ledHostList.push_back(ret);
	}

	return(ret);
}

lgl_LEDHost::
lgl_LEDHost
(
	const char*	hostname,
	int		port
)
{
	strcpy(Hostname,hostname);
	Port=port;
	SocketInstance=-1;

	for(int a=0;a<128;a++)
	{
		Red[a]=0;
		Green[a]=0;
		Blue[a]=0;
	}
	ChannelCount=0;
}

bool
lgl_LEDHost::
Matches
(
	const char*	hostname,
	int		port
)
{
	return
	(
		port==Port &&
		hostname != NULL &&
		strcmp(hostname,Hostname)==0
	);
}

void
lgl_LEDHost::
SetColor
(
	float	red,
	float	green,
	float	blue,
	int	channel
)
{
	if(channel<0 || channel >= 128)
	{
		return;
	}

	if(channel>=ChannelCount)
	{
		ChannelCount=channel+1;
	}

	Red[channel] = (char)(LGL_Clamp(0.0f,red,1.0f)*255);
	Green[channel] = (char)(LGL_Clamp(0.0f,green,1.0f)*255);
	Blue[channel] = (char)(LGL_Clamp(0.0f,blue,1.0f)*255);
}

void
lgl_LEDHost::
Send()
{
	if(ChannelCount==0)
	{
		return;
	}

	lgl_set_leds
	(
		Hostname,
		Port,
		ChannelCount,
		SocketInstance,
		Red,
		Green,
		Blue
	);
}



LGL_LEDClient::
LGL_LEDClient
(
	const char*	hostname,
	int		port,
	int		channel,
	int		group
)
{
	strcpy(Hostname,hostname);
	Port=port;
	Channel=channel;
	Group=group;

	LEDHost=NULL;
}

LGL_LEDClient::
~LGL_LEDClient()
{
	SetColor(0.0f,0.0f,0.0f);
}

void
LGL_LEDClient::
SetColor
(
	float	red,
	float	green,
	float	blue
)
{
	if(Port==0)
	{
		return;
	}

	if(LEDHost==NULL)
	{
		LEDHost = lgl_GetLEDHost(Hostname,Port);
	}

	if(LEDHost!=NULL)
	{
		LEDHost->SetColor
		(
			red,
			green,
			blue,
			Channel
		);
	}
}

int
LGL_LEDClient::
GetGroup()
{
	return(Group);
}










//Internals
	
int
LGL_NextPowerOfTwo
(
	float	target
)
{
	int a;
	for(a=0;pow(2,a)<target;a++)
	{
		//
	}
	return((int)pow(2,a));
}
/* lgl_fir:
 * Copyright 2000 by Grant R. Griffin
 *
 * The Wide Open License (WOL)
 *
 * Permission to use, copy, modify, distribute and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice and this license appear in all source copies. 
 * THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF
 * ANY KIND. See http://www.dspguru.com/wol.htm for more information.
 *
 */

double
lgl_fir
(
	double	input,
	int	ntaps,
	double*	h,
	double* z,
	int*	pstate
)
{
	double	accum=input;
	int state=*pstate;
	double const* ph;
	double const* pz;

	z[state]=z[state+ntaps]=input;
	ph=h;
	pz=z+state;
	accum=0;
	for(int ii=0;ii<ntaps;ii++)
	{
		accum+=*ph++ * *pz++;
	}
	accum/=ntaps;

	if(--state<0)
	{
		state+=ntaps;
	}
	*pstate=state;
	return(accum);
}

float tempStreamFL[4*2*LGL_SAMPLESIZE];
float tempStreamFR[4*2*LGL_SAMPLESIZE];
float tempStreamBL[4*2*LGL_SAMPLESIZE];
float tempStreamBR[4*2*LGL_SAMPLESIZE];
float tempStreamRecFL[4*2*LGL_SAMPLESIZE];
float tempStreamRecFR[4*2*LGL_SAMPLESIZE];
float tempStreamRecBL[4*2*LGL_SAMPLESIZE];
float tempStreamRecBR[4*2*LGL_SAMPLESIZE];
float tempStreamSilenceFloat[4*2*LGL_SAMPLESIZE];
bool tempStreamSilenceReady=false;

Sint16* tempStream16Silence=NULL;

bool soundRealtimePrioritySet=false;

void
lgl_AudioOutCallbackGenerator
(
	void*	userdata,
	Uint8*	stream8,
	int	len8
);

Uint8 audioOutResult[6*2*LGL_SAMPLESIZE*64];
Uint8 audioOutBuffer[6*2*LGL_SAMPLESIZE*64];
long audioOutBufferFilledBytes=0;

void
lgl_AudioOutCallback
(
	void*	userdata,
	Uint8*	stream8,
	int	len8
)
{
	LGL.AudioOutCallbackTimer.Reset();
	bool mix4to6 = LGL.AudioSpec->channels==6;
	for(;;)
	{
		if(audioOutBufferFilledBytes>=len8)
		{
			break;
		}
		int renderChannels=LGL.AudioSpec->channels;
		if(mix4to6) renderChannels=4;
		const int decodeBytes=renderChannels*2*LGL_SAMPLESIZE;
		const int decodeBytesOut=LGL.AudioSpec->channels*2*LGL_SAMPLESIZE;
		memset(audioOutResult,0,decodeBytes);
		lgl_AudioOutCallbackGenerator
		(
			userdata,
			audioOutResult,
			decodeBytes
		);
		if(mix4to6)
		{
			int len16=len8/2;
			Sint16* audioOutBuffer16 = (Sint16*)audioOutBuffer;
			Sint16* audioOutResult16 = (Sint16*)audioOutResult;
			for(int a=0;a<len16;a++)
			{
				//Front
				audioOutBuffer16[6*a+0]=audioOutResult16[4*a+0];
				audioOutBuffer16[6*a+1]=audioOutResult16[4*a+1];
			
				//Center/Sub
				audioOutBuffer16[6*a+2]=LGL.AudioSpec->silence;
				audioOutBuffer16[6*a+3]=LGL.AudioSpec->silence;
				audioOutBuffer16[6*a+2]=audioOutResult16[4*a+0];
				audioOutBuffer16[6*a+3]=audioOutResult16[4*a+1];

				//Rear
				audioOutBuffer16[6*a+4]=audioOutResult16[4*a+2];
				audioOutBuffer16[6*a+5]=audioOutResult16[4*a+3];
			}
		}
		else
		{
			memcpy(&(audioOutBuffer[audioOutBufferFilledBytes]),audioOutResult,decodeBytesOut);
		}
		audioOutBufferFilledBytes+=decodeBytesOut;
	}
	memcpy(stream8,audioOutBuffer,len8);

	//Save any leftovers for the next time we hit this function.
	Uint8* copyPtr=&(audioOutBuffer[len8]);
	long copySize=audioOutBufferFilledBytes-len8;
	if(copySize)
	{
		memcpy(audioOutResult,copyPtr,copySize);
		memcpy(audioOutBuffer,audioOutResult,copySize);
	}
	audioOutBufferFilledBytes=copySize;
}

float volumeArrayFL[LGL_SAMPLESIZE];
float volumeArrayFR[LGL_SAMPLESIZE];
float volumeArrayBL[LGL_SAMPLESIZE];
float volumeArrayBR[LGL_SAMPLESIZE];

void
lgl_AudioOutCallbackGenerator
(
	void*	userdata,
	Uint8*	stream8,
	int	len8
)
{
	if(1 || LGL.AudioUsingJack==false)
	{
		if(soundRealtimePrioritySet==false)
		{
			setpriority(PRIO_PROCESS,0,-20);

			//Use Only One CPU
			//LGL_ThreadSetCPUAffinity(1);
			//LGL_ThreadSetPriority(LGL_PRIORITY_AUDIO_OUT,"AudioOut");	//It's really that important!

			soundRealtimePrioritySet=true;
		}
	}

	if(LGL.AudioQuitting)
	{
		return;
	}

	unsigned int renderChannels=LGL_AudioChannels();
	if(renderChannels==6) renderChannels=4;

	Sint16* stream16=(Sint16*)stream8;
	int len16=len8/2;
	int decodeLen=len8/(renderChannels/2);//renderChannels==2 ? len8 : len8/2;
	assert(decodeLen/4==LGL_SAMPLESIZE);

	if(tempStream16Silence==NULL)
	{
		int size=LGL_Max(len16*2,LGL_SAMPLESIZE*4*2);
		tempStream16Silence=new Sint16[size];
		for(int l=0;l<size;l++)
		{
			tempStream16Silence[l]=LGL.AudioSpec->silence;
		}
	}
	memcpy(stream16,tempStream16Silence,len16);

	//if(LGL.AudioAvailable==false) return;
	unsigned long PosLastPrev;
	unsigned long PosLastNext;
	unsigned long PosNowPrev;
	unsigned long PosNowNext;
	double closenessPercentInvLast;
	double closenessPercentInvNow;

	double fl;
	double fr;
	double bl;
	double br;
	double flRecord;
	double frRecord;
	double blRecord;
	double brRecord;

	Sint16* stream16rec=(Sint16*)LGL.RecordBuffer;
	memcpy(stream16rec,tempStream16Silence,LGL_SAMPLESIZE*4*4);//len8);

	int encodeChannels=LGL.AudioEncoder ? LGL.AudioEncoder->GetChannelCount() : 0;

	for(int b=0;b<LGL_SOUND_CHANNEL_NUMBER;b++)
	{
		LGL_SoundChannel* sc=&LGL.SoundChannel[b];

		if
		(
			sc->Occupied==false ||
			sc->Paused==true ||
			sc->Buffer==NULL ||
			sc->BufferLength<=0
		)
		{
			continue;
		}

		if
		(
			sc->Occupied &&
			sc->ClearMe
		)
		{
			lgl_ClearAudioChannelNow(b);
		}
		if
		(
			sc->Occupied==false ||
			sc->LGLSound==NULL
		)
		{
			continue;
		}

		if(sc->LGLSound!=NULL)
		{
			//LGL_Sound* snd = sc->LGLSound;
			//snd->LockBuffer();
			//Below line can cause this thread to yield.
			//snd->LockBufferForReading(100);
		}

		int size=4*2*LGL_SAMPLESIZE;
		if(tempStreamSilenceReady==false)
		{
			for(int a=0;a<size;a++)
			{
				tempStreamSilenceFloat[a]=LGL.AudioSpec->silence;
			}
			tempStreamSilenceReady=true;
		}

		memcpy(tempStreamFL,tempStreamSilenceFloat,size);
		memcpy(tempStreamFR,tempStreamSilenceFloat,size);
		memcpy(tempStreamBL,tempStreamSilenceFloat,size);
		memcpy(tempStreamBR,tempStreamSilenceFloat,size);
		memcpy(tempStreamRecFL,tempStreamSilenceFloat,size);
		memcpy(tempStreamRecFR,tempStreamSilenceFloat,size);
		memcpy(tempStreamRecBL,tempStreamSilenceFloat,size);
		memcpy(tempStreamRecBR,tempStreamSilenceFloat,size);

		float vuNext=0.0f;
		double timeAdvancementMultiplier = sc->Hz/(double)LGL.AudioSpec->freq;

		if(sc->FuturePositionSamplesNow>=0)
		{
			sc->PositionSamplesNow=sc->FuturePositionSamplesNow;
			//FIXME: Race condition on this line for FuturePositionSamplesNow
			sc->FuturePositionSamplesNow=-1.0f;

			sc->SampleRateConverterBufferStartSamples=sc->PositionSamplesNow;
		}
		if(sc->FuturePositionSamplesPrev>=0)
		{
			sc->PositionSamplesPrev=sc->FuturePositionSamplesPrev;
			//FIXME: Race condition on this line for FuturePositionSamplesPrev
			sc->FuturePositionSamplesPrev=-1.0f;

			sc->SampleRateConverterBufferStartSamples=sc->PositionSamplesNow;
		}
		if(sc->FutureGlitchSettingsAvailable)
		{
			sc->FutureGlitchSettingsAvailable=false;
			sc->GlitchSpeedDesired=sc->FutureGlitchSpeedDesired;
			sc->GlitchLuminScratchPositionDesired=sc->FutureGlitchLuminScratchPositionDesired;
		}
		if(sc->FutureGlitchSamplesNow!=-10000)
		{
			sc->GlitchSamplesNow=sc->FutureGlitchSamplesNow;
			sc->FutureGlitchSamplesNow=-10000;
		}

		if
		(
			sc->LGLSound &&
			sc->LGLSound->IsLoaded()==false
		)
		{
			sc->LGLSound->MaybeSwapBuffers();
		}

		sc->PositionSamplesDeltaLastTime.Reset();

		for(int a=0;a<decodeLen;a+=4)
		{
			//SAMPLE ADDING LOOP: ALPHA

			fl=LGL.AudioSpec->silence;
			fr=LGL.AudioSpec->silence;
			bl=LGL.AudioSpec->silence;
			br=LGL.AudioSpec->silence;
			flRecord=LGL.AudioSpec->silence;
			frRecord=LGL.AudioSpec->silence;
			blRecord=LGL.AudioSpec->silence;
			brRecord=LGL.AudioSpec->silence;

			if(sc->DivergeCount==0)
			{
				//Track with sound
				sc->DivergeSamples=sc->PositionSamplesNow;
			}

			if(sc->DivergeRecallNow)
			{
				//Recall!
				sc->PositionSamplesNow=sc->DivergeRecallSamples;
				sc->SampleRateConverterBufferStartSamples=sc->DivergeRecallSamples;
				sc->SampleRateConverterBufferValidSamples=0;
				sc->DivergeRecallNow=false;
			}

			if(sc->DivergeCount>=1)
			{
				//Diverge!
				sc->DivergeSamples+=
					sc->DivergeSpeed*timeAdvancementMultiplier;
			}
			if(sc->WarpPointSecondsAlpha>=0.0f)
			{
				if(sc->PositionSamplesNow>=sc->WarpPointSecondsAlpha*sc->Hz)
				{
					if
					(
						sc->WarpPointSecondsOmega >= 0 ||
						sc->LGLSound->IsLoaded()
					)
					{
						sc->PositionSamplesNow=sc->WarpPointSecondsOmega*sc->Hz;
						while(sc->PositionSamplesNow<0) sc->PositionSamplesNow+=sc->LengthSamples*sc->Hz;
						sc->SampleRateConverterBufferStartSamples=sc->PositionSamplesNow;
						sc->SampleRateConverterBufferValidSamples=0;
					}
					if(sc->WarpPointLoop==false)
					{
						sc->WarpPointSecondsAlpha=-1.0f;
						sc->WarpPointSecondsOmega=-1.0f;
						sc->WarpPointLock=false;
					}
				}
				else if
				(
					sc->WarpPointLoop &&
					sc->PositionSamplesNow<=sc->WarpPointSecondsOmega*sc->Hz
				)
				{
					if
					(
						sc->WarpPointSecondsAlpha >= 0 ||
						sc->LGLSound->IsLoaded()
					)
					{
						sc->PositionSamplesNow=sc->WarpPointSecondsAlpha*sc->Hz;
						while(sc->PositionSamplesNow<0) sc->PositionSamplesNow+=sc->LengthSamples*sc->Hz;
						sc->SampleRateConverterBufferStartSamples=sc->PositionSamplesNow;
						sc->SampleRateConverterBufferValidSamples=0;
					}
				}
			}

			PosLastPrev=(unsigned long)(floor(sc->PositionSamplesPrev));
			PosLastNext=(unsigned long)(ceil(sc->PositionSamplesPrev));
			closenessPercentInvLast=sc->PositionSamplesPrev-PosLastPrev;

			PosNowPrev=(unsigned long)(floor(sc->PositionSamplesNow));
			PosNowNext=(unsigned long)(ceil(sc->PositionSamplesNow));
			closenessPercentInvNow=sc->PositionSamplesNow-PosNowPrev;

			int BPS=sc->Channels*2;	//Bytes Per Sample
			int BPS_half=BPS/2;
			bool goForIt=true;
			if
			(
				PosNowPrev*BPS>=sc->BufferLength ||
				PosNowNext*BPS>=sc->BufferLength ||
				(
					sc->PositionSamplesEnd>=0 &&
					(
						sc->PositionSamplesNow>=sc->PositionSamplesEnd ||
						sc->PositionSamplesPrev>=sc->PositionSamplesEnd
					)
				)
			)
			{
				//Loop or stop
				goForIt=false;
				if(sc->Loop)
				{
					if
					(
						sc->StickyEndpoints ||
						(
							(
								sc->LGLSound==NULL ||
								sc->LGLSound->IsLoaded()==false
							) &&
							sc->PositionSamplesEnd>=sc->Hz*50000.0f
						)
					)
					{
						//sc->PositionSamplesNow=sc->PositionSamplesStart;
						//sc->PositionSamplesPrev=sc->PositionSamplesStart;
						//sc->PositionSamplesNowLastReported=sc->PositionSamplesNow;
						sc->PositionSamplesNow=sc->BufferLength/BPS-1;
						sc->PositionSamplesPrev=sc->BufferLength/BPS-1;
						sc->PositionSamplesNowLastReported=sc->PositionSamplesNow;
						sc->FuturePositionSamplesNow=sc->BufferLength/BPS-1;
						sc->SpeedNow=0.0f;
						sc->SpeedDesired=0.0f;
					}
					else
					{
						//We are looping on a valid PositionSamplesEnd
						sc->PositionSamplesNow=sc->PositionSamplesStart;
						sc->PositionSamplesPrev=sc->PositionSamplesStart;
						sc->PositionSamplesNowLastReported=sc->PositionSamplesNow;
					}
					sc->SampleRateConverterBufferStartSamples=0;
					sc->SampleRateConverterBufferValidSamples=0;
					sc->SampleRateConverterBufferCurrentSamplesIndex=0;
				}
				else
				{
					sc->Occupied=false;
					sc->PositionSamplesNow=0;
					sc->PositionSamplesPrev=0;
					sc->FuturePositionSamplesNow=0;
					sc->Buffer=NULL;
					goForIt=false;
				}
			}
			if(sc->Buffer==NULL)
			{
				goForIt=false;
			}
			if(goForIt)
			{
				//Add the interpolated samples
				Sint16* myBuffer=(Sint16*)sc->Buffer;

				Sint16 flnp=SWAP16(myBuffer[PosNowPrev*BPS_half+0]);
				Sint16 frnp=SWAP16(myBuffer[PosNowPrev*BPS_half+1]);
				Sint16 flnn=SWAP16(myBuffer[PosNowNext*BPS_half+0]);
				Sint16 frnn=SWAP16(myBuffer[PosNowNext*BPS_half+1]);
				
				Sint16 blnp=flnp;
				Sint16 brnp=frnp;
				Sint16 blnn=flnn;
				Sint16 brnn=frnn;

				if(sc->Channels==4)
				{
					blnp=SWAP16(myBuffer[PosNowPrev*BPS_half+2]);
					brnp=SWAP16(myBuffer[PosNowPrev*BPS_half+3]);
					blnn=SWAP16(myBuffer[PosNowNext*BPS_half+2]);
					brnn=SWAP16(myBuffer[PosNowNext*BPS_half+3]);
					if(encodeChannels==2)
					{
						flnp=flnp/2+blnp/2;
						frnp=frnp/2+brnp/2;
						flnn=flnp/2+blnn/2;
						frnn=frnp/2+brnn/2;

						blnp=flnp;
						brnp=frnp;
						blnn=flnn;
						brnn=frnn;
					}
				}

				sc->SpeedNow=
					(1.0f-sc->SpeedInterpolationFactor)*sc->SpeedNow+
					(0.0f+sc->SpeedInterpolationFactor)*sc->SpeedDesired;
				if(fabs(sc->SpeedNow-sc->SpeedDesired)<.0003f)
				{
					sc->SpeedNow=sc->SpeedDesired;
				}
				
				if(sc->Glitch)
				{
					double GlitchSamplesNowNext=ceil(sc->GlitchSamplesNow);

					double gFLnn=(Sint16)(sc->GlitchVolume*SWAP16(myBuffer[(unsigned long)(GlitchSamplesNowNext*BPS/2+0)]));
					double gFRnn=(Sint16)(sc->GlitchVolume*SWAP16(myBuffer[(unsigned long)(GlitchSamplesNowNext*BPS/2+1)]));
					double gBLnn=gFLnn;
					double gBRnn=gFRnn;

					if(sc->Channels==4)
					{
						gBLnn=(Sint16)(sc->GlitchVolume*SWAP16(myBuffer[(unsigned long)(GlitchSamplesNowNext*BPS/2+2)]));
						gBRnn=(Sint16)(sc->GlitchVolume*SWAP16(myBuffer[(unsigned long)(GlitchSamplesNowNext*BPS/2+3)]));
						if(encodeChannels==2)
						{
							gFLnn=gFLnn/2 + gBLnn/2;
							gFRnn=gFLnn/2 + gBRnn/2;

							gBLnn=gFLnn;
							gBRnn=gFRnn;
						}
					}

					double myFL=gFLnn;
					double myFR=gFRnn;

					//ToMono is hereby DEPRECATED
					if(sc->ToMono)
					{
						myFL=(myFL+myFR)/2.0f;
						myFR=myFL;
					}

					double myBL=gBLnn;
					double myBR=gBRnn;

					flRecord+=myFL;
					frRecord+=myFR;
					blRecord+=myBL;
					brRecord+=myBR;

					fl+=myFL;
					fr+=myFR;
					bl+=myBL;
					br+=myBR;

					sc->GlitchLast=sc->GlitchSamplesNow;

					if
					(
						sc->GlitchLuminScratch &&
						sc->GlitchLuminScratchPositionDesired!=-10000
					)
					{
						assert(sc->GlitchLuminScratchPositionDesired>=0);
						double terp=1.0;

						//Normal
						double cand1=(double)sc->GlitchLuminScratchPositionDesired-(double)sc->GlitchSamplesNow;

						//Wrap Backwards
						double cand2=(double)sc->GlitchLuminScratchPositionDesired-(double)(sc->BufferLength/BPS)-(double)(sc->GlitchSamplesNow);
						
						//Wrap Forwards
						double cand3=(double)sc->GlitchLuminScratchPositionDesired+(double)(sc->BufferLength/BPS)-(double)(sc->GlitchSamplesNow);

						double win=0;
						if(fabs(cand1)<fabs(cand2))
						{
							win=cand1;
						}
						else
						{
							win=cand2;
						}
						if(fabs(cand3)<fabs(win))
						{
							win=cand3;
						}
						sc->GlitchSpeedDesired=
							(1.0f-terp)*sc->GlitchSpeedDesired+
							(0.0f+terp)*
							win/(.025f*sc->Hz);
					}
					sc->GlitchSpeedNow=
						(1.0f-sc->GlitchSpeedInterpolationFactor)*sc->GlitchSpeedNow+
						(0.0f+sc->GlitchSpeedInterpolationFactor)*sc->GlitchSpeedDesired;
					/*
					float proxFactor=LGL_Clamp
					(
						0,
						1.0f-(100*fabsf(sc->GlitchSamplesNow-sc->GlitchLuminScratchPositionDesired))/sc->Hz,
						1
					);
					sc->GlitchSpeedNow*=(1.0f-proxFactor);
					*/
					if(fabs(sc->GlitchSpeedNow-sc->GlitchSpeedDesired)<.0003f)
					{
						sc->GlitchSpeedNow=sc->GlitchSpeedDesired;
					}
					sc->GlitchSamplesNow+=sc->GlitchSpeedNow*timeAdvancementMultiplier;
					if(sc->GlitchLuminScratch==false)
					{
						if
						(
							sc->GlitchSamplesNow >=
							sc->GlitchBegin+
							sc->GlitchLength 
						)
						{
							sc->GlitchSamplesNow=sc->GlitchBegin;
						}
						if
						(
							sc->GlitchSamplesNow <
							sc->GlitchBegin
						)
						{
							sc->GlitchSamplesNow=
								sc->GlitchBegin+sc->GlitchLength;
						}
					}
				
					if
					(
						sc->GlitchSamplesNow >=
						sc->BufferLength/BPS
					)
					{
						sc->GlitchSamplesNow-=sc->BufferLength/BPS;
					}
					if(sc->GlitchSamplesNow < 0)
					{
						if
						(
							sc->LGLSound==NULL ||
							sc->LGLSound->IsLoaded()
						)
						{
							sc->GlitchSamplesNow+=(sc->BufferLength/BPS-1);
						}
						else
						{
							sc->GlitchSamplesNow=0;
						}
					}
				}

				//Volume
				float tmpFact = LGL_Min(1.0f,fabsf(sc->SpeedNow)/0.02f);
				float terp=0.95f;
				sc->SpeedVolumeFactor =
					(0.0f + terp) * sc->SpeedVolumeFactor +
					(1.0f - terp) * tmpFact;

				//Default to linear interpolation
				double myFL = (flnp*(1.0-closenessPercentInvNow) + flnn*closenessPercentInvNow);
				double myFR = (frnp*(1.0-closenessPercentInvNow) + frnn*closenessPercentInvNow);
				double myBL = (blnp*(1.0-closenessPercentInvNow) + blnn*closenessPercentInvNow);
				double myBR = (brnp*(1.0-closenessPercentInvNow) + brnn*closenessPercentInvNow);

				long sampleNow=(long)sc->PositionSamplesNow;

				//Attempt to use libsamplerate
				if(fabsf(sc->SpeedNow)<2.0f)
				{
					if
					(
						sc->SpeedNow==0.0f ||
						sc->SampleRateConverterBufferCurrentSamplesIndex<sc->SampleRateConverterBufferValidSamples
						//sampleNow>=sc->SampleRateConverterBufferStartSamples &&
						//sampleNow<sc->SampleRateConverterBufferStartSamples+sc->SampleRateConverterBufferValidSamples*(1.0f/sc->SampleRateConverterBufferSpeed)
					)
					{
						//We may use SampleRateConverterBuffer, below
					}
					else
					{
						//We must generate SampleRateConverterBuffer

						sc->SampleRateConverterBufferValidSamples=0;
						while(sc->SampleRateConverterBufferValidSamples==0)
						{
							for(int c=0;c<sc->Channels;c++)
							{
								SRC_DATA srcData;
								float srcBufIn[SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES];
								float srcBufOut[SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES];
								long indexStart=sampleNow-SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES_EDGE;
								if(sc->SampleRateConverterBufferStartSamples!=-1)
								{
									indexStart=(c==0) ?
										(sc->SampleRateConverterBufferStartSamples+sc->SampleRateConverterBufferConsumedSamples) :
										sc->SampleRateConverterBufferStartSamples;
								}
								for(int b=0;b<SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES;b++)
								{
									long index=indexStart+b;
									if(index<0) index=0;
									if(index*BPS>=(long)sc->BufferLength) index=0;
									srcBufIn[b]=myBuffer[index*BPS_half+c]/32767.0f;
								}

								srcData.data_in=srcBufIn;
								srcData.data_out=srcBufOut;
								srcData.input_frames=SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES;
								srcData.output_frames=SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES;
								srcData.src_ratio=(1.0f/LGL_Max(0.01f,fabsf(sc->SpeedNow)))*(LGL.AudioSpec->freq/(float)sc->Hz);
								srcData.end_of_input=0;

								src_set_ratio(sc->SampleRateConverter[c],srcData.src_ratio);
								src_process(sc->SampleRateConverter[c],&srcData);

								for(int b=0;b<srcData.output_frames_gen;b++)
								{
									float* buf=sc->SampleRateConverterBuffer[c];
									buf[b]=srcBufOut[b]*32767.0f;
								}
								sc->SampleRateConverterBufferSpeed=sc->SpeedNow;
								sc->SampleRateConverterBufferStartSamples=indexStart;
								sc->SampleRateConverterBufferConsumedSamples=srcData.input_frames_used*LGL_Sign(sc->SpeedNow);
								sc->SampleRateConverterBufferValidSamples=srcData.output_frames_gen;
								sc->SampleRateConverterBufferCurrentSamplesIndex=0;
							}
						}
					}

					double neoFL=0.0f;
					double neoFR=0.0f;
					double neoBL=0.0f;
					double neoBR=0.0f;

					if
					(
						sc->SampleRateConverterBufferCurrentSamplesIndex>=0 &&
						sc->SampleRateConverterBufferCurrentSamplesIndex<sc->SampleRateConverterBufferValidSamples
					)
					{
						int index=sc->SampleRateConverterBufferCurrentSamplesIndex;
						sc->SampleRateConverterBufferCurrentSamplesIndex++;
						if(index>=0 && index<SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES)
						{
							if(index<sc->SampleRateConverterBufferValidSamples)
							{
								neoFL = sc->SampleRateConverterBuffer[0][index];
								neoFR = sc->SampleRateConverterBuffer[1][index];
								neoBL = sc->SampleRateConverterBuffer[2%sc->Channels][index];
								neoBR = sc->SampleRateConverterBuffer[3%sc->Channels][index];
							}
						}
					}
					else if
					(
						sc->SpeedNow==0.0f &&
						sc->SampleRateConverterBufferValidSamples>0
					)
					{
						int index=LGL_Max(0,sc->SampleRateConverterBufferCurrentSamplesIndex-1);
						neoFL = sc->SampleRateConverterBuffer[0][index];
						neoFR = sc->SampleRateConverterBuffer[1][index];
						neoBL = sc->SampleRateConverterBuffer[2%sc->Channels][index];
						neoBR = sc->SampleRateConverterBuffer[3%sc->Channels][index];
					}

					if(encodeChannels==2)
					{
						neoFL=neoFL/2 + neoBL/2;
						neoFR=neoFR/2 + neoBR/2;

						neoBL=neoFL;
						neoBR=neoFR;
					}

					//Interpolate to linear resampling
					float sampleRateLinearFactor=0.0f;
					if(fabsf(sc->SpeedNow)>1.5f)
					{
						sampleRateLinearFactor = 2.0f*(fabsf(sc->SpeedNow)-1.5f);
					}
					myFL =	(1.0f-sampleRateLinearFactor)*neoFL +
						(0.0f+sampleRateLinearFactor)*myFL;
					myFR =	(1.0f-sampleRateLinearFactor)*neoFR +
						(0.0f+sampleRateLinearFactor)*myFR;
					myBL =	(1.0f-sampleRateLinearFactor)*neoBL +
						(0.0f+sampleRateLinearFactor)*myBL;
					myBR =	(1.0f-sampleRateLinearFactor)*neoBR +
						(0.0f+sampleRateLinearFactor)*myBR;
				}
				else
				{
					if(SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES==1)
					{
						for(int c=0;c<sc->Channels;c++)
						{
							SRC_DATA srcData;
							float srcBufIn[SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES];
							float srcBufOut[SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES];
							long indexStart=sampleNow-SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES_EDGE;
							if(sc->SampleRateConverterBufferStartSamples!=-1)
							{
								indexStart=(c==0) ?
									(sc->SampleRateConverterBufferStartSamples+sc->SampleRateConverterBufferConsumedSamples) :
									sc->SampleRateConverterBufferStartSamples;
							}
							for(int b=0;b<SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES;b++)
							{
								long index=indexStart+b;
								if(index<0) index=0;
								if(index*BPS>=(long)sc->BufferLength) index=0;
								srcBufIn[b]=myBuffer[index*BPS_half+c]/32767.0f;
							}

							srcData.data_in=srcBufIn;
							srcData.data_out=srcBufOut;
							srcData.input_frames=SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES;
							srcData.output_frames=SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES;
							srcData.src_ratio=(1.0f/LGL_Max(0.01f,fabsf(sc->SpeedNow)))*(LGL.AudioSpec->freq/(float)sc->Hz);
							srcData.end_of_input=0;

							src_set_ratio(sc->SampleRateConverter[c],srcData.src_ratio);
							src_process(sc->SampleRateConverter[c],&srcData);
						}
					}

					if(sc->SampleRateConverterBufferStartSamples!=-1)
					{
						sc->SampleRateConverterBufferStartSamples=-1;
						sc->SampleRateConverterBufferValidSamples=0;
						//The below calls would be nice, but are way too expensive to execute in the audio thread...
						//src_reset(sc->SampleRateConverterL);
						//src_reset(sc->SampleRateConverterR);
					}
				}

				double myFLStereo=myFL;
				double myFRStereo=myFR;
				double myBLStereo=myBL;
				double myBRStereo=myBR;

				if
				(
					sc->ToMono &&
					(
						sc->Glitch==false &&
						sc->GlitchDuo==0 &&
						sc->GlitchLuminScratch==false
					)
				)
				{
					//Stereo=>Mono Mode: Averaging
					myFL=(myFL+myFR)/2.0f;
					myFR=myFL;
				}

				//double myBL=myFL;
				//double myBR=myFR;

				double localSpeedVolFactor = sc->SpeedVolumeFactor;

				if
				(
					sc->Glitch &&
					sc->GlitchDuo>0
				)
				{
					localSpeedVolFactor*=sc->GlitchDuo;
				}
				else if
				(
					sc->Glitch &&
					!sc->GlitchDuo
				)
				{
					localSpeedVolFactor=0;
				}

				flRecord+=myFLStereo;
				frRecord+=myFRStereo;
				blRecord+=myBLStereo;
				brRecord+=myBRStereo;

				/*
				if(sc->VolumeFrontLeftDesired==0.0f)
				{
					sc->VolumeFrontLeft=sc->VolumeFrontLeftDesired;
				}
				if(sc->VolumeFrontRightDesired==0.0f)
				{
					sc->VolumeFrontRight=sc->VolumeFrontRightDesired;
				}
				if(sc->VolumeBackLeftDesired==0.0f)
				{
					sc->VolumeBackLeft=sc->VolumeBackLeftDesired;
				}
				if(sc->VolumeBackRightDesired==0.0f)
				{
					sc->VolumeBackRight=sc->VolumeBackRightDesired;
				}
				*/

				float lerp=0.05f;
				sc->VolumeFrontLeft=
					(1.0f-lerp) * sc->VolumeFrontLeft +
					(0.0f+lerp) * sc->VolumeFrontLeftDesired;
				sc->VolumeFrontRight=
					(1.0f-lerp) * sc->VolumeFrontRight +
					(0.0f+lerp) * sc->VolumeFrontRightDesired;
				sc->VolumeBackLeft=
					(1.0f-lerp) * sc->VolumeBackLeft +
					(0.0f+lerp) * sc->VolumeBackLeftDesired;
				sc->VolumeBackRight=
					(1.0f-lerp) * sc->VolumeBackRight +
					(0.0f+lerp) * sc->VolumeBackRightDesired;

				if(sc->VolumeFrontLeft<0.000001f)	sc->VolumeFrontLeft=0.0f;
				if(sc->VolumeFrontRight<0.000001f)	sc->VolumeFrontRight=0.0f;
				if(sc->VolumeBackLeft<0.000001f)	sc->VolumeBackLeft=0.0f;
				if(sc->VolumeBackRight<0.000001f)	sc->VolumeBackRight=0.0f;

				volumeArrayFL[a/4]=sc->VolumeFrontLeft;
				volumeArrayFR[a/4]=sc->VolumeFrontRight;
				volumeArrayBL[a/4]=sc->VolumeBackLeft;
				volumeArrayBR[a/4]=sc->VolumeBackRight;

				fl+=myFL*localSpeedVolFactor;
				fr+=myFR*localSpeedVolFactor;
				bl+=myBL*localSpeedVolFactor;
				br+=myBR*localSpeedVolFactor;

				if(sc->RhythmicVolumeInvert)
				{
					bool muted=(((long)fabsf(sc->PositionSamplesNow-sc->RhythmicInvertAlphaSamples)/sc->RhythmicInvertDeltaSamples)%2)==1;
					if(sc->PositionSamplesNow<sc->RhythmicInvertAlphaSamples)
					{
						muted=!muted;
					}
					if(muted)
					{
						volumeArrayFL[a/4]=0;
						volumeArrayFR[a/4]=0;
						volumeArrayBL[a/4]=0;
						volumeArrayBR[a/4]=0;
					}
				}

				if(sc->RespondToRhythmicSoloInvertChannel!=-1)
				{
					LGL_SoundChannel* scSolo=&LGL.SoundChannel[sc->RespondToRhythmicSoloInvertChannel];
					bool muted=
						(
							(
								(long)fabsf
									(
										scSolo->PositionSamplesNow-
										scSolo->RhythmicInvertAlphaSamples
									) /
									(
										(scSolo->RhythmicInvertDeltaSamples > 0) ?
										scSolo->RhythmicInvertDeltaSamples :
										1
									) %2
							)
						)==1;
					//TODO: This isn't sample-accurate, but is acceptable for most purposes
					if(scSolo->PositionSamplesNow<scSolo->RhythmicInvertAlphaSamples)
					{
						muted=!muted;
					}
					if(muted)
					{
						volumeArrayFL[a/4]=0;
						volumeArrayFR[a/4]=0;
						volumeArrayBL[a/4]=0;
						volumeArrayBR[a/4]=0;
					}
					sc->RespondToRhythmicSoloInvertCurrentValue=muted ? 0 : 1;
				}
				else
				{
					sc->RespondToRhythmicSoloInvertCurrentValue=1;
				}

				//Volume Omega

				sc->PositionSamplesPrev=
					sc->PositionSamplesNow;
				sc->PositionSamplesNow+=
					sc->SpeedNow*(sc->Hz/(float)LGL.AudioSpec->freq);
				if(sc->PositionSamplesNow<0)
				{
					if
					(
						sc->Loop &&
						sc->StickyEndpoints==false &&
						(
							sc->LGLSound==NULL ||
							sc->LGLSound->IsLoaded()
						)
					)
					{
						sc->PositionSamplesNow=-1;
						sc->PositionSamplesPrev=-1;
						sc->PositionSamplesNow+=
							sc->BufferLength/BPS;
						sc->PositionSamplesPrev+=
							sc->BufferLength/BPS;
						sc->PositionSamplesNowLastReported=sc->PositionSamplesNow;
					}
					else
					{
						sc->PositionSamplesNow=0;
						sc->PositionSamplesPrev=0;
					}
					sc->SampleRateConverterBufferStartSamples=sc->PositionSamplesNow;
					sc->SampleRateConverterBufferValidSamples=0;
				}
				if
				(
					sc->PositionSamplesNow >=
					sc->BufferLength*BPS
				)
				{
					if(sc->Loop)
					{
						sc->PositionSamplesNow-=
							sc->BufferLength/BPS;
						sc->PositionSamplesPrev-=
							sc->BufferLength/BPS;
					}
					else
					{
						sc->PositionSamplesNow=0;
						sc->PositionSamplesPrev=0;
					}
					sc->SampleRateConverterBufferStartSamples=sc->PositionSamplesNow;
					sc->SampleRateConverterBufferValidSamples=0;
				}
			}
			if(fl<-32767) fl=-32767;
			if(fr<-32767) fr=-32767;
			if(bl<-32767) bl=-32767;
			if(br<-32767) br=-32767;

			if(fl>32767) fl=32767;
			if(fr>32767) fr=32767;
			if(bl>32767) bl=32767;
			if(br>32767) br=32767;

			const float QUIET_FACTOR=0.5f;

			tempStreamFL[a/4+0]+=SWAP16((Sint16)(QUIET_FACTOR*fl));
			tempStreamFR[a/4+0]+=SWAP16((Sint16)(QUIET_FACTOR*fr));
			tempStreamBL[a/4+0]+=SWAP16((Sint16)(QUIET_FACTOR*bl));
			tempStreamBR[a/4+0]+=SWAP16((Sint16)(QUIET_FACTOR*br));

			//

			if(flRecord<-32767) flRecord=-32767;
			if(frRecord<-32767) frRecord=-32767;
			if(flRecord>32767) flRecord=32767;
			if(frRecord>32767) frRecord=32767;

			if(blRecord<-32767) blRecord=-32767;
			if(brRecord<-32767) brRecord=-32767;
			if(blRecord>32767) blRecord=32767;
			if(brRecord>32767) brRecord=32767;

			vuNext=LGL_Max
			(
				vuNext,
				LGL_Max
				(
					LGL_Max
					(
						fabsf(flRecord/32767.0f),
						fabsf(frRecord/32767.0f)
					),
					LGL_Max
					(
						fabsf(blRecord/32767.0f),
						fabsf(brRecord/32767.0f)
					)
				)
			);

			tempStreamRecFL[a/4+0]+=(Sint16)(QUIET_FACTOR*flRecord);		//Should we SWAP16 here??? I'm not sure.
			tempStreamRecFR[a/4+0]+=SWAP16((Sint16)(QUIET_FACTOR*frRecord));	//So we'll split the difference, to see.
			tempStreamRecBL[a/4+0]+=(Sint16)(QUIET_FACTOR*blRecord);		//Should we SWAP16 here??? I'm not sure.
			tempStreamRecBR[a/4+0]+=SWAP16((Sint16)(QUIET_FACTOR*brRecord));	//So we'll split the difference, to see.

			//SAMPLE ADDING LOOP: OMEGA
		}
		sc->PositionSamplesDeltaLastTime.Reset();

		//DSP!! Woo hoo!!
		float tempStreamDSPInStereo[LGL_SAMPLESIZE*2];
		float tempStreamDSPOutStereo[LGL_SAMPLESIZE*2];
		for(int a=0;a<sc->Channels/2;a++)
		{
			if(sc->LGLAudioDSP[a])
			{
				//Stereo
				float* streamL = (a!=0) ? tempStreamBL : tempStreamFL;
				float* streamR = (a!=0) ? tempStreamBR : tempStreamFR;
				float* recL = (a!=0) ? tempStreamRecBL : tempStreamRecFL;
				float* recR = (a!=0) ? tempStreamRecBR : tempStreamRecFR;
				for(unsigned int s=0;s<LGL_SAMPLESIZE*2;s+=2)
				{
					tempStreamDSPInStereo[s]=streamL[s/2];
					tempStreamDSPInStereo[s+1]=streamR[s/2];
				}
				sc->LGLAudioDSP[a]->ProcessStereo
				(
					tempStreamDSPInStereo,
					tempStreamDSPOutStereo,
					LGL_SAMPLESIZE
				);
				for(unsigned int s=0;s<LGL_SAMPLESIZE*2;s+=2)
				{
					streamL[s/2]=LGL_Clamp(-32767.0f,tempStreamDSPOutStereo[s],32767.0f);
					streamR[s/2]=LGL_Clamp(-32767.0f,tempStreamDSPOutStereo[s+1],32767.0f);
					recL[s/2]=streamL[s];
					recR[s/2]=streamR[s];
				}
			}
		}

		if(sc->Channels==2)
		{
			memcpy(tempStreamBL,tempStreamFL,4*2*LGL_SAMPLESIZE*sizeof(float));
			memcpy(tempStreamBR,tempStreamFR,4*2*LGL_SAMPLESIZE*sizeof(float));
		}

		for(int a=0;a<decodeLen/4;a++)
		{
			tempStreamFL[a]=(Sint16)LGL_Clamp(-32767,tempStreamFL[a]*volumeArrayFL[a],32767);
			tempStreamFR[a]=(Sint16)LGL_Clamp(-32767,tempStreamFR[a]*volumeArrayFR[a],32767);
			tempStreamBL[a]=(Sint16)LGL_Clamp(-32767,tempStreamBL[a]*volumeArrayBL[a],32767);
			tempStreamBR[a]=(Sint16)LGL_Clamp(-32767,tempStreamBR[a]*volumeArrayBR[a],32767);
		}
		memcpy(tempStreamRecFL,tempStreamFL,4*2*LGL_SAMPLESIZE*sizeof(float));
		memcpy(tempStreamRecFR,tempStreamFR,4*2*LGL_SAMPLESIZE*sizeof(float));
		memcpy(tempStreamRecBL,tempStreamBL,4*2*LGL_SAMPLESIZE*sizeof(float));
		memcpy(tempStreamRecBR,tempStreamBR,4*2*LGL_SAMPLESIZE*sizeof(float));

		int lgSize=0;
		while(pow(2,lgSize)!=LGL_SAMPLESIZE)
		{
			lgSize++;
		}

		int top=2*LGL_SAMPLESIZE;
		if(renderChannels==2)
		{
			for(int l=0,lHalf=0;l<top;l+=2,lHalf++)
			{
				stream16[l+0]=(Sint16)LGL_Clamp(-32767,stream16[l+0]+tempStreamFL[lHalf],32767);	//SWAP16()?
				stream16[l+1]=(Sint16)LGL_Clamp(-32767,stream16[l+1]+tempStreamFR[lHalf],32767);
				if(encodeChannels==2)
				{
					stream16rec[l+0]=(Sint16)LGL_Clamp(-32767,stream16rec[l+0]+tempStreamRecFL[lHalf]*LGL.RecordVolume,32767);
					stream16rec[l+1]=(Sint16)LGL_Clamp(-32767,stream16rec[l+1]+tempStreamRecFR[lHalf]*LGL.RecordVolume,32767);
				}
				else if(encodeChannels==4)
				{
					int index=(l*(encodeChannels/2));
					stream16rec[index]=(Sint16)
						LGL_Clamp
						(
							-32767,
							stream16rec[index]+tempStreamRecFL[lHalf]*LGL.RecordVolume,
							32767
						);
					index++;
					stream16rec[index]=(Sint16)
						LGL_Clamp
						(
							-32767,
							stream16rec[index]+tempStreamRecFR[lHalf]*LGL.RecordVolume,
							32767
						);
					index++;
					stream16rec[index]=(Sint16)
						LGL_Clamp
						(
							-32767,
							stream16rec[index]+tempStreamRecBL[lHalf]*LGL.RecordVolume,
							32767
						);
					index++;
					stream16rec[index]=(Sint16)
						LGL_Clamp
						(
							-32767,
							stream16rec[index]+tempStreamRecBR[lHalf]*LGL.RecordVolume,
							32767
						);
				}
				
				if(LGL.AudioBufferPos+lHalf<1024)
				{
					LGL.AudioBufferLBack[LGL.AudioBufferPos+lHalf]+=(tempStreamFL[lHalf]/32767.0f);
					LGL.AudioBufferRBack[LGL.AudioBufferPos+lHalf]+=(tempStreamFR[lHalf]/32767.0f);
				}
			}
		}
		else if(renderChannels==4)
		{
			int max=top/2;
			for(int l=0;l<max;l++)
			{
				//Mix in the front channels
				stream16[4*l+0]=(Sint16)LGL_Clamp(-32767,stream16[4*l+0]+tempStreamFL[l],32767);	//SWAP16()?
				stream16[4*l+1]=(Sint16)LGL_Clamp(-32767,stream16[4*l+1]+tempStreamFR[l],32767);

				//Mix in the back channels
				if(LGL.AudioMasterToHeadphones)
				{
					stream16[4*l+2]=(Sint16)LGL_Clamp(-32767,stream16[4*l+2]+tempStreamFL[l],32767);	//SWAP16()?
					stream16[4*l+3]=(Sint16)LGL_Clamp(-32767,stream16[4*l+3]+tempStreamFR[l],32767);
				}
				else
				{
					stream16[4*l+2]=(Sint16)LGL_Clamp(-32767,stream16[4*l+2]+tempStreamBL[l],32767);	//SWAP16()?
					stream16[4*l+3]=(Sint16)LGL_Clamp(-32767,stream16[4*l+3]+tempStreamBR[l],32767);
				}

				//And the recording channels (front).
				stream16rec[encodeChannels*l+0]=(Sint16)LGL_Clamp(-32767,stream16rec[encodeChannels*l+0]+tempStreamRecFL[l]*LGL.RecordVolume,32767);
				stream16rec[encodeChannels*l+1]=(Sint16)LGL_Clamp(-32767,stream16rec[encodeChannels*l+1]+tempStreamRecFR[l]*LGL.RecordVolume,32767);
				if(encodeChannels==4)
				{
					stream16rec[encodeChannels*l+2]=(Sint16)LGL_Clamp(-32767,stream16rec[encodeChannels*l+2]+tempStreamRecBL[l]*LGL.RecordVolume,32767);
					stream16rec[encodeChannels*l+3]=(Sint16)LGL_Clamp(-32767,stream16rec[encodeChannels*l+3]+tempStreamRecBR[l]*LGL.RecordVolume,32767);
				}

				if(LGL.AudioBufferPos+l<1024)
				{
					LGL.AudioBufferLBack[LGL.AudioBufferPos+l]+=(tempStreamFL[l]/32767.0f);
					LGL.AudioBufferRBack[LGL.AudioBufferPos+l]+=tempStreamFR[l]/32767.0f;
				}
			}
		}
		else
		{
			LGL_Assert(false);
		}
		
		sc->VU=vuNext;

		if(sc->LGLSound!=NULL)
		{
			//sc->LGLSound->UnlockBuffer();
			//Below line can cause this thread to yield.
			//sc->LGLSound->UnlockBufferForReading(101);
		}
	}

	if(lgl_AudioSwapOutputStreams)
	{
		int top=2*LGL_SAMPLESIZE;
		int max=top/2;
		for(int l=0;l<max;l++)
		{
			Sint16 frontL=stream16[4*l+0];
			Sint16 frontR=stream16[4*l+1];
			Sint16 backL=stream16[4*l+2];
			Sint16 backR=stream16[4*l+3];
			stream16[4*l+0]=backL;
			stream16[4*l+1]=backR;
			stream16[4*l+2]=frontL;
			stream16[4*l+3]=frontR;
		}
	}

	for(int b=0;b<LGL_SOUND_CHANNEL_NUMBER;b++)
	{
		LGL_SoundChannel* sc=&LGL.SoundChannel[b];
		sc->PositionSamplesNowOutwards=sc->PositionSamplesNow;
	}

	if(LGL.AudioEncoder)
	{
		LGL.AudioEncoder->Encode((const char*)(LGL.RecordBuffer),(encodeChannels/2)*len8/(renderChannels/2));
		LGL.RecordSamplesWritten+=len8/(2*renderChannels);
	}
	LGL.AudioBufferPos=LGL.AudioBufferPos+LGL_SAMPLESIZE;
}

Uint8* streamStereo8=NULL;
Sint16* streamStereo16=NULL;

void lgl_AudioInCallback(void *udata, Uint8 *stream, int len8)
{
	Sint16* stream16=(Sint16*)stream;
	int len16=len8/2;
	if(len16!=1024)
	{
		printf("len16: %i\n",len16);
		assert(len16==1024);
	}

	if(streamStereo8==NULL)
	{
		streamStereo8=new Uint8[len8*2];
		streamStereo16=(Sint16*)streamStereo8;
	}
	for(int a=0;a<len16;a++)
	{
		streamStereo16[2*a+0]=stream16[a]+stream16[a+1];
		streamStereo16[2*a+1]=stream16[a]+stream16[a+1];
	}

	LGL_AudioGrain* grain=new LGL_AudioGrain;
	grain->SetWaveformFromMemory
	(
		streamStereo8,
		len16		//LengthSamples
	);
	{
		LGL_ScopeLock lock(__FILE__,__LINE__,LGL.AudioInSemaphore);
		LGL.AudioInGrainListBack.push_back(grain);
	}
}

void
LGL_ShutDown()
{
	if(LGL.Running==false)
	{
		return;
	}

	if(LGL_FileExists("/tmp/gpuspeed")) LGL_FileDelete("/tmp/gpuspeed");
	if(LGL_FileExists("/tmp/gputemp")) LGL_FileDelete("/tmp/gputemp");

	LGL.Running=false;

	LGL_MouseVisible(true);

	//This SEEMS like a good idea, but it segfaults
	/*
	for(int a=0;a<8;a++)
	{
		LGL.Wiimote[8].Disconnect();
	}
	*/

	LGL.AudioQuitting=true;
//printf("LockAudio() 5\n");

	{
		LGL_ScopeLock(__FILE__,__LINE__,LGL.AudioEncoderSemaphore);
		if(LGL.AudioUsingJack==false)
		{
			SDL_LockAudio();
		}
		{
			LGL.AudioAvailable=false;
		}
		if(LGL.AudioUsingJack==false)
		{
			SDL_UnlockAudio();
		}
	}

	if(LGL.AudioStreamListSemaphore!=NULL)
	{
		delete LGL.AudioStreamListSemaphore;
		LGL.AudioStreamListSemaphore=NULL;
	}
	if(LGL.Font!=NULL)
	{
		delete LGL.Font;
		LGL.Font=NULL;
	}

	if(LGL.AudioUsingJack)
	{
		jack_client_close(jack_client);
	}
	else
	{
		SDL_CloseAudio();
	}

	delete LGL.AudioEncoder;

	SDLNet_Quit();

	if(LGL.DrawLogFD!=0)
	{
		fflush(LGL.DrawLogFD);
		fclose(LGL.DrawLogFD);
		LGL.DrawLogFD=0;
	}

	SDL_Quit();

	if(LGL.RecordMovie)
	{
		printf
		(
			"LGL_RecordMovie: Encoding %.1f seconds into xvid...\n",
			LGL_SecondsSinceExecution()-LGL.RecordMovieSecondsSinceExecutionInitial
		);
		int Delay=(int)(ceil(100*1.0/(float)LGL.RecordMovieFPS));
		char command[2048];
#ifdef	LGL_WIN32
		sprintf
		(
			command,
			"mencoder \"mf://%s*.png\" -mf fps=%i -o movie.avi -ovc lavc -lavcopts vcodec=mpeg4:vhq:vbitrate=4000 && echo \"Creating animated gif...\" && convert -delay %i -loop 0 \"%s*.png\" movie.gif && del %s*.png",
			LGL.RecordMovieFileNamePrefix,
			LGL.RecordMovieFPS,
			Delay,
			LGL.RecordMovieFileNamePrefix,
			LGL.RecordMovieFileNamePrefix
		);
#else	//LGL_WIN32
		sprintf
		(
			command,
			"mencoder \"mf://%s*.png\" -mf fps=%i -o movie.avi -ovc lavc -lavcopts vcodec=mpeg4:vhq:vbitrate=4000 && echo \"Creating animated gif...\" && convert -delay %i -loop 0 \"%s*.png\" movie.gif && rm -f %s*.png",
			LGL.RecordMovieFileNamePrefix,
			LGL.RecordMovieFPS,
			Delay,
			LGL.RecordMovieFileNamePrefix,
			LGL.RecordMovieFileNamePrefix
		);
#endif	//LGL_WIN32
		system(command);
	}

	//Delete tmp files
	char path[2048];
	sprintf(path,"%s/.dvj/video/tmp",LGL_GetHomeDir());
	if(LGL_DirectoryExists(path))
	{
		LGL_DirTree dirTree(path);
		for(unsigned int a=0;a<dirTree.GetFileCount();a++)
		{
			char delPath[2048];
			sprintf(delPath,"%s/%s",dirTree.GetPath(),dirTree.GetFileName(a));
			LGL_FileDelete(delPath);
		}
	}

	printf("LGL_Shutdown(): Omega\n");
}

