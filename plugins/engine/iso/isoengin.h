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

#ifndef __ISOENGIN_H__
#define __ISOENGIN_H__

#include "ivaria/iso.h"
#include "isomater.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "csutil/csvector.h"
#include "csutil/csobject.h"

/// generic wrapper around iBase that is a csObject 
class csIsoObjWrapper : public csObject
{
public:
  iBase *content;
public:
  csIsoObjWrapper() {content = NULL;}
  csIsoObjWrapper(iBase *p) {content = p;}
  csIsoObjWrapper(iBase *p, const char* name) {content = p; SetName(name);}
  /// you must decref content on your own
  virtual ~csIsoObjWrapper() {}
  /// get content
  iBase* GetContent() const {return content;}
  /// set content
  void SetContent(iBase *p) {content = p;}
};

/// a basicvector - with csIsoObjWrapper contents that supports names.
class csIsoNamedVector : public csBasicVector
{
public:
  /// create
  csIsoNamedVector(int ilimit = 0, int ithreshold = 0) 
    : csBasicVector(ilimit, ithreshold) {}
  /// delete, but and the wrapperobjects in it (but no decref of content)
  ~csIsoNamedVector() {int i; for(i=0; i<Length(); i++) 
    delete (csIsoObjWrapper*)Get(i); }

  /// find a wrapper by name
  csIsoObjWrapper *FindByName(const char* name) const
  {
	int i;
    for(i=0; i<Length(); i++)
    {
      csIsoObjWrapper *o = (csIsoObjWrapper*)Get(i);
      if(o->GetName() && (strcmp(o->GetName(), name) == 0))
	return o;
    }
    return NULL;
  }
  /// find iBase by name
  iBase *FindContentByName(const char* name) const
  {
	int i;
    for(i=0; i<Length(); i++)
    {
      csIsoObjWrapper *o = (csIsoObjWrapper*)Get(i);
      if(o->GetName() && (strcmp(o->GetName(), name) == 0))
	return o->GetContent();
    }
    return NULL;
  }
  /// find index by name, -1 means not found
  int FindIndexByName(const char* name) const
  {
	int i;
    for(i=0; i<Length(); i++)
    {
      csIsoObjWrapper *o = (csIsoObjWrapper*)Get(i);
      if(o->GetName() && (strcmp(o->GetName(), name) == 0))
	return i;
    }
    return -1;
  }
};


/**
 * This class implements the isometric engine.
*/
class csIsoEngine : public iIsoEngine
{
private:
  /// the system
  iObjectRegistry* object_reg;
  /// 2d canvas
  iGraphics2D* g2d;
  /// 3d renderer
  iGraphics3D* g3d;
  /// texturemanager
  iTextureManager* txtmgr;
  /// the material list
  csIsoMaterialList materials;
  /// mesh factories list (iMeshObjectFactory *)
  csIsoNamedVector meshfactories;

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
  struct eiEventHandler : public iEventHandler
  {
    SCF_DECLARE_EMBEDDED_IBASE(csIsoEngine);
    virtual bool HandleEvent (iEvent& e) { return scfParent->HandleEvent(e); }
  } scfiEventHandler;

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
  virtual iMaterialWrapper *CreateMaterialWrapper(iMaterial *material,
    const char *name);
  virtual iMaterialWrapper *CreateMaterialWrapper(iMaterialHandle *handle,
    const char *name);
  virtual iMaterialWrapper *CreateMaterialWrapper(const char *vfsfilename,
    const char *materialname);
  virtual iMaterialWrapper *FindMaterial(const char *name);
  virtual iMaterialWrapper *FindMaterial(int index);
  virtual void RemoveMaterial(const char *name);
  virtual void RemoveMaterial(int index);
  virtual int GetMaterialCount() const ;
  virtual iMeshObjectFactory *CreateMeshFactory(const char* classId,
      const char *name);
  virtual void AddMeshFactory(iMeshObjectFactory *fact, const char *name);
  virtual iMeshObjectFactory *FindMeshFactory(const char *name);
  virtual void RemoveMeshFactory(const char *name);
};

#endif
