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

#ifndef __CS_IVARIA_REPORTER_H__
#define __CS_IVARIA_REPORTER_H__

#include "csutil/ansicolor.h"
#include "csutil/scf.h"
#include "csutil/sysfunc.h"
#include "csutil/util.h"
#include "iutil/objreg.h"

/**\file
 * Reporter interface.
 */
/**\addtogroup util
 * @{ */
 
struct iReporter;

/**\name iReporter severity levels
 * @{ */
/**
 * BUG severity level.
 * This is the worst thing that can happen. It means that some code
 * detected a bug in Crystal Space.
 */
#define CS_REPORTER_SEVERITY_BUG 0

/**
 * ERROR severity level.
 * There was an error of some kind. Usually this is an error while
 * reading data.
 */
#define CS_REPORTER_SEVERITY_ERROR 1

/**
 * WARNING severity level.
 * There was some condition which is non fatal but is suspicious.
 */
#define CS_REPORTER_SEVERITY_WARNING 2

/**
 * NOTIFY severity level.
 * Just a notification message.
 */
#define CS_REPORTER_SEVERITY_NOTIFY 3

/**
 * DEBUG severity level.
 * This is for debugging and it will usually generate an entry
 * in some log.
 */
#define CS_REPORTER_SEVERITY_DEBUG 4
/** @} */

SCF_VERSION (iReporterListener, 0, 0, 1);

/**
 * Implement this interface if you're interested in hearing about
 * new messages on the reporter.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Application.
 *   <li>Standard reporter listener plugin (crystalspace.utilities.stdrep)
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>iReporter
 *   </ul>
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

SCF_VERSION (iReporterIterator, 0, 0, 1);

/**
 * An iterator to iterate over all messages in the reporter.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>iReporter::GetMessageIterator()
 *   </ul>
 */
struct iReporterIterator : public iBase
{
  /// Are there more elements?
  virtual bool HasNext () = 0;
  /**
   * Get next element. After calling this call one of the
   * GetBla() functions to get information for the current element.
   */
  virtual void Next () = 0;

  /**
   * Get the message severity for the current element in the iterator.
   */
  virtual int GetMessageSeverity () const = 0;

  /**
   * Get message id.
   */
  virtual const char* GetMessageId () const = 0;

  /**
   * Get message description.
   */
  virtual const char* GetMessageDescription () const = 0;
};

SCF_VERSION (iReporter, 0, 1, 0);

/**
 * This is the interface for the error/message reporter plugin.
 * Note. This plugin does not actually print out or display
 * messages in any way. The reporter simply collects messages
 * and sends them out to interested partners. Typically the
 * standard reporter listener (iStandardReporterListener) is such
 * a plugin. It will print out the messages that arrive
 * on the reporter.
 * <p>
 * Main creators of instances implementing this interface:
 *   <ul>
 *   <li>Reporter plugin (crystalspace.utilities.reporter)
 *   </ul>
 * Main ways to get pointers to this interface:
 *   <ul>
 *   <li>CS_QUERY_REGISTRY()
 *   </ul>
 * Main users of this interface:
 *   <ul>
 *   <li>Entire CS and application.
 *   <li>csReporterHelper
 *   </ul>
 */
struct iReporter : public iBase
{
  /**
   * Report something. The given message ID should be formed like:
   * 'crystalspace.{source}.{type}.{detail}'. Example:
   * 'crystalspace.sprite2dloader.parse.material'.
   * \sa \ref FormatterNotes
   */
  virtual void Report (int severity, const char* msgId,
  	const char* description, ...) CS_GNUC_PRINTF(4, 5) = 0;

  /**
   * Report something. va_list version.
   * \sa \ref FormatterNotes
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
   * Get an iterator to iterate over all messages. This will make a copy
   * of all messages so that the reporter is not locked.
   */
  virtual csPtr<iReporterIterator> GetMessageIterator () = 0;

  /**
   * Add a listener that listens to new reports. Listeners can optionally
   * remove reports too. This function does not check if the listener
   * is already there and will add it again if so. The listener will be
   * IncRef()'ed by this function.
   */
  virtual void AddReporterListener (iReporterListener* listener) = 0;

  /**
   * Remove a listener once. The listener will be DecRef()'ed by this function.
   * If the listener is on the list multiple times only one occurrence
   * is removed. If the listener cannot be found on the list no DecRef()
   * will happen.
   */
  virtual void RemoveReporterListener (iReporterListener* listener) = 0;

  /**
   * Check if the listener is already on the list.
   */
  virtual bool FindReporterListener (iReporterListener* listener) = 0;

  //----------------------------------------------------------------------
  // Convenience functions, these are not to be implemented in the plugin.
  //----------------------------------------------------------------------

  /**
   * Report error.
   * \sa \ref FormatterNotes
   */
  inline void ReportError (const char* msgId, const char* description, ...)
    CS_GNUC_PRINTF (3, 4);

  /**
   * Report warning.
   * \sa \ref FormatterNotes
   */
  inline void ReportWarning (const char* msgId, const char* description, ...)
    CS_GNUC_PRINTF (3, 4);

