# This is an submakefile which defines basical rules for building volatile.h

DO.MAKE.VOLATILE=$(VOLATILE_H.ALL)

# Blank lines in defines are significant.  Do not remove them.
VOLATILE_H.ALL=$(VOLATILE_H.PREFIX) $(VOLATILE_H.CONTENT) $(VOLATILE_H.SUFFIX)
VOLATILE_H.CONTENT=$(MAKE_VOLATILE_H)
define VOLATILE_H.PREFIX
  echo $"// Do not change.  File is generated automatically.$">volatile.tmp
  echo $"#ifndef __CS_VOLATILE_H__$">>volatile.tmp
  echo $"#define __CS_VOLATILE_H__$">>volatile.tmp

endef
define VOLATILE_H.SUFFIX

  echo $"#endif // __CS_VOLATILE_H__$">>volatile.tmp
endef

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
ifeq ($(DO_FAKE_BOOL),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_FAKE_BOOL$">>volatile.tmp
endif
ifeq ($(CS_BUILTIN_SIZED_TYPES),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_BUILTIN_SIZED_TYPES$">>volatile.tmp
endif
ifeq ($(DO_FAKE_SOCKLEN_T),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_FAKE_SOCKLEN_T$">>volatile.tmp
endif
ifeq ($(DO_COREDUMP),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_COREDUMP$">>volatile.tmp
endif
ifdef CS_LITTLE_ENDIAN
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_LITTLE_ENDIAN$">>volatile.tmp
endif
ifdef CS_BIG_ENDIAN
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_BIG_ENDIAN$">>volatile.tmp
endif
