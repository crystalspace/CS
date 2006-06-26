/*
   Crystal Space 3D engine: Event and module naming interface
   (C) 2005 by Adam D. Bradley <artdodge@cs.bu.edu>
   
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

#ifndef __CS_UTIL_EVENTHNAMES_H__
#define __CS_UTIL_EVENTHNAMES_H__

#include "cssysdef.h" /* CS_DEBUG changes our definitions... */
#include "csextern.h"
#include "iutil/eventnames.h"
#include "iutil/eventhandlers.h"
#include "iutil/eventh.h"
#include "csutil/csstring.h"
#include "csutil/eventnames.h"
#include "csutil/scf_implementation.h"
#include "csutil/hash.h"
#include "csutil/strset.h"
#include "csutil/ref.h"

struct iObjectRegistry;

/**\file
 * Event handler naming, name management, indexing, and instantiation
 */
/**
 * \addtogroup event_handling
 * @{ */

struct iEventHandler;

/**
 * The csEventHandlerRegistry maintains a global one-to-one mapping from strings
 * to csHandlerIDs, and a one-to-(zero or one) mapping from csHandlerIDs to
 * iEventHandler pointers.
 */
class CS_CRYSTALSPACE_EXPORT csEventHandlerRegistry : 
  public scfImplementation1<csEventHandlerRegistry, iEventHandlerRegistry>
{
public:
  csEventHandlerRegistry(iObjectRegistry*);
  ~csEventHandlerRegistry();
  /**
   * Get a csHandlerID based upon some string.
   * This should only ever be done to reference generic 
   * (non-instantiated) handler names or single-instance handlers.
   */	
  CS_CONST_METHOD csHandlerID GetGenericID (const char*);
  static CS_CONST_METHOD csHandlerID GetGenericID (iObjectRegistry *reg, 
    const csString &name) 
  {
    return GetRegistry (reg)->GetGenericID (name);
  }
  CS_CONST_METHOD csHandlerID GetGenericPreBoundID (csHandlerID);
  static CS_CONST_METHOD csHandlerID GetGenericPreBoundID (
    iObjectRegistry *reg, csHandlerID id) 
  {
    return GetRegistry (reg)->GetGenericPreBoundID (id);
  }
  CS_CONST_METHOD csHandlerID GetGenericPostBoundID (csHandlerID);
  static CS_CONST_METHOD csHandlerID GetGenericPostBoundID (
    iObjectRegistry *reg, csHandlerID id) 
  {
    return GetRegistry (reg)->GetGenericPostBoundID (id);
  }
    
  /**
   * Get the csHandlerID for a specified event handler, which provides
   * its own name via the iEventHandler::GetInstanceName() method.
   */
  csHandlerID GetID (iEventHandler *);
  static CS_CONST_METHOD csHandlerID GetID (iObjectRegistry *reg, 
    iEventHandler *h) 
  {
    return GetRegistry (reg)->GetID (h);
  }

  /**
   * Get the ID for a given event handler name.  This should usually
   * not be used, since it does not handle magic creation of
   * ":pre" and ":post" signpost handlers or any other such
   * bookkeeping magic.
   */
  csHandlerID GetID (const char*);
  static CS_CONST_METHOD csHandlerID GetID (iObjectRegistry *reg,
					    csString &name)
  {
    return GetRegistry (reg)->GetID (name);
  }

  /**
   * Used when an iEventHandler is desroyed to remove our reference.
   */
  void ReleaseID (csHandlerID id);
  static CS_CONST_METHOD void ReleaseID (iObjectRegistry *reg, 
    csHandlerID id)
  {
    GetRegistry (reg)->ReleaseID (id);
  }
  /**
   * Used when an iEventHandler is destroyed to remove our reference.
   */
  void ReleaseID (iEventHandler *);
  static CS_CONST_METHOD void ReleaseID (iObjectRegistry *reg, 
    iEventHandler *h) 
  {
    GetRegistry (reg)->ReleaseID (h);
  }
  /**
   * Returns the handler registered for a csHandlerID 
   * (will be 0 if csHandlerID is a generic name, i.e.,
   * if (!csEventHandlerRegistry->IsInstance(id)).
   */
  CS_CONST_METHOD iEventHandler* GetHandler (csHandlerID id);
  static inline CS_CONST_METHOD iEventHandler* GetHandler (
    iObjectRegistry *reg, csHandlerID id) 
  {
    return GetRegistry (reg)->GetHandler (id);
  };

  /**
   * returns true if instanceid is a handler instance,
   * genericid is a generic instance, and instanceid is an
   * instance of genericid in particular.
   */
  CS_CONST_METHOD bool const IsInstanceOf (csHandlerID instanceid, 
    csHandlerID genericid);
  static inline CS_CONST_METHOD bool IsInstanceOf (iObjectRegistry *reg, 
    csHandlerID instanceid, csHandlerID genericid) 
  {
    return GetRegistry (reg)->IsInstanceOf (instanceid, genericid);
  };

  /**
   * returns true if id is a handler instance (i.e., not a generic name).
   */
  CS_CONST_METHOD bool const IsInstance (csHandlerID id);
  static inline CS_CONST_METHOD bool IsInstance (iObjectRegistry *reg, 
  csHandlerID id) 
  {
    return GetRegistry (reg)->IsInstance (id);
  };

  /**
   * Returns the csHandleID for the generic name for instance name id.
   */
  CS_CONST_METHOD csHandlerID const GetGeneric (csHandlerID id);
  static inline CS_CONST_METHOD csHandlerID GetGeneric (iObjectRegistry *reg, 
    csHandlerID id) 
  {
    return GetRegistry (reg)->GetGeneric (id);
  };

  /**
   * Returns the string name for a csHandlerID.
   */
  CS_CONST_METHOD const char* GetString (csHandlerID id);
  static inline CS_CONST_METHOD const char* GetString (
    iObjectRegistry *reg, csHandlerID id) 
  {
    return GetRegistry (reg)->GetString (id);
  };

  static csRef<iEventHandlerRegistry> GetRegistry (
    iObjectRegistry *object_reg);

 private:
  iObjectRegistry *object_reg;
  csStringSet names;
  csHash<csHandlerID, csHandlerID> instantiation; 
  csHash<csRef<iEventHandler>, csHandlerID> idToHandler;
  csHash<csHandlerID, csRef<iEventHandler> > handlerToID;
  csHash<csHandlerID, csHandlerID> handlerPres;
  csHash<csHandlerID, csHandlerID> handlerPosts;
  uint32 instanceCounter;
};

