/*
    Metaballs
    Copyright (C) 1999 by Denis Dmitriev
    Pluggified (c) 2000 by Samuel Humphreys

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

#ifndef __IMESH_METABALL_H__
#define __IMESH_METABALL_H__

#include "csutil/scf.h"

struct iMaterialWrapper;

struct MetaParameters
{
  float iso_level;
  float charge;
  float rate;
};

enum EnvMappingModes    {TRUE_ENV_MAP,FAKE_ENV_MAP};

SCF_VERSION (iMetaBallState, 0, 0, 1);

///
struct iMetaBallState : public iBase
{
  /// Get number of balls currently being animated
  virtual int GetMetaBallCount () = 0;

  /// Set the number of balls being animated
  virtual void SetMetaBallCount (int number) = 0;

  /// Set True for true environment mapping, false for fake style mapping
  virtual void SetQualityEnvironmentMapping (bool toggle) = 0;

  ///
  virtual bool GetQualityEnvironmentMapping () = 0;

  ///
  virtual float GetEnvironmentMappingFactor () = 0;

  ///
  virtual void SetEnvironmentMappingFactor (float env_mult) = 0;

  /// Get modifiable parameters
  virtual MetaParameters *GetParameters () = 0;

  /// Set material to be environmentally mapped
  virtual void SetMaterial (iMaterialWrapper *mat) = 0;

  /// For statistics only
  virtual int ReportTriangleCount () = 0;

  /// Regular lighting and mixmode settings  
  virtual UInt GetMixMode () = 0;
  
  virtual void SetMixMode ( UInt mode ) = 0;
  
  virtual bool IsLighting () = 0;
  
  virtual void SetLighting ( bool set ) = 0;
  
  virtual iMaterialWrapper* GetMaterial () = 0;
};

#endif //  __IMESH_METABALL_H__
