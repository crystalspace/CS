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

#include "video/canvas/openglcommon/glextmanager.h"

#include "glshader_ps1.h"
#include "ps1_parser.h"

void csPixelShaderParser::RegisterInstructions ()
{
  // Just to make sure something doesn't accidentally get
  // mapped to the INVALID instruction placeholder
  PS_Instructions[CS_PS_INS_INVALID].id = strings.Request ("[INVALID]");

  for(int i=0;i<CS_PS_INS_END_OF_LIST;i++)
  {
    // Default all to supported
    PS_Instructions[i].supported = true;
  }
  // Arithmetic instructions (Color operations)
  PS_Instructions[CS_PS_INS_ADD].id = strings.Request ("add");
  PS_Instructions[CS_PS_INS_ADD].versions = CS_PS_ALLVERSIONS;
  PS_Instructions[CS_PS_INS_ADD].arguments = 3;
  PS_Instructions[CS_PS_INS_BEM].id = strings.Request ("bem");
  PS_Instructions[CS_PS_INS_BEM].versions = CS_PS_1_4;
  PS_Instructions[CS_PS_INS_BEM].arguments = 3;
  PS_Instructions[CS_PS_INS_CMP].id = strings.Request ("cmp");
  PS_Instructions[CS_PS_INS_CMP].versions = CS_PS_1_2 | CS_PS_1_3 | CS_PS_1_4;
  PS_Instructions[CS_PS_INS_CMP].arguments = 4;
  PS_Instructions[CS_PS_INS_CND].id = strings.Request ("cnd");
  PS_Instructions[CS_PS_INS_CND].versions = CS_PS_ALLVERSIONS;
  PS_Instructions[CS_PS_INS_CND].arguments = 4;
  PS_Instructions[CS_PS_INS_DP3].id = strings.Request ("dp3");
  PS_Instructions[CS_PS_INS_DP3].versions = CS_PS_ALLVERSIONS;
  PS_Instructions[CS_PS_INS_DP3].arguments = 3;
  PS_Instructions[CS_PS_INS_DP4].id = strings.Request ("dp4");
  PS_Instructions[CS_PS_INS_DP4].versions = CS_PS_1_2 | CS_PS_1_3 | CS_PS_1_4;
  PS_Instructions[CS_PS_INS_DP4].arguments = 3;
  PS_Instructions[CS_PS_INS_LRP].id = strings.Request ("lrp");
  PS_Instructions[CS_PS_INS_LRP].versions = CS_PS_ALLVERSIONS;
  PS_Instructions[CS_PS_INS_LRP].arguments = 4;
  PS_Instructions[CS_PS_INS_MAD].id = strings.Request ("mad");
  PS_Instructions[CS_PS_INS_MAD].versions = CS_PS_ALLVERSIONS;
  PS_Instructions[CS_PS_INS_MAD].arguments = 4;
  PS_Instructions[CS_PS_INS_MOV].id = strings.Request ("mov");
  PS_Instructions[CS_PS_INS_MOV].versions = CS_PS_ALLVERSIONS;
  PS_Instructions[CS_PS_INS_MOV].arguments = 2;
  PS_Instructions[CS_PS_INS_MUL].id = strings.Request ("mul");
  PS_Instructions[CS_PS_INS_MUL].versions = CS_PS_ALLVERSIONS;
  PS_Instructions[CS_PS_INS_MUL].arguments = 3;
  PS_Instructions[CS_PS_INS_NOP].id = strings.Request ("nop");
  PS_Instructions[CS_PS_INS_NOP].versions = CS_PS_ALLVERSIONS;
  PS_Instructions[CS_PS_INS_NOP].arguments = 0;
  PS_Instructions[CS_PS_INS_SUB].id = strings.Request ("sub");
  PS_Instructions[CS_PS_INS_SUB].versions = CS_PS_ALLVERSIONS;
  PS_Instructions[CS_PS_INS_SUB].arguments = 3;
  // Texture instructions
  PS_Instructions[CS_PS_INS_TEX].id = strings.Request ("tex");
  PS_Instructions[CS_PS_INS_TEX].versions = CS_PS_OLDVERSIONS;
  PS_Instructions[CS_PS_INS_TEX].arguments = 1;
  PS_Instructions[CS_PS_INS_TEXBEM].id = strings.Request ("texbem");
  PS_Instructions[CS_PS_INS_TEXBEM].versions = CS_PS_OLDVERSIONS;
  PS_Instructions[CS_PS_INS_TEXBEM].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXBEML].id = strings.Request ("texbeml");
  PS_Instructions[CS_PS_INS_TEXBEML].versions = CS_PS_OLDVERSIONS;
  PS_Instructions[CS_PS_INS_TEXBEML].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXCOORD].id = strings.Request ("texcoord");
  PS_Instructions[CS_PS_INS_TEXCOORD].versions = CS_PS_OLDVERSIONS;
  PS_Instructions[CS_PS_INS_TEXCOORD].arguments = 1;
  PS_Instructions[CS_PS_INS_TEXCRD].id = strings.Request ("texcrd");
  PS_Instructions[CS_PS_INS_TEXCRD].versions = CS_PS_1_4;
  PS_Instructions[CS_PS_INS_TEXCRD].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXDEPTH].id = strings.Request ("texdepth");
  PS_Instructions[CS_PS_INS_TEXDEPTH].versions = CS_PS_1_4;
  PS_Instructions[CS_PS_INS_TEXDEPTH].arguments = 1;
  PS_Instructions[CS_PS_INS_TEXDP3].id = strings.Request ("texdp3");
  PS_Instructions[CS_PS_INS_TEXDP3].versions = CS_PS_1_2 | CS_PS_1_3;
  PS_Instructions[CS_PS_INS_TEXDP3].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXDP3TEX].id = strings.Request ("texdp3tex");
  PS_Instructions[CS_PS_INS_TEXDP3TEX].versions = CS_PS_1_2 | CS_PS_1_3;
  PS_Instructions[CS_PS_INS_TEXDP3TEX].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXKILL].id = strings.Request ("texkill");
  PS_Instructions[CS_PS_INS_TEXKILL].versions = CS_PS_ALLVERSIONS;
  PS_Instructions[CS_PS_INS_TEXKILL].arguments = 1;
  PS_Instructions[CS_PS_INS_TEXLD].id = strings.Request ("texld");
  PS_Instructions[CS_PS_INS_TEXLD].versions = CS_PS_1_4;
  PS_Instructions[CS_PS_INS_TEXLD].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXM3X2DEPTH].id = strings.Request ("texm3x2depth");
  PS_Instructions[CS_PS_INS_TEXM3X2DEPTH].versions = CS_PS_1_3;
  PS_Instructions[CS_PS_INS_TEXM3X2DEPTH].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXM3X2PAD].id = strings.Request ("texm3x2pad");
  PS_Instructions[CS_PS_INS_TEXM3X2PAD].versions = CS_PS_OLDVERSIONS;
  PS_Instructions[CS_PS_INS_TEXM3X2PAD].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXM3X2TEX].id = strings.Request ("texm3x2tex");
  PS_Instructions[CS_PS_INS_TEXM3X2TEX].versions = CS_PS_OLDVERSIONS;
  PS_Instructions[CS_PS_INS_TEXM3X2TEX].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXM3X3].id = strings.Request ("texm3x3");
  PS_Instructions[CS_PS_INS_TEXM3X3].versions = CS_PS_1_2 | CS_PS_1_3;
  PS_Instructions[CS_PS_INS_TEXM3X3].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXM3X3PAD].id = strings.Request ("texm3x3pad");
  PS_Instructions[CS_PS_INS_TEXM3X2PAD].versions = CS_PS_OLDVERSIONS;
  PS_Instructions[CS_PS_INS_TEXM3X2PAD].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXM3X3SPEC].id = strings.Request ("texm3x3spec");
  PS_Instructions[CS_PS_INS_TEXM3X3SPEC].versions = CS_PS_OLDVERSIONS;
  PS_Instructions[CS_PS_INS_TEXM3X3SPEC].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXM3X3TEX].id = strings.Request ("texm3x3tex");
  PS_Instructions[CS_PS_INS_TEXM3X3TEX].versions = CS_PS_OLDVERSIONS;
  PS_Instructions[CS_PS_INS_TEXM3X3TEX].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXM3X3VSPEC].id = strings.Request ("texm3x3vspec");
  PS_Instructions[CS_PS_INS_TEXM3X3VSPEC].versions = CS_PS_OLDVERSIONS;
  PS_Instructions[CS_PS_INS_TEXM3X3VSPEC].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXREG2AR].id = strings.Request ("texreg2ar");
  PS_Instructions[CS_PS_INS_TEXREG2AR].versions = CS_PS_1_2 | CS_PS_1_3;
  PS_Instructions[CS_PS_INS_TEXREG2AR].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXREG2GB].id = strings.Request ("texreg2gb");
  PS_Instructions[CS_PS_INS_TEXREG2GB].versions = CS_PS_1_2 | CS_PS_1_3;
  PS_Instructions[CS_PS_INS_TEXREG2GB].arguments = 2;
  PS_Instructions[CS_PS_INS_TEXREG2RGB].id = strings.Request ("texreg2rgb");
  PS_Instructions[CS_PS_INS_TEXREG2RGB].versions = CS_PS_1_2 | CS_PS_1_3;
  PS_Instructions[CS_PS_INS_TEXREG2RGB].arguments = 2;
  // Phase Instruction (PS 1.4 only)
  PS_Instructions[CS_PS_INS_PHASE].id = strings.Request ("phase");
  PS_Instructions[CS_PS_INS_PHASE].versions = CS_PS_1_4;

  // Unsupported instructions
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
  int start = str.FindFirst (' ');
  if(start < 0) return 0;

  int argument = 0;
  int len = str.Length ();
  while(argument < 5 && start < len)
  {
    int end = str.FindFirst (',', start + 1);
    if(end < 0) end = str.Length ();
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
  if(version == CS_PS_1_4)
  {
    if(strstr(reg, ".r")) mods |= CS_PS_WMASK_RED;
    if(strstr(reg, ".g")) mods |= CS_PS_WMASK_GREEN;
    if(strstr(reg, ".b")) mods |= CS_PS_WMASK_BLUE;
    if(strstr(reg, ".a")) mods |= CS_PS_WMASK_ALPHA;
  }
  else
  {
    if(strstr(reg, ".rgba")) mods = (CS_PS_WMASK_RED |
      CS_PS_WMASK_GREEN | CS_PS_WMASK_BLUE | CS_PS_WMASK_ALPHA);
    else if(strstr(reg, ".rgb")) mods = (CS_PS_WMASK_RED |
      CS_PS_WMASK_GREEN | CS_PS_WMASK_BLUE);
    else if(strstr(reg, ".a")) mods = CS_PS_WMASK_ALPHA;
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

  if(version==CS_PS_INVALID) return false;

  csString line (str);
  line.Downcase ();

  int comment = line.FindFirst ('/');
  if(comment >= 0 && line.GetAt (comment + 1) == '/')
    line.Truncate (comment);
  comment = line.FindFirst (';');
  if(comment >= 0) line.Truncate (comment);
  line.Trim ();
  
  if(line.Length() < 1) return true; // Ignore blank lines

  csString istr;
  line.SubString (istr, 0, line.FindFirst (' '));
  csStringID inst_id = (csStringID)~0;

  if(!strcmp(istr, "def"))
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

  int mod_pos = line.FindFirst ('_');
  if(mod_pos >= 0)
  {
    csString unmod;
    istr.SubString (unmod, 0, mod_pos);
    inst_id = strings.Request (unmod);
  }
  else
  {
    inst_id = strings.Request (istr);
  }

  // Find the instruction based on its string ID
  int i;
  for(i=0;i<CS_PS_INS_END_OF_LIST;i++)
  {
    if(PS_Instructions[i].id == inst_id) break;
  }
  if(i==CS_PS_INS_INVALID || i==CS_PS_INS_END_OF_LIST) return false;
  if(!(version & PS_Instructions[i].versions))
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
      "Pixel Shader version %s does not support instruction '%s'",
      version_string.GetData (), istr.GetData ());
    return false;
  }
  if(!PS_Instructions[i].supported)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
      "Pixel shader instruction '%s' is not supported at this time",
      istr.GetData ());
    return false;
  }
  
  inst.instruction = i;

  // Find the instruction modifier(s)

  inst.inst_mods = CS_PS_IMOD_NONE;

  if(strstr(istr, "_x2")) inst.inst_mods |= CS_PS_IMOD_X2;
  else if(strstr(istr, "_x4")) inst.inst_mods |= CS_PS_IMOD_X4;
  else if(strstr(istr, "_x8")) inst.inst_mods |= CS_PS_IMOD_X8;
  else if(strstr(istr, "_d2")) inst.inst_mods |= CS_PS_IMOD_D2;
  else if(strstr(istr, "_d4")) inst.inst_mods |= CS_PS_IMOD_D4;
  else if(strstr(istr, "_d8")) inst.inst_mods |= CS_PS_IMOD_D8;
  // _sat can be combined with _xn or _dn
  if(strstr(istr, "_sat")) inst.inst_mods |= CS_PS_IMOD_SAT;

  csString dest, src1, src2, src3, trash;
  if(GetArguments(line, dest, src1, src2, src3, trash)
    != PS_Instructions[i].arguments)
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
      "Incorrect number of arguments for Pixel Shader instruction '%s'!",
      istr.GetData ());
    return false;
  }

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

  inst.src_reg[0] = CS_PS_REG_NONE;
  inst.src_reg[1] = CS_PS_REG_NONE;
  inst.src_reg[2] = CS_PS_REG_NONE;

  // Get source register(s)
  for(int j=0;j<PS_Instructions[i].arguments - 1;j++)
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
    while(p[0] != 'c' && p[0] != 'v' && p[0] != 't' && p[0] != 'r' && p[0]!='\0') p++;
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

  return true;
}

