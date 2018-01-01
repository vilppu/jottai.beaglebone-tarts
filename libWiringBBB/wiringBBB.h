/********************************************************************************
 * wiringBBB.h:  WiringBBB Library for the Beaglebones Black 
 *
 *            The BBB platform does not have an Arduino-like API that the Tarts
 *            library was initially built around.  On the Raspberry Pi, Gordon 
 *            Henderson wrote the WiringPI library that performs this function.
 *            With inspiration fron Wiring PI and Derek Molloy's videos and blog
 *            (https://github.com/derekmolloy/beaglebone), I created WiringBBB.
 *
 * Created:   Kelly Lewis, October 2014
 * Copyright (c) 2014 Tartssensors.com
 ********************************************************************************
 * This file is distributed in the hope that it will be useful, but WITHOUT    
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or      
 * FITNESS FOR A PARTICULAR PURPOSE.  Further inquiries in to licences can be 
 * found at www.tartssensors.com/licenses  
 *******************************************************************************/

 
#ifndef _WIRING_BBB_
#define _WIRING_BBB_

#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <wordexp.h>

#ifdef __cplusplus
extern "C" {
#endif

//from "cat /sys/kernel/debug/pinctrl/44e10800.pinmux/pins" to know the pins

//Associate pin header positions to gpioX in "/sys/class/gpio/gpioX"

#define P8_07 (66u)   //pin 36 (44e10890) 00000037 pinctrl-single (GPIO-IPU)
#define P8_08 (67u)   //pin 37 (44e10894) 00000037 pinctrl-single (GPIO-IPU)
#define P8_09 (69u)   //pin 39 (44e1089c) 00000037 pinctrl-single (GPIO-IPU)
#define P8_10 (68u)   //pin 38 (44e10898) 00000037 pinctrl-single (GPIO-IPU)
#define P8_11 (45u)   //pin 13 (44e10834) 00000027 pinctrl-single (GPIO-IPD)
#define P8_12 (44u)   //pin 12 (44e10830) 00000027 pinctrl-single (GPIO-IPD)
#define P8_13 (23u)   //pin 9  (44e10824) 00000027 pinctrl-single  (GPIO-IPD)
#define P8_14 (26u)   //pin 10 (44e10828) 00000027 pinctrl-single (GPIO-IPD)
#define P8_15 (47u)   //pin 15 (44e1083c) 00000027 pinctrl-single (GPIO-IPD)
#define P8_16 (46u)   //pin 14 (44e10838) 00000027 pinctrl-single (GPIO-IPD)
#define P8_17 (27u)   //pin 11 (44e1082c) 00000027 pinctrl-single (GPIO-IPD)
#define P8_18 (65u)   //pin 35 (44e1088c) 00000027 pinctrl-single (GPIO-IPD)
#define P8_19 (22u)   //pin 8  (44e10820) 00000027 pinctrl-single  (GPIO-IPD)  
#define P8_26 (61u)   //pin 31 (44e1087c) 00000037 pinctrl-single (GPIO-IPU)

#define P9_12 (60u)   //pin 30 (44e10878) 00000037 pinctrl-single (GPIO-IPU)
#define P9_14 (50u)   //pin 18 (44e10848) 00000027 pinctrl-single (GPIO-IPD)
#define P9_15 (48u)   //pin 16 (44e10840) 00000027 pinctrl-single (GPIO-IPD)
#define P9_16 (51u)   //pin 19 (44e1084c) 00000027 pinctrl-single (GPIO-IPD)
#define P9_17 (5u)    //pin 87 (44e1095c) 00000062 pinctrl-single (GPIO-IPD)
#define P9_18 (4u)    //pin 86 (44e10958) 00000062 pinctrl-single (GPIO-IPD)
#define P9_23 (49u)   //pin 17 (44e10844) 00000027 pinctrl-single (GPIO-IPD)
#define P9_27 (115u)  //pin 105 (44e109a4) 00000027 pinctrl-single (GPIO-IPD)
#define P9_41 (20u)   //pin 109 (44e109b4) 00000027 pinctrl-single (GPIO-IPD)
#define P9_42 (7u)    //pin 89 (44e10964) 00000027 pinctrl-single (GPIO-IPD)

#define HIGH 0x1
#define LOW  0x0

#define INPUT     false
#define OUTPUT    true


extern void pinMode(unsigned int  pin, bool direction);
extern void digitalWrite(unsigned int  pin, unsigned int  value);
extern int  digitalRead(unsigned int  pins);


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//Timing (ARDUINO) Specific Methods and Controls
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
extern void          delay             (unsigned int howLong) ;
extern unsigned int  millis            (void) ;
extern unsigned int  micros            (void) ;



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//Thread Specific Methods and Controls
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#define	BBB_THREAD(X)		void *X (void *dummy)
extern  int bbbThreadCreate     (void *(*fn)(void *)) ;



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//UART Specific Methods and Controls
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//This opens and initialises the serial device and sets the baud rate. It sets the port into “raw” mode (character at a time and no translations), and sets the read timeout to 10 seconds. The return value is the file descriptor or -1 for any error, in which case errno will be set as appropriate.
extern int   serialOpen      (const char *device, const int baud) ;
//Closes the device identified by the file descriptor given.
extern void  serialClose     (const int fd) ;
//Sends the single byte to the serial device identified by the given file descriptor.
extern void  serialPutchar   (const int fd, const unsigned char c) ;
//Returns the number of characters available for reading, or -1 for any error condition, in which case errno will be set appropriately.
extern int   serialDataAvail (const int fd) ;
//Returns the next character available on the serial device. This call will block for up to 10 seconds if no data is available (when it will return -1)
extern int   serialGetchar   (const int fd) ;



//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//Master FUNCTIONS
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int wiringbbb_Setup(int uartNum, int pinActivity, int pinPCTS, int pinPRTS, int pinNRST);
void wiringbbb_Close(int pinActivity, int pinPCTS, int pinPRTS, int pinNRST);


#ifdef __cplusplus
}
#endif

#endif /* _WIRING_BBB_ */