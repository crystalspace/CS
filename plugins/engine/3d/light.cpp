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
#include "csqsqrt.h"
#include "csgeom/frustum.h"
#include "plugins/engine/3d/light.h"
#include "plugins/engine/3d/sector.h"
#include "plugins/engine/3d/engine.h"
#include "plugins/engine/3d/halo.h"
#include "plugins/engine/3d/meshobj.h"
#include "csutil/debug.h"
#include "csutil/csmd5.h"
#include "csutil/memfile.h"
#include "csutil/csendian.h"

int csLight::ambient_red = CS_DEFAULT_LIGHT_LEVEL;
int csLight::ambient_green = CS_DEFAULT_LIGHT_LEVEL;
int csLight::ambient_blue = CS_DEFAULT_LIGHT_LEVEL;

float csLight::influenceIntensityFraction = 256;
#define HUGE_RADIUS 100000000

SCF_IMPLEMENT_IBASE_EXT(csLight)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE(iLight)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csLight::Light)
  SCF_IMPLEMENTS_INTERFACE(iLight)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csLight::csLight (
  float x, float y, float z,
  float d,
  float red, float green, float blue,
  csLightDynamicType dyntype) :
    csObject()
{
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiLight);
  light_id = 0;
  movable.scfParent = (iBase*)(csObject*)this;
  movable.SetLight (this);
  movable.SetPosition (csVector3 (x,y,z));

  dynamic_type = dyntype;
  type = CS_LIGHT_POINTLIGHT;

  if (dynamic_type != CS_LIGHT_DYNAMICTYPE_DYNAMIC)
    flags.SetAll (CS_LIGHT_THINGSHADOWS);

  SetName ("__light__");

  color.red = red;
  color.green = green;
  color.blue = blue;

  lightnr = 0;

  halo = 0;

  attenuation = CS_ATTN_LINEAR;
  influenceRadius = d;
  influenceRadiusSq = d * d;
  influenceValid = true;
  inv_dist = 1 / d;
  attenuationvec = csVector3(0, 1/d, 0); // inverse linear falloff
  direction = csVector3(1,0,0);
  spotlight_falloff = csVector2(1,1);

  if (ABS (influenceRadius) < SMALL_EPSILON)
    CalculateInfluenceRadius ();
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

  csGlobalHashIterator it (lightinginfos.GetHashMap ());
  while (it.HasNext ())
  {
    iLightingInfo* linfo = (iLightingInfo*)it.Next ();
    linfo->LightDisconnect (&scfiLight);
    linfo->DecRef ();
  }
  lightinginfos.DeleteAll ();

  SCF_DESTRUCT_EMBEDDED_IBASE (scfiLight);
}

void csLight::AddAffectedLightingInfo (iLightingInfo* li)
{
  if (!lightinginfos.In (li))
  {
    lightinginfos.AddNoTest (li);
    li->IncRef ();
  }
}

void csLight::RemoveAffectedLightingInfo (iLightingInfo* li)
{
  lightinginfos.Delete (li);
  li->DecRef ();
}

