/*
    Copyright (C) 1998 by Jorrit Tyberghein
		CSScript module created by Brandon Ehle (Azverkan)
  
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

#ifndef __CSSINT_H__
#define __CSSINT_H__

extern const GUID IID_IProgramConsole;

interface IProgramConsole:public IUnknown {
	STDMETHOD (Toggle) () PURE;
	STDMETHOD (TypeChar) (int keycode) PURE;
};

extern const GUID IID_ILanguageConsole;

interface ILanguageConsole:public IUnknown {
	STDMETHOD (Init)(IProgramConsole *iConsole) PURE;
	STDMETHOD (KeyPress)(int keycode) PURE;
};

extern const GUID IID_IApplication;

interface IApplication:public IUnknown {

};

#endif
