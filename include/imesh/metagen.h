/*
    Metagen
    Copyright (C) 2001 by Michael H. Voase
    Copyright (C) 1999 by Denis Dmitriev
    Pluggified (c) 2000 by Samuel Humphreys

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

#ifndef __IMESH_METAGEN_H__
#define __IMESH_METAGEN_H__

#include "csutil/scf.h"

class csVector2;
class csVector3;
class csColor;
struct csTriangle;

SCF_VERSION (iMetaGen, 0, 0, 1 );

struct iMetaGen : iBase
{
  enum EnvMappingModes { TRUE_ENV_MAP, FAKE_ENV_MAP };

  virtual int GenerateLinearSurface( int bone_index ) = 0;
	
  virtual int GenerateFieldSurface( int field_index ) = 0;

  virtual void SetCacheLimits( csVector3 start, csVector3 finish) = 0;
  
  virtual void MapTriangleMesh( csTriangle *tri, csVector3 *verts ) = 0;
  
  virtual void ClearCache() = 0;

  virtual void ZeroCache() = 0;
    
  virtual bool Initialize() = 0;
  
  virtual bool InitializeCache() = 0;
  
  virtual void SetMaxVertices( int limit ) = 0;
  
  virtual int GetMaxVertices() = 0;

  virtual void SetArcSineTableRes( int res ) = 0;
  
  virtual int GetArcSineTableRes() = 0;
  
  virtual void DeleteArcSineTable() = 0;
  
  virtual void InitArcSineTable() = 0;

  virtual void CreateBone( int start, float iso_lev) = 0;
  virtual void AddSlice( bool endcap ) = 0;
  virtual void AddCharge( csVector2 pos, float charge ) = 0;
  virtual void CreateField( float iso_level) = 0;
  virtual void AddPoint( csVector3 pos, float charge ) = 0;

//---------------------------------------------------------------
  virtual void SetQualityEnvironmentMapping(bool tog) = 0;
  
  virtual bool GetQualityEnvironmentMapping() = 0;
  
  virtual float GetEnvironmentMappingFactor() = 0;

  virtual void SetEnvironmentMappingFactor(float f) = 0;
  /// For statistics only
  virtual int ReportTriangleCount () = 0;

  virtual bool IsLighting () = 0;
  
  virtual void SetLighting ( bool set ) = 0;

  /// Data accessors for returning the resultant mesh
  /// from the generating buffer.

  virtual csVector3* GetVertices() = 0;

  virtual int GetVertexCount() = 0;
  
  virtual csVector2* GetTexels() = 0;
  
  virtual int GetTexelCount() = 0;
  
  virtual csTriangle* GetTriangles() = 0;

  virtual int GetTriangleCount() = 0;
  
  virtual void SetSplinterSize(float size) = 0;
  
  virtual float GetSplinterSize() = 0;
};

#endif //  __IMESH_METAGEN_H__
