# Library description
DESCRIPTION.csapputil = Crystal Space application utility library

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make csapputil    Make the $(DESCRIPTION.csapputil)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csapputil

all libs: csapputil
csapputil:
	$(MAKE_TARGET)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csapputil

CSAPPUTIL.LIB = $(OUT)$(LIB_PREFIX)csapputil$(LIB)
SRC.CSAPPUTIL = $(wildcard libs/csapputil/*.cpp)
OBJ.CSAPPUTIL = $(addprefix $(OUT),$(notdir $(SRC.CSAPPUTIL:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csapputil csapputilclean

all: $(CSAPPUTIL.LIB)
csapputil: $(OUTDIRS) $(CSAPPUTIL.LIB)
clean: csapputilclean

$(CSAPPUTIL.LIB): $(OBJ.CSAPPUTIL)
	$(DO.STATIC.LIBRARY)

csapputilclean:
	-$(RM) $(CSAPPUTIL.LIB)

ifdef DO_DEPEND
$(OUTOS)csapputil.dep: $(SRC.CSAPPUTIL)
	$(DO.DEP)
endif

-include $(OUTOS)csapputil.dep

endif # ifeq ($(MAKESECTION),targets)
