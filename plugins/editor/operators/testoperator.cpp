/*
    Copyright (C) 2011 by Jelle Hellemans

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
#include "csutil/scf.h"
#include "csutil/sysfunc.h"

#include "iutil/objreg.h"
#include "iutil/plugin.h"

#include "csutil/event.h"
#include "csutil/eventnames.h"


#include "testoperator.h"



CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

TestOperator::TestOperator (iObjectRegistry* obj_reg)
 : scfImplementationType (this), object_reg(obj_reg)
{
}

TestOperator::~TestOperator ()
{
}

bool TestOperator::Poll (iContext*)
{
  return true;
}

OperatorState TestOperator::Execute (iContext*)
{
  return OperatorRunningModal;
}

OperatorState TestOperator::Invoke (iContext*, iEvent*)
{
  return OperatorFinished;
}

OperatorState TestOperator::Modal (iContext*, iEvent* ev)
{
  if (CS_IS_KEYBOARD_EVENT(object_reg, *ev)) 
  {
    csKeyEventType eventtype = csKeyEventHelper::GetEventType(ev);
    if (eventtype == csKeyEventTypeDown)
    {
      utf32_char code = csKeyEventHelper::GetCookedCode(ev);
      if (code == CSKEY_ESC)
      {
        return OperatorFinished;
      }
    }
  }
  else if (CS_IS_MOUSE_EVENT(object_reg, *ev))
  {
    int mouse_x = csMouseEventHelper::GetX(ev);
    int mouse_y = csMouseEventHelper::GetY(ev);
    printf("Modal %d - %d\n", mouse_x, mouse_y);
  }
  return OperatorRunningModal;
}

}
CS_PLUGIN_NAMESPACE_END(CSE)
