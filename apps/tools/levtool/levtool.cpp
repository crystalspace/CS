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
#include "csutil/parser.h"
#include "csutil/indprint.h"
#include "csutil/scanstr.h"
#include "csutil/csstring.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "iutil/cmdline.h"
#include "cstool/initapp.h"
#include "ivaria/reporter.h"

//-----------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

static void Write (iFile* fout, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);

  csString str;
  str.FormatV (description, arg);

  va_end (arg);

  fout->Write (str.GetData (), str.Length ());
}

static void WriteStr (iFile* fout, const char* str)
{
  fout->Write (str, strlen (str));
}

static void WriteStruct (iFile* fout, int spaces, const char* token,
	const char* name, const char* params)
{
  while (spaces > 4) { WriteStr (fout, "    "); spaces -= 4; }
  while (spaces > 0) { WriteStr (fout, " "); spaces--; }
  if (name == NULL)
    Write (fout, "%s (%s)\n", token, params);
  else
    Write (fout, "%s '%s' (%s)\n", token, name, params);
}

//-----------------------------------------------------------------------------

// Define all tokens used through this file
CS_TOKEN_DEF_START
  CS_TOKEN_DEF (MESHOBJ)
  CS_TOKEN_DEF (SECTOR)
  CS_TOKEN_DEF (WORLD)
  CS_TOKEN_DEF (PARAMS)
  CS_TOKEN_DEF (V)
  CS_TOKEN_DEF (VERTEX)
  CS_TOKEN_DEF (VERTICES)
  CS_TOKEN_DEF (P)
  CS_TOKEN_DEF (POLYGON)
  CS_TOKEN_DEF (PART)
CS_TOKEN_DEF_END

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

