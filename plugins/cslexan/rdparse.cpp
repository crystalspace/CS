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

#include <stdlib.h>
#include <string.h>

#include "cssysdef.h"
#include "csrenode.h"
#include "rdparse.h"

/*****
 Changelog:
 
 1. Fri Jun 01 03:00:01 PM MDT 2001 paradox <paradox@bbhc.org>  Created new file, implemented basic parsing.
 2. Sat Jun 02 10:16:35 AM MDT 2001 paradox <paradox@bbhc.org>  Fixed some compile errors, add "csrdparse.h" interface file.
 3. Mon Jun 04 08:16:45 AM MDT 2001 paradox <paradox@bbhc.org>  Noted that Eric Sunshine made some useful modifications to the includes 
 															 and fixed my naming mistake.
 ****/

/// Does a foreward lookup to support wildcards
bool
SupportWildCards(unsigned char **pRE, csRESyntaxTree &tree, csRENode **node)
{
  unsigned char *p = *pRE;
  bool  wild_found=false;
	
  // Foreward-lookup the next char for special handling of alternates and wildcards
  switch(*p)
  { 
    case '|':    
      ++p; // skip the or bar, since we've already processed it.
      wild_found=true;
      *node = new csREAltNode(*node, tree.Build(&p));
      
        // If we hit an error, abort.
        if (tree.GetError()) return false;
    break;
    
    case '*':
      ++p; // eat the star
      wild_found=true;
      *node = new csREStarNode(*node);
    break;
   
    case '+':
      ++p; // eat the plus
      wild_found=true;
      *node = new csREPlusNode(*node);
    break;

    case '?':
      ++p; // eat the qm
      wild_found=true;
      *node = new csREQmNode(*node);
    break;
  }

  *pRE = p; 	
   return wild_found;
} 

/// Builds a character leaf node, with proper checking for alternate branching.
csRENode *
BuildCharLeaf(unsigned char **pRE, csRESyntaxTree &tree)
{
  unsigned char *p = *pRE;
  csRENode      *node = new csRECharLeaf(*p);
             
  // Look ahead one, to see if we need to perform alternating.
  ++p;
  
  // Support scoping wild cards (e.g):  (aabb)++ matches "aabbaabb" and "aabbaabbaabb" but not "aabb"
  while(SupportWildCards(&p, tree, &node));        
  
  *pRE = p;
  return node;
}

/// Builds a parenthetical branch (infix order override) with support for wildcards and alternate branches.
csRENode *
BuildParenBranch(unsigned char **pRE, csRESyntaxTree &tree)
{
  unsigned char     *p = *pRE;
  csRENode *node;
  
  csRENode *top_node = new csRECatNode(tree.Build(&p), tree.Build(&p));
  
  // Continue to process until we have finished this set of parentheses.
  while(*p && *p!=')')
  { 
    //Abort on error.
    if (tree.GetError()) return NULL;
 
    node = new csRECatNode(top_node, tree.Build(&p));
    top_node = node;     	
  }
  
  // Check for missing ')'
  if (*p == 0)
  {
    tree.SetErrorCondition(RE_COMP_ERR_MISSING_RIGHT_PAREN);
    return NULL;
  }
  
  // Support scoping wild cards 
  while(SupportWildCards(&p, tree, &node));        
   
  *pRE = p;
  return node;
};

