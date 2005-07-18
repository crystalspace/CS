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
#include "widget.h"

namespace aws
{

  void widget::SetupAutomation(const csString &object_name)
  {
    // Setup automation for our property bag. Automation objects will be able to get to us through this name.
    // Therefore, if the window's template name is 'toolbar' and the name is '1' then an automation script would
    // simply call :Set@win.toolbar.1.prop(name="Title", value="Toolbar") to set that property.
    prop_bag.SetupAutomation(object_name);
  }

}
