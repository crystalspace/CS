/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein

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

#define SYSDEF_SOFTWARE2D
#include "cssysdef.h"
#include "soft_g3d.h"
#include "protex3d.h"
#include "isystem.h"
#include "igraph2d.h"
#include "icfgnew.h"

IMPLEMENT_FACTORY (csGraphics3DSoftware)
IMPLEMENT_FACTORY (csSoftProcTexture3D)

EXPORT_CLASS_TABLE (soft3d)
  EXPORT_CLASS_DEP (csGraphics3DSoftware, "crystalspace.graphics3d.software",
    "Software 3D graphics driver for Crystal Space", "crystalspace.font.server.")
  EXPORT_CLASS (csSoftProcTexture3D, 
    "crystalspace.graphics3d.software.offscreen",
    "Software 3D off screen driver")
EXPORT_CLASS_TABLE_END

IMPLEMENT_IBASE (csGraphics3DSoftware)
  IMPLEMENTS_INTERFACE (iGraphics3D)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_EMBEDDED_INTERFACE (iConfig)
IMPLEMENT_IBASE_END

#define SysPrintf System->Printf

csGraphics3DSoftware::csGraphics3DSoftware (iBase *iParent)
  : csGraphics3DSoftwareCommon ()
{
  CONSTRUCT_IBASE (iParent);
  CONSTRUCT_EMBEDDED_IBASE (scfiConfig);
  csScan_Initialize ();
}

csGraphics3DSoftware::~csGraphics3DSoftware ()
{
  csScan_Finalize ();
}

bool csGraphics3DSoftware::Initialize (iSystem *iSys)
{
  (System = iSys)->IncRef ();

  NewInitialize ();

  const char *driver = iSys->GetOptionCL ("canvas");
  if (!driver)
    driver = config->GetStr ("Video.Software.Canvas", SOFTWARE_2D_DRIVER);

  G2D = LOAD_PLUGIN (System, driver, NULL, iGraphics2D);

  return G2D ? true : false;
}

bool csGraphics3DSoftware::Open (const char *Title)
{
  if (!csGraphics3DSoftwareCommon::Open (Title) || !NewOpen ())
    return false;

  bool bFullScreen = G2D->GetFullScreen ();
  SysPrintf(MSG_INITIALIZATION, 
	    "Using %s mode %dx%d (internal rendering at %dx%d).\n",
            bFullScreen ? "full screen" : "windowed", 
	    G2D->GetWidth (), G2D->GetHeight (), width, height);

  if (pfmt.PixelBytes == 4)
    SysPrintf (MSG_INITIALIZATION, 
	  "Using truecolor mode with %d bytes per pixel and %d:%d:%d RGB mode.\n",
          pfmt.PixelBytes, pfmt.RedBits, pfmt.GreenBits, pfmt.BlueBits);
  else if (pfmt.PixelBytes == 2)
    SysPrintf (MSG_INITIALIZATION, 
	   "Using truecolor mode with %d bytes per pixel and %d:%d:%d RGB mode.\n",
	   pfmt.PixelBytes, pfmt.RedBits, pfmt.GreenBits, pfmt.BlueBits);
  else
    SysPrintf (MSG_INITIALIZATION, 
	       "Using palette mode with 1 byte per pixel (256 colors).\n");

  return true;
}

//---------------------------------------------------------------------------

IMPLEMENT_EMBEDDED_IBASE (csGraphics3DSoftware::csSoftConfig)
  IMPLEMENTS_INTERFACE (iConfig)
IMPLEMENT_EMBEDDED_IBASE_END

#define NUM_OPTIONS 8

static const csOptionDescription config_options [NUM_OPTIONS] =
{
  { 0, "ilace", "Interlacing", CSVAR_BOOL },
  { 1, "light", "Texture lighting", CSVAR_BOOL },
  { 2, "alpha", "Semi-transparent textures", CSVAR_BOOL },
  { 3, "txtmap", "Texture mapping", CSVAR_BOOL },
  { 4, "mmx", "MMX support", CSVAR_BOOL },
  { 5, "gamma", "Gamma value", CSVAR_FLOAT },
  { 6, "gouraud", "Gouraud shading", CSVAR_BOOL },
  { 7, "smaller", "Smaller rendering", CSVAR_BOOL },
};

bool csGraphics3DSoftware::csSoftConfig::SetOption (int id, csVariant* value)
{
  if (value->type != config_options[id].type)
    return false;
  switch (id)
  {
    case 0: scfParent->do_interlaced = value->v.b ? 0 : -1; break;
    case 1: scfParent->do_lighting = value->v.b; break;
    case 2: scfParent->do_alpha = value->v.b; break;
    case 3: scfParent->do_textured = value->v.b; break;
#ifdef DO_MMX
    case 4: scfParent->do_mmx = value->v.b; break;
#endif
    case 5: scfParent->texman->Gamma = value->v.f; break;
    case 6: scfParent->do_gouraud = value->v.b; break;
    case 7: scfParent->do_smaller_rendering = value->v.b; break;
    default: return false;
  }
  scfParent->ScanSetup ();
  return true;
}

bool csGraphics3DSoftware::csSoftConfig::GetOption (int id, csVariant* value)
{
  value->type = config_options[id].type;
  switch (id)
  {
    case 0: value->v.b = scfParent->do_interlaced != -1; break;
    case 1: value->v.b = scfParent->do_lighting; break;
    case 2: value->v.b = scfParent->do_alpha; break;
    case 3: value->v.b = scfParent->do_textured; break;
#ifdef DO_MMX
    case 4: value->v.b = scfParent->do_mmx; break;
#endif
    case 5: value->v.f = scfParent->Gamma / 65536.; break;
    case 6: value->v.b = scfParent->do_gouraud; break;
    case 7: value->v.b = scfParent->do_smaller_rendering; break;
    default: return false;
  }
  return true;
}

bool csGraphics3DSoftware::csSoftConfig::GetOptionDescription
  (int idx, csOptionDescription* option)
{
  if (idx < 0 || idx >= NUM_OPTIONS)
    return false;
  *option = config_options[idx];
  return true;
}
