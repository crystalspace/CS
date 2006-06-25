#ifndef __CS_DECAL_MANAGER_H__
#define __CS_DECAL_MANAGER_H__

#include <iutil/comp.h>
#include <igeom/decal.h>
#include <igeom/polycallback.h>
#include <csutil/scf_implementation.h>
#include <csgeom/vector3.h>
#include <iengine/sector.h>
#include <iengine/engine.h>
#include <csgeom/poly3d.h>
#include <csgeom/tri.h>
#include <ivaria/collider.h>
#include "iutil/array.h"

struct iObjectRegistry;

#define CS_DECAL_CLIP_DECAL
#define CS_DECAL_DEFAULT_NORMAL_THRESHOLD 0.1f
#define CS_DECAL_DEFAULT_OFFSET 0.01f
#define CS_DECAL_DEFAULT_NEAR_FAR_SCALE 1.5f
#define CS_DECAL_DEFAULT_CLIP_NEAR_FAR false

class csDecal : public iDecal
{
private:
  iObjectRegistry * objectReg;
  csRef<iEngine> engine;
  iDecalManager * manager;
  csRef<iMeshFactoryWrapper> meshFact;
  csRef<iMeshWrapper> mesh;
  iMaterialWrapper * material;
  size_t id;
  
public:
  csDecal(iObjectRegistry * objectReg, iDecalManager * manager, size_t id);
  virtual ~csDecal();
  bool Create(iMaterialWrapper * material, iSectorList * sectors, 
      const csReversibleTransform * trans, const csArray<csPoly3D> * polys,
      const csVector3 * decalRelPos, const csVector3 * normal, 
      const csVector3 * up, const csVector3 * right, float width, 
      float height);
};

class csDecalManager : public scfImplementation3<csDecalManager,iDecalManager,
                                                 iComponent,iPolygonCallback>
{
private:
  iObjectRegistry * objectReg;
  csRef<iEngine> engine;
  csRef<iCollideSystem> cdsys;
  csArray<csDecal *> decals;
  csArray<csPoly3D> polygonBuffer;
  float polygonNormalThreshold;
  float decalOffset;
  bool clipNearFar;
  float nearFarScale;

public:
  csDecalManager(iBase * parent);
  virtual ~csDecalManager();

  virtual bool Initialize(iObjectRegistry * objectReg);

  /**
   * Creates a decal.
   * \param material The material to assign to the decal.
   * \param sector The sector to begin searching for nearby meshes.
   * \param pos The position of the decal in world coordinates.
   * \param up The up direction of the decal.
   * \param normal The overall normal of the decal.
   * \param width The width of the decal.
   * \param height The height of the decal.
   * \return True if the decal is created.
   */
  virtual bool CreateDecal(iMaterialWrapper *  material, iSector * sector, 
      const csVector3 * pos, const csVector3 * up, const csVector3 * normal, 
      float width, float height);

  /**
   * Projects a decal using TraceBeam.
   * \param material The material to assign to the decal.
   * \param sector The sector to begin in the TraceBeam.
   * \param start The starting vertex of the line passed to TraceBeam.
   * \param end The end vertex of the line passed to TraceBeam.
   * \param up The up direction of the decal.
   * \param normal The overall normal of the decal, or 0 to compute a normal
   *               based on the first intersecting triangle of the beam.
   * \param width The width of the decal.
   * \param height The height of the decal.
   * \return True if the decal is created.
   */
  virtual bool ProjectDecal(iMaterialWrapper * material, iSector * sector, 
      const csVector3 * start, const csVector3 * end, const csVector3 * up,
      const csVector3 * normal, float width, float height);

  /**
   *  Sets the threshold between polygon normal and decal normal.
   *
   *  If the dot product between the polygon's normal and the normal of the
   *  decal is less than this threshold, then the decal won't show up on that
   *  polygon.
   *
   *  \param threshold The new threshold for new decals.
   */
  virtual void SetPolygonNormalThreshold(float threshold);

  /**
   *  Gets the threshold between polygon normal and decal normal.
   *
   *  If the dot product between the polygon's normal and the normal of the
   *  decal is less than this threshold, then the decal won't show up on that
   *  polygon.
   *
   *  \return The current threshold for new decals.
   */
  virtual float GetPolygonNormalThreshold() const;

  /**
   * Sets the distance to offset the decal along its normal in order to avoid
   * z fighting.
   *
   * \param offset The new polygon offset for new decals.
   */
  virtual void SetDecalOffset(float offset);

  /**
   * Gets the distance to offset the decal along its normal in order to avoid
   * z fighting.
   *
   * \return The current polygon offset for new decals.
   */
  virtual float GetDecalOffset() const;

  /**
   * Turns on and off near/far plane clipping of the decals.
   * \param clip True if near/far clipping should be enabled.
   */
  virtual void SetNearFarClipping(bool clip);

  /**
   * Returns whether near/far plane clipping of decals is enabled.
   * \return True if near/far clipping is enabled.
   */
  virtual bool GetNearFarClipping() const;

  /**
   * Sets the distance between the decal position and the near/far plane based
   * on a multiple of the decal's radius.
   * \param scale The new near/far scale.
   */
  virtual void SetNearFarScale(float scale);

  /**
   * Gets the distance between the decal position and the near/far plane based
   * on a multiple of the decal's radius.
   * \return The current near/far scale.
   */
  virtual float GetNearFarScale() const;

  /**
   * Adds a polygon to the polygon buffer.
   */
  virtual void AddPolygon(const csPoly3D & poly);
};  

#endif // __CS_DECAL_MANAGER_H__
