/*
    Copyright (C) 2002 by Marten Svanfeldt
                          Anders Stenberg

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


#ifndef __CS_IVIDEO_SHADER_H__
#define __CS_IVIDEO_SHADER_H__

/**\file
 * Shader-related interfaces
 */

#include "csutil/scf.h"

#include "iutil/array.h"

#include "csgfx/shadervar.h"
#include "csutil/array.h"
#include "csutil/bitarray.h"
#include "csutil/refarr.h"
#include "csutil/set.h"
#include "csutil/strset.h"
#include "csutil/noncopyable.h"

struct iDocumentNode;
struct iHierarchicalCache;
struct iLight;
struct iObject;
struct iLoaderContext;
struct iString;

namespace CS
{
  namespace Graphics
  {
    struct RenderMesh;
  }
}
class csShaderVariable;

struct iShader;
struct iShaderCompiler;
struct iShaderManager;

//@{
/**
 * A "shader variable stack".
 * Stores a list of shader variables, indexed by it's name.
 */
class csShaderVariableStack
{
public:
  /// Construct an empty stack
  csShaderVariableStack ()
    : varArray (0), size (0), ownArray (false)
  {}

  /**
   * Copies from another stack.
   * If the other stack was created from a preallocated array, the new stack
   * points to that same array. If the other stack used internal storage the
   * new stack will allocate it's own internal array and copy over the contents.
   */
  csShaderVariableStack (const csShaderVariableStack& other)
    : size (other.size), ownArray (false)
  {
    if (other.ownArray)
    {
      Setup (size);
      memcpy (varArray, other.varArray, size * sizeof (csShaderVariable*));
    }
    else
    {
      varArray = other.varArray;
    }
  }

  /// Construct a stack from a preallocated array of shader variables
  csShaderVariableStack (csShaderVariable** va, size_t size)
    : varArray (va), size (size), ownArray (false)
  {}
  
  ~csShaderVariableStack ()
  {
    if (ownArray)
      cs_free (varArray);
  }

  /**
   * Copies from another stack.
   * If the other stack was created from a preallocated array, the stack
   * points to that same array after the assignment. If the other stack used 
   * internal storage the new stack will allocate it's own internal array and 
   * copy over the contents.
   */
  csShaderVariableStack& operator= (const csShaderVariableStack& other)
  {
    if (other.ownArray)
    {
      Setup (size);
      memcpy (varArray, other.varArray, size * sizeof (csShaderVariable*));
    }
    else
    {
      if (ownArray)
	cs_free (varArray);
      ownArray = false;
      varArray = other.varArray;
    }
    return *this;
  }
  
  /// Initialize stack internal storage
  void Setup (size_t size)
  {
    if (ownArray)
      cs_free (varArray);

    csShaderVariableStack::size = size;
    varArray = 0;

    if (size > 0)
    {      
      varArray = (csShaderVariable**)cs_malloc (size * sizeof(csShaderVariable*));
      ownArray = true;

      memset (varArray, 0, size * sizeof(csShaderVariable*));
    }
  }

  /// Initialize stack with external storage
  void Setup (csShaderVariable** stack, size_t size)
  {
    if (ownArray)
      cs_free (varArray);

    varArray = stack;
    csShaderVariableStack::size = size;
    ownArray = false;
  }

  /// Initialize stack with external storage taken from another stack
  void Setup (const csShaderVariableStack& stack)
  {
    if (ownArray)
      cs_free (varArray);

    varArray = stack.varArray;
    size = stack.size;
    ownArray = false;
  }

  /// Make a local copy if the array was preallocated.
  void MakeOwnArray ()
  {
    if (ownArray) return;
    csShaderVariable** newArray =
      (csShaderVariable**)cs_malloc (size * sizeof(csShaderVariable*));
    memcpy (newArray, varArray, size * sizeof(csShaderVariable*));
    varArray = newArray;
    ownArray = true;
  }

