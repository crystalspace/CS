# This is an submakefile which defines basical rules for building volatile.h

ifeq ($(USE_DLL),no)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define CS_STATIC_LINKED$">>volatile.tmp
endif
ifeq ($(BUGGY_EGCS_COMPILER),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define BUGGY_EGCS_COMPILER$">>volatile.tmp
endif
ifeq ($(MODE),debug)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DEBUG$">>volatile.tmp
endif
ifneq ($(NATIVE_COM),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define NO_COM_SUPPORT$">>volatile.tmp
endif
ifeq ($(DO_SOUND),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_SOUND$">>volatile.tmp
endif
ifneq ($(DO_ASM),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define NO_ASSEMBLER$">>volatile.tmp
endif
ifeq ($(USE_NASM),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_NASM$">>volatile.tmp
endif
ifeq ($(DO_MMX),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define DO_MMX$">>volatile.tmp
endif
ifeq ($(NEED_FAKE_BOOL),yes)
  MAKE_VOLATILE_H+=$(NEWLINE)echo $"\#define NO_BOOL_TYPE$">>volatile.tmp
endif
