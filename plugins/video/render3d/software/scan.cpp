/*
    Copyright (C) 1998-2000 by Jorrit Tyberghein
    Written by Andrew Zabolotny

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
#include "csqint.h"

#include "csutil/cscolor.h"

#include "ivideo/polyrender.h"

#include "scan.h"
#include "sft3dcom.h"
#include "soft_txt.h"
#include "tcache.h"
#include "sttest.h"

/// The only instance of this structure lives here
csScanSetup Scan;

//---------------------- This routine is pixel-depth independent ---

#ifndef NO_scan_zfil

void csScan_scan_zfil (int xx, unsigned char* d,
  uint32* z_buf, float inv_z, float u_div_z, float v_div_z)
{
  (void)u_div_z; (void)v_div_z; (void)inv_z; (void)d;
  uint32* lastZbuf = z_buf + xx-1;
  int dzz, izz;
  izz = csQfixed24 (inv_z);
  dzz = csQfixed24 (Scan.M);

  do
  {
    *z_buf++ = izz;
    izz += dzz;
  }
  while (z_buf <= lastZbuf);
}

#endif // NO_scan_zfil

//------------------------------------------------------------------

void csScan_scan_pi_zfil (void *dest, int len, uint32 *zbuff,
  int32 u, int32 du, int32 v, int32 dv, uint32 z, int32 dz,
  unsigned char *bitmap, int bitmap_log2w)
{
  (void)u; (void)du; (void)v; (void)dv;
  (void)bitmap; (void)bitmap_log2w; (void)dest;
  uint32 *lastZbuf = zbuff + len - 1;

  do
  {
    *zbuff++ = z;
    z += dz;
  }
  while (zbuff <= lastZbuf);
}

//------------------------------------------------------------------

void csScan_Initialize ()
{
  Scan.InterpolStep = 16;
  Scan.InterpolShift = 4;
  Scan.InterpolMode = INTER_MODE_SMART;

  // 16K
  Scan.one_div_z = new unsigned int [1 << 12];
  // ~1.5K
  Scan.exp_256 = new unsigned char [EXP_256_SIZE+3];
  // 6*8K
  memset (&Scan.BlendingTable, 0, sizeof (Scan.BlendingTable));
  memset (&Scan.BlendingTableProc, 0, sizeof (Scan.BlendingTableProc));

  int i;
  for (i = 1; i < (1 << 12); i++)
    Scan.one_div_z [i] = csQround (float (0x1000000) / float (i));
  Scan.one_div_z [0] = Scan.one_div_z [1];

  for (i = 0; i < EXP_256_SIZE; i++)
    Scan.exp_256 [i] = csQround (255 * exp (-float (i) / 256.));
}

void csScan_CalcBlendTables (unsigned char *BlendingTable[], int rbits,
  int gbits, int bbits)
{
  int i;
  // First free old blending tables
  for (i = 0; i < 6; i++)
    if (BlendingTable [i])
      delete [] BlendingTable [i];

  // Compute number of bits for our blending table
  unsigned int bits = MAX (MAX (rbits, gbits), bbits);
  unsigned int max = (1 << bits) - 1;
  // for modes with different bits per comp (i.e. R5G6B5) we create second table
  unsigned int bits2 = (rbits == gbits && gbits == bbits) ? 0 : rbits;
  unsigned int max2 = (1 << bits2) - 1;

  // For memory size reasons don't allow tables for more than 6 bits per color
  unsigned int add_shft = 0, add_val = 0;
  if (bits > 6)
  {
    add_shft = bits - 6;
    add_val = 1 << (add_shft - 1);
    bits = 6;
  }

  // Now allocate memory for blending tables
  unsigned int table_size = 1 << (2 * bits + 1);
  if (bits2)
    table_size += 1 << (2 * bits2 + 1);
  for (i = 0; i < NUMBLENDINGTABLES; i++)
    BlendingTable [i] = new unsigned char [table_size];

  unsigned int index = 0;

  do
  {
    unsigned int max_src, max_dest, max_val, max_bits;
    max_bits = bits2 ? bits2 : bits;
    max_src = 2 << max_bits;
    max_dest = 1 << max_bits;
    max_val = bits2 ? max2 : max;
    bits2 = 0; max_bits += add_shft;

    for (unsigned int s = 0; s < max_src; s++)
    {
      unsigned int src = (s << add_shft) + add_val;
      for (unsigned int d = 0; d < max_dest; d++)
      {
        unsigned int dst = (d << add_shft) + add_val;
        // Calculate all the available blendingmodes supported.
        #define CALC(idx,val)                            \
        {                                    \
          register unsigned int tmp = val;                   \
          BlendingTable [idx] [index] = (tmp < max_val) ? tmp : max_val;\
        }
        CALC (BLENDTABLE_ADD,       dst + src);
        CALC (BLENDTABLE_MULTIPLY,  ((dst * src) + (max_val / 2)) >> max_bits);
        CALC (BLENDTABLE_MULTIPLY2, ((dst * src * 2) + (max_val / 2)) >> max_bits);
        CALC (BLENDTABLE_ALPHA25,   (dst + src * 3 + 2) / 4);
        CALC (BLENDTABLE_ALPHA50,   (dst + src + 1) / 2);
        CALC (BLENDTABLE_ALPHA75,   (dst * 3 + src + 2) / 4);
        #undef CALC
        index++;
      }
    }
  } while (index < table_size);
}

void csScan_Finalize ()
{
  int i;
  for (i = 0; i < NUMBLENDINGTABLES; i++)
  {
    delete [] Scan.BlendingTable [i];
    delete [] Scan.BlendingTableProc [i];
  }
  delete [] Scan.exp_256;
  delete [] Scan.one_div_z;
}

void csScan_InitDraw (int MipMap, csSoftwareGraphics3DCommon* g3d,
  csPolyTextureMapping* tmapping,
  csSoftRendererLightmap* rlm, csSoftwareTextureHandle* texture,
  csSoftwareTexture *untxt)
{
  Scan.Texture = texture;
  Scan.bitmap = untxt->get_bitmap ();
  Scan.shf_w = untxt->get_w_shift ();
  Scan.and_w = untxt->get_w_mask ();
  Scan.shf_h = untxt->get_h_shift ();
  Scan.and_h = untxt->get_h_mask ();
  Scan.PaletteTable = texture->GetPaletteToGlobal ();

  Scan.FlatColor = g3d->GetDriver2D ()->FindRGB (255, 255, 255);

  if (g3d->do_lighting)
  {
    SoftwareCachedTexture *tclt =
      rlm ? (SoftwareCachedTexture *)rlm->cacheData[MipMap] : 0;
    if (tclt)
      Scan.bitmap2 = tclt->get_bitmap ();
    else
      Scan.bitmap2 = 0; // Not a lighted texture.
  }
  else
    Scan.bitmap2 = 0;
    
  if(tmapping) 
  {
      Scan.tw2 = tmapping->GetLitWidth () >> MipMap;
      Scan.th2 = tmapping->GetLitHeight () >> MipMap;
  } 
  else 
  {
      Scan.tw2 = 0;
      Scan.th2 = 0;
  }
  
  Scan.min_u = (tmapping->GetIMinU () >> MipMap) << 16;
  Scan.min_v = (tmapping->GetIMinV () >> MipMap) << 16;

#ifdef STUPID_TEST
  Scan.tw2fp = (Scan.tw2 << 16) - 1;
  Scan.th2fp = (Scan.th2 << 16) - 1;
#endif
  Scan.shf_u = tmapping->GetShiftU () - MipMap;

  Scan.and_h <<= Scan.shf_w;
  Scan.shf_h = 16 - Scan.shf_w;
}

void csScan_InitDrawFX (csSoftwareTextureHandle* texture,
        csSoftwareTexture *untxt)
{
  Scan.shf_w = untxt->get_w_shift ();
  Scan.and_w = untxt->get_w_mask ();
  Scan.shf_h = untxt->get_h_shift ();
  Scan.and_h = untxt->get_h_mask ();

  Scan.PaletteTable = texture->GetPaletteToGlobal ();
  Scan.TexturePalette = texture->GetColorMap ();
  Scan.AlphaMap = untxt->get_alphamap ();
}

