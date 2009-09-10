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

#include <stdlib.h>
#include <math.h>
#include <time.h>	//Purely for random# seeding
#include <string.h>
#include <ctype.h>	//isapha(), ispunct(), etc

#include <SDL_image.h>
#include <SDL_net.h>
#include <SDL_endian.h>

#include <stdlib.h>		//malloc()
#include <unistd.h>		//read(), LGL_Memory*
#include <sys/types.h>
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
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
#include <mach/thread_act.h>
#endif	//LGL_OSX

#define	LGL_PRIORITY_AUDIO_OUT		(1.0f)
#define	LGL_PRIORITY_MAIN		(0.9f)
#define	LGL_PRIORITY_VIDEO_DECODE	(0.8f)
#define	LGL_PRIORITY_AUDIO_DECODE	(0.7f)
#define	LGL_PRIORITY_DISKWRITER		(-0.1f)

#define LGL_EQ_SAMPLES_FFT	(512)
#define LGL_SAMPLESIZE		(256)
unsigned int LGL_SAMPLESIZE_SDL;
const int LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE = (AVCODEC_MAX_AUDIO_FRAME_SIZE*2);

void lgl_AudioOutCallback(void* userdata, Uint8* stream, int len8);

#ifdef	LGL_LINUX
void lgl_AudioInCallback(void *udata, Uint8 *stream, int len8);
#endif	//LGL_LINUX

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

	float			VU;

	bool			Loop;
	bool			StickyEndpoints;
	double			LengthSamples;
	unsigned long 		BufferLength;
	Uint8*			Buffer;
	LGL_Semaphore*		BufferSemaphore;
	LGL_Sound*		LGLSound;
	LGL_AudioDSP*		LGLAudioDSPFront;

	double			DivergeSamples;
	int			DivergeState;
	float			DivergeSpeed;
	double			WarpPointSecondsTrigger;
	double			WarpPointSecondsDestination;

	SRC_STATE*		SampleRateConverterL;
	SRC_STATE*		SampleRateConverterR;
	float			SampleRateConverterBufferL[SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES];
	float			SampleRateConverterBufferR[SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES];
	float			SampleRateConverterBufferSpeed;
	long			SampleRateConverterBufferStartSamples;
	int			SampleRateConverterBufferValidSamples;
	long			SampleRateConverterBufferConsumedSamples;
	int			SampleRateConverterBufferCurrentSamplesIndex;
} LGL_SoundChannel;

#define	LGL_SOUND_CHANNEL_NUMBER	32

typedef struct
{
	//Video
	
	int			ScreenCount;
	int			ScreenNow;
	int			ScreenResolutionX[LGL_SCREEN_MAX];
	int			ScreenResolutionY[LGL_SCREEN_MAX];
	int			VideoResolutionX;
	int			VideoResolutionY;
	bool			VideoFullscreen;

	SDL_WindowID		WindowID;
	SDL_GLContext		GLContext;
	//SDL_Surface*		GLVideoSurface;

	float			ScreenViewPortLeft[LGL_SCREEN_MAX];
	float			ScreenViewPortRight[LGL_SCREEN_MAX];
	float			ScreenViewPortBottom[LGL_SCREEN_MAX];
	float			ScreenViewPortTop[LGL_SCREEN_MAX];

	float			ViewPortLeft;
	float			ViewPortRight;
	float			ViewPortBottom;
	float			ViewPortTop;

	float			ViewPortEyeX;
	float			ViewPortEyeY;
	float			ViewPortEyeZ;
	float			ViewPortTargetX;
	float			ViewPortTargetY;
	float			ViewPortTargetZ;
	float			ViewPortUpX;
	float			ViewPortUpY;
	float			ViewPortUpZ;

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

	//Audio

	bool			AudioAvailable;
	bool			AudioWasOnceAvailable;
	bool			AudioQuitting;

	bool			AudioUsingJack;

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
	bool			RecordActive;
	char			RecordFilePath[1024];
	char			RecordFilePathShort[1024];
	FILE*			RecordFileDescriptor;
	unsigned long		RecordSamplesWritten;
	Uint8			RecordBuffer[LGL_SAMPLESIZE*4*2];
	float			RecordVolume;
	std::vector<LGL_AudioStream*>
				AudioStreamList;
	LGL_Semaphore*		AudioStreamListSemaphore;
	LGL_Semaphore*		AVCodecSemaphore;
	LGL_Semaphore*		AVOpenCloseSemaphore;
	bool			AudioMasterToHeadphones;

	//AudioIn

	bool			AudioInAvailable;
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
	int			FPSCounter;
	LGL_Timer		FPSTimer;
	float			FPSGraph[60];
	LGL_Timer		FPSGraphTimer;
	LGL_Font*		Font;
	char			TimeOfDay[1024];
	char			DateAndTimeOfDay[1024];
	char			DateAndTimeOfDayOfExecution[1024];

	//Net

	bool			NetAvailable;

	//Misc

	bool			Running;
	
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

	float			LastImageDrawAsLineLeftX;
	float			LastImageDrawAsLineLeftY;
	float			LastImageDrawAsLineRightX;
	float			LastImageDrawAsLineRightY;

	int			TexturePixels;
	bool			FrameBufferTextureGlitchFix;

	char			HomeDir[2048];
	char			Username[2048];
} LGL_State;

LGL_State LGL;

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
	/*
	double delayedUsecs=jack_get_xrun_delayed_usecs(jack_client);
	if(delayedUsecs>0)
	{
		printf("Jack xrun... %.5fms\n",delayedUsecs/1000.0f);
	}
	*/
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
		jack_default_audio_sample_t* out_fl = (jack_default_audio_sample_t*)jack_port_get_buffer(jack_output_port_fl,nframes);
		jack_default_audio_sample_t* out_fr = (jack_default_audio_sample_t*)jack_port_get_buffer(jack_output_port_fr,nframes);
		assert(out_fl);
		assert(out_fr);
		for(unsigned int a=0;a<nframes;a++)
		{
			jack_input_buffer16[a*2+0] = (Sint16)(in_l[a]*((1<<16)-1));
			jack_input_buffer16[a*2+1] = (Sint16)(in_r[a]*((1<<16)-1));
/*			
			out_fl[a]+=in_l[a];
			out_fr[a]+=in_r[a];
*/
		}
		lgl_AudioInCallback(NULL,jack_input_buffer8,nframes*2*2);
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
	LGL.SoundChannel[a].FuturePositionSamplesPrev=-1;
	LGL.SoundChannel[a].FuturePositionSamplesNow=-1;
	LGL.SoundChannel[a].PositionSamplesNowLastReported=0;
	LGL.SoundChannel[a].PositionSamplesEnd=-1;
	LGL.SoundChannel[a].PositionSamplesDeltaLastTime.Reset();
	LGL.SoundChannel[a].DivergeSamples=0;
	LGL.SoundChannel[a].DivergeState=0;
	LGL.SoundChannel[a].DivergeSpeed=1.0f;
	LGL.SoundChannel[a].WarpPointSecondsTrigger=-1.0f;
	LGL.SoundChannel[a].WarpPointSecondsDestination=-1.0f;
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
	if(LGL.SoundChannel[a].LGLAudioDSPFront!=NULL)
	{
		delete LGL.SoundChannel[a].LGLAudioDSPFront;
		LGL.SoundChannel[a].LGLAudioDSPFront=NULL;
	}
	LGL.SoundChannel[a].Occupied=false;
}

double myH[1024];
double myZ[1024];
int myPstate;

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

bool
LGL_JackInit()
{
	printf("\n\nLGL JACK Initialization\n");
	printf("---\n");
	LGL.AudioSpec->silence=0;
	jack_options_t jack_options = JackNullOption;
	jack_status_t status;

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
		printf("LGL_JackInit(): Error! jack_client_open() failed! status = 0x%2.0x\n",status);
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
	const char** jack_ports_out = jack_get_ports(jack_client, NULL, NULL, JackPortIsPhysical|JackPortIsOutput);
	int portCountOut=0;
	if(jack_ports_out)
	{
		while(jack_ports_out[portCountOut]!=NULL)
		{
			printf("Out found: '%s'\n",jack_ports_out[portCountOut]);
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
	LGL.AudioSpec->freq = jack_get_sample_rate(jack_client);

	//MEMLEAK: if we return early, these calls to free() don't happen. Meh.
	free(jack_ports_in);
	free(jack_ports_out);

	printf("LGL_JackInit(): Success! (%i channels)\n",LGL.AudioSpec->channels);
	printf("---\n\n");

	return(true);
}

int lgl_MidiInit2();

bool
LGL_Init
(
	int		inVideoResolutionX,
	int		inVideoResolutionY,
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
	
	LGL_ViewPortScreen(0,1,0,1);
	LGL_ViewPortWorld
	(
		1,1,1,
		0,0,0,
		0,0,1
	);

	LGL.AudioAvailable=false;
	LGL.AudioWasOnceAvailable=false;
	LGL.AudioQuitting=false;
	LGL.AudioUsingJack=true;

	int a;
	for(a=0;a<512;a++)
	{
		LGL.KeyDown[a]=false;
		LGL.KeyStroke[a]=false;
		LGL.KeyRelease[a]=false;
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
	}

	LGL.RecordMovie=false;
	LGL.RecordMovieFPS=60;
	sprintf(LGL.RecordMovieFileNamePrefix,"./");
	LGL.RecordMovieSecondsSinceExecutionInitial=0;

	LGL.DrawLogFD=0;
	LGL.DrawLogPause=false;
	LGL.DrawLogFileName[0]='\0';
	LGL.DrawLogTimeOfNextDrawnFrame=0.0f;

	LGL.AudioSpec=(SDL_AudioSpec*)malloc(sizeof(SDL_AudioSpec));

	atexit(LGL_ShutDown);

	//Initialize SDL
	if
	(
		SDL_Init
		(
#ifndef	LGL_NO_GRAPHICS
			SDL_INIT_VIDEO |
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
		return(false);
	}
	else
	{
		//printf("LGL: SDL_Init() Success!\n");
	}

#ifndef	LGL_NO_GRAPHICS
	LGL.ScreenCount=SDL_GetNumVideoDisplays();
	LGL.ScreenNow=0;

printf("%i screens!\n",LGL.ScreenCount);
	
	for(int a=0;a<LGL.ScreenCount;a++)
	{
		SDL_SelectVideoDisplay(a);
		SDL_DisplayMode mode;
		SDL_GetDesktopDisplayMode(&mode);

		LGL.ScreenResolutionX[a] = mode.w;
		LGL.ScreenResolutionY[a] = mode.h;

printf("\tScreen[%i]: %i x %i\n",a,
	LGL.ScreenResolutionX[a],
	LGL.ScreenResolutionY[a]
);
		SDL_DisplayMode displayMode;
		SDL_GetFullscreenDisplayMode(&displayMode);
		displayMode.w=LGL.ScreenResolutionX[a];
		displayMode.h=LGL.ScreenResolutionY[a];
		displayMode.refresh_rate=60;
		SDL_SetFullscreenDisplayMode(&displayMode);
	}

	SDL_SelectVideoDisplay(0);

	if
	(
		inVideoResolutionX==9999 &&
		inVideoResolutionY==9999
	)
	{
		LGL.VideoFullscreen=true;
		LGL.VideoResolutionX=0;
		LGL.VideoResolutionY=0;
		for(int a=0;a<LGL.ScreenCount;a++)
		{
			LGL.VideoResolutionX+=LGL.ScreenResolutionX[a];
			LGL.VideoResolutionY=LGL_Max
			(
				LGL.VideoResolutionY,
				LGL.ScreenResolutionY[a]
			);
		}
	}
	else
	{
		LGL.VideoFullscreen=false;
		LGL.VideoResolutionX=inVideoResolutionX;
		LGL.VideoResolutionY=inVideoResolutionY;
	}

	for(int a=0;a<LGL.ScreenCount;a++)
	{
		LGL.ScreenViewPortLeft[a]=0;
		for(int b=0;b<a;b++)
		{
			LGL.ScreenViewPortLeft[a]+=LGL.VideoFullscreen ? LGL.ScreenResolutionX[b] : LGL.VideoResolutionX;
		}
		LGL.ScreenViewPortRight[a]=LGL.ScreenViewPortLeft[a]+(LGL.VideoFullscreen ? LGL.ScreenResolutionX[a] : LGL.VideoResolutionX);
		LGL.ScreenViewPortLeft[a]/=LGL.VideoResolutionX;
		LGL.ScreenViewPortRight[a]/=LGL.VideoResolutionX;

#ifdef	LGL_OSX
		if
		(
			a==0 &&
			LGL.VideoFullscreen &&
			LGL.ScreenCount > 1
		)
		{
			LGL.ScreenResolutionY[a]-=70;	//OSX Dock
		}
#endif	//LGL_OSX
		LGL.ScreenViewPortBottom[a]=(LGL.VideoResolutionY-LGL.ScreenResolutionY[a])/(float)LGL.VideoResolutionY;
		LGL.ScreenViewPortTop[a]=1.0f;
	}

	if(LGL.VideoFullscreen==false)
	{
		LGL.ScreenViewPortLeft[0]=0.0f;
		LGL.ScreenViewPortRight[0]=1.0f;
		LGL.ScreenViewPortBottom[0]=0.0f;
		LGL.ScreenViewPortTop[0]=1.0f;
	}

#endif	//LGL_NO_GRAPHICS
	
	//Time

	LGL.FPS=0;
	LGL.FPSTimer.Reset();
	for(int a=0;a<60;a++)
	{
		LGL.FPSGraph[a]=0;
	}
	LGL.FPSGraphTimer.Reset();
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
	
	LGL.AudioSpec->freq=44100;
	LGL.AudioSpec->format=AUDIO_S16;
	LGL.AudioSpec->channels=inAudioChannels;
	LGL.AudioSpec->samples=LGL_SAMPLESIZE_SDL;
	LGL.AudioSpec->callback=lgl_AudioOutCallback;
	LGL.AudioSpec->userdata=malloc(LGL.AudioSpec->samples);
	LGL.RecordActive=false;
	LGL.RecordFileDescriptor=NULL;
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
	LGL.AudioBufferPos=0;

	for(int a=0;a<LGL_SOUND_CHANNEL_NUMBER;a++)
	{
		LGL.SoundChannel[a].SampleRateConverterL = NULL;
		LGL.SoundChannel[a].SampleRateConverterR = NULL;
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

		if(LGL.AudioUsingJack==false)
		{
			LGL_ThreadSetPriority(LGL_PRIORITY_AUDIO_OUT,"AudioOut / JACK");
		}
		if
		(
			pulserunning==false &&
			LGL_JackInit()
		)
		{
			//Huzzah!
		}
		else
		{
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

	sprintf(LGL.RecordFilePath,"%s/.dvj/record/%s.mp3",LGL_GetHomeDir(),LGL_DateAndTimeOfDayOfExecution());
	sprintf(LGL.RecordFilePathShort,"~/.dvj/record/%s.mp3",LGL_DateAndTimeOfDayOfExecution());

	if(LGL.AudioAvailable)
	{
		const char* diskWriterPath = "./diskWriter";
#ifdef	LGL_LINUX
		diskWriterPath = "./diskWriter.lin";
#endif	//LGL_LINUX
#ifdef	LGL_OSX
		diskWriterPath = "./diskWriter.osx";
#endif	//LGL_OSX
		if(LGL_FileExists(diskWriterPath))
		{
			char command[1024];
			sprintf(command,"%s \"%s\" --lame --freq %i",diskWriterPath,LGL.RecordFilePath,LGL.AudioSpec->freq);
			LGL_ThreadSetPriority(LGL_PRIORITY_DISKWRITER,"DiskWriter");
			LGL.RecordFileDescriptor=popen(command,"w");
			LGL_ThreadSetPriority(LGL_PRIORITY_MAIN,"Main");
		}
	}

	SDL_WM_SetCaption(inWindowTitle,inWindowTitle);
	SDL_EnableUNICODE(1);

	//Initialize Video

#ifndef	LGL_NO_GRAPHICS
	SDL_SelectVideoDisplay(0);

	int windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
	if(LGL.VideoFullscreen)
	{
		if(LGL.ScreenCount>1)
		{
			windowFlags |= SDL_WINDOW_BORDERLESS;
		}
		else
		{
			windowFlags |= SDL_WINDOW_FULLSCREEN;
			windowFlags |= SDL_WINDOW_BORDERLESS;
		}
	}
	LGL.WindowID = SDL_CreateWindow
	(
		inWindowTitle,
		LGL.VideoFullscreen ? 0 : SDL_WINDOWPOS_CENTERED,
		LGL.VideoFullscreen ? 0 : SDL_WINDOWPOS_CENTERED,
		LGL.VideoResolutionX,
		LGL.VideoResolutionY,
		windowFlags
	);

	if(LGL.WindowID==0)
	{
		printf
		(
			"LGL_Init(): SDL_SetVideoMode \
			(%i,%i) failed... %s\n",
			LGL.VideoResolutionX,
			LGL.VideoResolutionY,
			SDL_GetError()
		);
		return(false);
	}
	else
	{
		LGL.GLContext = SDL_GL_CreateContext(LGL.WindowID);
		SDL_GL_MakeCurrent(LGL.WindowID, LGL.GLContext);
		/*
		LGL.GLVideoSurface =
			SDL_CreateRGBSurfaceFrom
			(
				NULL,
				LGL.VideoResolutionX,
				LGL.VideoResolutionY,
				32,
				0, 0, 0, 0, 0
			);
		int surface_flags=SDL_OPENGL;
		if(LGL.VideoFullscreen)
		{
			surface_flags |= SDL_FULLSCREEN;
		}
		LGL.GLVideoSurface->flags |= surface_flags;
		*/
	}

	//GL Settings

	SDL_GL_SetSwapInterval((LGL.VideoFullscreen && LGL.ScreenCount>1) ? 0 : 1);//1);	//VSYNC

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

	LGL_SetActiveScreen(0);

	LGL.GPUSpeed=0;
	LGL.GPUTemp=0;

	LGL.ShaderProgramCurrent=0;
	
	LGL.StatsLinesPerFrame=0;
	LGL.StatsRectsPerFrame=0;
	LGL.StatsImagesPerFrame=0;
	LGL.StatsPixelsPerFrame=0;

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
#endif	//LGL_NO_GRAPHICS

	//Video Decoding via libavcodec
	av_register_all();
	avcodec_init(); 
	avcodec_register_all();
	av_log_set_level(AV_LOG_WARNING);	//QUIET, ERROR, WARNING, DEBUG

	//bool audioIn=inAudioChannels < 0;
	inAudioChannels=abs(inAudioChannels);

	//LGL_Assert(inAudioChannels==0 || inAudioChannels==2 || inAudioChannels==4);

	LGL.AVCodecSemaphore=new LGL_Semaphore("AV Codec");
	LGL.AVOpenCloseSemaphore=new LGL_Semaphore("AV Codec Open/Close");

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
	LGL.VidCamImageRaw=new LGL_Image
	(
		LGL.VidCamWidthNow,LGL.VidCamHeightNow,
		3,
		LGL.VidCamBufferRaw
	);
	LGL.VidCamImageProcessed=new LGL_Image
	(
		LGL.VidCamWidthNow,LGL.VidCamHeightNow,
		3,
		LGL.VidCamBufferProcessed
	);
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
			LGL_MouseVisible(false);
			LGL_VidCamCalibrate(NULL,.5);
			LGL_MouseVisible(true);
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
			"JACK\t\t\t%i channels, %ihz. %.1fms latency.\n",// RT Priority: %i\n",
			LGL.AudioSpec->channels,
			jack_get_sample_rate(jack_client),
			1000.0f*LGL_SAMPLESIZE_SDL/(44100.0f)
			//jack_client_real_time_priority(jack_client)
		);
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
	printf("OpenGL Renderer\t\t%s\n",(char*)(glGetString(GL_RENDERER)));
	printf("OpenGL VBO\t\t%s\n",LGL_VBOAvailable()?"Present":"Absent");
	printf("GLSL Vert Shader\t%s\n",LGL_ShaderAvailableVert()?"Present":"Absent");
	printf("GLSL Frag Shader\t%s\n",LGL_ShaderAvailableFrag()?"Present":"Absent");
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

	return(true);
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

double
LGL_Timer::
SecondsSinceLastReset() const
{
	double ret=((double)SDL_GetTicks()-(double)TimeAtLastReset)/1000.0;
	if(ret<0) ret=0;
	return(ret);
}

void
LGL_Timer::
Reset()
{
	TimeAtLastReset=SDL_GetTicks();
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
	float&	freqFactor
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
		freqFactor
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

	if(wisdomLoaded==false)
	{
		LGL_Image* fft_img=NULL;
		const char* fftwWisdomImagePath="data/image/fftw_wisdom.png";
		if(LGL_FileExists(fftwWisdomImagePath))
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

float*	tempAudioBufferBackSilence=NULL;
int	lgl_MidiUpdate();	//Internal LGL function

#define	FONT_BUFFER_SIZE (1024*1024)	//4MB for fonts... that sould be more than enough, right?
float lgl_font_gl_buffer[FONT_BUFFER_SIZE];
float* lgl_font_gl_buffer_ptr;

int lgl_font_gl_buffer_int[FONT_BUFFER_SIZE];
int* lgl_font_gl_buffer_int_ptr;

void
LGL_SwapBuffers()
{
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

		if(tempAudioBufferBackSilence==NULL)
		{
			tempAudioBufferBackSilence=new float[1024];
			for(int a=0;a<1024;a++)
			{
				tempAudioBufferBackSilence[a]=0.5f;
			}
		}

		memcpy
		(
			LGL.AudioBufferLBack,
			tempAudioBufferBackSilence,
			sizeof(LGL.AudioBufferLBack)
		);
		memcpy
		(
			LGL.AudioBufferRBack,
			tempAudioBufferBackSilence,
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
			LGL.AudioStreamListSemaphore->Lock("Main","Calling Update() on list");
			for(unsigned int a=0;a<LGL.AudioStreamList.size();a++)
			{
				LGL.AudioStreamList[a]->Update();
			}
			LGL.AudioStreamListSemaphore->Unlock();
		}
	}

	if(LGL.AudioInAvailable)
	{
		if(LGL.AudioInGrainListFront.empty()==false)
		{
			LGL.AudioInFallbackGrain->SetWaveformFromAudioGrain
			(
				LGL.AudioInGrainListFront[LGL.AudioInGrainListFront.size()-1]
			);
		}
		std::vector<LGL_AudioGrain*> oldFront = LGL.AudioInGrainListFront;
		//FIXME: We're blocking our audio thread!! (ever so briefly...)
		LGL.AudioInSemaphore->Lock("Main","Moving back grains to front");
		{
			LGL.AudioInGrainListFront=LGL.AudioInGrainListBack;
			LGL.AudioInGrainListBack.clear();
		}
		LGL.AudioInSemaphore->Unlock();

		std::vector<LGL_AudioGrain*> neoFixedSize;
		const unsigned int fixedSize=4;
		for(int a=(int)LGL.AudioInGrainListFront.size()-1;a>=0;a--)
		{
			if(neoFixedSize.size()<fixedSize)
			{
				neoFixedSize.push_back(LGL.AudioInGrainListFront[a]);
			}
			else
			{
				break;
			}
		}
		for(int a=(int)LGL.AudioInGrainListFixedSize.size()-1;a>=0;a--)
		{
			if(neoFixedSize.size()<fixedSize)
			{
				neoFixedSize.push_back(LGL.AudioInGrainListFixedSize[a]);
			}
			else
			{
				break;
			}
		}
		for(unsigned int a=0;a<oldFront.size();a++)
		{
			bool stillAlive=false;
			for(int b=0;b<(int)neoFixedSize.size();b++)
			{
				if(oldFront[a]==neoFixedSize[b])
				{
					stillAlive=true;
					break;
				}
			}
			if(stillAlive==false)
			{
				delete oldFront[a];
				for(unsigned int c=0;c<LGL.AudioInGrainListFixedSize.size();c++)
				{
					if(LGL.AudioInGrainListFixedSize[c]==oldFront[a])
					{
						LGL.AudioInGrainListFixedSize[c]=NULL;
					}
				}
			}
		}
		oldFront.clear();

		for(unsigned int a=0;a<LGL.AudioInGrainListFixedSize.size();a++)
		{
			bool stillAlive=false;
			for(int b=0;b<(int)neoFixedSize.size();b++)
			{
				if(LGL.AudioInGrainListFixedSize[a]==neoFixedSize[b])
				{
					stillAlive=true;
					break;
				}
			}
			if(stillAlive==false)
			{
				if(LGL.AudioInGrainListFixedSize[a])
				{
					delete LGL.AudioInGrainListFixedSize[a];
				}
			}
		}
		LGL.AudioInGrainListFixedSize.clear();
		for(int b=(int)neoFixedSize.size()-1;b>=0;b--)
		{
			LGL.AudioInGrainListFixedSize.push_back(neoFixedSize[b]);
		}
		neoFixedSize.clear();
	}

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

#ifdef	LGL_NO_GRAPHICS
	LGL_DelaySeconds(1.0f/60.0f-LGL_SecondsSinceThisFrame());
#else
	//SDL_GL_SwapBuffers();
	LGL_DelayMS(1);
	SDL_GL_SwapWindow(LGL.WindowID);
#endif	//LGL_NO_GRAPHICS

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

	LGL_ClearBackBuffer();

	if(LGL.RecordMovie)
	{
		LGL.FPS=LGL.RecordMovieFPS;
		LGL.FPSCounter=0;
		LGL.FPSTimer.Reset();

		LGL.SecondsSinceLastFrame=1.0/LGL.RecordMovieFPS;
		LGL.SecondsSinceLastFrameTimer.Reset();
	}
	else
	{
		LGL.FPSCounter++;
		if(LGL.FPSTimer.SecondsSinceLastReset()>=1)
		{
			LGL.FPS=LGL.FPSCounter;
			LGL.FPSCounter=0;
			LGL.FPSTimer.Reset();
		}

		LGL.SecondsSinceLastFrame=LGL.SecondsSinceLastFrameTimer.SecondsSinceLastReset();
		LGL.SecondsSinceLastFrameTimer.Reset();
	}
	LGL.SecondsSinceExecution=LGL.SecondsSinceExecutionTimer.SecondsSinceLastReset();
	LGL.FramesSinceExecution++;
	if(LGL.FPSGraphTimer.SecondsSinceLastReset()>=1.0/60.0)
	{
		for(int a=0;a<59;a++)
		{
			LGL.FPSGraph[a]=LGL.FPSGraph[a+1];
		}
		LGL.FPSGraph[59]=1.0/LGL.SecondsSinceLastFrame;
		LGL.FPSGraphTimer.Reset();
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
	LGL.VideoFullscreen=!LGL.VideoFullscreen;
}

void
LGL_FullScreen
(
	bool	inFullScreen
)
{
	if(inFullScreen!=LGL.VideoFullscreen)
	{
		LGL.VideoFullscreen=!LGL.VideoFullscreen;
		LGL_FullScreenToggle();
	}
}

bool
LGL_IsFullScreen()
{
	return(LGL.VideoFullscreen);
}

int
LGL_VideoResolutionX()
{
	return(LGL.VideoResolutionX);
}

int
LGL_VideoResolutionY()
{
	return(LGL.VideoResolutionY);
}

int
LGL_ScreenCount()
{
	return(LGL.ScreenCount);
}

int
LGL_ScreenResolutionX(int which)
{
	return(LGL.ScreenResolutionX[which]);
}

int
LGL_ScreenResolutionY(int which)
{
	return(LGL.ScreenResolutionY[which]);
}

float
LGL_VideoAspectRatio()
{
	return
	(
		LGL.VideoResolutionX/(float)
		LGL.VideoResolutionY
	);
}

float
LGL_ScreenAspectRatio()
{
	return
	(
		LGL.ScreenResolutionX[LGL.ScreenNow]/(float)
		LGL.ScreenResolutionY[LGL.ScreenNow]
	);
}

int
LGL_GetActiveScreen()
{
	return(LGL.ScreenNow);
}

void
LGL_SetActiveScreen(int screen)
{
	screen=(int)(LGL_Clamp(0,screen,LGL.ScreenCount-1));
	LGL.ScreenNow=screen;

	float left=	(0.0f-LGL.ScreenViewPortLeft[screen])*(1.0f/(LGL.ScreenViewPortRight[screen]-LGL.ScreenViewPortLeft[screen]));
	float right=	(1.0f-LGL.ScreenViewPortLeft[screen])*(1.0f/(LGL.ScreenViewPortRight[screen]-LGL.ScreenViewPortLeft[screen]));
	float bottom=	(0.0f-LGL.ScreenViewPortBottom[screen])*(1.0f/(LGL.ScreenViewPortTop[screen]-LGL.ScreenViewPortBottom[screen]));
	float top=	(1.0f-LGL.ScreenViewPortBottom[screen])*(1.0f/(LGL.ScreenViewPortTop[screen]-LGL.ScreenViewPortBottom[screen]));

	LGL_ViewPortScreen
	(
		left,
		right,
		bottom,
		top
	);
}

inline
void
lgl_glScreenify2D()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glOrtho(LGL.ViewPortLeft,LGL.ViewPortRight,LGL.ViewPortBottom,LGL.ViewPortTop,0,1);
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
		LGL.ViewPortEyeX,LGL.ViewPortEyeY,LGL.ViewPortEyeZ,
		LGL.ViewPortTargetX,LGL.ViewPortTargetY,LGL.ViewPortTargetZ,
		LGL.ViewPortUpX,LGL.ViewPortUpY,LGL.ViewPortUpZ
	);
	*/
	printf("lgl_glScreenify3D(): Warning! Function not yet implemented in linux...\n");
#else	//LGL_LINUX
//FIXME: Windows port canot use lgl_glScreenify3D(), due to a libglu32.a mingw32 linker error
	printf("lgl_glScreenify3D(): Warning! Function not yet implemented in win32...\n");
#endif	//LGL_LINUX
}

void
LGL_ViewPortScreen
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
			"LGL_ViewPortScreen|%.3f|%.3f|%.3f|%.3f\n",
			left,
			right,
			bottom,
			top
		);
	}
	LGL.ViewPortLeft=left;
	LGL.ViewPortRight=right;
	LGL.ViewPortBottom=bottom;
	LGL.ViewPortTop=top;
}

void
LGL_GetViewPortScreen
(
	float&	left,
	float&	right,
	float&	bottom,
	float&	top
)
{
	left=LGL.ViewPortLeft;
	right=LGL.ViewPortRight;
	bottom=LGL.ViewPortBottom;
	top=LGL.ViewPortTop;
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

	int sLeft = LGL.VideoResolutionX*(LGL.ScreenViewPortLeft[LGL.ScreenNow] + left *
		(
			LGL.ScreenViewPortRight[LGL.ScreenNow] -
			LGL.ScreenViewPortLeft[LGL.ScreenNow]
		));
	int sRight = LGL.VideoResolutionX*(LGL.ScreenViewPortLeft[LGL.ScreenNow] + right *
		(
			LGL.ScreenViewPortRight[LGL.ScreenNow] -
			LGL.ScreenViewPortLeft[LGL.ScreenNow]
		));
	int sBottom = LGL.VideoResolutionY*(LGL.ScreenViewPortBottom[LGL.ScreenNow] + bottom *
		(
			LGL.ScreenViewPortTop[LGL.ScreenNow] -
			LGL.ScreenViewPortBottom[LGL.ScreenNow]
		));
	int sTop = LGL.VideoResolutionY*(LGL.ScreenViewPortBottom[LGL.ScreenNow] + top *
		(
			LGL.ScreenViewPortTop[LGL.ScreenNow] -
			LGL.ScreenViewPortBottom[LGL.ScreenNow]
		));

	glEnable(GL_SCISSOR_TEST);
	if(LGL.VideoFullscreen)
	{
		glScissor
		(
			sLeft,
			sBottom,
			sRight,
			sTop
		);
	}
	else
	{
		glScissor
		(
			(int)(left*LGL.VideoResolutionX),
			(int)(bottom*LGL.VideoResolutionY),
			(int)((right-left)*LGL.VideoResolutionX),
			(int)((top-bottom)*LGL.VideoResolutionY)
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
LGL_ViewPortWorld
(
	float EyeX, float EyeY, float EyeZ,
	float TargetX, float TargetY, float TargetZ,
	float UpX, float UpY, float UpZ
)
{
	LGL.ViewPortEyeX=EyeX;
	LGL.ViewPortEyeY=EyeY;
	LGL.ViewPortEyeZ=EyeZ;
	
	LGL.ViewPortTargetX=TargetX;
	LGL.ViewPortTargetY=TargetY;
	LGL.ViewPortTargetZ=TargetZ;
	
	LGL.ViewPortUpX=UpX;
	LGL.ViewPortUpY=UpY;
	LGL.ViewPortUpZ=UpZ;
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

LGL_ShaderObj::
LGL_ShaderObj
(
	char* inDescription
)
{
	SetDescription(inDescription);
	VertObject=0;
	FragObject=0;
	ProgramObject=0;
}

LGL_ShaderObj::
~LGL_ShaderObj()
{
	OmniDelete();
}

char*
LGL_ShaderObj::
GetDescription()
{
	return(Description);
}

void
LGL_ShaderObj::
SetDescription
(
	char*	inDescription
)
{
	sprintf(Description,"%s",inDescription);
}

bool
LGL_ShaderObj::
VertCompile
(
	char*	inFileVert
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
			"LGL_ShaderObj('%s')::VertCompile(): Error! Couldn't open file '%s'...\n",
			Description,
			inFileVert
		);
		exit(-1);
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
			"LGL_ShaderObj('%s')::VertCompile(): Error! '%s' failed to compile:\n\n",
			Description,
			inFileVert
		);
		
		printf("---\n\n");
		InfoLogPrint(VertObject);
		printf("---\n");

		exit(-1);
	}
	
	fclose(file);
	return(true);
}

bool
LGL_ShaderObj::
FragCompile
(
	char*	inFileFrag
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
			"LGL_ShaderObj('%s')::FragCompile(): Error! Couldn't open file '%s'...\n",
			Description,
			inFileFrag
		);
		exit(-1);
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
			"LGL_ShaderObj('%s')::FragCompile(): Error! '%s' failed to compile:\n\n",
			Description,
			inFileFrag
		);
		
		printf("---\n\n");
		InfoLogPrint(FragObject);
		printf("---\n");

		exit(-1);
	}

	fclose(file);
	return(true);
}