/// Builds a table leaf with support for wildcards and alternate branches.
csRENode *
BuildTableLeaf(unsigned char **pRE, csRESyntaxTree &tree)
{
  unsigned char     	 *p = *pRE;
  bool      	table[256];  // The table size should never exceed 256 different unsigned characters.
  unsigned char tbl[256];    // Since we'll never have more than 256 unsigned chars, we won't need a larger table.
  bool      	invert_match = false;
  csRENode 	 *node;
  
  memset(table, sizeof(table), 0);
  
  // Check to see if this is a named table
  if (*p==':')
  {  // Named table resolution
    ++p;
    
    if (memcmp(p, "alpha", 5) == 0) 
    {
     p+=7;
     *pRE=p;
     return new csRENamedTableLeaf(MATCH_ALPHA_TABLE);
    }
    else if  (memcmp(p, "alnum", 5) == 0) 
    {
     p+=7;
     *pRE=p;
     return new csRENamedTableLeaf(MATCH_ALNUM_TABLE);
    }
    else if  (memcmp(p, "digit", 5) == 0) 
    {
     p+=7;
     *pRE=p;
     return new csRENamedTableLeaf(MATCH_DIGIT_TABLE);
    } 
    else if  (memcmp(p, "upper", 5) == 0) 
    {
     p+=7;
     *pRE=p;
     return new csRENamedTableLeaf(MATCH_UPPER_TABLE);
    }
    else if  (memcmp(p, "lower", 5) == 0) 
    {
     p+=7;
     *pRE=p;
     return new csRENamedTableLeaf(MATCH_LOWER_TABLE);
    }
    else if  (memcmp(p, "print", 5) == 0) 
    {
     p+=7;
     *pRE=p;
     return new csRENamedTableLeaf(MATCH_PRINT_TABLE);
    }
    else if  (memcmp(p, "space", 5) == 0) 
    {
     p+=7;
     *pRE=p;
     return new csRENamedTableLeaf(MATCH_SPACE_TABLE);
    }
    else if  (memcmp(p, "cntrl", 5) == 0) 
    {
     p+=7;
     *pRE=p;
     return new csRENamedTableLeaf(MATCH_CNTRL_TABLE);
    }
    else if  (memcmp(p, "graph", 5) == 0) 
    {
     p+=7;
     *pRE=p;
     return new csRENamedTableLeaf(MATCH_GRAPH_TABLE);
    }
    else if  (memcmp(p, "xdigit", 6) == 0) 
    {
     p+=8;
     *pRE=p;
     return new csRENamedTableLeaf(MATCH_XDIGIT_TABLE);
    }
      	
    tree.SetErrorCondition(RE_COMP_ERR_UNKNOWN_CHAR_CLASS);
    return NULL;
  } 
     
  // Preprocess special unsigned characters.
  if (*p=='^') 
  {
    invert_match=true;
    ++p;
  }
  
  if (*p==']')
  {
    table[*p]=true;   
    ++p;
  }
      
  // Process entire bracket set.
  while(*p && *p!=']')
  {
    // check for ranges!
    if (*(p+1) == '-')
    {
      unsigned char start = *p;
      unsigned char end   = *(p+2);
     
      // insert this range
      for(unsigned int i=start; i<=end; ++i)
        table[i]=true; 
      
    }
    else
     table[*p] = true;
   }
  
  // Check for missing ')'
  if (*p == 0)
  {
    tree.SetErrorCondition(RE_COMP_ERR_MISSING_RIGHT_BRACKET);
    return NULL;
  }
  
  // Create a perfect table (no overlaps or repeats)
  {
    int i, j;
    
    for(i=0, j=0; i<256; ++i)
    {
      if (table[i]) tbl[j++]=(unsigned char)i;
    }
    
    tbl[j] = 0;   
  
  }
  
  // Create the new leaf
  node = new csRETableLeaf(tbl, invert_match);
  
  // Support scoping wild cards 
  while(SupportWildCards(&p, tree, &node));        
   
  *pRE = p;
  return node;
};


/// Recursive descent parsing of regular expressions
csRENode *
csRESyntaxTree::Build(unsigned char **regexp)
{
  unsigned char *p = *regexp;
  csRENode  *node=0;
 
  switch(*p)
  {
     case '[':
     { // Begin table creation.  
     
        // Special case: is there only one unsigned char inside the table?
        if (*(p+2) == ']')
        {  // yes, treat it like it wasn't a table.
         node = BuildCharLeaf(&p, *this);
         
         // Abort on error
         if (GetError()) return NULL;
         
         p+=3; // Set pointer to next unsigned character to be consumed.
        }
        else
        { // no, begin recursion
         ++p; //eat '['
         node = BuildTableLeaf(&p, *this);
        
         // If we hit an error, abort.
         if (GetError()) return NULL;
        }
           	
     } // End table creation
     break;	
     
     case ')':
       SetErrorCondition(RE_COMP_ERR_MISSING_LEFT_PAREN);
       return NULL;
     break;
     
     case ']':
       SetErrorCondition(RE_COMP_ERR_MISSING_LEFT_BRACKET);
       return NULL;
     break;
     	
     case '(':
     { // Begin parenthetical recursion
        
        // Special case: is there only one unsigned char inside the parentheses?
        if (*(p+2) == ')')
        {  // yes, treat it like it wasn't in parentheses.
         node = BuildCharLeaf(&p, *this);
         
         // Abort on error
         if (GetError()) return NULL;
         
         p+=3; // Set pointer to next unsigned character to be consumed.
        }
        else
        { // no, begin recursion
         ++p; // eat '('
         node = BuildParenBranch(&p, *this);
        
         // If we hit an error, abort.
         if (GetError()) return NULL;
  
        }
        
      } // End parenthetical recursion 
      break;     	
    	
      default:
        node = BuildCharLeaf(&p, *this);
     
        // If we hit an error, abort.
 	   if (GetError()) return NULL;
  
      break;  	
    }  // end switch (*p)	
   
 // Save the location of our pointer.
 *regexp = p;
 
 // Return the top node
 return node;
}


bool
csRESyntaxTree::Compile(unsigned char *regexp)
{
 unsigned char *pRE = regexp;
 
 SetErrorCondition(RE_COMP_ERR_NONE);
 
 // Create the initial top node.
 csRENode *top_node= new csRECatNode(Build(&pRE), Build(&pRE));
  
 // Recursively create nodes, with the new node always being on top.  This allows us to create proper LALR syntax tree.
 while(*pRE)
 {
   // If we hit an error, abort.
   if (GetError()) return false;
 
   csRENode *node = new csRECatNode(top_node, Build(&pRE));	
   top_node = node;
 }
 
 return true;	
}
