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


#ifndef SNDSYS_RENDERER_SOFTWARE_LISTENER_H
#define SNDSYS_RENDERER_SOFTWARE_LISTENER_H

#include "csgeom/transfrm.h"

class ListenerProperties
{
public:
  ListenerProperties() {}
  ListenerProperties(ListenerProperties *copy_from) { Copy(copy_from); }
  ~ListenerProperties() {}

  void Copy(ListenerProperties *copy_from)
  {
    front.Set(copy_from->front);
    top.Set(copy_from->top);
    position.Set(copy_from->position);
    distance_factor=copy_from->distance_factor;
    rolloff_factor=copy_from->rolloff_factor;

    // Setup the world-to-listener translation
    world_to_listener.SetOrigin(position);
    csVector3 rightvec;
    rightvec.Cross(top, front);


    world_to_listener.SetO2T(csMatrix3(rightvec.x, rightvec.y, rightvec.z,
                                       top.x, top.y, top.z,
                                       front.x, front.y, front.z));
  }

public:
  csVector3 front,top,position;
  float distance_factor,rolloff_factor;
  csTransform world_to_listener;
};

class SndSysListenerSoftware : public iSndSysListener
{
public:
  SCF_DECLARE_IBASE;

  SndSysListenerSoftware();
  virtual ~SndSysListenerSoftware();

  /// Set direction of listener (front and top 3d vectors)
  virtual void SetDirection (const csVector3 &Front, const csVector3 &Top);
  /// Set position of listener
  virtual void SetPosition (const csVector3 &pos);
  /// Set a distance attenuator
  virtual void SetDistanceFactor (float factor);
  /// Set a RollOff factor
  virtual void SetRollOffFactor (float factor);
  /// set type of environment where 'live' listener
  //virtual void SetEnvironment (csSoundEnvironment env);


  /// Get direction of listener (front and top 3d vectors)
  virtual void GetDirection (csVector3 &Front, csVector3 &Top);
  /// Get position of listener
  virtual const csVector3 &GetPosition ();
  /// Get a distance attenuator
  virtual float GetDistanceFactor ();
  /// Get a RollOff factor
  virtual float GetRollOffFactor ();
  /// Get type of environment where 'live' listener
  //virtual csSoundEnvironment GetEnvironment ();

  /// Migrate any queued changes to the active set
  void UpdateQueuedProperties();

public:
  ListenerProperties active_properties,queued_properties;
  bool queued_update;
};



#endif // #ifndef SNDSYS_RENDERER_SOFTWARE_LISTENER_H



