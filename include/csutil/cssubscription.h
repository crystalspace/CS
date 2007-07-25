/*
  Crystal Space 3D engine - Event Subscription internals
  Copyright (C) 2005 by Adam D. Bradley <artdodge@cs.bu.edu>

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

#ifndef __CS_CSEVENTSUBSCRIPTION_H__
#define __CS_CSEVENTSUBSCRIPTION_H__

#include "csutil/partialorder.h"
#include "csutil/tree.h"
#include "csutil/list.h"
#include "csutil/eventhandlers.h"
#include "iutil/eventh.h"

class csEventQueue;


/**
 * This class is used to represent the event namespace (tree).
 * Each node represents an event name (e.g., "crystalspace.input.mouse")
 * and contains two data structures: a partial order graph representing
 * subscribers to this event (including those who subscribed to parent
 * event names) along with their ordering constraints, and a queue of
 * subscribers representing a valid total order of that graph (used to
 * speed up the common case).
 */
class csEventTree : public csTreeNode
{
public:
  // forward declarator
  class SubscriberIterator;

  /**
   * Find a node with a given name in the event tree owned by q .
   * If no such node yet exists, create it (along with any necessary
   * parent nodes, including the name root).
   */
  csEventTree *FindNode (csEventID name, csEventQueue *q);
  /**
   * Subscribe a given handler to a given event name subtree via 
   * a given event queue.  This is wrapped by csEventQueue::Subscribe 
   * which may be easier to use in some situations.
   * \sa csEventQueue::Subscribe
   */
  bool Subscribe (csHandlerID, csEventID, csEventQueue *q);
  /**
   * Unubscribe a given handler to a given event name subtree via
   * a given event queue.  This is wrapped by csEventQueue::Unsubscribe
   * which may be easier to use in some situations.
   * Note that unsubscribing is reentrant (an event handler can
   * unsubscribe itself) but NOT thread-safe (only event handlers
   * in the same thread as the event queue can unsubscribe).
   * \sa csEventQueue::Unsubscribe
   */
  void Unsubscribe(csHandlerID, csEventID, csEventQueue *q);
  
  /**
   * Send the provided event to all subscribers, regardless of their
   * return values.
   */
  void Notify();
  /**
   * Send the provided event to all subscribers, using the normal
   * return and Broadcast rules.
   */
  void Dispatch(iEvent &e);
  
#ifdef ADB_DEBUG
  // Dump the event tree and subscriptions to stderr.
  void Dump();
#endif
  
  static inline csEventTree *CreateRootNode(csRef<iEventHandlerRegistry> &reg1,
					    csRef<iEventNameRegistry> &reg2, 
					    csEventQueue *q)
  {
    return new csEventTree (reg1, reg2, reg2->GetID(""), 0, q);
  }
  
  static inline void DeleteRootNode(csEventTree *node)
  {
    CS_ASSERT(node);
    CS_ASSERT(node->self == node->name_reg->GetID(""));
    delete node;
  }
    
private:
  csRef<iEventHandlerRegistry> handler_reg;
  csRef<iEventNameRegistry> name_reg;
#ifdef ADB_DEBUG
  void Dump(int depth);
#endif

  csEventTree *FindNodeInternal(csEventID &name, csEventQueue *q);
  
  /**
   * Use the static csEventTree::CreateRootNode() method to get a 
   * root node, and use (root_node)->FindNode() to fill out the
   * csEventTree.  It will handle all of the internals that you don't 
   * understand.
   */
  csEventTree (csRef<iEventHandlerRegistry> &,
	       csRef<iEventNameRegistry> &, 
	       csEventID name, csEventTree *parent, csEventQueue *q);
  ~csEventTree ();
  
  csEventID self;
  csEventQueue *queue;
  
  bool SubscribeInternal (csHandlerID, csEventID);
  void UnsubscribeInternal (csHandlerID);
  /**
   * We use copy-on-write (COW) semantics to avoid making an 
   * unnecessary number of copies of the PO graph and the
   * subscriber queue.  If this is true, then we are simply
   * using a reference to the parent's graph and SubscriberQueue;
   * if a change must be made locally, you need to invoke the
   * ForceFatCopy() method to break the connection.
   */
  bool fatNode;

  /**
   * The object of COW in the event tree.  Having a single consolidated
   * control record allows us to couple the management of the two
   * subscriber data structures (graph/queue), their regeneration, and
   * the queue iteration process.  This makes it a lot easier to catch
   * all of the nasty corner cases (particularly, reentrant Unsubscribe).
   */
  class FatRecordObject
  {
  private:
    csRef<iEventHandlerRegistry> handler_reg;
    csRef<iEventNameRegistry> name_reg;
  public:
    FatRecordObject(csEventTree *root,
		    csRef<iEventHandlerRegistry> &h_reg,
		    csRef<iEventNameRegistry> &n_reg,
		    csPartialOrder<csHandlerID> *new_sg,
		    csList<iEventHandler *> *new_sq) :
      handler_reg (h_reg), name_reg (n_reg), 
      SubscriberGraph (new_sg), SubscriberQueue (new_sq), 
      my_root (root), iterator (0), iterating_for (0)
    {
      /* If there's no SQ, mark it for creation */
      StaleSubscriberQueue = (SubscriberQueue==0);
    }

    ~FatRecordObject()
    {
      delete SubscriberGraph;
      if (SubscriberQueue)
	delete SubscriberQueue;
      CS_ASSERT (iterator == 0);
      CS_ASSERT (iterating_for == 0);
    }

