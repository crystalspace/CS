/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifdef OS_AMIGAOS

#include "sysdef.h"
#include "util/inifile.h"
#include "system/amiga/amiga.h"

#ifdef COMP_GCC
#define __NOLIBBASE__
#endif

extern "C" {
	#include <exec/types.h>
	#include <exec/memory.h>
	#include <exec/execbase.h>

	#include <intuition/intuition.h>
	#include <intuition/screens.h>
	#include <intuition/intuitionbase.h>

	#include <libraries/asl.h>

	#include <graphics/gfxbase.h>
	#include <graphics/gfx.h>
	#include <graphics/modeid.h>

	#include <devices/inputevent.h>

	#include <proto/exec.h>
	#include <proto/intuition.h>
	#include <proto/graphics.h>
	#include <proto/asl.h>
	#include <proto/keymap.h>

	#include <cybergraphx/cybergraphics.h>
//    #include <clib/cybergraphics_protos.h>
	#include <inline/cybergraphics.h>

	#include <time.h>
	#include <sys/timeb.h>
	#include <stdarg.h>
}

#include <time.h>

extern struct ExecBase *SysBase;

static struct Screen *screen;
static struct Window *window;

struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;
struct Library *AslBase;
struct Library *CyberGfxBase;
struct Library *KeymapBase;

bool directgfx = false;
bool WarpMode = false;
static int Depth = 8;
static void *handle;
static unsigned short *empty_pointer;

long csSystemDriver::Time ()
{
//    return time(NULL)*1000;
	timeb tp;
	ftime (&tp);
	return tp.time*1000+tp.millitm;
}

SysKeyboardDriver::SysKeyboardDriver() : csKeyboardDriver()
{
	// Nothing to be done
}

SysKeyboardDriver::~SysKeyboardDriver(void)
{
	Close();
}

bool SysKeyboardDriver::Open(void)
{
	return true;
}

void SysKeyboardDriver::Close(void)
{
}

SysMouseDriver::SysMouseDriver() : csMouseDriver()
{
}

SysMouseDriver::~SysMouseDriver()
{
	Close();
}

bool SysMouseDriver::Open(void)
{
	return true;
}

void SysMouseDriver::Close(void)
{
}


bool SysSystemDriver::ParseArg(int argc, char* argv[], int& i)
{
	if (strcasecmp("-warp", argv[i]) == 0) {
		i++;
		if (i<argc) {
			if (strcasecmp(argv[i],"on") == 0) {
				WarpMode = true;
			} else {
				WarpMode = false;
			}
		}
	} else if (strcasecmp("-depth", argv[i]) == 0) {
		i++;
		if (i<argc) {
			Depth = atoi(argv[i]);
			if (Depth != 8 && Depth != 15) {
				Printf(MSG_INITIALIZATION, "Only 8 or 15 bits supported. Using 8 bits\n");
				Depth = 8;
			} else {
				Printf(MSG_INITIALIZATION, "Using %d bits\n", Depth);
			}
		}
	} else if (strcasecmp("-direct", argv[i]) == 0) {
		directgfx = true; i++;
	} else return csSystemDriver::ParseArg(argc, argv, i);
  return true;
}

void SysSystemDriver::Help()
{
	csSystemDriver::Help();
	Printf(MSG_STDOUT, "  -warp <on|off>     use Warp3D (default %s)\n", WarpMode == true ? "on" : "off");
	Printf(MSG_STDOUT, "  -depth <8|15>      display depth (default: %d)\n", Depth);
	Printf(MSG_STDOUT, "  -directgfx         use direct writes to the display\n");
}

