/*
    Copyright (C) 2000 by Jorrit Tyberghein
    Authored by Brandon Ehle

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

#ifndef __ISPAWN_H__
#define __ISPAWN_H__

#include "csutil/scf.h"

#define DECLARE_SPAWNER(interface, csclass) \
  inline interface* new_##csclass (iSCF* scf) { \
    return scf->scfCreateInstance("crystalspace.class."#csclass, #interface, VERSION_##interface); \
  }

//Azverkan This is a tad screwy, but works like a charm
#ifndef DECLARE_CSCLASS
#define DECLARE_CSCLASS(csclass) \
  struct i##csclass; \
  DECLARE_SPAWNER(i##csclass, cs##csclass)
#endif

/* Azverkan
  To add new classes to Crystal Script
  1) Put any new classes that you wish to wrap down here
  2) Edit spawn.cpp and add the necessary include paths
  3) Call the corresponding new_cs<class name> (iSCF* scf); function
       i.e.    iSector* sector=new_csSector(scf);
*/

DECLARE_CSCLASS(Sector);
DECLARE_CSCLASS(Camera);
DECLARE_CSCLASS(PolygonSet);
DECLARE_CSCLASS(PolygonTexture);

#endif
