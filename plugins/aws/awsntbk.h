/*
    Copyright (C) 2002 by Norman Kraemer
  
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

#ifndef __CS_AWS_NTBK_H__
#define __CS_AWS_NTBK_H__

#include "awscomp.h"
#include "awsPanel.h"
#include "awscmdbt.h"
#include "awsTabCtrl.h"
#include "csutil/parray.h"

class awsSliderButton;

class awsNotebookButton : public awsComponent
{
protected:
  /// Texture background, overlay and icon.
  iTextureHandle *tex[3];

  /// Does this button depict the active tab?
  bool is_active;

  /// Is this the first button that is drawn ? important for color selection.
  bool is_first;

  /// Does this bottom reside on the top of the notebook or below.
  bool is_top;

  /// Caption.
  iString *caption;

  /// Is the mouse currently held doen?
  bool captured;

  /// If theres an icon, how is it align.
  int icon_align;

  /// Alpha level for overlay texture if any.
  int alpha_level;

  /// Trigger event if needed.
  bool HandleClick (int x, int y);

  /// Determines the usable rect to expose itself.
  void GetClientRect (csRect &pf);
public:
  awsNotebookButton ();
  virtual ~awsNotebookButton ();

  virtual bool Setup (iAws *_wmgr, iAwsComponentNode *settings);
  virtual bool GetProperty (const char *name, intptr_t *parm);
  virtual bool SetProperty (const char *name, intptr_t parm);

  virtual void OnDraw (csRect clip);
  bool OnMouseDown (int, int, int);
  bool OnMouseUp (int, int x, int y);
  bool OnMouseClick (int, int x, int y);
  bool OnMouseDoubleClick (int, int x, int y);
  virtual csRect getMinimumSize ();

  virtual const char *Type () { return "Notebook Button"; }

  void SetActive (bool what) { is_active=what; }
  void SetFirst (bool what) { is_first=what; }
  void SetTop (bool what) { is_top=what; }

  /// Signal constants.
  enum
  {
    signalActivateTab = 1
  };

  // Icon placement constants.
  enum
  {
    iconLeft = 0,
    iconRight = 1,
    iconTop = 2,
    iconBottom = 3
  };
};

class awsNotebookButtonFactory : public awsComponentFactory
{
public:
  /**
   * Calls register to register the component that it builds with the
   * window manager.
   */
  awsNotebookButtonFactory (iAws *wmgr);

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

SCF_VERSION (iAwsClientRect, 0, 0, 1);

struct iAwsClientRect : public iBase
{
  /**
   * Return a sub rectangle of the components client rectangle where
   * children can be drawn.
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

  class TabVector : public csPDelArray<tabEntry>
  {
  public:
    ~TabVector () { FreeAll (); }

    int Push (awsNotebookButton *btn, awsSlot *slot,
    	iAwsComponent *comp, awsSink *sink)
    {
      tabEntry *te = new tabEntry;
      te->button = btn;
      te->slot = slot;
      te->comp = comp;
      te->sink = sink;
      sink->IncRef ();
      return csPDelArray<tabEntry>::Push (te);
    }

    void FreeAll ()
    {
      size_t i;
      for (i = 0 ; i < Length () ; i++)
        FreeItem (Get (i));
      DeleteAll ();
    }

    void FreeItem (tabEntry* te)
    {
      te->slot->Disconnect (te->button, awsCmdButton::signalClicked,
        te->sink, te->sink->GetTriggerID ("ActivateTab"));
      if (te->slot) te->slot->DecRef ();
      if (te->sink) te->sink->DecRef ();
    }

    static int CompareComp (tabEntry* const& te1, tabEntry* const& te2)
    {
      return (te1->comp < te2->comp ? -1 : te1->comp > te2->comp ? 1 : 0);
    }

    static int CompareButton (tabEntry* const& te1, tabEntry* const& te2)
    {
      return te1->button < te2->button ? -1 : te1->button>te2->button ? 1 : 0;
    }

    static int CompareKeyComp (tabEntry* const& te1,
			       iAwsComponent* const& comp)
    {
      return (te1->comp < comp ? -1 : te1->comp > comp ? 1 : 0);
    }

    static int CompareKeyButton (tabEntry* const& te1,
				 awsNotebookButton* const& button)
    {
      return (te1->button < button ? -1 : te1->button > button ? 1 : 0);
    }

    static csArrayCmp<tabEntry*,awsNotebookButton*>
      KeyButtonFunctor(awsNotebookButton* b)
    {
      return csArrayCmp<tabEntry*,awsNotebookButton*>(b, CompareKeyButton);
    }
  };

  TabVector vTabs;

  /// The two "next/prev" buttons.
  awsSliderButton *next, *prev;

  /// The slots for next/prev.
  awsSlot *next_slot, *prev_slot;

  /// Images for next/prev buttons.
  iTextureHandle *nextimg, *previmg;

  /// First visible button.
  int first;
  
  /// The active tab.
  int active;
  
  /// Button bar on top of notebook or below?
  bool is_top;

  /// Our kitchen sink.
  awsSink *sink;

  /// Max height of buttons in bar.
  int maxheight;

  /// Scroll list of button left.
  void ScrollLeft ();

  /// Scroll list of button right.
  void ScrollRight ();

  /// Scroll buttons until the <idx>-th becomes visible.
  void MakeVisible (int idx);

  /// Return the rectangle where buttons can be drawn in.
  csRect GetClientRect ();
public:
  SCF_DECLARE_IBASE_EXT (awsComponent);

  awsNotebookButtonBar ();
  virtual ~awsNotebookButtonBar ();

  /// Triggered when the component needs to draw.
  virtual void OnDraw (csRect clip);

  virtual bool Setup (iAws *_wmgr, iAwsComponentNode *settings);
  virtual const char *Type () { return "Notebook ButtonBar"; }

  /**
   * This will create a button based on the Caption property of the component.
   * The Icon and IconAlign properties are also taken into account. If this
   * component is the first that has been added it becomes the active one.
   */
  bool Add (iAwsComponent *comp);