  /**
   * Report notification.
   * \sa \ref FormatterNotes
   */
  inline void ReportNotify (const char* msgId, const char* description, ...)
    CS_GNUC_PRINTF (3, 4);

  /**
   * Report bug.
   * \sa \ref FormatterNotes
   */
  inline void ReportBug (const char* msgId, const char* description, ...)
    CS_GNUC_PRINTF (3, 4);

  /**
   * Report debug.
   * \sa \ref FormatterNotes
   */
  inline void ReportDebug (const char* msgId, const char* description, ...)
    CS_GNUC_PRINTF (3, 4);
};

inline void iReporter::ReportError
  (const char* msgId, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  ReportV (CS_REPORTER_SEVERITY_ERROR, msgId, description, arg);
  va_end (arg);
}

inline void iReporter::ReportWarning
  (const char* msgId, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  ReportV (CS_REPORTER_SEVERITY_WARNING, msgId, description, arg);
  va_end (arg);
}

inline void iReporter::ReportNotify
  (const char* msgId, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  ReportV (CS_REPORTER_SEVERITY_NOTIFY, msgId, description, arg);
  va_end (arg);
}

inline void iReporter::ReportBug
  (const char* msgId, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  ReportV (CS_REPORTER_SEVERITY_BUG, msgId, description, arg);
  va_end (arg);
}

inline void iReporter::ReportDebug
  (const char* msgId, const char* description, ...)
{
  va_list arg;
  va_start (arg, description);
  ReportV (CS_REPORTER_SEVERITY_DEBUG, msgId, description, arg);
  va_end (arg);
}


/**
 * Helper class for csReport().  Not all compilers allow a bare `vararg'
 * function to be inlined, but wrapping the function in a class seems to
 * appease such compilers.  The NextStep compiler exhibits this particular
 * behavior.
 */
class csReporterHelper
{
public:
  /**
   * Helper function to use a reporter easily.  This function will also work if
   * no reporter is present and use stdout in that case.
   * \remark You can use the #csReportV macro for even more convenience.
   * \sa \ref FormatterNotes
   */
  static inline void ReportV(iObjectRegistry* reg, int severity,
    char const* msgId, char const* description, va_list args)
    CS_GNUC_PRINTF (4, 0);

  /**
   * Helper function to use a reporter easily.  This function will also work if
   * no reporter is present and use stdout in that case.
   * \remark You can use the #csReport macro for even more convenience.
   * \sa \ref FormatterNotes
   */
  static inline void Report(iObjectRegistry* reg, int severity,
    char const* msgId, char const* description, ...)
    CS_GNUC_PRINTF (4, 5);
};

inline void csReporterHelper::ReportV(iObjectRegistry* reg, int severity,
  char const* msgId, char const* description, va_list args)
{
  csRef<iReporter> reporter;
  if (reg && (reporter = CS_QUERY_REGISTRY (reg, iReporter)))
    reporter->ReportV (severity, msgId, description, args);
  else
  {
    /*
      @@@ The csStrNCaseCmp()s are there because sometimes reported messages
      start with "Warning", and a "Warning: Warning" output looks rather
      crappy. The correct fix is obviously to remove "Warning" prefixes
      when the reporter is used.
     */
    switch (severity)
    {
      case CS_REPORTER_SEVERITY_BUG:
	csPrintf(CS_ANSI_BK CS_ANSI_FM CS_ANSI_FI "BUG: " CS_ANSI_RST);
	break;
      case CS_REPORTER_SEVERITY_ERROR:
        if (csStrNCaseCmp (description, "error", 5) != 0)
	  csPrintf(CS_ANSI_BK CS_ANSI_FR CS_ANSI_FI "ERROR: " CS_ANSI_RST);
	break;
      case CS_REPORTER_SEVERITY_WARNING:
        if (csStrNCaseCmp (description, "warning", 7) != 0)
	  csPrintf(CS_ANSI_BK CS_ANSI_FY CS_ANSI_FI "WARNING: " CS_ANSI_RST);
	break;
      case CS_REPORTER_SEVERITY_NOTIFY:
	csPrintf ("NOTIFY: ");
	break;
      case CS_REPORTER_SEVERITY_DEBUG:
	csPrintf(CS_ANSI_BK CS_ANSI_FW CS_ANSI_FI "DEBUG: " CS_ANSI_RST);
	break;
    }
    csPrintfV(description, args);
    csPrintf("\n");
  }
}

inline void csReporterHelper::Report(iObjectRegistry* reg, int severity,
  char const* msgId, char const* description, ...)
{
  va_list arg;
  va_start(arg, description);

  ReportV(reg,severity,msgId,description,arg);

  va_end (arg);
}

/**
 * Helper macro to use a reporter easily.
 */
#define csReport csReporterHelper::Report
/**
 * Helper macro to use a reporter easily.
 */
#define csReportV csReporterHelper::ReportV

/* @} */

#endif // __CS_IVARIA_REPORTER_H__

