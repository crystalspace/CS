/*
  Copyright (C) 2006 by Kapoulkine Arseny
                2007-2008 by Marten Svanfeldt

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
#include "csgeom/vector3.h"
#include "csgfx/imagebase.h"
#include "csgfx/imagemanipulate.h"
#include "csgfx/imagememory.h"

#include "iengine/material.h"
#include "imesh/terrain2.h"

#include "cell.h"
#include "terrainsystem.h"

CS_PLUGIN_NAMESPACE_BEGIN(Terrain2)
{

csTerrainCell::csTerrainCell (csTerrainSystem* terrain, const char* name, int gridWidth,
  int gridHeight, int materialMapWidth, int materialMapHeight,
  bool materialMapPersistent,
  const csVector2& position, const csVector3& size,
  iTerrainCellRenderProperties* renderProperties,
  iTerrainCellCollisionProperties* collisionProperties,
  iTerrainCellFeederProperties* feederProperties)
  : scfImplementationType (this),
  terrain (terrain), name (name), materialMapWidth (materialMapWidth), 
  materialMapHeight (materialMapHeight), position (position), size (size),
  minHeight (-FLT_MAX*0.9f), maxHeight (FLT_MAX*0.9f),
  renderProperties (renderProperties), collisionProperties (collisionProperties),
  feederProperties (feederProperties),
  needTangentsUpdate (true),
  loadState (NotLoaded),
  lruTicks (0)
{
  // Here we do grid width/height correction. The height map will be a
  // square with size 2^n + 1

  int maxsize = csMax (gridWidth, gridHeight) - 1;
  int temp = 1;
  while (temp < maxsize) 
    temp *= 2;  

  this->gridWidth = temp + 1;
  this->gridHeight = temp + 1;
  
  step_x = size.x / (this->gridWidth - 1);
  step_z = size.z / (this->gridHeight - 1);

  const csVector3 size01 = size * 0.1f;
  boundingBox.Set (position.x - size01.x, minHeight - size01.y, position.y - size01.z,
    position.x + size.x + size01.x, maxHeight + size01.y, position.y + size.z + size01.z);

  terrain->CellSizeUpdate (this);
}

csTerrainCell::~csTerrainCell ()
{
  SetLoadState (NotLoaded);
}

iTerrainSystem* csTerrainCell::GetTerrain()
{
  return terrain;
}

csTerrainCell::LoadState csTerrainCell::GetLoadState () const
{
  return loadState;
}

void csTerrainCell::SetLoadState(LoadState state)
{
  Touch();

  switch (loadState)
  {
    case NotLoaded:
    {
      switch (state)
      {
        case NotLoaded: 
          break;
        case PreLoaded:
        {
          heightmap.SetSize (gridWidth * gridHeight, 0);
          normalmap.SetSize (gridWidth * gridHeight, 0);
	  needTangentsUpdate = true;

          if (materialMapPersistent)
            materialmap.SetSize (materialMapWidth * materialMapHeight, 0);
         
          loadState = terrain->GetFeeder ()->PreLoad (this)
	    ? PreLoaded : NotLoaded;

          if (loadState == PreLoaded)
          {
            terrain->FirePreLoadCallbacks (this);
          }

          break;
        }
        case Loaded:
        {
          heightmap.SetSize (gridWidth * gridHeight);
          normalmap.SetSize (gridWidth * gridHeight);
	  needTangentsUpdate = true;

          if (materialMapPersistent)
            materialmap.SetSize (materialMapWidth * materialMapHeight, 0);

          loadState = terrain->GetFeeder ()->Load (this)
	    ? Loaded : NotLoaded;

          if (loadState == Loaded)
          {
            terrain->UnloadOldCells();
            terrain->FireLoadCallbacks (this);
          }

          break;
        }
      }

      break;
    }
    case PreLoaded:
    {
      switch (state)
      {
        case NotLoaded: 
          break;
        case PreLoaded: 
          break;
        case Loaded:
        {
          loadState = terrain->GetFeeder ()->Load (this) ? Loaded : NotLoaded;

          if (loadState == Loaded)
          {
            terrain->UnloadOldCells();
            terrain->FireLoadCallbacks (this);
          }

          break;
        }
      }

      break;
    }
    case Loaded:
    {
      switch (state)
      {
        case NotLoaded:
        {
          terrain->FireUnloadCallbacks (this);

          heightmap.DeleteAll ();
          normalmap.DeleteAll ();
          materialmap.DeleteAll ();
	  tangentmap.DeleteAll ();
	  bitangentmap.DeleteAll ();

          renderData = 0;
          collisionData = 0;
          feederData = 0;

          loadState = NotLoaded;

          break;
        }
        case PreLoaded: 
          break;
        case Loaded: 
          break;
      }

      break; 
    }
  }
}

csBox3 csTerrainCell::GetBBox () const
{  
  return boundingBox;
}

const char* csTerrainCell::GetName () const
{
  return name.GetData ();
}

void csTerrainCell::SetName (const char* name)
{
  this->name = name;
}

iTerrainCellRenderProperties* csTerrainCell::GetRenderProperties () const
{
  return renderProperties;
}

iTerrainCellCollisionProperties* csTerrainCell::GetCollisionProperties () const
{
  return collisionProperties;
}

iTerrainCellFeederProperties* csTerrainCell::GetFeederProperties () const
{
  return feederProperties;
}

int csTerrainCell::GetGridWidth () const
{
  return gridWidth;
}

int csTerrainCell::GetGridHeight () const
{
  return gridHeight;
}

csLockedHeightData csTerrainCell::GetHeightData ()
{
  csLockedHeightData data;
  data.data = heightmap.GetArray ();
  data.pitch = gridWidth;

  return data;
}

csLockedHeightData csTerrainCell::LockHeightData (const csRect& rectangle)
{
  csLockedHeightData data;

  lockedHeightRect = rectangle;

  data.data = heightmap.GetArray () + gridWidth * rectangle.ymin +
    rectangle.xmin;

  data.pitch = gridWidth;

  return data;
}

void csTerrainCell::UnlockHeightData ()
{
  Touch();

  minHeight = FLT_MAX;
  maxHeight = -FLT_MAX;

  for (size_t i = 0; i < heightmap.GetSize (); ++i)
  {
    minHeight = csMin (minHeight, heightmap[i]);
    maxHeight = csMax (maxHeight, heightmap[i]);
  }

  const csVector3 size01 = size * 0.1f;
  boundingBox.Set (position.x - size01.x, minHeight - size01.y, position.y - size01.z,
    position.x + size.x + size01.x, maxHeight + size01.y, position.y + size.z + size01.z);

  terrain->CellSizeUpdate (this);

  terrain->FireHeightUpdateCallbacks (this, lockedHeightRect);
}

csLockedNormalData csTerrainCell::GetNormalData ()
{
  csLockedNormalData data;
  data.data = normalmap.GetArray ();
  data.pitch = gridWidth;

  return data;
}

csLockedNormalData csTerrainCell::LockNormalData (const csRect& rectangle)
{
  csLockedNormalData data;

  data.data = normalmap.GetArray () + gridWidth * rectangle.ymin +
    rectangle.xmin;

  data.pitch = gridWidth;

  return data;
}

void csTerrainCell::UnlockNormalData ()
{
  Touch();
  needTangentsUpdate = true;
}

void csTerrainCell::RecalculateNormalData ()
{
  csLockedNormalData cellNData = GetNormalData ();

  for (int y = 0; y < gridWidth; ++y)
  {
    csVector3* nRow = cellNData.data + y * gridHeight;

    for (int x = 0; x < gridHeight; ++x)
    {
      *nRow++ = GetNormal (x, y);
    }
  }
  needTangentsUpdate = true;
}

csLockedNormalData csTerrainCell::GetTangentData ()
{
  RecalculateTangentData();
  
  csLockedNormalData data;
  data.data = tangentmap.GetArray ();
  data.pitch = gridWidth;

  return data;
}

csLockedNormalData csTerrainCell::GetBitangentData ()
{
  RecalculateTangentData();
  
  csLockedNormalData data;
  data.data = bitangentmap.GetArray ();
  data.pitch = gridWidth;

  return data;
}

void csTerrainCell::RecalculateTangentData ()
{
  if (!needTangentsUpdate) return;
  needTangentsUpdate = false;
  
  tangentmap.SetSize (gridWidth * gridHeight);
  bitangentmap.SetSize (gridWidth * gridHeight);
  csVector3* tData = tangentmap.GetArray ();
  csVector3* bData = bitangentmap.GetArray ();

  for (int y = 0; y < gridWidth; ++y)
  {
    csVector3* tRow = tData + y * gridHeight;
    csVector3* bRow = bData + y * gridHeight;

    for (int x = 0; x < gridHeight; ++x)
    {
      *tRow++ = GetTangent (x, y);
      *bRow++ = GetBinormal (x, y);
    }
  }
}

const csVector2& csTerrainCell::GetPosition () const
{
  return position;
}

const csVector3& csTerrainCell::GetSize () const
{
  return size;
}

int csTerrainCell::GetMaterialMapWidth () const
{
  return materialMapWidth;
}

int csTerrainCell::GetMaterialMapHeight () const
{
  return materialMapHeight;
}

bool csTerrainCell::GetMaterialPersistent() const
{
  return materialMapPersistent;
}

csLockedMaterialMap csTerrainCell::LockMaterialMap (const csRect& rectangle)
{
  csLockedMaterialMap data;
  
  if (!materialMapPersistent)
  {
    materialmap.SetSize (rectangle.Width () * rectangle.Height ());

    data.data = materialmap.GetArray ();
    data.pitch = rectangle.Width ();
  }
  else
  {
    data.data = materialmap.GetArray () + materialMapWidth * rectangle.ymin +
      rectangle.xmin;
    data.pitch = materialMapWidth;
  }

  lockedMaterialMapRect = rectangle;

  return data;
}

void csTerrainCell::UnlockMaterialMap ()
{
  Touch();

  terrain->GetRenderer ()->OnMaterialMaskUpdate (this, lockedMaterialMapRect, 
    materialmap.GetArray (), materialMapWidth);

  /*
  for (unsigned int i = 0; i < terrain->GetMaterialPalette ().GetSize (); ++i)
  {
    for (int y = 0; y < lockedMaterialMapRect.Height (); ++y)
    {
      for (int x = 0; x < lockedMaterialMapRect.Width (); ++x)
      {
        size_t idx = y * lockedMaterialMapRect.Width () + x;
        unsigned char p = materialmap[idx];
        alpha[idx] = (p == i) ? 255 : 0;
      }
    }
    
    //@@TODO! Send update to renderer   
  }*/

  if (!materialMapPersistent) 
    materialmap.DeleteAll ();
}

