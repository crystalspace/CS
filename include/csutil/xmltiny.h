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

#ifndef __CSUTIL_XMLTINY_H__
#define __CSUTIL_XMLTINY_H__

#include "iutil/document.h"

class csTinyXmlNode;
class TiDocumentNode;

/**
 * This is an SCF compatible wrapper for the TinyXml parser in csutil.
 */
class csTinyDocumentSystem : public iDocumentSystem
{
private:
  friend class csTinyXmlNode;
  csTinyXmlNode* pool;

public:
  csTinyDocumentSystem ();
  virtual ~csTinyDocumentSystem ();

  SCF_DECLARE_IBASE;

  virtual csRef<iDocument> CreateDocument ();

  /// Internal function: don't use!
  csTinyXmlNode* Alloc ();
  /// Internal function: don't use!
  csTinyXmlNode* Alloc (TiDocumentNode*);
  /// Internal function: don't use!
  void Free (csTinyXmlNode* n);
};

#endif // __CSUTIL_XMLTINY_H__

