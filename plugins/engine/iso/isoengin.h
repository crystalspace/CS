/*
    Copyright (C) 2001 by W.C.A. Wijngaards

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

#ifndef __CS_ISOENGIN_H__
#define __CS_ISOENGIN_H__

#include "ivaria/iso.h"
#include "isomater.h"
#include "isomesh.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/csobject.h"
#include "iengine/lightmgr.h"

/**
 * This class implements the isometric engine.
*/
class csIsoEngine : public iIsoEngine, public iLightManager
{
private:
  /// the system
  iObjectRegistry* object_reg;
  /// 2d canvas
  csRef<iGraphics2D> g2d;
  /// 3d renderer
  csRef<iGraphics3D> g3d;
  /// texturemanager
  iTextureManager* txtmgr;
  /// the material list
  csIsoMaterialList materials;
  /// mesh factories list (iMeshFactoryWrapper *)
  csIsoMeshFactoryList meshfactories;

  /// current world
  iIsoWorld *world;

public:
  SCF_DECLARE_IBASE;

  /// Create engine
  csIsoEngine (iBase *iParent);
  /// Destroy engine
  virtual ~csIsoEngine ();

  void Report (int severity, const char* msg, ...);

  //----- iPlugin ------------------------------------------------------
  /// For the system to initialize the plugin, and return success status
  virtual bool Initialize (iObjectRegistry* p);
  /// Intercept events
  virtual bool HandleEvent (iEvent& e);

  struct eiComponent : public iComponent
  {
    SCF_DECLARE_EMBEDDED_IBASE(csIsoEngine);
    virtual bool Initialize (iObjectRegistry* p)
    { return scfParent->Initialize(p); }
  } scfiComponent;
  struct EventHandler : public iEventHandler
  {
  private:
    csIsoEngine* parent;
  public:
    SCF_DECLARE_IBASE;
    EventHandler (csIsoEngine* parent)
    {
      SCF_CONSTRUCT_IBASE (0);
      EventHandler::parent = parent;
    }
    virtual ~EventHandler ()
    {
      SCF_DESTRUCT_IBASE();
    }
    virtual bool HandleEvent (iEvent& e) { return parent->HandleEvent(e); }
  } * scfiEventHandler;

  //----- iLightManager ------------------------------------------------
  virtual const csArray<iLight*>& GetRelevantLights (iBase* logObject,
  	int maxLights, bool desireSorting);

  //----- iIsoEngine ---------------------------------------------------
  virtual iObjectRegistry* GetObjectRegistry() const {return object_reg;}
  virtual iGraphics2D* GetG2D() const {return g2d;}
  virtual iGraphics3D* GetG3D() const {return g3d;}
  virtual iTextureManager* GetTextureManager() const {return txtmgr;}
  virtual iIsoWorld* CreateWorld();
  virtual void SetCurrentWorld(iIsoWorld *world) {csIsoEngine::world=world;}
  virtual iIsoWorld *GetCurrentWorld() const {return world;}
  virtual iIsoView* CreateView(iIsoWorld *world);
  virtual iIsoLight* CreateLight();
  virtual iIsoSprite* CreateSprite();
  virtual iIsoMeshSprite* CreateMeshSprite();
  virtual int GetBeginDrawFlags () const;
  virtual iIsoSprite* CreateFloorSprite(const csVector3& pos, float w,
    float h);
  virtual iIsoSprite* CreateFrontSprite(const csVector3& pos, float w,
    float h);
  virtual iIsoSprite* CreateZWallSprite(const csVector3& pos, float w,
    float h);
  virtual iIsoSprite* CreateXWallSprite(const csVector3& pos, float w,
    float h);
  virtual iMaterialList* GetMaterialList ()
  {
    return &(materials.scfiMaterialList);
  }
  virtual iMaterialWrapper *CreateMaterialWrapper(const char *vfsfilename,
    const char *materialname);
  virtual iMeshFactoryList* GetMeshFactories ()
  {
    return &(meshfactories.scfiMeshFactoryList);
  }
  virtual iMeshFactoryWrapper *CreateMeshFactory(const char* classId,
      const char *name);
  virtual iMeshFactoryWrapper *CreateMeshFactory(const char *name);

};

#endif // __CS_ISOENGIN_H__
