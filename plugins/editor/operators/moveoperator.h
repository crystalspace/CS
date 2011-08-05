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

#ifndef __CSEDITOR_MOVEOPERATOR_H__
#define __CSEDITOR_MOVEOPERATOR_H__


#include <csutil/scf_implementation.h>
#include <iutil/comp.h>
#include <csgeom/transfrm.h>

#include "ieditor/operator.h"
#include "ieditor/action.h"

using namespace CS::EditorApp;

CS_PLUGIN_NAMESPACE_BEGIN(CSE)
{
class MoveOperator : public scfImplementation1<MoveOperator,iOperator>
{
public:
  MoveOperator (iBase* parent);
  virtual ~MoveOperator ();

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
  csVector3 worldStart;
  
  OperatorState Commit (iContext* ctx, iMeshWrapper* mesh);
  csReversibleTransform GetOperationTransform(csVector3 worldDiff);
};

class MoveObjectAction : public scfImplementation1<MoveObjectAction, iAction>
{
public:
  MoveObjectAction (iObjectRegistry* object_reg, iObject*, const csReversibleTransform&);
  
  virtual bool Do ();
  virtual bool Undo ();
  virtual const char* GetDescription () const { return "Move an object"; }

private:
  iObjectRegistry* object_reg;
  csWeakRef<iObject> object;
  csReversibleTransform transform;
};

}
CS_PLUGIN_NAMESPACE_END(CSE)

#endif
