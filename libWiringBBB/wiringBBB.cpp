/********************************************************************************
 * wiringBBB.cpp:  WiringBBB Library for the Beaglebones Black 
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


#include "wiringBBB.h" 

#include <unistd.h> 
#include <time.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//Pin Specific Operations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool pin_export(unsigned int pin){
  char buf[8];
  int fd = open("/sys/class/gpio/export",O_WRONLY);
  int len = snprintf(buf,sizeof(buf),"%d",pin);
  if(fd<0){
    printf("wiringBBB::Error during export of GPIO%d [%d]\n", pin, fd);
    return true;
  }
  write(fd,buf,len);
  close(fd);
  return false;
}

bool pin_unexport(unsigned int pin){
  char buf[8];
  int fd = open("/sys/class/gpio/unexport",O_WRONLY);
  int len = snprintf(buf,sizeof(buf),"%d",pin);
  if(fd<0){
    printf("wiringBBB::Error during unexport of GPIO%d [%d]\n", pin, fd);
    return true;
  }
  write(fd,buf,len);
  close(fd);
  return false;
}

bool uart_prep(unsigned int uartnum){
  char buf[64];
  int len = snprintf(buf,sizeof(buf),"BB-UART%d",uartnum);
  int fd = open("/sys/devices/bone_capemgr.*/slots",O_WRONLY);
  if(fd<0){
    printf("wiringBBB::Error accessing capemgr [%d]\n", fd);
    return true;
  }
  write(fd,buf,len);
  close(fd);
  return false;
}

void pinMode(unsigned int  pin, bool direction){
  int fd;
	char buf[64];
	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(buf, O_WRONLY);
	if (fd < 0) { printf("wiringBBB::gpio/direction error [%d]\n", fd); return; }
	if (direction == OUTPUT) write(fd, "out", 4);
	else write(fd, "in", 3);
	close(fd);
}

void digitalWrite(unsigned int  pin, unsigned int  value){
	int fd;
	char buf[64];
	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", pin);
	fd = open(buf, O_WRONLY);
	if (fd < 0) { printf("wiringBBB::gpio write error [%d]\n", fd); return; }
	if (value != 0) write(fd, "1", 2);
	else write(fd, "0", 2);
	close(fd);
}	

int digitalRead(unsigned int pin){
	int fd;
	char buf[64];
	char ch;
	snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", pin);
	fd = open(buf, O_RDONLY);
	if (fd < 0) { printf("wiringBBB::gpio read error[%d]\n", fd); return LOW; }
	read(fd, &ch, 1);
	if (ch != '0') return HIGH;
	else return LOW;
	close(fd);
	return 0;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//TIMING FUNCTIONS
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
 

struct timespec prog_start_time;

unsigned int micros()
{
	struct timespec gettime_now;
	clock_gettime(CLOCK_MONOTONIC, &gettime_now);
	return ((gettime_now.tv_sec - prog_start_time.tv_sec)*1000000 +
			(gettime_now.tv_nsec - prog_start_time.tv_nsec)/1000);
}

unsigned int millis()
{
	return micros()/1000;
}

void delay(unsigned int ms)
{
	usleep(ms*1000);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//Thread Specific Methods and Controls
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int bbbThreadCreate (void *(*fn)(void *))
{
  pthread_t myThread ;
  return pthread_create (&myThread, NULL, fn, NULL) ;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//UART FUNCTIONS
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//This opens and initialises the serial device and sets the baud rate. It sets the port into “raw” mode (character at a time and no translations), and sets the read timeout to 10 seconds. The return value is the file descriptor or -1 for any error, in which case errno will be set as appropriate.
int serialOpen (const char *device, const int baud){
  struct termios options;
  speed_t myBaud;
  int status, fd;

  switch (baud){
    case   9600:	myBaud =   B9600; break;
    case  19200:	myBaud =  B19200; break;
    case  38400:	myBaud =  B38400; break;
    case  57600:	myBaud =  B57600; break;
    case 115200:	myBaud = B115200; break;
    case 230400:	myBaud = B230400; break;
    default:
      return -2;
  }

  if ((fd = open (device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) == -1) return -1;
  fcntl (fd, F_SETFL, O_RDWR);

// Get and modify current options:
  tcgetattr(fd, &options);
  cfmakeraw(&options);
  cfsetispeed(&options, myBaud);
  cfsetospeed(&options, myBaud);

  options.c_cflag |= (CLOCAL | CREAD);
  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  options.c_oflag &= ~OPOST;
  options.c_cc [VMIN]  =   0;
  options.c_cc [VTIME] = 100;	// Ten seconds (100 deciseconds)
  tcsetattr (fd, TCSANOW | TCSAFLUSH, &options);
  ioctl (fd, TIOCMGET, &status);
  status |= TIOCM_DTR;
  status |= TIOCM_RTS;
  ioctl (fd, TIOCMSET, &status);
  usleep (10000);	// 10mS
  return fd;
}

//Closes the device identified by the file descriptor given.
void serialClose (const int fd){ close (fd); }

//Sends the single byte to the serial device identified by the given file descriptor.
void serialPutchar (const int fd, const unsigned char c){ write (fd, &c, 1); }

//Returns the number of characters available for reading, or -1 for any error condition, in which case errno will be set appropriately.
int serialDataAvail (const int fd){
  int result;
  if (ioctl (fd, FIONREAD, &result) == -1) return -1;
  return result;
}

//Returns the next character available on the serial device. This call will block for up to 10 seconds if no data is available (when it will return -1)
int serialGetchar (const int fd){
  uint8_t x;
  if (read (fd, &x, 1) != 1) return -1;
  return ((int)x) & 0xFF;
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//Master FUNCTIONS
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int wiringbbb_Setup(int uartNum, int pinActivity, int pinPCTS, int pinPRTS, int pinNRST){
  //FAILS TO Make sure the UART is MUXED IN CORRECTLY.
  //if(uart_prep(uartNum)) return 1;      //Failed
  if(pin_export(pinActivity)) return 2; //Failed
  if(pin_export(pinPCTS)) return 2;     //Failed
  if(pin_export(pinPRTS)) return 3;     //Failed
  if(pin_export(pinNRST)) return 4;     //Failed
  clock_gettime(CLOCK_MONOTONIC, &prog_start_time);
  return 0;
}

void wiringbbb_Close(int pinActivity, int pinPCTS, int pinPRTS, int pinNRST){
  pin_unexport(pinActivity);
  pin_unexport(pinPCTS);
  pin_unexport(pinPRTS);
  pin_unexport(pinNRST);
  //Cannot remove 
}
