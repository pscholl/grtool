CPPFLAGS=`pkg-config --cflags grt` -O0 -g -std=c++11
LDFLAGS=`pkg-config --libs grt`

all: grtool

PREFIX=/usr
bindir=$(PREFIX)/bin
DESTDIR=
INSTALL_PROGRAM=install

install: grtool
	$(INSTALL_PROGRAM) -D grtool "$(DESTDIR)$(bindir)/grtool"