  /// Get the number of variable slots in the stack
  inline size_t GetSize () const
  {
    return size;
  }

  /// Access a single element in the stack
  csShaderVariable*& operator[] (size_t index)
  {
    CS_ASSERT(index < size);
    return varArray[index];
  }

  csShaderVariable* const& operator[] (size_t index) const
  {
    CS_ASSERT(index < size);
    return varArray[index];
  }

  /// Clear the current array
  void Clear ()
  {
    if(varArray && size > 0)
      memset (varArray, 0, sizeof(csShaderVariable*)*size);
  }

  /// Merge one stack onto the "front" of this one
  void MergeFront (const csShaderVariableStack& other)
  {
    CS_ASSERT(other.size >= size);
    for (size_t i = 0; i < size; ++i)
    {
      if (!varArray[i])
        varArray[i] = other.varArray[i];
    }
  }

  /// Merge one stack onto the "back" of this one
  void MergeBack (const csShaderVariableStack& other)
  {
    CS_ASSERT(other.size >= size);
    for (size_t i = 0; i < size; ++i)
    {
      if (other.varArray[i])
        varArray[i] = other.varArray[i];
    }
  }

  /**
   * Copy contents of another stack.
   * Unlike operator=() it does not change the storage of this stack.
   * This stack must have the same number of elements as \a other.
   * Returns whether a copy was actually made. No copy is made if both
   * stacks point to the same storage.
   */
  bool Copy (const csShaderVariableStack& other)
  {
    CS_ASSERT(other.size == size);
    if (varArray == other.varArray) return false;
    memcpy (varArray, other.varArray, sizeof(csShaderVariable*)*size);
    return true;
  }
private:
  csShaderVariable** varArray;
  size_t size;
  bool ownArray;
};
//@}

//@{
/**
 * Helper function to retrieve a single value from a shader variable stack.
 */
static inline csShaderVariable* csGetShaderVariableFromStack (
  const csShaderVariableStack& stack,
  const CS::ShaderVarStringID &name)
{
  if ((name != CS::InvalidShaderVarStringID)
    && (size_t (name) < stack.GetSize ()))
  {
    return stack[name];
  }
  return 0;
}
//@}

/**
 * This is a baseclass for all interfaces which provides shadervariables
 * both dynamically and static
 */
struct iShaderVariableContext : public virtual iBase
{
  SCF_INTERFACE(iShaderVariableContext, 2, 2, 1);

  /**
   * Add a variable to this context
   * \remarks If a variable of the same name exists in the current context,
   *   its contents are replaced with those of \a variable.
   */
  virtual void AddVariable (csShaderVariable *variable) = 0;
  
  /// Get a named variable from this context
  virtual csShaderVariable* GetVariable (CS::ShaderVarStringID name) const = 0;

  /// Like GetVariable(), but it also adds it if doesn't exist already.
  csShaderVariable* GetVariableAdd (CS::ShaderVarStringID name)
  {
    csShaderVariable* sv;
    sv = GetVariable (name);
    if (sv == 0)
    {
      csRef<csShaderVariable> nsv (
	csPtr<csShaderVariable> (new csShaderVariable (name)));
      AddVariable (nsv);
      sv = nsv; // OK, sv won't be destructed, SV context takes ownership
    }
    return sv;
  }

  /// Get Array of all ShaderVariables
  virtual const csRefArray<csShaderVariable>& GetShaderVariables () const = 0;

  /**
   * Push the variables of this context onto the variable stacks
   * supplied in the "stacks" argument
   */
  virtual void PushVariables (csShaderVariableStack& stack) const = 0;  

  /// Determine whether this SV context contains any variables at all.
  virtual bool IsEmpty () const = 0;

  /**
   * Replace the current variable object of the same name as \a variable with
   * the latter, add \a variable otherwise.
   * \remarks This differs from AddVariable() as this method replaces the
   *   variable *object*, not just the contents.
   */
  virtual void ReplaceVariable (csShaderVariable* variable) = 0;
  
