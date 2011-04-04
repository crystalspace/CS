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
 * Printing of command line help
 */

#include "csextern.h"

#include "csutil/stringarray.h"
#include "iutil/pluginconfig.h"

struct iObjectRegistry;
struct iCommandLineParser;

/**
 * This class can be used to print the commandline help, eg when the '-help' option
 * has been used.
 *
 * A first functionality provided is the ability to print the options of all the
 * currently loaded plugins, iff they implement iPluginConfig. Use Help() for this.
 *
 * A more advanced usage is to print a help message for the whole application usage,
 * not only for the available plugins. Use PrintApplicationHelp() for this.
 *
 * To use PrintApplicationHelp(), you need to provide the \a command, \a usage,
 * and \a description parameters which will contain the main description
 * of your application and its usage. You can also add usage examples with
 * AddCommandLineExample().
 *
 * You can add specific options for your application through AddCommandLineOption(). If
 * the list of your options is big, then you can structure these options in sections by
 * using AddCommandLineSection().
 *
 * When PrintApplicationHelp() is called, this tool will write to the standard
 * output a help text of the following structure:
 * \verbatim
<description>

Usage: <usage>

Examples: <example>

Available options:

Specific options for <command>:
<list of options>

<section name>-specific options:
<list of options>

General options:
<list of CS general options>
\endverbatim
 *
 * \note This class requires the iPluginManager and iCommandLineParser to be
 * in the object registry (alternatively you can provide them as parameters).
 */
class CS_CRYSTALSPACE_EXPORT csCommandLineHelper
{
public:
  csCommandLineHelper ();

  /**
   * Print a title on standard output
   * \param title The title
   * \param level The level of the title. 0 is the higher level, other levels
   * will be displayed with less emphasis.
   */
  static void PrintTitle (const char* title, unsigned int level = 0);

  /**
   * Print an option on standard output
   * \param option The description of the option
   * \param value The default value of the option (it can be uninitialized).
   */
  static void PrintOption (const csOptionDescription& option, const csVariant& value);

  /**
   * Print an option on standard output
   * \param name The name of the option
   * \param description A user friendly description of the option
   * \param value The default value of the option, at least initialized to the correct type.
   */
  static void PrintOption (const char* name, const char* description, const csVariant& value);

  /**
   * Print the plugin help on standard output. This function will first send a broadcast
   * message of type csevCommandLineHelp, then it will check all loaded plugins and print their
   * options if they implement iPluginConfig.
   *
   * If the commandline parser is not given then the default commandline
   * parser from the registry will be used.
   */
  static void Help (iObjectRegistry* object_reg,
  	iCommandLineParser* cmdline = 0);

  /**
   * Return whether or not the '-help' option has been given on the commandline.
   *
   * If the commandline parser is not given then the default commandline
   * parser from the registry will be used.
   */
  static bool CheckHelp (iObjectRegistry* object_reg,
  	iCommandLineParser* cmdline = 0);

  /**
   * Add a new section of options. Return the index of this new section.
   * \note A default 'application' section with index 0 has already been created in the
   * constructor of this object.
   */
  size_t AddCommandLineSection (const char* name);

  /**
   * Add a commandline option to be described in the help text.
   * \param description The description of the option
   * \param section The index of the section of the option. A value of 0
   * means the default 'application' section.
   */
  void AddCommandLineOption (csOptionDescription& description, csVariant& value,
			     size_t section = 0);

  /**
   * Add a commandline option to be described in the help text.
   * \param name The name of the option, eg "debug"
   * \param description A user friendly description of the option, eg 'Enable
   * output of debug information'.
   * \param section The index of the section of the option. A value of 0
   * means the default 'application' section.
   */
  void AddCommandLineOption (const char* name, const char* description, csVariant value, size_t section = 0);

  /**
   * Add a commandline example to be printed in the list of usage examples
   * (eg "myapp -l file.xml").
   */
  void AddCommandLineExample (const char* example);

  /**
   * Print to standard output all command options and usages of this application
   * and its plugins.
   * \param registry Main object registry
   * \param command Name of the executable (eg "myapp").
   * \param usage Syntax to launch the executable (eg "myapp <OPTIONS> filename").
   * \param description User friendly description of the application.
   */
  void PrintApplicationHelp (iObjectRegistry* registry,
			     const char* command,
			     const char* usage,
			     const char* description) const;

 private:

  struct Option
  {
    csOptionDescription description;
    csVariant value;
  };

  struct CommandSection
  {
    // Constructor
    CommandSection (const char* name)
    : name (name) {}

    // Name of the section
    csString name;

    // Array of options
    csArray<Option> commandOptions;
  };

  // Array of command line sections
  csArray<CommandSection> commandSections;

  // Array of usage examples
  csStringArray examples;
};

#endif // __CS_CMDHELP_H__

