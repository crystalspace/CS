/*
    Copyright (C) 2000 by Thomas Hieber
    
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

#ifndef __KEYVAL_H__
#define __KEYVAL_H__

#include "csgeom/vector3.h"
#include "csutil/csobject.h"
#include "iengine/keyval.h"

/**
 * A Key Value pair. This object contains a 'key' string and a 'value' string.
 * The 'key' string is the same as the name of the object as returned from
 * the iObject.
 */
class csKeyValuePair : public csObject
{
public:
  /// The constructor. Requires both key and value. Data is being copied!
  csKeyValuePair (const char* Key, const char* Value);
  /// The destructor as usual
  virtual ~csKeyValuePair ();

  /// Get the key string of the pair.
  const char *GetKey () const;

  /// Set the key string of the pair.
  void SetKey (const char *s);

  /// Get the value string of the pair
  const char *GetValue () const;

  /// Set the value of a key in an object.
  void SetValue (const char* value);

  DECLARE_IBASE_EXT (csObject);
  //----------------------- iKeyValuePair --------------------------
  struct KeyValuePair : public iKeyValuePair
  {
    DECLARE_EMBEDDED_IBASE (csKeyValuePair);
    virtual iObject *QueryObject() { return scfParent; }
    virtual const char *GetKey () const { return scfParent->GetKey (); }
    virtual void SetKey (const char* s) { scfParent->SetKey (s); }
    virtual const char *GetValue () const { return scfParent->GetValue (); }
    virtual void SetValue (const char* value) { scfParent->SetValue (value); }
  } scfiKeyValuePair;

private:
  char *m_Value;
};

/**
 * A node. This is an iObject that is bound to a position and a sector in
 * the world.
 */
class csMapNode : public csObject
{
public:
  /// The constructor. Requires the Nodes name!
  csMapNode (const char *Name);
  /// The destructor as usual
  virtual ~csMapNode ();

  /// Set the position of the node
  void SetPosition (const csVector3& pos);
  /// Get the position of the node
  const csVector3& GetPosition () const;

  /// Set the sector of the node
  void SetSector (iSector *pSector);
  /// Get the sector of the node
  iSector *GetSector () const;

  /// Get a node with the given name and a given classname. (shortcut)
  static iMapNode *GetNode (iSector *pSector, const char *name,
    const char *classname = NULL);

  DECLARE_IBASE_EXT (csObject);
  //----------------------- iMapNode --------------------------
  struct MapNode : public iMapNode
  {
    DECLARE_EMBEDDED_IBASE (csMapNode);
    virtual iObject *QueryObject()
    { return scfParent; }
    virtual void SetPosition (const csVector3& pos)
    { scfParent->SetPosition (pos); }
    virtual const csVector3& GetPosition () const
    { return scfParent->GetPosition (); }
    virtual void SetSector (iSector *sec)
    { scfParent->SetSector (sec); }
    virtual iSector *GetSector () const
    { return scfParent->GetSector (); }
  } scfiMapNode;

private:
  iSector *m_pSector;
  csVector3 m_Position;
};

/**
 * A node iterator.
 */
class csNodeIterator
{
public:
  /**
   * The constructor. Theorectially, we could handle any iObject, but
   * that doesn't make sense for the current implementation, so we 
   * restrict it to iSector to avoid some pitfalls.<p>
   *
   * If a classname is given, search is restricted to nodes, in which
   * the key "classname" has the same value as the given classname.
   * the classname string is _not_ duplicated, so the caller is
   * responsible to take care, that the string is available while
   * the Iterator is alive.
   */
  csNodeIterator (iSector *pSector, const char *classname = NULL);

  /// The destructor as usual
  ~csNodeIterator ();

  /// Reuse the iterator for an other search
  void Reset (iSector *pSector, const char *classname = NULL);
  /// Get the object we are pointing at
  iMapNode *GetNode ();
  /// Move forward
  void Next ();
  /// Check if there are other nodes
  bool IsFinished () const;

protected:
  /// Skip all nodes with wrong classname
  void SkipWrongClassname ();
  /// Step to the next node in the sector, ignoring its classname
  void NextNode ();

  iObjectIterator *Iterator;
  const char *Classname;
  iMapNode *CurrentNode;
};

#endif // __KEYVAL_H__
