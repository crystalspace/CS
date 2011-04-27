/*
    Copyright (C) 2010 by Frank Richter

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

#ifndef __CS_CSPLUGINCOMMON_RENDERMANAGER_RENDERGROUPINGHANDLER_H__
#define __CS_CSPLUGINCOMMON_RENDERMANAGER_RENDERGROUPINGHANDLER_H__

/**\file
 * Handling of render grouping
 */

namespace CS
{
  namespace RenderManager
  {
    template<typename RenderTree, typename ContextSetup>
    class RenderGroupingHandler
    {
    public:
      typedef typename RenderTree::ContextNode ContextNode;

      RenderGroupingHandler (iEngine* engine, RenderTree& renderTree, ContextSetup& contextSetup)
       : engine (engine), renderTree (renderTree), contextSetup (contextSetup) {}

      void operator() (ContextNode* context)
      {
	typename RenderTree::MeshNodeTreeType::Iterator meshNodeIter (
	  context->meshNodes.GetIterator());
	int lastGrouping = -1;
	ContextNode* targetContext = nullptr;

	while (meshNodeIter.HasNext ())
	{
	  typename RenderTree::TreeTraitsType::MeshNodeKeyType key;
	  meshNodeIter.PeekNext (key);

	  CS::RenderPriorityGrouping newGrouping = engine->GetRenderPriorityGrouping (
	    CS::Graphics::RenderPriority (uint (key.priority)));
	  if (lastGrouping == -1)
	  {
	    lastGrouping = newGrouping;
	    context->renderGrouping = newGrouping;
	  }
	  else if (lastGrouping != newGrouping)
	  {
	    lastGrouping = newGrouping;

	    // Set up last context
	    if (targetContext)
	      contextSetup (*targetContext);

	    // Create new context to add coming mesh nodes to
	    ContextNode* newContext =
	      renderTree.CloneContext (targetContext ? targetContext : context);
	    targetContext = newContext;
	    targetContext->renderGrouping = newGrouping;
	  }

	  if (targetContext)
	    context->MoveRenderMeshes (meshNodeIter, targetContext);
	  else
	    meshNodeIter.Next ();
	}
	// Set up last context
	if (targetContext)
	  contextSetup (*targetContext);
      }
    protected:
      iEngine* engine;
      RenderTree& renderTree;
      ContextSetup& contextSetup;
    };
  } // namespace RenderManager
} // namespace CS

#endif // __CS_CSPLUGINCOMMON_RENDERMANAGER_RENDERGROUPINGHANDLER_H__
