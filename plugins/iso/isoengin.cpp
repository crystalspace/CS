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

#include "cssysdef.h"
#include "isoengin.h"
#include "isoworld.h"
#include "isoview.h"
#include "isospr.h"
#include "isomesh.h"
#include "isolight.h"
#include "ivideo/graph2d.h"
#include "ivideo/graph3d.h"
#include "ivideo/txtmgr.h"
#include "isys/evdefs.h"
#include "isys/event.h"
#include "isys/system.h"
#include "isys/vfs.h"
#include "iutil/cfgmgr.h"
#include "csutil/util.h"
#include "iobject/object.h"
#include "igraphic/imageio.h"


IMPLEMENT_IBASE (csIsoEngine)
  IMPLEMENTS_INTERFACE (iIsoEngine)
  IMPLEMENTS_INTERFACE (iPlugIn)
IMPLEMENT_IBASE_END

IMPLEMENT_FACTORY (csIsoEngine)

EXPORT_CLASS_TABLE (iso)
  EXPORT_CLASS_DEP (csIsoEngine, "crystalspace.engine.iso",
    "Crystal Space Isometric Engine",
    "crystalspace.kernel., crystalspace.graphics3d., crystalspace.graphics2d.")
EXPORT_CLASS_TABLE_END

csIsoEngine::csIsoEngine (iBase *iParent)
{
  CONSTRUCT_IBASE (iParent);
  system = NULL;
  g2d = NULL;
  g3d = NULL;
  txtmgr = NULL;

  world = NULL;
}

csIsoEngine::~csIsoEngine ()
{
  for(int i=0; i<materials.Length(); i++)
    RemoveMaterial(i);
  if(g3d) g3d->DecRef();
}

bool csIsoEngine::Initialize (iSystem* p)
{
  system = p;
  // Tell system driver that we want to handle broadcast events
  if (!system->CallOnEvents (this, CSMASK_Broadcast))
    return false;
  return true;
}

bool csIsoEngine::HandleEvent (iEvent& Event)
{
  // Handle some system-driver broadcasts
  if (Event.Type == csevBroadcast)
    switch (Event.Command.Code)
    {
      case cscmdSystemOpen:
      {
        // system is open we can get ptrs now
        g3d = QUERY_PLUGIN_ID (system, CS_FUNCID_VIDEO, iGraphics3D);
        if (!g3d)
        {
          system->Printf(MSG_INTERNAL_ERROR, "IsoEngine: could not get G3D.\n");
          return false;
        }
        g2d = g3d->GetDriver2D ();
        if (!g2d) 
        {
          system->Printf(MSG_INTERNAL_ERROR, "IsoEngine: could not get G2D.\n");
          return false;
        }
        txtmgr = g3d->GetTextureManager();
        if (!txtmgr) 
        {
          system->Printf(MSG_INTERNAL_ERROR, 
            "IsoEngine: could not get TextureManager.\n");
          return false;
        }
        return true;
      }
      case cscmdSystemClose:
      {
        // We must free all material and texture handles since after
        // G3D->Close() they all become invalid, no matter whenever
        // we did or didn't an IncRef on them.
	for(int i=0; i<materials.Length(); i++)
	  RemoveMaterial(i);
        return true;
      }
      case cscmdContextResize:
      {
        return false;
      }
      case cscmdContextClose:
      {
        return false;
      }
    } /* endswitch */

  return false;
}

iIsoWorld* csIsoEngine::CreateWorld()
{
  return new csIsoWorld(this);
}

iIsoView* csIsoEngine::CreateView(iIsoWorld *world)
{
  return new csIsoView(this, this, world);
}

iIsoSprite* csIsoEngine::CreateSprite()
{
  return new csIsoSprite(this);
}

iIsoMeshSprite* csIsoEngine::CreateMeshSprite()
{
  return new csIsoMeshSprite(this);
}

int csIsoEngine::GetBeginDrawFlags () const
{
  return CSDRAW_CLEARZBUFFER;
}

iIsoSprite* csIsoEngine::CreateFloorSprite(const csVector3& pos, float w, 
      float h)
{
  iIsoSprite *spr = new csIsoSprite(this);
  spr->AddVertex(csVector3(0,0,0), 0, 0);
  spr->AddVertex(csVector3(0, 0, w), 1, 0);
  spr->AddVertex(csVector3(h, 0, w), 1, 1);
  spr->AddVertex(csVector3(h, 0, 0), 0, 1);
  spr->SetPosition(pos);
  return spr;
}

iIsoSprite* csIsoEngine::CreateFrontSprite(const csVector3& pos, float w, 
      float h)
{
  iIsoSprite *spr = new csIsoSprite(this);
  float hw = w * 0.25;
  spr->AddVertex(csVector3(-hw, 0,-hw), 0, 0);
  spr->AddVertex(csVector3(-hw, h, -hw), 1, 0);
  spr->AddVertex(csVector3(+hw, h, +hw), 1, 1);
  spr->AddVertex(csVector3(+hw, 0, +hw), 0, 1);
  spr->SetPosition(pos);
  return spr;
}

