/*
 *
 * InputOscElement.h - Input abstraction object
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

#ifndef	_INPUT_OSC_ELEMENT_H_
#define	_INPUT_OSC_ELEMENT_H_

#include "LGL.module/LGL.h"

class InputOscElementObj
{

public:

				InputOscElementObj();
				~InputOscElementObj();

	void			AddAddressPattern(const char* pattern);

	void			SetTweakFocusTarget(int target);
	void			SetFloatValues
				(
					int	floatIndex,
					float	floatMapZero,
					float	floatMapOne,
					float	floatDefault
				);
	
	bool			GetTweak();
	int			GetTweakFocusTarget();
	float			GetFloat() const;

	bool			ProcessMessage
				(
					const osc::ReceivedMessage&	m,
					const IpEndpointName&		remoteEndpoint
				);
	
	void			SwapBackFront();

private:

	std::vector<char*>	AddressPatterns;

	bool			TweakBack;
	bool			TweakFront;
	int			TweakFocusTarget;

	int			FloatIndex;
	float			FloatMapZero;
	float			FloatMapOne;
	float			FloatDefault;
	float			FloatBack;
	float			FloatFront;

	LGL_Semaphore		BackFrontSemaphore;

};

#endif	//_INPUT_OSC_ELEMENT_H_

