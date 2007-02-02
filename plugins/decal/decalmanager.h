#ifndef __CS_DECAL_MANAGER_H__
#define __CS_DECAL_MANAGER_H__

#include <iutil/comp.h>
#include <igeom/decal.h>
#include <csutil/scf_implementation.h>
#include <csgeom/vector3.h>
#include <iengine/sector.h>
#include <iengine/engine.h>
#include <csgeom/poly3d.h>
#include <csgeom/tri.h>
#include <ivaria/collider.h>
#include <csgfx/renderbuffer.h>
#include <ivideo/rendermesh.h>
#include "iutil/array.h"
#include "iutil/eventh.h"
#include "csutil/eventnames.h"
#include "iutil/virtclk.h"

struct iObjectRegistry;
class csDecal;

class csDecalManager : public scfImplementation3<csDecalManager,
                                                 iDecalManager,
                                                 iComponent,
						 iEventHandler>
{
private:
  iObjectRegistry *     objectReg;
  csRef<iEngine>        engine;
  csArray<csDecal *>    decals;
  csRef<iEventHandler>  weakEventHandler;
  csRef<iVirtualClock>  vc;

public:
  csDecalManager(iBase * parent);
  virtual ~csDecalManager();

  virtual bool Initialize(iObjectRegistry * objectReg);

  /**
   * Creates a decal.
   * \param decalTemplate The template used to create the decal.
   * \param sector The sector to begin searching for nearby meshes.
   * \param pos The position of the decal in world coordinates.
   * \param up The up direction of the decal.
   * \param normal The overall normal of the decal.
   * \param width The width of the decal.
   * \param height The height of the decal.
   * \return True if the decal is created.
   */
  virtual bool CreateDecal(csRef<iDecalTemplate> & decalTemplate, 
      iSector * sector, const csVector3 * pos, const csVector3 * up, 
      const csVector3 * normal, float width, float height);

  /**
   * Creates a decal template and fills it with default values.
   *  \param material The material wrapper for this decal template.
   *  \return The newly created decal template.
   */
  virtual csRef<iDecalTemplate> CreateDecalTemplate(
      iMaterialWrapper* material);

  /**
   * Deletes the given decal.
   *  \param decal The decal to be deleted.
   */
  virtual void DeleteDecal(const iDecal * decal);

  /**
   * Gets the number of decals.
   * \return The number of decals.
   */
  virtual size_t GetDecalCount() const;

  /**
   * Gets the specified decal.
   *  \param idx The index of the decal to get, must be between 0 and
   *    GetDecalCount()-1
   */
  virtual iDecal * GetDecal(size_t idx) const;

  // event handler stuff
  virtual bool HandleEvent(iEvent & ev);
  CS_EVENTHANDLER_NAMES ("crystalspace.decals")
  CS_EVENTHANDLER_NIL_CONSTRAINTS
  CS_DECLARE_EVENT_SHORTCUTS;
};  

#endif // __CS_DECAL_MANAGER_H__
