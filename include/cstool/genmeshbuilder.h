/*
    Copyright (C) 2007 by Jorrit Tyberghein

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

#ifndef __CS_GENMESHPRIM_H__
#define __CS_GENMESHPRIM_H__

/**\file
 * Primitive Mesh Generator for GenMesh.
 */

#include "csextern.h"

#include "csgeom/sphere.h"
#include "cstool/primitives.h"
#include "iengine/mesh.h"
#include "imesh/object.h"
#include "imesh/genmesh.h"

struct iEngine;
struct iSector;

namespace CS
{
namespace Geometry
{

/**
 * Superclass for all primitives.
 */
struct Primitive
{
public:
  virtual ~Primitive () { }

  /// Append this primitive to the given factory.
  virtual void Append (iGeneralFactoryState* factory) = 0;

  /**
   * Append this primitive to the given factory.
   * Returns false if the primitive is not a genmesh.
   */
  virtual bool Append (iMeshFactoryWrapper* factory)
  {
    csRef<iGeneralFactoryState> state = scfQueryInterface<
      iGeneralFactoryState> (factory->GetMeshObjectFactory ());
    if (!state) return false;
    Append (state);
    return true;
  }
};

/**
 * A tesselated quad.
 */
class CS_CRYSTALSPACE_EXPORT TesselatedQuad : public Primitive
{
private:
  csVector3 v0, v1, v2;
  int tesselations;
  TextureMapper* mapper;

public:
  /**
   * Generate a single-sided tesselations quad. v0-v1 and v0-v2 should
   * be oriented clockwise from the visible side.
   * \param v0 is the origin of the quad.
   * \param v1 is the first axis.
   * \param v2 is the second axis.
   */
  TesselatedQuad (const csVector3& v0, const csVector3& v1, const csVector3& v2);
  virtual ~TesselatedQuad () { }

  /// Set the tesselation level for this quad. Default is 1.
  void SetLevel (int level) { tesselations = level; }

  /// Get the tesselation level.
  int GetLevel () const { return tesselations; }

  /**
   * Set the mapper. Default mapper is Primitives::DensityTextureMapper
   * with density 1.
   */
  void SetMapper (TextureMapper* mapper)
  {
    TesselatedQuad::mapper = mapper;
  }

  virtual void Append (iGeneralFactoryState* state);
  virtual bool Append (iMeshFactoryWrapper* factory)
  {
    return Primitive::Append (factory);
  }
};

/**
 * A Tesselated box.
 */
class CS_CRYSTALSPACE_EXPORT TesselatedBox : public Primitive
{
private:
  csBox3 box;
  int tesselations;
  TextureMapper* mapper;
  uint32 flags;

  void Init (const csBox3& box);

public:
  /**
   * Generate a tesselated box so the normals of every face point inwards
   * or outwards (the normals of the vertices belonging to a face will point
   * with the correct normal of the face).
   */
  TesselatedBox (const csBox3& box)
  {
    Init (box);
  }
  /**
   * Generate a tesselated box so the normals of every face point inwards
   * or outwards (the normals of the vertices belonging to a face will point
   * with the correct normal of the face).
   */
  TesselatedBox (const csVector3& v1, const csVector3& v2)
  {
    Init (csBox3 (v1, v2));
  }
  virtual ~TesselatedBox () { }

  /// Set the tesselation level for this box. Default is 1.
  void SetLevel (int level) { tesselations = level; }

  /// Get the tesselation level.
  int GetLevel () const { return tesselations; }

  /**
   * Set the mapper. Default mapper is Primitives::DensityTextureMapper
   * with density 1.
   */
  void SetMapper (TextureMapper* mapper)
  {
    TesselatedBox::mapper = mapper;
  }

  /**
   * Set the flags. These are a combination of csPrimitives::BoxFlags
   * enumeration values. CS_PRIMBOX_SMOOTH is not supported here. Default
   * is 0.
   */
  void SetFlags (uint32 flags)
  {
    TesselatedBox::flags = flags;
  }

  /// Get the flags.
  uint32 GetFlags () const { return flags; }

  virtual void Append (iGeneralFactoryState* state);
  virtual bool Append (iMeshFactoryWrapper* factory)
  {
    return Primitive::Append (factory);
  }
};

/**
 * A box.
 */
class CS_CRYSTALSPACE_EXPORT Box : public Primitive
{
private:
  csBox3 box;
  TextureMapper* mapper;
  uint32 flags;

  void Init (const csBox3& box);

public:
  Box (const csBox3& box)
  {
    Init (box);
  }
  Box (const csVector3& v1, const csVector3& v2)
  {
    Init (csBox3 (v1, v2));
  }
  virtual ~Box () { }

  /**
   * Set the mapper. Default mapper is Primitives::DensityTextureMapper
   * with density 1.
   */
  void SetMapper (TextureMapper* mapper)
  {
    Box::mapper = mapper;
  }

  /**
   * Set the flags. These are a combination of csPrimitives::BoxFlags
   * enumeration values. Default is CS_PRIMBOX_SMOOTH.
   */
  void SetFlags (uint32 flags)
  {
    Box::flags = flags;
  }

  /// Get the flags.
  uint32 GetFlags () const { return flags; }

  virtual void Append (iGeneralFactoryState* state);
  virtual bool Append (iMeshFactoryWrapper* factory)
  {
    return Primitive::Append (factory);
  }
};

/**
 * A capsule.
 */
class CS_CRYSTALSPACE_EXPORT Capsule : public Primitive
{
private:
  float l, r;
  uint sides;
  TextureMapper* mapper;

public:
  /**
   * Generate a capsule of given length and radius.
   * \param l Capsule length.
   * \param r Capsule radius.
   * \param sides Number of sides.
   */
  Capsule (float l, float r, uint sides);
  virtual ~Capsule () { }

