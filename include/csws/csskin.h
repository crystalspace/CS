/*
    Crystal Space Windowing System: Skin interface
    Copyright (C) 2000 by Andrew Zabolotny, <bit@eltech.ru>

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

#ifndef __CS_CSSKIN_H__
#define __CS_CSSKIN_H__

/**\file
 * Crystal Space Windowing System: Skin interface
 */

/**
 * \addtogroup csws_skins
 * @{ */
 
#include "csextern.h"
 
#include "csutil/parray.h"

class csSkin;
class csApp;
class csComponent;
class csButton;
class csWindow;
class csDialog;
class csListBox;
class csListBoxItem;
class csBackground;

/**
 * A `skin slice' is responsible for managing the external view of a
 * certain component. Every `skin slice' is designed to take care of
 * some specific component type; you should be careful to provide all
 * `slices' for all used types of components, otherwise the windowing
 * system will print a error message and abort the program. Every
 * csComponent has a pointer to a csSkinSlice object inside, which
 * points to the respective object responsible for his exterior.
 *<p>
 * This is an abstract class, meant to be uses as a common parent
 * for all derived classes. Since some derived objects will implement
 * additional functionality over the basic csSkinSlice functionality,
 * it is recommended to #define a SKIN macro at the top of every
 * file with the implementation of the respective component like this:
 *<p>
 * #define SKIN ((csDefaultButtonSkin *)skinslice)
 *<p>
 * Then you can use the SKIN macro as if it was a member variable.
 * Note that the safety of the typecast is guaranteed by the uniquity
 * of the string returned by the csSkinSlice::GetName() method.
 *<p>
 * Also note that most skin slices don't have a constructor. This is
 * because skin slices are usually created before the application, thus
 * you don't have any place to gather initialization information from.
 * If you need to query some data from the application (e.g. some textures
 * and so on) you will have to overload the Initialize() virtual method.
 */
class CS_CRYSTALSPACE_EXPORT csSkinSlice
{
public:
  /// Destroy this skin slice
  virtual ~csSkinSlice ()
  { Deinitialize (); }

  /**
   * Initialize the skin slice, if desired, by querying any information
   * you want from given application object. At the time this routine is
   * called, the application is fully initialized, configuration file is
   * opened, textures are loaded and so on. This method is called once
   * from csApp::Initialize() method and also every time you call
   * csApp::SetSkin().
   */
  virtual void Initialize (csApp * /*iApp*/, csSkin * /*Parent*/) {}

  /**
   * This method is called from application's destructor.
   * The skin should free any resources allocated in Initialize()
   * here, because later (at destructor time) some plugins (e.g. the
   * texture manager) may be unloaded already. It is legal to call
   * Deinitialize() multiple times; if the resources were already
   * freed, the method should do nothing.
   */
  virtual void Deinitialize () {}

  /// Get the identifier of the component this skin slice is for
  virtual const char *GetName () const = 0;

  /// Initialize the object we are responsible for: called once we set the skin
  virtual void Apply (csComponent &This);

  /**
   * Reset a component to its "default" state.
   * This removes the custom palette if any, resets
   * component size etc. This is usually called by skin slices
   * inside Apply() method before applying any changes
   * to the component. If the component has the `skinslice'
   * non-0, the Initialize() method calls this method of
   * the old skin slice so that it will restore the component
   * to the initial state.
   */
  virtual void Reset (csComponent &This);

  /// Draw the component we are responsible for
  virtual void Draw (csComponent &This) = 0;
};

/*
  @@@ Hack:
  When CS_CRYSTALSPACE_EXPORT is configured for exporting from a shared library,
  VC refuses to compile csSkin if it inherits from 
  csPDelArray<csSkinSlice>, as csSkinSlice contains abstract methods.
  Work around this by inheriting from csSkinSliceNonAbstr instead which
  has no abstract methods.
 */
class CS_CRYSTALSPACE_EXPORT csSkinSliceNonAbstr : public csSkinSlice
{
public:
  virtual const char *GetName () const { return 0; };
  virtual void Draw (csComponent &This) {};
};

