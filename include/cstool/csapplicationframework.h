/*
Copyright (C) 2003 by Odes B. Boatwright.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CSAPPLICATIONFRAMEWORK_H__
#define __CS_CSAPPLICATIONFRAMEWORK_H__

/**\file 
 * Application framework class
 */

/**
 * \addtogroup appframe
 * @{ */

#include "csextern.h"
#include "cstool/initapp.h"
#include "iutil/event.h"
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

/**
 * Application framework class.
 * \remarks
 * This class provides a handy object-oriented wrapper around the Crystal Space
 * initialization and start-up functions. It encapsulates a callback paradigm
 * which provides methods such as OnInitialize() and OnExit() which you can
 * override to customize the framework's behavior. You should also consider
 * using csBaseEventHandler (csutil/csbaseeventh.h), which provides the same
 * sort of object-oriented wrapper for the Crystal Space event mechanism;
 * providing methods such as OnMouseClick(), OnKeyboard(), OnBroadcast(), etc.
 * \par
 * In order to properly use this class, you must derive your own class from it,
 * providing a constructor and implementation for the OnInitialize() and
 * Application() methods. You may only have one csApplicationFramework derived
 * object in existence at any time (and generally, you will only have one such
 * object in your application).  In your source code create a global instance of
 * the overridden object, as follows:
 *
 * \code
 * //--------------------------
 * // Example.h
 * class MyApp : public csApplicationFramework
 * {
 * public:
 *   MyApp();
 *   virtual bool OnInitialize(int argc, char* argv[]);
 *   virtual bool Application();
 * };
 *
 * //--------------------------
 * // Example.cpp
 * // File scope
 *
 * MyApp::MyApp() : csApplicationFramework()
 * {
 *   SetApplicationName ("my.example.app");
 * }
 *
 * bool MyApp::OnInitialize(int argc, char* argv[])
 * {
 *   // Request plugins, initialize any global non-CS data and structures
 *   return true;
 * }
 * 
 * bool MyApp::Application()
 * {
 *   // Perform initialization of CS data and structures, set event handler,
 *   // load world, etc.
 *
 *   if (!Open())
 *     return false;
 *
 *   Run();
 *   return true;
 * }
 * 
 * //--------------------------
 * // main.cpp
 * CS_IMPLEMENT_APPLICATION
 * 
 * int main (int argc, char* argv[]) 
 * {
 *   MyApp myApp;
 *   return myApp.Main (argc, argv);
 * }
 * \endcode
 * \par
 * csApplicationFramework itself is derived from csInitializer for convenience,
 * allowing overridden members to call csInitializer methods without qualifying
 * them with csInitializer::.
 */
class CS_CRYSTALSPACE_EXPORT csApplicationFramework : public csInitializer
{
private:
  /**\internal
   * Pointer to the application engine object.
   * \remarks
   * An application will have one and only one csApplicationFramework derived
   * object, a pointer to which will be stored in this private variable.
   * This variable will be set by the csApplicationFramework constructor and
   * cleared by the destructor.
   * \warning
   * DO NOT ATTEMPT TO ALTER THIS VARIABLE IN ANY OTHER WAY OR THE
   * FRAMEWORK LIBRARY WILL CRASH.
   */
  static csApplicationFramework* m_Ptr;

  /**
   * Foundation class string name.
   * \remarks
   * This string is passed to the reporter to indicate message
   * origins from within the framework library.
   */
  static const char* m_FoundationStringName;

  /**
   * User application string name.
   * \remarks This string is passed to the reporter to indicate message
   * origins from within the derived user application class. It should be
   * set in the derived class' constructor.
   */
  static char* m_ApplicationStringName;

protected:
  /**
   * Constructor
   * \remarks The csApplicationFramework constructor initializes framework
   * application variables. You must call this constructor from your derived
   * class' constructor.
   * \par
   * This constructor is protected to force the derived class to provide
   * its own constructor.
   */
  csApplicationFramework ();

  /**\internal
   * Initialize the csApplicationFramework class.
   * \param argc number of arguments passed on the command line
   * (from Main()).
   * \param argv[] list of arguments passed on the command line
   * (from Main()).
   * \return true if the initialization was successful, otherwise false.
   * \remarks This function is called by the framework's Main() function.
   * It initializes the default environment, then calls
   * the application's overridden OnInitialize() function.
   * \warning This function is intended for use by the framework library's
   * Main() implementation and should not be called by the user.
   */
  static bool Initialize (int argc, char* argv[]);

  /**\internal
   * Start application logic.
   * \return true if the application ran successfully, otherwise false.
   * \remarks
   * This function is called by the framework's Main() implementation to
   * essentially start the program. It is called after OnInitialize() is called.
   * \warning This function is intended for use by the framework library's
   * Main() implementation and should not be called by the user.
   */
  static bool Start ();

  /**\internal
   * End application logic.
   * \remarks
   * This function is called by the framework's Main() implementation to
   * essentially end the program.
   * \warning This function is intended for use by the framework library's
   * Main() implementation and should not be called by the user.
   */
  static void End ();
public:
  /**
   * Destructor
   */
  virtual ~csApplicationFramework ();

