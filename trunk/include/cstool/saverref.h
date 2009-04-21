/* 
    Copyright (C) 2006 by Seth Yastrov

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

#ifndef __CS_SAVERREF_H__
#define __CS_SAVERREF_H__

/**\file
 * Saver reference implementations.
 */

#include "csextern.h"

#include "csutil/csobject.h"
#include "csutil/csstring.h"
#include "csutil/scf_implementation.h"
#include "imap/saverref.h"
#include "iutil/selfdestruct.h"

/**
 * An object representing a reference to a library.
 */
class CS_CRYSTALSPACE_EXPORT csPluginReference :
  public scfImplementationExt2<csPluginReference, 
                               csObject, 
                               iPluginReference,
                               iSelfDestruct>
{
  csString name;
  csString id;
  
public:
  /// The constructor.
  csPluginReference (const char* name, const char* id);
  /// The destructor
  virtual ~csPluginReference ();

  virtual const char* GetName () const;

  virtual const char* GetClassID () const;

  virtual iObject *QueryObject () { return (csObject*)this; }

  void SelfDestruct () {}
};

/**
 * An object representing a reference to a library.
 */
class CS_CRYSTALSPACE_EXPORT csLibraryReference :
  public scfImplementationExt2<csLibraryReference, 
                               csObject, 
                               iLibraryReference,
                               iSelfDestruct>
{
  csString file;
  csString path;
  bool checkDupes;
  
public:
  /// The constructor.
  csLibraryReference (const char* file, const char* path = 0,
    bool checkDupes = false);
  /// The destructor
  virtual ~csLibraryReference ();

  virtual const char* GetFile () const;

  virtual const char* GetPath () const;

  virtual bool GetCheckDupes () const;

  virtual iObject *QueryObject () { return (csObject*)this; }

  void SelfDestruct () {}
};

/**
 * An object representing an addon.
 */
class CS_CRYSTALSPACE_EXPORT csAddonReference :
  public scfImplementationExt2<csAddonReference, 
                               csObject, 
                               iAddonReference,
                               iSelfDestruct>
{
  csString plugin;
  csString paramsfile;
  csRef<iBase> addonobj;
  
public:
  /// The constructor.
  csAddonReference (const char* plugin, const char* paramsfile,
    iBase* addonobj = 0);
  /// The destructor
  virtual ~csAddonReference ();
  
  virtual const char* GetPlugin () const;

  virtual const char* GetParamsFile () const;

  virtual iBase* GetAddonObject () const;

  virtual iObject *QueryObject () { return (csObject*)this; }

  void SelfDestruct () {}
};

#endif // __CS_SAVERREF_H__
