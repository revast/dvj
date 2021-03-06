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

#include "Config.h"

class OscElementObj
{

public:

				OscElementObj();
				~OscElementObj();

	void			SetDVJAction(DVJ_Action action);

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

	void			SetSticky(bool sticky=true);
	bool			GetSticky() const;
	bool			GetStickyNow() const;
	void			Unstick();

	bool			GetTweak() const;
	unsigned int		GetTweakFocusTarget() const;
	float			GetFloat() const;
	float			GetFloatDelta() const;
	float			GetFloatDefault() const;
	float			ConvertOscToDvj(float osc);
	float			ConvertDvjToOsc(float dvj);
	float			GetLastSentFloat() const;
	void			SetLastSentFloat(float sent);
	void			SetSendDefaultOnZRelease(bool send=true);
	const char*		GetRemoteControllerBack() const;
	const char*		GetRemoteControllerFront() const;
	std::vector<char*>&	GetAddressPatterns();

	bool			ProcessMessage
				(
					const osc::ReceivedMessage&	m,
					const IpEndpointName&		remoteEndpoint
				);

	void			SwapBackFront();

private:

	void			AddAddressPattern(const char* pattern);
	void			UpdateAddressPatterns();
	void			ClearAddressPatterns();

	DVJ_Action		Action;

	std::vector<char*>	AddressPatterns;

	bool			Sticky;
	bool			StickyNow;
	bool			SendDefaultOnZRelease;
	float			LastSentFloat;
	float			LastRecvFloat;

	bool			TweakBack;
	bool			TweakFront;
	unsigned int		TweakFocusTarget;

	int			FloatIndex;
	float			FloatMapZero;
	float			FloatMapOne;
	float			FloatDefault;
	float			FloatBack;
	float			FloatFront;
	float			FloatDeltaBack;
	float			FloatDeltaFront;
	LGL_Timer		FloatDeltaDeltaTimer;

	char			RemoteControllerBack[2048];
	char			RemoteControllerFront[2048];

	LGL_Semaphore		BackFrontSemaphore;

	MasterInputGetFnType	MasterInputGetFn;

};

#endif	//_OSC_ELEMENT_H_

