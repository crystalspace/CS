# Library description
DESCRIPTION.csappframe = Crystal Space application framework library

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += \
  $(NEWLINE)echo $"  make csappframe   Make the $(DESCRIPTION.csappframe)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csappframe

all libs: csappframe
csappframe:
	$(MAKE_TARGET)
csappframeclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/libs/csappframe libs/csappframe/basic \
  libs/csappframe/light libs/csappframe/objects

CSAPPFRAME.LIB = $(OUT)/$(LIB_PREFIX)csappframe$(LIB_SUFFIX)
INC.CSAPPFRAME = $(wildcard $(addprefix $(SRCDIR)/,libs/csappframe/*.h))
SRC.CSAPPFRAME = $(wildcard $(addprefix $(SRCDIR)/, \
  libs/csappframe/*.cpp libs/csappframe/*/*.cpp))
OBJ.CSAPPFRAME = $(addprefix $(OUT)/,$(notdir $(SRC.CSAPPFRAME:.cpp=$O)))

TO_INSTALL.STATIC_LIBS += $(CSAPPFRAME.LIB)

MSVC.DSP += CSAPPFRAME
DSP.CSAPPFRAME.NAME = csappframe
DSP.CSAPPFRAME.TYPE = library

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csappframe csappframeclean

all: $(CSAPPFRAME.LIB)
csappframe: $(OUTDIRS) $(CSAPPFRAME.LIB)
clean: csappframeclean

$(CSAPPFRAME.LIB): $(OBJ.CSAPPFRAME)
	$(DO.LIBRARY)

csappframeclean:
	-$(RM) $(CSAPPFRAME.LIB) $(OBJ.CSAPPFRAME)

ifdef DO_DEPEND
dep: $(OUTOS)/csappframe.dep
$(OUTOS)/csappframe.dep: $(SRC.CSAPPFRAME)
	$(DO.DEP)
else
-include $(OUTOS)/csappframe.dep
endif

endif # ifeq ($(MAKESECTION),targets)
