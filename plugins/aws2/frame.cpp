/*
    Copyright (C) 2005 by Christopher Nelson

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
#include "frame.h"
#include "manager.h"

namespace aws
{
  frame::frame ()
    : parent (0) 
  {}

  frame::~frame() 
  {}

  void frame::GetScreenPos (float &x, float &y)
  {
    if (parent) parent->GetScreenPos (x,y);

    x += bounds.xmin;
    y += bounds.ymin;
  }

  void frame::Transform (iPen *pen, float angle, float x, float y)
  {
    pen->ClearTransform ();
    pen->SetOrigin (csVector3 (bounds.Width ()*-0.5, bounds.Height ()*-0.5, 0));
    pen->Rotate (angle);

    // Adjust the translation x and y to get the absolute translation.
    GetScreenPos (x,y);
    pen->Translate (csVector3 ((bounds.Width ()*0.5)+x, 
                               (bounds.Height ()*0.5)+y,0));
  }

  void frame::Prepare (iPen *pen)
  {
    float tx=0, ty=0;

    GetScreenPos (tx,ty);

    // Adjust the pen so that children are translated correctly.
    pen->PushTransform ();
    pen->Translate (csVector3 (bounds.xmin, bounds.ymin, 0));

	// For debugging: draw the bounding box.	
//     int color = AwsMgr()->G2D()->FindRGB(255,0,0,255);
//     AwsMgr()->G2D()->DrawLine((int)tx,(int)ty, (int)tx+bounds.Width()+1, (int)ty,color);
//     AwsMgr()->G2D()->DrawLine((int)tx,(int)ty, (int)tx, (int)ty + bounds.Height()+1,color);
//     AwsMgr()->G2D()->DrawLine((int)tx+bounds.Width()+1,(int)ty, (int)tx+bounds.Width()+1, (int)ty + bounds.Height()+1,color);
//     AwsMgr()->G2D()->DrawLine((int)tx,(int)ty + bounds.Height()+1, (int)tx+bounds.Width()+1, (int)ty + bounds.Height()+1,color);
    
    // Clip all the widgets to the bounding box they occupy.
    AwsMgr()->G2D()->SetClipRect((int)tx-1,(int)ty-1, (int)tx+bounds.Width()+1, (int)ty + bounds.Height()+1);
  }
  
  void frame::Finish (iPen *pen)
  {
    pen->PopTransform ();	  
  }
  
}
