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

#include "dochelp.h"

CCSThing::CCSThing(CMapEntity* pEntity)
  : CIThing(pEntity)
{
}

CCSThing::~CCSThing()
{
}

bool CCSThing::Write(csRef<iDocumentNode> node, CIWorld* pIWorld, CISector* pISector)
{
  if (!ContainsVisiblePolygons()) return true;

  assert(pIWorld);
  assert(pISector);

  CCSWorld*  pWorld  = (CCSWorld*)  pIWorld;
  CCSSector* pSector = (CCSSector*) pISector;

  DocNode meshobj = CreateNode (node, "meshobj");
  meshobj->SetAttribute ("name", GetName());
  CreateNode (meshobj, "plugin", "thing");
  CreateNode (meshobj, "zuse");

  CCSWorld::WriteKeys(meshobj, pWorld, m_pOriginalEntity);

  DocNode params = CreateNode (meshobj, "params");

  bool Sky;
  WriteAsPart(params, pWorld, pSector, Sky);
  
  // New Alpha
  double a = m_pOriginalEntity->GetNumValueOfKey("alpha", -1.00);
  if (a != -1.00) {
    DocNode mixmode = CreateNode (params, "mixmode");
    DocNode alpha = CreateNode (mixmode, "alpha", (float)a);
  }

  DocNode priority = meshobj->CreateNodeBefore (CS_NODE_ELEMENT,
    params);
  priority->SetValue ("priority");
  priority = priority->CreateNodeBefore (CS_NODE_TEXT);
  
  if (IsSky())
    priority->SetValue ("sky");
  else
  {
    const char*  prior;
    prior = m_pOriginalEntity->GetValueOfKey("priority", "");
    if (strcmp(prior, "alpha")==0)
      priority->SetValue("alpha");
    else if (strcmp(prior, "mirror")==0)
      priority->SetValue("mirror");
    else
      priority->SetValue("object");

    //priority->SetValue (m_pOriginalEntity->GetValueOfKey("priority", 
    //  m_pOriginalEntity->GetValueOfKey("mirror")?"mirror":"object"));
  }

  return true;
}

bool CCSThing::WriteAsPart(csRef<iDocumentNode> node, CIWorld* pIWorld, CISector* pISector,
			   bool &Sky)
{
  if (!ContainsVisiblePolygons()) return true;

  assert(pIWorld);
  assert(pISector);

  CCSWorld*  pWorld  = (CCSWorld*)  pIWorld;
  CCSSector* pSector = (CCSSector*) pISector;


  CMapFile* pMap         = pWorld->GetMap();

  assert(pMap);

  size_t i, j;

  //DocNode part = CreateNode (node, "part");
  //part->SetAttribute ("name", 
  //  csString().Format ("part_%s", GetName()));
  DocNode part = node;

  CVertexBuffer Vb;
  Vb.AddVertices(&m_Polygon);

  Vb.WriteCS(part, pWorld);

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
	bool poly_sky;
        pWorld->WritePolygon(part, pPolygon, pSector, false, Vb, poly_sky);
	Sky = Sky | poly_sky;
      } //if texture is visible
    }
  }

  return true;
}
