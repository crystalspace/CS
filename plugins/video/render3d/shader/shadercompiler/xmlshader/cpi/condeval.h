/*
  Copyright (C) 2004-2006 by Frank Richter
	    (C) 2004-2006 by Jorrit Tyberghein

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef __CS_CONDEVAL_H__
#define __CS_CONDEVAL_H__

#include "csplugincommon/shader/shadercachehelper.h"
#include "csutil/hashr.h"
#include "csutil/memfile.h"
#include "csutil/weakref.h"
#include "csgfx/shadervararrayhelper.h"
#include "csgfx/shadervarnameparser.h"
#include "iengine/engine.h"

#include "condition.h"
#include "expparser.h"
#include "mybitarray.h"

#include "condeval_var.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{
/// Container for shader expression constants
class csConditionEvaluator;

class ConditionIDMapper
{
  csConditionID nextConditionID;
  csHashReversible<csConditionID, CondOperation> conditions;

public:
  ConditionIDMapper() : nextConditionID (0) {}

  /// Get number of conditions allocated so far
  size_t GetNumConditions() { return nextConditionID; }

  csConditionID GetConditionID (const CondOperation& operation,
    bool get_new = true);
  CondOperation GetCondition (csConditionID condition);
};

struct EvaluatorShadervarValues;
struct EvaluatorShadervarValuesSimple;

/**
 * Processes an expression tree and converts it into an internal 
 * representation and allows later evaluation of this expression.
 */