void csTerrainCell::SetMaterialMask (unsigned int material, iImage* image)
{
  if (image->GetFormat () != CS_IMGFMT_PALETTED8) 
    return;

  Touch();
  
  if (image->GetWidth () != materialMapWidth || 
    image->GetHeight () != materialMapHeight)
  {
    image = csImageManipulate::Rescale (image, materialMapWidth,
      materialMapHeight);
  }
    
  terrain->GetRenderer ()->OnMaterialMaskUpdate (this, material,
    csRect(0, 0, image->GetWidth (), image->GetHeight ()),
    (const unsigned char*)image->GetImageData (), image->GetWidth ());
}

void csTerrainCell::SetMaterialMask (unsigned int material,
  const unsigned char* data, unsigned int width, unsigned int height)
{
  csImageMemory image(width, height, (void*)data, false, CS_IMGFMT_PALETTED8);
	
  SetMaterialMask (material, &image);
}

void csTerrainCell::SetAlphaMask (iMaterialWrapper* material, iImage* alphaMap)
{
  Touch();

  // Make sure we have a true color image
  csRef<iImage> image;
  image.AttachNew (new csImageMemory (alphaMap, 
    CS_IMGFMT_TRUECOLOR | (alphaMap->GetFormat () & ~CS_IMGFMT_MASK)));

  terrain->GetRenderer ()->OnAlphaMapUpdate (this, material, image);
}

