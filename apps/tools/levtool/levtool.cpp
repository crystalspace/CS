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
#include "levtool.h"
#include "csutil/util.h"
#include "csutil/xmltiny.h"
#include "csutil/csstring.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "iutil/cmdline.h"
#include "iutil/document.h"
#include "cstool/initapp.h"
#include "ivaria/reporter.h"

//-----------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

void LevTool::ReportError (const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (object_reg, CS_REPORTER_SEVERITY_ERROR,
  	"crystalspace.apps.levtool", description, arg);
  va_end (arg);
}

//-----------------------------------------------------------------------------

ltVertex::ltVertex ()
{
  polygons = NULL;
  num_polygons = 0;
  max_polygons = 0;
}

ltVertex::ltVertex (const ltVertex& vt)
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
    polygons = NULL;
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
  name = NULL;
  num_vertices = 0;
  max_vertices = 0;
  vertices = NULL;
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
  ltPolygon::name = name ? csStrNew (name) : NULL;
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
  name = NULL;
  vertices = NULL;
  num_vertices = 0;
  max_vertices = 0;
  polygons = NULL;
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
  ltThing::name = name ? csStrNew (name) : NULL;
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

void LevTool::AnalyzePluginSection (iDocument* doc)
{
  thing_plugins.Push (new csString ("crystalspace.mesh.loader.thing"));
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
      }
    }
  }
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
  int i;
  for (i = 0 ; i < thing_plugins.Length () ; i++)
  {
    csString* str = (csString*)thing_plugins.Get (i);
    if (str->Compare (plugname))
      return true;
  }
  return false;
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

void LevTool::FindAllThings (iDocument* doc)
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
	if (!strcmp (value2, "meshobj") && IsMeshAThing (child2))
	{
	  ParseThing (child2);
	}
      }
    }
  }
}

void LevTool::CloneNode (iDocumentNode* from, iDocumentNode* to)
{
  to->SetValue (from->GetValue ());
  csRef<iDocumentNodeIterator> it = from->GetNodes ();
  while (it->HasNext ())
  {
    csRef<iDocumentNode> child = it->Next ();
    csRef<iDocumentNode> child_clone = to->CreateNodeBefore (
    	child->GetType (), NULL);
    CloneNode (child, child_clone);
  }
  csRef<iDocumentAttributeIterator> atit = from->GetAttributes ();
  while (atit->HasNext ())
  {
    csRef<iDocumentAttribute> attr = atit->Next ();
    to->SetAttribute (attr->GetName (), attr->GetValue ());
  }
}

void LevTool::RewriteThing (ltThing* thing, iDocumentNode* newthing)
{
  csRef<iDocumentNode> meshnode = thing->GetMeshNode ();
  csRef<iDocumentNode> partnode = thing->GetPartNode ();

  CloneNode (meshnode, newthing);
  csRef<iDocumentNode> oldparams = newthing->GetNode ("params");
  csRef<iDocumentNode> newparams = newthing->CreateNodeBefore (
  	CS_NODE_ELEMENT, NULL);
  CloneNode (partnode, newparams);
  newthing->RemoveNode (oldparams);

  newparams->SetValue ("params");
  newthing->SetAttribute ("name", thing->GetName ());
  csRef<iDocumentNode> vnode = newparams->GetNode ("v");
  while (vnode != NULL)
  {
    newparams->RemoveNode (vnode);
    vnode = newparams->GetNode ("v");
  }
  csRef<iDocumentNodeIterator> it = newparams->GetNodes ();
  // We will add all vertices before 'firstchild'.
  csRef<iDocumentNode> firstchild = it->Next ();

  int i;
  for (i = 0 ; i < thing->GetVertexCount () ; i++)
  {
    const ltVertex& vt = thing->GetVertex (i);
    csRef<iDocumentNode> newv = newparams->CreateNodeBefore (
    	CS_NODE_ELEMENT, firstchild);
    newv->SetAttributeAsFloat ("x", vt.x);
    newv->SetAttributeAsFloat ("y", vt.y);
    newv->SetAttributeAsFloat ("z", vt.z);
    newv->SetValue ("v");
  }
}

//-----------------------------------------------------------------------------

void LevTool::ListMeshPart (iDocumentNode* meshpart, int level)
{
  char indent[1024];
  int i; for (i = 0 ; i < level ; i++) indent[i] = ' '; indent[level] = 0;
  const char* name = meshpart->GetAttributeValue ("name");
  printf ("%spart(%s)\n", indent, name);
}

