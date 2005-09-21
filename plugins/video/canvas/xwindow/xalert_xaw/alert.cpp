/*
    Copyright (C) 2004 by Jorrit Tyberghein
              (C) 2004 by Frank Richter

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

/* Show a message box with Xaw. Based upon xmessage program. */

#include "cssysdef.h"
#include "csutil/sysfunc.h"
#include "../xwindow.h"

extern "C" 
{
#  include <X11/Intrinsic.h>
#  include <X11/StringDefs.h>
#  include <X11/Shell.h>
};

/*
 * data used by xmessage
 */

static String fallback_resources[] = {
    "*baseTranslations: #override :<Key>Return: default-exit()",
    "*message.Scroll: whenNeeded",
    0};

/*
 * Action to implement ICCCM delete_window and other translations.
 * Takes one argument, the exit status.
 */
static Atom wm_delete_window;
/* ARGSUSED */
static void
exit_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
  if((event->type == ClientMessage)
     && ((Atom)event->xclient.data.l[0] != wm_delete_window))
	return;
    
  XtAppSetExitFlag (XtWidgetToApplicationContext (w));
}

/* ARGSUSED */
static void
default_exit_action(Widget w, XEvent *event, String *params, 
    Cardinal *num_params)
{
  XtAppSetExitFlag (XtWidgetToApplicationContext (w));
}

static XtActionsRec actions_list[] = {
    {"exit", exit_action},
    {"default-exit", default_exit_action},
};

static String top_trans =
    "<ClientMessage>WM_PROTOCOLS: exit(1)\n";

extern "C" Widget make_queryform(Widget parent, const char* message, 
				 const char* button, const char* title);

bool csXWindow::AlertV (int type, const char* title, const char* okMsg, 
			const char* msg, va_list args)
{
    Widget top, queryform;
    XtAppContext app_con;
    csString msgStr;
    msgStr.FormatV (msg, args);
    int fake_argc = 1;
    const char* fake_argv[] = {title}; 
    // mild hack to get the desired form title.

    XtSetLanguageProc(0, (XtLanguageProc) 0, 0);

    top = XtAppInitialize (&app_con, "Xmessage-color",
			   0, 0, &fake_argc, 
			   CS_CONST_CAST(char**, fake_argv), // @@@ Urgh.
			   fallback_resources, 0, 0);

    wm_delete_window = XInternAtom(XtDisplay(top), "WM_DELETE_WINDOW", False);
    XtAppAddActions(app_con, actions_list, XtNumber(actions_list));
    XtOverrideTranslations(top, XtParseTranslationTable(top_trans));

    /*
     * create the query form; this is where most of the real work is done
     */
    queryform = make_queryform (top, msgStr, okMsg, title);

    if (!queryform) 
    {
      csPrintfErr ("unable to create form");
      return false;
    }

    XtSetMappedWhenManaged(top, FALSE);
    XtRealizeWidget(top);

    /* do WM_DELETE_WINDOW before map */
    XSetWMProtocols(XtDisplay(top), XtWindow(top), &wm_delete_window, 1);

    XtMapWidget(top);

    XtAppMainLoop(app_con);

    XtDestroyWidget (queryform);
    XtUnmapWidget (top);
    XtUnrealizeWidget (top);
    XtDestroyWidget (top);
    
    XSync (XtDisplay (top), false); // To really do widget destruction

    return true;
}

