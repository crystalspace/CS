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
#include <ctype.h>

#include "cssysdef.h"
#include "cs2xml.h"
#include "parser.h"
#include "csgeom/vector3.h"
#include "csgeom/vector2.h"
#include "csutil/cscolor.h"
#include "csutil/util.h"
#include "csutil/indprint.h"
#include "csutil/scanstr.h"
#include "csutil/csstring.h"
#include "csutil/xmltiny.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "iutil/cmdline.h"
#include "iutil/strvec.h"
#include "cstool/initapp.h"
#include "ivaria/reporter.h"

//-----------------------------------------------------------------------------

CS_IMPLEMENT_APPLICATION

//-----------------------------------------------------------------------------

void Cs2Xml::ReportError (const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (object_reg, CS_REPORTER_SEVERITY_ERROR,
  	"crystalspace.apps.cs2xml", description, arg);
  va_end (arg);
}

//-----------------------------------------------------------------------------

// Define all tokens used through this file
CS_TOKEN_DEF_START
  CS_TOKEN_DEF (ACCEL)
  CS_TOKEN_DEF (AGING)
  CS_TOKEN_DEF (ALLMATERIAL)
  CS_TOKEN_DEF (AMBIENT)
  CS_TOKEN_DEF (ANIMATE)
  CS_TOKEN_DEF (ATTACH)
  CS_TOKEN_DEF (BASECOLOR)
  CS_TOKEN_DEF (BLOCKS)
  CS_TOKEN_DEF (BLOCKSIZE)
  CS_TOKEN_DEF (BOX)
  CS_TOKEN_DEF (BC)
  CS_TOKEN_DEF (BH)
  CS_TOKEN_DEF (C)
  CS_TOKEN_DEF (CENTER)
  CS_TOKEN_DEF (CURVECENTER)
  CS_TOKEN_DEF (CURVECONTROL)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (COLORS)
  CS_TOKEN_DEF (CORRECTSEAMS)
  CS_TOKEN_DEF (DIRECTION)
  CS_TOKEN_DEF (DIRECTIONAL)
  CS_TOKEN_DEF (DIRLIGHT)
  CS_TOKEN_DEF (DROPSIZE)
  CS_TOKEN_DEF (DURATION)
  CS_TOKEN_DEF (DOFLATTEN)
  CS_TOKEN_DEF (EMITBOX)
  CS_TOKEN_DEF (EMITLINE)
  CS_TOKEN_DEF (EMITFIXED)
  CS_TOKEN_DEF (EMITSPHERE)
  CS_TOKEN_DEF (EMITCYLINDER)
  CS_TOKEN_DEF (EMITCYLINDERTANGENT)
  CS_TOKEN_DEF (EULER)
  CS_TOKEN_DEF (END)
  CS_TOKEN_DEF (F)
  CS_TOKEN_DEF (FADE)
  CS_TOKEN_DEF (FALLSPEED)
  CS_TOKEN_DEF (FARPLANE)
  CS_TOKEN_DEF (FILE)
  CS_TOKEN_DEF (FIRST)
  CS_TOKEN_DEF (FLATTEN)
  CS_TOKEN_DEF (FOG)
  CS_TOKEN_DEF (FORWARD)
  CS_TOKEN_DEF (FRAME)
  CS_TOKEN_DEF (GENERATE)
  CS_TOKEN_DEF (GRID)
  CS_TOKEN_DEF (GROUPMATERIAL)
  CS_TOKEN_DEF (HALO)
  CS_TOKEN_DEF (HAZEBOX)
  CS_TOKEN_DEF (HAZECONE)
  CS_TOKEN_DEF (HEIGHTMAP)
  CS_TOKEN_DEF (IDENTITY)
  CS_TOKEN_DEF (KEY)
  CS_TOKEN_DEF (LIGHT)
  CS_TOKEN_DEF (LOD)
  CS_TOKEN_DEF (LODCOST)
  CS_TOKEN_DEF (LODDIST)
  CS_TOKEN_DEF (MATRIX)
  CS_TOKEN_DEF (MATERIAL)
  CS_TOKEN_DEF (MAXCOLOR)
  CS_TOKEN_DEF (MOVE)
  CS_TOKEN_DEF (MULT)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (NUM)
  CS_TOKEN_DEF (ORIG)
  CS_TOKEN_DEF (ORIGIN)
  CS_TOKEN_DEF (ORIGINBOX)
  CS_TOKEN_DEF (PATH)
  CS_TOKEN_DEF (PARTSIZE)
  CS_TOKEN_DEF (POLYGON)
  CS_TOKEN_DEF (PORTAL)
  CS_TOKEN_DEF (POS)
  CS_TOKEN_DEF (POSITION)
  CS_TOKEN_DEF (PUSH)
  CS_TOKEN_DEF (Q)
  CS_TOKEN_DEF (PRIORITY)
  CS_TOKEN_DEF (RADIUS)
  CS_TOKEN_DEF (RECTPARTICLES)
  CS_TOKEN_DEF (REGULARPARTICLES)
  CS_TOKEN_DEF (ROT)
  CS_TOKEN_DEF (ROT_X)
  CS_TOKEN_DEF (ROT_Y)
  CS_TOKEN_DEF (ROT_Z)
  CS_TOKEN_DEF (ROTPART)
  CS_TOKEN_DEF (SPACE)
  CS_TOKEN_DEF (SCALE)
  CS_TOKEN_DEF (SCALE_X)
  CS_TOKEN_DEF (SCALE_Y)
  CS_TOKEN_DEF (SCALE_Z)
  CS_TOKEN_DEF (SECOND)
  CS_TOKEN_DEF (SETUPMESH)
  CS_TOKEN_DEF (SHIFT)
  CS_TOKEN_DEF (SINGLE)
  CS_TOKEN_DEF (SIZE)
  CS_TOKEN_DEF (SLOPE)
  CS_TOKEN_DEF (SOLID)
  CS_TOKEN_DEF (SOURCE)
  CS_TOKEN_DEF (SPEED)
  CS_TOKEN_DEF (START)
  CS_TOKEN_DEF (SYSDIST)
  CS_TOKEN_DEF (T)
  CS_TOKEN_DEF (TEXTURE)
  CS_TOKEN_DEF (TIMES)
  CS_TOKEN_DEF (TOPLEFT)
  CS_TOKEN_DEF (TRANSPARENT)
  CS_TOKEN_DEF (TRIANGLE)
  CS_TOKEN_DEF (TRIANGLES)
  CS_TOKEN_DEF (TYPE)
  CS_TOKEN_DEF (UV)
  CS_TOKEN_DEF (UVA)
  CS_TOKEN_DEF (UVEC)
  CS_TOKEN_DEF (UV_SHIFT)
  CS_TOKEN_DEF (UP)
  CS_TOKEN_DEF (V)
  CS_TOKEN_DEF (VVEC)
  CS_TOKEN_DEF (VERTEX)
  CS_TOKEN_DEF (VERTICES)
  CS_TOKEN_DEF (W)
  CS_TOKEN_DEF (WARP)
CS_TOKEN_DEF_END

//-----------------------------------------------------------------------------

Cs2Xml::Cs2Xml (iObjectRegistry* object_reg)
{
  Cs2Xml::object_reg = object_reg;
}

Cs2Xml::~Cs2Xml ()
{
}

bool Cs2Xml::IsEmpty (const char* in)
{
  if (!in) return true;
  while (isspace (*in)) in++;
  if (*in == 0) return true;
  return false;
}

bool Cs2Xml::IsNumeric (const char* in)
{
  while (isspace (*in)) in++;
  if (*in == '-' || *in == '+') in++;
  while (*in >= '0' && *in <= '9') in++;
  if (*in == '.')
  {
    in++;
    while (*in >= '0' && *in <= '9') in++;
  }

  if (*in == 'e' || *in == 'E') in++;
  else return (*in == 0);

  if (*in == '-' || *in == '+') in++;
  while (*in >= '0' && *in <= '9') in++;
  while (isspace (*in)) in++;
  if (*in == 0) return true;
  return false;
}

bool Cs2Xml::IsString (const char* in)
{
  while (isspace (*in)) in++;
  if (*in == '\'')
  {
    in++;
    while (*in != 0 && *in != '\'')
    {
      if (*in == '\\')
      {
        in++;
	if (*in != 0) in++;
      }
      else in++;
    }
    if (*in != '\'') return false;
    in++;
  }
  else
  {
    // There are no quotes. Check if it is a valid identifier.
    if ((*in >= 'a' && *in <= 'z') || (*in >= 'A' && *in <= 'Z') ||
    	(*in == '_' || *in == '$'))
    {
      in++;
      while (*in && !isspace (*in))
      {
        if ((*in >= 'a' && *in <= 'z') || (*in >= 'A' && *in <= 'Z') ||
    	    (*in == '-' || *in == '_' || *in == '$' || *in == '.') ||
	    (*in >= '0' && *in <= '9'))
          in++;
        else break;
      }
    }
    else
      return false;
  }
  while (isspace (*in)) in++;
  if (*in == 0) return true;
  return false;
}

bool Cs2Xml::IsBoolean (const char* in, bool& val)
{
  while (isspace (*in)) in++;
  if (!strncasecmp (in, "yes", 3)) { val = true; in += 3; }
  else if (!strncasecmp (in, "true", 4)) { val = true; in += 4; }
  else if (!strncasecmp (in, "on", 2)) { val = true; in += 2; }
  else if (!strncasecmp (in, "no", 2)) { val = false; in += 2;} 
  else if (!strncasecmp (in, "false", 5)) { val = false; in += 5;} 
  else if (!strncasecmp (in, "off", 3)) { val = false; in += 3; }
  while (isspace (*in)) in++;
  if (*in == 0) return true;
  return false;
}

