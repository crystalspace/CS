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
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "iutil/cmdline.h"
#include "cstool/initapp.h"
#include "ivaria/reporter.h"

//-----------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------
// Define all tokens used through this file
CS_TOKEN_DEF_START
  CS_TOKEN_DEF (MESHOBJ)
  CS_TOKEN_DEF (SECTOR)
  CS_TOKEN_DEF (WORLD)
  CS_TOKEN_DEF (PARAMS)
  CS_TOKEN_DEF (V)
  CS_TOKEN_DEF (VERTEX)
  CS_TOKEN_DEF (P)
  CS_TOKEN_DEF (POLYGON)
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
  delete[] vertices;
  int i;
  for (i = 0 ; i < num_polygons ; i++)
  {
    delete polygons[i];
  }
  delete[] polygons;
}

void ltThing::AddVertex (const csVector3& vt)
{
  if (num_vertices >= max_vertices)
  {
    if (max_vertices < 10000)
      max_vertices += max_vertices+2;
    else
      max_vertices += 10000;
    csVector3* new_verts = new csVector3 [max_vertices];
    if (num_vertices > 0)
    {
      memcpy (new_verts, vertices, sizeof (csVector3) * num_vertices);
    }
    delete[] vertices;
    vertices = new_verts;
  }
  vertices[num_vertices++] = vt;
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

void LevTool::ParseWorld (csParser* parser, char* buf)
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
      ParseSector (parser, params);
    }
  }
}

void LevTool::ParseSector (csParser* parser, char* buf)
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
      ParseMeshObj (parser, name, params);
    }
  }
}

void LevTool::ParseMeshObj (csParser* parser, const char* thname, char* buf)
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
      ParseThingParams (parser, thname, params);
    }
  }
}

void LevTool::ParseThingParams (csParser* parser, const char* thname, char* buf)
{
  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (V)
    CS_TOKEN_TABLE (VERTEX)
    CS_TOKEN_TABLE (P)
    CS_TOKEN_TABLE (POLYGON)
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
	break;
      }
    }
  }
}

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
      ParseWorld (parser, params);
    int i;
    for (i = 0 ; i < things.Length () ; i++)
    {
      ltThing* th = (ltThing*)things.Get (i);
      printf ("found thing %s\n", th->GetName ());
    }
  }
  else
  {
    ReportError ("Error parsing 'WORLD'!");
  }

  delete parser;

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

