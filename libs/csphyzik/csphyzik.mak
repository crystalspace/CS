# Library description
DESCRIPTION.csphyzik = Crystal Time Phyziks library

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += \
  $(NEWLINE)echo $"  make csphyzik     Make the $(DESCRIPTION.csphyzik)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csphyzik

all libs: csphyzik
csphyzik:
	$(MAKE_TARGET)
csphyzikclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csphyzik libs/csphyzik/math

CSPHYZIK.LIB = $(OUT)$(LIB_PREFIX)csphyzik$(LIB)
INC.CSPHYZIK = $(wildcard include/csphyzik/*.h)
SRC.CSPHYZIK = $(wildcard libs/csphyzik/*.cpp libs/csphyzik/*/*.cpp)
OBJ.CSPHYZIK = $(addprefix $(OUT),$(notdir $(SRC.CSPHYZIK:.cpp=$O)))

TO_INSTALL.STATIC_LIBS += $(CSPHYZIK.LIB)

MSVC.DSP += CSPHYZIK
DSP.CSPHYZIK.NAME = csphyzik
DSP.CSPHYZIK.TYPE = library

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csphyzik csphyzikclean

all: $(CSPHYZIK.LIB)
csphyzik: $(OUTDIRS) $(CSPHYZIK.LIB)
clean: csphyzikclean

$(CSPHYZIK.LIB): $(OBJ.CSPHYZIK)
	$(DO.STATIC.LIBRARY)

csphyzikclean:
	-$(RM) $(CSPHYZIK.LIB) $(OBJ.CSPHYZIK)

ifdef DO_DEPEND
dep: $(OUTOS)csphyzik.dep
$(OUTOS)csphyzik.dep: $(SRC.CSPHYZIK)
	$(DO.DEP)
else
-include $(OUTOS)csphyzik.dep
endif

endif # ifeq ($(MAKESECTION),targets)