bool
LGL_ShaderObj::
VertCompiled()
{
	return(VertObject!=0);
}

bool
LGL_ShaderObj::
FragCompiled()
{
	return(FragObject!=0);
}

bool
LGL_ShaderObj::
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
			"LGL_ShaderObj('%s')::Link(): Error! Could link program... Source files:\n",
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
		exit(-1);
	}
	return(true);
}

bool
LGL_ShaderObj::
Enable
(
	bool	enable
)
{
	if
	(
		strstr
		(
			(char*)glGetString(GL_EXTENSIONS),
			"GL_ARB_SHADING_LANGUAGE_100"
		)==NULL
	)
	{
		printf("LGL_ShaderObj::Enable(): Warning! Your graphics driver compiles but cannot run GLSL programs!\n");
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
LGL_ShaderObj::
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
LGL_ShaderObj::
IsEnabled()
{
	return
	(
		ProgramObject!=0 &&
		ProgramObject==LGL.ShaderProgramCurrent
	);
}

bool
LGL_ShaderObj::
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
LGL_ShaderObj::
SetVertAttributeInt
(
	char*	name,
	int	value0
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
LGL_ShaderObj::
SetVertAttributeInt
(
	char*	name,
	int	value0,	int	value1
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
LGL_ShaderObj::
SetVertAttributeInt
(
	char*	name,
	int	value0,	int	value1,	int	value2
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
LGL_ShaderObj::
SetVertAttributeInt
(
	char*	name,
	int	value0,	int	value1,	int	value2,	int	value3
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
LGL_ShaderObj::
SetVertAttributeFloat
(
	char*	name,
	float	value0
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
LGL_ShaderObj::
SetVertAttributeFloat
(
	char*	name,
	float	value0,	float	value1
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
LGL_ShaderObj::
SetVertAttributeFloat
(
	char*	name,
	float	value0,	float	value1,	float	value2
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
LGL_ShaderObj::
SetVertAttributeFloat
(
	char*	name,
	float	value0,	float	value1,	float	value2,	float	value3
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
LGL_ShaderObj::
SetUniformAttributeInt
(
	char*	name,
	int	value0
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
LGL_ShaderObj::
SetUniformAttributeInt
(
	char*	name,
	int	value0,	int	value1
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
LGL_ShaderObj::
SetUniformAttributeInt
(
	char*	name,
	int	value0,	int	value1,	int	value2
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
LGL_ShaderObj::
SetUniformAttributeInt
(
	char*	name,
	int	value0,	int	value1,	int	value2,	int	value3
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
LGL_ShaderObj::
SetUniformAttributeFloat
(
	char*	name,
	float	value0
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
LGL_ShaderObj::
SetUniformAttributeFloat
(
	char*	name,
	float	value0,	float	value1
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
LGL_ShaderObj::
SetUniformAttributeFloat
(
	char*	name,
	float	value0,	float	value1,	float	value2
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
LGL_ShaderObj::
SetUniformAttributeFloat
(
	char*	name,
	float	value0,	float	value1,	float	value2,	float	value3
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
LGL_ShaderObj::
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
LGL_ShaderObj::
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
LGL_ShaderObj::
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
LGL_ShaderObj::
OmniDelete()
{
	ProgramDelete();
	FragDelete();
	VertDelete();
}

bool
LGL_ShaderObj::
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
LGL_ShaderObj::
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
			"LGL_ShaderObj('%s')::InfoLogPrint(): Error! Couldn't read log (expected %i, got %i)!\n",
			Description,
			(int)length,
			(int)written
		);
		exit(-1);
	}

	printf("%s",log);

	free(log);

	return(true);
}

bool
LGL_ShaderObj::
SetVertAttributeIntPrivate
(
	char*	name,	int	num,
	int	value0,	int	value1,
	int	value2,	int	value3
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
			"LGL_ShaderObj('%s')::SetVertAttributeInt(): Error! "
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
		exit(-1);
	}

	gl2VertexAttrib4iv(index,arglist);
	return(true);
}

bool
LGL_ShaderObj::
SetVertAttributeFloatPrivate
(
	char*	name,	int	num,
	float	value0,	float	value1,
	float	value2,	float	value3
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
			"LGL_ShaderObj('%s')::SetVertAttributeFloat(): Error! "
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
		exit(-1);
	}

	gl2VertexAttrib4fv(index,arglist);
	return(true);
}

bool
LGL_ShaderObj::
SetUniformAttributeIntPrivate
(
	char*	name,		int	num,
	int	value0,		int	value1,
	int	value2,		int	value3
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
			"LGL_ShaderObj('%s')::SetUniformAttributeInt(): Error! "
			"Could not resolve uniform variable '%s'...\n",
			Description,
			name
		);
		exit(-1);
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
			"LGL_ShaderObj('%s')::SetUniformAttributeInt(): Error! "
			"Num is '%i', but must be [1,4]...\n",
			Description,
			num
		);
		exit(-1);
	}

	return(true);
}

bool
LGL_ShaderObj::
SetUniformAttributeFloatPrivate
(
	char*	name,		int	num,
	float	value0,		float	value1,
	float	value2,		float	value3
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
			"LGL_ShaderObj('%s')::SetUniformAttributeFloat(): Error! "
			"Could not resolve uniform variable '%s'...\n",
			Description,
			name
		);
		exit(-1);
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
			"LGL_ShaderObj('%s')::SetUniformAttributeFloat(): Error! "
			"Num is '%i', but must be [1,4]...\n",
			Description,
			num
		);
		exit(-1);
	}
	return(true);
}

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
#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS

	FileLoad(filename);

	LinearInterpolation=inLinearInterpolation;
	PixelBufferEnable=true;
	PixelBufferObjectFrontGL=0;
	PixelBufferObjectBackGL=0;

	if(loadToGLTexture)
	{
		LoadSurfaceToTexture
		(
			LinearInterpolation,
			loadToExistantGLTexture,
			loadToExistantGLTextureX,
			loadToExistantGLTextureY
		);
	}

	ReferenceCount=0;
	Timestamp=-1;
	VideoPath[0]='\0';
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
			getwd(wd);
			printf("Working Directory: %s\n",wd);
			exit(0);
		}
	}

	AlphaChannel=bytesperpixel==4;//mySDL_Surface1->flags & SDL_SRCALPHA;
	PixelBufferEnable=true;
	PixelBufferObjectFrontGL=0;
	PixelBufferObjectBackGL=0;

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

	w=width;//SurfaceSDL->w;
	h=height;//SurfaceSDL->h;
	TexW=LGL_NextPowerOfTwo(w);
	TexH=LGL_NextPowerOfTwo(h);

	TextureGL=0;
	TextureGLMine=true;
	FrameBufferImage=false;
	if(data)
	{
		LoadSurfaceToTexture(LinearInterpolation);
		UpdateTexture
		(
			w,
			h,
			bytesperpixel,
			data,
			LinearInterpolation,
			PathShort
		);
	}

	//Delete temporary SDL_Surface

	if(mySDL_Surface1)
	{
		SDL_FreeSurface(mySDL_Surface1);
	}

	ReferenceCount=0;
	Timestamp=-1;
	VideoPath[0]='\0';
}

LGL_Image::
LGL_Image
(
	float	left,	float	right,
	float	bottom,	float	top,
	bool	inReadFromFrontBuffer
)
{
#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS

	LinearInterpolation=true;
	//Baka bounds checking... Never trust users.
	
	if
	(
		left<0		|| left>=1 ||
		right<=0	|| right>1 ||
		bottom<0	|| bottom>=1 ||
		top<=0		|| top>1 ||
		left>=right	|| bottom>=top
	)
	{
		printf("LGL_Image::LGL_Image(): glFrameBuffer bounds error.\n");
		printf
		(
			"(left, right), (bottom ,top) = (%.2f, %.2f), (%.2f, %.2f)\n",
			left,right,bottom,top
		);
		exit(-1);
	}
	
	sprintf(Path,"FrameBuffer Image");
	sprintf(PathShort,"FrameBuffer Image");

	AlphaChannel=false;
	PixelBufferEnable=true;
	PixelBufferObjectFrontGL=0;
	PixelBufferObjectBackGL=0;

	SurfaceSDL=NULL;

	FrameBufferImage=true;
	ReadFromFrontBuffer=inReadFromFrontBuffer;
	FrameBufferViewPort(left,right,bottom,top);

	TexW=LGL_NextPowerOfTwo(LGL.VideoResolutionX);
	TexH=LGL_NextPowerOfTwo(LGL.VideoResolutionY);
	TextureGL=0;
	TextureGLMine=true;

	glGenTextures(1,&(TextureGL));
	glBindTexture(GL_TEXTURE_2D,TextureGL);
	glTexImage2D
	(
		GL_TEXTURE_2D,
		0,			//Level of Detail=0
		GL_RGB,			//Internal Format
		TexW,TexH,
		0,			//Boarder=0
		GL_RGB,
		GL_UNSIGNED_BYTE,
		NULL
	);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	FrameBufferUpdate();

	ReferenceCount=0;
	Timestamp=-1;
	VideoPath[0]='\0';
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
	bool fadetoedges,
	bool fademanual, float fademanualangle,
	float fademanualmag0,
	float fademanualmag1,
	float leftsubimage, float rightsubimage,
	float bottomsubimage, float topsubimage
)
{
#ifdef	LGL_NO_GRAPHICS
	return;
#endif	//LGL_NO_GRAPHICS
	lgl_glScreenify2D();

	if
	(
		fadetoedges ||
		fademanual
	)
	{
		glShadeModel(GL_SMOOTH);
	}

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
				LeftInt/(float)LGL.VideoResolutionX,BottomInt/(float)LGL.VideoResolutionY,
				WidthInt/(float)LGL.VideoResolutionX,HeightInt/(float)LGL.VideoResolutionY,
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
				"LGL_Image::DrawToScreen|%s|%i|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f\n",
				Path,
				LinearInterpolation?1:0,
				left,right,
				bottom,top,
				rotation,
				r,g,b,a,
				leftsubimage, rightsubimage, bottomsubimage, topsubimage
			);
		}
		LGL.DrawLog.push_back(neo);
	}

	if(r<0) r=0;
	if(g<0) g=0;
	if(b<0) b=0;
	if(a<0) a=0;
	
	if(r>1) r=1;
	if(g>1) g=1;
	if(b>1) b=1;
	if(a>1) a=1;

	if(TextureGL==0)
	{
		LoadSurfaceToTexture(LinearInterpolation);
	}

	float width=right-left;
	float height=top-bottom;

	glTranslatef(left+width*.5,bottom+height*.5,0);
	glRotatef(360.0*(rotation/(LGL_PI*2)),0,0,1);

	//Draw

	glColor4f(r,g,b,a);
	if(AlphaChannel==true || a<1 || fadetoedges)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,TextureGL);
	glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);

	if(fadetoedges && fademanual)
	{
		fadetoedges=false;
		fademanual=true;
	}

	float cxsubimage=.5*(leftsubimage+rightsubimage);
	float cysubimage=.5*(bottomsubimage+topsubimage);

	if(fadetoedges)
	{
		//Assumes SDL image, not a framebuffer image
		glBegin(GL_QUADS);
		{
			glNormal3f(0,0,-1);
			//Lower Left

			glColor4f(0,0,0,0);
			glTexCoord2f
			(
				leftsubimage*(float)w/(float)TexW,
				cysubimage*(float)h/(float)TexH
			);
			glVertex2f(-width*.5,0);

			glColor4f(r,g,b,a);
			glTexCoord2f
			(
				cxsubimage*(float)w/(float)TexW,
				cysubimage*(float)h/(float)TexH
			);
			glVertex2f(0,0);
			
			glColor4f(0,0,0,0);
			glTexCoord2f
			(
				cxsubimage*(float)w/(float)TexW,
				rightsubimage*(float)h/(float)TexH
			);
			glVertex2f(0,-height*.5);
			
			glColor4f(0,0,0,0);
			glTexCoord2f
			(
				leftsubimage*(float)w/(float)TexW,
				topsubimage*(float)h/(float)TexH
			);
			glVertex2f(-width*.5,-height*.5);
			
			//Lower Right

			glColor4f(r,g,b,a);
			glTexCoord2f
			(
				cxsubimage*(float)w/(float)TexW,
				cysubimage*(float)h/(float)TexH
			);
			glVertex2f(0,0);
			
			glColor4f(0,0,0,0);
			glTexCoord2f
			(
				rightsubimage*(float)w/(float)TexW,
				cysubimage*(float)h/(float)TexH
			);
			glVertex2f( width*.5,0);
			
			glColor4f(0,0,0,0);
			glTexCoord2f
			(
				rightsubimage*(float)w/(float)TexW,
				topsubimage*(float)h/(float)TexH
			);
			glVertex2f( width*.5,-height*.5);
			
			glColor4f(0,0,0,0);
			glTexCoord2f
			(
				cxsubimage*(float)w/(float)TexW,
				topsubimage*(float)h/(float)TexH
			);
			glVertex2f(0,-height*.5);
			
			//Upper Left
			
			glColor4f(0,0,0,0);
			glTexCoord2f
			(
				leftsubimage*(float)w/(float)TexW,
				bottomsubimage*(float)h/(float)TexH
			);
			glVertex2f(-width*.5, height*.5);
			
			glColor4f(0,0,0,0);
			glTexCoord2f
			(
				cxsubimage*(float)w/(float)TexW,
				bottomsubimage*(float)h/(float)TexH
			);
			glVertex2f(0, height*.5);
			
			glColor4f(r,g,b,a);
			glTexCoord2f
			(
				cxsubimage*(float)w/(float)TexW,
				cysubimage*(float)h/(float)TexH
			);
			glVertex2f(0,0);
			
			glColor4f(0,0,0,0);
			glTexCoord2f
			(
				leftsubimage*(float)w/(float)TexW,
				cysubimage*(float)h/(float)TexH
			);
			glVertex2f(-width*.5,0);
			
			//Upper Right
			
			glColor4f(0,0,0,0);
			glTexCoord2f
			(
				cxsubimage*(float)w/(float)TexW,
				bottomsubimage*(float)h/(float)TexH
			);
			glVertex2f(0, height*.5);
			
			glColor4f(0,0,0,0);
			glTexCoord2f
			(
				rightsubimage*(float)w/(float)TexW,
				bottomsubimage*(float)h/(float)TexH
			);
			glVertex2f( width*.5, height*.5);
			
			glColor4f(0,0,0,0);
			glTexCoord2f
			(
				rightsubimage*(float)w/(float)TexW,
				cysubimage*(float)h/(float)TexH
			);
			glVertex2f( width*.5,0);
			
			glColor4f(r,g,b,a);
			glTexCoord2f
			(
				cxsubimage*(float)w/(float)TexW,
				cysubimage*(float)h/(float)TexH
			);
			glVertex2f(0,0);
		}
		glEnd();
	}
	else if(fademanual)
	{
//FIXME: DrawToScreen fade manual doesn't respect the subimage parameters
		float& angle=fademanualangle;
		float mag0=fademanualmag0;//*LGL_Max(width,height);
		float mag1=fademanualmag1;//*LGL_Max(width,height);
		float nx;
		float ny;
		float u;
		float radius=LGL_Max(.5*width,.5*height); //.5* for inner circle mag=1. sqrt(2)=out
		float fade0x=cos(angle)*mag0*radius;
		float fade0y=sin(angle)*mag0*radius;
		float fade1x=cos(angle)*mag1*radius;
		float fade1y=sin(angle)*mag1*radius;
		float fadeslope;
		bool fadevertical=false;
		if(fabs(cos(angle))<.0001)
		{
			fadevertical=true;
		}
		else
		{
			fadeslope=fade0y/fade0x;
		}
		float normslope=0;
		float norm0b=0;	//as in y=a*x+b, where a is normslope
		float norm1b=0;
		bool normvertical=false;
		if(fabs(sin(angle))<.0001)
		{
			normvertical=true;
		}
		else
		{
			normslope=-fade0x/fade0y;
			//b=y-a*x
			norm0b=fade0y-normslope*fade0x;
			norm1b=fade1y-normslope*fade1x;
		}
		bool normhorizontal=fadevertical;
		float xInt0;
		float xInt1;
		float yInt0;
		float yInt1;
		float texX;
		float texY;

		glBegin(GL_POLYGON);
		{
			glNormal3f(0,0,-1);
			//Begin [LL,UL)

			//Find LL's fade-value, u
			lgl_NearestPointOnLine
			(
				fade0x, fade0y,
				fade1x, fade1y,
				-radius, -radius,
				&nx,&ny,&u
			);
			if(u>=0 && u<=1)
			{
				glColor4f(r*u,g*u,b*u,a*u);
				glTexCoord2f
				(
					0*(float)w/(float)TexW,
					1*(float)h/(float)TexH
				);
				glVertex2f(-width*.5,-height*.5);
			}

			if(normvertical==false)
			{
				//Find y-intercepts of Norm0, Norm1 with left edge
				yInt0=normslope*-radius+norm0b;
				yInt1=normslope*-radius+norm1b;
				for(int z=0;z<2;z++)
				{
					if
					(
						(z==0 && yInt0<=yInt1) ||
						(z==1 && yInt0>yInt1)
					)
					{
						if(yInt0>-radius && yInt0<radius)
						{
							texY=1.0-.5*((yInt0+radius)/radius);
							glColor4f(0,0,0,0);
							glTexCoord2f
							(
								0*(float)w/(float)TexW,
								texY*(float)h/(float)TexH
							);
							glVertex2f(-width*.5,yInt0);
						}
					}
					else
					{
						if(yInt1>-radius && yInt1<radius)
						{
							texY=1.0-.5*((yInt1+radius)/radius);
							glColor4f(r,g,b,a);
							glTexCoord2f
							(
								0*(float)w/(float)TexW,
								texY*(float)h/(float)TexH
							);
							glVertex2f(-width*.5,yInt1);
						}
					}
				}
			}
			//End [LL,UL)
			//Begin [UL,UR)

			//Find UL's fade-value, u
			lgl_NearestPointOnLine
			(
				fade0x, fade0y,
				fade1x, fade1y,
				-radius, radius,
				&nx,&ny,&u
			);
			if(u>=0 && u<=1)
			{
				glColor4f(r*u,g*u,b*u,a*u);
				glTexCoord2f
				(
					0*(float)w/(float)TexW,
					0*(float)h/(float)TexH
				);
				glVertex2f(-width*.5,height*.5);
			}
			
			if(normhorizontal==false)
			{
				//Find x-intercepts of Norm0, Norm1 with top edge
				//x=(y-b)/a
				xInt0=(radius-norm0b)/normslope;
				xInt1=(radius-norm1b)/normslope;
				for(int z=0;z<2;z++)
				{
					if
					(
						(z==0 && xInt0<=xInt1) ||
						(z==1 && xInt0>xInt1)
					)
					{
						if(xInt0>-radius && xInt0<radius)
						{
							texX=.5*((xInt0+radius)/radius);
							glColor4f(0,0,0,0);
							glTexCoord2f
							(
								texX*(float)w/(float)TexW,
								0*(float)h/(float)TexH
							);
							glVertex2f(xInt0,.5*height);
						}
					}
					else
					{
						if(xInt1>-radius && xInt1<radius)
						{
							texX=.5*((xInt1+radius)/radius);
							glColor4f(r,g,b,a);
							glTexCoord2f
							(
								texX*(float)w/(float)TexW,
								0*(float)h/(float)TexH
							);
							glVertex2f(xInt1,.5*height);
						}
					}
				}
			}
			//End [UL,UR)
			//Begin [UR,LR)

			//Find UR's fade-value, u
			lgl_NearestPointOnLine
			(
				fade0x, fade0y,
				fade1x, fade1y,
				radius, radius,
				&nx,&ny,&u
			);
			if(u>=0 && u<=1)
			{
				glColor4f(r*u,g*u,b*u,a*u);
				glTexCoord2f
				(
					1*(float)w/(float)TexW,
					0*(float)h/(float)TexH
				);
				glVertex2f(width*.5,height*.5);
			}
			
			if(normvertical==false)
			{
				//Find y-intercepts of Norm0, Norm1 with right edge
				yInt0=normslope*radius+norm0b;
				yInt1=normslope*radius+norm1b;
				for(int z=0;z<2;z++)
				{
					if
					(
						(z==0 && yInt0>=yInt1) ||
						(z==1 && yInt0<yInt1)
					)
					{
						if(yInt0>-radius && yInt0<radius)
						{
							texY=1.0-.5*((yInt0+radius)/radius);
							glColor4f(0,0,0,0);
							glTexCoord2f
							(
								1*(float)w/(float)TexW,
								texY*(float)h/(float)TexH
							);
							glVertex2f(width*.5,yInt0);
						}
					}
					else
					{
						if(yInt1>-radius && yInt1<radius)
						{
							texY=1.0-.5*((yInt1+radius)/radius);
							glColor4f(r,g,b,a);
							glTexCoord2f
							(
								1*(float)w/(float)TexW,
								texY*(float)h/(float)TexH
							);
							glVertex2f(width*.5,yInt1);
						}
					}
				}
			}
			//End [UR,LR)
			//Begin [LR,LL)

			//Find LR's fade-value, u
			lgl_NearestPointOnLine
			(
				fade0x, fade0y,
				fade1x, fade1y,
				radius, -radius,
				&nx,&ny,&u
			);
			if(u>=0 && u<=1)
			{
				glColor4f(r*u,g*u,b*u,a*u);
				glTexCoord2f
				(
					1*(float)w/(float)TexW,
					1*(float)h/(float)TexH
				);
				glVertex2f(width*.5,-height*.5);
			}
			
			if(normhorizontal==false)
			{
				//Find x-intercepts of Norm0, Norm1 with bottom edge
				//x=(y-b)/a
				xInt0=(-radius-norm0b)/normslope;
				xInt1=(-radius-norm1b)/normslope;
				for(int z=0;z<2;z++)
				{
					if
					(
						(z==0 && xInt0>=xInt1) ||
						(z==1 && xInt0<xInt1)
					)
					{
						if(xInt0>-radius && xInt0<radius)
						{
							texX=.5*((xInt0+radius)/radius);
							glColor4f(0,0,0,0);
							glTexCoord2f
							(
								texX*(float)w/(float)TexW,
								1*(float)h/(float)TexH
							);
							glVertex2f(xInt0,-.5*height);
						}
					}
					else
					{
						if(xInt1>-radius && xInt1<radius)
						{
							texX=.5*((xInt1+radius)/radius);
							glColor4f(r,g,b,a);
							glTexCoord2f
							(
								texX*(float)w/(float)TexW,
								1*(float)h/(float)TexH
							);
							glVertex2f(xInt1,-.5*height);
						}
					}
				}
			}
			//End [UL,UR)
		}
		glEnd();
	}
	else
	{
		if(FrameBufferImage)
		{
			//We're not upside down
			glBegin(GL_QUADS);
			{
				glNormal3f(0,0,-1);
				float d=0;
//#ifdef	LGL_LINUX
//FIXME: FrameBufferImage UGLY fudge factor, due to lousy nVidia Drivers
				if(LGL.FrameBufferTextureGlitchFix)
				{
					d=-0.05/LGL.VideoResolutionX;
				}
//#endif	//LGL_LINUX
				glTexCoord2d
				(
					leftsubimage*(float)w/(float)TexW,
					topsubimage*(float)h/(float)TexH
				);
				glVertex2d(-width*.5+d, height*.5);
				
				glTexCoord2d
				(
					rightsubimage*(float)w/(float)TexW,
					topsubimage*(float)h/(float)TexH
				);
				glVertex2d(width*.5+d, height*.5);
				
				glTexCoord2d
				(
					rightsubimage*(float)w/(float)TexW,
					bottomsubimage*(float)h/(float)TexH
				);
				glVertex2d(width*.5+d,-height*.5);
				
				glTexCoord2d
				(
					leftsubimage*(float)w/(float)TexW,
					bottomsubimage*(float)h/(float)TexH
				);
				glVertex2d(-width*.5+d,-height*.5);
			}
			glEnd();
		}
		else
		{
			//We're an SDL image, so we're upside down

			//Below fixes gibberish lines on right, bottom
			int delta=LinearInterpolation?1:0;
			
			glBegin(GL_QUADS);
			{
				glNormal3f(0,0,-1);
				glTexCoord2f
				(
					leftsubimage*(float)w/(float)TexW,
					bottomsubimage*(float)h/(float)TexH
				);
				glVertex2f(-width*.5, height*.5);
				
				glTexCoord2f
				(
					rightsubimage*(float)(w-delta)/(float)TexW,
					bottomsubimage*(float)h/(float)TexH
				);
				glVertex2f( width*.5, height*.5);
				
				glTexCoord2f
				(
					rightsubimage*(float)(w-delta)/(float)TexW,
					topsubimage*(float)(h-delta)/(float)TexH
				);
				glVertex2f( width*.5,-height*.5);
				
				glTexCoord2f
				(
					leftsubimage*(float)w/(float)TexW,
					topsubimage*(float)(h-delta)/(float)TexH
				);
				glVertex2f(-width*.5,-height*.5);
			}
			glEnd();
		}
	}

	if
	(
		fadetoedges ||
		fademanual
	)
	{
		glShadeModel(GL_FLAT);
	}

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
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
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,TextureGL);
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
				0*(float)w/(float)TexW,
				1*(float)h/(float)TexH
			);
			glVertex2f(LGL.LastImageDrawAsLineRightX,LGL.LastImageDrawAsLineRightY);
			
			//Begin Left
			glTexCoord2f
			(
				1*(float)w/(float)TexW,
				1*(float)h/(float)TexH
			);
			glVertex2f(LGL.LastImageDrawAsLineLeftX,LGL.LastImageDrawAsLineLeftY);
		}
		else
		{
			//Begin Right
			glTexCoord2f
			(
				0*(float)w/(float)TexW,
				1*(float)h/(float)TexH
			);
			glVertex2f(x1+NormX,y1+NormY);
			
			//Begin Left
			glTexCoord2f
			(
				1*(float)w/(float)TexW,
				1*(float)h/(float)TexH
			);
			glVertex2f(x1-NormX,y1-NormY);
		}
		
		//End Left
		glTexCoord2f
		(
			1*(float)w/(float)TexW,
			0*(float)h/(float)TexH
		);
		glVertex2f(x2-NormX,y2-NormY);
		LGL.LastImageDrawAsLineLeftX=x2-NormX;
		LGL.LastImageDrawAsLineLeftY=y2-NormY;
		
		//End Right
		glTexCoord2f
		(
			0*(float)w/(float)TexW,
			0*(float)h/(float)TexH
		);
		glVertex2f(x2+NormX,y2+NormY);
		LGL.LastImageDrawAsLineRightX=x2+NormX;
		LGL.LastImageDrawAsLineRightY=y2+NormY;
	}
	glEnd();

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
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
		glBindTexture(GL_TEXTURE_2D,TextureGL);
		glTexImage2D
		(
			GL_TEXTURE_2D,
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
				GL_TEXTURE_2D,
				0,					//Level of Detail=0
				0,					//X-Offset
				0,					//Y-Offset
				w,
				h,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				SurfaceSDL->pixels
			);
		}

		if(LinearInterpolation)
		{
			glTexParameteri
			(
				GL_TEXTURE_2D,
				GL_TEXTURE_MIN_FILTER,
				GL_LINEAR
			);
			glTexParameteri
			(
				GL_TEXTURE_2D,
				GL_TEXTURE_MAG_FILTER,
				GL_LINEAR
			);
		}
		else
		{
			glTexParameteri
			(
				GL_TEXTURE_2D,
				GL_TEXTURE_MIN_FILTER,
				GL_NEAREST
			);
			glTexParameteri
			(
				GL_TEXTURE_2D,
				GL_TEXTURE_MAG_FILTER,
				GL_NEAREST
			);
		}
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

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
		glBindTexture(GL_TEXTURE_2D,TextureGL);
		if(SurfaceSDL)
		{
			glTexSubImage2D
			(
				GL_TEXTURE_2D,
				0,				//Level of Detail=0
				loadToExistantGLTextureX,	//X-Offset
				loadToExistantGLTextureY,	//Y-Offset
				w,
				h,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				SurfaceSDL->pixels
			);
		}
	}

	GLsizei size = w*h*(AlphaChannel?4:3);

	if(PixelBufferEnable)
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
	}
	else
	{
		PixelBufferObjectFrontGL=0;
		PixelBufferObjectBackGL=0;
	}
}

