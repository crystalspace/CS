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
			  perform bluring; level one scales down to 1/2 the width/height.
    MIPMAP_PALETTED	- Source bitmap is a 8-bit paletted image
    MIPMAP_ALPHA	- Process image as alpha channel (i.e. like paletted
			  image without an actual palette).
    MIPMAP_TRANSPARENT	- Accept an additional parameter that specifies an
			  transparent color. Key colors are handled
			  in a special way.
			  
	When scaling down a non-transparent image, one destination pixel is 
	calculated by averaging 4 source pixels.
	Transparent images work the same; however, the number of transparent
	and non-transparent pixels are counted. The resulting pixel is transparent 
	if more than 2 source pixels are transparent, otherwise the non-transparent
	pixels are averaged.
	
	When blurring an image, a destination pixel is calculated from 9 source 
	pixels. Transparent images work analogue to scaling down an image. 
	Transparency will NOT change.
*/

#ifndef __MIPMAP_INC__
#define __MIPMAP_INC__

#if defined (CS_LITTLE_ENDIAN)
#elif defined (CS_BIG_ENDIAN)
#endif

#endif // __MIPMAP_INC__

#define MIPMAP_STEP	(1 << MIPMAP_LEVEL)
#if MIPMAP_LEVEL == 0
# define MIPMAP_DIVIDER (1+2+1+2+4+2+1+2+1)
#else
# define MIPMAP_DIVIDER	(MIPMAP_STEP * MIPMAP_STEP)
#endif

#if defined (MIPMAP_PALETTED) || defined (MIPMAP_ALPHA)
#  define MIPMAP_PIXTYPE	uint8
#else
#  define MIPMAP_PIXTYPE	uint32
#endif