/**
 * This class defines the interface for a container of skins.
 * Most of the functionality you will want is already here; the
 * only method that you will sometimes want to override is the
 * constructor and the Initialize() method. In constructor you
 * should assign the appropiate value to the "Prefix" member
 * variable which is used to find the sections that refer
 * to the respective theme. Initialize() used to query all
 * kinds of resources from the application object when a
 * skin is initialized.
 *<p>
 * Generally there should be only one object of this class,
 * which is stored into the csApp object. However, you can use
 * a skin for some component and all his children if you wish,
 * but you will have to store the respective skin container
 * somewhere inside that component (and override his GetSkin()
 * method so that child components can use the skin when initialized).
 *<p>
 * The repository contains a number of objects that are responsible
 * for the exterior of the respective components (and which are called
 * `skins'). These objects are identified by a text string. After you change
 * the skin repository, you should call the Apply() method of the repository
 * so that the given objects and all components which are inserted into that
 * component (note that this does NOT have to do anything with class
 * hierarchy!) will receive the cscmdSkinChanged broadcast.
 */
class CS_CRYSTALSPACE_EXPORT csSkin : public csPDelArray<csSkinSliceNonAbstr>
{
  /// The application
  csApp *app;

public:
  /// This is the prefix for section names in CSWS' configuration file
  const char *Prefix;

  /// Create the skin repository object
  csSkin () : csPDelArray<csSkinSliceNonAbstr> (16, 16), Prefix (0) {}

  virtual ~csSkin () { }

  /// Compare a item from this array with some key
  static int CompareKey (csSkinSliceNonAbstr* const&, char const* const& Key);

  /// Return a functor wrapping CompareKey() for a name.
  static csArrayCmp<csSkinSliceNonAbstr*,char const*> KeyCmp(char const* n)
  { return csArrayCmp<csSkinSliceNonAbstr*,char const*>(n, CompareKey); }

  /// Compare two items from this array
  static int Compare(csSkinSliceNonAbstr* const&, csSkinSliceNonAbstr* const&);

  /// Apply this skin to some component and all components inserted into it
  void Apply (csComponent *iComp);

  /// Initialize all skin slices with given application object
  virtual void Initialize (csApp *iApp);

  /// Free any resources allocated by the skin slices
  virtual void Deinitialize ();

  /// Utility function: get a skin-specific string from csws config file
  const char *GetConfigStr (const char *iSection, const char *iKey,
			    const char *iDefault);
  /// Same but get a boolean value
  bool GetConfigYesNo (const char *iSection, const char *iKey, bool iDefault);

  /// Utility: Read background from given section with given key prefix.
  void Load (csBackground &oBack, const char *iSection, const char *iPrefix);

private:
  bool ReadGradient (const char *iText, csRGBcolor *color, int iNum);
};

/**
 * This class defines the interface for a button skin slice.
 * Every skin slice that is meant for buttons should inherit
 * from this interface.
 */
class CS_CRYSTALSPACE_EXPORT csButtonSkin : public csSkinSlice
{
public:
  /// Get the identifier of the component this skin slice is for
  virtual const char *GetName () const
  { return "Button"; }

  /// Suggest the optimal size of the button, given an already filled object
  virtual void SuggestSize (csButton &This, int &w, int &h) = 0;
};

/**
 * This class defines the interface for a window skin slice.
 * Every skin slice that is meant for windows should inherit
 * from this interface.
 */
class CS_CRYSTALSPACE_EXPORT csWindowSkin : public csSkinSlice
{
public:
  /// Get the identifier of the component this skin slice is for
  virtual const char *GetName () const
  { return "Window"; }

  /// Create a button for window's titlebar
  virtual csButton *CreateButton (csWindow &This, int ButtonID) = 0;

  /// Place all gadgets (titlebar, buttons, menu and toolbar)
  virtual void PlaceGadgets (csWindow &This) = 0;

  /// Called to reflect some specific window state change on gagdets
  virtual void SetState (csWindow &This, int Which, bool State) = 0;

  /// Set window border width and height depending on frame style
  virtual void SetBorderSize (csWindow &This) = 0;
};

