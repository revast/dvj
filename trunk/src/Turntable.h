/*
 *
 * Turntable.h
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

#ifndef	_DVJ_TURNTABLE_H_
#define	_DVJ_TURNTABLE_H_

#include "LGL.module/LGL.h"
#include "Visualizer.h"

#include "Database.h"

const int NOISE_IMAGE_COUNT_256_64 = 64;
const int ENTIRE_WAVE_ARRAY_COUNT_MAX = 1920*2;

class TurntableObj
{

public:

					TurntableObj
					(
						float	left,	float	right,
						float	bottom,	float	top,
						DatabaseObj* database
					);
					~TurntableObj();

	void				NextFrame(float secondsElapsed);
	void				DrawFrame(float glow, bool visualsQuadrent, float visualizerZoomOutPercent);

	void				DrawVU(float left, float right, float bottom, float top, float glow);
	void				DrawWave(float left, float right, float bottom, float top, float brightness, bool preview);
	void				DrawSpectrum(float left, float right, float bottom, float top, float glow);
	void				DrawWholeTrack(float left, float right, float bottom, float top, float glow);

	void				SetViewport
					(
						float	left,	float	right,
						float	bottom,	float	top
					);
	void				SetFocus(bool inFocus=true);
	bool				GetFocus() const;
	int				GetWhich() const;
	int				GetTarget() const;
	void				SetTurntableNumber(int num);
	void				SetVisualizer(VisualizerObj* viz);
	float				GetVisualBrightnessPreview();
	float				GetVisualBrightnessFinal();
	float				GetVideoBrightnessPreview();
	float				GetVideoBrightnessFinal();
	float				GetOscilloscopeBrightnessPreview();
	float				GetOscilloscopeBrightnessFinal();
	float				GetFreqSenseBrightnessPreview();
	float				GetFreqSenseBrightnessFinal();
	float				GetEjectVisualBrightnessScalar();
	bool				GetAudioInputMode();

	void				SetMixerVolumeFront(float scalar);
	void				SetMixerVolumeBack(float scalar);
	void				SetMixerCrossfadeFactorFront(float factor);
	void				SetMixerCrossfadeFactorBack(float factor);

	float				GetMixerVolumeFront();
	float				GetMixerVolumeBack();
	float				GetMixerCrossfadeFactorFront();
	float				GetMixerCrossfadeFactorBack();
	float				GetGain();

	void				SetMixerEQ(float low, float mid, float high);
	void				SetMixerNudge(float mixerNudge);
	bool				GetMixerVideoMute();
	void				SetMixerVideoMute(bool mixerVideoMute=true);

	std::vector<char*>		GetTrackListFileUpdates();

	LGL_VideoDecoder*		GetVideo();
	LGL_VideoDecoder*		GetVideoFront();
	LGL_VideoDecoder*		GetVideoBack();
	LGL_VideoDecoder*		GetVideoLo();
	LGL_VideoDecoder*		GetVideoHi();
	float				GetVideoTimeSeconds();
	bool				GetVideoSolo();
	float				GetTimeSeconds();
	bool				GetFreqMetaData(float& volAve, float& volMax, float& freqFactor);
	float				GetVolumePeak();

	bool				GetSavePointIndexAtNull() const;

	void				GetMetaDataPath(char* dst);
	void				GetCacheMetaDataPath(char* dst);
	void				LoadMetaData();
	void				LoadMetaData(const char* data);
	void				SaveMetaData();
	const char*			GetMetaDataSavedThisFrame() const;

	void				LoadAllCachedData();
	void				SaveAllCachedData();

	void				LoadWaveArrayData();
	void				SaveWaveArrayData();

	void				LoadCachedMetadata();
	void				SaveCachedMetadata();

const	char*				GetSoundPath();
const	char*				GetSoundPathShort();

	bool				GetPaused();
	float				GetFinalSpeed();
	bool				GetRecordScratch();
	bool				GetSoundLoaded();
	bool				GetSoundLoadedFully();
	float				GetSoundPositionPercent();
	int				GetMode();

	void				SetLowRez(bool lowRez);

private:

	void				ProcessHintFile(char* path);

private:

	float				ViewportLeft;
	float				ViewportRight;
	float				ViewportBottom;
	float				ViewportTop;
	float				ViewportWidth;
	float				ViewportHeight;

	bool				Focus;

	int				Mode;	//0=File Select
						//1=Loading...
						//2=Waveform
	LGL_Timer			Mode1Timer;
	LGL_Sound*			Sound;
	char				SoundName[2048];
	char				SoundSrcPath[2048];
	char				SoundSrcPathShort[2048];
	char				SoundSrcDir[2048];
	Uint8*				SoundBuffer;
	unsigned long			SoundBufferLength;
	unsigned long			SoundBufferCurrentPageIndex;
	LGL_AudioGrainStream		GrainStream;
	bool				GrainStreamActive;
	float				GrainStreamActiveSeconds;
	float				GrainStreamInactiveSeconds;
	float				GrainStreamCrossfader;
	float				GrainStreamMainVolumeMultiplier;
	float				GrainStreamGrainsPerSecond;
	float				GrainStreamStartDelayVariance;
	float				GrainStreamSourcePoint;
	float				GrainStreamSourcePointVariance;
	float				GrainStreamLength;
	float				GrainStreamLengthVariance;
	float				GrainStreamPitch;
	float				GrainStreamPitchVariance;
	float				GrainStreamVolume;
	float				GrainStreamVolumeVariance;
	float				GrainStreamSpawnDelaySeconds;
	
	float				Left;
	float				Right;
	float				Bottom;
	float				Top;
	float				Width;
	float				Height;
	float				CenterX;
	float				CenterY;

	DatabaseObj*			Database;
	DatabaseFilterObj		DatabaseFilter;
	std::vector<DatabaseEntryObj*>	DatabaseFilteredEntries;
	DatabaseEntryObj*		DatabaseEntryNow;

	LGL_InputBuffer			FilterText;
	char				FilterTextMostRecent[1024];
	int				FileTop;
	int				FileSelectInt;
	float				FileSelectFloat;
	float				FileBPM[5];

	LGL_Timer			DecodeTimer;
	float				BadFileFlash;

	int				Channel;
	int				PauseMultiplier;		//0=Paused	1=Playing
	float				Pitchbend;
	bool				PitchbendLastSetByXponentSlider;
	float				Nudge;
	float				MixerNudge;
	bool				MixerVideoMute;
	float				FinalSpeed;
	float				FinalSpeedLastFrame;
	bool				RewindFF;
	LGL_Timer			RecordHoldReleaseTimer;
	bool				RecordHoldLastFrame;
	bool				RecordScratch;
	bool				RecordSpeedAsZeroUntilZero;
	LGL_Timer			EjectTimer;
	bool				LuminScratch;	//Pointer scratch...?
	long				LuminScratchSamplePositionDesired;
	bool				GlitchDuo;
	float				GlitchVolume;
	double				GlitchBegin;
	float				GlitchLength;
	bool				GlitchPure;	//Pointer scratch...?
	bool				GlitchPureDuo;
	double				GlitchPureSpeed;
	double				GlitchPureVideoNow;
	double				GlitchPitch;
	bool				VolumeKill;
	float				VolumeSlider;
	bool				VolumeSolo;
	float				VolumeMultiplierNow;
	float				VolumeInvertBinary;
	bool				RhythmicVolumeInvert;
	bool				RhythmicSoloInvert;
	bool				VideoFileExists;
	float				MixerVolumeFront;
	float				MixerVolumeBack;
	float				MixerCrossfadeFactorFront;
	float				MixerCrossfadeFactorBack;
	float				MixerEQ[3];
	bool				Looping();
	double				LoopAlphaSeconds;
	double				LoopOmegaSeconds;
	int				QuantizePeriodMeasuresExponent;
	double				QuantizePeriodNoBPMSeconds;
	bool				LoopActive;
	bool				LoopThenRecallActive;
	bool				AutoDivergeRecallActive;
	int				SavePointIndex;
	double				SavePointSeconds[18];
	char*				MetaDataSavedThisFrame;
	double				RecallOrigin;
	unsigned long			RecallSampleCounter;
	double				SmoothWaveformScrollingSample;
	float				SavePointUnsetNoisePercent[18];
	float				SavePointUnsetFlashPercent[18];

	void				SetRecallOrigin();
	void				ClearRecallOrigin();
	bool				RecallIsSet();
	void				Recall();
	double				GetQuantizePeriodSeconds();
	double				GetBeatLengthSeconds();
	double				GetMeasureLengthSeconds();

	void				UpdateSoundFreqResponse();

	int				Which;		//Which turntable we are.
static	int				Master;

	char				ImageSetPrefix[1024];
	char				MovieClipPrefix[1024];
static	VisualizerObj*			Visualizer;

	std::vector<char*>		TrackListFileUpdates;

	LGL_VideoDecoder*		VideoFront;
	LGL_VideoDecoder*		VideoBack;
	LGL_VideoDecoder*		VideoLo;
	LGL_VideoDecoder*		VideoHi;
	float				VideoSwitchInterval;
	float				VideoOffsetSeconds;
	float				VideoAdvanceRate;
	float				VideoBrightness;
	float				OscilloscopeBrightness;
	float				FreqSenseBrightness;
	bool				AudioInputMode;
	LGL_Timer			VideoFrontRadiusIncreaseDelayTimer;

public:

	LGL_VideoEncoder*		VideoEncoder;
	LGL_Semaphore			VideoEncoderSemaphore;
	SDL_Thread*			VideoEncoderThread;
	float				VideoEncoderPercent;
	float				VideoEncoderEtaSeconds;
	bool				VideoEncoderAudioOnly;
	char				VideoEncoderPathSrc[2048];
	int				VideoEncoderTerminateSignal;
	int				VideoEncoderBeginSignal;
	int				VideoEncoderEndSignal;
	float				VideoEncoderUnsupportedCodecTime;
	char				VideoEncoderUnsupportedCodecName[64];

	int				GetVideoFrequencySensitiveMode();
	float				GetEQLo();
	float				GetEQMid();
	float				GetEQHi();

	bool				GetMaster();
	void				SetMaster();

	int				GetSoundChannel();

static	bool				GetFileEverOpened();
static	float				GetSecondsSinceFileEverOpened();

private:

	float				BPM;
	bool				BPMRecalculationRequired;
	float				BPMMaster;
	double				SecondsLast;
	double				SecondsNow;

	float				EQFinal[3];
	float				EQKnob[3];
	bool				EQKill[3];
	float				FreqResponse[513];

	float				WhiteFactor;
	float				NoiseFactor;
	float				NoiseFactorVideo;
static	LGL_Image*			NoiseImage[NOISE_IMAGE_COUNT_256_64];
static	LGL_Image*			LoopImage;

	bool				LowRez;

	float				EntireWaveArrayMagnitudeAve[ENTIRE_WAVE_ARRAY_COUNT_MAX];
	float				EntireWaveArrayMagnitudeMax[ENTIRE_WAVE_ARRAY_COUNT_MAX];
	float				EntireWaveArrayFreqFactor[ENTIRE_WAVE_ARRAY_COUNT_MAX];
	int				EntireWaveArrayFillIndex;
	float				CachedLengthSeconds;
	float				CachedVolumePeak;

	LGL_Timer			Mode0BackspaceTimer;
static	bool				FileEverOpened;
static	LGL_Timer			FileEverOpenedTimer;
static	bool				SurroundMode;

	int				AspectRatioMode;	//0: Respect AR
								//1: Fill
								//2: Tile
	bool				EncodeEveryTrack;
	int				EncodeEveryTrackIndex;

public:

static	bool				GetSurroundMode();
	int				GetAspectRatioMode();
	void				SwapVideos();
	void				SelectNewVideo(bool forceAmbient=false);
	bool				BPMAvailable();
	float				GetBPM();
	float				GetBPMAdjusted();
	float				GetBPMFirstBeatSeconds();
	float				GetBPMFirstMeasureSeconds();
	float				GetBPMLastMeasureSeconds();
	void				SetBPMAdjusted(float bpmAdjusted);
	void				SetBPMMaster(float bpmMaster);
	bool				GetBeatThisFrame(float fractionOfBeat=1.0f);
	double				GetPercentOfCurrentMeasure(float measureMultiplier=1.0f);
	double				GetBeginningOfCurrentMeasureSeconds(float measureMultiplier=1.0f);
	double				GetBeginningOfArbitraryMeasureSeconds(float seconds, float measureMultiplier=1.0f);
	bool				GetSolo();
	bool				GetRhythmicSoloInvert();
	void				SetRespondToRhythmicSoloInvert(int soloChannel);
	void				BlankFilterTextIfMode0();
	void				FileSelectToString(const char* str);
	float				GetNoiseFactorVideo();
	void				SetNoiseFactorVideo(float noiseFactor);

};

#endif	//_DVJ_TURNTABLE_H_

