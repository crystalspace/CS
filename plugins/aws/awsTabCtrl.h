// awsTabCtrl.h: interface for the awsTabCtrl class.
//
//////////////////////////////////////////////////////////////////////
#ifndef __AWSTABCTRL_H__
#define __AWSTABCTRL_H__

#include "awscomp.h"
#include "awsscrbr.h"

/** This class implements a basic tab button on a tab control. */
class awsTab : public awsComponent
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

  // A parameter that the user can store with this tab
  // Often its helpful as an identifier for which tab was pressed
  // or what to do about it
  void* user_param;

  // trigger event if needed
  bool HandleClick (int x, int y);

 public:

  awsTab ();
  virtual ~awsTab ();

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

  virtual char *Type (){return "Tab Button";}

  void SetActive (bool what);
  void SetFirst (bool what){is_first=what;}
  void SetTop (bool what){is_top=what;}

  static const int signalActivateTab;
  static const int signalDeactivateTab;
  static const int iconLeft;
  static const int iconRight;
  static const int iconTop;
  static const int iconBottom;
};

/* This class implements a simple tab control. 

   Basic usage is to insert tabs using the Add action, supplying a caption and optional parameter.
   Add will return an iAwsSource* which you would then connect however you like.
   When a tab is selcted/deselected a sActivateTab/sDeactivateTab signal will be sent
   through the source you received. Also from the source's component you can query
   the "User Param" property to retrieve the parameter you passed in Add. Either the
   index, the source, or the parameter can be used to identify tabs in other method calls.
   However indexes can change by adding/removing tabs, and it is your responsibilty
   to ensure the user_param is unique if you desire to use it.
*/

class awsTabCtrl : public awsComponent  
{
protected:


  class TabVector : public csVector
  {
  public:
    virtual ~TabVector (){DeleteAll ();}
    awsTab *Get(int idx) const {return (awsTab*)csVector::Get (idx);}
    int Push (awsTab *tab)
    {
      return csVector::Push ((csSome)tab);
    }
    virtual bool FreeItem (csSome Item)
    {
      delete (awsTab*)Item;
      return true;
    }
    virtual int Compare (csSome Item1, csSome Item2) const
    {
      awsTab *te1 = (awsTab *)Item1;
      awsTab *te2 = (awsTab *)Item2;
      return (te1 < te2 ? -1 : te1 > te2 ? 1 : 0);
    }
  };

  TabVector vTabs;

  // first visible button
  int first;
  // the active tab
  int active;
  // button bar on top of notebook or below ?
  bool is_top;

  // true if the client area should be shrunk because of the scroll buttons
  bool clip_to_scroll;

  // our kitchen sink
  awsSink *sink;

  // max height of buttons in bar
  int maxheight;

  // the two "next/prev" buttons
  awsSliderButton next, prev;

  // slots for the next/prev buttons
  awsSlot slot_activate, slot_next, slot_prev;

  // images for next/prev buttons
  iTextureHandle *nextimg, *previmg;

  // layout the buttons, hide or show them, align the next/prev handles
  void DoLayout ();

  // Finds the index of the first tab which uses this param
  int FindTab(void* user_param);

 public:

  //SCF_DECLARE_IBASE_EXT (awsComponent);

  awsTabCtrl();
  virtual ~awsTabCtrl ();

  virtual void OnDraw(csRect clip);
  virtual bool Setup (iAws *_wmgr, awsComponentNode *settings);
  virtual char *Type (){return "Simple Tab Control";}

  virtual csRect getInsets();

  // This will create a button based on the caption
  // This source will fire sActivateTab and sDeactivateTab
  // The parameter can also be retrieved from the source component
  // by querying it's "User Param" property
  // If this component is the first that has been added it becomes the active one.
  iAwsSource* AddTab (iString* caption, void* user_param = NULL);

  // This will remove the tab at index
  // The next tab will become active (or the prev if no next exist) if this was the active one.
  void RemoveTab (int index);
  // Remove the tab that uses this source
  // The next tab will become active (or the prev if no next exist) if this was the active one.
  void RemoveTab (iAwsSource* src);
  // Remove the tab that has this user param
  // The next tab will become active (or the prev if no next exist) if this was the active one.
  void RemoveTab (void* user_param);

  // Activate the <idx>-th tab
  void ActivateTab (int idx);
  // Activate the tab that uses this src
  void ActivateTab (iAwsSource* src);
  // Activate the tab that uses this user_param
  void ActivateTab (void* param);

  // Returns the source for the currently active tab
  iAwsSource* GetActiveTab();
  // Returns the index of the currently active tab. 
  int GetActiveTabIndex();
  // Returns the user_param of the currently active tab.
  void* GetActiveTabParam(); 

  // Scroll list of button left
  void ScrollLeft ();

  // Scroll list of button right
  void ScrollRight ();

  // scroll buttons until the <idx>-th becomes visible
  void MakeVisible (int idx);

  virtual csRect getPreferredSize();

  virtual void OnResized();

  static void ActivateTabCallback (void *sk, iAwsSource *source);

  static void PrevClicked (void *sk, iAwsSource *source);
  static void NextClicked (void *sk, iAwsSource *source);

  static const int HandleSize;

  // show buttonbar at top or bottom
  void SetTopBottom (bool to_top);

};

#endif

