/*
  Copyright (C) 2006 by Frank Richter

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
#include "cstool/numberedfilenamehelper.h"
#include <ctype.h>

namespace CS
{
  void NumberedFilenameHelper::SetMask (const char* mask)
  {
    if ((mask == 0) || (*mask == 0))
    {
      format = "%u";
      return;
    }
    
    csString sanitizedMask;
    // since this string is passed to format later,
    // replace all '%' with '%%'
    {
      const char* pos = mask;
      while (pos)
      {
	const char* percent = strchr (pos, '%');
	if (percent)
	{
	  sanitizedMask.Append (pos, percent-pos);
	  sanitizedMask.Append ("%%");
	  pos = percent + 1;
	}
	else
	{
	  sanitizedMask.Append (pos);
	  pos = 0;
	}
      }
    }
    // scan for the rightmost string of digits
    // and create an appropriate format string
    {
      uint formatNumberDigits = 0;
  
      size_t end = sanitizedMask.Length();
      while ((end > 0) && (!isdigit (sanitizedMask[end-1])))
      {
	end--;
      }
      if (end > 0)
      {
	while ((end > 0) && (isdigit (sanitizedMask[end-1])))
	{
	  formatNumberDigits++; 
	  end--;
	}
	
	csString nameForm;
	nameForm.Format ("%%0%uu", formatNumberDigits);

	format.Replace (sanitizedMask, end);
	format.Append (nameForm);
	format.Append (sanitizedMask.Slice (end+formatNumberDigits));
      }
      else
      {
	format = sanitizedMask;
        size_t dot = sanitizedMask.FindLast ('.');
        if (dot != (size_t)-1)
        {
          format.Insert (dot, "%u");
        }
        else
        {
          format.Append ("%u");
        }
      }
    }
  }
  
  csString NumberedFilenameHelper::FindNextFilename (iVFS* vfs)
  {
    csString fn;
    if (vfs)
    {
      do
      {
        fn = NextFilename();
      } 
      while (vfs->Exists (fn));
      return fn;
    }
    else
    {
      struct stat s;
      do
      {
        fn = NextFilename();
      } 
      while (stat (fn, &s) == 0);
      return fn;
    }
  }
}
