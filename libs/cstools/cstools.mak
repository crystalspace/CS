# Library description
DESCRIPTION.cstools = Crystal Space application tools library

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make cstools      Make the $(DESCRIPTION.cstools)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: cstools

all libs: cstools
cstools:
	$(MAKE_TARGET)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cstools

CSTOOLS.LIB = $(OUT)$(LIB_PREFIX)cstools$(LIB)
SRC.CSTOOLS = $(wildcard libs/cstools/*.cpp)
OBJ.CSTOOLS = $(addprefix $(OUT),$(notdir $(SRC.CSTOOLS:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: cstools cstoolsclean

all: $(CSTOOLS.LIB)
cstools: $(OUTDIRS) $(CSTOOLS.LIB)
clean: cstoolsclean

$(CSTOOLS.LIB): $(OBJ.CSTOOLS)
	$(DO.STATIC.LIBRARY)

cstoolsclean:
	-$(RM) $(CSTOOLS.LIB)

ifdef DO_DEPEND
$(OUTOS)cstools.dep: $(SRC.CSTOOLS)
	$(DO.DEP)
endif

-include $(OUTOS)cstools.dep

endif # ifeq ($(MAKESECTION),targets)
