/*********************************************************************************
 * (C)opyright 2001 Christopher Nelson
 *
 *  The point of this tool is to allow people to instantiate controls in code, in effect
 * to bypass the definitions files.  This may be useful in some instances, and since it's
 * been requested, I've decided to make it available.
 */                         

#ifndef __AWS_KEY_FACTORY__
#define __AWS_KEY_FACTORY__
   
#include "iaws/aws.h"
#include "awsprefs.h"
           
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
  awsComponentNode *base;

  /// This is true if we canNOT delete the base when we go.
  bool base_in_use;

public:
   SCF_DECLARE_IBASE;

   awsKeyFactory();
   virtual ~awsKeyFactory();
  
   /// Initializes the factory , name is the name of this component, component type is it's type.
   virtual void Initialize(iString *name, iString *component_type);
   /// Adds this factory's base to the window manager IF the base is a window
   virtual void AddToWindowList(iAwsPrefManager *pm);
   /// Adds the given factory's base in as a child of this factory.
   virtual void AddFactory(iAwsKeyFactory *factory);
   /// Add an integer key
   virtual void AddIntKey(iString *name, int v);
   /// Add a string key
   virtual void AddStringKey(iString *name, iString *v);
   /// Add a rect key
   virtual void AddRectKey(iString *name, csRect v);
   /// Add an RGB key
   virtual void AddRGBKey(iString *name, unsigned char r, unsigned char g, unsigned char b);
   /// Add a point key
   virtual void AddPointKey(iString *name, csPoint v);
   /// Add a connection key
   virtual void AddConnectionKey(iString *name, iAwsSink *s, unsigned long t, unsigned long sig);
   /// Get the base node
   awsComponentNode *GetThisNode();
   
};

#endif

