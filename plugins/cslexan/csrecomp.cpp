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

#include <ctype.h>
#include <string.h>
#include "cssysdef.h"
#include "cslex.h"

SCF_IMPLEMENT_IBASE (csRegExpCompiler)
  SCF_IMPLEMENTS_INTERFACE (iRegExpCompiler)
SCF_IMPLEMENT_IBASE_END

csRegExpCompiler::csRegExpCompiler(iBase* p):
  paren_recursion_depth(0), paren_recursion_ended_at(0)
{
  SCF_CONSTRUCT_IBASE (p);
}

bool 
csRegExpCompiler::Compile(char *regexp, iRegExp &re, unsigned int start, unsigned int end)
{
 re_index=start;
 
 if (start==0)
   op_index=0;
 
 if (end==0)
   end=strlen(regexp);
 
 while(re_index<end);
 {
   char c = regexp[re_index];
 
   switch(c)
   {
     case '[': // start a table or character class
      if (regexp[re_index+1] == ':')
      {
       //perform well-known class compiling.
       unsigned int i;
              
       // first get the string we need
       for (i=re_index+1; regexp[i] != ':'; ++i);
       
       unsigned int length = i - re_index + 1;
       char *bn = new char[length];
       
       memset(bn, 0, length);
       memcpy(bn, regexp+re_index+2, length-1);
       
       re.SetOp(op_index, OP_ESC);
       
       if      (strcmp(bn, "alpha") == 0)  re.SetOp(op_index+1, OP_EXT_ALPHA_TABLE);
       else if (strcmp(bn, "digit") == 0)  re.SetOp(op_index+1, OP_EXT_DIGIT_TABLE);
       else if (strcmp(bn, "alnum") == 0)  re.SetOp(op_index+1, OP_EXT_ALNUM_TABLE);
       else if (strcmp(bn, "upper") == 0)  re.SetOp(op_index+1, OP_EXT_UPPER_TABLE);
       else if (strcmp(bn, "lower") == 0)  re.SetOp(op_index+1, OP_EXT_LOWER_TABLE);
       else if (strcmp(bn, "print") == 0)  re.SetOp(op_index+1, OP_EXT_PRINT_TABLE);
       else if (strcmp(bn, "space") == 0)  re.SetOp(op_index+1, OP_EXT_SPACE_TABLE);
       else if (strcmp(bn, "cntrl") == 0)  re.SetOp(op_index+1, OP_EXT_CNTRL_TABLE);
       else if (strcmp(bn, "graph") == 0)  re.SetOp(op_index+1, OP_EXT_GRAPH_TABLE);
       else if (strcmp(bn, "xdigit") == 0)  re.SetOp(op_index+1, OP_EXT_XDIGIT_TABLE);
       else
       {
         compile_error = RE_COMP_ERR_UNKNOWN_CHAR_CLASS;
         return false;
       }

       if (regexp[i+1] != ']')
       {
         compile_error = RE_COMP_ERR_MISSING_RIGHT_BRACKET;
         return false;
       }
       
       // write empty mod 
       re.SetOp(op_index+2, 0);
       
       // fixup the index pointer to skip the ending bracket.
       re_index=i+1;
       
       // fixup the op pointer
       op_index+=3;
       
      } // end if is character class
      else
      {
        bool keep_going=true;
        unsigned int  op_count_index=0;
        unsigned char count=0;
        
        // okay, setup the table prologue
        re.SetOp(op_index, OP_ESC);
        re.SetOp(op_index+1, OP_EXT_CUSTOM_TABLE);
        re.SetOp(op_index+2, OP_PUSH_ADDRESS);
        
        op_index+=3;
        
        // setup and clear the table length position
        op_count_index=op_index++;
		        
	// special case this parse, and do it now
	while(re_index<end && keep_going)
	{
	 c = regexp[re_index];
	 
	 // if we encounter an end bracket not by the start bracket, get out. (POSIX rules)
	 if (c==']' && regexp[re_index-1]!='[')
	  keep_going=false;
	  
	 // otherwise, every character is taken to be a literal
	 re.SetOp(op_index++, c);
	 
	 // update table size
	 re.SetOp(op_count_index, ++count);
	 
	 re_index++;
	}
	
	if (keep_going)
	{
	  compile_error = RE_COMP_ERR_MISSING_RIGHT_BRACKET;
	  return false;
	}
	
	// okay, setup the table postlogue
        re.SetOp(op_index, OP_ESC);
        re.SetOp(op_index+1, OP_EXT_NOP);
        re.SetOp(op_index+2, 0);	
      
      } // else is a custom table
     break;
     
     case ']':
       compile_error = RE_COMP_ERR_MISSING_LEFT_BRACKET;
       return false;
     break;
     
     case '(': // start recursion
      paren_recursion_depth++;
      
      re.SetOp(op_index,   OP_ESC);
      re.SetOp(op_index+1, OP_EXT_NOP);
      re.SetOp(op_index+2, OP_PUSH_ADDRESS);
      op_index+=3;
         
      if (!Compile(regexp, re, re_index+1)) return false;
     break;
     
     case ')': // end recursion
       if (paren_recursion_depth)
       {
         paren_recursion_depth--;
       
         // insert a NOP so that we can have postlogue modifiers work right
         re.SetOp(op_index,   OP_ESC);
         re.SetOp(op_index+1, OP_EXT_NOP);
         re.SetOp(op_index+2, 0);
         op_index+=3;
         return true;
       }
       else
       {
         compile_error = RE_COMP_ERR_MISSING_LEFT_PAREN;
         return false;
       }
     break;    
        
     
     case '|': // perform logical or
       re.SetOp(op_index,   OP_ESC);
       re.SetOp(op_index+1, OP_EXT_LOGICAL_OR);
       re.SetOp(op_index+2, 0);
       
       op_index+=3;
     break;
     
     case '{': // start custom character class
     break;
     
     case '*': // add MATCH_NONE_OR_MORE to modifier byte of previous index
     {
       unsigned char modb;
       
       if (re.GetOp(op_index-1, modb))
         re.SetOp(op_index-1, (modb & 0xfc) | OP_MATCH_NONE_OR_MORE);         
       
     } break;
     
     case '+': // add MATCH_ONE_OR_MORE to modifier byte of previous index
     {
       unsigned char modb;
       
       if (re.GetOp(op_index-1, modb))
         re.SetOp(op_index-1, (modb & 0xfc) | OP_MATCH_ONE_OR_MORE);         
       
     } break;
     
     case '?': // add MATCH_ONE_OR_NONE to modifier byte of previous index
     {
       unsigned char modb;
       
       if (re.GetOp(op_index-1, modb))
         re.SetOp(op_index-1, (modb & 0xfc) | OP_MATCH_ONE_OR_NONE);         
       
     } break;
     
     case '\\': // escape the next character, and set as a single match
     {
       ++re_index;
       
       // set op
       re.SetOp(op_index, STATIC_CAST(unsigned char,regexp[re_index]));
       
       // clear modifier
       re.SetOp(op_index+1, 0);
       
       op_index+=2;
     
     } break;
          
     default:  // add character as a single match
     
       // set op
       re.SetOp(op_index, STATIC_CAST(unsigned char,c));
       
       // clear modifier
       re.SetOp(op_index+1, 0);
       
       op_index+=2;
     break;		   		
   
   } // end switch(regexp[i])
   
   ++re_index;
   
 } // end while regexp[i]
 
 // return an error if we're still waiting for the end of a set of parenthesis.

 if (paren_recursion_depth)
 {
   compile_error = RE_COMP_ERR_MISSING_RIGHT_PAREN;
   return false;
 }

 return true;
}
