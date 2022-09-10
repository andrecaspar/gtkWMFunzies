# dwm version
VERSION = 6.3

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

# Xinerama, comment if you don't want it
XINERAMALIBS  = -lXinerama
XINERAMAFLAGS = -DXINERAMA

# freetype
FREETYPELIBS = -lfontconfig -lXft
FREETYPEINC = /usr/include/freetype2
# OpenBSD (uncomment)
#FREETYPEINC = ${X11INC}/freetype2
#MANPREFIX = ${PREFIX}/man

# includes and libs
INCS = -I${X11INC} -I${FREETYPEINC} -I/usr/include/gtkmm-4.0 -I/usr/lib/gtkmm-4.0/include -I/usr/include/giomm-2.68 -I/usr/lib/giomm-2.68/include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/sysprof-4 -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/glibmm-2.68 -I/usr/lib/glibmm-2.68/include -I/usr/include/sigc++-3.0 -I/usr/lib/sigc++-3.0/include -I/usr/include/gtk-4.0 -I/usr/include/pango-1.0 -I/usr/include/harfbuzz -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/fribidi -I/usr/include/cairo -I/usr/include/lzo -I/usr/include/pixman-1 -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/graphene-1.0 -I/usr/lib/graphene-1.0/include -I/usr/include/gio-unix-2.0 -I/usr/include/cairomm-1.16 -I/usr/lib/cairomm-1.16/include -I/usr/include/pangomm-2.48 -I/usr/lib/pangomm-2.48/include -I/usr/include/gtk-4.0/unix-print
LIBS = -L${X11LIB} -lX11 ${XINERAMALIBS} ${FREETYPELIBS} -mfpmath=sse -msse -msse2 -pthread -lgtkmm-4.0 -lpangomm-2.48 -lgiomm-2.68 -lglibmm-2.68 -lcairomm-1.16 -lsigc-3.0 -lgtk-4 -lpangocairo-1.0 -lpango-1.0 -lharfbuzz -lgdk_pixbuf-2.0 -lcairo-gobject -lcairo -lgraphene-1.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0

# flags
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L -DVERSION=\"${VERSION}\" ${XINERAMAFLAGS}
#CFLAGS   = -g -std=c99 -pedantic -Wall -O0 ${INCS} ${CPPFLAGS}
CFLAGS   = -march=native -mtune=native -std=c99 -pedantic -Wall -Wno-unused-function -Wno-deprecated-declarations -flto -O2 ${INCS} ${CPPFLAGS}
LDFLAGS  = ${LIBS}

# Solaris
#CFLAGS = -fast ${INCS} -DVERSION=\"${VERSION}\"
#LDFLAGS = ${LIBS}

# compiler and linker
CC = cc