int Cs2Xml::IsNumberList (const char* in)
{
  int cnt = 0;
  char* comma = strchr (in, ',');
  while (comma != NULL)
  {
    *comma = 0;
    if (IsNumeric (in))
    {
      *comma = ',';
      cnt++;
      in = comma+1;
    }
    else
    {
      *comma = ',';
      return 0;
    }
    comma = strchr (in, ',');
  }
  if (IsNumeric (in))
    return cnt+1;
  else
    return 0;
}

char* Cs2Xml::ToLower (const char* in, bool remove_under)
{
  char* rc = new char [strlen (in)+1];
  char* out = rc;
  while (*in)
  {
    if (remove_under && *in == '_') { in++; continue; }
    *out = tolower (*in);
    out++;
    in++;
  }
  *out = 0;
  return rc;
}

csRef<iDocumentNode> Cs2Xml::CreateValueNode (
	csRef<iDocumentNode>& parent,
	const char* name, const char* value)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	  CS_NODE_ELEMENT, NULL);
  child->SetValue (name);
  csRef<iDocumentNode> text = child->CreateNodeBefore (
		  CS_NODE_TEXT, NULL);
  text->SetValue (value);
  return child;
}

csRef<iDocumentNode> Cs2Xml::CreateValueNodeAsInt (
	csRef<iDocumentNode>& parent,
	const char* name, int value)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	  CS_NODE_ELEMENT, NULL);
  child->SetValue (name);
  csRef<iDocumentNode> text = child->CreateNodeBefore (
		  CS_NODE_TEXT, NULL);
  text->SetValueAsInt (value);
  return child;
}

csRef<iDocumentNode> Cs2Xml::CreateValueNodeAsFloat (
	csRef<iDocumentNode>& parent,
	const char* name, float value)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	  CS_NODE_ELEMENT, NULL);
  child->SetValue (name);
  csRef<iDocumentNode> text = child->CreateNodeBefore (
		  CS_NODE_TEXT, NULL);
  text->SetValueAsFloat (value);
  return child;
}

void Cs2Xml::ParseMatrix (csParser *parser, csRef<iDocumentNode>& parent,
	char* buf)
{
  CS_TOKEN_TABLE_START(commands)
    CS_TOKEN_TABLE (IDENTITY)
    CS_TOKEN_TABLE (ROT_X)
    CS_TOKEN_TABLE (ROT_Y)
    CS_TOKEN_TABLE (ROT_Z)
    CS_TOKEN_TABLE (ROT)
    CS_TOKEN_TABLE (SCALE_X)
    CS_TOKEN_TABLE (SCALE_Y)
    CS_TOKEN_TABLE (SCALE_Z)
    CS_TOKEN_TABLE (SCALE)
  CS_TOKEN_TABLE_END

  char* params;
  int cmd;
  float angle;
  float scaler;
  int num;
  float list[1000];
  char* orig_buf = csStrNew (buf);

  while ((cmd = parser->GetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case CS_TOKEN_IDENTITY:
        {
	  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	  CS_NODE_ELEMENT, NULL);
	  child->SetValue ("scale");
	  child->SetAttributeAsFloat ("all", 1);
	}
        break;
      case CS_TOKEN_ROT_X:
        csScanStr (params, "%f", &angle);
	CreateValueNodeAsFloat (parent, "rotx", angle);
        break;
      case CS_TOKEN_ROT_Y:
        csScanStr (params, "%f", &angle);
	CreateValueNodeAsFloat (parent, "roty", angle);
        break;
      case CS_TOKEN_ROT_Z:
        csScanStr (params, "%f", &angle);
	CreateValueNodeAsFloat (parent, "rotz", angle);
        break;
      case CS_TOKEN_ROT:
        csScanStr (params, "%F", list, &num);
        if (num == 3)
        {
	  CreateValueNodeAsFloat (parent, "rotx", list[0]);
	  CreateValueNodeAsFloat (parent, "rotz", list[2]);
	  CreateValueNodeAsFloat (parent, "roty", list[1]);
        }
        else
	{
	  // Error@@@
	}
        break;
      case CS_TOKEN_SCALE_X:
        {
          csScanStr (params, "%f", &scaler);
	  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	  CS_NODE_ELEMENT, NULL);
	  child->SetValue ("scale");
	  child->SetAttributeAsFloat ("x", scaler);
	}
        break;
      case CS_TOKEN_SCALE_Y:
        {
          csScanStr (params, "%f", &scaler);
	  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	  CS_NODE_ELEMENT, NULL);
	  child->SetValue ("scale");
	  child->SetAttributeAsFloat ("y", scaler);
	}
        break;
      case CS_TOKEN_SCALE_Z:
        {
          csScanStr (params, "%f", &scaler);
	  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	  CS_NODE_ELEMENT, NULL);
	  child->SetValue ("scale");
	  child->SetAttributeAsFloat ("z", scaler);
	}
        break;
      case CS_TOKEN_SCALE:
        csScanStr (params, "%F", list, &num);
        if (num == 1)      // One scaler; applied to entire matrix.
	{
	  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	  CS_NODE_ELEMENT, NULL);
	  child->SetValue ("scale");
	  child->SetAttributeAsFloat ("all", list[0]);
	}
        else if (num == 3) // Three scalers; applied to X, Y, Z individually.
	{
	  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	  CS_NODE_ELEMENT, NULL);
	  child->SetValue ("scale");
	  child->SetAttributeAsFloat ("x", list[0]);
	  child->SetAttributeAsFloat ("y", list[1]);
	  child->SetAttributeAsFloat ("z", list[2]);
	}
        else
	{
	  printf ("error 1\n"); fflush (stdout);
	  // Error@@@
	}
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    // Neither SCALE, ROT, nor IDENTITY, so matrix may contain a single scaler
    // or the nine values of a 3x3 matrix.
    csScanStr (orig_buf, "%F", list, &num);
    if (num == 1)
    {
      csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	  CS_NODE_ELEMENT, NULL);
      child->SetValue ("scale");
      child->SetAttributeAsFloat ("all", list[0]);
    }
    else if (num == 9)
    {
      CreateValueNodeAsFloat (parent, "m11", list[0]);
      CreateValueNodeAsFloat (parent, "m12", list[1]);
      CreateValueNodeAsFloat (parent, "m13", list[2]);
      CreateValueNodeAsFloat (parent, "m21", list[3]);
      CreateValueNodeAsFloat (parent, "m22", list[4]);
      CreateValueNodeAsFloat (parent, "m23", list[5]);
      CreateValueNodeAsFloat (parent, "m31", list[6]);
      CreateValueNodeAsFloat (parent, "m32", list[7]);
      CreateValueNodeAsFloat (parent, "m33", list[8]);
    }
    else
    {
      printf ("error 2 num=%d\n", num); fflush (stdout);
      // Error @@@
    }
  }
  delete[] orig_buf;
}

void Cs2Xml::ParsePortal (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname, csRef<iDocumentNode>& portal_node)
{
  if (!portal_node)
  {
    portal_node = parent->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    portal_node->SetValue ("portal");
  }
  char buf[2048];
  csScanStr (params, "%s", buf);
  CreateValueNode (portal_node, "sector", buf);
}

void Cs2Xml::ParseWarp (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname, csRef<iDocumentNode>& portal_node)
{
  if (!portal_node)
  {
    portal_node = parent->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    portal_node->SetValue ("portal");
  }
  ParseGeneral ("portal", parser, portal_node, params);
}

void Cs2Xml::ParseMaterial(char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  if (!strcmp (parent_token, "materials"))
  {
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	  CS_NODE_ELEMENT, NULL);
    child->SetValue ("material");
    if (name && *name) child->SetAttribute ("name", name);
    if (IsString (params))	// Isometric engine
    {
      csRef<iDocumentNode> text = child->CreateNodeBefore (
	CS_NODE_TEXT, NULL);
      char buf[2048];
      csScanStr (params, "%s", buf);
      text->SetValue (buf);
    }
    else	// 3D engine
    {
      ParseGeneral (tokname, parser, child, params);
    }
  }
  else if (!strcmp (parent_token, "tile2d"))	// Iso engine.
  {
    char buf[512];
    csScanStr (params, "%s", buf);
    csRef<iDocumentNode> child = CreateValueNode (parent,
		"material", buf);
    if (name && *name) child->SetAttribute ("name", name);
  }
  else
  {
    int x, z;
    int i;
    char buf[512];
    int num = csScanStr (params, "%d,%d,%s", &x, &z, buf);
    if (num == 3)
    {
      // Material for bcterr.
      csRef<iDocumentNode> child = CreateValueNode (parent,
		"material", buf);
      if (name && *name) child->SetAttribute ("name", name);
      child->SetAttributeAsInt ("x", x);
      child->SetAttributeAsInt ("y", z);
    }
    else
    {
      num = csScanStr (params, "%d,%s", &i, buf);
      if (num < 2)
      {
	csScanStr (params, "%s", buf);
	csRef<iDocumentNode> child = CreateValueNode (parent,
		"material", buf);
	if (name && *name) child->SetAttribute ("name", name);
      }
      else
      {
	csRef<iDocumentNode> child = CreateValueNode (parent,
		"material", buf);
	if (name && *name) child->SetAttribute ("name", name);
	child->SetAttributeAsInt ("index", i);
      }
    }
  }
}

void Cs2Xml::ParseMaterialGroup (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  int rangeStart, rangeEnd;
  char buf[512];
  csScanStr (params, "%s,%d,%d", buf, &rangeStart, &rangeEnd);
  csRef<iDocumentNode> child = CreateValueNode (parent, "material",
      buf);
  if (name && *name) child->SetAttribute ("name", name);
  child->SetAttributeAsInt ("start", rangeStart);
  child->SetAttributeAsInt ("end", rangeEnd);
}

