#ifndef __CS_AWS_FLEXIBLE_PARAMETER_LIST_H__
#define __CS_AWS_FLEXIBLE_PARAMETER_LIST_H__

# include "iaws/awsparm.h"
# include "csgeom/csrect.h"
# include "csgeom/cspoint.h"
# include "csutil/scfstr.h"
# include "csutil/csvector.h"
# include "csutil/csstrvec.h"

/***********************************************************************************
 * Provides support for safely passing named parameters through to different functions
 * in a portable manner.  Note that awsParmList does not utilize copy semantics.  In
 * the interests of space and speed, it simply takes a reference to the pointers passed
 * in.  This means that you should NOT use an awsParmList if any parm it references
 * has gone out of scope!
 ***********************************************************************************/
class awsParmList :
  public iAwsParmList
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
  static const int VOPAQUE;

  struct parmItem
  {
    int type;
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
      void *v;
    } parm;
  };
private:
  parmItem *FindParm (const char *name, int type);
public:
  awsParmList ();
  virtual ~awsParmList ();

  SCF_DECLARE_IBASE;

  ////////////////////
  //
  //  /// Adds an integer to the parmeter list
  virtual void AddInt (const char *name, int value);

  /// Adds a float to the parmeter list
  virtual void AddFloat (const char *name, float value);

  /// Adds a bool to the parmeter list
  virtual void AddBool (const char *name, bool value);

  /// Adds a string to the parmeter list
  virtual void AddString (const char *name, const char* value);

  /// Adds a vector to the parmeter list
  virtual void AddBasicVector (const char *name, csBasicVector *value);

  /// Adds a string vector to the parmeter list
  virtual void AddStringVector (const char *name, csStrVector *value);

  /// Adds a rect to the parmeter list
  virtual void AddRect (const char *name, csRect *value);

  /// Adds a point to the parmeter list
  virtual void AddPoint (const char *name, csPoint *value);

  /** Adds an opaque, undefined value to the parm list. This is stored as a void *, but
   * should never be assumed to be anything at all, except some value that fits in
   * sizeof(void *)
   */
  virtual void AddOpaque (const char *name, void *value);

  /// Returns the int named "name" in value.  True if it was found, otherwise false.
  virtual bool GetInt (const char *name, int *value);

  /// Returns the float named "name" in value.  True if it was found, otherwise false.
  virtual bool GetFloat (const char *name, float *value);

  /// Returns the bool named "name" in value.  True if it was found, otherwise false.
  virtual bool GetBool (const char *name, bool *value);

  /// Returns the string named "name" in value.  True if it was found, otherwise false.
  virtual bool GetString (const char *name, iString **value);

  /// Returns the basic vector named "name" in value.  True if it was found, otherwise false.
  virtual bool GetBasicVector (const char *name, csBasicVector **value);

  /// Returns the string vector named "name" in value.  True if it was found, otherwise false.
  virtual bool GetStringVector (const char *name, csStrVector **value);

  /// Returns the rect named "name" in value.  True if it was found, otherwise false.
  virtual bool GetRect (const char *name, csRect **value);

  /// Returns the point named "name" in value.  True if it was found, otherwise false.
  virtual bool GetPoint (const char *name, csPoint **value);

  /// Returns the opaque value named "name" in value.  True if it was found, otherwise false.
  virtual bool GetOpaque (const char *name, void **value);

  /// Clears the parameter list
  virtual void Clear ();
};
#endif // __CS_AWS_FLEXIBLE_PARAMETER_LIST_H__
