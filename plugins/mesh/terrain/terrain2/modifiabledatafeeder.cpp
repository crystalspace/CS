/*
  Copyright (C) 2010 by Jelle Hellemans

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

#include "csgeom/csrect.h"
#include "csgfx/imagemanipulate.h"
#include "csutil/dirtyaccessarray.h"
#include "csutil/refarr.h"
#include "csutil/stringconv.h"

#include "iengine/material.h"
#include "igraphic/image.h"
#include "imap/loader.h"
#include "imesh/terrain2.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "modifiabledatafeeder.h"
#include "feederhelper.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{
SCF_IMPLEMENT_FACTORY (csTerrainModifiableDataFeeder)

csTerrainModifiableDataFeeder::csTerrainModifiableDataFeeder (iBase* parent)
  : scfImplementationType (this, parent)
{
}

csTerrainModifiableDataFeeder::~csTerrainModifiableDataFeeder ()
{
}

csPtr<iTerrainCellFeederProperties> csTerrainModifiableDataFeeder::CreateProperties ()
{
  return csPtr<iTerrainCellFeederProperties> (
      new csTerrainModifiableDataFeederProperties);
}

bool csTerrainModifiableDataFeeder::PreLoad (iTerrainCell* cell)
{
  return false;
}

bool csTerrainModifiableDataFeeder::Load (iTerrainCell* cell)
{
  cells.PushSmart(cell);

  csTerrainModifiableDataFeederProperties* properties = 
    (csTerrainModifiableDataFeederProperties*)cell->GetFeederProperties ();

  if (!loader || !properties)
    return false;

  if (properties->heightmapSource.IsEmpty ())
    return false;

  int width = cell->GetGridWidth ();
  int height = cell->GetGridHeight ();

  csLockedHeightData data = cell->LockHeightData (csRect(0, 0, width, height));
  
  HeightFeederParser mapReader (properties->heightmapSource, 
    properties->heightmapFormat, loader, objectReg);
  mapReader.Load (data.data, width, height, data.pitch, cell->GetSize ().y, 
    properties->heightOffset);

  if (properties->smoothHeightmap)
  {
    SmoothHeightmap (data.data, width, height, data.pitch);    
  }

  cell->UnlockHeightData ();
  
  if (!properties->materialmapSource.IsEmpty ())
  {  
    csRef<iImage> materialMap = loader->LoadImage (properties->materialmapSource.GetDataSafe (),
      CS_IMGFMT_PALETTED8);

    if (!materialMap) 
      return false;
    
    if (materialMap->GetWidth () != cell->GetMaterialMapWidth () ||
        materialMap->GetHeight () != cell->GetMaterialMapHeight ())
    {
      materialMap = csImageManipulate::Rescale (materialMap,
        cell->GetMaterialMapWidth (), cell->GetMaterialMapHeight ());
    }
    
    int mwidth = materialMap->GetWidth ();
    int mheight = materialMap->GetHeight ();

    csLockedMaterialMap mdata = cell->LockMaterialMap (csRect (0, 0, mwidth, mheight));

    const unsigned char* materialmap = (const unsigned char*)materialMap->GetImageData ();
      
    for (int y = 0; y < mheight; ++y)
    {
      memcpy (mdata.data, materialmap, mwidth);
      mdata.data += mdata.pitch;
      materialmap += mwidth;
    } 
    
    cell->UnlockMaterialMap ();
  }

  if (engine)
  {
    for (size_t i = 0; i < properties->alphaMaps.GetSize (); ++i)
    {
      iMaterialWrapper* mat = engine->FindMaterial (
        properties->alphaMaps[i].material.GetDataSafe ());

      csRef<iImage> alphaMap = loader->LoadImage (
        properties->alphaMaps[i].alphaSource.GetDataSafe (),
        CS_IMGFMT_ANY);

      if (mat && alphaMap)
      {
        cell->SetAlphaMask (mat, alphaMap);
      }
    }
  }

  return true;
}

iTerrainModifier* csTerrainModifiableDataFeeder::AddModifier (const csVector3& center, float width, float height)
{
  csRef<iTerrainModifier> mod;
  mod.AttachNew(new csRectFlattenModifier(center, width, height));

  modifiers.Push(mod);

  cells.Compact();

  for (size_t i = 0; i < cells.GetSize(); i++)
  {
    int width = cells[i]->GetGridWidth();
    int height = cells[i]->GetGridHeight();
    csVector2 v = cells[i]->GetPosition();
    if (mod->InBounds(csBox2(v.x, v.y, v.x+width, v.y+height)))
    {
      mod->Displace(cells[i], 1.0f);
    }
  }

  return mod;
}

void csTerrainModifiableDataFeeder::RemoveModifier (iTerrainModifier* modifier)
{
  if (modifiers.Delete(modifier))
  {
    csRefArray<iTerrainCell> affectedCells;
    for (size_t i = 0; i < cells.GetSize(); i++)
    {
      int width = cells[i]->GetGridWidth();
      int height = cells[i]->GetGridHeight();
      csVector2 v = cells[i]->GetPosition();
      if (modifier->InBounds(csBox2(v.x, v.y, v.x+width, v.y+height)))
      {
        affectedCells.Push(cells[i]);
      }
    }

    // Reload affected cells
    for (size_t i = 0; i < affectedCells.GetSize(); i++)
    {
      iTerrainCell* cell = affectedCells[i];

      csTerrainModifiableDataFeederProperties* properties = 
        (csTerrainModifiableDataFeederProperties*)cell->GetFeederProperties ();

      if (!loader || !properties)
        continue;

      if (properties->heightmapSource.IsEmpty ())
        continue;

      int width = cell->GetGridWidth ();
      int height = cell->GetGridHeight ();

      csLockedHeightData data = cell->LockHeightData (csRect(0, 0, width, height));

      HeightFeederParser mapReader (properties->heightmapSource, 
        properties->heightmapFormat, loader, objectReg);
      mapReader.Load (data.data, width, height, data.pitch, cell->GetSize ().y, 
        properties->heightOffset);

      if (properties->smoothHeightmap)
      {
        SmoothHeightmap (data.data, width, height, data.pitch);    
      }

      cell->UnlockHeightData ();
    }

    //Reapply modifiers
    for (size_t j = 0; j < modifiers.GetSize(); j++)
      for (size_t i = 0; i < affectedCells.GetSize(); i++)
      {
        int width = affectedCells[i]->GetGridWidth();
        int height = affectedCells[i]->GetGridHeight();
        csVector2 v = affectedCells[i]->GetPosition();
        if (modifiers[j]->InBounds(csBox2(v.x, v.y, v.x+width, v.y+height)))
        {
          modifiers[j]->Displace(affectedCells[i], 1.0f);
        }
      }

  }  
}

bool csTerrainModifiableDataFeeder::Initialize (iObjectRegistry* object_reg)
{
  this->objectReg = object_reg;

  loader = csQueryRegistry<iLoader> (objectReg);
  engine = csQueryRegistry<iEngine> (objectReg);

  return true;
}

void csTerrainModifiableDataFeeder::SetParameter (const char* param, const char* value)
{
}

void csRectFlattenModifier::Displace(iTerrainCell* cell, float intensity) const
{
  int width = cell->GetGridWidth();
  int height = cell->GetGridHeight();
  csVector2 v = cell->GetPosition();

  csRect area(0, 0, width, height);
  area.Intersect(bb.MinX()-v.x, bb.MinY()-v.y, bb.MaxX()-v.x, bb.MaxY()-v.y);

  //printf("area: %d, %d - %d, %d\n\n", area.xmin, area.ymin%256, area.xmax, area.ymax);

  csLockedHeightData data = cell->LockHeightData (area);

  float val = (center.y/cell->GetSize().z)*intensity;

  for (size_t y = 0; y < area.ymax-area.ymin; y++)
    for (size_t x = 0; x < area.xmax-area.xmin; x++)
    {
      data.data[y * data.pitch + x] = val;
    }

  cell->UnlockHeightData ();
}


csTerrainModifiableDataFeederProperties::csTerrainModifiableDataFeederProperties ()
  : scfImplementationType (this), heightOffset (0.0f), smoothHeightmap (false)
{
}

csTerrainModifiableDataFeederProperties::csTerrainModifiableDataFeederProperties (
  csTerrainModifiableDataFeederProperties& other)
  : scfImplementationType (this),
    heightmapSource (other.heightmapSource),
    heightmapFormat (other.heightmapFormat),
    materialmapSource (other.materialmapSource),
    alphaMaps (other.alphaMaps),
    heightOffset (other.heightOffset),
    smoothHeightmap (other.smoothHeightmap)
{
}


void csTerrainModifiableDataFeederProperties::SetHeightmapSource (const char* source,
                                                              const char* format)
{
  heightmapSource = source;
  heightmapFormat = format;
}

void csTerrainModifiableDataFeederProperties::SetMaterialMapSource (const char* source)
{
  materialmapSource = source;
}

void csTerrainModifiableDataFeederProperties::SetHeightOffset (float offset)
{
  heightOffset = offset;
}

void csTerrainModifiableDataFeederProperties::AddAlphaMap (const char* material, 
                                                       const char* alphaMapSource)
{
  for (size_t i = 0; i < alphaMaps.GetSize (); ++i)
  {
    if (alphaMaps[i].material == material)
    {
      alphaMaps[i].alphaSource = alphaMapSource;
      return;
    }
  }

  AlphaPair p = {material, alphaMapSource};
  alphaMaps.Push (p);
}

void csTerrainModifiableDataFeederProperties::SetParameter (const char* param, const char* value)
{
  if (strcasecmp (param, "heightmap source") == 0)
  {
    heightmapSource = value;
  }
  else if (strcasecmp (param, "heightmap format") == 0)
  {
    heightmapFormat = value;
  }
  else if (strcasecmp (param, "materialmap source") == 0)
  {
    materialmapSource = value;
  }
  else if (strcasecmp (param, "offset") == 0)
  {
    heightOffset = CS::Utility::strtof (value);
  }
  else if (strcasecmp (param, "smooth heightmap") == 0)
  {
    if (strcasecmp (value, "yes") == 0 ||
        strcasecmp (value, "true") == 0)
    {
      smoothHeightmap = true;
    }
    if (strcasecmp (value, "no") == 0 ||
        strcasecmp (value, "false") == 0)
    {
      smoothHeightmap = false;
    }
  }
}

size_t csTerrainModifiableDataFeederProperties::GetParameterCount() 
{ 
  return 5; 
}

const char* csTerrainModifiableDataFeederProperties::GetParameterName (size_t index)
{
  switch (index)
  {
    case 0: return "heightmap source";
    case 1: return "heightmap format";
    case 2: return "materialmap source";
    case 3: return "offset";
    case 4: return "smooth heightmap";
    default: return 0;
  }
}

const char* csTerrainModifiableDataFeederProperties::GetParameterValue (size_t index)
{ return GetParameterValue (GetParameterName (index)); }

const char* csTerrainModifiableDataFeederProperties::GetParameterValue (const char* name)
{
  // @@@ Not nice
  static char scratch[32];

  if (strcasecmp (name, "heightmap source") == 0)
  {
    return heightmapSource;
  }
  else if (strcasecmp (name, "heightmap format") == 0)
  {
    return heightmapFormat;
  }
  else if (strcasecmp (name, "materialmap source") == 0)
  {
    return materialmapSource;
  }
  else if (strcasecmp (name, "offset") == 0)
  {
    snprintf (scratch, sizeof (scratch), "%f", heightOffset);
    return scratch;
  }
  else if (strcasecmp (name, "smooth heightmap") == 0)
  {
    return smoothHeightmap ? "yes" : "no";
  }
  return 0;
}

csPtr<iTerrainCellFeederProperties> csTerrainModifiableDataFeederProperties::Clone ()
{
  return csPtr<iTerrainCellFeederProperties> (new csTerrainModifiableDataFeederProperties (*this));
}
  
const char* csTerrainModifiableDataFeederProperties::GetAlphaMapSource (
  const char* material)
{
  if (material == 0) return 0;
  for (size_t i = 0; i < alphaMaps.GetSize (); ++i)
  {
    if (alphaMaps[i].material == material)
    {
      return alphaMaps[i].alphaSource;
    }
  }
  return 0;
}

void csTerrainModifiableDataFeederProperties::SetHeightmapSmooth (bool doSmooth)
{
  smoothHeightmap = doSmooth;
}

bool csTerrainModifiableDataFeederProperties::GetHeightmapSmooth () const
{
  return smoothHeightmap;
}

}
CS_PLUGIN_NAMESPACE_END(Terrain2)
