/*
    Crystal Space 3D engine
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

#ifndef __ISECTOR_H__
#define __ISECTOR_H__

#include "csutil/scf.h"

SCF_VERSION (iSector, 0, 1, 0);

/**
 * The iSector interface is used to work with "sectors". A "sector"
 * is a convex polyhedron, that possibly contains portals, things,
 * sprites, lights and so on. Simply speaking, a "sector" is analogous
 * to an real-world room, rooms are interconnected with doors and windows
 * (e.g. portals), and rooms contain miscelaneous things, sprites
 * and lights.
 */
struct iSector : public iBase
{
};

#endif // __ISECTOR_H__
