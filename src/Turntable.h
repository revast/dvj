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
#include "ListSelector.h"

#include "Database.h"

const int NOISE_IMAGE_COUNT_256_64 = 64;
const int ENTIRE_WAVE_ARRAY_COUNT_MAX = 1920*2;

const float WAVE_WIDTH_PERCENT = 0.60f;

const int SAVEPOINT_NUM = 18;
const int BPM_UNDEF = -1;
const int BPM_NONE = -2;

class SavepointObj
{

public:

					SavepointObj();
					~SavepointObj();

	float				Seconds;
	float				BPM;
	float				UnsetNoisePercent;
	float				UnsetFlashPercent;


};

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

	TurntableObj*			GetOtherTT();

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
	float				GetSyphonBrightnessPreview();
	float				GetSyphonBrightnessFinal();
	float				GetOscilloscopeBrightnessPreview();
	float				GetOscilloscopeBrightnessFinal();
	float				GetFreqSenseBrightnessPreview();
	float				GetFreqSenseBrightnessFinal();
	float				GetFreqSenseLEDBrightnessPreview();
	float				GetFreqSenseLEDBrightnessFinal(int group);
	float				GetEjectVisualBrightnessScalar();
	LGL_Color			GetFreqSenseLEDColorLow(int group);
	LGL_Color			GetFreqSenseLEDColorHigh(int group);
	float				GetFreqSenseLEDBrightnessWash(int group);
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
	LGL_VideoDecoder*		GetVideoLo();
	LGL_VideoDecoder*		GetVideoHi();
	const char*			GetVideoLoPath();
	const char*			GetVideoHiPath();
	float				GetVideoTimeSeconds();
	bool				GetVideoSolo();
	float				GetTimeSeconds(bool forceNonSmooth=false);
	float				GetTimeSecondsPrev();
	bool				GetFreqMetadata
					(
						float&	volAve,
						float&	volMax,
						float&	freqFactor
					);
	float				GetVolumePeak();

	void				GetMetadataPathOld(char* dst);
	void				GetMetadataPathNew(char* dst);
	void				GetCacheMetadataPath(char* dst);
	void				LoadMetadataOld(const char* data);
	void				LoadMetadataNew(const char* data);
	void				SaveMetadataOld();
	void				SaveMetadataNew();
	const char*			GetMetadataSavedThisFrame() const;

	void				ResetAllCachedData();
	void				LoadAllCachedData();
	void				SaveAllCachedData();

	void				LoadWaveArrayData();
	void				SaveWaveArrayData();

	void				LoadCachedMetadata();
	void				SaveCachedMetadata();

const	char*				GetSoundPath();
const	char*				GetSoundPathShort();
const	char*				GetHighlightedPath();
const	char*				GetHighlightedPathShort();
const	char*				GetHighlightedNameDisplayed();

	bool				GetPaused();
	float				GetFinalSpeed();
	bool				GetRecordScratch();
	bool				GetSoundLoaded();
	bool				GetSoundLoadedFully();
	float				GetSoundPositionPercent();
	int				GetMode();

	void				SetLowRez(bool lowRez);

	SavepointObj*			GetCurrentSavepoint();
	float				GetCurrentSavepointSeconds();
	float				GetCurrentSavepointBPM();
	void				SortSavepoints();
	bool				SavepointIndexAtPlus();
	float				GetSecondsToSync();
	float				GetSecondsToSyncToOtherTT();
	float				GetSecondsToSyncToMidiClock();
	void				DeriveSoundStrings();
	void				CreateFindAudioPathThread();
	void				AttemptToCreateSound();

private:

	float				ViewportLeft;
	float				ViewportRight;
	float				ViewportBottom;
	float				ViewportTop;
	float				ViewportWidth;
	float				ViewportHeight;

	float				WaveformLeft;
	float				WaveformRight;
	float				WaveformBottom;
	float				WaveformTop;
	float				WaveformWidth;
	float				WaveformHeight;

	bool				Focus;
	bool				FocusPrev;

	int				Mode;	//0=File Select
						//1=Loading...
						//2=Waveform
	LGL_Timer			Mode0Timer;
	LGL_Timer			Mode1Timer;
	LGL_Timer			Mode2Timer;
	LGL_Sound*			Sound;
