/*
  Crystal Space Windowing System - Event subscription internals
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

#include "cssysdef.h"
#include "csutil/csevent.h"
#include "csutil/cssubscription.h"
#include "csutil/eventnames.h"
#include "iutil/eventh.h"
#include "csutil/cseventq.h"
#include "csutil/partialorder.h"
#include "ivaria/reporter.h"

#ifdef ADB_DEBUG
#include <iostream>
#endif

csEventTree::csEventTree (csRef<iEventHandlerRegistry> &h_reg,
			  csRef<iEventNameRegistry> &n_reg, 
			  csEventID name, csEventTree *_parent, csEventQueue *q) :
  csTreeNode (_parent), 
  handler_reg (h_reg), name_reg (n_reg), 
  self (name), queue (q)
{
  CS_ASSERT(name != CS_EVENT_INVALID);
  if (parent) 
  {
    fatRecord = ((csEventTree *)parent)->fatRecord; // I wish csEventTree was polymorphic.
    fatNode = false;
  } 
  else 
  {
    /* this is the root node.
     * create the root PO and Queue so everyone else can COW it. */
    fatRecord = new FatRecordObject(this, handler_reg, name_reg, 
				    new csPartialOrder<csHandlerID>, 0);
    fatNode = true;
  }
  queue->EventHash.PutUnique(name, this);
}

csEventTree::~csEventTree() 
{
  queue->EventHash.DeleteAll(self);
  if (fatNode) delete fatRecord;
}

/**
 * Use head-recursion to find the path.
 */
csEventTree *csEventTree::FindNode(csEventID name, csEventQueue *q)
{
  CS_ASSERT(name != CS_EVENT_INVALID);
  csEventTree *res;
  if ((res=q->EventHash.Get(name, 0))!=0) 
  { /* shortcut */
    return res;
  }
  if (!q->EventTree) 
  {
    csEventID root = csevAllEvents(name_reg);
    q->EventTree = new csEventTree(handler_reg, name_reg, root, 0, q);
    q->EventHash.PutUnique(root, q->EventTree);
  }
  return q->EventTree->FindNodeInternal(name, q);
}

csEventTree *csEventTree::FindNodeInternal(csEventID &name, csEventQueue *q)
{
  CS_ASSERT(name != CS_EVENT_INVALID);
  if (name == self)
    return this;
  else 
  {
    csEventID parentID = name_reg->GetParentID(name);
    CS_ASSERT(parentID != CS_EVENT_INVALID);
    csEventTree *wrk_parent = FindNodeInternal(parentID, q);
    for (size_t iter=0 ; iter<wrk_parent->children.GetSize () ; iter++) 
    {
      csEventTree* child = (static_cast<csEventTree *> (wrk_parent->children[iter]));
      if (child->self == name)
	return child;
    }
    csEventTree *added = new csEventTree(handler_reg, name_reg, name,
    	wrk_parent, q);
    return added;
  }
}

void csEventTree::PushFatCopy (FatRecordObject *r)
{
  if (!fatNode) 
  {
    fatRecord = r;
    for (size_t i=0 ; i<children.GetSize () ; i++) 
    {
      ((csEventTree *)children[i])->PushFatCopy(r);
    }
  }
}

