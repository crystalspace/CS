# Library description
DESCRIPTION.csterr = Landscape Engine

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make csterr       Make the $(DESCRIPTION.csterr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csterr

all libs: csterr
csterr:
	$(MAKE_TARGET)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csterr

CSTERR.LIB = $(OUT)$(LIB_PREFIX)csterr$(LIB_SUFFIX)
SRC.CSTERR = $(wildcard libs/csterr/*.cpp)
OBJ.CSTERR = $(addprefix $(OUT),$(notdir $(SRC.CSTERR:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csterr csterrclean

all: $(CSTERR.LIB)
csterr: $(OUTDIRS) $(CSTERR.LIB)
clean: csterrclean

$(CSTERR.LIB): $(OBJ.CSTERR)
	$(DO.LIBRARY)

csterrclean:
	-$(RM) $(CSTERR.LIB)

ifdef DO_DEPEND
depend: $(OUTOS)csterr.dep
$(OUTOS)csterr.dep: $(SRC.CSTERR)
	$(DO.DEP)
else
-include $(OUTOS)csterr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
