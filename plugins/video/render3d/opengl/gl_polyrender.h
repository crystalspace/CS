/*
    Copyright (C) 2003 by Jorrit Tyberghein
              (C) 2003 by Frank Richter
              (C) 2005 by Marten Svanfeldt

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

#ifndef __CS_GL_POLYRENDER_H__
#define __CS_GL_POLYRENDER_H__

#include "csutil/array.h"
#include "csutil/leakguard.h"
#include "ivideo/polyrender.h"
#include "cstool/userrndbuf.h"

class csGLGraphics3D;
class csShaderVariableContext;

class csGLPolygonRenderer : public iPolygonRenderer
{
private:
  csGLGraphics3D* parent;
  uint renderBufferNum;
  uint polysNum;

  csArray<csPolygonRenderData*> polys;
  csRefArray<iUserRenderBufferIterator> extraBuffers;
  csRef<iShaderManager> shadermanager;

  csRef<csRenderBufferHolder> bufferHolder;

  csRef<iRenderBuffer> vertex_buffer;
  csRef<iRenderBuffer> texel_buffer;
  csRef<iRenderBuffer> index_buffer;
  csRef<iRenderBuffer> lmcoords_buffer;
  uint rbIndexStart, rbIndexEnd;

  csRef<csShaderVariableContext> svcontext;
  
  void PrepareBuffers (uint& indexStart, uint& indexEnd);

  class BufferAccessor : public iRenderBufferAccessor
  {
  private: 
    csGLPolygonRenderer *renderer;
    csRef<iRenderBuffer> normal_buffer;
    csRef<iRenderBuffer> binormal_buffer;
    csRef<iRenderBuffer> tangent_buffer;
    
    uint normalVerticesNum;
    uint binormalVerticesNum;
    uint tangentVerticesNum;
  public:
    CS_LEAKGUARD_DECLARE (BufferAccessor);
    SCF_DECLARE_IBASE;

    BufferAccessor (csGLPolygonRenderer *renderer)
      : renderer(renderer), normalVerticesNum (0), binormalVerticesNum (0),
        tangentVerticesNum (0)
    {
      SCF_CONSTRUCT_IBASE(0);
    }

    virtual ~BufferAccessor()
    {
      SCF_DESTRUCT_IBASE();
    }

    virtual void PreGetBuffer (csRenderBufferHolder* holder, csRenderBufferName buffer);
    
    bool UpdateNormals ();
    bool UpdateBinormals ();
    bool UpdateTangents ();
  };
  friend class BufferAccessor;
  csRef<BufferAccessor> buffer_accessor;

public:
  CS_LEAKGUARD_DECLARE (csGLPolygonRenderer);

  SCF_DECLARE_IBASE;

  csGLPolygonRenderer (csGLGraphics3D* parent);
  virtual ~csGLPolygonRenderer ();

  // ---- iPolygonRenderer ----
  virtual void PrepareRenderMesh (csRenderMesh& mesh);
  
  virtual void Clear ();
  virtual void AddPolygon (csPolygonRenderData* poly,
    iUserRenderBufferIterator* extraBuffers);

};

#endif // __CS_GL_POLYRENDER_H__
