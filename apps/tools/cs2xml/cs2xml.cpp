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

static void PrintIndent (int cur_indent_level, char* msg, ...)
{
  int i = cur_indent_level;
  while (i >= 20) { printf ("                    "); i -= 20; }
  while (i >= 4) { printf ("    "); i -= 4; }
  while (i > 0) { printf (" "); i--; }
  va_list arg;
  va_start (arg, msg);
  vprintf (msg, arg);
  va_end (arg);
  fflush (stdout);
}

//-----------------------------------------------------------------------------

void Cs2Xml::ReportError (const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  csReportV (object_reg, CS_REPORTER_SEVERITY_ERROR,
  	"crystalspace.apps.levtool", description, arg);
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
  CS_TOKEN_DEF (KEY)
  CS_TOKEN_DEF (LIGHT)
  CS_TOKEN_DEF (MATRIX)
  CS_TOKEN_DEF (NUM)
  CS_TOKEN_DEF (ORIG)
  CS_TOKEN_DEF (ORIGIN)
  CS_TOKEN_DEF (POSITION)
  CS_TOKEN_DEF (PRIORITY)
  CS_TOKEN_DEF (RADIUS)
  CS_TOKEN_DEF (RECTPARTICLES)
  CS_TOKEN_DEF (ROT)
  CS_TOKEN_DEF (SCALE)
  CS_TOKEN_DEF (SECOND)
  CS_TOKEN_DEF (SHIFT)
  CS_TOKEN_DEF (T)
  CS_TOKEN_DEF (TRANSPARENT)
  CS_TOKEN_DEF (TRIANGLE)
  CS_TOKEN_DEF (UV)
  CS_TOKEN_DEF (V)
  CS_TOKEN_DEF (VERTEX)
  CS_TOKEN_DEF (VERTICES)
  CS_TOKEN_DEF (W)
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

bool Cs2Xml::IsBoolean (const char* in)
{
  while (isspace (*in)) in++;
  if (!strncasecmp (in, "yes", 3)) in += 3;
  else if (!strncasecmp (in, "true", 4)) in += 4;
  else if (!strncasecmp (in, "on", 2)) in += 2;
  else if (!strncasecmp (in, "1", 1)) in += 1;
  else if (!strncasecmp (in, "false", 5)) in += 5;
  else if (!strncasecmp (in, "off", 3)) in += 3;
  else if (!strncasecmp (in, "0", 1)) in += 1;
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

void Cs2Xml::WriteToken (int indent, const char* token, const char* name,
  	bool shortform, bool newline)
{
  if (shortform)
  {
    if (name == NULL)
      PrintIndent (indent, "<%s ", token);
    else
      PrintIndent (indent, "<%s name='%s' ", token, name);
  }
  else
  {
    if (name == NULL)
      PrintIndent (indent, "<%s>", token);
    else
      PrintIndent (indent, "<%s name='%s'>", token, name);
  }
  if (newline) PrintIndent (0, "\n");
}

void Cs2Xml::WriteVector3 (const char* params,
  	const char* xname, const char* yname,
	const char* zname)
{
  float x, y, z;
  csScanStr (params, "%f,%f,%f", &x, &y, &z);
  PrintIndent (0, "%s=%g %s=%g %s=%g", xname, x, yname, y, zname, z);
}

void Cs2Xml::ParseGeneral (const char* parent_token,
	int indent, csParser* parser, iFile* fout, char* buf)
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
    CS_TOKEN_TABLE (POSITION)
    CS_TOKEN_TABLE (PRIORITY)
    CS_TOKEN_TABLE (RADIUS)
    CS_TOKEN_TABLE (RECTPARTICLES)
    CS_TOKEN_TABLE (ROT)
    CS_TOKEN_TABLE (SCALE)
    CS_TOKEN_TABLE (SECOND)
    CS_TOKEN_TABLE (SHIFT)
    CS_TOKEN_TABLE (T)
    CS_TOKEN_TABLE (TRANSPARENT)
    CS_TOKEN_TABLE (TRIANGLE)
    CS_TOKEN_TABLE (UV)
    CS_TOKEN_TABLE (V)
    CS_TOKEN_TABLE (VERTEX)
    CS_TOKEN_TABLE (VERTICES)
    CS_TOKEN_TABLE (W)
  CS_TOKEN_TABLE_END

  char *name, *params;
  long cmd;

  while ((cmd = parser->GetObject (&buf, tokens, &name, &params))
  	!= CS_PARSERR_EOF)
  {
    char* tokname = ToLower (parser->GetUnknownToken ());
    if (params == NULL || IsEmpty (params))
    {
      WriteToken (indent, tokname, name, true, false);
      PrintIndent (0, "/>\n");
    }
    else
    {
      switch (cmd)
      {
        case CS_TOKEN_COLOR:
	  WriteToken (indent, tokname, name, true, false);
	  WriteVector3 (params, "r", "g", "b");
	  PrintIndent (0, "/>\n");
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
        case CS_TOKEN_VERTEX:
	  WriteToken (indent, tokname, name, true, false);
	  WriteVector3 (params);
	  PrintIndent (0, "/>\n");
	  break;
        case CS_TOKEN_V:
	  {
	    if (strchr (params, ':'))
	    {
	      // For sprites statement.
	      float x, y, z, u, v;
	      csScanStr (params, "%f,%f,%f:%f,%f", &x, &y, &z, &u, &v);
	      WriteToken (indent, tokname, name, false, false);
	      PrintIndent (0, "x=%g y=%g z=%g u=%g v=%g", x, y, z, u, v);
              PrintIndent (0, "</%s>\n", tokname);
	    }
	    else if (!strcmp (parent_token, "polygon"))
	    {
	      // In this case we have a VERTICES from a POLYGON.
	      int i;
	      int list[100];
	      int num;
	      csScanStr (params, "%D", list, &num);
	      PrintIndent (indent, "<v>%d</v>", list[0]);
	      for (i = 1 ; i < num ; i++)
	      {
	        PrintIndent (0, " <v>%d</v>", list[i]);
	      }
	      PrintIndent (0, "\n");
	    }
	    else
	    {
	      WriteToken (indent, tokname, name, true, false);
	      WriteVector3 (params);
	      PrintIndent (0, "/>\n");
	      break;
	    }
	  }
	  break;
        case CS_TOKEN_VERTICES:
	  {
	    // In this case we have a VERTICES from a POLYGON.
	    int list[100];
	    int num;
	    csScanStr (params, "%D", list, &num);
	    int i;
	    PrintIndent (indent, "<v>%d</v>", list[0]);
	    for (i = 1 ; i < num ; i++)
	    {
	      PrintIndent (0, " <v>%d</v>", list[i]);
	    }
	    PrintIndent (0, "\n");
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
        case CS_TOKEN_FILE:
        case CS_TOKEN_FOG:
        case CS_TOKEN_KEY:
        case CS_TOKEN_LIGHT:
        case CS_TOKEN_MATRIX:
        case CS_TOKEN_NUM:
        case CS_TOKEN_PRIORITY:
        case CS_TOKEN_RADIUS:
        case CS_TOKEN_RECTPARTICLES:
        case CS_TOKEN_ROT:
        case CS_TOKEN_SCALE:
        case CS_TOKEN_T:
        case CS_TOKEN_TRANSPARENT:
        case CS_TOKEN_TRIANGLE:
        case CS_TOKEN_UV:
        case CS_TOKEN_W:
	  WriteToken (indent, tokname, name, true, false);
	  PrintIndent (0, "/>\n");
          break;
        default:
	  if (IsString (params))
	  {
	    WriteToken (indent, tokname, name, false, false);
	    PrintIndent (0, "%s", params);
            PrintIndent (0, "</%s>\n", tokname);
	  }
	  else if (IsBoolean (params))
	  {
	    WriteToken (indent, tokname, name, false, false);
	    PrintIndent (0, "%s", params);
	    PrintIndent (0, "</%s>\n", tokname);
	  }
	  else if (IsNumeric (params))
	  {
	    WriteToken (indent, tokname, name, false, false);
	    PrintIndent (0, "%s", params);
	    PrintIndent (0, "</%s>\n", tokname);
	  }
	  else
	  {
	    WriteToken (indent, tokname, name, false, true);
            ParseGeneral (tokname, indent+2, parser, fout, params);
            PrintIndent (indent, "</%s>\n", tokname);
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
  CS_TOKEN_TABLE_END

  char *data = **buf;
  char *name, *params;
  long cmd;

  if ((cmd = parser->GetObject (&data, tokens, &name, &params))
  	!= CS_PARSERR_EOF)
  {
    if (params)
    {
      Write (fout, "%s (\n", parser->GetUnknownToken ());
      ParseGeneral ("", 2, parser, fout, params);
      WriteStr (fout, ")\n");
    }
  }
  else
  {
    ReportError ("Error parsing 'file'!");
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

