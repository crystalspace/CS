/*
    Copyright (C) 200 by Thomas Hieber
    
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
#include "csobject/csobject.h"
#include "csobject/nobjvec.h"

class csSector;

/**
 * A Key Value Pair.
 */
class csKeyValuePair : public csObject
{
public:
  /// The constructor. Requires both key and value. Data is being copied!
  csKeyValuePair (const char* Key, const char* Value);
  /// The destructor as usual
  virtual ~csKeyValuePair ();

  /// Get the key string of the pair.
  const char *GetKey () const { return GetName (); }

  /// Get the value string of the pair
  const char *GetValue () const { return m_Value; }

  /// Get the value of a key in an object. (shortcut)
  static const char *GetValue (csObject *pObject, const char *key);

  /// Set the value of a key in an object.
  void SetValue (const char* value);

private:
  char *m_Value;
  CSOBJTYPE;
  DECLARE_OBJECT_INTERFACE;
};

/**
 * Iterator for key value pairs.
 */
class csKeyValueIterator
{
public:
  /// The constructor. Requires the Object to search within!
  csKeyValueIterator (const csObject* pObject);
  /// The destructor as usual
  ~csKeyValueIterator ();

  /// Reuse the iterator for an other search
  void Reset (const csObject *pObject);
  /// Get the object we are pointing at
  csKeyValuePair *GetPair ();
  /// Move forward
  void Next ();
  /// Check if there are other keys
  bool IsFinished () const;
  /// Search for a key
  bool FindKey (const char *Name) { return m_Iterator.FindName (Name); }

protected:
  csObjIterator m_Iterator;
};

/**
 * A node.
 */
class csMapNode : public csObject
{
public:
  /// The constructor. Requires the Nodes name!
  csMapNode (const char *Name);
  /// The destructor as usual
  virtual ~csMapNode ();

  ///
  void SetPosition (const csVector3& pos) { m_Position = pos; }
  ///
  const csVector3& GetPosition () { return m_Position; }

  ///
  void SetSector (csSector *pSector) { m_pSector = pSector; }
  ///
  csSector *GetSector () { return m_pSector; }

  /// Get a node with the given name and a given classname. (shortcut)
  static csMapNode *GetNode (csSector *pSector, const char *name,
    const char *classname = NULL);

private:
  csSector *m_pSector;
  csVector3 m_Position;
  CSOBJTYPE;
  DECLARE_OBJECT_INTERFACE;
};

/**
 * A node iterator.
 */
class csNodeIterator
{
public:
  /**
   * The constructor. Theorectially, we could handle any csObject, but
   * that doesn't make sense for the current implementation, so we 
   * restrict it to csSector to avoid some pitfalls.<p>
   *
   * If a classname is given, search is restricted to nodes, in which
   * the key "classname" has the same value as the given classname.
   * the classname string is _not_ duplicated, so the caller is
   * responsible to take care, that the string is available while
   * the Iterator is alive.
   */
  csNodeIterator (const csSector *pSector, const char *classname = NULL);

  /// The destructor as usual
  ~csNodeIterator ();

  /// Reuse the iterator for an other search
  void Reset (const csSector *pSector, const char *classname = NULL);
  /// Get the object we are pointing at
  csMapNode *GetNode ();
  /// Move forward
  void Next ();
  /// Check if there are other nodes
  bool IsFinished () const;

protected:
  /// Skip all nodes with wrong classname
  void SkipWrongClassname ();

  csObjIterator m_Iterator;
  const char *m_Classname;
  csMapNode *m_pCurrentNode;
};

#endif // __KEYVAL_H__
