/*
    Copyright (C) 1998 by Jorrit Tyberghein

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

#ifndef I_DD8_H
#define I_DD8_H

#include <ddraw.h>

#include "csutil/scf.h"

SCF_VERSION (iGraphics2DDDraw8, 0, 0, 1);

struct iGraphics2DDDraw8 : public iBase
{

  virtual void GetDirectDrawDriver (LPDIRECTDRAW7* lplpDirectDraw) = 0;

  virtual void GetDirectDrawPrimary (LPDIRECTDRAWSURFACE7* lplpDirectDrawPrimary) = 0;

  virtual void GetDirectDrawBackBuffer (LPDIRECTDRAWSURFACE7* lplpDirectDrawBackBuffer) = 0;

  virtual void GetDirectDetection (IDirectDetectionInternal** lplpDDetection) = 0;

  virtual void SetColorPalette (void) = 0;

  virtual void SetFor3D(bool For3D) = 0;

  virtual void SetModeSwitchCallback (void (*Callback) (void *), void *Data) = 0;

};

#endif // End of I_DD8_H