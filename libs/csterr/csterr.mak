# Library description
DESCRIPTION.csterr = Landscape engine

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += $(NEWLINE)echo $"  make csterr       Make the $(DESCRIPTION.csterr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csterr

#all libs: csterr
csterr:
	$(MAKE_TARGET)
csterrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csterr/math libs/csterr/struct libs/csterr/util

# XXX is temporary until this library builds.
XXXCSTERR.LIB = $(OUT)$(LIB_PREFIX)csterr$(LIB_SUFFIX)
SRC.CSTERR.MATH   = $(wildcard libs/csterr/math/*.cpp)
SRC.CSTERR.STRUCT = $(wildcard libs/csterr/struct/*.cpp)
SRC.CSTERR.UTIL   = $(wildcard libs/csterr/util/*.cpp)
SRC.CSTERR = $(SRC.CSTERR.MATH) $(SRC.CSTERR.STRUCT) $(SRC.CSTERR.UTIL)
OBJ.CSTERR.MATH   = $(addprefix $(OUT),$(notdir $(SRC.CSTERR.MATH:.cpp=$O)))
OBJ.CSTERR.STRUCT = $(addprefix $(OUT),$(notdir $(SRC.CSTERR.STRUCT:.cpp=$O)))
OBJ.CSTERR.UTIL   = $(addprefix $(OUT),$(notdir $(SRC.CSTERR.UTIL:.cpp=$O)))
OBJ.CSTERR = $(OBJ.CSTERR.MATH) $(OBJ.CSTERR.STRUCT) $(OBJ.CSTERR.UTIL)
CFLAGS.CSTERR = -Ilibs/csterr

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csterr csterrclean

#all: $(XXXCSTERR.LIB)
csterr: $(OUTDIRS) $(XXXCSTERR.LIB)
clean: csterrclean

$(OBJ.CSTERR.MATH): $(SRC.CSTERR.MATH)
	$(DO.COMPILE.CPP) $(CFLAGS.CSTERR)

$(OBJ.CSTERR.STRUCT): $(SRC.CSTERR.STRUCT)
	$(DO.COMPILE.CPP) $(CFLAGS.CSTERR)

$(OBJ.CSTERR.UTIL): $(SRC.CSTERR.UTIL)
	$(DO.COMPILE.CPP) $(CFLAGS.CSTERR)

$(XXXCSTERR.LIB): $(OBJ.CSTERR)
	$(DO.LIBRARY)

csterrclean:
	-$(RM) $(XXXCSTERR.LIB) $(OBJ.CSTERR)

ifdef DO_DEPEND
dep: $(OUTOS)csterr.dep
$(OUTOS)csterr.dep: $(SRC.CSTERR)
	$(DO.DEP)
else
-include $(OUTOS)csterr.dep
endif

endif # ifeq ($(MAKESECTION),targets)