void csEventTree::ForceFatCopy ()
{
  if (!fatNode)
  {
#ifdef ADB_DEBUG
    std::cerr << "  ... forcing fat copy here ..." << std::endl;
#endif

    /**
     * If there is an iterator in play for the current node's fatRecord
     * AND it is for one of the current node's children
     */
    if ((fatRecord->iterator != 0)
	&& name_reg->IsKindOf(fatRecord->iterator->baseevent, self)) 
    {
      // we MUST NOT get here for a record that is already fat
      CS_ASSERT (self != fatRecord->iterator->baseevent);
      /**
       * This is a messy situation.  Say a handler subscribes to:
       *     cs.input
       * And say an event gets delivered:
       *     cs.input.keyboard.down
       * And say that the handler creates a new handler which subscribes to:
       *     cs.input.keyboard
       *
       * Then the fatRecord belongs to cs.input, but the iterator is for
       * cs.input.keybaord.down, but we need to create a new fatRecord 
       * belonging to cs.input.keyboard (and all of its children) while
       * not perturbing the iteration over the subscribers to 
       * cs.input.keyboard.down.
       *
       * Ordinarily (see the "else" below) we would just create a new
       * fatRecord at the subscribe point (cs.input.keyboard) and
       * propogate that to its children.  However, since a child is
       * in use with an iterator intimately intertwined with the 
       * fatRecord, that's no good.
       * 
       * Instead of creating a new fatRecord at cs.input.keybaord,
       * move the root of the existing fatRecord to cs.input.keyboard
       * and create a new fatRecord at cs.input and all of its
       * children (EXCEPT cs.input.keyboard), which is done by setting
       * this.fatNode to true, setting the old root's fatRecord pointer
       * to point to the new FRO, and calling the old root's PushFatCopy 
       * method.
       */

      csEventTree *old_FR_root = fatRecord->my_root;

      CS_ASSERT (this != old_FR_root);

      FatRecordObject *newFatRecord = new FatRecordObject
	(fatRecord->my_root, 
	 handler_reg, name_reg,
	 new csPartialOrder<csHandlerID>(fatRecord->SubscriberGraph),
	 (fatRecord->SubscriberQueue?
	  new csList<iEventHandler *>(*fatRecord->SubscriberQueue):
	  0));

      fatRecord->my_root = this;
      old_FR_root->fatRecord = newFatRecord;
      fatNode = true;
      old_FR_root->PushFatCopy(newFatRecord);

      CS_ASSERT (name_reg->IsKindOf(fatRecord->iterator->baseevent, self));
      CS_ASSERT (fatRecord != old_FR_root->fatRecord);
    } 
    else 
    {
      FatRecordObject *newFatRecord = new FatRecordObject
	(this,
	 handler_reg, name_reg,
	 new csPartialOrder<csHandlerID>(fatRecord->SubscriberGraph),
	 (fatRecord->SubscriberQueue?
	  new csList<iEventHandler *>(*fatRecord->SubscriberQueue):
	  0));
      
      newFatRecord->StaleSubscriberQueue = true;
      
      // There is no iterator, or it belongs to the parent.  Ignore it.
      newFatRecord->iterator = 0;
      newFatRecord->iterating_for = 0;
      PushFatCopy(newFatRecord);
      fatNode = true;
    }
  }
}

void csEventTree::KillFatCopy()
{
  CS_ASSERT(fatRecord->iterator == 0);
  CS_ASSERT(fatRecord->iterating_for == 0);
  if (fatNode)
  {
    delete fatRecord;
    PushFatCopy(((csEventTree*)parent)->fatRecord);
    fatNode = false;
  }
}

void csEventTree::FatRecordObject::RebuildQueue()
{
  csList<const csHandlerID> hlist;
  SubscriberGraph->Solve(hlist);
  csList<const csHandlerID>::Iterator it(hlist);
  if (SubscriberQueue)
    SubscriberQueue->DeleteAll();
  else
    SubscriberQueue = new csList<iEventHandler*>;
  while (it.HasNext()) 
  {
    csHandlerID h = it.Next();
    if (handler_reg->IsInstance(h))
      SubscriberQueue->PushBack(handler_reg->GetHandler(h));
  }
  StaleSubscriberQueue = false;
}

