/*
  Copyright (C) 2011 by Michael Gist

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
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

#ifndef __CS_IMAP_LOADABLE_H__
#define __CS_IMAP_LOADABLE_H__

#include "csutil/ref.h"
#include "csutil/scf_interface.h"
#include "iutil/document.h"

struct iLoadable : public virtual iBase
{
  SCF_INTERFACE(iLoadable, 1, 0, 0);

  /**
   * Loads an object from a document node.
   * Returns an invalid object on failure.
   */
  virtual csPtr<iBase> Load (iDocumentNode* node) = 0;

  /**
   * Returns whether this loadable is thread-safe.
   */
  virtual bool IsThreadSafe () const = 0;
};

#endif // __CS_IMAP_LOADABLE_H__
