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
#include "mparser.h"
#include "texplane.h"
#include "mpoly.h"
#include "zipfile.h"
#include "csutil/cfgfile.h"

CMapFile::CMapFile()
{
  m_pConfigFile    = new csConfigFile;
  m_IniFilename = 0;
  m_NumBrushes  = 0;
}

CMapFile::~CMapFile()
{
  DELETE_VECTOR_MEMBERS(m_Planes);
  DELETE_VECTOR_MEMBERS(m_Entities);

  m_pConfigFile->Save (m_IniFilename);

  delete m_pConfigFile;
  delete m_IniFilename;
}

bool CMapFile::Read(const char* filename, const char* configfile)
{
  m_IniFilename = strdup(configfile);
  m_pConfigFile->Load (configfile);
  m_TextureManager.LoadTextureArchives(this);

  m_NumBrushes  = 0;

  /***********************
  Kind of sloppy, ill find
  a nicer way to do it in 
  the future!
  ***********************/

  CMapParser preParser;
  if (!preParser.Open(filename)) return false;

  bool inBrush = false;

  csString Buff, Key, Value;
  while (preParser.GetNextToken(Buff))
  {
    if (strcmp(Buff, "{")==0) 
    {
      //new entity//
      while (preParser.GetNextToken(Buff)) 
      {	
	if (strcmp(Buff, "}")==0) 
	{
	  //end of entity or brush//
	  if (inBrush)
	    inBrush = false;
	  else
	    break;
	} 
	else if (strcmp(Buff, "{")==0) 
	{
	  //Beginning of brush//
	  inBrush = true;
	}
	else if (strcmp(Buff, "{")==0)
	{
	  //Info in a brush//
	}
	else
	{
	  //Key Pair//
	  Key.Replace (Buff);
	  if (strcmp(Key, "archive")==0) 
	  {
	    if (!preParser.GetNextToken (Buff))
	    {
	      preParser.ReportError("Format error. Keys and values for entities must"
			    "always come in pairs. Found no match for key \"%s\"",
			    Key.GetData());
	      return false;
	    }
	    Value.Replace (Buff);
	    m_TextureManager.LoadArchive (Value);
	  }
	}
      }
    }
  }

  CMapParser parser;
  if (!parser.Open(filename)) return false;

  csString Buffer;

  while (parser.GetNextToken(Buffer))
  {
    if (strcmp(Buffer, "{") == 0)
    {
      //a new entity follows.
      CMapEntity* pEntity = new CMapEntity;
      if (!pEntity->Read(&parser, this))
      {
        return false;
      }
      m_Entities.Push(pEntity);
      m_NumBrushes += pEntity->GetNumBrushes();
    }
    else
    {
      parser.ReportError("Format error! Expected an entity, that starts with \"{\" "
                         "but found \"%s\"", Buffer.GetData());
      return false;
    }
  }

  csPrintf("Map contains:\n");
  csPrintf("%zu Entites\n", m_Entities.Length());
  csPrintf("%zu Brushes\n", m_NumBrushes);
  csPrintf("%zu Unique planes\n", m_Planes.Length());
  return true;
}

bool CMapFile::WriteTextureinfo()
{
  m_pConfigFile->Save (m_IniFilename);

  return true;
}

CMapTexturedPlane* CMapFile::AddPlane(CdVector3 v1, CdVector3 v2, CdVector3 v3,
                                      const char* TextureName,
                                      double x_off, double y_off,
                                      double rot_angle,
                                      double x_scale, double y_scale,
				      CdVector3 v_tx_right, CdVector3 v_tx_up,
                                      bool QuarkModeTexture,
                                      bool QuarkMirrored,
				      bool HLTexture)
{
  CTextureFile* pTexture = m_TextureManager.GetTexture(TextureName);
  if (!pTexture)
  {
    csPrintf("Fatal error: Texture '%s', not found! aborting!\n", TextureName);
    exit(1);
  }

  CMapTexturedPlane* pNewPlane =
    new CMapTexturedPlane(this, v1, v2, v3, pTexture, x_off,
                          y_off, rot_angle, x_scale, y_scale,
			  v_tx_right, v_tx_up,
                          QuarkModeTexture, QuarkMirrored,
			  HLTexture);
  return AddPlane(pNewPlane);
}

CMapTexturedPlane* CMapFile::AddPlane(CdVector3 v1, CdVector3 v2, CdVector3 v3,
                                      int r, int g, int b)
{
  CMapTexturedPlane* pNewPlane = new CMapTexturedPlane(v1, v2, v3, r, g, b);
  return AddPlane(pNewPlane);
}

CMapTexturedPlane* CMapFile::AddPlane(CMapTexturedPlane* pNewPlane)
{
  //first we look in m_Planes to check, if a similar plane is already stored.
  size_t i, NumPlanes = m_Planes.Length();
  for (i=0; i<NumPlanes; i++)
  {
    CMapTexturedPlane* pPlane = m_Planes[i];
    assert(pPlane);

    if (pPlane->IsEqual(pNewPlane))
    {
      //That plane is already stored. So we return a pointer to it.
      delete pNewPlane;
      return pPlane;
    }
  }

  //There is no such plane, so we will add the new plane.
  m_Planes.Push(pNewPlane);

  //Create a mirror plane for this plane.
  CMapTexturedPlane* pMirrorPlane = new CMapTexturedPlane(pNewPlane, true);

  //Add the mirror plane too.
  m_Planes.Push(pMirrorPlane);

  //Set Mirrorinfo to both planes.
  pNewPlane->   SetMirror(pMirrorPlane);
  pMirrorPlane->SetMirror(pNewPlane);

  //return the unmirrored plane.
  return pNewPlane;
}

