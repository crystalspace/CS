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

# Macros used to build a subtarget
define MAKE_TARGET
	@echo $",------=======xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx=======------$"
	@echo $"$| Compiling $(DESCRIPTION.$@)$"
	@echo $"$| Compiling for $(OS)/$(COMP)/$(PROC) in $(MODE) mode$"
	@echo $"`----------==============xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx==============----------$"
	@$(MAKE) --no-print-directory -f mk/cs.mak $@
endef
