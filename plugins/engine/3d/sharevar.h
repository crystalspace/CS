/*
    Copyright (C) 1998-2006 by Jorrit Tyberghein and Keith Fulton

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

#ifndef __CS_SHAREVAR_H__
#define __CS_SHAREVAR_H__

#include "csgeom/vector3.h"
#include "csutil/cscolor.h"
#include "csutil/csobject.h"
#include "csutil/csstring.h"
#include "csutil/leakguard.h"
#include "csutil/nobjvec.h"
#include "csutil/refarr.h"
#include "csutil/scf_implementation.h"
#include "csutil/threading/rwmutex.h"
#include "iutil/selfdestruct.h"
#include "iengine/sharevar.h"

class csSharedVariableList;
struct iSharedVarLoaderIterator;

/**
 * A SharedVariable is a refcounted floating point value.
 */
class csSharedVariable : public scfImplementationExt2<csSharedVariable, 
                                                      csObject,
                                                      iSharedVariable,
						      iSelfDestruct>
{
private:
  int   type;
  float value;
  csColor color;
  csVector3 vec;
  csString str;
  csRefArray<iSharedVariableListener> listeners;
  csSharedVariableList* variables;

  void FireListeners ();

protected:
  virtual void InternalRemove() { SelfDestruct(); }

public:
  CS_LEAKGUARD_DECLARE (csSharedVariable);


  /**
   * Construct a SharedVariable. This SharedVariable will be initialized to
   * zero and unnamed.
   */
  csSharedVariable (csSharedVariableList* variables) : 
    scfImplementationType (this), type (iSharedVariable::SV_UNKNOWN), 
      value (0), variables (variables)
  {
    vec.Set (0, 0, 0);
    color.Set (0, 0, 0);
  }

  virtual ~csSharedVariable()
  {
  }

  void Set (float val)
  {
    value = val;
    type = iSharedVariable::SV_FLOAT;
    FireListeners ();
  }
  
  float Get () const
  {
    return (type == iSharedVariable::SV_FLOAT) ? value : 0;
  }
  
  void SetColor (const csColor& col)
  {
    color.Set(col.red,col.green,col.blue);
    type = iSharedVariable::SV_COLOR;
    FireListeners ();
  }

  const csColor& GetColor() const
  {
    return color;
  }

  void SetVector (const csVector3& v)
  {
    vec = v;
    type = iSharedVariable::SV_VECTOR;
    FireListeners ();
  }

  const csVector3& GetVector() const
  {
    return vec;
  }

  void SetString (const char* str)
  {
    csSharedVariable::str = str;
    type = iSharedVariable::SV_STRING;
  }

  const char* GetString () const
  {
    return (type == iSharedVariable::SV_STRING) ? str.GetData () : 0;
  }

  int GetType () const
  {
    return type;
  }

  void AddListener (iSharedVariableListener* listener)
  {
    listeners.Push (listener);
  }

  void RemoveListener (iSharedVariableListener* listener)
  {
    listeners.Delete (listener);
  }

  //---------------------- iSharedVariable interface --------------------------
  virtual iObject* QueryObject () { return this; }
 
  virtual void SetName (const char *iName)
  { csObject::SetName (iName); }
  virtual const char *GetName () const
  { return csObject::GetName (); }

  //--------------------- iSelfDestruct implementation -------------------//

  virtual void SelfDestruct ();
};

/// List of 3D engine SharedVariables.
class csSharedVariableList : public scfImplementation1<csSharedVariableList,
                                                       iSharedVariableList>
{
  mutable CS::Threading::ReadWriteMutex shvarLock;
public:
  
  /// constructor
  csSharedVariableList ();
  /// destructor
  virtual ~csSharedVariableList ();

  /// iSharedVariable Factory method. This does not add the new var to the list.
  csPtr<iSharedVariable> New();

  virtual int GetCount () const;
  virtual iSharedVariable *Get (int n) const;
  virtual int Add (iSharedVariable *obj);
  void AddBatch (csRef<iSharedVarLoaderIterator> itr);
  virtual bool Remove (iSharedVariable *obj);
  virtual bool Remove (int n);
  virtual void RemoveAll ();
  virtual int Find (iSharedVariable *obj) const;
  virtual iSharedVariable *FindByName (const char *Name) const;
private:
  csRefArrayObject<iSharedVariable> list;
   
};

#endif // __CS_SHAREVAR_H__

