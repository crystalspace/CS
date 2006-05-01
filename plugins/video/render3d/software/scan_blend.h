/*
    Copyright (C) 2005 by Jorrit Tyberghein
              (C) 2005 by Frank Richter

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

#ifndef __CS_SOFT3D_SCAN_BLEND_H__
#define __CS_SOFT3D_SCAN_BLEND_H__

CS_PLUGIN_NAMESPACE_BEGIN(Soft3D)
{

  enum
  {
    FactorColorSrc = 0,
    FactorColorDst = 1
  };

  template<int Color, bool Inv>
  struct Factor_Zero
  {
    CS_FORCEINLINE
    int GetBlendFact() const 
    { return Inv ? CS_MIXMODE_FACT_ONE : CS_MIXMODE_FACT_ZERO; }

    CS_FORCEINLINE
    Pixel Apply (const Pixel src, const Pixel dst) const
    {
      return Inv ? ((Color == FactorColorSrc) ? src : dst) : Pixel (0);
    }
  };

  template<int Color, bool Inv>
  struct Factor_Src
  {
    CS_FORCEINLINE
    int GetBlendFact() const 
    { return Inv ? CS_MIXMODE_FACT_SRCCOLOR_INV : CS_MIXMODE_FACT_SRCCOLOR; }

    CS_FORCEINLINE
    Pixel Apply (const Pixel src, const Pixel dst)
    {
      return ((Color == FactorColorSrc) ? src : dst) * (Inv ? ~src : src);
    }
  };

  template<int Color, bool Inv>
  struct Factor_SrcAlpha
  {
    CS_FORCEINLINE
    int GetBlendFact() const 
    { return Inv ? CS_MIXMODE_FACT_SRCALPHA_INV : CS_MIXMODE_FACT_SRCALPHA; }

    CS_FORCEINLINE
    Pixel Apply (const Pixel src, const Pixel dst)
    {
      return ((Color == FactorColorSrc) ? src : dst) * (Inv ? ~src.c.a : src.c.a);
    }
  };

  template<int Color, bool Inv>
  struct Factor_Dst
  {
    CS_FORCEINLINE
    int GetBlendFact() const 
    { return Inv ? CS_MIXMODE_FACT_DSTCOLOR_INV : CS_MIXMODE_FACT_DSTCOLOR; }

    CS_FORCEINLINE
    Pixel Apply (const Pixel src, const Pixel dst)
    {
      return ((Color == FactorColorSrc) ? src : dst) * (Inv ? ~dst : dst);
    }
  };

  template<int Color, bool Inv>
  struct Factor_DstAlpha
  {
    CS_FORCEINLINE
    int GetBlendFact() const 
    { return Inv ? CS_MIXMODE_FACT_DSTALPHA_INV : CS_MIXMODE_FACT_DSTALPHA; }

    CS_FORCEINLINE
    Pixel Apply (const Pixel src, const Pixel dst)
    {
      return ((Color == FactorColorSrc) ? src : dst) * (Inv ? ~dst.c.a : dst.c.a);
    }
  };

}
CS_PLUGIN_NAMESPACE_END(Soft3D)

#endif // __CS_SOFT3D_SCAN_BLEND_H__
