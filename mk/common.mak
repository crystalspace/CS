#----------------------------------------------------#
# This makefile contains definitions that are common
# for both root and cs makefiles
#----------------------------------------------------#

# Several symbols with special meaning
# The following macro should contain TWO empty lines
define NEWLINE


endef
COMMA=,
EMPTY=
SPACE=$(EMPTY) $(EMPTY)

# The suffixes for $(OUT) directory when making PIC and non-PIC code
# Can be changed from system-dependent makefile
OUTSUFX. =
OUTSUFX.no =
OUTSUFX.yes = .pic

# Depending on the type of optimization choosen we disable assembler support.
ifneq ($(MODE),optimize)
ifneq ($(USE_NASM),yes)
  DO_ASM=no
endif
endif
ifeq ($(DO_ASM),no)
  USE_NASM=no
  DO_MMX=no
endif

# This macro should update target only if it has changed
define UPD
  cmp -s $@ DEST || rm -f DEST && cp $@ DEST
  rm -f $@
endef

# Macro used to build a subtarget
define MAKE_TARGET
  @echo $",------=======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx=======------$"
  @echo $"$| Compiling $(DESCRIPTION.$@)$"
  @echo $"$| Compiling for $(OS)/$(COMP)/$(PROC) in $(MODE) mode$"
  @echo $"`----------==============xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx==============----------$"
  @$(MAKE) --no-print-directory -f mk/cs.mak $@
endef

ifeq ($(ROOTCONFIG),volatile)

# This macro is used to rebuild "volatile.h"
# You're free to add any commands you want to it in submakefiles
define MAKE_VOLATILE_H
  echo $"#define OS_$(OS)$">>volatile.tmp
  echo $"#define PROC_$(PROC)$">>volatile.tmp
  echo $"#define COMP_$(COMP)$">>volatile.tmp
endef
ifeq ($(USE_DLL),no)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define CS_STATIC_LINKED$">>volatile.tmp
endif
ifeq ($(BUGGY_EGCS_COMPILER),yes)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define BUGGY_EGCS_COMPILER$">>volatile.tmp
endif
ifeq ($(MODE),debug)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define DEBUG$">>volatile.tmp
endif
ifneq ($(NATIVE_COM),yes)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define NO_COM_SUPPORT$">>volatile.tmp
endif
ifeq ($(DO_SOUND),yes)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define DO_SOUND$">>volatile.tmp
endif
ifneq ($(DO_ASM),yes)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define NO_ASSEMBLER$">>volatile.tmp
endif
ifeq ($(USE_NASM),yes)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define DO_NASM$">>volatile.tmp
endif
ifeq ($(DO_MMX),yes)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define DO_MMX$">>volatile.tmp
endif

endif # ifeq ($(ROOTCONFIG),volatile)
