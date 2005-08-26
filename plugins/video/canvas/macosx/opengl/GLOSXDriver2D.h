//
//  GLOSXDriver2D.h
//
//
//  Created by mreda on Tue Oct 30 2001.
//  Copyright (c) 2001 Matt Reda. All rights reserved.
//


#ifndef __CS_GLOSXDRIVER2D_H__
#define __CS_GLOSXDRIVER2D_H__

#include "csplugincommon/macosx/OSXDriver2D.h"


#if defined(__cplusplus)

#include "csutil/macosx/OSXAssistant.h"
#include "csutil/scf.h"
#include "csplugincommon/opengl/glcommon2d.h"
#include "csplugincommon/iopengl/openglinterface.h"

#include "OSXDelegate2D_OpenGL.h"

#include <CoreFoundation/CoreFoundation.h>

#import <mach-o/dyld.h>
#import <stdlib.h>
#import <string.h>


class GLOSXDriver2D : public scfImplementationExt1<GLOSXDriver2D, 
						     csGraphics2DGLCommon, 
						     iOpenGLInterface>, 
		       public OSXDriver2D 
{
public:
    // Constructor
    GLOSXDriver2D(iBase *p);

    // Destructor
    virtual ~GLOSXDriver2D();

    // Plugin initialization
    virtual bool Initialize (iObjectRegistry *reg);

    // Open the window/switch to fullscreen as necessary
    virtual bool Open();

    // Close drawing operations
    virtual void Close();

    // Set window title
    virtual void SetTitle(char *title);

    // Flip video page (or dump to framebuffer)
    virtual void Print(csRect const* area = 0);

    // Set mouse position
    virtual bool SetMousePosition(int x, int y);

    // Set the mouse cursor
    virtual bool SetMouseCursor(csMouseCursorID cursor);

    // Enable/disable canvas resize
    virtual void AllowResize(bool allow);

    // Resize the canvas
    virtual bool Resize(int w, int h);

    // Toggle between fullscreen/windowed mode
    virtual bool ToggleFullscreen();

    virtual void *GetProcAddress (const char *name) 
    {
      // Get the address of a procedure (for OGL use.)
      NSSymbol symbol;
      csString symbolName;
      // Prepend a '_' for the Unix C symbol mangling convention
      symbolName << '_' << name;
      if (NSIsSymbolNameDefined (symbolName))
      {
	symbol = NSLookupAndBindSymbol (symbolName);
	return NSAddressOfSymbol (symbol);
      }
      else
	return 0;
    }
protected:

    // Set up the function pointers for drawing based on the current Depth
    virtual void SetupDrawingFunctions();

    // OpenGL context for drawing
    CGLContextObj context;
};



#endif // __cplusplus

#endif // __CS_GLOSXDRIVER2D_H__
