/*
    Copyright (C) 2003 by Jorrit Tyberghein
              (C) 2003 by Frank Richter

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

#include "csutil/array.h"
#include "ivideo/polyrender.h"

class csSoftwareGraphics3DCommon;

class csSoftPolygonRenderer : public iPolygonRenderer
{
  csSoftwareGraphics3DCommon* parent;
  uint renderBufferNum;
  uint polysNum;

  csRef<iRenderBuffer> vertex_buffer;
  //csRef<iRenderBuffer> texel_buffer;
  //csRef<iRenderBuffer> normal_buffer;
  //csRef<iRenderBuffer> color_buffer;
  csRef<iRenderBuffer> index_buffer;

  static csStringID vertex_name;
  //static csStringID texel_name;
  //static csStringID normal_name;
  //static csStringID color_name;
  static csStringID index_name;

  uint rbIndexStart, rbIndexEnd;

  void PrepareBuffers (uint& indexStart, uint& indexEnd);
public:
  csArray<csPolygonRenderData*> polys;

  SCF_DECLARE_IBASE;

  csSoftPolygonRenderer (csSoftwareGraphics3DCommon* parent);
  virtual ~csSoftPolygonRenderer ();

  virtual void PrepareRenderMesh (csRenderMesh& mesh);

  virtual void Clear ();
  virtual void AddPolygon (csPolygonRenderData* poly);
};

