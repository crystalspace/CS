/*
    Copyright (C) 1998-2001 by Jorrit Tyberghein

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
#include "csgeom/frustum.h"
#include "csengine/light.h"
#include "csengine/sector.h"
#include "csengine/engine.h"
#include "csengine/halo.h"
#include "csengine/meshobj.h"
#include "csutil/debug.h"

int csLight:: ambient_red = CS_DEFAULT_LIGHT_LEVEL;
int csLight:: ambient_green = CS_DEFAULT_LIGHT_LEVEL;
int csLight:: ambient_blue = CS_DEFAULT_LIGHT_LEVEL;
unsigned long csLight:: last_light_id = 0;

SCF_IMPLEMENT_IBASE_EXT(csLight)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iLight)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csLight::Light)
  SCF_IMPLEMENTS_INTERFACE(iLight)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csLight::csLight (
  float x,
  float y,
  float z,
  float d,
  float red,
  float green,
  float blue) :
    csObject()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLight);
  DG_TYPE (this, "csLight");
  light_id = last_light_id++;
  center.x = x;
  center.y = y;
  center.z = z;

  SetName ("__light__");

  dist = d;
  sqdist = d * d;
  inv_dist = 1 / d;
  halo = NULL;

  color.red = red;
  color.green = green;
  color.blue = blue;

  attenuation = CS_ATTN_LINEAR;
  lightnr = 0;
}

csLight::~csLight ()
{
  int i = light_cb_vector.Length ()-1;
  while (i >= 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnDestroy (&scfiLight);
    i--;
  }

  if (flags.Check (CS_LIGHT_ACTIVEHALO))
    csEngine::current_engine->RemoveHalo (this);
  delete halo;
}

void csLight::SetHalo (csHalo *Halo)
{
  delete halo;
  halo = Halo;
}

float csLight::GetBrightnessAtDistance (float d)
{
  switch (attenuation)
  {
    case CS_ATTN_NONE:      return 1;
    case CS_ATTN_LINEAR:    return 1 - d * inv_dist;
    case CS_ATTN_INVERSE:   return 1 / d;
    case CS_ATTN_REALISTIC: return 1 / (d * d);
  }

  return 0;
}

void csLight::SetCenter (const csVector3 &pos)
{
  int i = light_cb_vector.Length ()-1;
  while (i >= 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnPositionChange (&scfiLight, pos);
    i--;
  }

  center = pos;
  lightnr++;
}

iSector *csLight::Light::GetSector ()
{
  return &scfParent->GetSector ()->scfiSector;
}

void csLight::SetSector (csSector* sector)
{
  int i = light_cb_vector.Length ()-1;
  while (i >= 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnSectorChange (&scfiLight, sector ? &(sector->scfiSector) : NULL);
    i--;
  }

  csLight::sector = sector;
  lightnr++;
}

void csLight::Light::SetSector (iSector *sector)
{
  scfParent->SetSector (sector ? sector->GetPrivateObject () : NULL);
}

void csLight::SetRadius (float radius)
{
  int i = light_cb_vector.Length ()-1;
  while (i >= 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnRadiusChange (&scfiLight, radius);
    i--;
  }
  dist = radius;
  sqdist = dist*dist;
  inv_dist = 1 / dist;
  lightnr++;
}

void csLight::SetColor (const csColor& col) 
{
  int i = light_cb_vector.Length ()-1;
  while (i >= 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnColorChange (&scfiLight, col);
    i--;
  }

  color = col; 
  lightnr++;
}

iCrossHalo *csLight::Light::CreateCrossHalo (float intensity, float cross)
{
  csCrossHalo *halo = new csCrossHalo (intensity, cross);
  scfParent->SetHalo (halo);

  csRef<iCrossHalo> ihalo (SCF_QUERY_INTERFACE (halo, iCrossHalo));
  return ihalo;	// DecRef is ok here.
}

iNovaHalo *csLight::Light::CreateNovaHalo (
  int seed,
  int num_spokes,
  float roundness)
{
  csNovaHalo *halo = new csNovaHalo (seed, num_spokes, roundness);
  scfParent->SetHalo (halo);

  csRef<iNovaHalo> ihalo (SCF_QUERY_INTERFACE (halo, iNovaHalo));
  return ihalo;	// DecRef is ok here.
}

iFlareHalo *csLight::Light::CreateFlareHalo ()
{
  csFlareHalo *halo = new csFlareHalo ();
  scfParent->SetHalo (halo);

  csRef<iFlareHalo> ihalo (SCF_QUERY_INTERFACE (halo, iFlareHalo));
  return ihalo;	// DecRef is ok here.
}

//---------------------------------------------------------------------------
SCF_IMPLEMENT_IBASE_EXT(csStatLight)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iStatLight)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csStatLight::eiStaticLight)
  SCF_IMPLEMENTS_INTERFACE(iStatLight)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csStatLight::csStatLight (
  float x,
  float y,
  float z,
  float dist,
  float red,
  float green,
  float blue,
  bool dynamic) :
    csLight(x, y, z, dist, red, green, blue)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiStatLight);
  DG_TYPE (this, "csStatLight");
  csStatLight::dynamic = dynamic;
  flags.SetAll (CS_LIGHT_THINGSHADOWS);
}

csStatLight::~csStatLight ()
{
  lightinginfos.DeleteAll ();
}

static void object_light_func (iMeshWrapper *mesh, iFrustumView *lview,
	bool vis)
{
  if (!vis) return;
  iShadowReceiver* receiver = mesh->GetShadowReceiver ();
  if (receiver)
    receiver->CastShadows (mesh->GetMovable (), lview);
}

void csStatLight::CalculateLighting ()
{
  csFrustumView lview;
  csFrustumContext *ctxt = lview.GetFrustumContext ();
  lview.SetObjectFunction (object_light_func);
  lview.SetRadius (GetRadius ());
  lview.EnableThingShadows (flags.Get () & CS_LIGHT_THINGSHADOWS);
  lview.SetShadowMask (CS_ENTITY_NOSHADOWS, 0);
  lview.SetProcessMask (CS_ENTITY_NOLIGHTING, 0);

  csLightingProcessInfo *lpi = new csLightingProcessInfo (
      this, false);
  lview.SetUserdata (lpi);

  ctxt->SetLightFrustum (new csFrustum (center));
  ctxt->GetLightFrustum ()->MakeInfinite ();
  sector->CheckFrustum ((iFrustumView *) &lview);

  lpi->FinalizeLighting ();
  lpi->DecRef ();
}

void csStatLight::CalculateLighting (iMeshWrapper *th)
{
  csFrustumView lview;
  csFrustumContext *ctxt = lview.GetFrustumContext ();
  lview.SetObjectFunction (object_light_func);
  lview.SetRadius (GetRadius ());
  lview.EnableThingShadows (flags.Get () & CS_LIGHT_THINGSHADOWS);
  lview.SetShadowMask (CS_ENTITY_NOSHADOWS, 0);
  lview.SetProcessMask (CS_ENTITY_NOLIGHTING, 0);

  csLightingProcessInfo *lpi = new csLightingProcessInfo (
      this, false);
  lview.SetUserdata (lpi);

  ctxt->SetLightFrustum (new csFrustum (center));
  ctxt->GetLightFrustum ()->MakeInfinite ();

  lview.CallObjectFunction (th, true);

  lpi->FinalizeLighting ();
  lpi->DecRef ();
}

void csStatLight::AddAffectedLightingInfo (iLightingInfo* li)
{
  if (!dynamic) return ;
  lightinginfos.Add (li);
}

void csStatLight::SetColor (const csColor &col)
{
  csLight::SetColor (col);
  csHashIterator it (lightinginfos.GetHashMap ());
  while (it.HasNext ())
  {
    iLightingInfo* linfo = (iLightingInfo*)it.Next ();
    linfo->StaticLightChanged (&scfiStatLight);
  }
}

//---------------------------------------------------------------------------

SCF_IMPLEMENT_IBASE_EXT(csDynLight)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iDynLight)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csDynLight::eiDynLight)
  SCF_IMPLEMENTS_INTERFACE(iDynLight)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csDynLight::csDynLight (
  float x,
  float y,
  float z,
  float dist,
  float red,
  float green,
  float blue) :
    csLight(x, y, z, dist, red, green, blue)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiDynLight);
  DG_TYPE (this, "csDynLight");
}

csDynLight::~csDynLight ()
{
  //csEngine::current_engine->RemoveDynLight (this);

  csHashIterator it (lightinginfos.GetHashMap ());
  while (it.HasNext ())
  {
    iLightingInfo* linfo = (iLightingInfo*)it.Next ();
    linfo->DynamicLightDisconnect (&scfiDynLight);
  }
  lightinginfos.DeleteAll ();
}

void csDynLight::Setup ()
{
  csHashIterator it (lightinginfos.GetHashMap ());
  while (it.HasNext ())
  {
    iLightingInfo* linfo = (iLightingInfo*)it.Next ();
    linfo->DynamicLightDisconnect (&scfiDynLight);
  }
  lightinginfos.DeleteAll ();

  csFrustumView lview;
  csFrustumContext *ctxt = lview.GetFrustumContext ();
  lview.SetObjectFunction (object_light_func);
  lview.SetRadius (GetRadius ());
  lview.EnableThingShadows (flags.Get () & CS_LIGHT_THINGSHADOWS);
  lview.SetShadowMask (CS_ENTITY_NOSHADOWS, 0);
  lview.SetProcessMask (CS_ENTITY_NOLIGHTING, 0);

  csLightingProcessInfo *lpi = new csLightingProcessInfo (
      this, true);
  lview.SetUserdata (lpi);

  ctxt->SetLightFrustum (new csFrustum (center));
  ctxt->GetLightFrustum ()->MakeInfinite ();
  sector->CheckFrustum ((iFrustumView *) &lview);

  lpi->FinalizeLighting ();
  lpi->DecRef ();
}

void csDynLight::AddAffectedLightingInfo (iLightingInfo* li)
{
  lightinginfos.Add (li);
}

void csDynLight::RemoveAffectedLightingInfo (iLightingInfo* li)
{
  lightinginfos.Delete (li);
}

void csDynLight::SetColor (const csColor &col)
{
  csLight::SetColor (col);

  csHashIterator it (lightinginfos.GetHashMap ());
  while (it.HasNext ())
  {
    iLightingInfo* linfo = (iLightingInfo*)it.Next ();
    linfo->DynamicLightChanged (&scfiDynLight);
  }
}

// --- csLightList -----------------------------------------------------------
SCF_IMPLEMENT_IBASE(csLightList)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iLightList)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csLightList::LightList)
  SCF_IMPLEMENTS_INTERFACE(iLightList)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csLightList::csLightList ()
{
  SCF_CONSTRUCT_IBASE (NULL);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLightList);
}

iLight *csLightList::FindByID (unsigned long id) const
{
  int i;
  for (i = 0; i < Length (); i++)
  {
    iLight *l = Get (i);
    if (l->GetLightID () == id) return l;
  }

  return NULL;
}

int csLightList::LightList::GetCount () const
{
  return scfParent->Length ();
}

iLight *csLightList::LightList::Get (int n) const
{
  return scfParent->Get (n);
}

int csLightList::LightList::Add (iLight *obj)
{
  return scfParent->Push (obj);
}

bool csLightList::LightList::Remove (iLight *obj)
{
  return scfParent->Delete (obj);
}

bool csLightList::LightList::Remove (int n)
{
  return scfParent->Delete (n);
}

void csLightList::LightList::RemoveAll ()
{
  scfParent->DeleteAll ();
}

int csLightList::LightList::Find (iLight *obj) const
{
  return scfParent->Find (obj);
}

iLight *csLightList::LightList::FindByName (const char *Name) const
{
  return scfParent->FindByName (Name);
}

iLight *csLightList::LightList::FindByID (unsigned long id) const
{
  return scfParent->FindByID (id);
}

// --- csLightingProcessInfo --------------------------------------------------

SCF_IMPLEMENT_IBASE(csLightingProcessInfo)
  SCF_IMPLEMENTS_INTERFACE(iLightingProcessInfo)
SCF_IMPLEMENT_IBASE_END

csLightingProcessInfo::csLightingProcessInfo (csLight* light, bool dynamic)
{
  SCF_CONSTRUCT_IBASE (NULL);
  csLightingProcessInfo::light = light;
  csLightingProcessInfo::dynamic = dynamic;
  csLightingProcessInfo::color = light->GetColor ();
}

void csLightingProcessInfo::AttachUserdata (iLightingProcessData* userdata)
{
  userdatas.Push (userdata);
}

csPtr<iLightingProcessData> csLightingProcessInfo::QueryUserdata (
	scfInterfaceID id, int version)
{
  int i;
  for (i = 0 ; i < userdatas.Length () ; i++)
  {
    iLightingProcessData* ptr = (iLightingProcessData*)(
    	userdatas[i]->QueryInterface (id, version));
    if (ptr)
    {
      return csPtr<iLightingProcessData> (ptr);
    }
  }
  return NULL;
}

void csLightingProcessInfo::FinalizeLighting ()
{
  int i;
  for (i = 0 ; i < userdatas.Length () ; i++)
  {
    userdatas[i]->FinalizeLighting ();
  }
}

// ---------------------------------------------------------------------------

