/////////////////////////////////////////////////////////////////////////////
// Copyright:   (c) Aleksandras Gluchovas
// Licence:     wxWindows license
//
// Ported to Crystal Space by Norman Krämer <norman@users.sourceforge.net>
/////////////////////////////////////////////////////////////////////////////

#ifndef __GRIDBAGLAYOUT_G__
#define __GRIDBAGLAYOUT_G__

#include "cslayout.h"

/**
 * This is a gridbag layout. For an extensive documentation look at GridBagLayout
 * in the java documentation.
 */

class csGridBagConstraint : public csLayoutConstraint
{
public:
  csGridBagConstraint (csComponent *comp);
  csGridBagConstraint (const csGridBagConstraint &c);
  csGridBagConstraint (csComponent *comp, int _gridx, int _gridy,
    int _gridwidth, int _gridheight, float _weightx, float _weighty,
    int _anchor, int _fill, csRect _insets, int _ipadx, int _ipady);
  csLayoutConstraint *Clone ();
public:
  int gridx;
  int gridy;
  int gridwidth;
  int gridheight;
  float weightx;
  float weighty;
  int anchor;
  int fill;
  csRect insets;
  int ipadx;
  int ipady;

  enum Bsdfas
  {
    Afsds  = 5,
    asdfd = 44
  };

#undef RELATIVE
#undef _LEFT
#undef _CENTER

  enum GRID_BAG_CONSTANTS
  {
    RELATIVE   = -1,
    REMAINDER  = 0,

    NONE       = 0,
    BOTH       = 1,
    HORIZONTAL = 2,
    VERTICAL   = 3,

    CENTER    = 10,
    NORTH     = 11,
    NORTHEAST = 12,
    EAST      = 13,
    SOUTHEAST = 14,
    SOUTH     = 15,
    SOUTHWEST = 16,
    WEST      = 17,
    NORTHWEST = 18,
  };

  // used interally  (do not use!)

  enum  GRID_BAG_CONSTANTS_INTERNAL
  {
    _LEFT     = 20,
    _CENTER   = 21,
    _RIGHT    = 22
  };

  // for internal uses
  bool bSized;
  csPoint mPrefCompSize;
};

struct CellInfo
{
  csComponent* comp;

  int prefSize;	 // actually, it can be calculated on-the-fly
  int prefCompSize; 

  int cellSpan;
  int leftInset;
  int rightInset;
  int pad;
  int fill;
  int anchor;

  float extraSpace;
  float weight;

  int finalSize;
  int finalPos;
  int finalCompSize;
  int finalCompPos;
};

struct CellHolder
{
  csGridBagConstraint* constr;
  float weightx;
  float weighty;

  int gridwidth;
  int gridheight;
  int actualWidth;
  int actualHeight;

  bool isFirstCellForComp;
  int x, y;
};

typedef CellInfo& (*CellGetterFnctT)(int,int);

DECLARE_TYPED_VECTOR(CellHolderArrayT, CellHolder);

class csGridBagLayout : public csLayout2
{
public:
  csGridBagLayout (csComponent *pParent);
  ~csGridBagLayout ();

  virtual void RemoveLayoutComponent (csComponent* comp);
  virtual void SuggestSize (int &sugw, int &sugh);
  virtual void LayoutContainer ();

  virtual void MaximumLayoutSize (int &w, int &h);
  virtual float GetLayoutAlignmentX ();
  virtual float GetLayoutAlignmentY ();

  csGridBagConstraint c;

protected:
  int CalcPrefSize (CellInfo* cells, int xCnt, int yCnt, int _arrayWidth);
  void LayoutCells (CellInfo* cells, int xCnt, int yCnt, 
		    int outterSize, int outterPos, int _arrayWidth );

  void InitializeCellArray (CellInfo* cells, int size);
  void InitCellFromHolder (CellHolder& holder);
  void CreateMatrix ();
  long GetCellCode (int x, int y);
  void CleanUpConstraints ();
  void ClearCachedData ();
  bool HasCashedInfo ();
  void SetComponentLocations ();

protected:
  CellInfo* mpHorizCellInfos;
  CellInfo* mpVertCellInfos;
  int mColCount;
  int mRowCount;
};

#endif // __GRIDBAGLAYOUT_G__