void Cs2Xml::ParseAmbient (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  if (!strcmp (parent_token, "settings"))
  {
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
    child->SetValue (tokname);
    float x, y, z;
    csScanStr (params, "%f,%f,%f", &x, &y, &z);
    child->SetAttributeAsFloat ("red", x);
    child->SetAttributeAsFloat ("green", y);
    child->SetAttributeAsFloat ("blue", z);
    if (name && *name) child->SetAttribute ("name", name);
  }
  else
  {
    float f;
    csScanStr (params, "%f", &f);
    csRef<iDocumentNode> child = CreateValueNodeAsFloat (
	parent, tokname, f);
    if (name && *name) child->SetAttribute ("name", name);
  }
}

void Cs2Xml::ParseColor (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  float x, y, z;
  csScanStr (params, "%f,%f,%f", &x, &y, &z);
  child->SetAttributeAsFloat ("red", x);
  child->SetAttributeAsFloat ("green", y);
  child->SetAttributeAsFloat ("blue", z);
  if (name && *name) child->SetAttribute ("name", name);
}

void Cs2Xml::ParsePolygon (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue ("p");
  if (name && *name) child->SetAttribute ("name", name);
  ParseGeneral ("p", parser, child, params);
}

void Cs2Xml::ParseQ (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  float x, y, z, r;
  csScanStr (params, "%f,%f,%f,%f", &x, &y, &z, &r);
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  child->SetAttributeAsFloat ("x", x);
  child->SetAttributeAsFloat ("y", y);
  child->SetAttributeAsFloat ("z", z);
  child->SetAttributeAsFloat ("r", r);
  if (name && *name) child->SetAttribute ("name", name);
}

void Cs2Xml::ParseTimes (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  if (!strcmp (parent_token, "path"))
  {
    int n;
    float* list = new float[10000];
    csScanStr (params, "%F", list, &n);
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
    child->SetValue (tokname);
    if (name && *name) child->SetAttribute ("name", name);
    const char* tt;
    if (!strcmp (tokname, "speed"))
      tt = "s";
    else
      tt = "t";
    int i;
    for (i = 0 ; i < n ; i++)
      CreateValueNodeAsFloat (child, tt, list[i]);
    delete[] list;
  }
  else
  {
    float f;
    csScanStr (params, "%f", &f);
    csRef<iDocumentNode> child = CreateValueNodeAsFloat (
	parent, tokname, f);
    if (name && *name) child->SetAttribute ("name", name);
  }
}

void Cs2Xml::ParseRotPart (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  char meshName[100];
  int t;
  float angle_speed;
  csScanStr (params, "%d,%s,%f", &t, meshName, &angle_speed);
  CreateValueNode (child, "mesh", meshName);
  CreateValueNodeAsInt (child, "time", t);
  CreateValueNodeAsFloat (child, "anglespeed", angle_speed);
}

void Cs2Xml::ParseAttach (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  char meshName[100];
  char pathName[100];
  csScanStr (params, "%s,%s", meshName, pathName);
  CreateValueNode (child, "mesh", meshName);
  CreateValueNode (child, "path", pathName);
}

void Cs2Xml::ParsePath (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  if (!strcmp (parent_token, "sequences"))
  {
    ParseGeneral (tokname, parser, child, params);
  }
  else
  {
    char meshName[100];
    char pathName[100];
    int t;
    csScanStr (params, "%d,%s,%s", &t, meshName, pathName);
    CreateValueNode (child, "mesh", meshName);
    CreateValueNode (child, "path", pathName);
    CreateValueNodeAsInt (child, "time", t);
  }
}

void Cs2Xml::ParseSetupMesh(char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);

  char meshName[100];
  char sectName[100];
  csVector3 p;
  csScanStr (params, "%s,%s,%f,%f,%f", meshName, sectName,
      &p.x, &p.y, &p.z);
  CreateValueNode (child, "mesh", meshName);
  CreateValueNode (child, "sector", sectName);
  csRef<iDocumentNode> childchild =
      child->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  childchild->SetValue ("position");
  childchild->SetAttributeAsFloat ("x", p.x);
  childchild->SetAttributeAsFloat ("y", p.y);
  childchild->SetAttributeAsFloat ("z", p.z);
}

void Cs2Xml::ParseFade (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  float start, end;
  int t;
  csScanStr (params, "%d,%f,%f", &t, &start, &end);
  CreateValueNodeAsInt (child, "time", t);
  CreateValueNodeAsFloat (child, "start", start);
  CreateValueNodeAsFloat (child, "end", end);
}

void Cs2Xml::ParseFarPlane (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  float a, b, c, d;
  csScanStr (params, "%f,%f,%f,%f", &a, &b, &c, &d);
  child->SetAttributeAsFloat ("a", a);
  child->SetAttributeAsFloat ("b", b);
  child->SetAttributeAsFloat ("c", c);
  child->SetAttributeAsFloat ("d", d);
}

void Cs2Xml::ParseUVShift (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child;
  float u, v;
  csScanStr (params, "%f,%f", &u, &v);
  child = parent->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  child->SetAttributeAsFloat ("u", u);
  child->SetAttributeAsFloat ("v", v);
}

void Cs2Xml::ParseVector (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child;
  if (!strcmp (parent_token, "path") && !strcmp (tokname, "pos"))
  {
    // For sequences
    child = parent->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    child->SetValue (tokname);
    ParseGeneral (tokname, parser, child, params);
  }
  else
  {
    float x, y, z;
    int num = csScanStr (params, "%f,%f,%f", &x, &y, &z);
    if (num == 3)
    {
      child = parent->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
      child->SetValue (tokname);
      child->SetAttributeAsFloat ("x", x);
      child->SetAttributeAsFloat ("y", y);
      child->SetAttributeAsFloat ("z", z);
    }
    else
    {
      child = CreateValueNodeAsFloat (parent, tokname, x);
    }
  }
  if (name && *name) child->SetAttribute ("name", name);
}

void Cs2Xml::ParseCurveControl (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  float x, y, z, u, v;
  csScanStr (params, "%f,%f,%f:%f,%f", &x, &y, &z, &u, &v);
  child->SetAttributeAsFloat ("x", x);
  child->SetAttributeAsFloat ("y", y);
  child->SetAttributeAsFloat ("z", z);
  child->SetAttributeAsFloat ("u", u);
  child->SetAttributeAsFloat ("v", v);
  if (name && *name) child->SetAttribute ("name", name);
}

void Cs2Xml::ParseDuration (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue ("duration");
  CreateValueNode (child, "time", name);
  ParseGeneral (tokname, parser, child, params);
}

void Cs2Xml::ParseAnimate (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);

  char uvaniName[100];
  int timing;
  bool loop;
  csScanStr (params, "%s,%d,%b", uvaniName, &timing, &loop);
  child->SetAttribute ("name", uvaniName);
  CreateValueNodeAsInt (child, "timing", timing);
  CreateValueNode (child, "loop", loop ? "yes" : "no");
}

void Cs2Xml::ParseFrame (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (!strcmp (parent_token, "bone"))
  {
    CreateValueNode (child, "time", name);
  }
  else if (!strcmp (parent_token, "uvanimation"))
  {
    int duration;
    float u1, v1, u2, v2, u3, v3, u4, v4;
    csScanStr (params, "%d,%f,%f,%f,%f,%f,%f,%f,%f", 
	       &duration, &u1, &v1, &u2, &v2, &u3, &v3, &u4, &v4);

    CreateValueNodeAsInt (child, "duration", duration);
    csRef<iDocumentNode> childchild = child->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    childchild->SetValue ("v");
    childchild->SetAttributeAsFloat ("u", u1);
    childchild->SetAttributeAsFloat ("v", v1);
    childchild = child->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    childchild->SetValue ("v");
    childchild->SetAttributeAsFloat ("u", u2);
    childchild->SetAttributeAsFloat ("v", v2);
    childchild = child->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    childchild->SetValue ("v");
    childchild->SetAttributeAsFloat ("u", u3);
    childchild->SetAttributeAsFloat ("v", v3);
    childchild = child->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    childchild->SetValue ("v");
    childchild->SetAttributeAsFloat ("u", u4);
    childchild->SetAttributeAsFloat ("v", v4);
    return;
  }
  else
  {
    if (name && *name) child->SetAttribute ("name", name);
  }
  ParseGeneral (tokname, parser, child, params);
}

void Cs2Xml::ParseVertex (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue ("v");
  float x, y, z;
  csScanStr (params, "%f,%f,%f", &x, &y, &z);
  child->SetAttributeAsFloat ("x", x);
  child->SetAttributeAsFloat ("y", y);
  child->SetAttributeAsFloat ("z", z);
  if (name && *name) child->SetAttribute ("name", name);
}

void Cs2Xml::ParseV (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  if (strchr (params, ':'))
  {
    // For sprites or genmesh statement.
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
    child->SetValue (tokname);
    float x, y, z, u, v, nx, ny, nz;
    int num = csScanStr (params, "%f,%f,%f:%f,%f:%f,%f,%f",
      &x, &y, &z, &u, &v, &nx, &ny, &nz);
    child->SetAttributeAsFloat ("x", x);
    child->SetAttributeAsFloat ("y", y);
    child->SetAttributeAsFloat ("z", z);
    child->SetAttributeAsFloat ("u", u);
    child->SetAttributeAsFloat ("v", v);
    if (num > 5)
    {
      child->SetAttributeAsFloat ("nx", nx);
      child->SetAttributeAsFloat ("ny", ny);
      child->SetAttributeAsFloat ("nz", nz);
    }
    if (name && *name) child->SetAttribute ("name", name);
  }
  else if (!strcmp (parent_token, "p"))
  {
    // In this case we have a VERTICES from a POLYGON.
    int i;
    int list[1000];
    int num;
    csScanStr (params, "%D", list, &num);
    for (i = 0 ; i < num ; i++)
      CreateValueNodeAsInt (parent, "v", list[i]);
  }
  else
  {
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
    child->SetValue (tokname);
    float x, y, z;
    csScanStr (params, "%f,%f,%f", &x, &y, &z);
    child->SetAttributeAsFloat ("x", x);
    child->SetAttributeAsFloat ("y", y);
    child->SetAttributeAsFloat ("z", z);
    if (name && *name) child->SetAttribute ("name", name);
  }
}

