// awsControlBar.h: interface for the awsControlBar class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AWSCONTROLBAR_H__610A57D1_4F71_4AF3_A31F_23D1445E4AB1__INCLUDED_)
#define AFX_AWSCONTROLBAR_H__610A57D1_4F71_4AF3_A31F_23D1445E4AB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "awscomp.h"
#include "awsPanel.h"


/* This is the base class for a number of different component types. A control bar
   is just a simple component that has any number of any type of component in it,
   layed out in a row.
*/

class awsControlBar : public awsPanel
{
private:

  /// the components in the bar
  awsComponentVector comps;

  /// spacing desired
  int hGap, vGap;

  /// alignment
  int vert_align;

  /// true if the control bar should automatically resize its width
  /// to fit all components
  bool size_to_fit_horz;

  /// true if the control bar should automatically resize its height
  /// to fit all components
  bool size_to_fit_vert;

  // true if the components are to be layed out in a column rather than row
  bool vertical;

  // true if items should be stretched perpendicular to the row direction
  bool stretch_items;

  // layout the child components and resize this component if necessary
  void DoLayout ();

 public:

  awsControlBar ();
  virtual ~awsControlBar ();

  virtual bool Setup (iAws *_wmgr, awsComponentNode *settings);
  virtual char *Type (){return "Control Bar";}

  bool Execute(char* action, iAwsParmList &parmlist);

  /// Adds a component to the bar
  void AddChild(iAwsComponent *comp);

  /// This will remove the component from the bar
  void RemoveChild(iAwsComponent *comp);

  /// Returns true if the component resizes width to fit the controls when they change
  bool GetSizeToFitHorz();

  /// Returns true if the component resizes height to fit the controls when they change
  bool GetSizeToFitVert();

  /// Sets whether the component resizes width to fit the controls when they change
  void SetSizeToFitHorz(bool b);
 
  /// Sets whether the component resizes height to fit the controls when they change
  void SetSizeToFitVert(bool b);

  /// Resizes the component to fit the controls snuggly
  void SizeToFit();

  /// Resizes the component to fit the width of the controls snuggly
  void SizeToFitHorz();

  /// Resizes the component to fit the height of the controls snuggly
  void SizeToFitVert();

  bool GetVertical();
  void SetVertical(bool vertical);

  int GetAlignment();
  void SetAlignment(int alignment);

  int GetHorzGap();
  void SetHorzGap(int gap);

  int GetVertGap();
  void SetVertGap(int gap);

  bool GetStretchComponents();
  void SetStretchComponents(bool stretch);

  static const int alignTop;
  static const int alignBottom;
  static const int alignCenter;
};

class awsControlBarFactory : public awsComponentFactory
{
public:
  awsControlBarFactory(iAws* _wmgr);
  
  iAwsComponent* Create();
};

#endif // !defined(AFX_AWSCONTROLBAR_H__610A57D1_4F71_4AF3_A31F_23D1445E4AB1__INCLUDED_)
