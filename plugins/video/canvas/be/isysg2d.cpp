/*
    Copyright (C) 1998,1999 by Jorrit Tyberghein
	Written by Xavier Planet.
	Overhauled and re-engineered by Eric Sunshine <sunshine@sunshineco.com>
  
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

#include <sys/param.h>
#include "sysdef.h"
#include "cscom/com.h"
#include "cs2d/be/belibg2d.h"
#include "cs2d/be/besysg2d.h"


IMPLEMENT_COMPOSITE_UNKNOWN(csGraphics2DBeLib, XBeLibGraphicsInfo)

STDMETHODIMP IXBeLibGraphicsInfo::GetDisplay (BView ** dpy)
{
    METHOD_PROLOGUE( csGraphics2DBeLib, XBeLibGraphicsInfo);
    
    *dpy = pThis->dpy;
    return S_OK;
}

STDMETHODIMP IXBeLibGraphicsInfo::GetWindow (BWindow ** cryst_window)
{
    METHOD_PROLOGUE( csGraphics2DBeLib, XBeLibGraphicsInfo);
    
    *cryst_window = pThis->window;
    return S_OK;
}

STDMETHODIMP IXBeLibGraphicsInfo::DirectConnect (direct_buffer_info *info)
{
// FIXME: Re-implement/re-enable DirectWindow mode.
#if 0
    METHOD_PROLOGUE( csGraphics2DBeLib, XBeLibGraphicsInfo);

	if (!pThis->fConnected && pThis->fConnectionDisabled) {
		return S_OK;
	}
	pThis->locker->Lock();
	
	switch (info->buffer_state & B_DIRECT_MODE_MASK) {
		case B_DIRECT_START:
			printf("DirectConnect: B_DIRECT_START \n");
			pThis->fConnected = true;
			if (pThis->fDrawingThreadSuspended)	{
				while (resume_thread(find_thread("LoopThread")) == B_BAD_THREAD_STATE)	{
					//	this is done to cope with thread setting fDrawingThreadSuspended then getting
					//	rescheduled before it can suspend itself.  It just makes repeated attempts to
					//	resume that thread.
					snooze(1000);
				}
				pThis->fDrawingThreadSuspended = false;
			}
				
		case B_DIRECT_MODIFY:
			printf("DirectConnect: B_DIRECT_MODIFY \n");
			pThis->Memory = (unsigned char *) info->bits;
			printf("Memory allocated is %p. bytes_per_row is %ld \n", pThis->Memory, info->bytes_per_row);
			pThis->curr_color_space = info->pixel_format;
			pThis->ApplyDepthInfo(pThis->curr_color_space);
			
		// Create scanline address array
		if (pThis->LineAddress == NULL)	{
//			printf ("IXBeLibGraphicsInfo::DirectConnect() -- Creating LineAddress[].\n");
			CHK (pThis->LineAddress = new int [pThis->Height]);
			if (pThis->LineAddress == NULL)	{
			    printf ("IXBeLibGraphicsInfo::DirectConnect() -- Couldn't create LineAddress[].\n");
			    exit (1);
			}
		}
		
		//	initialise array
		{
//		printf("Window bounds: %ld %ld %ld %ld \n", info->window_bounds.left, info->window_bounds.top, info->window_bounds.right, info->window_bounds.bottom);
		int i,addr,bpl = info->bytes_per_row;
		for (i = 0, addr = info->window_bounds.left * pThis->pfmt.PixelBytes + info->window_bounds.top * bpl; 
			i < pThis->Height; 
			i++, addr += bpl)
			pThis->LineAddress[i] = addr;
		}
		break;
		
		case B_DIRECT_STOP:
			printf("DirectConnect: B_DIRECT_STOP \n");
			pThis->fConnected = false;
		break;
	}
	
	pThis->locker->Unlock();
//    printf("leaving IXBeLibGraphicsInfo::DirectConnected \n");
#endif
    return S_OK;
}
