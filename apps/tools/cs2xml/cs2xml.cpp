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
#include "csutil/util.h"
#include "csutil/parser.h"
#include "csutil/indprint.h"
#include "csutil/scanstr.h"
#include "csutil/csstring.h"
#include "csutil/xmltiny.h"
#include "iutil/document.h"
#include "iutil/objreg.h"
#include "iutil/vfs.h"
#include "iutil/cmdline.h"
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
  CS_TOKEN_DEF (BOX)
  CS_TOKEN_DEF (CURVECENTER)
  CS_TOKEN_DEF (CURVECONTROL)
  CS_TOKEN_DEF (COLOR)
  CS_TOKEN_DEF (COLORS)
  CS_TOKEN_DEF (DROPSIZE)
  CS_TOKEN_DEF (EMITBOX)
  CS_TOKEN_DEF (EMITFIXED)
  CS_TOKEN_DEF (F)
  CS_TOKEN_DEF (FALLSPEED)
  CS_TOKEN_DEF (FILE)
  CS_TOKEN_DEF (FIRST)
  CS_TOKEN_DEF (FOG)
  CS_TOKEN_DEF (IDENTITY)
  CS_TOKEN_DEF (KEY)
  CS_TOKEN_DEF (LIGHT)
  CS_TOKEN_DEF (MATRIX)
  CS_TOKEN_DEF (NUM)
  CS_TOKEN_DEF (ORIG)
  CS_TOKEN_DEF (ORIGIN)
  CS_TOKEN_DEF (PORTAL)
  CS_TOKEN_DEF (POSITION)
  CS_TOKEN_DEF (PRIORITY)
  CS_TOKEN_DEF (RADIUS)
  CS_TOKEN_DEF (RECTPARTICLES)
  CS_TOKEN_DEF (ROT)
  CS_TOKEN_DEF (ROT_X)
  CS_TOKEN_DEF (ROT_Y)
  CS_TOKEN_DEF (ROT_Z)
  CS_TOKEN_DEF (SCALE)
  CS_TOKEN_DEF (SCALE_X)
  CS_TOKEN_DEF (SCALE_Y)
  CS_TOKEN_DEF (SCALE_Z)
  CS_TOKEN_DEF (SECOND)
  CS_TOKEN_DEF (SHIFT)
  CS_TOKEN_DEF (T)
  CS_TOKEN_DEF (TEXTURE)
  CS_TOKEN_DEF (TRANSPARENT)
  CS_TOKEN_DEF (TRIANGLE)
  CS_TOKEN_DEF (TYPE)
  CS_TOKEN_DEF (UV)
  CS_TOKEN_DEF (V)
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

char* Cs2Xml::ToLower (const char* in)
{
  char* rc = new char [strlen (in)+1];
  char* out = rc;
  while (*in)
  {
    *out = tolower (*in);
    out++;
    in++;
  }
  *out = 0;
  return rc;
}

void Cs2Xml::CreateValueNode (csRef<iDocumentNode>& parent,
	const char* name, const char* value)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	  CS_NODE_ELEMENT, NULL);
  child->SetValue (name);
  csRef<iDocumentNode> text = child->CreateNodeBefore (
		  CS_NODE_TEXT, NULL);
  text->SetValue (value);
}

void Cs2Xml::CreateValueNodeAsInt (csRef<iDocumentNode>& parent,
	const char* name, int value)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	  CS_NODE_ELEMENT, NULL);
  child->SetValue (name);
  csRef<iDocumentNode> text = child->CreateNodeBefore (
		  CS_NODE_TEXT, NULL);
  text->SetValueAsInt (value);
}

void Cs2Xml::CreateValueNodeAsFloat (csRef<iDocumentNode>& parent,
	const char* name, float value)
{
  csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	  CS_NODE_ELEMENT, NULL);
  child->SetValue (name);
  csRef<iDocumentNode> text = child->CreateNodeBefore (
		  CS_NODE_TEXT, NULL);
  text->SetValueAsFloat (value);
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
  float list[100];
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