class csConditionEvaluator :
  public csRefCount,
  public CS::Memory::CustomAllocated
{
  typedef CS::Threading::Mutex MutexType;
  typedef CS::Threading::ScopedLock<MutexType> LockType;
  mutable MutexType mutex;

  struct EvalState;
  struct EvaluatorShadervar
  {
    typedef bool EvalResult;
    typedef bool BoolType;
    struct FloatType
    {
      float v;

      FloatType () {}
      FloatType (float f) : v (f) {}
      operator float() const { return v; }
      bool operator== (const FloatType& other)
      { return fabsf (v - other.v) < SMALL_EPSILON; }
      bool operator!= (const FloatType& other)
      { return !operator==(other); }
    };
    typedef int IntType;
    csConditionEvaluator& evaluator;
    EvalState* evalState;
    const CS::Graphics::RenderMeshModes* modes;
    const csShaderVariableStack* stack;

    EvalResult GetDefaultResult() const { return false; }

    EvaluatorShadervar (csConditionEvaluator& evaluator,
      EvalState* evalState,
      const CS::Graphics::RenderMeshModes* modes, const csShaderVariableStack* stack) : 
        evaluator (evaluator), evalState (evalState), modes (modes), stack (stack)
    { }
    BoolType Boolean (const CondOperand& operand);
    IntType Int (const CondOperand& operand);
    FloatType Float (const CondOperand& operand);

    EvalResult LogicAnd (const CondOperand& a, const CondOperand& b)
    { return Boolean (a) && Boolean (b); }
    EvalResult LogicOr (const CondOperand& a, const CondOperand& b)
    { return Boolean (a) || Boolean (b); }
    
   private:
    csShaderVariable* GetShaderVar (const CondOperand& operand)
    {
      csShaderVariable* sv = 0;
      if (stack && stack->GetSize () > operand.svLocation.svName)
      {
	sv = (*stack)[operand.svLocation.svName];
	if (sv && operand.svLocation.indices != 0)
	{
	  sv = CS::Graphics::ShaderVarArrayHelper::GetArrayItem (sv,
	    operand.svLocation.indices + 1, *operand.svLocation.indices,
	    CS::Graphics::ShaderVarArrayHelper::maFail);
	}
      }
      return sv;
    }
  };
  friend class TicketEvaluator;
public:
  class TicketEvaluator : public CS::Utility::FastRefCount<TicketEvaluator>
  {
  protected:
    friend class csConditionEvaluator;
    union
    {
      csConditionEvaluator* owner;
      TicketEvaluator* poolNext;
    };
    MutexType& mutex;
    bool inEval : 1;
    bool hasLock : 1;
    EvalState* evalState;
    EvaluatorShadervar eval;
    
    TicketEvaluator (csConditionEvaluator* owner, bool hasLock,
      EvalState* evalState, EvaluatorShadervar& eval);
    ~TicketEvaluator();
  public:
    void DecRef ();
    
    /// Evaluate a condition and return the result.
    bool Evaluate (csConditionID condition);
    /// End evaluation (cleanup)
    void EndEvaluation();
  private:
    TicketEvaluator (const TicketEvaluator& other); // unimplemented, verboten
  };
private:
  friend struct EvaluatorShadervarValues;
  friend struct EvaluatorShadervarValuesSimple;

  /// Used to resolve SV names.
  csRef<iShaderVarStringSet> strings;
  /// Needed for frame counter
  csWeakRef<iEngine> engine;
  
  ConditionIDMapper conditions;

  // Evaluation cache (checked conditions + results)
  struct EvalState
  {
    EvalState* poolNext;

    MyBitArrayMalloc condChecked;
    MyBitArrayMalloc condResult;

    EvalState() : poolNext (0) {}
    void Clear() { condChecked.Clear(); }
  };
  // Evaluation state cache for caching between frames
  struct EvalCacheState : public EvalState
  {
    uint lastEvalFrame;
    csArray<const csShaderVariable*> lastShaderVars;

    EvalCacheState() { Clear(); }
    void Clear() { lastEvalFrame = ~0; EvalState::Clear(); }
  };
  EvalCacheState evalCache;
  
  TicketEvaluator* ticketEvalPool;
  void RecycleTicketEvaluator (TicketEvaluator* p);
  EvalState* evalStatePool;
  void RecycleEvalState (EvalState* p);
  
  csMemoryPool scratch;

  // Constants
  const csConditionConstants& constants;
  
  // For each shadervar, marks which conditions are affected
  struct SVAffection
  {
    CS::ShaderVarStringID svName;
    MyBitArrayMalloc affectedConditions;
      
    explicit SVAffection (CS::ShaderVarStringID svName) : svName (svName) {}
    bool operator< (const SVAffection& other) const { return svName < other.svName; }
    bool operator< (const CS::ShaderVarStringID& other) const { return svName < other; }
    friend bool operator< (const CS::ShaderVarStringID& a,
      const SVAffection& b)
    { return a < b.svName; }
  };
  csSafeCopyArray<SVAffection> svAffectedConditions;
  MyBitArrayMalloc bufferAffectConditions;

  csString lastError;
  const char* SetLastError (const char* msg, ...) CS_GNUC_PRINTF (2, 3);

  /**
   * Check whether to operand types are compatible, ie can be compared.
   * E.g. 'bool' and 'int' are not compatible.
   */
  static bool OpTypesCompatible (OperandType t1, OperandType t2);
  /// Get a name for an operand type, for error reporting purposes.
  static const char* OperandTypeDescription (OperandType t);
  const char* ResolveExpValue (const csExpressionToken& value,
    CondOperand& operand);
  const char* ResolveOperand (csExpression* expression, 
    CondOperand& operand);
  const char* ResolveSVIdentifier (csExpression* expression, 
    CondOperand& operand);
  const char* ResolveConst (csExpression* expression, 
    CondOperand& operand);

  bool EvaluateConst (const CondOperation& operation, bool& result);
  bool EvaluateOperandBConst (const CondOperand& operand, bool& result);
  bool EvaluateOperandIConst (const CondOperand& operand, int& result);
  bool EvaluateOperandFConst (const CondOperand& operand, float& result);

  void GetUsedSVs2 (csConditionID condition, MyBitArrayTemp& affectedSVs);

  void MarkAffectionBySVs (csConditionID condition, const CondOperand& operand);

  csString OperandToString (const CondOperand& operand);
  csString OperationToString (const CondOperation& operation);

  /* "Internal" methods - do the same as their non-internal namesakes
     except they don't lock. Used for internal/recursive calls. */
  template<typename Evaluator>
  typename Evaluator::EvalResult EvaluateInternal (Evaluator& eval,
    csConditionID condition);
  size_t* AllocSVIndicesInternal (const CS::Graphics::ShaderVarNameParser& parser);
  size_t* AllocSVIndicesInternal (size_t num);
  const char* ProcessExpressionInternal (csExpression* expression, 
    csConditionID& cond);
  void SetupEvalCacheInternal (const csShaderVariableStack* stack);
  bool EvaluateCachedInternal (EvalState* evalState, EvaluatorShadervar& eval,
    csConditionID condition);
  Logic3 CheckConditionResultsInternal (csConditionID condition,
    const Variables& vars, Variables& trueVars, Variables& falseVars);
  Logic3 CheckConditionResultsInternal (csConditionID condition,
    const Variables& vars);
  const char* ProcessExpressionInternal (csExpression* expression, 
    CondOperation& operation);
  csConditionID FindOptimizedConditionInternal (
    const CondOperation& operation);
  bool IsConditionPartOfInternal (csConditionID condition,
    csConditionID containerCondition);
  csString GetConditionStringInternal (csConditionID id);
public:
  template<typename Evaluator>
  typename Evaluator::EvalResult Evaluate (Evaluator& eval, csConditionID condition);

  csConditionEvaluator (iShaderVarStringSet* strings,
    const csConditionConstants& constants);
  ~csConditionEvaluator();
  void SetEngine (iEngine* engine) { this->engine = engine; }
    
  iShaderVarStringSet* GetStrings() const { return strings; }
  size_t* AllocSVIndices (size_t num);

  /// Convert expression into internal representation.
  const char* ProcessExpression (csExpression* expression, 
    csConditionID& cond);

  /**
   * Create an evaluator object with the given parameters.
   * \warning Locks the csConditionEvaluator until evaluation was ended!
   */
  csPtr<TicketEvaluator> BeginTicketEvaluationCaching (const CS::Graphics::RenderMeshModes& modes,
    const csShaderVariableStack* stack);
  /**
   * Create an evaluator object with a given set of condition results.
   */
  csPtr<TicketEvaluator> BeginTicketEvaluation (
    const csBitArray& condSet, const csBitArray& condResults);
  
  /// Get number of conditions allocated so far
  size_t GetNumConditions()
  { 
    LockType lock (mutex);
    return conditions.GetNumConditions(); 
  }

  /*
   * Check whether a condition, given a set of possible values for
   * variables, is definitely true, definitely false, or uncertain.
   *
   * If uncertain returns the possible variable values in the case
   * the condition would be true resp. false.
   */
  Logic3 CheckConditionResults (csConditionID condition,
    const Variables& vars, Variables& trueVars, Variables& falseVars);
  Logic3 CheckConditionResults (csConditionID condition,
    const Variables& vars);

  const char* ProcessExpression (csExpression* expression, 
    CondOperation& operation);
  /**
   * Get the ID for an operation, but also do some optimization of the 
   * expression. 
   */
  csConditionID FindOptimizedCondition (const CondOperation& operation);
  
  CondOperation GetCondition (csConditionID condition)
  { 
    LockType lock (mutex);
    return conditions.GetCondition (condition);
  }

  /**
   * Test if \c condition is a sub-condition of \c containerCondition 
   * (e.g. true if \c condition is AND-combined with some other condition).
   */
  bool IsConditionPartOf (csConditionID condition, 
    csConditionID containerCondition);

  /// Determine which SVs are used in some condition.
  void GetUsedSVs (csConditionID condition, MyBitArrayTemp& affectedSVs);

  /// Try to release unused temporary memory
  static void CompactMemory ();
  
  /// Debugging: get string for condition
  csString GetConditionString (csConditionID id);
};