/**
 * This class defines the interface for a dialog skin slice.
 * Every skin slice that is meant for dialogs should inherit
 * from this interface.
 */
class CS_CRYSTALSPACE_EXPORT csDialogSkin : public csSkinSlice
{
public:
  /// Get the identifier of the component this skin slice is for
  virtual const char *GetName () const
  { return "Dialog"; }

  /// Set dialog border width and height depending on frame style
  virtual void SetBorderSize (csDialog &This) = 0;
};

/**
 * This class defines the interface for a window titlebar skin slice.
 * Every skin slice that is meant for title bars should inherit
 * from this interface.
 */
class CS_CRYSTALSPACE_EXPORT csTitlebarSkin : public csSkinSlice
{
public:
  /// Get the identifier of the component this skin slice is for
  virtual const char *GetName () const
  { return "Titlebar"; }
};

/**
 * This class defines the interface for a listbox skin slice.
 * Every skin slice that is meant for listboxes should inherit
 * from this interface.
 */
class CS_CRYSTALSPACE_EXPORT csListBoxSkin : public csSkinSlice
{
public:
  /// Get the identifier of the component this skin slice is for
  virtual const char *GetName () const
  { return "Listbox"; }

  /// Suggest the optimal size of the button, given an already filled object
  virtual void SuggestSize (csListBox &This, int &w, int &h) = 0;
};

/**
 * This class defines the interface for a listbox item skin slice.
 * Every skin slice that is meant for listboxe items should inherit
 * from this interface.
 */
class CS_CRYSTALSPACE_EXPORT csListBoxItemSkin : public csSkinSlice
{
public:
  /// Get the identifier of the component this skin slice is for
  virtual const char *GetName () const
  { return "ListboxItem"; }
};


/**
 * This class defines the interface for a scrollbar skin slice.
 * Every skin slice that is meant for scrollbars should inherit
 * from this interface.
 */
class CS_CRYSTALSPACE_EXPORT csScrollBarSkin : public csSkinSlice
{
public:
  /// Get the identifier of the component this skin slice is for
  virtual const char *GetName () const
  { return "ScrollBar"; }
};

/**
 * Use the following macros to declare a skin.
 * You should declare all the skins that your program will use
 * preferably inside the main module of your program. The following
 * macros are used to select custom skin slices if you know in advance
 * which components (e.g. buttons, windows, listboxes etc) your application
 * will use. This has the advantage of avoiding insclusion of dead code
 * into your program (e.g. if you don't use the tree view, you don't need
 * the tree view skin either - and a single skin slice can be as big as
 * 30K of code or even more!). For simple programs you can use the simple
 * CSWS_SKIN_DECLARE_XXX macros (defined inside the respective skin header file)
 * which includes ALL the components of the skin; but if you want a finer
 * grade of control you should use the macros below.
 */

/**
 * Start the declaration of a skin.
 * <p>`name' is the name of the skin (e.g. "Default")
 * <p>`base' is the base class of the skin (e.g. "csSkin")
 */
#define CSWS_SKIN_DECLARE(name,base)	\
  class name : public base	\
  {				\
  public:			\
    name ()			\
    {
/**
 * Declare a single skin slice to be included into the skin
 * that is in process of definition. "comp" is the component
 * name, e.g. "Button", "Window" and so on.
 */
#define CSWS_SKIN_SLICE(comp)	\
      InsertSorted ((csSkinSliceNonAbstr*)new cs##comp##Skin, Compare);

/**
 * Finish the definition of a skin.
 * After CSWS_SKIN_DECLARE_END you should put either a variable name
 * (e.g. "CSWS_SKIN_DECLARE_END myskin;") or simply a ';' - in the later
 * case you will define just a <b>type</b> called [name], where
 * `name' is the first parameter passed to CSWS_SKIN_DECLARE(), and you will
 * have to declare somewhere a variable of this type in order to use
 * the skin.
 */
#define CSWS_SKIN_DECLARE_END	\
    }				\
  }

/** @} */

#endif // __CS_CSSKIN_H__