void csTerrainCell::SetBaseMaterial (iMaterialWrapper* material)
{
  Touch ();

  baseMaterial = material;
}

iMaterialWrapper* csTerrainCell::GetBaseMaterial () const
{
  return baseMaterial;
}

void csTerrainCell::SetAlphaSplatMaterial (iMaterialWrapper* material)
{
  Touch ();

  alphaSplatMaterial = material;
}

iMaterialWrapper* csTerrainCell::GetAlphaSplatMaterial () const
{
  return alphaSplatMaterial;
}

void csTerrainCell::SetSplatBaseMaterial (iMaterialWrapper* material)
{
  Touch ();

  splatBaseMaterial = material;
}

iMaterialWrapper* csTerrainCell::GetSplatBaseMaterial () const
{
  return splatBaseMaterial;
}

bool csTerrainCell::CollideSegment (const csVector3& start, 
  const csVector3& end, bool oneHit, iTerrainVector3Array* points)
{
  iTerrainCollider* collider = terrain->GetCollider ();

  if (!collider || !collisionProperties->GetCollidable ()) 
    return false;

  return collider->CollideSegment (this, start, end, oneHit, points);  
}

csTerrainColliderCollideSegmentResult csTerrainCell::CollideSegment (
    const csVector3& start, const csVector3& end)
{
  iTerrainCollider* collider = terrain->GetCollider ();

  if (!collider || !collisionProperties->GetCollidable ()) 
  {
    return csTerrainColliderCollideSegmentResult ();
  }

  return collider->CollideSegment (this, start, end);
}

