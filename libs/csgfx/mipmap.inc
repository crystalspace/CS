/*
    Crystal Space 3D engine
    Copyright (C) 2000 by Jorrit Tyberghein
  
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

/*
    This is a general macro used to build all blending and mipmapping functions.
    The macro can blend images as well as scale them down by applying blending
    filters. The following preprocessor macros and variables should be defined
    or respectively declared before including this file:

    MIPMAP_NAME		- the name for the mipmapping function
    MIPMAP_LEVEL	- Mipmap filter level: level zero filter means to
			  perform blending; level one to three scales down.
    MIPMAP_PALETTED	- Source bitmap is a 8-bit paletted image
    MIPMAP_ALPHA	- Process image as alpha channel (i.e. like paletted
			  image without an actual palette).
    MIPMAP_TRANSPARENT	- Accept an additional parameter that specifies an
			  transparent color. Key colors are handled
			  in a special way.
*/

#ifndef __MIPMAP_INC__
#define __MIPMAP_INC__

#if defined (CS_LITTLE_ENDIAN)
#elif defined (CS_BIG_ENDIAN)
#endif

#endif // __MIPMAP_INC__

#define MIPMAP_STEP	(1 << MIPMAP_LEVEL)
#define MIPMAP_DIVIDER	(MIPMAP_STEP * MIPMAP_STEP)

#if defined (MIPMAP_PALETTED) || defined (MIPMAP_ALPHA)
#  define MIPMAP_PIXTYPE	UByte
#else
#  define MIPMAP_PIXTYPE	uint32
#endif

