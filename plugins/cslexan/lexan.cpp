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

#include "cssysdef.h"
#include "cslex.h"
#include "csutil/scfstr.h"
#include <ctype.h>

CS_IMPLEMENT_PLUGIN

SCF_IMPLEMENT_IBASE (csLexicalAnalyzer)
  SCF_IMPLEMENTS_INTERFACE (iLexicalAnalyzer)
  SCF_IMPLEMENTS_EMBEDDED_INTERFACE (iPlugin)
SCF_IMPLEMENT_IBASE_END

SCF_IMPLEMENT_EMBEDDED_IBASE (csLexicalAnalyzer::eiPlugIn)
  SCF_IMPLEMENTS_INTERFACE (iPlugin)
SCF_IMPLEMENT_EMBEDDED_IBASE_END

SCF_IMPLEMENT_FACTORY (csLexicalAnalyzer)

SCF_EXPORT_CLASS_TABLE (cslexan)
  SCF_EXPORT_CLASS (csLexicalAnalyzer, "crystalspace.scanner.regex",
    "Crystal Space regular-expression-based scanner")
SCF_EXPORT_CLASS_TABLE_END

csLexicalAnalyzer::csLexicalAnalyzer(iBase* p):next_key(1)
{
  SCF_CONSTRUCT_IBASE (p);
  SCF_CONSTRUCT_EMBEDDED_IBASE(scfiPlugin);
}

csLexicalAnalyzer::~csLexicalAnalyzer()
{
  void *item=re_list.GetFirstItem();
 
  while(item)
  {
    key_re_pair *check = (key_re_pair *)item;
    
    delete check;
    
    re_list.SetCurrentItem(0);
  
    item = re_list.GetNextItem();
  }
}

bool 
csLexicalAnalyzer::Initialize(iSystem *sys)
{
  return true;
}

bool 
csLexicalAnalyzer::RegisterRegExp(unsigned int key, iRegExp &re)
{
 re_list.AddItem(new key_re_pair(re, key));
 
 return true;
}

bool 
csLexicalAnalyzer::UnregisterRegExp(unsigned int key)
{
 void *item=re_list.GetFirstItem();
 
 while(item)
 {
   key_re_pair *check = (key_re_pair *)item;
   
   if (check->key == key)
   {
     re_list.RemoveItem(item);
     return true;
   }
 
   item = re_list.GetNextItem();
 }
 
 exec_error = RE_EXEC_ERR_KEY_DOES_NOT_EXIST;
 
 return false;
}

unsigned int 
csLexicalAnalyzer::GetMatchedKey()
{
 return last_matched_key;
}

iString *
csLexicalAnalyzer::GetMatchedText()
{
 return new scfString(last_matched_text.GetData());
}

bool 
csLexicalAnalyzer::PushStream(iDataBuffer &buf)
{
 return false;
}

bool 
csLexicalAnalyzer::PopStream()
{
 return false;
}

