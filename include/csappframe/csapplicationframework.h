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
#include "iutil/objreg.h"
#include "ivaria/reporter.h"

/**
* Application framework class.
* \remarks
* In order to properly use this class, you must derive a class from it and
* provide a constructor and implementation for the OnInitialize() and
* Application() methods. You may only have one csApplicationFramework derived
* object in existence at any time (and generally, you will only have one such
* object in your application). The library containing the implementation of this
* class contains an implementation of the main() function that controls the
* use of this class. When using the csApplicationFramework class, do not
* provide your own implementation of main(). In your source code create a
* global instance of the overridden object, as follows:
* \code
* // Example.h
* class myDerivedEngine : public csApplicationFramework
* {
* public:
*   myDerivedEngine();
*   virtual bool OnInitialize();
*   virtual bool Application();
* };
*
* extern myDerivedEngine myApp;
*
*
* //--------------------------
*
* // Example.cpp
* // File scope
* myDerivedEngine myApp;
*
* myDerivedEngine::myDerivedEngine() : myDerivedEngine()
* {
*   SetApplicationName ("crystal.space.example.app");
* }
*
* myDerivedEngine::OnIntialize()
* {
*   // Request plugins, initialize any global non-CS data and structures
*   return true;
* }
* 
* myDerivedEngine::Application()
* {
*   // Perform initialization of CS data and structures, set event handler,
*   // load world, etc.
*
*   if ( ! Open())
*   {
*     return false;
*   }
*
*   ProcessQueue();
*
*   return true;
* }
* 
* CS_IMPLEMENT_APPLICATION
* 
* int main (int argc, char* argv[]) 
* {
*   return csApplicationFramework::Main (argc, argv);
* }
* 
* \endcode
* \par
* This class is derived from csInitializer for convenience, allowing
* overridden members to call csInitializer methods without qualifying them
* with csInitializer::.
* \par
* This class is not related to csApp or other classes related to the CSWS.
*/
class CS_CSAPPFRAME_EXPORT csApplicationFramework : public csInitializer
{
private:
  /**
  * Pointer to the application's object registry.
  */
  static iObjectRegistry* mp_object_reg;

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
  * \remarks This string is passed to the reporter to indacate message
  * origins from within the derived user application class. It should be
  * set in the derived class' constructor.
  */
  static char* m_ApplicationStringName;

  /**
  * Constructor
  * \remarks The csApplicationFramework constructor initializes framework
  * application variables. You must call this constructor from your derived
  * class' constructor.
  * \par
  * This constructor is protected to force the derived class to provide
  * its own constructor.
  */
protected:
  csApplicationFramework ();

  /**
  * Destructor.
  */
public:
  virtual ~csApplicationFramework ();


public:
  /**\internal
  * Initialize the csApplicationFramework class.
  * \param argc number of arguments passed on the command line
  * (from main()).
  * \param argv[] list of arguments passed on the command line
  * (from main()).
  * \return true if the initialization was successful, otherwise false.
  * \remarks This function is called by the framework's main()
  * implementation. It initializes the default environment, then calls
  * the application's overridden OnInitialize() function.
  * \warning This function is intended for use by the framework library's
  * main() implementation and should not be called by the user.
  */
  static bool Initialize (int argc, char* argv[]);

  /**\internal
  * Start application logic.
  * \return true if the application ran successfully, otherwise false.
  * \remarks
  * This function is called by the framework's main() implementation to
  * essentially start the program. It is called after Initialize() is called.
  * \warning This function is intended for use by the framework library's
  * main() implementation and should not be called by the user.
  */
  static bool Start ();

  /**\internal
  * End application logic.
  * \remarks
  * This function is called by the framework's main() implementation to
  * essentially end the program.
  * \warning This function is intended for use by the framework library's
  * main() implementation and should not be called by the user.
  */
	static void End ();

  /**
  * Quit running the appliation.
  * \remarks
  * This function will send a cscmdQuit event through the event queue. If no
  * queue has been initialized, then it will terminate the program with an
  * exit call.
  */
  static void Quit (void);

protected:
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
	* /remarks
	* This method is called after the crystal space engine has been shut down,
	* just before the framework is about to end the program. Unlike the other
	* overridables of this class, you need not bother overriding this method.
	* In general, this is provided to allow end of program debugging support.
	*/
	virtual void OnExit ();

  // Inline Helper Functions
public:
  /**
  * Returns a pointer to the object registry.
  */
  static iObjectRegistry* GetObjectRegistry () { return mp_object_reg; };

  /**
  * Allow a csApplicationFramework object to be used as an iObjectRegistry*.
  * \remarks
  * Using this implicit cast operator is a shorthand for calling
  * GetObjectRegistry(), and allows the developer to use his derived
  * csApplicationFramework object as a parameter to any function (and some
  * macros) which require an iObjectRegistry reference.
  */
  operator iObjectRegistry* () { return mp_object_reg; }

  /**
  * Open plugins and open application window.
  */
  bool Open () { return OpenApplication (mp_object_reg); }

  /**
  * Set the application's string name identifier.
  * \remarks
  * This string is used by DisplayError() and DisplayInfo() to identify the
  * source of a message as generated by the user application (as opposed to
  * one generated by code within the framework library or other code with the
  * Crystal Space libraries and plugins).
  * \par
  * Generally, you will call this function once in the constructor for
  * your derived csApplicationFramework class, but it is safe to call it any
  * number of times.
  */
  static void SetApplicationName (char *name)
  {
    m_ApplicationStringName = name;
  }

  /**
  * Start event queue.
  * \remarks
  * This is a shorthand method of calling csDefaultRunLoop().
  */
  static void ProcessQueue ()
  {
    csDefaultRunLoop (mp_object_reg);
  }

private:
  /**
  * Display an error notification.
  * \remarks
  * The error displayed with this function will be identified with the
  * framework library string name identifier.
  */
  static bool DisplayLibError (const char* description)
  {
    csReport (mp_object_reg, CS_REPORTER_SEVERITY_ERROR,
      m_FoundationStringName,
      description);
    return false;
  }

public:
  /**
  * Display an error notification.
  * \remarks
  * The error displayed with this function will be identified with the
  * application string name identifier set with SetApplicationName().
  */
  static bool DisplayError (const char* description)
  {
    csReport (mp_object_reg, CS_REPORTER_SEVERITY_ERROR,
      m_ApplicationStringName,
      description);
    return false;
  }

  /**
  * Display an information notification.
  * \remarks
  * The info displayed with this function will be identified with the
  * application string name identifier set with SetApplicationName().
  */
  static void DisplayInfo (const char* description)
  {
    csReport (mp_object_reg, CS_REPORTER_SEVERITY_NOTIFY,
      m_ApplicationStringName,
      description);
  }
  
  /**
   * Starts up the application framework, to be called from main().
   */
  int Main (int argc, char* argv[]);
};

/** @} */

#endif //__CS_CSAPPLICATIONFRAMEWORK_H__
