 /*
    Copyright (C) 2007 by Jorrit Tyberghein
	      (C) 2007 by Frank Richter

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

#include "cssysdef.h"
#include "csutil/documenthelper.h"

namespace CS
{
  namespace DocumentHelper
  {
    csString FlattenNode (iDocumentNode* node)
    {
      csString str;
      str << node->GetValue ();
      csRef<iDocumentAttributeIterator> attrIter = node->GetAttributes ();
      if (attrIter)
      {
	str << '[';
	while (attrIter->HasNext())
	{
	  csRef<iDocumentAttribute> attr = attrIter->Next();
	  str << attr->GetName () << '=' << attr->GetValue() << ',';
	}
	str << ']';
      }
      str << '(';
      csRef<iDocumentNodeIterator> it = node->GetNodes ();
      while (it->HasNext ())
      {
	csRef<iDocumentNode> child = it->Next ();
	str << FlattenNode (child);
	str << ',';
      }
      str << ')';
      
      return str;
    }
  } // namespace DocumentHelper
} // namespace CS
