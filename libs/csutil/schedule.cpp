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

#include "cssysdef.h"
#include "csutil/schedule.h"
#include "cssys/system.h"

//----- csSchedule ----------------------------------------------

csSchedule::csSchedule()
{
  first = NULL;
}

csSchedule::~csSchedule()
{
  csSchedulePart *p, *np;
  p =first;
  while(p)
  {
    np = p->next;
    delete p;
    p = np;
  }
}


void csSchedule::TimePassed(int elapsed_time)
{
  int todotime = elapsed_time;
  while(first)
  {
    if(first->after > todotime)
    {
      /// we are done
      first->after -= todotime;
      return;
    }
    /// first part must be called
    /// reschedule the first part (if needed), then call the first part
    
    /// move the 'now' so that first->after msec have passed.
    csSchedulePart *part = first;
    todotime -= part->after;
    part->after = 0;
    /// unlink, possibly relink
    first = part->next;
    if(part->period != 0) 
      InsertCall(part, part->period); // play it again, Sam!

    /// do the callback
    /// now that it has been relinked/unlinked etc, the callback can
    /// use csSchedule methods without problems. (to add/change it's own or
    /// other callbacks)
    part->callback(part->arg);

    if(part->period == 0)
      delete part; /// get rid of single-shot callback
  } /// end while - look if there are any parts in the (possibly altered) list
}


void csSchedule::AddCallback(void (*func)(void*), void *arg, int delay)
{
  if(delay <= 0) return;
  csSchedulePart *p = new csSchedulePart;
  p->callback = func;
  p->arg = arg;
  p->period = 0;
  p->after=0;
  p->next=NULL;
  InsertCall(p, delay);
}

void csSchedule::AddRepeatCallback(void (*func)(void*), void *arg, int period)
{
  if(period <= 0) return;
  csSchedulePart *p = new csSchedulePart;
  p->callback = func;
  p->arg = arg;
  p->period = period;
  p->after=0;
  p->next=NULL;
  InsertCall(p, period);
}

void csSchedule::RemoveCallback(void (*func)(void*), void *arg, int period)
{
  /// find it
  csSchedulePart *prev=NULL, *p=first;
  while(p)
  {
    if((p->callback == func) && (p->arg==arg) && (p->period == period))
    {
      /// delete it
      RemoveCall(prev, p);
      delete p;
      return;
    }
    prev = p;
    p = p->next;
  }
  /// not found
}

void csSchedule::RemoveCallback(void (*func)(void*), void *arg)
{
  /// find it
  csSchedulePart *prev=NULL, *p=first;
  while(p)
  {
    if((p->callback == func) && (p->arg==arg) && (p->period == 0))
    {
      /// delete it
      RemoveCall(prev, p);
      delete p;
      return;
    }
    prev = p;
    p = p->next;
  }
  /// not found
}

void csSchedule::RemoveCallback(void *arg)
{
  /// find all that match
  csSchedulePart *prev=NULL, *p=first;
  while(p)
  {
    if(p->arg==arg)
    {
      csSchedulePart *pnext = p->next;
      /// delete it
      RemoveCall(prev, p);
      delete p;
      /// prev stays prev. (p has been snipped out between prev and pnext)
      p = pnext;
    }
    else
    {
      // continue
      prev = p;
      p = p->next;
    }
  } // while parts
  /// removed all matches
}


/// internal helping functions

void csSchedule::InsertCall(csSchedulePart *part, int afternow)
{
  csSchedulePart *p = first;
  csSchedulePart *prev= NULL;
  int afterprev = afternow; /// nr of msec part must go after 'prev'
  while(p && (p->after <= afterprev))
  {
    afterprev -= p->after;
    prev = p;
    p = p->next;
  }
  if(afterprev < 0)
  {
    //System->Printf(CS_MSG_INTERNAL_ERROR, "csSchedule::InsertCall: delay<0\n");
    delete part;
    return;
  }
  /// Link the part into the list
  if(prev==NULL)
  { // insert as first part
    part->next = first;
    first = part;
  }
  else 
  {
    part->next = prev->next;
    prev->next = part;
  }
  /// handle timing
  part->after = afterprev;
  if(part->next)
    part->next->after -= afterprev;
}


void csSchedule::RemoveCall(csSchedulePart *prev, csSchedulePart *part)
{
  if(part==NULL) return;
  /// handle timing
  if(part->next)
    part->next->after += part->after;
  /// unlink
  if(prev==NULL)
  {
    // part is the first in the list.
    first = part->next;
  }
  else
  {
    prev->next = part->next;
  }
  part->next = NULL;
  part->after = 0;
}
