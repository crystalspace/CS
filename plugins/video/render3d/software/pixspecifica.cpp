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

#include "cssysdef.h"

#include "csgeom/math2d.h"
#include "csgeom/tri.h"
#include "csgeom/vector3.h"
#include "cstool/rbuflock.h"

#include "sft3dcom.h"
#include "clipper.h"
#include "clip_znear.h"
#include "clip_iclipper.h"
#include "scan_pix.h"
#include "tridraw.h"
#include "pixspecifica.h"

namespace cspluginSoft3d
{
  template<typename Pix>
  static void SetupPixTypeSpecifica (csSoftwareGraphics3DCommon* g3d)
  {
    TriDrawMatrixFiller<Pix>::Fill (g3d, g3d->triDraw);
    g3d->specifica = new Specifica<Pix> (Pix (
      *g3d->GetDriver2D()->GetPixelFormat()));
  }

  void csSoftwareGraphics3DCommon::SetupSpecifica ()
  {
    if (pfmt.PixelBytes == 4)
    {
      if (((pfmt.BlueMask == 0x0000ff) || (pfmt.RedMask == 0x0000ff))
	&& (pfmt.GreenMask == 0x00ff00)
	&& ((pfmt.RedMask == 0xff0000) || (pfmt.BlueMask == 0xff0000)))
	SetupPixTypeSpecifica <Pix_Fix<uint32, 24, 0xff,
					       16, 0xff,
					       8,  0xff,
					       0,  0xff> > (this);
      else
	SetupPixTypeSpecifica <Pix_Generic<uint32> > (this);
    }
    else
    {
      if (((pfmt.RedMask == 0xf800) || (pfmt.BlueMask == 0xf800))
	&& (pfmt.GreenMask == 0x07e0)
	&& ((pfmt.BlueMask == 0x001f) || (pfmt.RedMask == 0x001f)))
	SetupPixTypeSpecifica <Pix_Fix<uint16, 0,  0,
					       8,  0xf8,
					       3,  0xfc,
					      -3, 0xf8> > (this);
      else if (((pfmt.RedMask == 0x7c00) || (pfmt.BlueMask == 0x7c00))
	&& (pfmt.GreenMask == 0x03e0)
	&& ((pfmt.BlueMask == 0x001f) || (pfmt.RedMask == 0x001f)))
	SetupPixTypeSpecifica <Pix_Fix<uint16, 0,  0,
					       7,  0xf8,
					       2,  0xf8,
					       -3, 0xf8> > (this);
      else
	SetupPixTypeSpecifica <Pix_Generic<uint16> > (this);
    }
  }

} // namespace cspluginSoft3d