  /**
   * Quit running the application.
   * \remarks
   * This function will send a cscmdQuit event through the event queue. If no
   * queue has been initialized, then it will terminate the program with an
   * exit() call.
   */
  static void Quit ();

protected:
  /**
   * Pointer to the application's object registry.
   */
  static iObjectRegistry* object_reg;
  
  /**
   * Initialize the subclassed csApplicationFramework object.
   * \param argc number of arguments passed on the command line.
   * \param argv[] list of arguments passed on the command line.
   * \return true if the initialization was successful, otherwise false.
   * \remarks
   * You must override this function in the derived class. It will be called
   * after the framework has performed all necessary framework initialization.
   * \par
   * This method is where the user application should load any plug-ins via
   * RequestPlugins() and initialize any global application variables or class
   * members. Do not attempt to set or initialize any other Crystal Space
   * structures or objects in this method.
   */
  virtual bool OnInitialize (int argc, char* argv[]) = 0;

  /**
   * Perform application logic.
   * \remarks
   * You must override this method in the derived class. It will be called
   * after the OnInitialize() method is called and the framework has checked
   * the commandline for the help argument.
   * \par
   * This method is where the user application should perform all of its
   * main program logic, including initializing any Crystal Space variables
   * and states, starting the event queue loop, etc.
   */
  virtual bool Application () = 0;

  /**
   * Perform any end of program processing.
   * \remarks This method is called after the crystal space engine has been
   * shut down, just before the framework is about to end the program. Unlike
   * the other overridables of this class, you need not bother overriding this
   * method.  In general, this is provided to allow end of program debugging
   * support.
   */
  virtual void OnExit ();
public:
  // Inline Helper Functions
  /**
   * Returns a pointer to the object registry.
   */
  static iObjectRegistry* GetObjectRegistry () { return object_reg; };

  /**
   * Allow a csApplicationFramework object to be used as an iObjectRegistry*.
   * \remarks
   * Using this implicit cast operator is a shorthand for calling
   * GetObjectRegistry(), and allows the developer to use his derived
   * csApplicationFramework object as a parameter to any function (and some
   * macros) which require an iObjectRegistry reference.
   */
  operator iObjectRegistry* () { return object_reg; }

  /**
   * Open plugins and open application window.
   */
  bool Open () { return OpenApplication (object_reg); }

  /**
   * Set the application's string name identifier.
   * \remarks
   * This string is used by DisplayError() and DisplayInfo() to identify the
   * source of a message as generated by the user application (as opposed to
   * one generated by code within the framework library or other code with the
   * Crystal Space libraries and plugins).
   * \remarks
   * Generally, you will call this function once in the constructor for
   * your derived csApplicationFramework class, but it is safe to call it any
   * number of times.
   * \remarks
   * The string should be in the form "vendor.application-name". Spaces should
   * be avoided.
   */
  static void SetApplicationName (char *name)
  {
    m_ApplicationStringName = name;
  }

  /**
   * Get the application name.
   * \remarks This string is passed to the reporter to indicate message
   * origins from within the derived user application class. It should be
   * set in the derived class' constructor via SetApplicationName ().
   */
  static const char* GetApplicationName ()
  {
    return m_ApplicationStringName;
  }

  /**
   * Start event queue.
   * \remarks
   * This is a shorthand method of calling csDefaultRunLoop().
   */
  static void Run ()
  {
    csDefaultRunLoop (object_reg);
  }
private:
  /**
   * Display an error notification.
   * \remarks
   * The error displayed with this function will be identified with the
   * framework library string name identifier.
   */
  static bool ReportLibError (const char* description, ...)
  {
    va_list args;
    va_start (args, description);
    csReportV (object_reg, CS_REPORTER_SEVERITY_ERROR,
      m_FoundationStringName,
      description, args);
    va_end (args);
    return false;
  }

public:
  /**
   * Display an error notification.
   * \remarks
   * The error displayed with this function will be identified with the
   * application string name identifier set with SetApplicationName().
   */
  static bool ReportError (const char* description, ...)
  {
    va_list args;
    va_start (args, description);
    csReportV (object_reg, CS_REPORTER_SEVERITY_ERROR,
      m_ApplicationStringName,
      description, args);
    va_end (args);
    return false;
  }

  /**
   * Display a warning notification.
   * \remarks
   * The warning displayed with this function will be identified with the
   * application string name identifier set with SetApplicationName().
   */
  static void ReportWarning (const char* description, ...)
  {
    va_list args;
    va_start (args, description);
    csReportV (object_reg, CS_REPORTER_SEVERITY_WARNING,
      m_ApplicationStringName,
      description, args);
    va_end (args);
  }
  
  /**
   * Display an information notification.
   * \remarks
   * The info displayed with this function will be identified with the
   * application string name identifier set with SetApplicationName().
   */
  static void ReportInfo (const char* description, ...)
  {
    va_list args;
    va_start (args, description);
    csReportV (object_reg, CS_REPORTER_SEVERITY_NOTIFY,
      m_ApplicationStringName,
      description, args);
    va_end (args);
  }
  
  /**
   * Starts up the application framework, to be called from main().
   */
  int Main (int argc, char* argv[]);
};

/** @} */

#endif //__CS_CSAPPLICATIONFRAMEWORK_H__
