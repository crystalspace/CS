# Library description
DESCRIPTION.csterr = Landscape engine

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += \
  $(NEWLINE)echo $"  make csterr       Make the $(DESCRIPTION.csterr)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csterr

#all libs: csterr
csterr:
	$(MAKE_TARGET)
csterrclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/csterr/math libs/csterr/struct libs/csterr/util

# XXX is temporary until this library builds.
XXXCSTERR.LIB = $(OUT)$(LIB_PREFIX)csterr$(LIB_SUFFIX)
SRC.CSTERR = $(wildcard libs/csterr/*/*.cpp)
OBJ.CSTERR = $(addprefix $(OUT),$(notdir $(SRC.CSTERR:.cpp=$O)))
CFLAGS.CSTERR = -Ilibs/csterr

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csterr csterrclean

#all: $(XXXCSTERR.LIB)
csterr: $(OUTDIRS) $(XXXCSTERR.LIB)
clean: csterrclean

$(OUT)%$O: libs/csterr/math/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.CSTERR)

$(OUT)%$O: libs/csterr/struct/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.CSTERR)

$(OUT)%$O: libs/csterr/util/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.CSTERR)

# @@@ Some versions of GNU make appear to be sensitive to the order in which
# implicit rules are seen.  Without the following rule (which is just a
# reiteration of the original implicit rule in cs.mak), these buggy make
# programs fail to choose the correct rules above.
$(OUT)%$O: %.cpp
	$(DO.COMPILE.CPP)

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
