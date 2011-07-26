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

  /**
   * Check whether this operator can run.
   */
  virtual bool Poll (iContext*) = 0;

  /**
   * Execute the operator with pre-defined settings.
   */
  virtual OperatorState Execute (iContext*) = 0;
  
  /**
   * Initialize the operator from the context and/or event.
   */
  virtual OperatorState Invoke (iContext*, iEvent*) = 0;
  
  /**
   * if Invoke returns 'OperatorRunningModal' this function will become
   * part of the event loop until it returns OperatorFinished/OperatorCancel
   */
  virtual OperatorState Modal (iContext*, iEvent*) = 0;
  
  //virtual void Draw (iContext*) = 0; can be handled in Modal, no?
};


struct iOperatorFactory : public virtual iBase
{
  SCF_INTERFACE (iOperatorFactory, 0, 0, 1);
  
  virtual csPtr<iOperator> Create() = 0;
  
  /**
   * Get the object that uniquely idenifies this operator type.
   */
  //virtual const char* GetIdentifier () = 0;
  
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
  
  virtual iOperator* Execute(const char*) = 0;
  
  virtual iOperator* Invoke(const char*, iEvent*) = 0;
  
  virtual void Register(const char*, iOperatorFactory*) = 0;
  
  virtual const char* GetLabel (const char*) = 0;
  virtual const char* GetDescription (const char*) = 0;
};  

#define CS_EDITOR_IMPLEMENT_OPERATOR(klass, pluginName, label, description)                   \
class klass##Factory : public scfImplementation2<klass##Factory,iOperatorFactory,iComponent>  \
{ \
public: \
  klass##Factory (iBase* parent) : scfImplementationType (this, parent) {} \
  virtual ~klass##Factory () {} \
  virtual bool Initialize (iObjectRegistry* obj_reg) \
  {  \
    object_reg = obj_reg; \
    csRef<iOperatorManager> operatorManager = csQueryRegistry<iOperatorManager> (object_reg); \
    operatorManager->Register(pluginName, this); \
    return true; \
  } \
  virtual csPtr<iOperator>  Create()  \
  { \
    csRef<iOperator> ref; ref.AttachNew (new klass(object_reg)); \
    return csPtr<iOperator> (ref); \
  } \
  virtual const char* GetLabel () { return label; } \
  virtual const char* GetDescription () { return description; } \
private: \
  iObjectRegistry* object_reg; \
}; \
SCF_IMPLEMENT_FACTORY (klass##Factory) \


} // namespace EditorApp
} // namespace CS

#endif
