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

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csutil

CSUTIL.LIB = $(OUT)$(LIB_PREFIX)csutil$(LIB_SUFFIX)
SRC.CSUTIL = $(wildcard libs/csutil/*.cpp libs/csutil/*/*.cpp libs/csutil/*/*/*.cpp)
OBJ.CSUTIL = $(addprefix $(OUT),$(notdir $(SRC.CSUTIL:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csutil csutilclean

all: $(CSUTIL.LIB)
csutil: $(OUTDIRS) $(CSUTIL.LIB)
clean: csutilclean

$(OUT)%$O: libs/csutil/impexp/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SOFT3D)

$(OUT)%$O: libs/csutil/impexp/3DS/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SOFT3D)

$(OUT)%$O: libs/csutil/impexp/ASE/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SOFT3D)

$(OUT)%$O: libs/csutil/impexp/DXF/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SOFT3D)

$(OUT)%$O: libs/csutil/impexp/HRC/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SOFT3D)

$(OUT)%$O: libs/csutil/impexp/iv/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SOFT3D)

$(OUT)%$O: libs/csutil/impexp/obj/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SOFT3D)

$(OUT)%$O: libs/csutil/impexp/md2/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SOFT3D)

$(OUT)%$O: libs/csutil/impexp/pov/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SOFT3D)

$(OUT)%$O: libs/csutil/impexp/smf/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SOFT3D)

$(OUT)%$O: libs/csutil/impexp/stla/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SOFT3D)

$(OUT)%$O: libs/csutil/impexp/txt/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SOFT3D)

$(OUT)%$O: libs/csutil/impexp/vla/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.SOFT3D)

$(CSUTIL.LIB): $(OBJ.CSUTIL)
	$(DO.LIBRARY)

csutilclean:
	-$(RM) $(CSUTIL.LIB)

ifdef DO_DEPEND
depend: $(OUTOS)csutil.dep
$(OUTOS)csutil.dep: $(SRC.CSUTIL)
	$(DO.DEP)
else
-include $(OUTOS)csutil.dep
endif

endif # ifeq ($(MAKESECTION),targets)