void
LGL_Image::
UnloadSurfaceFromTexture()
{
	if(TextureGL==0) return;
	if(TextureGLMine==false)
	{
		TextureGL=0;
		return;
	}
	else
	{
		glDeleteTextures(1,&(TextureGL));
		TextureGL=0;
		LGL.TexturePixels-=TexW*TexH;
	}

	//Delete Pixel Buffer Object
	gl2DeleteBuffers(1,&(PixelBufferObjectFrontGL));
	gl2DeleteBuffers(1,&(PixelBufferObjectBackGL));
}

void
LGL_Image::
UpdateTexture
(
	int		x,
	int		y,
	int		bytesperpixel,
	unsigned char*	data,
	bool		inLinearInterpolation,
	const char*	name
)
{
	if(x!=w)
	{
		//printf("x!=w (%i vs %i)\n",x,w);
		w=LGL_Min(x,TexW);
	}
	if(y!=h)
	{
		//printf("y!=h (%i vs %i)\n",y,h);
		h=LGL_Min(y,TexH);
	}
	assert(bytesperpixel==3 || bytesperpixel==4);
	assert(data!=NULL);
	LinearInterpolation=inLinearInterpolation;
	assert(name);
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
/*
	if(PixelBufferObjectFrontGL==0)
	{
		GLsizei size = w*h*(AlphaChannel?4:3);
		gl2GenBuffers(1,&PixelBufferObjectFrontGL);
		gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,PixelBufferObjectFrontGL);
		gl2BufferData
		(
			GL_PIXEL_UNPACK_BUFFER,
			(GLsizeiptr)(&size),
			NULL,
			GL_STREAM_DRAW
		);
		gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
	}
*/
	bool pboReady = gl2IsBuffer(PixelBufferObjectFrontGL);

	if(TextureGL==0)
	{
		//Though surface might be quite undefined at this point... Hmm...
		LoadSurfaceToTexture(LinearInterpolation);
	}

	glBindTexture(GL_TEXTURE_2D,TextureGL);
	if(pboReady)
	{
		gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,PixelBufferObjectFrontGL);
		gl2BufferData(GL_PIXEL_UNPACK_BUFFER, w*h*bytesperpixel, data, GL_STREAM_DRAW);
	}
	else
	{
		gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,0);
	}

	glTexSubImage2D
	(
		GL_TEXTURE_2D,
		0,			//Level of Detail=0
		0,			//X-Offset
		0,			//Y-Offset
		w,
		h,
		bytesperpixel==3?GL_RGB:GL_BGRA,
		GL_UNSIGNED_BYTE,
		pboReady?0:data
	);

	if(pboReady)
	{
		//gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,PixelBufferObjectBackGL);
		//gl2BufferData(GL_PIXEL_UNPACK_BUFFER, w*h*bytesperpixel, 0, GL_STREAM_DRAW);
		/*
		GLubyte* pbo = (GLubyte*)gl2MapBuffer(GL_PIXEL_UNPACK_BUFFER,GL_WRITE_ONLY);
		if(pbo)
		{
			memcpy(pbo,data,w*h*bytesperpixel);
			gl2UnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		}
		*/
	}

	gl2BindBuffer(GL_PIXEL_UNPACK_BUFFER,0);

	//Swap
	/*
	GLuint tmp=PixelBufferObjectBackGL;
	PixelBufferObjectBackGL=PixelBufferObjectFrontGL;
	PixelBufferObjectFrontGL=tmp;
	*/
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
		exit(-1);
	}

	if(ReadFromFrontBuffer)
	{
		glReadBuffer(GL_FRONT_LEFT);
	}
	else
	{
		glReadBuffer(GL_BACK_LEFT);
	}
	glBindTexture(GL_TEXTURE_2D,TextureGL);
	glCopyTexSubImage2D
	(
		GL_TEXTURE_2D,	//target
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
FrameBufferViewPort
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
		printf("LGL_Image::FrameBufferViewPort(): Error! ");
		printf("'%s' is not a FrameBufferImage!\n",Path);
		exit(-1);
	}

	LeftInt=	(int)floor(left*LGL.VideoResolutionX);
	BottomInt=	(int)floor(bottom*LGL.VideoResolutionY);
	WidthInt=	((int)ceil(right*LGL.VideoResolutionX))-LeftInt;
	HeightInt=	((int)ceil(top*LGL.VideoResolutionY))-BottomInt;
	w=WidthInt;
	h=HeightInt;
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
	return(w);
}

