 #ifndef __AWSCNVS_H__
 #define __AWSCNVS_H__
/**************************************************************************
    Copyright (C) 2000-2001 by Christopher Nelson 
    
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
*****************************************************************************/

#include "cstool/proctex.h"
#include "ivideo/graph2d.h"
#include "csutil/typedvec.h"
#include "ivideo/graph3d.h"

SCF_VERSION (iAwsCanvas, 0, 0, 1);

/// Provider for AWS 2D / 3D Driver
struct iAwsCanvas : public iBase
{
public:
  /// This is actually not used ever.  The window manager doesn't "animate", and only refreshes the canvas when needed.
  virtual void Animate (csTicks current_time)=0;

  /// Get the iGraphics2D interface so that components can use it.
  virtual iGraphics2D *G2D()=0;

  /// Get the iGraphics3D interface so that components can use it.
  virtual iGraphics3D *G3D()=0;

  virtual void Show (csRect *area = NULL)=0;
};

#endif // __AWSCNVS_H__