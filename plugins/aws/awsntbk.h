#ifndef __AWS_NOTEBOOK_H__
#define __AWS_NOTEBOOK_H__

/*
    Copyright (C) 2002 by Norman Krämer
  
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

#include "awscomp.h"
#include "awscmdbt.h"

class awsSliderButton;

class awsNotebookButton : public awsComponent
{
 protected:

  // texture background, overlay and icon
  iTextureHandle *tex[3];

  // does this button depict the active tab ?
  bool is_active;

  // is this the first button that is drawn ? important for color selection
  bool is_first;

  // does this bottom reside on the top of the notebook or below
  bool is_top;

  // caption
  iString *caption;

  // is the mouse currently held doen ?
  bool captured;

  // if theres an icon, how is it align
  int icon_align;

  // alpha level for overlay texture if any
  int alpha_level;

  // trigger event if needed
  bool HandleClick (int x, int y);

  // determines the usable rect to expose itself
  void GetClientRect (csRect &pf);

 public:

  awsNotebookButton ();
  virtual ~awsNotebookButton ();

  virtual bool Setup (iAws *_wmgr, awsComponentNode *settings);
  virtual bool GetProperty (char *name, void **parm);
  virtual bool SetProperty (char *name, void *parm);

  virtual void OnDraw (csRect clip);
  bool OnMouseDown (int, int, int);
  bool OnMouseUp (int, int x, int y);
  bool OnMouseClick (int, int x, int y);
  bool OnMouseDoubleClick (int, int x, int y);
  virtual csRect getPreferredSize ();
  virtual csRect getMinimumSize ();

  virtual char *Type (){return "Notebook Button";}

  void SetActive (bool what){is_active=what;}
  void SetFirst (bool what){is_first=what;}
  void SetTop (bool what){is_top=what;}

  static const int signalActivateTab;
  static const int iconLeft;
  static const int iconRight;
  static const int iconTop;
  static const int iconBottom;
};

class awsNotebookButtonFactory :
  public awsComponentFactory
{
public:
  /// Calls register to register the component that it builds with the window manager
  awsNotebookButtonFactory (iAws *wmgr);

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

SCF_VERSION (iAwsClientRect, 0, 0, 1);
struct iAwsClientRect : public iBase
{
  /**
   * Return a sub rectangle of the components client rectangle where children can be drawn.
   */
  virtual csRect GetClientRect () = 0;
};

class awsNotebookButtonBar : public awsComponent
{
 protected:

  struct tabEntry
  {
    awsNotebookButton *button;
    awsSlot *slot;
    iAwsComponent *comp;
    awsSink *sink;
  };

  class TabVector : public csVector
  {
  public:
    virtual ~TabVector (){DeleteAll ();}
    tabEntry *Get(int idx) const {return (tabEntry*)csVector::Get (idx);}
    int Push (awsNotebookButton *btn, awsSlot *slot, iAwsComponent *comp, awsSink *sink)
    {
      tabEntry *te = new tabEntry;
      te->button = btn;
      te->slot = slot;
      te->comp = comp;
      te->sink = sink;
      sink->IncRef ();
      return csVector::Push ((csSome)te);
    }
    virtual bool FreeItem (csSome Item)
    {
      tabEntry *te = (tabEntry*)Item;
      te->slot->Disconnect (te->button, awsCmdButton::signalClicked, 
                           te->sink, te->sink->GetTriggerID ("ActivateTab"));
      SCF_DEC_REF (te->button);
      SCF_DEC_REF (te->slot);
      SCF_DEC_REF (te->sink);
      delete te;
      return true;
    }
    virtual int Compare (csSome Item1, csSome Item2, int Mode=0) const
    {
      tabEntry *te1 = (tabEntry *)Item1;
      tabEntry *te2 = (tabEntry *)Item2;
      if (Mode==0)
        return (te1->comp < te2->comp ? -1 : te1->comp > te2->comp ? 1 : 0);
      else
        return (te1->button < te2->button ? -1 : te1->button > te2->button ? 1 : 0);
    }
    virtual int CompareKey (csSome Item1, csConstSome Key, int Mode=0) const
    {
      (void)Mode;
      tabEntry *te1 = (tabEntry *)Item1;
      iAwsComponent *comp = (iAwsComponent *)Key;
      awsNotebookButton *button = (awsNotebookButton *)Key;
      if (Mode==0)
        return (te1->comp < comp ? -1 : te1->comp > comp ? 1 : 0);
      else
        return (te1->button < button ? -1 : te1->button > button ? 1 : 0);
    }
  };

  TabVector vTabs;

  // the two "next/prev" buttons
  awsSliderButton *next, *prev;

  // the slots for next/prev
  awsSlot *next_slot, *prev_slot;

