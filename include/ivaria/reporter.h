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

#ifndef __IVARIA_REPORTER_H__
#define __IVARIA_REPORTER_H__

#include <stdarg.h>
#include "csutil/scf.h"
#include "cssys/sysfunc.h"
#include "iutil/objreg.h"

struct iReporter;

/**
 * Severity level for iReporter: BUG severity level.
 * This is the worst thing that can happen. It means that some code
 * detected a bug in Crystal Space.
 */
#define CS_REPORTER_SEVERITY_BUG 0

/**
 * Severity level for iReporter: ERROR severity level.
 * There was an error of some kind. Usually this is an error while
 * reading data.
 */
#define CS_REPORTER_SEVERITY_ERROR 1

/**
 * Severity level for iReporter: WARNING severity level.
 * There was some condition which is non fatal but is suspicious.
 */
#define CS_REPORTER_SEVERITY_WARNING 2

/**
 * Severity level for iReporter: NOTIFY severity level.
 * Just a notification message.
 */
#define CS_REPORTER_SEVERITY_NOTIFY 3

/**
 * Severity level for iReporter: DEBUG severity level.
 * This is for debugging and it will usually generate an entry
 * in some log.
 */
#define CS_REPORTER_SEVERITY_DEBUG 4

SCF_VERSION (iReporterListener, 0, 0, 1);

/**
 * Implement this interface if you're interested in hearing about
 * new messages on the reporter.
 */
struct iReporterListener : public iBase
{
  /**
   * Something has been reported. If this function returns true
   * then the report is considered handled and the reporter will not
   * add it anymore.
   */
  virtual bool Report (iReporter* reporter, int severity, const char* msgId,
  	const char* description) = 0;
};

SCF_VERSION (iReporter, 0, 0, 3);

/**
 * This is the interface for the error/message reporter plugin.
 */
struct iReporter : public iBase
{
  /**
   * Report something. The given message string should be formed like:
   * 'crystalspace.<source>.<type>.<detail>'. Example:
   * 'crystalspace.sprite2dloader.parse.material'.
   */
  virtual void Report (int severity, const char* msgId,
  	const char* description, ...) CS_GNUC_PRINTF(4, 5) = 0;

  /**
   * Report something. va_list version.
   */
  virtual void ReportV (int severity, const char* msgId,
  	const char* description, va_list) CS_GNUC_PRINTF(4, 0) = 0;

  /**
   * Clear all messages in the reporter. If severity is -1 then all
   * will be deleted. Otherwise only messages of the specified severity
   * will be deleted.
   */
  virtual void Clear (int severity = -1) = 0;

  /**
   * Clear all messages in the reporter for which the id matches with
   * the given mask. The mask can contain '*' or '?' wildcards. This
   * can be used to clear all messages from some source like:
   * Clear("crystalspace.sprite2dloader.*")
   */
  virtual void Clear (const char* mask) = 0;

  /**
   * Give the number of reported messages currently registered.
   */
  virtual int GetMessageCount () const = 0;

  /**
   * Get message severity. Returns -1 if message doesn't exist.
   */
  virtual int GetMessageSeverity (int idx) const = 0;

  /**
   * Get message id. Returns NULL if message doesn't exist.
   */
  virtual const char* GetMessageId (int idx) const = 0;

  /**
   * Get message description. Returns NULL if message doesn't exist.
   */
  virtual const char* GetMessageDescription (int idx) const = 0;

  /**
   * Add a listener that listens to new reports. Listeners can optionally
   * remove reports too. This function does not check if the listener
   * is already there and will add it again if so. The listener will be
   * IncRef()'ed by this function.
   */
  virtual void AddReporterListener (iReporterListener* listener) = 0;

  /**
   * Remove a listener once. The listener will be DecRef()'ed by this function.
   * If the listener is on the list multiple times only one occurance
   * is removed. If the listener cannot be found on the list no DecRef()
   * will happen.
   */
  virtual void RemoveReporterListener (iReporterListener* listener) = 0;

  /**
   * Check if the listener is already on the list.
   */
  virtual bool FindReporterListener (iReporterListener* listener) = 0;

  //----------------------------------------------------------------------
  // Conveniance functions, these are not to be implemented.
  //----------------------------------------------------------------------

  /**
   * Report error.
   */
  void CS_GNUC_PRINTF (3, 4) 
      ReportError (const char* msgId, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    ReportV (CS_REPORTER_SEVERITY_ERROR, msgId, description, arg);
    va_end (arg);
  }

  /**
   * Report warning.
   */
  void CS_GNUC_PRINTF (3, 4)
      ReportWarning (const char* msgId, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    ReportV (CS_REPORTER_SEVERITY_WARNING, msgId, description, arg);
    va_end (arg);
  }

  /**
   * Report notification.
   */
  void CS_GNUC_PRINTF (3, 4)
      ReportNotify (const char* msgId, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    ReportV (CS_REPORTER_SEVERITY_NOTIFY, msgId, description, arg);
    va_end (arg);
  }

  /**
   * Report bug.
   */
  void CS_GNUC_PRINTF (3, 4)
      ReportBug (const char* msgId, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    ReportV (CS_REPORTER_SEVERITY_BUG, msgId, description, arg);
    va_end (arg);
  }

  /**
   * Report debug.
   */
  void CS_GNUC_PRINTF (3, 4)
      ReportDebug (const char* msgId, const char* description, ...)
  {
    va_list arg;
    va_start (arg, description);
    ReportV (CS_REPORTER_SEVERITY_DEBUG, msgId, description, arg);
    va_end (arg);
  }
};


/*
 * Helper class for csReport().  Not all compilers allow a bare `vararg'
 * function to be inlined, but wrapping the function in a class seems to
 * appease such compilers.  The NextStep compiler exhibits this particular
 * behavior.
 */
class csReporterHelper
{
public:
  static void CS_GNUC_PRINTF (5, 0)
      ReportV(iObjectRegistry* reg, int severity, char const* msgId,
      char const* description, va_list args)
  {
    iReporter* reporter = CS_QUERY_REGISTRY(reg, iReporter);
    if (reporter)
    {
      reporter->ReportV(severity, msgId, description, args);
      reporter->DecRef ();
    }
    else
    {
      csPrintfV(description, args);
      csPrintf("\n");
    }
  }
    
  static void CS_GNUC_PRINTF (5, 6)
      Report(iObjectRegistry* reg, int severity, char const* msgId,
      char const* description, ...)
  {
    va_list arg;
    va_start(arg, description);

    ReportV(reg,severity,msgId,description,arg);

    va_end (arg);
  }
};

/**
 * Helper function to use a reporter easily.  This function will also work if
 * no reporter is present and use stdout in that case.
 */
#define csReport csReporterHelper::Report
#define csReportV csReporterHelper::ReportV

#endif // __IVARIA_REPORTER_H__