public:
	int				SoundSampleNow;
private:
	char				SoundName[2048];	//I want to factor this out
public:
	char				SoundSrcPath[2048];
private:
	char				SoundSrcNameDisplayed[2048];
	char				SoundSrcDir[2048];
public:
	char				FoundVideoPath[2048];
	char				FoundAudioPath[2048];
	bool				FoundAudioPathIsDir;
	bool				FindAudioPathDone;
private:
	SDL_Thread*			FindAudioPathThread;
	SDL_Thread*			WarmMemoryThread;

	bool				UpdateFilterListViaThread;
	SDL_Thread*			UpdateFilterListThread;
	LGL_Timer			UpdateFilterListTimer;
	DatabaseFilterObj		UpdateFilterListDatabaseFilterNext;
	bool				UpdateFilterListResetHighlightedRow;
	char				UpdateFilterListDesiredSelection[2048];
	void				UpdateDatabaseFilterFn();
	void				GetEntryListFromFilterDance(const char* oldSelection=NULL);
public:
	DatabaseFilterObj		UpdateFilterListDatabaseFilterNow;
	bool				UpdateFilterListAbortSignal;

	int				WarmMemoryThreadTerminateSignal;
	int				UpdateFilterListThreadTerminateSignal;
	Uint8*				SoundBuffer;
	unsigned long			SoundBufferLength;
private:
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

	float				CenterX;
	float				CenterY;

public:
	DatabaseObj*			Database;
private:
	DatabaseFilterObj		DatabaseFilter;
	std::vector<DatabaseEntryObj*>	DatabaseFilteredEntries;
public:
	std::vector<DatabaseEntryObj*>	DatabaseFilteredEntriesNext;
	bool				DatabaseFilteredEntriesNextReady;
private:
	DatabaseEntryObj*		DatabaseEntryNow;

	LGL_InputBuffer			FilterText;
	char				FilterTextMostRecent[1024];
	int				FilterTextMostRecentBPM;

	LGL_Timer			DecodeTimer;

	int				Channel;
	int				PauseMultiplier;		//0=Paused	1=Playing
	float				Pitchbend;
	bool				PitchbendLastSetByXponentSlider;
	float				Nudge;
	float				MixerNudge;
	bool				MixerVideoMute;
	float				ReverseMultiplier;
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
public:
	bool				VideoFileExists;
private:
	float				MixerVolumeFront;
	float				MixerVolumeBack;
	float				MixerCrossfadeFactorFront;
	float				MixerCrossfadeFactorBack;
	float				MixerEQ[3];
	bool				Looping();
	double				LoopAlphaSeconds;
	double				LoopOmegaSeconds;
	int				QuantizePeriodMeasuresExponent;
	int				QuantizePeriodMeasuresExponentRemembered;
	double				QuantizePeriodNoBPMSeconds;
	bool				LoopActive;
	bool				LoopThenRecallActive;
	bool				AutoDivergeRecallActive;
	int				SavepointIndex;
	std::vector<SavepointObj>	Savepoints;
#if	USE_OLD_SAVEPOINTS
	double				SavepointSeconds[SAVEPOINT_NUM];
	float				SavepointUnsetNoisePercent[SAVEPOINT_NUM];
	float				SavepointUnsetFlashPercent[SAVEPOINT_NUM];