bool 
csLexicalAnalyzer::Exec(iRegExp &re)
{
  stream_state *ss   = STATIC_CAST(stream_state*,re_list.GetFirstItem());
  uint8	       *buf  = ss->buf->GetUint8();
  unsigned int  pos  = ss->pos;
  unsigned int  i    = 0,
  	        saved_i,
  	        num_matches=0;
  unsigned char op=0;
  unsigned char opts;
  bool          matched=false;
   
  csString	str;
  
  while(op!=OP_END && pos < ss->buf->GetSize())
  {
    saved_i = i;
    re.GetOp(i++, op);
    
    if (op == OP_END) break;
     
    bool extended_op = (op == 0);
    bool extended_match=false;
    bool op_is_nop=false;
    bool keep_match=false;
   
    
    // if the instruction is NOT an escape, then match it exactly to the buffer contents
    if (!extended_op)
    {
      
      if (op == buf[pos])
      {
        matched=true;
        str+=buf[pos];
      }  // end if buffer matches instruction (direct op match)
    } // end if instruction isn't extended
    else
    {
      // get escaped op
      re.GetOp(i++, op);
      
      switch(op)
      {
        case OP_EXT_NOP:
          op_is_nop=true;
        break;
        
        case OP_EXT_ALPHA_TABLE:		// op matches isalpha
          if (isalpha(buf[pos]))
          {
            matched=true;
            str+=buf[pos];
          }
        break;
        
	case OP_EXT_DIGIT_TABLE:		// op matches isdigit
	  if (isdigit(buf[pos]))
          {
            matched=true;
            str+=buf[pos];
          }
        break;
        
	case OP_EXT_ALNUM_TABLE:		// op matches isalnum
	  if (isalnum(buf[pos]))
          {
            matched=true;
            str+=buf[pos];
          }
        break;
        
	case OP_EXT_PUNCT_TABLE:		// op matches ispunct
	  if (ispunct(buf[pos]))
          {
            matched=true;
            str+=buf[pos];
          }
        break;
        
	case OP_EXT_SPACE_TABLE:		// op matches isspace
	  if (isspace(buf[pos]))
          {
            matched=true;
            str+=buf[pos];
          }
        break;
        
	case OP_EXT_CNTRL_TABLE:		// op matches iscntrl
	  if (iscntrl(buf[pos]))
          {
            matched=true;
            str+=buf[pos];
          }
        break;
        
	case OP_EXT_GRAPH_TABLE:		// op matches isgraph
	  if (isgraph(buf[pos]))
          {
            matched=true;
            str+=buf[pos];
          }
        break;
        
	case OP_EXT_LOWER_TABLE:		// op matches islower
	  if (islower(buf[pos]))
          {
            matched=true;
            str+=buf[pos];
          }
        break;
        
	case OP_EXT_UPPER_TABLE:		// op matches isupper
	  if (isupper(buf[pos]))
          {
            matched=true;
            str+=buf[pos];
          }
        break;
        
	case OP_EXT_PRINT_TABLE:		// op matches isprint
	  if (isprint(buf[pos]))
          {
            matched=true;
            str+=buf[pos];
          }
        break;
        
	case OP_EXT_XDIGIT_TABLE:		// op matches isxdigit
	  if (isxdigit(buf[pos]))
          {
            matched=true;
            str+=buf[pos];
          }
        break;
        
        case OP_EXT_CUSTOM_TABLE:
          {
            
          
          }  // end custom table scope
        break;
        
	case OP_EXT_LOGICAL_OR:			// turns on the OR flag in the VM
	default:
	break;
	    
      } // end switch instruction
    } // end else instruction IS extended
    
    
    // examine modifiers and stuff
    re.GetOp(i++, opts);
    
    // if there are no modifiers and the instruction did not match, then return false
    if (opts == 0 && !matched) return false;
    
    // this is a non or many match (optional sequence) then keep going
    switch(opts & 0x3)
    {
      case OP_MATCH_NONE_OR_MORE:
        if   (matched)
        {
         num_matches++;
         i=saved_i;
         extended_match=true;
        }
        else num_matches = 0;
      break;
      
      case OP_MATCH_ONE_OR_NONE:
       if (matched && num_matches<1)
        {
          num_matches++;
          i=saved_i;
          extended_match=true;
        }
        else num_matches = 0;
      break;
        
      case OP_MATCH_ONE_OR_MORE:
        if   (!matched && num_matches==0) return false;
        else if (matched)
        {
          num_matches++;
          i=saved_i;
          extended_match=true;
        }
        else num_matches=0;
      break;
      
    }  // end modifier op
 
    // special handling for NOP's with OP_MATCH   
    if (opts & 3 !=0 && op_is_nop)
    {
      // if there was an OP_MATCH, but it failed to continue, pop the execution state stack
      if (!extended_match)
      {
        execution_state *es = STATIC_CAST(execution_state*,es_list.GetFirstItem());
      
        if (es)
        {
          delete es;
          es_list.RemoveItem();
        }
      }
      // We may need to reset the instruction pointer farther back, if the stack holds anything
      else
      {
        execution_state *es = STATIC_CAST(execution_state*,es_list.GetFirstItem());
      
        if (es)
        {
          i = es->ip;
        } 
      } // end else we need to return to the last place pushed
    }  // end if extended match and instruction is NOP
        
    // check the push flag
    if (opts & OP_PUSH_ADDRESS)
    {
      es_list.AddItem(new execution_state(i));
    }
        
    // possibly reset the matched var for next time around
    if (!keep_match) matched=false;
    
    // increment the buffer position
    ++pos;
    
  } // end while i is not at the end of instruction sequence
  
  // if we ran out of buffer before running out of instructions, return false
  // if (op!=OP_END) return false;  // does this cause false negatives? // FIXME! 
  
  // save the matched text
  last_matched_text = str;
  
  return true;
}

unsigned int 
csLexicalAnalyzer::Match()
{

 return 0;
}
