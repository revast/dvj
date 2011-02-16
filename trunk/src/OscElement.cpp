/*
 *
 * OscElement.cpp - Input abstraction object
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

#include "OscElement.h"

#include "Config.h"

OscElementObj::
OscElementObj() :
	BackFrontSemaphore("BackFrontSemaphore")
{
	Action = NOOP;

	Sticky=false;
	StickyNow=false;
	
	SendDefaultOnZRelease=false;
	LastSentFloat=-9999.0f;
	LastRecvFloat=-9999.0f;

	TweakBack=false;
	TweakFront=false;
	TweakFocusTarget=TARGET_NONE;

	FloatIndex=-1;
	FloatMapZero=0.0f;
	FloatMapOne=1.0f;
	FloatDefault=-1.0f;
	FloatBack=FloatDefault;
	FloatFront=FloatDefault;
	FloatDeltaBack=0.0f;
	FloatDeltaFront=0.0f;

	RemoteControllerBack[0]='\0';
	RemoteControllerFront[0]='\0';

	MasterInputGetFn=NULL;
}

OscElementObj::
~OscElementObj()
{
	ClearAddressPatterns();
}

void
OscElementObj::
SetDVJAction
(
	DVJ_Action	action
)
{
	Action=action;

	UpdateAddressPatterns();
}

void
OscElementObj::
SetTweakFocusTarget
(
	unsigned int	target
)
{
	TweakFocusTarget=target;

	UpdateAddressPatterns();
}

void
OscElementObj::
SetFloatValues
(
	int	floatIndex,
	float	floatMapZero,
	float	floatMapOne,
	float	floatDefault
)
{
	FloatIndex=floatIndex;
	FloatMapZero=floatMapZero;
	FloatMapOne=floatMapOne;
	FloatDefault=floatDefault;
	FloatBack=FloatDefault;
	FloatFront=FloatDefault;
	FloatDeltaBack=0.0f;
	FloatDeltaFront=0.0f;
}

void
OscElementObj::
SetMasterInputGetFn
(
	OscElementObj::MasterInputGetFnType	fn
)
{
	MasterInputGetFn=fn;
}

OscElementObj::MasterInputGetFnType
OscElementObj::
GetMasterInputGetFn()
{
	return(MasterInputGetFn);
}

void
OscElementObj::
SetSticky
(
	bool	sticky
)
{
	Sticky=sticky;
}

bool
OscElementObj::
GetSticky()	const
{
	return(Sticky);
}

bool
OscElementObj::
GetStickyNow()	const
{
	return(StickyNow);
}

void
OscElementObj::
Unstick()
{
	StickyNow=false;
	FloatBack=FloatDefault;
	FloatDeltaBack=0.0f;
}

bool
OscElementObj::
GetTweak()	const
{
	return(TweakFront);
}

unsigned int
OscElementObj::
GetTweakFocusTarget()	const
{
	return(TweakFocusTarget);
}

float
OscElementObj::
GetFloat()	const
{
	return(FloatFront);
}

float
OscElementObj::
GetFloatDelta()	const
{
	return(FloatDeltaFront);
}

float
OscElementObj::
GetFloatDefault()	const
{
	return(FloatDefault);
}

float
OscElementObj::
ConvertOscToDvj
(
	float	osc
)
{
	if(osc==-1.0f)
	{
		return(FloatDefault);
	}

	return(FloatMapZero + (FloatMapOne-FloatMapZero)*osc);
}

float
OscElementObj::
ConvertDvjToOsc
(
	float	dvj
)
{
	if
	(
		dvj==FloatDefault ||
		FloatMapOne==FloatMapZero
	)
	{
		return(-1.0f);
	}

	return
	(
		LGL_Clamp
		(
			0.0f,
			(dvj-FloatMapZero)/(FloatMapOne-FloatMapZero),
			1.0f
		)
	);
}

float
OscElementObj::
GetLastSentFloat()	const
{
	return(LastSentFloat);
}

void
OscElementObj::
SetLastSentFloat
(
	float	sent
)
{
	LastSentFloat=sent;
}

void
OscElementObj::
SetSendDefaultOnZRelease
(
	bool	send
)
{
	SendDefaultOnZRelease=send;
}

const char*
OscElementObj::
GetRemoteControllerBack()	const
{
	return(RemoteControllerBack);
}

const char*
OscElementObj::
GetRemoteControllerFront()	const
{
	return(RemoteControllerFront);
}

std::vector<char*>&
OscElementObj::
GetAddressPatterns()
{
	return(AddressPatterns);
}

bool
OscElementObj::
ProcessMessage
(
	const osc::ReceivedMessage&	m,
	const IpEndpointName&		remoteEndpoint
)
{
	bool messageRecognized=false;

	try
	{
		bool patternMatch=false;
		//Try to make patternMatch true
		{
			for(unsigned int a=0;a<AddressPatterns.size();a++)
			{
				if(strcmp(AddressPatterns[a],m.AddressPattern())==0)
				{
					patternMatch=true;
					break;
				}
				else
				{
					char tmp[2048];
					sprintf(tmp,"%s/z",AddressPatterns[a]);
					if(strcmp(m.AddressPattern(),tmp)==0)
					{
						//Z Message!
						messageRecognized=true;
						if(Sticky)
						{
							osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
							if(arg!=m.ArgumentsEnd())
							{
								if(arg->AsFloat()==1.0f)
								{
									//Z Hold
									StickyNow=true;
								}
								else if(arg->AsFloat()==0.0f)
								{
									//Z Release
									StickyNow=false;
									if(SendDefaultOnZRelease)
									{
										LastRecvFloat=-9999.0f;

										//Restore element to default on clients
										std::vector<LGL_OscClient*> oscClientList=GetInputOsc().GetLGLOscClientList();
										for(unsigned int c=0;c<oscClientList.size();c++)
										{
											for(unsigned int p=0;p<AddressPatterns.size();p++)
											{
												oscClientList[c]->Stream() <<
													osc::BeginMessage(AddressPatterns[p]) <<
													FloatDefault <<
													osc::EndMessage;
												oscClientList[c]->Send();
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if(patternMatch)
		{
			if(FloatIndex>=0)
			{
				int floatIndexNow=FloatIndex;
				osc::ReceivedMessage::const_iterator arg = m.ArgumentsBegin();
				while(arg!=m.ArgumentsEnd())
				{
					if(arg->IsFloat())
					{
						floatIndexNow--;
						if(floatIndexNow<0)
						{
							LGL_ScopeLock lock(__FILE__,__LINE__,BackFrontSemaphore);
							if(Sticky)
							{
								StickyNow=true;
							}
							TweakBack=true;
							
							if(LastRecvFloat!=-9999.0f)
							{
								float scalar = LGL_Max
								(
									60.0f,
									1.0f /
									(
										LGL_Max
										(
											1.0f/60.0f,
											FloatDeltaDeltaTimer.SecondsSinceLastReset()
										)
									)
								);
								scalar=60.0f;
								FloatDeltaBack=scalar*ConvertOscToDvj(arg->AsFloat()-LastRecvFloat);
								FloatDeltaDeltaTimer.Reset();
							}
							LastRecvFloat=arg->AsFloat();

							FloatBack=ConvertOscToDvj(arg->AsFloat());
							remoteEndpoint.AddressAsString(RemoteControllerBack);
							messageRecognized=true;
							break;
						}
					}
					arg++;
				}
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

	return(messageRecognized);
}

void
OscElementObj::
SwapBackFront()
{
	LGL_ScopeLock lock(__FILE__,__LINE__,BackFrontSemaphore);

	TweakFront=TweakBack;
	TweakBack=false;

	FloatFront=FloatBack;
	if(StickyNow==false)
	{
		FloatBack=FloatDefault;
	}

	FloatDeltaFront=FloatDeltaBack;
	if(FloatDeltaDeltaTimer.SecondsSinceLastReset()>0.1f)
	{
		//FloatDeltaBack=0.0f;
	}

	strcpy(RemoteControllerFront,RemoteControllerBack);
	RemoteControllerBack[0]='\0';
}

void
OscElementObj::
AddAddressPattern
(
	const char*		pattern
)
{
	if
	(
		pattern==NULL ||
		pattern[0]!='/'
	)
	{
		return;
	}

	char* neo = new char[strlen(pattern)+1];
	strcpy(neo,pattern);
	AddressPatterns.push_back(neo);
}

void
OscElementObj::
UpdateAddressPatterns()
{
	ClearAddressPatterns();

	std::vector<const char*> addressPatterns = GetOscAddressPatternList(Action,TweakFocusTarget);
	for(unsigned int a=0;a<addressPatterns.size();a++)
	{
		AddAddressPattern(addressPatterns[a]);
	}
}

void
OscElementObj::
ClearAddressPatterns()
{
	for(unsigned int a=0;a<AddressPatterns.size();a++)
	{
		delete AddressPatterns[a];
	}
	AddressPatterns.clear();
}

