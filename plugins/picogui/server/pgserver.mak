
DESCRIPTION.pgserver = Crystal Space PicoGUI Server

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PLUGINHELP += \
  $(NEWLINE)@echo $"  make pgserver     Make the $(DESCRIPTION.pgserver)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: pgserver pgserverclean

all plugins: pgserver

pgserver:
	$(MAKE_TARGET) MAKE_DLL=yes

pgserverclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/picogui/server

ifeq ($(USE_PLUGINS),yes)
  PGSERVER = $(OUTDLL)/pgserver$(DLL)
  LIB.PGSERVER = $(foreach d,$(DEP.PGSERVER),$($d.LIB))
  LIB.PGSERVER.LOCAL = $(PGSERVER.LFLAGS)
  TO_INSTALL.DYNAMIC_LIBS += $(PGSERVER)
else
  PGSERVER = $(OUT)/$(LIBPREFIX)pgserver$(LIB)
  DEP.EXE += $(PGSERVER)
  SCF.STATIC += pgserver
  TO_INSTALL.STATIC_LIBS += $(PGSERVER)
endif

INC.PGSERVER = $(wildcard plugins/picogui/server/*.h)
SRC.PGSERVER = $(wildcard plugins/picogui/server/*.cpp)
OBJ.PGSERVER = $(addprefix $(OUT)/,$(notdir $(SRC.PGSERVER:.cpp=$O)))
DEP.PGSERVER = CSTOOL CSGEOM CSGFX CSUTIL CSSYS CSUTIL

MSVC.DSP += PGSERVER
DSP.PGSERVER.NAME = pgserver
DSP.PGSERVER.TYPE = plugin
DSP.PGSERVER.LIBS = pgserver

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: pgserver pgserverclean

pgserver: $(OUTDIRS) $(PGSERVER)
$(PGSERVER): $(OBJ.PGSERVER) $(LIB.PGSERVER)
	$(DO.PLUGIN.PREAMBLE)
	$(DO.PLUGIN.CORE) $(LIB.PGSERVER.LOCAL)
	$(DO.PLUGIN.POSTAMBLE)

clean: pgserverclean
pgserverclean:
	$(RM) $(PGSERVER) $(OBJ.PGSERVER)

ifdef DO_DEPEND
dep: $(OUTOS)/pgserver.dep
$(OUTOS)/pgserver.dep: $(SRC.PGSERVER)
	$(DO.DEP)
else
-include $(OUTOS)/PGSERVER.dep
endif

endif # ifeq ($(MAKESECTION),targets)
