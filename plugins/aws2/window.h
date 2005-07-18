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

#ifndef __AWS_WINDOW_H__
#define __AWS_WINDOW_H__

#include "widget.h"

namespace aws
{
  class window : public widget
  {
    /// The name of the window.
    csString name;

    /// The template name of the window (the definition template that it was taken from.)
    csString template_name;
    
  public:
    window();
    virtual ~window() {}
 
  protected:
    /// Sets up whatever automation is necessary.
    virtual void SetupAutomation(const csString &object_name);
  };
}

#endif
