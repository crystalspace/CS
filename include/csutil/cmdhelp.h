/*
    Copyright (C) 2001 by Jorrit Tyberghein

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

#ifndef __CS_CMDHELP_H__
#define __CS_CMDHELP_H__

/**\file
 * Command line parsing helper
 */

#include "csextern.h"

struct iObjectRegistry;
struct iCommandLineParser;
struct iConfig;

/**
 * This class can be used to help parsing the commandline. One of the
 * functionalities it gives is the ability to have commandline help.
 * To do that it will look at all plugins and see if they implement iConfig.
 * This class requires the iPluginManager and iCommandLineParser to be
 * in the object registry (or else you give it as a parameter).
 */
class CS_CRYSTALSPACE_EXPORT csCommandLineHelper
{
private:
  static void Help (iConfig* config);

public:
  /**
   * Ask for Help. This function will first send a broadcast message
   * of type cscmdCommandLineHelp and then it will check all plugins and
   * see if they implement iConfig.
   * If the commandline parser is not given then the default commandline
   * parser from the registry will be used.
   */
  static void Help (iObjectRegistry* object_reg,
  	iCommandLineParser* cmdline = 0);

  /**
   * Check if -help is given on the commandline and return true.
   */
  static bool CheckHelp (iObjectRegistry* object_reg,
  	iCommandLineParser* cmdline = 0);
};

#endif // __CS_CMDHELP_H__

