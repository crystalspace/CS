/////////////////////////////////////////////////////////////////////////////
// Copyright:   (c) Aleksandras Gluchovas
// Licence:     wxWindows license
//
// Ported to Crystal Space by Norman Krämer <norman@users.sourceforge.net>
/////////////////////////////////////////////////////////////////////////////

#ifndef __FLOWLAYOUT_G__
#define __FLOWLAYOUT_G__

#include "cslayout.h"

/**
 * In a flow layout components are displayed in a row
 * and wraped at parents boundaries.
 */
class csFlowLayout : public csLayout
{
protected:
  int mAlign;
  int mHgap;
  int mVgap;

  csPoint mPrefDimOfPhase1;
public:
  csFlowLayout (csComponent *pParent);
  csFlowLayout (csComponent *pParent, int align);
  csFlowLayout (csComponent *pParent, int align, int hgap, int vgap);

  int GetAlignment ();
  int GetHgap ();
  int GetVgap ();

  void SetAlignment (int align);
  void SetHgap (int hgap);
  void SetVgap (int vgap);

  // impl. of LayoutManager interface

  virtual void SuggestSize (int &sugw, int& sugh);

  virtual void LayoutContainer ();

public: 

  enum ALIGNMNET_ENUM
  {
    CENTER   = 0,
    LEFT     = 1,
    RIGHT    = 2,

    LEADING  = LEFT,  // for Western, Europian text-orientation
    TRAILING = RIGHT  // -/-
  };
};

#endif // __FLOWLAYOUT_G__