void Cs2Xml::ParseColors (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  int i;
  float list[300];
  int num;
  csScanStr (params, "%F", list, &num);
  for (i = 0 ; i < num/3 ; i++)
  {
    csRef<iDocumentNode> colnode = parent->CreateNodeBefore (
	      CS_NODE_ELEMENT, NULL);
    colnode->SetValue ("color");
    colnode->SetAttributeAsFloat ("red", list[i*3+0]);
    colnode->SetAttributeAsFloat ("green", list[i*3+1]);
    colnode->SetAttributeAsFloat ("blue", list[i*3+2]);
  }
}

void Cs2Xml::ParseUVA (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  int i;
  float list[300];
  int num;
  csScanStr (params, "%F", list, &num);
  for (i = 0 ; i < num/3 ; i++)
  {
    csRef<iDocumentNode> uvnode = parent->CreateNodeBefore (
	      CS_NODE_ELEMENT, NULL);
    uvnode->SetValue ("uva");
    uvnode->SetAttributeAsFloat ("angle", list[i*3+0]);
    uvnode->SetAttributeAsFloat ("scale", list[i*3+1]);
    uvnode->SetAttributeAsFloat ("shift", list[i*3+2]);
  }
}

void Cs2Xml::ParseUV (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  if (!strcmp (parent_token, "p"))
  {
    // UV inside polygon.
    int i;
    float list[300];
    int num;
    csScanStr (params, "%F", list, &num);
    for (i = 0 ; i < num/2 ; i++)
    {
      csRef<iDocumentNode> uvnode = parent->CreateNodeBefore (
	      CS_NODE_ELEMENT, NULL);
      uvnode->SetValue ("uv");
      uvnode->SetAttributeAsFloat ("u", list[i*2+0]);
      uvnode->SetAttributeAsFloat ("v", list[i*2+1]);
    }
  }
  else if (!strcmp (parent_token, "texmap"))
  {
    // UV inside texture.
    int idx1, idx2, idx3;
    float u1, v1;
    float u2, v2;
    float u3, v3;
    csScanStr (params, "%d,%f,%f,%d,%f,%f,%d,%f,%f",
      &idx1, &u1, &v1, &idx2, &u2, &v2, &idx3, &u3, &v3);
    csRef<iDocumentNode> uvnode = parent->CreateNodeBefore (
	      CS_NODE_ELEMENT, NULL);
    uvnode->SetValue ("uv");
    uvnode->SetAttributeAsFloat ("idx", idx1);
    uvnode->SetAttributeAsFloat ("u", u1);
    uvnode->SetAttributeAsFloat ("v", v1);
    uvnode = parent->CreateNodeBefore (
	      CS_NODE_ELEMENT, NULL);
    uvnode->SetValue ("uv");
    uvnode->SetAttributeAsFloat ("idx", idx2);
    uvnode->SetAttributeAsFloat ("u", u2);
    uvnode->SetAttributeAsFloat ("v", v2);
    uvnode = parent->CreateNodeBefore (
	      CS_NODE_ELEMENT, NULL);
    uvnode->SetValue ("uv");
    uvnode->SetAttributeAsFloat ("idx", idx3);
    uvnode->SetAttributeAsFloat ("u", u3);
    uvnode->SetAttributeAsFloat ("v", v3);
  }
  else
  {
    int i;
    float list[300];
    int num;
    csScanStr (params, "%F", list, &num);
    for (i = 0 ; i < num/2 ; i++)
    {
      csRef<iDocumentNode> uvnode = parent->CreateNodeBefore (
	      CS_NODE_ELEMENT, NULL);
      uvnode->SetValue ("uv");
      uvnode->SetAttributeAsFloat ("u", list[i*2+0]);
      uvnode->SetAttributeAsFloat ("v", list[i*2+1]);
    }
  }
}

void Cs2Xml::ParseVertices (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  if (!strcmp (parent_token, "params") && strchr (params, ':'))
  {
    // In this case we have a VERTICES from a genmesh.
    // Here we don't generate a local node but just recurse
    // and add vertices that way.
    ParseGeneral (tokname, parser, parent, params);
  }
  else if (!strcmp (parent_token, "params"))
  {
    // In this case we have VERTICES from spr2d or bezier addon.
    // To disinguish that we'll look at the parent of the parent
    // to see if it is an ADDON or not.
    csRef<iDocumentNode> parparnode = parent->GetParent ();
    if (!strcmp (parparnode->GetValue (), "addon"))
    {
      // In this case we have VERTICES from a bezier.
      int i;
      int list[1000];
      int num;
      csScanStr (params, "%D", list, &num);
      for (i = 0 ; i < num ; i++)
	CreateValueNodeAsInt (parent, "v", list[i]);
    }
    else
    {
      int i;
      float list[200];
      int num;
      csScanStr (params, "%F", list, &num);
      for (i = 0 ; i < num/2 ; i++)
      {
	csRef<iDocumentNode> vnode = parent->CreateNodeBefore (
		CS_NODE_ELEMENT, NULL);
	vnode->SetValue ("v");
	vnode->SetAttributeAsFloat ("x", list[i*2+0]);
	vnode->SetAttributeAsFloat ("y", list[i*2+1]);
      }
    }
  }
  else
  {
    // In this case we have VERTICES from a POLYGON.
    int i;
    int list[1000];
    int num;
    csScanStr (params, "%D", list, &num);
    for (i = 0 ; i < num ; i++)
      CreateValueNodeAsInt (parent, "v", list[i]);
  }
}

void Cs2Xml::ParseTriangles (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  if (!strcmp (parent_token, "params"))
  {
    // In this case we have a TRIANGLES from a genmesh.
    // Here we don't generate a local node but just recurse
    // and add triangles that way.
    ParseGeneral (tokname, parser, parent, params);
  }
  else
  {
    // Does this ever occur???
    // @@@
    printf ("ERROR TRIANGLES!\n");
    exit (0);
  }
}

void Cs2Xml::ParseTexture (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  if (!strcmp (parent_token, "p"))
  {
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
    child->SetValue ("texmap");
    if (name && *name) child->SetAttribute ("name", name);
    ParseGeneral ("texmap", parser, child, params);
  }
  else if (!strcmp (parent_token, "material"))
  {
    char buf[2048];
    csScanStr (params, "%s", buf);
    CreateValueNode (parent, "texture", buf);
  }
  else if (!strcmp (parent_token, "layer"))
  {
    csRef<iDocumentNode> parparnode = parent->GetParent ();
    if (!strcmp (parparnode->GetValue (), "material"))
    {
      // Layer in a material.
      char buf[2048];
      csScanStr (params, "%s", buf);
      CreateValueNode (parent, "texture", buf);
    }
    else
    {
      // Layer in a heightgen.
      csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
      child->SetValue ("texture");
      if (name && *name) child->SetAttribute ("name", name);
      ParseGeneral ("texture", parser, child, params);
    }
  }
  else
  {
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
    child->SetValue ("texture");
    if (name && *name) child->SetAttribute ("name", name);
    ParseGeneral ("texture", parser, child, params);
  }
}

void Cs2Xml::ParseMatrix (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  ParseMatrix (parser, child, params);
}

void Cs2Xml::ParseFilename (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  char filename[2048];
  csScanStr (params, "%s", filename);
  CreateValueNode (parent, "file", filename);
}

void Cs2Xml::ParseTriangle (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  int list[1000];
  int num;
  csScanStr (params, "%D", list, &num);
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue ("t");
  child->SetAttributeAsInt ("v1", list[0]);
  child->SetAttributeAsInt ("v2", list[1]);
  child->SetAttributeAsInt ("v3", list[2]);
  if (name && *name) child->SetAttribute ("name", name);
}

void Cs2Xml::ParseSingle (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);

  char imagename[255];
  csVector2 scale, offset;
  csScanStr (params, "%s,%f,%f,%f,%f",
      imagename, &scale.x, &scale.y,
      &offset.x, &offset.y);
  CreateValueNode (child, "image", imagename);
  csRef<iDocumentNode> scalenode = child->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  scalenode->SetValue ("scale");
  scalenode->SetAttributeAsFloat ("x", scale.x);
  scalenode->SetAttributeAsFloat ("y", scale.y);
  csRef<iDocumentNode> offsetnode = child->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  offsetnode->SetValue ("offset");
  offsetnode->SetAttributeAsFloat ("x", offset.x);
  offsetnode->SetAttributeAsFloat ("y", offset.y);
}

void Cs2Xml::ParseSlope (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  if (!strcmp (parent_token, "params") && IsString (params) &&
      !strcmp (tokname, "heightmap"))
  {
    // We have a single heightmap string (for bcterr).
    char buf[2048];
    csScanStr (params, "%s", buf);
    CreateValueNode (parent, tokname, buf);
  }
  else
  {
    // A heightmap or slope for terrfunc or heightgen.
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
    child->SetValue (tokname);
    if (name && *name) child->SetAttribute ("name", name);
    char heightmap[255];
    float hscale, hshift;
    csScanStr (params, "%s,%f,%f", &heightmap, &hscale, &hshift);
    CreateValueNode (child, "image", heightmap);
    CreateValueNodeAsFloat (child, "scale", hscale);
    CreateValueNodeAsFloat (child, "shift", hshift);
  }
}

