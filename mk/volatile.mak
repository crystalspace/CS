# This is an submakefile which defines basical rules for building volatile.h

# Platforms which need to perform some sort of special transformation on the
# entire content of volatile.h before it is written to disk may override this
# definition.
DO.MAKE.VOLATILE=$(VOLATILE_H.ALL)

# Blank lines in defines are significant.  Do not remove them.
VOLATILE_H.ALL=$(VOLATILE_H.PREFIX) $(VOLATILE_H.CONTENT) $(VOLATILE_H.SUFFIX)
VOLATILE_H.CONTENT=$(MAKE_VOLATILE_H)
define VOLATILE_H.PREFIX
  echo $"// Do not change.  File generated automatically.$">volatile.tmp
  echo $"#ifndef __CS_VOLATILE_H__$">>volatile.tmp
  echo $"#define __CS_VOLATILE_H__$">>volatile.tmp

endef
define VOLATILE_H.SUFFIX

  echo $"#endif // __CS_VOLATILE_H__$">>volatile.tmp
endef
define VOLATILE_H.OS_FAMILY

  echo $"#if !defined($(OS_FAMILY.$(OS_FAMILY)))$">>volatile.tmp
  echo $"#  define $(OS_FAMILY.$(OS_FAMILY))$">>volatile.tmp
  echo $"#endif$">>volatile.tmp

endef

OS_FAMILY.UNIX=OS_UNIX

PROC.NAME.X86=X86
PROC.NAME.SPARC=Sparc
PROC.NAME.MIPS=MIPS
PROC.NAME.POWERPC=PowerPC
PROC.NAME.M68K=M68K
PROC.NAME.HPPA=PA-RISC
PROC.NAME.ALPHA=Alpha

ifeq ($(PROC.NAME),)
  PROC.NAME=$(PROC.NAME.$(PROC))
endif

COMP.NAME.GCC=GCC
COMP.NAME.VC=VisualC
COMP.NAME.BCC32=Borland
COMP.NAME.MWERKS=MetroWorks

ifeq ($(COMP.NAME),)
  COMP.NAME=$(COMP.NAME.$(COMP))
endif

MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define OS_$(OS)$">>volatile.tmp
MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define PROC_$(PROC)$">>volatile.tmp
MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define COMP_$(COMP)$">>volatile.tmp

ifneq ($(DESCRIPTION.$(TARGET)),)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_PLATFORM_NAME "$(DESCRIPTION.OS.$(TARGET))"$">>volatile.tmp
endif
ifneq ($(PROC.NAME),)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_PROCESSOR_NAME "$(PROC.NAME)"$">>volatile.tmp
endif
ifneq ($(COMP.NAME),)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_COMPILER_NAME "$(COMP.NAME)"$">>volatile.tmp
endif
ifeq ($(USE_PLUGINS),no)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_STATIC_LINKED$">>volatile.tmp
endif
ifeq ($(DO_SOUND),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_SOUND$">>volatile.tmp
endif
ifeq ($(NASM.INSTALLED),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_NASM$">>volatile.tmp
endif
ifeq ($(DO_MMX),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_MMX$">>volatile.tmp
endif
ifeq ($(CS_USE_FAKE_BOOL_TYPE),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_USE_FAKE_BOOL_TYPE$">>volatile.tmp
endif
ifeq ($(CS_USE_FAKE_EXPLICIT_KEYWORD),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_USE_FAKE_EXPLICIT_KEYWORD$">>volatile.tmp
endif
ifeq ($(CS_USE_OLD_STYLE_CASTS),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_USE_OLD_STYLE_CASTS$">>volatile.tmp
endif
ifeq ($(CS_BUILTIN_SIZED_TYPES),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_BUILTIN_SIZED_TYPES$">>volatile.tmp
endif
ifeq ($(CS_USE_FAKE_SOCKLEN_TYPE),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_USE_FAKE_SOCKLEN_TYPE$">>volatile.tmp
endif
ifeq ($(CS_NO_QSQRT),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_NO_QSQRT$">>volatile.tmp
endif
ifeq ($(CS_QINT_WORKAROUND),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_QINT_WORKAROUND$">>volatile.tmp
endif
ifdef CS_LITTLE_ENDIAN
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_LITTLE_ENDIAN$">>volatile.tmp
endif
ifdef CS_BIG_ENDIAN
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_BIG_ENDIAN$">>volatile.tmp
endif
ifneq ($(OS_FAMILY),)
  MAKE_VOLATILE_H+=$(VOLATILE_H.OS_FAMILY)
endif
