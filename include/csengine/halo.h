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

#ifndef HALO_H
#define HALO_H

#include "csgeom/math3d.h"
#include "light.h"
#include "ihalo.h"

/**
 * This is used to keep track of halos.<p>
 * When the engine detects that a light that is marked to have an halo
 * is directly visible, an object of this type is created and put into
 * a global queue maintained within the world object. The light starts
 * to brighten until it reaches maximal intensity; when the halo becomes
 * obscured by something or goes out of view the intensity starts to
 * decrease until it reaches zero; upon this event the halo object is
 * destroyed and removed from halo queue.
 */
class csLightHalo
{
public:
  /// The light this halo is attached to
  csLight *Light;

  /// Halo handle as returned by 3D rasterizer
  iHalo *Handle;

  /// Create an light halo object
  csLightHalo (csLight *iLight, iHalo *iHandle)
  {
    Handle = iHandle;
    (Light = iLight)->SetHaloInQueue (true);
  }

  /// Destroy the light halo object
  ~csLightHalo ()
  {
    if (Handle)
      Handle->DecRef ();
    if (Light)
      Light->SetHaloInQueue (false);
  }
};

#endif /*HALO_H*/
