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

static void PrintIndent (int cur_indent_level, char* msg, ...)
{
  int i = cur_indent_level;
  while (i >= 8) { printf ("\t"); i -= 8; }
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

void Cs2Xml::ParseMatrix (csParser *parser, char *buf, int indent)
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
        WriteToken (indent, "scale", NULL, false, false);
	PrintIndent (0, "1</%s>\n", "scale");
        break;
      case CS_TOKEN_ROT_X:
        csScanStr (params, "%f", &angle);
        WriteToken (indent, "rotx", NULL, false, false);
	PrintIndent (0, "%g</%s>\n", angle, "rotx");
        break;
      case CS_TOKEN_ROT_Y:
        csScanStr (params, "%f", &angle);
        WriteToken (indent, "roty", NULL, false, false);
	PrintIndent (0, "%g</%s>\n", angle, "roty");
        break;
      case CS_TOKEN_ROT_Z:
        csScanStr (params, "%f", &angle);
        WriteToken (indent, "rotz", NULL, false, false);
	PrintIndent (0, "%g</%s>\n", angle, "rotz");
        break;
      case CS_TOKEN_ROT:
        csScanStr (params, "%F", list, &num);
        if (num == 3)
        {
          WriteToken (indent, "rotx", NULL, false, false);
	  PrintIndent (0, "%g</%s> ", list[0], "rotx");
          WriteToken (indent, "rotz", NULL, false, false);
	  PrintIndent (0, "%g</%s> ", list[2], "rotz");
          WriteToken (indent, "roty", NULL, false, false);
	  PrintIndent (0, "%g</%s>\n", list[1], "roty");
        }
        else
	{
	  // Error@@@
	}
        break;
      case CS_TOKEN_SCALE_X:
        csScanStr (params, "%f", &scaler);
        WriteToken (indent, "scalex", NULL, false, false);
	PrintIndent (0, "%g</%s>\n", scaler, "scalex");
        break;
      case CS_TOKEN_SCALE_Y:
        csScanStr (params, "%f", &scaler);
        WriteToken (indent, "scaley", NULL, false, false);
	PrintIndent (0, "%g</%s>\n", scaler, "scaley");
        break;
      case CS_TOKEN_SCALE_Z:
        csScanStr (params, "%f", &scaler);
        WriteToken (indent, "scalez", NULL, false, false);
	PrintIndent (0, "%g</%s>\n", scaler, "scalez");
        break;
      case CS_TOKEN_SCALE:
        csScanStr (params, "%F", list, &num);
        if (num == 1)      // One scaler; applied to entire matrix.
	{
          WriteToken (indent, "scale", NULL, false, false);
	  PrintIndent (0, "%g</%s>\n", list[0], "scale");
	}
        else if (num == 3) // Three scalers; applied to X, Y, Z individually.
	{
          WriteToken (indent, "scalex", NULL, false, false);
	  PrintIndent (0, "%g</%s> ", list[0], "scalex");
          WriteToken (indent, "scaley", NULL, false, false);
	  PrintIndent (0, "%g</%s> ", list[1], "scaley");
          WriteToken (indent, "scalez", NULL, false, false);
	  PrintIndent (0, "%g</%s>\n", list[2], "scalez");
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
      WriteToken (indent, "scale", NULL, false, false);
      PrintIndent (0, "%g</%s>\n", list[0], "scale");
    }
    else if (num == 9)
    {
      WriteToken (indent, "m11", NULL, false, false);
      PrintIndent (0, "%g</%s> ", list[0], "m11");
      WriteToken (0, "m12", NULL, false, false);
      PrintIndent (0, "%g</%s> ", list[1], "m12");
      WriteToken (0, "m13", NULL, false, false);
      PrintIndent (0, "%g</%s>\n", list[2], "m13");
      WriteToken (indent, "m21", NULL, false, false);
      PrintIndent (0, "%g</%s> ", list[3], "m21");
      WriteToken (0, "m22", NULL, false, false);
      PrintIndent (0, "%g</%s> ", list[4], "m22");
      WriteToken (0, "m23", NULL, false, false);
      PrintIndent (0, "%g</%s>\n", list[5], "m23");
      WriteToken (indent, "m31", NULL, false, false);
      PrintIndent (0, "%g</%s> ", list[6], "m31");
      WriteToken (0, "m32", NULL, false, false);
      PrintIndent (0, "%g</%s> ", list[7], "m32");
      WriteToken (0, "m33", NULL, false, false);
      PrintIndent (0, "%g</%s>\n", list[8], "m33");
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
        case CS_TOKEN_MATRIX:
	  {
	    WriteToken (indent, tokname, name, false, true);
	    ParseMatrix (parser, params, indent+2);
            PrintIndent (indent, "</%s>\n", tokname);
	  }
	  break;
        case CS_TOKEN_FILE:
	  {
	    char filename[2048];
            csScanStr (params, "%s", filename);
	    WriteToken (indent, tokname, name, false, false);
	    PrintIndent (0, "%s", filename);
            PrintIndent (0, "</%s>\n", tokname);
	  }
	  break;
        case CS_TOKEN_T:
        case CS_TOKEN_TRIANGLE:
	  {
	    WriteToken (indent, "t", name, false, false);
	    int list[100];
	    int num;
	    csScanStr (params, "%D", list, &num);
	    PrintIndent (0, "<t1>%d</t1> <t2>%d</t2> <t3>%d</t3>",
	    	list[0], list[1], list[2]);
            PrintIndent (0, "</%s>\n", "t");
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
        case CS_TOKEN_SCALE:
        case CS_TOKEN_TRANSPARENT:
        case CS_TOKEN_UV:
        case CS_TOKEN_W:
	  WriteToken (indent, tokname, name, true, false);
	  PrintIndent (0, "/>\n");
          break;
        default:
	  if (IsString (params))
	  {
	    WriteToken (indent, tokname, name, false, false);
	    char* p = NULL;
	    if (*params == '\'')
	    {
	      params++;
	      p = strchr (params, '\'');
	      if (p) *p = 0;
	    }
	    PrintIndent (0, "%s", params);
	    if (p) *p = '\'';
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

  csRef<iFile> fout;
  //fout.Take (vfs->Open ("/this/world", VFS_FILE_WRITE));
  //if (!fout)
  //{
    //ReportError ("Could not open file '/this/world'!");
    //return;
  //}

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
      WriteToken (0, "world", NULL, false, true);
      ParseGeneral ("", 2, parser, fout, params);
      PrintIndent (0, "</world>\n");
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

