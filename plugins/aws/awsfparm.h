#ifndef __AWS_FLEXIBLE_PARAMETER_LIST__
#define __AWS_FLEXIBLE_PARAMETER_LIST__

#include "iaws/iawsparm.h"
#include "csgeom/csrect.h"
#include "csgeom/cspoint.h"
#include "csutil/scfstr.h"
#include "csutil/csvector.h"
#include "csutil/csstrvec.h"

/***********************************************************************************
 * Provides support for safely passing named parameters through to different functions
 * in a portable manner.  Note that awsParmList does not utilize copy semantics.  In
 * the interests of space and speed, it simply takes a reference to the pointers passed
 * in.  This means that you should NOT use an awsParmList if any parm it references
 * has gone out of scope!
 ***********************************************************************************/
class awsParmList : public iAwsParmList
{
  csBasicVector parms;

public:
  static const int INT;    
  static const int FLOAT;
  static const int STRING;   
  static const int BASICVECTOR;
  static const int STRINGVECTOR;
  static const int RECT;
  static const int POINT;
  static const int BOOL;

  struct parmItem
  {
    int  type;
    unsigned long name;
    union parmValue 
    {
      int i;
      float f;
      bool b;
      iString *s;
      csBasicVector *bv;
      csStrVector *sv;
      csRect *r;
      csPoint *p;
    } parm;
  };

private:
  parmItem *FindParm(char *name, int type);

public:
  awsParmList();
  virtual ~awsParmList();

  SCF_DECLARE_IBASE;
  ////////////////////

  /// Adds an integer to the parmeter list
  virtual void AddInt(char *name, int value);
  /// Adds a float to the parmeter list
  virtual void AddFloat(char *name, float value);
  /// Adds a bool to the parmeter list
  virtual void AddBool(char *name, bool value);
  /// Adds a string to the parmeter list
  virtual void AddString(char *name, iString* value);
  /// Adds a vector to the parmeter list
  virtual void AddBasicVector(char *name, csBasicVector* value);
  /// Adds a string vector to the parmeter list
  virtual void AddStringVector(char *name, csStrVector* value);
  /// Adds a rect to the parmeter list
  virtual void AddRect(char *name, csRect *value);
  /// Adds a point to the parmeter list
  virtual void AddPoint(char *name, csPoint *value);

  /// Returns the int named "name" in value.  True if it was found, otherwise false.
  virtual bool GetInt(char *name, int *value);
  /// Returns the float named "name" in value.  True if it was found, otherwise false.
  virtual bool GetFloat(char *name, float *value);
  /// Returns the bool named "name" in value.  True if it was found, otherwise false.
  virtual bool GetBool(char *name, bool *value);
  /// Returns the string named "name" in value.  True if it was found, otherwise false.
  virtual bool GetString(char *name, iString **value);
  /// Returns the basic vector named "name" in value.  True if it was found, otherwise false.
  virtual bool GetBasicVector(char *name, csBasicVector **value);
  /// Returns the string vector named "name" in value.  True if it was found, otherwise false.
  virtual bool GetStringVector(char *name, csStrVector **value);
  /// Returns the rect named "name" in value.  True if it was found, otherwise false.
  virtual bool GetRect(char *name, csRect **value);
  /// Returns the point named "name" in value.  True if it was found, otherwise false.
  virtual bool GetPoint(char *name, csPoint **value);

  /// Clears the parameter list
  virtual void Clear();
};

#endif