/* $XConsortium: makeform.c,v 1.6 95/01/04 16:28:51 gildea Exp $ */
/*

Copyright (c) 1988, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the X Consortium.

*/
/* $XFree86: xc/programs/xmessage/makeform.c,v 1.6 2002/11/22 03:56:39 paulo Exp $ */

/* Stripped-down version of makeform.c from xmessage. */ 

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Shell.h>

#include <X11/Xaw/Form.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Scrollbar.h>

/* ARGSUSED */
static void 
handle_button (Widget w, XtPointer closure, XtPointer client_data)
{
    XtAppSetExitFlag (XtWidgetToApplicationContext (w));
}

Widget 
make_queryform(Widget parent,	    /* into whom widget should be placed */
	       const char* msgstr,  /* message string */
	       const char* button,  /* button title */
	       const char* title)   /* form title */
{
    Widget form, text, prev;
    Arg args[10];
    Cardinal n;
    char *shell_geom;
    int x, y, geom_flags;
    unsigned int shell_w, shell_h;
    Dimension max_width, max_height;

    form = XtVaCreateManagedWidget ("form", formWidgetClass, parent, 
				    XtNtitle, button,
				    0);

    text = XtVaCreateManagedWidget
	("message", asciiTextWidgetClass, form,
	 XtNleft, XtChainLeft,
	 XtNright, XtChainRight,
	 XtNtop, XtChainTop,
	 XtNbottom, XtChainBottom,
	 XtNdisplayCaret, False,
	 XtNlength, strlen (msgstr),
	 XtNstring, msgstr,
	 0);
    /*
     * Did the user specify our geometry?
     * If so, don't bother computing it ourselves, since we will be overridden.
     */
    XtVaGetValues(parent, XtNgeometry, &shell_geom, 0);
    geom_flags = XParseGeometry(shell_geom, &x, &y, &shell_w, &shell_h);
    if (!(geom_flags & WidthValue && geom_flags & HeightValue))
    {
	Dimension width, height, height_addons = 0;
	Dimension scroll_size, border_width;
	Widget label, scroll;
	Position left, right, top, bottom;
	const char *tmp;
	/*
	 * A Text widget is used for the automatic scroll bars.
	 * But Text widget doesn't automatically compute its size.
	 * The Label widget does that nicely, so we create one and examine it.
	 * This widget is never visible.
	 */
	XtVaGetValues(text, XtNtopMargin, &top, XtNbottomMargin, &bottom,
		      XtNleftMargin, &left, XtNrightMargin, &right, 0);
	label = XtVaCreateWidget("message", labelWidgetClass, form,
				 XtNlabel, msgstr,
				 XtNinternalWidth, (left+right+1)/2,
				 XtNinternalHeight, (top+bottom+1)/2,
				 0);
	XtVaGetValues(label, XtNwidth, &width, XtNheight, &height, 0);
	XtDestroyWidget(label);
	max_width = .7 * WidthOfScreen(XtScreen(text));
	max_height = .7 * HeightOfScreen(XtScreen(text));
	if (width > max_width)
	{
	    width = max_width;
	    /* add in the height of any horizontal scroll bar */
	    scroll = XtVaCreateWidget("hScrollbar", scrollbarWidgetClass, text,
				      XtNorientation, XtorientHorizontal,
				      0);
	    XtVaGetValues(scroll, XtNheight, &scroll_size,
			  XtNborderWidth, &border_width, 0);
	    XtDestroyWidget(scroll);
	    height_addons = scroll_size + border_width;
	}

	/* This fixes the xmessage assumption that the label widget and the
	 * text widget have the same size. In Xaw 7, the text widget has
	 * one extra pixel between lines.
	 * Xmessage is not internationalized, so the code bellow is harmless.
	 */
	tmp = msgstr;
	while (tmp != 0 && *tmp)
	{
	    ++tmp;
	    ++height;
	    tmp = strchr(tmp, '\n');
	}

	if (height > max_height)
	{
	    height = max_height;
	    /* add in the width of any vertical scroll bar */
	    scroll = XtVaCreateWidget("vScrollbar", scrollbarWidgetClass, text,
				      XtNorientation, XtorientVertical, 0);
	    XtVaGetValues(scroll, XtNwidth, &scroll_size,
			  XtNborderWidth, &border_width, 0);
	    XtDestroyWidget(scroll);
	    width += scroll_size + border_width;
	}
	height += height_addons;
	XtVaSetValues(text, XtNwidth, width, XtNheight, height, 0);
    }
    /*
     * Create the buttons
     */
    n = 0;
    XtSetArg (args[n], XtNleft, XtChainLeft); n++;
    XtSetArg (args[n], XtNright, XtChainLeft); n++;
    XtSetArg (args[n], XtNtop, XtChainBottom); n++;
    XtSetArg (args[n], XtNbottom, XtChainBottom); n++;
    XtSetArg (args[n], XtNfromVert, text); n++;
    XtSetArg (args[n], XtNvertDistance, 5); n++;

    prev = 0;
    XtSetArg (args[n], XtNfromHoriz, prev); 
    prev = XtCreateManagedWidget (button, commandWidgetClass,
				      form, args, n);
    XtAddCallback (prev, XtNcallback, handle_button, 0);
    Dimension border;

    XtVaGetValues(prev, XtNborderWidth, &border, 0);
    border *= 2;
    XtVaSetValues(prev, XtNborderWidth, border, 0);
    return form;
}
