#ifndef __CS_CSRENODE_H__
#define __CS_CSRENODE_H__

#include <string.h>

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
  1. Fri Jun 01 09:59:08 AM MDT 2001 paradox <paradox@bbhc.org> Creation of file
******/

/**********
   In order to derive the proper parsing states, we must create a syntax-directed parse tree of the regular expression.  Once we
 finish this tree, we must perform a depth-first traversal of it in order find out our state transition information.  The tree
 has a few types of nodes:
 
 	1. cat-node: a cat node is a concantenation node, meaning that this value, if matched, should be concantenated onto our 
 current string.  This is the most common type of node.  Cat nodes are binary.
     2. star-node: a star node is a many or none match, equivalent to "*"
     3. plus-node: a plus node is a one or more match, equivalent to "+"
     4. alt-node:  matches an alternate.  alternates are binary. equivalent to "|"
     5. qm-node:   matches one or none.  equivalent to "?"
     
   Leaves are always the individual unsigned characters to match, unless they are special table leaves.  A table leaf supports "[]", and
 therefore may match any of the unsigned characters in the table.  However, it still only matches ONE unsigned character from the table.  

   These classes are for the INTERNAL use of the engine ONLY.  They are NOT intended to be exported by any module.

 */
 
 const unsigned char CSRE_CATNODE 	= 1;
 const unsigned char CSRE_STARNODE	= 2;
 const unsigned char CSRE_PLUSNODE	= 3;
 const unsigned char CSRE_ALTNODE	 = 4;
 const unsigned char CSRE_QMNODE	  = 5;
 
 const unsigned char CSRE_CHLEAF	  = 6;
 const unsigned char CSRE_TBLEAF	  = 7;
 
 /// The base class for nodes.  All nodes have a type. 
 class csRENode
 {
 public:
    /// Returns the type of node this is.
    virtual unsigned char Type()=0;
 
    /// Gets the left branch, or if this is a unary node, gets the only branch.
    virtual csRENode *Left()=0;
    
    /// Gets the right branch.  Returns NULL if there's none.
    virtual csRENode *Right()=0;
    
    /// Returns an acceptance match for this leaf (if node is a leaf)
    virtual bool Accept(int)
    { return false; }    
};
 
 /// The cat-node class.  Contains a pointer to the next level down.  Performs a cat operation.
 class csRECatNode : public csRENode
 {
   csRENode *left, *right;
 
 public:
   csRECatNode(csRENode *l, csRENode *r):left(l), right(r) {};
   virtual ~csRECatNode() {};
   
 public:
   virtual unsigned char Type()
   { return CSRE_CATNODE; }
 
   virtual csRENode *Left()
   { return left; }
   
   virtual csRENode *Right()
   { return right;  } 
 };
 
 /// The star-node class.  Contains a pointer to the next level down.  Performs a none or many operation.
 class csREStarNode : public csRENode
 {
   csRENode *child;
   
 public:
   csREStarNode(csRENode *c):child(c) {};
   virtual ~csREStarNode() {};
   
 public:
   virtual unsigned char Type()
   { return CSRE_STARNODE; }
 
   virtual csRENode *Left()
   { return child; }
   
   virtual csRENode *Right()
   { return NULL;  } 
 };
 
 /// The plus-node class.  Contains a pointer to the next level down.  Performs a one or many operation.
 class csREPlusNode : public csRENode
 {
   csRENode *child;
    
 public:
   csREPlusNode(csRENode *c):child(c) {};
   virtual ~csREPlusNode() {};
   
 public:
   virtual unsigned char Type()
   { return CSRE_PLUSNODE; }
 
   virtual csRENode *Left()
   { return child; }
   
   virtual csRENode *Right()
   { return NULL;  } 
 };
 
 /// The qm-node class.  Contains a pointer to the next level down.  Performs a cat operation.
 class csREQmNode : public csRENode
 {
   csRENode *child;
   
 public:
   csREQmNode(csRENode *c):child(c) {};
   virtual ~csREQmNode() {};
   
 public:
   virtual unsigned char Type()
   { return CSRE_QMNODE; }
 
   virtual csRENode *Left()
   { return child; }
   
   virtual csRENode *Right()
   { return NULL;  } 
 };
 
 /// The alt-node class.  Contains a pointer to the next level down.  Performs an Or operation.
 class csREAltNode : public csRENode
 {
   csRENode *left, *right;
   
 public:
   csREAltNode(csRENode *l, csRENode *r):left(l), right(r) {}
   virtual ~csREAltNode() {}
   
 public:
   virtual unsigned char Type()
   { return CSRE_ALTNODE; }
 
   virtual csRENode *Left()
   { return left; }
   
   virtual csRENode *Right()
   { return right;  } 
 };
 
 
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 //                  								Leaves
 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 /// The unsigned char leaf class.  Contains a unsigned character to match.
 class csRECharLeaf : public csRENode
 {
  int c;
   
 public:
   csRECharLeaf(int ch):c(ch) {};
   virtual ~csRECharLeaf() {};
   
 public:
   virtual unsigned char Type()
   { return CSRE_CHLEAF; }
 
   virtual csRENode *Left()
   { return NULL; }
   
   virtual csRENode *Right()
   { return NULL;  }
   
   /// Returns a true if this unsigned char matches the stored unsigned char.
   virtual bool Accept(int ch)
   { return c==ch; } 
 };
 
 /// The table leaf class.  Contains a set of unsigned characters to match.
 class csRETableLeaf : public csRENode
 {
  /// The set of unsigned characters that can be matched by this node.
  unsigned char *set;
  
  /// If true, then Accept returns true if the unsigned char is NOT in this table, otherwise returns true if unsigned char IS in the table.
  bool invert_match;
   
 public:
   csRETableLeaf(unsigned char *s, bool invert=false):invert_match(invert) 
   { set = (unsigned char *)strdup((char *)s);  };
   
   virtual ~csRETableLeaf() 
   { if (set) delete set; };
   
 public:
   virtual unsigned char Type()
   { return CSRE_TBLEAF; }
 
   virtual csRENode *Left()
   { return NULL; }
   
   virtual csRENode *Right()
   { return NULL;  }
   
   /// Returns a true if this unsigned char matches the stored unsigned char.
   virtual bool Accept(int ch)
   { 
     bool matched=false;
     
     unsigned char mc = (unsigned char)ch;
     unsigned char *p = set;
     
     // see if the unsigned char is in this set.
     while(*p)
     {
       if (*p == mc) 
       {
         matched=true;
         break;
       }
     }
     
     return (invert_match ? !matched : matched);
   
   } 
 };

#endif // __CS_CSRENODE_H__
