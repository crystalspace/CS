/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein
    Plug-In modifications by Richard D Shank
  
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

#ifndef __CS_DDGTERRAIN_H__
#define __CS_DDGTERRAIN_H__

#include "csgeom/transfrm.h"
#include "csgeom/vector3.h"
#include "csobject/csobject.h"
#include "csutil/cscolor.h"
#include "iterrain/iterrobj.h"
#include "iengine/iterrobj.h"
#include "iterrain/itddg.h"

class ddgContext;
class csDDGTerrainObjectFactory;
class csEngine;
class ddgHeightMap;
class ddgTBinMesh;
class ddgTBinTree;
class ddgVArray;

struct iEngine;
struct iMaterialWrapper;
struct iSystem;

typedef int ddgVBIndex;

// csDDGTerrain terrain class.
class csDDGTerrainObject : public iTerrainObject
{
private:
  iEngine *pEngine;
  iSystem *pSystem;
  const char *pHeightmapName;

  iTerrainObjectFactory *pFactory;

  ///
  ddgTBinMesh* mesh;
  ///
  ddgHeightMap* heightMap;
  ///
  ddgVArray *vbuf;
  ///
  ddgContext *context;

  /// Terrain handle.
  iMaterialWrapper **_materialMap;
  ///
  iMaterialWrapper *pFirstMat;
  ///
  int iMatIndex;
  /// Texture scale factor.
  float _texturescale;

  /// Terrain's location offset in world space.
  csVector3 _pos;
  /// Terrain's size in world space.
  csVector3 _size;

  /**
   * A direction vector for a directional light hitting
   * the terrain.
   */
  csVector3 dirlight;
  /// Color for the directional light.
  csColor dircolor;
  /// If true then a directional light is used.
  bool do_dirlight;

  // Put a triangle into the vertex buffer.
  bool drawTriangle (ddgTBinTree *bt, ddgVBIndex tvc, ddgVArray *vbuf);

  void InitMaterials( void );
  void AddMaterial( char *pName );

public:
  /// Constructor.
  csDDGTerrainObject( iSystem* pSys, iTerrainObjectFactory* factory );

  virtual ~csDDGTerrainObject();

  ///
  ddgContext* GetContext () { return context; }
  ///
  ddgTBinMesh* GetMesh () { return mesh; }
  ///
  ddgHeightMap* GetHeightMap () { return heightMap; }

  ///--------------------- iTerrainObject implementation ---------------------
  DECLARE_IBASE;

  virtual void LoadHeightMap( const char *pName );

  virtual void LoadMaterial( const char *pName );

  virtual void LoadMaterialGroup( const char *pName, int iStart, int iEnd );

  /// Set a directional light.
//  virtual void SetDirLight (const csVector3& dirl, const csColor& dirc);
  virtual void SetDirLight ( csVector3& dirl, csColor& dirc );
  /// Disable directional light.
  virtual void DisableDirLight ();

  /**
   * Draw this terrain given a view and transformation.
   */
  virtual void Draw( iRenderView* rview, bool use_z_buf = true);

  /// Set a material for this surface.
  virtual void SetMaterial (int i, iMaterialWrapper *material)
  {
    _materialMap[i] = material;
  }
  /// Get the number of materials required.
  virtual int GetNumMaterials ();
  /// Set the amount of triangles
  virtual void SetLOD (unsigned int detail);
  /**
   * If current transformation puts us below the terrain at the given
   * x,z location then we have hit the terrain.  We adjust position to
   * be at level of terrain and return 1.  If we are above the terrain we
   * return 0.
   */
  virtual int CollisionDetect (csTransform *p);

  //------------------------- iDDGState implementation ----------------
  class DDGState : public iDDGState
  {
    DECLARE_EMBEDDED_IBASE (csDDGTerrainObject);

    virtual bool LoadHeightMap( const char *pName )
    {
      scfParent->LoadHeightMap( pName );
      return true;
    }
    virtual bool LoadMaterial( const char *pGroup )
    {
      scfParent->LoadMaterial( pGroup );
      return true;
    }
    virtual bool LoadMaterialGroup( const char *pGroup, int iStart, int iEnd )
    {
      scfParent->LoadMaterialGroup( pGroup, iStart, iEnd );
      return true;
    }
    virtual bool SetEngine( iEngine *pEng )
    {
      scfParent->pEngine = pEng;
      return true;
    }
    virtual bool SetLOD( int detailLevel )
    {
      scfParent->SetLOD( detailLevel );
      return true;
    }
  } scfiDDGState;
  friend class DDGState;
};

/**
 * Factory for DDG Terrain. 
 * This factory also implements iDDGFactoryState
 * so that you can set the heightmap of the terrain and the material to use
 * for all instances that are created from this factory.
 */
class csDDGTerrainObjectFactory : public iTerrainObjectFactory
{
private:
  iSystem *pSystem;

public:
  /// Constructor.
  csDDGTerrainObjectFactory( iSystem* pSys );

  /// Destructor.
  virtual ~csDDGTerrainObjectFactory ();

  DECLARE_IBASE;

  virtual iTerrainObject* NewInstance ();

};

/**
 * DDG type. This is the plugin you have to use to create instances
 * of csDDGTerrainObjectFactory.
 */
class csDDGTerrainObjectType : public iTerrainObjectType
{
private:
  iSystem *pSystem;

public:
  /// Constructor.
  csDDGTerrainObjectType (iBase*);

  /// Destructor.
  virtual ~csDDGTerrainObjectType ();

  /// Register plugin with the system driver
  virtual bool Initialize (iSystem *pSys);

  //---------------------- iTerrainObjectType implementation --------------
  DECLARE_IBASE;

  /// Draw.
  virtual iTerrainObjectFactory* NewFactory ();
};

#endif // __CS_DDGTERRAIN_H__