void Cs2Xml::ParseGeneral (const char* parent_token,
	csParser* parser, csRef<iDocumentNode>& parent, char* buf)
{
  CS_TOKEN_TABLE_START (tokens)
    CS_TOKEN_TABLE (ACCEL)
    CS_TOKEN_TABLE (AGING)
    CS_TOKEN_TABLE (BOX)
    CS_TOKEN_TABLE (CURVECENTER)
    CS_TOKEN_TABLE (CURVECONTROL)
    CS_TOKEN_TABLE (COLOR)
    CS_TOKEN_TABLE (COLORS)
    CS_TOKEN_TABLE (DROPSIZE)
    CS_TOKEN_TABLE (EMITBOX)
    CS_TOKEN_TABLE (EMITFIXED)
    CS_TOKEN_TABLE (F)
    CS_TOKEN_TABLE (FALLSPEED)
    CS_TOKEN_TABLE (FILE)
    CS_TOKEN_TABLE (FIRST)
    CS_TOKEN_TABLE (FOG)
    CS_TOKEN_TABLE (KEY)
    CS_TOKEN_TABLE (LIGHT)
    CS_TOKEN_TABLE (MATRIX)
    CS_TOKEN_TABLE (NUM)
    CS_TOKEN_TABLE (ORIG)
    CS_TOKEN_TABLE (ORIGIN)
    CS_TOKEN_TABLE (PORTAL)
    CS_TOKEN_TABLE (POSITION)
    CS_TOKEN_TABLE (PRIORITY)
    CS_TOKEN_TABLE (RADIUS)
    CS_TOKEN_TABLE (RECTPARTICLES)
    CS_TOKEN_TABLE (ROT)
    CS_TOKEN_TABLE (SCALE)
    CS_TOKEN_TABLE (SECOND)
    CS_TOKEN_TABLE (SHIFT)
    CS_TOKEN_TABLE (T)
    CS_TOKEN_TABLE (TEXTURE)
    CS_TOKEN_TABLE (TRANSPARENT)
    CS_TOKEN_TABLE (TRIANGLE)
    CS_TOKEN_TABLE (TYPE)
    CS_TOKEN_TABLE (UV)
    CS_TOKEN_TABLE (V)
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
    char* tokname = ToLower (parser->GetUnknownToken ());
      switch (cmd)
      {
        case CS_TOKEN_PORTAL:
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
	  break;
        case CS_TOKEN_WARP:
	  {
	    if (!portal_node)
	    {
	      portal_node = parent->CreateNodeBefore (CS_NODE_ELEMENT, NULL);
	      portal_node->SetValue ("portal");
	    }
            ParseGeneral ("portal", parser, portal_node, params);
	  }
	  break;
        case CS_TOKEN_COLOR:
	  {
	    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	CS_NODE_ELEMENT, NULL);
	    child->SetValue (tokname);
	    float x, y, z;
	    csScanStr (params, "%f,%f,%f", &x, &y, &z);
	    child->SetAttributeAsFloat ("red", x);
	    child->SetAttributeAsFloat ("green", y);
	    child->SetAttributeAsFloat ("blue", z);
	    if (name) child->SetAttribute ("name", name);
	  }
	  break;
        case CS_TOKEN_ACCEL:
        case CS_TOKEN_CURVECONTROL:
        case CS_TOKEN_FALLSPEED:
        case CS_TOKEN_FIRST:
        case CS_TOKEN_SECOND:
        case CS_TOKEN_ORIG:
        case CS_TOKEN_ORIGIN:
        case CS_TOKEN_POSITION:
        case CS_TOKEN_SHIFT:
	  {
	    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	CS_NODE_ELEMENT, NULL);
	    child->SetValue (tokname);
	    float x, y, z;
	    csScanStr (params, "%f,%f,%f", &x, &y, &z);
	    child->SetAttributeAsFloat ("x", x);
	    child->SetAttributeAsFloat ("y", y);
	    child->SetAttributeAsFloat ("z", z);
	    if (name) child->SetAttribute ("name", name);
	  }
	  break;
        case CS_TOKEN_VERTEX:
	  {
	    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	CS_NODE_ELEMENT, NULL);
	    child->SetValue ("v");
	    float x, y, z;
	    csScanStr (params, "%f,%f,%f", &x, &y, &z);
	    child->SetAttributeAsFloat ("x", x);
	    child->SetAttributeAsFloat ("y", y);
	    child->SetAttributeAsFloat ("z", z);
	    if (name) child->SetAttribute ("name", name);
	  }
	  break;
        case CS_TOKEN_V:
	  {
	    if (strchr (params, ':'))
	    {
	      // For sprites statement.
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
	      if (name) child->SetAttribute ("name", name);
	    }
	    else if (!strcmp (parent_token, "polygon"))
	    {
	      // In this case we have a VERTICES from a POLYGON.
	      int i;
	      int list[100];
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
	      if (name) child->SetAttribute ("name", name);
	      break;
	    }
	  }
	  break;
        case CS_TOKEN_VERTICES:
	  {
	    // In this case we have a VERTICES from a POLYGON.
	    int i;
	    int list[100];
	    int num;
	    csScanStr (params, "%D", list, &num);
	    for (i = 0 ; i < num ; i++)
	      CreateValueNodeAsInt (parent, "v", list[i]);
	  }
	  break;
	case CS_TOKEN_TEXTURE:
	  {
	    if (!strcmp (parent_token, "polygon"))
	    {
	      csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	CS_NODE_ELEMENT, NULL);
	      child->SetValue ("texmap");
	      if (name) child->SetAttribute ("name", name);
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
	      char buf[2048];
              csScanStr (params, "%s", buf);
	      CreateValueNode (parent, "texture", buf);
	    }
	    else
	    {
	      csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	CS_NODE_ELEMENT, NULL);
	      child->SetValue ("texture");
	      if (name) child->SetAttribute ("name", name);
              ParseGeneral ("texture", parser, child, params);
	    }
	  }
	  break;
        case CS_TOKEN_MATRIX:
	  {
	    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	CS_NODE_ELEMENT, NULL);
	    child->SetValue (tokname);
	    if (name) child->SetAttribute ("name", name);
	    ParseMatrix (parser, child, params);
	  }
	  break;
        case CS_TOKEN_FILE:
	  {
	    char filename[2048];
            csScanStr (params, "%s", filename);
	    CreateValueNode (parent, "file", filename);
	  }
	  break;
        case CS_TOKEN_T:
        case CS_TOKEN_TRIANGLE:
	  {
	    int list[100];
	    int num;
	    csScanStr (params, "%D", list, &num);
	    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	CS_NODE_ELEMENT, NULL);
	    child->SetValue ("t");
	    child->SetAttributeAsInt ("v1", list[0]);
	    child->SetAttributeAsInt ("v2", list[1]);
	    child->SetAttributeAsInt ("v3", list[2]);
	    if (name) child->SetAttribute ("name", name);
	  }
	  break;
        case CS_TOKEN_AGING:
        case CS_TOKEN_BOX:
        case CS_TOKEN_CURVECENTER:
        case CS_TOKEN_COLORS:
        case CS_TOKEN_DROPSIZE:
        case CS_TOKEN_EMITBOX:
        case CS_TOKEN_EMITFIXED:
        case CS_TOKEN_F:
        case CS_TOKEN_FOG:
        case CS_TOKEN_KEY:
        case CS_TOKEN_LIGHT:
        case CS_TOKEN_NUM:
        case CS_TOKEN_PRIORITY:
        case CS_TOKEN_RADIUS:
        case CS_TOKEN_RECTPARTICLES:
        case CS_TOKEN_ROT:
        case CS_TOKEN_UV:
        case CS_TOKEN_W:
	  {
	    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	  	CS_NODE_ELEMENT, NULL);
	    child->SetValue (tokname);
	    if (name) child->SetAttribute ("name", name);
	    // @@@ TODO
	  }
          break;
        case CS_TOKEN_TRANSPARENT:
	  {
	    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	  	CS_NODE_ELEMENT, NULL);
	    child->SetValue (tokname);
	    if (name) child->SetAttribute ("name", name);
	    float r, g, b;
	    csScanStr (params, "%f,%f,%f", &r, &g, &b);
	    child->SetAttributeAsFloat ("red", r);
	    child->SetAttributeAsFloat ("green", g);
	    child->SetAttributeAsFloat ("blue", b);
	  }
	  break;
        case CS_TOKEN_SCALE:
	  {
	    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	  CS_NODE_ELEMENT, NULL);
	    child->SetValue (tokname);
	    float x, y;
	    csScanStr (params, "%f,%f", &x, &y);
	    child->SetAttributeAsFloat ("x", x);
	    child->SetAttributeAsFloat ("y", y);
	  }
	  break;
	case CS_TOKEN_TYPE:
	  {
	    char buf[2048];
            csScanStr (params, "%s", buf);
	    char* tt = ToLower (buf);
	    CreateValueNode (parent, tokname, tt);
	    delete[] tt;
	  }
	  break;
        default:
	{
	  bool val;
	  if (IsEmpty (params))
	  {
	    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	CS_NODE_ELEMENT, NULL);
	    child->SetValue (tokname);
	  }
	  else if (IsString (params))
	  {
	    char buf[2048];
            csScanStr (params, "%s", buf);
	    CreateValueNode (parent, tokname, buf);
	  }
	  else if (IsNumeric (params))
	  {
	    float f;
	    csScanStr (params, "%f", &f);
	    CreateValueNodeAsFloat (parent, tokname, f);
	  }
	  else if (IsBoolean (params, val))
	  {
	    CreateValueNode (parent, tokname, val ? "yes" : "no");
	  }
	  else
	  {
	    csRef<iDocumentNode> child = parent->CreateNodeBefore (
	    	CS_NODE_ELEMENT, NULL);
	    child->SetValue (tokname);
	    if (name) child->SetAttribute ("name", name);
            ParseGeneral (tokname, parser, child, params);
	  }
          break;
        }
      }
    delete[] tokname;
  }
}

