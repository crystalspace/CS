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
cstoolsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cstools

CSTOOLS.LIB = $(OUT)$(LIB_PREFIX)cstools$(LIB_SUFFIX)
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
	$(DO.LIBRARY)

cstoolsclean:
	-$(RM) $(CSTOOLS.LIB) $(OBJ.CSTOOLS)

ifdef DO_DEPEND
depend: $(OUTOS)cstools.dep
$(OUTOS)cstools.dep: $(SRC.CSTOOLS)
	$(DO.DEP)
else
-include $(OUTOS)cstools.dep
endif

endif # ifeq ($(MAKESECTION),targets)
