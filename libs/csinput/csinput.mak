# Library description
DESCRIPTION.csinput = Crystal Space user input library

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make csinput      Make the $(DESCRIPTION.csinput)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csinput

all libs: csinput
csinput:
	$(MAKE_TARGET)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csinput

CSINPUT.LIB = $(OUT)$(LIB_PREFIX)csinput$(LIB_SUFFIX)
SRC.CSINPUT = $(wildcard libs/csinput/*.cpp)
OBJ.CSINPUT = $(addprefix $(OUT),$(notdir $(SRC.CSINPUT:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csinput csinputclean

all: $(CSINPUT.LIB)
csinput: $(OUTDIRS) $(CSINPUT.LIB)
clean: csinputclean

$(CSINPUT.LIB): $(OBJ.CSINPUT)
	$(DO.LIBRARY)

csinputclean:
	-$(RM) $(CSINPUT.LIB)

ifdef DO_DEPEND
depend: $(OUTOS)csinput.dep
$(OUTOS)csinput.dep: $(SRC.CSINPUT)
	$(DO.DEP)
else
-include $(OUTOS)csinput.dep
endif

endif # ifeq ($(MAKESECTION),targets)
