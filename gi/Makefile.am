PLATFORM_VERSION = 2.0

pkgincludedir = $(includedir)/pygtk-$(PLATFORM_VERSION)
pkgpyexecdir = $(pyexecdir)/gtk-2.0

SUBDIRS = \
	repository \
	overrides
AM_CFLAGS = \
	$(PYTHON_INCLUDES) \
	$(GNOME_CFLAGS) \
	$(PYCAIRO_CFLAGS)

pygidir = $(pkgpyexecdir)/gi
pygi_PYTHON = \
	types.py \
	module.py \
	importer.py \
	__init__.py

_gi_la_LDFLAGS = \
	-module \
	-avoid-version \
	-export-symbols-regex init_gi
_gi_la_LIBADD = \
	$(GNOME_LIBS) \
	$(PYCAIRO_LIBS)
_gi_la_SOURCES = \
	pygi-repository.c \
	pygi-repository.h \
	pygi-info.c \
	pygi-info.h \
	pygi-foreign.c \
	pygi-foreign.h \
	pygi-foreign-cairo.c \
	pygi-foreign-cairo.h \
	pygi-struct.c \
	pygi-struct.h \
	pygi-argument.c \
	pygi-argument.h \
	pygi-type.c \
	pygi-type.h \
	pygi-boxed.c \
	pygi-boxed.h \
	pygi-closure.c \
	pygi-closure.h \
	pygi-callbacks.c \
	pygi-callbacks.h \
	pygi.h \
	pygi-private.h \
	pygobject-external.h \
	gimodule.c

pygi_LTLIBRARIES = _gi.la

# This is to ensure we have a symlink to the .so in the
# build directory, which the Python interpreter can load
# directly without having to know how to parse .la files.
_gi.so: _gi.la
	rm -f $@ && $(LN_S) .libs/$@ $@

all-local: _gi.so
clean-local:
	rm -f _gi.so