  /**
   * This will remove the button associated with this component.
   * The next tab will become active (or the prev if no next exist) if
   * this was the active one.
   */
  bool Remove (iAwsComponent *comp);

  /// Activate the <idx>-th tab.
  void Activate (int idx);

  static void ActivateTab (intptr_t sk, iAwsSource *source);
  static void PrevClicked (intptr_t sk, iAwsSource *source);
  static void NextClicked (intptr_t sk, iAwsSource *source);

  static const int HandleSize;

  /// Layout the buttons, hide or show them, align the next/prev handles.
  void DoLayout ();

  /// Show buttonbar at top or bottom.
  void SetTopBottom (bool to_top);

  struct eiAwsClientRect : public iAwsClientRect
  {
    SCF_DECLARE_EMBEDDED_IBASE (awsNotebookButtonBar);
    virtual csRect GetClientRect (){ return scfParent->GetClientRect (); }
  } scfiAwsClientRect;
  friend struct eiAwsClientRect;
};

class awsNotebookPage : public awsComponent
{
protected:
  /// Holds the texture handle for the background.
  iTextureHandle *tex;

  /// Caption we show in the tab button.
  iString *caption;
  iString *icon;
  int iconalign;
public:
  awsNotebookPage ();
  virtual ~awsNotebookPage ();

  virtual bool Setup (iAws *wmgr, iAwsComponentNode *settings);

  /// Get properties.
  bool GetProperty (const char *name, intptr_t *parm);

  /// Set properties.
  bool SetProperty (const char *name, intptr_t parm);

  /// Returns the named TYPE of the component, like "Notebook Page", etc.
  virtual const char *Type ();
};

class awsNotebookPageFactory : public awsComponentFactory
{
public:
  /**
   * Calls register to register the component that it builds with
   * the window manager.
   */
  awsNotebookPageFactory (iAws *wmgr);

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

class awsNotebook : public awsPanel
{
protected:
  /// Button bar location.
  int bb_location;

  /// Button bar style.
  int bb_style;

  /// Maximum height of a tab.
  int maxheight;

  /// Our kitchen sink.
  awsSink* sink;

  /// Our slot.
  awsSlot slot;

  /// Our tab control.
  awsTabCtrl tab_ctrl;
public:
  awsNotebook ();
  virtual ~awsNotebook ();

  /// Button bar location.
  static const int nbTop;    // Buttons are above page.
  static const int nbBottom; // Buttons are below page.

  /// Button bar style.
  static const int nbBreak; // Button are broken onto next line to be visible.
  static const int nbSlide; // Add slider to make outside buttons visible.

  /// Frame styles.
  static const int fsBump;
  static const int fsSimple;
  static const int fsRaised;
  static const int fsSunken;
  static const int fsFlat;
  static const int fsNone;

  virtual bool Setup (iAws *wmgr, iAwsComponentNode *settings);

  /// Get properties.
  bool GetProperty (const char *name, intptr_t *parm);

  /// Set properties.
  bool SetProperty (const char *name, intptr_t parm);

  /// Executres a scriptable action. Currently supported (on top of standard
  /// ones from awsComponent) : ActivateTab.
  virtual bool Execute(const char* action, iAwsParmList* parmlist);

  /// Returns the named TYPE of the component, like "Notebook Page", etc.
  virtual const char *Type ();

  /// Triggered when the component needs to draw.
  virtual void OnDraw (csRect clip);

  virtual void AddChild (iAwsComponent *child);

  static void OnActivateTab(intptr_t param, iAwsSource* src);
  static void OnDeactivateTab(intptr_t param, iAwsSource* src);
};

class awsNotebookFactory : public awsComponentFactory
{
public:
  /**
   * Calls register to register the component that it builds with
   * the window manager.
   */
  awsNotebookFactory (iAws *wmgr);

  /// Returns a newly created component of the type this factory handles.
  virtual iAwsComponent *Create ();
};

#endif // __CS_AWS_NTBK_H__
