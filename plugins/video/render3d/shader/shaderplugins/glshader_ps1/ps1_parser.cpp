/*
Copyright (C) 2002 by John Harger

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

#include "csutil/hashmap.h"
#include "csutil/objreg.h"
#include "csutil/ref.h"
#include "csutil/scf.h"
#include "csutil/scfstr.h"
#include "csutil/stringreader.h"
#include "csutil/csmd5.h"
#include "csgeom/vector3.h"
#include "csutil/xmltiny.h"

#include "iutil/document.h"
#include "iutil/string.h"
#include "iutil/strset.h"
#include "iutil/vfs.h"
#include "ivaria/reporter.h"
#include "ivideo/graph3d.h"
#include "ivideo/rndbuf.h"
#include "ivideo/shader/shader.h"

#include "csplugincommon/opengl/glextmanager.h"

#include "glshader_ps1.h"
#include "ps1_parser.h"

void csPixelShaderParser::RegisterInstructions ()
{
  PS_Instructions[CS_PS_INS_INVALID].arguments = 0;
  PS_Instructions[CS_PS_INS_INVALID].supported = false;
  PS_Instructions[CS_PS_INS_INVALID].versions = 0;
#define PS_INSTR(instr, args, psversion)				    \
  PS_Instructions[CS_PS_INS_ ## instr].arguments = args;		    \
  PS_Instructions[CS_PS_INS_ ## instr].versions = psversion;		    \
  PS_Instructions[CS_PS_INS_ ## instr].supported = true;		    \
  instrStrings.Register (#instr, CS_PS_INS_ ## instr);
#define PS_VER_INSTR(x,y)						    \
  PS_Instructions[CS_PS_INS_PS_ ## x ## _ ## y].arguments = 0;		    \
  PS_Instructions[CS_PS_INS_PS_ ## x ## _ ## y].versions = CS_PS_ALLVERSIONS;\
  PS_Instructions[CS_PS_INS_PS_ ## x ## _ ## y].supported = true;	    \
  instrStrings.Register ("PS_" #x "_" #y, CS_PS_INS_PS_ ## x ## _ ## y);    \
  instrStrings.Register ("PS." #x "." #y, CS_PS_INS_PS_ ## x ## _ ## y);   
#include "ps1_instr.inc"

  PS_Instructions[CS_PS_INS_BEM].supported = false;
}

void csPixelShaderParser::Report (int severity, const char* msg, ...)
{
  va_list args;
  va_start (args, msg);
  csReportV (object_reg, severity, 
    "crystalspace.graphics3d.shader.glshader_ps1", msg, args);
  va_end (args);
}

csPixelShaderParser::csPixelShaderParser (iObjectRegistry *obj_reg)
{
  object_reg = obj_reg;
  RegisterInstructions ();
  version = CS_PS_INVALID;
}

csPixelShaderParser::~csPixelShaderParser ()
{
}

int csPixelShaderParser::GetArguments (const csString &str, csString &dest,
  csString &src1, csString &src2, csString &src3, csString &src4)
{
  size_t start = str.FindFirst (' ');
  if (start == (size_t)-1) return 0;

  int argument = 0;
  size_t len = str.Length ();
  while(argument < 5 && start < len)
  {
    size_t end = str.FindFirst (',', start + 1);
    if(end == (size_t)-1) end = str.Length ();
    if(end - start < 1) break;;

    csString reg;
    str.SubString (reg, start, end - start);
    reg.Trim ();

    switch(argument)
    {
      default: break;
      case 0: dest = reg; break;
      case 1: src1 = reg; break;
      case 2: src2 = reg; break;
      case 3: src3 = reg; break;
      case 4: src4 = reg; break;
    }
    argument ++;
    start = end + 1;
  }

  return argument;
}

unsigned short csPixelShaderParser::GetDestRegMask (const char *reg)
{
  unsigned short mods = CS_PS_WMASK_NONE;
  const char* dot = strchr (reg, '.');
  if (dot != 0)
  {
    if(version == CS_PS_1_4)
    {
      if(strstr (dot + 1, "r")) mods |= CS_PS_WMASK_RED;
      if(strstr (dot + 1, "x")) mods |= CS_PS_WMASK_RED;
      if(strstr (dot + 1, "g")) mods |= CS_PS_WMASK_GREEN;
      if(strstr (dot + 1, "y")) mods |= CS_PS_WMASK_GREEN;
      if(strstr (dot + 1, "b")) mods |= CS_PS_WMASK_BLUE;
      if(strstr (dot + 1, "z")) mods |= CS_PS_WMASK_BLUE;
      if(strstr (dot + 1, "a")) mods |= CS_PS_WMASK_ALPHA;
      if(strstr (dot + 1, "w")) mods |= CS_PS_WMASK_ALPHA;
    }
    else
    {
      if(strcmp (dot + 1, "rgba") == 0) mods = (CS_PS_WMASK_RED |
	CS_PS_WMASK_GREEN | CS_PS_WMASK_BLUE | CS_PS_WMASK_ALPHA);
      else if(strcmp (dot + 1, "xyzw") == 0) mods = (CS_PS_WMASK_RED |
	CS_PS_WMASK_GREEN | CS_PS_WMASK_BLUE | CS_PS_WMASK_ALPHA);
      else if(strcmp (dot + 1, "rgb") == 0) mods = (CS_PS_WMASK_RED |
	CS_PS_WMASK_GREEN | CS_PS_WMASK_BLUE);
      else if(strcmp (dot + 1, "xyz") == 0) mods = (CS_PS_WMASK_RED |
	CS_PS_WMASK_GREEN | CS_PS_WMASK_BLUE);
      else if(strcmp (dot + 1, "a") == 0) mods = CS_PS_WMASK_ALPHA;
      else if(strcmp (dot + 1, "w") == 0) mods = CS_PS_WMASK_ALPHA;
    }
  }
  return mods;
}

unsigned short csPixelShaderParser::GetSrcRegMods (const char *reg)
{
  unsigned short mods = 0;
  if(strstr(reg, "_bias")) mods |= CS_PS_RMOD_BIAS;
  else if(strstr(reg, "_x2")) mods |= CS_PS_RMOD_SCALE;
  else if(strstr(reg, "_bx2")) mods |= (CS_PS_RMOD_BIAS | CS_PS_RMOD_SCALE);
  if(strchr(reg, '-')) mods |= CS_PS_RMOD_NEGATE;
  if(strchr(reg, '1') && strchr(reg, '-')) mods = CS_PS_RMOD_INVERT;
    
  // Replication/Selector
  if((strstr(reg, "_dz") || strstr(reg, "_db")) 
    && version == CS_PS_1_4) mods |= CS_PS_RMOD_DZ;
  else if((strstr(reg, "_dw") || strstr(reg, "_da")) 
    && version == CS_PS_1_4) mods |= CS_PS_RMOD_DW;
  else if((strstr(reg, ".rgb") || strstr(reg, ".xyz")) 
    && version == CS_PS_1_4) mods |= CS_PS_RMOD_XYZ;
  else if((strstr(reg, ".rga") || strstr(reg, ".xyw")) 
    && version == CS_PS_1_4) mods |= CS_PS_RMOD_XYW;
  else if(strstr(reg, ".r") && version == CS_PS_1_4) mods |= CS_PS_RMOD_REP_RED;
  else if(strstr(reg, ".g") && version == CS_PS_1_4) mods |= CS_PS_RMOD_REP_GREEN;
  else if(strstr(reg, ".b")) mods |= CS_PS_RMOD_REP_BLUE;
  else if(strstr(reg, ".a")) mods |= CS_PS_RMOD_REP_ALPHA;

  return mods;
}

bool csPixelShaderParser::GetInstruction (const char *str,
  csPSProgramInstruction &inst)
{
  // Initialize the struct to an invalid instruction
  inst.instruction = CS_PS_INS_INVALID;

  csString line (str);
  line.Downcase ();

  size_t comment = line.FindFirst ('/');
  if(comment != (size_t)-1 && line.GetAt (comment + 1) == '/')
    line.Truncate (comment);
  comment = line.FindFirst (';');
  if(comment >= 0) line.Truncate (comment);
  line.Trim ();
  
  if(line.Length() < 1) return true; // Ignore blank lines

  csString istr;
  line.SubString (istr, 0, line.FindFirst (' '));
  istr.Upcase();
  csStringID inst_id = (csStringID)~0;

  if(version == CS_PS_INVALID) 
  {
    inst_id = instrStrings.Request (istr);
    if (inst_id != csInvalidStringID)
    {
      inst.instruction = (csPixelShaderInstruction)inst_id;
      return true;
    }
  }

  // Ignore pairing modifier
  if (istr.GetAt (0) == '+')
  {
    istr.DeleteAt (0);
    istr.Trim ();
  }

  if(!strcmp(istr, "DEF"))
  {
    // Define a constant
    csString dest, f1, f2, f3, f4;
    if(GetArguments(line, dest, f1, f2, f3, f4) != 5)
    {
      Report(CS_REPORTER_SEVERITY_ERROR,
        "You must use a four component vector when declaring a constant!");
      return false;
    }
    csPSConstant constant;
    constant.reg = dest.GetData()[1] - '0';
    constant.value.x = atof(f1.GetData());
    constant.value.y = atof(f2.GetData());
    constant.value.z = atof(f3.GetData());
    constant.value.w = atof(f4.GetData());
    program_constants.Push (constant);
    return true;
  }

  size_t mod_pos = istr.FindFirst ('_');
  if(mod_pos != (size_t)-1)
  {
    csString unmod;
    istr.SubString (unmod, 0, mod_pos);
    inst_id = instrStrings.Request (unmod);
  }
  else
  {
    inst_id = instrStrings.Request (istr);
  }

  if (inst_id == csInvalidStringID)
  {
    Report (CS_REPORTER_SEVERITY_WARNING, 
      "Unknown pixel shader instruction '%s'",
      istr.GetData ());
    return false;
  }
  if(!PS_Instructions[inst_id].supported)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
      "Pixel shader instruction '%s' is not supported at this time",
      istr.GetData ());
    return false;
  }
  
  inst.instruction = (csPixelShaderInstruction)inst_id;

  // Find the instruction modifier(s)

  inst.inst_mods = CS_PS_IMOD_NONE;

  if(strstr(istr, "_X2")) inst.inst_mods |= CS_PS_IMOD_X2;
  else if(strstr(istr, "_X4")) inst.inst_mods |= CS_PS_IMOD_X4;
  else if(strstr(istr, "_X8")) inst.inst_mods |= CS_PS_IMOD_X8;
  else if(strstr(istr, "_D2")) inst.inst_mods |= CS_PS_IMOD_D2;
  else if(strstr(istr, "_D4")) inst.inst_mods |= CS_PS_IMOD_D4;
  else if(strstr(istr, "_D8")) inst.inst_mods |= CS_PS_IMOD_D8;
  // _sat can be combined with _xn or _dn
  if(strstr(istr, "_SAT")) inst.inst_mods |= CS_PS_IMOD_SAT;

  csString dest, src1, src2, src3, trash;
  if(GetArguments(line, dest, src1, src2, src3, trash)
    != PS_Instructions[inst_id].arguments)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
      "Incorrect number of arguments for Pixel Shader instruction '%s'!",
      istr.GetData ());
    return false;
  }

  if (PS_Instructions[inst_id].arguments > 0)
  {
    // Get destination argument
    switch(dest.GetAt (0))
    {
      case 'c':
	Report (CS_REPORTER_SEVERITY_ERROR, 
	  "Destination register can not be a constant register!");
	return false;
      case 'v':
	Report (CS_REPORTER_SEVERITY_ERROR, 
	  "Destination register can not be a color register!");
	return false;
      case 't':
	inst.dest_reg = CS_PS_REG_TEX;
	break;
      case 'r':
	inst.dest_reg = CS_PS_REG_TEMP;
	break;
    }

    inst.dest_reg_mods = GetDestRegMask (dest.GetData ());
    inst.dest_reg_num = dest.GetAt (1) - '0';

    if(inst.dest_reg_num >= max_registers[inst.dest_reg])
    {
      Report (CS_REPORTER_SEVERITY_ERROR,
	"Destination register out of range, max for version '%s' is '%d'!",
	version_string.GetData(),
	max_registers[inst.dest_reg]);
      return false;
    }

    // Get source register(s)
    for(int j = 0; j < PS_Instructions[inst_id].arguments - 1; j++)
    {
      const char *reg = NULL;
      switch(j)
      {
	default:
	case 0: reg = src1.GetData (); break;
	case 1: reg = src2.GetData (); break;
	case 2: reg = src3.GetData (); break;
      }
      if(reg==NULL) break;

      const char *p = reg;
      while(p[0] != 'c' && p[0] != 'v' && p[0] != 't' && p[0] != 'r'
      	&& p[0]!='\0') p++;
      if(p[0]=='\0') return false;
      switch(p[0])
      {
	case 'c':
	  inst.src_reg[j] = CS_PS_REG_CONSTANT;
	  break;
	case 'v':
	  inst.src_reg[j] = CS_PS_REG_COLOR;
	  break;
	case 't':
	  inst.src_reg[j] = CS_PS_REG_TEX;
	  break;
	case 'r':
	  inst.src_reg[j] = CS_PS_REG_TEMP;
	  break;
      }

      inst.src_reg_mods[j] = GetSrcRegMods (reg);
      inst.src_reg_num[j] = reg[1] - '0';
      if(inst.src_reg_num[j] >= max_registers[inst.src_reg[j]])
      {
	Report (CS_REPORTER_SEVERITY_ERROR,
	  "Source register out of range, max for version '%s' is '%d'!",
	  version_string.GetData(),
	  max_registers[inst.src_reg[j]]);
	return false;
      }
    }
  }

  return true;
}

bool csPixelShaderParser::ParseProgram (iDataBuffer* program)
{
  version = CS_PS_INVALID;

  csString prog;
  prog.Append ((char*)program->GetData(), program->GetSize());

  // Trim any leading/trailing blank lines
  prog.Trim ();

  size_t len = prog.Length ();

  if (len == 0)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
      "Empty program!");
    return false;
  }


  max_registers[CS_PS_REG_TEX] = 4;
  max_registers[CS_PS_REG_TEMP] = 2;
  max_registers[CS_PS_REG_CONSTANT] = 8;
  max_registers[CS_PS_REG_COLOR] = 2;

  bool hasVersion = false;
  csStringReader reader (prog);
  csString line;
  int lineCount = 0;
  while (reader.HasMoreLines ())
  {
    if (!reader.GetLine (line)) break;
    lineCount++;
    csPSProgramInstruction inst;
    if(!GetInstruction (line, inst)) return false;

    // Probably a blank line or comment ... ignore
    if(inst.instruction == CS_PS_INS_INVALID) continue;

    // Identify the version
    bool isVerInstr = (inst.instruction >= CS_PS_INS_PS_1_1) &&
      (inst.instruction <= CS_PS_INS_PS_1_4);
    if (!hasVersion)
    {
      if (!isVerInstr)
      {
	Report (CS_REPORTER_SEVERITY_WARNING, 
	  "Expected version, got %s", GetInstructionName (inst.instruction));
	return false;
      }

      switch (inst.instruction)
      {
	case CS_PS_INS_PS_1_1:
	  version = CS_PS_1_1;
	  hasVersion = true;
	  break;
	case CS_PS_INS_PS_1_2:
	  version = CS_PS_1_2;
	  hasVersion = true;
	  break;
	case CS_PS_INS_PS_1_3:
	  version = CS_PS_1_3;
	  hasVersion = true;
	  break;
	case CS_PS_INS_PS_1_4:
	  version = CS_PS_1_4;
	  hasVersion = true;
	  max_registers[CS_PS_REG_TEX] = 6;
	  max_registers[CS_PS_REG_TEMP] = 6;
	  break;
	default:
	  break;
      }
    }
    else if (isVerInstr)
    {
      Report (CS_REPORTER_SEVERITY_WARNING, 
	"Invalid version instruction %s in line %d", 
	GetInstructionName (inst.instruction), lineCount);
      return false;
    }

    if(!(version & PS_Instructions[inst.instruction].versions))
    {
      Report (CS_REPORTER_SEVERITY_ERROR, 
	"Pixel Shader version %s does not support instruction '%s'",
	GetVersionString (version), GetInstructionName (inst.instruction));
      return false;
    }

    if (!isVerInstr)
      program_instructions.Push (inst);
  }

  return true;
}

#ifdef CS_DEBUG

static char GetRegType (csPSRegisterType type)
{
  switch (type)
  {
    case CS_PS_REG_TEX:		return 't'; break;
    case CS_PS_REG_CONSTANT:	return 'c'; break;
    case CS_PS_REG_TEMP:	return 'r'; break;
    case CS_PS_REG_COLOR:	return 'v'; break;
    default:			return '?'; break;
  }
}

static void GetSrcRegname (csPSRegisterType type, int num, uint mods, 
			   csString& str)
{
  if (mods & CS_PS_RMOD_NEGATE) str << '-';
  if (mods & CS_PS_RMOD_INVERT) str << "1-";

  str << GetRegType (type);
  str << num;

  const uint baseMods = (CS_PS_RMOD_BIAS | CS_PS_RMOD_SCALE);
  if (mods & baseMods) str << '_';
  if (mods & CS_PS_RMOD_BIAS) str << 'b';
  if (mods & CS_PS_RMOD_SCALE) str << "x2";

  if (mods & CS_PS_RMOD_DZ) str << "_dz";
  if (mods & CS_PS_RMOD_DW) str << "_dw";

  const uint repMask = (CS_PS_RMOD_REP_RED | CS_PS_RMOD_REP_GREEN | 
    CS_PS_RMOD_REP_BLUE | CS_PS_RMOD_REP_ALPHA);
  if (mods & repMask) str << '.';
  if (mods & CS_PS_RMOD_REP_RED) str << 'r';
  if (mods & CS_PS_RMOD_REP_GREEN) str << 'g';
  if (mods & CS_PS_RMOD_REP_BLUE) str << 'b';
  if (mods & CS_PS_RMOD_REP_ALPHA) str << 'a';

  if (mods & CS_PS_RMOD_XYZ) str << ".xyz";
  if (mods & CS_PS_RMOD_XYW) str << ".xyw";
}
#endif

void csPixelShaderParser::GetInstructionString (
  const csPSProgramInstruction& instr, csString& str)
{
  str << instrStrings.Request (instr.instruction);
  if (instr.inst_mods & CS_PS_IMOD_X2) str << "_x2";
  if (instr.inst_mods & CS_PS_IMOD_X4) str << "_x4";
  if (instr.inst_mods & CS_PS_IMOD_X8) str << "_x8";
  if (instr.inst_mods & CS_PS_IMOD_D2) str << "_d2";
  if (instr.inst_mods & CS_PS_IMOD_D4) str << "_d4";
  if (instr.inst_mods & CS_PS_IMOD_D8) str << "_d8";
  if (instr.inst_mods & CS_PS_IMOD_SAT) str << "_sat";
}

#ifdef CS_DEBUG
void csPixelShaderParser::WriteProgram (
	const csArray<csPSProgramInstruction>& instrs, 
	csString& str)
{
  for(size_t i = 0; i < instrs.Length (); i++)
  {
    const csPSProgramInstruction& instr = instrs.Get (i);

    csString instrStr;
    GetInstructionString (instr, instrStr);
    str << instrStr;

    str << ' ';
    str << GetRegType (instr.dest_reg);
    str << instr.dest_reg_num;

    if (instr.dest_reg_mods != 0) str << '.';
    if (instr.dest_reg_mods & CS_PS_WMASK_RED) str << 'r';
    if (instr.dest_reg_mods & CS_PS_WMASK_GREEN) str << 'g';
    if (instr.dest_reg_mods & CS_PS_WMASK_BLUE) str << 'b';
    if (instr.dest_reg_mods & CS_PS_WMASK_ALPHA) str << 'a';

    for (int j = 0; j < 3; j++)
    {
      if (instr.src_reg[j] == CS_PS_REG_NONE) break;
      str << ", ";
      
      GetSrcRegname (instr.src_reg[j], instr.src_reg_num[j], 
	instr.src_reg_mods[j], str);
    }

    str << '\n';
  }
}
#endif
