#ifndef __AWS_PREFERENCES_MGR_2_H__
#define __AWS_PREFERENCES_MGR_2_H__

#include "registry.h"
#include "iutil/objreg.h"

/**\file
 * Defines the preferences object, which is used as a key tree where settings for window creation are stored.  Other information may also be
 * stored in a preferences object.
 */


namespace aws
{

	/** This maintains a set of preferences.  Generally only one of these exists at a time, but there's no reason why there couldn't be more. */
	class preferences
	{
		/** The root registry.  All important registries (i.e. for windows or skins) hang off this registry. */
		registry root;

	public:
		preferences():root("root") {}
		~preferences() {}

		/** Loads an xml-based definitions file into this preferences object.  Multiple files may be loaded, one after the other.  The contents are essentially merged. */
		bool load(iObjectRegistry* objreg, const std::string& filename);		

		/** Clears all definitions for this preferences object. */
		void clear() { root.clear(); }

		/** Finds a registry in the given category. If the reference is invalid, then the given registry doesn't exist. */
		csRef< registry > findReg(const std::string &category, const std::string &name)
		{
            return root.findChild(category, name);			
		}
	};

} // end namespace

#endif
