/*
    Copyright (C) 1998 by Jorrit Tyberghein

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


/*----------------------------------------------------------------------------
  Handles the different lights.  Used to determine the intensity
  of a light at any given moment.  Allows for flashing and flickering lights.

  Lights basically have two states, like on and off.  They are called
  Active and Inactive.  A light will normally be in one of the two states
  unless it is a stateless light in which case it will loop through all states.
  When a light is active, it will change from the primary active intensity
  to the secondary active intensity and back again during the period specified.
  Similarly when it is inactive with the primary and secondary inactive states.
  When a light is inactive, and it is told to become active (player hits a switch)
  it will go into the becoming active state and then into the primary active state
  and loop between primary and secondary states.  Similarly when it is turned off.
  This allows for example a flourescent light that can be off, but when it is
  turned on it flickers madly for a while, then snaps into the on state where
  it pulsates a bit at a high frequency.

  The intensity of a light is in 16.16 fixed point format and should probably always
  be less than 65536. The period is in 60ths of a second.

  Not all the light functions are implemented.  Currently only constant and
  linear transitions are implemented.
  The random variances are not implemented yet either.
----------------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>

#include "sysdef.h"
#include "csengine/dynlight.h"
#include "csengine/world.h"
#include "csutil/util.h"
#include "csutil/garray.h"
#include "isystem.h"

IMPLEMENT_CSOBJTYPE (CLights, csObject);

DECLARE_GROWING_ARRAY (static, theLights, CLights *);
static int numLights = 0;

static inline unsigned long CLOCK ()
{
  time_t tm = csWorld::System->GetTime ();
  return (unsigned long)(tm * 60 / 1000);
}

void CLights::AddLight (CLights *newLight)
{
  if (numLights >= theLights.GetLimit ())
    theLights.SetLimit (numLights + 8);

  theLights [numLights++] = newLight;
}

void CLights::DeleteLight (CLights *newLight)
{
  for (int x = 0; x < numLights; ++x)
    if (theLights [x] == newLight)
    {
      theLights.Delete (x);
      numLights--;
      break;
    }
}

void CLights::DeleteAll ()
{
  for (int x = numLights - 1; x >= 0; x--)
    delete theLights [x];
  theLights.SetLimit (0);
}

CLights *CLights::FindByName(char *theName)
{
  for (int x = 0; x < numLights; ++x)
  {
    if (!strcmp (theLights [x]->GetName (), theName))
      return theLights [x];
  }
  return (CLights *)NULL; // this could be bad news
}

/* Call this to give the lights some idle time. */
void CLights::LightIdle ()
{
  for (int x = 0; x < numLights; ++x)
    theLights [x]->Idle ();
}

CLights::CLights () : csObject ()
{
  theLights.IncRef ();
  AddLight (this);
  resultIntensity = 0; // JTY: initialize for safety
}

CLights::~CLights()
{
  DeleteLight (this);
  theLights.DecRef ();
}

void CLights::SetFunctionData(long which, long type, unsigned long period,
  unsigned long deltaP, unsigned long intensity, unsigned long deltaI)
{
  functions[which].type = type;
  functions[which].period = period;
  functions[which].dp = deltaP;
  functions[which].intensity = intensity;
  functions[which].di = deltaI;
}

/* After the light is fully constructed, call this to start the light. */
void CLights::Start ()
{
  timer = CLOCK () - 1;  // force the state data to be calced NOW
  state = (initallyActive) ? kStatePrimaryActive : kStatePrimaryInactive;
  curIntensity = functions[state].intensity;
  targetPeriod = 0; // Initialize (JTY)
  Idle (); // calc the lights current intensity
}

unsigned long CLights::RawIntensity ()
{
  return resultIntensity;
}

/*----------------------------------------------------------------------------
  The different light types will vary the light levels over time
  according to their settings.  Anything is possible with a complex
  enough algorythm here.
----------------------------------------------------------------------------*/
void CLights::Idle ()
{
  // should we change states?
  unsigned long deltaTime = timer - CLOCK ();
  if (deltaTime > targetPeriod) // change states
  {
    ChangeState ();
    return;
  }

  lightFunction &curFn = functions [state];
  switch (curFn.type)
  {
    default:
    case kConstant:
      resultIntensity = curIntensity = targetIntensity;
      break;
    case kConstantFlicker:
      resultIntensity = RndNum(curIntensity, targetIntensity); // flicker between them
      break;
    case kFlicker:
      // subtract/add the fraction of the delta based on the fraction of the time
      if (targetIntensity > curIntensity)
        curIntensity = targetIntensity - (deltaIntensity * deltaTime) / targetPeriod;
      else
        curIntensity = targetIntensity + (deltaIntensity * deltaTime) / targetPeriod;
      resultIntensity = RndNum(curIntensity, targetIntensity); // flicker between them
      break;
    case kLinear:
    case kSmooth:
      // subtract/add the fraction of the delta based on the fraction of the time
      if (targetIntensity > curIntensity)
        curIntensity = targetIntensity - (deltaIntensity * deltaTime) / targetPeriod;
      else
        curIntensity = targetIntensity + (deltaIntensity * deltaTime) / targetPeriod;
      resultIntensity = curIntensity;
      break;
  }
}

void CLights::ChangeState ()
{
  // ensure the current intensity is actually reached exactly
  curIntensity = functions[state].intensity;

  ++state;
  if (stateless)
  {
    if (state > kStateSecondaryInactive)
      state = kStateBecomingActive;
  }
  else
  {
    if (state > kStateSecondaryInactive)    // loop in the inactive state
      state = kStatePrimaryInactive;
    else if (state == kStateBecomingInactive) // loop in the active state
      state = kStatePrimaryActive;
  }

  // we need to calc the timer and the rate
  lightFunction   &curFn = functions[state];

  // calculate the next state change time period
  targetPeriod = curFn.period + RndNum(-long(curFn.dp), curFn.dp);
  // calc the actual time for the next change
  timer = CLOCK () + targetPeriod;

  // calculate the new intensity we are aiming for
  targetIntensity = curFn.intensity + RndNum(-long(curFn.di), curFn.di);
  targetIntensity &= 0x0000ffff; // ensure it is in the correct range
  // calc the change in intensity required
  if (targetIntensity > curIntensity) // unsigned longs require this comparison
    deltaIntensity = targetIntensity - curIntensity;
  else
    deltaIntensity = curIntensity - targetIntensity;
}
