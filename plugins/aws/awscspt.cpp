/**************************************************************************
    Copyright (C) 2000-2001 by Christopher Nelson
    	      (c) 2001 F.Richter	
    
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

#include "cssysdef.h"
#include "iutil/comp.h"
#include "iutil/objreg.h"
#include "iutil/plugin.h"
#include "aws.h"
#include "iaws/awscnvs.h"
#include "ivideo/txtmgr.h"
#include "cstool/proctex.h"
#include "iengine/engine.h"
#include "iengine/material.h"
#include "ivideo/material.h"
#include "ivideo/fontserv.h"
#include "ivaria/reporter.h"
#include "qint.h"
#include "awscmpt.h"

//// Canvas stuff  //////////////////////////////////////////////////////////////////////////////////

awsSimpleCanvas::awsSimpleCanvas ()
{
  mat_w=256;
  mat_h=256;
  
  // this doesn't work:
  //texFlags = CS_TEXTURE_2D | CS_TEXTURE_PROC;
  // but this is ok either:
  texFlags = CS_TEXTURE_3D | CS_TEXTURE_PROC | CS_TEXTURE_NOMIPMAPS;
}

void 
awsSimpleCanvas::Animate (csTicks current_time)
{
  (void)current_time;
}

void 
awsSimpleCanvas::SetSize(int w, int h)
{  
  mat_w=w; 
  mat_h=h; 
}

/////////

SCF_IMPLEMENT_IBASE (awsSingleProctexCanvas)
  SCF_IMPLEMENTS_INTERFACE (iAwsCanvas)
  static scfInterfaceID scfID_iTextureWrapper = (scfInterfaceID)-1;		
  if (scfID_iTextureWrapper == (scfInterfaceID)-1)				
    scfID_iTextureWrapper = iSCF::SCF->GetInterfaceID ("iTextureWrapper");		
  if (iInterfaceID == scfID_iTextureWrapper)		
  {									
    iTextureWrapper *tex = canvas->GetTextureWrapper();
    (tex)->IncRef ();						
    return tex;				
  }
SCF_IMPLEMENT_IBASE_END

awsSingleProctexCanvas::awsSingleProctexCanvas(int w, int h, iObjectRegistry* object_reg, iEngine* engine,
      	iTextureManager* txtmgr, const char *name)
{
  SCF_CONSTRUCT_IBASE (NULL);

  canvas = new awsSimpleCanvas ();
  canvas->DisableAutoUpdate();
  canvas->SetSize(w, h);
  canvas->SetKeyColor(255,0,255);
  canvas->Initialize(object_reg, engine, txtmgr, name);
  canvas->PrepareAnim();
}

awsSingleProctexCanvas::~awsSingleProctexCanvas ()
{
  delete canvas;
}

void 
awsSingleProctexCanvas::Animate (csTicks current_time)
{
  (void)current_time;
}

void awsSingleProctexCanvas::Show (csRect *area, iGraphics3D *g3d, uint8 Alpha)
{
  int w = ((!g3d) || (G2D()->GetWidth()  < g3d->GetWidth())  ? G2D()->GetWidth() : g3d->GetWidth());
  int h = ((!g3d) || (G2D()->GetHeight() < g3d->GetHeight()) ? G2D()->GetHeight() : g3d->GetHeight());
  
  area->xmin = (area->xmin < 0 ? 0 : area->xmin);
  area->ymin = (area->ymin < 0 ? 0 : area->ymin);

  area->xmax = (area->xmax > w ? w : area->xmax);
  area->ymax = (area->ymax > h ? h : area->ymax);

  canvas->G3D()->Print(area);

  if (g3d)
  {
     g3d->DrawPixmap(canvas->GetTextureWrapper()->GetTextureHandle(),
                     area->xmin,area->ymin,area->Width(),area->Height(),
                     area->xmin,area->ymin,area->Width(),area->Height(),
                     Alpha);
  }
}

