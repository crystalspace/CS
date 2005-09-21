/*
	Copyright (C) 2004 by Andrew Mann

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

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "iutil/objreg.h"
#include "isndsys/ss_listener.h"

#include "listener.h"



SCF_IMPLEMENT_IBASE(SndSysListenerSoftware)
	SCF_IMPLEMENTS_INTERFACE(iSndSysListener)
SCF_IMPLEMENT_IBASE_END;


SndSysListenerSoftware::SndSysListenerSoftware()
{
  SCF_CONSTRUCT_IBASE(0);

  active_properties.front.Set(0,0,1.0f);
  active_properties.top.Set(0.0f,1.0f,0.0f);
  active_properties.position.Set(0.0f,0.0f,0.0f);
  active_properties.distance_factor=1.0f;
  active_properties.rolloff_factor=1.0f;

  queued_properties.Copy(&active_properties);

  queued_update=false;
}

SndSysListenerSoftware::~SndSysListenerSoftware()
{
  SCF_DESTRUCT_IBASE();
}

void SndSysListenerSoftware::SetDirection(const csVector3 &Front, const csVector3 &Top)
{
  queued_properties.front.Set(Front.Unit());
  queued_properties.top.Set(Top.Unit());
  queued_update=true;
}

void SndSysListenerSoftware::SetPosition(const csVector3 &pos)
{
  queued_properties.position.Set(pos);
  queued_update=true;
}

void SndSysListenerSoftware::SetDistanceFactor(float factor)
{
  queued_properties.distance_factor=factor;
  queued_update=true;
}

void SndSysListenerSoftware::SetRollOffFactor(float factor)
{
  queued_properties.rolloff_factor=factor;
  queued_update=true;
}

/*
void SndSysListenerSoftware::SetEnvironment(csSoundEnvironment env)
{

}
*/


void SndSysListenerSoftware::GetDirection(csVector3 &Front, csVector3 &Top)
{
  Front.Set(active_properties.front);
  Top.Set(active_properties.top);
}

const csVector3 &SndSysListenerSoftware::GetPosition()
{
  return active_properties.position;
}

float SndSysListenerSoftware::GetDistanceFactor()
{
  return active_properties.distance_factor;
}

float SndSysListenerSoftware::GetRollOffFactor()
{
  return active_properties.rolloff_factor;
}


/*
csSoundEnvironment SndSysListenerSoftware::GetEnvironment()
{
  return (csSoundEnvironment) 0;
}
*/


void SndSysListenerSoftware::UpdateQueuedProperties()
{
  if (queued_update)
  {
    active_properties.Copy(&queued_properties);
    queued_update=false;
  }
}


















