/*
    Map2cs: a convertor to convert the frequently used MAP format, into
    something, that can be directly understood by Crystal Space.

    Copyright (C) 1999 Thomas Hieber (thieber@gmx.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "cssysdef.h"
#include "mapstd.h"
#include "map.h"
#include "entity.h"
#include "mpoly.h"
#include "texplane.h"
#include "zipfile.h"
#include "mcurve.h"
#include "ithing.h"
#include "isector.h"
#include "iworld.h"

#define TEMPWORLD "map2cs2.$$$"

CIWorld::CIWorld()
{
  m_pMap   = 0;
  m_ScaleFactor = 1.0/40.0;
}

CIWorld::~CIWorld()
{
  DELETE_VECTOR_MEMBERS(m_Sectors);

  DELETE_VECTOR_MEMBERS(m_TextureFileNames);

  //m_Entities are only stored as references, so we don't need to delete
  //them here.
}

void CIWorld::FindSectors()
{
  size_t j, SectorCounter = 0;
  size_t i;

  //iterate all entities and brushes
  for (i=0; i<m_pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = m_pMap->GetEntity((int)i);
    if (strcmp(pEntity->GetClassname(), "cs_sector")==0)
    {
      //This entity is a sector!
      for (j=0; j<pEntity->GetNumBrushes(); j++)
      {
        SectorCounter++;
        CMapBrush* pBrush  = pEntity->GetBrush(j);
        CISector* pSector = CreateNewSector(pBrush);
        m_Sectors.Push(pSector);
      } //for brush
    }
    else
    {
      //no sector. Remember the pointer to it.
      m_Entities.Push(pEntity);
    }
  } //for entity

  if (m_Sectors.Length() == 0)
  {
    GenerateDefaultsector();
  }
}

void CIWorld::GenerateDefaultsector()
{
  if (m_pMap->GetPlaneCount()<=0)
  {
    printf("Map contains no data. Aborting!");
    exit(1);
  }

  //Find out the size of this sector
  CdVector3 Min, Max;
  m_pMap->GetMapSize(Min, Max);

  Min.x -= 10;
  Min.y -= 10;
  Min.z -= 10;

  Max.x += 10;
  Max.y += 10;
  Max.z += 10;

  CdVector3 v[8];
  v[0] = CdVector3(Min.x, Min.y, Min.z);
  v[1] = CdVector3(Max.x, Min.y, Min.z);
  v[2] = CdVector3(Max.x, Min.y, Max.z);
  v[3] = CdVector3(Min.x, Min.y, Max.z);
  v[4] = CdVector3(Min.x, Max.y, Min.z);
  v[5] = CdVector3(Max.x, Max.y, Min.z);
  v[6] = CdVector3(Max.x, Max.y, Max.z);
  v[7] = CdVector3(Min.x, Max.y, Max.z);

  static int Planes[6][4] =
  {
    {4,5,1,0},
    {5,6,2,1},
    {6,7,3,2},
    {7,4,0,3},
    {0,1,2,3},
    {7,6,5,4}
  };


  //Create the Brush for that sector;
  CMapBrush* pBrush = new CMapBrush(0);

  int i;
  for (i=0; i<6; i++)
  {
    //For every side of the default sector create a flatshaded
    //plane in black color.
    CMapTexturedPlane* pPlane = m_pMap->AddPlane(v[Planes[i][2]],
                                                 v[Planes[i][1]],
                                                 v[Planes[i][0]],
                                                 0,0,0); //black
    assert(pPlane);
    pBrush->AddPlane(pPlane);
  }

  pBrush->CreatePolygons();

  CISector* pSector = CreateNewSector(pBrush);
  m_Sectors.Push(pSector);
}

void CIWorld::FindPortals()
{
  size_t i, j;
  for (i=0; i<m_Sectors.Length(); i++)
  {
    CISector* pSector1 = m_Sectors[i];
    assert(pSector1);

    for (j=0; j<m_Sectors.Length(); j++)
    {
      if (j==i) continue;

      CISector* pSector2 = m_Sectors[j];
      assert(pSector2);

      pSector1->CreatePortal(pSector2);
    }
    pSector1->TextureWalls(this);
  }
}

void CIWorld::InsertThings()
{
  size_t i;
  for (i=0; i<m_Sectors.Length(); i++)
  {
    CISector* pSector = m_Sectors[i];
    assert(pSector);

    pSector->InsertThings(this);
  }
}

CISector* CIWorld::FindSectorForPoint(CdVector3& v)
{
  size_t i;
  for (i=0; i<m_Sectors.Length(); i++)
  {
    CISector* pSector = m_Sectors[i];
    assert(pSector);

    if (pSector->IsInside(v))
    {
      return pSector;
    }
  }

  return 0;
}


void CIWorld::BuildTexturelist()
{
  size_t c;
  size_t j, i;

  for (i=0; i<m_pMap->GetPlaneCount(); i++)
  {
    CTextureFile* pTexture = m_pMap->GetPlane((int)i)->GetTexture();

    bool TextureFound = false;
    for (j=0; j<m_TextureFileNames.Length(); j++)
    {
      if (strcmp(m_TextureFileNames[j], pTexture->GetFilename()) == 0)
      {
        //That texture is already registered, so we don't need
        //to register it again.
        TextureFound = true;
        break;
      }
    }
    if (!TextureFound)
    {
      // Add the name to the list.
      m_TextureFileNames.Push(strdup(pTexture->GetFilename()));
    }
  }

  for (i=0; i<m_pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = m_pMap->GetEntity((int)i);
    for (c=0; c<pEntity->GetCurveCount(); c++)
    {
      CMapCurve*    pCurve   = pEntity->GetCurve(c);
      CTextureFile* pTexture = pCurve->GetTexture();

      bool TextureFound = false;
      for (j=0; j<m_TextureFileNames.Length(); j++)
      {
        if (strcmp(m_TextureFileNames[j], pTexture->GetFilename()) == 0)
        {
          //That texture is already registered, so we don't need
          //to register it again.
          TextureFound = true;
          break;
        }
      }
      if (!TextureFound)
      {
        // Add the name to the list.
        m_TextureFileNames.Push(strdup(pTexture->GetFilename()));
      }
    }
  }

  //add textures for all 2D cs_sprite
  for (i=0; i<m_pMap->GetNumEntities(); i++)
  {
    CMapEntity* pEntity = m_pMap->GetEntity((int)i);
    if (strcmp(pEntity->GetClassname(), "cs_sprite")==0)
    {
      CTextureManager* pTexMan  = m_pMap->GetTextureManager();

	  const char* texturefilename = pEntity->GetValueOfKey("texture");
      CTextureFile* pTexture = pTexMan->GetTexture(texturefilename);

      bool TextureFound = false;
      for (j=0; j<m_TextureFileNames.Length(); j++)
      {
        if (strcmp(m_TextureFileNames[j], pTexture->GetFilename()) == 0)
        {
          //That texture is already registered, so we don't need
          //to register it again.
          TextureFound = true;
          break;
        }
      }
      if (!TextureFound)
      {
        // Add the name to the list.
        m_TextureFileNames.Push(strdup(pTexture->GetFilename()));
      }
	}
  }
}

bool CIWorld::PrepareData(const char* filename, CMapFile* pMap)
{
  m_pMap = pMap;
  m_ScaleFactor = pMap->GetConfigFloat("Map2CS.General.Scaling", 1.0/40.0);

  /**************************
  Added in to let scaleing be
  specified in the worldspawn
  **************************/

  size_t maxEnt = pMap->GetNumEntities();

  for (size_t i = 0; i < maxEnt; i++) 
  {
    CMapEntity*	curEnt = pMap->GetEntity(i);

    const char* classname = curEnt->GetValueOfKey("classname");
    if ((classname != 0) && (strcmp(classname, "worldspawn") == 0))
    {
      if (curEnt->GetValueOfKey ("world_scale", 0) != 0)
      {
	m_ScaleFactor = curEnt->GetNumValueOfKey ("world_scale");
	break;
      }
    }
  }

  BuildTexturelist();
  FindSectors();
  FindPortals();
  InsertThings();

  return true;
}

