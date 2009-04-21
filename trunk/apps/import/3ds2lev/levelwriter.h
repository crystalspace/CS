/*
    Crystal Space 3ds2lev xml writer
    Copyright (C) 2002 by Matze Braun

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
#ifndef __LEVELWRITER_H__
#define __LEVELWRITER_H__

#include <stdarg.h>

#include <csutil/ref.h>
#include <iutil/document.h>

// includes for lib3ds
#include <lib3ds/camera.h>
#include <lib3ds/file.h>
#include <lib3ds/io.h>
#include <lib3ds/light.h>
#include <lib3ds/material.h>
#include <lib3ds/matrix.h>
#include <lib3ds/mesh.h>
#include <lib3ds/node.h>
#include <lib3ds/vector.h>

class csDVector3;
class csDPlane;
class csTinyDocumentSystem;

class LevelWriter
{
public:
  enum
  {
    FLAG_VERBOSE  = 0x0001,
    FLAG_LIGHTING = 0x0002,
    FLAG_SWAP_V	  = 0x0010,
    FLAG_COMBINEFACES = 0x0020,
    FLAG_REMOVEDOUBLEVERTICES = 0x0040,
    FLAG_CLEARZBUFCLEARSCREEN = 0x0080
  };
  
  LevelWriter();
  ~LevelWriter();
  
  void Set3dsFile (Lib3dsFile* file3ds);
  void SetFlags (int flags);
  csPtr<iDocument> WriteDocument ();
  csPtr<iDocument> WriteSprite (const char* name);
  
  void WriteTexturesMaterials (iDocumentNode* worldnode);
  void WritePlugins (iDocumentNode* worldnode);
  void WriteStartPoints (iDocumentNode* worldnode);

  void WriteObjects (iDocumentNode* sectornore);
  void WriteLights (iDocumentNode* sectornode);
  
  void WriteVertices (iDocumentNode* paramsnode, Lib3dsMesh* mesh,
		      bool writesprite = false);
  void WriteFaces (iDocumentNode* paramsnode, Lib3dsMesh* mesh,
		   bool lighting, unsigned int numMesh,
		   bool writesprite = false);

  void SetScale(float x, float y, float z);
  void SetTranslate(float x, float y, float z);
protected:
  inline bool CombineTriangle (Lib3dsMesh* mesh, csDPlane*& plane, int* poly,
      int& plen, int trinum);
    
  Lib3dsFile* p3dsFile;
  int* newpointmap;
  csDVector3* vectors;
  csDPlane* planes;
  bool* used;
  float xscale, yscale, zscale;
  float xrelocate, yrelocate, zrelocate;

  int flags;
  csRef<csTinyDocumentSystem> xml;
};

#endif // __LEVELWRITER_H__

