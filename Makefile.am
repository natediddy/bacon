noinst_HEADERS = \
	bacon.h \
	bacon-colors.h \
	bacon-ctype.h \
	bacon-device.h \
	bacon-env.h \
	bacon-gtk.h \
	bacon-hash.h \
	bacon-inter.h \
	bacon-license.h \
	bacon-net.h \
	bacon-out.h \
	bacon-parse.h \
	bacon-pixbufs.h \
	bacon-progress.h \
	bacon-rom.h \
	bacon-search.h \
	bacon-str.h \
	bacon-sys.h \
	bacon-util.h

bin_PROGRAMS = bacon

bacon_SOURCES = \
	bacon.c \
	bacon-ctype.c \
	bacon-device.c \
	bacon-env.c \
	bacon-gtk.c \
	bacon-hash.c \
	bacon-inter.c \
	bacon-net.c \
	bacon-out.c \
	bacon-parse.c \
	bacon-progress.c \
	bacon-rom.c \
	bacon-search.c \
	bacon-str.c \
	bacon-util.c

dist_man_MANS = bacon.1

DESKTOP_FILES = bacon.desktop

Productivitydir = $(datadir)/applications
Productivity_DATA = $(DESKTOP_FILES)

#icon_DATA = bacon.png
#icondir = $(datadir)/pixmaps

EXTRA_DIST = \
	$(DESKTOP_FILES) \
	COPYING \
	README \
	README.md

dist_noinst_SCRIPTS = buildconf 
