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
#include "csutil/csmd5.h"
#include "csutil/memfile.h"
#include "cssys/csendian.h"

int csLight:: ambient_red = CS_DEFAULT_LIGHT_LEVEL;
int csLight:: ambient_green = CS_DEFAULT_LIGHT_LEVEL;
int csLight:: ambient_blue = CS_DEFAULT_LIGHT_LEVEL;

#ifdef CS_USE_NEW_RENDERER
  float csLight::influenceIntensityFraction = 256;
#endif

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
  light_id = NULL;
  center.x = x;
  center.y = y;
  center.z = z;

  SetName ("__light__");

  color.red = red;
  color.green = green;
  color.blue = blue;

  lightnr = 0;

  halo = NULL;

#ifndef CS_USE_NEW_RENDERER
  dist = d;
  sqdist = d * d;
  inv_dist = 1 / d;

  attenuation = CS_ATTN_LINEAR;
#else
  //attenuationvec = csVector3(1,0,0); //default lightattenuation is kc = 1, kl=0,kq=0
  attenuationvec = csVector3(0, 1/d, 0); // inverse linear falloff
  CalculateInfluenceRadius ();
#endif
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
  delete[] light_id;
}

const char* csLight::GenerateUniqueID ()
{
  if (light_id) return light_id;
  csMemFile mf;

  mf.Write ("light", 5);
  if (sector)
  {
    if (sector->GetName ())
      mf.Write (sector->GetName (), strlen (sector->GetName ()));
  }

  int32 l;
  l = convert_endian ((int32)QInt ((center.x * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)QInt ((center.y * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)QInt ((center.z * 1000)+.5));
  mf.Write ((char*)&l, 4);
#ifndef CS_USE_NEW_RENDERER
  l = convert_endian ((int32)QInt ((dist * 1000)+.5));
  mf.Write ((char*)&l, 4);
#else
  l = convert_endian ((int32)QInt ((attenuationvec.x * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)QInt ((attenuationvec.y * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = convert_endian ((int32)QInt ((attenuationvec.z * 1000)+.5));
  mf.Write ((char*)&l, 4);
#endif

  csMD5::Digest digest = csMD5::Encode (mf.GetData (), mf.GetSize ());

  delete[] light_id;
  light_id = new char[16];
  memcpy (light_id, digest.data, 16);
  return light_id;
}

void csLight::SetHalo (csHalo *Halo)
{
  delete halo;
  halo = Halo;
}

float csLight::GetBrightnessAtDistance (float d)
{
#ifndef CS_USE_NEW_RENDERER
  switch (attenuation)
  {
    case CS_ATTN_NONE:      return 1;
    case CS_ATTN_LINEAR:    return 1 - d * inv_dist;
    case CS_ATTN_INVERSE:   return 1 / d;
    case CS_ATTN_REALISTIC: return 1 / (d * d);
  }

  return 0;
#else
  return (attenuationvec * csVector3 (1, d, d*d));
#endif
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

#ifndef CS_USE_NEW_RENDERER
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
#endif

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

#ifndef CS_USE_NEW_RENDERER
void csLight::SetAttenuation (int a)
{
  attenuation = a;
}

#else

void csLight::SetAttenuationVector(csVector3 &pattenv)
{
  attenuationvec = pattenv;
  
  if (attenuationvec.x < 0)
    attenuationvec.x = 0;
  if (attenuationvec.y < 0)
    attenuationvec.y = 0;
  if (attenuationvec.z < 0)
    attenuationvec.z = 0;

  CalculateInfluenceRadius ();
}

csVector3 &csLight::GetAttenuationVector()
{
  return attenuationvec;
}

float csLight::GetInfluenceRadius ()
{
  return influenceRadius;
}

float csLight::GetInfluenceRadiusSq ()
{
  return influenceRadiusSq;
}

void csLight::SetInfluenceRadius (float radius)
{
  if (radius > 0)
  {
    influenceRadius = radius;
    influenceRadiusSq = radius*radius;
  }
}

void csLight::CalculateInfluenceRadius ()
{
  // simple cases
  if (attenuationvec.z == 0)
  {
    if (attenuationvec.y == 0)
    {
      //no solution - brightness constant, infinite radius
      SetInfluenceRadius (100000000); 
      return;
    }
    else
    {
      float kc = attenuationvec.x;
      float kl = attenuationvec.y;
      float y = 0.28*color.red + 0.59*color.green + 0.13*color.blue;
      SetInfluenceRadius ((influenceIntensityFraction*y - kc) / kl);
    }
  }

  /*
   calculate radius where the light has the intensity of 
   influenceIntensityFraction using the standard light model:    

     brightness = y*1/(kc + kl*d + kq*d^2)

   solving equation:
           /-kl +- sqrt( kl^2 - 4*kc*kq + 4*kq*y*brightness)\
   d = 0.5*|------------------------------------------------|
           \                       kq                       /
  */
  float kc = attenuationvec.x;
  float kl = attenuationvec.y;
  float kq = attenuationvec.z;
  float discr;
  float y = 0.28*color.red + 0.59*color.green + 0.13*color.blue;
  
  discr = kl*kl - 4*kq*(kc + influenceIntensityFraction*y);
  if (discr < 0)
  {
    //no solution - brightness constant, infinite radius
    SetInfluenceRadius (100000000); 
  }
  else 
  {
    float radius1, radius2, det;
    det = sqrtf (discr);
    float denom = 0.5 / kq;
    radius1 = denom * (-kl + det);
    radius2 = denom * (-kl - det);
    SetInfluenceRadius (MAX (radius1,radius2));
  }
}

#endif

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
  csHashIterator it (lightinginfos.GetHashMap ());
  while (it.HasNext ())
  {
    iLightingInfo* linfo = (iLightingInfo*)it.Next ();
    linfo->DecRef ();
  }
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
#ifdef CS_USE_NEW_RENDERER
  lview.SetRadius (GetInfluenceRadius ());
#else
  lview.SetRadius (GetRadius ());
#endif
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
#ifdef CS_USE_NEW_RENDERER
  lview.SetRadius (GetInfluenceRadius ());
#else
  lview.SetRadius (GetRadius ());
#endif
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
  li->IncRef ();
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
    linfo->DecRef ();
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
    linfo->DecRef ();
  }
  lightinginfos.DeleteAll ();

  csFrustumView lview;
  csFrustumContext *ctxt = lview.GetFrustumContext ();
  lview.SetObjectFunction (object_light_func);
#ifdef CS_USE_NEW_RENDERER
  lview.SetRadius (GetInfluenceRadius ());
#else
  lview.SetRadius (GetRadius ());
#endif
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
  li->IncRef ();
}

void csDynLight::RemoveAffectedLightingInfo (iLightingInfo* li)
{
  lightinginfos.Delete (li);
  li->DecRef ();
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
  SCF_IMPLEMENTS_INTERFACE(iLightList)
SCF_IMPLEMENT_IBASE_END

csLightList::csLightList ()
{
  SCF_CONSTRUCT_IBASE (NULL);
}

iLight *csLightList::FindByID (const char* id) const
{
  int i;
  for (i = 0; i < list.Length (); i++)
  {
    iLight *l = list.Get (i);
    if (memcmp (l->GetLightID (), id, 16) == 0) return l;
  }

  return NULL;
}

int csLightList::Add (iLight *obj)
{
  PrepareItem (obj);
  return list.Push (obj);
}

bool csLightList::Remove (iLight *obj)
{
  FreeItem (obj);
  return list.Delete (obj);
}

bool csLightList::Remove (int n)
{
  FreeItem (list[n]);
  return list.Delete (n);
}

void csLightList::RemoveAll ()
{
  int i;
  for (i = 0 ; i < list.Length () ; i++)
  {
    FreeItem (list[i]);
  }
  list.DeleteAll ();
}

int csLightList::Find (iLight *obj) const
{
  return list.Find (obj);
}

iLight *csLightList::FindByName (const char *Name) const
{
  return list.FindByName (Name);
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

