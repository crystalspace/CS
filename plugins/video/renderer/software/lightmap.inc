/*
    This file contains the routine that creates lightmapped textures.
    Basically it takes a texture, a lightmap and combines them together.
    This routine is used for all three bit depths - 8, 16 and 32bpp.
    The following macros can be defined in order to control the way
    output lighted texture is created:

	LM_NAME		The name of the routine.
	PI_INDEX8,
	PI_R5G5B5,
	PI_R5G6B5,
	PI_R8G8B8	The pixel format.
*/

/*
    Lots of actors comes into play inside the following routine. All those
    polygon textures, lightmaps, textureMM, bitmaps etc causes big troubles
    if you're trying to understand how the algorithm works. Let's try
    to classify them and introduce a notation for intermediate variable
    that will (I hope) ease the understanding:

	- iPolygonTexture is the object that is in charge for containing
	  all parameters related to a single mipmap level on a Polygon3D.
	  The variables that contain values related to iPolygonTexture
	  will have the "pt" prefix, i.e. pt_h for example is PolygonTexture's
	  height.
	- iLightMap is the object that is in charge for containing all
	  lighting information for the iPolygonTexture. Basically it contains
	  three lightmaps: red, green an blue, each cell of the lightmap
	  covers a square of texels. The square size can be queried from
	  polygontexture object using GetMipMapSize () method. For example,
	  if it returns 16, this means that every cell of the lightmap
	  covers a 16x16 square of texels on the texture. The prefix for
	  intermediate variables related to the lightmap is "lm".
	- csTexture is the actual unlighted (original) texture in
          driver-dependent format. The csTextureMMSoftware object
	  contains four of these objects - one for each mipmap level.
	  The intermediate variables related to csTexture are prefixed
	  with "tx".
*/

#include "pixtype.inc"

void LM_NAME (csBitSet *dirty, iPolygonTexture *pt,
  void *dst, csTextureManagerSoftware *texman)
{
  int pt_minu = pt->GetIMinU ();
  int pt_minv = pt->GetIMinV ();
  int pt_mipmap_size = pt->GetMipMapSize ();
  int pt_mipmap_shift = pt->GetMipMapShift ();
  int pt_shfw = pt->GetShiftU ();
  int pt_deltaw = pt->GetWidth ();
  int pt_h = pt->GetHeight ();

  iLightMap *lm = pt->GetLightMap ();
  int lm_w = lm->GetWidth ();
  UByte *lm_R = lm->GetMap (0);
  UByte *lm_G = lm->GetMap (1);
  UByte *lm_B = lm->GetMap (2);

  csTextureMMSoftware *texmm =
    (csTextureMMSoftware *)pt->GetTextureHandle ()->GetPrivateObject ();
  csTexture *tx = texmm->get_texture (pt->GetMipmapLevel ());
  RGBPixel *tx_palette = texmm->GetColorMap ();
  UByte *tx_bitmap = (UByte *)tx->get_bitmap ();

  int tx_shfw = tx->get_w_shift ();
  int tx_andw = tx->get_w_mask ();
  int tx_andh = tx->get_h_mask ();

/*
    The textures are lighted in squares. A single lightmap value corresponds
    to all pixels in a square on the texture. To get more pleasant results,
    we interpolate the lightmap along square edges. The square edges are
    labeled 00, 10, 01 and 11 as shown below.
                We hold the R/G/B values for all four corners of the square
    00      10  in variables called r00, g00, b00, r10, g10 and so on.
    *--------*  The square is scanned from top to bottom; during this scan
    |        |  the corresponding R/G/B values are kept in r0, g0, b0 and
   0*--------*1 r1, g1, b1 and interpolated while moving from top to bottom
    |        |  using r0d/g0d/b0d and r1d/g1d/b1d deltas. When scanning
    |        |  from left to right the deltas are kept in rd, gd and bd.
    *--------*
    01      11
*/

  int max_lu = ((pt->GetOriginalWidth () + pt_mipmap_size - 1) >> pt_mipmap_shift);
  int max_lv = lm->GetHeight ();
  int src = 0;

  // Get the multiplication tables for red, green and blue
  UByte *lt_R = texman->lightmap_tables [0];
  UByte *lt_G = texman->lightmap_tables [1];
  UByte *lt_B = texman->lightmap_tables [2];

#if defined (PI_INDEX8)
  UByte *inv_cmap = texman->inv_cmap;
#endif

  int dirty_bit = 0;
  for (int lv = 0; lv < max_lv; lv++)
  {
    for (int lu = 0; lu < max_lu; lu++)
    {
      if (dirty && !dirty->Get (dirty_bit++))
        continue;

#define GET(map, col)				\
      UByte col##00, col##10, col##01, col##11;	\
      col##00 = map [src];			\
      col##10 = map [src + 1];			\
      col##01 = map [src + lm_w];		\
      col##11 = map [src + lm_w + 1];

      GET (lm_R, r);
      GET (lm_G, g);
      GET (lm_B, b);

#undef GET

      int u = lu << pt_mipmap_shift;
      int v = lv << pt_mipmap_shift;
      PI_PIXTYPE *out = ((PI_PIXTYPE *)dst) + (v << pt_shfw) + u;

      int r0 = r00 << 6, r0d = ((r01 - r00) << (6 - pt_mipmap_shift));
      int r1 = r10 << 6, r1d = ((r11 - r10) << (6 - pt_mipmap_shift));
      int g0 = g00 << 6, g0d = ((g01 - g00) << (6 - pt_mipmap_shift));
      int g1 = g10 << 6, g1d = ((g11 - g10) << (6 - pt_mipmap_shift));
      int b0 = b00 << 6, b0d = ((b01 - b00) << (6 - pt_mipmap_shift));
      int b1 = b10 << 6, b1d = ((b11 - b10) << (6 - pt_mipmap_shift));

      for (int vv = 0; vv < pt_mipmap_size; vv++)
      {
        if (v + vv >= pt_h) break;

        int tx_idx = ((v + vv + pt_minv) & tx_andh) << tx_shfw;
        PI_PIXTYPE *__out = out;

        int r = r0, rd = (r1 - r0) >> pt_mipmap_shift;
        int g = g0, gd = (g1 - g0) >> pt_mipmap_shift;
        int b = b0, bd = (b1 - b0) >> pt_mipmap_shift;

        int uu = u + pt_minu;
        int end_uu = uu + pt_mipmap_size;

        for (; uu < end_uu; uu++)
        {
          RGBPixel pix = tx_palette [tx_bitmap [tx_idx + (uu & tx_andw)]];
          *out++ =
#if defined (PI_INDEX8)
            inv_cmap [
#endif
            (lt_R [pix.red   | (r & 0x3f00)] << PI_RS) |
            (lt_G [pix.green | (g & 0x3f00)] << PI_GS) |
            (lt_B [pix.blue  | (b & 0x3f00)] << PI_BS)
#if defined (PI_INDEX8)
            ]
#endif
          ;
          r += rd; g += gd; b += bd;
        }

        out = __out + pt_deltaw;

        r0 += r0d; r1 += r1d;
        g0 += g0d; g1 += g1d;
        b0 += b0d; b1 += b1d;
      }

      src++;
    }
    src += lm_w - max_lu;
  }
}

#undef LM_NAME
#undef PI_PIXTYPE
#undef PI_INDEX8
#undef PI_R5G5B5
#undef PI_R5G6B5
#undef PI_R8G8B8
#undef PI_RM
#undef PI_RS
#undef PI_RB
#undef PI_GM
#undef PI_GS
#undef PI_GB
#undef PI_BM
#undef PI_BS
#undef PI_BB
