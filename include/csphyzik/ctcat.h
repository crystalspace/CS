#ifndef __CT_CATMANAGER__
#define __CT_CATMANAGER__

#include "csphyzik/phyztype.h"

#define CAT_DEFAULT_EPSILON 0.01

/**
 * this class provides an interface to sources of discontinuity
 * in the physical simulation. i.e. collision detection etc...
 * Register it with ctWorld and
 * check_cata- will be called each frame and handle_cat
 * will be called if check_cata- return > 0.
 */

class ctCatastropheManager
{
public:

  ctCatastropheManager(){ cat_epsilon = CAT_DEFAULT_EPSILON; }

  /**
   * check for a catastrophe and return a real indicating the "magnitude"
   * of the worst ( bigger number ) catastrophe.  Return 0 for no catastrophe
   */
  virtual real check_catastrophe() = 0;

  /**
   * take care of the catastrophe so that when integrated forward that
   * catasrophe will not exist.
   */
  virtual void handle_catastrophe() = 0;

  /**
   * Return epsilon value such that anything less than epsilon is treated
   * as 0 for the catastrophe value ( and thus there is no catastrophe )
   */
  virtual real get_epsilon(){ return cat_epsilon; }

protected:
  real cat_epsilon;

};

#endif