void Cs2Xml::ParseGenerate (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  int x, y;
  csScanStr (params, "%d,%d", &x, &y);
  child->SetAttributeAsInt ("x", x);
  child->SetAttributeAsInt ("y", y);
  if (name && *name) child->SetAttribute ("name", name);
}

void Cs2Xml::ParseHaloDefault(csRef<iDocumentNode> child, char* params)
{
  CreateValueNode (child, "type", "cross");
  float intensity = 2.0;
  float cross = 0.45;
  if (params)
    csScanStr (params, "%f,%f", &intensity, &cross);
  CreateValueNodeAsFloat (child, "intensity", intensity);
  CreateValueNodeAsFloat (child, "cross", cross);
}

void Cs2Xml::ParseHalo (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);

  char str[256];
  int cnt = csScanStr (params, "%s", str);
  if (cnt == 0 || !strcmp (str, "CROSS"))
  {
    params = strchr (params, ',');
    if (params) params++;
    ParseHaloDefault (child, params);
  }
  else if (!strcmp (str, "NOVA"))
  {
    params = strchr (params, ',');
    if (params) params++;
    CreateValueNode (child, "type", "nova");
    int seed = 0;
    int numSpokes = 100;
    float roundness = 0.5;
    if (params)
      csScanStr (params, "%d,%d,%f", &seed, &numSpokes, &roundness);
    CreateValueNodeAsInt (child, "seed", seed);
    CreateValueNodeAsInt (child, "numspokes", numSpokes);
    CreateValueNodeAsFloat (child, "roundness", roundness);
  }
  else if (!strcmp (str, "FLARE"))
  {
    CreateValueNode (child, "type", "flare");
    params = strchr (params, ',');
    if (params) params++;
    char mat_names[8][255];
    int cur_idx = 0;
    while (params && cur_idx < 6)
    {
      char* end = strchr (params, ',');
      int l;
      if (end) l = end-params;
      else l = strlen (params);
      strncpy (mat_names[cur_idx], params, l);
      mat_names[cur_idx][l] = 0;
      cur_idx++;
      params = end+1;
    }
    CreateValueNode (child, "centermaterial", mat_names[0]);
    CreateValueNode (child, "spark1material", mat_names[1]);
    CreateValueNode (child, "spark2material", mat_names[2]);
    CreateValueNode (child, "spark3material", mat_names[3]);
    CreateValueNode (child, "spark4material", mat_names[4]);
    CreateValueNode (child, "spark5material", mat_names[5]);
  }
  else
    ParseHaloDefault (child, params);
}

void Cs2Xml::ParseDoFlatten (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  bool t, r, d, l;
  csScanStr (params, "%b, %b, %b, %b", &t, &d, &l, &r);
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  if (t)
  {
    csRef<iDocumentNode> c = child->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
    c->SetValue ("up");
  }
  if (d)
  {
    csRef<iDocumentNode> c = child->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
    c->SetValue ("down");
  }
  if (l)
  {
    csRef<iDocumentNode> c = child->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
    c->SetValue ("left");
  }
  if (r)
  {
    csRef<iDocumentNode> c = child->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
    c->SetValue ("right");
  }
}

void Cs2Xml::ParseFlatten (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  float t, r, d, l;
  csScanStr (params, "%f, %f, %f, %f", &t, &d, &l, &r);
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  CreateValueNodeAsFloat (child, "up", t);
  CreateValueNodeAsFloat (child, "down", d);
  CreateValueNodeAsFloat (child, "left", l);
  CreateValueNodeAsFloat (child, "right", r);
}

void Cs2Xml::ParseFog (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  float r, g, b, density;
  csScanStr (params, "%f,%f,%f,%f", &r, &g, &b, &density);
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  child->SetAttributeAsFloat ("red", r);
  child->SetAttributeAsFloat ("green", g);
  child->SetAttributeAsFloat ("blue", b);
  child->SetAttributeAsFloat ("density", density);
}

void Cs2Xml::ParseKey (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  char Key  [256];
  char Value[10000];
  csScanStr (params, "%S,%S", Key, Value);
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  child->SetAttribute ("name", Key);
  child->SetAttribute ("value", Value);
}

void Cs2Xml::ParseAging (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  int time;
  float r, g, b, alpha, swirl, rotspeed, scale;
  csScanStr (params, "%d,%f,%f,%f,%f,%f,%f,%f", &time,
    &r, &g, &b, &alpha, &swirl, &rotspeed, &scale);
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  csRef<iDocumentNode> colornode = child->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  colornode->SetValue ("color");
  colornode->SetAttributeAsFloat ("red", r);
  colornode->SetAttributeAsFloat ("green", g);
  colornode->SetAttributeAsFloat ("blue", b);
  CreateValueNodeAsInt (child, "time", time);
  CreateValueNodeAsFloat (child, "alpha", alpha);
  CreateValueNodeAsFloat (child, "swirl", swirl);
  CreateValueNodeAsFloat (child, "rotspeed", rotspeed);
  CreateValueNodeAsFloat (child, "scale", scale);
}

void Cs2Xml::ParseRegularParticles (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  int sides;
  float radius;
  csScanStr (params, "%d,%f", &sides, &radius);
  child->SetAttributeAsInt ("sides", sides);
  child->SetAttributeAsFloat ("radius", radius);
}

void Cs2Xml::ParseIsoSpace (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  int minx=0,minz=0;
  float miny=0,maxy=0;
  csScanStr (params, "%d,%d,%f,%f", &minx, &minz, &miny, &maxy);
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  child->SetAttributeAsInt ("minx", minx);
  child->SetAttributeAsInt ("minz", minz);
  child->SetAttributeAsFloat ("miny", miny);
  child->SetAttributeAsFloat ("maxy", maxy);
}

void Cs2Xml::ParseIsoSize (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  if (!strcmp (parent_token, "grid"))	// Iso engine.
  {
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
    child->SetValue (tokname);
    if (name && *name) child->SetAttribute ("name", name);
    int x, z;
    csScanStr (params, "%d,%d", &x, &z);
    child->SetAttributeAsInt ("x", x);
    child->SetAttributeAsInt ("z", z);
  }
  else
    ParseParticleSize (parent_token, parser, parent, name, params, tokname);
}

void Cs2Xml::ParseParticleSize (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  if (!strcmp (parent_token, "mixmode"))
  {
    // Should be only for multiply.
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
    child->SetValue (tokname);
    if (name && *name) child->SetAttribute ("name", name);
    ParseGeneral (tokname, parser, child, params);
  }
  else
  {
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
    child->SetValue (tokname);
    if (name && *name) child->SetAttribute ("name", name);
    int w, h;
    csScanStr (params, "%d,%d", &w, &h);
    child->SetAttributeAsInt ("w", w);
    child->SetAttributeAsInt ("h", h);
  }
}

void Cs2Xml::ParseDropSize (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  float w, h;
  csScanStr (params, "%f,%f", &w, &h);
  child->SetAttributeAsFloat ("w", w);
  child->SetAttributeAsFloat ("h", h);
}

void Cs2Xml::ParseCorrectSeams (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  int w, h;
  csScanStr (params, "%d,%d", &w, &h);
  child->SetAttributeAsInt ("w", w);
  child->SetAttributeAsInt ("h", h);
}

void Cs2Xml::ParseBC (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  csVector3 cp;
  int x, z;
  csScanStr (params, "%d, %d, %f, %f, %f",
      &x, &z, &cp.x, &cp.y, &cp.z);
  child->SetAttributeAsFloat ("x", cp.x);
  child->SetAttributeAsFloat ("y", cp.y);
  child->SetAttributeAsFloat ("z", cp.z);
  child->SetAttributeAsInt ("cpx", x);
  child->SetAttributeAsInt ("cpy", z);
}

void Cs2Xml::ParseBH (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  float h;
  int x, z;
  csScanStr (params, "%d, %d, %f", &x, &z, &h );
  child->SetAttributeAsFloat ("height", h);
  child->SetAttributeAsInt ("cpx", x);
  child->SetAttributeAsInt ("cpy", z);
}

void Cs2Xml::ParseSysDist (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  float d, s;
  csScanStr (params, "%f,%f", &d, &s);
  CreateValueNodeAsFloat (child, "start", s);
  CreateValueNodeAsFloat (child, "distance", d);
}

void Cs2Xml::ParseGrid (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  if (!strcmp (parent_token, "grids"))	// Iso engine
  {
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
    child->SetValue (tokname);
    if (name && *name) child->SetAttribute ("name", name);
    ParseGeneral (tokname, parser, child, params);
  }
  else
    ParseBlocks (parent_token, parser, parent, name, params, tokname);
}

void Cs2Xml::ParseBlocks (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  int x, y;
  csScanStr (params, "%d,%d", &x, &y);
  child->SetAttributeAsInt ("x", x);
  child->SetAttributeAsInt ("y", y);
}

void Cs2Xml::ParseEmitCylinder (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  float x1, y1, z1, x2, y2, z2, p, q;
  csScanStr (params, "%f,%f,%f,%f,%f,%f,%f,%f", &x1, &y1, &z1,
      &x2, &y2, &z2, &p, &q);
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  csRef<iDocumentNode> minnode = child->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  minnode->SetValue ("min");
  minnode->SetAttributeAsFloat ("x", x1);
  minnode->SetAttributeAsFloat ("y", y1);
  minnode->SetAttributeAsFloat ("z", z1);
  csRef<iDocumentNode> maxnode = child->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  maxnode->SetValue ("max");
  maxnode->SetAttributeAsFloat ("x", x2);
  maxnode->SetAttributeAsFloat ("y", y2);
  maxnode->SetAttributeAsFloat ("z", z2);
  child->SetAttributeAsFloat ("p", p);
  child->SetAttributeAsFloat ("q", q);
}

