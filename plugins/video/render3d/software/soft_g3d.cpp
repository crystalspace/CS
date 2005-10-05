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

#include "cssysdef.h"
#include "soft_g3d.h"
#include "iutil/plugin.h"
#include "iutil/eventh.h"
#include "iutil/comp.h"
#include "ivideo/graph2d.h"
#include "iutil/cfgfile.h"
#include "iutil/cmdline.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

CS_IMPLEMENT_PLUGIN

namespace cspluginSoft3d
{

SCF_IMPLEMENT_FACTORY (csSoftwareGraphics3D)

csSoftwareGraphics3D::csSoftwareGraphics3D (iBase *iParent)
  : scfImplementationType (this, iParent)
{
  is_for_procedural_textures = false;
  csScan_Initialize ();
}

csSoftwareGraphics3D::~csSoftwareGraphics3D ()
{
  csScan_Finalize ();
}

bool csSoftwareGraphics3D::Initialize (iObjectRegistry *object_reg)
{
  csSoftwareGraphics3DCommon::Initialize(object_reg);
  csRef<iPluginManager> plugin_mgr (
  	CS_QUERY_REGISTRY (object_reg, iPluginManager));
  csRef<iCommandLineParser> cmdline (CS_QUERY_REGISTRY (object_reg,
  	iCommandLineParser));

  NewInitialize ();
  const char *driver = cmdline->GetOption ("canvas");
  if (!driver)
    driver = config->GetStr ("Video.Software.Canvas", CS_SOFTWARE_2D_DRIVER);
  G2D = CS_LOAD_PLUGIN (plugin_mgr, driver, iGraphics2D);
  if (G2D)
  {
    if (!object_reg->Register (G2D, "iGraphics2D"))
    {
      Report (CS_REPORTER_SEVERITY_ERROR,
	  "Could not register the canvas!");
      return false;
    }
  }
  return G2D ? true : false;
}

bool csSoftwareGraphics3D::Open ()
{
  if (!csSoftwareGraphics3DCommon::Open () || !NewOpen ())
    return false;

  bool bFullScreen = G2D->GetFullScreen ();
  Report (CS_REPORTER_SEVERITY_NOTIFY,
	    "Using %s mode %dx%d (internal rendering at %dx%d).",
            bFullScreen ? "full screen" : "windowed",
	    G2D->GetWidth (), G2D->GetHeight (), width, height);

  if (pfmt.PixelBytes == 4)
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Using truecolor mode with %d bytes per pixel and %d:%d:%d RGB mode.",
      pfmt.PixelBytes, pfmt.RedBits, pfmt.GreenBits, pfmt.BlueBits);
  else if (pfmt.PixelBytes == 2)
    Report (CS_REPORTER_SEVERITY_NOTIFY,
      "Using truecolor mode with %d bytes per pixel and %d:%d:%d RGB mode.",
      pfmt.PixelBytes, pfmt.RedBits, pfmt.GreenBits, pfmt.BlueBits);
  else
    Report (CS_REPORTER_SEVERITY_NOTIFY,
	       "Using palette mode with 1 byte per pixel (256 colors).");

  return true;
}

//---------------------------------------------------------------------------

#define NUM_OPTIONS 7

static const csOptionDescription config_options [NUM_OPTIONS] =
{
  { 0, "ilace", "Interlacing", CSVAR_BOOL },
  { 1, "light", "Texture lighting", CSVAR_BOOL },
  { 2, "alpha", "Semi-transparent textures", CSVAR_BOOL },
  { 3, "txtmap", "Texture mapping", CSVAR_BOOL },
  { 4, "mmx", "MMX support", CSVAR_BOOL },
  { 5, "gouraud", "Gouraud shading", CSVAR_BOOL },
  { 6, "smaller", "Smaller rendering", CSVAR_BOOL },
};

bool csSoftwareGraphics3D::SetOption (int id, csVariant* value)
{
  if (value->GetType () != config_options[id].type)
    return false;
  switch (id)
  {
    case 0: do_interlaced = value->GetBool () ? 0 : -1; break;
    case 1: do_lighting = value->GetBool (); break;
    case 2: do_alpha = value->GetBool (); break;
    case 3: do_textured = value->GetBool (); break;
#ifdef CS_HAVE_MMX
    case 4: do_mmx = value->GetBool (); break;
#endif
    case 5: do_gouraud = value->GetBool (); break;
    case 6: do_smaller_rendering = value->GetBool (); break;
    default: return false;
  }
  return true;
}

bool csSoftwareGraphics3D::GetOption (int id, csVariant* value)
{
  switch (id)
  {
    case 0: value->SetBool (do_interlaced != -1); break;
    case 1: value->SetBool (do_lighting); break;
    case 2: value->SetBool (do_alpha); break;
    case 3: value->SetBool (do_textured); break;
#ifdef CS_HAVE_MMX
    case 4: value->SetBool (do_mmx); break;
#else
    case 4: value->SetBool (false); break;
#endif
    case 5: value->SetBool (do_gouraud); break;
    case 6: value->SetBool (do_smaller_rendering); break;
    default: return false;
  }
  return true;
}

bool csSoftwareGraphics3D::GetOptionDescription
  (int idx, csOptionDescription* option)
{
  if (idx < 0 || idx >= NUM_OPTIONS)
    return false;
  *option = config_options[idx];
  return true;
}

} // namespace cspluginSoft3d