/* @} */





struct iFrameEventSignpost : public iEventHandler 
{
 public:
  iFrameEventSignpost () { }
  virtual ~iFrameEventSignpost () { }
  CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS
  virtual bool HandleEvent (iEvent&) 
  { 
    return false;
  }
};


class FrameSignpost_Logic3D
: public scfImplementation2<FrameSignpost_Logic3D, 
  iFrameEventSignpost, 
  scfFakeInterface<iEventHandler> > 
{
 private:
  FrameSignpost_Logic3D () : scfImplementationType (this) { }
 public:
  CS_EVENTHANDLER_NAMES("crystalspace.signpost.logic3d")
  CS_CONST_METHOD virtual const csHandlerID * GenericPrec
    (csRef<iEventHandlerRegistry> &,
     csRef<iEventNameRegistry> &,
     csEventID) const;
  CS_CONST_METHOD virtual const csHandlerID * GenericSucc
    (csRef<iEventHandlerRegistry> &r1,
     csRef<iEventNameRegistry> &r2,
     csEventID e) const;
};

class FrameSignpost_3D2D
: public scfImplementation2<FrameSignpost_3D2D, 
  iFrameEventSignpost, 
  scfFakeInterface<iEventHandler> > 
{
 private:
  FrameSignpost_3D2D () : scfImplementationType (this) { }
 public:
  CS_EVENTHANDLER_NAMES("crystalspace.signpost.3d2d")
  CS_CONST_METHOD virtual const csHandlerID * GenericPrec
    (csRef<iEventHandlerRegistry> &,
     csRef<iEventNameRegistry> &,
     csEventID) const;
  CS_CONST_METHOD virtual const csHandlerID * GenericSucc
    (csRef<iEventHandlerRegistry> &r1,
     csRef<iEventNameRegistry> &r2,
     csEventID e) const;
};

