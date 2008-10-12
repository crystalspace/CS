/*
    Copyright (C) 2008 by Frank Richter

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

#include "csgfx/shadervarnameparser.h"

namespace CS
{
  namespace Graphics
  {
    ShaderVarNameParser::ShaderVarNameParser (const char* identifier)
      : errorPos ((size_t)~0)
    {
      if (identifier == 0) return;
      
      size_t nameLen = 0;
      while ((identifier[nameLen] != 0) && (identifier[nameLen] != '['))
        nameLen++;
      
      name.Replace (identifier, nameLen);
      
      const char* bracket = identifier + nameLen;
      while (*bracket != 0)
      {
        if (*bracket != '[')
        {
          errorPos = bracket - identifier;
          return;
        }
        bracket++;
        
        char* indexEnd;
        long val = strtol (bracket, &indexEnd, 10);
        if (indexEnd != bracket) indices.Push (val);
        bracket = indexEnd;
        
        if (*bracket != ']')
        {
          errorPos = bracket - identifier;
          return;
        }
        bracket++;
      }
    }
    
  } // namespace Graphics
} // namespace CS
