#
# This submakefile requires the following variables to be defined by
# each platform-specific makefile:
#
# SRC.SYS_CSUTIL
#   - All system-dependent source files that should be included in csutil
#     library

# Library description
DESCRIPTION.csutil = Crystal Space utility library

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Library-specific help commands
LIBHELP += \
  $(NEWLINE)echo $"  make csutil       Make the $(DESCRIPTION.csutil)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csutil

all libs: csutil
csutil:
	$(MAKE_TARGET)
csutilclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp $(SRCDIR)/libs/csutil $(filter-out $(SRCDIR)/libs/csutil/generic/, $(sort $(dir $(SRC.SYS_CSUTIL)))) $(SRCDIR)/libs/csutil/generic
vpath %.c   $(SRCDIR)/libs/csutil $(filter-out $(SRCDIR)/libs/csutil/generic/, $(sort $(dir $(SRC.SYS_CSUTIL)))) $(SRCDIR)/libs/csutil/generic

CSUTIL.LIB = $(OUT)/$(LIB_PREFIX)csutil$(LIB_SUFFIX)

INC.CSUTIL = $(INC.SYS_CSUTIL) \
  $(wildcard $(addprefix $(SRCDIR)/,include/csutil/*.h))

SRC.CSUTIL.LOCAL = $(wildcard $(addprefix $(SRCDIR)/libs/csutil/,*.cpp *.c))
ifneq ($(REGEX.AVAILABLE),yes)
  SRC.CSUTIL.LOCAL += $(SRCDIR)/libs/csutil/generic/regex.c
endif
SRC.CSUTIL = $(SRC.SYS_CSUTIL) $(SRC.CSUTIL.LOCAL)

# Platform-specific makefiles may want to provide their own value for
# OBJ.SYS_CSUTIL (for instance, they might recognize other file types in
# addition to .s, .c, and .cpp), so we set OBJ.SYS_CSUTIL only if not already
# set by the platform-specific makefile.
ifeq (,$(strip $(OBJ.SYS_CSUTIL)))
OBJ.SYS_CSUTIL = $(addprefix $(OUT)/,$(notdir \
  $(subst .s,$O,$(subst .c,$O,$(SRC.SYS_CSUTIL:.cpp=$O)))))
endif
OBJ.CSUTIL = $(OBJ.SYS_CSUTIL) $(addprefix $(OUT)/, \
  $(notdir $(patsubst %.c,%$O,$(SRC.CSUTIL.LOCAL:.cpp=$O))))

CFG.CSUTIL = $(SRCDIR)/data/config/mouse.cfg

TO_INSTALL.CONFIG += $(CFG.CSUTIL)
TO_INSTALL.STATIC_LIBS += $(CSUTIL.LIB)

MSVC.DSP += CSUTIL
DSP.CSUTIL.NAME = csutil
DSP.CSUTIL.TYPE = library

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csutil csutilclean

all: $(CSUTIL.LIB)
csutil: $(OUTDIRS) $(CSUTIL.LIB)
clean: csutilclean

$(OUT)/archive$O: $(SRCDIR)/libs/csutil/archive.cpp
	$(DO.COMPILE.CPP) $(ZLIB.CFLAGS)

$(CSUTIL.LIB): $(OBJ.CSUTIL)
	$(DO.LIBRARY)

csutilclean:
	-$(RM) $(CSUTIL.LIB) $(OBJ.CSUTIL)

ifdef DO_DEPEND
dep: $(OUTOS)/csutil.dep
$(OUTOS)/csutil.dep: $(SRC.CSUTIL)
	$(DO.DEP1) \
	$(ZLIB.CFLAGS) \
	$(CFLAGS.D)CS_CONFIGDIR='"$(CS_CONFIGDIR)"' \
	$(CFLAGS.D)CS_PLUGINDIR'"=$(CS_PLUGINDIR)"' \
	$(DO.DEP2)
else
-include $(OUTOS)/csutil.dep
endif

endif # ifeq ($(MAKESECTION),targets)
