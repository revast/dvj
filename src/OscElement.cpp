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

OscElementObj::
OscElementObj() :
	BackFrontSemaphore("BackFrontSemaphore")
{
	TweakBack=false;
	TweakFront=false;
	TweakFocusTarget=TARGET_NONE;

	FloatIndex=-1;
	FloatMapZero=0.0f;
	FloatMapOne=1.0f;
	FloatDefault=-1.0f;
	FloatBack=FloatDefault;
	FloatFront=FloatDefault;
	RemoteControllerBack[0]='\0';
	RemoteControllerFront[0]='\0';
}

OscElementObj::
~OscElementObj()
{
	for(unsigned int a=0;a<AddressPatternsRecv.size();a++)
	{
		delete AddressPatternsRecv[a];
	}
	AddressPatternsRecv.clear();

	for(unsigned int a=0;a<AddressPatternsSend.size();a++)
	{
		delete AddressPatternsSend[a];
	}
	AddressPatternsSend.clear();
}

void
OscElementObj::
AddAddressPatternRecv
(
	const char*		pattern
)
{
	AddAddressPattern(pattern,AddressPatternsRecv);
}

void
OscElementObj::
AddAddressPatternSend
(
	const char*		pattern
)
{
	AddAddressPattern(pattern,AddressPatternsSend);
}

void
OscElementObj::
AddAddressPattern
(
	const char*		pattern,
	std::vector<char*>&	patternList
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
	patternList.push_back(neo);
}

void
OscElementObj::
SetTweakFocusTarget
(
	unsigned int	target
)
{
	TweakFocusTarget=target;
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

bool
OscElementObj::
GetTweak()
{
	return(TweakFront);
}

unsigned int
OscElementObj::
GetTweakFocusTarget()
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
GetAddressPatternsSend()
{
	return(AddressPatternsSend);
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
			for(unsigned int a=0;a<AddressPatternsRecv.size();a++)
			{
				if(strcmp(AddressPatternsRecv[a],m.AddressPattern())==0)
				{
					patternMatch=true;
					break;
				}
				else
				{
					char tmp[2048];
					sprintf(tmp,"%s/z",AddressPatternsRecv[a]);
					if(strcmp(m.AddressPattern(),tmp)==0)
					{
						messageRecognized=true;
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
							LGL_ScopeLock lock(BackFrontSemaphore);
							TweakBack=true;
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
	LGL_ScopeLock lock(BackFrontSemaphore);

	TweakFront=TweakBack;
	TweakBack=false;

	FloatFront=FloatBack;
	FloatBack=FloatDefault;

	strcpy(RemoteControllerFront,RemoteControllerBack);
	RemoteControllerBack[0]='\0';
}

