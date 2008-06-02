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
  public scfImplementation2<SndSysListenerOpenAL,
                            iSndSysListener,
                            iSndSysListenerDoppler>
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
   * iSndSysListenerDoppler interface
   */
public:
  /// Set velocity (speed) of the listener
  virtual void SetVelocity (const csVector3 &Velocity);
  /// Set the Doppler factor
  virtual void SetDopplerFactor (const float DopplerFactor);
  /// Set the speed of sound
  virtual void SetSpeedOfSound (const float SpeedOfSound);

  /// Get velocity (speed) of th elistener
  virtual const csVector3 &GetVelocity ();
  /// Get the Doppler factor
  virtual float GetDopplerFactor ();
  /// Get the speed of sound
  virtual float GetSpeedOfSound ();

  /*
   * SndSysListenerOpenAL implementation
   */
public:
  /// Set the listener volume
  void SetVolume (float vol);
  /// Get the listener volume
  float GetVolume ();
  /**
   * Perform any pending updates
   * @return true if some changes need to be propagates, false otherwise.
   */
  bool Update ();

private:
  /// Direction vectors
  csVector3 m_Front, m_Top;
  /// Position of the listener
  csVector3 m_Position;
  /// Current distance and rolloff factoes
  float m_Distance, m_RollOff;
  /// Current volume setting
  float m_Volume;
  /// Velocity of the listener
  csVector3 m_Velocity;
  /// The global doppler factor
  float m_DopplerFactor;
  /// The speed of sound
  float m_SpeedOfSound;
  /// Flag indicating that updates are pending.
  bool m_Update;
  /// Flag indicating that updates are pending that need propagation.
  bool m_ExternalUpdate;

  /// The renderer this listener is attached to
  //iSndSysRendererOpenAL *m_Renderer;

};

#endif // #ifndef SNDSYS_RENDERER_OPENAL_LISTENER_H
