/*
    Copyright (C) 2007 by Seth Yastrov

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


#include "propertiesspace.h"

#include "ieditor/context.h"
#include "ieditor/operator.h"

#include <wx/wx.h>


CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{

BEGIN_EVENT_TABLE(PropertiesSpace::Space, wxPanel)
  EVT_SIZE(PropertiesSpace::Space::OnSize)
END_EVENT_TABLE()

SCF_IMPLEMENT_FACTORY (PropertiesSpace)

PropertiesSpace::PropertiesSpace (iBase* parent)
 : scfImplementationType (this, parent), object_reg(0)
{  
}

bool PropertiesSpace::Initialize (iObjectRegistry* obj_reg, iSpaceFactory* fact, wxWindow* parent)
{
  object_reg = obj_reg;
  factory = fact;

  window = new PropertiesSpace::Space (this, parent, -1, wxPoint(0,0), wxSize(-1,-1));
  //window->SetBackgroundColour(*wxRED);
  
  return true;
}

PropertiesSpace::~PropertiesSpace()
{
  window->Destroy();
}

wxWindow* PropertiesSpace::GetWindow ()
{
  return window;
}

void PropertiesSpace::OnSize (wxSizeEvent& event)
{
  //window->SetSize (event.GetSize());
  window->Layout();
  event.Skip();
}

}
CS_PLUGIN_NAMESPACE_END(CSE)

