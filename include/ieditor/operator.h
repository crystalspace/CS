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

#ifndef __IEDITOR_OPERATOR_H__
#define __IEDITOR_OPERATOR_H__

#include <csutil/scf.h>
#include <csutil/scf_implementation.h>

struct iEvent;

namespace CS {
namespace EditorApp {
  
struct iContext;


enum OperatorState
{
  OperatorFinished = 0,
  OperatorRunningModal = 1,
  OperatorCanceled = 2
};

/**
 * 
 */
struct iOperator : public virtual iBase
{
  SCF_INTERFACE (iOperator, 0, 0, 1);
  
  virtual bool Initialize (iObjectRegistry* obj_reg, const char* identifier, const char* label, const char* desc) = 0;

  /**
   * Check whether this operator can run.
   */
  virtual bool Poll (iContext*) = 0;

  /**
   * Execute the operator with pre-defined settings.
   * Don't call this directly, use iOperatorManager instead!
   */
  virtual OperatorState Execute (iContext*) = 0;
  
  /**
   * Initialize the operator from the context and/or event.
   * Don't call this directly, use iOperatorManager instead!
   */
  virtual OperatorState Invoke (iContext*, iEvent*) = 0;
  
  /**
   * if Invoke returns 'OperatorRunningModal' this function will become
   * part of the event loop until it returns OperatorFinished/OperatorCancel
   * Don't call this directly!
   */
  virtual OperatorState Modal (iContext*, iEvent*) = 0;
  
  /**
   * Get the identifier that uniquely idenifies this operator type.
   */
  virtual const char* GetIdentifier () = 0;
  
  /**
   * Get the human readable name of this operator type.
   */
  virtual const char* GetLabel () = 0;
  
  /**
   * Get the human readable description of this operator type.
   */
  virtual const char* GetDescription () = 0;
};


struct iOperatorManager : public virtual iBase
{
  SCF_INTERFACE (iOperatorManager, 0, 0, 1);
  
  virtual csPtr<iOperator> Create(const char*) = 0;
  
  virtual iOperator* Execute(iOperator*) = 0;
  
  virtual iOperator* Invoke(iOperator*, iEvent*) = 0; 
};  

} // namespace EditorApp
} // namespace CS

#endif
