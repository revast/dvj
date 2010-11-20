/*
 *
 * InputOscElement.cpp - Input abstraction object
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

#include "InputOscElement.h"

InputOscElementObj::
InputOscElementObj() :
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
}

InputOscElementObj::
~InputOscElementObj()
{
	for(unsigned int a=0;a<AddressPatterns.size();a++)
	{
		delete AddressPatterns[a];
	}
	AddressPatterns.clear();
}

void
InputOscElementObj::
AddAddressPattern
(
	const char*	pattern
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
InputOscElementObj::
SetTweakFocusTarget
(
	int	target
)
{
	TweakFocusTarget=target;
}

void
InputOscElementObj::
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


bool
InputOscElementObj::
GetTweak()
{
	return(TweakFront);
}

int
InputOscElementObj::
GetTweakFocusTarget()
{
	return(TweakFocusTarget);
}

float
InputOscElementObj::
GetFloat()	const
{
	return(FloatFront);
}

bool
InputOscElementObj::
ProcessMessage
(
	const	osc::ReceivedMessage&	m,
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
							FloatBack=FloatMapZero + (FloatMapOne-FloatMapZero)*arg->AsFloat();
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
InputOscElementObj::
SwapBackFront()
{
	LGL_ScopeLock lock(BackFrontSemaphore);

	TweakFront=TweakBack;
	TweakBack=false;

	FloatFront=FloatBack;
	FloatBack=FloatDefault;
}

