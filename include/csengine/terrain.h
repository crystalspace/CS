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

#ifndef TERRAIN_H
#define TERRAIN_H

#include "csgeom/transfrm.h"
#include "csengine/rview.h"
#include "csobject/csobj.h"

class ddgTBinMesh;
class ddgHeightMap;
class ddgBBox;

/**
 * This object encapsulates a terrain surface so that it
 * can be used in a csSector.
 */
class csTerrain : public csObject
{
private:
  ///
  ddgTBinMesh* mesh;
  ///
  ddgHeightMap* height;
  ///
  ddgBBox* clipbox;

public:
  /**
   * Create an empty terrain.
   */
  csTerrain ();

  /// Destructor.
  virtual ~csTerrain ();

  ///
  ddgTBinMesh* GetMesh () { return mesh; }
  ///
  ddgHeightMap* GetHeightMap () { return height; }
  ///
  ddgBBox* GetBBox () { return clipbox; }

  ///
  bool Initialize (char* heightmap);

  /**
   * Draw this terrain given a view and transformation.
   */
  void Draw (csRenderView& rview, bool use_z_buf = true);

  CSOBJTYPE;
};

#endif /*TERRAIN_H*/

