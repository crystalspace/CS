/*
    Copyright (C) 2000 by W.C.A. Wijngaards

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

#ifndef __CS_SCHEDULE_H__
#define __CS_SCHEDULE_H__

#include "csextern.h"

class csSchedulePart;

/**
 * The csSchedule class provides an easy way to get timers in applications.
 * It can handle both repeating and single-shot callbacks. It is useful
 * for handling time in 3D virtual worlds.
 * <p>
 * Use it like this:
 * <PRE>
 * class myEntity {
 *   public:
 *   virtual void Update();
 * };
 * </PRE>
 * <p>
 * Suppose you have an object of class myEntity, which looks like a button
 * in your virtual world, and you want the button to blink. Calling Update
 * every NextFrame would look bad, and handling the timing yourself is
 * a hassle (and can be lots slower then mass-handling by csSchedule). So
 * you can use the csSchedule to call the myEntity::Update method every
 * second.
 * <p>
 * You can do it this way:
 * <PRE>
 * void call_entity_update(void *arg)
 * {
 *   myEntity *mp = (myEntity*)arg;
 *   mp->Update();
 * }
 * </PRE>
 * <p>
 * You would then use the csSchedule method
 *   AddCallback(call_entity_update, (void*)my_entity, 1000);
 * to have it call the function with the object pointer as argument after
 * 1000 milliseconds (= 1 second) once.
 * or you can use:
 *   AddRepeatCallback(call_entity_update, (void*)my_entity, 1000);
 * to have the function called repeatedly, every 1000 msec (= second).
 * <p>
 * To notify the schedule that time has passed, each frame, for example
 * in the NextFrame() method, you must call the TimePassed(elapsed_time)
 * function.
 * <p>
 * This class is useful for callbacks in 3D virtual worlds, but the callbacks
 * can have some jitter due to framerates.  For mission-critical hardware IO
 * calls (like controlling a floppy drive or controlling the UART) this jitter
 * will be too big.  In those cases use interrupt-driven callbacks, and place
 * the controlling code in some platform-specific implementation file, since
 * this type of use is typically platform-dependent.  However, although this
 * class cannot give callbacks inside a single frame, it will behave as best as
 * possible using callbacks every frame.
 */
class CS_CRYSTALSPACE_EXPORT csSchedule
{
private:
  /// first part of the scheduled callbacks
  csSchedulePart *first;

  /// internal, insert part given msec after now into list
  void InsertCall(csSchedulePart *part, int afternow);
  /// internal, unlink part from list given prev. (prev can be 0)
  void RemoveCall(csSchedulePart *prev, csSchedulePart *part);

public:
  /// create an empty schedule
  csSchedule();
  ///
  ~csSchedule();

  /**
   *  Notify the schedule that time has passed, elapsed_time is in msec.
   *  It will update the internal data and call any callbacks if necessary.
   */
  void TimePassed(int elapsed_time);

  /**
   * Add a single-shot callback
   * the function must be of type:    void function(void *arg)
   * arg is passed as argument to the function.
   * delay: the function is called this many msec have passed.
   */
  void AddCallback(void (*func)(void*), void *arg, int delay);

  /**
   * Add a repeating callback
   * the function must be of type:    void function(void *arg)
   * arg is passed as argument to the function.
   * period: the function is called every time this many msec pass.
   */
  void AddRepeatCallback(void (*func)(void*), void *arg, int period);

  /**
   * Remove a repeating callback.
   * (if multiple identical calls exist, the first is removed)
   */
  void RemoveCallback(void (*func)(void*), void *arg, int period);

  /**
   * Remove a single-shot callback.
   * (if multiple identical calls exist, the first is removed)
   */
  void RemoveCallback(void (*func)(void*), void *arg);

  /**
   * Remove a single-shot or repeating callback.
   * (if multiple identical calls exist, all are removed)
   * removes all callbacks using given argument, whatever their function
   * or period. Useful if the argument in question is an object that is
   * destructed.
   * So, in a class myEntity, in ~myEntity() you can call:
   * schedule->RemoveCallback(this);
   */
  void RemoveCallback(void *arg);
};

#endif // __CS_SCHEDULE_H__
