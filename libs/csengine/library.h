/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef LIBRARY_H
#define LIBRARY_H

#include "csobject/csobj.h"
#include "csengine/basic/csobjvec.h"

class ThingTemplate;
class csSpriteTemplate;
class Archive;

/**
 * A library is a collection of templates and other stuff.
 */
class csLibrary : public csObject
{
public:
  /// List of Thing templates.
  csObjVector thing_templates;
  /// List of sprite templates.
  csObjVector sprite_templates;

private:
  /// The archive of this library.
  Archive* ar;

public:
  ///
  csLibrary ();
  ///
  virtual ~csLibrary ();

  ///
  void Clear ();

  ///
  void SetArchive (Archive* ar) { csLibrary::ar = ar; }

  ///
  Archive* GetArchive () { return ar; }

  CSOBJTYPE;
};

#endif /*LIBRARY_H*/

