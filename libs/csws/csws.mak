# Library description
DESCRIPTION.csws = Crystal Space Windowing System library

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make csws         Make the $(DESCRIPTION.csws)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csws

all libs: csws
csws:
	$(MAKE_TARGET)
cswsclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csws

CSWS.LIB = $(OUT)$(LIB_PREFIX)csws$(LIB_SUFFIX)
SRC.CSWS = $(wildcard libs/csws/*.cpp)
OBJ.CSWS = $(addprefix $(OUT),$(notdir $(SRC.CSWS:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csws cswsclean

all: $(CSWS.LIB)
csws: $(OUTDIRS) $(CSWS.LIB)
clean: cswsclean

$(CSWS.LIB): $(OBJ.CSWS)
	$(DO.LIBRARY)

cswsclean:
	-$(RM) $(CSWS.LIB) $(OBJ.CSWS)

ifdef DO_DEPEND
depend: $(OUTOS)csws.dep
$(OUTOS)csws.dep: $(SRC.CSWS)
	$(DO.DEP)
else
-include $(OUTOS)csws.dep
endif

endif # ifeq ($(MAKESECTION),targets)
