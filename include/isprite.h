/*
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

#ifndef __ISPRITE_H__
#define __ISPRITE_H__

#include "csutil/scf.h"

class csSprite;
class csSpriteTemplate;
struct iMovable;

SCF_VERSION (iSprite, 0, 0, 1);

/**
 * This is the generalized interface to iSprites.<p>
 * A sprite is a (movable) 3D model.
 */
struct iSprite : public iBase
{
  /// Used by the engine to retrieve internal sprite object (ugly)
  virtual csSprite *GetPrivateObject () = 0;

  /// Get the movable for this sprite.
  virtual iMovable* GetMovable () = 0;
};

SCF_VERSION (iSpriteTemplate, 0, 0, 1);

/**
 * A sprite template.
 */
struct iSpriteTemplate : public iBase
{
};

#endif

