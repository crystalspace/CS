/*********************************************************************************
 * (C)opyright 2001 Christopher Nelson
 *
 *  The point of this tool is to allow people to instantiate controls in code, in effect
 * to bypass the definitions files.  This may be useful in some instances, and since it's
 * been requested, I've decided to make it available.
 */
#ifndef __AWS_KEY_FACTORY__
# define __AWS_KEY_FACTORY__

# include "iaws/aws.h"
# include "awsprefs.h"

/**   Allows you to manually create a hierarchical arrangement of keys to deliver to the
 *  preference manager or to a component that you want to stick inside your own.  Note that
 *  the base component delviered to the preference manager MUST be a window.
 *    Another important thing: the script loader does some error checking to make sure that
 *  some of the values are sane. This code does NOT do those checks.  Using this will allow
 *  you to destroy yourself.  Have fun.
 */
class awsKeyFactory : public iAwsKeyFactory
{
  /** Base container.  Normally the base container is a window, but this can start at any level
   *  of the hierarchy.  If it's not a window, then it must be added to a window eventually to be
   *  useful.
   */
  iAwsComponentNode *base;

public:
  SCF_DECLARE_IBASE;

  awsKeyFactory ();
  virtual ~awsKeyFactory ();

  /// Initializes the factory , name is the name of this component, component type is it's type.
  virtual void Initialize (const char* name, const char* component_type);

  /// Adds this factory's base to the window manager IF the base is a window
  virtual void AddToWindowList (iAwsPrefManager *pm);

  /// Adds the given factory's base in as a child of this factory.
  virtual void AddFactory (iAwsKeyFactory *factory);

  /// Add an integer key
  virtual void AddIntKey (const char* name, int v);

  /// Add a string key
  virtual void AddStringKey (const char* name, const char* v);

  /// Add a rect key
  virtual void AddRectKey (const char* name, csRect v);

  /// Add an RGB key
  virtual void AddRGBKey (
                const char *name,
                unsigned char r,
                unsigned char g,
                unsigned char b);

  /// Add a point key
  virtual void AddPointKey (const char* name, csPoint v);

  /// Add a connection node
  virtual void AddConnectionNode (iAwsConnectionNodeFactory *node);

  /// Add a connection key
  virtual void AddConnectionKey (
                const char* name,
                iAwsSink *s,
                unsigned long t,
                unsigned long sig);

  /// Get the base node
  iAwsComponentNode *GetThisNode ();
};

class awsConnectionNodeFactory : public iAwsConnectionNodeFactory
{
  /** Connection container. All connection keys get added to this, then
   *  it is added to a component to be useful.
   */
  awsConnectionNode *base;

  /// This is true if we canNOT delete the base when we go.
  bool base_in_use;
public:
  SCF_DECLARE_IBASE;

  awsConnectionNodeFactory ();
  virtual ~awsConnectionNodeFactory ();

  /// Initializes the factory
  virtual void Initialize ();

  /// Add a connection key
  virtual void AddConnectionKey (
                const char* name,
                iAwsSink *s,
                unsigned long t,
                unsigned long sig);

  /// Get the base node
  awsConnectionNode *GetThisNode ();

  friend class awsKeyFactory;
 };

#endif
