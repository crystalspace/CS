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
#include "csqint.h"

#include "ivideo/txtmgr.h"
#include "csgeom/frustum.h"
#include "csgeom/trimesh.h"
#include "csgfx/imagememory.h"
#include "csutil/csendian.h"
#include "csutil/csmd5.h"
#include "csutil/memfile.h"

#include "engine.h"
#include "halo.h"
#include "light.h"
#include "meshobj.h"
#include "sector.h"
#include "portal.h"

#define HUGE_RADIUS 100000000

CS_PLUGIN_NAMESPACE_BEGIN(Engine)
{

csLight::csLight (csEngine* engine,
  float x, float y, float z,
  float d,
  float red, float green, float blue,
  csLightDynamicType dyntype) :
    scfImplementationType (this), light_id (0),
    userSpecular (false), halo (0), dynamicType (dyntype), 
    type (CS_LIGHT_POINTLIGHT), cutoffDistance (d),
    directionalCutoffRadius (d), userDirectionalCutoffRadius (false),
    lightnr (0), removingLight (false), engine (engine)
{
  //movable.scfParent = (iBase*)(csObject*)this; //@@MS: Look at this?
  movable.SetLight (this);
  
  // Call setters explicitly since they also set SVs
  csLight::SetCenter (csVector3 (x,y,z));
  csLight::SetColor (csColor (red, green, blue));
  csLight::SetAttenuationMode (CS_ATTN_LINEAR);
  csLight::SetSpotLightFalloff (1, 0);

  SetName ("__light__");


  //if (ABS (cutoffDistance) < SMALL_EPSILON)
  //  CalculateInfluenceRadius ();
  CalculateAttenuationVector ();
  UpdateBBox ();
  
  CS::ShaderVariableContextImpl::AddVariable (
    engine->GetLightAttenuationTextureSV ());
}

csLight::~csLight ()
{
  // Copy the array because we are going to unlink the children.
  csRefArray<iSceneNode> children = movable.GetChildren ();
  size_t j;
  for (j = 0 ; j < children.GetSize () ; j++)
    children[j]->SetParent (0);

  int i = (int)light_cb_vector.GetSize ()-1;
  while (i >= 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnDestroy (this);
    i--;
  }

  if (flags.Check (CS_LIGHT_ACTIVEHALO))
    engine->RemoveHalo (this);
  delete halo;
  delete[] light_id;
}
  
csShaderVariable* csLight::GetPropertySV (
  csLightShaderVarCache::LightProperty prop)
{
  return CS::ShaderVariableContextImpl::GetVariableAdd (
    engine->lightSvNames.GetLightSVId (prop));
}

void csLight::SelfDestruct ()
{
  if (GetSector ())
    GetSector ()->GetLights ()->Remove ((iLight*)this);
}

const char* csLight::GenerateUniqueID ()
{
  if (light_id) return light_id;
  csMemFile mf;
  int32 l;

  mf.Write ("light", 5);

  l = csLittleEndian::Convert ((int32)type);
  mf.Write ((char*)&l, 4);
  l = csLittleEndian::Convert ((int32)dynamicType);
  mf.Write ((char*)&l, 4);

  iSector* sector = GetSector ();
  if (sector)
  {
    if (sector->QueryObject ()->GetName ())
      mf.Write (sector->QueryObject ()->GetName (),
      	strlen (sector->QueryObject ()->GetName ()));
  }

  csVector3 center = GetCenter ();
  l = csLittleEndian::Convert ((int32)csQint ((center.x * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csLittleEndian::Convert ((int32)csQint ((center.y * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csLittleEndian::Convert ((int32)csQint ((center.z * 1000)+.5));
  mf.Write ((char*)&l, 4);

  l = csLittleEndian::Convert ((int32)csQint ((cutoffDistance * 1000)+.5));
  mf.Write ((char*)&l, 4);

  l = csLittleEndian::Convert ((int32)attenuation);
  mf.Write ((char*)&l, 4);
  l = csLittleEndian::Convert ((int32)csQint ((attenuationConstants.x * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csLittleEndian::Convert ((int32)csQint ((attenuationConstants.y * 1000)+.5));
  mf.Write ((char*)&l, 4);
  l = csLittleEndian::Convert ((int32)csQint ((attenuationConstants.z * 1000)+.5));
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

float csLight::GetLuminanceAtSquaredDistance (float sqdist) const
{
  float bright;
  switch (attenuation)
  {
    default:
    case CS_ATTN_NONE:
      bright = 1;
      break;
    case CS_ATTN_LINEAR:
      bright = 1 - sqrt (sqdist) * attenuationConstants.w;
      break;
    case CS_ATTN_INVERSE:
      {
        float d = sqrt (sqdist);
        if (d < SMALL_EPSILON) d = SMALL_EPSILON;
        bright = 1 / d;
      }
      break;
    case CS_ATTN_REALISTIC:
      if (sqdist < SMALL_EPSILON) sqdist = SMALL_EPSILON;
      bright = 1 / sqdist;
      break;
    case CS_ATTN_CLQ:
      bright = (attenuationConstants * csVector3 (1, sqrt (sqdist), sqdist));
      break;
  }
  float lum = color.red * 0.3 + color.green * 0.59 + color.blue * 0.11;
  return bright * lum;
}

float csLight::GetBrightnessAtDistance (float d) const
{
  switch (attenuation)
  {
    case CS_ATTN_NONE:      
      return 1;
    case CS_ATTN_LINEAR:    
      return csClamp (1.0f - d * attenuationConstants.w, 1.0f, 0.0f);
    case CS_ATTN_INVERSE:
      if (d < SMALL_EPSILON) d = SMALL_EPSILON;
      return 1 / d;
    case CS_ATTN_REALISTIC:
      if (d < SMALL_EPSILON) d = SMALL_EPSILON;
      return 1 / (d * d);
    case CS_ATTN_CLQ:
      return (attenuationConstants * csVector3 (1, d, d*d));
  }

  return 0;
}

void csLight::CalculateAttenuationVector ()
{
  switch (attenuation)
  {
    case CS_ATTN_NONE:
      attenuationConstants.Set (1, 0, 0, 0);
      break;
    case CS_ATTN_LINEAR:    
      // @@@ FIXME: cutoff distance != radius, really
      attenuationConstants.Set (1.0f/SMALL_EPSILON, 0, 0, 1.0f/cutoffDistance);
      break;
    case CS_ATTN_INVERSE:
      attenuationConstants.Set (0, 1, 0, 0);
      break;
    case CS_ATTN_REALISTIC:
      attenuationConstants.Set (0, 0, 1, 0);
      break;
    case CS_ATTN_CLQ:
      /* Nothing to do */
    default:
      return;
  }
  GetPropertySV (csLightShaderVarCache::lightAttenuation)->SetValue (
    attenuationConstants);
}

void csLight::SetParent (iSceneNode* parent)
{
    if(!parent && GetParent())
    {
      for (size_t i = 0; i < sectors.GetSize (); ++i)
        sectors[i]->GetLights ()->Remove (this);     
      sectors.Empty();
    }

    csSceneNode::SetParent ((iSceneNode*)this, parent, &movable);
}

void csLight::OnSetPosition ()
{
  // Update the AABB
  {
    const csBox3 oldBox = worldBoundingBox;
    UpdateBBox ();
    
    iSectorList* list = movable.csMovable::GetSectors ();
    if (list)
    {
      for (int i = 0; i < list->GetCount (); ++i)
      {
        csSector* sect = static_cast<csSector*> (list->Get (i));
        if(GetParent() && (size_t) sectors.Find(sect) == csArrayItemNotFound)
        {
            sectors.Push (sect);
            sect->GetLights ()->Add (this);
        }

        sect->UpdateLightBounds (this, oldBox);
      }

      if(!removingLight && GetParent())
      {
        for(size_t i = 0; i < sectors.GetSize (); ++i)
        {
          if((size_t) list->Find(sectors[i]) == csArrayItemNotFound)
          {
            removingLight = true;
            sectors[i]->GetLights ()->Remove (this);
            sectors.DeleteIndex (i--);
            removingLight = false;
          }
        }
      }
    }
  }

  csVector3 pos = GetFullCenter ();
  size_t i = light_cb_vector.GetSize ();
  while (i-- > 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnPositionChange (this, pos);
  }

  GetPropertySV (csLightShaderVarCache::lightPositionWorld)->SetValue (pos);
  const csReversibleTransform& lightT = 
    movable.csMovable::GetFullTransform ();
  GetPropertySV (csLightShaderVarCache::lightTransformWorld)->SetValue (
    lightT);
  GetPropertySV (csLightShaderVarCache::lightTransformWorldInverse)->SetValue (
    lightT.GetInverse());
      
  const csVector3 lightDirW = 
    lightT.This2OtherRelative (csVector3 (0, 0, 1));
  if (!lightDirW.IsZero())
  {
    GetPropertySV (csLightShaderVarCache::lightDirectionWorld)->SetValue (
      lightDirW.Unit());
  }
  else
  {
    GetPropertySV (csLightShaderVarCache::lightDirectionWorld)->SetValue (
      csVector3 (0));
  }
  lightnr++;
}

void csLight::OnSetSector (iSector *sector)
{
  size_t i = light_cb_vector.GetSize ();
  while (i-- > 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnSectorChange (this, sector);
  }

  lightnr++;
}

void csLight::SetColor (const csColor& col) 
{
  size_t i = light_cb_vector.GetSize ();
  while (i-- > 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnColorChange (this, col);
  }

  color = col; 
  GetPropertySV (csLightShaderVarCache::lightDiffuse)->SetValue (col);
  if (!userSpecular)
  {
    specularColor = col;
    GetPropertySV (csLightShaderVarCache::lightSpecular)->SetValue (col);
  }
  lightnr++;
}


void csLight::SetAttenuationMode (csLightAttenuationMode a)
{
  //@@TODO : Implement!
  
  /*float dist;
  if (!GetDistanceForBrightness (1.0f, dist))
    dist = HUGE_RADIUS; // can't determine distance
  if (ABS (dist) > SMALL_EPSILON)
  {
    // Jorrit: @@@ To avoid a crash using old renderer I have to test
    // for 'dist'.
    CalculateAttenuationVector (a, dist, 1.0f);
  }*/

  attenuation = a;
  CalculateAttenuationVector();
  GetPropertySV (csLightShaderVarCache::lightAttenuationMode)->SetValue (a);

  size_t i = light_cb_vector.GetSize ();
  while (i-- > 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnAttenuationChange (this, a);
  }
}

void csLight::SetAttenuationConstants (const csVector4& attenv)
{
  //@@TODO : Implement!
  /*attenuation = CS_ATTN_CLQ;
  attenuationvec.Set (attenv);
  influenceValid = false;*/
  attenuationConstants = attenv;
  GetPropertySV (csLightShaderVarCache::lightAttenuation)->SetValue (
    attenuationConstants);

  size_t i = light_cb_vector.GetSize ();
  while (i-- > 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnAttenuationChange (this, attenuation);
  }
}

void csLight::SetCutoffDistance (float radius)
{
  if (radius <= 0) return;
  
  size_t i = light_cb_vector.GetSize ();
  while (i-- > 0)
  {
    iLightCallback* cb = light_cb_vector[i];
    cb->OnRadiusChange (this, radius);
  }
  lightnr++;

  cutoffDistance = radius;
  if (!userDirectionalCutoffRadius)
    directionalCutoffRadius = radius;

  // Update the AABB
  {
    const csBox3 oldBox = worldBoundingBox;
    UpdateBBox ();
    
    iSectorList* list = movable.csMovable::GetSectors ();
    if (list)
    {
      for (int i = 0; i < list->GetCount (); ++i)
      {
        csSector* sect = static_cast<csSector*> (list->Get (i));
        sect->UpdateLightBounds (this, oldBox);
      }      
    }
  }

  CalculateAttenuationVector();
}

iCrossHalo *csLight::CreateCrossHalo (float intensity, float cross)
{
  csCrossHalo *halo = new csCrossHalo (intensity, cross);
  SetHalo (halo);

  csRef<iCrossHalo> ihalo (scfQueryInterface<iCrossHalo> (halo));
  return ihalo; // DecRef is ok here.
}

iNovaHalo *csLight::CreateNovaHalo (
  int seed,
  int num_spokes,
  float roundness)
{
  csNovaHalo *halo = new csNovaHalo (seed, num_spokes, roundness);
  SetHalo (halo);

  csRef<iNovaHalo> ihalo (scfQueryInterface<iNovaHalo> (halo));
  return ihalo; // DecRef is ok here.
}

iFlareHalo *csLight::CreateFlareHalo ()
{
  csFlareHalo *halo = new csFlareHalo ();
  SetHalo (halo);

  csRef<iFlareHalo> ihalo (scfQueryInterface<iFlareHalo> (halo));
  return ihalo; // DecRef is ok here.
}

//---------------------------------------------------------------------------

iSector* csLight::GetFullSector ()
{
  iSector* s = GetSector ();
  if (s) return s;
  iSceneNode* node = (iSceneNode*)this;
  iSceneNode* parent = node->GetParent ();
  while (parent)
  {
    iSectorList* sl = parent->GetMovable ()->GetSectors ();
    if (sl && sl->GetCount () > 0)
    {
      return sl->Get (0);
    }

    parent = parent->GetParent ();
  }
  return 0;
}

csBox3 csLight::GetBBox () const
{
  return worldBoundingBox;
}

void csLight::UpdateBBox ()
{
  switch (type)
  {
  case CS_LIGHT_DIRECTIONAL:
      //@@TODO: Implement
  case CS_LIGHT_SPOTLIGHT:
    // @@@ This could be tighter if the falloff is taken into account.
    lightBoundingBox.Set (
      csVector3 (-cutoffDistance, -cutoffDistance, 0),
      csVector3 (cutoffDistance, cutoffDistance, cutoffDistance));
    break;
  case CS_LIGHT_POINTLIGHT:
    {
      lightBoundingBox.SetSize (csVector3 (cutoffDistance*2));
      lightBoundingBox.SetCenter (csVector3 (0));
      break;
    }
  }

  worldBoundingBox = movable.GetFullTransform ().This2Other (lightBoundingBox);
}

//---------------------------------------------------------------------------

// --- csLightList -----------------------------------------------------------
csLightList::csLightList ()
  : scfImplementationType (this)
{
  listener.AttachNew (new NameChangeListener (this));
}

csLightList::~csLightList ()
{
  RemoveAll ();
}

void csLightList::NameChanged (iObject* object, const char* oldname,
  	const char* newname)
{
  csRef<iLight> light = scfQueryInterface<iLight> (object);
  CS_ASSERT (light != 0);
  if (oldname) lights_hash.Delete (oldname, light);
  if (newname) lights_hash.Put (newname, light);
}

iLight *csLightList::FindByID (const char* id) const
{
  size_t i;
  for (i = 0; i < list.GetSize (); i++)
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
  obj->QueryObject ()->AddNameChangeListener (listener);
  return (int)list.Push (obj);
}

bool csLightList::Remove (iLight *obj)
{
  FreeLight (obj);
  const char* lightname = obj->QueryObject ()->GetName ();
  if (lightname)
    lights_hash.Delete (lightname, obj);
  obj->QueryObject ()->RemoveNameChangeListener (listener);
  return list.Delete (obj);
}

bool csLightList::Remove (int n)
{
  FreeLight (list[n]);
  iLight* obj = list[n];
  const char* lightname = obj->QueryObject ()->GetName ();
  if (lightname)
    lights_hash.Delete (lightname, obj);
  obj->QueryObject ()->RemoveNameChangeListener (listener);
  return list.DeleteIndex (n);
}

void csLightList::RemoveAll ()
{
  size_t i;
  for (i = 0 ; i < list.GetSize () ; i++)
  {
    list[i]->QueryObject ()->RemoveNameChangeListener (listener);
    FreeLight (list[i]);
  }
  lights_hash.DeleteAll ();
  list.DeleteAll ();
}

int csLightList::Find (iLight *obj) const
{
  return (int)list.Find (obj);
}

iLight *csLightList::FindByName (const char *Name) const
{
  return lights_hash.Get (Name, 0);
}

// ---------------------------------------------------------------------------
  
void LightAttenuationTextureAccessor::CreateAttenuationTexture ()
{
#define CS_ATTTABLE_SIZE	  128
#define CS_HALF_ATTTABLE_SIZE	  ((float)CS_ATTTABLE_SIZE/2.0f)
  csRGBpixel *attenuationdata = 
    new csRGBpixel[CS_ATTTABLE_SIZE * CS_ATTTABLE_SIZE];
  csRGBpixel* data = attenuationdata;
  for (int y=0; y < CS_ATTTABLE_SIZE; y++)
  {
    for (int x=0; x < CS_ATTTABLE_SIZE; x++)
    {
      float yv = 3.0f * ((y + 0.5f)/CS_HALF_ATTTABLE_SIZE - 1.0f);
      float xv = 3.0f * ((x + 0.5f)/CS_HALF_ATTTABLE_SIZE - 1.0f);
      float i = exp (-0.7 * (xv*xv + yv*yv));
      unsigned char v = i>1.0f ? 255 : csQint (i*255.99f);
      (data++)->Set (v, v, v, v);
    }
  }

  csRef<iImage> img = csPtr<iImage> (new csImageMemory (
    CS_ATTTABLE_SIZE, CS_ATTTABLE_SIZE, attenuationdata, true, 
    CS_IMGFMT_TRUECOLOR | CS_IMGFMT_ALPHA));
   attTex = engine->G3D->GetTextureManager()->RegisterTexture (
      img, CS_TEXTURE_3D | CS_TEXTURE_CLAMP | CS_TEXTURE_NOMIPMAPS);
  attTex->SetTextureClass ("lookup");
}

LightAttenuationTextureAccessor::LightAttenuationTextureAccessor (
  csEngine* engine) : scfImplementationType (this), engine (engine)
{
}

LightAttenuationTextureAccessor::~LightAttenuationTextureAccessor ()
{
}

void LightAttenuationTextureAccessor::PreGetValue (csShaderVariable *variable)
{
  if (!attTex.IsValid()) CreateAttenuationTexture ();
  variable->SetValue (attTex);
}

}
CS_PLUGIN_NAMESPACE_END(Engine)