#endif
	char*				MetadataSavedThisFrame;
	LGL_FileToRam*			MetadataFileToRam;
	std::vector<LGL_FileToRam*>	MetadataFileToRamDeathRow;
	double				SmoothWaveformScrollingSample;
	double				SmoothWaveformScrollingSampleRate;
	double				SmoothWaveformScrollingSampleRateRemembered;
	double				SmoothWaveformScrollingSampleExactUponDifferent;

	void				Diverge();
	void				Recall();
	bool				RecallIsSet();
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

	LGL_VideoDecoder*		Video;
	LGL_VideoDecoder*		VideoLo;
	LGL_VideoDecoder*		VideoHi;
	char				VideoLoPath[2048];
	char				VideoHiPath[2048];
	char				VideoLoPathShort[2048];
	char				VideoHiPathShort[2048];
	float				VideoSwitchInterval;
	float				VideoOffsetSeconds;
	float				VideoAdvanceRate;
	float				VideoBrightness;
	float				SyphonBrightness;
	float				OscilloscopeBrightness;
	float				FreqSenseBrightness;
	float				FreqSensePathBrightness;
	int				GetFreqSenseLEDGroupInt();
	float				FreqSenseLEDGroupFloat;
	float				FreqSenseLEDBrightness[LED_GROUP_MAX];
	float				FreqSenseLEDColorScalarLow[LED_GROUP_MAX];
	float				FreqSenseLEDColorScalarHigh[LED_GROUP_MAX];
	float				FreqSenseLEDBrightnessWash[LED_GROUP_MAX];
	bool				AudioInputMode;
	bool				TesterEverEnabled;
	LGL_Timer			VideoRadiusIncreaseDelayTimer;

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
	char				VideoEncoderReason[2048];

	int				GetVideoFrequencySensitiveMode();
	float				GetEQLo();
	float				GetEQMid();
	float				GetEQHi();
	float				GetEQVUL();
	float				GetEQVUM();
	float				GetEQVUH();
	float				GetEQVUPeakL();
	float				GetEQVUPeakM();
	float				GetEQVUPeakH();
	float				GetVU();
	float				GetVUPeak();

	bool				GetMaster();
	void				SetMaster();

	int				GetSoundChannel();

static	bool				GetFileEverOpened();
static	float				GetSecondsSinceFileEverOpened();

private:

	float				BPMMaster;
	double				SecondsLast;
	double				SecondsNow;

	float				EQFinal[3];
	float				EQKnob[3];
	bool				EQKill[3];
	float				EQPeak[3];
	LGL_Timer			EQPeakDropTimer[3];
	float				VUPeak;
	LGL_Timer			VUPeakDropTimer;
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
	SDL_Thread*			LoadAllCachedDataThread;
	bool				LoadAllCachedDataDone;

	LGL_Timer			Mode0BackspaceTimer;
static	bool				FileEverOpened;
static	LGL_Timer			FileEverOpenedTimer;
static	bool				SurroundMode;

	int				AspectRatioMode;	//0: Respect AR
								//1: Fill
								//2: Tile
	bool				EncodeEveryTrack;
	int				EncodeEveryTrackIndex;

	char				DenyPreviewNameDisplayed[2048];

	dvjListSelector			ListSelector;
	void				UpdateListSelector();

	bool				InputUnsetDebounce;

public:

static	bool				GetSurroundMode();
	int				GetAspectRatioMode();
	void				SelectNewVideo();
	void				SelectNewVideoLow();
	void				SelectNewVideoHigh();
	bool				BPMAvailable();
	float				GetBPM();
	float				GetBPMAdjusted(bool normalize=true);
	float				GetBPMFromDeltaSeconds(float deltaSeconds);
	float				GetBPMFirstBeatSeconds();
	float				GetBPMFirstMeasureSeconds();
	float				GetBPMLastMeasureSeconds();
	float				GetBPMAnchorMeasureSeconds();
	void				SetBPMAdjusted(float bpmAdjusted);
	void				SetBPMMaster(float bpmMaster);
	void				SetPitchbend(float pitchbend=1.0f);
	float				GetPitchbend();
	bool				GetBeatThisFrame(float fractionOfBeat=1.0f);
	double				GetPercentOfCurrentMeasure(float measureMultiplier=1.0f, bool recalculateSecondsNow=false);
	double				GetBeginningOfCurrentMeasureSeconds(float measureMultiplier=1.0f, bool recalculateSecondsNow=false);
	double				GetBeginningOfArbitraryMeasureSeconds(float seconds, float measureMultiplier=1.0f);
	bool				GetSolo();
	bool				GetRhythmicSoloInvert();
	void				SetRespondToRhythmicSoloInvert(int soloChannel);
	void				BlankFilterTextIfMode0();
	bool				ListSelectorToString(const char* str);
	float				GetNoiseFactorVideo();
	void				SetNoiseFactorVideo(float noiseFactor);

};

#endif	//_DVJ_TURNTABLE_H_

