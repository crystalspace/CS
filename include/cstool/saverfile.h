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

#ifndef __CS_SAVERFILE_H__
#define __CS_SAVERFILE_H__

/**\file
 * Saver file
 */

#include "csextern.h"

#include "csutil/csobject.h"
#include "csutil/scf_implementation.h"
#include "imap/saverfile.h"
#include "iutil/selfdestruct.h"

/**
 * An object containing information about where to save a file.
 * Parent this iObject to engine objects.
 */
class CS_CRYSTALSPACE_EXPORT csSaverFile : 
  public scfImplementationExt2<csSaverFile, 
                               csObject, 
                               iSaverFile,
                               iSelfDestruct>
{
  const char* file;
  int filetype;

public:
  /// The constructor.
  csSaverFile (const char* file, int filetype);
  /// The destructor
  virtual ~csSaverFile ();
  
  virtual iObject *QueryObject () { return (csObject*)this; }
  
  virtual const char* GetFile () const;
  
  virtual int GetFileType () const;

  void SelfDestruct () {}
};

#endif // __CS_SAVERFILE_H__
