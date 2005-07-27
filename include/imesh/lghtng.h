/*
    Copyright (C) 2003 by Boyan Hristov

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

#ifndef __CS_IMESH_LGHTNG_H__
#define __CS_IMESH_LGHTNG_H__

#include "csutil/scf.h"

struct iMaterialWrapper;

class csVector3;

SCF_VERSION (iLightningFactoryState, 0, 0, 1);

/// Document me! @@@
struct iLightningFactoryState : public iBase
{
  /// Set material of sprite.
  virtual void SetMaterialWrapper (iMaterialWrapper* material) = 0;
  /// Get material of sprite.
  virtual iMaterialWrapper* GetMaterialWrapper () const = 0;
  /// Set mix mode.
  virtual void SetMixMode (uint mode) = 0;
  /// Get mix mode.
  virtual uint GetMixMode () const = 0;

  /// Set the point of origin, the center of the texture.
  virtual void SetOrigin (const csVector3& pos) = 0;
  /// Get the point of origin
  virtual const csVector3& GetOrigin () const = 0;

  ///
  virtual float GetLength () const = 0;
  /// 
  virtual void SetLength (float value) = 0;
  /// 
  virtual int GetPointCount () const = 0;
  /// 
  virtual void SetPointCount (int n) = 0;
  /// 
  virtual float GetWildness () const = 0;
  ///
  virtual void SetWildness (float value) = 0;
  ///
  virtual float GetVibration () const = 0;
  ///
  virtual void SetVibration (float value) = 0;
  ///
  virtual void SetDirectional (const csVector3 &pos) = 0;
  ///
  virtual const csVector3& GetDirectional () = 0;
  ///
  virtual csTicks GetUpdateInterval () const = 0;
  ///
  virtual void SetUpdateInterval (csTicks value) = 0;
  ///
  virtual float GetBandWidth () const = 0;
  ///
  virtual void SetBandWidth (float value) = 0;
};

SCF_VERSION (iLightningState, 0, 0, 1);

///
struct iLightningState : public iLightningFactoryState
{
};

#endif // __CS_IMESH_LGHTNG_H__

