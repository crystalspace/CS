#include "cssysdef.h"
#include "property.h"

namespace aws
{
    /// Creates a new property mapping with the given property. Returns true on sucess, false on failure.
    bool property_bag::CreateProperty(const scfString &name,  const property& p)
    {
      	props[name]=p;
	return true;     
    }

    /// Gets the value of the named property. Returns true on sucess, false on failure.
    bool property_bag::Set(const scfString &name, const autom::keeper &value)
    {
      property_map::iterator pos = props.find(name);

      if (pos==props.end()) return false;
      else return pos->second.Set(value);
    }

    /// Gets the value of the named property.  Returns true on sucess, false on failure.
    bool property_bag::Get(const scfString &name, autom::keeper &value)
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
      scfString name = fn["name"]->toString().Value().c_str();
      autom::keeper value = fn["value"];

      return autom::func_parm(new autom::integer(Set(name, value)));
    }

    autom::func_parm property_bag::_get(autom::function &fn)
    {
      scfString name = fn["name"]->toString().Value().c_str();
      autom::keeper value = fn["value"];

      return autom::func_parm(new autom::integer(Get(name, value)));
    }

    autom::func_parm property_bag::_create(autom::function &fn)
    {
      if (allow_automation_create)
      {
	scfString name = fn["name"]->toString().Value().c_str();
	autom::keeper value = fn["value"];

	return autom::func_parm(new autom::integer(Set(name, value)));
      }
      else
      {
        return autom::func_parm(new autom::integer(0));	
      }
    }

    void property_bag::setup_automation(const scfString &oname)
    {
      // This creates object names like this:
      //
      //  :Set@win.toolbox.1.prop(name="frame", value=100)
      //
      //
      scfString _name = oname + ".prop";
      scfString fname;

      fname = "Set@"; fname += _name;	 AUTOM_REGISTER(fname, this, &property_bag::_set);
      fname = "Get@"; fname += _name;	 AUTOM_REGISTER(fname, this, &property_bag::_get);
      fname = "Create@"; fname += _name; AUTOM_REGISTER(fname, this, &property_bag::_create);
    }
}