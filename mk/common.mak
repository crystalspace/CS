#----------------------------------------------------#
# This makefile contains definitions that are common
# for both root and cs makefiles
#----------------------------------------------------#

# Several symbols with special meaning
# the following macro should contain TWO empty lines
define NEWLINE


endef
COMMA=,
EMPTY=
SPACE=$(EMPTY) $(EMPTY)

# Friendly names for building environments
DESCRIPTION.linux	= Linux
DESCRIPTION.solaris	= Solaris
DESCRIPTION.irix   	= IRIX
DESCRIPTION.freebsd	= FreeBSD
DESCRIPTION.beos	= BeOS
DESCRIPTION.os2gcc	= OS/2 with GCC/EMX
DESCRIPTION.os2wcc	= OS/2 with Watcom C
DESCRIPTION.djgpp	= DOS with DJGPP
DESCRIPTION.nextstep	= NextStep 3.3
DESCRIPTION.openstep	= OpenStep 4.2
DESCRIPTION.rhapsody	= Rhapsody (MacOS/X Server) DR2
DESCRIPTION.amiga	= Amiga with GCC
DESCRIPTION.unknown	= Unknown invalid target

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

# Macro used to build a subtarget
define MAKE_TARGET
	@echo $",------=======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx=======------$"
	@echo $"$| Compiling $(DESCRIPTION.$@)$"
	@echo $"$| Compiling for $(OS)/$(COMP)/$(PROC) in $(MODE) mode$"
	@echo $"`----------==============xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx==============----------$"
	@$(MAKE) --no-print-directory -f mk/cs.mak $@
endef

# This macro is used to rebuild "volatile.h"
# You're free to add any commands you want to it in submakefiles
define MAKE_VOLATILE_H
	echo $"#define OS_$(OS)$">>$@
	echo $"#define PROC_$(PROC)$">>$@
	echo $"#define COMP_$(COMP)$">>$@
endef
ifeq ($(USE_DLL),no)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define CS_STATIC_LINKED$">>$@
endif
ifeq ($(BUGGY_EGCS_COMPILER),yes)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define BUGGY_EGCS_COMPILER$">>$@
endif
ifeq ($(MODE),debug)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define DEBUG$">>$@
endif
ifneq ($(NATIVE_COM),yes)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define NO_COM_SUPPORT$">>$@
endif
ifeq ($(DO_SOUND),yes)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define DO_SOUND$">>$@
endif
ifneq ($(DO_ASM),yes)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define NO_ASSEMBLER$">>$@
endif
ifeq ($(USE_NASM),yes)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define DO_NASM$">>$@
endif
ifeq ($(DO_MMX),yes)
  MAKE_VOLATILE_H += $(NEWLINE)echo $"\#define DO_MMX$">>$@
endif
