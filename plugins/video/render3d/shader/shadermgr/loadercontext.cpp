/*
    Copyright (C) 2008 by Frank Richter

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

#include "loadercontext.h"

#include "iengine/texture.h"
#include "imap/loader.h"

CS_PLUGIN_NAMESPACE_BEGIN(ShaderManager)
{

LoaderContext::LoaderContext (iLoader* loader, iTextureManager* tm) : 
  scfImplementationType (this), loader (loader), tm (tm)
{
}

void LoaderContext::RegisterTexture (iTextureWrapper* tex)
{
  if (!tex) return;
  
  // @@@ Guess at what this texture is likely used for ...
  tex->SetTextureClass ("lookup");
  if (tm) tex->Register (tm);
}

iTextureWrapper* LoaderContext::FindTexture (const char* name, bool doLoad)
{
  csRef<iTextureWrapper> rc = loader->LoadTexture (name, name,
    CS_TEXTURE_3D, 0, false, false);
  RegisterTexture (rc);
  return rc;
}

iTextureWrapper* LoaderContext::FindNamedTexture (const char* name,
                                                  const char *filename)
{
  csRef<iTextureWrapper> rc = loader->LoadTexture(name, filename,
    CS_TEXTURE_3D, 0, false, false);
  RegisterTexture (rc);
  return rc;
}

}
CS_PLUGIN_NAMESPACE_END(ShaderManager)
