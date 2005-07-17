#include "cssysdef.h"
#include "property.h"

namespace aws
{
    /// Creates a new property mapping with the given property. Returns true on sucess, false on failure.
    bool property_bag::CreateProperty(const csString &name,  const property& p)
    {
      	props[name]=p;
	return true;     
    }

    /// Gets the value of the named property. Returns true on sucess, false on failure.
    bool property_bag::Set(const csString &name, const autom::keeper &value)
    {
      property_map::iterator pos = props.find(name);

      if (pos==props.end()) return false;
      else return pos->second.Set(value);
    }

    /// Gets the value of the named property.  Returns true on sucess, false on failure.
    bool property_bag::Get(const csString &name, autom::keeper &value)
    {
      property_map::iterator pos = props.find(name);

      if (pos==props.end()) return false;
      else return pos->second.Get(value);
    }
    
    ///////////////////////////////////////////////////////////
    //////////////////// Automation ///////////////////////////
    ///////////////////////////////////////////////////////////

    autom::func_parm property_bag::_set(autom::function &fn)
    {
      csString name = fn["name"]->ToString().Value().c_str();
      autom::keeper value = fn["value"];

      return autom::func_parm(new autom::integer(Set(name, value)));
    }

    autom::func_parm property_bag::_get(autom::function &fn)
    {
      csString name = fn["name"]->ToString().Value().c_str();
      autom::keeper value = fn["value"];

      return autom::func_parm(new autom::integer(Get(name, value)));
    }

    autom::func_parm property_bag::_create(autom::function &fn)
    {
      if (allow_automation_create)
      {
	csString name = fn["name"]->ToString().Value().c_str();
	autom::keeper value = fn["value"];

	bool readable = fn["readable"]->ToInt().Value();
	bool writeable = fn["writeable"]->ToInt().Value();

	return autom::func_parm(new autom::integer (
	  CreateProperty(name, property(value, readable, writeable))));
      }
      else
      {
	return autom::func_parm(autom::Nil());	
      }
    }

    void property_bag::SetupAutomation(const csString &oname)
    {
      // This creates object names like this:
      //
      //  :Set@win.toolbox.1.prop(name="frame", value=100)
      //
      //
      csString _name = oname + ".prop";
      csString fname;

      fname = "Set@"; fname += _name;	 AUTOM_REGISTER(fname, this, &property_bag::_set);
      fname = "Get@"; fname += _name;	 AUTOM_REGISTER(fname, this, &property_bag::_get);
      fname = "Create@"; fname += _name; AUTOM_REGISTER(fname, this, &property_bag::_create);
    }
}