csPartialOrder<csHandlerID> *csEventTree::FatRecordObject::SubscribeInternal(
	csHandlerID id, csEventID baseevent)
{
  csPartialOrder<csHandlerID> *NewSubscriberGraph;
  iEventHandler *handler = handler_reg->GetHandler(id);
  /* 
   * Build a copy of the graph and see if the new subscription "fits".
   * If it does, and if this also succeeds for all of the children,
   * we will replace the current graph with the new one below.
   * If not, we simply discard the new graph instead of trying to
   * mess with "backing out" our changes.
   *
   * We will solve the PO below once we know our children have
   * succeeded.  This conserves work - no delivery_queue is rebuilt
   * until we know that the subscription succeeds completely. 
   *
   * Also, if the current event name is an in-delivery event name,
   * we switch the event queue iterator into graph-solver mode,
   * and we do not regenerate the subscriber queue until the iterator
   * has finished its pass.
   */
  NewSubscriberGraph = new csPartialOrder<csHandlerID> (SubscriberGraph);
  
  CS_ASSERT (id != CS_HANDLER_INVALID);
  NewSubscriberGraph->Add (id); /* ensure this node is present */
  
  /* id is an actual event handler, so we must sandwich it between
   * the abstract handler name's magic ":pre" and ":post" instances
   * to make sure abstract ordering works. */
  
  csHandlerID preBound = handler_reg->GetGenericPreBoundID (id);
  csHandlerID postBound = handler_reg->GetGenericPostBoundID (id);

  CS_ASSERT (preBound != CS_HANDLER_INVALID);
  NewSubscriberGraph->Add (preBound);
  if (!NewSubscriberGraph->AddOrder (preBound, id)) 
    goto fail; /* This edge introduced a cycle... possible if generic(id) 
		* is already ruled out by conflicting generic constraints. */

#ifdef ADB_DEBUG
  std::cerr << "About to add to graph:" << std::endl;
#endif

  CS_ASSERT (postBound != CS_HANDLER_INVALID);
  NewSubscriberGraph->Add (postBound);
  if (!NewSubscriberGraph->AddOrder (id, postBound))
    goto fail; /* Ditto */

  // Make sure every ":pre" or ":post" that was implicitly created has a 
  // "pre -> post" rule as well.
  for (size_t i=0 ; i<NewSubscriberGraph->Size() ; i++)
  {
    csHandlerID Idx = NewSubscriberGraph->GetByIndex (i);
    csString HandlerName = handler_reg->GetString(Idx);
    if (HandlerName.Find(":pre") != (size_t)-1)
    {
      HandlerName.ReplaceAll (":pre", ":post");
      csHandlerID PostIdx = handler_reg->GetID(HandlerName);
      NewSubscriberGraph->Add (PostIdx); // Makes sure it's present
      if (!NewSubscriberGraph->Contains (Idx, PostIdx)) {
	NewSubscriberGraph->AddOrder (Idx, PostIdx);
      }
    }
    else if (HandlerName.Find(":post") != (size_t)-1)
    {
      HandlerName.ReplaceAll (":post", ":pre");
      csHandlerID PreIdx = handler_reg->GetID(HandlerName);
      NewSubscriberGraph->Add (PreIdx); // Makes sure it's present
      if (!NewSubscriberGraph->Contains (PreIdx, Idx)) {
	NewSubscriberGraph->AddOrder (PreIdx, Idx);
      }
    }
  }

  if (!NewSubscriberGraph->AddOrder(preBound, postBound))
    goto fail;

  /* If this is an in-delivery event name, then if the event is
     still eligible to execute (subject to all constraints) 
     we will let it, otherwise we mark it, so it will not get 
     executed until a new instance of the event gets dispatched. 
     Note that this is not deterministic unless your order is
     specifically with request to the currently in-service handler,
     since PO solutions are not fully deterministic. */
  if ((iterator) && (NewSubscriberGraph->IsMarked (postBound)))
    NewSubscriberGraph->Mark (id);

  do
  {
    const csHandlerID *precs = handler->InstancePrec (handler_reg, name_reg,
    	baseevent);
    if (precs != 0)
    {
      for (size_t i=0 ; precs[i]!=CS_HANDLERLIST_END ; i++)
      {
	csHandlerID prec = precs[i];
	/* This is a little subtle.  If the predecessor ID is a generic
	 * handler name, we actually want to mark that ID's ":post" 
	 * magic instance as our predecessor; since all instances 
	 * will be predecessors of this (see above), we get the desired 
	 * effect. */
	if (!handler_reg->IsInstance (prec))
	{
	  prec = handler_reg->GetGenericPostBoundID (prec);
	}

	NewSubscriberGraph->Add (prec);
	if (!NewSubscriberGraph->AddOrder (prec, id))
	  goto fail; /* This edge introduced a cycle */
      }
    }
  } while (0);
  
  do
  {
    const csHandlerID *succs = handler->InstanceSucc (handler_reg, name_reg,
    	baseevent);
    if (succs != 0)
    {
      for (size_t i=0 ; succs[i]!=CS_HANDLERLIST_END ; i++)
      {
	csHandlerID succ = succs[i];
	/* Same rationale as above. */
	if (!handler_reg->IsInstance (succ))
	  succ = handler_reg->GetGenericPreBoundID (succ);
	NewSubscriberGraph->Add (succ);
	if (iterator && (NewSubscriberGraph->IsMarked (succ))) 
	{
	  /* See the above comment about in-delivery event names */
	  NewSubscriberGraph->Mark (id);
	  break;
	}
	if (!NewSubscriberGraph->AddOrder (id, succ))
	  goto fail; /* This edge introduced a cycle */
      }
    }
  } while (0);
  
#ifdef ADB_DEBUG
  std::cerr << "Done adding." << std::endl << std::endl << std::endl;
#endif

  return NewSubscriberGraph;
  
 fail:
  if (NewSubscriberGraph)
    delete NewSubscriberGraph;
#ifdef ADB_DEBUG
  std::cerr << "Failing." << std::endl << std::endl << std::endl;
#endif
  return 0;
}

