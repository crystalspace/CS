# Library description
DESCRIPTION.csscript = Crystal Space scripting library

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make csscript     Make the $(DESCRIPTION.csscript)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csscript

all libs: csscript
csscript:
	$(MAKE_TARGET)
csscriptclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csscript 

CSSCRIPT.LIB = $(OUT)$(LIB_PREFIX)csscript$(LIB_SUFFIX)
SRC.CSSCRIPT = $(wildcard libs/csscript/*.cpp libs/csscript/*/*.cpp)
OBJ.CSSCRIPT = $(addprefix $(OUT),$(notdir $(SRC.CSSCRIPT:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csscript csscriptclean

all: $(CSENGINE.LIB)
csscript: $(OUTDIRS) $(CSSCRIPT.LIB)
clean: csscriptclean

$(CSSCRIPT.LIB): $(OBJ.CSSCRIPT)
	$(DO.LIBRARY)

csscriptclean:
	-$(RM) $(CSSCRIPT.LIB)

ifdef DO_DEPEND
depend: $(OUTOS)csscript.dep
$(OUTOS)csscript.dep: $(SRC.CSSCRIPT)
	$(DO.DEP)
else
-include $(OUTOS)csscript.dep
endif

endif # ifeq ($(MAKESECTION),targets)