bool csTerrainCell::CollideSegment (const csVector3& start, const csVector3& end,
				    csVector3& hitPoint)
{
  iTerrainCollider* collider = terrain->GetCollider ();

  if (!collider || !collisionProperties->GetCollidable ()) 
    return false;

  return collider->CollideSegment (this, start, end, hitPoint);  
}

bool csTerrainCell::CollideTriangles (const csVector3* vertices,
  size_t tri_count, const unsigned int* indices, float radius,
  const csReversibleTransform& trans, bool oneHit,
  iTerrainCollisionPairArray* pairs)
{
  iTerrainCollider* collider = terrain->GetCollider ();

  if (!collider || !collisionProperties->GetCollidable ()) 
    return false;

  return collider->CollideTriangles (this, vertices, tri_count, indices,
                                     radius, trans, oneHit, pairs);
}

bool csTerrainCell::Collide (iCollider* col, float radius,
  const csReversibleTransform& trans, bool oneHit,
  iTerrainCollisionPairArray* pairs)
{
  iTerrainCollider* collider = terrain->GetCollider ();

  if (!collider || !collisionProperties->GetCollidable ()) 
    return false;

  return collider->Collide (this, col, radius, trans, oneHit, pairs);
}

static inline float Lerp (const float x1, const float x2, const float t)
{
  return x1 + (x2 - x1) * t;
}

void csTerrainCell::LerpHelper (const csVector2& pos, int& x1, int& x2,
  float& xfrac, int& y1, int& y2, float& yfrac) const
{
  float x = (pos.x / size.x) * (gridWidth - 1);
  float y = (pos.y / size.z) * (gridHeight - 1);

  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x > gridWidth - 1) x = gridWidth - 1;
  if (y > gridHeight - 1) y = gridHeight - 1;

  x1 = int (floorf (x));
  x2 = int (ceilf (x));
  xfrac = x - x1;

  y1 = int (floorf (y));
  y2 = int (ceilf (y));
  yfrac = y - y1;
}

float csTerrainCell::GetHeight (int x, int y) const
{
  return heightmap[y * gridWidth + x];
}

float csTerrainCell::GetHeight (const csVector2& pos) const
{
  int x1, y1, x2, y2;
  float xfrac, yfrac;

  LerpHelper (pos, x1, x2, xfrac, y1, y2, yfrac);

  float h1 = Lerp (GetHeight (x1, y1), GetHeight (x2, y1), xfrac);
  float h2 = Lerp (GetHeight (x1, y2), GetHeight (x2, y2), xfrac);

  return Lerp (h1, h2, yfrac);
}

static inline csVector3 Lerp (const csVector3& x, const csVector3& y,
  const float t)
{
  return x + (y - x) * t;
}

csVector3 csTerrainCell::GetTangentDN (int x, int y) const
{
  //@@TODO! check if this is correct
  float center = GetHeight (x, y);
  
  float dfdx = 0;
  if (x - 1 >= 0 && x + 1 < gridWidth)
    dfdx = (GetHeight (x + 1, y) - GetHeight (x - 1, y)) / (2*step_x); 
  else if (x - 1 >= 0)
    dfdx = (center - GetHeight (x - 1, y)) / step_x;
  else if (x + 1 < gridWidth)
    dfdx = (GetHeight (x + 1, y) - center) / step_x;

  return csVector3 (1, dfdx, 0);
}

csVector3 csTerrainCell::GetTangent (int x, int y) const
{
  return GetTangentDN (x, y).Unit ();
}

