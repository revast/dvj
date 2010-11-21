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
	OscElementObj* elements;

	//XfaderSpeakers
	{
		elements=&XfaderSpeakersOscElement;
		elements->AddAddressPatternRecv("/1/fader6");
		elements->AddAddressPatternRecv("/1/fader3");
		elements->AddAddressPatternSend("/1/fader6");
		elements->SetFloatValues
		(
			0,
			1.0f,
			0.0f,
			-1.0f
		);
		elements->SetMasterInputGetFn(InputXfaderSpeakers);
		AddOscElement(*elements);
	}

	//XfaderHeadphones
	{
		elements=&XfaderHeadphonesOscElement;
		//elements->AddAddressPatternRecv("/1/fader6");
		elements->SetFloatValues
		(
			0,
			1.0f,
			0.0f,
			-1.0f
		);
		AddOscElement(*elements);
	}

	//EQLow
	{
		elements=WaveformEqLowOscElement;
		elements[0].AddAddressPatternRecv("/1/rotary8");
		elements[1].AddAddressPatternRecv("/1/rotary15");
		for(int a=0;a<2;a++)
		{
			elements[a].SetTweakFocusTarget((a==0) ? TARGET_TOP : TARGET_BOTTOM);
			elements[a].SetFloatValues
			(
				0,
				0.0f,
				1.0f,
				-1.0f
			);
			AddOscElement(elements[a]);
		}
	}
	
	//EQMid
	{
		elements=WaveformEqMidOscElement;
		elements[0].AddAddressPatternRecv("/1/rotary5");
		elements[1].AddAddressPatternRecv("/1/rotary13");
		for(int a=0;a<2;a++)
		{
			elements[a].SetTweakFocusTarget((a==0) ? TARGET_TOP : TARGET_BOTTOM);
			elements[a].SetFloatValues
			(
				0,
				0.0f,
				1.0f,
				-1.0f
			);
			AddOscElement(elements[a]);
		}
	}

	//EQHigh
	{
		elements=WaveformEqHighOscElement;
		elements[0].AddAddressPatternRecv("/1/rotary2");
		elements[1].AddAddressPatternRecv("/1/rotary11");
		for(int a=0;a<2;a++)
		{
			elements[a].SetTweakFocusTarget((a==0) ? TARGET_TOP : TARGET_BOTTOM);
			elements[a].SetFloatValues
			(
				0,
				0.0f,
				1.0f,
				-1.0f
			);
			AddOscElement(elements[a]);
		}
	}

	//VideoBrightness
	{
		elements=WaveformVideoBrightnessOscElement;
		elements[0].AddAddressPatternRecv("/1/rotary7");
		elements[1].AddAddressPatternRecv("/1/rotary16");
		for(int a=0;a<2;a++)
		{
			elements[a].SetTweakFocusTarget((a==0) ? TARGET_TOP : TARGET_BOTTOM);
			elements[a].SetFloatValues
			(
				0,
				0.0f,
				1.0f,
				-1.0f
			);
			AddOscElement(elements[a]);
		}
	}

	//OscilloscopeBrightness
	{
		elements=WaveformOscilloscopeBrightnessOscElement;
		elements[0].AddAddressPatternRecv("/1/rotary4");
		elements[1].AddAddressPatternRecv("/1/rotary14");
		for(int a=0;a<2;a++)
		{
			elements[a].SetTweakFocusTarget((a==0) ? TARGET_TOP : TARGET_BOTTOM);
			elements[a].SetFloatValues
			(
				0,
				0.0f,
				1.0f,
				-1.0f
			);
			AddOscElement(elements[a]);
		}
	}

	//FreqSenseBrightness
	{
		elements=WaveformFreqSenseBrightnessOscElement;
		elements[0].AddAddressPatternRecv("/1/rotary1");
		elements[1].AddAddressPatternRecv("/1/rotary12");
		for(int a=0;a<2;a++)
		{
			elements[a].SetTweakFocusTarget((a==0) ? TARGET_TOP : TARGET_BOTTOM);
			elements[a].SetFloatValues
			(
				0,
				0.0f,
				1.0f,
				-1.0f
			);
			AddOscElement(elements[a]);
		}
	}

	//Gain
	{
		elements=WaveformGainOscElement;
		elements[0].AddAddressPatternRecv("/1/fader2");
		elements[1].AddAddressPatternRecv("/1/fader3");
		for(int a=0;a<2;a++)
		{
			elements[a].SetTweakFocusTarget((a==0) ? TARGET_TOP : TARGET_BOTTOM);
			elements[a].SetFloatValues
			(
				0,
				0.0f,
				2.0f,
				-1.0f
			);
			AddOscElement(elements[a]);
		}
	}

	//Pitchbend
	{
		elements=WaveformPitchbendOscElement;
		elements[0].AddAddressPatternRecv("/1/fader4");
		elements[1].AddAddressPatternRecv("/1/fader5");
		for(int a=0;a<2;a++)
		{
			elements[a].SetTweakFocusTarget((a==0) ? TARGET_TOP : TARGET_BOTTOM);
			elements[a].SetFloatValues
			(
				0,
				0.92f,
				1.08f,
				0.0f
			);
			AddOscElement(elements[a]);
		}
	}

	//TEST
	AddOscClient("idPad",7001);
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

	//Send master input updates to osc clients
	for(unsigned int e=0;e<OscElementList.size();e++)
	{
		if(OscElementObj* element = OscElementList[e])
		{
			if(OscElementObj::MasterInputGetFnType getFn = element->GetMasterInputGetFn())
			{
				float cand=getFn(element->GetTweakFocusTarget());
				if(cand!=-1.0f)
				{
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
							std::vector<char*> addressPatternsSend=element->GetAddressPatternsSend();
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
	bool change=false;
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

bool
InputOscObj::
SyncTopToBottom()	const
{
	bool sync=false;
	return(sync);
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

bool
InputOscObj::
SyncBottomToTop()	const
{
	bool sync=false;
	return(sync);
}

//Mode 0: File Selection

float
InputOscObj::
FileScroll
(
	unsigned int	target
)	const
{
	float scroll=0.0f;
	return(scroll);
}

int
InputOscObj::
FileSelect
(
	unsigned int	target
)	const
{
	int choose=0;
	return(choose);
}

bool
InputOscObj::
FileMarkUnopened
(
	unsigned int	target
)	const
{
	bool mark=false;
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

//Mode 1: Decoding...

bool
InputOscObj::
DecodeAbort
(
	unsigned int	target
)	const
{
	bool abort=false;
	return(abort);
}

//Mode 2: Waveform

int
InputOscObj::
WaveformEject
(
	unsigned int	target
)	const
{
	int eject=0;
	return(eject);
}

bool
InputOscObj::
WaveformTogglePause
(
	unsigned int	target
)	const
{
	bool toggle=false;
	return(toggle);
}

float
InputOscObj::
WaveformNudge
(
	unsigned int	target
)	const
{
	float nudge=0.0f;
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
	float low=-1.0f;
	low = WaveformEqLowOscElement[GetIndexFromTarget(target)].GetFloat();
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
	bool kill=false;
	return(kill);
}

float
InputOscObj::
WaveformEQMid
(
	unsigned int	target
)	const
{
	float mid=-1.0f;
	mid = WaveformEqMidOscElement[GetIndexFromTarget(target)].GetFloat();
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
	bool kill=false;
	return(kill);
}

float
InputOscObj::
WaveformEQHigh
(
	unsigned int	target
)	const
{
	float high=-1.0f;
	high = WaveformEqHighOscElement[GetIndexFromTarget(target)].GetFloat();
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
	bool kill=false;
	return(kill);
}

float
InputOscObj::
WaveformGain
(
	unsigned int	target
)	const
{
	float gain=-1.0f;
	gain = WaveformGainOscElement[GetIndexFromTarget(target)].GetFloat();
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
	bool kill=false;
	return(kill);
}

float
InputOscObj::
WaveformVolumeSlider
(
	unsigned int	target
)	const
{
	float volume=-1.0f;
	return(volume);
}

bool
InputOscObj::
WaveformVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputOscObj::
WaveformRapidVolumeInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputOscObj::
WaveformRapidSoloInvert
(
	unsigned int	target
)	const
{
	bool invert=false;
	return(invert);
}

bool
InputOscObj::
WaveformVolumeSolo
(
	unsigned int	target
)	const
{
	bool solo=false;
	return(solo);
}

float
InputOscObj::
WaveformRewindFF
(
	unsigned int	target
)	const
{
	float rewindff=0.0f;
	return(rewindff);
}

bool
InputOscObj::
WaveformRecordHold
(
	unsigned int	target
)	const
{
	bool hold=false;
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
	bool prev=false;
	return(prev);
}

bool
InputOscObj::
WaveformSavePointNext
(
	unsigned int	target
)	const
{
	bool next=false;
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
	bool set=false;
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
	return(percent);
}

float
InputOscObj::
WaveformSavePointShift
(
	unsigned int	target
)	const
{
	float shift=0.0f;
	return(shift);
}

float
InputOscObj::
WaveformSavePointShiftAll
(
	unsigned int	target
)	const
{
	float shift=0.0f;
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
	bool jump=false;
	return(jump);
}

bool
InputOscObj::
WaveformSavePointJumpAtMeasure
(
	unsigned int	target
)	const
{
	bool jump=false;
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
WaveformLoopMeasuresHalf
(
	unsigned int	target
)	const
{
	bool half=false;
	return(half);
}

bool
InputOscObj::
WaveformLoopMeasuresDouble
(
	unsigned int	target
)	const
{
	bool twoX=false;
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
	bool toggle=false;
	return(toggle);
}

bool
InputOscObj::
WaveformLoopThenRecallActive
(
	unsigned int	target
)	const
{
	bool active=false;
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
	bool select=false;
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

int
InputOscObj::
WaveformAudioInputMode
(
	unsigned int	target
)	const
{
	int mode=-1;
	return(mode);
}

bool
InputOscObj::
WaveformVideoAspectRatioNext
(
	unsigned int	target
)	const
{
	bool next=false;
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
WaveformSyncBPM
(
	unsigned int	target
)	const
{
	bool sync=false;
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
					else
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
					strcpy(OscMessageUnknown,m.AddressPattern());
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
	const char*	host,
	int		port
)
{
	LGL_OscClient* client = new LGL_OscClient(host,port);
	OscClientList.push_back(client);
}

