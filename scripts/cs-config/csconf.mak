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

all apps: csconfig
csconfig:
	$(MAKE_TARGET)
csconfigclean:
	$(MAKE_CLEAN)

endif

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

CSCONFIG.EXE = cs-config

TO_INSTALL.EXE	+= $(CSCONFIG.EXE)

endif

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: csconfig csconfigclean $(CSCONFIG.EXE)

all: $(CSCONFIG.EXE)
csconfig: $(OUTDIRS) $(CSCONFIG.EXE)
clean: csconfigclean

# create out/csconfig.tmp for the makevars that need to be transferred
$(CSCONFIG.EXE):
	@echo Generating cs-config script...
	@cd out
	@echo $"# Crystal Space config make variables for $(DESCRIPTION.$(TARGET)).$" > csconfig.tmp
	@echo $"# Note: automatically generated. $" >> csconfig.tmp
	@echo $"EXE=$(EXE)$" >> csconfig.tmp
	@echo $"DLL=$(DLL)$" >> csconfig.tmp
	@echo $"LFLAGS.EXE=$(LFLAGS.EXE")$" >> csconfig.tmp
	@echo $"DO.SHARED.PLUGIN.PREAMBLE=$(DO.SHARED.PLUGIN.PREAMBLE)$" >> csconfig.tmp
	@echo $"DO.SHARED.PLUGIN.POSTAMBLE=$(DO.SHARED.PLUGIN.POSTAMBLE)$" >> csconfig.tmp
	@echo $"LIBS.EXE.PLATFORM=$(LIBS.EXE.PLATFORM)$" >> csconfig.tmp
ifneq ($(LINK.PLUGIN),)
	@echo $"LINK.PLUGIN=$(LINK.PLUGIN)$" >> csconfig.tmp
	@echo $"LFLAGS.DLL=$(LFLAGS.DLL) \$$@$" >> csconfig.tmp
else
	@echo $"LINK.PLUGIN=$" >> csconfig.tmp
	@echo $"LFLAGS.DLL=$(LFLAGS.DLL)$" >> csconfig.tmp
endif
	@echo $"PLUGIN.POSTFLAGS=$(PLUGIN.POSTFLAGS)$" >> csconfig.tmp

	@cd ..
	/bin/sh scripts/cs-config/genscript.sh "$(INSTALL_DIR)" "$(CXXFLAGS)" \
	"$(CFLAGS)" "$(LIBS.EXE)" scripts/cs-config
	@cd out
	$(RM) csconfig.tmp
	@cd ..

csconfigclean:
	-$(RM) $(CSCONFIG.EXE)

endif

