/*
    Copyright (C) 1998 by Jorrit Tyberghein
  
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

#ifndef ENGINE_CONFIG_H
#define ENGINE_CONFIG_H

#include "iconfig.h"

extern const IID IID_IEngineConfig;

///
interface IEngineConfig : public IConfig
{
  ///
  STDMETHOD (SetOption) (int id, csVariant* value);
  ///
  STDMETHOD (GetOption) (int id, csVariant* value);
  ///
  STDMETHOD (GetNumberOptions) (int& num);
  ///
  STDMETHOD (GetOptionDescription) (int idx, csOptionDescription* option);

  DECLARE_IUNKNOWN()
};

/**
 * Configurator object for the 3D engine.
 */
class csEngineConfig
{
private:
  static csOptionDescription config_options[];

public:
  ///
  bool SetOption (int id, csVariant* value);
  ///
  bool GetOption (int id, csVariant* value);
  ///
  int GetNumberOptions ();
  ///
  bool GetOptionDescription (int idx, csOptionDescription* option);

  DECLARE_INTERFACE_TABLE (csEngineConfig)
  DECLARE_IUNKNOWN()
  DECLARE_COMPOSITE_INTERFACE (EngineConfig)
};

#define GetIConfigFromcsEngineConfig(a)  &a->m_xEngineConfig
#define GetcsEngineConfigFromIConfig(a)  ((csEngineConfig*)((size_t)a - offsetof(csEngineConfig, m_xEngineConfig)))

#endif //ENGINE_CONFIG_H

