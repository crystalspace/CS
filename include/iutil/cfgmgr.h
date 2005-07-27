/*
    Copyright (C) 2001 by Martin Geisse <mgeisse@gmx.net>

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

#ifndef __CS_IUTIL_CFGMGR_H__
#define __CS_IUTIL_CFGMGR_H__

/**\file
 */
/**\addtogroup util
 * @{ */
#include "csutil/scf.h"

#include "iutil/cfgfile.h"

struct iVFS;

SCF_VERSION(iConfigManager, 0, 0, 3);

/**
 * The configuration manager is used to make a number of iConfigFile object
 * appear like a single object. To do this, each iConfigFile object (also
 * called a 'domain') is assigned a priority value. Options from config files
 * with higher priority override or shadow options from configuration objects
 * with lower priority.  The lower priority options are still present, so if
 * you access the lower priority iConfigFile directly you will still find their
 * real values. If two iConfigFile objects use the same priority value, then
 * one will shadow the other (but it is not possible to predict which will be
 * the victor).<p>
 *
 * One iConfigFile object is the so-called 'dynamic' domain.  When you alter a
 * setting in the configuration manager, the change is applied to the dynamic
 * iConfigFile object.  As a side-effect, the changed key is also removed from
 * all objects with higher priority.  The dynamic domain has always priority 0
 * (medium).<p>
 *
 * Differences in behaviour compared to a normal configuration object are:
 * <ul>
 * <li> Deleting a key will not always remove the key from the configuration
 *      completely. It will only remove the key from the dyamic iConfigFile
 *      object and all higher-priority objects; and will thus reveal a value in
 *      a lower priority domain, if present.  This also applies to the Clear()
 *      method.
 * <li> The Load() and Save() methods will load or save the configuration of
 *      the dynamic domain. The other domains are not affected by Load(); and
 *      Save() will not write any keys from other domains.  (In the unlikely
 *      event that you need to load or save one of the other domains, simply
 *      access the iConfigFile object for that domain directly and invoke its
 *      Load() and Save() methods rather than the methods of iConfigManager.)
 * <li> Iterators: If you change an option after an iterator has passed the
 *      option, it may appear again, this time with the new value. If you
 *      change the option while the iterator looks at it, you may even read
 *      it twice after this change, once with the old and once with the new
 *      value. In general it is a bad idea to change something while an
 *      iterator exists.
 * </ul>
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>csInitializer::CreateEnvironment()
 *   <li>csInitializer::CreateConfigManager()
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>CS_QUERY_REGISTRY()
 *   </ul>
 */
struct iConfigManager : public iConfigFile
{
  enum
  {
    PriorityMin =       -1000000000,
    PriorityVeryLow =   -100,
    PriorityLow =       -50,
    PriorityMedium =    0,
    PriorityHigh =      50,
    PriorityVeryHigh =  100,
    PriorityMax =       1000000000
  };

  /// Default priority values (you may use other values if you want)
  enum
  {
    // plug-in priority
    ConfigPriorityPlugin        = PriorityVeryLow,
    // application priority
    ConfigPriorityApplication   = PriorityLow,
    // user priority (application-neutral)
    ConfigPriorityUserGlobal    = PriorityMedium,
    // user priority (application-specific)
    ConfigPriorityUserApp       = PriorityHigh,
    // command-line priority
    ConfigPriorityCmdLine       = PriorityVeryHigh
  };

  /**
   * Add a configuration domain.  The configuration manager invokes IncRef()
   * upon the incoming iConfigFile.
   */
  virtual void AddDomain(iConfigFile*, int priority) = 0;
  /**
   * Add a configuration domain by loading it from a file.  The new iConfigFile
   * object which represents the loaded file is also returned.  If you want to
   * hold onto the iConfigFile even after it is removed from this object or
   * after the configuration manager is destroyed, be sure to invoke IncRef()
   * or assign it to a csRef<>.  The incoming iVFS* may be null, in which case
   * the path is assumed to point at a file in the pyhysical filesystem, rather
   * than at a file in the virtual filesystem.
   */
  virtual iConfigFile* AddDomain(char const* path, iVFS*, int priority) = 0;
  /**
   * Remove a configuration domain.  If registered, the configuration manager
   * will relinquish its reference to the domain by invoking DecRef() on it to
   * balance the IncRef() it performed when the domain was added.  If the
   * domain is not registered, the RemoveDomain() request is ignored.  It is
   * not legal to remove the dynamic domain.
   */
  virtual void RemoveDomain(iConfigFile*) = 0;
  /// Remove a configuration domain.
  virtual void RemoveDomain(char const* path) = 0;
  /**
   * Find the iConfigFile object for a registered domain.  Returns null if the
   * domain is not registered.
   */
  virtual iConfigFile* LookupDomain(char const* path) const = 0;
  /// Set the priority of a configuration domain.
  virtual void SetDomainPriority(char const* path, int priority) = 0;
  /**
   * Set the priority of a registered configuration domain.  If the domain is
   * not registered, the request is ignored.
   */
  virtual void SetDomainPriority(iConfigFile*, int priority) = 0;
  /**
   * Return the priority of a configuration domain.  If the domain is not
   * registered, PriorityMedium is returned.
   */
  virtual int GetDomainPriority(char const* path) const = 0;
  /**
   * Return the priority of a configuration domain.  If the domain is not
   * registered, PriorityMedium is returned.
   */
  virtual int GetDomainPriority(iConfigFile*) const = 0;

  /**
   * Change the dynamic domain.  The domain must already have been registered
   * with AddDomain() before calling this method.  If the domain is not
   * registered, then false is returned.
   */
  virtual bool SetDynamicDomain(iConfigFile*) = 0;
  /** 
   * Return a pointer to the dynamic configuration domain.  The returned
   * pointer will remain valid as long as the domain is registered with the
   * configuration manager.
   */
  virtual iConfigFile* GetDynamicDomain() const = 0;
  /// Set the priority of the dynamic configuration domain.
  virtual void SetDynamicDomainPriority(int priority) = 0;
  /// Return the priority of the dynamic configuration domain.
  virtual int GetDynamicDomainPriority() const = 0;

  /// Flush all removed configuration files (only required in optimize mode).
  virtual void FlushRemoved() = 0;
};
/** @} */

#endif // __CS_IUTIL_CFGMGR_H__