void LevTool::ListMeshObject (iDocumentNode* mesh, int level)
{
  char indent[1024];
  int i; for (i = 0 ; i < level ; i++) indent[i] = ' '; indent[level] = 0;
  const char* name = mesh->GetAttributeValue ("name");
  csRef<iDocumentNode> pluginnode = mesh->GetNode ("plugin");
  const char* plugin = pluginnode->GetContentsValue ();
  printf ("%smeshobj(%s,%s)\n", indent, name, plugin);

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
  printf ("%smeshfact(%s,%s)\n", indent, name, plugin);

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
  printf ("%ssector(%s)\n", indent, name);
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
  printf ("world\n");
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

LevTool::LevTool ()
{
  object_reg = NULL;
}

LevTool::~LevTool ()
{
  int i;
  for (i = 0 ; i < things.Length () ; i++)
  {
    ltThing* th = (ltThing*)things.Get (i);
    delete th;
  }
  for (i = 0 ; i < thing_plugins.Length () ; i++)
  {
    csString* str = (csString*)thing_plugins.Get (i);
    delete str;
  }
}

#define OP_LIST 1
#define OP_DYNAVIS 2

//----------------------------------------------------------------------------

void LevTool::Main ()
{
  cmdline = CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);

  int op = OP_LIST;

  if (cmdline->GetOption ("help"))
  {
    printf ("levtool <options> <zipfile>\n");
    printf ("  -list: list world contents\n");
    printf ("  -dynavis: convert to Dynavis\n");
    exit (0);
  }
  if (cmdline->GetOption ("dynavis")) op = OP_DYNAVIS;
  if (cmdline->GetOption ("list")) op = OP_LIST;

  const char* val = cmdline->GetName ();
  if (!val)
  {
    ReportError ("Please give VFS world file name or name of the zip archive!");
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
  vfs->WriteFile (filename+".bak", **buf, buf->GetSize ());

  csRef<iDocumentSystem> xml (csPtr<iDocumentSystem> (
  	new csTinyDocumentSystem ()));
  csRef<iDocument> doc (xml->CreateDocument ());
  const char* error = doc->Parse (buf);
  if (error != NULL)
  {
    ReportError ("Error parsing XML: %s!", error);
    return;
  }

  //---------------------------------------------------------------

  if (!TestValidXML (doc))
    return;

  csRef<iDocumentNode> root = doc->GetRoot ();
  csRef<iDocumentNode> worldnode = root->GetNode ("world");

  switch (op)
  {
    case OP_LIST:
      {
	AnalyzePluginSection (doc);
        ListContents (doc->GetRoot ());
      }
      break;
    case OP_DYNAVIS:
      {
	AnalyzePluginSection (doc);
	FindAllThings (doc);
        int i;
        for (i = 0 ; i < things.Length () ; i++)
        {
          ltThing* th = (ltThing*)things.Get (i);
          printf ("found thing %s\n", th->GetName ());
          th->CompressVertices ();
          th->RemoveUnusedVertices ();
          th->RemoveDuplicateVertices ();
          th->CreateVertexInfo ();

          csRef<iDocumentNode> sectornode = th->GetMeshNode ()->GetParent ();
          csRef<iDocumentNode> newnode = sectornode->CreateNodeBefore (
    	      CS_NODE_ELEMENT, NULL);
          RewriteThing (th, newnode);
        }
        for (i = 0 ; i < thing_nodes.Length () ; i++)
        {
          ltDocNodeWrap* w = (ltDocNodeWrap*)thing_nodes.Get (i);
	  csRef<iDocumentNode> node = w->node;
          csRef<iDocumentNode> parent = node->GetParent ();
printf ("  parent '%s'\n", parent->GetValue ());
printf ("  node   '%s'\n", node->GetValue ());
printf ("REMOVE NODE %p->%p\n", (iDocumentNode*)parent, (iDocumentNode*)node);
          parent->RemoveNode (node);
	  delete w;
        }
      }
      break;
  }

  //---------------------------------------------------------------

  error = doc->Write (vfs, filename);
  if (error != NULL)
  {
    ReportError ("Error writing '%s': %s!", (const char*)filename, error);
    return;
  }
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  // Initialize the random number generator
  srand (time (NULL));

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