bool csPixelShaderParser::ParseProgram (const char *program)
{
  version = CS_PS_INVALID;

  csString prog(program);

  // Trim any leading/trailing blank lines
  prog.Trim ();

  max_registers[CS_PS_REG_TEX] = 4;
  max_registers[CS_PS_REG_TEMP] = 2;
  max_registers[CS_PS_REG_CONSTANT] = 8;
  max_registers[CS_PS_REG_COLOR] = 2;

  // Identify the version
  if(!strncmp(prog, "ps_1_1", 6)) version = CS_PS_1_1;
  else if(!strncmp(prog, "ps_1_2", 6)) version = CS_PS_1_2;
  else if(!strncmp(prog, "ps_1_3", 6)) version = CS_PS_1_3;
  else if(!strncmp(prog, "ps_1_4", 6)) 
  {
    max_registers[CS_PS_REG_TEX] = 6;
    max_registers[CS_PS_REG_TEMP] = 6;
    version = CS_PS_1_4;
  }
  else
  {
    Report (CS_REPORTER_SEVERITY_ERROR, 
      "No version instruction found!");
    return false;
  }

  prog.SubString (version_string, 0, 6);

  // Start position at 8 to skip version inst
  int pos = 8, len = prog.Length ();
  while(pos < len)
  {
    if(pos >= len) break;
    csString line;
    int end = prog.FindFirst ('\n', pos);
    if(end<0) end = len;
    prog.SubString (line, pos, end - pos);
    csPSProgramInstruction inst;
    if(!GetInstruction (line, inst)) return false;

    pos = end + 1;

    // Probably a blank line or comment ... ignore
    if(inst.instruction == CS_PS_INS_INVALID) continue;

    program_instructions.Push (inst);
  }

  return true;
}
