#ifndef __CS_RDPARSE_H__
#define __CS_RDPARSE_H__

/*****************************************************************************
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
****************************************************************************/

 /****** Change log
  1. Sat Jun 02 10:06:15 AM MDT 2001 paradox <paradox@bbhc.org> Creation of file
 ******/
 
 const unsigned int RE_COMP_ERR_NONE				  = 0; 
 const unsigned int RE_COMP_ERR_MISSING_RIGHT_PAREN   = 1;
 const unsigned int RE_COMP_ERR_MISSING_LEFT_PAREN    = 2;
 const unsigned int RE_COMP_ERR_MISSING_RIGHT_BRACKET = 3;
 const unsigned int RE_COMP_ERR_MISSING_LEFT_BRACKET  = 4;
 const unsigned int RE_COMP_ERR_UNKNOWN_CHAR_CLASS    = 5;

 class csRESyntaxTree
 {
    unsigned int last_error;
     	
 public:
	csRENode *Build(char **regexp);
	bool      Compile(char *regexp);
	
 public:
    void SetErrorCondition(unsigned int errc)
    { last_error = errc; }
    
    unsigned int GetError()
    { return last_error; }
 };

#endif // __CS_RDPARSE_H__
