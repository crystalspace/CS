 /*
  Copyright (C) 2007 by Jorrit Tyberghein
	            2007 by Frank Richter
                2007 by Marten Svanfeldt

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
#include "csutil/stringarray.h"

namespace CS
{
namespace DocSystem
{
  csString FlattenNode (iDocumentNode* node)
  {
    csString str;
    str.SetGrowsBy (0);
    str << node->GetValue ();
    csRef<iDocumentAttributeIterator> attrIter = node->GetAttributes ();
    if (attrIter)
    {
      csStringArray attrStrs;
      while (attrIter->HasNext())
      {
	csRef<iDocumentAttribute> attr = attrIter->Next();
	csString str;
	str << attr->GetName () << '=' << attr->GetValue() << ',';
	attrStrs.Push (str);
      }
      str << '[';
      attrStrs.Sort (true);
      for (size_t i = 0; i < attrStrs.GetSize(); i++)
        str << attrStrs[i]; 
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
} // namespace DocSystem
} // namespace CS