static void MIPMAP_NAME (unsigned w, unsigned h
#if defined (MIPMAP_ALPHA)
  , UByte *src, UByte *dst
#elif defined (MIPMAP_PALETTED)
  , UByte *src, csRGBpixel *dst, csRGBpixel *pal
#  ifdef MIPMAP_TRANSPARENT
  , UByte transp
#  endif
#else
  , csRGBpixel *src, csRGBpixel *dst
#  ifdef MIPMAP_TRANSPARENT
  , csRGBpixel transp
#  endif
#endif
  )
{
  unsigned x, y;

#if defined (MIPMAP_ALPHA)
#    define ACCUMULATE_PIXEL(p, m)					\
     a += p * m;
#elif defined (MIPMAP_PALETTED)
#  if defined (MIPMAP_TRANSPARENT) && (MIPMAP_LEVEL == 0)
#    define ACCUMULATE_PIXEL(p, m)					\
     {									\
       UByte pix = p;							\
       if (pix == transp) { *dst++ = pal [src2 [1]]; FINALLY; continue; }\
       uint32 rgb = *(uint32 *)&pal [pix];				\
       rb += (rgb & 0x00ff00ff) * m;					\
       ga += ((rgb >> 8) & 0x00ff00ff) * m;				\
     }
#  elif defined (MIPMAP_TRANSPARENT)
#    define ACCUMULATE_PIXEL(p, m)					\
     {									\
       UByte pix = p;							\
       uint32 rgb = *(uint32 *)&pal [pix];				\
       if (pix == transp) { *(uint32 *)dst++ = rgb; FINALLY; continue; }	\
       rb += (rgb & 0x00ff00ff) * m;					\
       ga += ((rgb >> 8) & 0x00ff00ff) * m;				\
     }
#  else
#    define ACCUMULATE_PIXEL(p, m)					\
     {									\
       uint32 rgb = *(uint32 *)&pal [p];					\
       rb += (rgb & 0x00ff00ff) * m;					\
       ga += ((rgb >> 8) & 0x00ff00ff) * m;				\
     }
#  endif	
#else
#  if defined (MIPMAP_TRANSPARENT) && (MIPMAP_LEVEL == 0)
#    define ACCUMULATE_PIXEL(p, m)		    			\
     {									\
       uint32 rgb = *(uint32 *)&p;					\
       if (rgb == *(uint32 *)&transp) { *(uint32 *)dst++ = src2 [1]; FINALLY; continue; }\
       rb += (rgb & 0x00ff00ff) * m;					\
       ga += ((rgb >> 8) & 0x00ff00ff) * m;				\
     }
#  elif defined (MIPMAP_TRANSPARENT)
#    define ACCUMULATE_PIXEL(p, m)		    			\
     {									\
       uint32 rgb = *(uint32 *)&p;					\
       if (rgb == *(uint32 *)&transp) { *(uint32 *)dst++ = rgb; FINALLY; continue; }\
       rb += (rgb & 0x00ff00ff) * m;					\
       ga += ((rgb >> 8) & 0x00ff00ff) * m;				\
     }
#  else
#    define ACCUMULATE_PIXEL(p, m)		    			\
     {									\
       uint32 rgb = *(uint32 *)&p;					\
       rb += (rgb & 0x00ff00ff) * m;					\
       ga += ((rgb >> 8) & 0x00ff00ff) * m;				\
     }
#  endif	
#endif

#if defined (MIPMAP_ALPHA)
#define MIPMAP_OUTPUT							\
    *dst++ = a / MIPMAP_DIVIDER;
#else
#define MIPMAP_OUTPUT							\
    *(uint32 *)dst++ = ((rb / MIPMAP_DIVIDER) & 0x00ff00ff) |		\
                      (((ga / MIPMAP_DIVIDER) << 8) & 0xff00ff00);
#endif

#define ACCUMULATE_2X2(delta)						\
  ACCUMULATE_PIXEL (src [delta], 1);					\
  ACCUMULATE_PIXEL (src [delta + 1], 1);				\
  ACCUMULATE_PIXEL (src [delta + w], 1);				\
  ACCUMULATE_PIXEL (src [delta + w + 1], 1);

#define ACCUMULATE_1X2(delta)						\
  ACCUMULATE_PIXEL (src [delta], 1);					\
  ACCUMULATE_PIXEL (src [delta + 1], 1);				

// We could call ACCUMULATE_2X2 four times, but
// this way its faster by about 10%...
#define ACCUMULATE_4X4(delta)						\
  ACCUMULATE_PIXEL (src [delta], 1);					\
  ACCUMULATE_PIXEL (src [delta + 1], 1);				\
  ACCUMULATE_PIXEL (src [delta + 2], 1);				\
  ACCUMULATE_PIXEL (src [delta + 3], 1);				\
  ACCUMULATE_PIXEL (src [delta + w], 1);				\
  ACCUMULATE_PIXEL (src [delta + w + 1], 1);				\
  ACCUMULATE_PIXEL (src [delta + w + 2], 1);				\
  ACCUMULATE_PIXEL (src [delta + w + 3], 1);				\
  ACCUMULATE_PIXEL (src [delta + w * 2], 1);				\
  ACCUMULATE_PIXEL (src [delta + w * 2 + 1], 1);			\
  ACCUMULATE_PIXEL (src [delta + w * 2 + 2], 1);			\
  ACCUMULATE_PIXEL (src [delta + w * 2 + 3], 1);			\
  ACCUMULATE_PIXEL (src [delta + w * 3], 1);				\
  ACCUMULATE_PIXEL (src [delta + w * 3 + 1], 1);			\
  ACCUMULATE_PIXEL (src [delta + w * 3 + 2], 1);			\
  ACCUMULATE_PIXEL (src [delta + w * 3 + 3], 1);			\

// Well... if we do it the way above it should consist of 64 lines...
// Besides, it rises performance just a little (if even ...)
#define ACCUMULATE_8X8(delta)						\
  ACCUMULATE_4X4 (delta);						\
  ACCUMULATE_4X4 (delta + 4);						\
  ACCUMULATE_4X4 (delta + w * 4);					\
  ACCUMULATE_4X4 (delta + w * 4 + 4);

#if MIPMAP_LEVEL == 0
  MIPMAP_PIXTYPE * const srcorg = (MIPMAP_PIXTYPE *)src;
  MIPMAP_PIXTYPE * const srcmax = (MIPMAP_PIXTYPE *)src + w * (h - 1);
#endif

  const unsigned nh = h >> MIPMAP_LEVEL;
  const unsigned nw = w >> MIPMAP_LEVEL;
  // Compute the adjusted width of mipmap
  const unsigned w1 = (w & ~((1 << MIPMAP_LEVEL) - 1)) - 1;

#if MIPMAP_LEVEL == 1
  if (nh == 0 && nw == 0) return;
  if (nh == 0 || nw == 0)
  {
#undef FINALLY
#define FINALLY
    unsigned int n = MAX (1, nh) * MAX (1, nw);
    for (x = 0; x <= n; x += MIPMAP_STEP)
    {
#ifndef MIPMAP_ALPHA
      unsigned rb = 0, ga = 0;
#else
      unsigned a = 0;
#endif
      ACCUMULATE_1X2 (x)
      MIPMAP_OUTPUT;
      src += 2;
    }
    return;
  }
#endif

  // Loop through all rows of output image
  for (y = 0; y < nh; y++)
  {
#if MIPMAP_LEVEL == 0
#undef MIPMAP_DIVIDER
#define MIPMAP_DIVIDER (1+2+1+2+4+2+1+2+1)
#define MIPMAP_PROCESS(d1,d2,d3)					\
    {									\
      ACCUMULATE_PIXEL (src1 [d1], 1);					\
      ACCUMULATE_PIXEL (src1 [d2], 2);					\
      ACCUMULATE_PIXEL (src1 [d3], 1);					\
      ACCUMULATE_PIXEL (src2 [d1], 2);					\
      ACCUMULATE_PIXEL (src2 [d2], 4);					\
      ACCUMULATE_PIXEL (src2 [d3], 2);					\
      ACCUMULATE_PIXEL (src3 [d1], 1);					\
      ACCUMULATE_PIXEL (src3 [d2], 2);					\
      ACCUMULATE_PIXEL (src3 [d3], 1);					\
    }

    // Precompute a pointer to previous row and to next row
    MIPMAP_PIXTYPE * src1 = (y == 0) ? srcmax : (MIPMAP_PIXTYPE *)src - w;
    MIPMAP_PIXTYPE * src2 = (MIPMAP_PIXTYPE *)src;
    MIPMAP_PIXTYPE * src3 = (y == nh - 1) ? srcorg : (MIPMAP_PIXTYPE *)src + w;

    // Process first and last column separately since they wrap around
#ifndef MIPMAP_ALPHA
    unsigned rb = 0, ga = 0;
#else
    unsigned a = 0;
#endif
#define FINALLY goto startloop;
    MIPMAP_PROCESS (w1, 0, 1);
    MIPMAP_OUTPUT;

#ifdef MIPMAP_TRANSPARENT
startloop:
#endif

    for (x = 1; x < w1; x++)
#else
    for (x = 0; x <= w1; x += MIPMAP_STEP)
#endif
    {
#ifndef MIPMAP_ALPHA
      unsigned rb = 0, ga = 0;
#else
      unsigned a = 0;
#endif

#undef FINALLY
#define FINALLY
#if MIPMAP_LEVEL == 0
#  undef FINALLY
#  define FINALLY src1++; src2++; src3++;
      MIPMAP_PROCESS (0, 1, 2);
      FINALLY;
#elif MIPMAP_LEVEL == 1
      ACCUMULATE_2X2 (x);
#elif MIPMAP_LEVEL == 2
      ACCUMULATE_4X4 (x);
#elif MIPMAP_LEVEL == 3
      ACCUMULATE_8X8 (x);
#endif

      MIPMAP_OUTPUT;
    }

#undef FINALLY
#define FINALLY src += w * MIPMAP_STEP;

#if MIPMAP_LEVEL == 0
    // Process last pixel in column
#ifndef MIPMAP_ALPHA
    rb = ga = 0;
#else
    a = 0;
#endif
    MIPMAP_PROCESS (0, 1, 0-(w - 2));
    MIPMAP_OUTPUT;
#endif

    FINALLY;
#undef FINALLY
  }
}

#undef ACCUMULATE_2X2
#undef ACCUMULATE_4X4
#undef ACCUMULATE_8X8
#undef ACCUMULATE_PIXEL
#undef MIPMAP_NAME
#undef MIPMAP_LEVEL
#undef MIPMAP_PALETTED
#undef MIPMAP_ALPHA
#undef MIPMAP_TRANSPARENT
#undef MIPMAP_PIXTYPE
#undef MIPMAP_PROCESS
#undef MIPMAP_OUTPUT
#undef MIPMAP_DIVIDER
#undef MIPMAP_STEP
