/*
    Copyright (C) 2000-2001 by Christopher Nelson
  
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

#ifndef __CS_AWS_SLOT_H__
#define __CS_AWS_SLOT_H__

#include "iaws/aws.h"
#include "iutil/comp.h"
#include "iutil/strset.h"
#include "csutil/parray.h"

/**
 * This implements the signal source architecture.  A signal source is
 * an object which knows how to register and unregister slots. A signal
 * source always defines and emits signals. A signal source does not
 * care if someone is listening or not.
 */

/**
 * This class implements the sink managaer, which controls all other
 * registered sinks.
 */
class awsSinkManager : public iAwsSinkManager
{
private:
  struct SinkMap
  {
    unsigned long name;
    csRef<iAwsSink> sink;

    SinkMap (unsigned long n, iAwsSink *s) : name (n), sink (s) { };
  };

  csPDelArray<SinkMap> sinks;
  csRef<iStringSet> strset;

  unsigned long NameToId (const char*) const;
public:
  SCF_DECLARE_IBASE;

  /// Constructor.
  awsSinkManager (iBase *p);
  /// Destructor.
  virtual ~awsSinkManager ();

  /// Performs whatever initialization is necessary.
  virtual bool Setup (iObjectRegistry*);

  /// Registers a sink by name for lookup.
  virtual void RegisterSink (const char *name, iAwsSink *sink);

  /// Removes a sink.
  virtual bool RemoveSink (iAwsSink* sink);

  /// Finds a sink by name for connection.
  virtual iAwsSink *FindSink (const char *name);

  /**
   * Create a new embeddable sink, with parm as the intptr_t passed
   * into the triggers.
   */
  virtual iAwsSink *CreateSink (intptr_t parm);

#ifndef AWS_VOIDP_IS_ERROR
  /**
   * Create a new embeddable sink, with parm as the void* passed into the
   * triggers.
   * \deprecated For proper 64-bit platform support, use the intptr_t version
   *   of CreateSink().
   */
  virtual iAwsSink *CreateSink(void *parm)
  { return CreateSink((intptr_t)parm); }
#endif

  /// Create a new embeddable slot.
  virtual iAwsSlot *CreateSlot ();
};

/**
 * This class is the implementation of a signal sink.  It handles trigger
 * registration and dispatch.
 */
class awsSink : public iAwsSink
{
private:
  struct TriggerMap
  {
    unsigned long name;
    void (*trigger) (intptr_t, iAwsSource *);

    TriggerMap (unsigned long n, void (*t) (intptr_t, iAwsSource *))
      : name(n), trigger(t) { };
  };

  /// List of triggers registered.
  csPDelArray<TriggerMap> triggers;

  /// Parameter to pass to triggers.
  intptr_t parm;

  /// Last error code.
  unsigned int sink_err;

  /// Shared string table.
  csRef<iStringSet> strset;

  /// Convert string to ID.
  unsigned long NameToId (const char*) const;
public:
  SCF_DECLARE_IBASE;

  awsSink (iAws*);
  awsSink (iStringSet*);
  virtual ~awsSink ();

  intptr_t GetParm () { return parm; }
  /// P is the parm that is passed to triggers.
  void SetParm (intptr_t p) { parm = p; }

  /// Maps a trigger name to a trigger id.
  virtual unsigned long GetTriggerID (const char *name);

  /// Handles trigger events.
  virtual void HandleTrigger (int trigger, iAwsSource *source);

  /// A sink should call this to register trigger events.
  virtual void RegisterTrigger (const char *name,
    void (*Trigger) (intptr_t, iAwsSource *));

#ifndef AWS_VOIDP_IS_ERROR
  /**
   * A sink should call this to register trigger events
   * \deprecated For proper 64-bit platform support, use the intptr_t version
   *   of RegisterTrigger().
   */
  virtual void RegisterTrigger(const char *name,
  	void (*Trigger)(void *, iAwsSource *))
  { RegisterTrigger(name, (void(*)(intptr_t,iAwsSource*))Trigger); }
#endif

  /// Gets the last error code.
  virtual unsigned int GetError () { return sink_err; }
};

/**
 * This class is the signal source implementation, subclassed by all
 * other components.
 */
class awsSource : public iAwsSource
{
private:
  /// Owner.
  iAwsComponent *owner;

  struct SlotSignalMap
  {
    /// The slot that's registered.
    csRef<iAwsSlot> slot;

    /// The signal it's registered for.
    unsigned long signal;
  };

  /// contains a list of all slots that we have registered.
  csPDelArray<SlotSignalMap> slots;
public:
  SCF_DECLARE_IBASE;

  /// Initializes a couple things.
  awsSource ();
  void SetOwner (iAwsComponent *_owner) { owner = _owner; }

  /// Does nothing.
  virtual ~awsSource ();

  /// Gets the component owner for this (sources are embedded).
  virtual iAwsComponent *GetComponent ();

  /**
   * Registers a slot for any one of the signals defined by a source.
   * Each sources's signals exist in it's own namespace.
   */
  virtual bool RegisterSlot (iAwsSlot *slot, unsigned long signal);

  /// Unregisters a slot for a signal.
  virtual bool UnregisterSlot (iAwsSlot *slot, unsigned long signal);

  /// Broadcasts a signal to all slots that are interested.
  virtual void Broadcast (unsigned long signal);
};

/**                                                                                                                    *
 * This implements the slot architecture.  Slots are conduits for signals.
 * Signals always have a source.  Slots may be a conduit for multiple
 * signals from multiple sources. It is up to the user to determine how
 * to use his slots. Slots do not care, other than they route based on
 * signal, NOT source.  Therefore, if a slot is hooked up to two different
 * sources that emit the same signal, then any trigger registered for
 * that signal will recieve the signal from EVERY source that emits it.                                                                                   *
 */
class awsSlot : public iAwsSlot
{
private:
  /// The sink that this slot manages.

  /**
   * A mapping between signals and triggers.  One signal may map to
   * multiple triggers, or vice versa. The mapping list is traversed
   * everytime there is a signal emitted.  All mappings are evaluated
   * and if they qualify, are activated. Note that a slot may be
   * connected to multiple sources, but that it routes based on
   * signal, NOT source.
   */
  struct SignalTriggerMap
  {
    unsigned long signal;
    unsigned long trigger;
    csRef<iAwsSink> sink;
    unsigned long refs;

    SignalTriggerMap (
      unsigned long s,
      iAwsSink *sk,
      unsigned long t,
      unsigned long r)
    : signal (s), trigger (t), sink (sk), refs (r)
    { };
  };

  csPDelArray<SignalTriggerMap> stmap;
public:
  SCF_DECLARE_IBASE;

  /// Does nothing.
  awsSlot ();

  /// Also does nothing.
  virtual ~awsSlot ();

  /**
   * Creates a connection from the source:signal to the sink:trigger specified.
   */
  virtual void Connect (
    iAwsSource *source,
    unsigned long signal,
    iAwsSink *sink,
    unsigned long trigger);

  /**
   * Disconnects the slot from a signal source, also properly unmaps
   * a signal::trigger binding.
   */
  virtual void Disconnect (
    iAwsSource *source,
    unsigned long signal,
    iAwsSink *sink,
    unsigned long trigger);

  /// Emit a signal to activate all necessary triggers.
  virtual void Emit (iAwsSource &source, unsigned long signal);
};

#endif // __CS_AWS_SLOT_H__