bool csEventTree::SubscribeInternal (csHandlerID id, csEventID baseevent)
{
  CS_ASSERT(baseevent != CS_EVENT_INVALID);
  CS_ASSERT(id != CS_HANDLER_INVALID);
  csPartialOrder<csHandlerID> *NewSubscriberGraph = 
    fatNode ? fatRecord->SubscribeInternal(id, baseevent) : 0;

  /**
   * Descend the event name tree, looking for fat copies needing subscriptions.
   * Note that, in the common case, this will amount to a no-op true.
   * The scheme only becomes expensive when there are subscribers at a lot of
   * subtrees of the graft point (i.e., a lot of fat copies).
   */
  for (size_t i=0 ; i<children.GetSize () ; i++) 
  {
    if (!((csEventTree *)children[i])->SubscribeInternal (id, baseevent))
      goto fail;
  }

  // Success!  Push the changes into the live event tree.
  if (NewSubscriberGraph) 
  {
    // Install new partial order
    *fatRecord->SubscriberGraph = *NewSubscriberGraph;
    delete NewSubscriberGraph;

    fatRecord->StaleSubscriberQueue = true;

    if (fatRecord->iterator) 
    {
      fatRecord->iterator->GraphMode();
    } 

    /* We do not re-build the queue now.  Instead, we do it on-demand,
     * in the hopes of "batching up" subscribes and unsubscribes. */
  }

  return true;

 fail: 
  /* We either failed locally or on a subtree.  
   * Don't change the live tree. */
  if (NewSubscriberGraph)
    delete NewSubscriberGraph;
  return false;
}

/**
 * If we are in graph mode, this is done automagically for us; the PO
 * implementation doesn't use pointer references, so simply removing a node
 * (whether marked or not) doesn't endanger us.  We can't delete nodes out 
 * from under a SubscriberQueue iterator, so we need to switch over to 
 * graph solver mode in such cases and flag the SQ for regeneration.
 */
void csEventTree::FatRecordObject::UnsubscribeInternal(csHandlerID id)
{
  /* It is possible we've been called for a "universal unsubscribe"
   * (baseevent==CS_EVENT_INVALID), so it could be there's nothing
   * to remove from this event node because the handler didn't
   * subscribe here. */
  if (SubscriberGraph->Contains(id)) 
  {
    /* We may waste a bit of effort going to graph-solver mode, but
     * it greatly simplifies the bookkeeping for dealing with all sorts of
     * corner cases (esp. deleting a subscription that is currently 
     * in-process).
     */
    if (iterator)
    {
      iterator->GraphMode();
    }

    /* Automagically removes all of the edges for us.
     * Dangling edges are a non-issue, since they go away
     * and their deleted endpoint will never appear again
     * (instance IDs are non-repeating). */
    SubscriberGraph->Delete(id);

    // Flag SQ for regeneration
    StaleSubscriberQueue = true;
  }
}

/*
 * This has to be "reentrant", by which we simply mean it must be safe for
 * an event handler to un-subscribe itself while it is being delivered to.
 */
void csEventTree::UnsubscribeInternal(csHandlerID id)
{
  CS_ASSERT(id != CS_HANDLER_INVALID);
  if (fatNode)
  {
    fatRecord->UnsubscribeInternal(id);
  }

  /* Since we manipulate fatRecord in place, we only really need
   * to propagate to (fatNode) children.  
   * Descend recursively and look for them. */

  for (size_t i=0 ; i<children.GetSize () ; i++) 
  {
    ((csEventTree *)children[i])->UnsubscribeInternal(id);
  }
}

bool csEventTree::Subscribe (csHandlerID id, csEventID event, csEventQueue *q)
{
#ifdef ADB_DEBUG
  std::cerr << __func__ << " : " 
	    << handler_reg->GetString(id)
	    << "/" << name_reg->GetString(event) << std::endl;
#endif

  CS_ASSERT(id != CS_HANDLER_INVALID);
  CS_ASSERT(event != CS_EVENT_INVALID);
  csEventTree *w = FindNode(event, q);
  bool wasFat = w->fatNode;
  w->ForceFatCopy ();
  if (!w->SubscribeInternal (id, event)) 
  {
    if (!wasFat) {
#ifdef ADB_DEBUG
      std::cerr << __func__ << " (killing fat copy)" << std::endl;
#endif
      w->KillFatCopy();
    }
#ifdef ADB_DEBUG
    std::cerr << __func__ << " FAILED" << std::endl;
#endif
    return false;
  } 
  else 
  {
#ifdef ADB_DEBUG
    std::cerr << __func__ << " SUCCEEDED" << std::endl;
#endif
    return true;
  }
}

