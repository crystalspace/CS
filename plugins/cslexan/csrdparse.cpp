#include <stdlib.h>

#include "csrenode.h"
#include "csrdparse.h"

/*****
 Changelog:
 
 Fri Jun 01 03:00:01 PM MDT 2001 paradox <paradox@bbhc.org>  Created new file, implemented basic parsing

 ****/

/// Does a foreward lookup to support wildcards
bool
SupportWildCards(char **pRE, csRESyntaxTree &tree, csRENode **node)
{
  char *p = *pRE;
  bool  wild_found=false;
	
  // Foreward-lookup the next char for special handling of alternates and wildcards
  switch(*p)
  { 
    case '|':    
      ++p; // skip the or bar, since we've already processed it.
      wild_found=true;
      *node = new csREAltNode(*node, tree.Build(&p));
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
BuildCharLeaf(char **pRE, csRESyntaxTree &tree)
{
  char     *p = *pRE;
  csRENode *node = new csRECharLeaf(*p);
             
  // Look ahead one, to see if we need to perform alternating.
  ++p;
  
  // Support scoping wild cards (e.g):  (aabb)++ matches "aabbaabb" and "aabbaabbaabb" but not "aabb"
  while(SupportWildCards(&p, tree, &node));        
  
  *pRE = p;
  return node;
}

/// Builds a parenthetical branch (infix order override) with support for wildcards and alternate branches.
csRENode *
BuildParenBranch(char **pRE, csRESyntaxTree &tree)
{
  char     *p = *pRE;
  csRENode *node;
  
  csRENode *top_node = new csRECatNode(tree.Build(&p), tree.Build(&p));
  
  // Continue to process until we have finished this set of parentheses.
  while(*p && *p!=')')
  { 
    node = new csRECatNode(top_node, tree.Build(&p));
    top_node = node;     	
  }
  
  // Support scoping wild cards 
  while(SupportWildCards(&p, tree, &node));        
   
  *pRE = p;
  return node;
};

/// Recursive descent parsing of regular expressions
csRENode *
csRESyntaxTree::Build(char **regexp)
{
  char *p = *regexp;
  csRENode  *node=0;
 
  switch(*p)
  {
     case '[':
     { // Begin table creation.  
       
         
     	
     	
     } // End table creation
     break;	
  	
     case '(':
      { // Begin parenthetical recursion
        
        // Special case: is there only one char inside the parentheses?
        if (*(p+2) == ')')
        {  // yes, treat it like it wasn't in parentheses.
         node = BuildCharLeaf(&p, *this);
         p+=3; // Set pointer to next character to be consumed.
        }
        else
        { // no, begin recursion
         node = BuildParenBranch(&p, *this); 
        }
        
      } // End parenthetical recursion 
      break;     	
    	
      default:
        node = BuildCharLeaf(&p, *this); 
      break;  	
    }  // end switch (*p)	
   
 // Save the location of our pointer.
 *regexp = p;
 
 // Return the top node
 return node;
}


bool
csRESyntaxTree::Compile(char *regexp)
{
 char *pRE = regexp;
 
 // Create the initial top node.
 csRENode *top_node= new csRECatNode(Build(&pRE), Build(&pRE));
 
 // Recursively create nodes, with the new node always being on top.  This allows us to create proper LALR syntax tree.
 while(*pRE)
 {
   csRENode *node = new csRECatNode(top_node, Build(&pRE));	
   top_node = node;	
 }
 
 return true;	
}