class FrameSignpost_2DConsole
: public scfImplementation2<FrameSignpost_2DConsole, 
  iFrameEventSignpost, 
  scfFakeInterface<iEventHandler> > 
{
 private:
  FrameSignpost_2DConsole () : scfImplementationType (this) { }
 public:
  CS_EVENTHANDLER_NAMES("crystalspace.signpost.2dconsole")
  CS_CONST_METHOD virtual const csHandlerID * GenericPrec
    (csRef<iEventHandlerRegistry> &,
     csRef<iEventNameRegistry> &,
     csEventID) const;
  CS_CONST_METHOD virtual const csHandlerID * GenericSucc
    (csRef<iEventHandlerRegistry> &r1,
     csRef<iEventNameRegistry> &r2,
     csEventID e) const;
};

class FrameSignpost_ConsoleDebug
: public scfImplementation2<FrameSignpost_ConsoleDebug, 
  iFrameEventSignpost, 
  scfFakeInterface<iEventHandler> > 
{
 private:
  FrameSignpost_ConsoleDebug () : scfImplementationType (this) { }
 public:
  CS_EVENTHANDLER_NAMES("crystalspace.signpost.consoledebug")
  CS_CONST_METHOD virtual const csHandlerID * GenericPrec
    (csRef<iEventHandlerRegistry> &,
     csRef<iEventNameRegistry> &,
     csEventID) const;
  CS_CONST_METHOD virtual const csHandlerID * GenericSucc
    (csRef<iEventHandlerRegistry> &r1,
     csRef<iEventNameRegistry> &r2,
     csEventID e) const;
};

class FrameSignpost_DebugFrame
: public scfImplementation2<FrameSignpost_DebugFrame, 
  iFrameEventSignpost, 
  scfFakeInterface<iEventHandler> > 
{
 private:
  FrameSignpost_DebugFrame () : scfImplementationType (this) { }
 public:
  CS_EVENTHANDLER_NAMES("crystalspace.signpost.debugframe")
  CS_CONST_METHOD virtual const csHandlerID * GenericPrec
    (csRef<iEventHandlerRegistry> &,
     csRef<iEventNameRegistry> &,
     csEventID) const;
  CS_CONST_METHOD virtual const csHandlerID * GenericSucc
    (csRef<iEventHandlerRegistry> &r1,
     csRef<iEventNameRegistry> &r2,
     csEventID e) const;
};


/**
 * Use this macro to declare your event handler as wanting to
 * handle the csevFrame event in the "logic" phase (i.e.,
 * before any handlers in the "3D" phase).
 */
#define CS_EVENTHANDLER_PHASE_LOGIC(x)					\
CS_EVENTHANDLER_NAMES(x)						\
CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS				\
CS_CONST_METHOD virtual const csHandlerID * GenericPrec			\
(csRef<iEventHandlerRegistry> &, csRef<iEventNameRegistry> &,		\
 csEventID) const {							\
  return 0;								\
}									\
CS_CONST_METHOD virtual const csHandlerID * GenericSucc			\
(csRef<iEventHandlerRegistry> &r1, csRef<iEventNameRegistry> &r2,	\
 csEventID event) const {						\
  static csHandlerID succConstraint[6];					\
  if (event != csevFrame(r2))						\
    return 0;								\
  succConstraint[0] = FrameSignpost_Logic3D::StaticID(r1);		\
  succConstraint[1] = FrameSignpost_3D2D::StaticID(r1);			\
  succConstraint[2] = FrameSignpost_2DConsole::StaticID(r1);		\
  succConstraint[3] = FrameSignpost_ConsoleDebug::StaticID(r1);		\
  succConstraint[4] = FrameSignpost_DebugFrame::StaticID(r1);		\
  succConstraint[5] = CS_HANDLERLIST_END;				\
  return succConstraint;						\
}

