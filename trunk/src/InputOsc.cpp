/*
 *
 * InputOsc.cpp - Input abstraction object
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

#include "InputOsc.h"

InputOscObj&
GetInputOsc()
{
	static InputOscObj inputOsc(GetOscServerPort());
	return(inputOsc);
}

InputOscObj::
InputOscObj(int port) :
	LGL_OscServer(port),
	OscMessageUnknownSemaphore("OscMessageUnknownSemaphore")
{
	//Setup clients
	{
		std::vector<IpEndpointName> endpointList = GetOscClientList();
		for(unsigned int a=0;a<endpointList.size();a++)
		{
			AddOscClient(endpointList[a]);
		}
	}

	//Setup OscElements
	{
		//FocusChange
		{
			InitializeOscElement
			(
				FocusChangeOscElement,
				FOCUS_CHANGE,
				TARGET_NONE,
				0,
				0.0f,
				1.0f,
				0.0f,
				InputFocusChange,
				false
			);
		}

		//XfaderSpeakers
		{
			InitializeOscElement
			(
				XfaderSpeakersOscElement,
				XFADER_SPEAKERS,
				TARGET_NONE,
				0,
				1.0f,
				0.0f,
				-1.0f,
				InputXfaderSpeakers,
				false
			);
		}

		//XfaderHeadphones
		{
			InitializeOscElement
			(
				XfaderHeadphonesOscElement,
				XFADER_HEADPHONES,
				TARGET_NONE,
				0,
				1.0f,
				0.0f,
				-1.0f,
				InputXfaderHeadphones,
				false
			);
		}

		//FileScroll
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					FileScrollOscElement[a],
					FILE_SCROLL,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputFileScroll,
					false
				);
			}
		}

		//FileScrollPrev
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					FileScrollPrevOscElement[a],
					FILE_SCROLL_PREV,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					NULL,
					false
				);
			}
		}

		//FileScrollNext
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					FileScrollNextOscElement[a],
					FILE_SCROLL_NEXT,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					NULL,
					false
				);
			}
		}

		//FileSelect
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					FileSelectOscElement[a],
					FILE_SELECT,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputFileSelect,
					false
				);
			}
		}

		//FileMarkUnopened
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					FileMarkUnopenedOscElement[a],
					FILE_MARK_UNOPENED,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputFileMarkUnopened,
					false
				);
			}
		}

		//WaveformEject
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformEjectOscElement[a],
					WAVEFORM_EJECT,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformEject,
					true
				);
			}
		}

		//WaveformPauseToggle
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformPauseToggleOscElement[a],
					WAVEFORM_PAUSE_TOGGLE,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformPauseToggle,
					false
				);
			}
		}

		//WaveformNudge
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformNudgeOscElement[a],
					WAVEFORM_NUDGE,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformNudge,
					true
				);
			}
		}

		//WaveformNudgeSlower
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformNudgeSlowerOscElement[a],
					WAVEFORM_NUDGE_SLOWER,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					NULL,
					true
				);
			}
		}

		//WaveformNudgeFaster
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformNudgeFasterOscElement[a],
					WAVEFORM_NUDGE_FASTER,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					NULL,
					true
				);
			}
		}

		//Pitchbend
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformPitchbendOscElement[a],
					WAVEFORM_PITCHBEND,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.92f,
					1.08f,
					0.0f,
					InputWaveformPitchbend,
					false
				);
			}
		}

		//EQLow
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformEQLowOscElement[a],
					WAVEFORM_EQ_LOW,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					-1.0f,
					InputWaveformEQLow,
					false
				);
			}
		}

		//EQLowKill
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformEQLowKillOscElement[a],
					WAVEFORM_EQ_LOW_KILL,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformEQLowKill,
					true
				);
			}
		}

		//EQMid
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformEQMidOscElement[a],
					WAVEFORM_EQ_MID,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					-1.0f,
					InputWaveformEQMid,
					false
				);
			}
		}

		//EQMidKill
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformEQMidKillOscElement[a],
					WAVEFORM_EQ_MID_KILL,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformEQMidKill,
					true
				);
			}
		}

		//EQHigh
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformEQHighOscElement[a],
					WAVEFORM_EQ_HIGH,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					-1.0f,
					InputWaveformEQHigh,
					false
				);
			}
		}

		//EQHighill
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformEQHighKillOscElement[a],
					WAVEFORM_EQ_HIGH_KILL,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformEQHighKill,
					true
				);
			}
		}

		//Gain
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformGainOscElement[a],
					WAVEFORM_GAIN,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					-1.0f,
					InputWaveformGain,
					false
				);
			}
		}

		//GainKill
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformGainKillOscElement[a],
					WAVEFORM_GAIN_KILL,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformGainKill,
					true
				);
			}
		}

		//Volume
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformVolumeOscElement[a],
					WAVEFORM_VOLUME,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					-1.0f,
					InputWaveformVolumeSlider,
					false
				);
			}
		}

		//VolumeInvert
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformVolumeInvertOscElement[a],
					WAVEFORM_VOLUME_INVERT,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformVolumeInvert,
					true
				);
			}
		}
		
		//RhythmicVolumeInvert
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformRhythmicVolumeInvertOscElement[a],
					WAVEFORM_RHYTHMIC_VOLUME_INVERT,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformRhythmicVolumeInvert,
					true
				);
			}
		}
		
		//RhythmicVolumeInvertOther
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformRhythmicVolumeInvertOtherOscElement[a],
					WAVEFORM_RHYTHMIC_VOLUME_INVERT_OTHER,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformRhythmicVolumeInvertOther,
					true
				);
			}
		}
		
		//VolumeSolo
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformVolumeSoloOscElement[a],
					WAVEFORM_VOLUME_SOLO,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformVolumeSolo,
					true
				);
			}
		}

		//SeekBackwardSlow
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformSeekBackwardSlowOscElement[a],
					WAVEFORM_SEEK_BACKWARD_SLOW,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					NULL,
					true
				);
			}
		}

		//SeekBackwardFast
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformSeekBackwardFastOscElement[a],
					WAVEFORM_SEEK_BACKWARD_FAST,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					NULL,
					true
				);
			}
		}

		//SeekForwardSlow
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformSeekForwardSlowOscElement[a],
					WAVEFORM_SEEK_FORWARD_SLOW,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					NULL,
					true
				);
			}
		}

		//SeekForwardFast
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformSeekForwardFastOscElement[a],
					WAVEFORM_SEEK_FORWARD_FAST,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					NULL,
					true
				);
			}
		}

		//ScratchSpeed
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformScratchSpeedOscElement[a],
					WAVEFORM_SCRATCH_SPEED,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					0.5f,
					0.0f,
					InputWaveformRecordSpeed,
					true
				);
			}
		}

		//SavePointPrev
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformSavePointPrevOscElement[a],
					WAVEFORM_SAVEPOINT_PREV,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformSavePointPrev,
					false
				);
			}
		}

		//SavePointNext
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformSavePointNextOscElement[a],
					WAVEFORM_SAVEPOINT_NEXT,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformSavePointNext,
					false
				);
			}
		}

		//SavePointSet
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformSavePointSetOscElement[a],
					WAVEFORM_SAVEPOINT_SET,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformSavePointSet,
					true
				);
			}
		}

		//SavePointShiftBackward
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformSavePointShiftBackwardOscElement[a],
					WAVEFORM_SAVEPOINT_SHIFT_BACKWARD,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					NULL,
					true
				);
			}
		}

		//SavePointShiftForward
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformSavePointShiftForwardOscElement[a],
					WAVEFORM_SAVEPOINT_SHIFT_FORWARD,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					NULL,
					true
				);
			}
		}

		//SavePointShiftAllBackward
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformSavePointShiftAllBackwardOscElement[a],
					WAVEFORM_SAVEPOINT_SHIFT_ALL_BACKWARD,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					NULL,
					true
				);
			}
		}

		//SavePointShiftAllForward
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformSavePointShiftAllForwardOscElement[a],
					WAVEFORM_SAVEPOINT_SHIFT_ALL_FORWARD,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					NULL,
					true
				);
			}
		}

		//SavePointJumpNow
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformSavePointJumpNowOscElement[a],
					WAVEFORM_SAVEPOINT_JUMP_NOW,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformSavePointJumpNow,
					false
				);
			}
		}

		//SavePointJumpAtMeasure
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformSavePointJumpAtMeasureOscElement[a],
					WAVEFORM_SAVEPOINT_JUMP_AT_MEASURE,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformSavePointJumpAtMeasure,
					false
				);
			}
		}

		//QuantizationPeriodHalf
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformQuantizationPeriodHalfOscElement[a],
					WAVEFORM_QUANTIZATION_PERIOD_HALF,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformQuantizationPeriodHalf,
					false
				);
			}
		}

		//QuantizationPeriodDouble
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformQuantizationPeriodDoubleOscElement[a],
					WAVEFORM_QUANTIZATION_PERIOD_DOUBLE,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformQuantizationPeriodDouble,
					false
				);
			}
		}

		//LoopToggle
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformLoopToggleOscElement[a],
					WAVEFORM_LOOP_TOGGLE,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformLoopToggle,
					false
				);
			}
		}

		//LoopThenRecall
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformLoopThenRecallOscElement[a],
					WAVEFORM_LOOP_THEN_RECALL,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformLoopThenRecallActive,
					true
				);
			}
		}

		//VideoSelect
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformVideoSelectOscElement[a],
					WAVEFORM_VIDEO_SELECT,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformVideoSelect,
					false
				);
			}
		}

		//VideoBrightness
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformVideoBrightnessOscElement[a],
					WAVEFORM_VIDEO_BRIGHTNESS,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					-1.0f,
					InputWaveformVideoBrightness,
					false
				);
			}
		}

		//OscilloscopeBrightness
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformOscilloscopeBrightnessOscElement[a],
					WAVEFORM_OSCILLOSCOPE_BRIGHTNESS,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					-1.0f,
					InputWaveformOscilloscopeBrightness,
					false
				);
			}
		}

		//FreqSenseBrightness
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformFreqSenseBrightnessOscElement[a],
					WAVEFORM_FREQ_SENSE_BRIGHTNESS,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					-1.0f,
					InputWaveformFreqSenseBrightness,
					false
				);
			}
		}

		//AudioInputToggle
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformAudioInputToggleOscElement[a],
					WAVEFORM_AUDIO_INPUT_TOGGLE,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformAudioInputToggle,
					false
				);
			}
		}

		//VideoAspectRatioNext
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformVideoAspectRatioNextOscElement[a],
					WAVEFORM_VIDEO_ASPECT_RATIO_NEXT,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformVideoAspectRatioNext,
					false
				);
			}
		}

		//Sync
		{
			for(int a=0;a<2;a++)
			{
				InitializeOscElement
				(
					WaveformSyncOscElement[a],
					WAVEFORM_SYNC,
					(a==0) ? TARGET_TOP : TARGET_BOTTOM,
					0,
					0.0f,
					1.0f,
					0.0f,
					InputWaveformSync,
					true
				);
			}
		}
	}
}

InputOscObj::
~InputOscObj()
{
	for(unsigned int a=0;a<OscClientList.size();a++)
	{
		delete OscClientList[a];
	}
	OscClientList.clear();
}

//Core

void
InputOscObj::
NextFrame()
{
	//Update elements
	for(unsigned int e=0;e<OscElementList.size();e++)
	{
		OscElementList[e]->SwapBackFront();
	}

	//Update SavePointSet/Unset
	{
		for(int a=0;a<2;a++)
		{
			if(WaveformSavePointSetOscElement[a].GetFloat()==0.0f)
			{
				SavePointUnsetTimer[a].Reset();
			}
		}
	}

	//Send master input updates to osc clients
	for(unsigned int e=0;e<OscElementList.size();e++)
	{
		if(OscElementObj* element = OscElementList[e])
		{
			if(OscElementObj::MasterInputGetFnType getFn = element->GetMasterInputGetFn())
			{
				int target = element->GetTweakFocusTarget();
				if(target==TARGET_TOP)
				{
					if(GetMixer().GetFocus()==0)
					{
						target|=TARGET_FOCUS;
					}
				}
				else if(target==TARGET_BOTTOM)
				{
					if(GetMixer().GetFocus()==1)
					{
						target|=TARGET_FOCUS;
					}
				}

				float cand=getFn(target);
				if(cand!=element->GetFloatDefault())
				{
					if(cand!=element->GetFloat())
					{
						element->Unstick();
					}
					for(unsigned int a=0;a<OscClientList.size();a++)
					{
						if
						(
							strcmp
							(
								element->GetRemoteControllerFront(),
								OscClientList[a]->GetAddress()
							)!=0
						)
						{
							std::vector<char*> addressPatternsSend=element->GetAddressPatterns();
							for(unsigned int b=0;b<addressPatternsSend.size();b++)
							{
								OscClientList[a]->Stream() <<
									osc::BeginMessage(addressPatternsSend[b]) <<
									element->ConvertDvjToOsc(cand) <<
									osc::EndMessage;
								OscClientList[a]->Send();
							}
						}
					}
				}
			}
		}
	}

	//Display unknown received OSC messages
	if(OscMessageUnknownBrightness>0.0f)
	{
		LGL_ScopeLock lock(OscMessageUnknownSemaphore);
		OscMessageUnknownBrightness-=1.0f/60.0f;
		if(OscMessageUnknownBrightness<0.0f)
		{
			OscMessageUnknownBrightness=0.0f;
		}
		float bri=LGL_Clamp(0,OscMessageUnknownBrightness,1);
		LGL_GetFont().DrawString
		(
			0.025f,
			0.525f,
			0.02f,
			bri,bri,bri,bri,
			false,
			0.75f*bri,
			"OSC Unknown: %s",
			OscMessageUnknown
		);
	}
}

//Global Input

bool
InputOscObj::
FocusChange()	const
{
	bool change=FocusChangeOscElement.GetFloat()!=0.0f;
	return(change);
}

bool
InputOscObj::
FocusBottom()	const
{
	bool bottom=false;
	for(unsigned int a=0;a<OscElementList.size();a++)
	{
		if
		(
			OscElementList[a]->GetTweak() &&
			OscElementList[a]->GetTweakFocusTarget()==TARGET_BOTTOM
		)
		{
			bottom=true;
		}
	}
	return(bottom);
}

bool
InputOscObj::
FocusTop()	const
{
	bool top=false;
	for(unsigned int a=0;a<OscElementList.size();a++)
	{
		if
		(
			OscElementList[a]->GetTweak() &&
			OscElementList[a]->GetTweakFocusTarget()==TARGET_TOP
		)
		{
			top=true;
		}
	}
	return(top);
}

float
InputOscObj::
XfaderSpeakers()	const
{
	float xfade=-1.0f;
	xfade = XfaderSpeakersOscElement.GetFloat();
	return(xfade);
}

float
InputOscObj::
XfaderSpeakersDelta()	const
{
	float delta=0.0f;
	return(delta);
}

float
InputOscObj::
XfaderHeadphones()	const
{
	float xfade=-1.0f;
	xfade = XfaderHeadphonesOscElement.GetFloat();
	return(xfade);
}

float
InputOscObj::
XfaderHeadphonesDelta()	const
{
	float delta=0.0f;
	return(delta);
}

int
InputOscObj::
MasterToHeadphones()	const
{
	int to=-1;
	if(XfaderHeadphones()==1.0f) to=0;
	if(XfaderSpeakers()!=-1.0f) to=1;
	return(to);
}

//Mode 0: File Selection

float
InputOscObj::
FileScroll
(
	unsigned int	target
)	const
{
	float scroll = FileScrollOscElement[GetIndexFromTarget(target)].GetFloat();
	scroll -= FileScrollPrevOscElement[GetIndexFromTarget(target)].GetFloat();
	scroll += FileScrollNextOscElement[GetIndexFromTarget(target)].GetFloat();
	return(scroll);
}

int
InputOscObj::
FileSelect
(
	unsigned int	target
)	const
{
	int select = FileSelectOscElement[GetIndexFromTarget(target)].GetFloat();
	return(select);
}

bool
InputOscObj::
FileMarkUnopened
(
	unsigned int	target
)	const
{
	int mark = FileMarkUnopenedOscElement[GetIndexFromTarget(target)].GetFloat();
	return(mark);
}

bool
InputOscObj::
FileRefresh
(
	unsigned int	target
)	const
{
	bool refresh=false;
	return(refresh);
}



//Mode 2: Waveform

int
InputOscObj::
WaveformEject
(
	unsigned int	target
)	const
{
	int eject=WaveformEjectOscElement[GetIndexFromTarget(target)].GetFloat();
	return(eject);
}

bool
InputOscObj::
WaveformPauseToggle
(
	unsigned int	target
)	const
{
	bool toggle=WaveformPauseToggleOscElement[GetIndexFromTarget(target)].GetFloat();
	return(toggle);
}

float
InputOscObj::
WaveformNudge
(
	unsigned int	target
)	const
{
	const float SLOWER_FASTER_SCALAR=0.04;

	float nudge=WaveformNudgeOscElement[GetIndexFromTarget(target)].GetFloat();
	nudge-=(WaveformNudgeSlowerOscElement[GetIndexFromTarget(target)].GetFloat() ? SLOWER_FASTER_SCALAR : 0.0f);
	nudge+=(WaveformNudgeFasterOscElement[GetIndexFromTarget(target)].GetFloat() ? SLOWER_FASTER_SCALAR : 0.0f);

	return(nudge);
}

float
InputOscObj::
WaveformPitchbend
(
	unsigned int	target
)	const
{
	float pitchbend=0.0f;
	pitchbend = WaveformPitchbendOscElement[GetIndexFromTarget(target)].GetFloat();
	return(pitchbend);
}

float
InputOscObj::
WaveformPitchbendDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

float
InputOscObj::
WaveformEQLow
(
	unsigned int	target
)	const
{
	float low = WaveformEQLowOscElement[GetIndexFromTarget(target)].GetFloat();
	return(low);
}

float
InputOscObj::
WaveformEQLowDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputOscObj::
WaveformEQLowKill
(
	unsigned int	target
)	const
{
	bool kill = WaveformEQLowKillOscElement[GetIndexFromTarget(target)].GetFloat();
	return(kill);
}

float
InputOscObj::
WaveformEQMid
(
	unsigned int	target
)	const
{
	float mid = WaveformEQMidOscElement[GetIndexFromTarget(target)].GetFloat();
	return(mid);
}

float
InputOscObj::
WaveformEQMidDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputOscObj::
WaveformEQMidKill
(
	unsigned int	target
)	const
{
	bool kill = WaveformEQMidKillOscElement[GetIndexFromTarget(target)].GetFloat();
	return(kill);
}

float
InputOscObj::
WaveformEQHigh
(
	unsigned int	target
)	const
{
	float high = WaveformEQHighOscElement[GetIndexFromTarget(target)].GetFloat();
	return(high);
}

float
InputOscObj::
WaveformEQHighDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputOscObj::
WaveformEQHighKill
(
	unsigned int	target
)	const
{
	bool kill = WaveformEQHighKillOscElement[GetIndexFromTarget(target)].GetFloat();
	return(kill);
}

float
InputOscObj::
WaveformGain
(
	unsigned int	target
)	const
{
	float gain = WaveformGainOscElement[GetIndexFromTarget(target)].GetFloat();
	return(gain);
}

float
InputOscObj::
WaveformGainDelta
(
	unsigned int	target
)	const
{
	float delta=0.0f;
	return(delta);
}

bool
InputOscObj::
WaveformGainKill
(
	unsigned int	target
)	const
{
	bool kill = WaveformGainKillOscElement[GetIndexFromTarget(target)].GetFloat();
	return(kill);
}

float
InputOscObj::
WaveformVolumeSlider
(
	unsigned int	target
)	const
{
	float volume = WaveformVolumeOscElement[GetIndexFromTarget(target)].GetFloat();
	return(volume);
}

bool
InputOscObj::
WaveformVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert = WaveformVolumeInvertOscElement[GetIndexFromTarget(target)].GetFloat();
	return(invert);
}

bool
InputOscObj::
WaveformRhythmicVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert = WaveformRhythmicVolumeInvertOscElement[GetIndexFromTarget(target)].GetFloat();
	return(invert);
}

bool
InputOscObj::
WaveformRhythmicVolumeInvertOther
(
	unsigned int	target
)	const
{
	bool invert = WaveformRhythmicVolumeInvertOtherOscElement[GetIndexFromTarget(target)].GetFloat();
	return(invert);
}

bool
InputOscObj::
WaveformVolumeSolo
(
	unsigned int	target
)	const
{
	bool solo = WaveformVolumeSoloOscElement[GetIndexFromTarget(target)].GetFloat();
	return(solo);
}

float
InputOscObj::
WaveformRewindFF
(
	unsigned int	target
)	const
{
	const float RATE_FAST=32.0f;
	const float RATE_SLOW=2.0f;

	float rewindff=0.0f;

	rewindff-=(WaveformSeekBackwardSlowOscElement[GetIndexFromTarget(target)].GetFloat() ? RATE_SLOW : 0.0f);
	rewindff+=(WaveformSeekForwardSlowOscElement[GetIndexFromTarget(target)].GetFloat() ? RATE_SLOW : 0.0f);
	rewindff-=(WaveformSeekBackwardFastOscElement[GetIndexFromTarget(target)].GetFloat() ? RATE_FAST : 0.0f);
	rewindff+=(WaveformSeekForwardFastOscElement[GetIndexFromTarget(target)].GetFloat() ? RATE_FAST : 0.0f);

	return(rewindff);
}

bool
InputOscObj::
WaveformRecordHold
(
	unsigned int	target
)	const
{
	bool hold = WaveformScratchSpeedOscElement[GetIndexFromTarget(target)].GetStickyNow();
	return(hold);
}

float
InputOscObj::
WaveformRecordSpeed
(
	unsigned int	target
)	const
{
	float speed=0.0f;
	if(WaveformRecordHold(target))
	{
		speed = WaveformScratchSpeedOscElement[GetIndexFromTarget(target)].GetFloat();
	}
	return(speed);
}

bool
InputOscObj::
WaveformStutter
(
	unsigned int	target
)	const
{
	bool stutter=false;
	return(stutter);
}

float
InputOscObj::
WaveformStutterPitch
(
	unsigned int	target
)	const
{
	float pitch=-1.0f;
	return(pitch);
}

float
InputOscObj::
WaveformStutterSpeed
(
	unsigned int	target
)	const
{
	float speed=-1.0f;
	return(speed);
}

bool
InputOscObj::
WaveformSavePointPrev
(
	unsigned int	target
)	const
{
	bool prev = WaveformSavePointPrevOscElement[GetIndexFromTarget(target)].GetFloat();
	return(prev);
}

bool
InputOscObj::
WaveformSavePointNext
(
	unsigned int	target
)	const
{
	bool next = WaveformSavePointNextOscElement[GetIndexFromTarget(target)].GetFloat();
	return(next);
}

int
InputOscObj::
WaveformSavePointPick
(
	unsigned int	target
)	const
{
	int pick=-9999;
	return(pick);
}

bool
InputOscObj::
WaveformSavePointSet
(
	unsigned int	target
)	const
{
	bool set = WaveformSavePointSetOscElement[GetIndexFromTarget(target)].GetFloat();
	set &= WaveformSavePointSetOscElement[GetIndexFromTarget(target)].GetTweak();
	return(set);
}

float
InputOscObj::
WaveformSavePointUnsetPercent
(
	unsigned int	target
)	const
{
	float percent=0.0f;
	
	if(WaveformSavePointSetOscElement[GetIndexFromTarget(target)].GetFloat())
	{
		percent=LGL_Min(1.0f,SavePointUnsetTimer[GetIndexFromTarget(target)].SecondsSinceLastReset()*2.0f);
	}

	return(percent);
}

float
InputOscObj::
WaveformSavePointShift
(
	unsigned int	target
)	const
{
	const float SPEED = 0.025f * 1.0f/60.0f;
	float shift = 0.0f;
	shift -= SPEED * WaveformSavePointShiftBackwardOscElement[GetIndexFromTarget(target)].GetFloat();
	shift += SPEED * WaveformSavePointShiftForwardOscElement[GetIndexFromTarget(target)].GetFloat();
	return(shift);
}

float
InputOscObj::
WaveformSavePointShiftAll
(
	unsigned int	target
)	const
{
	const float SPEED = 0.025f * 1.0f/60.0f;
	float shift = 0.0f;
	shift -= SPEED * WaveformSavePointShiftAllBackwardOscElement[GetIndexFromTarget(target)].GetFloat();
	shift += SPEED * WaveformSavePointShiftAllForwardOscElement[GetIndexFromTarget(target)].GetFloat();
	return(shift);
}

bool
InputOscObj::
WaveformSavePointShiftAllHere
(
	unsigned int	target
)	const
{
	bool here=false;
	return(here);
}

bool
InputOscObj::
WaveformSavePointJumpNow
(
	unsigned int	target
)	const
{
	bool jump = WaveformSavePointJumpNowOscElement[GetIndexFromTarget(target)].GetFloat();
	return(jump);
}

bool
InputOscObj::
WaveformSavePointJumpAtMeasure
(
	unsigned int	target
)	const
{
	bool jump = WaveformSavePointJumpAtMeasureOscElement[GetIndexFromTarget(target)].GetFloat();
	return(jump);
}

int
InputOscObj::
WaveformLoopMeasuresExponent
(
	unsigned int	target
)	const
{
	int exponent=WAVEFORM_LOOP_MEASURES_EXPONENT_NULL;
	return(exponent);
}

bool
InputOscObj::
WaveformQuantizationPeriodHalf
(
	unsigned int	target
)	const
{
	bool half = WaveformQuantizationPeriodHalfOscElement[GetIndexFromTarget(target)].GetFloat();
	return(half);
}

bool
InputOscObj::
WaveformQuantizationPeriodDouble
(
	unsigned int	target
)	const
{
	bool twoX = WaveformQuantizationPeriodDoubleOscElement[GetIndexFromTarget(target)].GetFloat();
	return(twoX);
}

bool
InputOscObj::
WaveformLoopSecondsLess
(
	unsigned int	target
)	const
{
	bool less=false;
	return(less);
}

bool
InputOscObj::
WaveformLoopSecondsMore
(
	unsigned int	target
)	const
{
	bool more=false;
	return(more);
}
bool
InputOscObj::
WaveformLoopAll
(
	unsigned int	target
)	const
{
	bool all=false;
	return(all);
}


bool
InputOscObj::
WaveformLoopToggle
(
	unsigned int	target
)	const
{
	bool toggle = WaveformLoopToggleOscElement[GetIndexFromTarget(target)].GetFloat();
	return(toggle);
}

bool
InputOscObj::
WaveformLoopThenRecallActive
(
	unsigned int	target
)	const
{
	bool active = WaveformLoopThenRecallOscElement[GetIndexFromTarget(target)].GetFloat();
	return(active);
}

int
InputOscObj::
WaveformAutoDivergeRecall
(
	unsigned int	target
)	const
{
	int ret=0;
	return(ret);
}

bool
InputOscObj::
WaveformVideoSelect
(
	unsigned int	target
)	const
{
	bool select = WaveformVideoSelectOscElement[GetIndexFromTarget(target)].GetFloat();
	return(select);
}

float
InputOscObj::
WaveformVideoBrightness
(
	unsigned int	target
)	const
{
	float bright=-1.0f;
	bright = WaveformVideoBrightnessOscElement[GetIndexFromTarget(target)].GetFloat();
	return(bright);
}

float
InputOscObj::
WaveformVideoAdvanceRate
(
	unsigned int	target
)	const
{
	float rate=-1.0f;
	return(rate);
}

float
InputOscObj::
WaveformFreqSenseBrightness
(
	unsigned int	target
)	const
{
	float brightness=-1;
	brightness = WaveformFreqSenseBrightnessOscElement[GetIndexFromTarget(target)].GetFloat();
	return(brightness);
}

bool
InputOscObj::
WaveformAudioInputToggle
(
	unsigned int	target
)	const
{
	bool toggle = WaveformAudioInputToggleOscElement[GetIndexFromTarget(target)].GetFloat();
	return(toggle);
}

bool
InputOscObj::
WaveformVideoAspectRatioNext
(
	unsigned int	target
)	const
{
	bool next = WaveformVideoAspectRatioNextOscElement[GetIndexFromTarget(target)].GetFloat();
	return(next);
}

float
InputOscObj::
WaveformOscilloscopeBrightness
(
	unsigned int	target
)	const
{
	float brightness=-1.0f;
	brightness = WaveformOscilloscopeBrightnessOscElement[GetIndexFromTarget(target)].GetFloat();
	return(brightness);
}

bool
InputOscObj::
WaveformSync
(
	unsigned int	target
)	const
{
	bool sync = WaveformSyncOscElement[GetIndexFromTarget(target)].GetFloat();
	return(sync);
}

float
InputOscObj::
WaveformPointerScratch
(
	unsigned int	target
)	const
{
	float targetX=-1.0f;
	return(targetX);
}

void
InputOscObj::
ProcessMessage
(
	const osc::ReceivedMessage&	m,
	const IpEndpointName&		remoteEndpoint
)
{
	//See if we have a new client!
	char hostStr[2048];
	remoteEndpoint.AddressAsString(hostStr);
	bool clientFound=false;
	for(unsigned int a=0;a<OscClientList.size();a++)
	{
		if(strcmp(hostStr,OscClientList[a]->GetAddress())==0)
		{
			clientFound=true;
		}
	}
	if(clientFound==false)
	{
		std::vector<int> autoPortList = GetOscClientAutoPortList();
		for(unsigned int a=0;a<autoPortList.size();a++)
		{
			AddOscClient(hostStr,autoPortList[a]);
		}
	}

	try
	{
		/*
                osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
		while(arg!=m.ArgumentsEnd())
		{
			if(arg->IsFloat())
			{
				printf
				(
					"[%.2f] OSC Message: %s\n",
					arg->AsFloat(),
					m.AddressPattern()
				);
			}
			arg++;
		}
		*/

		bool messageRecognized=false;
		for(unsigned int a=0;a<OscElementList.size();a++)
		{
			messageRecognized|=
				OscElementList[a]->ProcessMessage(m,remoteEndpoint);
		}

		if(messageRecognized==false)
		{
			if(strlen(m.AddressPattern())<1024)
			{
				LGL_ScopeLock lock(OscMessageUnknownSemaphore);
				osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
				if(arg!=m.ArgumentsEnd())
				{
					if(m.ArgumentCount()>=2)
					{
						float arg1=arg->AsFloat();
						arg++;
						float arg2=arg->AsFloat();
						sprintf
						(
							OscMessageUnknown,
							"%s [%.2f, %.2f]",
							m.AddressPattern(),
							arg1,
							arg2
						);
					}
					else if(m.ArgumentCount()==1)
					{
						sprintf
						(
							OscMessageUnknown,
							"%s [%.2f]",
							m.AddressPattern(),
							arg->AsFloat()
						);
					}
				}
				else
				{
					sprintf
					(
						OscMessageUnknown,
						"%s (No Args!)",
						m.AddressPattern()
					);
				}
				OscMessageUnknownBrightness=5.0f;
			}
		}
	}
	catch(osc::Exception& e)
	{
		// any parsing errors such as unexpected argument types, or 
		// missing arguments get thrown as exceptions.
		std::cout << "error while parsing message: "
			<< m.AddressPattern() << ": " << e.what() << "\n";
	}
}