void csEventTree::Unsubscribe(csHandlerID id, csEventID event, csEventQueue *q)
{
  CS_ASSERT(id != CS_HANDLER_INVALID);
  if (event == CS_EVENT_INVALID)
    q->EventTree->UnsubscribeInternal(id);
  else 
  {
    csEventTree *w = FindNode(event, q);
    w->ForceFatCopy ();
    w->UnsubscribeInternal (id);
    // TODO : test if UnsubscribeInternal fails (i.e., is a no-op); 
    // if it turns out we created a fat copy unnecessarily, kill it.
  }
}

void csEventTree::Notify ()
{
  csRef<iEvent> e(queue->CreateEvent(self));
  e->Broadcast = true;
  Dispatch(*e);
}

void csEventTree::Dispatch (iEvent &e)
{
  CS_ASSERT(e.Name == self);

  if (fatRecord->StaleSubscriberQueue)
    fatRecord->RebuildQueue();

  /* automatic variable will be destroyed for us... */
  SubscriberIterator it (handler_reg, this, e.Name); 

#ifdef ADB_DEBUG
  std::cerr << "DISPATCH [" 
	    << name_reg->GetString(e.Name) 
	    << "] on event node "
	    << name_reg->GetString(self)
	    << std::endl;
#endif

  while (it.HasNext()) 
  {
    iEventHandler *h = it.Next();
    CS_ASSERT(h != 0);
#ifdef ADB_DEBUG
    std::cerr << " -- dispatching to "
	      << h->GenericName()
	      << std::endl;
#endif
    if (h->HandleEvent(e) && (!e.Broadcast))
      break;
  }
#ifdef ADB_DEBUG
  if (it.HasNext())
  {
    std::cerr << "  SKIPPING:" << std::endl;
    do
    {
      std::cerr << "    " << it.Next()->GenericName() << std::endl;
    } while (it.HasNext());
  }
  std::cerr << "END DISPATCH"
	    << std::endl;
#endif
}






void csEventTree::SubscriberIterator::GraphMode()
{
  if (mode==SI_GRAPH)
    return;
  CS_ASSERT(mode==SI_LIST);

  record->SubscriberGraph->ClearMark();

  if (record->SubscriberQueue)
  {
    csList<iEventHandler *>::Iterator zit(*record->SubscriberQueue);
    while (zit.HasNext()) 
    {
      /**
       * This loop marks every node in the graph that the queue iterator (qit)
       * has already visited.  From here forward, we do everything on the graph
       * itself and do not look at the SubscriberQueue.
       */
      iEventHandler *h = zit.Next();
      csHandlerID hid = handler_reg->GetID(h);
      record->SubscriberGraph->Mark(hid);
      if (qit && (h == qit->FetchCurrent()))
        break;
    }
  }

  mode = SI_GRAPH;
}




#ifdef ADB_DEBUG

void Indent (int n)
{
  for (int i=0 ; i<n ; i++)
  {
    std::cerr << "  ";
  }
}

void csEventTree::Dump ()
{
  std::cerr << "-------------------------------------------------------------" 
	    << std::endl << "Event tree:" << std::endl;
  queue->EventTree->Dump (0);
  std::cerr << "-------------------------------------------------------------"
	    << std::endl;
}

void csEventTree::Dump (int depth)
{
  Indent(depth);
  std::cerr << "Node: [" << name_reg->GetString(self) << "]" << std::endl;
  if ((fatRecord->iterator) && (fatRecord->iterator->mode != SubscriberIterator::SI_LIST)) 
  {
    Indent(depth+3);
    std::cerr << "NOTE - Node is in graph iterator mode, this is probably wrong" << std::endl;
  }

  if (fatRecord->SubscriberGraph && (fatRecord->SubscriberGraph->Size()>0)) {
    Indent(depth+1);
    std::cerr << "Subscribers:" << std::endl;
    for (size_t i=0 ; i<fatRecord->SubscriberGraph->Size() ; i++) {
      csHandlerID hid = fatRecord->SubscriberGraph->GetByIndex(i);
      iEventHandler *h = handler_reg->IsInstance(hid) ?
	handler_reg->GetHandler(hid) : 0;
      std::cerr << "[" << handler_reg->GetString(hid) << "] <"
		<< std::hex << ((unsigned long) h) << std::dec << ">"
		<< std::endl;
    }
  } else {
    Indent(depth+1);
    std::cerr << "No subscribers" << std::endl;
  }

  for (size_t i=0 ; i<children.GetSize () ; i++) 
  {
    ((csEventTree *) children[i])->Dump(depth+1);
  }
}

#endif
