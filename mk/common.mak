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
SEPARATOR=$"*-------------------------------------------------------------------------*$"

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
  @echo $(SEPARATOR)
  @echo $"  Compiling $(DESCRIPTION.$@)$"
  @echo $"  Compiling for $(OS)/$(COMP)/$(PROC) in $(MODE) mode$"
  @echo $(SEPARATOR)
  @$(MAKE) --no-print-directory -f mk/cs.mak $@
endef