ltPolygon::ltPolygon ()
{
  name = NULL;
  num_vertices = 0;
  max_vertices = 0;
  vertices = NULL;
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

ltThing::ltThing ()
{
  name = NULL;
  vertices = NULL;
  num_vertices = 0;
  max_vertices = 0;
  polygons = NULL;
  num_polygons = 0;
  max_polygons = 0;
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

ltPolygon* ltThing::AddPolygon ()
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
  ltPolygon* np = new ltPolygon ();
  polygons[num_polygons++] = np;
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

LevTool::LevTool ()
{
  object_reg = NULL;
  cmdline = NULL;
  vfs = NULL;
}

LevTool::~LevTool ()
{
  int i;
  for (i = 0 ; i < things.Length () ; i++)
  {
    ltThing* th = (ltThing*)things.Get (i);
    delete th;
  }

  if (cmdline) cmdline->DecRef ();
  if (vfs) vfs->DecRef ();
  if (object_reg)
    csInitializer::DestroyApplication (object_reg);
}

void LevTool::ParseWorld (csParser* parser, iFile* fout, char* buf)
{
  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (SECTOR)
  CS_TOKEN_TABLE_END

  char *name, *params;
  long cmd;

  while ((cmd = parser->GetObject (&buf, tokens, &name, &params))
  	!= CS_PARSERR_EOF)
  {
    if (params && cmd == CS_TOKEN_SECTOR)
    {
      Write (fout, "  SECTOR '%s' (\n", name);
      ParseSector (parser, fout, params);
      WriteStr (fout, "  )\n");
    }
    else
    {
      WriteStruct (fout, 2, parser->GetUnknownToken (), name, params);
    }
  }
}

void LevTool::ParseSector (csParser* parser, iFile* fout, char* buf)
{
  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (MESHOBJ)
  CS_TOKEN_TABLE_END

  char *name, *params;
  long cmd;

  while ((cmd = parser->GetObject (&buf, tokens, &name, &params))
  	!= CS_PARSERR_EOF)
  {
    if (params && cmd == CS_TOKEN_MESHOBJ)
    {
      Write (fout, "    MESHOBJ '%s' (\n", name);
      ParseMeshObj (parser, fout, name, params);
      WriteStr (fout, "    )\n");
    }
    else
    {
      WriteStruct (fout, 4, parser->GetUnknownToken (), name, params);
    }
  }
}

void LevTool::ParseMeshObj (csParser* parser, iFile* fout,
	const char* thname, char* buf)
{
  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (PARAMS)
  CS_TOKEN_TABLE_END

  char *name, *params;
  long cmd;

  while ((cmd = parser->GetObject (&buf, tokens, &name, &params))
  	!= CS_PARSERR_EOF)
  {
    if (params && cmd == CS_TOKEN_PARAMS)
    {
      Write (fout, "      PARAMS (\n");
      ParseThingParams (parser, fout, thname, params);
      Write (fout, "      )\n");
    }
    else
    {
      WriteStruct (fout, 6, parser->GetUnknownToken (), name, params);
    }
  }
}

void LevTool::ParseThingParams (csParser* parser, iFile* fout,
	const char* thname, char* buf)
{
  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (V)
    CS_TOKEN_TABLE (VERTEX)
    CS_TOKEN_TABLE (P)
    CS_TOKEN_TABLE (POLYGON)
    CS_TOKEN_TABLE (PART)
  CS_TOKEN_TABLE_END

  char *name, *params;
  long cmd;

  ltThing* th = new ltThing ();
  th->SetName (thname);
  things.Push (th);

  while ((cmd = parser->GetObject (&buf, tokens, &name, &params))
  	!= CS_PARSERR_EOF)
  {
    if (!params) continue;
    switch (cmd)
    {
      case CS_TOKEN_V:
      case CS_TOKEN_VERTEX:
      {
	csVector3 v;
        csScanStr (params, "%f,%f,%f", &v.x, &v.y, &v.z);
	th->AddVertex (v);
	break;
      }
      case CS_TOKEN_P:
      case CS_TOKEN_POLYGON:
      {
	ltPolygon* p = th->AddPolygon ();
	p->SetName (name);
	ParsePolygonParams (parser, fout, p, params);
	break;
      }
      case CS_TOKEN_PART:
      {
        ParseThingParams (parser, fout, name, params);
        break;
      }
      case CS_PARSERR_TOKENNOTFOUND:
      {
        WriteStruct (fout, 8, parser->GetUnknownToken (), name, params);
	break;
      }
    }
  }
}

void LevTool::ParsePolygonParams (csParser* parser, iFile* fout,
	ltPolygon* polygon, char* buf)
{
  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (V)
    CS_TOKEN_TABLE (VERTICES)
  CS_TOKEN_TABLE_END

  char *name, *params;
  long cmd;

  while ((cmd = parser->GetObject (&buf, tokens, &name, &params))
  	!= CS_PARSERR_EOF)
  {
    if (!params) continue;
    switch (cmd)
    {
      case CS_TOKEN_V:
      case CS_TOKEN_VERTICES:
      {
	char* p = params;
	while (*p && *p == ' ') p++;
	if (*p < '0' || *p > '9')
	{
#if 0
// @@@ TODO
	  // We have a special vertex selection depending on
	  // a VBLOCK or VROOM command previously generated.
	  int vtidx;
	  if (*(p+1) == ',')
	  {
	    csScanStr (p+2, "%d", &vtidx);
	    vtidx += vt_offset;
	  }
	  else
	    vtidx = thing_state->GetVertexCount ()-8;
	  switch (*p)
	  {
	    case 'w':
	      poly3d->CreateVertex (vtidx+6);
	      poly3d->CreateVertex (vtidx+4);
	      poly3d->CreateVertex (vtidx+0);
	      poly3d->CreateVertex (vtidx+2);
	      break;
	    case 'e':
	      poly3d->CreateVertex (vtidx+5);
	      poly3d->CreateVertex (vtidx+7);
	      poly3d->CreateVertex (vtidx+3);
	      poly3d->CreateVertex (vtidx+1);
	      break;
	    case 'n':
	      poly3d->CreateVertex (vtidx+7);
	      poly3d->CreateVertex (vtidx+6);
	      poly3d->CreateVertex (vtidx+2);
	      poly3d->CreateVertex (vtidx+3);
	      break;
	    case 's':
	      poly3d->CreateVertex (vtidx+4);
	      poly3d->CreateVertex (vtidx+5);
	      poly3d->CreateVertex (vtidx+1);
	      poly3d->CreateVertex (vtidx+0);
	      break;
	    case 'u':
	      poly3d->CreateVertex (vtidx+6);
	      poly3d->CreateVertex (vtidx+7);
	      poly3d->CreateVertex (vtidx+5);
	      poly3d->CreateVertex (vtidx+4);
	      break;
	    case 'd':
	      poly3d->CreateVertex (vtidx+0);
	      poly3d->CreateVertex (vtidx+1);
	      poly3d->CreateVertex (vtidx+3);
	      poly3d->CreateVertex (vtidx+2);
	      break;
	  }
#endif
	}
	else
	{
          int list[100], num;
          csScanStr (params, "%D", list, &num);
          for (int i = 0 ; i < num ; i++)
	  {
	    //polygon->AddVertex (list[i]);
	  }
        }
	break;
      }
    }
  }
}

//----------------------------------------------------------------------------

void LevTool::Main ()
{
  cmdline = CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);

  const char* val = cmdline->GetName ();
  if (!val)
  {
    ReportError ("Please give VFS world file name or name of the zip archive!");
    return;
  }

  iDataBuffer* buf = NULL;
  if (strstr (val, ".zip"))
  {
    vfs->Mount ("/tmp/levtool_data", val);
    buf = vfs->ReadFile ("/tmp/levtool_data/world");
    if (!buf || !buf->GetSize ())
    {
      ReportError ("Archive '%s' does not seem to contain a 'world' file!",
      	val);
      return;
    }
  }
  else
  {
    buf = vfs->ReadFile (val);
    if (!buf || !buf->GetSize ())
    {
      ReportError ("Could not load file '%s'!", val);
      return;
    }
  }

  iFile* fout = vfs->Open ("/this/world", VFS_FILE_WRITE);
  if (!fout)
  {
    buf->DecRef ();
    ReportError ("Could not open file '/this/world'!");
    return;
  }

  csParser* parser = new csParser (true);
  parser->ResetParserLine ();

  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (WORLD)
  CS_TOKEN_TABLE_END

  char *data = **buf;
  char *name, *params;
  long cmd;

  if ((cmd = parser->GetObject (&data, tokens, &name, &params))
  	!= CS_PARSERR_EOF)
  {
    if (params)
    {
      WriteStr (fout, "WORLD (\n");
      ParseWorld (parser, fout, params);
      WriteStr (fout, ")\n");
    }
    int i;
    for (i = 0 ; i < things.Length () ; i++)
    {
      ltThing* th = (ltThing*)things.Get (i);
      printf ("found thing %s\n", th->GetName ());
      th->CompressVertices ();
      th->RemoveUnusedVertices ();
      th->RemoveDuplicateVertices ();
      th->CreateVertexInfo ();
    }
  }
  else
  {
    ReportError ("Error parsing 'WORLD'!");
  }

  delete parser;

  fout->DecRef ();
  buf->DecRef ();
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  // Initialize the random number generator
  srand (time (NULL));

  LevTool* lt = new LevTool ();

  lt->object_reg = csInitializer::CreateEnvironment (argc, argv);
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

  return 0;
}

