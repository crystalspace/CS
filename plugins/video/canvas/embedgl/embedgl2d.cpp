/*
2d canvas for embedding in existing gl context for Crystal Space (source)
Copyright (C) 2006 by Matze Braun <MatzeBraun@gmx.de>
Copyright (C) 2006 by Anders Stenberg <dentoid@users.sourceforge.net>
Copyright (C) 2006 by Pablo Martin <caedesv@users.sourceforge.net>

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

#include "cssysdef.h"
#include "csplugincommon/opengl/assumedstate.h"
#include "embedgl2d.h"



SCF_IMPLEMENT_FACTORY (csGraphics2DGLEmbed)

csGraphics2DGLEmbed::csGraphics2DGLEmbed(iBase* iParent)
  : scfImplementationType (this, iParent)
{
}

bool csGraphics2DGLEmbed::Open ()
{
  bool ret = csGraphics2DGLCommon::Open ();
  AllowResizing = true;
  return ret;
}

bool csGraphics2DGLEmbed::PerformExtensionV (char const* command, va_list args)
{
  if (!strcasecmp (command, "resetstatecache"))
  {
    statecontext->InitCache ();
    CS::PluginCommon::GL::SetAssumedState (statecache, &ext);
    return true;
  }
  return csGraphics2DGLCommon::PerformExtensionV (command,args);
}
