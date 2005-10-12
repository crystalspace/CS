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

#ifndef __CS_SOFTSHADER_SCAN_BLEND_H__
#define __CS_SOFTSHADER_SCAN_BLEND_H__

namespace cspluginSoftshader
{
  
  template<typename Pix, typename Target>
  struct Blend_Zero
  {
    typedef typename_qualifier Pix::PixType PixType;
    typedef Target TargetType;

    Target& target;
    Blend_Zero (const Pix& /*pix*/, Target& target) : target (target) 
    { }
    
    CS_FORCEINLINE
    void Apply (PixType* /*dest*/, 
      uint8 /*r*/, uint8 /*g*/, uint8 /*b*/, uint8 /*a*/)
    {
      target.Set (0);
    }
  };
  
  template<typename Pix, typename Target>
  struct Blend_SrcColor
  {
    typedef typename_qualifier Pix::PixType PixType;
    typedef Target TargetType;

    Target& target;
    Blend_SrcColor (const Pix& /*pix*/, Target& target) : target (target) 
    { }
    
    CS_FORCEINLINE
    void Apply (PixType* /*dest*/, uint8 r, uint8 g, uint8 b, uint8 a)
    {
      target.Set (r, g, b, a);
    }
  };
  
  template<typename Pix, typename Target>
  struct Blend_DstColor
  {
    typedef typename_qualifier Pix::PixType PixType;
    typedef Target TargetType;

    const Pix& pix;
    Target& target;
    Blend_DstColor (const Pix& pix, Target& target) : pix(pix), 
      target (target) { }
    
    CS_FORCEINLINE
    void Apply (PixType* dest, 
      uint8 /*r*/, uint8 /*g*/, uint8 /*b*/, uint8 /*a*/)
    {
      uint8 r, g, b, a;
      pix.GetPix (dest, r, g, b, a);
      target.Set (r, g, b, a);
    }
  };
  
  template<typename Pix, typename Target>
  struct Blend_SrcAlpha
  {
    typedef typename_qualifier Pix::PixType PixType;
    typedef Target TargetType;

    Target& target;
    Blend_SrcAlpha (const Pix& /*pix*/, Target& target) : target (target) 
    { }
    
    CS_FORCEINLINE
    void Apply (PixType* /*dest*/,
      uint8 /*r*/, uint8 /*g*/, uint8 /*b*/, uint8 a)
    {
      target.Set (a);
    }
  };
  
  template<typename Pix, typename Target>
  struct Blend_DstAlpha
  {
    typedef typename_qualifier Pix::PixType PixType;
    typedef Target TargetType;

    const Pix& pix;
    Target& target;
    Blend_DstAlpha (const Pix& pix, Target& target) : pix(pix), 
      target (target) { }
    
    CS_FORCEINLINE
    void Apply (PixType* dest, 
      uint8 /*r*/, uint8 /*g*/, uint8 /*b*/, uint8 /*a*/)
    {
      uint8 r, g, b, a;
      pix.GetPix (dest, r, g, b, a);
      target.Set (a);
    }
  };
  
  template <typename RealTarget>
  struct Target_Inv
  {
    RealTarget& target;
    Target_Inv (RealTarget& target) : target (target) {}

    CS_FORCEINLINE
    void Set (uint8 a) { target.Set (~a); }
    CS_FORCEINLINE
    void Set (uint8 r, uint8 g, uint8 b, uint8 a) 
    { target.Set (~r, ~g, ~b, ~a); }

    operator RealTarget& () { return target; }
  };
  
  struct Target_Src
  {
    uint8 sr, sg, sb, sa;
    uint8 r, g, b, a;
    
    Target_Src (uint8 sr, uint8 sg, uint8 sb, uint8 sa) :
      sr(sr), sg(sg), sb(sb), sa(sa) {}

    CS_FORCEINLINE
    void Set (uint8 A) 
    { 
      const uint v = A+1;
      r = (sr * v) >> 8;
      g = (sg * v) >> 8;
      b = (sb * v) >> 8;
      a = (sa * v) >> 8;
    }
    CS_FORCEINLINE
    void Set (uint8 R, uint8 G, uint8 B, uint8 A) 
    { 
      r = (sr * (R+1)) >> 8; 
      g = (sg * (G+1)) >> 8; 
      b = (sb * (B+1)) >> 8; 
      a = (sa * (A+1)) >> 8; 
    }
  };
  
  template<typename Pix>
  struct Target_Dst
  {
    typedef typename_qualifier Pix::PixType PixType;
    const Target_Src& src;
    Pix& pix;
    PixType* const dest;
    
    Target_Dst (const Target_Src& src, Pix& pix, PixType* const dest) : 
      src(src), pix(pix), dest(dest) {}
    
    CS_FORCEINLINE
    void Set (uint8 A) 
    { 
      pix.MultiplyDstAdd (dest, A, src.r, src.g, src.b, src.a);
    }
    CS_FORCEINLINE
    void Set (uint8 R, uint8 G, uint8 B, uint8 A) 
    {
      pix.MultiplyDstAdd (dest, R, G, B, A, src.r, src.g, src.b, src.a);
    }
  };
  
} // namespace cspluginSoftshader

#endif // __CS_SOFTSHADER_SCAN_BLEND_H__
