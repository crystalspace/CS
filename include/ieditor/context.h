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

#ifndef __IEDITOR_CONTEXT_H__
#define __IEDITOR_CONTEXT_H__

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>
#include <csutil/weakrefarr.h>

struct iCamera;
struct iObject;

namespace CS {
namespace EditorApp {

struct iContext;
  
struct iContextListener : public virtual iBase
{
  SCF_INTERFACE (iContextListener, 0,0,1);
  virtual void OnChanged (iContext*) = 0;
};

/**
 * 
 */
struct iContext : public virtual iBase
{
  SCF_INTERFACE (iContext, 0, 0, 1);
  
  virtual iObject* GetActiveObject () = 0;
  
  virtual const csWeakRefArray<iObject>& GetSelectedObjects () = 0;
  virtual void AddSelectedObject (iObject*) = 0;
  virtual void RemoveSelectedObject (iObject*) = 0;
  virtual void ClearSelectedObjects () = 0;
  
  virtual iCamera* GetCamera () = 0;
  virtual void SetCamera (iCamera*) = 0;
  
  virtual void AddListener(iContextListener* listener) = 0;
  
  virtual void RemoveListener(iContextListener* listener) = 0;
};


} // namespace EditorApp
} // namespace CS

#endif
