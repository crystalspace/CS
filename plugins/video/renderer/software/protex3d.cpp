/*
    Copyright (C) 2000 by Samuel Humphreys

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
#include "protex3d.h"
#include "isystem.h"
#include "igraph2d.h"
#include "soft_txt.h"
#include "csgfxldr/memimage.h"
#include "csgfxldr/rgbpixel.h"
#include "itexture.h"

DECLARE_FACTORY (csSoftProcTexture3D)

IMPLEMENT_IBASE (csSoftProcTexture3D)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics3D)
  IMPLEMENTS_INTERFACE (iSoftProcTexture)
IMPLEMENT_IBASE_END

csSoftProcTexture3D *csSoftProcTexture3D::head_texG3D = NULL;

csSoftProcTexture3D::csSoftProcTexture3D (iBase *iParent)
  : csGraphics3DSoftwareCommon ()
{
  CONSTRUCT_IBASE (iParent);
  soft_tex_mm = NULL;
  System = NULL;
  G2D = NULL;
  reprepare = false;
}

csSoftProcTexture3D::~csSoftProcTexture3D ()
{ 
  if ((head_texG3D != this) || reprepare)
  {
    // We are sharing the cache and texture manager
    tcache = NULL;
    texman = NULL;
  }
}

bool csSoftProcTexture3D::Initialize (iSystem *iSys)
{
  (System = iSys)->IncRef();
  return true;
}

void csSoftProcTexture3D::Print (csRect *area)
{
  csGraphics3DSoftwareCommon::Print (area);

  if (reprepare)
    soft_tex_mm->ReprepareProcTexture ();
}

bool csSoftProcTexture3D::Prepare 
    (csTextureMMSoftware *tex_mm, csGraphics3DSoftwareCommon *parent_g3d,
     csSoftProcTexture3D *first_texG3D, void *buffer,  uint8 *bitmap, 
     csPixelFormat *ipfmt, RGBPixel *palette, bool alone_hint)
{
  parent_tex_mm = tex_mm;
  tex_mm->GetMipMapDimensions (0, width, height);
  (System = parent_g3d->System)->IncRef ();
  iGraphics2D *g2d = parent_g3d->G2D;
  G2D = g2d->CreateOffScreenCanvas (width, height, 
				    bitmap ? (void*) bitmap : (void*) buffer, 
			      alone_hint ? csosbSoftwareAlone : csosbSoftware,
 			      ipfmt, palette, 256);

  if (!G2D)
  {
    System->Printf (MSG_FATAL_ERROR, 
		    "Error opening Graphics2D texture context.\n");
    return false;
  }

  if (!alone_hint && (ipfmt->PixelBytes != 1))
    reprepare = true;

  if ((ipfmt->PixelBytes == 1) || !alone_hint)
  {
    // We are utilising the main texture manager only
    SharedInitialize ((csGraphics3DSoftwareCommon*)parent_g3d);
    if (!Open (NULL) || !SharedOpen ())
      return false;
    soft_tex_mm = tex_mm;
  }
  else
  {
    if (!first_texG3D)
    {
      // We are the first procedural texture utilising a dedicated 
      // procedural texture manager working in 8bit
      head_texG3D = this;
      NewInitialize ();
      if (!Open (NULL) || !NewOpen ())
	return false;
      csTextureManagerSoftware *main_txtmgr = (csTextureManagerSoftware*)parent_g3d->GetTextureManager();
      texman->SetMainTextureManager (main_txtmgr);
      main_txtmgr->SetProcTextureManager (texman);
    }
    else
    {
      SharedInitialize ((csGraphics3DSoftwareCommon*)head_texG3D);
      if (!Open (NULL) || !SharedOpen ())
	return false;
    }

    // In order to keep the palette synchronised we register ourselves with
    // this texture manager.
    iImage *im = (iImage*) new csImageMemory (width, height, 
					      (RGBPixel*) buffer, false);
    soft_tex_mm = (csTextureMMSoftware *)
                  texman->RegisterTexture (im, CS_TEXTURE_2D | CS_TEXTURE_PROC);
  }

  return true;
}


  // The entry point for other than software graphics drivers..
iTextureHandle *csSoftProcTexture3D::CreateOffScreenRenderer 
    (iGraphics3D *parent_g3d, iGraphics3D* g3d_partner, int width, int height, 
     void *buffer, csOffScreenBuffer hint, csPixelFormat *ipfmt)
{
  // Always in 32bit
  // Here we create additional images for this texture which are registered as 
  // procedural textures with our own texture manager. This way if the procedural
  // texture is written to with itself, this texture manager will know about it
  // and act accordingly. This is done for both alone and sharing procedural
  // textures. In the case of the alone procedural texture it will never be 
  // utilised unless a sharing procedural texture is introduced into the 
  // system in which case the previous alone procedural textures are all converted
  // to sharing ones.

  // If this is a sharing procedural texture then the texture needs repreparing
  // each update.
  if (hint == csosbHardware)
    reprepare = true;

  G2D = parent_g3d->GetDriver2D ()->CreateOffScreenCanvas (width, height, buffer, 
							   hint, ipfmt, NULL, 0);

  if (!G2D)
  {
    System->Printf (MSG_FATAL_ERROR, 
		    "Error opening Graphics2D texture context.\n");
    return NULL;
  }

  if (g3d_partner)
  {
    partner = (csGraphics3DSoftwareCommon*)g3d_partner;
    SharedInitialize (partner);
    if (!Open (NULL) || !SharedOpen ())
      return NULL;
  }
  else
  {
    NewInitialize ();
    if (!Open (NULL) || !NewOpen ())
      return NULL;
  }

  // Register our own buffer as a procedural texture. 
  // The texture handles GetProcTextureInterface () is never called so
  // no additional interfaces are created.

  // We set the 'destroy' parameter for csImageMemory to false as these buffers 
  // are destroyed else where.
  iImage *tex_image = new csImageMemory (width, height, 
					 (RGBPixel*)buffer, false);

  soft_tex_mm = (csTextureMMSoftware *) texman->RegisterTexture (tex_image,
						 CS_TEXTURE_PROC | CS_TEXTURE_2D);
  texman->PrepareTexture ((iTextureHandle*)soft_tex_mm);

  // Return the software texture managers handle for this procedural texture.
  return soft_tex_mm;
}

void csSoftProcTexture3D::ConvertMode ()
{
  reprepare = true;
}
