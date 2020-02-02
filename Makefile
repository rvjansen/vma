#
# VMA is developed under Cygwin/Mingw32, so this makefile may have
# related dependencies.
#

#
# Get current version
#
VERSION     = $(shell bash ./version.sh src)

#
# wxWidgets settings (preference is static)
#
WXREL       = $(shell wx-config --unicode=yes --static --release)
WXCXXFLAGS  = $(shell wx-config --unicode=yes --static --cxxflags)
WXCFLAGS    = $(shell wx-config --unicode=yes --static --cflags)
WXLDFLAGS   = $(shell wx-config --unicode=yes --static --libs)

#
# If no static libraries exist, then check for dynamic ones
#
ifeq ($(WXLDFLAGS),)
WXREL       = $(shell wx-config --unicode=yes --release)
WXCXXFLAGS  = $(shell wx-config --unicode=yes --cxxflags)
WXCFLAGS    = $(shell wx-config --unicode=yes --cflags)
WXLDFLAGS   = $(shell wx-config --unicode=yes --libs)
endif

#
# If no static libraries exist, then check for dynamic ones
#
ifeq ($(WXLDFLAGS),)
WXREL       = $(shell wx-config --unicode=yes --static --release)
WXCXXFLAGS  = $(shell wx-config --unicode=yes --static --cxxflags)
WXCFLAGS    = $(shell wx-config --unicode=yes --static --cflags)
WXLDFLAGS   = $(shell wx-config --unicode=no --static --libs)
endif

#
# Couldn't find the libraries we need
#
ifeq ($(WXLDFLAGS),)
MISSINGLIBS = yes
endif

#
# Debugging
#
DEBUG       = -g
#DEBUG       = -O9

#
# Generic compiler settings
#
CC          = gcc
CFLAGS      = -I. -Isrc -pedantic -Wall -Wno-long-long $(WXCXXFLAGS) $(DEBUG)
CXXFLAGS    = $(CFLAGS)

#
# Generic link settings
#
LDFLAGS     = $(DEBUG)
LIBS        =

#
# Command line objects
#
CLIOBJS     = src/vma.o                             \
              src/vmalib.o

#
# GUI objects
#
GUIOBJS     = src/vmagui.o                          \
              src/properties.o                      \
              src/settings.o                        \
              src/vmalib.o

#
# XPM dependencies
#
XPMS        = res/add16.xpm                         \
              res/binary16.xpm                      \
              res/downarrow16.xpm                   \
              res/extall16.xpm                      \
              res/extract16.xpm                     \
              res/find16.xpm                        \
              res/new16.xpm                         \
              res/open16.xpm                        \
              res/props16.xpm                       \
              res/remove16.xpm                      \
              res/save16.xpm                        \
              res/settings16.xpm                    \
              res/text16.xpm                        \
              res/unknown16.xpm                     \
              res/uparrow16.xpm                     \
              res/view16.xpm                        \
              res/vmagui.xpm

#
# Windows specific settings
#
ifneq ($(findstring Windows,$(OS)),)
WXLDFLAGS   += -mwindows
LIBS        += -lshlwapi
GUIOBJS     += src/vmaguirc.o
endif

#
# Mac specific settings
#
ifneq ($(findstring WXMAC,$(CXXFLAGS)),)
CFLAGS      += 
CXXFLAGS    += 
LDFLAGS     += 
endif

#
# Desired targets
#
TARGETS     = src/vma
ifneq ($(findstring WXMAC,$(CXXFLAGS)),)
TARGETS     += VMAgui.app
else
ifneq ($(WXREL),)
TARGETS     += src/vmagui
endif
endif

#
# If no static libraries exist, then check for dynamic ones
#
ifeq ($(MISSINGLIBS),yes)
TARGETS     = missing_wx_libraries
endif

ifeq ($(shell which wx-config),)
TARGETS     = missing_wx_config
endif

#
# Distribution files
#
MISCFILES   = Changes.txt                           \
              Readme.txt                            \
              *.ucm

#
# Distribution files for z/OS
#
ZOSFILES    = Run.jcl                               \
              $(MISCFILES)

#
# Distribution files
#
DISTFILES   = $(XPMS)                               \
              res/Info.plist                        \
              res/PkgInfo                           \
              res/VMAgui.icns                       \
              res/vmagui.ico                        \
              res/vmagui.rc                         \
              src/properties.cpp                    \
              src/properties.h                      \
              src/settings.cpp                      \
              src/settings.h                        \
              src/version.h                         \
              src/vma.c                             \
              src/vmagui.cpp                        \
              src/vmagui.h                          \
              src/vmalib.c                          \
              src/vmalib.h                          \
              src/vmapriv.h                         \
              Changes.txt                           \
              Readme.txt                            \
              *.ucm                                 \
              Makefile                              \
              Makefile.jcl                          \
              Run.jcl                               \
              version.sh                            \
              VMA.sln                               \
              VMA.vcproj                            \
              VMAgui.vcproj                         \
              .version                              \
              .revision

#
# Default target is just the CLI version
#
all: $(TARGETS)

