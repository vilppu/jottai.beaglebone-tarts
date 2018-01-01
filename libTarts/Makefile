#################################################################################
# Makefile:  TartsLib - Tarts Library for the Beaglebones Black
# Created:   Kelly Lewis, October 2014
#	Copyright (c) 2014 Tartssensors.com
#################################################################################
#This file is distributed in the hope that it will be useful, but WITHOUT    
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      
#  FITNESS FOR A PARTICULAR PURPOSE.  Further inquiries in to licences can be 
#  found at www.tartssensors.com/licenses  
#################################################################################

DYN_VERS_MAJ=1
DYN_VERS_MIN=1
VERSION=$(DYN_VERS_MAJ).$(DYN_VERS_MIN)
DESTDIR=/usr
PREFIX=/local
DEBUG	= -O2
CC	= g++
INCLUDE	= -I. -I/usr/local/include
DEFS	= -D_GNU_SOURCE -DBB_BLACK_ARCH
CFLAGS = $(DEBUG) $(DEFS) -Wformat=2 -Wall -Winline $(INCLUDE) -pipe -fPIC

STATIC=libTarts.a
DYNAMIC=libTarts.so.$(VERSION)

SRC	=	TartsSensors.cpp TartsPlatform.cpp Tarts.cpp
		
OBJ	=	$(SRC:.cpp=.o)

all:		$(DYNAMIC)

$(STATIC):	$(OBJ)
		@echo "[Linking to static library] $(STATIC)"
		@ar rcs $(STATIC) $(OBJ)
		@ranlib $(STATIC)

$(DYNAMIC):	$(OBJ)
		@echo "[Linking (Dynamic)]"
		@$(CC) -shared -o libTarts.so.$(VERSION) -lpthread $(OBJ)

.cpp.o:
		@echo [Compiling] $<
		@$(CC) -c $(CFLAGS) $< -o $@

.PHONY: 	clean
clean:
		@echo "[Cleaned all object and library files]"
		@rm -f $(OBJ) *.a *~ core tags libTarts.*

.PHONEY:	tags
tags:		$(SRC)
		@echo [ctags]
		@ctags $(SRC)

.PHONEY:	install-headers
install-headers:
		@echo "[Installing Tarts Library Headers]"
		@install -m 0755 -d $(DESTDIR)$(PREFIX)/include
		@install -m 0644 Tarts.h $(DESTDIR)$(PREFIX)/include
		@install -m 0644 TartsPlatform.h $(DESTDIR)$(PREFIX)/include
		@install -m 0644 TartsSensors.h $(DESTDIR)$(PREFIX)/include
		@install -m 0644 TartsStrings.h $(DESTDIR)$(PREFIX)/include

.PHONEY:	install
install:	$(DYNAMIC) install-headers
		@echo "[Installing Tarts as Dynamic Library]"
		@install -m 0755 -d	$(DESTDIR)$(PREFIX)/lib
		@install -m 0755 libTarts.so.$(VERSION)	$(DESTDIR)$(PREFIX)/lib/libTarts.so.$(VERSION)
		@ln -sf $(DESTDIR)$(PREFIX)/lib/libTarts.so.$(VERSION)	$(DESTDIR)/lib/libTarts.so
		@ldconfig

PHONEY:		uninstall
uninstall:
		@echo "[Uninstalling Tarts]"
		@rm -f $(DESTDIR)$(PREFIX)/include/Tarts.h
		@rm -f $(DESTDIR)$(PREFIX)/include/TartsPlatform.h
		@rm -f $(DESTDIR)$(PREFIX)/include/TartsSensors.h
		@rm -f $(DESTDIR)$(PREFIX)/include/TartsStrings.h
		@rm -f $(DESTDIR)$(PREFIX)/lib/libTarts.*
		@ldconfig

.PHONEY:	depend
depend:
		makedepend -Y $(SRC)

# DO NOT DELETE
Tarts.o: Tarts.h

