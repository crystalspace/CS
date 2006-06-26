/*
	Copyright (C) 2006 by Søren Bøg

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


#ifndef SNDSYS_RENDERER_OPENAL_LISTENER_H
#define SNDSYS_RENDERER_OPENAL_LISTENER_H

#include "cssysdef.h"

#include "csutil/scf_implementation.h"

#include "isndsys/ss_listener.h"

class SndSysListenerOpenAL : 
  public scfImplementation1<SndSysListenerOpenAL, iSndSysListener>
{
public:
  SndSysListenerOpenAL();
  virtual ~SndSysListenerOpenAL();

  /*
   * iSndSysListener interface
   */
public:
  /// Set direction of listener (front and top 3d vectors)
  virtual void SetDirection (const csVector3 &Front, const csVector3 &Top);
  /// Set position of listener
  virtual void SetPosition (const csVector3 &pos);
  /// Set a distance attenuator
  virtual void SetDistanceFactor (float factor);
  /// Set a RollOff factor
  virtual void SetRollOffFactor (float factor);

  /// Get direction of listener (front and top 3d vectors)
  virtual void GetDirection (csVector3 &Front, csVector3 &Top);
  /// Get position of listener
  virtual const csVector3 &GetPosition ();
  /// Get a distance attenuator
  virtual float GetDistanceFactor ();
  /// Get a RollOff factor
  virtual float GetRollOffFactor ();

  /*
   * SndSysListenerOpenAL implementation
   */
public:
  /// Set the listener volume
  float SetVolume (float vol);
  /// Get the listener volume
  float GetVolume ();
  /// Perform any pending updates
  void Update ();

private:
  csVector3 Front, Top;
  csVector3 Position;
  float Distance, RollOff;
  float Volume;
  bool update;
};

#endif // #ifndef SNDSYS_RENDERER_OPENAL_LISTENER_H
