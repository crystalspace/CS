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

#ifndef __ICFGMGR_H__
#define __ICFGMGR_H__

#include "csutil/scf.h"
#include "icfgnew.h"

SCF_VERSION(iConfigManager, 0, 0, 1);

/**
 * The config manager is used to make a number of config file objects look
 * like a single object. To do this, every config file object (also called
 * 'domain') is assigned a priority value. Options from config files with
 * higher priority override options from config files with lower priority.
 * These 'lower' options are not really deleted, so if you access the 'lower'
 * config file directly you will still find their real value. If two config
 * objects use the same priority value, it is not defined which of them takes
 * precedence. <p>
 *
 * One config object is the so-called 'dynamic' object. When you change
 * something in the global configuration, these changes are applied to the
 * dynamic config object. The changed key is removed from all objects with
 * higher priority. The dynamic object is given to the config manager at
 * construction and cannot be changed later. It has always priority 0
 * (medium). <p>
 *
 * Differences in behaviour compared to a normal confguration object are:
 * <ul>
 * <li> Deleting a key will not always remove the key from the configuration
 *      completely. It will only remove the key from the dyamic config object
 *      and all higher-priority objects and thus reveal the lower-priority
 *      value. This also applies to the Clear() method.
 * <li> The Load() and Save() methods will load or save the configuration of
 *      the dynamic domain. The other domains are not affected by Load(), and
 *      Save() won't write any keys from other domains.
 * <li> Iterators: If you change an option after an iterator has passed this
 *      option, it may appear again, this time with the new value. If you
 *      change the option while the iterator looks at it, you may even read
 *      it twice after this change, once with the old and once with the new
 *      value. In general it's a bad idea to change something while an
 *      iterator exists.
 * </ul>
 */
struct iConfigManager : public iConfigFileNew
{
  enum
  {
    PriorityMin =       -1000000000,
    PriorityVeryLow =   -100,
    PriorityLow =       -50,
    PriorityMedium =    0,
    PriorityHigh =      +50,
    PriorityVeryHigh =  +100,
    PriorityMax =       +1000000000,
  };

  /// add a configuration domain
  virtual void AddDomain(iConfigFileNew*, int priority) = 0;
  /// add a configuration domain
  virtual void AddDomain(char const* path, iVFS*, int priority) = 0;
  /// remove a configuration domain
  virtual void RemoveDomain(iConfigFileNew*) = 0;
  /// remove a configuration domain
  virtual void RemoveDomain(char const* path, iVFS*) = 0;
  /// return a pointer to a single config domain
  virtual iConfigFileNew* LookupDomain(char const* path, iVFS*) const = 0;
  /// set the priority of a config domain
  virtual void SetDomainPriority(char const* path, iVFS*, int priority) = 0;
  /// set the priority of a config domain
  virtual void SetDomainPriority(iConfigFileNew*, int priority) = 0;
  /// return the priority of a config domain
  virtual int GetDomainPriority(char const* path, iVFS*) const = 0;
  /// return the priority of a config domain
  virtual int GetDomainPriority(iConfigFileNew*) const = 0;

  /// return a pointer to the dynamic config domain
  virtual iConfigFileNew *GetDynamicDomain() const = 0;
  /// set the priority of the dynamic config domain
  virtual void SetDynamicDomainPriority(int priority) = 0;
  /// return the priority of the dynamic config domain
  virtual int GetDynamicDomainPriority() const = 0;
};

#endif // __ICFGMGR_H__