const char* csLight::GenerateUniqueID ()
{
  if (light_id) return light_id;
  csMemFile mf;

  mf.Write ("light", 5);
  iSector* sector = GetSector ();
  if (sector)
  {
    if (sector->QueryObject ()->GetName ())
      mf.Write (sector->QueryObject ()->GetName (),
      	strlen (sector->QueryObject ()->GetName ()));
  }

  int32 l;
  csVector3 center = GetCenter ();
  l = csConvertEndian ((int32)csQint ((center.x * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)csQint ((center.y * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)csQint ((center.z * 1000)+.5));
  mf.Write ((char*)&l, 4);

  l = csConvertEndian ((int32)csQint ((influenceRadius * 1000)+.5));
  mf.Write ((char*)&l, 4);

  l = csConvertEndian ((int32)csQint ((attenuationvec.x * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)csQint ((attenuationvec.y * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csConvertEndian ((int32)csQint ((attenuationvec.z * 1000)+.5));
  mf.Write ((char*)&l, 4);

  csMD5::Digest digest = csMD5::Encode (mf.GetData (), mf.GetSize ());

  delete[] light_id;
  light_id = new char[sizeof(digest.data)];
  memcpy (light_id, digest.data, sizeof(digest.data));
  return light_id;
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
    case CS_ATTN_CLQ:
	return (attenuationvec * csVector3 (1, d, d*d));
  }

  return 0;
}

void csLight::OnSetPosition ()
{
  csVector3 pos = GetCenter ();
  int i = light_cb_vector.Length ()-1;
  while (i >= 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnPositionChange (&scfiLight, pos);
    i--;
  }

  lightnr++;
}

void csLight::OnSetSector (iSector *sector)
{
  int i = light_cb_vector.Length ()-1;
  while (i >= 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnSectorChange (&scfiLight, sector);
    i--;
  }

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

  csGlobalHashIterator it (lightinginfos.GetHashMap ());
  while (it.HasNext ())
  {
    iLightingInfo* linfo = (iLightingInfo*)it.Next ();
    linfo->LightChanged (&scfiLight);
  }
}

void csLight::SetAttenuation (csLightAttenuationMode a)
{
  float dist;
  if (!GetDistanceForBrightness (1.0f, dist))
    dist = HUGE_RADIUS; // can't determine distance
  if (ABS (dist) > SMALL_EPSILON)
  {
    // Jorrit: @@@ To avoid a crash using old renderer I have to test
    // for 'dist'.
    CalculateAttenuationVector (a, dist, 1.0f);
  }
  attenuation = a;

  int i = light_cb_vector.Length ()-1;
  while (i >= 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnAttenuationChange (&scfiLight, a);
    i--;
  }
}

void csLight::SetAttenuationVector (const csVector3& attenv)
{
  attenuation = CS_ATTN_CLQ;
  attenuationvec.Set (attenv);
  influenceValid = false;

  int i = light_cb_vector.Length ()-1;
  while (i >= 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnAttenuationChange (&scfiLight, attenuation);
    i--;
  }
}

const csVector3 &csLight::GetAttenuationVector()
{
  return attenuationvec;
}

float csLight::GetInfluenceRadius ()
{
  if (!influenceValid)
    CalculateInfluenceRadius ();
  return influenceRadius;
}

float csLight::GetInfluenceRadiusSq ()
{
  if (!influenceValid)
    CalculateInfluenceRadius ();
  return influenceRadiusSq;
}

void csLight::SetInfluenceRadius (float radius)
{
  if (radius <= 0) return;
  int i = light_cb_vector.Length ()-1;
  while (i >= 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnRadiusChange (&scfiLight, radius);
    i--;
  }
  lightnr++;
  influenceRadius = radius;
  influenceRadiusSq = radius*radius;
  inv_dist = 1.0 / influenceRadius;

  csLightAttenuationMode oldatt = attenuation;
  CalculateAttenuationVector (attenuation, radius, 
    1.0f / influenceIntensityFraction);
  attenuation = oldatt;
  influenceValid = true;

}

void csLight::CalculateInfluenceRadius ()
{

  float y = 0.28*color.red + 0.59*color.green + 0.13*color.blue;
  float radius;
  if (!GetDistanceForBrightness (1 / (y * influenceIntensityFraction), radius))
    // can't determine distance
    radius = HUGE_RADIUS;
  influenceRadius = radius;
  influenceRadiusSq = radius*radius;
  inv_dist = 1.0 / influenceRadius;

  influenceValid = true;
}

void csLight::CalculateAttenuationVector (csLightAttenuationMode atttype, float radius,
  float brightness)
{
  if (brightness < EPSILON)
    brightness = EPSILON;
  switch (atttype)
  {
    case CS_ATTN_NONE:
      SetAttenuationVector (csVector3 (1, 0, 0));
      return;
    case CS_ATTN_LINEAR:
    case CS_ATTN_INVERSE:
      SetAttenuationVector (csVector3 (0, 1 / (brightness * radius), 0));
      return;
    case CS_ATTN_REALISTIC:
      SetAttenuationVector (
      	csVector3 (0, 0, 1 / (brightness * radius * radius)));
      return;
    case CS_ATTN_CLQ:
      return;
  }
}

bool csLight::GetDistanceForBrightness (float brightness, float& distance)
{
  switch (attenuation)
  {
    case CS_ATTN_NONE:      
      return false;
    case CS_ATTN_LINEAR:
      distance = (1 - brightness) / inv_dist;
      return true;
    case CS_ATTN_INVERSE:   
      distance = 1 / brightness;
      return true;
    case CS_ATTN_REALISTIC: 
      distance = sqrt (1 / brightness);
      return true;
    case CS_ATTN_CLQ:
    {
      // simple cases
      if (attenuationvec.z == 0)
      {
        if (attenuationvec.y == 0)
        {
          //no solution
          return false;
        }
        else
        {
          float kc = attenuationvec.x;
          float kl = attenuationvec.y;
          distance = (1 / brightness - kc) / kl;
          return true;
        }
      }

      /*
      calculate radius where the light has the intensity of 
      influenceIntensityFraction using the standard light model:    

        brightness = 1/(kc + kl*d + kq*d^2)

      solving equation:
              /-kl +- sqrt( kl^2 - 4*kc*kq - 4*kq/brightness)\
      d = 0.5*|----------------------------------------------|
              \                       kq                     /
      */
      float kc = attenuationvec.x;
      float kl = attenuationvec.y;
      float kq = attenuationvec.z;
      float discr;
      
      discr = kl*kl - 4*kq*(kc - 1/brightness);
      if (discr < 0)
      {
        //no solution
        return false;
      }
      else 
      {
        float radius1, radius2, det;
        det = csQsqrt (discr);
        float denom = 0.5 / kq;
        radius1 = denom * (-kl + det);
        radius2 = denom * (-kl - det);
        distance = MAX (radius1,radius2);
        return true;
      }
    }
  }
  return false;
}

iCrossHalo *csLight::Light::CreateCrossHalo (float intensity, float cross)
{
  csCrossHalo *halo = new csCrossHalo (intensity, cross);
  scfParent->SetHalo (halo);

  csRef<iCrossHalo> ihalo (SCF_QUERY_INTERFACE (halo, iCrossHalo));
  return ihalo; // DecRef is ok here.
}

iNovaHalo *csLight::Light::CreateNovaHalo (
  int seed,
  int num_spokes,
  float roundness)
{
  csNovaHalo *halo = new csNovaHalo (seed, num_spokes, roundness);
  scfParent->SetHalo (halo);

  csRef<iNovaHalo> ihalo (SCF_QUERY_INTERFACE (halo, iNovaHalo));
  return ihalo; // DecRef is ok here.
}

iFlareHalo *csLight::Light::CreateFlareHalo ()
{
  csFlareHalo *halo = new csFlareHalo ();
  scfParent->SetHalo (halo);

  csRef<iFlareHalo> ihalo (SCF_QUERY_INTERFACE (halo, iFlareHalo));
  return ihalo; // DecRef is ok here.
}

//---------------------------------------------------------------------------

static void object_light_func (iMeshWrapper *mesh, iFrustumView *lview,
  bool vis)
{
  // @@@ Note: In case of dynamic light and static lod we probably only want
  // to calculate lighting on the currently visible mesh. Have to think on
  // how to do that exactly.

  if (!vis) return;
  iShadowReceiver* receiver = mesh->GetShadowReceiver ();
  if (receiver)
    receiver->CastShadows (mesh->GetMovable (), lview);

  csMeshWrapper* cmw = ((csMeshWrapper::MeshWrapper*)mesh)->GetCsMeshWrapper ();
  cmw->InvalidateRelevantLights ();
}

void csLight::CalculateLighting ()
{
  csFrustumView lview;
  csFrustumContext *ctxt = lview.GetFrustumContext ();

  csGlobalHashIterator it (lightinginfos.GetHashMap ());
  while (it.HasNext ())
  {
    iLightingInfo* linfo = (iLightingInfo*)it.Next ();
    linfo->LightDisconnect (&scfiLight);
    linfo->DecRef ();
  }
  lightinginfos.DeleteAll ();

  lview.SetObjectFunction (object_light_func);
  lview.SetRadius (GetInfluenceRadius ());
  lview.EnableThingShadows (flags.Get () & CS_LIGHT_THINGSHADOWS);
  lview.SetShadowMask (CS_ENTITY_NOSHADOWS, 0);
  lview.SetProcessMask (CS_ENTITY_NOLIGHTING, 0);

  csRef<csLightingProcessInfo> lpi;
  lpi.AttachNew (new csLightingProcessInfo (
        this, dynamic_type == CS_LIGHT_DYNAMICTYPE_DYNAMIC));
  lview.SetUserdata (lpi);

  ctxt->SetNewLightFrustum (new csFrustum (GetCenter ()));
  ctxt->GetLightFrustum ()->MakeInfinite ();

  if (dynamic_type == CS_LIGHT_DYNAMICTYPE_DYNAMIC)
  {
    csRef<iMeshWrapperIterator> it = csEngine::current_engine
    	->GetNearbyMeshes (GetSector (), GetCenter (), GetInfluenceRadius ());
    while (it->HasNext ())
    {
      iMeshWrapper* m = it->Next ();
      iShadowReceiver* receiver = m->GetShadowReceiver ();
      if (receiver)
      {
        receiver->CastShadows (m->GetMovable (), &lview);
        csMeshWrapper* cmw = ((csMeshWrapper::MeshWrapper*)m)
		->GetCsMeshWrapper ();
        cmw->InvalidateRelevantLights ();
      }
    }
  }
  else
  {
    GetSector ()->CheckFrustum ((iFrustumView *) &lview);
    lpi->FinalizeLighting ();
  }
}

void csLight::CalculateLighting (iMeshWrapper *th)
{
  csFrustumView lview;
  csFrustumContext *ctxt = lview.GetFrustumContext ();
  lview.SetObjectFunction (object_light_func);
  lview.SetRadius (GetInfluenceRadius ());
  lview.EnableThingShadows (flags.Get () & CS_LIGHT_THINGSHADOWS);
  lview.SetShadowMask (CS_ENTITY_NOSHADOWS, 0);
  lview.SetProcessMask (CS_ENTITY_NOLIGHTING, 0);

  csRef<csLightingProcessInfo> lpi;
  lpi.AttachNew (new csLightingProcessInfo (
      this, dynamic_type == CS_LIGHT_DYNAMICTYPE_DYNAMIC));
  lview.SetUserdata (lpi);

  ctxt->SetNewLightFrustum (new csFrustum (GetCenter ()));
  ctxt->GetLightFrustum ()->MakeInfinite ();

  lview.CallObjectFunction (th, true);

  lpi->FinalizeLighting ();
}

//---------------------------------------------------------------------------

// --- csLightList -----------------------------------------------------------
SCF_IMPLEMENT_IBASE(csLightList)
  SCF_IMPLEMENTS_INTERFACE(iLightList)
SCF_IMPLEMENT_IBASE_END

csLightList::csLightList ()
{
  SCF_CONSTRUCT_IBASE (0);
}

csLightList::~csLightList ()
{
  RemoveAll ();
  SCF_DESTRUCT_IBASE ();
}

iLight *csLightList::FindByID (const char* id) const
{
  size_t i;
  for (i = 0; i < list.Length (); i++)
  {
    iLight *l = list.Get (i);
    if (memcmp (l->GetLightID (), id, 16) == 0) return l;
  }

  return 0;
}

int csLightList::Add (iLight *obj)
{
  PrepareLight (obj);
  const char* lightname = obj->QueryObject ()->GetName ();
  if (lightname)
    lights_hash.Put (lightname, obj);
  return list.Push (obj);
}

bool csLightList::Remove (iLight *obj)
{
  FreeLight (obj);
  const char* lightname = obj->QueryObject ()->GetName ();
  if (lightname)
    lights_hash.Delete (lightname, obj);
  return list.Delete (obj);
}

bool csLightList::Remove (int n)
{
  FreeLight (list[n]);
  iLight* obj = list[n];
  const char* lightname = obj->QueryObject ()->GetName ();
  if (lightname)
    lights_hash.Delete (lightname, obj);
  return list.DeleteIndex (n);
}

void csLightList::RemoveAll ()
{
  size_t i;
  for (i = 0 ; i < list.Length () ; i++)
  {
    FreeLight (list[i]);
  }
  lights_hash.DeleteAll ();
  list.DeleteAll ();
}

int csLightList::Find (iLight *obj) const
{
  return list.Find (obj);
}

iLight *csLightList::FindByName (const char *Name) const
{
  return lights_hash.Get (Name, 0);
}

// --- csLightingProcessInfo --------------------------------------------------

SCF_IMPLEMENT_IBASE(csLightingProcessInfo)
  SCF_IMPLEMENTS_INTERFACE(iLightingProcessInfo)
SCF_IMPLEMENT_IBASE_END

csLightingProcessInfo::csLightingProcessInfo (csLight* light, bool dynamic)
{
  SCF_CONSTRUCT_IBASE (0);
  csLightingProcessInfo::light = light;
  csLightingProcessInfo::dynamic = dynamic;
  csLightingProcessInfo::color = light->GetColor ();
}

csLightingProcessInfo::~csLightingProcessInfo()
{
  SCF_DESTRUCT_IBASE ();
}

void csLightingProcessInfo::AttachUserdata (iLightingProcessData* userdata)
{
  userdatas.Push (userdata);
}

csPtr<iLightingProcessData> csLightingProcessInfo::QueryUserdata (
  scfInterfaceID id, int version)
{
  size_t i;
  for (i = 0 ; i < userdatas.Length () ; i++)
  {
    iLightingProcessData* ptr = (iLightingProcessData*)(
      userdatas[i]->QueryInterface (id, version));
    if (ptr)
    {
      return csPtr<iLightingProcessData> (ptr);
    }
  }
  return 0;
}

void csLightingProcessInfo::FinalizeLighting ()
{
  size_t i;
  for (i = 0 ; i < userdatas.Length () ; i++)
  {
    userdatas[i]->FinalizeLighting ();
  }
}

// ---------------------------------------------------------------------------