  // images for next/prev buttons
  iTextureHandle *nextimg, *previmg;

  // first visible button
  int first;
  // the active tab
  int active;
  // button bar on top of notebook or below ?
  bool is_top;

  // our kitchen sink
  awsSink *sink;

  // max height of buttons in bar
  int maxheight;

  // Scroll list of button left
  void ScrollLeft ();

  // Scroll list of button right
  void ScrollRight ();

  // scroll buttons until the <idx>-th becomes visible
  void MakeVisible (int idx);

  // return the rectangle where buttons can be drawn in
  csRect GetClientRect ();

 public:

  SCF_DECLARE_IBASE_EXT (awsComponent);

  awsNotebookButtonBar ();
  virtual ~awsNotebookButtonBar ();

  /// Triggered when the component needs to draw
  virtual void OnDraw (csRect clip);

  virtual bool Setup (iAws *_wmgr, awsComponentNode *settings);
  virtual char *Type (){return "Notebook ButtonBar";}

  // This will create a button based on the Caption property of the component.
  // the Icon and IconAlign properties are also taken into account.
  // If this component is the first that has been added it becomes the active one.
  bool Add (iAwsComponent *comp);

  // This will remove the button associated with this component.
  // The next tab will become active (or the prev if no next exist) if this was the active one.
  bool Remove (iAwsComponent *comp);

  // Activate the <idx>-th tab
  void Activate (int idx);

  static void ActivateTab (void *sk, iAwsSource *source);
  static void PrevClicked (void *sk, iAwsSource *source);
  static void NextClicked (void *sk, iAwsSource *source);

  static const int HandleSize;

  // layout the buttons, hide or show them, align the next/prev handles
  void DoLayout ();

  // show buttonbar at top or bottom
  void SetTopBottom (bool to_top);

  struct eiAwsClientRect : public iAwsClientRect
  {
    SCF_DECLARE_EMBEDDED_IBASE (awsNotebookButtonBar);
    virtual csRect GetClientRect (){ return scfParent->GetClientRect (); }
  } scfiAwsClientRect;
  friend struct eiAwsClientRect;
};

class awsNotebookPage :
  public awsComponent
{
 protected:
  /// Holds the texture handle for the background
  iTextureHandle *tex;

  // caption we show in the tab button
  iString *caption;
  iString *icon;
  int iconalign;

public:
  awsNotebookPage ();
  virtual ~awsNotebookPage ();

public:
  virtual bool Setup (iAws *wmgr, awsComponentNode *settings);

  /// Gets properties
  bool GetProperty (char *name, void **parm);

  /// Sets properties
  bool SetProperty (char *name, void *parm);

  /// Returns the named TYPE of the component, like "Notebook Page", etc.
  virtual char *Type ();

};

class awsNotebookPageFactory :
  public awsComponentFactory
{
public:

  /// Calls register to register the component that it builds with the window manager
  awsNotebookPageFactory (iAws *wmgr);

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

class awsNotebook :
  public awsComponent
{
 protected:

 protected:
  /// Holds the texture handle for the background
  iTextureHandle *tex;

  // hold frame style
  int frame_style;
  
  // button bar location
  int bb_location;

  // button bar style
  int bb_style;

  // maximum height of a tab
  int maxheight;

  // our sink
  awsSink *sink;

  // our button bar
  awsNotebookButtonBar *bb;

  // alpha_level
  int alpha_level;

  // calculate position and style of tabs
  void DoLayout ();

public:
  awsNotebook ();
  virtual ~awsNotebook ();

public:

  /************** button bar location ***********/
  static const int nbTop;    // buttons are above page
  static const int nbBottom; // buttons are below page

  /************** button bar style ***********/
  static const int nbBreak; // button are broken onto next line to be visible
  static const int nbSlide; // a slider is added to make outside buttons visible

  /************** frame styles ***********/
  static const int fsBump;
  static const int fsSimple;
  static const int fsRaised;
  static const int fsSunken;
  static const int fsFlat;
  static const int fsNone;

  virtual bool Setup (iAws *wmgr, awsComponentNode *settings);

  /// Gets properties
  bool GetProperty (char *name, void **parm);

  /// Sets properties
  bool SetProperty (char *name, void *parm);

  /// Returns the named TYPE of the component, like "Notebook Page", etc.
  virtual char *Type ();
public:

  /// Triggered when the component needs to draw
  virtual void OnDraw (csRect clip);

  virtual void AddChild (iAwsComponent *child, bool has_layout=false);
};

class awsNotebookFactory :
  public awsComponentFactory
{
public:
  /// Calls register to register the component that it builds with the window manager
  awsNotebookFactory (iAws *wmgr);

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

#endif