iIsoSprite* csIsoEngine::CreateZWallSprite(const csVector3& pos, float w,
  float h)
{
  iIsoSprite *spr = new csIsoSprite(this);
  spr->AddVertex(csVector3(0, 0, 0), 0, 0);
  spr->AddVertex(csVector3(0, h, 0), 1, 0);
  spr->AddVertex(csVector3(0, h, w), 1, 1);
  spr->AddVertex(csVector3(0, 0, w), 0, 1);
  spr->SetPosition(pos);
  return spr;
}

iIsoSprite* csIsoEngine::CreateXWallSprite(const csVector3& pos, float w,
  float h)
{
  iIsoSprite *spr = new csIsoSprite(this);
  spr->AddVertex(csVector3(0, 0, 0), 0, 0);
  spr->AddVertex(csVector3(0, h, 0), 1, 0);
  spr->AddVertex(csVector3(w, h, 0), 1, 1);
  spr->AddVertex(csVector3(w, 0, 0), 0, 1);
  spr->SetPosition(pos);
  return spr;
}

iIsoLight* csIsoEngine::CreateLight()
{
  return new csIsoLight(this);
}

iMaterialWrapper *csIsoEngine::CreateMaterialWrapper(iMaterial *material,
      const char *name)
{
  iMaterialWrapper* wrap = QUERY_INTERFACE(materials.NewMaterial(material),
    iMaterialWrapper);
  iObject *object = QUERY_INTERFACE(wrap, iObject);
  object->SetName(name);
  object->DecRef();
  return wrap;
}

iMaterialWrapper *csIsoEngine::CreateMaterialWrapper(iMaterialHandle *handle,
	    const char *name)
{
  iMaterialWrapper* wrap = QUERY_INTERFACE(materials.NewMaterial(handle),
    iMaterialWrapper);
  iObject *object = QUERY_INTERFACE(wrap, iObject);
  object->SetName(name);
  object->DecRef();
  //printf("name %s = %d \n", name, materials.FindByName(name)->GetIndex());
  return wrap;
}

iMaterialWrapper *csIsoEngine::CreateMaterialWrapper(const char *vfsfilename,
	          const char *materialname)
{
  iImageIO *imgloader = QUERY_PLUGIN(system, iImageIO);
  if(imgloader==NULL)
  {
    system->Printf(MSG_INTERNAL_ERROR, "Could not get image loader plugin.\n");
    system->Printf(MSG_INTERNAL_ERROR, "Failed to load file %s.\n", 
      vfsfilename);
    return NULL;
  }
  iVFS *VFS = QUERY_PLUGIN(system, iVFS);
  if(VFS==NULL)
  {
    system->Printf(MSG_INTERNAL_ERROR, "Could not get VFS plugin.\n");
    system->Printf(MSG_INTERNAL_ERROR, "Failed to load file %s.\n", 
      vfsfilename);
    return NULL;
  }

  iDataBuffer *buf = VFS->ReadFile (vfsfilename);
  if(!buf) 
  {
    system->Printf(MSG_INTERNAL_ERROR, "Could not read vfs file %s\n", 
      vfsfilename);
    return NULL;
  }
  iImage *image = imgloader->Load(buf->GetUint8 (), buf->GetSize (),
    txtmgr->GetTextureFormat ());
  if(!image) 
  {
    system->Printf(MSG_INTERNAL_ERROR, 
      "The imageloader could not load image %s\n", vfsfilename);
    return NULL;
  }
  iTextureHandle *handle = txtmgr->RegisterTexture(image, CS_TEXTURE_2D |
    CS_TEXTURE_3D);
  if(!handle) 
  {
    system->Printf(MSG_INTERNAL_ERROR, 
      "Texturemanager could not register texture %s\n", vfsfilename);
    return NULL;
  }
  csIsoMaterial *material = new csIsoMaterial(handle);
  iMaterialHandle *math = txtmgr->RegisterMaterial(material);
  if(!math) 
  {
    system->Printf(MSG_INTERNAL_ERROR, 
      "Texturemanager could not register material %s\n", materialname);
    return NULL;
  }

  buf->DecRef();
  imgloader->DecRef();
  return CreateMaterialWrapper(math, materialname);
}

iMaterialWrapper *csIsoEngine::FindMaterial(const char *name)
{
  return QUERY_INTERFACE(materials.FindByName(name), iMaterialWrapper);
}

iMaterialWrapper *csIsoEngine::FindMaterial(int index)
{
  return QUERY_INTERFACE(materials.Get(index), iMaterialWrapper);
}

void csIsoEngine::RemoveMaterial(const char *name)
{
  csIsoMaterialWrapper *wrap = materials.FindByName(name);
  if(!wrap) return;
  int i;
  for(i=0; i< materials.Length(); i++)
  {
    if(materials.Get(i) == wrap) break;
  }
  materials.RemoveIndex(i);
  delete wrap;
}

void csIsoEngine::RemoveMaterial(int index)
{
  csIsoMaterialWrapper *wrap = materials.Get(index);
  if(!wrap) return;
  materials.RemoveIndex(index);
  delete wrap;
}

int csIsoEngine::GetNumMaterials() const
{
  return materials.Length();
}
