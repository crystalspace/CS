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

#ifndef __CS_SOFTSHADER_SCAN_Z_H__
#define __CS_SOFTSHADER_SCAN_Z_H__

namespace cspluginSoftshader
{
  using namespace CrystalSpace::SoftShader;

  struct ZBufMode_ZNone
  {
    ZBufMode_ZNone (const InterpolateScanlinePerspCommon& /*ipol*/, 
      uint32* /*zBuff*/) {}

    bool Test () { return true; }
    void Update () {}
    void Advance () {}
  };

  struct ZBufMode_ZFill
  {
    const uint32 Iz;
    uint32* zBuff;

    ZBufMode_ZFill (const InterpolateScanlinePerspCommon& ipol, 
      uint32* zBuff) : Iz(ipol.Iz.GetFixed()), zBuff(zBuff) {}

    bool Test () { return true; }
    void Update () { *zBuff = Iz; }
    void Advance () { zBuff++; }
  };

  struct ZBufMode_ZTest
  {
    const uint32 Iz;
    uint32* zBuff;

    ZBufMode_ZTest (const InterpolateScanlinePerspCommon& ipol, 
      uint32* zBuff) : Iz(ipol.Iz.GetFixed()), zBuff(zBuff) {}

    bool Test () { return Iz >= *zBuff; }
    void Update () { }
    void Advance () { zBuff++; }
  };

  struct ZBufMode_ZUse
  {
    const uint32 Iz;
    uint32* zBuff;

    ZBufMode_ZUse (const InterpolateScanlinePerspCommon& ipol, 
      uint32* zBuff) : Iz(ipol.Iz.GetFixed()), zBuff(zBuff) {}

    bool Test () { return Iz >= *zBuff; }
    void Update () { *zBuff = Iz; }
    void Advance () { zBuff++; }
  };

  struct ZBufMode_ZEqual
  {
    const uint32 Iz;
    uint32* zBuff;

    ZBufMode_ZEqual (const InterpolateScanlinePerspCommon& ipol, 
      uint32* zBuff) : Iz(ipol.Iz.GetFixed()), zBuff(zBuff) {}

    bool Test () { return Iz == *zBuff; }
    void Update () { }
    void Advance () { zBuff++; }
  };

  struct ZBufMode_ZInvert
  {
    const uint32 Iz;
    uint32* zBuff;

    ZBufMode_ZInvert (const InterpolateScanlinePerspCommon& ipol, 
      uint32* zBuff) : Iz(ipol.Iz.GetFixed()), zBuff(zBuff) {}

    bool Test () { return Iz < *zBuff; }
    void Update () { }
    void Advance () { zBuff++; }
  };
} // namespace cspluginSoftshader

#endif // __CS_SOFTSHADER_SCAN_Z_H__
