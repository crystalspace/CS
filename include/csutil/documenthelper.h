/*
  Copyright (C) 2005 by Marten Svanfeldt

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

#ifndef __CSUTIL_DOCUMENTHELPER_H__
#define __CSUTIL_DOCUMENTHELPER_H__

/**\file
 * Helper functions and classes which operate on iDocumentNode and 
 * iDocumentNodeIterator.
 */

#include "csutil/refarr.h"
#include "csutil/regexp.h"
#include "csutil/scf_implementation.h"

#include "iutil/document.h"

namespace CrystalSpace
{
  /**
   * Namespace contains helper functions and classes which operate on
   * iDocumentNode and iDocumentNodeIterator.
   */
  namespace DocumentHelper
  {
    namespace Implementation
    {
      /**
      * Filtering iDocumentNodeIterator.
      * Filters another iterator with a functor.
      */
      template<class T>
      class FilterDocumentNodeIterator : public 
        scfImplementation1 <FilterDocumentNodeIterator<T>, 
	  iDocumentNodeIterator>
      {
      public:
        FilterDocumentNodeIterator (csRef<iDocumentNodeIterator> parent,
          T filter) : scfImplementation1<FilterDocumentNodeIterator<T>, 
	    iDocumentNodeIterator> (this), parent (parent), filter (filter)
        {
          ForwardIterator ();
        }

        // -- iDocumentNodeIterator
        /// Are there more elements?
        virtual bool HasNext ()
        {
          return nextElement.IsValid ();
        }

        /// Get next element.
        virtual csRef<iDocumentNode> Next ()
        {
          csRef<iDocumentNode> current = nextElement;
          ForwardIterator ();
          return current;
        }

      private:
        void ForwardIterator ()
        {
          if (!parent) nextElement = 0;

          while (parent->HasNext ())
          {
            csRef<iDocumentNode> parentNext = parent->Next ();
            if (filter (parentNext))
            {
              nextElement = parentNext;
              return;
            }
          }
          nextElement = 0;
          parent = 0;
        }

        csRef<iDocumentNodeIterator> parent;
        T filter;
        csRef<iDocumentNode> nextElement;
      };
    }
    
    /**
     * Remove duplicate child-nodes.
     * The functor T is used to determine what should be seen
     * as equal nodes.
     * This is potentially an O(n^2) operation!
     */
    template<class T>
    void RemoveDuplicateChildren (iDocumentNode *rootNode, T eq)
    {
      csRef<iDocumentNodeIterator> it = rootNode->GetNodes ();
      RemoveDuplicateChildren (rootNode, it, eq);
    }

    /**
     * Remove duplicate child-nodes.
     * The functor T is used to determine what should be seen
     * as equal nodes.
     * This is potentially an O(n^2) operation!
     */
    template<class T>
    void RemoveDuplicateChildren (iDocumentNode *rootNode,
      csRef<iDocumentNodeIterator> childIt, T eq)
    {
      typedef csRefArray<iDocumentNode> NodeListType;
      NodeListType nodesToRemove;
      NodeListType nodesToKeep;

      if (!childIt) return;

      while (childIt->HasNext ())
      {
        csRef<iDocumentNode> node = childIt->Next ();
        //compare it to those we already have
        bool keep = true;

        NodeListType::Iterator it = nodesToKeep.GetIterator ();
        while (it.HasNext ())
        {
          csRef<iDocumentNode> keepNode = it.Next ();
          if (keepNode->Equals (node))
          {
            keep = false; 
            break;
          }
          if (eq (node, keepNode))
          {
            keep = false;
            break;
          }
        }

        if (keep)
        {
          nodesToKeep.Push (node);
        }
        else
        {
          nodesToRemove.Push (node);
        }
      }

      while (nodesToRemove.GetSize ())
      {
        csRef<iDocumentNode> node = nodesToRemove.Pop ();
        rootNode->RemoveNode (node);
      }
    }

    /**\name Functors 
     * @{ */

    /**
     * Node comparator. Compares the names of the nodes (case-insensitive).
     */
    struct NodeNameCompare
    {
      bool operator () (iDocumentNode *node1, iDocumentNode *node2) const
      {
        if (node1->GetType () != CS_NODE_ELEMENT) return false;
        if (node2->GetType () != CS_NODE_ELEMENT) return false;

        const char* name1 = node1->GetValue ();
        const char* name2 = node2->GetValue ();
        if (!csStrCaseCmp (name1, name2)) return true;
        return false;
      }
    };

    /**
     * Node comparator. Compares a given attribute between two nodes 
     * (case-insensitive).
     */
    struct NodeAttributeCompare
    {
      NodeAttributeCompare (const char* attributeName)
        : attributeName (attributeName)
      {
      }

      bool operator () (iDocumentNode *node1, iDocumentNode *node2) const
      {
        if (node1->GetType () != CS_NODE_ELEMENT) return false;
        if (node2->GetType () != CS_NODE_ELEMENT) return false;

        csRef<iDocumentAttribute> attribute1 = 
          node1->GetAttribute (attributeName.GetData ());
        csRef<iDocumentAttribute> attribute2 = 
          node2->GetAttribute (attributeName.GetData ());
        if (!attribute1 || !attribute2) return false;

        if (!csStrCaseCmp (attribute1->GetValue (), attribute2->GetValue ())) 
          return true;

        return false;
      }
    private:
      csString attributeName;
    };

    /**
     * Compare (case-sensitive) node value to given.
     */
    struct NodeValueTest
    {
      NodeValueTest (const char* value)
        : value (value)
      {}

      bool operator () (iDocumentNode *node)
      {
        if (!node) return false;

        const char *nodeValue = node->GetValue ();
        return (value == nodeValue);
      }

    private:
      csString value;
    };

    /**
     * Compare (case-sensitive) node attribute to given.
     */
    struct NodeAttributeValueTest
    {
      NodeAttributeValueTest (const char *attribute, const char* value)
        : attribute (attribute), value (value)
      {}

      bool operator () (iDocumentNode *node)
      {
        if (!node) return false;

        const char* attributeValue = node->GetAttributeValue (attribute.GetData ());

        return (value == attributeValue);
      }

    private:
      csString attribute;
      csString value;
    };

    /**
     * Check if a regular expression matches(case-insensitive) with the value 
     * of the given attribute.
     */
    struct NodeAttributeRegexpTest
    {
      NodeAttributeRegexpTest (const char *attribute, const char* regexp)
        : attribute (attribute), valueMatcher (regexp)
      {
      }

      bool operator () (iDocumentNode *node)
      {
        if (!node) return false;

        const char* attributeValue = node->GetAttributeValue (attribute.GetData ());

        return (valueMatcher.Match (attributeValue, csrxIgnoreCase) == csrxNoError);
      }

    private:
      csString attribute;
      csRegExpMatcher valueMatcher;
    };
    /** @} */

    /** 
     * Get a filtering iDocumentNodeIterator
     * Example usage: 
     * \code
     * DocumentHelper::NodeAttributeValueTest test ("name", "Marten");
     * csRef<iDocumentNodeIterator> it = 
     *   DocumentHelper::FilterDocumentNodeIterator (node->GetNodes(), test);
     * while (it->HasNext ())
     * { ... }
     * \endcode
     */
    template<class T>
    csPtr<iDocumentNodeIterator> FilterDocumentNodeIterator(
      csRef<iDocumentNodeIterator> parent, T filter)
    {
      return new Implementation::FilterDocumentNodeIterator<T>
        (parent, filter);
    }
  }
}

#endif
