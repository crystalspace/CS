/*
    Copyright (C) 2006 by Jorrit Tyberghein
	      (C) 2006 by Frank Richter
    
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

#ifndef __CS_IUTIL_KITCHENSINK_H__
#define __CS_IUTIL_KITCHENSINK_H__

#include "csutil/scf_interface.h"

/**\file
 * Kitchen sink interface
 */
 
/**
 * Interface to interact with a kitchen sink.
 * \todo Iterator for contained objects.
 */
struct iKitchenSink : public virtual iBase
{
  SCF_INTERFACE (iKitchenSink, 0,0,1);
  
  /**
   * Insert an object in the kitchen sink. 
   */
  virtual void Insert (iBase* object) = 0;
  /**
   * Remove an object from the kitchen sink.
   * \return Whether the removal succeeded. Failure usually means that it
   *   wasn't in the sink in first place.
   */
  virtual bool Remove (iBase* object) = 0;
  /**
   * Enable or disable the shredder fitted to the drain.
   * \remarks May only work in American implementations.
   */
  virtual void SetShredderActive (bool enable) = 0;
  /**
   * Retrieve whether the shredder fitted to the drain was enabled.
   */
  virtual bool GetShredderActive () = 0;
  /**
   * Fill a certain amount of water into the sink.
   * \param amount Amount of water in relation to the complete volume of the
   *  sink.
   * \remarks You cannot use negative amounts to drain water; use Drain()
   *  instead.
   * \remarks Multiple calls to Fill() with a summed amount greater than 1
   *  should not cause an overflow.
   */
  virtual float Fill (float amount) = 0;
  /**
   * Drain the water from the sink.
   * \warning If small objects were inserted, they might get flushed away
   *  and (if a shredder is present and enabled) shredded.
   */
  virtual void Drain () = 0;
};

#endif // __CS_IUTIL_KITCHENSINK_H__
