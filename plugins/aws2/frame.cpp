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

namespace aws
{
  frame::frame():parent(0) {}
  frame::~frame() {}

  void frame::GetScreenPos(float &x, float &y)
  {
    if (parent) parent->GetScreenPos(x,y);

    x+=bounds.xmin;
    y+=bounds.ymin;
  }

  void frame::Transform(iPen *pen, float angle, float x, float y)
  {
    pen->ClearTransform();
    pen->SetOrigin(csVector3(bounds.Width()*-0.5, bounds.Height()*-0.5, 0));
    pen->Rotate(angle);

    // Adjust the translation x and y to get the absolute translation.
    GetScreenPos(x,y);
    pen->Translate(csVector3((bounds.Width()*0.5)+x, (bounds.Height()*0.5)+y,0));
  }

  void frame::Draw(iPen *pen)
  {
    float tx=0, ty=0;

    GetScreenPos(tx,ty);

    pen->PushTransform();
    pen->Translate(csVector3(tx, ty, 0));

    OnDraw(pen);
  }
  
}
