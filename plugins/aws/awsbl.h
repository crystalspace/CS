#ifndef __AWS_BORDER_LAYOUT__
#define __AWS_BORDER_LAYOUT__

#include "awslayot.h"

struct iAwsComponent;


/// Lays out a class according to the Java AWT/Swing BorderLayout defintion.
class awsBorderLayout : public awsLayoutManager
{
  /// Contains all components, there may be one or more missing.
  iAwsComponent *components[5];

  /// The horizontal gap for components
  int hGap;

  /// The vertical gap for components
  int vGap;

public:
  enum
  {
    /**
     * Put the component in the center of its display area.
     */
    GBS_CENTER    = 0,

    /**
     * Put the component at the top of its display area,
     * centered horizontally. 
     */
    GBS_NORTH     = 1,

    
    /**
     * Put the component on the right side of its display area, 
     * centered vertically.
     */
    GBS_EAST      = 2,

    
    /**
     * Put the component at the bottom of its display area, centered 
     * horizontally. 
     */
    GBS_SOUTH     = 3,

    
    /**
     * Put the component on the left side of its display area, 
     * centered vertically.
     */
    GBS_WEST      = 4,
  };


public:
  /// Constructor, clears all components to NULL
  awsBorderLayout(iAwsComponent *owner, 
		  iAwsPrefManager *pm,
                  awsComponentNode *settings);

  /// Empty destructor
  virtual ~awsBorderLayout() {}

  /** Adds a component to the layout, returning it's actual rect.  This function needs
    *  to get some more info from the key node, so that is passed in as well as the
    *  preference manager to decode the information.
    */
  virtual csRect AddComponent (
                  iAwsPrefManager *pm,
                  awsComponentNode *settings,
                  iAwsComponent *cmp);

  /// Lays out components properly
  virtual void LayoutComponents ();
};


#endif