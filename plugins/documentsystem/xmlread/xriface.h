/*
    Copyright (C) 2002 by Jorrit Tyberghein

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_XRIFACE_H__
#define __CS_XRIFACE_H__

#include "iutil/document.h"

struct csXmlReadNode;
class TrDocumentNode;

/**
 * This is an SCF compatible wrapper for the TinyXml parser in csutil.
 */
class csXmlReadDocumentSystem : public iDocumentSystem
{
private:
  friend struct csXmlReadNode;
  csXmlReadNode* pool;

public:
  csXmlReadDocumentSystem ();
  virtual ~csXmlReadDocumentSystem ();

  SCF_DECLARE_IBASE;

  virtual csRef<iDocument> CreateDocument ();

  /// Internal function: don't use!
  csXmlReadNode* Alloc ();
  /// Internal function: don't use!
  csXmlReadNode* Alloc (TrDocumentNode*);
  /// Internal function: don't use!
  void Free (csXmlReadNode* n);
};

#endif // __CS_XRIFACE_H__

