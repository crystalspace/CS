#ifndef __CT_CATMANAGER__
#define __CT_CATMANAGER__

#include "csphyzik/phyziks.h"

// this class provides an interface to sources of discontinuity
// in the physical simulation. i.e. collision detection etc...
// Register it with ctWorld and 
// check_cata- will be called each frame and handle_cat
// will be called if check_cata- return > 0.
class ctCatastropheManager
{
public:

  // check for a catastrophe and return a real indicating the "magnitude"
  // of the worst ( bigger number ) catastrophe.  Return 0 for no catastrophe
  virtual real check_catastrophe() = 0;
  
  // take care of the catastrophe so that when integrated forward that
  // catasrophe will not exist.
  virtual void handle_catastrophe() = 0;

};

#endif
