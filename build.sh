#################################################################################
# Simple build script to easily make and load all libraries and example programs
# created by www.tartssensors.com.   Any 3rd party libraries (found in the 
# documentation) must be loaded elsewhere. 
#
# USAGE:    "./build"  to execute the build script.
#           "./build all" to clean / un-install everything, then build again.
#
# NOTE:     UART1, UART2, and UART4 are loaded by this script by default.  If you
#           want UART5 enabled, you need to uncomment it below on line 72.
# Created:  Kelly Lewis, October 2014
#	Copyright (c) 2014 Tartssensors.com
#################################################################################
#This file is distributed in the hope that it will be useful, but WITHOUT    
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      
#  FITNESS FOR A PARTICULAR PURPOSE.  Further inquiries in to licences can be 
#  found at www.tartssensors.com/licenses  
#################################################################################

if [ "$1" = "all" ]; then
	cd libWiringBBB
	make uninstall
	make clean > /dev/null 2>&1
	cd ..
	cd libTarts
	make uninstall
	make clean > /dev/null 2>&1
	cd ..
	make clean
fi

test -e /usr/local/include/wiringBBB.*
if [ $? -eq 1 ]; then
	echo "[wiringBBB library files not detected -- Loading library]"
	cd libWiringBBB
        make install > /dev/null 2>&1
	cd ..
fi

test -e /usr/local/lib/libwiringBBB.*
if [ $? -eq 1 ]; then
	echo "[wiringBBB library files not detected -- Loading library]"
	cd libWiringBBB
	make install > /dev/null 2>&1
	cd ..
fi

test -e /usr/local/include/Tarts.*
if [ $? -eq 1 ]; then
	echo "[Tarts library files are not detected -- Loading library]"
	cd libTarts
	make install > /dev/null 2>&1
	cd ..
fi

test -e /usr/local/lib/libTarts.*
if [ $? -eq 1 ]; then
	echo "[Tarts library files are not detected -- Loading library]"
	cd libTarts
	make install > /dev/null 2>&1
	cd ..
fi

cat /sys/devices/bone_capemgr.*/slots | grep BB-UART > /dev/null
if [ $? -eq 1 ]; then
	echo "[UART Virtual Capes not loaded -- Loading UARTs 1, 2 and 4]"
	#IF YOU DO NOT WANT ONE OR MORE OF THESE, COMMENT THEM OUT!
	#UART5 CONFLICTS WITH THE HDMI.  IF YOU DO NOT NEED IT, THEN DISABLED IT AND UART5 CAN NOW BE USED
	echo BB-UART1 > /sys/devices/bone_capemgr.9/slots
	echo BB-UART2 > /sys/devices/bone_capemgr.9/slots
	echo BB-UART4 > /sys/devices/bone_capemgr.9/slots
	#echo BB-UART5 > /sys/devices/bone_capemgr.9/slots	
fi

echo "[Making Example Applications Now...]"
make

