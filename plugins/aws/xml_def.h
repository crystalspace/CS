#ifndef __AWS_XML_DEF_PARSER_H__
#define __AWS_XML_DEF_PARSER_H__

#include "registry.h"
#include "csutil/xmltiny.h"

namespace aws
{
  /** Parses a definition file. */
  class defFile
  {
    /** Worker function, parses a node.  Creates subnodes if necessary. */
    void ParseNode (registry *reg, csRef<iDocumentNodeIterator> &pos);

  public:
    defFile() {}
    virtual ~defFile() {}

    /** Parses the given text into the given registry. */
    virtual bool Parse (const std::string &txt, registry &reg);
  };

} // end namespace

#endif
