/*
    Copyright (C) 1998-2002 by Jorrit Tyberghein and Keith Fulton

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
#include "iengine/sharevar.h"


SCF_VERSION (csSharedVariable, 0, 0, 2);

/**
 * A SharedVariable is a refcounted floating point value.
 */
class csSharedVariable : public scfImplementationExt1<csSharedVariable, 
                                                      csObject,
                                                      iSharedVariable>
{
private:
  int   type;
  float value;
  csColor color;
  csVector3 vec;
  csRefArray<iSharedVariableListener> listeners;

  void FireListeners ();

public:
  CS_LEAKGUARD_DECLARE (csSharedVariable);


  /**
   * Construct a SharedVariable. This SharedVariable will be initialized to
   * zero and unnamed.
   */
  csSharedVariable () : scfImplementationType (this)
  { 
    value = 0;
    type = iSharedVariable::SV_UNKNOWN;
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
    return (type == iSharedVariable::SV_COLOR)
    	? color
	: csColor (0,0,0),color;
  }

  void SetVector (const csVector3& v)
  {
    vec = v;
    type = iSharedVariable::SV_VECTOR;
    FireListeners ();
  }

  const csVector3& GetVector() const
  {
    return (type == iSharedVariable::SV_VECTOR) ? vec : csVector3 (0,0,0),vec;
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
};

/// List of 3D engine SharedVariables.
class csSharedVariableList : public scfImplementation1<csSharedVariableList,
                                                       iSharedVariableList>
{
public:
  
  /// constructor
  csSharedVariableList ();
  /// destructor
  virtual ~csSharedVariableList ();

  /// iSharedVariable Factory method. This does not add the new var to the list.
  csPtr<iSharedVariable> New() const;

  virtual int GetCount () const;
  virtual iSharedVariable *Get (int n) const;
  virtual int Add (iSharedVariable *obj);
  virtual bool Remove (iSharedVariable *obj);
  virtual bool Remove (int n);
  virtual void RemoveAll ();
  virtual int Find (iSharedVariable *obj) const;
  virtual iSharedVariable *FindByName (const char *Name) const;
private:
  csRefArrayObject<iSharedVariable> list;
   
};

#endif // __CS_SHAREVAR_H__