    /**
     * Re-builds csEventTree::SubscriberQueue from csEventTree::SubscriberGraph.
     */
    void RebuildQueue();
    /**
     * Handle subscription nitty-gritty
     */
    csPartialOrder<csHandlerID> *SubscribeInternal(csHandlerID, csEventID);
    /**
     * Handle unsubscribe nitty-gritty
     */
    void UnsubscribeInternal(csHandlerID);

    /**
     * The current partial-order graph.
     */
    csPartialOrder<csHandlerID> *SubscriberGraph;
    /**
     * The (usually) current subscriber queue, derived from the PO graph
     * to improve common-case performance.
     */
    csList<iEventHandler *> *SubscriberQueue;
    /**
     * Flag set when the graph is manipulated without re-generating the
     * SubscriberQueue.  This allows us to handle the rare "adding a
     * subscriber to a current in-process event" case efficiently.
     */
    bool StaleSubscriberQueue;
    /**
     * The root of the event name subtree which this record represents
     */
    csEventTree *my_root;
    /**
     * When this is set, there is an iterator operating upon this particular
     * event subscription record.  Any subscription record may have at most 
     * one iterator at any given time.
     */
    SubscriberIterator *iterator;
    /**
     * This points to the particular node in the event name tree for which
     * the iterator is currently running.
     */
    csEventTree *iterating_for;
  };

  FatRecordObject *fatRecord;

  /**
   * Ensure we are using a local fat copy of the PO graph and
   * the delivery queue.  Also descends the tree so our children
   * aren't referencing our parents instead of us.
   * If we already are, this has no effect.
   */
  void ForceFatCopy();

  /**
   * Revert a fat node to a shallow node.
   * Used to throw away failed modifications.
   */
  void KillFatCopy();

  /**
   * Propagate current copy pointers down the tree, overwriting
   * existing shallow node pointers until other fat nodes are 
   * found.
   */
  void PushFatCopy(FatRecordObject *);
  

 public:
  /**
   * The SubscriberIterator is a wrapper for the messy internals
   * of figuring out which event handler to call next.  In the
   * common case, this is simply iterating over a pre-existing
   * list (csEventTree::SubscriberQueue).  There are degenerative
   * cases where it must fall back on progressively solving the 
   * partial order graph (csEventTree::SubscriberGraph).
   * 
   * Only one iterator may exist for a given event node at a time.
   */
  class SubscriberIterator 
  {
  public:
    /**
     * Constructor.  Establishes the csEventTree reference to this
     * iterator to ensure there can be only one.
     */
    SubscriberIterator (iEventHandlerRegistry* r, csEventTree *t, 
      csEventID bevent) : handler_reg(r), record(t->fatRecord), 
        baseevent(bevent), mode(SI_LIST), qit(0)
    {
      CS_ASSERT(record->iterator == 0);
      record->iterator = this;
      record->iterating_for = t;
      if (t->fatRecord->SubscriberQueue)
        qit = new csList<iEventHandler *>::Iterator (
          *t->fatRecord->SubscriberQueue);
      else
        GraphMode();
    }

    /**
     * Destructor.  Remove the csEventTree reference to us.
     */
    ~SubscriberIterator ()
    {
      CS_ASSERT(record->iterator == this);
      record->iterator = 0;
      record->iterating_for = 0;
      delete qit;
    }

    /// Test if there is another available handler
    inline bool HasNext () 
    {
      switch(mode) 
      {
      case SI_LIST:
        return qit->HasNext ();
        
      case SI_GRAPH:
        do 
        {
          csHandlerID id = record->SubscriberGraph->GetEnabled (
	  	CS_HANDLER_INVALID);
          if (id == CS_HANDLER_INVALID)
            break;
          else if (handler_reg->IsInstance(id))
            return true;
          else
            record->SubscriberGraph->Mark(id);
        } 
        while (true);
        return false;
        
      default:
        CS_ASSERT((mode == SI_LIST) ||
                  (mode == SI_GRAPH));
        return false;
      }
    }

    /// Return an available handler and mark it as "done"
    inline iEventHandler *Next () 
    {
      switch(mode) 
      {
      case SI_LIST:
	/* DOME : see if the current element has been deleted. */
        return qit->Next ();
        
      case SI_GRAPH:
	/* see if the current element has been flagged for deletion. */
        do 
        {
          csHandlerID id = record->SubscriberGraph->GetEnabled (
	  	CS_HANDLER_INVALID);
          if (id == CS_HANDLER_INVALID)
            break;
          else if (handler_reg->IsInstance (id)) 
          {
            record->SubscriberGraph->Mark (id);
            return handler_reg->GetHandler (id);
          } 
          else
            record->SubscriberGraph->Mark(id);
        } 
        while (true);
        return 0;
        
      default:
        CS_ASSERT((mode == SI_LIST) ||
                  (mode == SI_GRAPH));
        return 0;
      }
    }
    
    /* SI_GRAPH mode data structures and methods */
    void GraphMode (); // change to graph (SI_GRAPH) iterator mode.
    
  private:
    friend class csEventTree;
    friend class csEventQueueTest;

    csRef<iEventHandlerRegistry> handler_reg;
    FatRecordObject *record;
    csEventID baseevent;
    enum
    {
      SI_LIST,
      SI_GRAPH
    } mode;
    
    /* SI_LIST mode data structures */
    csList<iEventHandler *>::Iterator* qit;
  };
  friend class SubscriberIterator;
  friend class csEventQueueTest;

  /**
   * Return a csEventTree::SubscriberIterator for all subscribers
   * to this event name (and to its parents).
   */
  SubscriberIterator *GetIterator();

};

#endif // __CS_CSEVENTSUBSCRIPTION_H__
