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

# If no-one's supplied a value for $(FORCEBUCK), turn that feature off
ifeq ($(strip $(FORCEBUCK)),)
  FORCEBUCK = no
endif

# If $(FORCEBUCK) is set, use it to set the value of $(BUCK) manually
ifeq ($(strip $(FORCEBUCK)),SS)
  BUCK = $$
endif
ifeq ($(strip $(FORCEBUCK)),BSS)
  BUCK = \$$
endif

# Unix shells tend to use "$" as delimiter for variable names.
# Test for this behaviour and set $(BUCK) variable correspondigly
# if someone hasn't already supplied a value.
ifeq ($(strip $(BUCK)),)
  __TMP__:=$(shell echo $$$$)
  ifeq ($(__TMP__),$$$$)
    BUCK = $$
  else
    BUCK = \$$
  endif
endif

# The suffixes for $(OUT) directory when making PIC and non-PIC code
# Can be changed from system-dependent makefile
OUTSUFX. =
OUTSUFX.no =
OUTSUFX.yes = .pic

# This macro should update target only if it has changed
define UPD
  cmp -s $@ DEST || (rm -f DEST && cp $@ DEST)
  rm -f $@
endef

# Macro used to build a subtarget
define MAKE_TARGET
  @echo $(SEPARATOR)
  @echo $"  Building $(DESCRIPTION.$@)$"
  @echo $"  Building for $(OS)/$(COMP)/$(PROC) in $(MODE) mode$"
  @echo $(SEPARATOR)
  +@$(MAKE) $(RECMAKEFLAGS) -f mk/cs.mak $@
endef

# Macro used to make a sub-clean target
define MAKE_CLEAN
  @echo $(SEPARATOR)
  @echo $"  Cleaning up the $(DESCRIPTION.$(subst clean,,$@))$"
  @echo $(SEPARATOR)
  +@$(MAKE) $(RECMAKEFLAGS) -f mk/cs.mak $@
endef
