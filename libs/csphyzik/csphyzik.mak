# Library description
DESCRIPTION.csphyzik = Crystal Time physics library

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make csphyzik     Make the $(DESCRIPTION.csphyzik)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csphyzik

all libs: csphyzik
csphyzik:
	$(MAKE_TARGET)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csphyzik libs/csphyzik/math

CSPHYZIK.LIB = $(OUT)$(LIB_PREFIX)csphyzik$(LIB)
SRC.CSPHYZIK = $(wildcard libs/csphyzik/*.cpp libs/csphyzik/*/*.cpp)
OBJ.CSPHYZIK = $(addprefix $(OUT),$(notdir $(SRC.CSPHYZIK:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csphyzik csphyzikclean

all: $(CSPHYZIK.LIB)
csphyzik: $(OUTDIRS) $(CSPHYZIK.LIB)
clean: csphyzikclean

$(CSPHYZIK.LIB): $(OBJ.CSPHYZIK)
	$(DO.STATIC.LIBRARY)

csphyzikclean:
	-$(RM) $(CSPHYZIK.LIB)

ifdef DO_DEPEND
depend: $(OUTOS)csphyzik.dep
$(OUTOS)csphyzik.dep: $(SRC.CSPHYZIK)
	$(DO.DEP)
else
-include $(OUTOS)csphyzik.dep
endif

endif # ifeq ($(MAKESECTION),targets)
