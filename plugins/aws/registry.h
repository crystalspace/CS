#ifndef __AWS_REGISTRY_H__
#define __AWS_REGISTRY_H__

#include <map>
#include <string>
#include "object.h"

/** Namespace which includes aws specific code. */
namespace aws
{
	/** Maintains a name to value mapping. */
	class registry : public csRefCount
	{
		/** The type of map that we use here. */
		typedef std::map<std::string, autom::keeper> value_type;

		/** The registry. */
		value_type reg;

		/** The type for the list of child registries that we may use. */
		typedef std::vector< csRef<registry> > child_list_type;

		/** The list of children. */
		child_list_type children;

		/** The parent, if there is one. */
		csRef<registry> parent;

		/** The name of this registry. */
		std::string rname;

	public:
		registry(const std::string &_name):rname(_name) {}
		virtual ~registry() {}

		/** Gets the name of this registry. */
		const std::string &Name() { return rname; }

		/** Sets the parent of this registry. */
		void setParent(csRef<registry> _parent) { parent=_parent; }
		
		/** Adds a child registry. */
		void addChild(csRef<registry> _child) { children.push_back(_child); }
		
		/** Finds a named value and returns a keeper to it. */
		bool findValue(const std::string &name, autom::keeper &k);

		/** Inserts a new value into this registry. */
		void insert(const std::string &name, const autom::keeper &k)
		{
			reg.insert(std::make_pair(name, k));
		}
	};

} // end namespace

#endif