void SysSystemDriver::Loop(void)
{
	struct IntuiMessage *imsg, msg;
	bool shift=false, alt=false, ctrl=false;
	struct InputEvent ie;
	char Key1;

	while (!Shutdown && !ExitLoop) {
		static long prev_time = -1;
		NextFrame((prev_time == -1) ? 0 : Time() - prev_time, Time());

		while ((imsg = (struct IntuiMessage *)GetMsg(window->UserPort))) {
			CopyMem(imsg, &msg, sizeof(struct IntuiMessage));
			ReplyMsg((struct Message *)imsg);

			switch(msg.Class) {
			case IDCMP_RAWKEY:
				if (msg.Code & IECODE_UP_PREFIX) {
					msg.Code &= ~IECODE_UP_PREFIX;
					switch(msg.Code) {
					case 0x4c:
						Keyboard->do_keyrelease(System->Time(), CSKEY_UP);
						break;
					case 0x4d:
						Keyboard->do_keyrelease(System->Time(), CSKEY_DOWN);
						break;
					case 0x4f:
						Keyboard->do_keyrelease(System->Time(), CSKEY_LEFT);
						break;
					case 0x4e:
						Keyboard->do_keyrelease(System->Time(), CSKEY_RIGHT);
						break;
					case 0x3f:
						Keyboard->do_keyrelease(System->Time(), CSKEY_PGUP);
						break;
					case 0x1f:
						Keyboard->do_keyrelease(System->Time(), CSKEY_PGDN);
						break;
					case 0x1d:
						Keyboard->do_keyrelease(System->Time(), CSKEY_END);
						break;
					case 0x45:
						Keyboard->do_keyrelease(System->Time(), CSKEY_ESC);
						break;
					case 0x44:
						Keyboard->do_keyrelease(System->Time(), '\n');
						break;
					case 96:
					case 97:
						shift = true;
						break;
					case 100:
					case 101:
						alt = true;
						break;
					case 99:
						ctrl = true;
						break;
					default:
						ie.ie_Class         = IECLASS_RAWKEY;
						ie.ie_SubClass      = 0;
						ie.ie_Code          = msg.Code;
						ie.ie_Qualifier     = msg.Qualifier;
						ie.ie_EventAddress  = (APTR *) *((ULONG *)msg.IAddress);
						MapRawKey(&ie, &Key1, 1, NULL);
						Keyboard->do_keyrelease(System->Time(), int(Key1));
						break;
					}
				} else {
					switch(msg.Code) {
					case 0x4c:
						Keyboard->do_keypress(System->Time(), CSKEY_UP);
						break;
					case 0x4d:
						Keyboard->do_keypress(System->Time(), CSKEY_DOWN);
						break;
					case 0x4f:
						Keyboard->do_keypress(System->Time(), CSKEY_LEFT);
						break;
					case 0x4e:
						Keyboard->do_keypress(System->Time(), CSKEY_RIGHT);
						break;
					case 0x3f:
						Keyboard->do_keypress(System->Time(), CSKEY_PGUP);
						break;
					case 0x1f:
						Keyboard->do_keypress(System->Time(), CSKEY_PGDN);
						break;
					case 0x1d:
						Keyboard->do_keypress(System->Time(), CSKEY_END);
						break;
					case 0x45:
						Keyboard->do_keypress(System->Time(), CSKEY_ESC);
						break;
					case 0x44:
						Keyboard->do_keypress(System->Time(), '\n');
						break;
					case 96:
					case 97:
						shift = false;
						break;
					case 100:
					case 101:
						alt = false;
						break;
					case 99:
						ctrl = false;
						break;
					default:
						ie.ie_Class         = IECLASS_RAWKEY;
						ie.ie_SubClass      = 0;
						ie.ie_Code          = msg.Code;
						ie.ie_Qualifier     = msg.Qualifier;
						ie.ie_EventAddress  = (APTR *) *((ULONG *)msg.IAddress);
						MapRawKey(&ie, &Key1, 1, NULL);
						Keyboard->do_keypress(System->Time(), int(Key1));
						break;
					}
				}
				break;
			case IDCMP_MOUSEBUTTONS:
				if        (msg.Code == SELECTDOWN) {
					Mouse->do_buttonpress(System->Time(), 1, int(msg.MouseX), int(msg.MouseY), shift, alt, ctrl);
				} else if (msg.Code == MIDDLEDOWN) {
					Mouse->do_buttonpress(System->Time(), 2, int(msg.MouseX), int(msg.MouseY), shift, alt, ctrl);
				} else if (msg.Code == MENUDOWN) {
					Mouse->do_buttonpress(System->Time(), 3, int(msg.MouseX), int(msg.MouseY), shift, alt, ctrl);
				} else if (msg.Code == SELECTUP) {
					Mouse->do_buttonrelease(System->Time(), 1, int(msg.MouseX), int(msg.MouseY));
				} else if (msg.Code == MIDDLEUP) {
					Mouse->do_buttonrelease(System->Time(), 2, int(msg.MouseX), int(msg.MouseY));
				} else if (msg.Code == MENUUP) {
					Mouse->do_buttonrelease(System->Time(), 3, int(msg.MouseX), int(msg.MouseY));
				}
				break;
			case IDCMP_MOUSEMOVE:
				Mouse->do_mousemotion(System->Time(), int(msg.MouseX), int(msg.MouseY));
				break;
			}

			//if (msg.Qualifier & (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) shift = true; else shift = false;
			//if (msg.Qualifier & (IEQUALIFIER_LALT|IEQUALIFIER_RALT))     alt = true; else alt = false;
			//if (msg.Qualifier & IEQUALIFIER_CONTROL)                     ctrl = true; else ctrl = false;
		}
		
		prev_time = Time();
	}
}

#endif

