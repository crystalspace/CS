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

class csSoftPolygonRenderer : public iPolygonRenderer,
			      public iRenderBuffer
{
  csSoftwareGraphics3DCommon* parent;
  uint renderBufferNum;
  uint polysNum;

  uint rbIndexStart, rbIndexEnd;

  void PrepareBuffers (uint& indexStart, uint& indexEnd);
public:
  csArray<csPolygonRenderData*> polys;
  csArray<int> rlmIDs;

  SCF_DECLARE_IBASE;

  csSoftPolygonRenderer (csSoftwareGraphics3DCommon* parent);
  virtual ~csSoftPolygonRenderer ();

  virtual void PrepareRenderMesh (csRenderMesh& mesh);

  virtual void Clear ();
  virtual void AddPolygon (csPolygonRenderData* poly);

  virtual void* Lock(csRenderBufferLockType lockType) 
  { return 0; }
  virtual void Release() {}
  virtual void CopyInto (const void *data, size_t length) {}
  virtual int GetComponentCount () const { return 0; }
  virtual csRenderBufferComponentType GetComponentType () const 
  { return CS_BUFCOMP_FLOAT; }
  virtual csRenderBufferType GetBufferType() const
  { return CS_BUF_STATIC; }
  virtual size_t GetSize() const { return 0; }
  virtual size_t GetStride() const { return 0; }
  virtual size_t GetElementDistance() const { return 0; }
  virtual size_t GetOffset() const { return 0; }
  virtual uint GetVersion () {return 0;}
  virtual iRenderBuffer* GetMasterBuffer () const { return 0; }
  virtual bool IsIndexBuffer() const { return false; }
  virtual size_t GetRangeStart() const { return 0; }
  virtual size_t GetRangeEnd() const { return 0; }
};

