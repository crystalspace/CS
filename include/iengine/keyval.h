/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __IENGINE_KEYVAL_H__
#define __IENGINE_KEYVAL_H__

#include "csutil/scf.h"

struct iObject;
struct iSector;

SCF_VERSION (iKeyValuePair, 0, 0, 1);

/**
 * A Key Value pair. This object contains a 'key' string and a 'value' string.
 * The 'key' string is the same as the name of the object as returned from
 * the iObject.
 */
struct iKeyValuePair : public iBase
{
  /// Get the iObject.
  virtual iObject *QueryObject() = 0;

  /// Get the key string of the pair.
  virtual const char *GetKey () const = 0;

  /// Set the key string of the pair
  virtual void SetKey (const char* key) = 0;

  /// Get the value string of the pair
  virtual const char *GetValue () const = 0;

  /// Set the value string of the pair
  virtual void SetValue (const char* value) = 0;
};

SCF_VERSION (iMapNode, 0, 0, 1);

/**
 * A node. This is an iObject that is bound to a position and a sector in
 * the world.
 */
struct iMapNode : public iBase
{
  /// Get the iObject.
  virtual iObject *QueryObject() = 0;

  /// Set the position of the node
  virtual void SetPosition (const csVector3& pos) = 0;
  /// Get the position of the node
  virtual const csVector3& GetPosition () const = 0;

  /// Set the sector of the node
  virtual void SetSector (iSector *pSector) = 0;
  /// Get the sector of the node
  virtual iSector *GetSector () const = 0;
};

#endif
