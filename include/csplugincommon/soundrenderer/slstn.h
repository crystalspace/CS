/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __CS_SLSTN_H__
#define __CS_SLSTN_H__

#include "csextern.h"
#include "isound/listener.h"

class CS_CRYSTALSPACE_EXPORT csSoundListener : public iSoundListener
{
public:
  SCF_DECLARE_IBASE;
  csSoundListener();
  virtual ~csSoundListener();

  virtual void SetDirection (const csVector3 &Front, const csVector3 &Top);
  virtual void SetPosition (const csVector3 &pos);
  virtual void SetVelocity (const csVector3 &v);
  virtual void SetDistanceFactor (float factor);
  virtual void SetRollOffFactor (float factor);
  virtual void SetDopplerFactor (float factor);
  virtual void SetHeadSize (float size);
  virtual void SetEnvironment (csSoundEnvironment env);
  virtual void GetDirection (csVector3 &Front, csVector3 &Top);
  virtual const csVector3 &GetPosition ();
  virtual const csVector3 &GetVelocity ();
  virtual float GetDistanceFactor ();
  virtual float GetRollOffFactor ();
  virtual float GetDopplerFactor ();
  virtual float GetHeadSize ();
  virtual csSoundEnvironment GetEnvironment ();

protected:
  csVector3 Position, Velocity;
  csVector3 Front, Top;
  float DistanceFactor, RollOffFactor, DopplerFactor;
  float HeadSize;
  csSoundEnvironment Environment;
};

#endif // __CS_SLSTN_H__
