/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_IUTIL_EVENTH_H__
#define __CS_IUTIL_EVENTH_H__

/**\file
 * Event handler interface
 */
/**
 * \addtogroup event_handling
 * @{ */

#include "csutil/scf_interface.h"
#include "iutil/eventnames.h"
#include "iutil/eventhandlers.h"

struct iEvent;


/**
 * This interface describes an entity that can receive events.
 */
struct iEventHandler : public virtual iBase
{
  SCF_INTERFACE(iEventHandler, 2,0,0);
  /**
   * This is the basic event handling function.  To receive events, a component
   * must implement iEventHandler and register with an event queue using
   * iEventQueue::RegisterListener() and iEventQueue::Subscribe().
   * The event handler should return true if the event was handled.  Returning 
   * true prevents the event from being passed along to other event handlers
   * (unless the event's Broadcast flag has been set, in which case it is sent
   * to all subscribers regardless of the return value).  If the event was not
   * handled, then false should be returned, in which case other event handlers 
   * are given a shot at the event.  Do \b not return true unless you really 
   * handled the event and want event dispatch to stop at your handler.
   */
  virtual bool HandleEvent (iEvent&) = 0;

  /**
   * This function returns a string which "names" this event handler generically
   * (i.e., it identifies all instances of this event handler as a group).
   * For example, the core application logic would be "application", a 
   * window system plugin would be "crystalspace.windowsystem", etc.
   * This is used, in combination with the GenericPrec, GenericSucc,
   * InstancePrec, and InstanceSucc functions, by the subscription 
   * scheduler to establish the order in which event handlers are to be called.
   *
   * Too bad C++ doesn't allow virtual static functions, because this would be one.
   * To make up for this, it is conventional to also define a static method
   * StaticHandlerName() which can be used to reference a class of event handlers
   * abstractly without it having been instantiated, e.g.,
   * csBaseEventHandler::StaticHandlerName().
   *
   * The csEventHandlerRegistry also uses this method to construct a unique instance 
   * name for each iEventHandler.  
   *
   * Usually, you will want to use the CS_EVENTHANDLER_NAMES macro instead of
   * defining this yourself.
   * \sa csHandlerRegistry::GetID
   * \sa csHandlerRegistry::ReleaseID
   * \sa CS_EVENTHANDLER_NAMES
   */
  virtual const char * GenericName() const = 0; /* really is "static" */

  /**
   * This function returns a csHandlerID corresponding with GenericName,
   * i.e., it should always return 
   * csHandlerRegistry::GetGenericID (this->GenericName()).
   * Normally, it will actually wrap a static method StaticID() which can be 
   * used to reference a class of event handlers abstractly without it having 
   * been instantiated. Usually, you will want to use the 
   * CS_EVENTHANDLER_NAMES macro instead of defining this yourself.
   * \sa iEventHandler::GenericName
   * \sa CS_EVENTHANDLER_NAMES
   */
  virtual csHandlerID GenericID (
    csRef<iEventHandlerRegistry> &) const = 0; 
  // wish the above could be "virtual static"

  /**
   * This function takes a csEventID as an argument and returns an array of 
   * csHandlerIDs identifying those event handlers which must, for the given
   * event, only be called before this one (if they have been instantiated).
   * Should only return generic identifiers, not instance identifiers; in 
   * other words, every member of the array should be the result of a call to
   * csHandlerRegistry::GetGenericID("name"), where "name" may be some
   * class's static GenericName() function or a literal string.
   *
   * This should also be a "virtual static" function, but C++ doesn't have them.
   */
  virtual const csHandlerID * GenericPrec (
    csRef<iEventHandlerRegistry> &, csRef<iEventNameRegistry> &, 
    csEventID) const = 0;

