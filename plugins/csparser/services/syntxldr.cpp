/*
    Copyright (C) 2001 by Norman Krämer
  
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
#include "syntxldr.h"
#include "csutil/parser.h"
#include "csutil/scanstr.h"
#include "csutil/util.h"
#include "csgeom/matrix3.h"
#include "csgeom/vector3.h"
#include "ivideo/graph3d.h"
#include "imesh/thing/ptextype.h"

CS_IMPLEMENT_PLUGIN;

SCF_IMPLEMENT_IBASE (csTextSyntaxService)
  SCF_IMPLEMENTS_INTERFACE (iSyntaxService)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csTextSyntaxService::eiComponent)
  SCF_IMPLEMENTS_INTERFACE (iComponent)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csTextSyntaxService);

SCF_EXPORT_CLASS_TABLE (cssynldr)
  SCF_EXPORT_CLASS (csTextSyntaxService, "crystalspace.syntax.loader.service.text", 
		    "Crystal Space loader services for textual CS syntax")
SCF_EXPORT_CLASS_TABLE_END

CS_TOKEN_DEF_START
  CS_TOKEN_DEF (IDENTITY)
  CS_TOKEN_DEF (ROT_X)
  CS_TOKEN_DEF (ROT_Y)
  CS_TOKEN_DEF (ROT_Z)
  CS_TOKEN_DEF (ROT)
  CS_TOKEN_DEF (SCALE_X)
  CS_TOKEN_DEF (SCALE_Y)
  CS_TOKEN_DEF (SCALE_Z)
  CS_TOKEN_DEF (SCALE)
  CS_TOKEN_DEF (COPY)
  CS_TOKEN_DEF (MULTIPLY2)
  CS_TOKEN_DEF (MULTIPLY)
  CS_TOKEN_DEF (ADD)
  CS_TOKEN_DEF (ALPHA)
  CS_TOKEN_DEF (TRANSPARENT)
  CS_TOKEN_DEF (KEYCOLOR)
  CS_TOKEN_DEF (TILING)
  CS_TOKEN_DEF (NONE)
  CS_TOKEN_DEF (FLAT)
  CS_TOKEN_DEF (GOURAUD)
  CS_TOKEN_DEF (LIGHTMAP)
CS_TOKEN_DEF_END

csTextSyntaxService::csTextSyntaxService (iBase *parent)
{
  SCF_CONSTRUCT_IBASE (parent);
  SCF_CONSTRUCT_EMBEDDED_IBASE (scfiComponent);

  last_error = new char[200];
  last_error[0] = '\0';
  success = true;
}

csTextSyntaxService::~csTextSyntaxService ()
{
  delete [] last_error;
}

const char *csTextSyntaxService::GetLastError ()
{
  return last_error;
}

void csTextSyntaxService::SetError (const char *msg, ...)
{
  va_list args;
  va_start (args, msg);
  vsprintf (last_error, msg, args);
  va_end (args);
  success = false;
}

bool csTextSyntaxService::ParseMatrix (char *buf, csMatrix3 &m)
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
  const csMatrix3 identity;

  success = true;
  while ((cmd = csGetCommand (&buf, commands, &params)) > 0)
  {
    switch (cmd)
    {
      case CS_TOKEN_IDENTITY:
        m = identity;
        break;
      case CS_TOKEN_ROT_X:
        csScanStr (params, "%f", &angle);
        m *= csXRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT_Y:
        csScanStr (params, "%f", &angle);
        m *= csYRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT_Z:
        csScanStr (params, "%f", &angle);
        m *= csZRotMatrix3 (angle);
        break;
      case CS_TOKEN_ROT:
        csScanStr (params, "%F", list, &num);
        if (num == 3)
        {
          m *= csXRotMatrix3 (list[0]);
          m *= csZRotMatrix3 (list[2]);
          m *= csYRotMatrix3 (list[1]);
        }
        else
	  SetError ("Badly formed rotation: '%s'\n", params);
        break;
      case CS_TOKEN_SCALE_X:
        csScanStr (params, "%f", &scaler);
        m *= csXScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE_Y:
        csScanStr (params, "%f", &scaler);
        m *= csYScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE_Z:
        csScanStr (params, "%f", &scaler);
        m *= csZScaleMatrix3(scaler);
        break;
      case CS_TOKEN_SCALE:
        csScanStr (params, "%F", list, &num);
        if (num == 1)      // One scaler; applied to entire matrix.
	  m *= list[0];
        else if (num == 3) // Three scalers; applied to X, Y, Z individually.
	  m *= csMatrix3 (list[0],0,0,0,list[1],0,0,0,list[2]);
        else
	  SetError ("Badly formed scale: '%s'\n", params);
        break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
  {
    // Neither SCALE, ROT, nor IDENTITY, so matrix may contain a single scaler
    // or the nine values of a 3x3 matrix.
    csScanStr (buf, "%F", list, &num);
    if (num == 1)
      m = csMatrix3 () * list[0];
    else if (num == 9)
      m = csMatrix3 (
        list[0], list[1], list[2],
        list[3], list[4], list[5],
        list[6], list[7], list[8]);
    else
      SetError ("Badly formed matrix '%s'\n", buf);
  }
  return success;
}

bool csTextSyntaxService::ParseVector (char *buf, csVector3 &v)
{
  success = true;

  csScanStr (buf, "%F", list, &num);
  if (num == 3)
  {
    v.x = list[0];
    v.y = list[1];
    v.z = list[2];
  }
  else
    SetError("Malformed vector parameter");
  return success;
}

bool csTextSyntaxService::ParseMixmode (char *buf, UInt &mixmode)
{
  CS_TOKEN_TABLE_START (modes)
    CS_TOKEN_TABLE (COPY)
    CS_TOKEN_TABLE (MULTIPLY2)
    CS_TOKEN_TABLE (MULTIPLY)
    CS_TOKEN_TABLE (ADD)
    CS_TOKEN_TABLE (ALPHA)
    CS_TOKEN_TABLE (TRANSPARENT)
    CS_TOKEN_TABLE (KEYCOLOR)
    CS_TOKEN_TABLE (TILING)
  CS_TOKEN_TABLE_END

  char* name;
  long cmd;
  char* params;

  mixmode = 0;
  success = true;

  while ((cmd = csGetObject (&buf, modes, &name, &params)) > 0)
  {
    if (!params)
    {
      SetError ("Expected parameters instead of '%s'!\n", buf);
      break;
    }
    switch (cmd)
    {
      case CS_TOKEN_COPY: mixmode |= CS_FX_COPY; break;
      case CS_TOKEN_MULTIPLY: mixmode |= CS_FX_MULTIPLY; break;
      case CS_TOKEN_MULTIPLY2: mixmode |= CS_FX_MULTIPLY2; break;
      case CS_TOKEN_ADD: mixmode |= CS_FX_ADD; break;
      case CS_TOKEN_ALPHA:
	mixmode &= ~CS_FX_MASK_ALPHA;
	float alpha;
        csScanStr (params, "%f", &alpha);
	mixmode |= CS_FX_SETALPHA(alpha);
	break;
      case CS_TOKEN_TRANSPARENT: mixmode |= CS_FX_TRANSPARENT; break;
      case CS_TOKEN_KEYCOLOR: mixmode |= CS_FX_KEYCOLOR; break;
      case CS_TOKEN_TILING: mixmode |= CS_FX_TILING; break;
    }
  }
  if (cmd == CS_PARSERR_TOKENNOTFOUND)
    SetError ("Token '%s' not found while parsing the modes!\n",
	      csGetLastOffender ());
  return success;
}

bool csTextSyntaxService::ParseShading (char *buf, int &shading)
{
  CS_TOKEN_TABLE_START (texturing_commands)
    CS_TOKEN_TABLE (NONE)
    CS_TOKEN_TABLE (FLAT)
    CS_TOKEN_TABLE (GOURAUD)
    CS_TOKEN_TABLE (LIGHTMAP)
  CS_TOKEN_TABLE_END

  char *params, *name;
  long cmd;

  success = true;
  shading = 0;
  while ((cmd = csGetObject (&buf, texturing_commands, &name, &params)) > 0)
    switch (cmd)
    {
    case CS_TOKEN_NONE:
      shading = POLYTXT_NONE;
      break;
    case CS_TOKEN_FLAT:
      shading = POLYTXT_FLAT;
      break;
    case CS_TOKEN_GOURAUD:
      shading = POLYTXT_GOURAUD;
      break;
    case CS_TOKEN_LIGHTMAP:
      shading = POLYTXT_LIGHTMAP;
      break;
    default:
      if (cmd == CS_PARSERR_TOKENNOTFOUND)
	SetError ("Token '%s' not found while parsing the shading specification!\n",
		  csGetLastOffender ());
    };

  return success;
}

