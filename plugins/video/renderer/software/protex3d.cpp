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

DECLARE_FACTORY (csSoftProcTexture3D)

IMPLEMENT_IBASE (csSoftProcTexture3D)
  IMPLEMENTS_INTERFACE (iPlugIn)
  IMPLEMENTS_INTERFACE (iGraphics3D)
  IMPLEMENTS_INTERFACE (iSoftProcTexture)
IMPLEMENT_IBASE_END

csSoftProcTexture3D::csSoftProcTexture3D (iBase *iParent)
  : csGraphics3DSoftwareCommon ()
{
  CONSTRUCT_IBASE (iParent);
  tex_mm = NULL;
  System = NULL;
  partner_g3d = NULL;
  G2D = NULL;
}

csSoftProcTexture3D::~csSoftProcTexture3D ()
{ 
  if (partner_g3d || tex_mm)
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

  if (tex_mm && pfmt.PixelBytes != 1)
    tex_mm->RePrepareProcTexture ();
}

bool csSoftProcTexture3D::Prepare 
    (csTextureMMSoftware *tex_mm, csGraphics3DSoftwareCommon *parent_g3d,
     csSoftProcTexture3D *partner_g3d, bool alone_hint, void *buffer, 
     csPixelFormat *ipfmt, RGBPixel *palette, int pal_size)
{
  int width, height;
  tex_mm->GetMipMapDimensions (0, width, height);
  (System = parent_g3d->System)->IncRef ();
  this->partner_g3d = partner_g3d;

  iGraphics2D *g2d = parent_g3d->G2D;
    
  G2D = g2d->CreateOffScreenCanvas (width, height, buffer, 
			      alone_hint ? csosbSoftwareAlone : csosbSoftware,
 			      ipfmt, palette, pal_size);

  if (!G2D)
  {
    System->Printf (MSG_FATAL_ERROR, 
		    "Error opening Graphics2D texture context.\n");
    return false;
  }

  if (partner_g3d)
  {
    SharedInitialize ((csGraphics3DSoftwareCommon*)partner_g3d);
    if (!Open (NULL) || !SharedOpen ())
      return false;
  }
  else if (!alone_hint)
  {
    this->tex_mm = tex_mm;
    SharedInitialize ((csGraphics3DSoftwareCommon*)parent_g3d);
    if (!Open (NULL) || !SharedOpen ())
      return false;
  }
  else
  {
    // alone_hint is true, but we are the first procedural texture
    NewInitialize ();
    if (!Open (NULL) || !NewOpen ())
      return false;
  }

  return true;
}

  // The entry point for other than software graphics drivers..
iTextureHandle *csSoftProcTexture3D::CreateOffScreenRenderer 
    (iGraphics3D *parent_g3d, iGraphics3D* partner, int width, int height, 
     void *buffer, csOffScreenBuffer hint, csPixelFormat *ipfmt)
{
  partner_g3d = (csGraphics3DSoftwareCommon*)partner;
  iGraphics2D *parent_g2d = parent_g3d->GetDriver2D ();

  // Here we create additional images for this texture which are registered as 
  // procedural textures with out own texture manager. This way if the procedural
  // texture is written to with itself, this texture manager will know about it
  // and act accordingly. This is done for both alone and sharing procedural
  // textures. In the case of the alone procedural texture it will never be 
  // utilised unless there is another sharing procedural texture in the system
  // in which case the previous alone procedural textures are converted to
  // sharing ones.
 
  // We set the 'destroy' parameter for csImageMemory to false as these buffers 
  // are destroyed else where.
  iImage *tex_image;

  if (ipfmt->PixelBytes == 4)
  {
    tex_image = new csImageMemory (width, height, 
				   (RGBPixel*)buffer, false);
    G2D = parent_g2d->CreateOffScreenCanvas (width, height, buffer, 
					     hint, ipfmt, NULL, 0);
  }
  else
  {
    // 16bit, no paletted opengl
    RGBPixel *image_buffer = new RGBPixel[width*height];
    tex_image = new csImageMemory (width, height, 
				  image_buffer, true);

    // Here we pass the image_buffer which is updated within our own texture
    // manager through the 'palette' parameter
    G2D = parent_g2d->CreateOffScreenCanvas (width, height, buffer, 
					     hint, ipfmt, image_buffer, 0);
  }

  if (!G2D)
  {
    System->Printf (MSG_FATAL_ERROR, 
		    "Error opening Graphics2D texture context.\n");
    return NULL;
  }

  if (partner)
  {
    SharedInitialize (partner_g3d);
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
  int flags = CS_TEXTURE_PROC | CS_TEXTURE_3D | CS_TEXTURE_NOMIPMAPS;
  iTextureHandle *soft_tex_handle = texman->RegisterTexture (tex_image, flags);
  texman->PrepareTexture (soft_tex_handle);

  if (hint == csosbHardware)
    tex_mm = (csTextureMMSoftware*) soft_tex_handle;
  else
    tex_mm_alone = (csTextureMMSoftware*) soft_tex_handle;

  // Return the software texture managers handle for this procedural texture.
  return soft_tex_handle;
}

void csSoftProcTexture3D::ConvertMode ()
{
  if (!tex_mm) tex_mm = tex_mm_alone;
}
