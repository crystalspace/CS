#ifndef __AWS_INTERFACE_H__
#define __AWS_INTERFACE_H__

/**\file 
 * Advanced Windowing System
 */

#include "csutil/scf.h"
#include "csutil/refarr.h"
#include "csutil/stringarray.h"
#include "csgeom/csrect.h"
#include "csgeom/cspoint.h"
#include "iutil/event.h"
#include "iutil/string.h"


SCF_VERSION(iAwsWindow, 1, 0, 1);
struct iAwsWindow : public iBase
{
  int empty;

};

SCF_VERSION(iAws, 1, 0, 1);
struct iAws  : public iBase
{
  int empty;
  
};


#endif
