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

namespace cspluginSoft3d
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
  
  struct BlendBase
  {
    typedef void (*BlendProc) (BlendBase* _This, uint32* src,
      void* dest, uint len);

    virtual ~BlendBase() {}
    virtual BlendProc GetBlendProc (uint mixmode) = 0;
  };

  template<typename Pix>
  struct BlendImpl : public BlendBase
  {
  public:
    Pix pix;
  private:
    template <typename SrcBlend, typename DstBlend>
    struct BlendImpl2
    {
      static void Blend (BlendBase* _This, uint32* src,
	void* dest, uint len)
      {
	BlendImpl<Pix>* This = (BlendImpl<Pix>*)_This;

	Pix& pix = This->pix;
	typename_qualifier Pix::PixType* _dest = 
	  (typename_qualifier Pix::PixType*)dest;
	typename_qualifier Pix::PixType* _destend = _dest + len;

	while (_dest < _destend)
	{
	  uint8 r, g, b, a;
	  const uint32 sp = *src++;
	  a = sp >> 24;

	  if (a & 0x80)
	  {
	    r = (sp >> 16) & 0xff;
	    g = (sp >> 8) & 0xff;
	    b = sp & 0xff;
	    a <<= 1;

	    Target_Src srcTarget (r, g, b, a);
	    typename_qualifier SrcBlend::TargetType src (srcTarget);
	    Target_Dst<Pix> dstTarget (src, pix, _dest);
	    typename_qualifier DstBlend::TargetType dst (dstTarget);

	    SrcBlend blendSrc (pix, src);
	    DstBlend blendDst (pix, dst);
	    blendSrc.Apply (_dest, r, g, b, a);
	    blendDst.Apply (_dest, r, g, b, a);
	    
  	    _dest++;
	  }
	} /* endwhile */
      }
    };
    template<typename SrcBlend>
    static BlendBase::BlendProc GetScanlineProcM (uint mixmode)
    {
      switch (CS_MIXMODE_BLENDOP_DST (mixmode))
      {
	default:
	case CS_MIXMODE_FACT_ZERO:
	  return BlendImpl2<SrcBlend, Blend_Zero<Pix, Target_Dst<Pix> > >::Blend;
	case CS_MIXMODE_FACT_ONE:
	  return BlendImpl2<SrcBlend, Blend_Zero<Pix, Target_Inv<Target_Dst<Pix> > > >::Blend;

	case CS_MIXMODE_FACT_SRCCOLOR:
	  return BlendImpl2<SrcBlend, Blend_SrcColor<Pix, Target_Dst<Pix> > >::Blend;
	case CS_MIXMODE_FACT_SRCCOLOR_INV:
	  return BlendImpl2<SrcBlend, Blend_SrcColor<Pix, Target_Inv<Target_Dst<Pix> > > >::Blend;

	case CS_MIXMODE_FACT_SRCALPHA:
	  return BlendImpl2<SrcBlend, Blend_SrcAlpha<Pix, Target_Dst<Pix> > >::Blend;
	case CS_MIXMODE_FACT_SRCALPHA_INV:
	  return BlendImpl2<SrcBlend, Blend_SrcAlpha<Pix, Target_Inv<Target_Dst<Pix> > > >::Blend;

	case CS_MIXMODE_FACT_DSTCOLOR:
	  return BlendImpl2<SrcBlend, Blend_DstColor<Pix, Target_Dst<Pix> > >::Blend;
	case CS_MIXMODE_FACT_DSTCOLOR_INV:
	  return BlendImpl2<SrcBlend, Blend_DstColor<Pix, Target_Inv<Target_Dst<Pix> > > >::Blend;

	case CS_MIXMODE_FACT_DSTALPHA:
	  return BlendImpl2<SrcBlend, Blend_DstAlpha<Pix, Target_Dst<Pix> > >::Blend;
	case CS_MIXMODE_FACT_DSTALPHA_INV:
	  return BlendImpl2<SrcBlend, Blend_DstAlpha<Pix, Target_Inv<Target_Dst<Pix> > > >::Blend;
      }
    }
    static BlendBase::BlendProc GetScanlineProc (uint mixmode)
    {
      switch (CS_MIXMODE_BLENDOP_SRC (mixmode))
      {
	default:
	case CS_MIXMODE_FACT_ZERO:
	  return GetScanlineProcM<Blend_Zero<Pix, Target_Src> > (mixmode);
	case CS_MIXMODE_FACT_ONE:
	  return GetScanlineProcM<Blend_Zero<Pix, Target_Inv<Target_Src> > > (mixmode);

	case CS_MIXMODE_FACT_SRCCOLOR:
	  return GetScanlineProcM<Blend_SrcColor<Pix, Target_Src> > (mixmode);
	case CS_MIXMODE_FACT_SRCCOLOR_INV:
	  return GetScanlineProcM<Blend_SrcColor<Pix, Target_Inv<Target_Src> > > (mixmode);

	case CS_MIXMODE_FACT_SRCALPHA:
	  return GetScanlineProcM<Blend_SrcAlpha<Pix, Target_Src> > (mixmode);
	case CS_MIXMODE_FACT_SRCALPHA_INV:
	  return GetScanlineProcM<Blend_SrcAlpha<Pix, Target_Inv<Target_Src> > > (mixmode);

	case CS_MIXMODE_FACT_DSTCOLOR:
	  return GetScanlineProcM<Blend_DstColor<Pix, Target_Src> > (mixmode);
	case CS_MIXMODE_FACT_DSTCOLOR_INV:
	  return GetScanlineProcM<Blend_DstColor<Pix, Target_Inv<Target_Src> > > (mixmode);

	case CS_MIXMODE_FACT_DSTALPHA:
	  return GetScanlineProcM<Blend_DstAlpha<Pix, Target_Src> > (mixmode);
	case CS_MIXMODE_FACT_DSTALPHA_INV:
	  return GetScanlineProcM<Blend_DstAlpha<Pix, Target_Inv<Target_Src> > > (mixmode);
      }
    }
  public:
    BlendImpl (const csPixelFormat& pfmt) : pix (pfmt) 
    {}

    BlendBase::BlendProc GetBlendProc (uint mixmode)
    {
      return GetScanlineProc (mixmode);
    }
  };

} // namespace cspluginSoft3d

#endif // __CS_SOFT3D_SCAN_BLEND_H__