/**
 * Use this macro to declare your event handler as wanting to
 * handle the csevFrame event in the "3D" phase (i.e.,
 * after any handlers in the "logic" phase and before any
 * handlers in the "2D" phase).
 */
#define CS_EVENTHANDLER_PHASE_3D(x)					\
CS_EVENTHANDLER_NAMES(x)						\
CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS				\
CS_CONST_METHOD virtual const csHandlerID * GenericPrec			\
(csRef<iEventHandlerRegistry> &r1, csRef<iEventNameRegistry> &r2,	\
 csEventID event) const {						\
  static csHandlerID precConstraint[2];					\
  if (event != csevFrame(r2))						\
    return 0;								\
  precConstraint[0] = FrameSignpost_Logic3D::StaticID(r1);		\
  precConstraint[1] = CS_HANDLERLIST_END;				\
  return precConstraint;						\
}									\
CS_CONST_METHOD virtual const csHandlerID * GenericSucc			\
(csRef<iEventHandlerRegistry> &r1, csRef<iEventNameRegistry> &r2,	\
 csEventID event) const {						\
  static csHandlerID succConstraint[5];					\
  if (event != csevFrame(r2))						\
    return 0;								\
  succConstraint[0] = FrameSignpost_3D2D::StaticID(r1);			\
  succConstraint[1] = FrameSignpost_2DConsole::StaticID(r1);		\
  succConstraint[2] = FrameSignpost_ConsoleDebug::StaticID(r1);		\
  succConstraint[3] = FrameSignpost_DebugFrame::StaticID(r1);		\
  succConstraint[4] = CS_HANDLERLIST_END;				\
  return succConstraint;						\
}

/**
 * Use this macro to declare your event handler as wanting to
 * handle the csevFrame event in the "2D" phase (i.e.,
 * after any handlers in the "3D" phase and before any
 * handlers in the "console" phase).
 */
#define CS_EVENTHANDLER_PHASE_2D(x)					\
CS_EVENTHANDLER_NAMES(x)						\
CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS				\
CS_CONST_METHOD virtual const csHandlerID * GenericPrec			\
(csRef<iEventHandlerRegistry> &r1, csRef<iEventNameRegistry> &r2,	\
 csEventID event) const {						\
  static csHandlerID precConstraint[3];					\
  if (event != csevFrame(r2))						\
    return 0;								\
  precConstraint[0] = FrameSignpost_Logic3D::StaticID(r1);		\
  precConstraint[1] = FrameSignpost_3D2D::StaticID(r1);			\
  precConstraint[2] = CS_HANDLERLIST_END;				\
  return precConstraint;						\
}									\
CS_CONST_METHOD virtual const csHandlerID * GenericSucc			\
(csRef<iEventHandlerRegistry> &r1, csRef<iEventNameRegistry> &r2,	\
 csEventID event) const {						\
  static csHandlerID succConstraint[4];					\
  if (event != csevFrame(r2))						\
    return 0;								\
  succConstraint[0] = FrameSignpost_2DConsole::StaticID(r1);		\
  succConstraint[1] = FrameSignpost_ConsoleDebug::StaticID(r1);		\
  succConstraint[2] = FrameSignpost_DebugFrame::StaticID(r1);		\
  succConstraint[3] = CS_HANDLERLIST_END;				\
  return succConstraint;						\
}

/**
 * Use this macro to declare your event handler as wanting to
 * handle the csevFrame event in the "Console" phase (i.e.,
 * after any handlers in the "2D" phase and before any
 * handlers in the "debug" phase).
 */
