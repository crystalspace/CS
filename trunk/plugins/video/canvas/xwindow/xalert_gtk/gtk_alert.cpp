/*
    Copyright (C) 2007 by Frank Richter

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

/* Show a message box with GTK. */

#include "cssysdef.h"
#include "ivideo/natwin.h"

#include "../xwindow.h"

#include <gtk/gtk.h>

extern "C" int libgtk_x11_2_0_is_present;
extern "C" int libgtk_x11_2_0_symbol_is_present(const char* s);

static void RenameButton (GtkWidget* widget, gpointer text)
{
  if (GTK_IS_BUTTON (widget))
  {
    gtk_button_set_label (GTK_BUTTON (widget), (const gchar*)text);
    gtk_button_set_image (GTK_BUTTON (widget), 
      gtk_image_new_from_stock (GTK_STOCK_OK, GTK_ICON_SIZE_BUTTON));
  }
  else if (GTK_IS_CONTAINER (widget))
  {
    gtk_container_foreach (GTK_CONTAINER (widget), &RenameButton, text);
  }
}

bool csXWindow::AlertV_GTK (int type, const char* title, const char* okMsg, 
			    const char* msg, va_list args)
{
  if (!libgtk_x11_2_0_is_present) return false;
  
  int gtk_argc = 0;
  if (!gtk_init_check (&gtk_argc, 0)) return false;
  
  bool doSecondary = libgtk_x11_2_0_symbol_is_present (
    "gtk_message_dialog_format_secondary_text");
  
  GtkMessageType gtkType;
  switch (type)
  {
  case CS_ALERT_ERROR:   gtkType = GTK_MESSAGE_ERROR;   break;
  case CS_ALERT_WARNING: gtkType = GTK_MESSAGE_WARNING; break;
  case CS_ALERT_NOTE:    gtkType = GTK_MESSAGE_INFO;    break;
  default:               gtkType = GTK_MESSAGE_INFO;    break;
  }
  
  csString messageString;
  messageString = title;
#ifdef GTK_MESSAGE_HAS_SECONDARY
  if (!doSecondary)
#endif
  {
    messageString += ":\n";
    messageString.AppendFmtV (msg, args);
  }
  
  GtkWidget* messageBox = gtk_message_dialog_new (0, GTK_DIALOG_MODAL, 
    gtkType, GTK_BUTTONS_OK, "%s", messageString.GetData());
#ifdef GTK_MESSAGE_HAS_SECONDARY
  if (doSecondary)
  {
    csString secondary;
    secondary.FormatV (msg, args);
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (messageBox),
      "%s", secondary.GetData());
  }
#endif
  
  // Rename OK button with user-provided text
  RenameButton (messageBox, const_cast<char*> (okMsg));

  gtk_dialog_run (GTK_DIALOG (messageBox));
  gtk_widget_destroy (messageBox);
  
  return true;
}