csVector3 csTerrainCell::GetTangent (const csVector2& pos) const
{
  //@@TODO! check if this is correct
  int x1, y1, x2, y2;
  float xfrac, yfrac;

  LerpHelper (pos, x1, x2, xfrac, y1, y2, yfrac);

  csVector3 n1 = Lerp (GetTangentDN (x1, y1), GetTangentDN (x2, y1), xfrac);
  csVector3 n2 = Lerp (GetTangentDN (x1, y2), GetTangentDN (x2, y2), xfrac);

  return Lerp (n1, n2, yfrac).Unit ();
}

csVector3 csTerrainCell::GetBinormalDN (int x, int y) const
{
  //@@TODO! check if this is correct
  float center = GetHeight (x, y);
  
  float dfdy = 0;
  if (y - 1 >= 0 && y + 1 < gridHeight)
    dfdy = (GetHeight (x, y + 1) - GetHeight (x, y - 1)) / (2*step_z); 
  else if (y - 1 >= 0)
    dfdy = (center - GetHeight (x, y - 1)) / step_z;
  else if (y + 1 < gridHeight)
    dfdy = (GetHeight (x, y + 1) - center) / step_z;

  return csVector3(0, dfdy, -1);
}

csVector3 csTerrainCell::GetBinormal (int x, int y) const
{
  return GetBinormalDN (x, y).Unit ();
}

csVector3 csTerrainCell::GetBinormal (const csVector2& pos) const
{
  //@@TODO! check if this is correct
  int x1, y1, x2, y2;
  float xfrac, yfrac;

  LerpHelper (pos, x1, x2, xfrac, y1, y2, yfrac);

  csVector3 n1 = Lerp (GetBinormalDN (x1, y1), GetBinormalDN (x2, y1), xfrac);
  csVector3 n2 = Lerp (GetBinormalDN (x1, y2), GetBinormalDN (x2, y2), xfrac);

  return Lerp (n1, n2, yfrac).Unit ();
}

csVector3 csTerrainCell::GetNormalDN (int x, int y) const
{
  float center = GetHeight (x, y);

  float dfdy = 0;
  if (y - 1 >= 0 && y + 1 < gridHeight)
    dfdy = (GetHeight (x, y + 1) - GetHeight (x, y - 1)) / (2*step_z); 
  else if (y - 1 >= 0)
    dfdy = (center - GetHeight (x, y - 1)) / step_z;
  else if (y + 1 < gridHeight)
    dfdy = (GetHeight (x, y + 1) - center) / step_z;

  float dfdx = 0;
  if (x - 1 >= 0 && x + 1 < gridWidth)
    dfdx = (GetHeight (x + 1, y) - GetHeight (x - 1, y)) / (2*step_x); 
  else if (x - 1 >= 0)
    dfdx = (center - GetHeight (x - 1, y)) / step_x;
  else if (x + 1 < gridWidth)
    dfdx = (GetHeight (x + 1, y) - center) / step_x;

  return csVector3 (-dfdx, 1, dfdy);
}

csVector3 csTerrainCell::GetNormal (int x, int y) const
{
  return GetNormalDN (x, y).Unit ();
}

csVector3 csTerrainCell::GetNormal (const csVector2& pos) const
{
  int x1, y1, x2, y2;
  float xfrac, yfrac;

  LerpHelper (pos, x1, x2, xfrac, y1, y2, yfrac);

  csVector3 n1 = Lerp (GetNormalDN (x1, y1), GetNormalDN (x2, y1), xfrac);
  csVector3 n2 = Lerp (GetNormalDN (x1, y2), GetNormalDN (x2, y2), xfrac);

  return Lerp (n1, n2, yfrac).Unit ();
}

csRefCount* csTerrainCell::GetRenderData () const
{
  return renderData;
}
void csTerrainCell::SetRenderData (csRefCount* data)
{
  renderData = data;
}

csRefCount* csTerrainCell::GetCollisionData () const
{
  return collisionData;
}
void csTerrainCell::SetCollisionData (csRefCount* data)
{
  collisionData = data;
}

csRefCount* csTerrainCell::GetFeederData () const
{
  return feederData;
}
void csTerrainCell::SetFeederData (csRefCount* data)
{
  feederData = data;
}


}
CS_PLUGIN_NAMESPACE_END(Terrain2)
