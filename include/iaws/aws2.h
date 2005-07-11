#ifndef __AWS_INTERFACE_20_H__
#define __AWS_INTERFACE_20_H__

#ifdef __AWS_INTERFACE_10_H__
# error "aws.h included before aws2.h.  You cannot mix and match the two versions!"
#endif

/**\file 
 * Advanced Windowing System
 */

#include "csutil/scf.h"
#include "csutil/refarr.h"
#include "csutil/stringarray.h"
#include "csutil/scfstr.h"
#include "csgeom/csrect.h"
#include "csgeom/cspoint.h"
#include "iutil/event.h"
#include "iutil/string.h"


namespace autom
{
  class string;
  class integer;
  class floating;

  SCF_VERSION(iObject, 1, 0, 1);

  /** Base class for all Keila objects. */
  struct iObject 
  {    
      enum TYPE { T_STRING, T_INT, T_FLOAT, T_LIST, T_MAP, T_FUNCTION, T_REFERENCE, T_BLOB, T_NIL };

      /** Returns the type of the object, a member of the object::TYPE enumeration. */
      virtual TYPE ObjectType()=0;
      
      /** Sets the name of the object. */
      virtual void setName(const scfString &_name)=0;
      
      /** Gets the name of the object. */
      virtual const scfString& getName()=0;
      
      /** Converts the object into a string object if possible. */
      virtual string toString()=0;
      
      /** Converts the object into an integer object, if possible. */
      virtual integer toInt()=0;
      
      /** Converts the object into a float object, if possible. */
      virtual floating toFloat()=0;	
      
      /** Converts the object into the text representation of it. This is the inverse of parsing. */
      virtual scfString reprObject()=0;      
  };

} // end autom namespace

SCF_VERSION(iAwsWindow, 1, 0, 1);
struct iAwsWindow : public iBase
{
  int empty;

};

SCF_VERSION(iAws, 1, 0, 1);
struct iAws  : public iBase
{

  /// Dispatches events to the proper components.
  virtual bool HandleEvent (iEvent &)=0;  

  /// 
};


#endif
