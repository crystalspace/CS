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
#include "mpoly.h"
#include "mpolyset.h"
#include "texplane.h"
#include "cthing.h"
#include "csector.h"
#include "vertbuf.h"
#include "entity.h"
#include "cworld.h"

CCSThing::CCSThing(CMapEntity* pEntity)
  : CIThing(pEntity)
{
}

CCSThing::~CCSThing()
{
}

bool CCSThing::Write(CIWorld* pIWorld, CISector* pISector)
{
  if (!ContainsVisiblePolygons()) return true;

  assert(pIWorld);
  assert(pISector);

  CCSWorld*  pWorld  = (CCSWorld*)  pIWorld;
  CCSSector* pSector = (CCSSector*) pISector;

  FILE*     fd           = pWorld->GetFile();
  assert(fd);

  pWorld->WriteIndent();
  fprintf(fd, "MESHOBJ '%s'(\n", GetName());
  pWorld->Indent();

  pWorld->WriteIndent();
  fprintf(fd, "PLUGIN('thing')\n");
  pWorld->WriteIndent();
  fprintf(fd, "ZUSE()\n");
  pWorld->WriteIndent();
  fprintf(fd, "PRIORITY('object')\n");

  //Activate a script
  char scriptname[99] = "none";
  const char* activateval = m_pOriginalEntity->GetValueOfKey("activate");
  if (activateval)
  {
    char dummy;
    if (sscanf(activateval, "%s%c",scriptname, &dummy) == 1)
    {
      pWorld->WriteIndent();
      fprintf(fd, "ACTIVATE ('%s')\n", scriptname);
    }
  }
  //Trigger a script TRIGGER ('activate', 'scriptname')
  const char* triggerval = m_pOriginalEntity->GetValueOfKey("trigger");
  if (triggerval)
  {
    char dummy;
    if (sscanf(triggerval, "%s%c",scriptname, &dummy) == 1)
    {
      pWorld->WriteIndent();
      fprintf(fd, "TRIGGER ('activate', '%s')\n", scriptname);
    }
  }

  CCSWorld::WriteKeys(pWorld, m_pOriginalEntity);

  pWorld->WriteIndent();
  fprintf(fd, "PARAMS(\n");
  pWorld->Indent();

  WriteAsPart(pWorld, pSector);

  pWorld->Unindent();
  pWorld->WriteIndent();
  fprintf(fd, ")\n"); //PARAMS

  pWorld->Unindent();
  pWorld->WriteIndent();
  fprintf(fd, ")\n\n"); //MESHOBJ

  return true;
}

bool CCSThing::WriteAsPart(CIWorld* pIWorld, CISector* pISector)
{
  if (!ContainsVisiblePolygons()) return true;

  assert(pIWorld);
  assert(pISector);

  CCSWorld*  pWorld  = (CCSWorld*)  pIWorld;
  CCSSector* pSector = (CCSSector*) pISector;


  CMapFile* pMap         = pWorld->GetMap();
  FILE*     fd           = pWorld->GetFile();

  assert(pMap);
  assert(fd);

  int i, j;

  //If things are moveable, they need special tagging. (needed for scripting)
  if (IsMoveable())
  {
    pWorld->WriteIndent();
    fprintf(fd, "MOVEABLE ()\n"); 
  } 

  pWorld->WriteIndent();
  fprintf(fd, "PART 'part_%s' (\n", GetName());

  CVertexBuffer Vb;
  Vb.AddVertices(&m_Polygon);

  pWorld->Indent();
  Vb.WriteCS(pWorld);

  for (i=0; i<m_Polygon.Length(); i++)
  {
    CMapPolygonSet* pPolySet = m_Polygon[i];
    for (j=0; j<pPolySet->GetPolygonCount(); j++)
    {
      CMapPolygon*             pPolygon = pPolySet->GetPolygon(j);
      const CMapTexturedPlane* pPlane   = pPolygon->GetBaseplane();
      CTextureFile*            pTexture = pPlane->GetTexture();

      if (pTexture->IsVisible())
      {
        pWorld->WritePolygon(pPolygon, pSector, false, Vb);
      } //if texture is visible
    }
  }  

  pWorld->Unindent();
  pWorld->WriteIndent();
  fprintf(fd, ")\n");

  return true;
}
