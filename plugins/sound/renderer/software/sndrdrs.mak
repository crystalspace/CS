# This is a subinclude file used to define the rules needed
# to build the software sound renderer

# Driver description
DESCRIPTION.sndrdrs = Crystal Space software sound renderer

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRVHELP += $(NEWLINE)echo $"  make sndrdrs      Make the $(DESCRIPTION.sndrdrs)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: sndrdrs

ifeq ($(USE_DLL),yes)
all drivers snddrivers: sndrdrs
endif

sndrdrs:
	$(MAKE_TARGET) MAKE_DLL=yes

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cssndrdr/software

# The Software Sound renderer
ifeq ($(USE_DLL),yes)
  SNDRDRS=$(OUTDLL)sndrdrs$(DLL)
  DEP.SNDRDRS=$(CSCOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB) $(CSSNDLDR.LIB)
else
  SNDRDRS=$(OUT)$(LIB_PREFIX)sndrdrs$(LIB)
  DEP.EXE+=$(SNDRDRS)
  CFLAGS.STATIC_COM+=$(CFLAGS.D)SCL_SNDRDRS
endif
DESCRIPTION.$(SNDRDRS) = $(DESCRIPTION.sndrdrs)
SRC.SNDRDRS = $(wildcard libs/cssndrdr/software/*.cpp)
OBJ.SNDRDRS = $(addprefix $(OUT),$(notdir $(SRC.SNDRDRS:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: sndrdrs sndrdrsclean sndrdrscleanlib

# Chain rules
snd: sndrdrs
clean: sndrdrsclean
cleanlib: sndrdrscleanlib

sndrdrs: $(OUTDIRS) $(SNDRDRS)

$(SNDRDRS): $(OBJ.SNDRDRS) $(DEP.SNDRDRS)
	$(DO.LIBRARY)

sndrdrsclean:
	$(RM) $(SNDRDRS)

sndrdrscleanlib:
	$(RM) $(OBJ.SNDRDRS) $(SNDRDRS)

ifdef DO_DEPEND
$(OUTOS)sndrdrs.dep: $(SRC.SNDRDRS)
	$(DO.DEP)
endif

-include $(OUTOS)sndrdrs.dep

endif # ifeq ($(MAKESECTION),targets)
