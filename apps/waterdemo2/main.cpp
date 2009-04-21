/*
 * main.cpp
 *
 * Main entry point for waterdemo2.
 */

#include "waterdemo2.h"
#include "appwaterdemo2.h"
#include <csutil/sysfunc.h> // Provides csPrintf()

CS_IMPLEMENT_APPLICATION

int main(int argc, char** argv)
{
  csPrintf ("waterdemo2 version 0.0.1 by Pavel Krajcevski.\n");

  /* Runs the application.  
   *
   * csApplicationRunner<> cares about creating an application instance 
   * which will perform initialization and event handling for the entire game. 
   *
   * The underlying csApplicationFramework also performs some core 
   * initialization.  It will set up the configuration manager, event queue, 
   * object registry, and much more.  The object registry is very important, 
   * and it is stored in your main application class (again, by 
   * csApplicationFramework). 
   */
  return csApplicationRunner<AppWaterdemo2>::Run (argc, argv);
}