#
# wx-config couldn't be found
#
missing_wx_config:
	@echo "==================================================="
	@echo "You must install the wxWidgets development package."
	@echo "==================================================="

#
# Required wxWidgets configuration not found
#
missing_wx_libraries:
	@echo "=================================================="
	@echo "You must install the unicode version of wxWidgets."
	@echo "=================================================="

#
# CLI target
#
cli: src/vma

#
# GUI target
#
gui: src/vmagui

#
# Command line utility dependencies
#
src/vma:        $(CLIOBJS)
#
src/vma.o:      src/vma.c src/vmalib.h

#
# GUI utility dependencies
#
src/vmagui:     $(GUIOBJS)
	$(LINK.cc) ${LDFLAGS} -o $@ $^ $(WXLDFLAGS) $(LIBS)

#
VMAgui.app: src/vmagui
	rm -rf VMAgui.app
	mkdir -p VMAgui.app/Contents/MacOS
	mkdir -p VMAgui.app/Contents/Resources
	sed -e "s/__VERSION__/$(VERSION)/" res/Info.plist >VMAgui.app/Contents/Info.plist
	cp res/PkgInfo VMAgui.app/Contents
	cp res/VMAgui.icns VMAgui.app/Contents/Resources
	strip -o VMAgui.app/Contents/MacOS/VMAgui src/vmagui
	cp src/vmagui VMAgui.app/Contents/MacOS/VMAgui
	

#
src/vmagui.o:   src/vmagui.cpp src/vmagui.h src/settings.h src/properties.h src/vmalib.h $(XPMS)
#
src/settings.o: src/settings.cpp src/settings.h res/find16.xpm
#
src/properties.o: src/properties.cpp src/properties.h src/vmalib.h 
#
src/vmaguirc.o: res/vmagui.rc res/vmagui.ico
	windres --use-temp-file $(WXCXXFLAGS) $< $@

#
# Common dependencies
#
src/vmalib.o:    src/vmalib.c src/vmalib.h src/vmapriv.h

#
# Pick up after ourselves
#
clean:
	-rm -rf src/*.o src/*.exe src/vma src/vmagui VMAgui.app

#
# Build the source dist
#
dist: clean
	-rm -rf vma-$(VERSION).tar.gz vma-$(VERSION)
	mkdir vma-$(VERSION) vma-$(VERSION)/res vma-$(VERSION)/src
	for f in $(DISTFILES) ; do cp -pR $$f vma-$(VERSION)/$$f ; done
#
# add me later
#
#	sed -e 's/\r//g;s/\t/    /g;s/ *$//g' ...
#	dos2unix vma-$(VERSION)/* vma-$(VERSION)/src/*
	find vma-$(VERSION) -name .DS_Store -exec rm \{\} \;
	tar zcf ../vma-$(VERSION).tar.gz vma-$(VERSION)
	rm -rf vma-$(VERSION)

#
# Build the Windows dist
#
win:
	-rm -rf vmawin-$(VERSION).zip vmawin-$(VERSION)
	mkdir vmawin-$(VERSION)
	cp Release/*.exe vmawin-$(VERSION)
	cp $(MISCFILES) vmawin-$(VERSION)
	zip -9 -r ../vmawin-$(VERSION).zip vmawin-$(VERSION)
	rm -rf vmawin-$(VERSION)

#
# Build the Linux dist
#
lin: clean all
	-rm -rf vmalin-$(VERSION).$(shell uname -m).tar.gz vmalin-$(VERSION).$(shell uname -m)
	mkdir vmalin-$(VERSION).$(shell uname -m)
	cp $(TARGETS) vmalin-$(VERSION).$(shell uname -m)
	strip vmalin-$(VERSION).$(shell uname -m)/*
	cp $(MISCFILES) vmalin-$(VERSION).$(shell uname -m)
	tar jcf ../vmalin-$(VERSION).$(shell uname -m).tar.bz2 vmalin-$(VERSION).$(shell uname -m)

#
# Build the Mac dist
#
mac: clean all
	-rm -rf ../vmamac-$(VERSION).dmg vmamac-$(VERSION)
	mkdir vmamac-$(VERSION)
	cp -pR $(MISCFILES) $(TARGETS) vmamac-$(VERSION)
	hdiutil create -ov -srcdir "vmamac-$(VERSION)" -fs HFS+ -volname "VMAgui $(VERSION)" TMP.dmg
	hdiutil convert TMP.dmg -format UDZO -imagekey zlib-level=9 -o "../vmamac-$(VERSION).dmg"
	rm TMP.dmg
	rm -rf vmamac-$(VERSION)

#
# Build the z/OS dist
#
zos:
	-rm -rf vmazos-$(VERSION).zip vmazos-$(VERSION)
	mkdir vmazos-$(VERSION)
	cp -pR $(ZOSFILES) vma.xmit vmazos-$(VERSION)
	zip -9 -r ../vmazos-$(VERSION).zip vmazos-$(VERSION)
	rm -rf vmazos-$(VERSION)