  /**
   * This function takes a csEventID as an argument and returns an array of
   * csHandlerIDs identifying those event handlers which must, for the given
   * event, only be called after this one (if they have been instantiated).
   * Should only return generic identifiers, not instance identifiers; in
   * other words, every member of the array should be the result of a call to
   * csHandlerRegistry::GetGenericID("name"), where "name" may be some
   * class's static GenericString() function or a literal string.
   *
   * This should also be a "virtual static" function, but C++ doesn't have them.
   */
  virtual const csHandlerID * GenericSucc (
    csRef<iEventHandlerRegistry> &, csRef<iEventNameRegistry> &,
    csEventID) const = 0;

  /**
   * This function takes a csEventID as an argument and returns an array of
   * csHandlerIDs identifying those event handlers which must, for the given
   * event, only be called before this one.  May include both generic
   * and instance identifiers, i.e., the results of both
   * csHandlerRegistry::GetGenericID() and csHandlerRegistry::GetID() calls.
   *
   * If the instance constraints are the same as the generic ones, use
   * the CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS macro instead of
   * defining this for yourself.
   * \sa CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS
   */
  virtual const csHandlerID * InstancePrec (
    csRef<iEventHandlerRegistry> &, csRef<iEventNameRegistry> &,
    csEventID) const = 0;

  /**
   * This function takes a csEventID as an argument and returns an array of
   * csHandlerIDs identifying those event handlers which must, for the given
   * event, only be called after this one.  May include both generic
   * and instance identifiers, i.e., the results of both
   * csHandlerRegistry::GetGenericID() and csHandlerRegistry::GetID() calls.
   * <p>
   * If the instance constraints are the same as the generic ones, use
   * the CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS macro instead of
   * defining this for yourself.
   * \sa CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS
   */
  virtual const csHandlerID * InstanceSucc (
    csRef<iEventHandlerRegistry> &, csRef<iEventNameRegistry> &,
    csEventID) const = 0;
};

/**
 * Macro to create default GenericName and GenericName methods.
 * Also declares StaticHandlerName, which can be used to reference
 * a class of event handlers abstractly without having any of them loaded.
 */
#define CS_EVENTHANDLER_NAMES(x)					\
  static const char * StaticHandlerName()		\
  { return (x); }							\
  static const csHandlerID StaticID(csRef<iEventHandlerRegistry> &reg) \
  {return reg->GetGenericID(StaticHandlerName()); }			\
  virtual const char * GenericName() const		\
  { return StaticHandlerName(); }					\
  virtual csHandlerID GenericID(csRef<iEventHandlerRegistry> &reg) const \
  { return StaticID(reg); }

/**
 * Macro to create "nil" generic and instance constraints.
 */
#define CS_EVENTHANDLER_NIL_CONSTRAINTS			\
	CS_EVENTHANDLER_NIL_GENERIC_CONSTRAINTS		\
	CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS 

/**
 * Macro to create nil generic constraints.
 */
#define CS_EVENTHANDLER_NIL_GENERIC_CONSTRAINTS				\
  virtual const csHandlerID * GenericPrec (		\
    csRef<iEventHandlerRegistry> &, csRef<iEventNameRegistry> &, 	\
    csEventID) const { return 0; }	\
  virtual const csHandlerID * GenericSucc (		\
    csRef<iEventHandlerRegistry> &, csRef<iEventNameRegistry> &, 	\
    csEventID) const { return 0; }

/**
 * Macro to declare instance constraints which are the same as the generics.
 */
#define CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS			\
  virtual const csHandlerID * InstancePrec (		\
    csRef<iEventHandlerRegistry> &r1, csRef<iEventNameRegistry> &r2, 	\
    csEventID e) const { return GenericPrec(r1, r2, e); } \
  virtual const csHandlerID * InstanceSucc (		\
    csRef<iEventHandlerRegistry> &r1, csRef<iEventNameRegistry> &r2, 	\
    csEventID e) const { return GenericSucc(r1, r2, e); }

/** @} */

#endif // __CS_IUTIL_EVENTH_H__