void Cs2Xml::ParseHazeCone (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  int number;
  float x1, y1, z1, x2, y2, z2, p, q;
  csScanStr (params, "%d, %f,%f,%f,%f,%f,%f, %f, %f", &number,
    &x1, &y1, &z1, &x2, &y2, &z2, &p, &q);
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  csRef<iDocumentNode> minnode = child->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  minnode->SetValue ("min");
  minnode->SetAttributeAsFloat ("x", x1);
  minnode->SetAttributeAsFloat ("y", y1);
  minnode->SetAttributeAsFloat ("z", z1);
  csRef<iDocumentNode> maxnode = child->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  maxnode->SetValue ("max");
  maxnode->SetAttributeAsFloat ("x", x2);
  maxnode->SetAttributeAsFloat ("y", y2);
  maxnode->SetAttributeAsFloat ("z", z2);
  child->SetAttributeAsFloat ("p", p);
  child->SetAttributeAsFloat ("q", q);
  child->SetAttributeAsInt ("number", number);
}

void Cs2Xml::ParseBox (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  float x1, y1, z1, x2, y2, z2;
  csScanStr (params, "%f,%f,%f,%f,%f,%f", &x1, &y1, &z1,
      &x2, &y2, &z2);
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  csRef<iDocumentNode> minnode = child->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  minnode->SetValue ("min");
  minnode->SetAttributeAsFloat ("x", x1);
  minnode->SetAttributeAsFloat ("y", y1);
  minnode->SetAttributeAsFloat ("z", z1);
  csRef<iDocumentNode> maxnode = child->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  maxnode->SetValue ("max");
  maxnode->SetAttributeAsFloat ("x", x2);
  maxnode->SetAttributeAsFloat ("y", y2);
  maxnode->SetAttributeAsFloat ("z", z2);
}

void Cs2Xml::ParseEmitFixed (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  float x1, y1, z1;
  csScanStr (params, "%f,%f,%f", &x1, &y1, &z1);
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  child->SetAttributeAsFloat ("x", x1);
  child->SetAttributeAsFloat ("y", y1);
  child->SetAttributeAsFloat ("z", z1);
}

void Cs2Xml::ParseEmitSphere (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  float x1, y1, z1, p, q;
  csScanStr (params, "%f,%f,%f,%f,%f", &x1, &y1, &z1, &p, &q);
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  child->SetAttributeAsFloat ("x", x1);
  child->SetAttributeAsFloat ("y", y1);
  child->SetAttributeAsFloat ("z", z1);
  child->SetAttributeAsFloat ("p", p);
  child->SetAttributeAsFloat ("q", q);
}

void Cs2Xml::ParseF (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  char framename[256];
  int time;
  csScanStr (params, "%s,%d", framename, &time);
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  child->SetAttribute ("name", framename);
  child->SetAttributeAsInt ("delay", time);
}

void Cs2Xml::ParseNum (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  if (strchr (params, ','))
  {
    // For genmesh.
    int vt, tri;
    csScanStr (params, "%d,%d", &vt, &tri);
    CreateValueNodeAsInt (parent, "numvt", vt);
    CreateValueNodeAsInt (parent, "numtri", tri);
  }
  else
  {
    int num;
    csScanStr (params, "%d", &num);
    CreateValueNodeAsInt (parent, tokname, num);
  }
}

void Cs2Xml::ParsePriority (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  if (!strcmp (parent_token, "renderpriorities"))
  {
    long pri;
    char sorting[100];
    csScanStr (params, "%d,%s", &pri, sorting);
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
    child->SetValue (tokname);
    child->SetAttribute ("name", name);
    CreateValueNodeAsInt (child, "level", pri);
    CreateValueNode (child, "sort", sorting);
  }
  else
  {
    char priority[100];
    csScanStr (params, "%s", priority);
    CreateValueNode (parent, tokname, priority);
  }
}

void Cs2Xml::ParseStart (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  if (!strcmp (parent_token, "tile2d"))	// Iso engine.
  {
    float x, y, z;
    csScanStr (params, "%f,%f,%f", &x, &y, &z);
    child->SetAttributeAsFloat ("x", x);
    child->SetAttributeAsFloat ("y", y);
    child->SetAttributeAsFloat ("z", z);
  }
  else
  {
    if (strchr (params, '('))
    {
      // New format.
      ParseGeneral (tokname, parser, child, params);
    }
    else
    {
      // Old format.
      char start_sector[1000];
      csVector3 pos;
      csScanStr (params, "%s,%f,%f,%f", &start_sector,
	&pos.x, &pos.y, &pos.z);
      CreateValueNode (child, "sector", start_sector);
      csRef<iDocumentNode> childchild;
      childchild = child->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
      childchild->SetValue ("position");
      childchild->SetAttributeAsFloat ("x", pos.x);
      childchild->SetAttributeAsFloat ("y", pos.y);
      childchild->SetAttributeAsFloat ("z", pos.z);
    }
  }
}

void Cs2Xml::ParseLOD (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);

  float distance;
  int inc;
  int num = csScanStr (params, "%f,%d", &distance, &inc);
  if (!strcmp (parent_token, "params") && num == 2)
  {
    // Most likely lod for a bcterr.
    CreateValueNodeAsFloat (child, "distance", distance);
    CreateValueNodeAsInt (child, "inc", inc);
  }
  else
  {
    // A lod for a general mesh object or factory.
    ParseGeneral (tokname, parser, child, params);
  }
}

void Cs2Xml::ParseLODDistance (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  int lod;
  float dist;
  csScanStr (params, "%d,%f", &lod, &dist);
  child->SetAttributeAsInt ("level", lod);
  child->SetAttributeAsFloat ("distance", dist);
}

void Cs2Xml::ParseLODCost (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  int lod;
  float cost;
  csScanStr (params, "%d,%f", &lod, &cost);
  child->SetAttributeAsInt ("level", lod);
  child->SetAttributeAsFloat ("cost", cost);
}

void Cs2Xml::ParseDirLight (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  csVector3 pos;
  csColor col;
  csScanStr (params, "%f,%f,%f:%f,%f,%f",
      &pos.x, &pos.y, &pos.z,
      &col.red, &col.green, &col.blue);
  csRef<iDocumentNode> childchild;
  childchild = child->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  childchild->SetValue ("position");
  childchild->SetAttributeAsFloat ("x", pos.x);
  childchild->SetAttributeAsFloat ("y", pos.y);
  childchild->SetAttributeAsFloat ("z", pos.z);
  childchild = child->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
  childchild->SetValue ("color");
  childchild->SetAttributeAsFloat ("red", col.red);
  childchild->SetAttributeAsFloat ("green", col.green);
  childchild->SetAttributeAsFloat ("blue", col.blue);
}

void Cs2Xml::ParseLight (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  if (strchr (params, ':'))
  {
    int d;
    csVector3 pos;
    float dist;
    csColor col;
    csScanStr (params, "%f,%f,%f:%f,%f,%f,%f,%d",
      &pos.x, &pos.y, &pos.z, &dist,
      &col.red, &col.green, &col.blue, &d);
    bool dyn = bool (d);
    csRef<iDocumentNode> childchild;
    childchild = child->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    childchild->SetValue ("center");
    childchild->SetAttributeAsFloat ("x", pos.x);
    childchild->SetAttributeAsFloat ("y", pos.y);
    childchild->SetAttributeAsFloat ("z", pos.z);
    CreateValueNodeAsFloat (child, "radius", dist);
    childchild = child->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    childchild->SetValue ("color");
    childchild->SetAttributeAsFloat ("red", col.red);
    childchild->SetAttributeAsFloat ("green", col.green);
    childchild->SetAttributeAsFloat ("blue", col.blue);
    if (dyn)
    {
      childchild = child->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
      childchild->SetValue ("dynamic");
    }
  }
  else
  {
    ParseGeneral (tokname, parser, child, params);
  }
}

void Cs2Xml::ParseTransparent (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
      CS_NODE_ELEMENT, NULL);
  child->SetValue (tokname);
  if (name && *name) child->SetAttribute ("name", name);
  float r, g, b;
  csScanStr (params, "%f,%f,%f", &r, &g, &b);
  child->SetAttributeAsFloat ("red", r);
  child->SetAttributeAsFloat ("green", g);
  child->SetAttributeAsFloat ("blue", b);
}

void Cs2Xml::ParseScale (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  csRef<iDocumentNode> child;
  float x, y, z;
  int num = csScanStr (params, "%f,%f,%f", &x, &y, &z);
  if (num == 3)
  {
    child = parent->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
    child->SetValue (tokname);
    child->SetAttributeAsFloat ("x", x);
    child->SetAttributeAsFloat ("y", y);
    child->SetAttributeAsFloat ("z", z);
  }
  else
  {
    num = csScanStr (params, "%f,%f", &x, &y);
    if (num == 2)
    {
      child = parent->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
      child->SetValue (tokname);
      child->SetAttributeAsFloat ("u", x);
      child->SetAttributeAsFloat ("v", y);
    }
    else
    {
      child = CreateValueNodeAsFloat (parent, tokname, x);
    }
  }
}

void Cs2Xml::ParseType (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  char buf[2048];
  csScanStr (params, "%s", buf);
  char* tt = ToLower (buf, true);
  CreateValueNode (parent, tokname, tt);
  delete[] tt;
}

