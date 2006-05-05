#ifndef __SCRIPT_CONSOLE_H__
#define __SCRIPT_CONSOLE_H__

#include "iaws/aws2.h"

#include "iutil/comp.h"
#include "iutil/eventh.h"
#include "iutil/csinput.h"

#include "ivideo/fontserv.h"
#include "ivaria/reporter.h"

#include "csutil/array.h"
#include "csutil/csstring.h"

/** 
 * Provides for debug output, and eventually input and maybe a debugger 
 * for the scripting system. 
 */

class scriptConsole
{
  /// A reference to the font that we'll use here.  
  csRef<iFont> font;

  /// The key composer.
  csRef<iKeyComposer> composer;

  /// The array on console items.
  csArray<csString> msgs;

  /// The array on console items.
  csArray<csString> cmds;

  /// The command-line for the shell.
  csString cmd;

  /// The amount of history that should be kept.
  uint32 history_size;

  /// The current command history pointer.
  size_t cmd_ptr;

  /// The current cursor position.
  size_t cursor_pos;

  /** 
   * Set to true if the console is active.
   * It will accept keystrokes when active, and it's appearance is different.
   */
  bool active;
  
  /** Set to true if the console is visible. */
  bool visible;

public:
  scriptConsole ()
    : history_size (500), cmd_ptr (0), cursor_pos (0), active (false), visible(true) 
  {}

  ~scriptConsole() 
  {}

  void Initialize (iObjectRegistry *obj_reg);

  void OnKeypress (csKeyEventData &data);
  
  /// Shows or hides the console.
  void SetConsoleVisible(bool setting)
  {
	visible=setting;
	if (!visible) active=false;	  
  }
  
  /// Returns true if the console is visible.
  bool Visible()
  {
	return visible;	  
  }
  
  /// Returns true if the console is active.
  bool Active () { if (visible==false) return false; else return active; }

  /// Flips the active state of the console (turns it on or off.)
  void FlipActiveState () 
  { 
    active = (active ? false : true); 
    if (active==true && cmds.Length ()==0)
    {
      cmd.Clear ();	
    }
  }

  void SetFont (csRef<iFont> _font)
  {
    font = _font;	
  }

  /**
   * Write a message to the console. 
   * \param txt The message to write.
   */
  void Message (const csString &txt);

  /** 
   * Redraws the console
   * \param g2d The graphics device to draw it to.
   */
  void Redraw (iGraphics2D * g2d);

};

extern scriptConsole *ScriptCon ();

#endif
