/*
	  A thread safe general purpose queue.
	  Copyright (C) 2001 by Michael H. Voase
	  Based on csEventQueue (c) 1998 1999, Andrew Zablotony.

	  This library is free software; you can redistribute it and/or
	  modify it under the terms of the GNU Library General Public
	  License as published by the Free Software Foundation; either
	  version 2 of the License, or (at your option) any later version.

	  This library is distributed in the hope that it will be useful,
	  but WITHOUT ANY WARRANTY; without even the implied warranty of
	  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	  Library General Public License for more details.

	  You should have received a copy of the GNU Library General Public
	  License along with this library; if not, write to the Free
	  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CSQUEUE_H__
#define __CSQUEUE_H__

/*
 *	A general purpose thread safe queue. You may use this queue directly
 *	by using the DECLARE_TYPED_QUEUE( QueueType, Content type ). This is
 *	loosely based on csEventQueue, but does not specifically use iEvent.
 *	Place the declare typed queue somewhere in a header file then place
 *	declare typed queue base in a code page to complete the macro.
 */

#define DECLARE_TYPED_QUEUE( NAME, TYPE, DEF_QUEUE_LENGTH ) \
class NAME \
{ \
  volatile TYPE **Content; \
  volatile int qHead, qTail; \
  volatile int Length; \
  volatile int Spinlock; \
public: \
  NAME ( int len = DEF_QUEUE_LENGTH ); \
  virtual ~NAME(); \
  void Put( TYPE *item ); \
  TYPE *Get(); \
  void Clear(); \
  bool IsEmpty() { return qHead == qTail; } \
private: \
  void Resize( int length ); \
  inline void Lock() \
  { while (Spinlock) {} Spinlock++; } \
  inline void Unlock() \
  { Spinlock--; } \
};



#define DECLARE_TYPED_QUEUE_BASE( NAME, TYPE, DEF_QUEUE_LENGTH ) \
NAME::NAME ( int len ) : Content(NULL), Length(0), Spinlock(0) \
{ \
  qHead = qTail = 0; \
  Resize( len ); \
} \
NAME::~NAME () \
{ \
  Clear(); \
  if ( Content ) \
	delete[] Content; \
} \
void NAME::Put( TYPE *item ) \
{ \
again: \
  Lock(); \
  int newHead = qHead + 1; \
  if ( newHead == Length ) \
	newHead = 0; \
  if ( newHead == qTail ) \
  { \
	Unlock(); \
	Resize( Length + 2 ); \
	goto again; \
  } \
  Content [ qHead ] = item; \
  qHead = newHead; \
  Unlock(); \
} \
TYPE *NAME::Get() \
{ \
  if (IsEmpty()) \
	return NULL; \
  else \
  { \
	Lock(); \
	int oldTail = qTail++; \
	if ( qTail == Length ) \
	  qTail = 0; \
	TYPE *item = ( TYPE * ) Content [ oldTail ]; \
	Unlock(); \
	return item; \
  } \
} \
void NAME::Clear() \
{ \
  TYPE *item; \
  while (( item = Get()) != NULL ) \
	{ delete item; } \
} \
void NAME::Resize ( int len ) \
{ \
  if ( len <= 0 ) \
	len = DEF_QUEUE_LENGTH; \
  if ( len == Length ) \
	return; \
  Lock(); \
  volatile TYPE **oldQueue = Content; \
  Content = ( volatile TYPE **) new TYPE *[len]; \
  int oldHead = qHead, oldTail = qTail; \
  qHead = qTail = 0; \
  int oldLength = Length; \
  Length = len; \
  if ( oldQueue ) \
  { while (( oldTail != oldHead ) && ( qHead < Length - 1 )) \
	{ Content [ qHead++ ] = oldQueue [ oldTail++ ]; \
	  if ( oldTail == oldLength ) \
		oldTail = 0; \
	} \
  } \
  delete[] oldQueue; \
  Unlock(); \
}

#endif // __CSQUEUE_H__