void Cs2Xml::ParseOther (char const* parent_token, csParser* parser,
	csRef<iDocumentNode>& parent, char*& name, char* params,
	char const* tokname)
{
  bool val;
  if (IsEmpty (params))
  {
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
    child->SetValue (tokname);
    if (name && *name) child->SetAttribute ("name", name);
  }
  else if (IsNumeric (params))
  {
    float f;
    csScanStr (params, "%f", &f);
    csRef<iDocumentNode> child = CreateValueNodeAsFloat (
	parent, tokname, f);
    if (name && *name) child->SetAttribute ("name", name);
  }
  else if (IsBoolean (params, val))
  {
    csRef<iDocumentNode> child = CreateValueNode (
	parent, tokname, val ? "yes" : "no");
    if (name && *name) child->SetAttribute ("name", name);
  }
  else if (IsString (params))
  {
    char buf[2048];
    csScanStr (params, "%s", buf);
    csRef<iDocumentNode> child = CreateValueNode (parent, tokname, buf);
    if (name && *name) child->SetAttribute ("name", name);
  }
  else
  {
    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	CS_NODE_ELEMENT, NULL);
    child->SetValue (tokname);
    if (name && *name) child->SetAttribute ("name", name);
    ParseGeneral (tokname, parser, child, params);
  }
}

