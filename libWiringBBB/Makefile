#################################################################################
# Makefile:  WiringBBB Library for the Beaglebones Black GPIO/TIMING/THREAD/UART
#
#            The BBB platform does not have an Arduino-like API that the Tarts
#            library was initially built around.  On the Raspberry Pi, Gordon 
#            Henderson wrote the WiringPI library that performs this function.
#            With inspiration fron Wiring PI and Derek Molloy's videos and blog
#            (https://github.com/derekmolloy/beaglebone), I created WiringBBB.
#
# Created:   Kelly Lewis, October 2014
#	Copyright (c) 2014 Tartssensors.com
#################################################################################
#This file is distributed in the hope that it will be useful, but WITHOUT    
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      
#  FITNESS FOR A PARTICULAR PURPOSE.  Further inquiries in to licences can be 
#  found at www.tartssensors.com/licenses  
#################################################################################

DYN_VERS_MAJ=1
DYN_VERS_MIN=0
VERSION=$(DYN_VERS_MAJ).$(DYN_VERS_MIN)
DESTDIR=/usr
PREFIX=/local

STATIC=libwiringBBB.a
DYNAMIC=libwiringBBB.so.$(VERSION)

CFLAGS = $(DEBUG) $(DEFS) -Wformat=2 -Wall -Winline $(INCLUDE) -pipe -fPIC

SRC	=	wiringBBB.cpp 
		
OBJ	=	$(SRC:.cpp=.o)

all:		$(DYNAMIC)

$(STATIC):	$(OBJ)
		@echo "[Linking to library] $(STATIC)"
		@ar rcs $(STATIC) $(OBJ)
		@ranlib $(STATIC)

$(DYNAMIC):	$(OBJ)
		@echo "[Linking (Dynamic)]"
		@$(CC) -shared -o libwiringBBB.so.$(VERSION) -lpthread $(OBJ)

.cpp.o:
		@echo [Compiling] $<
		@$(CC) -c $(CFLAGS) $< -o $@

.PHONY: 	clean
clean:
		@echo "[Cleaned all object and library files]"
		@rm -f $(OBJ) *.a *~ core tags libwiringBBB.*

.PHONEY:	tags
tags:		$(SRC)
		@echo [ctags]
		@ctags $(SRC)

.PHONEY:	install-headers
install-headers:
		@echo "[Install Library Headers]"
		@install -m 0755 -d $(DESTDIR)$(PREFIX)/include
		@install -m 0644 wiringBBB.h $(DESTDIR)$(PREFIX)/include

.PHONEY:	install
install:	$(DYNAMIC) install-headers
		@echo "[Install Dynamic Library]"
		@install -m 0755 -d	$(DESTDIR)$(PREFIX)/lib
		@install -m 0755 libwiringBBB.so.$(VERSION)	$(DESTDIR)$(PREFIX)/lib/libwiringBBB.so.$(VERSION)
		@ln -sf $(DESTDIR)$(PREFIX)/lib/libwiringBBB.so.$(VERSION)	$(DESTDIR)/lib/libwiringBBB.so
		@ldconfig

PHONEY:		uninstall
uninstall:
		@echo "[Uninstalling wiringBBB]"
		@rm -f $(DESTDIR)$(PREFIX)/include/wiringBBB.h
		@rm -f $(DESTDIR)$(PREFIX)/lib/libwiringBBB.*
		@ldconfig

.PHONEY:	depend
depend:
		makedepend -Y $(SRC)

# DO NOT DELETE
wiringBBB.o: wiringBBB.h
