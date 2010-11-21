/*
 *
 * OscElement.h - Input abstraction object
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

#ifndef	_OSC_ELEMENT_H_
#define	_OSC_ELEMENT_H_

#include "LGL.module/LGL.h"

class OscElementObj
{

public:

				OscElementObj();
				~OscElementObj();

	void			AddAddressPatternRecv(const char* pattern);
	void			AddAddressPatternSend(const char* pattern);
private:
	void			AddAddressPattern
				(
					const char*		pattern,
					std::vector<char*>&	patternList
				);
public:

	void			SetTweakFocusTarget(unsigned int target);
	void			SetFloatValues
				(
					int	floatIndex,
					float	floatMapZero,
					float	floatMapOne,
					float	floatDefault
				);

	typedef float(*MasterInputGetFnType)(unsigned int target);

	void			SetMasterInputGetFn(MasterInputGetFnType fn);
	MasterInputGetFnType	GetMasterInputGetFn();

	bool			GetTweak();
	unsigned int		GetTweakFocusTarget();
	float			GetFloat() const;
	float			ConvertOscToDvj(float osc);
	float			ConvertDvjToOsc(float dvj);
	const char*		GetRemoteControllerBack() const;
	const char*		GetRemoteControllerFront() const;
	std::vector<char*>&	GetAddressPatternsSend();

	bool			ProcessMessage
				(
					const osc::ReceivedMessage&	m,
					const IpEndpointName&		remoteEndpoint
				);

	void			SwapBackFront();

private:

	std::vector<char*>	AddressPatternsRecv;
	std::vector<char*>	AddressPatternsSend;

	bool			TweakBack;
	bool			TweakFront;
	unsigned int		TweakFocusTarget;

	int			FloatIndex;
	float			FloatMapZero;
	float			FloatMapOne;
	float			FloatDefault;
	float			FloatBack;
	float			FloatFront;

	char			RemoteControllerBack[2048];
	char			RemoteControllerFront[2048];

	LGL_Semaphore		BackFrontSemaphore;

	MasterInputGetFnType	MasterInputGetFn;

};

#endif	//_OSC_ELEMENT_H_