void Cs2Xml::ParseGeneral (const char* parent_token,
	csParser* parser, csRef<iDocumentNode>& parent, char* buf)
{
  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (ACCEL)
    CS_TOKEN_TABLE (AGING)
    CS_TOKEN_TABLE (ALLMATERIAL)
    CS_TOKEN_TABLE (AMBIENT)
    CS_TOKEN_TABLE (ANIMATE)
    CS_TOKEN_TABLE (ATTACH)
    CS_TOKEN_TABLE (BASECOLOR)
    CS_TOKEN_TABLE (BC)
    CS_TOKEN_TABLE (BH)
    CS_TOKEN_TABLE (BLOCKS)
    CS_TOKEN_TABLE (BLOCKSIZE)
    CS_TOKEN_TABLE (BOX)
    CS_TOKEN_TABLE (C)
    CS_TOKEN_TABLE (CENTER)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (COLORS)
    CS_TOKEN_TABLE (CORRECTSEAMS)
    CS_TOKEN_TABLE (CURVECENTER)
    CS_TOKEN_TABLE (CURVECONTROL)
    CS_TOKEN_TABLE (DIRECTION)
    CS_TOKEN_TABLE (DIRECTIONAL)
    CS_TOKEN_TABLE (DIRLIGHT)
    CS_TOKEN_TABLE (DOFLATTEN)
    CS_TOKEN_TABLE (DROPSIZE)
    CS_TOKEN_TABLE (DURATION)
    CS_TOKEN_TABLE (EMITBOX)
    CS_TOKEN_TABLE (EMITCYLINDER)
    CS_TOKEN_TABLE (EMITCYLINDERTANGENT)
    CS_TOKEN_TABLE (EMITFIXED)
    CS_TOKEN_TABLE (EMITLINE)
    CS_TOKEN_TABLE (EMITSPHERE)
    CS_TOKEN_TABLE (END)
    CS_TOKEN_TABLE (EULER)
    CS_TOKEN_TABLE (F)
    CS_TOKEN_TABLE (FADE)
    CS_TOKEN_TABLE (FALLSPEED)
    CS_TOKEN_TABLE (FARPLANE)
    CS_TOKEN_TABLE (FILE)
    CS_TOKEN_TABLE (FIRST)
    CS_TOKEN_TABLE (FLATTEN)
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (FORWARD)
    CS_TOKEN_TABLE (FRAME)
    CS_TOKEN_TABLE (GENERATE)
    CS_TOKEN_TABLE (GRID)
    CS_TOKEN_TABLE (GROUPMATERIAL)
    CS_TOKEN_TABLE (HALO)
    CS_TOKEN_TABLE (HAZEBOX)
    CS_TOKEN_TABLE (HAZECONE)
    CS_TOKEN_TABLE (HEIGHTMAP)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (LIGHT)
    CS_TOKEN_TABLE (LOD)
    CS_TOKEN_TABLE (LODCOST)
    CS_TOKEN_TABLE (LODDIST)
    CS_TOKEN_TABLE (MATERIAL)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (MAXCOLOR)
    CS_TOKEN_TABLE (MULT)
    CS_TOKEN_TABLE (MULTIPLY)
    CS_TOKEN_TABLE (NUM)
    CS_TOKEN_TABLE (ORIG)
    CS_TOKEN_TABLE (ORIGIN)
    CS_TOKEN_TABLE (ORIGINBOX)
    CS_TOKEN_TABLE (PARTSIZE)
    CS_TOKEN_TABLE (PATH)
    CS_TOKEN_TABLE (POLYGON)
    CS_TOKEN_TABLE (PORTAL)
    CS_TOKEN_TABLE (POS)
    CS_TOKEN_TABLE (POSITION)
    CS_TOKEN_TABLE (PRIORITY)
    CS_TOKEN_TABLE (PUSH)
    CS_TOKEN_TABLE (Q)
    CS_TOKEN_TABLE (RADIUS)
    CS_TOKEN_TABLE (RECTPARTICLES)
    CS_TOKEN_TABLE (REGULARPARTICLES)
    CS_TOKEN_TABLE (ROT)
    CS_TOKEN_TABLE (ROTPART)
    CS_TOKEN_TABLE (SCALE)
    CS_TOKEN_TABLE (SECOND)
    CS_TOKEN_TABLE (SETUPMESH)
    CS_TOKEN_TABLE (SHIFT)
    CS_TOKEN_TABLE (SINGLE)
    CS_TOKEN_TABLE (SIZE)
    CS_TOKEN_TABLE (SLOPE)
    CS_TOKEN_TABLE (SOLID)
    CS_TOKEN_TABLE (SOURCE)
    CS_TOKEN_TABLE (SPACE)
    CS_TOKEN_TABLE (SPEED)
    CS_TOKEN_TABLE (START)
    CS_TOKEN_TABLE (SYSDIST)
    CS_TOKEN_TABLE (T)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (TIMES)
    CS_TOKEN_TABLE (TOPLEFT)
    CS_TOKEN_TABLE (TRANSPARENT)
    CS_TOKEN_TABLE (TRIANGLE)
    CS_TOKEN_TABLE (TRIANGLES)
    CS_TOKEN_TABLE (TYPE)
    CS_TOKEN_TABLE (UP)
    CS_TOKEN_TABLE (UV)
    CS_TOKEN_TABLE (UVEC)
    CS_TOKEN_TABLE (UV_SHIFT)
    CS_TOKEN_TABLE (UVA)
    CS_TOKEN_TABLE (V)
    CS_TOKEN_TABLE (VVEC)
    CS_TOKEN_TABLE (VERTEX)
    CS_TOKEN_TABLE (VERTICES)
    CS_TOKEN_TABLE (W)
    CS_TOKEN_TABLE (WARP)
  CS_TOKEN_TABLE_END

  char *name, *params;
  long cmd;
  csRef<iDocumentNode> portal_node;

  while ((cmd = parser->GetObject (&buf, tokens, &name, &params))
  	!= CS_PARSERR_EOF)
  {
    char* tokname = ToLower (parser->GetUnknownToken (), true);
    switch (cmd)
    {
      case CS_TOKEN_PORTAL:
	ParsePortal (parent_token, parser, parent, name, params, 
	  tokname, portal_node);
	break;
      case CS_TOKEN_WARP:
	ParseWarp (parent_token, parser, parent, name, params, 
	  tokname, portal_node);
	break;
      case CS_TOKEN_ALLMATERIAL:
      case CS_TOKEN_MATERIAL:
	ParseMaterial (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_GROUPMATERIAL:
	ParseMaterialGroup (parent_token, parser, parent, name, params,
	  tokname);
	break;
      case CS_TOKEN_AMBIENT:
	ParseAmbient (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_SOLID:
      case CS_TOKEN_BASECOLOR:
      case CS_TOKEN_MAXCOLOR:
      case CS_TOKEN_COLOR:
	ParseColor (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_POLYGON:
	ParsePolygon (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_Q:
	ParseQ (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_SPEED:	// For sequences
      case CS_TOKEN_TIMES:	// For sequences
	ParseTimes (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_ROTPART:	// For sequences
	ParseRotPart (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_ATTACH:	// For sequences
	ParseAttach (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_PATH:	// For sequences
	ParsePath (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_SETUPMESH:	// For sequences
	ParseSetupMesh (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_FADE:	// For sequences
	ParseFade (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_FARPLANE:
	ParseFarPlane (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_C:
      case CS_TOKEN_W:
      case CS_TOKEN_UVEC:
      case CS_TOKEN_VVEC:
      case CS_TOKEN_FORWARD:
      case CS_TOKEN_UP:
      case CS_TOKEN_EULER:
      case CS_TOKEN_END:
      case CS_TOKEN_TOPLEFT:
      case CS_TOKEN_DIRECTION:
      case CS_TOKEN_DIRECTIONAL:
      case CS_TOKEN_SOURCE:
      case CS_TOKEN_RADIUS:
      case CS_TOKEN_CENTER:
      case CS_TOKEN_ACCEL:
      case CS_TOKEN_FALLSPEED:
      case CS_TOKEN_FIRST:
      case CS_TOKEN_SECOND:
      case CS_TOKEN_ORIG:
      case CS_TOKEN_ORIGIN:
      case CS_TOKEN_POS:
      case CS_TOKEN_POSITION:
      case CS_TOKEN_PUSH:
      case CS_TOKEN_SHIFT:
      case CS_TOKEN_CURVECENTER:
	ParseVector (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_UV_SHIFT:
	ParseUVShift (parent_token, parser, parent, name, params, "uvshift");
	break;
      case CS_TOKEN_CURVECONTROL:
	ParseCurveControl (parent_token, parser, parent, name, params,
	  tokname);
	break;
      case CS_TOKEN_DURATION:
	ParseDuration (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_FRAME:
	ParseFrame (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_VERTEX:
	ParseVertex (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_V:
	ParseV (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_COLORS:
	ParseColors (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_UVA:
	ParseUVA (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_UV:
	ParseUV (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_VERTICES:
	ParseVertices (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_TRIANGLES:
	ParseTriangles (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_TEXTURE:
	ParseTexture (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_MATRIX:
	ParseMatrix (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_FILE:
	ParseFilename (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_T:
      case CS_TOKEN_TRIANGLE:
	ParseTriangle (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_SINGLE:
	ParseSingle (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_SLOPE:
      case CS_TOKEN_HEIGHTMAP:
	ParseSlope (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_GENERATE:
	ParseGenerate (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_HALO:
	ParseHalo (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_DOFLATTEN:
	ParseDoFlatten (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_FLATTEN:
	ParseFlatten (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_FOG:
	ParseFog (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_KEY:
	ParseKey (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_AGING:
	ParseAging (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_REGULARPARTICLES:
	ParseRegularParticles (parent_token, parser, parent, name, params,
	  tokname);
	break;
      case CS_TOKEN_SPACE: // Iso engine.
	ParseIsoSpace (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_MULT:
      case CS_TOKEN_SIZE:
	ParseIsoSize (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_PARTSIZE:
      case CS_TOKEN_MULTIPLY:
	ParseParticleSize (parent_token, parser, parent, name, params,
	  tokname);
	break;
      case CS_TOKEN_RECTPARTICLES:
      case CS_TOKEN_DROPSIZE:
	ParseDropSize (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_CORRECTSEAMS:
	ParseCorrectSeams (parent_token, parser, parent, name, params,
	  tokname);
	break;
      case CS_TOKEN_BC:	// bcterr
	ParseBC (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_BH:	// bcterr
	ParseBH (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_SYSDIST:	// bcterr
	ParseSysDist (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_GRID:
	ParseGrid (parent_token, parser, parent, name, params, tokname);
	break;
      case CS_TOKEN_BLOCKSIZE:
      case CS_TOKEN_BLOCKS:
	ParseBlocks (parent_token, parser, parent, name, params, tokname);
	break;
      default:
	ParseGeneral1 (cmd, parent_token, parser, parent, name, params,
	  tokname);
	break;
    }
    delete[] tokname;
  }
}

// This function exists solely to appease the MacOS/X assembler which can not
// deal with the extremely large switch() statement in ParseGeneral().  The
// assembler uses only 16-bit jump table offsets for the switch, but the switch
// statement is so large that the offsets exceed 16-bits.  Unfortunately, there
// doesn't seem to be any way to instruct the assembler to use 32-bit offsets.
// This function, ParseGeneral1(), is merely a continuation of the switch()
// statement in ParseGeneral().
void Cs2Xml::ParseGeneral1 (long cmd, char const* parent_token,
	csParser* parser, csRef<iDocumentNode>& parent, char*& name,
	char* params, char const* tokname)
{
  switch (cmd)
  {
    case CS_TOKEN_ANIMATE:
      ParseAnimate (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_EMITCYLINDER:
    case CS_TOKEN_EMITCYLINDERTANGENT:
      ParseEmitCylinder (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_HAZECONE:
      ParseHazeCone (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_HAZEBOX:
    case CS_TOKEN_BOX:
    case CS_TOKEN_ORIGINBOX:
    case CS_TOKEN_EMITBOX:
    case CS_TOKEN_EMITLINE:
      ParseBox (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_EMITFIXED:
      ParseEmitFixed (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_EMITSPHERE:
      ParseEmitSphere (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_F:
      ParseF (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_NUM:
      ParseNum (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_PRIORITY:
      ParsePriority (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_START:
      ParseStart (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_LOD:
      ParseLOD (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_LODDIST:
      ParseLODDistance (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_LODCOST:
      ParseLODCost (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_DIRLIGHT:
      ParseDirLight (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_LIGHT:
      ParseLight (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_TRANSPARENT:
      ParseTransparent (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_SCALE:
      ParseScale (parent_token, parser, parent, name, params, tokname);
      break;
    case CS_TOKEN_TYPE:
      ParseType (parent_token, parser, parent, name, params, tokname);
      break;
    default:
      ParseOther (parent_token, parser, parent, name, params, tokname);
      break;
  }
}

void Cs2Xml::ConvertDir (const char* vfspath, bool backup)
{
  if (strstr (vfspath, "/cache/") != NULL) return;
  vfs->PushDir ();
  vfs->ChDir (vfspath);
  csRef<iStrVector> files (vfs->FindFiles ("."));
  int i;
  for (i = 0 ; i < files->Length () ; i++)
  {
    char* str = files->Get (i);

    // Test if it is a dir (@@@ rather ugly test! Needs support in VFS).
    csRef<iStrVector> recfiles (vfs->FindFiles (str));
    if (recfiles->Length () > 0 && strcmp (recfiles->Get (0), str) != 0)
    {
      ConvertDir (str, backup);
    }
    else
    {
      ConvertFile (str, backup);
    }
  }
  vfs->PopDir ();
}

bool Cs2Xml::ConvertFile (const char* vfspath, bool backup)
{
  if (!TestCSFile (vfspath)) return false;

  printf ("Trying to convert '%s' ... \n", vfspath); fflush (stdout);
  csRef<iDataBuffer> buf (vfs->ReadFile (vfspath));
  if (!buf || !buf->GetSize ())
  {
    ReportError ("Could not read file '%s'!", vfspath);
    return false;
  }

  // First make backup
  if (backup)
  {
    char* newname = new char [strlen (vfspath)+5];
    strcpy (newname, vfspath);
    strcat (newname, ".bak");
    vfs->WriteFile (newname, **buf, buf->GetSize ());
    delete[] newname;
  }

  csParser* parser = new csParser (true);
  parser->ResetParserLine ();

  CS_TOKEN_TABLE_START (tokens)
  CS_TOKEN_TABLE_END

  char *data = **buf;
  char *name, *params;
  long cmd;

  if ((cmd = parser->GetObject (&data, tokens, &name, &params))
  	!= CS_PARSERR_EOF)
  {
    if (params)
    {
      char* tokname = ToLower (parser->GetUnknownToken (), true);
      csRef<iDocumentSystem> xml (csPtr<iDocumentSystem> (
      	new csTinyDocumentSystem ()));
      csRef<iDocument> doc = xml->CreateDocument ();
      csRef<iDocumentNode> root = doc->CreateRoot ();
      csRef<iDocumentNode> parent = root->CreateNodeBefore (
    	    CS_NODE_ELEMENT, NULL);
      parent->SetValue (tokname);
      ParseGeneral (tokname, parser, parent, params);

      doc->Write (vfs, vfspath);
      delete[] tokname;
    }
  }

  delete parser;
  return true;
}

bool Cs2Xml::TestCSFile (const char* vfspath)
{
  char* ext = strrchr (vfspath, '.');
  if (ext)
  {
    if (!strcasecmp (ext, ".gif")) return false;
    if (!strcasecmp (ext, ".jpg")) return false;
    if (!strcasecmp (ext, ".jpeg")) return false;
    if (!strcasecmp (ext, ".tga")) return false;
    if (!strcasecmp (ext, ".pcx")) return false;
    if (!strcasecmp (ext, ".png")) return false;
    if (!strcasecmp (ext, ".jng")) return false;

    if (!strcasecmp (ext, ".mpg")) return false;
    if (!strcasecmp (ext, ".avi")) return false;

    if (!strcasecmp (ext, ".wav")) return false;

    if (!strcasecmp (ext, ".bak")) return false;
  }

  csRef<iDataBuffer> buf (vfs->ReadFile (vfspath));
  if (!buf || !buf->GetSize ()) return false;

  csParser* parser = new csParser (true);
  parser->ResetParserLine ();

  CS_TOKEN_TABLE_START (tokens)
  CS_TOKEN_TABLE_END

  char *data = **buf;
  char *name, *params;
  long cmd;

  if ((cmd = parser->GetObject (&data, tokens, &name, &params))
  	!= CS_PARSERR_EOF)
  {
    if (params)
    {
      char* tokname = ToLower (parser->GetUnknownToken (), true);
      bool is_cs = !strcmp (tokname, "world") ||
      		   !strcmp (tokname, "library") ||
      		   !strcmp (tokname, "meshobj") ||
      		   !strcmp (tokname, "sequences") ||
      		   !strcmp (tokname, "path") ||
      		   !strcmp (tokname, "params") ||
      		   !strcmp (tokname, "meshfact");
      delete[] tokname;
      if (is_cs)
      {
        char* params2;
        cmd = parser->GetObject (&params, tokens, &name, &params2);
        if (cmd != CS_PARSERR_EOF && params2 != NULL)
        {
	  delete parser;
	  return true;
        }
      }
    }
  }

  delete parser;
  return false;
}

//----------------------------------------------------------------------------

void Cs2Xml::Main ()
{
  cmdline = CS_QUERY_REGISTRY (object_reg, iCommandLineParser);
  vfs = CS_QUERY_REGISTRY (object_reg, iVFS);

  const char* val = cmdline->GetName ();
  if (!val)
  {
    ReportError ("Please give VFS world file name or name of the zip archive!");
    return;
  }

  if (strstr (val, ".zip"))
  {
    vfs->Mount ("/tmp/cs2xml_data", val);
    ConvertDir ("/tmp/cs2xml_data", true);
  }
  else
  {
    ConvertFile (val, true);
  }

  vfs->Sync ();
}

/*---------------------------------------------------------------------*
 * Main function
 *---------------------------------------------------------------------*/
int main (int argc, char* argv[])
{
  // Initialize the random number generator
  srand (time (NULL));

  iObjectRegistry* object_reg = csInitializer::CreateEnvironment (argc, argv);
  if (!object_reg)
    return -1;

  if (!csInitializer::RequestPlugins (object_reg,
	CS_REQUEST_VFS,
	CS_REQUEST_END))
    return -1;

  Cs2Xml* lt = new Cs2Xml (object_reg);
  lt->Main ();
  delete lt;

  csInitializer::DestroyApplication (object_reg);

  return 0;
}

