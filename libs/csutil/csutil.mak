# Library description
DESCRIPTION.csutil = Crystal Space utility library

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make csutil       Make the $(DESCRIPTION.csutil)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csutil

all libs: csutil
csutil:
	$(MAKE_TARGET)
csutilclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csutil

CSUTIL.LIB = $(OUT)$(LIB_PREFIX)csutil$(LIB_SUFFIX)
SRC.CSUTIL = $(wildcard libs/csutil/*.cpp)
OBJ.CSUTIL = $(addprefix $(OUT),$(notdir $(SRC.CSUTIL:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csutil csutilclean

all: $(CSUTIL.LIB)
csutil: $(OUTDIRS) $(CSUTIL.LIB)
clean: csutilclean

$(CSUTIL.LIB): $(OBJ.CSUTIL)
	$(DO.LIBRARY)

csutilclean:
	-$(RM) $(CSUTIL.LIB) $(OBJ.CSUTIL)

ifdef DO_DEPEND
depend: $(OUTOS)csutil.dep
$(OUTOS)csutil.dep: $(SRC.CSUTIL)
	$(DO.DEP)
else
-include $(OUTOS)csutil.dep
endif

endif # ifeq ($(MAKESECTION),targets)