int
LGL_Image::
GetHeight()
{
	return(h);
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
		x>=0 && x<w &&
		y>=0 && y<h
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		return
		(
			((char*)SurfaceSDL->pixels)[(h-y)*w*Bpp + x*Bpp]/255.0
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
		x>=0 && x<w &&
		y>=0 && y<h
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		return
		(
			((char*)SurfaceSDL->pixels)[(h-y)*w*Bpp + x*Bpp + 1]/255.0
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
		x>=0 && x<w &&
		y>=0 && y<h
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		return
		(
			((char*)SurfaceSDL->pixels)[(h-y)*w*Bpp + x*Bpp + 2]/255.0
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
		x>=0 && x<w &&
		y>=0 && y<h
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		return
		(
			((char*)SurfaceSDL->pixels)[(h-y)*w*Bpp + x*Bpp + 3]/255.0
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
		x>=0 && x<w &&
		y>=0 && y<h
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		((char*)SurfaceSDL->pixels)[(h-y)*w*Bpp + x*Bpp]=(int)floor(r*255.0);
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
		x>=0 && x<w &&
		y>=0 && y<h
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		((char*)SurfaceSDL->pixels)[(h-y)*w*Bpp + x*Bpp + 1]=(int)floor(g*255.0);
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
		x>=0 && x<w &&
		y>=0 && y<h
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		((char*)SurfaceSDL->pixels)[(h-y)*w*Bpp + x*Bpp + 2]=(int)floor(b*255.0);
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
		x>=0 && x<w &&
		y>=0 && y<h
	)
	{
		int Bpp=SurfaceSDL->format->BytesPerPixel;
		((char*)SurfaceSDL->pixels)[(h-y)*w*Bpp + x*Bpp + 3]=(int)floor(a*255.0);
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
	if(strlen(inFilename)>512)
	{
		printf
		(
			"LGL_Image::LGL_Image(): filename '%s' is too long (512 max)\n",
			inFilename
		);
		exit(-1);
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
		getwd(wd);
		printf("Working Directory: %s\n",wd);
		exit(0);
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

	w=SurfaceSDL->w;
	h=SurfaceSDL->h;
	TexW=LGL_NextPowerOfTwo(w);
	TexH=LGL_NextPowerOfTwo(h);

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
		SDL_SaveBMP(SurfaceSDL, bmpFile);
		return;
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
		exit(-1);
	}

	pixels=(unsigned char*)malloc(3*WidthInt*HeightInt);
	if(pixels==NULL)
	{
		printf("LGL_Image::Save(): Error! Unable to malloc() pixel buffer.\n");
		exit(-1);
	}

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
GetTimestamp()
{
	return(Timestamp);
}

void
LGL_Image::
SetTimestamp
(
	long	timestamp
)
{
	Timestamp=timestamp;
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

	AnimationSem.Lock("Main","Accessing Animation array");
	{
		indexmax=Animation.size()-1;
		ret=Animation[(int)LGL_Clamp(0,index,indexmax)];
	}
	AnimationSem.Unlock();

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
	
	AnimationSem.Lock("Main","Calling Animation.size()");
	{
		ret=Animation.size();
	}
	AnimationSem.Unlock();
	
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
		exit(-1);
	}
	char target[1024];
	for(unsigned int a=0;a<ImageCountMax;a++)
	{
		sprintf(target,"%s/%s",Path,DirList[a]);
		LGL_Image* newbie=new LGL_Image(target);
		AnimationSem.Lock("Main or Animation Loader","Calling Animation.push_back()");
		{
			Animation.push_back(newbie);
		}
		AnimationSem.Unlock();
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



bool lgl_FrameBufferSortPredicate(const lgl_FrameBuffer* d1, const lgl_FrameBuffer* d2)
{
	  return(d1->GetTimestamp() < d2->GetTimestamp());
}

lgl_FrameBuffer::
lgl_FrameBuffer()
{
	Buffer = NULL;
	BufferBytes=0;
	Timestamp=-1;
}

lgl_FrameBuffer::
~lgl_FrameBuffer()
{
	free(Buffer);
	Buffer=NULL;
	BufferBytes=0;
	Timestamp=-1;
}

unsigned char*
lgl_FrameBuffer::
SwapInNewBuffer
(
	char*		videoPath,
	unsigned char*	buffer,
	unsigned int&	bufferBytes,
	long		timestamp
)
{
	strcpy(VideoPath,videoPath);
	unsigned char* bufferOld=Buffer;
	unsigned int bufferBytesOld=BufferBytes;
	Buffer=buffer;
	BufferBytes=bufferBytes;
	bufferBytes=bufferBytesOld;
	Timestamp=timestamp;
	return(bufferOld);
}

const char*
lgl_FrameBuffer::
GetVideoPath()	const
{
	return(VideoPath);
}

unsigned char*
lgl_FrameBuffer::
GetBuffer()	const
{
	return(Buffer);
}

long
lgl_FrameBuffer::
GetTimestamp()	const
{
	return(Timestamp);
}



int
lgl_video_decoder_thread
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

		dec->MaybeLoadVideo();
		dec->MaybeDecodeImage();
		dec->MaybeRecycleBuffers();

		LGL_DelayMS(1);
	}

	return(0);
}

LGL_VideoDecoder::
LGL_VideoDecoder
(
	const char* path
) :
	FrameBufferReadySemaphore("Path Semaphore"),
	PathSemaphore("Path Semaphore")
{
	Init();
	SetVideo(path);
}

LGL_VideoDecoder::
~LGL_VideoDecoder()
{
	UnloadVideo();

	ThreadTerminate=true;
	LGL_ThreadWait(Thread);
	Thread=NULL;
}

void
LGL_VideoDecoder::
Init()
{
	Path[0]='\0';
	PathShort[0]='\0';
	PathNext[0]='\0';

	FPS=0.0f;
	FPSDisplayed=0;
	FPSMissed=0;
	FPSDisplayedHitCounter=0;
	FPSDisplayedMissCounter=0;
	LengthSeconds=0;
	TimeSeconds=0;
	TimeSecondsPrev=0;
	TimestampNext=-1;

	SetFrameBufferAddRadius(30);

	FormatContext=NULL;
	CodecContext=NULL;
	Codec=NULL;
	VideoStreamIndex=-1;
	FrameNative=NULL;
	FrameRGB=NULL;
	BufferRGB=NULL;
	BufferWidth=-1;
	BufferHeight=-1;
	BufferBytes=0;
	SwsConvertContext=NULL;

	Image = NULL;

	ThreadTerminate=false;
	Thread=LGL_ThreadCreate(lgl_video_decoder_thread,this);
}

void
LGL_VideoDecoder::
UnloadVideo()
{
	//
}

void
LGL_VideoDecoder::
SetVideo
(
	const char*	path
)
{
	LGL_ScopeLock pathLock(PathSemaphore);
	if(strcmp(Path,path)!=0)
	{
		if(Image)
		{
			Image->SetTimestamp(-1);
		}
	}
	strcpy(PathNext,path);
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

void
LGL_VideoDecoder::
SetTime
(
	float	seconds
)
{
	TimeSecondsPrev=TimeSeconds;
	TimeSeconds=seconds;
}

float
LGL_VideoDecoder::
GetTime()
{
	return(TimeSeconds);
}

float
LGL_VideoDecoder::
GetLengthSeconds()
{
	return(LengthSeconds);
}

int
LGL_VideoDecoder::
GetFPS()
{
	return((int)ceilf(FPS));
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
GetImage()
{
	//Ensure Image Exists
	if(Image==NULL)
	{
		Image = new LGL_Image
		(
			1920,
			1080,
			4,
			NULL,
			true,
			"Empty LGL_VideoDecoder"
		);
	}

	char path[2048];
	{
		LGL_ScopeLock pathLock(PathSemaphore);
		strcpy(path,Path);
	}
	if(strcmp(Image->GetVideoPath(),path)!=0)
	{
		Image->SetTimestamp(-1);
	}

	long timestamp = SecondsToTimestamp(TimeSeconds);
	lgl_FrameBuffer* buffer=NULL;

	{
		LGL_ScopeLock lock(FrameBufferReadySemaphore);

		//Early out if nothing decoded...
		if(FrameBufferReady.size()==0)
		{
			return(Image);
		}

		//Find the nearest framebuffer
		if(timestamp<FrameBufferReady[0]->GetTimestamp())
		{
			FPSDisplayedMissCounter+=(timestamp!=TimestampDisplayed) ? 1 : 0;
			buffer=FrameBufferReady[0];
		}
		else if(timestamp>FrameBufferReady[FrameBufferReady.size()-1]->GetTimestamp())
		{
			FPSDisplayedMissCounter+=(timestamp!=TimestampDisplayed) ? 1 : 0;
			buffer=FrameBufferReady[FrameBufferReady.size()-1];
		}
		else
		{
			long nearestDistance=99999999;
			for(unsigned int a=0;a<FrameBufferReady.size();a++)
			{
				if((long)fabsf(timestamp-FrameBufferReady[a]->GetTimestamp())<nearestDistance)
				{
					buffer=FrameBufferReady[a];
					nearestDistance=(long)fabsf(timestamp-FrameBufferReady[a]->GetTimestamp());
				}
			}
			
			if(buffer)
			{
				if(timestamp==buffer->GetTimestamp())
				{
					FPSDisplayedHitCounter+=(timestamp!=TimestampDisplayed) ? 1 : 0;
				}
				else
				{
					FPSDisplayedMissCounter+=(timestamp!=TimestampDisplayed) ? 1 : 0;
				}
			}
		}

		if(buffer==NULL)
		{
			return(Image);
		}

		//Is our image already up to date?
		{
			if(Image->GetTimestamp()==buffer->GetTimestamp())
			{
				TimestampDisplayed=timestamp;
				return(Image);
			}
		}
	}

	//Update Image
	char name[1024];
	sprintf(name,"%s|%li\n",PathShort,timestamp);
	Image->UpdateTexture
	(
		BufferWidth,
		BufferHeight,
		4,
		buffer->GetBuffer(),
		true,
		name
	);
	Image->SetTimestamp(timestamp);
	Image->SetVideoPath(path);
	TimestampDisplayed=timestamp;

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

float
LGL_VideoDecoder::
GetSecondsBufferedLeft()
{
	LGL_ScopeLock lock(FrameBufferReadySemaphore);

	if(FrameBufferReady.size()==0)
	{
		return(0.0f);
	}

	long currentTimestamp = SecondsToTimestamp(TimeSeconds);
	int currentIndex=-1;
	for(unsigned int a=0;a<FrameBufferReady.size();a++)
	{
		if(FrameBufferReady[a]->GetTimestamp()==currentTimestamp)
		{
			currentIndex=a;
			break;
		}
	}

	if(currentIndex==-1)
	{
		return(0.0f);
	}

	float seconds=0.0f;

	bool wrap=false;
	for(int a=currentIndex;;a--)
	{
		if(a==-1)
		{
			a=FrameBufferReady.size()-1;
			wrap=true;
		}

		if(wrap && a==currentIndex)
		{
			printf("\t\t\t\tB-1: %i\n\n\n",(int)FrameBufferReady.size());
			break;
		}

		int next=a-1;
		if(next==-1)
		{
			next=FrameBufferReady.size()-1;
		}

		if
		(
			FrameBufferReady[a]->GetTimestamp()-FrameBufferReady[next]->GetTimestamp() == 1 ||
			(
				next==(int)FrameBufferReady.size()-1 &&
				fabsf(FrameBufferReady[next]->GetTimestamp() - SecondsToTimestamp(LengthSeconds))<=1.0f &&
				FrameBufferReady[a]->GetTimestamp() == 0
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

float
LGL_VideoDecoder::
GetSecondsBufferedRight()
{
	LGL_ScopeLock lock(FrameBufferReadySemaphore);

	if(FrameBufferReady.size()==0)
	{
		return(0.0f);
	}

	long currentTimestamp = SecondsToTimestamp(TimeSeconds);
	int currentIndex=-1;
	for(unsigned int a=0;a<FrameBufferReady.size();a++)
	{
		if(FrameBufferReady[a]->GetTimestamp()==currentTimestamp)
		{
			currentIndex=a;
			break;
		}
	}

	if(currentIndex==-1)
	{
		return(0.0f);
	}
	
	float seconds=0.0f;
	bool wrap=false;
	for(int a=currentIndex;;a++)
	{
		if(a==(int)FrameBufferReady.size())
		{
			a=0;
			wrap=true;
		}

		if(wrap && a==currentIndex)
		{
			break;
		}

		int next=a+1;
		if(next==(int)FrameBufferReady.size())
		{
			next=0;
		}

		if
		(
			FrameBufferReady[next]->GetTimestamp()-FrameBufferReady[a]->GetTimestamp() == 1 ||
			(
				next==0 &&
				fabsf(FrameBufferReady[a]->GetTimestamp() - SecondsToTimestamp(LengthSeconds))<=1.0f &&
				FrameBufferReady[next]->GetTimestamp() == 0
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
SetFrameBufferAddRadius
(
	int	frames
)
{
	FrameBufferAddRadius=LGL_Max(2,frames);
	FrameBufferSubtractRadius=(int)(FrameBufferAddRadius*1.5f);;
}

void
LGL_VideoDecoder::
MaybeLoadVideo()
{
	if(PathNext[0]=='\0')
	{
		return;
	}

	//PathNext => Path
	{
		LGL_ScopeLock pathLock(PathSemaphore);

		if(LGL_FileExists(PathNext)==false)
		{
			PathNext[0]='\0';
			return;
		}

		strcpy(Path,PathNext);
		if(const char* lastSlash = strrchr(Path,'/'))
		{
			strcpy(PathShort,&(lastSlash[1]));
		}
		else
		{
			strcpy(PathShort,Path);
		}

		PathNext[0]='\0';
	}

	//Go for it!!

	UnloadVideo();

	LGL_ScopeLock avCodecLock(LGL.AVCodecSemaphore);

	//Open file
	AVFormatContext* fc=NULL;
	{
		LGL_ScopeLock avOpenCloseLock(LGL.AVOpenCloseSemaphore);
		if(av_open_input_file(&fc, Path, NULL, 0, NULL)!=0)
		{
			printf("LGL_VideoDecoder::MaybeLoadVideo(): Couldn't open '%s'\n",Path);
			return;
		}
	}

	//Find streams
	if(av_find_stream_info(fc)<0)
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
	FormatContext=fc;	//Only set this once fc is fully initialized

	// Find the decoder for the video stream
	Codec=avcodec_find_decoder(CodecContext->codec_id);
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
		LGL_ScopeLock avOpenCloseLock(LGL.AVOpenCloseSemaphore);
		if(avcodec_open(CodecContext, Codec)<0)
		{
			printf("LGL_VideoDecoder::MaybeLoadVideo(): Couldn't open codec for '%s'. Codec = '%s'\n",Path,CodecContext->codec_name);
			UnloadVideo();
			return;
		}
	}

	FPS=CodecContext->time_base.den/(float)CodecContext->time_base.num;
	LengthSeconds=FormatContext->duration/(float)(AV_TIME_BASE);

	FrameNative=avcodec_alloc_frame();
	FrameRGB=avcodec_alloc_frame();

	if(FrameNative==NULL || FrameRGB==NULL)
	{
		printf("LGL_Video::MaybeChangeVideo(): Couldn't open frames for '%s'\n",Path);
		return;
	}

	// Determine required buffer size and pseudo-allocate buffer
	BufferWidth=CodecContext->width;
	BufferHeight=CodecContext->height;

	SwsConvertContext = sws_getContext
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
	if(SwsConvertContext==NULL)
	{
		printf("LGL_VideoDecoder::MaybeLoadVideo(): NULL SwsConvertContext for '%s'\n",Path);
		return;
	}
}

void
LGL_VideoDecoder::
MaybeDecodeImage()
{
	if
	(
		FormatContext==NULL ||
		CodecContext==NULL ||
		FrameNative==NULL ||
		FrameRGB==NULL
	)
	{
		return;
	}

	//Find timestamp of image to add
	long timestampTarget=GetNextTimestampToDecode();
	if(timestampTarget==-1)
	{
		return;
	}

	//Seek to the appropriate frame...
	if(TimestampNext!=timestampTarget)
	{
		LGL_ScopeLock avCodecLock(LGL.AVCodecSemaphore);
		av_seek_frame
		(
			FormatContext,
			VideoStreamIndex,
			timestampTarget,
			AVSEEK_FLAG_ANY
		);
		TimestampNext=timestampTarget+1;
	}

	AVPacket Packet;
	bool frameRead=false;
	for(;;)
	{
		int result=0;
		{
			LGL_ScopeLock avCodecLock(LGL.AVCodecSemaphore);
			result = av_read_frame(FormatContext, &Packet);
		}
		if(result>=0)
		{
			// Is this a packet from the video stream?
			if(Packet.stream_index==VideoStreamIndex)
			{
				// Decode video frame
				int frameFinished=0;
				{
					LGL_ScopeLock avCodecLock(LGL.AVCodecSemaphore);
					avcodec_decode_video
					(
						CodecContext,
						FrameNative,
						&frameFinished, 
						Packet.data,
						Packet.size
					);
				}

				// Did we get a video frame?
				if(frameFinished)
				{
					{
						LGL_ScopeLock avCodecLock(LGL.AVCodecSemaphore);

						unsigned int bufferBytesNow=avpicture_get_size
						(
							PIX_FMT_BGRA,
							BufferWidth,
							BufferHeight
						);
						if
						(
							BufferRGB==NULL ||
							BufferBytes<bufferBytesNow
						)
						{
							BufferBytes=bufferBytesNow;
							delete BufferRGB;
							BufferRGB=new uint8_t[BufferBytes];
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
						sws_scale
						(
							SwsConvertContext,
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
			}

			// Free the packet that was allocated by av_read_frame
			{
				LGL_ScopeLock avCodecLock(LGL.AVCodecSemaphore);
				av_free_packet(&Packet);
			}
		}
		else
		{
			break;
		}

		if(frameRead)
		{
			break;
		}
	}

	//If we read a frame, put it into an lgl_FrameBuffer
	if(frameRead)
	{
		LGL_ScopeLock pathLock(PathSemaphore);

		//Prepare a framebuffer, and swap its buffer with BufferRGB
		lgl_FrameBuffer* frameBuffer = GetRecycledFrameBuffer();
		unsigned char* swappedOutBuffer = frameBuffer->SwapInNewBuffer
		(
			Path,
			BufferRGB,
			BufferBytes,	//Changes...
			timestampTarget
		);
		BufferRGB=swappedOutBuffer;

		//Add framebuffer to FrameBufferReady, and sort.
		{
			LGL_ScopeLock lock(FrameBufferReadySemaphore);
			FrameBufferReady.push_back(frameBuffer);
			std::sort
			(
				FrameBufferReady.begin(),
				FrameBufferReady.end(),
				lgl_FrameBufferSortPredicate
			);
		}
	}
}

void
LGL_VideoDecoder::
MaybeRecycleBuffers()
{
	char path[2048];
	{
		LGL_ScopeLock pathLock(PathSemaphore);
		strcpy(path,Path);
	}

	LGL_ScopeLock frameBufferReadyLock(FrameBufferReadySemaphore);

	long timestampNow = SecondsToTimestamp(TimeSeconds);
	long timestampLength = SecondsToTimestamp(LengthSeconds);

	long timestampPrev = SecondsToTimestamp(TimeSecondsPrev);
	long timestampTarget = timestampNow + (timestampNow-timestampPrev);

	//Handle wrap-around
	long timestampPredict=timestampTarget;
	while(timestampPredict>timestampLength)
	{
		timestampPredict-=timestampLength;
	}

	for(unsigned int a=0;a<FrameBufferReady.size();a++)
	{
		if
		(
			strcmp(FrameBufferReady[a]->GetVideoPath(),path)!=0 ||
			(
				fabsf(timestampNow-FrameBufferReady[a]->GetTimestamp())>FrameBufferSubtractRadius &&
				fabsf(timestampPredict-FrameBufferReady[a]->GetTimestamp())>FrameBufferSubtractRadius &&
				fabsf((timestampNow-timestampLength)-FrameBufferReady[a]->GetTimestamp())>FrameBufferSubtractRadius &&
				fabsf((timestampNow+timestampLength)-FrameBufferReady[a]->GetTimestamp())>FrameBufferSubtractRadius
			)
		)
		{
			FrameBufferRecycled.push_back(FrameBufferReady[a]);
			FrameBufferReady.erase
			(
				(std::vector<lgl_FrameBuffer*>::iterator)
				(&(FrameBufferReady[a]))
			);
		}
	}
}

bool
LGL_VideoDecoder::
GetThreadTerminate()
{
	return(ThreadTerminate);
}

float
LGL_VideoDecoder::
TimestampToSeconds(long timestamp)
{
	return(timestamp/FPS);
}

long
LGL_VideoDecoder::
SecondsToTimestamp
(
	float	seconds
)
{
	return(seconds*FPS);
}

long
LGL_VideoDecoder::
GetNextTimestampToDecode()
{
	LGL_ScopeLock lock(FrameBufferReadySemaphore);

	long ret=-1;

	ret = GetNextTimestampToDecodePredictNext();
	if(ret!=-1)
	{
		return(ret);
	}

	if(TimeSeconds>=TimeSecondsPrev)
	{
		ret = GetNextTimestampToDecodeForwards();
		if(ret!=-1)
		{
			return(ret);
		}
		ret = GetNextTimestampToDecodeBackwards();
		if(ret!=-1)
		{
			return(ret);
		}
	}
	else
	{
		ret = GetNextTimestampToDecodeBackwards();
		if(ret!=-1)
		{
			return(ret);
		}
		ret = GetNextTimestampToDecodeForwards();
		if(ret!=-1)
		{
			return(ret);
		}
	}

	return(-1);
}

long
LGL_VideoDecoder::
GetNextTimestampToDecodePredictNext()
{
	long timestampNow = SecondsToTimestamp(TimeSeconds);
	long timestampLength = SecondsToTimestamp(LengthSeconds);

	long timestampPrev = SecondsToTimestamp(TimeSecondsPrev);
	long timestampTarget = timestampNow + (timestampNow-timestampPrev);

	//Handle wrap-around
	long timestampFind=timestampTarget;
	while(timestampFind>timestampLength)
	{
		timestampFind-=timestampLength;
	}

	if(timestampFind<0) timestampFind=0;
	if(timestampFind>timestampLength) timestampFind=timestampLength;

	bool found=false;
	for(unsigned int b=0;b<FrameBufferReady.size();b++)
	{
		long timestampImage=FrameBufferReady[b]->GetTimestamp();
		if(timestampFind<timestampImage)
		{
			return(timestampFind);
		}
		else if(timestampFind==timestampImage)
		{
			found=true;
			break;
		}
		else //timestampFind>timestampImage
		{
			continue;
		}
	}

	if(found==false)
	{
		return(timestampFind);
	}

	return(-1);
}

long
LGL_VideoDecoder::
GetNextTimestampToDecodeForwards()
{
	long timestampNow = SecondsToTimestamp(TimeSeconds);
	long timestampLength = SecondsToTimestamp(LengthSeconds);

	int frameBufferIndex=0;
	long timestampFinal = timestampNow+FrameBufferAddRadius;
	for(long a=timestampNow;a<timestampFinal;a++)
	{
		//Handle wrap-around
		long timestampFind=a;
		while(timestampFind>=timestampLength)
		{
			timestampFind-=timestampLength;
			frameBufferIndex=0;
		}

		bool found=false;
		for(unsigned int b=frameBufferIndex;b<FrameBufferReady.size();b++)
		{
			frameBufferIndex++;
			long timestampImage=FrameBufferReady[b]->GetTimestamp();
			if(timestampFind<timestampImage)
			{
				return(timestampFind);
			}
			else if(timestampFind==timestampImage)
			{
				found=true;
				break;
			}
			else //timestampFind>timestampImage
			{
				continue;
			}
		}

		if(found==false)
		{
			return(timestampFind);
		}
	}

	return(-1);
}

long
LGL_VideoDecoder::
GetNextTimestampToDecodeBackwards()
{
	long timestampNow = SecondsToTimestamp(TimeSeconds);
	long timestampLength = SecondsToTimestamp(LengthSeconds);

	//Second search backwards
	int frameBufferIndex=FrameBufferReady.size()-1;
	long timestampFinal = timestampNow-FrameBufferAddRadius;
	for(long a=timestampNow-1;a>timestampFinal;a--)
	{
		//Handle wrap-around
		long timestampFind=a;
		while(timestampFind<0)
		{
			timestampFind+=timestampLength;
			frameBufferIndex=FrameBufferReady.size()-1;
		}

		bool found=false;
		for(int b=frameBufferIndex;b>=0;b--)
		{
			frameBufferIndex--;
			long timestampImage=FrameBufferReady[b]->GetTimestamp();
			if(timestampFind>timestampImage)
			{
				return(timestampFind);
			}
			else if(timestampFind==timestampImage)
			{
				found=true;
				break;
			}
			else //timestampFind<timestampImage
			{
				continue;
			}
		}

		if(found==false)
		{
			return(timestampFind);
		}
	}

	return(-1);
}

void
LGL_VideoDecoder::
RecycleFrameBuffer
(
	lgl_FrameBuffer*	frameBuffer
)
{
	FrameBufferRecycled.push_back(frameBuffer);
}

lgl_FrameBuffer*
LGL_VideoDecoder::
GetRecycledFrameBuffer()
{
	if(FrameBufferRecycled.size()>0)
	{
		lgl_FrameBuffer* frameBuffer = FrameBufferRecycled[FrameBufferRecycled.size()-1];
		FrameBufferRecycled.pop_back();
		return(frameBuffer);
	}
	else
	{
		lgl_FrameBuffer* frameBuffer = new lgl_FrameBuffer;
		return(frameBuffer);
	}
}



LGL_VideoEncoder::
LGL_VideoEncoder
(
	const char*	src,
	const char*	dstVideo,
	const char*	dstAudio
)
{
	strcpy(SrcPath,src);
	strcpy(DstPath,dstVideo);
	sprintf(DstMp3Path,dstAudio);

	Valid=false;
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
	SrcFrameRGB=NULL;
	SrcBufferRGB=NULL;
	SrcPacketPosMax=0;
	SrcPacket.pos=0;

	SwsConvertContext=NULL;

	DstOutputFormat=NULL;
	DstFormatContext=NULL;
	DstCodecContext=NULL;
	DstCodec=NULL;
	DstStream=NULL;
	DstFrameYUV=NULL;
	DstBuffer=NULL;

	DstMp3OutputFormat=NULL;
	DstMp3FormatContext=NULL;
	DstMp3CodecContext=NULL;
	DstMp3Codec=NULL;
	DstMp3Stream=NULL;
	DstMp3Buffer=NULL;
	DstMp3BufferSamples=NULL;
	DstMp3BufferSamplesIndex=0;
	DstMp3Buffer2=NULL;

	{
		LGL_ScopeLock avCodecLock(LGL.AVCodecSemaphore);

		av_init_packet(&DstPacket);
		av_init_packet(&DstMp3Packet);

		//Open file
		AVFormatContext* fc=NULL;
		{
			LGL_ScopeLock avOpenCloseLock(LGL.AVOpenCloseSemaphore);
			if(av_open_input_file(&fc, src, NULL, 0, NULL)!=0)
			{
				printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't open '%s'\n",src);
				return;
			}
		}

		//Find streams
		if(av_find_stream_info(fc)<0)
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
		if(SrcAudioStreamIndex==-1)
		{
			printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't find audio stream for '%s' in %i streams\n",src,fc->nb_streams);
			return;
		}

		// Get a pointer to the codec context for the video stream
		SrcCodecContext=fc->streams[SrcVideoStreamIndex]->codec;
		strcpy(SrcCodecName,SrcCodecContext->codec_name);
		SrcAudioCodecContext=fc->streams[SrcAudioStreamIndex]->codec;
		SrcFormatContext=fc;	//Only set this once fc is fully initialized

		// Find the decoder for the video stream
		SrcCodec=avcodec_find_decoder(SrcCodecContext->codec_id);
		if(SrcCodec==NULL)
		{
			printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't find codec for '%s'. Codec = '%s'\n",src,SrcCodecContext->codec_name);
			SrcCodecContext=NULL;
			UnsupportedCodec=true;
			return;
		}

		// Find the decoder for the audio stream
		SrcAudioCodec=avcodec_find_decoder(SrcAudioCodecContext->codec_id);
		if(SrcAudioCodec==NULL)
		{
			printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't find audio codec for '%s'. Codec = '%s'\n",src,SrcAudioCodecContext->codec_name);
			SrcAudioCodecContext=NULL;
			return;
		}

		// Inform the codec that we can handle truncated bitstreams -- i.e.,
		// bitstreams where frame boundaries can fall in the middle of packets
		if(SrcCodec->capabilities & CODEC_CAP_TRUNCATED)
		{
			SrcCodecContext->flags|=CODEC_FLAG_TRUNCATED;
		}

		// Open codec
		{
			LGL_ScopeLock avOpenCloseLock(LGL.AVOpenCloseSemaphore);
			if(avcodec_open(SrcCodecContext, SrcCodec)<0)
			{
				printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't open codec for '%s'\n",src);
				SrcCodecContext=NULL;
				return;
			}
		}
		
		// Open audio codec
		{
			LGL_ScopeLock avOpenCloseLock(LGL.AVOpenCloseSemaphore);
			if(avcodec_open(SrcAudioCodecContext, SrcAudioCodec)<0)
			{
				printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't open audio codec for '%s'\n",src);
				SrcAudioCodecContext=NULL;
				return;
			}
		}

		//assert(SrcFrame==NULL);
		//assert(SrcFrameRGB==NULL);
		SrcFrame=avcodec_alloc_frame();
		SrcFrameRGB=avcodec_alloc_frame();

		if(SrcFrame==NULL || SrcFrameRGB==NULL)
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

		SrcBufferRGB=new uint8_t[SrcBufferBytes];

		SwsConvertContext = sws_getContext
		(
			//src
			SrcBufferWidth,
			SrcBufferHeight, 
			SrcCodecContext->pix_fmt, 
			//dst
			SrcBufferWidth,
			SrcBufferHeight,
			PIX_FMT_YUVJ422P,
			SWS_FAST_BILINEAR,
			NULL,
			NULL,
			NULL
		);
		if(SwsConvertContext==NULL)
		{
			printf("LGL_VideoEncoder::LGL_VideoEncoder(): NULL SwsConvertContext for '%s'\n",src);
			return;
		}

		SrcFrameNow=0;
		SrcSecondsNow=0;
	}

	//Prepare dst video

	{
		LGL_ScopeLock avCodecLock(LGL.AVCodecSemaphore);

		// find the video encoder
		DstCodec = avcodec_find_encoder(CODEC_ID_MJPEG);
		if(DstCodec==NULL)
		{
			printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't find MJPEG codec for '%s'\n",src);
			return;
		}

		// prepare the header

		// Set AVI format
		DstOutputFormat = guess_format("avi", NULL, NULL);
		DstOutputFormat->audio_codec = CODEC_ID_NONE;
		DstOutputFormat->video_codec = CODEC_ID_MJPEG;

		// FormatContext
		DstFormatContext = (AVFormatContext*)av_mallocz(sizeof(AVFormatContext));
		DstFormatContext->oformat = DstOutputFormat;
		strcpy(DstFormatContext->filename, DstPath);

		// Video stream
		DstStream = av_new_stream(DstFormatContext,0);
		DstStream->r_frame_rate=SrcFormatContext->streams[SrcVideoStreamIndex]->r_frame_rate;
		DstStream->sample_aspect_ratio=SrcFormatContext->streams[SrcVideoStreamIndex]->sample_aspect_ratio;
		DstStream->time_base=SrcFormatContext->streams[SrcVideoStreamIndex]->time_base;
		DstStream->quality=1.0f;	//It's unclear whether that actually affects mjpeg encoding...
		DstFormatContext->streams[0] = DstStream;
		DstFormatContext->nb_streams = 1;

		DstFrameYUV = avcodec_alloc_frame();

		DstCodecContext = DstStream->codec;
		avcodec_get_context_defaults(DstCodecContext);
		//DstCodecContext->pix_fmt=PIX_FMT_YUVJ422P;
		DstCodecContext->codec_id=CODEC_ID_LJPEG;
		DstCodecContext->codec_type=CODEC_TYPE_VIDEO;
		//const int bitrate = 8*1024*1024*8;	//Provides "quite good" quality for 1280x720. Completely unscientific.
		const int bitrate = 1024*1024*128;	//This value is beyond the highest quality.
		DstCodecContext->bit_rate=bitrate;	//This line dictates the output's quality.
		DstCodecContext->bit_rate_tolerance=bitrate/8;	//Allow for some variance
		DstCodecContext->sample_aspect_ratio=DstStream->sample_aspect_ratio;
		DstCodecContext->width = SrcBufferWidth;
		DstCodecContext->height = SrcBufferHeight;
		//The next line appears goofy and incorrect, but is necessary to ensure a proper framerate.
		//The SrcCodecContext's time_base can be incorrect, for reasons not entirely clear. ffmpeg can be like that.
		DstCodecContext->time_base = DstStream->time_base;//SrcCodecContext->time_base;
		DstCodecContext->pix_fmt = PIX_FMT_YUVJ422P;

		{
			LGL_ScopeLock avOpenCloseLock(LGL.AVOpenCloseSemaphore);
			if(avcodec_open(DstCodecContext, DstCodec) < 0)
			{
				printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't open MJPEG codec for '%s'\n",src);
				DstCodecContext=NULL;
				return;
			}
		}

		dump_format(DstFormatContext,0,DstFormatContext->filename,1);

		int result = url_fopen(&(DstFormatContext->pb), DstFormatContext->filename, URL_WRONLY);
		if(result<0)
		{
			printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't url_fopen() output file '%s' (%i)\n",DstPath,result);
			DstCodecContext=NULL;
			return;
		}

		SrcFileBytes=LGL_FileLengthBytes(src);

		av_write_header(DstFormatContext);

		avpicture_alloc
		(
			(AVPicture*)DstFrameYUV,
			PIX_FMT_YUVJ422P,
			SrcBufferWidth,
			SrcBufferHeight
		);

		DstBuffer = (uint8_t*)malloc(SrcBufferWidth*SrcBufferHeight*4);

#ifndef	NOT_YET
		//Mp3 output

		CodecID acodec = CODEC_ID_VORBIS;

		// find the audio encoder
		DstMp3Codec = avcodec_find_encoder(acodec);
		if(DstMp3Codec==NULL)
		{
			printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't find audio encoding codec for '%s'\n",src);
			return;
		}

		// Set mp3 format
		DstMp3OutputFormat = guess_format("ogg", NULL, NULL);
		DstMp3OutputFormat->audio_codec = acodec;	//FIXME: mp3?? vorbis?? Pick one and name appropriately!
		DstMp3OutputFormat->video_codec = CODEC_ID_NONE;

		// FormatContext
		DstMp3FormatContext = (AVFormatContext*)av_mallocz(sizeof(AVFormatContext));
		DstMp3FormatContext->oformat = DstMp3OutputFormat;
		sprintf(DstMp3FormatContext->filename, "%s",DstMp3Path);

		// audio stream
		DstMp3Stream = av_new_stream(DstMp3FormatContext,0);
		DstMp3Stream->r_frame_rate=SrcFormatContext->streams[SrcAudioStreamIndex]->r_frame_rate;
		DstMp3Stream->quality=1.0f;
		DstMp3FormatContext->streams[0] = DstMp3Stream;
		DstMp3FormatContext->nb_streams = 1;

		DstMp3CodecContext = DstMp3Stream->codec;
		avcodec_get_context_defaults(DstMp3CodecContext);
		DstMp3CodecContext->codec_id=acodec;
		DstMp3CodecContext->codec_type=CODEC_TYPE_AUDIO;
		DstMp3CodecContext->bit_rate=256*1000;	//This doesn't seem to matter much
		DstMp3CodecContext->bit_rate_tolerance=DstMp3CodecContext->bit_rate/4;
		DstMp3CodecContext->global_quality=20000;	//This is so fucking arbitrary.
		DstMp3CodecContext->flags |= CODEC_FLAG_QSCALE;
		DstMp3CodecContext->sample_rate=SrcAudioCodecContext->sample_rate;
		DstMp3CodecContext->channels=2;//SrcAudioCodecContext->channels;

		int openResult=-1;
		{
			LGL_ScopeLock avOpenCloseLock(LGL.AVOpenCloseSemaphore);
			openResult = avcodec_open(DstMp3CodecContext, DstMp3Codec);
		}
		if(openResult < 0)
		{
			printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't open audio encoder codec for '%s'\n",src);
			EncodeAudio=false;
		}
		else
		{
			dump_format(DstMp3FormatContext,0,DstMp3FormatContext->filename,1);
			
			result = url_fopen(&(DstMp3FormatContext->pb), DstMp3FormatContext->filename, URL_WRONLY);
			if(result<0)
			{
				printf("LGL_VideoEncoder::LGL_VideoEncoder(): Couldn't url_fopen() audio output file '%s' (%i)\n",DstMp3FormatContext->filename,result);
				return;
			}

			av_write_header(DstMp3FormatContext);

			DstMp3Buffer = (int16_t*)malloc(LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE);
			DstMp3BufferSamples = (int16_t*)malloc(LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE);
			DstMp3Buffer2 = (int16_t*)malloc(LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE);
		}
#endif	//NOT_YET
	}

	Valid=true;
}

LGL_VideoEncoder::
~LGL_VideoEncoder()
{
	LGL_ScopeLock avCodecLock(LGL.AVCodecSemaphore);

	//Src
	if(SrcCodecContext)
	{
		LGL_ScopeLock avOpenCloseLock(LGL.AVOpenCloseSemaphore);
		avcodec_close(SrcCodecContext);
		//av_free(SrcCodecContext);
	}
	if(SrcAudioCodecContext && SrcAudioCodec)
	{
		LGL_ScopeLock avOpenCloseLock(LGL.AVOpenCloseSemaphore);
		avcodec_close(SrcAudioCodecContext);
		//av_free(SrcAudioCodecContext);
	}
	if(SrcFormatContext)
	{
		LGL_ScopeLock avOpenCloseLock(LGL.AVOpenCloseSemaphore);
		av_close_input_file(SrcFormatContext);
		//av_free(SrcFormatContext);
	}
	if(SrcCodec)
	{
		//???
	}
	if(SrcAudioCodec)
	{
		//???
	}
	if(SrcFrame)
	{
		av_free(SrcFrame);
	}
	if(SrcFrameRGB)
	{
		av_free(SrcFrameRGB);
	}
	if(SrcBufferRGB)
	{
		free(SrcBufferRGB);
	}

	if(SwsConvertContext)
	{
		sws_freeContext(SwsConvertContext);
	}

	//Dst
	if(DstOutputFormat)
	{
		//Don't free this...
		//av_free(DstOutputFormat);
	}
	if(DstCodecContext)
	{
		LGL_ScopeLock avOpenCloseLock(LGL.AVOpenCloseSemaphore);
		avcodec_close(DstCodecContext);
	}
	if(DstCodec)
	{
		//???
	}
	if(DstFormatContext)
	{
		free(DstFormatContext);
	}
	if(DstStream)
	{
		av_free(DstStream);
	}
	if(DstFrameYUV)
	{
		av_free(DstFrameYUV);
	}
	if(DstBuffer)
	{
		free(DstBuffer);
	}
	if(DstMp3Buffer)
	{
		free(DstMp3Buffer);
	}
	if(DstMp3BufferSamples)
	{
		free(DstMp3BufferSamples);
	}
	if(DstMp3Buffer2)
	{
		free(DstMp3Buffer2);
	}
}

bool
LGL_VideoEncoder::
IsValid()
{
	return(Valid);
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
		LGL.AVCodecSemaphore->Lock("videoEncoderThread","Calling av_read_frame() (meh)");
		{
			result = av_read_frame(SrcFormatContext, &SrcPacket);
		}
		LGL.AVCodecSemaphore->Unlock();
		if(result<0)
		{
			//We're done!
			if(EncodeVideo)
			{
				av_write_trailer(DstFormatContext);
				url_fclose(DstFormatContext->pb);	//FIXME: Memleak
			}
			DstFormatContext->pb=NULL;

#ifndef	NOT_YET
			if(EncodeAudio)
			{
				av_write_trailer(DstMp3FormatContext);
				url_fclose(DstMp3FormatContext->pb);	//FIXME: Memleak
			}
			DstMp3FormatContext->pb=NULL;
#endif	//NOT_YET
			break;
		}

		// Is this a packet from the video stream?
		if
		(
			SrcPacket.stream_index==SrcVideoStreamIndex &&
			EncodeVideo
		)
		{
			int frameFinished=0;
			LGL.AVCodecSemaphore->Lock("videoEncoderThread","Calling avcodec_decode_video() (meh)");
			{
				avcodec_decode_video
				(
					SrcCodecContext,
					SrcFrame,
					&frameFinished, 
					SrcPacket.data,
					SrcPacket.size
				);
			}
			LGL.AVCodecSemaphore->Unlock();

			// Did we get a video frame?
			if(frameFinished)
			{
				// Convert the image from src format to dst
				LGL.AVCodecSemaphore->Lock("lgl_video_thread","Calling sws_scale() (meh)");
				{
					//Is this sws_scale line actually necessary...?
					sws_scale
					(
						SwsConvertContext,
						SrcFrame->data,
						SrcFrame->linesize,
						0, 
						SrcBufferHeight,
						DstFrameYUV->data,
						DstFrameYUV->linesize
					);
					DstPacket.size = avcodec_encode_video(DstCodecContext, DstBuffer, SrcBufferWidth*SrcBufferHeight*4, DstFrameYUV);
				}
				LGL.AVCodecSemaphore->Unlock();

				DstPacket.dts = SrcPacket.dts;
				DstPacket.pts = SrcPacket.pts;//DstCodecContext->coded_frame->pts;
				DstPacket.flags |= PKT_FLAG_KEY;
				DstPacket.stream_index = 0;
				DstPacket.data=DstBuffer;
				DstPacket.duration=SrcPacket.duration;
				LGL.AVCodecSemaphore->Lock("videoEncoderThread","Calling av_write_frame() (meh)");
				{
					result = av_write_frame(DstFormatContext, &DstPacket);
				}
				LGL.AVCodecSemaphore->Unlock();
				SrcFrameNow++;
			}
		}
		
		// Is this a packet from the audio stream?
		else if
		(
			SrcPacket.stream_index==SrcAudioStreamIndex &&
			EncodeAudio
		)
		{
#ifndef	NOT_YET
			// Convert the audio from src format to dst
			LGL.AVCodecSemaphore->Lock("lgl_video_thread","Calling avcodec_decode_audio2() (meh)");
			{
				int outbufsize = LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE;
				result = avcodec_decode_audio2
				(
					SrcAudioCodecContext,
					DstMp3Buffer,
					&outbufsize,
					SrcPacket.data,
					SrcPacket.size
				);
				if(SrcAudioCodecContext->channels==2)
				{
					memcpy(&(DstMp3BufferSamples[DstMp3BufferSamplesIndex]),DstMp3Buffer,outbufsize);
					DstMp3BufferSamplesIndex+=outbufsize/2;
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
				}
			}
			LGL.AVCodecSemaphore->Unlock();

			while(DstMp3BufferSamplesIndex>=DstMp3CodecContext->frame_size*DstMp3CodecContext->channels)
			{
				LGL.AVCodecSemaphore->Lock("lgl_video_thread","Calling avcodec_decode_audio2() (meh)");
				{
					DstMp3Packet.size = avcodec_encode_audio
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

					DstMp3Packet.pts = av_rescale_q(DstMp3CodecContext->coded_frame->pts,DstMp3CodecContext->time_base,DstMp3Stream->time_base);
					DstMp3Packet.dts = DstMp3Packet.pts;
					DstMp3Packet.flags |= PKT_FLAG_KEY;
					DstMp3Packet.stream_index = 0;
					DstMp3Packet.data=(uint8_t*)DstMp3Buffer2;
					DstMp3Packet.duration=0;
					result = av_write_frame(DstMp3FormatContext, &DstMp3Packet);
				}
				LGL.AVCodecSemaphore->Unlock();
			}
#endif	//NOT_YET
		}
		LGL.AVCodecSemaphore->Lock("LGL_VideoEncoder::Encode()","Calling av_free_packet() (meh)");
		{
			av_free_packet(&SrcPacket);
		}
		LGL.AVCodecSemaphore->Unlock();
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

//LGL_Font

LGL_Font::
LGL_Font
(
	const
	char*	dirpath
)
{
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
	glBindTexture(GL_TEXTURE_2D,TextureGL);
	glTexImage2D
	(
		GL_TEXTURE_2D,
		0,			//Level of Detail=0
		GL_RGBA,		//Internal Format
		TextureSideLength,TextureSideLength,
		0,			//Boarder=0
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		NULL
	);
	glTexParameteri
	(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_NEAREST
	);
	glTexParameteri
	(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_NEAREST
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	LGL.TexturePixels+=TextureSideLength*TextureSideLength;

	//Second pass: load all glyphs to a single large texture, using a special LGL_Image constructor

	int x=0;
	int y=0;

	for(int a=0;a<256;a++)
	{
		sprintf(path,"%s/%i.png",Path,a);
		
		if(LGL_FileExists(path))
		{
			Glyph[a]=new LGL_Image(path,false,true,TextureGL,x,y);/*{{{*//*}}}*/
			GlyphTexLeft[a]=x;
			GlyphTexRight[a]=x+Glyph[a]->w;
			GlyphTexBottom[a]=y+Glyph[a]->h;
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
	ReferenceCount=0;
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

	LGL_Assert(string!=NULL);

	//Process the formatted part of the string
	char tmpstr[1024];
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
	float glyph32h = Glyph[32]->h;
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
		float texBottom = (GlyphTexBottom[ch]-safety)/(float)TextureSideLength;	//Mote: bottom is numerically greater than top, here.
		float texTop = (GlyphTexTop[ch]+safety)/(float)TextureSideLength;

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

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,TextureGL);

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

	glDisable(GL_TEXTURE_2D);

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
		height*(float)Glyph[(unsigned int)in]->w/(float)Glyph[(unsigned int)in]->h +
		height*(float)1/(float)Glyph[(unsigned int)in]->h
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
				exit(-1);
			}
		}
		LGL.Font=new LGL_Font(fontDir);
	}
	return(*(LGL.Font));
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
	for(int a=0;a<1024;a++)
	{
		Buffer[a]='\0';
	}
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
HasFocus()
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
		exit(-1);
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
		exit(-1);
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
		exit(-1);
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
		exit(-1);
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
		exit(-1);
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
			) && length>0
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
	LGL.AudioStreamListSemaphore->Lock("AddAudioStream","Calling AudioStreamList.push_back()");
	{
		LGL.AudioStreamList.push_back(stream);
	}
	LGL.AudioStreamListSemaphore->Unlock();
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

	FreqResponseNextSemaphore.Lock("Main","AudioDSP::SetFreqResponse()");
	{
		memcpy(FreqResponseNextReal,freqResponseActualReal,1024*sizeof(float));
		memcpy(FreqResponseNextImaginary,freqResponseActualImaginary,1024*sizeof(float));
		FreqResponseNextAvailable=true;
	}
	FreqResponseNextSemaphore.Unlock();
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

		unsigned long sampleCount=(unsigned long)LGL_Min(samples-sampleStart,samplesMaxFFTHalf);

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
		for(unsigned int a=0;a<=samplesMaxFFT;a++)
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
		FreqResponseNextSemaphore.Lock("AudioOut","ProcessChannelStereo() updating FreqResponse",false);
		{
			memcpy(FreqResponseReal,FreqResponseNextReal,1024*sizeof(float));
			memcpy(FreqResponseImaginary,FreqResponseNextImaginary,1024*sizeof(float));
			FreqResponseNextAvailable=false;
		}
		FreqResponseNextSemaphore.Unlock();
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

	StartDelaySamples=0;
	CurrentPositionSamplesInt=0;
	CurrentPositionSamplesFloat=0.0f;
	LengthSamples=0;
	SpectrumSamples=512;
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
	for(int z=0;z<LengthSamples;z++)
	{
		pointsXY[2*z+0] = left+width*(z/(float)LengthSamples);
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
		LengthSamples,
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
	float&	freqFactor
)
{
	CalculateWaveformDerivatives();

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
		printf("LGL_AudioGrain::SetWaveformFromLGLSound(): FIXME: Only works with stereo sounds.\n");
		return;
	}

	long centerPositionSamples=(int)LGL_Max(0,(centerSeconds+LGL_RandFloat(-centerSecondsVariance,centerSecondsVariance))*sound->GetHz());
	LengthSamples=(int)LGL_Max(0,(lengthSeconds+LGL_RandFloat(-lengthSecondsVariance,lengthSecondsVariance))*sound->GetHz());
	long startPositionSamples=centerPositionSamples-(LengthSamples/2);
	long endPositionSamples=startPositionSamples+LengthSamples;

	if(endPositionSamples >= sound->GetLengthSamples())
	{
		endPositionSamples=sound->GetLengthSamples()-1;
		LengthSamples=endPositionSamples-startPositionSamples;
	}

	Waveform=new Uint8[LengthSamples*4];
	sound->LockBufferForReading(2);
	{
		Uint8* soundBuffer=sound->GetBuffer();
		memcpy(Waveform,&(soundBuffer[startPositionSamples*4]),LengthSamples*4);
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

void
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
	int		hz
)
{
	int sampleAdvanceFactor=1;
	assert(sampleLast>=sampleFirst);
	zeroCrossingFactor=0.0f;
	magnitudeAve=0.0f;
	magnitudeMax=0.0f;

	float magnitudeTotal=0.0f;
	int zeroCrossings=0;

	for(int a=0;a<2;a++)
	{
		unsigned int myIndex = sampleFirst+a;
		if(myIndex>len16-1) myIndex=len16-1;
		int zeroCrossingSign=(int)LGL_Sign(buf16[myIndex]);

		for(long b=sampleFirst*2+a;b<sampleLast*2;b+=2*sampleAdvanceFactor)
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
		}
	}

	int samplesScanned=(int)(sampleLast-sampleFirst);
	magnitudeAve=(magnitudeTotal/(samplesScanned/sampleAdvanceFactor))/(1<<15);
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
	zeroCrossingFactor=sqrtf(zeroCrossingFactor);
}

void
LGL_AudioGrain::
CalculateWaveformDerivatives()
{
	if(WaveformMonoFloat!=NULL)
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
		44100	//HACK
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

	GrainListsSemaphore.Lock("AudioOut","Mixing grains");
	{
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
	GrainListsSemaphore.Unlock();

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
	GrainListsSemaphore.Lock("AudioOut","AddNextGrain()");
	{
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
	GrainListsSemaphore.Unlock();
}

void
LGL_AudioGrainStream::
AddNextGrains
(
	std::vector<LGL_AudioGrain*>&	grains
)
{
	GrainListsSemaphore.Lock("AudioOut","AddNextGrains()");
	{
		for(unsigned int a=0;a<grains.size();a++)
		{
			LGL_AudioGrain* grain=grains[a];
			AudioGrainsQueued.push_back(grain);
		}
	}
	GrainListsSemaphore.Unlock();
}

long
LGL_AudioGrainStream::
GetGrainsQueuedCount()
{
	return(AudioGrainsQueued.size());
}

//Audio1

int
lgl_SoundDecoderThread
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
		Buffer=(Uint8*)malloc(BufferLengthTotal);
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

	BadFile=false;

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
		DeleteSemaphore.Lock("SoundDecoderThread","Decoding sound in new thread");
		DecoderThread=LGL_ThreadCreate(lgl_SoundDecoderThread,this);
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

	DeleteSemaphore.Lock("PrepareForDeleteThreadFunc","Final Part (meh)");
	{
		if(DecoderThread!=NULL)
		{
			LGL_ThreadWait(DecoderThread);
			DecoderThread=NULL;
		}

		if(Buffer!=NULL)
		{
			if(BufferAllocatedFromElsewhere==false)
			{
				BufferSemaphore.Lock("Main","PrepareForDeleteThreadFunc calling free(Buffer)");
				{
					free(Buffer);
					Buffer=NULL;
				}
				BufferSemaphore.Unlock();
			}
			else
			{
				Buffer=NULL;
			}
		}
		if(BufferBack!=NULL)
		{
			BufferSemaphore.Lock("Main","PrepareForDeleteThreadFunc calling free(BufferBack)");
			{
				free(BufferBack);
				BufferBack=NULL;
			}
			BufferSemaphore.Unlock();
		}
	}
	DeleteSemaphore.Unlock();

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
		exit(-1);
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

	int Available=-1;
	for(int a=0;a<LGL_SOUND_CHANNEL_NUMBER;a++)
	{
		if(LGL.SoundChannel[a].Occupied==false)
		{
			Available=a;
			break;
		}
	}
	if(Available!=-1)
	{
		LGL.SoundChannel[Available].Paused=false;
		LGL.SoundChannel[Available].PositionSamplesStart=startSeconds*Hz;
		LGL.SoundChannel[Available].PositionSamplesPrev=startSeconds*Hz;
		LGL.SoundChannel[Available].PositionSamplesNow=startSeconds*Hz;
		LGL.SoundChannel[Available].FuturePositionSamplesPrev=-1;
		LGL.SoundChannel[Available].FuturePositionSamplesNow=-1;
		LGL.SoundChannel[Available].PositionSamplesNowLastReported=0;
		LGL.SoundChannel[Available].PositionSamplesEnd=startSeconds*Hz+
			lengthSeconds*Hz;
		LGL.SoundChannel[Available].PositionSamplesDeltaLastTime.Reset();
		LGL.SoundChannel[Available].DivergeSamples=0;
		LGL.SoundChannel[Available].DivergeState=0;
		LGL.SoundChannel[Available].DivergeSpeed=1.0f;
		LGL.SoundChannel[Available].WarpPointSecondsTrigger=-1.0f;
		LGL.SoundChannel[Available].WarpPointSecondsDestination=-1.0f;
		LGL.SoundChannel[Available].SampleRateConverterBufferValidSamples=0;
		LGL.SoundChannel[Available].SampleRateConverterBufferCurrentSamplesIndex=0;
		LGL.SoundChannel[Available].SampleRateConverterBufferStartSamples=0;
		if(LGL.SoundChannel[Available].SampleRateConverterL)
		{
			src_reset(LGL.SoundChannel[Available].SampleRateConverterL);
		}
		else
		{
			int error=0;
			LGL.SoundChannel[Available].SampleRateConverterL = src_new(SRC_SINC_FASTEST,1,&error);
			assert(LGL.SoundChannel[Available].SampleRateConverterL);
		}
		if(LGL.SoundChannel[Available].SampleRateConverterR)
		{
			src_reset(LGL.SoundChannel[Available].SampleRateConverterR);
		}
		else
		{
			int error=0;
			LGL.SoundChannel[Available].SampleRateConverterR = src_new(SRC_SINC_FASTEST,1,&error);
			assert(LGL.SoundChannel[Available].SampleRateConverterR);
		}
		LGL.SoundChannel[Available].VolumeFrontLeftDesired=volume;
		LGL.SoundChannel[Available].VolumeFrontRightDesired=volume;
		LGL.SoundChannel[Available].VolumeBackLeftDesired=volume;
		LGL.SoundChannel[Available].VolumeBackRightDesired=volume;
		LGL.SoundChannel[Available].VolumeFrontLeft=volume;
		LGL.SoundChannel[Available].VolumeFrontRight=volume;
		LGL.SoundChannel[Available].VolumeBackLeft=volume;
		LGL.SoundChannel[Available].VolumeBackRight=volume;
		LGL.SoundChannel[Available].Channels=Channels;
		LGL.SoundChannel[Available].Hz=Hz;
		LGL.SoundChannel[Available].ToMono=false;
		LGL.SoundChannel[Available].SpeedNow=speed;
		LGL.SoundChannel[Available].SpeedDesired=speed;
		LGL.SoundChannel[Available].SpeedInterpolationFactor=1;
		LGL.SoundChannel[Available].SpeedVolumeFactor=1;
		LGL.SoundChannel[Available].Glitch=false;
		LGL.SoundChannel[Available].FutureGlitchSettingsAvailable=false;
		LGL.SoundChannel[Available].GlitchVolume=0;
		LGL.SoundChannel[Available].GlitchSpeedNow=1;
		LGL.SoundChannel[Available].GlitchSpeedDesired=1;
		LGL.SoundChannel[Available].GlitchSpeedInterpolationFactor=1;
		LGL.SoundChannel[Available].GlitchDuo=0;
		LGL.SoundChannel[Available].GlitchLuminScratch=false;
		LGL.SoundChannel[Available].GlitchLuminScratchPositionDesired=-10000;
		LGL.SoundChannel[Available].FutureGlitchSamplesNow=-10000;
		LGL.SoundChannel[Available].GlitchSamplesNow=0;
		LGL.SoundChannel[Available].GlitchLast=0;
		LGL.SoundChannel[Available].GlitchBegin=0;
		LGL.SoundChannel[Available].GlitchLength=0;
		LGL.SoundChannel[Available].Loop=looping;
		LGL.SoundChannel[Available].StickyEndpoints=false;
		LGL.SoundChannel[Available].LengthSamples=lengthSeconds*Hz;
		LGL.SoundChannel[Available].BufferLength=BufferLength;
		LGL.SoundChannel[Available].Buffer=Buffer;
		LGL.SoundChannel[Available].BufferSemaphore=&BufferSemaphore;
		LGL.SoundChannel[Available].LGLSound=this;
		if(LGL.SoundChannel[Available].LGLAudioDSPFront!=NULL)
		{
			delete LGL.SoundChannel[Available].LGLAudioDSPFront;
			LGL.SoundChannel[Available].LGLAudioDSPFront=NULL;
		}
		LGL.SoundChannel[Available].ClearMe=false;
		LGL.SoundChannel[Available].Occupied=true;
	}
	else
	{
		printf("LGL_Sound.Play(): Unable to play '%s'. No free channels.\n",Path);
	}

	return(Available);
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
		exit(-1);
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
		exit(-1);
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

	LGL_AudioDSP* dsp;
	dsp=LGL.SoundChannel[channel].LGLAudioDSPFront;
	if(dsp==NULL)
	{
		dsp=new LGL_AudioDSP;
	}
	dsp->SetFreqResponse(freqResponseArrayOf513);
	LGL.SoundChannel[channel].LGLAudioDSPFront=dsp;
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

	signed long ret=(signed long)((LGL.SoundChannel[channel].PositionSamplesNow));
	if(LGL.SoundChannel[channel].FuturePositionSamplesNow>=0.0f)
	{
		ret = (signed long)LGL.SoundChannel[channel].FuturePositionSamplesNow;
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
	return(LGL.SoundChannel[channel].WarpPointSecondsTrigger>=0);
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
	LGL.SoundChannel[channel].WarpPointSecondsTrigger=-1.0f;
	LGL.SoundChannel[channel].WarpPointSecondsDestination=-1.0f;

	return(true);
}

bool
LGL_Sound::
SetWarpPoint
(
	int	channel,
	double	triggerSeconds,
	double	dstSeconds
)
{
if(channel<0)
{
	printf("LGL_Sound::SetWarpPoint(3): WARNING! channel < 0\n");
	return(false);
}
	LGL.SoundChannel[channel].WarpPointSecondsTrigger=triggerSeconds;
	LGL.SoundChannel[channel].WarpPointSecondsDestination=dstSeconds;
	
	return(true);
}

float
LGL_Sound::
GetWarpPointSecondsTrigger
(
	int	channel
)
{
	return(LGL.SoundChannel[channel].WarpPointSecondsTrigger);
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
	BufferSemaphore.Lock("AudioOut or Main?","LGL_Sound::LockBuffer()");
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
	BufferReaderCountSemaphore.Lock("Main","LockBufferForReading()");
	if(BufferReaderCount==0)
	{
		BufferSemaphore.Lock("Main","LockBufferForReading()");
	}
	assert(BufferReaderCount>=0);
	BufferReaderCount++;
	BufferReaderCountSemaphore.Unlock();
}

void
LGL_Sound::
UnlockBufferForReading(int id)
{
	BufferReaderCountSemaphore.Lock("Main","UnlockBufferForReading()");
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
	return(BufferLength/(4.0f*Hz));
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

void
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
	LockBufferForReading(10);
	{
		const Sint16* buf16=(Sint16*)GetBuffer();
		unsigned long len16=(GetBufferLength()/2);
		bool loaded=IsLoaded();
		int hz=Hz;

		lgl_analyze_wave_segment
		(
			sampleFirst,
			sampleLast,
			zeroCrossingFactor,
			magnitudeAve,
			magnitudeMax,
			buf16,
			len16,
			loaded,
			hz
		);
	}
	UnlockBufferForReading(10);
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
		AnalyzeWaveSegment
		(
			secondsBegin*Hz,
			secondsEnd*Hz,
			zeroCrossingFactor,
			magnitudeAve,
			magnitudeMax
		);
		return(true);
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

	AVFormatContext*	formatContext;
	AVCodecContext*		codecContext;
	AVCodec*		codec;

	int			audioStreamIndex=-1;
	int			channels=0;

	{
		LGL_ScopeLock avCodecLock(LGL.AVCodecSemaphore);
		//Open file
		{
			LGL_ScopeLock avOpenCloseLock(LGL.AVOpenCloseSemaphore);
			if(av_open_input_file(&formatContext, Path, NULL, 0, NULL)!=0)
			{
				printf("LGL_Sound::LoadToMemory(): av_open_input_file() couldn't open '%s'\n",Path);
				BadFile=true;
				return;
			}
		}

		//Find streams
		if(av_find_stream_info(formatContext)<0)
		{
			printf("LGL_Sound::LoadToMemory(): Couldn't find streams for '%s'\n",Path);
			BadFile=true;
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

		if(audioStreamIndex==-1)
		{
			printf("LGL_Sound::LoadToMemory(): Couldn't find audio stream for '%s' in %i streams\n",Path,formatContext->nb_streams);
			BadFile=true;
			return;
		}

		// Get a pointer to the codec context for the audio stream
		codecContext=formatContext->streams[audioStreamIndex]->codec;

		// Find the decoder for the audio stream
		codec=avcodec_find_decoder(codecContext->codec_id);
		if(codec==NULL)
		{
			printf("LGL_Sound::LoadToMemory: Couldn't find audio codec for '%s'. Codec = '%s'\n",Path,codecContext->codec_name);
			BadFile=true;
			return;
		}

		channels=codecContext->channels;
		if
		(
			channels!=1 &&
			channels!=2
		)
		{
			printf("LGL_Sound::LoadToMemory(): Invalid channel found for '%s': %i\n",Path,channels);
			BadFile=true;
			return;
		}

		Hz=codecContext->sample_rate;

		// Open codec
		{
			LGL_ScopeLock avOpenCloseLock(LGL.AVOpenCloseSemaphore);
			if(avcodec_open(codecContext,codec)<0)
			{
				printf("LGL_Sound::LoadToMemory(): Couldn't open codec for '%s'\n",Path);
				av_close_input_file(formatContext);
				BadFile=true;
				return;
			}
		}
	}

	// Inform the codec that we can handle truncated bitstreams -- i.e.,
	// bitstreams where frame boundaries can fall in the middle of packets
	if(codec->capabilities & CODEC_CAP_TRUNCATED)
	{
		codecContext->flags|=CODEC_FLAG_TRUNCATED;
	}
    
	int cyclesNow=0;
	int cyclesMax=8;
	int delayMS=1;
	int16_t* outbuf=new int16_t[LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE];

	for(;;)
	{
		if(DestructorHint)
		{
			break;
		}
	
		AVPacket packet;

		int result=0;
		{
			LGL_ScopeLock avCodecLock(LGL.AVCodecSemaphore);
			result = av_read_frame(formatContext, &packet);
		}

		if(result>=0)
		{
			totalFileBytesUsed+=packet.size;
			// Is this a packet from the audio stream?
			if(packet.stream_index==audioStreamIndex)
			{
				// Decode audio frame
				int outbufsize = LGL_AVCODEC_MAX_AUDIO_FRAME_SIZE;
				{
					LGL_ScopeLock avCodecLock(LGL.AVCodecSemaphore);
					avcodec_decode_audio2	//FIXME: This undercounts totalFileBytesUsed...
					(
						codecContext,
						(int16_t*)outbuf,
						&outbufsize,
						packet.data,
						packet.size
					);
				}

				PercentLoaded = totalFileBytesUsed/(float)totalFileBytes;

				if(BufferLength+outbufsize < BufferLengthTotal)
				{
					//There's enough room at the end of Buffer

					//Copy Extra Bits To Buffer
					if(channels==1)
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
					else
					{
						//channels==2
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
					break;
				}

				float secondsLoaded=GetLengthSeconds();
				float secondsMetadataAlpha=MetadataFilledSize/(float)LGL_SOUND_METADATA_ENTRIES_PER_SECOND;
				float secondsMetadataDelta=1.0f/(float)LGL_SOUND_METADATA_ENTRIES_PER_SECOND;
				while(secondsMetadataAlpha+secondsMetadataDelta<secondsLoaded)
				{
					long sampleFirst=secondsMetadataAlpha*Hz;
					long sampleLast=(secondsMetadataAlpha+secondsMetadataDelta)*Hz;
					AnalyzeWaveSegment
					(
						sampleFirst,
						sampleLast,
						MetadataFreqFactor[MetadataFilledSize],
						MetadataVolumeAve[MetadataFilledSize],
						MetadataVolumeMax[MetadataFilledSize]
					);
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
				LGL_DelayMS(5);
			}

			// Free the data in the packet that was allocated by av_read_frame (but not the packet object itself)
			{
				LGL.AVCodecSemaphore->Unlock();
				av_free_packet(&packet);
			}
		}
		else
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

	Loaded=true;
	LoadedMin=1;
	PercentLoaded=1.0f;
	delete outbuf;
	outbuf=NULL;
	
	{
		LGL_ScopeLock avCodecLock(LGL.AVCodecSemaphore);
		LGL_ScopeLock avOpenCloseLock(LGL.AVOpenCloseSemaphore);
		avcodec_close(codecContext);
		av_close_input_file(formatContext);
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
	return(1.0f);
	if(BadFile)
	{
		return (60.0f*88.0f);
	}
	//if(LGL.AudioAvailable==false) return(0);
#ifdef	LGL_WIN32
	return(20);
#endif	//LGL_WIN32
	float r=LoadTimer.SecondsSinceLastReset();
	float t=r/GetPercentLoaded()-r;
	if(r<1)
	{
		return(t);
	}
	else
	{
		if(t<LoadTimerMin) LoadTimerMin=t;
		return(LoadTimerMin);
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

bool
LGL_RecordDVJToFileStart()
{
	if(LGL.RecordFileDescriptor)
	{
		if(LGL_FileExists(LGL.RecordFilePath)==false)
		{
			pclose(LGL.RecordFileDescriptor);
			LGL.RecordFileDescriptor=NULL;
			return(false);
		}
		else
		{
			LGL.RecordActive=true;
			printf("\nLGL_RecordDVJToFileStart(): Recording audio to path:\n\t%s\n\n",LGL.RecordFilePath);
			return(true);
		}
	}

	return(false);
}

const char*
LGL_RecordDVJToFilePath()
{
	return(LGL.RecordFilePath);
}

const char*
LGL_RecordDVJToFilePathShort()
{
	return(LGL.RecordFilePathShort);
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
	LGL.WiimoteSemaphore->Lock("Main","LGL_ProcessInput()");
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
	LGL.WiimoteSemaphore->Unlock();

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
			exit(0);
		}
		
		//Keyboard
		
		if(event.type==SDL_KEYDOWN)
		{
			if(event.key.keysym.sym > 256)
			{
				event.key.keysym.sym = 256 + (event.key.keysym.sym % LGL_KEY_MAX);
			}
			if(event.key.keysym.sym > 512)
			{
				event.key.keysym.sym=0;
			}

			LGL.KeyDown[event.key.keysym.sym]=true;
			LGL.KeyStroke[event.key.keysym.sym]=true;
			if
			(
				StreamCounter<255 &&
				event.key.keysym.unicode<0x80 &&
				event.key.keysym.unicode>0
			)
			{
printf("AAAAAAAA\n");
				if(isalpha((char)event.key.keysym.unicode))
				{
printf("BBBBBBBBBB\n");
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
		if(event.type==SDL_KEYUP)
		{
			if(event.key.keysym.sym > 256)
			{
				event.key.keysym.sym = 256 + (event.key.keysym.sym % LGL_KEY_MAX);
			}
			LGL.KeyDown[event.key.keysym.sym]=false;
			LGL.KeyRelease[event.key.keysym.sym]=true;
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
				LGL.MouseDX=event.motion.x/(float)LGL.VideoResolutionX-LGL.MouseX;
				LGL.MouseDY=(1.0-event.motion.y/(float)LGL.VideoResolutionY)-LGL.MouseY;
				LGL.MouseX=event.motion.x/(float)LGL.VideoResolutionX;
				LGL.MouseY=(1.0-event.motion.y/(float)LGL.VideoResolutionY);
			}
		}

		if(event.type==SDL_MOUSEBUTTONDOWN)
		{
			if(event.button.button==SDL_BUTTON_LEFT)
			{
				LGL.MouseDown[LGL_MOUSE_LEFT]=true;
				LGL.MouseStroke[LGL_MOUSE_LEFT]=true;
			}
			if(event.button.button==SDL_BUTTON_MIDDLE)
			{
				LGL.MouseDown[LGL_MOUSE_MIDDLE]=true;
				LGL.MouseStroke[LGL_MOUSE_MIDDLE]=true;
			}
			if(event.button.button==SDL_BUTTON_RIGHT)
			{
				LGL.MouseDown[LGL_MOUSE_RIGHT]=true;
				LGL.MouseStroke[LGL_MOUSE_RIGHT]=true;
			}
		}
		
		if(event.type==SDL_MOUSEBUTTONUP)
		{
			if(event.button.button==SDL_BUTTON_LEFT)
			{
				LGL.MouseDown[LGL_MOUSE_LEFT]=false;
				LGL.MouseRelease[LGL_MOUSE_LEFT]=true;
			}
			if(event.button.button==SDL_BUTTON_MIDDLE)
			{
				LGL.MouseDown[LGL_MOUSE_MIDDLE]=false;
				LGL.MouseRelease[LGL_MOUSE_MIDDLE]=true;
			}
			if(event.button.button==SDL_BUTTON_RIGHT)
			{
				LGL.MouseDown[LGL_MOUSE_RIGHT]=false;
				LGL.MouseRelease[LGL_MOUSE_RIGHT]=true;
			}
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
	return(LGL.KeyDown[key]);
}

bool
LGL_KeyStroke
(
	int	key
)
{
	if(lgl_KeySanityCheck(key)==false) return(false);
	return(LGL.KeyStroke[key]);
}

bool
LGL_KeyRelease
(
	int	key
)
{
	if(lgl_KeySanityCheck(key)) return(false);
	return(LGL.KeyRelease[key]);
}

const
char*
LGL_KeyStream()
{
	return(LGL.KeyStream);
}

//Mouse

void
LGL_MouseSanityCheck
(
	int	button
)
{
	if(button<0 || button>=3)
	{
		printf
		(
			"LGL_MouseSanityCheck(): Invalid parameter (%i)\n",
			button
		);
		exit(-1);
	}
}

float
LGL_MouseX()
{
	return
	(
		(LGL.MouseX-LGL.ScreenViewPortLeft[LGL.ScreenNow])/
		(LGL.ScreenViewPortRight[LGL.ScreenNow]-LGL.ScreenViewPortLeft[LGL.ScreenNow])
	);
}

float
LGL_MouseY()
{
	return
	(
		(LGL.MouseY-LGL.ScreenViewPortBottom[LGL.ScreenNow])/
		(LGL.ScreenViewPortTop[LGL.ScreenNow]-LGL.ScreenViewPortBottom[LGL.ScreenNow])
	);
}

float
LGL_MouseDX()
{
	return
	(
		(LGL.MouseDX-LGL.ScreenViewPortLeft[LGL.ScreenNow])/
		(LGL.ScreenViewPortRight[LGL.ScreenNow]-LGL.ScreenViewPortLeft[LGL.ScreenNow])
	);
}

float
LGL_MouseDY()
{
	return
	(
		(LGL.MouseDY-LGL.ScreenViewPortBottom[LGL.ScreenNow])/
		(LGL.ScreenViewPortTop[LGL.ScreenNow]-LGL.ScreenViewPortBottom[LGL.ScreenNow])
	);
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
	LGL_MouseSanityCheck(button);
	return(LGL.MouseDown[button]);
}

bool
LGL_MouseStroke
(
	int	button
)
{
	LGL_MouseSanityCheck(button);
	return(LGL.MouseStroke[button]);
}

bool
LGL_MouseRelease
(
	int	button
)
{
	LGL_MouseSanityCheck(button);
	return(LGL.MouseRelease[button]);
}

void
LGL_MouseVisible
(
	bool	visible
)
{
	SDL_SelectMouse(0);
	if(visible)
	{
		SDL_ShowCursor(1);
	}
	else
	{
		SDL_ShowCursor(0);
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
		(int)(x*LGL.VideoResolutionX),
		(int)((1.0-y)*LGL.VideoResolutionY)
	);
	LGL.MouseDX=x-LGL.MouseX;
	LGL.MouseDY=x-LGL.MouseY;
	LGL.MouseX=x;
	LGL.MouseY=y;
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
		exit(-1);
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
	exit(0);
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
		exit(0);
	}
	if(Joystick<-1)
	{
		printf("lgl_JoySanityCheck(): Joysticks %i is invalid! You can't have a negative joystick (except -1, which means any)!\n",Joystick);
		exit(0);
	}
	if(Button>=32 || Button<0)
	{
		printf("lgl_JoySanityCheck(): There is no button %i!\n",Button);
		exit(0);
	}
	if(Side<0 || Side>1)
	{
		printf("lgl_JoySanityCheck(): Invalid Side parameter (%i)\n",Side);
		exit(0);
	}
	if(Axis<0 || Axis>1)
	{
		printf("lgl_JoySanityCheck(): Invalid Axis parameter (%i)\n",Axis);
		exit(0);
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
	WiimoteSemaphore.Lock("?","Calling cwiid_close()");
	{
		cwiid_close(Wiimote);
		Wiimote=NULL;
	}
	WiimoteSemaphore.Unlock();
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
	LGL_Semaphore* semaphore = wiimote->INTERNAL_GetWiimoteSemaphore();
	semaphore->Lock("?","Calling cwiid_command() from lgl_Wiimote_SetRumble()");
	{
		cwiid_command(wiimote->INTERNAL_GetWiimote(), CWIID_CMD_RUMBLE, wiimote->GetRumble());
	}
	semaphore->Unlock();
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
		WiimoteSemaphore.Lock("Main","Calling cwiid_command() from LGL_Wiimote::SetRumble()");
		{
			cwiid_command(Wiimote, CWIID_CMD_RUMBLE, Rumble);
		}
		WiimoteSemaphore.Unlock();
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

		WiimoteSemaphore.Lock("Main","Calling cwiid_command() from LGL_Wiimote::SetLED()");
		{
			cwiid_command(Wiimote, CWIID_CMD_LED, ledState);
		}
		WiimoteSemaphore.Unlock();
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
#ifdef	LGL_LINUX_WIIMOTE
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
		ButtonArraySemaphore.Lock("Wiimote Callback","INTERNAL_Callback() calling INTERNAL_UpdateButton()");
		{
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
		ButtonArraySemaphore.Unlock();
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
			
			PointerMotionSemaphore.Lock("INTERNAL_Callback()","PointerMotionThisFrameBack.push_back()");
			{
				PointerMotionThisFrameBack.push_back(PointerBack);
			}
			PointerMotionSemaphore.Unlock();
			
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

	PointerMotionSemaphore.Lock("INTERNAL_ProcessInput()","Moving PointerMotionThisFrameBack to PointerMotionThisFrameFront");
	{
		PointerMotionThisFrameFront.clear();
		PointerMotionThisFrameFront=PointerMotionThisFrameBack;
		PointerMotionThisFrameBack.clear();
	}
	PointerMotionSemaphore.Unlock();

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

	if(LGL.WiimoteSemaphore) LGL.WiimoteSemaphore->Lock("?","Reset()");
	for(int a=0;a<11;a++)
	{
		ButtonDownArrayFront[a]=false;
		ButtonStrokeArrayFront[a]=false;
		ButtonReleaseArrayFront[a]=false;

		ButtonDownArrayBack[a]=false;
		ButtonStrokeArrayBack[a]=false;
		ButtonReleaseArrayBack[a]=false;
	}
	if(LGL.WiimoteSemaphore) LGL.WiimoteSemaphore->Unlock();

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

					device->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (1)");
					{
						device->KnobTweakBack[knobWhich]=true;
						device->KnobStatusBack[knobWhich]=knobValue;
					}
					device->BackBufferSemaphore.Unlock();
				}
				else if(message[0]==177)
				{
					//Right Knob
					int knobWhich=message[1]+100;
					unsigned char knobValue=message[2];
					device->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (2)");
					{
						device->KnobTweakBack[knobWhich]=true;
						device->KnobStatusBack[knobWhich]=knobValue;
					}
					device->BackBufferSemaphore.Unlock();
				}
				else if(message[0]==224)
				{
					//Left Pitchbend
					unsigned char knobWhich=30;	//o_0
					unsigned char knobValue=message[2];
					device->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (3)");
					{
						device->KnobTweakBack[knobWhich]=true;
						device->KnobStatusBack[knobWhich]=knobValue;
					}
					device->BackBufferSemaphore.Unlock();
				}
				else if(message[0]==225)
				{
					//Right Pitchbend
					int knobWhich=30+100;	//o_0
					unsigned char knobValue=message[2];
					device->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (4)");
					{
						device->KnobTweakBack[knobWhich]=true;
						device->KnobStatusBack[knobWhich]=knobValue;
					}
					device->BackBufferSemaphore.Unlock();
				}
				else if(message[0]==178)
				{
					//Xfader or touchpad or Cue
					if(message[1]==7)
					{
						//Xfader
						unsigned char knobWhich=91;
						unsigned char knobValue=message[2];
						device->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (5)");
						{
							device->KnobTweakBack[knobWhich]=true;
							device->KnobStatusBack[knobWhich]=knobValue;
						}
						device->BackBufferSemaphore.Unlock();
					}
					else if(message[1]==8)
					{
						//Touchpad X
						unsigned char knobWhich=92;
						unsigned char knobValue=message[2];
						device->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (6)");
						{
							device->KnobTweakBack[knobWhich]=true;
							device->KnobStatusBack[knobWhich]=knobValue;
						}
						device->BackBufferSemaphore.Unlock();
					}
					else if(message[1]==9)
					{
						//Touchpad Y
						unsigned char knobWhich=93;
						unsigned char knobValue=message[2];
						device->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (7)");
						{
							device->KnobTweakBack[knobWhich]=true;
							device->KnobStatusBack[knobWhich]=knobValue;
						}
						device->BackBufferSemaphore.Unlock();
					}
					else if(message[1]==13)
					{
						unsigned char knobWhich=90;
						unsigned char knobValue=message[2];
						device->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (8)");
						{
							device->KnobTweakBack[knobWhich]=true;
							device->KnobStatusBack[knobWhich]=knobValue;
						}
						device->BackBufferSemaphore.Unlock();
					}
				}
				else if(message[0]==144)
				{
					//Left Button Stroke
					int button = message[1];
					device->BackBufferSemaphore.Lock("Main","ProcessInput() and buttons (1)");
					{
						device->ButtonStrokeBack[button]=true;
						device->ButtonForceBack[button]=1.0f;
						device->ButtonDownBack[button]=true;
						device->ButtonReleaseBack[button]=false;
					}
					device->BackBufferSemaphore.Unlock();
				}
				else if(message[0]==128)
				{
					//Left Button Release
					int button = message[1];
					device->BackBufferSemaphore.Lock("Main","ProcessInput() and buttons (2)");
					{
						device->ButtonStrokeBack[button]=false;
						device->ButtonForceBack[button]=0.0f;
						device->ButtonDownBack[button]=false;
						device->ButtonReleaseBack[button]=true;
					}
					device->BackBufferSemaphore.Unlock();
				}
				else if(message[0]==145)
				{
					//Right Button Stroke
					int button = message[1] + 100;
					device->BackBufferSemaphore.Lock("Main","ProcessInput() and buttons (3)");
					{
						device->ButtonStrokeBack[button]=true;
						device->ButtonForceBack[button]=1.0f;
						device->ButtonDownBack[button]=true;
						device->ButtonReleaseBack[button]=false;
					}
					device->BackBufferSemaphore.Unlock();
				}
				else if(message[0]==129)
				{
					//Right Button Release
					int button = message[1] + 100;
					device->BackBufferSemaphore.Lock("Main","ProcessInput() and buttons (4)");
					{
						device->ButtonStrokeBack[button]=false;
						device->ButtonForceBack[button]=0.0f;
						device->ButtonDownBack[button]=false;
						device->ButtonReleaseBack[button]=true;
					}
					device->BackBufferSemaphore.Unlock();
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
					LGL_GetXsession()->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (9)");
					{
						LGL_GetXsession()->KnobTweakBack[knobWhich]=true;
						LGL_GetXsession()->KnobStatusBack[knobWhich]=knobValue;
					}
					LGL_GetXsession()->BackBufferSemaphore.Unlock();
				}
				else if(message[0]==144)
				{
					//Button
					int button = message[1];
					bool down = (message[2]==127);
					LGL_GetXsession()->BackBufferSemaphore.Lock("Main","ProcessInput() and buttons (5)");
					{
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
					LGL_GetXsession()->BackBufferSemaphore.Unlock();
				}
			}
		}

		signal(SIGINT,oldie);
	}

	return(0);
}

#if 0
int lgl_MidiUpdate()
{
#ifndef	LGL_OSX
	unsigned char readPacket[4];
	int knobNow=-1;
	int knobValue=-1;
#endif	//LGL_OSX

#ifndef	LGL_OSX
	for(;;)
	{
		int result = read(LGL.MidiFD, &readPacket, sizeof(readPacket));
		if(result==-1)
		{
			return(0);
		}
		else if
		(
			readPacket[0] == 2 ||	//Begin message-group
			readPacket[0] == 5	//Begin message
		)
		{
			if(readPacket[0] == 2)
			{
				read(LGL.MidiFD, &readPacket, sizeof(readPacket));
			}
			unsigned int device=readPacket[2];
			//printf("Dev: %s (%i)\n",LGL_MidiDeviceName(device),readPacket[1]);
			if
			(
				device < LGL_MidiDeviceCount() &&
				strcmp(LGL_MidiDeviceName(device),"Xponent MIDI 1")==0
			)
			{
				//Xponent!
				if(readPacket[1]==176)
				{
					//Left Knob
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					knobNow=readPacket[1];
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					assert(knobNow>=0 && readPacket[0] == SEQ_MIDIPUTC);
					if(readPacket[1]!=knobValue)
					{
						//printf("L Knob %i = %i\n",knobNow,readPacket[1]);
						knobValue=readPacket[1];

						LGL_GetXponent()->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (1)");
						{
							LGL_GetXponent()->KnobTweakBack[knobNow]=true;
							LGL_GetXponent()->KnobStatusBack[knobNow]=knobValue;
						}
						LGL_GetXponent()->BackBufferSemaphore.Unlock();
					}
				}
				else if(readPacket[1]==177)
				{
					//Right Knob
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					knobNow=readPacket[1]+100;
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					assert(knobNow>=0 && readPacket[0] == SEQ_MIDIPUTC);
					if(readPacket[1]!=knobValue)
					{
						//printf("R Knob %i = %i\n",knobNow,readPacket[1]);
						knobValue=readPacket[1];
						LGL_GetXponent()->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (2)");
						{
							LGL_GetXponent()->KnobTweakBack[knobNow]=true;
							LGL_GetXponent()->KnobStatusBack[knobNow]=knobValue;
						}
						LGL_GetXponent()->BackBufferSemaphore.Unlock();
					}
				}
				else if(readPacket[1]==224)
				{
					//Left Pitchbend
					knobNow=30;
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));	//Junk?
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					assert(knobNow>=0 && readPacket[0] == SEQ_MIDIPUTC);
					if(readPacket[1]!=knobValue)
					{
						//printf("L Pitchbend %i = %i\n",knobNow,readPacket[1]);
						knobValue=readPacket[1];
						LGL_GetXponent()->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (3)");
						{
							LGL_GetXponent()->KnobTweakBack[knobNow]=true;
							LGL_GetXponent()->KnobStatusBack[knobNow]=knobValue;
						}
						LGL_GetXponent()->BackBufferSemaphore.Unlock();
					}
				}
				else if(readPacket[1]==225)
				{
					//Right Pitchbend
					knobNow=30+100;
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));	//Junk?
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					assert(knobNow>=0 && readPacket[0] == SEQ_MIDIPUTC);
					if(readPacket[1]!=knobValue)
					{
						//printf("R Pitchbend %i = %i\n",knobNow,readPacket[1]);
						knobValue=readPacket[1];
						LGL_GetXponent()->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (4)");
						{
							LGL_GetXponent()->KnobTweakBack[knobNow]=true;
							LGL_GetXponent()->KnobStatusBack[knobNow]=knobValue;
						}
						LGL_GetXponent()->BackBufferSemaphore.Unlock();
					}
				}
				else if(readPacket[1]==178)
				{
					//Xfader or touchpad or Cue
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					knobNow=readPacket[1];
					if(readPacket[1]==7)
					{
						//Xfader
						knobNow=91;
						read(LGL.MidiFD, &readPacket, sizeof(readPacket));
						if(readPacket[1]!=knobValue)
						{
							//printf("Xfade %i\n",readPacket[1]);
							knobValue=readPacket[1];
							LGL_GetXponent()->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (5)");
							{
								LGL_GetXponent()->KnobTweakBack[knobNow]=true;
								LGL_GetXponent()->KnobStatusBack[knobNow]=knobValue;
							}
							LGL_GetXponent()->BackBufferSemaphore.Unlock();
						}
					}
					else if(readPacket[1]==8)
					{
						//Touchpad X
						read(LGL.MidiFD, &readPacket, sizeof(readPacket));
						knobNow=92;
						knobValue=readPacket[1];
						LGL_GetXponent()->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (6)");
						{
							LGL_GetXponent()->KnobTweakBack[knobNow]=true;
							LGL_GetXponent()->KnobStatusBack[knobNow]=knobValue;
						}
						LGL_GetXponent()->BackBufferSemaphore.Unlock();

						//Touchpad Y
						read(LGL.MidiFD, &readPacket, sizeof(readPacket));
						if(readPacket[1]!=178)
						{
							printf("readPacket[1]: %i\n",readPacket[1]);
							read(LGL.MidiFD, &readPacket, sizeof(readPacket));
							read(LGL.MidiFD, &readPacket, sizeof(readPacket));
							//assert(readPacket[1]==178);
						}
						else
						{
							read(LGL.MidiFD, &readPacket, sizeof(readPacket));
							assert(readPacket[1]==9);
							read(LGL.MidiFD, &readPacket, sizeof(readPacket));
							knobNow=93;
							knobValue=readPacket[1];
							LGL_GetXponent()->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (7)");
							{
								LGL_GetXponent()->KnobTweakBack[knobNow]=true;
								LGL_GetXponent()->KnobStatusBack[knobNow]=knobValue;
							}
							LGL_GetXponent()->BackBufferSemaphore.Unlock();
						}
					}
					else if(readPacket[1]==13)
					{
						knobNow=90;
						read(LGL.MidiFD, &readPacket, sizeof(readPacket));
						if(readPacket[1]!=knobValue)
						{
							//printf("Cue %i\n",readPacket[1]);
							knobValue=readPacket[1];
							LGL_GetXponent()->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (8)");
							{
								LGL_GetXponent()->KnobTweakBack[knobNow]=true;
								LGL_GetXponent()->KnobStatusBack[knobNow]=knobValue;
							}
							LGL_GetXponent()->BackBufferSemaphore.Unlock();
						}
					}
				}
				else if(readPacket[1]==144)
				{
					//Left Button Stroke
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					//printf("L Button Stroke: %i\n",readPacket[1]);
					int button = readPacket[1];
					LGL_GetXponent()->BackBufferSemaphore.Lock("Main","ProcessInput() and buttons (1)");
					{
						LGL_GetXponent()->ButtonStrokeBack[button]=true;
						LGL_GetXponent()->ButtonForceBack[button]=1.0f;
						LGL_GetXponent()->ButtonDownBack[button]=true;
						LGL_GetXponent()->ButtonReleaseBack[button]=false;
					}
					LGL_GetXponent()->BackBufferSemaphore.Unlock();
				}
				else if(readPacket[1]==128)
				{
					//Left Button Release
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					//printf("L Button Release: %i\n",readPacket[1]);
					int button = readPacket[1];
					LGL_GetXponent()->BackBufferSemaphore.Lock("Main","ProcessInput() and buttons (2)");
					{
						LGL_GetXponent()->ButtonStrokeBack[button]=false;
						LGL_GetXponent()->ButtonForceBack[button]=0.0f;
						LGL_GetXponent()->ButtonDownBack[button]=false;
						LGL_GetXponent()->ButtonReleaseBack[button]=true;
					}
					LGL_GetXponent()->BackBufferSemaphore.Unlock();
				}
				else if(readPacket[1]==145)
				{
					//Right Button Stroke
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					//printf("R Button Stroke: %i\n",readPacket[1]);
					int button = readPacket[1] + 100;
					LGL_GetXponent()->BackBufferSemaphore.Lock("Main","ProcessInput() and buttons (3)");
					{
						LGL_GetXponent()->ButtonStrokeBack[button]=true;
						LGL_GetXponent()->ButtonForceBack[button]=1.0f;
						LGL_GetXponent()->ButtonDownBack[button]=true;
						LGL_GetXponent()->ButtonReleaseBack[button]=false;
					}
					LGL_GetXponent()->BackBufferSemaphore.Unlock();
				}
				else if(readPacket[1]==129)
				{
					//Right Button Release
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					//printf("R Button Release: %i\n",readPacket[1]);
					int button = readPacket[1] + 100;
					LGL_GetXponent()->BackBufferSemaphore.Lock("Main","ProcessInput() and buttons (4)");
					{
						LGL_GetXponent()->ButtonStrokeBack[button]=false;
						LGL_GetXponent()->ButtonForceBack[button]=0.0f;
						LGL_GetXponent()->ButtonDownBack[button]=false;
						LGL_GetXponent()->ButtonReleaseBack[button]=true;
					}
					LGL_GetXponent()->BackBufferSemaphore.Unlock();
				}
				else
				{
					//printf("Unexpected value: %i\n",readPacket[1]);
				}
			}
			else if
			(
				device < LGL_MidiDeviceCount() &&
				strcmp(LGL_MidiDeviceName(device),"USB X-Session MIDI 1")==0
			)
			{
				//Xsession!

				if(readPacket[1]==176)
				{
					//Knob
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					knobNow=readPacket[1];
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					assert(knobNow>=0 && readPacket[0] == SEQ_MIDIPUTC);
					if(readPacket[1]!=knobValue)
					{
						//printf("Knob %i = %i\n",knobNow,readPacket[1]);
						knobValue=readPacket[1];

						LGL_GetXsession()->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (9)");
						{
							LGL_GetXsession()->KnobTweakBack[knobNow]=true;
							LGL_GetXsession()->KnobStatusBack[knobNow]=knobValue;
						}
						LGL_GetXsession()->BackBufferSemaphore.Unlock();
					}
				}
				else if(readPacket[1]==144)
				{
					//Button
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					//printf("Button %i\n",readPacket[1]);
					int button = readPacket[1];

					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					//printf("State %i\n",readPacket[1]);
					bool down = (readPacket[1]==127);
					LGL_GetXsession()->BackBufferSemaphore.Lock("Main","ProcessInput() and buttons (5)");
					{
						bool wasDown=LGL_GetXsession()->ButtonDownBack[button];
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
					LGL_GetXsession()->BackBufferSemaphore.Unlock();
				}
			}
			else if
			(
				device < LGL_MidiDeviceCount() &&
				strcmp(LGL_MidiDeviceName(device),"USB Trigger Finger MIDI 1")==0
			)
			{
				/*
				printf("TF[0]:\t%i\t%i\t%i\t%i\n",
					readPacket[0],
					readPacket[1],
					readPacket[2],
					readPacket[3]
				);
				for(int a=0;a<2;a++)
				{
					printf("TF[%i]:\t%i\t%i\t%i\t%i\n",
						a+1,
						readPacket[0],
						readPacket[1],
						readPacket[2],
						readPacket[3]
					);
				}
				*/

				//TriggerFinger!
printf("readPacket[1]: %i\n",readPacket[1]);
				if(readPacket[1]==153)
				{
					//Button
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					int button = readPacket[1];
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					int force = readPacket[1];
printf("button %i @ %i\n",button,force);
/*
					LGL_GetTriggerFinger()->BackBufferSemaphore.Lock();
					{
						LGL_GetTriggerFinger()->ButtonStrokeBack[button]=(force>0);
						LGL_GetTriggerFinger()->ButtonForceBack[button]=force;
						LGL_GetTriggerFinger()->ButtonDownBack[button]=(force>0);
						LGL_GetTriggerFinger()->ButtonReleaseBack[button]=(force==0);
					}
					LGL_GetTriggerFinger()->BackBufferSemaphore.Unlock();
*/
				}
				else if(readPacket[1]==185)
				{
					//Knob / Slider
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					int knob = readPacket[1];
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					int value = readPacket[1];
printf("knob %i @ %i\n",knob,value);
					LGL_GetTriggerFinger()->BackBufferSemaphore.Lock("Main","ProcessInput() and tweaking knobs (10)");
					{
						LGL_GetTriggerFinger()->KnobTweakBack[knob]=true;
						LGL_GetTriggerFinger()->KnobStatusBack[knob]=value;
					}
					LGL_GetTriggerFinger()->BackBufferSemaphore.Unlock();
				}
				else
				{
					//
				}
			}
			else if
			(
				device < LGL_MidiDeviceCount() &&
				strcmp(LGL_MidiDeviceName(device),"USB Uno MIDI Interface MIDI 1")==0
			)
			{
				//JP8k!
				if(readPacket[1]==254)
				{
					//Tick: Useless
				}
				else if(readPacket[1]==144)
				{
					//Key Stroke!
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					int button = readPacket[1];
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					int force = readPacket[1];

					LGL_GetJP8k()->BackBufferSemaphore.Lock("Main","ProcessInput() and buttons (6)");
					{
						LGL_GetJP8k()->ButtonStrokeBack[button]=true;
						LGL_GetJP8k()->ButtonForceBack[button]=(force+1.0f)/128.0f;
						LGL_GetJP8k()->ButtonDownBack[button]=true;
						LGL_GetJP8k()->ButtonReleaseBack[button]=false;
					}
					LGL_GetJP8k()->BackBufferSemaphore.Unlock();
				}
				else if(readPacket[1]==144)
				{
					//Key Stroke echo
				}
				else if(readPacket[1]==128)
				{
					//Key Release!
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					int button = readPacket[1];
					read(LGL.MidiFD, &readPacket, sizeof(readPacket));
					int force = readPacket[1];

					LGL_GetJP8k()->BackBufferSemaphore.Lock("Main","ProcessInput() and buttons (7)");
					{
						LGL_GetJP8k()->ButtonStrokeBack[button]=false;
						LGL_GetJP8k()->ButtonForceBack[button]=(force+1.0f)/128.0f;
						LGL_GetJP8k()->ButtonDownBack[button]=false;
						LGL_GetJP8k()->ButtonReleaseBack[button]=true;
					}
					LGL_GetJP8k()->BackBufferSemaphore.Unlock();
				}
				else if(readPacket[1]==129)
				{
					//Key Release echo
				}
				else
				{
					printf("JP8k!\t\t%i\t%i\t%i\t%i\n",readPacket[0],readPacket[1],readPacket[2],readPacket[3]);
				}
			}
			else
			{
				printf("What. (Strange MIDI sequence)\n");
			}
		}
	}
#endif	//LGL_OSX

	return(0);
}
#endif

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
	if(which < 0 || which >= LGL_MidiDeviceCount())
	{
		printf("LGL_MidiDeviceName(%i): Error! [0 < arg < %i] violated!\n",which,LGL_MidiDeviceCount());
		assert(which<LGL_MidiDeviceCount());
	}
	return(LGL.MidiRtInDeviceNames[which]);
}



//LGL_MidiDevice

LGL_MidiDevice::
LGL_MidiDevice() : BackBufferSemaphore("MidiDevice BackBuffer")
{
	DeviceID=-1;

	BackBufferSemaphore.Lock("Main","Iinitializing Buttons, knobs");
	{
		for(int a=0;a<LGL_MIDI_CONTROL_MAX;a++)
		{
			ButtonStrokeFront[a]=false;
			ButtonStrokeBack[a]=false;
			ButtonDownFront[a]=false;
			ButtonDownBack[a]=false;
			ButtonReleaseFront[a]=false;
			ButtonReleaseBack[a]=false;
			ButtonForceFront[a]=0.0f;
			ButtonForceBack[a]=0.0f;
			KnobTweakFront[a]=false;
			KnobTweakBack[a]=false;
			KnobStatusFront[a]=-127.0f;
			KnobStatusBack[a]=-127.0f;
		}
	}
	BackBufferSemaphore.Unlock();
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
	BackBufferSemaphore.Lock("Main","LGL_INTERNAL_SwapBuffers()");
	{
		for(int a=0;a<LGL_MIDI_CONTROL_MAX;a++)
		{
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
	BackBufferSemaphore.Unlock();
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
		exit(0);
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
		exit(0);
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
		exit(0);
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
		exit(0);
	}
	if(IP[0]==-1)
	{
		Connection_Status=-1;
		return(false);
	}
	if(strcmp(Host,"Server")==0)
	{
		printf("LGL_NetConnection::ConnectTCP(%i): Error! Servers can't establish connections!\n",port);
		exit(0);
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
		exit(0);
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
		exit(0);
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
		exit(0);
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
		LGL_Assertf(false,("LGL_DirTree::SetPath(): Error! For path '%s', absPath '%s' isn't a directory!",path,absPath));
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

	std::vector<char*> everything=LGL_DirectoryListCreate(Path,false);

	for(unsigned int a=0;a<everything.size();a++)
	{
		char check[2048];

		sprintf(check,"%s/%s",Path,everything[a]);
		
		if(LGL_DirectoryExists(check))
		{
			//It's a directory

			DirList.push_back(everything[a]);
		}
		else
		{
			//It's a file

			FileList.push_back(everything[a]);
		}
	}

	everything.clear();

	GenerateFilterLists();

	WorkerThreadDone=true;
}

unsigned int
LGL_DirTree::
GetFileCount()
{
	WaitOnWorkerThread();
	return(FileList.size());
}

unsigned int
LGL_DirTree::
GetDirCount()
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
)
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
)
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
WaitOnWorkerThread()
{
	if(WorkerThread)
	{
		LGL_ThreadWait(WorkerThread);
		WorkerThread=NULL;
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

void
lgl_MergeSortDirectoryList
(
	std::vector<char*>*	list,
	int			begin,
	int			end
)
{
	if(begin<end)
	{
		//Sort both halves

		int middle=(begin+end)/2;
		lgl_MergeSortDirectoryList(list,begin,middle);
		lgl_MergeSortDirectoryList(list,middle+1,end);

		std::vector<char*>& List=(*list);

		std::vector<char*> GoodList;
		int counter1=begin;
		int counter2=middle+1;
		int counter3=begin;
		
		while(counter1<=middle && counter2<=end)
		{
			if(strcasecmp(List[counter1],List[counter2])<=0)
			{
				GoodList.push_back(List[counter1]);
				counter1++;
			}
			else
			{
				GoodList.push_back(List[counter2]);
				counter2++;
			}
			counter3++;
		}
		while(counter1<=middle)
		{
			GoodList.push_back(List[counter1]);
			counter1++;
			counter3++;
		}
		while(counter2<=end)
		{
			GoodList.push_back(List[counter2]);
			counter2++;
			counter3++;
		}

		int baka=0;
		for(int a=begin;a<=end;a++)
		{
			List[a]=GoodList[baka];
			baka++;
		}
		GoodList.clear();
	}
}

LGL_FileToMemory::
LGL_FileToMemory
(
	const char*	path
)
{
	Path[0]='\0';
	FileDescriptor=-1;
	Pointer=NULL;
	Size=0;
	Status=-1;

	LoadFile(path);
}

LGL_FileToMemory::
~LGL_FileToMemory()
{
	LoadFile(NULL);
}

void
LGL_FileToMemory::
LoadFile
(
	const char*	path
)
{
	if(path==NULL || strlen(path)>=2040)
	{
printf("Load File Omega A\n");
		//Unload
		Path[0]='\0';
		if(FileDescriptor!=-1)
		{
			close(FileDescriptor);
			FileDescriptor=-1;
		}
		if(Pointer!=NULL)
		{
			free(Pointer);
			Pointer=NULL;
		}
		Size=0;
		Status=-1;
		return;
	}

	strcpy(Path,path);

	FILE* fd=fopen(Path,"r");
printf("Loading '%s'\n",Path);
	if(fd)
	{
		fseek(fd,0,SEEK_END);
		Size = ftell(fd);
		Pointer = malloc(Size);
		fclose(fd);
	}
	else
	{
printf("Load File Omega B\n");
		LoadFile(NULL);
		return;
	}

	FileDescriptor = open
	(
		Path,
		O_RDONLY | O_NONBLOCK | O_ASYNC
	);
	perror("open");
	printf("FD: %i\n",FileDescriptor);
	if(FileDescriptor==-1)
	{
		perror("bad fd");
		LoadFile(NULL);
		return;
	}
	printf("read alpha\n");
	int result = read
	(
		FileDescriptor,
		Pointer,
		Size
	);
	perror("read");
	printf("read omega\n");
	if(result==-1)
	{
		Status=-1;
	}
	else
	{
		Status=0;
	}
}

bool
LGL_FileToMemory::
GetReady()
{
	if(FileDescriptor==-1)
	{
		printf("Fail A\n");
		return(false);
	}

	if(Status==-1)
	{
		printf("Fail B\n");
		return(false);
	}
	else if(Status==1)
	{
		return(true);
	}
	else
	{
		fd_set setRead;
		fd_set setWrite;
		fd_set setException;
		FD_ZERO(&setRead);
		FD_ZERO(&setWrite);
		FD_ZERO(&setException);
		FD_SET(FileDescriptor,&setRead);
		struct timeval timeVal;
		timeVal.tv_sec=0;
		timeVal.tv_usec=0;
		printf("Select Start!\n");
		select(FD_SETSIZE,&setRead,&setWrite,&setException,&timeVal);
		printf("Select Done!\n");
		bool result = FD_ISSET(FileDescriptor,&setRead);
		if(result==false)
		{
			//Still reading...
		}
		else
		{
			//We're done!
			Status=1;
		}

		return(Status==1);
	}
}

bool
LGL_FileToMemory::
GetFailed()
{
	GetReady();
	return(Status==-1);
}

void*
LGL_FileToMemory::
GetPointer()
{
	if(GetReady())
	{
		return(Pointer);
	}
	else
	{
		return(NULL);
	}
}

long
LGL_FileToMemory::
GetSize()
{
	return(Size);
}

bool
LGL_FileExists
(
	const
	char*	file
)
{
#ifndef	LGL_WIN32
	FILE* f=fopen64(file,"r");
	if(f==NULL)
	{
		return(false);
	}
	else
	{
		fclose(f);
		return(true);
	}
#else	//LGL_WIN32
	WIN32_FIND_DATA	findData;
	HANDLE		handle;
	handle=FindFirstFile(file, &findData);
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

std::vector<char*>
LGL_DirectoryListCreate
(
	const
	char*	targetDir,
	bool	justFiles,
	bool	seeHidden
)
{
	std::vector<char*> ReturnMe;

#ifdef	LGL_LINUX

	assert(targetDir!=NULL);
	DIR* myDir=opendir(targetDir);
	if(myDir==NULL)
	{
		printf("LGL_DirectoryListCreate(): Error! opendir(%s) failed...\n",targetDir);
		assert(false);
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

			if(ret==-1)
			{
				printf("LGL_DirectoryListCreate(): Error! stat(%s) returns NULL or -1.\n",temp);
				assert(false);
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
				ReturnMe.push_back(temp2);
			}
		}
	}
	closedir(myDir);

	if(ReturnMe.empty()==false)
	{
		lgl_MergeSortDirectoryList(&ReturnMe,0,ReturnMe.size()-1);
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
			ReturnMe.push_back(neo);
		}
		if(FindNextFile(handle,&findData)==0)
		{
			break;
		}
	}
	FindClose(handle);
	if(ReturnMe.empty()==false)
	{
		lgl_MergeSortDirectoryList(&ReturnMe,0,ReturnMe.size()-1);
	}
#endif	//LGL_WIN32
	return(ReturnMe);
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
	struct stat64 stbuf;
	if(stat64(file, &stbuf) < 0)
	{
		printf("Can't stat() %s\n",file);
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
	MD5state *nil=NULL;

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

	s = nil;
	n = 0;
	buf = (byte*)calloc(256,64);
	for(;;){
		i = fread(buf+n, 1, 128*64-n, fd);
		if(i <= 0)
			break;
		n += i;
		if(n & 0x3f)
			continue;
		s = md5(buf, n, 0, s, nil, tab);
		n = 0;
	}
	md5(buf, n, digest, s, nil, tab);

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
md5(byte *p, uint len, byte *digest, MD5state *s, MD5state *nil, Table* tab)
{
	uint a, b, c, d, tmp;
	uint i, done;
	Table *t;
	byte *end;
	uint x[16];

	if(s == nil){
		s = (MD5state*)calloc(sizeof(*s),1);
		if(s == nil)
			return nil;

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
		return nil;
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

//Memory

//Misc

float
LGL_MemoryFreePercent()
{
#ifdef	LGL_LINUX
	float memTotal=-1.0f;
	float memFree=-1.0f;
	float memCached=-1.0f;
	if(FILE* fd=fopen("/proc/meminfo","r"))
	{
		char buf[2048];
		while(feof(fd)==false)
		{
			fgets(buf,2048,fd);
			if(strstr(buf,"MemTotal:"))
			{
				memTotal=atof(&(strchr(buf,':')[1]));
			}
			else if(strstr(buf,"MemFree:"))
			{
				memFree=atof(&(strchr(buf,':')[1]));
			}
			else if(strstr(buf,"Cached:")==buf)
			{
				memCached=atof(&(strchr(buf,':')[1]));
			}
		}
		fclose(fd);
	}
	if
	(
		memTotal>=0 &&
		memFree>=0 &&
		memCached>=0
	)
	{
		return((memFree+memCached)/memTotal);
	}
	else
	{
		return(1.0f);
	}
#else	//LGL_LINUX
	return(1.0f);
#endif	//LGL_LINUX
}

bool
LGL_BatteryChargeDraining()
{
#ifdef	LGL_LINUX
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
#ifdef	LGL_LINUX
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
	for(int a=0;a<60;a++)
	{
		if(LGL.FPSGraph[a]>=50)
		{
			r=0;
			g=.5*brightness;
			b=0;
		}
		else if(LGL.FPSGraph[a]>=30)
		{
			r=.5*brightness;
			g=.5*brightness;
			b=0;
		}
		else
		{
			r=.5*brightness;
			g=0;
			b=0;
		}

		LGL_DrawRectToScreen
		(
			left+(right-left)*(a+0)/60.0,
			left+(right-left)*(a+1)/60.0,
			bottom,
			bottom+(top-bottom)*LGL_Min(100,LGL.FPSGraph[a])/100.0,
			r,g,b,alpha
		);
	}
	LGL_DrawLineToScreen
	(
		left,
		bottom+(top-bottom)*60.0/100.0,
		right,
		bottom+(top-bottom)*60.0/100.0,
		0,brightness,0,.5*alpha
	);
	LGL_DrawLineToScreen
	(
		left,
		bottom+(top-bottom)*30.0/100.0,
		right,
		bottom+(top-bottom)*30.0/100.0,
		brightness,brightness,0,.5*alpha
	);
	
	char temp[1024];
	sprintf(temp,"%i",LGL.FPS);

	LGL_GetFont().DrawString
	(
		.5*(left+right),
		.25*bottom+.75*top-.05*(top-bottom)-.05*(right-left),
		.10*(top-bottom)+.10*(right-left),
		.75*brightness,.75*brightness,.75*brightness,.75*alpha,
		true,.5*alpha,
		temp
	);
	
	int lowest=999;
	for(int a=0;a<60;a++)
	{
		lowest=(int)LGL_Min(lowest,LGL.FPSGraph[a]);
	}
	sprintf(temp,"%i",lowest);
	LGL_GetFont().DrawString
	(
		.5*(left+right),
		.75*bottom+.25*top-.05*(top-bottom)-.05*(right-left),
		.10*(top-bottom)+.10*(right-left),
		.75*brightness,.75*brightness,.75*brightness,.75*alpha,
		true,.5*alpha,
		temp
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

//FILE* mencoderFD=NULL;

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
		LGL.VideoResolutionX, LGL.VideoResolutionY, 24,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		0x000000FF, 0x0000FF00, 0x00FF0000, 0
#else
		0x00FF0000, 0x0000FF00, 0x000000FF, 0
#endif
	);
	if(temp==NULL)
	{
		printf("LGL_ScreenShot(): Error! Unable to SDL_CreateGFBSurface().\n");
		exit(-1);
	}

	pixels=(unsigned char*)malloc(3*LGL.VideoResolutionX*LGL.VideoResolutionY);
	if(pixels==NULL)
	{
		printf("LGL_ScreenShot(): Error! Unable to malloc() pixel buffer.\n");
		exit(-1);
	}

	glReadBuffer(GL_FRONT_LEFT);
	glReadPixels
	(
		0, 0,
		LGL.VideoResolutionX, LGL.VideoResolutionY,
		GL_RGB, GL_UNSIGNED_BYTE,
		pixels
	);

	for(int i=0;i<LGL.VideoResolutionY;i++)
	{
		memcpy
		(
			((char*)temp->pixels)+temp->pitch*i,
			pixels+3*LGL.VideoResolutionX*(LGL.VideoResolutionY-i-1),
			LGL.VideoResolutionX*3
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
		exit(0);
	}
		
	FILE* f=fopen(inFile,"r");
	if(f==NULL)
	{
		printf("lgl_DrawLogPlayback(%s): Error! Couldn't fopen file!\n",inFile);
		exit(0);
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
		LGL.DrawLogFD=popen
		(
			command,
			"w"
		);
		if(LGL.DrawLogFD<=0)
		{
			printf("LGL_DrawLogStart('%s'): Error! Couldn't open file...\n",outFile);
			assert(LGL.DrawLogFD>0);
		}

		char intro[1024];
		sprintf(intro,"LGL_DrawLogStart|%f\n",(LGL.RecordActive==false)?0.0f:(LGL.RecordSamplesWritten/(double)LGL.AudioSpec->freq));
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
	if(target<lowerbound) target=lowerbound;
	if(target>upperbound) target=upperbound;
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
#ifdef	LGL_LINUX
	std::vector<int> cpuSpeeds;
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
	return(cpuSpeeds);
#else	//LGL_LINUX
	printf("lgl_read_proc_cpuinfo(): Error!! Why are you calling this outside of linux??\n");
	assert(false);
	return(false);
#endif	//LGL_LINUX
}

unsigned int
LGL_CPUCount()
{
#ifdef	LGL_LINUX
	std::vector<int> cpuSpeeds = lgl_read_proc_cpuinfo();
	return(cpuSpeeds.size());
#else	//LGL_LINUX
	printf("LGL_CPUCount(): Warning! This function not yet implemented outside of linux port!\n");
	return(1);
#endif	//LGL_LINUX
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

bool
LGL_IsOsxAppBundle()
{
	char wd[2048];
	getcwd(wd,20480);
	return(strstr(wd,"Contents/MacOS"));
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
		exit(0);
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
	const char*	threadName,
	const char*	note,
	bool		blockUntilTimeout,
	float		timeoutSeconds
)
{
	if(Promiscuous) return(true);

	bool ret=false;
	if(blockUntilTimeout==false)
	{
		ret = (SDL_SemTryWait(Sem)==0);
	}
	else
	{
		if(timeoutSeconds<0)
		{
			ret = (SDL_SemWait(Sem)==0);
		}
		else
		{
			ret = (SDL_SemWaitTimeout(Sem,(Uint32)(timeoutSeconds/1000.0))==0);
		}
	}

	if(ret)
	{
		strcpy(LockOwner,threadName);
		strcpy(Note,note);
		TimeOfLock = LGL_SecondsSinceExecution();
	}
	return(ret);
}

bool
LGL_Semaphore::
Unlock()
{
	if(Promiscuous) return(true);

	return(SDL_SemPost(Sem));
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
	LGL_Semaphore&	semaphore,
	float		timeoutSeconds
)
{
	Init
	(
		&semaphore,
		timeoutSeconds
	);
}

LGL_ScopeLock::
LGL_ScopeLock
(
	LGL_Semaphore*	semaphore,
	float		timeoutSeconds
)
{
	Init
	(
		semaphore,
		timeoutSeconds
	);
}

void
LGL_ScopeLock::
Init
(
	LGL_Semaphore*	semaphore,
	float		timeoutSeconds
)
{
	Semaphore=semaphore;
	LockObtained=false;
	if(Semaphore)
	{
		LockObtained=Semaphore->Lock
		(
			"Nameless ScopeLock",
			"Noteless ScopeLock (meh)",
			timeoutSeconds!=0,
			timeoutSeconds
		);
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
float tempStreamRecL[4*2*LGL_SAMPLESIZE];
float tempStreamRecR[4*2*LGL_SAMPLESIZE];
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
	if(LGL.AudioUsingJack==false)
	{
		if(soundRealtimePrioritySet==false)
		{
			setpriority(PRIO_PROCESS,0,-20);

			//Use Only One CPU
			//LGL_ThreadSetCPUAffinity(1);
			LGL_ThreadSetPriority(LGL_PRIORITY_AUDIO_OUT,"AudioOut");	//It's really that important!

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
		tempStream16Silence=new Sint16[len16];
		for(int l=0;l<len16;l++)
		{
			tempStream16Silence[l]=LGL.AudioSpec->silence;
		}
	}
	memcpy(stream16,tempStream16Silence,len16);

//I'm not down with any locks in the audio thread
#if 0
	if(LGL.AudioStreamListSemaphore->IsLocked())
	{
		printf("Locked 1\n");
	}
	LGL.AudioStreamListSemaphore->Lock("AudioOut","Calling MixIntoStream()");
	for(unsigned int a=0;a<LGL.AudioStreamList.size();a++)
	{
		LGL.AudioStreamList[a]->MixIntoStream
		(
			userdata,
			stream8,
			len8
		);

		if(LGL.AudioStreamList[a]->Finished())
		{
			delete LGL.AudioStreamList[a];
			LGL.AudioStreamList.erase
			(
				(std::vector<LGL_AudioStream*>::iterator)(&(LGL.AudioStreamList[a]))
			);
			a--;
		}
	}
	LGL.AudioStreamListSemaphore->Unlock();
#endif


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
	double lRecord;
	double rRecord;

	Sint16* stream16rec=(Sint16*)LGL.RecordBuffer;
	memcpy(stream16rec,tempStream16Silence,len8);

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
		memcpy(tempStreamRecL,tempStreamSilenceFloat,size);
		memcpy(tempStreamRecR,tempStreamSilenceFloat,size);

		float vuNext=0.0f;
		double timeAdvancementMultiplier = sc->Hz/LGL.AudioSpec->freq;
		
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
			lRecord=LGL.AudioSpec->silence;
			rRecord=LGL.AudioSpec->silence;

			if(sc->DivergeState==0)
			{
				//Track with sound
				sc->DivergeSamples=sc->PositionSamplesNow;
			}
			else if(sc->DivergeState==-1)
			{
				//Recall!
				sc->PositionSamplesNow=sc->DivergeSamples;
				sc->DivergeState=0;
			}
			else if(sc->DivergeState==1)
			{
				//Diverge!
				sc->DivergeSamples+=
					sc->DivergeSpeed*timeAdvancementMultiplier;
			}

			if(sc->WarpPointSecondsTrigger>=0.0f)
			{
				if(sc->PositionSamplesNow>=sc->WarpPointSecondsTrigger*sc->Hz)
				{
					if
					(
						sc->WarpPointSecondsDestination >= 0 ||
						sc->LGLSound->IsLoaded()
					)
					{
						sc->PositionSamplesNow=sc->WarpPointSecondsDestination*sc->Hz;
						while(sc->PositionSamplesNow<0) sc->PositionSamplesNow+=sc->LengthSamples*sc->Hz;
						sc->SampleRateConverterBufferStartSamples=sc->PositionSamplesNow;
						sc->SampleRateConverterBufferValidSamples=0;
					}
					sc->WarpPointSecondsTrigger=-1.0f;
					sc->WarpPointSecondsDestination=-1.0f;
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
			bool GoForIt=true;
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
				GoForIt=false;
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
					sc->Buffer=NULL;
					GoForIt=false;
				}
			}
			if(sc->Buffer==NULL)
			{
				GoForIt=false;
			}
			if(GoForIt)
			{
				//Add the interpolated samples
				Sint16* myBuffer=(Sint16*)sc->Buffer;

				Sint16 Lnp=SWAP16(myBuffer[PosNowPrev*BPS_half+0]);
				Sint16 Rnp=SWAP16(myBuffer[PosNowPrev*BPS_half+1]);

				Sint16 Lnn=SWAP16(myBuffer[PosNowNext*BPS_half+0]);
				Sint16 Rnn=SWAP16(myBuffer[PosNowNext*BPS_half+1]);

				sc->SpeedNow=
					(1.0f-sc->SpeedInterpolationFactor)*sc->SpeedNow+
					(0.0f+sc->SpeedInterpolationFactor)*sc->SpeedDesired;
				if(fabs(sc->SpeedNow-sc->SpeedDesired)<.0003f)
				{
					sc->SpeedNow=sc->SpeedDesired;
				}
				
				if(sc->Glitch)
				{
					double GlitchSamplesNowPrev=floor(sc->GlitchSamplesNow);
					double GlitchSamplesNowNext=ceil(sc->GlitchSamplesNow);
					double GlitchiNow=sc->GlitchSamplesNow-GlitchSamplesNowPrev;

					double gLnn=(Sint16)(sc->GlitchVolume*SWAP16(myBuffer[(unsigned long)(GlitchSamplesNowNext*BPS/2+0)]));
					double gRnn=(Sint16)(sc->GlitchVolume*SWAP16(myBuffer[(unsigned long)(GlitchSamplesNowNext*BPS/2+1)]));

					double myFL = (gLnn*(1.0-GlitchiNow) + gLnn*GlitchiNow);
					double myFR = (gRnn*(1.0-GlitchiNow) + gRnn*GlitchiNow);

					if(sc->ToMono)
					{
						myFL=(myFL+myFR)/2.0f;
						myFR=myFL;
					}

					double myBL=myFL;
					double myBR=myFR;

					lRecord+=myFL;
					rRecord+=myFR;

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
				double myFL = (Lnp*(1.0-closenessPercentInvNow) + Lnn*closenessPercentInvNow);
				double myFR = (Rnp*(1.0-closenessPercentInvNow) + Rnn*closenessPercentInvNow);

				//Attempt to use libsamplerate
				if(fabsf(sc->SpeedNow)<2.0f)
				{
					long sampleNow=(long)sc->PositionSamplesNow;
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
							for(int c=0;c<2;c++)
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

								src_set_ratio((c==0)?sc->SampleRateConverterL:sc->SampleRateConverterR,srcData.src_ratio);
								src_process((c==0)?sc->SampleRateConverterL:sc->SampleRateConverterR,&srcData);

								for(int b=0;b<srcData.output_frames_gen;b++)
								{
									float* buf=(c==0) ?
										sc->SampleRateConverterBufferL :
										sc->SampleRateConverterBufferR;
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

					double neoFL;
					double neoFR;

					if
					(
						sc->SampleRateConverterBufferCurrentSamplesIndex>=0 &&
						sc->SampleRateConverterBufferCurrentSamplesIndex<sc->SampleRateConverterBufferValidSamples
					)
					{
						int index=sc->SampleRateConverterBufferCurrentSamplesIndex;
						sc->SampleRateConverterBufferCurrentSamplesIndex++;
						assert(index>=0 && index<SAMPLE_RATE_CONVERTER_BUFFER_SAMPLES);
						if(index<sc->SampleRateConverterBufferValidSamples)
						{
							neoFL = sc->SampleRateConverterBufferL[index];
							neoFR = sc->SampleRateConverterBufferR[index];
						}
					}
					else if(sc->SpeedNow==0.0f)
					{
						int index=LGL_Max(0,sc->SampleRateConverterBufferCurrentSamplesIndex-1);
						neoFL = sc->SampleRateConverterBufferL[index];
						neoFR = sc->SampleRateConverterBufferR[index];
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
				}
				else
				{
					if(sc->SampleRateConverterBufferStartSamples!=-1)
					{
						sc->SampleRateConverterBufferStartSamples=-1;
						//The below calls would be nice, but are way too expensive to execute in the audio thread...
						//src_reset(sc->SampleRateConverterL);
						//src_reset(sc->SampleRateConverterR);
					}
				}

				double myLStereo=myFL;
				double myRStereo=myFR;

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

				double myBL=myFL;
				double myBR=myFR;

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

				lRecord+=myLStereo;
				rRecord+=myRStereo;

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
						//FIXME: The next two lines don't quite seem right...
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

			if(lRecord<-32767) lRecord=-32767;
			if(rRecord<-32767) rRecord=-32767;

			if(lRecord>32767) lRecord=32767;
			if(rRecord>32767) rRecord=32767;

			vuNext=LGL_Max
			(
				vuNext,
				LGL_Max
				(
					fabsf(lRecord/32767.0f),
					fabsf(rRecord/32767.0f)
				)
			);

			tempStreamRecL[a/4+0]+=(Sint16)(QUIET_FACTOR*lRecord);		//Should we SWAP16 here??? I'm not sure.
			tempStreamRecR[a/4+0]+=SWAP16((Sint16)(QUIET_FACTOR*rRecord));	//So we'll split the difference, to see.

			//SAMPLE ADDING LOOP: OMEGA
		}
		sc->PositionSamplesDeltaLastTime.Reset();

		//DSP!! Woo hoo!!
		float tempStreamDSPInStereo[LGL_SAMPLESIZE*2];
		float tempStreamDSPOutStereo[LGL_SAMPLESIZE*2];
		if(sc->LGLAudioDSPFront)
		{
			//Stereo
			for(unsigned int s=0;s<LGL_SAMPLESIZE*2;s+=2)
			{
				tempStreamDSPInStereo[s]=tempStreamFL[s/2];
				tempStreamDSPInStereo[s+1]=tempStreamFR[s/2];
			}
			sc->LGLAudioDSPFront->ProcessStereo
			(
				tempStreamDSPInStereo,
				tempStreamDSPOutStereo,
				LGL_SAMPLESIZE
			);
			for(unsigned int s=0;s<LGL_SAMPLESIZE*2;s+=2)
			{
				tempStreamFL[s/2]=LGL_Clamp(-32767.0f,tempStreamDSPOutStereo[s],32767.0f);
				tempStreamFR[s/2]=LGL_Clamp(-32767.0f,tempStreamDSPOutStereo[s+1],32767.0f);
				tempStreamRecL[s/2]=tempStreamFL[s];
				tempStreamRecR[s/2]=tempStreamFR[s];
			}

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
		memcpy(tempStreamRecL,tempStreamFL,4*2*LGL_SAMPLESIZE*sizeof(float));
		memcpy(tempStreamRecR,tempStreamFR,4*2*LGL_SAMPLESIZE*sizeof(float));

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
				stream16rec[l+0]=(Sint16)LGL_Clamp(-32767,stream16rec[l+0]+tempStreamRecL[lHalf]*LGL.RecordVolume,32767);
				stream16rec[l+1]=(Sint16)LGL_Clamp(-32767,stream16rec[l+1]+tempStreamRecR[lHalf]*LGL.RecordVolume,32767);
				
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
				stream16rec[2*l+0]=(Sint16)LGL_Clamp(-32767,stream16rec[2*l+0]+tempStreamRecL[l]*LGL.RecordVolume,32767);
				stream16rec[2*l+1]=(Sint16)LGL_Clamp(-32767,stream16rec[2*l+1]+tempStreamRecR[l]*LGL.RecordVolume,32767);

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
	if(LGL.RecordActive)
	{
		assert(LGL.RecordFileDescriptor);
		fwrite(LGL.RecordBuffer,LGL_SAMPLESIZE*2*2,1,LGL.RecordFileDescriptor);
		LGL.RecordSamplesWritten+=LGL_SAMPLESIZE;
	}
	LGL.AudioBufferPos=LGL.AudioBufferPos+LGL_SAMPLESIZE;
}

Uint8* streamStereo8=NULL;
Sint16* streamStereo16=NULL;

void lgl_AudioInCallback(void *udata, Uint8 *stream, int len8)
{
	Sint16* stream16=(Sint16*)stream;
	int len16=len8/2;
	assert(len16==1024);

	if(streamStereo8==NULL)
	{
		streamStereo8=new Uint8[len8*2];
		streamStereo16=(Sint16*)streamStereo8;
	}
	for(int a=0;a<len16;a++)
	{
		streamStereo16[2*a+0]=stream16[a];
		streamStereo16[2*a+1]=stream16[a];
	}

	LGL_AudioGrain* grain=new LGL_AudioGrain;
	grain->SetWaveformFromMemory
	(
		streamStereo8,
		len16		//LengthSamples
	);
	LGL.AudioInSemaphore->Lock("AudioIn","AudioInGrainListBack.push_back()");
	{
		LGL.AudioInGrainListBack.push_back(grain);
	}
	LGL.AudioInSemaphore->Unlock();
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

