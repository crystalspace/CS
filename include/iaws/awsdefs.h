#ifndef __AWS_DEFINITIONS__
#define __AWS_DEFINITIONS__
/**************************************************************************
    Copyright (C) 2001 by Christopher Nelson

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
*****************************************************************************/

/**\file
 */

/**
 * \addtogroup aws
 * @{ */
/// Component flag: component is transparent
const unsigned int AWSF_CMP_TRANSPARENT = 2;  
/// Component flag: component always needs erased by parent first
const unsigned int AWSF_CMP_ALWAYSERASE = 4;  
/// Component flag: component is hidden
const unsigned int AWSF_CMP_HIDDEN      = 8;  
/// Component flag: set by window manager when a component gets too small during resize
const unsigned int AWSF_CMP_INVISIBLE   = 16; 
/// Component flag: component should not receive events
const unsigned int AWSF_CMP_DEAF        = 32; 

/// Window transitons: window slides left (start out to curframe)
const unsigned int AWS_TRANSITION_SLIDE_IN_LEFT  = 0;  
/// Window transitons: window slides right
const unsigned int AWS_TRANSITION_SLIDE_IN_RIGHT = 1;  
/// Window transitons: window slides up
const unsigned int AWS_TRANSITION_SLIDE_IN_UP    = 2;  
/// Window transitons: window slides down
const unsigned int AWS_TRANSITION_SLIDE_IN_DOWN  = 3;  

/// Window transitons: window slides left (start curframe to out, and window disappears)
const unsigned int AWS_TRANSITION_SLIDE_OUT_LEFT  = 4;  
/// Window transitons: window slides right
const unsigned int AWS_TRANSITION_SLIDE_OUT_RIGHT = 5;  
/// Window transitons: window slides up
const unsigned int AWS_TRANSITION_SLIDE_OUT_UP    = 6;  
/// Window transitons: window slides down
const unsigned int AWS_TRANSITION_SLIDE_OUT_DOWN  = 7;  

/// Window transitons: window slides left (start out with cur frame, end with user frame)
const unsigned int AWS_TRANSITION_SLIDE_LEFT  = 8;  
/// Window transitons: window slides right
const unsigned int AWS_TRANSITION_SLIDE_RIGHT = 9;  
/// Window transitons: window slides up
const unsigned int AWS_TRANSITION_SLIDE_UP    = 10;  
/// Window transitons: window slides down
const unsigned int AWS_TRANSITION_SLIDE_DOWN  = 11;  

/// Sink error codes: no error
const unsigned int AWS_ERR_SINK_NONE	      = 0;  
/// Sink error codes: the requested trigger was not found
const unsigned int AWS_ERR_SINK_TRIGGER_NOT_FOUND   = 1;  
/// Sink error codes: could not handle trigger, because there are none.
const unsigned int AWS_ERR_SINK_NO_TRIGGERS         = 2;  
/* @} */

#endif