int
InputOscObj::
GetIndexFromTarget
(
	unsigned int target
)	const
{
	int ret=0;
	if(target & TARGET_BOTTOM)
	{
		ret=1;
	}
	else
	{
		ret=0;
	}
	return(ret);
}

void
InputOscObj::
AddOscElement
(
	OscElementObj&	oscElement
)
{
	OscElementList.push_back(&oscElement);
}

void
InputOscObj::
AddOscClient
(
	IpEndpointName	endpoint
)
{
	char host[2048];
	endpoint.AddressAsString(host);
	AddOscClient
	(
		host,
		endpoint.port
	);
}

void
InputOscObj::
AddOscClient
(
	const char*	host,
	int		port
)
{
	if(strcmp(host,"0.0.0.0")!=0)
	{
		LGL_OscClient* client = new LGL_OscClient(host,port);
		OscClientList.push_back(client);
	}
}

void
InputOscObj::
InitializeOscElement
(
	OscElementObj&	element,
	DVJ_Action	dvjAction,
	int		target,
	float		floatValueIndex,
	float		floatValueMapZero,
	float		floatValueMapOne,
	float		floatValueDefault,
	OscElementObj::MasterInputGetFnType
			getFn,
	bool		sticky
)
{
	element.SetDVJAction(dvjAction);
	element.SetTweakFocusTarget(target);
	element.SetFloatValues
	(
		floatValueIndex,
		floatValueMapZero,
		floatValueMapOne,
		floatValueDefault
	);
	element.SetMasterInputGetFn(getFn);
	element.SetSticky(sticky);
	AddOscElement(element);
}

