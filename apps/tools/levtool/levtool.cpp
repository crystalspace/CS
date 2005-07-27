/*
    Copyright (C) 2002 by Jorrit Tyberghein

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

#include <stdarg.h>
#include "cssysdef.h"

#include "csgeom/math3d.h"
#include "csgeom/plane3.h"
#include "csutil/util.h"
#include "csutil/xmltiny.h"
#include "csutil/csstring.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "iutil/vfs.h"
#include "iutil/cmdline.h"
#include "iutil/document.h"
#include "cstool/initapp.h"
#include "ivaria/reporter.h"
#include "iutil/databuff.h"

#include "levtool.h"

//-----------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

#define OP_HELP 0
#define OP_LIST 1
#define OP_DYNAVIS 2
#define OP_VALIDATE 3
#define OP_SPLITUNIT 4
#define OP_SPLITGEOM 5
#define OP_COMPRESS 7
#define OP_ANALYZE 8
#define OP_FLAGCLEAR 10
#define OP_FLAGGOOD 11
#define OP_FLAGBAD 12
#define OP_TRANSLATE 13
#define OP_PLANES 14

//-----------------------------------------------------------------------------

void LevTool::ReportError (const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (object_reg, CS_REPORTER_SEVERITY_ERROR,
  	"crystalspace.apps.levtool", description, arg);
  va_end (arg);
}

void LevTool::Report (int severity, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (object_reg, severity,
  	"crystalspace.apps.levtool", description, arg);
  va_end (arg);
}

//-----------------------------------------------------------------------------

ltVertex::ltVertex ()
{
  polygons = 0;
  num_polygons = 0;
  max_polygons = 0;
}

ltVertex::ltVertex (const ltVertex& vt) : csVector3 (vt.x, vt.y, vt.z)
{
  if (vt.max_polygons > 0)
  {
    polygons = new int [vt.max_polygons];
    num_polygons = vt.num_polygons;
    max_polygons = vt.max_polygons;
    memcpy (polygons, vt.polygons, sizeof (int)*num_polygons);
  }
  else
  {
    max_polygons = num_polygons = 0;
    polygons = 0;
  }
}

ltVertex::~ltVertex ()
{
  delete[] polygons;
}

void ltVertex::AddPolygon (int idx)
{
  if (num_polygons >= max_polygons)
  {
    max_polygons += 4;
    int* new_poly = new int [max_polygons];
    if (num_polygons > 0)
    {
      memcpy (new_poly, polygons, sizeof (int) * num_polygons);
    }
    delete[] polygons;
    polygons = new_poly;
  }
  polygons[num_polygons++] = idx;
}

//-----------------------------------------------------------------------------

ltPolygon::ltPolygon (iDocumentNode* polynode)
{
  name = 0;
  num_vertices = 0;
  max_vertices = 0;
  vertices = 0;
  ltPolygon::polynode = polynode;
}

ltPolygon::~ltPolygon ()
{
  delete[] name;
  delete[] vertices;
}

void ltPolygon::AddVertex (int idx)
{
  if (num_vertices >= max_vertices)
  {
    max_vertices += 4;
    int* new_verts = new int [max_vertices];
    if (num_vertices > 0)
    {
      memcpy (new_verts, vertices, sizeof (int) * num_vertices);
    }
    delete[] vertices;
    vertices = new_verts;
  }
  vertices[num_vertices++] = idx;
}

void ltPolygon::SetName (const char* name)
{
  delete[] ltPolygon::name;
  ltPolygon::name = name ? csStrNew (name) : 0;
}

void ltPolygon::RemoveDuplicateVertices ()
{
  int i;
  int i1 = num_vertices-1;
  for (i = 0 ; i < num_vertices ; )
  {
    if (vertices[i1] == vertices[i])
    {
      if (i < num_vertices-1)
        memmove (vertices+i, vertices+i+1, (num_vertices-i-1) * sizeof (int));
      num_vertices--;
    }
    else
    {
      i1 = i;
      i++;
    }
  }
}

//-----------------------------------------------------------------------------

ltThing::ltThing (iDocumentNode* meshnode, iDocumentNode* partnode)
{
  name = 0;
  vertices = 0;
  num_vertices = 0;
  max_vertices = 0;
  polygons = 0;
  num_polygons = 0;
  max_polygons = 0;
  ltThing::meshnode = meshnode;
  ltThing::partnode = partnode;
}

ltThing::~ltThing ()
{
  delete[] name;
  int i;
  for (i = 0 ; i < num_polygons ; i++)
  {
    delete polygons[i];
  }
  delete[] polygons;
  for (i = 0 ; i < num_vertices ; i++)
  {
    delete vertices[i];
  }
  delete[] vertices;
}

void ltThing::AddVertex (const csVector3& vt)
{
  if (num_vertices >= max_vertices)
  {
    if (max_vertices < 10000)
      max_vertices += max_vertices+2;
    else
      max_vertices += 10000;
    ltVertex** new_verts = new ltVertex* [max_vertices];
    if (num_vertices > 0)
    {
      memcpy (new_verts, vertices, sizeof (ltVertex*) * num_vertices);
    }
    delete[] vertices;
    vertices = new_verts;
  }
  vertices[num_vertices] = new ltVertex ();
  vertices[num_vertices++]->Set (vt);
}

ltPolygon* ltThing::AddPolygon (iDocumentNode* polynode)
{
  if (num_polygons >= max_polygons)
  {
    if (max_polygons < 1000)
      max_polygons += max_polygons+2;
    else
      max_polygons += 1000;
    ltPolygon** new_poly = new ltPolygon* [max_polygons];
    if (num_polygons > 0)
    {
      memcpy (new_poly, polygons, sizeof (ltPolygon*) * num_polygons);
    }
    delete[] polygons;
    polygons = new_poly;
  }
  ltPolygon* np = new ltPolygon (polynode);
  polygons[num_polygons++] = np;
  const char* name = polynode->GetAttributeValue ("name");
  if (name) np->SetName (name);
  return np;
}

void ltThing::SetName (const char* name)
{
  delete[] ltThing::name;
  ltThing::name = name ? csStrNew (name) : 0;
}

struct CompressVertex
{
  int orig_idx;
  float x, y, z;
  int new_idx;
  bool used;
};

static int compare_vt (const void *p1, const void *p2)
{
  CompressVertex *sp1 = (CompressVertex *)p1;
  CompressVertex *sp2 = (CompressVertex *)p2;
  if (sp1->x < sp2->x)
    return -1;
  else if (sp1->x > sp2->x)
    return 1;
  if (sp1->y < sp2->y)
    return -1;
  else if (sp1->y > sp2->y)
    return 1;
  if (sp1->z < sp2->z)
    return -1;
  else if (sp1->z > sp2->z)
    return 1;
  return 0;
}

static int compare_vt_orig (const void *p1, const void *p2)
{
  CompressVertex *sp1 = (CompressVertex *)p1;
  CompressVertex *sp2 = (CompressVertex *)p2;
  if (sp1->orig_idx < sp2->orig_idx)
    return -1;
  else if (sp1->orig_idx > sp2->orig_idx)
    return 1;
  return 0;
}

void ltThing::CompressVertices ()
{
  if (num_vertices <= 0) return ;

  // Copy all the vertices.
  CompressVertex *vt = new CompressVertex[num_vertices];
  int i, j;
  for (i = 0; i < num_vertices; i++)
  {
    vt[i].orig_idx = i;
    vt[i].x = (float)ceil (vertices[i]->x * 1000000);
    vt[i].y = (float)ceil (vertices[i]->y * 1000000);
    vt[i].z = (float)ceil (vertices[i]->z * 1000000);
  }

  // First sort so that all (nearly) equal vertices are together.
  qsort (vt, num_vertices, sizeof (CompressVertex), compare_vt);

  // Count unique values and tag all doubles with the index of the unique one.
  // new_idx in the vt table will be the index inside vt to the unique vector.
  int count_unique = 1;
  int last_unique = 0;
  vt[0].new_idx = last_unique;
  for (i = 1; i < num_vertices; i++)
  {
    if (
      vt[i].x != vt[last_unique].x ||
      vt[i].y != vt[last_unique].y ||
      vt[i].z != vt[last_unique].z)
    {
      last_unique = i;
      count_unique++;
    }

    vt[i].new_idx = last_unique;
  }

  // If count_unique == num_vertices then there is nothing to do.
  if (count_unique == num_vertices)
  {
    delete[] vt;
    return ;
  }

  // Now allocate and fill new vertex tables.
  // After this new_idx in the vt table will be the new index
  // of the vector.
  ltVertex **new_obj = new ltVertex*[count_unique];
  memset (new_obj, 0, sizeof (ltVertex*)*count_unique);
  new_obj[0] = new ltVertex (*vertices[vt[0].orig_idx]);

  vt[0].new_idx = 0;
  j = 1;
  for (i = 1; i < num_vertices; i++)
  {
    if (vt[i].new_idx == i)
    {
      new_obj[j] = new ltVertex (*vertices[vt[i].orig_idx]);
      vt[i].new_idx = j;
      j++;
    }
    else
      vt[i].new_idx = j - 1;
  }

  // Now we sort the table back on orig_idx so that we have
  // a mapping from the original indices to the new one (new_idx).
  qsort (vt, num_vertices, sizeof (CompressVertex), compare_vt_orig);

  // Replace the old vertex tables.
  for (i = 0 ; i < num_vertices ; i++)
  {
    delete vertices[i];
  }
  delete[] vertices;
  vertices = new_obj;
  num_vertices = max_vertices = count_unique;

  // Now we can remap the vertices in all polygons.
  for (i = 0 ; i < num_polygons ; i++)
  {
    ltPolygon *p = polygons[i];
    int *idx = p->GetVertexIndices ();
    for (j = 0 ; j < p->GetVertexCount () ; j++)
      idx[j] = vt[idx[j]].new_idx;
  }

  delete[] vt;
}

void ltThing::RemoveUnusedVertices ()
{
  if (num_vertices <= 0) return ;

  // Copy all the vertices that are actually used by polygons.
  bool *used = new bool[num_vertices];
  int i, j;
  for (i = 0; i < num_vertices; i++) used[i] = false;

  // Mark all vertices that are used as used.
  for (i = 0 ; i < num_polygons ; i++)
  {
    ltPolygon *p = polygons[i];
    int *idx = p->GetVertexIndices ();
    for (j = 0 ; j < p->GetVertexCount () ; j++) used[idx[j]] = true;
  }

  // Count relevant values.
  int count_relevant = 0;
  for (i = 0; i < num_vertices; i++)
  {
    if (used[i]) count_relevant++;
  }

  // If all vertices are relevant then there is nothing to do.
  if (count_relevant == num_vertices)
  {
    delete[] used;
    return ;
  }

  // Now allocate and fill new vertex tables.
  // Also fill the 'relocate' table.
  ltVertex **new_obj = new ltVertex*[count_relevant];
  memset (new_obj, 0, sizeof (ltVertex*)*count_relevant);
  int *relocate = new int[num_vertices];
  j = 0;
  for (i = 0; i < num_vertices; i++)
  {
    if (used[i])
    {
      new_obj[j] = new ltVertex (*vertices[i]);
      relocate[i] = j;
      j++;
    }
    else
      relocate[i] = -1;
  }

  // Replace the old vertex tables.
  for (i = 0 ; i < num_vertices ; i++)
  {
    delete vertices[i];
  }
  delete[] vertices;
  vertices = new_obj;
  num_vertices = max_vertices = count_relevant;

  // Now we can remap the vertices in all polygons.
  for (i = 0 ; i < num_polygons ; i++)
  {
    ltPolygon *p = polygons[i];
    int *idx = p->GetVertexIndices ();
    for (j = 0 ; j < p->GetVertexCount () ; j++) idx[j] = relocate[idx[j]];
  }

  delete[] relocate;
  delete[] used;
}

void ltThing::RemoveDuplicateVertices ()
{
  int i;
  for (i = 0 ; i < num_polygons ; i++)
  {
    ltPolygon* p = polygons[i];
    p->RemoveDuplicateVertices ();
  }
}

void ltThing::CreateBoundingBox ()
{
  if (num_vertices <= 0) return;
  int i;
  bbox.Set (*vertices[0], *vertices[0]);
  for (i = 1 ; i < num_vertices ; i++)
    bbox.AddBoundingVertexSmart (*vertices[i]);
}

void ltThing::CreateVertexInfo ()
{
  int i, j;
  for (i = 0 ; i < num_polygons ; i++)
  {
    ltPolygon* p = polygons[i];
    for (j = 0 ; j < p->GetVertexCount () ; j++)
    {
      ltVertex* v = vertices[p->GetVertex (j)];
      v->AddPolygon (i);
    }
  }
}

void ltThing::PaintConnectedPolygons (ltPolygon* sweep, int obj_number)
{
  if (sweep->GetObjectNumber () != -1)
  {
    CS_ASSERT (sweep->GetObjectNumber () == obj_number);
    return;
  }
  sweep->SetObjectNumber (obj_number);
  int i;
  for (i = 0 ; i < sweep->GetVertexCount () ; i++)
  {
    int vtidx = sweep->GetVertex (i);
    ltVertex& vt = GetVertex (vtidx);
    if (vt.GetObjectNumber () == -1)
    {
      vt.SetObjectNumber (obj_number);
    }
    CS_ASSERT (vt.GetObjectNumber () == obj_number);
    int j;
    for (j = 0 ; j < vt.GetPolygonCount () ; j++)
    {
      int ptidx = vt.GetPolygon (j);
      ltPolygon* p = GetPolygon (ptidx);
      PaintConnectedPolygons (p, obj_number);
    }
  }
}

void ltThing::SplitThingInCenter ()
{
  const csVector3& center = bbox.GetCenter ();
  max_obj_number = 7;
  int i, j;
  for (i = 0 ; i < num_polygons ; i++)
  {
    ltPolygon* p = polygons[i];
    int where = 0;
    for (j = 0 ; j < p->GetVertexCount () ; j++)
    {
      ltVertex* vt = vertices[p->GetVertex (j)];
      if (vt->x < center.x) where |= 1;
      if (vt->y < center.y) where |= 2;
      if (vt->z < center.z) where |= 4;
    }
    p->SetObjectNumber (where);
    for (j = 0 ; j < p->GetVertexCount () ; j++)
    {
      ltVertex* vt = vertices[p->GetVertex (j)];
      vt->SetObjectNumber (where);
    }
  }
}

void ltThing::SplitThingSeparateUnits ()
{
  max_obj_number = -1;
  int i;
  for (i = 0 ; i < num_polygons ; i++)
  {
    ltPolygon* p = polygons[i];
    p->SetObjectNumber (-1);
  }
  for (i = 0 ; i < num_vertices ; i++)
  {
    vertices[i]->SetObjectNumber (-1);
  }

  while (true)
  {
    ltPolygon* sweep = 0;
    for (i = 0 ; i < num_polygons ; i++)
    {
      ltPolygon* p = polygons[i];
      if (p->GetObjectNumber () == -1)
      {
        sweep = p;
        break;
      }
    }
    if (!sweep) break;
    max_obj_number++;
    PaintConnectedPolygons (sweep, max_obj_number);
  }
  csPrintf ("Found %d sub-objects\n", max_obj_number); fflush (stdout);
}

void ltThing::DoNotSplitThingSeparateUnits ()
{
  max_obj_number = 0;
  int i;
  for (i = 0 ; i < num_polygons ; i++)
  {
    ltPolygon* p = polygons[i];
    p->SetObjectNumber (0);
  }
  for (i = 0 ; i < num_vertices ; i++)
  {
    vertices[i]->SetObjectNumber (0);
  }
}

int* ltThing::CreateUnitMapping (int obj_number)
{
  int* map = new int [num_vertices];
  int new_i = 0;
  int i;
  for (i = 0 ; i < num_vertices ; i++)
  {
    ltVertex* vt = vertices[i];
    if (vt->GetObjectNumber () == obj_number)
    {
      map[i] = new_i;
      new_i++;
    }
    else
    {
      map[i] = -1;
    }
  }

  return map;
}

void ltThing::DuplicateSharedVertices ()
{
  int i;
  int total_vt = 0;
  for (i = 0 ; i < num_polygons ; i++)
  {
    total_vt += polygons[i]->GetVertexCount ();
  }
  ltVertex** new_vt = new ltVertex* [total_vt];
  int vt_idx = 0; 
  for (i = 0 ; i < num_polygons ; i++)
  {
    ltPolygon* p = polygons[i];
    int* idx = p->GetVertexIndices ();
    int j;
    for (j = 0 ; j < p->GetVertexCount () ; j++)
    {
      new_vt[vt_idx] = new ltVertex ();
      new_vt[vt_idx]->Set ((const csVector3&) *vertices[idx[j]]);
      idx[j] = vt_idx;
      vt_idx++;
    }
  }
  CS_ASSERT (total_vt == vt_idx);
  delete[] vertices;
  vertices = new_vt;
  num_vertices = max_vertices = total_vt;
}

//-----------------------------------------------------------------------------

bool LevTool::TestValidXML (iDocument* doc)
{
  csRef<iDocumentNode> root = doc->GetRoot ();
  csRef<iDocumentNode> worldnode = root->GetNode ("world");
  if (!worldnode)
  {
    ReportError ("The <world> node seems to be missing!");
    return false;
  }
  csRef<iDocumentNode> sector = worldnode->GetNode ("sector");
  if (!sector)
  {
    ReportError ("There appear to be no sectors in this world!");
    return false;
  }
  return true;
}

void LevTool::AnalyzePluginSection (iDocument* doc, bool meshfacts)
{
  thing_plugins.Push (new csString ("crystalspace.mesh.loader.thing"));
  if (meshfacts)
    thing_plugins.Push (
    	new csString ("crystalspace.mesh.loader.factory.thing"));
  plane_plugins.Push (new csString ("crystalspace.mesh.loader.thing.plane"));

  csRef<iDocumentNode> root = doc->GetRoot ();
  csRef<iDocumentNode> worldnode = root->GetNode ("world");
  csRef<iDocumentNode> pluginsnode = worldnode->GetNode ("plugins");
  if (pluginsnode)
  {
    csRef<iDocumentNodeIterator> it = pluginsnode->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      if (!strcmp (value, "plugin"))
      {
        const char* plugname = child->GetContentsValue ();
        if (plugname && !strcmp (plugname, "crystalspace.mesh.loader.thing"))
	{
	  thing_plugins.Push (new csString (child->GetAttributeValue ("name")));
	}
	else if (meshfacts && plugname &&
		!strcmp (plugname, "crystalspace.mesh.loader.factory.thing"))
	{
	  thing_plugins.Push (new csString (child->GetAttributeValue ("name")));
	}
	else if (plugname &&
		!strcmp (plugname, "crystalspace.mesh.loader.thing.plane"))
	{
	  plane_plugins.Push (new csString (child->GetAttributeValue ("name")));
	}
      }
    }
  }
}

bool LevTool::IsAddonAPlane (iDocumentNode* addonnode)
{
  csRef<iDocumentNode> pluginnode = addonnode->GetNode ("plugin");
  if (!pluginnode)
  {
    // Very weird. Should not happen.
    return false;
  }
  const char* plugname = pluginnode->GetContentsValue ();
  if (!plugname)
  {
    // Very weird. Should not happen.
    return false;
  }
  size_t i;
  for (i = 0 ; i < plane_plugins.Length () ; i++)
  {
    csString* str = plane_plugins.Get (i);
    if (str->Compare (plugname))
      return true;
  }
  return false;
}

bool LevTool::IsMeshAThing (iDocumentNode* meshnode)
{
  csRef<iDocumentNode> pluginnode = meshnode->GetNode ("plugin");
  if (!pluginnode)
  {
    // Very weird. Should not happen.
    return false;
  }
  const char* plugname = pluginnode->GetContentsValue ();
  if (!plugname)
  {
    // Very weird. Should not happen.
    return false;
  }
  size_t i;
  for (i = 0 ; i < thing_plugins.Length () ; i++)
  {
    csString* str = thing_plugins.Get (i);
    if (str->Compare (plugname))
      return true;
  }
  return false;
}

bool LevTool::IsMeshMovable (iDocumentNode* meshnode)
{
  csRef<iDocumentNode> paramsnode = meshnode->GetNode ("params");
  if (!paramsnode) return false;
  csRef<iDocumentNode> movnode = paramsnode->GetNode ("moveable");
  if (!movnode) return false;
  return true;
}

void LevTool::ParsePart (ltThing* thing, iDocumentNode* partnode,
	iDocumentNode* meshnode)
{
  csRef<iDocumentNodeIterator> it = partnode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    if (!strcmp (value, "v"))
    {
      csVector3 v;
      v.x = child->GetAttributeValueAsFloat ("x");
      v.y = child->GetAttributeValueAsFloat ("y");
      v.z = child->GetAttributeValueAsFloat ("z");
      thing->AddVertex (v);
    }
    else if (!strcmp (value, "p"))
    {
      ltPolygon* p = thing->AddPolygon (child);
      csRef<iDocumentNodeIterator> it2 = child->GetNodes ();
      while (it2->HasNext ())
      {
	csRef<iDocumentNode> child2 = it2->Next ();
	if (child2->GetType () != CS_NODE_ELEMENT) continue;
	const char* value2 = child2->GetValue ();
	if (!strcmp (value2, "v"))
	{
	  int vtidx = child2->GetContentsValueAsInt ();
	  p->AddVertex (vtidx);
	}
      }
    }
    else if (!strcmp (value, "part"))
    {
      ltThing* partthing = new ltThing (meshnode, child);
      const char* childname = child->GetAttributeValue ("name");
      if (childname)
        partthing->SetName (childname);
      else
      {
        csString newname (thing->GetName ());
	newname += "_part";
        partthing->SetName (newname);
      }
      things.Push (partthing);
      ParsePart (partthing, child, 0);
    }
  }
}

void LevTool::ParseThing (iDocumentNode* meshnode)
{
  csRef<iDocumentNode> paramsnode = meshnode->GetNode ("params");
  if (!paramsnode) return;	// Very weird!

  ltThing* th = new ltThing (meshnode, paramsnode);
  const char* name = meshnode->GetAttributeValue ("name");
  if (name) th->SetName (name);
  things.Push (th);
  ltDocNodeWrap* w = new ltDocNodeWrap ();
  w->node = meshnode;
  thing_nodes.Push (w);

  ParsePart (th, paramsnode, meshnode);
}

void LevTool::FindAllPlanes (iDocument* doc)
{
  csRef<iDocumentNode> root = doc->GetRoot ();
  csRef<iDocumentNode> worldnode = root->GetNode ("world");
  csRef<iDocumentNodeIterator> it = worldnode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    if (!strcmp (value, "addon") && IsAddonAPlane (child))
    {
      csRef<iDocumentNode> params = child->GetNode ("params");
      ltPlane* pl = new ltPlane;
      csRef<iDocumentNode> name = params->GetNode ("name");
      if (name)
      {
        const char* plname = name->GetContentsValue ();
        pl->name = csStrNew (plname);
      }
      csVector3 v;
      csRef<iDocumentNode> orig = params->GetNode ("orig");
      if (orig)
      {
        v.x = orig->GetAttributeValueAsFloat ("x");
        v.y = orig->GetAttributeValueAsFloat ("y");
        v.z = orig->GetAttributeValueAsFloat ("z");
        pl->orig = v;
      }
      csRef<iDocumentNode> first = params->GetNode ("first");
      if (first)
      {
        v.x = first->GetAttributeValueAsFloat ("x");
        v.y = first->GetAttributeValueAsFloat ("y");
        v.z = first->GetAttributeValueAsFloat ("z");
        pl->first = v;
      }
      csRef<iDocumentNode> second = params->GetNode ("second");
      if (second)
      {
        v.x = second->GetAttributeValueAsFloat ("x");
        v.y = second->GetAttributeValueAsFloat ("y");
        v.z = second->GetAttributeValueAsFloat ("z");
        pl->second = v;
      }
      csRef<iDocumentNode> firstlen = params->GetNode ("firstlen");
      if (firstlen)
	pl->firstlen = firstlen->GetContentsValueAsFloat ();
      csRef<iDocumentNode> secondlen = params->GetNode ("secondlen");
      if (secondlen)
	pl->secondlen = secondlen->GetContentsValueAsFloat ();
      planes.Push (pl);
    }
  }
}

void LevTool::FindAllThings (iDocument* doc, bool meshfacts, bool movable)
{
  csRef<iDocumentNode> root = doc->GetRoot ();
  csRef<iDocumentNode> worldnode = root->GetNode ("world");
  csRef<iDocumentNodeIterator> it = worldnode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    if (!strcmp (value, "sector"))
    {
      csRef<iDocumentNodeIterator> it2 = child->GetNodes ();
      while (it2->HasNext ())
      {
        csRef<iDocumentNode> child2 = it2->Next ();
        if (child2->GetType () != CS_NODE_ELEMENT) continue;
        const char* value2 = child2->GetValue ();
	if (!strcmp (value2, "meshobj") && IsMeshAThing (child2)
		&& (movable || !IsMeshMovable (child2)))
	{
	  ParseThing (child2);
	}
      }
    }
    else if (meshfacts && !strcmp (value, "meshfact") && IsMeshAThing (child))
    {
      ParseThing (child);
    }
  }
}

void LevTool::WriteOutPolygon (iDocumentNode* poly_node, ltPolygon* p,
	int* mapping)
{
  csRef<iDocumentNodeIterator> it = p->GetNode ()->GetNodes ();
  bool write_vertices = true;
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    const char* value = child->GetValue ();
    if (child->GetType () == CS_NODE_ELEMENT && !strcmp (value, "v"))
    {
      if (write_vertices)
      {
        write_vertices = false;
	int i;
	for (i = 0 ; i < p->GetVertexCount () ; i++)
	{
	  csRef<iDocumentNode> newchild = poly_node->CreateNodeBefore (
	    CS_NODE_ELEMENT);
	  newchild->SetValue ("v");
	  csRef<iDocumentNode> text = newchild->CreateNodeBefore (
	    CS_NODE_TEXT);
	  int newvtidx = mapping[p->GetVertex (i)];
	  CS_ASSERT (newvtidx != -1);
	  text->SetValueAsInt (newvtidx);
	}
      }
    }
    else
    {
      csRef<iDocumentNode> newchild = poly_node->CreateNodeBefore (
        CS_NODE_ELEMENT);
      CloneNode (child, newchild);
    }
  }
}

void LevTool::WriteOutThing (iDocumentNode* params_node, ltThing* th,
	int obj_number)
{
  int* mapping = th->CreateUnitMapping (obj_number);
  csRef<iDocumentNodeIterator> it = th->GetPartNode ()->GetNodes ();
  bool write_vertices = true;
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    const char* value = child->GetValue ();
    if (child->GetType () == CS_NODE_ELEMENT && !strcmp (value, "v"))
    {
      if (write_vertices)
      {
        write_vertices = false;
	int i;
	for (i = 0 ; i < th->GetVertexCount () ; i++)
	{
	  const ltVertex& vt = th->GetVertex (i);
	  if (vt.GetObjectNumber () == obj_number)
	  {
	    csRef<iDocumentNode> newchild = params_node->CreateNodeBefore (
	      CS_NODE_ELEMENT);
	    newchild->SetValue ("v");
	    newchild->SetAttributeAsFloat ("x", vt.x);
	    newchild->SetAttributeAsFloat ("y", vt.y);
	    newchild->SetAttributeAsFloat ("z", vt.z);
	  }
	}
      }
    }
    else if (child->GetType () == CS_NODE_ELEMENT && !strcmp (value, "p"))
    {
      int i;
      for (i = 0 ; i < th->GetPolygonCount () ; i++)
      {
        ltPolygon* p = th->GetPolygon (i);
	if (p->GetNode ()->Equals (child))
	{
	  if (p->GetObjectNumber () == obj_number)
	  {
	    csRef<iDocumentNode> newchild = params_node->CreateNodeBefore (
	      CS_NODE_ELEMENT);
	    newchild->SetValue ("p");
	    if (p->GetName ())
	      newchild->SetAttribute ("name", p->GetName ());
	    WriteOutPolygon (newchild, p, mapping);
	  }
	  break;
	}
      }
    }
    else if (child->GetType () == CS_NODE_ELEMENT && !strcmp (value, "part"))
    {
    }
    else if (child->GetType () == CS_NODE_ELEMENT && !strcmp (value,
    	"vistree"))
    {
    }
    else
    {
      csRef<iDocumentNode> newchild = params_node->CreateNodeBefore (
        CS_NODE_ELEMENT);
      CloneNode (child, newchild);
    }
  }
  delete[] mapping;
}

void LevTool::SplitThing (iDocumentNode* meshnode, iDocumentNode* parentnode)
{
  size_t i;
  for (i = 0 ; i < things.Length () ; i++)
  {
    ltThing* th = things.Get (i);
    if (th->GetMeshNode ()->Equals (meshnode))
    {
      if (th->GetPolygonCount () == 0) continue;
      int j;
      for (j = 0 ; j <= th->GetMaxObjectNumber () ; j++)
      {
        csRef<iDocumentNode> newmesh = parentnode->CreateNodeBefore (
          CS_NODE_ELEMENT);
        newmesh->SetValue (meshnode->GetValue ());
        csRef<iDocumentAttributeIterator> atit = meshnode->GetAttributes ();
        while (atit->HasNext ())
        {
	  csRef<iDocumentAttribute> attr = atit->Next ();
	  newmesh->SetAttribute (attr->GetName (), attr->GetValue ());
        }
	if (j == 0)
	{
          newmesh->SetAttribute ("name", th->GetName ());
	  csPrintf ("Processing '%s' ...\n", th->GetName ()); fflush (stdout);
	}
	else
	{
	  csString newname;
	  newname.Format ("%s_%d", th->GetName (), j);
          newmesh->SetAttribute ("name", newname);
	  csPrintf ("Processing '%s' ...\n", newname.GetData()); 
	  fflush (stdout);
	}

        csRef<iDocumentNodeIterator> it = meshnode->GetNodes ();
        while (it->HasNext ())
        {
          csRef<iDocumentNode> child = it->Next ();
          const char* value = child->GetValue ();
          if (child->GetType () == CS_NODE_ELEMENT &&
    	      (!strcmp (value, "params")))
          {
	    csRef<iDocumentNode> params_clone = newmesh->CreateNodeBefore (
	      child->GetType ());
	    params_clone->SetValue ("params");
	    WriteOutThing (params_clone, th, j);
	  }
          else if (child->GetType () == CS_NODE_ELEMENT &&
    	      (!strcmp (value, "zfill")))
	  {
	    csRef<iDocumentNode> child_clone = newmesh->CreateNodeBefore (
	      child->GetType ());
	    CloneNode (child, child_clone);
	    child_clone->SetValue ("zuse");
	  }
	  else
	  {
	    csRef<iDocumentNode> child_clone = newmesh->CreateNodeBefore (
	      child->GetType ());
	    CloneNode (child, child_clone);
	  }
        }
      }
    }
  }
}

void LevTool::CloneAndSplitDynavis (iDocumentNode* node, iDocumentNode* newnode,
	bool is_dynavis)
{
  const char* parentvalue = node->GetValue ();

  bool is_world = !strcmp (parentvalue, "world");
  bool is_sector = !strcmp (parentvalue, "sector");
  bool is_root = !strcmp (parentvalue, "");
  if (!is_root && !is_sector && !is_world)
  {
    CloneNode (node, newnode);
    return;
  }

  // First copy the world or sector name and attributes.
  newnode->SetValue (node->GetValue ());
  csRef<iDocumentAttributeIterator> atit = node->GetAttributes ();
  while (atit->HasNext ())
  {
    csRef<iDocumentAttribute> attr = atit->Next ();
    newnode->SetAttribute (attr->GetName (), attr->GetValue ());
  }

  csRef<iDocumentNode> settingsnode;

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    const char* value = child->GetValue ();
    if (is_sector && child->GetType () == CS_NODE_ELEMENT &&
    	(!strcmp (value, "meshobj")) && IsMeshAThing (child) &&
		!IsMeshMovable (child))
    {
      SplitThing (child, newnode);
    }
    else if (is_dynavis && is_sector && child->GetType () == CS_NODE_ELEMENT &&
    	(!strcmp (value, "culler")))
    {
      csRef<iDocumentNode> newchild = newnode->CreateNodeBefore (
      	child->GetType ());
      newchild->SetValue ("cullerp");
      csRef<iDocumentNode> text = newchild->CreateNodeBefore (
      	CS_NODE_TEXT);
      text->SetValue ("crystalspace.culling.dynavis");
    }
    else if (is_root && !strcmp (value, "world"))
    {
      csRef<iDocumentNode> newchild = newnode->CreateNodeBefore (
      	child->GetType ());
      CloneAndSplitDynavis (child, newchild, is_dynavis);
    }
    else if (is_world && !strcmp (value, "sector"))
    {
      csRef<iDocumentNode> newchild = newnode->CreateNodeBefore (
      	child->GetType ());
      CloneAndSplitDynavis (child, newchild, is_dynavis);
    }
    else
    {
      csRef<iDocumentNode> newchild = newnode->CreateNodeBefore (
      	child->GetType ());
      CloneNode (child, newchild);
      if (is_world && !strcmp (value, "settings"))
        settingsnode = newchild;
    }
  }
  if (is_dynavis && is_world)
  {
    if (settingsnode == 0)
    {
      settingsnode = newnode->CreateNodeBefore (CS_NODE_ELEMENT);
      settingsnode->SetValue ("settings");
    }
    bool found = false;
    csRef<iDocumentNodeIterator> it = settingsnode->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      const char* value = child->GetValue ();
      if (child->GetType () == CS_NODE_ELEMENT && !strcmp (value, "clearzbuf"))
      {
        found = true;
      }
    }
    if (!found)
    {
      csRef<iDocumentNode> newchild = settingsnode->CreateNodeBefore (
      	CS_NODE_ELEMENT);
      newchild->SetValue ("clearzbuf");
      csRef<iDocumentNode> text = newchild->CreateNodeBefore (
      	CS_NODE_TEXT);
      text->SetValue ("yes");
    }
  }
}

void LevTool::CloneAndSplitDynavis (iDocument* doc, iDocument* newdoc,
	bool is_dynavis)
{
  csRef<iDocumentNode> root = doc->GetRoot ();
  csRef<iDocumentNode> newroot = newdoc->CreateRoot ();
  CloneAndSplitDynavis (root, newroot, is_dynavis);
}

void LevTool::CloneAndMovePlanes (iDocumentNode* node, iDocumentNode* newnode)
{
  const char* parentvalue = node->GetValue ();

  bool is_addon = !strcmp (parentvalue, "addon");
  if (is_addon && IsAddonAPlane (node)) return;	// Discard.

  bool is_meshfact = !strcmp (parentvalue, "meshfact");
  if (is_meshfact && !IsMeshAThing (node))
  {
    CloneNode (node, newnode);
    return;
  }
  bool is_meshobj = !strcmp (parentvalue, "meshobj");
  if (is_meshobj && !IsMeshAThing (node))
  {
    CloneNode (node, newnode);
    return;
  }

  bool is_world = !strcmp (parentvalue, "world");
  bool is_sector = !strcmp (parentvalue, "sector");
  bool is_plugins = !strcmp (parentvalue, "plugins");
  bool is_p = !strcmp (parentvalue, "p");
  bool is_params = !strcmp (parentvalue, "params");
  bool is_part = !strcmp (parentvalue, "part");
  bool is_texmap = !strcmp (parentvalue, "texmap");
  bool is_root = !strcmp (parentvalue, "");
  if (!is_root && !is_sector && !is_world && !is_meshfact && !is_meshobj
  	&& !is_p && !is_params && !is_part && !is_texmap && !is_plugins)
  {
    CloneNode (node, newnode);
    return;
  }

  // First copy the name and attributes.
  newnode->SetValue (node->GetValue ());
  csRef<iDocumentAttributeIterator> atit = node->GetAttributes ();
  while (atit->HasNext ())
  {
    csRef<iDocumentAttribute> attr = atit->Next ();
    newnode->SetAttribute (attr->GetName (), attr->GetValue ());
  }

  if (is_texmap)
  {
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      const char* value = child->GetValue ();
      if (child->GetType () == CS_NODE_ELEMENT && (!strcmp (value, "plane")))
      {
        const char* planename = child->GetContentsValue ();
	size_t i;
	for (i = 0 ; i < planes.Length () ; i++)
	{
	  ltPlane* pl = planes[i];
	  if (strcmp (pl->name, planename) == 0)
	  {
            csRef<iDocumentNode> orig = newnode->CreateNodeBefore (
      	      child->GetType ());
	    orig->SetValue ("orig");
	    orig->SetAttributeAsFloat ("x", pl->orig.x);
	    orig->SetAttributeAsFloat ("y", pl->orig.y);
	    orig->SetAttributeAsFloat ("z", pl->orig.z);
            csRef<iDocumentNode> first = newnode->CreateNodeBefore (
      	      child->GetType ());
	    first->SetValue ("first");
	    first->SetAttributeAsFloat ("x", pl->first.x);
	    first->SetAttributeAsFloat ("y", pl->first.y);
	    first->SetAttributeAsFloat ("z", pl->first.z);

            csRef<iDocumentNode> firstlen = newnode->CreateNodeBefore (
      	      child->GetType ());
	    firstlen->SetValue ("firstlen");
	    csRef<iDocumentNode> firstlen_text = firstlen->CreateNodeBefore (
	      CS_NODE_TEXT);
	    firstlen_text->SetValueAsFloat (pl->firstlen);

            csRef<iDocumentNode> second = newnode->CreateNodeBefore (
      	      child->GetType ());
	    second->SetValue ("second");
	    second->SetAttributeAsFloat ("x", pl->second.x);
	    second->SetAttributeAsFloat ("y", pl->second.y);
	    second->SetAttributeAsFloat ("z", pl->second.z);

            csRef<iDocumentNode> secondlen = newnode->CreateNodeBefore (
      	      child->GetType ());
	    secondlen->SetValue ("secondlen");
	    csRef<iDocumentNode> secondlen_text = secondlen->CreateNodeBefore (
	      CS_NODE_TEXT);
	    secondlen_text->SetValueAsFloat (pl->secondlen);
	    break;
	  }
	}
      }
      else
      {
        csRef<iDocumentNode> newchild = newnode->CreateNodeBefore (
      	  child->GetType ());
        CloneNode (child, newchild);
      }
    }
  }
  else if (is_plugins)
  {
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      const char* value = child->GetValue ();
      if (child->GetType () == CS_NODE_ELEMENT && !strcmp (value, "plugin"))
      {
        const char* plugname = child->GetContentsValue ();
	if (!strcmp (plugname, "crystalspace.mesh.loader.thing.plane"))
	{
	  // Drop.
	  continue;
	}
      }
      csRef<iDocumentNode> newchild = newnode->CreateNodeBefore (
      	  child->GetType ());
      CloneNode (child, newchild);
    }
  }
  else
  {
    csRef<iDocumentNodeIterator> it = node->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      const char* value = child->GetValue ();
      if (strcmp (value, "addon") == 0 && IsAddonAPlane (child))
      {
        continue;	// Skip.
      }
      csRef<iDocumentNode> newchild = newnode->CreateNodeBefore (
      	  child->GetType ());
      CloneAndMovePlanes (child, newchild);
    }
  }
}

void LevTool::CloneAndMovePlanes (iDocument* doc, iDocument* newdoc)
{
  csRef<iDocumentNode> root = doc->GetRoot ();
  csRef<iDocumentNode> newroot = newdoc->CreateRoot ();
  CloneAndMovePlanes (root, newroot);
}

static int cnt;

void LevTool::CloneAndChangeFlags (iDocumentNode* node, iDocumentNode* newnode,
  	int op, int minsize, int maxsize, int minpoly, int maxpoly,
	float global_area)
{
  const char* parentvalue = node->GetValue ();

  bool is_world = !strcmp (parentvalue, "world");
  bool is_sector = !strcmp (parentvalue, "sector");
  bool is_meshobj = !strcmp (parentvalue, "meshobj");
  bool is_root = !strcmp (parentvalue, "");
  if (!is_root && !is_sector && !is_world && !is_meshobj)
  {
    CloneNode (node, newnode);
    return;
  }

  // First copy the world or sector name and attributes.
  newnode->SetValue (node->GetValue ());
  csRef<iDocumentAttributeIterator> atit = node->GetAttributes ();
  while (atit->HasNext ())
  {
    csRef<iDocumentAttribute> attr = atit->Next ();
    newnode->SetAttribute (attr->GetName (), attr->GetValue ());
  }

  bool is_thing = is_meshobj && IsMeshAThing (node);

  csRef<iDocumentNodeIterator> it = node->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    const char* value = child->GetValue ();
    if (op != OP_FLAGBAD && is_thing && child->GetType () == CS_NODE_ELEMENT &&
    	!strcmp (value, "goodoccluder"))
    {
    }
    else if (op != OP_FLAGGOOD && is_thing
    	&& child->GetType () == CS_NODE_ELEMENT &&
    	!strcmp (value, "badoccluder"))
    {
    }
    else if (child->GetType () == CS_NODE_ELEMENT &&
    	    ((is_sector && !strcmp (value, "meshobj")) ||
             (is_root && !strcmp (value, "world")) ||
             (is_world && !strcmp (value, "sector"))))
    {
      csRef<iDocumentNode> newchild = newnode->CreateNodeBefore (
      	child->GetType ());
      CloneAndChangeFlags (child, newchild, op, minsize, maxsize,
      	minpoly, maxpoly, global_area);
    }
    else
    {
      csRef<iDocumentNode> newchild = newnode->CreateNodeBefore (
      	child->GetType ());
      CloneNode (child, newchild);
    }
  }
  if (is_thing)
  {
    ltThing* th = 0;
    size_t i;
    for (i = 0 ; i < things.Length () ; i++)
    {
      th = things.Get (i);
      if (th->GetMeshNode ()->Equals (node)) break;
    }

    if (th->GetPolygonCount () < minpoly || th->GetPolygonCount () > maxpoly)
      return;

    const csBox3& b = th->GetBoundingBox ();
    float area = (b.MaxX () - b.MinX ()) *
		     (b.MaxY () - b.MinY ()) *
		     (b.MaxZ () - b.MinZ ());
    area = pow (area, 1.0f/3.0f);
    int where = int (100 * (area / global_area));
    if (where < minsize || where > maxsize)
      return;

    if (op == OP_FLAGGOOD)
    {
      csRef<iDocumentNode> newchild = newnode->CreateNodeBefore (
      	  CS_NODE_ELEMENT);
      newchild->SetValue ("goodoccluder");
      cnt++;
    }
    else if (op == OP_FLAGBAD)
    {
      csRef<iDocumentNode> newchild = newnode->CreateNodeBefore (
      	  CS_NODE_ELEMENT);
      newchild->SetValue ("badoccluder");
      cnt++;
    }
  }
}

void LevTool::CloneAndChangeFlags (iDocument* doc, iDocument* newdoc,
  	int op, int minsize, int maxsize, int minpoly, int maxpoly,
	float global_area)
{
  csRef<iDocumentNode> root = doc->GetRoot ();
  csRef<iDocumentNode> newroot = newdoc->CreateRoot ();
  CloneAndChangeFlags (root, newroot, op, minsize, maxsize, minpoly, maxpoly,
  	global_area);
}

void LevTool::CloneNode (iDocumentNode* from, iDocumentNode* to)
{
  to->SetValue (from->GetValue ());
  csRef<iDocumentNodeIterator> it = from->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
    	child->GetType (), 0);
    CloneNode (child, child_clone);
  }
  csRef<iDocumentAttributeIterator> atit = from->GetAttributes ();
  while (atit->HasNext ())
  {
    csRef<iDocumentAttribute> attr = atit->Next ();
    to->SetAttribute (attr->GetName (), attr->GetValue ());
  }
}

//-----------------------------------------------------------------------------

void LevTool::ListMeshPart (iDocumentNode* meshpart, int level)
{
  char indent[1024];
  int i; for (i = 0 ; i < level ; i++) indent[i] = ' '; indent[level] = 0;
  const char* name = meshpart->GetAttributeValue ("name");
  csPrintf ("%spart(%s)\n", indent, name);
}

void LevTool::ListMeshObject (iDocumentNode* mesh, int level)
{
  char indent[1024];
  int i; for (i = 0 ; i < level ; i++) indent[i] = ' '; indent[level] = 0;
  const char* name = mesh->GetAttributeValue ("name");
  csRef<iDocumentNode> pluginnode = mesh->GetNode ("plugin");
  const char* plugin = pluginnode->GetContentsValue ();
  csPrintf ("%smeshobj(%s,%s)\n", indent, name, plugin);

  if (IsMeshAThing (mesh))
  {
    csRef<iDocumentNode> paramsnode = mesh->GetNode ("params");
    if (!paramsnode) return;
    csRef<iDocumentNodeIterator> it = paramsnode->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      if (!strcmp (value, "part"))
      {
        ListMeshPart (child, level+1);
      }
    }
  }
}

void LevTool::ListFactory (iDocumentNode* factory, int level)
{
  char indent[1024];
  int i; for (i = 0 ; i < level ; i++) indent[i] = ' '; indent[level] = 0;
  const char* name = factory->GetAttributeValue ("name");
  csRef<iDocumentNode> pluginnode = factory->GetNode ("plugin");
  const char* plugin = pluginnode->GetContentsValue ();
  csPrintf ("%smeshfact(%s,%s)\n", indent, name, plugin);

  if (IsMeshAThing (factory))
  {
    csRef<iDocumentNode> paramsnode = factory->GetNode ("params");
    if (!paramsnode) return;
    csRef<iDocumentNodeIterator> it = paramsnode->GetNodes ();
    while (it->HasNext ())
    {
      csRef<iDocumentNode> child = it->Next ();
      if (child->GetType () != CS_NODE_ELEMENT) continue;
      const char* value = child->GetValue ();
      if (!strcmp (value, "part"))
      {
        ListMeshPart (child, level+1);
      }
    }
  }
}

void LevTool::ListSector (iDocumentNode* sector, int level)
{
  char indent[1024];
  int i; for (i = 0 ; i < level ; i++) indent[i] = ' '; indent[level] = 0;
  const char* name = sector->GetAttributeValue ("name");
  csPrintf ("%ssector(%s)\n", indent, name);
  csRef<iDocumentNodeIterator> it = sector->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    if (!strcmp (value, "meshobj"))
    {
      ListMeshObject (child, level+1);
    }
  }
}

void LevTool::ListContents (iDocumentNode* world)
{
  csPrintf ("world\n");
  csRef<iDocumentNode> worldnode = world->GetNode ("world");
  csRef<iDocumentNodeIterator> it = worldnode->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    if (child->GetType () != CS_NODE_ELEMENT) continue;
    const char* value = child->GetValue ();
    if (!strcmp (value, "sector"))
    {
      ListSector (child, 1);
    }
    else if (!strcmp (value, "meshfact"))
    {
      ListFactory (child, 1);
    }
  }
  fflush (stdout);
}

//-----------------------------------------------------------------------------

void LevTool::ValidateContents (ltThing* thing)
{
  int i, j;
  for (i = 0 ; i < thing->GetPolygonCount () ; i++)
  {
    ltPolygon* pol = thing->GetPolygon (i);
    if (pol->GetVertexCount () > 3)
    {
      csPlane3 plane (
	thing->GetVertex (pol->GetVertex (0)),
	thing->GetVertex (pol->GetVertex (1)),
	thing->GetVertex (pol->GetVertex (2)));
      for (j = 3 ; j < pol->GetVertexCount () ; j++)
      {
	float d = plane.Distance (thing->GetVertex (pol->GetVertex (j)));
	if (d > 0.01)
	{
	  csPrintf ("WARNING! polygon '%s' in thing/part '%s' seems to be non-coplanar with distance %g\n",
			  pol->GetName (), thing->GetName (), d);
	}
      }
    }
  }
}

void LevTool::ValidateContents ()
{
  size_t i;
  for (i = 0 ; i < things.Length () ; i++)
  {
    ltThing* th = things.Get (i);
    ValidateContents (th);
  }
}

//-----------------------------------------------------------------------------

LevTool::LevTool ()
{
  object_reg = 0;
}

LevTool::~LevTool ()
{
}

//----------------------------------------------------------------------------

// @@@ TODO: support hardmove!

void LevTool::Main ()
{
  cmdline = CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);
  csRef<iPluginManager> plugin_mgr = 
    CS_QUERY_REGISTRY (object_reg, iPluginManager);

  int op = OP_HELP;

  if (cmdline->GetOption ("inds") || cmdline->GetOption ("outds"))
    op = OP_TRANSLATE;

  if (cmdline->GetOption ("dynavis")) op = OP_DYNAVIS;
  if (cmdline->GetOption ("list")) op = OP_LIST;
  if (cmdline->GetOption ("analyze")) op = OP_ANALYZE;
  if (cmdline->GetOption ("validate")) op = OP_VALIDATE;
  if (cmdline->GetOption ("splitunit")) op = OP_SPLITUNIT;
  if (cmdline->GetOption ("splitgeom")) op = OP_SPLITGEOM;
  if (cmdline->GetOption ("compress")) op = OP_COMPRESS;
  if (cmdline->GetOption ("flagclear")) op = OP_FLAGCLEAR;
  if (cmdline->GetOption ("flaggood")) op = OP_FLAGGOOD;
  if (cmdline->GetOption ("flagbad")) op = OP_FLAGBAD;
  if (cmdline->GetOption ("planes")) op = OP_PLANES;
  if (cmdline->GetOption ("help")) op = OP_HELP;

  if (op == OP_HELP)
  {
    csPrintf ("levtool <options> <zipfile>\n");
    csPrintf ("  -list:\n");      
    csPrintf ("     List world contents.\n");
    csPrintf ("  -analyze:\n");         
    csPrintf ("     Analyze all things and show the distribution of size.\n");
    csPrintf ("              You can use this to decide on the 'minsize' parameter.\n");
    csPrintf ("  -validate:\n");        
    csPrintf ("     Validate world contents. This will currently check\n");
    csPrintf ("              only for non-coplanar polygons.\n");
    csPrintf ("  -compress:\n");        
    csPrintf ("     Compress all vertices and remove duplicate and unused vertices.\n");
    csPrintf ("     Unless you specify -nocompress, -dynavis does this by default.\n");
    csPrintf ("  -dynavis:\n");         
    csPrintf ("     Convert to Dynavis. This takes a level that uses\n");
    csPrintf ("     old style octree culler and convert it to Dynavis.\n");
    csPrintf ("     All objects will be split in their logical parts.\n");
    csPrintf ("  -splitunit:\n");       
    csPrintf ("     Split large things into separate units. A separate unit\n");
    csPrintf ("     is defined as a collection of connected polygons that doesn't\n");
    csPrintf ("     connect with any other polygon from the thing.\n");
    csPrintf ("     This option will first do -compress.\n");
    csPrintf ("     -minsize=<percent>: Only split objects larger than the given\n");
    csPrintf ("     percentage. Default 20%%.\n");
    csPrintf ("  -splitgeom:\n");       
    csPrintf ("     Split large things geometrically. After doing this you\n");
    csPrintf ("     should run levtool with -compress option for efficiency.\n");
    csPrintf ("     -minsize=<percent>: Only split objects larger than the given\n");
    csPrintf ("     percentage. Default 20%%.\n");
    csPrintf ("  -flagclear:\n");       
    csPrintf ("     Clear all goodoccluder/badoccluder flags.\n");
    csPrintf ("  -flaggood:\n");        
    csPrintf ("     Add goodoccluder flag for all things that satisfy\n");
    csPrintf ("     conditions defined by the following flags:\n");
    csPrintf ("     -minsize=<percent>: min object size (default 10%%).\n");
    csPrintf ("     -maxsize=<percent>: max object size (default 100%%).\n");
    csPrintf ("     -minpoly=<number>: min number of polygons (default 1).\n");
    csPrintf ("     -maxpoly=<number>: max number of polygons (default 50).\n");
    csPrintf ("  -flagbad:\n");         
    csPrintf ("     Add badoccluder flag for all things that satisfy\n");
    csPrintf ("     conditions defined by the following flags:\n");
    csPrintf ("     -minsize=<percent>: mim object size (default 0%%).\n");
    csPrintf ("     -maxsize=<percent>: max object size (default 3%%).\n");
    csPrintf ("     -minpoly=<number>: min number of polygons (default 6).\n");
    csPrintf ("     -maxpoly=<number>: max number of polygons (default 1000000000).\n");
    csPrintf ("  -planes:\n");          
    csPrintf ("     Move all 'addon' planes to the polygons that use them.\n");
    csPrintf ("  -inds=<plugin>:\n");       
    csPrintf ("     Document system plugin for reading world.\n");
    csPrintf ("  -outds=<plugin>:\n");       
    csPrintf ("     Document system plugin for writing world.\n");
    return;
  }

  int minsize, maxsize;
  int minpoly, maxpoly;
  switch (op)
  {      
    case OP_FLAGGOOD:
      minsize = 10;
      maxsize = 120;
      minpoly = 1;
      maxpoly = 50;
      break;
    case OP_FLAGBAD:
      minsize = 0;
      maxsize = 3;
      minpoly = 6;
      maxpoly = 1000000000;
      break;
    default:
      minsize = 20;
      maxsize = 120;
      minpoly = 0;
      maxpoly = 1000000000;
      break;
  }

  {
    const char* minsize_arg = cmdline->GetOption ("minsize");
    if (minsize_arg != 0) sscanf (minsize_arg, "%d", &minsize);
    const char* maxsize_arg = cmdline->GetOption ("maxsize");
    if (maxsize_arg != 0) sscanf (maxsize_arg, "%d", &maxsize);
    const char* minpoly_arg = cmdline->GetOption ("minpoly");
    if (minpoly_arg != 0) sscanf (minpoly_arg, "%d", &minpoly);
    const char* maxpoly_arg = cmdline->GetOption ("maxpoly");
    if (maxpoly_arg != 0) sscanf (maxpoly_arg, "%d", &maxpoly);
  }

  csRef<iDocumentSystem> inputDS;
  csRef<iDocumentSystem> outputDS;
  {
    const char *inds = cmdline->GetOption ("inds");
    if (inds)
    {
/*      inputDS = csPtr<iDocumentSystem> (CS_LOAD_PLUGIN (plugin_mgr,
	inds, iDocumentSystem));
      if (!inputDS)*/
      {
        inputDS = csPtr<iDocumentSystem> (CS_LOAD_PLUGIN (plugin_mgr,
	  csString().Format ("crystalspace.documentsystem.%s", inds),
	  iDocumentSystem));
      }
      if (!inputDS)
      {
	Report (CS_REPORTER_SEVERITY_ERROR,
	  "Unable to load input document system '%s'!",
	  inds);
        return;
      }
    }

    const char *outds = cmdline->GetOption ("outds");
    if (outds)
    {
/*      outputDS = csPtr<iDocumentSystem> (CS_LOAD_PLUGIN (plugin_mgr,
	outds, iDocumentSystem));
      if (!outputDS)*/
      {
	outputDS = csPtr<iDocumentSystem> (CS_LOAD_PLUGIN (plugin_mgr,
	  csString().Format ("crystalspace.documentsystem.%s", outds),
	  iDocumentSystem));
      }
      if (!outputDS)
      {
	Report (CS_REPORTER_SEVERITY_ERROR,
	  "Unable to load output document system '%s'!",
	  outds);
        return;
      }
    }
  }

  const char* val = cmdline->GetName ();
  if (!val)
  {
    ReportError ("Please give VFS world file name or name of the zip archive! "
      "Use 'levtool -help' to get a list of possible options.");
    return;
  }

  csString filename;
  if (strstr (val, ".zip"))
  {
    vfs->Mount ("/tmp/levtool_data", val);
    filename = "/tmp/levtool_data/world";
  }
  else
  {
    filename = val;
  }

  csRef<iDataBuffer> buf = vfs->ReadFile (filename);
  if (!buf || !buf->GetSize ())
  {
    ReportError ("File '%s' does not exist!", (const char*)filename);
    return;
  }

  // Make backup.
  switch (op)
  {
    case OP_LIST:  
    case OP_ANALYZE: 
    case OP_VALIDATE: 
      break;
    default:
      vfs->WriteFile (filename+".bak", **buf, buf->GetSize ());
      break;
  }

  csRef<iDocumentSystem> xml;
  if (inputDS)
  {
    xml = inputDS;
  }
  else
  {
    xml = (csPtr<iDocumentSystem> (
      new csTinyDocumentSystem ()));
  }
  csRef<iDocument> doc = xml->CreateDocument ();
  Report (CS_REPORTER_SEVERITY_NOTIFY, "Parsing...");
  csTicks parse_start = csGetTicks();
  const char* error = doc->Parse (buf, true);
  csTicks parse_end = csGetTicks();
  buf = 0;
  if (error != 0)
  {
    ReportError ("Error parsing XML: %s!", error);
    return;
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_NOTIFY, " time taken: %f s",
      (float)(parse_end - parse_start) / (float)1000);
  }

  //---------------------------------------------------------------

  if (op != OP_TRANSLATE)
    if (!TestValidXML (doc))
      return;

  csRef<iDocumentSystem> newsys;
  if (outputDS)
  {
    newsys = outputDS;
  }
  else
  {
    newsys = (csPtr<iDocumentSystem> (
      new csTinyDocumentSystem ()));
  }

  csRef<iDocumentNode> root = doc->GetRoot ();
  csRef<iDocumentNode> worldnode = root->GetNode ("world");

  switch (op)
  {
    case OP_VALIDATE:
      {
	Report (CS_REPORTER_SEVERITY_NOTIFY, "Analyzing...");
	csTicks analyze_start = csGetTicks();
	AnalyzePluginSection (doc);
	FindAllThings (doc);
	csTicks analyze_end = csGetTicks();
	Report (CS_REPORTER_SEVERITY_NOTIFY, " time taken: %f s",
	  (float)(analyze_end - analyze_start) / (float)1000);

	Report (CS_REPORTER_SEVERITY_NOTIFY, "Validating...");
	csTicks validate_start = csGetTicks();
	ValidateContents ();
	csTicks validate_end = csGetTicks();
	Report (CS_REPORTER_SEVERITY_NOTIFY, " time taken: %f s",
	  (float)(validate_end - validate_start) / (float)1000);
      }
      break;

    case OP_LIST:
      {
	Report (CS_REPORTER_SEVERITY_NOTIFY, "Analyzing...");
	csTicks analyze_start = csGetTicks();
	AnalyzePluginSection (doc);
	csTicks analyze_end = csGetTicks();
	Report (CS_REPORTER_SEVERITY_NOTIFY, " time taken: %f s",
	  (float)(analyze_end - analyze_start) / (float)1000);
        ListContents (doc->GetRoot ());
      }
      break;

    case OP_ANALYZE:
      {
	Report (CS_REPORTER_SEVERITY_NOTIFY, "Analyzing...");
	csTicks analyze_start = csGetTicks();
	AnalyzePluginSection (doc);
	FindAllThings (doc);
	csBox3 global_bbox;
	global_bbox.StartBoundingBox ();
	size_t i;
	for (i = 0 ; i < things.Length () ; i++)
	{
	  ltThing* th = things.Get (i);
	  th->CreateBoundingBox ();
	  global_bbox += th->GetBoundingBox ();
	}
	csTicks analyze_end = csGetTicks();
	Report (CS_REPORTER_SEVERITY_NOTIFY, " time taken: %f s",
	  (float)(analyze_end - analyze_start) / (float)1000);
	csPrintf ("Global bounding box: (%g,%g,%g) - (%g,%g,%g)\n",
		global_bbox.MinX (), global_bbox.MinY (), global_bbox.MinZ (),
		global_bbox.MaxX (), global_bbox.MaxY (), global_bbox.MaxZ ());
	float global_area = (global_bbox.MaxX () - global_bbox.MinX ()) *
		     (global_bbox.MaxY () - global_bbox.MinY ()) *
		     (global_bbox.MaxZ () - global_bbox.MinZ ());
	global_area = pow (global_area, 1.0f/3.0f);
        int counts[51];
	for (i = 0 ; i <= 50 ; i++)
	  counts[i] = 0;
	for (i = 0 ; i < things.Length () ; i++)
	{
	  ltThing* th = things.Get (i);
	  const csBox3& b = th->GetBoundingBox ();
	  float area = (b.MaxX () - b.MinX ()) *
		     (b.MaxY () - b.MinY ()) *
		     (b.MaxZ () - b.MinZ ());
	  area = pow (area, 1.0f/3.0f);
	  int where = int (50 * (area / global_area));
	  counts[where]++;
	}
	int sum = 0;
	for (i = 0 ; i <= 50 ; i++)
	{
	  sum += counts[i];
	  csPrintf ("  %3zu%% #local=%-5d #<=thissize=%d\n",
		  (i * 2), counts[i], sum);
	}
	fflush (stdout);
      }
      break;

    case OP_PLANES:
      {
	AnalyzePluginSection (doc, true);
	FindAllThings (doc, true, true);
	FindAllPlanes (doc);
	csRef<iDocument> newdoc = newsys->CreateDocument ();
	CloneAndMovePlanes (doc, newdoc);
        error = newdoc->Write (vfs, filename);
	if (error != 0)
	{
	  ReportError ("Error writing '%s': %s!", (const char*)filename, error);
	  return;
	}
      }
      break;

    case OP_DYNAVIS:
      {
	AnalyzePluginSection (doc);
	FindAllThings (doc);
	csRef<iDocument> newdoc = newsys->CreateDocument ();

	size_t i;
	for (i = 0 ; i < things.Length () ; i++)
	{
	  ltThing* th = things.Get (i);
	  th->CreateVertexInfo ();
	  th->DoNotSplitThingSeparateUnits ();
	}

	CloneAndSplitDynavis (doc, newdoc, true);
        error = newdoc->Write (vfs, filename);
	if (error != 0)
	{
	  ReportError ("Error writing '%s': %s!", (const char*)filename, error);
	  return;
	}
      }
      break;

    case OP_COMPRESS:
      {
	AnalyzePluginSection (doc);
	FindAllThings (doc);
	csRef<iDocument> newdoc = newsys->CreateDocument ();

	size_t i;
	for (i = 0 ; i < things.Length () ; i++)
	{
	  ltThing* th = things.Get (i);
	  th->CompressVertices ();
	  th->RemoveUnusedVertices ();
	  th->RemoveDuplicateVertices ();
	  th->CreateVertexInfo ();
	  th->DoNotSplitThingSeparateUnits ();
	}

	CloneAndSplitDynavis (doc, newdoc, false);
        error = newdoc->Write (vfs, filename);
	if (error != 0)
	{
	  ReportError ("Error writing '%s': %s!", (const char*)filename, error);
	  return;
	}
      }
      break;

    case OP_SPLITUNIT:
      {
	AnalyzePluginSection (doc);
	FindAllThings (doc);
	csRef<iDocument> newdoc = newsys->CreateDocument ();

	csBox3 global_bbox;
	global_bbox.StartBoundingBox ();
	size_t i;
	for (i = 0 ; i < things.Length () ; i++)
	{
	  ltThing* th = things.Get (i);
	  th->CompressVertices ();
	  th->RemoveUnusedVertices ();
	  th->RemoveDuplicateVertices ();
	  th->CreateVertexInfo ();
	  th->CreateBoundingBox ();
	  global_bbox += th->GetBoundingBox ();
	}

	float global_area = (global_bbox.MaxX () - global_bbox.MinX ()) *
		     (global_bbox.MaxY () - global_bbox.MinY ()) *
		     (global_bbox.MaxZ () - global_bbox.MinZ ());
	global_area = pow (global_area, 1.0f/3.0f);

	for (i = 0 ; i < things.Length () ; i++)
	{
	  ltThing* th = things.Get (i);
	  const csBox3& b = th->GetBoundingBox ();
	  float area = (b.MaxX () - b.MinX ()) *
		     (b.MaxY () - b.MinY ()) *
		     (b.MaxZ () - b.MinZ ());
	  area = pow (area, 1.0f/3.0f);
	  int where = int (100 * (area / global_area));
	  if (where >= minsize)
	    th->SplitThingSeparateUnits ();
	  else
	    th->DoNotSplitThingSeparateUnits ();
        }

	CloneAndSplitDynavis (doc, newdoc, false);
        error = newdoc->Write (vfs, filename);
	if (error != 0)
	{
	  ReportError ("Error writing '%s': %s!", (const char*)filename, error);
	  return;
	}
      }
      break;

    case OP_SPLITGEOM:
      {
	AnalyzePluginSection (doc);
	FindAllThings (doc);
	csRef<iDocument> newdoc = newsys->CreateDocument ();

	csBox3 global_bbox;
	global_bbox.StartBoundingBox ();
	size_t i;
	for (i = 0 ; i < things.Length () ; i++)
	{
	  ltThing* th = things.Get (i);
	  th->CreateBoundingBox ();
	  global_bbox += th->GetBoundingBox ();
	}

	float global_area = (global_bbox.MaxX () - global_bbox.MinX ()) *
		     (global_bbox.MaxY () - global_bbox.MinY ()) *
		     (global_bbox.MaxZ () - global_bbox.MinZ ());
	global_area = pow (global_area, 1.0f/3.0f);

	for (i = 0 ; i < things.Length () ; i++)
	{
	  ltThing* th = things.Get (i);
	  const csBox3& b = th->GetBoundingBox ();
	  float area = (b.MaxX () - b.MinX ()) *
		     (b.MaxY () - b.MinY ()) *
		     (b.MaxZ () - b.MinZ ());
	  area = pow (area, 1.0f/3.0f);
	  int where = int (100 * (area / global_area));
	  if (where >= minsize)
	  {
	    th->DuplicateSharedVertices ();
	    th->SplitThingInCenter ();
	  }
	  else
	    th->DoNotSplitThingSeparateUnits ();
        }

	CloneAndSplitDynavis (doc, newdoc, false);
        error = newdoc->Write (vfs, filename);
	if (error != 0)
	{
	  ReportError ("Error writing '%s': %s!", (const char*)filename, error);
	  return;
	}
      }
      break;

    case OP_FLAGBAD:
    case OP_FLAGGOOD:
    case OP_FLAGCLEAR:
      {
	AnalyzePluginSection (doc);
	FindAllThings (doc);
	csRef<iDocument> newdoc = newsys->CreateDocument ();

	csBox3 global_bbox;
	global_bbox.StartBoundingBox ();
	size_t i;
	for (i = 0 ; i < things.Length () ; i++)
	{
	  ltThing* th = things.Get (i);
	  th->CreateBoundingBox ();
	  global_bbox += th->GetBoundingBox ();
	}

	float global_area = (global_bbox.MaxX () - global_bbox.MinX ()) *
		     (global_bbox.MaxY () - global_bbox.MinY ()) *
		     (global_bbox.MaxZ () - global_bbox.MinZ ());
	global_area = pow (global_area, 1.0f/3.0f);

	cnt = 0;
	CloneAndChangeFlags (doc, newdoc, op, minsize, maxsize,
		minpoly, maxpoly, global_area);
	if (op != OP_FLAGCLEAR) csPrintf ("%d things flagged!\n", cnt);
        error = newdoc->Write (vfs, filename);
	if (error != 0)
	{
	  ReportError ("Error writing '%s': %s!", (const char*)filename, error);
	  return;
	}
      }
      break;

    case OP_TRANSLATE:
      {
	Report (CS_REPORTER_SEVERITY_NOTIFY, "Cloning...");
	csTicks cloning_start = csGetTicks();
	csRef<iDocumentNode> root = doc->GetRoot ();
	csRef<iDocument> newdoc = newsys->CreateDocument ();
	csRef<iDocumentNode> newroot = newdoc->CreateRoot ();
	CloneNode (root, newroot);
	csTicks cloning_end = csGetTicks();
	Report (CS_REPORTER_SEVERITY_NOTIFY, " time taken: %f s",
	  (float)(cloning_end - cloning_start) / (float)1000);
	root = 0;
	doc = 0;
	Report (CS_REPORTER_SEVERITY_NOTIFY, "Writing...");
	csTicks writing_start = csGetTicks();
        error = newdoc->Write (vfs, filename);
	csTicks writing_end = csGetTicks();
	if (error != 0)
	{
	  ReportError ("Error writing '%s': %s!", (const char*)filename, error);
	  return;
	}
	else
	{
	  Report (CS_REPORTER_SEVERITY_NOTIFY, " time taken: %f s",
	    (float)(writing_end - writing_start) / (float)1000);
	}
	newroot = 0;
	newdoc = 0;
	Report (CS_REPORTER_SEVERITY_NOTIFY, "Updating VFS...");
	vfs->Sync();
      }
      break;
  }

  //---------------------------------------------------------------
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  // Initialize the random number generator
  srand (time (0));

  LevTool* lt = new LevTool ();

  iObjectRegistry* object_reg;
  lt->object_reg = csInitializer::CreateEnvironment (argc, argv);
  object_reg = lt->object_reg;
  if (!lt->object_reg)
    return -1;
  if (!csInitializer::RequestPlugins (lt->object_reg,
	CS_REQUEST_VFS,
	CS_REQUEST_END))
  {
    delete lt;
    return -1;
  }

  lt->Main ();
  delete lt;

  csInitializer::DestroyApplication (object_reg);

  return 0;
}

