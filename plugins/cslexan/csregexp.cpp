/*
    Copyright (C) 2001 by Christopher Nelson
  
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

#include <string.h>
#include "cslex.h"

IMPLEMENT_IBASE (csRegExp)
  IMPLEMENTS_INTERFACE (iRegExp)
IMPLEMENT_IBASE_END

csRegExp::csRegExp(iBase* p):code(0), length(0)
{
  CONSTRUCT_IBASE (p);
}

csRegExp::~csRegExp() { if (code) delete [] code; }

bool 
csRegExp::GetOp(unsigned index, unsigned char &op)
{
 if (index<length)
 {
   op = code[index];
   return true;
 }
 else
 {
   return false;
 }
}

bool
csRegExp::SetOp(unsigned index, unsigned char op)
{
 if (index>=length && index < 0xfff)
 {
  unsigned char *newcode = new unsigned char[index+16];
  
  memset(newcode, 0, index+16);
  
  if (code)
  {
    memcpy(newcode, code, length);
    delete [] code;
  }
  
  code = newcode;
  length=index+16;
 }
 else if (index > 0xfff)
   return false;
 
 code[index] = op;

 return true;
}

bool 
csRegExp::Compact()
{

 for(unsigned int i=0; i<length; ++i)
 {
   if (code[i]==OP_END)
   {
    unsigned char *newcode = new unsigned char[i+1];
    
    memcpy(newcode, code, i+1);
    
    delete [] code;
    
    code = newcode;
    
    length = i+1;
    
    return true;
   }
 }
 
 // fail if we can't find the OP_END
 return false;
}
