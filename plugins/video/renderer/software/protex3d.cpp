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
#include "isys/system.h"
#include "ivideo/graph2d.h"
#include "soft_txt.h"
#include "tcache.h"
#include "csgfx/memimage.h"
#include "csgfx/rgbpixel.h"
#include "ivideo/texture.h"

SCF_IMPLEMENT_FACTORY (csSoftProcTexture3D)

SCF_IMPLEMENT_IBASE_EXT (csSoftProcTexture3D)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iSoftProcTexture)
SCF_IMPLEMENT_IBASE_EXT_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csSoftProcTexture3D::eiSoftProcTexture)
  SCF_IMPLEMENTS_INTERFACE (iSoftProcTexture)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

csSoftProcTexture3D::csSoftProcTexture3D (iBase *iParent)
  : csGraphics3DSoftwareCommon (iParent)
{
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiSoftProcTexture);
  is_for_procedural_textures = true;
  soft_tex_mm = NULL;
  parent_tex_mm = NULL;
  reprepare = false;
}

csSoftProcTexture3D::~csSoftProcTexture3D ()
{ 
  if (sharing)
  {
    // We are sharing the cache and texture manager
    tcache = NULL;
    texman = NULL;
  }
}

void csSoftProcTexture3D::Print (csRect *area)
{
  csGraphics3DSoftwareCommon::Print (area);

  if (reprepare)
    soft_tex_mm->ReprepareProcTexture ();

  // As we've printed something we need to uncache it so the effects will be 
  // registered next frame.
  if (parent_tex_mm)
    main_tcache->uncache_texture (0, (iTextureHandle*)parent_tex_mm);
}

bool csSoftProcTexture3D::Prepare (csTextureManagerSoftware *main_texman,
  csTextureHandleSoftware *tex_mm, void *buffer,  uint8 *bitmap)
{
  csGraphics3DSoftwareCommon* parent_g3d = main_texman->G3D;
  iGraphics2D *g2d = parent_g3d->GetDriver2D ();
  System = parent_g3d->System;

  main_tcache = parent_g3d->tcache;
  parent_tex_mm = tex_mm;
  parent_tex_mm->GetMipMapDimensions (0, width, height);
  int flags = parent_tex_mm->GetFlags ();
  bool use8bit = (flags & CS_TEXTURE_PROC_ALONE_HINT) == 
                     CS_TEXTURE_PROC_ALONE_HINT;
  // Cases:
  // Current Display..Texture Manager..tex_mm flags.....Reprepare
  //       8bit         main             none               no
  //     15/16bit       main             none               yes
  //     15/16bit       proc   CS_TEXTURE_PROC_ALONE_HINT   no
  //       32bit        main             none               yes
  //       32bit        proc   CS_TEXTURE_PROC_ALONE_HINT   no
  //

  if (main_texman->pfmt.PixelBytes == 1)
  {
    sharing = true;
    reprepare = false;
    // Global palette
    G2D = g2d->CreateOffScreenCanvas (width, height, bitmap, use8bit,
		     &main_texman->pfmt,  &main_texman->cmap.palette[0], 256);
    if (!G2D) return false;

    SharedInitialize (parent_g3d);
    if (!Open (NULL) || !SharedOpen ())
      return false;

#ifdef CS_DEBUG
    System->Printf (CS_MSG_INITIALIZATION, "8bit procedural texture\n");
#endif
  }
  else
  {
    if (!use8bit)
    {
      sharing = true;
      reprepare = true;
      soft_tex_mm = tex_mm;

      G2D = g2d->CreateOffScreenCanvas (width, height, buffer, false,
        &main_texman->pfmt, parent_tex_mm->GetColorMap (), 256);

      SharedInitialize (parent_g3d);
      if (!Open (NULL) || !SharedOpen ())
	return false;

#ifdef CS_DEBUG
      System->Printf (CS_MSG_INITIALIZATION, "%sbit procedural texture\n",
        main_texman->pfmt.PixelBytes == 2 ? "16" : "32");
#endif
    }
    else
    {
      reprepare = false;
      G2D = g2d->CreateOffScreenCanvas (width, height, bitmap, true,
        &main_texman->pfmt, parent_tex_mm->GetColorMap (), 256);
      if (!G2D) return false;

      if (!main_texman->GetFirst8bitProcTexture ())
      {
        sharing = false;
        // We are the first procedural texture utilising a dedicated 
        // procedural texture manager working in 8bit
        NewInitialize ();
        if (!Open (NULL) || !NewOpen ())
          return false;
        // Inform each texture manager aboout the other
        texman->SetMainTextureManager (main_texman);
        main_texman->SetProcTextureManager (texman);
        texman->ResetPalette ();
#ifdef CS_DEBUG
        System->Printf (CS_MSG_INITIALIZATION, 
          "Preparing dedicated procedural texture manager\n");
#endif
        // Initialize the texture manager
        int r,g,b;
        for (r = 0; r < 8; r++)
          for (g = 0; g < 8; g++)
            for (b = 0; b < 4; b++)
              texman->ReserveColor (r * 32, g * 32, b * 64);
        texman->PrepareTextures ();
      }
      else
      {
        sharing = true;
        SharedInitialize (main_texman->GetFirst8bitProcTexture ());
        if (!Open (NULL) || !SharedOpen ())
          return false;
      }

      if (buffer)
      {
        // In order to keep the palette synchronised 
        iImage *im = (iImage*) new csImageMemory (width, height, 
          (csRGBpixel *) buffer, false);
        iTextureHandle *dummy =
          texman->RegisterTexture (im, CS_TEXTURE_2D | CS_TEXTURE_PROC);
        dummy->Prepare ();
        dummy->DecRef ();
      }

#ifdef CS_DEBUG
      System->Printf (CS_MSG_INITIALIZATION, "%s/8bit procedural texture\n",
        main_texman->pfmt.PixelBytes == 2 ? "16" : "32");
#endif
    }
  }
  return true;
}

