/*
    Crystal Space Windowing System: Standard dialogs
    Copyright (C) 1998,1999 by Andrew Zabolotny <bit@eltech.ru>

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

#ifndef __CS_CSSTDDLG_H__
#define __CS_CSSTDDLG_H__

/**\file
 * Crystal Space Windowing System: Standard dialogs
 */

/**
 * \addtogroup csws_stddlg
 * @{ */
 
#include "csextern.h"
 
#include "csutil/scf.h"
#define CSWS_INTERNAL
#include "csws.h"

/**
 * \name Message box style flags (used in csMessageBox (...) as last parameter)
 * These style flags can be combined using logical OR (|) operation;
 * some styles are mutually exclusive (for example, a message box cannot
 * be CSMBS_INFORMATION and CSMBS_WARNING at same time)
 * @{ */
/// Message box style: Message has no type
#define CSMBS_NOTYPE		0x00000000
/// Message box style: Informational message
#define CSMBS_INFORMATION	0x00000001
/// Message box style: Same for lazy ones (like me :-)
#define CSMBS_INFO		CSMBS_INFORMATION
/// Message box style: Warning message
#define CSMBS_WARNING		0x00000002
/// Message box style: Same but shorter
#define CSMBS_WARN		CSMBS_WARNING
/// Message box style: Question message
#define CSMBS_QUESTION		0x00000003
/// Message box style: Fatal error
#define CSMBS_ERROR		0x00000004
/// Message box style: Invalid operation
#define CSMBS_STOP		0x00000005
/// Message box style: Custom bitmap (texture name/x/y/w/h follows last parameter)
#define CSMBS_CUSTOMICON	0x0000000f
/// Message box style: This is the mask used to separate message box style from other flags
#define CSMBS_TYPEMASK		0x0000000f

/// Message box style: Message box contains a "OK" button
#define CSMBS_OK		0x00000010
/// Message box style: Message box contains a "Cancel" button
#define CSMBS_CANCEL		0x00000020
/// Message box style: Message box contains a "Abort" button
#define CSMBS_ABORT		0x00000040
/// Message box style: Message box contains a "Retry" button
#define CSMBS_RETRY		0x00000080
/// Message box style: Message box contains a "Ignore" button
#define CSMBS_IGNORE		0x00000100
/// Message box style: Message box contains a "OK" button and a "Cancel" button
#define CSMBS_OKCANCEL		(CSMBS_OK | CSMBS_CANCEL)
/// Message box style: Message box contains a "Abort" button, a "Retry" button and a "Ignore" button
#define CSMBS_ABORTRETRYIGNORE	(CSMBS_ABORT | CSMBS_RETRY | CSMBS_IGNORE)

/// Message box style: Message is not program-modal
#define CSMBS_NONMODAL		0x80000000
/// Message box style: Align text vertically so that it occupies given height (given as '...')
#define CSMBS_USEHEIGHT		0x40000000
/// Message box style: Center all text lines
#define CSMBS_CENTER		0x20000000
/** @} */

SCF_VERSION (iMessageBoxData, 0, 0, 1);

/**
 * The application will receive a cscmdStopModal event
 * when csMessageBox is done. The data of this event will be of this
 * type. You can check this with QUERY_INTERFACE.
 */
struct iMessageBoxData : public iBase
{
  /// Return userdata given by caller.
  virtual iBase* GetUserData () = 0;
};

/**
 * Display a message box and return immediatelly.
 * When message box finishes it will send a cscmdStopModal message
 * to the csApp::HandleEvent() with iMessageBoxData as userdata.
 * The pressed button will be given as the 'Info' field in
 * Event.Command.Info (cscmdCancel, cscmdOk, ...).
 */
extern CS_CRYSTALSPACE_EXPORT  void csMessageBox (csComponent *iParent, 
  const char *iTitle, const char *iMessage, iBase* userdata, 
  int iFlags = CSMBS_INFO | CSMBS_OK, ...) CS_GNUC_PRINTF (3, 6);

/// File name entry field in file dialogs
#define CSWID_FILENAME		0xC509
/// Path name entry field in file dialogs
#define CSWID_PATHNAME		0xC50A
/// Directory list box in file dialogs
#define CSWID_DIRLIST		0xC50B
/// File list box in file dialogs
#define CSWID_FILELIST		0xC50C

/// Create and return a new file open dialog
extern CS_CRYSTALSPACE_EXPORT csWindow *csFileDialog (csComponent *iParent, 
  const char *iTitle, const char *iFileName = "./", 
  const char *iOpenButtonText = "~Load", bool vfspaths=false);
  
/// Query full name, filename and pathname from a file dialog
extern CS_CRYSTALSPACE_EXPORT void csQueryFileDialog (csWindow *iFileDialog, 
  char *iFileName, size_t iFileNameSize);

/// Color wheel in color choose dialogs
#define CSWID_COLORWHEEL	0xC50D
/// Color hue/red scrollbar in color choose dialogs
#define CSWID_COLORHR		0xC50E
/// Color light/green scrollbar in color choose dialogs
#define CSWID_COLORLG		0xC50F
/// Color saturation/blue scrollbar in color choose dialogs
#define CSWID_COLORSB		0xC510
/// Color sample (static rectangle) in color choose dialogs
#define CSWID_COLORSAMPLE	0xC511
/// "HLS" radio button identifier
#define CSWID_COLORHLS		0xC512
/// "RGB" radio button identifier
#define CSWID_COLORRGB		0xC513

/// Create and return a new color choose dialog
extern CS_CRYSTALSPACE_EXPORT csWindow *csColorDialog (csComponent *iParent, 
  const char *iTitle, int iColor = 0);
/// Same but accepts R/G/B separately
extern CS_CRYSTALSPACE_EXPORT  csWindow *csColorDialog (csComponent *iParent, 
  const char *iTitle, float iR, float iG, float iB);
/// Query color dialog contents as a single color value
extern CS_CRYSTALSPACE_EXPORT void csQueryColorDialog (csWindow *iColorDialog, 
  int &oColor);
/// Query color dialog contents as R,G,B floating-point numbers
extern CS_CRYSTALSPACE_EXPORT void csQueryColorDialog (csWindow *iColorDialog, 
  float &oR, float &oG, float &oB);

/** @} */

#endif // __CS_CSSTDDLG_H__