#if defined (MIPMAP_ALPHA)
static void MIPMAP_NAME (unsigned int w, unsigned int h, const uint8 *src, uint8 *dst)
#elif defined (MIPMAP_PALETTED)
#  ifdef MIPMAP_TRANSPARENT
static void MIPMAP_NAME (unsigned int w, unsigned int h, const uint8 *src, csRGBpixel *dst, const csRGBpixel *pal, uint8 transp)
#  else
static void MIPMAP_NAME (unsigned int w, unsigned int h, const uint8 *src, csRGBpixel *dst, const csRGBpixel *pal)
#  endif
#else
#  ifdef MIPMAP_TRANSPARENT
static void MIPMAP_NAME (unsigned int w, unsigned int h, const csRGBpixel *src, csRGBpixel *dst, csRGBpixel transp)
#  else
static void MIPMAP_NAME (unsigned int w, unsigned int h, const csRGBpixel *src, csRGBpixel *dst)
#  endif
#endif
{
  unsigned int x, y;

#ifndef MIPMAP_ALPHA
# ifdef MIPMAP_PALETTED
  // retrieve a source pixel as RGB
#  define MIPMAP_RGB(p)		*((uint32 *)pal + *((uint8*)p))
  // check if source pixel is transparent
#  define MIPMAP_ISKEY(p)	((unsigned int)(*((uint8*)p)) == (unsigned int)transp)
# else
#  define MIPMAP_RGB(p)		*(uint32 *)p
#  define MIPMAP_ISKEY(p)	(*(uint32 *)p == *(uint32 *)( (intptr_t)&transp))
# endif 
#endif

// ACCUMULATE_PIXEL - retrieve a source pixel
#if defined (MIPMAP_ALPHA)
#    define ACCUMULATE_PIXEL(p, m)					\
     a += *((uint8*)p) * (long)m;
#else
#  if defined (MIPMAP_TRANSPARENT)
#    define ACCUMULATE_PIXEL(p, m)					\
     {									\
       if (MIPMAP_ISKEY(p))						\
       {								\
	 transcnt += m;							\
       }								\
       else								\
       {								\
         uint32 rgb = MIPMAP_RGB(p);					\
         rb += (rgb & 0x00ff00ff) * m;					\
         ga += ((rgb >> 8) & 0x00ff00ff) * m;				\
       }								\
     }
#  else
#    define ACCUMULATE_PIXEL(p, m)					\
     {									\
       uint32 rgb = MIPMAP_RGB(p);					\
       rb += (rgb & 0x00ff00ff) * m;					\
       ga += ((rgb >> 8) & 0x00ff00ff) * m;				\
     }
#  endif
#endif

// MIPMAP_OUTPUT - calculates destination pixel 
#if defined (MIPMAP_ALPHA)
#define MIPMAP_OUTPUT							\
    *dst++ = a / MIPMAP_DIVIDER;
#else
# ifdef MIPMAP_TRANSPARENT
# define MIPMAP_OUTPUT							\
	if (transcnt)							\
	{								\
	  if (transcnt <= (MIPMAP_DIVIDER / 2))				\
	  {								\
	    int div = (MIPMAP_DIVIDER - transcnt);			\
	    /* need to treat every component separately...  */		\
	    *(uint32 *)dst++ =  (((rb & 0x0000ffff) / div) |		\
				 ((rb / div) & 0x00ff0000)) |		\
				 (((((ga & 0x0000ffff) / div) |		\
				 ((ga / div)& 0x00ff0000)) << 8) & 0xff00ff00); \
	  }								\
	  else								\
	  {								\
	    *(uint32 *)dst++ = MIPMAP_RGB(&transp);			\
	  }								\
	}								\
	else								\
	{								\
	  *(uint32 *)dst++ = ((rb / (MIPMAP_DIVIDER)) & 0x00ff00ff) |	\
	  		     (((ga / (MIPMAP_DIVIDER)) << 8) & 0xff00ff00); \
	}
# else
# define MIPMAP_OUTPUT							\
    *(uint32 *)dst++ = ((rb / MIPMAP_DIVIDER) & 0x00ff00ff) |		\
                      (((ga / MIPMAP_DIVIDER) << 8) & 0xff00ff00);
# endif
#endif

#define ACCUMULATE_2X2(delta)						\
  ACCUMULATE_PIXEL ((src + delta), 1);					\
  ACCUMULATE_PIXEL ((src + delta + 1), 1);				\
  ACCUMULATE_PIXEL ((src + delta + w), 1);				\
  ACCUMULATE_PIXEL ((src + delta + w + 1), 1);

#define ACCUMULATE_1X2(delta)						\
  ACCUMULATE_PIXEL ((src + delta), 1);					\
  ACCUMULATE_PIXEL ((src + delta + 1), 1);

#if MIPMAP_LEVEL == 0
  MIPMAP_PIXTYPE * const srcorg = (MIPMAP_PIXTYPE *)src;
  MIPMAP_PIXTYPE * const srcmax = (MIPMAP_PIXTYPE *)src + w * (h - 1);
#endif

  // Compute the adjusted width of mipmap
  const unsigned int w1 = (w & ~((1 << MIPMAP_LEVEL) - 1)) - 1;
  const unsigned int nh = h >> MIPMAP_LEVEL;
  
#if MIPMAP_LEVEL == 1
  const unsigned int nw = w >> MIPMAP_LEVEL;
  if (nh == 0 && nw == 0) return;
  // special case: just one row or column
  if (nh == 0 || nw == 0)
  {
    unsigned int n = h * w;
    for (x = 0; x < n; x += MIPMAP_STEP)
    {
#ifndef MIPMAP_ALPHA
      unsigned int rb = 0, ga = 0;
# ifdef MIPMAP_TRANSPARENT
	  int transcnt = 0;
# endif
#else
      unsigned int a = 0;
#endif
      ACCUMULATE_1X2 (x)
      MIPMAP_OUTPUT;
    }
    return;
  }
#endif

#if MIPMAP_LEVEL == 0

# define MIPMAP_PROCESS(d1,d2,d3)					\
      {									\
        ACCUMULATE_PIXEL ((src1 + d1), 1);				\
        ACCUMULATE_PIXEL ((src1 + d2), 2);				\
        ACCUMULATE_PIXEL ((src1 + d3), 1);				\
        ACCUMULATE_PIXEL ((src2 + d1), 2);				\
        ACCUMULATE_PIXEL ((src2 + d2), 4);				\
        ACCUMULATE_PIXEL ((src2 + d3), 2);				\
        ACCUMULATE_PIXEL ((src3 + d1), 1);				\
	ACCUMULATE_PIXEL ((src3 + d2), 2);				\
	ACCUMULATE_PIXEL ((src3 + d3), 1);				\
      }

  if (w1 == 0) // special case: width == 1
  {
    for (y = 0; y < nh; y++)
    {
      // Precompute a pointer to previous row and to next row
      MIPMAP_PIXTYPE * src1 = (y == 0) ? srcmax : (MIPMAP_PIXTYPE *)src - w;
      MIPMAP_PIXTYPE * src2 = (MIPMAP_PIXTYPE *)src;
      MIPMAP_PIXTYPE * src3 = (y == nh - 1) ? srcorg : (MIPMAP_PIXTYPE *)src + w;
#ifndef MIPMAP_ALPHA
      unsigned int rb = 0, ga = 0;
# ifdef MIPMAP_TRANSPARENT
      unsigned int transcnt = 0;
# endif
#else
      unsigned int a = 0;
#endif
		
# ifdef MIPMAP_TRANSPARENT
      // transparent and non-transparent areas should be the same 
      // on src and dst
      if (MIPMAP_ISKEY(src2))
      {
        *(uint32 *)dst++ = MIPMAP_RGB(src2);
        src += MIPMAP_STEP;
        continue;
      }
# endif
      MIPMAP_PROCESS (0, 0, 0);
      MIPMAP_OUTPUT;
      src += MIPMAP_STEP;
    }
  }
  else
#endif
  {
    // Loop through all rows of output image
    for (y = 0; y < nh; y++)
    {
#if MIPMAP_LEVEL == 0
      // Precompute a pointer to previous row and to next row
      MIPMAP_PIXTYPE * src1 = (y == 0) ? srcmax : (MIPMAP_PIXTYPE *)src - w;
      MIPMAP_PIXTYPE * src2 = (MIPMAP_PIXTYPE *)src;
      MIPMAP_PIXTYPE * src3 = (y == nh - 1) ? srcorg : (MIPMAP_PIXTYPE *)src + w;
#endif

      for (x = 0; x <= w1; x += MIPMAP_STEP)
      {
#ifndef MIPMAP_ALPHA
        unsigned int rb = 0, ga = 0;
# ifdef MIPMAP_TRANSPARENT
    	unsigned int transcnt = 0;
# endif
#else
        unsigned int a = 0;
#endif

#if MIPMAP_LEVEL == 0

# ifdef MIPMAP_TRANSPARENT
  	// transparent and non-transparent areas should be the same 
	// on src and dst
	if (MIPMAP_ISKEY(src2))
	{
	  *(uint32 *)dst++ = MIPMAP_RGB(src2);
	  src1++; src2++; src3++;
	  continue;
	}
# endif
	// Process first and last column separately since they wrap around
	if (x == 0)
	{
	  MIPMAP_PROCESS (w1, 0, 1);
	} 
	else if (x == w1)
	{
	  MIPMAP_PROCESS (-1, 0, -((int)w1));
	}
	else
	{
          MIPMAP_PROCESS (-1, 0, 1);
        }
	src1++; src2++; src3++;
#elif MIPMAP_LEVEL == 1
        ACCUMULATE_2X2 (x);
#endif

        MIPMAP_OUTPUT;
      }
      src += w * MIPMAP_STEP;
    }
  }
}

#undef MIPMAP_RGB
#undef MIPMAP_ISKEY
#undef ACCUMULATE_2X2
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
