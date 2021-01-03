#################################################################################
# Makefile:  Tarts Example Applications
# Created:   Kelly Lewis, October 2014
#	Copyright (c) 2014 Tartssensors.com
#################################################################################
#This file is distributed in the hope that it will be useful, but WITHOUT    
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      
#  FITNESS FOR A PARTICULAR PURPOSE.  Further inquiries in to licences can be 
#  found at www.tartssensors.com/licenses  
#################################################################################
# Modified:   Timo Vilppu, November 2015
#################################################################################


#DEBUG	= 	-g -O0
DEBUG	= 	-O3
CC	= 	g++
INCLUDE	= 	-I. -I/usr/local/include -I./libTarts -I./libWiringBBB
DEFS	= 	-DBB_BLACK_ARCH
CFLAGS	= 	$(DEBUG) $(DEFS) -std=c++17 -Wall $(INCLUDE) -pipe
LDLIBS  = 	-L/usr/local/lib -L./libTarts -L./libWiringBBB -lwiringBBB -lTarts -lpthread -lm -lrt -lcurl

SRC	=	TartsWebClient.cpp
		
OBJ	=	$(SRC:.cpp=.o)
BINS	=	$(SRC:.cpp=)

all:		$(OBJ) $(BINS)

TartsWebClient:	TartsWebClient.o
		@echo [Linking : TartsWebClient]
		@$(CC) -o $@ TartsWebClient.o $(LDLIBS) 

.cpp.o:		
		@echo [Compiling] $<
		@$(CC) -c $(CFLAGS) $< -o $@

clean:		
		@echo "[Cleaning all example object and executable files]"
		@rm -f $(OBJ)
		@rm -f $(BINS)
		@rm -f utils.o