//----------------------------------------------------------------------------

void Cs2Xml::Main ()
{
  cmdline.Take (CS_QUERY_REGISTRY (object_reg, iCommandLineParser));
  vfs.Take (CS_QUERY_REGISTRY (object_reg, iVFS));

  const char* val = cmdline->GetName ();
  if (!val)
  {
    ReportError ("Please give VFS world file name or name of the zip archive!");
    return;
  }

  csRef<iDataBuffer> buf;
  if (strstr (val, ".zip"))
  {
    vfs->Mount ("/tmp/cs2xml_data", val);
    buf.Take (vfs->ReadFile ("/tmp/cs2xml_data/world"));
    if (!buf || !buf->GetSize ())
    {
      ReportError ("Archive '%s' does not seem to contain a 'world' file!",
      	val);
      return;
    }
  }
  else
  {
    buf.Take (vfs->ReadFile (val));
    if (!buf || !buf->GetSize ())
    {
      ReportError ("Could not load file '%s'!", val);
      return;
    }
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
      csRef<iDocumentSystem> xml;
      xml.Take (new csTinyDocumentSystem ());
      csRef<iDocument> doc = xml->CreateDocument ();
      csRef<iDocumentNode> root = doc->CreateRoot ();
      csRef<iDocumentNode> parent = root->CreateNodeBefore (
    	  CS_NODE_ELEMENT, NULL);
      parent->SetValue ("world");
      ParseGeneral ("", parser, parent, params);
      doc->Write (vfs, "/this/test.xml");
    }
  }
  else
  {
    ReportError ("Error parsing 'file'!");
  }

  delete parser;
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

