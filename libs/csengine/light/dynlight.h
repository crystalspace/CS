/*
    Copyright (C) 1998 by Jorrit Tyberghein and Steve Israelson
  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.
  
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.
  
    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef DYNLIGHT_H
#define DYNLIGHT_H

#include "csobject/csobj.h"

#define kMaxLights		16

class CLights;

/**
 * This defines the parameters for the particular light states
 */
typedef struct {
	long				type;					// the type of light
	unsigned long		period;					// the period this state is active in 60ths of a second
	unsigned long		dp;						// random variance in period
	unsigned long		intensity;				// 16.16 fixed point intensity from 0 - 1.0
	unsigned long		di;						// random variance in intensity
	} lightFunction;

extern const    char *kDynamicLightID;

/**
 * A uniform dynamic light.
 */
class CLights : public csObject
	{
public:
	///
	enum		{kConstant = 0, kLinear, kSmooth, kFlicker, kConstantFlicker};
	///
	enum		{kStateBecomingActive = 0, kStatePrimaryActive, kStateSecondaryActive,
					kStateBecomingInactive, kStatePrimaryInactive, kStateSecondaryInactive};

	///
	static long		numLights;
	///
	static CLights	*theLights[kMaxLights];

	///
	static void		AddLight(CLights *newLight);
	///
	static void		DeleteLight(CLights *newLight);
	///
	static void		LightIdle(void);
	///
	static CLights	*FindByName(char *theName);

	///
	CLights();
	///
	~CLights();
	///
	void			SetStateType(bool stateType) { stateless = stateType;};
	///
	void			SetInitallyActive(bool active) { initallyActive = active;};
	///
	void			SetFunctionData(long which, long type, unsigned long period, unsigned long deltaP,
								unsigned long intensity, unsigned long deltaI);
	///
	void			Start(void);
	///
	void			Idle(void);
	/// Return the current intensity of the light
	unsigned long	RawIntensity(void);
	///
	void			ChangeState(void);

private:
	/// Does not stay in the on or off state but always changes
	bool				stateless;
	/// Starts in the on state
	bool				initallyActive;
	/// the lighting state functions
	lightFunction		functions[6];
	/// which function we are currently in
	long				state;
	/// the next time we will change functions
	unsigned long		timer;
	/// the current intensity of the light
	unsigned long		curIntensity;
	/// the previous intensity.
	unsigned long		prevIntensity;
	/// the needed change in intensity.
	unsigned long		deltaIntensity;
	/// the new intensity we are trying to get to
	unsigned long		targetIntensity;
	/// the time we are going to take to get there
	unsigned long		targetPeriod;

	/// the intensity of the light returned when queried 
	unsigned long		resultIntensity;
   
 CSOBJTYPE;
	};


#endif

