/*
    Copyright (C) 2001

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
#include "awstest_config.h"

#ifndef TEST_AWS2

#include "cuscomp.h"
#include "iaws/aws.h"
#include "ivideo/graph2d.h"


SCF_IMPLEMENT_IBASE (CustomComponent)
  SCF_IMPLEMENTS_INTERFACE (iAwsComponent)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_IBASE (CustomComponentFactory)
  SCF_IMPLEMENTS_INTERFACE (iAwsComponentFactory)
SCF_IMPLEMENT_IBASE_END

CustomComponent::CustomComponent ()
{
  SCF_CONSTRUCT_IBASE (0);
}

CustomComponent::~CustomComponent ()
{
  SCF_DESTRUCT_IBASE ();
}

bool CustomComponent::Setup(iAws *manager, iAwsComponentNode *settings)
{
  // first thing is to let the base class setup
  // if it returns false then you should too
  if (!awsEmbeddedComponent::Setup(manager, settings))
    return false;

  // here you would read whatever settings you liked from the settings node
  // and perform any other work required to initialize your component for use
  
  // until I get more inspired to customize this sample I'm not going to do anything
  // besides just set the component to show

  Show ();

  // now we return true to indicate that setup was succesful. If something had failed
  // then we could return false to gracefully abort the construction of this component
  // and its children

  return true;
}
  
const char* CustomComponent::Type ()
{
  // here we return whatever this kind of component should be called
  return "Demo Component";
}

void CustomComponent::OnDraw (csRect /*clip*/)
{
  // lets draw a black box
  // yeah its dull... I know

  iGraphics2D* g2d = WindowManager ()->G2D ();

  int black = WindowManager ()->GetPrefMgr ()->FindColor (0,0,0);

  // now we can use whatever 2d graphics functions we like to draw
  // actually the g3d funcs are available too and a ref to g3d can be retrieved from
  // the window manager. Also you generally won't be drawing outside the bounds of Frame()
  // but the clip rect has been properly set in advance just in case you try.

  g2d->DrawBox (Frame ().xmin, Frame ().ymin, Frame ().Width (), Frame ().Height (), black);

}

/*-------------------------- Custom Component Factory ------------------------------- */

CustomComponentFactory::CustomComponentFactory (iAws* manager) : awsEmbeddedComponentFactory (manager)
{
  SCF_CONSTRUCT_IBASE(0);
  Register ("Demo Component");
}

CustomComponentFactory::~CustomComponentFactory ()
{
  SCF_DESTRUCT_IBASE();
}

iAwsComponent* CustomComponentFactory::Create ()
{
  // create our component
  CustomComponent* my_comp = new CustomComponent ();

  // we create a label component to embed into our own component.
  iAwsComponent* embedded_comp =
    WindowManager ()->CreateEmbeddableComponent (my_comp);

  // Now we must embed the label into our component
  my_comp->Initialize (embedded_comp);

  return my_comp;
}

#endif //end only compile if NOT testing aws2.