  /// Remove all variables from this context.
  virtual void Clear() = 0;

  /// Remove the given variable from this context.
  virtual bool RemoveVariable (csShaderVariable* variable) = 0;

  /// Remove the variable with the given name from this context.
  virtual bool RemoveVariable (CS::ShaderVarStringID name) = 0;
};

/**
 * Possible settings regarding a techique tag's presence.
 */
enum csShaderTagPresence
{
  /**
   * The tag is neither required nor forbidden. However, it's priority still
   * contributes to technique selection.
   */
  TagNeutral,
  /**
   * Techniques were this tag is present are rejected to be loaded.
   */
  TagForbidden,
  /**
   * Techniques are required to have one such tag. If at least one required 
   * tag exists and no required tag is present in a technique, it doesn't 
   * validate. 
   */
  TagRequired
};

/**
 * A manager for all shaders. Will only be one at a given time
 */
struct iShaderManager : public virtual iShaderVariableContext
{
  SCF_INTERFACE (iShaderManager, 4, 0, 0);
  /**
   * Register a shader to the shadermanager.
   * Compiler should register all shaders
   */
  virtual void RegisterShader (iShader* shader) = 0;
  /// Unregister a shader.
  virtual void UnregisterShader (iShader* shader) = 0;
  /// Remove all shaders.
  virtual void UnregisterShaders () = 0;
  /// Get a shader by name
  virtual iShader* GetShader (const char* name) = 0;
  /// Returns all shaders that have been created
  virtual const csRefArray<iShader> &GetShaders ()  = 0;

  /// Register a compiler to the manager
  virtual void RegisterCompiler (iShaderCompiler* compiler) = 0;
  /// Get a shadercompiler by name
  virtual iShaderCompiler* GetCompiler(const char* name) = 0;

  /**
   * Register a named shader variable accessor.
   */
  virtual void RegisterShaderVariableAccessor (const char* name,
      iShaderVariableAccessor* accessor) = 0;
  /**
   * Unregister a shader variable accessor.
   */
  virtual void UnregisterShaderVariableAccessor (const char* name,
      iShaderVariableAccessor* accessor) = 0;
  /**
   * Find a shader variable accessor.
   */
  virtual iShaderVariableAccessor* GetShaderVariableAccessor (
      const char* name) = 0;

  /**
   * Remove all shader variable accessors.
   */
  virtual void UnregisterShaderVariableAcessors () = 0;

  /// Get the shadervariablestack used to handle shadervariables on rendering
  virtual csShaderVariableStack& GetShaderVariableStack () = 0;

  /**
   * Set a technique tag's options.
   * \param tag The ID of the tag.
   * \param presence Whether the presence of a tag is required, forbidden or
   *  neither of both.
   * \param priority The tag's priority. The sum of all tag priorities is 
   *  decisive when two shader techniques have the same technique priority.
   */
  virtual void SetTagOptions (csStringID tag, csShaderTagPresence presence, 
    int priority = 0) = 0;
  /**
   * Get a technique tag's options.
   * \copydoc SetTagOptions(csStringID,csShaderTagPresence,int)
   */
  virtual void GetTagOptions (csStringID tag, csShaderTagPresence& presence, 
    int& priority) = 0;

  /**
   * Get the list of all tags with a specific \a presence setting.
   */
  virtual const csSet<csStringID>& GetTags (csShaderTagPresence presence,
    int& count) = 0;

  /**
   * Get the stringset used for shader variable names
   */
  virtual iShaderVarStringSet* GetSVNameStringset () const = 0;
  
  /// Get the cache for storing precompiled shader data
  virtual iHierarchicalCache* GetShaderCache() = 0;
  