  /**
   * Set the mapper. There is no default mapper. You have to specify one.
   */
  void SetMapper (TextureMapper* mapper)
  {
    Capsule::mapper = mapper;
  }

  virtual void Append (iGeneralFactoryState* state);
  virtual bool Append (iMeshFactoryWrapper* factory)
  {
    return Primitive::Append (factory);
  }
};

/**
 * A sphere.
 */
class CS_CRYSTALSPACE_EXPORT Sphere : public Primitive
{
private:
  csEllipsoid ellips;
  int num;
  TextureMapper* mapper;
  bool cyl_mapping;
  bool toponly;
  bool reversed;

public:
  /**
   * Generate a sphere with 'num' vertices on the rim.
   * \param ellips Properties of the ellipsoid to create.
   * \param num Number of vertices in the generated  mesh.
   */
  Sphere (const csEllipsoid& ellips, int num);
  virtual ~Sphere () { }

  /**
   * Set cylindrical mapping to true. This is only useful if no
   * mapper was used. Default is false.
   */
  void SetCylindricalMapping (bool cyl) { cyl_mapping = cyl; }
  /// Get cylindrical mapping setting.
  bool HasCylindricalMapping () const { return cyl_mapping; }

  /**
   * If this flag is set then only the top half of the sphere is generated.
   * Default is false.
   */
  void SetTopOnly (bool top) { toponly = top; }
  /// Get toponly settings.
  bool IsTopOnly () const { return toponly; }

  /**
   * If this flag is set then generate the sphere so that it is visible
   * from the inside. Default false.
   */
  void SetReversed (bool rev) { reversed  = rev; }
  /// Return reversed setting.
  bool IsReversed () const { return reversed; }

  /**
   * Set the mapper. If not given the mapping as defined by
   * SetCylindricalMapping() will be used.
   */
  void SetMapper (TextureMapper* mapper)
  {
    Sphere::mapper = mapper;
  }

  virtual void Append (iGeneralFactoryState* state);
  virtual bool Append (iMeshFactoryWrapper* factory)
  {
    return Primitive::Append (factory);
  }
};

/**
 * A Cone.
 */
class CS_CRYSTALSPACE_EXPORT Cone : public Primitive
{
private:
  float l, r;
  uint sides;
  TextureMapper* mapper;

public:
  /**
   * Generate a cone of given length and radius.
   * \param l Capsule length.
   * \param r Capsule radius.
   * \param sides Number of sides.
   */
  Cone (float h, float r, uint sides);
  virtual ~Cone () { }

  /**
   * Set the mapper. There is no default mapper. You have to specify one.
   */
  void SetMapper (TextureMapper* mapper)
  {
    Cone::mapper = mapper;
  }

  virtual void Append (iGeneralFactoryState* state);
  virtual bool Append (iMeshFactoryWrapper* factory)
  {
    return Primitive::Append (factory);
  }
};

/**
 * Tools related to creating genmesh instances and factories.
 */
class CS_CRYSTALSPACE_EXPORT GeneralMeshBuilder
{
public:
  /**
   * Create a genmesh factory with an optional primitive. Assign to a csRef.
   * \param name the engine name of the factory that will be created
   * \param primitive is an optional primitive that can be used to augment
   * the factory.
   */
  static csPtr<iMeshFactoryWrapper> CreateFactory (iEngine* engine, 
    const char* name, Primitive* primitive = 0);

  /**
   * Create a genmesh instance from a factory.
   * This mesh will have #CS_ZBUF_USE set (use Z-buffer fully) and have
   * 'object' as render priority. This means this function is useful
   * for general objects. Assign to a csRef. The object will be placed
   * at position 0,0,0 in the sector.
   * \param sector the sector to add the object to
   * \param name the engine name of the mesh that will be created
   * \param factoryname the engine name of the factory to use.
   */
  static csPtr<iMeshWrapper> CreateMesh (iEngine* engine, iSector* sector,
    const char* name, iMeshFactoryWrapper* factory);

  /**
   * Create a genmesh instance from a named factory.
   * This mesh will have #CS_ZBUF_USE set (use Z-buffer fully) and have
   * 'object' as render priority. This means this function is useful
   * for general objects. Assign to a csRef. The object will be placed
   * at position 0,0,0 in the sector.
   * \param sector the sector to add the object to
   * \param name the engine name of the mesh that will be created
   * \param factoryname the engine name of the factory to use.
   */
  static csPtr<iMeshWrapper> CreateMesh (iEngine* engine, iSector* sector,
    const char* name, const char* factoryname);

  /**
   * Create a factory and a genmesh from a primitive. This is a common
   * used method where you only need one mesh from a factory. So with this
   * method you can create both at once.
   * This mesh will have #CS_ZBUF_USE set (use Z-buffer fully) and have
   * 'object' as render priority. This means this function is useful
   * for general objects. Assign to a csRef. The object will be placed
   * at position 0,0,0 in the sector.
   * \param sector the sector to add the object to
   * \param name the engine name of the mesh that will be created
   * \param factoryname the engine name of the factory to create.
   * \param primitive is an optional primitive that can be used to augment
   * the factory.
   */
  static csPtr<iMeshWrapper> CreateFactoryAndMesh (iEngine* engine,
      iSector* sector, const char* name, const char* factoryname,
      Primitive* primitive = 0);
};
} // namespace Geometry
} // namespace CS

/** @} */

#endif // __CS_GENMESHPRIM_H__