class ConditionsWriter
{
  csConditionEvaluator& evaluator;
  
  csMemFile* savedConds;
  CS::PluginCommon::ShaderCacheHelper::StringStoreWriter stringStore;
  csHash<uint32, csConditionID> condToDiskID;
  uint32 currentDiskID;

  bool WriteCondition (iFile* cacheFile,
    CS::PluginCommon::ShaderCacheHelper::StringStoreWriter& strStore,
    const CondOperation& cond);
    
  bool WriteCondOperand (iFile* cacheFile,
    CS::PluginCommon::ShaderCacheHelper::StringStoreWriter& strStore,
    const CondOperand& operand, uint32 operationID);
public:
  ConditionsWriter (csConditionEvaluator& evaluator);
  ~ConditionsWriter();
  
  uint32 GetDiskID (csConditionID cond);
  uint32 GetDiskID (csConditionID cond) const;
  
  csPtr<iDataBuffer> GetPersistentData ();
};

class ConditionsReader
{
  bool status;
  csConditionEvaluator& evaluator;
  
  csHash<csConditionID, uint32> diskIDToCond;

  bool ReadCondition (iFile* cacheFile,
    const CS::PluginCommon::ShaderCacheHelper::StringStoreReader& strStore,
    CondOperation& cond);
  
  bool ReadCondOperand (iFile* cacheFile,
    const CS::PluginCommon::ShaderCacheHelper::StringStoreReader& strStore,
    CondOperand& operand, bool hasIndices);
public:
  ConditionsReader (csConditionEvaluator& evaluator,
    iDataBuffer* src);
  ~ConditionsReader ();
  
  bool GetStatus() const { return status; }
  csConditionID GetConditionID (uint32 diskID) const;
};

}
CS_PLUGIN_NAMESPACE_END(XMLShader)

#endif // __CS_CONDEVAL_H__
