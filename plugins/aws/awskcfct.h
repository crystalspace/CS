/*
    Copyright (C) 2001 by Christopher Nelson

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_AWS_KCFCT_H__
#define __CS_AWS_KCFCT_H__

#include "iaws/aws.h"
#include "iutil/strset.h"
#include "awsprefs.h"

/**
 * Allows you to manually create a hierarchical arrangement of keys to
 * deliver to the preference manager or to a component that you want to
 * stick inside your own. Note that the base component delviered to the
 * preference manager MUST be a window. Another important thing: the script
 * loader does some error checking to make sure that some of the values
 * are sane. This code does NOT do those checks. Using this will allow
 * you to destroy yourself.
 */
class awsKeyFactory : public iAwsKeyFactory
{
private:
  /**
   * Base container. Normally the base container is a window, but this can
   * start at any level of the hierarchy. If it's not a window, then it
   * must be added to a window eventually to be useful.
   */
  csRef<iAwsComponentNode> base;
  /// Aws manager.
  iAws* wmgr;	// Not a csRef<>; avoid circular references.
public:
  SCF_DECLARE_IBASE;

  awsKeyFactory (iAws* a);
  virtual ~awsKeyFactory ();

  /**
   * Initializes the factory, name is the name of this component,
   * component type is it's type.
   */
  virtual void Initialize (const char* name, const char* component_type);

  /// Adds this factory's base to the window manager IF the base is a window.
  virtual void AddToWindowList (iAwsPrefManager *pm);

  /// Adds the given factory's base in as a child of this factory.
  virtual void AddFactory (iAwsKeyFactory *factory);

  /// Add an integer key.
  virtual void AddIntKey (const char* name, int v);

  /// Add a string key.
  virtual void AddStringKey (const char* name, const char* v);

  /// Add a rect key.
  virtual void AddRectKey (const char* name, csRect v);

  /// Add an RGB key.
  virtual void AddRGBKey (
    const char *name,
    unsigned char r,
    unsigned char g,
    unsigned char b);

  /// Add a point key.
  virtual void AddPointKey (const char* name, csPoint v);

  /// Add a connection node.
  virtual void AddConnectionNode (iAwsConnectionNodeFactory *node);

  /// Add a connection key.
  virtual void AddConnectionKey (
    const char* name,
    iAwsSink *s,
    unsigned long t,
    unsigned long sig);

  /// Get the base node.
  iAwsComponentNode *GetThisNode ();
};

class awsConnectionNodeFactory : public iAwsConnectionNodeFactory
{
private:
  /**
   * Connection container. All connection keys get added to this, then
   * it is added to a component to be useful.
   */
  awsConnectionNode *base;

  /// This is true if we cannot delete the base when we go.
  bool base_in_use;

  /// Aws manager.
  iAws* wmgr;	// Not a csRef<>; avoid circular references.
public:
  SCF_DECLARE_IBASE;

  awsConnectionNodeFactory (iAws*);
  virtual ~awsConnectionNodeFactory ();

  /// Initializes the factory.
  virtual void Initialize ();

  /// Add a connection key.
  virtual void AddConnectionKey (
    const char* name,
    iAwsSink *s,
    unsigned long t,
    unsigned long sig);

  /// Get the base node.
  awsConnectionNode *GetThisNode ();

  friend class awsKeyFactory;
 };

#endif // __CS_AWS_KCFCT_H__
