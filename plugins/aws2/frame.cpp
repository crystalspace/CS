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

  void frame::Transform(iPen *pen, float angle, float x, float y, const csRect &box)
  {
    pen->ClearTransform();
    //FIXME pen->SetOrigin(csVector3(box.Width()*-0.5, box.Height()*-0.5, 0));
    //FIXME pen->Rotate(angle);

    // Adjust the translation x and y to get the absolute translation.
    GetScreenPos(x,y);
    pen->Translate(csVector3((box.Width()*0.5)+x, (box.Height()*0.5)+y,0));
  }

  void frame::Draw(iPen *pen)
  {
    float tx=0, ty=0;

    GetScreenPos(tx,ty);

    pen->PushTransform();
    pen->Translate(csVector3(tx, ty, 0));
  }
  
};