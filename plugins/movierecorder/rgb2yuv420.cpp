/*
 * This file is from:
 * http://mri.beckman.uiuc.edu/~pan/software.html
 *
 * It doesn't have any copyright notice that I could find.
 */


#include "cssysdef.h"
#include "cstypes.h"

static int RGB2YUV_YR[256], RGB2YUV_YG[256], RGB2YUV_YB[256];
static int RGB2YUV_UR[256], RGB2YUV_UG[256], RGB2YUV_UBVR[256];
static int                  RGB2YUV_VG[256], RGB2YUV_VB[256];

void InitLookupTable();

/************************************************************************
 *
 *  int RGB2YUV420 (int x_dim, int y_dim, 
 *				unsigned char *bmp, 
 *				unsigned char *yuv, int flip)
 *
 *	Purpose :	It takes a 24-bit RGB bitmap and convert it into
 *				YUV (4:2:0) format
 *
 *  Input :		x_dim	the x dimension of the bitmap
 *				y_dim	the y dimension of the bitmap
 *				bmp		pointer to the buffer of the bitmap
 *				yuv		pointer to the YUV structure
 *
 ************************************************************************/

#ifdef CS_BIG_ENDIAN
# define R(c)	 (c >> 24)
# define G(c)	((c >> 16) & 0xff)
# define B(c)	((c >>  8) & 0xff)
#else
# define R(c)	 (c & 0xff)
# define G(c)	((c >>  8) & 0xff)
# define B(c)	((c >> 16) & 0xff)
#endif

int RGB2YUV420 (int x_dim, int y_dim, 
		uint8 *bmp, 
		uint8 *yuv)
{
  int i, j;
  uint32 *rgb_line1, *rgb_line2;
  uint8 *y1, *y2, *u, *v;
  int pitch;
  pitch = x_dim;

  y1 = yuv;
  y2 = y1 + pitch;
  u = yuv + (x_dim * y_dim);
  x_dim >>= 1;
  y_dim >>= 1;
  v = u + (x_dim * y_dim);
  rgb_line1 = (uint32*)bmp;
  rgb_line2 = rgb_line1 + pitch;

  for (i=0; i < y_dim; i++){
    for (j=0; j < x_dim; j++){
      uint32 tu = 0, tv = 0;
      uint32 c;
      int r, g, b;
      
      c = *rgb_line1++;
      r = R(c), g = G(c), b = B(c);
      *y1++ =
	( RGB2YUV_YR[r]  +RGB2YUV_YG[g]+RGB2YUV_YB[b])>>16;
      tu += ( RGB2YUV_UR[r]  +RGB2YUV_UG[g]+RGB2YUV_UBVR[b]);
      tv += ( RGB2YUV_UBVR[r]+RGB2YUV_VG[g]+RGB2YUV_VB[b]);

      c = *rgb_line1++;
      r = R(c), g = G(c), b = B(c);
      *y1++ = 
	( RGB2YUV_YR[r]  +RGB2YUV_YG[g]+RGB2YUV_YB[b])>>16;
      tu += ( RGB2YUV_UR[r]  +RGB2YUV_UG[g]+RGB2YUV_UBVR[b]);
      tv += ( RGB2YUV_UBVR[r]+RGB2YUV_VG[g]+RGB2YUV_VB[b]);

      c = *rgb_line2++;
      r = R(c), g = G(c), b = B(c);
      *y2++ = 
	( RGB2YUV_YR[r]  +RGB2YUV_YG[g]+RGB2YUV_YB[b])>>16;
      tu += ( RGB2YUV_UR[r]  +RGB2YUV_UG[g]+RGB2YUV_UBVR[b]);
      tv += ( RGB2YUV_UBVR[r]+RGB2YUV_VG[g]+RGB2YUV_VB[b]);

      c = *rgb_line2++;
      r = R(c), g = G(c), b = B(c);
      *y2++ = 
	( RGB2YUV_YR[r]  +RGB2YUV_YG[g]+RGB2YUV_YB[b])>>16;
      tu += ( RGB2YUV_UR[r]  +RGB2YUV_UG[g]+RGB2YUV_UBVR[b]);
      tv += ( RGB2YUV_UBVR[r]+RGB2YUV_VG[g]+RGB2YUV_VB[b]);

      *u++ = tu >> 18;
      *v++ = tv >> 18;
    }
    rgb_line1 += pitch;
    rgb_line2 += pitch;
    y1 += pitch;
    y2 += pitch;
  }

  return 0;
}

void InitLookupTable()
{
  int i;

  for (i = 0; i < 256; i++) 
  {
    int isl8 = i << 8;
    RGB2YUV_YR[i] = (int) ((float)65.481 * (isl8));
    RGB2YUV_YG[i] = (int) ((float)128.553 * (isl8)) + 1048576;
    RGB2YUV_YB[i] = (int) ((float)24.966 * (isl8));
    RGB2YUV_UR[i] = (int) ((float)-37.797 * (isl8));
    RGB2YUV_UG[i] = (int) ((float)-74.203 * (isl8)) - 8388608;
    RGB2YUV_VG[i] = (int) ((float)-93.786 * (isl8)) - 8388608;
    RGB2YUV_VB[i] = (int) ((float)-18.214 * (isl8));
    RGB2YUV_UBVR[i] = (int) ((float)112 * (isl8));
  }
}

