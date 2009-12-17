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

#include "cssysdef.h"

#include "condeval.h"

#include "csgfx/renderbuffer.h"
#include "csutil/csendian.h"

CS_PLUGIN_NAMESPACE_BEGIN(XMLShader)
{

  ConditionsWriter::ConditionsWriter (csConditionEvaluator& evaluator)
   : evaluator (evaluator), currentDiskID (0)
  {
    condToDiskID.Put (csCondAlwaysFalse, (uint32)csCondAlwaysFalse);
    condToDiskID.Put (csCondAlwaysTrue, (uint32)csCondAlwaysTrue);
    
    savedConds = new csMemFile();
    stringStore.StartUse (savedConds);
  }

  ConditionsWriter::~ConditionsWriter ()
  {
    delete savedConds;
  }
    
  struct ConditionHeader
  {
    uint8 op;
    uint8 leftType;
    uint8 rightType;
    uint8 flags;
    
    enum
    { 
      leftHasIndices = 1, 
      rightHasIndices = 2
    };
    
    ConditionHeader() : op (0), leftType (0), rightType (0), flags (0) {}
  };

  bool ConditionsWriter::WriteCondition (iFile* cacheFile,
    CS::PluginCommon::ShaderCacheHelper::StringStoreWriter& strStore,
    const CondOperation& cond)
  {
    uint32 leftOperation = 0;
    uint32 rightOperation = 0;
    if (cond.left.type == operandOperation)
      leftOperation = GetDiskID (cond.left.operation);
    if (cond.right.type == operandOperation)
      rightOperation = GetDiskID (cond.right.operation);

    ConditionHeader head;
    head.op = cond.operation;
    head.leftType = cond.left.type;
    if ((cond.left.type >= operandSV) && (cond.left.svLocation.indices != 0))
      head.flags |= ConditionHeader::leftHasIndices;
    head.rightType = cond.right.type;
    if ((cond.right.type >= operandSV) && (cond.right.svLocation.indices != 0))
      head.flags |= ConditionHeader::rightHasIndices;
      
    if (cacheFile->Write ((char*)&head, sizeof (head)) != sizeof (head))
      return false;
      
    if (!WriteCondOperand (cacheFile, strStore, cond.left, leftOperation))
      return false;
    if (!WriteCondOperand (cacheFile, strStore, cond.right, rightOperation))
      return false;
    return true;
  }
    
  bool ConditionsWriter::WriteCondOperand (iFile* cacheFile,
    CS::PluginCommon::ShaderCacheHelper::StringStoreWriter& strStore,
    const CondOperand& operand, uint32 operationID)
  {
    switch (operand.type)
    {
      case operandOperation:
	{
	  uint32 condLE = csLittleEndian::UInt32 (operationID);
	  return (cacheFile->Write ((char*)&condLE, sizeof (condLE))
	    == sizeof (condLE));
	}
	break;
      case operandFloat:
	{
	  uint32 valLE = csLittleEndian::UInt32 (
	    csIEEEfloat::FromNative (operand.floatVal));
	  return (cacheFile->Write ((char*)&valLE, sizeof (valLE))
	    == sizeof (valLE));
	}
	break;
      case operandInt:
	{
	  int32 valLE = csLittleEndian::Int32 (operand.intVal);
	  return (cacheFile->Write ((char*)&valLE, sizeof (valLE))
	    == sizeof (valLE));
	}
	break;
      case operandBoolean:
	{
	  int32 valLE = csLittleEndian::Int32 (int (operand.boolVal));
	  return (cacheFile->Write ((char*)&valLE, sizeof (valLE))
	    == sizeof (valLE));
	}
	break;
      case operandSV:
      case operandSVValueInt:
      case operandSVValueFloat:
      case operandSVValueX:
      case operandSVValueY:
      case operandSVValueZ:
      case operandSVValueW:
      case operandSVValueTexture:
      case operandSVValueBuffer:
	{
	  const char* nameStr = evaluator.GetStrings()->Request (operand.svLocation.svName);
	  uint32 nameIDLE = csLittleEndian::UInt32 (strStore.GetID (nameStr));
	  if (cacheFile->Write ((char*)&nameIDLE, sizeof (nameIDLE))
	      != sizeof (nameIDLE))
	    return false;
	  if (operand.svLocation.indices != 0)
	  {
	    size_t numInd = *operand.svLocation.indices;
	    uint32 numIndLE = csLittleEndian::UInt32 (uint32 (numInd));
	    if (cacheFile->Write ((char*)&numIndLE, sizeof (numIndLE))
		!= sizeof (numIndLE))
	      return false;
	    for (size_t i = 0; i < numInd; i++)
	    {
	     size_t ind = operand.svLocation.indices[i+1];
	      uint32 indLE = csLittleEndian::UInt32 (uint32 (ind));
	      if (cacheFile->Write ((char*)&indLE, sizeof (indLE))
		  != sizeof (indLE))
		return false;
	    }
	  }
	  return true;
	}
	break;
      default:
	CS_ASSERT(false);
    }
    return false;
  }

  uint32 ConditionsWriter::GetDiskID (csConditionID cond)
  {
    const uint32* diskID = condToDiskID.GetElementPointer (cond);
    if (diskID == 0)
    {
      WriteCondition (savedConds, stringStore,
	evaluator.GetCondition (cond));
      uint32 newID = currentDiskID++;
      condToDiskID.Put (cond, newID);
      return newID;
    }
    return *diskID;
  }

  uint32 ConditionsWriter::GetDiskID (csConditionID cond) const
  {
    const uint32* diskID = condToDiskID.GetElementPointer (cond);
    CS_ASSERT(diskID != 0);
    return *diskID;
  }

  csPtr<iDataBuffer> ConditionsWriter::GetPersistentData ()
  {
    stringStore.EndUse();
    
    uint32 numCondsLE = csLittleEndian::UInt32 (currentDiskID);
    savedConds->Write ((char*)&numCondsLE, sizeof (currentDiskID));
    
    csPtr<iDataBuffer> buf (savedConds->GetAllData());
    delete savedConds; savedConds = 0;
    return buf;
  }


  ConditionsReader::ConditionsReader (csConditionEvaluator& evaluator,
				      iDataBuffer* src)
   : status (false), evaluator (evaluator)
  {
    diskIDToCond.Put ((uint32)csCondAlwaysFalse, csCondAlwaysFalse);
    diskIDToCond.Put ((uint32)csCondAlwaysTrue, csCondAlwaysTrue);

    csMemFile savedConds (src, true);
    
    savedConds.SetPos (savedConds.GetSize() - sizeof (uint32));
    uint32 numCondsLE;
    if (savedConds.Read ((char*)&numCondsLE, sizeof (numCondsLE))
      != sizeof (numCondsLE)) return;
    numCondsLE = csLittleEndian::UInt32 (numCondsLE);
    savedConds.SetPos (0);
    
    CS::PluginCommon::ShaderCacheHelper::StringStoreReader stringStore;
    stringStore.StartUse (&savedConds);
    
    for (uint32 currentID = 0; currentID < numCondsLE; currentID++)
    {
      CondOperation newCond;
      if (!ReadCondition (&savedConds, stringStore, newCond))
	return;
      diskIDToCond.Put (currentID,
	evaluator.FindOptimizedCondition (newCond));
    }
    
    stringStore.EndUse();

    status = true;
  }

  ConditionsReader::~ConditionsReader ()
  {
  }
    
  bool ConditionsReader::ReadCondition (iFile* cacheFile,
    const CS::PluginCommon::ShaderCacheHelper::StringStoreReader& strStore,
    CondOperation& cond)
  {
    ConditionHeader head;
    if (cacheFile->Read ((char*)&head, sizeof (head)) != sizeof (head))
      return false;
    cond.operation = (ConditionOp)head.op;
    cond.left.type = (OperandType)head.leftType;
    cond.right.type = (OperandType)head.rightType;
      
    if (!ReadCondOperand (cacheFile, strStore, cond.left,
	(head.flags & ConditionHeader::leftHasIndices) != 0))
      return false;
    if (!ReadCondOperand (cacheFile, strStore, cond.right,
	(head.flags & ConditionHeader::rightHasIndices) != 0))
      return false;
    return true;
  }
    
  bool ConditionsReader::ReadCondOperand (iFile* cacheFile,
    const CS::PluginCommon::ShaderCacheHelper::StringStoreReader& strStore,
    CondOperand& operand, bool hasIndices)
  {
    switch (operand.type)
    {
      case operandOperation:
	{
	  uint32 condLE;
	  if (cacheFile->Read ((char*)&condLE, sizeof (condLE))
	      != sizeof (condLE))
	    return false;
	  operand.operation = GetConditionID (csLittleEndian::UInt32 (condLE));
	  return true;
	}
	break;
      case operandFloat:
	{
	  uint32 valLE;
	  if (cacheFile->Read ((char*)&valLE, sizeof (valLE))
	      != sizeof (valLE))
	    return false;
	  operand.floatVal = csIEEEfloat::ToNative (
	    csLittleEndian::UInt32 (valLE));
	  return true;
	}
	break;
      case operandInt:
	{
	  int32 valLE;
	  if (cacheFile->Read ((char*)&valLE, sizeof (valLE))
	      != sizeof (valLE))
	    return false;
	  operand.intVal = csLittleEndian::UInt32 (valLE);
	  return true;
	}
	break;
      case operandBoolean:
	{
	  int32 valLE;
	  if (cacheFile->Read ((char*)&valLE, sizeof (valLE))
	      != sizeof (valLE))
	    return false;
	  operand.boolVal = csLittleEndian::UInt32 (valLE) != 0;
	  return true;
	}
	break;
      case operandSV:
      case operandSVValueInt:
      case operandSVValueFloat:
      case operandSVValueX:
      case operandSVValueY:
      case operandSVValueZ:
      case operandSVValueW:
      case operandSVValueTexture:
      case operandSVValueBuffer:
	{
	  uint32 nameIDLE;
	  if (cacheFile->Read ((char*)&nameIDLE, sizeof (nameIDLE))
	      != sizeof (nameIDLE))
	    return false;
	  const char* nameStr = strStore.GetString (
	    csLittleEndian::UInt32 (nameIDLE));
	  operand.svLocation.svName = evaluator.GetStrings()->Request (nameStr);
	  operand.svLocation.bufferName = csRenderBuffer::GetBufferNameFromDescr (
	    nameStr);
	  if (hasIndices)
	  {
	    uint32 numIndLE;
	    if (cacheFile->Read ((char*)&numIndLE, sizeof (numIndLE))
		!= sizeof (numIndLE))
	      return false;
	    size_t numInd = csLittleEndian::UInt32 (numIndLE);
	    operand.svLocation.indices = evaluator.AllocSVIndices (numInd);
	    *operand.svLocation.indices = numInd;
	    for (size_t i = 0; i < numInd; i++)
	    {
	      size_t& ind = operand.svLocation.indices[i+1];
	      uint32 indLE;
	      if (cacheFile->Read ((char*)&indLE, sizeof (indLE))
		  != sizeof (indLE))
		return false;
	      ind = csLittleEndian::UInt32 (indLE);
	    }
	  }
	  return true;
	}
	break;
      default:
	/* This should _not_ occur ... but does in case of corrupted 
	   files - source of corruption unclear :/ */
	return false;
    }
    return false;
  }
    
  csConditionID ConditionsReader::GetConditionID (uint32 diskID) const
  {
    const csConditionID* cond = diskIDToCond.GetElementPointer (diskID);
    if (cond == 0) return (csConditionID)~0;
    return *cond;
  }

}
CS_PLUGIN_NAMESPACE_END(XMLShader)