#define CS_EVENTHANDLER_PHASE_CONSOLE(x)				\
CS_EVENTHANDLER_NAMES(x)						\
CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS				\
CS_CONST_METHOD virtual const csHandlerID * GenericPrec			\
(csRef<iEventHandlerRegistry> &r1, csRef<iEventNameRegistry> &r2,	\
 csEventID event) const {						\
  static csHandlerID precConstraint[4];					\
  if (event != csevFrame(r2))						\
    return 0;								\
  precConstraint[0] = FrameSignpost_Logic3D::StaticID(r1);		\
  precConstraint[1] = FrameSignpost_3D2D::StaticID(r1);			\
  precConstraint[2] = FrameSignpost_2DConsole::StaticID(r1);		\
  precConstraint[3] = CS_HANDLERLIST_END;				\
  return precConstraint;						\
}									\
CS_CONST_METHOD virtual const csHandlerID * GenericSucc			\
(csRef<iEventHandlerRegistry> &r1, csRef<iEventNameRegistry> &r2,	\
 csEventID event) const {						\
  static csHandlerID succConstraint[3];					\
  if (event != csevFrame(r2))						\
    return 0;								\
  succConstraint[0] = FrameSignpost_ConsoleDebug::StaticID(r1);		\
  succConstraint[1] = FrameSignpost_DebugFrame::StaticID(r1);		\
  succConstraint[2] = CS_HANDLERLIST_END;				\
  return succConstraint;						\
}

/**
 * Use this macro to declare your event handler as wanting to
 * handle the csevFrame event in the "Debug" phase (i.e.,
 * after any handlers in the "Console" phase and before any
 * handlers in the "Frame" phase).
 */
#define CS_EVENTHANDLER_PHASE_DEBUG(x)					\
CS_EVENTHANDLER_NAMES(x)						\
CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS				\
CS_CONST_METHOD virtual const csHandlerID * GenericPrec			\
(csRef<iEventHandlerRegistry> &r1, csRef<iEventNameRegistry> &r2,	\
 csEventID event) const {						\
  static csHandlerID precConstraint[5];					\
  if (event != csevFrame(r2))						\
    return 0;								\
  precConstraint[0] = FrameSignpost_Logic3D::StaticID(r1);		\
  precConstraint[0] = FrameSignpost_3D2D::StaticID(r1);			\
  precConstraint[0] = FrameSignpost_2DConsole::StaticID(r1);		\
  precConstraint[0] = FrameSignpost_ConsoleDebug::StaticID(r1);		\
  precConstraint[1] = CS_HANDLERLIST_END;				\
  return precConstraint;						\
}									\
CS_CONST_METHOD virtual const csHandlerID * GenericSucc			\
(csRef<iEventHandlerRegistry> &r1, csRef<iEventNameRegistry> &r2,	\
 csEventID event) const {						\
  static csHandlerID succConstraint[2];					\
  if (event != csevFrame(r2))						\
    return 0;								\
  succConstraint[0] = FrameSignpost_DebugFrame::StaticID(r1);		\
  succConstraint[1] = CS_HANDLERLIST_END;				\
  return succConstraint;						\
}

/**
 * Use this macro to declare your event handler as wanting to
 * handle the csevFrame event in the "Debug" phase (i.e.,
 * after any handlers in the "Console" phase and before any
 * handlers in the "Frame" phase).
 */
#define CS_EVENTHANDLER_PHASE_FRAME(x)					\
CS_EVENTHANDLER_NAMES(x)						\
CS_EVENTHANDLER_DEFAULT_INSTANCE_CONSTRAINTS				\
CS_CONST_METHOD virtual const csHandlerID * GenericPrec			\
(csRef<iEventHandlerRegistry> &r1, csRef<iEventNameRegistry> &r2,	\
 csEventID event) const {						\
  static csHandlerID precConstraint[6];					\
  if (event != csevFrame(r2))						\
    return 0;								\
  precConstraint[0] = FrameSignpost_Logic3D::StaticID(r1);		\
  precConstraint[0] = FrameSignpost_3D2D::StaticID(r1);			\
  precConstraint[0] = FrameSignpost_2DConsole::StaticID(r1);		\
  precConstraint[0] = FrameSignpost_ConsoleDebug::StaticID(r1);		\
  precConstraint[0] = FrameSignpost_DebugFrame::StaticID(r1);		\
  precConstraint[1] = CS_HANDLERLIST_END;				\
  return precConstraint;						\
}									\
CS_CONST_METHOD virtual const csHandlerID * GenericSucc			\
(csRef<iEventHandlerRegistry> &, csRef<iEventNameRegistry> &,		\
 csEventID) const {							\
  return 0;								\
}


#endif // __CS_UTIL_EVENTHNAMES_H__
