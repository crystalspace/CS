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
#include "util/def.h"
#include "system/system.h"

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

struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;
struct Library *AslBase;
struct Library *CyberGfxBase;
struct Library *KeymapBase;

SysGraphics2D::SysGraphics2D(int argc, char* argv[]) : csGraphics2D()
{
	if (Depth == 8) {
		CsPrintf(MSG_INITIALIZATION, "Indirect 8 Bit rendering\n");
		if (!directgfx) Memory = (unsigned char *)AllocVec(FRAME_WIDTH*FRAME_HEIGHT, MEMF_CLEAR|MEMF_ANY);
	} else if (Depth == 15) {
		pfmt.PixelBytes = 2;
		pfmt.PalEntries = 0;
		pfmt.RedMask   = 0x1f<<10;
		pfmt.GreenMask = 0x1f<<5;
		pfmt.BlueMask  = 0x1f;

		complete_pixel_format();
		CsPrintf(MSG_INITIALIZATION, "Direct 15 bit rendering\n");
		if (pfmt.PixelBytes == 2) {
			DrawPixel = DrawPixel16;
			WriteChar = WriteChar16;
			GetPixelAt = GetPixelAt16;
			DrawSprite = DrawSprite16;
		}

	}
}

SysGraphics2D::~SysGraphics2D(void)
{
	if (Depth == 15) return;
	if (Memory && !directgfx) FreeVec(Memory);
	Memory = 0;
}

bool SysGraphics2D::Open(char *Title)
{
	if (!csGraphics2D::Open(Title)) return false;

	printf("Opening Graphics '%s'\n", Title);

	GfxBase = (struct GfxBase *)OpenLibrary((unsigned char *)"graphics.library", (unsigned long)39);
	IntuitionBase = (struct IntuitionBase *)OpenLibrary((unsigned char *)"intuition.library", (unsigned long)39);
	AslBase = OpenLibrary((unsigned char *)"asl.library", (unsigned long)37);
	KeymapBase = OpenLibrary((unsigned char *)"keymap.library", (unsigned long)37);
	CyberGfxBase = OpenLibrary((unsigned char *)"cybergraphics.library", (unsigned long)0);
	if (!CyberGfxBase) {
		CsPrintf(MSG_STDOUT, "This version of Crystal Space requires CyberGraphics or Picasso96\n");
		return false;
	}

	ULONG ModeID = BestCModeIDTags(
		CYBRBIDTG_Depth,            Depth,
		CYBRBIDTG_NominalWidth,     FRAME_WIDTH,
		CYBRBIDTG_NominalHeight,    FRAME_HEIGHT,
		CYBRBIDTG_BoardName,        "CVision3D",
	TAG_DONE);

	if (ModeID == INVALID_ID) {
		CsPrintf(MSG_STDOUT, "Invalid Mode ID: No suitable mode found\n");
		return FALSE;
	}

	screen = OpenScreenTags(NULL,
		SA_Height,    FRAME_HEIGHT*2,
		SA_DisplayID, ModeID,
		SA_Depth,     Depth,
		SA_ShowTitle, FALSE,
		SA_Draggable, FALSE,
	TAG_DONE);

	window = OpenWindowTags(NULL,
		WA_CustomScreen,    screen,
		WA_Activate,        TRUE,
		WA_Width,           screen->Width,
		WA_Height,          screen->Height,
		WA_Left,            0,
		WA_Top,             0,
		WA_Title,           NULL,
		WA_CloseGadget,     FALSE,
		WA_Backdrop,        TRUE,
		WA_Borderless,      TRUE,
		WA_IDCMP,           IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE,
		WA_Flags,           WFLG_REPORTMOUSE|WFLG_RMBTRAP,
	TAG_DONE);

	empty_pointer = (unsigned short *)AllocVec(8, MEMF_CHIP|MEMF_CLEAR);

	if (Depth == 15) {
		ULONG xy;
		UnLockBitMap(LockBitMapTags(window->RPort->BitMap,
			LBMI_BASEADDRESS, &xy,
		TAG_DONE));
		Memory = (unsigned char *)xy;
	}

	return true;
}

void SysGraphics2D::Close(void)
{
	if (window)         CloseWindow(window);
	if (screen)         CloseScreen(screen);
	if (CyberGfxBase)   CloseLibrary(CyberGfxBase);
	if (AslBase)        CloseLibrary(AslBase);
	if (IntuitionBase)  CloseLibrary((struct Library *)IntuitionBase);
	if (GfxBase)        CloseLibrary((struct Library *)GfxBase);
	if (KeymapBase)     CloseLibrary(KeymapBase);
	GfxBase = 0; AslBase = 0; IntuitionBase = 0;
	CyberGfxBase = 0; KeymapBase = 0;
	window = 0; screen = 0;
	csGraphics2D::Close();
}

void SysGraphics2D::Print(csRect *area)
{
	if (Depth == 15) return;
	if (!area) {
		(void)WritePixelArray(APTR(Memory), 0,0, UWORD(FRAME_WIDTH), window->RPort,
			0,0, FRAME_WIDTH, FRAME_HEIGHT, RECTFMT_LUT8);
	} else {
		(void)WritePixelArray(APTR(Memory), UWORD(area->xmin), UWORD(area->ymin),
			UWORD(FRAME_WIDTH), window->RPort, UWORD(area->xmin), UWORD(area->ymin),
			UWORD(area->xmax-area->xmin), UWORD(area->ymax-area->ymin),
			RECTFMT_LUT8);
	}
}

void SysGraphics2D::SetRGB(int i, int r, int g, int b)
{
	SetRGB32(&(screen->ViewPort), i, r*0x01010101, g*0x01010101, b*0x01010101);
	csGraphics2D::SetRGB(i,r,g,b);
}

bool SysGraphics2D::SetMouseCursor (int iShape, TextureMM* iBitmap)
{
	(void)iBitmap;
	if (iShape != csmcArrow) {
		SetPointer(window, empty_pointer, 0, 0, 0, 0);
	} else {
		ClearPointer(window);
	}

	return (iShape == csmcArrow);
}

void SysGraphics2D::FinishDraw()
{
	if (directgfx) UnLockBitMap(handle);
}

bool SysGraphics2D::BeginDraw()
{
	ULONG where;
	if (directgfx) {
		if ((handle = LockBitMapTags(window->RPort->BitMap,
				LBMI_BASEADDRESS, &where,
			TAG_DONE))) {
			Memory = (unsigned char *)where;
			return true;
		} else return false;
	}
	return true;
}

#endif