// The entry point for other than software graphics drivers..
iTextureHandle *csSoftProcTexture3D::CreateOffScreenRenderer
  (iGraphics3D *parent_g3d, iGraphics3D* g3d_partner, int width, int height, 
   void *buffer, csPixelFormat *ipfmt, int flags)
{
  // Always in 32bit
  // Here we create additional images for this texture which are registered as
  // procedural textures with our own texture manager.  This way if the
  // procedural texture is written to with itself, this texture manager will
  // know about it and act accordingly.  This is done for both alone and
  // sharing procedural textures.  In the case of the alone procedural texture
  // it will never be utilised unless a sharing procedural texture is
  // introduced into the system in which case the previous alone procedural
  // textures are all converted to sharing ones.

  // If this is a sharing procedural texture then the texture needs repreparing
  // each update.
  if ((flags & CS_TEXTURE_PROC_ALONE_HINT) !=  CS_TEXTURE_PROC_ALONE_HINT)
    reprepare = true;

  G2D = parent_g3d->GetDriver2D ()->CreateOffScreenCanvas (
    width, height, buffer, false, ipfmt, NULL, 0);

  if (!G2D)
  {
    System->Printf (CS_MSG_FATAL_ERROR, 
		    "Error opening Graphics2D texture context.\n");
    return NULL;
  }

  if (g3d_partner)
  {
    sharing = true;
    partner = (csGraphics3DSoftwareCommon*)g3d_partner;
    SharedInitialize (partner);
    if (!Open (NULL) || !SharedOpen ())
      return NULL;
  }
  else
  {
    sharing = false;
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
					 (csRGBpixel*)buffer, false);

  soft_tex_mm = (csTextureHandleSoftware *) 
        texman->RegisterTexture (tex_image, CS_TEXTURE_PROC | CS_TEXTURE_2D);
  soft_tex_mm->Prepare ();

  // Return the software texture managers handle for this procedural texture.
  return soft_tex_mm;
}
