/*
    Copyright (C) 2011 by Alin Baciu

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

#ifndef __CS_VPL_PLAYER_H__
#define __CS_VPL_PLAYER_H__

/**\file
 * Video Player: player
 */

#include "csutil/scf.h"
#include "csutil/ref.h"

/**\addtogroup vpl
 * @{ */

struct iDataBuffer;
struct iVPLCodec;

/**
 * The video player is used to play video files
 */
struct iVPLPlayer : public virtual iBase
{
  SCF_INTERFACE(iVPLPlayer,0,1,0);

  /**
   * Initialize the video player
   */
  virtual void InitPlayer (csPtr<iVPLCodec> codec) = 0;
};

/** @} */

#endif // __CS_VPL_PLAYER_H__