  enum
  {
    cachePriorityLowest = 0,
    cachePriorityGlobal = 100,
    cachePriorityApp = 200,
    cachePriorityUser = 300,
    cachePriorityHighest = 400
  };
  virtual void AddSubShaderCache (iHierarchicalCache* cache,
    int priority = cachePriorityApp) = 0;
  /// Shortcut to add a subcache located in a VFS directory
  virtual iHierarchicalCache* AddSubCacheDirectory (const char* cacheDir,
    int priority = cachePriorityApp, bool readOnly = false) = 0;
  virtual void RemoveSubShaderCache (iHierarchicalCache* cache) = 0;
  virtual void RemoveAllSubShaderCaches () = 0;
};

/**
 * Shader metadata.
 * This struct holds shader metadata such as how many lights per pass
 * the shader supports, description etc that is not directly describing
 * exactly what the shader does, but tells the rest of the engine how it
 * should be used.
 */
struct csShaderMetadata
{
  /// Descriptive string
  const char *description;

  /**
   * Number of lights this shader can process in a pass. 
   * 0 means either that the shader does not do any lighting,
   * or that it can provide any number of lights.
   */
  uint numberOfLights;

  /// Constructor to null out parameters
  csShaderMetadata ()
    : description (0), numberOfLights (0)
  {}
};

/**
 * A list of priorities as returned by iShaderCompiler::GetPriorities()
 */
struct iShaderPriorityList : public virtual iBase
{
  SCF_INTERFACE (iShaderPriorityList, 1,0,0);
  /// Get number of priorities.
  virtual size_t GetCount () const = 0;
  /// Get priority.
  virtual int GetPriority (size_t idx) const = 0;
};

/**
 * Specific shader. Can/will be either render-specific or general
 * The shader in this form is "compiled" and cannot be modified.
 */
struct iShader : public virtual iShaderVariableContext
{
  SCF_INTERFACE(iShader, 5, 0, 0);

  /// Query the object.
  virtual iObject* QueryObject () = 0;

  /// Get name of the File where it was loaded from.
  virtual const char* GetFileName () = 0;

  /// Set name of the File where it was loaded from.
  virtual void SetFileName (const char* filename) = 0;

  /**
   * Query a "shader ticket".
   * Internally, a shader may choose one of several actual techniques
   * or variants at runtime. However, the variant has to be known in
   * order to determine the number of passes or to do pass preparation.
   * As the decision what variant is to be used is made based on the
   * mesh modes and the shader vars used for rendering, those have
   * to be provided to get the actual variant, which is then identified
   * by the "ticket".
   */
  virtual size_t GetTicket (const CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack& stack) = 0;

  /// Get number of passes this shader has
  virtual size_t GetNumberOfPasses (size_t ticket) = 0;

  /// Activate a pass for rendering
  virtual bool ActivatePass (size_t ticket, size_t number) = 0;

  /// Setup a pass
  virtual bool SetupPass (size_t ticket, const CS::Graphics::RenderMesh *mesh,
    CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack& stack) = 0;

  /**
   * Tear down current state, and prepare for a new mesh 
   * (for which SetupPass is called)
   */
  virtual bool TeardownPass (size_t ticket) = 0;

  /// Completly deactivate a pass
  virtual bool DeactivatePass (size_t ticket) = 0;
  
  /// Flags for SV users to be considered by GetUsedShaderVars
  enum SVUserFlags
  {
    /// Used by texture mappings
    svuTextures = (1 << 0),
    /// Used by buffer bindings
    svuBuffers = (1 << 1),
    /// Used by VProc program
    svuVProc = (1 << 2),
    /// Used by vertex program
    svuVP = (1 << 3),
    /// Used by fragment program
    svuFP = (1 << 4),
    
    /// All users
    svuAll = 0xffff
  };

  /**
   * Request all shader variables used by a certain shader ticket.
   * \param ticket The ticket for which to retrieve the information.
   * \param bits Bit array with one bit for each shader variable set; if a 
   *   shader variable is used, the bit corresponding to the name of the
   *   variable is note set. Please note: first, the array passed in must 
   *   initially have enough bits for all possible shader variables, it will 
   *   not be resized - thus a good size would be the number of strings in the
   *   shader variable string set. Second, bits corresponding to unused
   *   shader variables will not be reset. It is the responsibility of the 
   *   caller to do so.
   * \param userFlags What users to consider when collecting used SVs.
   *   Combination of SVUserFlags values.
   */
  virtual void GetUsedShaderVars (size_t ticket, csBitArray& bits,
				  uint userFlags = svuAll) const = 0;
  
