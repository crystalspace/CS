

//Depending on the Colorformat, we have different masks and shiftvalues 
//to get the accrding values for rgb:
#undef REDSHIFT
#undef GREENSHIFT
#undef GREENMASK
#undef BLUEMASK
#undef BITTYPE
#undef REDBITS
#undef GREENBITS
#undef BLUEBITS

#ifdef BPP16
  #define BITTYPE UShort

  #ifdef COLOR555
    #define REDSHIFT     10
    #define REDBITS       5
    #define GREENSHIFT    5
    #define GREENBITS     5
    #define GREENMASK  0x1f  
    #define BLUEMASK   0x1f
    #define BLUEBITS      5
  #endif

  #ifdef COLOR565
    #define REDSHIFT     11
    #define REDBITS       5
    #define GREENSHIFT    5
    #define GREENBITS     6
    #define GREENMASK  0x3f  
    #define BLUEMASK   0x1f
    #define BLUEBITS      5
  #endif
#endif

#ifdef BPP32
  #define BITTYPE ULong

  #define REDSHIFT     16
  #define REDBITS       8
  #define GREENSHIFT    8
  #define GREENBITS     8
  #define GREENMASK  0xff  
  #define BLUEMASK   0xff
  #define BLUEBITS      8
#endif

//This is the mother for all 16 and 32 Bit scanfunctions, as used by 
//DrawPolygonQuick and DrawPolygonFX.
//To use this include file, you will have to define the following Macros:
//
//PI_SCANFUNC: Name of the function
//BPP16:       generate code for 16 Bit display
//BPP32:       generate code for 32 Bit display
//GOURAUD:     define only, if you want gouraud shading
//COLOR555:    define, if you are using RGB555 format. Only needed, if you are
//             also using GOURAUD shading and you are using BPP16
//COLOR565:    define, if you are using RGB565 format. Only needed, if you are
//             also using GOURAUD shading and you are using BPP16
//COLORKEY:    define, if you want transparent sections of bitmaps to be drawn
//             transparent.
//FLAT:        The base for all drawing will be a virtual white texture. 
//             this setting makes no sense together with COLORKEY.
//ZFILL:       define, if the function is supposed to fill only the z-buffer,
//             but don't test it.
//ZUSE:        define, if you want full z-buffer support (set and test). This
//             can not be combined with ZFILL, but you can omit both.
void PI_SCANFUNC (void *dest, int len,
  unsigned long *zbuff, long u, long du, long v, long dv,
  unsigned long z, long dz, unsigned char *bitmap, int bitmap_log2w
#ifdef GOURAUD
  ,ULong r, ULong g, ULong b, long dr, long dg, long db
#endif
  )
{
  BITTYPE *_dest = (BITTYPE *)dest;
  BITTYPE *_destend = _dest + len;
  BITTYPE tex;

#ifdef GOURAUD
  ULong r1, g1, b1; //was int
#endif

  while (_dest < _destend)
  {

#ifdef ZUSE
    if (z >= *zbuff) 
#endif
    {

#ifdef FLAT
      //if flat shading is desired, we will just use white as our texel color
      (void)u; (void)v; (void)du; (void)dv; (void)bitmap; (void)bitmap_log2w;
      #ifndef GOURAUD
        //if Gouraud is selected, we don't need to intialize tex here.
        #ifdef BPP16
          tex = 0xffff;
        #else
          tex = 0xffffff;
        #endif
      #endif
#else
      //read the texel from the texture
      #ifdef BPP16
        tex = Scan.PaletteTable[*(bitmap + ((v >> 16) << bitmap_log2w) + (u >> 16))];
      #else
        tex = *(((ULong*)bitmap) + ((v >> 16) << bitmap_log2w) + (u >> 16));
      #endif
#endif

      #ifdef COLORKEY
        //if colorkeying is enabled, we will ignore texels with the value 0.
        if (tex)
      #endif
      {
        #ifdef GOURAUD
          #ifdef FLAT
            r1 = r >> 16;
            g1 = g >> 16;
            b1 = b >> 16;
          #else
            r1 =  tex >> REDSHIFT;
            g1 = (tex >> GREENSHIFT) & GREENMASK;
            b1 =  tex  & BLUEMASK;
            r1 = (r1*r) >> (16+REDBITS);
            g1 = (g1*g) >> (16+GREENBITS);
            b1 = (b1*b) >> (16+BLUEBITS);
          #endif
          tex = (r1<<REDSHIFT) | (g1<<GREENSHIFT) | b1;
        #endif

        *_dest = tex;
        #if defined(ZUSE) || defined (ZFILL)
          *zbuff = z;
        #endif
      }
    }
    _dest++;
    zbuff++;
    u += du; v += dv; z += dz;
#ifdef GOURAUD
    r += dr; g += dg; b += db;
#endif
  } /* endwhile */
}


#undef PI_SCANFUNC
#undef ZUSE
#undef ZFILL
#undef GOURAUD
#undef COLOR555
#undef COLOR565
#undef COLORKEY
#undef FLAT
#undef BPP16
#undef BPP32
