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

#ifndef __IEDITOR_LAYOUT_H__
#define __IEDITOR_LAYOUT_H__

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>


namespace CS {
namespace EditorApp {
  
struct iOperator;
struct iMenu;
struct iProperty;
struct iResource;
  
/**
 * 
 */
struct iLayout : public virtual iBase
{
  SCF_INTERFACE (iLayout, 0, 0, 1);

  virtual iOperator* AppendOperator(const char* id, const char* label, const char* icon);
  virtual iMenu* AppendMenu(const char* id, const char* label);
  virtual iProperty* AppendProperty(iResource*, const char* id);
  virtual void AppendLabel(const char* label);
  virtual void AppendSeperator();

  virtual iLayout* Row();
  virtual iLayout* Column();
};



} // namespace EditorApp
} // namespace CS
#endif
