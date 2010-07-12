/*
    Copyright (C) 2010 Jelle Hellemans

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

#ifndef _CS_CEGUI_WINDOWFACTORY_H
#define _CS_CEGUI_WINDOWFACTORY_H

#include "cssysdef.h"

/**\file 
*/
/**
* \addtogroup CEGUI
* @{ */

CS_PLUGIN_NAMESPACE_BEGIN(cegui)
{
  template <typename T>
  class csWindowFactory : public CEGUI::WindowFactory
  {
  public:
    iObjectRegistry* obj_reg;

  public:
    //! Default constructor.
    csWindowFactory();

    // Implement WindowFactory interface
    CEGUI::Window* createWindow(const CEGUI::String& name);
    void destroyWindow(CEGUI::Window* window);
  };

  //----------------------------------------------------------------------------//
  template <typename T>
  csWindowFactory<T>::csWindowFactory() :
  CEGUI::WindowFactory(T::WidgetTypeName)
  {
  }

  //----------------------------------------------------------------------------//
  template <typename T>
  CEGUI::Window* csWindowFactory<T>::createWindow(const CEGUI::String& name)
  {
    return new T(d_type, name, obj_reg);
  }

  //----------------------------------------------------------------------------//
  template <typename T>
  void csWindowFactory<T>::destroyWindow(CEGUI::Window* window)
  {
    delete window;
  }

  //----------------------------------------------------------------------------//

} CS_PLUGIN_NAMESPACE_END(cegui)

#endif  // end of guard _CS_CEGUI_WINDOWFACTORY_H
