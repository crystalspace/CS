WIX_FLAGS = -v -nologo #-pedantic
CANDLEFLAGS = 
LIGHTFLAGS = -ext c:\bin\WixUIExtension.dll -cultures:en-us -ss -sval
CELBASE = cel
CSBASE = CS
B2CSBASE = b2cs
CS_MSMS =  CS.include.msm \
	CS.data.msm \
	CS.vfs.msm \
	CS.register.msm \
	CS.doc.msm \
	CS.arch.GCC.executable.msm \
	CS.arch.GCC.support.msm \
	CS.arch.GCC.include.msm \
	CS.arch.GCC.lib.msm

CEL_MSMS = cel.include.msm \
	cel.data.msm \
	cel.vfs.msm \
	cel.register.msm \
	cel.doc.msm \
	cel.arch.GCC.executable.msm \
	cel.arch.GCC.include.msm \
	cel.arch.GCC.lib.msm

B2CS_MSMS = b2cs.blender.plugin.msm

PYTHON_MSMS = python_new.msm

all: CS.msi cel.msi CS_cel.msi CS_cel_b2cs.msi b2cs.msi CS_cel_b2cs_python.msi

CS.msi: $(CS_MSMS) \
	CS.wixobj
	light $(WIX_FLAGS) $(LIGHTFLAGS) CS.wixobj


cel.msi: $(CEL_MSMS) \
	cel.wixobj
	light $(WIX_FLAGS) $(LIGHTFLAGS) cel.wixobj

CS_cel.msi: $(CS_MSMS) \
	$(CEL_MSMS) \
	CS_cel.wixobj
	light $(WIX_FLAGS) $(LIGHTFLAGS) CS_cel.wixobj

CS_cel_b2cs.msi: $(CS_MSMS) \
	$(CEL_MSMS) \
	$(B2CS_MSMS) \
	CS_cel_b2cs.wixobj
	light $(WIX_FLAGS) $(LIGHTFLAGS) CS_cel_b2cs.wixobj


CS_cel_b2cs_python.msi: $(CS_MSMS) \
	$(CEL_MSMS) \
	$(B2CS_MSMS) \
	$(PYTHON_MSMS) \
	CS_cel_b2cs_python.wixobj
	light $(WIX_FLAGS) $(LIGHTFLAGS) CS_cel_b2cs_python.wixobj

b2cs.msi: $(B2CS_MSMS) \
	b2cs.wixobj
	light $(WIX_FLAGS) $(LIGHTFLAGS) b2cs.wixobj

cel.include.msm: cel.include.wixobj
	light -b $(CELBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
cel.vfs.msm: cel.vfs.wixobj
	light -b $(CELBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
cel.register.msm: cel.register.wixobj
	light -b $(CELBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
cel.doc.msm: cel.doc.wixobj
	light -b $(CELBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
cel.data.msm: cel.data.wixobj
	light -b $(CELBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
cel.arch.GCC.include.msm: cel.arch.GCC.include.wixobj
	light -b $(CELBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
cel.arch.GCC.lib.msm: cel.arch.GCC.lib.wixobj
	light -b $(CELBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
cel.arch.GCC.executable.msm: cel.arch.GCC.executable.wixobj
	light -b $(CELBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^

CS.include.msm: CS.include.wixobj
	light -b $(CSBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
CS.vfs.msm: CS.vfs.wixobj
	light -b $(CSBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
CS.register.msm: CS.register.wixobj
	light -b $(CSBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
CS.doc.msm: CS.doc.wixobj
	light -b $(CSBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
CS.data.msm: CS.data.wixobj
	light -b $(CSBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
CS.arch.GCC.include.msm: CS.arch.GCC.include.wixobj
	light -b $(CSBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
CS.arch.GCC.lib.msm: CS.arch.GCC.lib.wixobj
	light -b $(CSBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
CS.arch.GCC.executable.msm: CS.arch.GCC.executable.wixobj
	light -b $(CSBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^
CS.arch.GCC.support.msm: CS.arch.GCC.support.wixobj
	light -b $(CSBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^

b2cs.blender.plugin.msm: b2cs.blender.plugin.wixobj
	light -b $(B2CSBASE) $(WIX_FLAGS) $(LIGHTFLAGS) $^

.SECONDARY: %msm %wixobj

%.msi: %.wixobj
	light $(WIX_FLAGS) $(LIGHTFLAGS) $^

%.msm: %.wixobj
	light $(BASE) ${^/.*/} $(WIX_FLAGS) $(LIGHTFLAGS) $^

%.wixobj: %.wxs
	candle $(WIX_FLAGS) $(CANDLEFLAGS) $^



install: CS.msi
	msiexec /i CS.msi

validate: CS.msi
	smoke $^

uninstall: CS.msi
	msiexec /X
clean:
	find . -maxdepth 1 -type f '(' -name '*msm' -or -name '*msi' -or -name '*wixpdb' -or -name '*wixobj' ')' -exec rm '{}' \;

