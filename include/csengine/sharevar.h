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

#include "csutil/nobjvec.h"
#include "csutil/csvector.h"
#include "csutil/csstring.h"
#include "iengine/sharevar.h"
#include "csutil/csobject.h"



SCF_VERSION (csSharedVariable, 0, 0, 2);

/**
 * A SharedVariable is a refcounted floating point value.
 */
class csSharedVariable : public csObject
{
private:
  float value;

public:

  SCF_DECLARE_IBASE_EXT (csObject);

  /**
   * Construct a SharedVariable. This SharedVariable will be initialized to zero and unnamed.
   */
  csSharedVariable () : csObject()
  { 
    SCF_CONSTRUCT_EMBEDDED_IBASE (scfiSharedVariable);
    value = 0;
  }

  void Set (float val)
  {    value = val; }
  float Get () const
  {    return value; }


  //------------------------- iSharedVariable interface -------------------------------
  struct eiSharedVariable : public iSharedVariable
  {
    SCF_DECLARE_EMBEDDED_IBASE (csSharedVariable);

    void Set(float val)
    {	scfParent->Set(val); }
    float Get() const
    {   return scfParent->Get(); }
    void SetName (const char *iName)
    {   scfParent->SetName(iName); }
    const char *GetName () const
    {   return scfParent->GetName(); }
  } scfiSharedVariable;
  friend struct eiSharedVariable;
};

CS_DECLARE_OBJECT_VECTOR (csSharedVariableListHelper, iSharedVariable);

/// List of 3D engine SharedVariables.
class csSharedVariableList : public csSharedVariableListHelper
{
public:
  SCF_DECLARE_IBASE;

  /// constructor
  csSharedVariableList ();
  /// destructor
  ~csSharedVariableList ();

  /// override FreeItem
  virtual bool FreeItem (csSome Item);

  /// iSharedVariable Factory method.  This does not add the new var to the list.
  iSharedVariable *New() const;

  class SharedVariableList : public iSharedVariableList
  {
  public:
    SCF_DECLARE_EMBEDDED_IBASE (csSharedVariableList);

    virtual int GetCount () const;
    virtual iSharedVariable *Get (int n) const;
    virtual int Add (iSharedVariable *obj);
    virtual bool Remove (iSharedVariable *obj);
    virtual bool Remove (int n);
    virtual void RemoveAll ();
    virtual int Find (iSharedVariable *obj) const;
    virtual iSharedVariable *FindByName (const char *Name) const;
    virtual iSharedVariable *New() const
    { return scfParent->New(); }
  } scfiSharedVariableList;
};

#endif // __CS_SHAREVAR_H__