  /// Get shader metadata
  virtual const csShaderMetadata& GetMetadata () const = 0;

  /**
   * Push the variables of this shader onto the variable stack
   * supplied in the "stack" argument
   */
  virtual void PushShaderVariables (csShaderVariableStack& stack,
    size_t ticket) const = 0;
  
  /**\name Shader technique selection and metadata
   * @{ */
  /**
   * Query a "priorities ticket".
   * This is a shader-internal token representing the techniques available
   * with the given mesh modes and shader variables.
   * \sa GetTicket
   */
  virtual size_t GetPrioritiesTicket (const CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack& stack) = 0;
  /**
   * Get a list of all available techniques (resp. their priorities) for a
   * priority ticket.
   */
  virtual csPtr<iShaderPriorityList> GetAvailablePriorities (size_t prioTicket) const = 0;
  /**
   * Query metadata from a technique.
   * Returns 0 if no metadata with the given key is available.
   */
  virtual csPtr<iString> GetTechniqueMetadata (int priority, const char* dataKey) const = 0;
  /**
   * Return a shader that wraps a certain technique of this shader.
   * Returns 0 if no technique with that priority is actually present.
   */
  virtual csPtr<iShader> ForceTechnique (int priority) = 0;
  /** @} */
};


/**
 * Compiler of shaders. Compile from a description of the shader to a 
 * compiled shader. The exact schema for input is specific to each shader-
 * compiler.
 */
struct iShaderCompiler : public virtual iBase
{
  SCF_INTERFACE (iShaderCompiler, 1,0,0);
  /// Get a name identifying this compiler
  virtual const char* GetName() = 0;

  /**
   * Compile a template into a shader. Will return 0 if it fails.
   * If the optional 'forcepriority' parameter is given then only
   * the technique with the given priority will be considered. If
   * this technique fails then the shader cannot be compiled.
   * If no priority is forced then the highest priority technique
   * that works will be selected.
   */
  virtual csPtr<iShader> CompileShader (
	iLoaderContext* ldr_context, iDocumentNode *templ,
	int forcepriority = -1) = 0;

  /// Validate if a template is a valid shader to this compiler
  virtual bool ValidateTemplate (iDocumentNode *templ) = 0;

  /// Check if template is parsable by this compiler
  virtual bool IsTemplateToCompiler (iDocumentNode *templ) = 0;

  /**
   * Return a list of all possible priorities (for techniques)
   * for this shader. This is a full list. It might also contain
   * priorities for techniques that don't work on this hardware.
   */
  virtual csPtr<iShaderPriorityList> GetPriorities (
		  iDocumentNode* templ) = 0;
		  
  /**
   * 'Precache' a shader.
   * Compiles a shader but stores results of that in the cache \a cacheTo
   * for faster loading at runtime.
   * \param node Root node of the shader to cache.
   * \param cacheTo Cache object to store data in. Usually an instance of
   *   VfsHierarchicalCache pointing to a VFS dir that is used as a cache
   *   directory for the shader manager at runtime.
   * \param quick Do a "quick" precache. That means thoroughness is traded
   *   for time: the precache will take less time than a full one, but will
   *   be incomplete, meaning that some more compilation will take place
   *   at load time.
   * \note In practice, 'quick' precaching means that the XMLShader plugin
   *  does not compile shader programs; this is deferred to the application
   *  run time.
   */
  virtual bool PrecacheShader (iDocumentNode* node,
    iHierarchicalCache* cacheTo, bool quick = false) = 0;
};

#endif // __CS_IVIDEO_SHADER_H__