void CMapFile::CreatePolygons()
{
  size_t i;
  for (i=0; i<m_Entities.Length(); i++)
  {
    m_Entities[i]->CreatePolygons();
  }
}

void CMapFile::GetMapSize(CdVector3& Min, CdVector3& Max)
{
  bool inited = false;
  Min = CdVector3(0,0,0);
  Max = CdVector3(0,0,0);

  //iterate all entities, brushes, polygons and vertices:
  size_t i;
  for (i=0; i<GetNumEntities(); i++)
  {
    CMapEntity* pEntity = GetEntity((int)i);

    // First take care of the "origin" of entities, because otherwise
    // in very open maps, we might miss some lights.
    CdVector3 v(0,0,0);
    if (pEntity->GetOrigin(v))
    {
      if (!inited)
      {
        //we will intialize Min and Max to the value of the first
        //vertex we find. (We can't just intialise Min and Max to
        //(0,0,0), because we can'T guarantee, that nobody will
        //create a map, that does not even enclude that point
        //inside the maps objects.
        inited = true;
        Min = v;
        Max = v;
      }
      else
      {
        //Extend Min and Max if needed.
        if (v.x > Max.x) Max.x = v.x;
        if (v.y > Max.y) Max.y = v.y;
        if (v.z > Max.z) Max.z = v.z;

        if (v.x < Min.x) Min.x = v.x;
        if (v.y < Min.y) Min.y = v.y;
        if (v.z < Min.z) Min.z = v.z;
      }
    }

    size_t j, k, l;
    for (j=0; j<pEntity->GetNumBrushes(); j++)
    {
      CMapBrush* pBrush = pEntity->GetBrush(j);
      for (k=0; k<pBrush->GetPolygonCount(); k++)
      {
        CMapPolygon* pPolygon = pBrush->GetPolygon(k);
        for (l=0; l<pPolygon->GetVertexCount(); l++)
        {
          CdVector3 v = pPolygon->GetVertex(l);
          if (!inited)
          {
            //we will intialize Min and Max to the value of the first
            //vertex we find. (We can't just intialise Min and Max to
            //(0,0,0), because we can'T guarantee, that nobody will
            //create a map, that does not even enclude that point
            //inside the maps objects.
            inited = true;
            Min = v;
            Max = v;
          }
          else
          {
            //Extend Min and Max if needed.
            if (v.x > Max.x) Max.x = v.x;
            if (v.y > Max.y) Max.y = v.y;
            if (v.z > Max.z) Max.z = v.z;

            if (v.x < Min.x) Min.x = v.x;
            if (v.y < Min.y) Min.y = v.y;
            if (v.z < Min.z) Min.z = v.z;
          }
        } //for vertex
      } //for poly
    } //for brush
  } //for entity
}

CTextureFile* CMapFile::GetTexture(const char* TextureName)
{
  return m_TextureManager.GetTexture(TextureName);
}

/*
bool CMapFile::AddTexture(const char* TextureName, CZipFile* pZipFile)
{
  char* pData;
  int   Size;
  char simplename[256];
  char fullname[256];
  strcpy(simplename, TextureName);
  int len = strlen(simplename);
  if (len > 4 && simplename[len-4] == '.')
  {
    //remove the extension ".bmp"
    simplename[len-4] = 0;
  }

  for (int i=0; i<m_Wads.Length(); i++)
  {
    if (m_Wads[i]->Extract(simplename, pData, Size))
    {
      bool ok = pZipFile->AddData(pData, Size, TextureName);
      delete[] pData;
      return ok;
    }
  }

  if (FindTextureFile(TextureName, fullname))
  {
    if (pZipFile->AddFile(fullname, TextureName))
    {
      return true;
    }
    else
    {
      csPrintf("Can't add '%s' to Archive! Aborting!\n", TextureName);
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool CMapFile::FindTextureFile(const char* name, char* fullname)
{
  for (int pathnr = 1; true; pathnr++)
  {
    char keyname[50];
    char Path[256];
    scsPrintf(keyname, "path%d", pathnr);
    strcpy(Path, GetIniStr("texturesettings", keyname, ""));
    if (Path[0])
    {
      char Buffer[300];
      scsPrintf(Buffer, "%s/%s", Path, name);
      FILE* fd = fopen(Buffer, "r");
      if (fd)
      {
        fclose(fd);
        strcpy(fullname, Buffer);
        return true;
      }
    }
    else
    {
      return false;
    }
  }
}
*/

int CMapFile::GetConfigInt(const char* Path, int def)
{
  assert(m_pConfigFile);
  return m_pConfigFile->GetInt(Path, def);
}

double CMapFile::GetConfigFloat(const char* Path, double def)
{
  assert(m_pConfigFile);
  return m_pConfigFile->GetFloat(Path, def);
}

const char* CMapFile::GetConfigStr(const char* Path, const char* def)
{
  assert(m_pConfigFile);
  return m_pConfigFile->GetStr(Path, def);
}

/*
void CMapFile::LoadWadFiles()
{
  int wadnr = 1;
  do
  {
    char keyname[200];
    char wadname[300];
    scsPrintf(keyname, "wad%d", wadnr);
    strcpy(wadname, GetIniStr("texturesettings", keyname, ""));
    if (wadname[0])
    {
      wadnr++;
      CWad3File* pWad = new CWad3File;
      if (pWad->Open(wadname))
      {
        m_Wads.Push(pWad);
      }
      else
      {
        delete pWad;
      }
    }
    else
    {
      wadnr = -1;
    }
  }
  while (wadnr>0);
}
*/
