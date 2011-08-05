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

#ifndef __CSEDITOR_SELECTOPERATOR_H__
#define __CSEDITOR_SELECTOPERATOR_H__


#include <csutil/scf_implementation.h>
#include <iutil/comp.h>

#include "ieditor/operator.h"
#include "ieditor/action.h"

using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{
class SelectOperator : public scfImplementation1<SelectOperator,iOperator>
{
public:
  SelectOperator (iBase* parent);
  virtual ~SelectOperator ();

  // iOperator
  virtual bool Initialize (iObjectRegistry* obj_reg, const char* identifier, const char* label, const char* desc);
  virtual const char* GetIdentifier ();
  virtual const char* GetLabel ();
  virtual const char* GetDescription ();
  
  virtual bool Poll (iContext*);
  virtual OperatorState Execute (iContext*);
  virtual OperatorState Invoke (iContext*, iEvent*);
  virtual OperatorState Modal (iContext*, iEvent*);

private:
  iObjectRegistry* object_reg;
  csString identifier;
  csString label;
  csString desc;
};

class SelectObjectAction : public scfImplementation1<SelectObjectAction, iAction>
{
public:
  SelectObjectAction (iObjectRegistry* object_reg, iObject*, bool);
  
  virtual bool Do ();
  virtual bool Undo ();
  virtual const char* GetDescription () const { return "Select object"; }

private:
  iObjectRegistry* object_reg;
  csWeakRef<iObject> object;
  bool multiple;
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
