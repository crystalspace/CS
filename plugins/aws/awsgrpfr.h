 #ifndef __AWS_GROUP_FRAME_H__
 #define __AWS_GROUP_FRAME_H__
/**************************************************************************
    Copyright (C) 2000-2001 by Christopher Nelson 
    
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
#include "awscomp.h"


class awsGroupFrame : public awsComponent
{  
   /// Flags for frame style.
   int frame_style;

   /// Alpha level for this component
   int alpha_level;

   /// Handle for texture background
   iTextureHandle *bkg;
   
   /// Caption text for this component
   iString *caption;
   
public:
    awsGroupFrame();
    virtual ~awsGroupFrame();

   /******* Frame Styles **********************/
    
   /// A frame that's a bump
   static const int fsBump;

   /// A simple frame
   static const int fsSimple;

   /// A frame that looks like a sunk button
   static const int fsSunken;

   /// A frame that looks like a raised button
   static const int fsRaised;
   
   /******* Signals **********************/

   /// An up and down motion for the button
   static const int signalClicked;

         
public:
    /// Get's the texture handle and the title, plus style if there is one.
    virtual bool Setup(iAws *wmgr, awsComponentNode *settings);

    /// Gets properties
    bool GetProperty(char *name, void **parm);

    /// Sets properties
    bool SetProperty(char *name, void *parm);

    /// Returns the named TYPE of the component, like "Radio Button", etc.
    virtual char *Type();

public:
    SCF_DECLARE_IBASE;

    /// Triggered when the component needs to draw
    virtual void OnDraw(csRect clip);

    /// Triggered when the user presses a mouse button down
    virtual bool OnMouseDown(int button, int x, int y);
    
    /// Triggered when the user unpresses a mouse button 
    virtual bool OnMouseUp(int button, int x, int y);
    
    /// Triggered when the user moves the mouse
    virtual bool OnMouseMove(int button, int x, int y);

    /// Triggered when the user clicks the mouse
    virtual bool OnMouseClick(int button, int x, int y);

    /// Triggered when the user double clicks the mouse
    virtual bool OnMouseDoubleClick(int button, int x, int y);

    /// Triggered when this component loses mouse focus
    virtual bool OnMouseExit();

    /// Triggered when this component gains mouse focus
    virtual bool OnMouseEnter();

    /// Triggered when the user presses a key
    virtual bool OnKeypress(int key, int modifiers);
    
    /// Triggered when the keyboard focus is lost
    virtual bool OnLostFocus();

    /// Triggered when the keyboard focus is gained
    virtual bool OnGainFocus();

};

class awsGroupFrameFactory : public awsComponentFactory
{
public:
    SCF_DECLARE_IBASE;

    /// Calls register to register the component that it builds with the window manager
    awsGroupFrameFactory(iAws *wmgr);

    /// Does nothing
    virtual ~awsGroupFrameFactory();

    /// Returns a newly created component of the type this factory handles. 
    virtual iAwsComponent *Create();
};

#endif

