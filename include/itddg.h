/*
    Copyright (C) 2001 by Jorrit Tyberghein
    Plug-In by Richard D Shank
  
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

#ifndef __ITDDG_H__
#define __ITDDG_H__

#include "csutil/scf.h"

struct iEngine;

SCF_VERSION (iDDGState, 0, 0, 1);

/**
 * This interface describes the API for the terrain object.
 */
struct iDDGState : public iBase
{
  /// Set the heightmap file name.
  virtual bool LoadHeightMap (const char *pHeightmapName) = 0;
  /// load a single material to be used in the heightfield.
  virtual bool LoadMaterial (const char *pMaterial) = 0;
  /// load a material group to be used in the heightfield.
  virtual bool LoadMaterialGroup (const char *pGroup, int iStart, int iEnd) = 0;
  /// set the Level Of Detail.
  virtual bool SetLOD (int detailLevel) = 0;
  /// set the engine (@@@ remove when engine is a plugin).
  virtual bool SetEngine (iEngine *pEng) = 0;
};

#endif // __ITDDG_H__

