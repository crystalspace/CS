# Target description
DESCRIPTION.csconfig = cs-config compile help script

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

PSEUDOHELP += \
  $(NEWLINE)echo $"  make csconfig     Build the $(DESCRIPTION.csconfig)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: csconfig csconfigclean

all: csconfig
csconfig:
	$(MAKE_TARGET)
csconfigclean:
	$(MAKE_CLEAN)

endif

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSCONFIG.EXE = cs-config
CSCONFIG.TMP = $(OUT)csconfig.tmp

TO_INSTALL.EXE	+= $(CSCONFIG.EXE)

endif

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csconfig csconfigclean $(CSCONFIG.EXE)

all: $(CSCONFIG.EXE)
csconfig: $(OUTDIRS) $(CSCONFIG.EXE)
clean: csconfigclean

# Create csconfig.tmp for the make variables that need to be transferred.
$(CSCONFIG.EXE):
	@echo Generating cs-config script...
	@echo $"# Crystal Space config make variables for $(DESCRIPTION.$(TARGET)).$" > $(CSCONFIG.TMP)
	@echo $"# Note: automatically generated. $" >> $(CSCONFIG.TMP)
	@echo $"EXE=$(EXE)$" >> $(CSCONFIG.TMP)
	@echo $"DLL=$(DLL)$" >> $(CSCONFIG.TMP)
	@echo $"LFLAGS.EXE=$(LFLAGS.EXE)$" >> $(CSCONFIG.TMP)
	@echo $"DO.SHARED.PLUGIN.PREAMBLE=$(DO.SHARED.PLUGIN.PREAMBLE)$" >> $(CSCONFIG.TMP)
	@echo $"DO.SHARED.PLUGIN.POSTAMBLE=$(DO.SHARED.PLUGIN.POSTAMBLE)$" >> $(CSCONFIG.TMP)
	@echo $"LIBS.EXE.PLATFORM=$(LIBS.EXE.PLATFORM)$" >> $(CSCONFIG.TMP)
ifneq ($(LINK.PLUGIN),)
	@echo $"LINK.PLUGIN=$(LINK.PLUGIN)$" >> $(CSCONFIG.TMP)
	@echo $"LFLAGS.DLL=$(LFLAGS.DLL) \$$@$" >> $(CSCONFIG.TMP)
else
	@echo $"LINK.PLUGIN=$" >> $(CSCONFIG.TMP)
	@echo $"LFLAGS.DLL=$(LFLAGS.DLL)$" >> $(CSCONFIG.TMP)
endif
	@echo $"PLUGIN.POSTFLAGS=$(PLUGIN.POSTFLAGS)$" >> $(CSCONFIG.TMP)

	$(RUN_SCRIPT) scripts/cs-config/genscript.sh $"$(INSTALL_DIR)$" \
	$"$(CXXFLAGS)$" $"$(CFLAGS)$" $"$(LIBS.EXE)$" scripts/cs-config \
	$"$(CSCONFIG.TMP)$"
	$(RM) $(CSCONFIG.TMP)

csconfigclean:
	-$(RM) $(CSCONFIG.EXE) $(CSCONFIG.TMP)

endif
