# Library description
DESCRIPTION.csobject = Crystal Space component object library

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make csobject     Make the $(DESCRIPTION.csobject)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csobject

all libs: csobject
csobject:
	$(MAKE_TARGET)
csobjectclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csobject

CSOBJECT.LIB = $(OUT)$(LIB_PREFIX)csobject$(LIB_SUFFIX)
SRC.CSOBJECT = $(wildcard libs/csobject/*.cpp)
OBJ.CSOBJECT = $(addprefix $(OUT),$(notdir $(SRC.CSOBJECT:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csobject csobjectclean

all: $(CSOBJECT.LIB)
csobject: $(OUTDIRS) $(CSOBJECT.LIB)
clean: csobjectclean

$(CSOBJECT.LIB): $(OBJ.CSOBJECT)
	$(DO.LIBRARY)

csobjectclean:
	-$(RM) $(CSGEOM.LIB)

ifdef DO_DEPEND
depend: $(OUTOS)csobject.dep
$(OUTOS)csobject.dep: $(SRC.CSOBJECT)
	$(DO.DEP)
else
-include $(OUTOS)csobject.dep
endif

endif # ifeq ($(MAKESECTION),targets)
