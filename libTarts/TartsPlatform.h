/**********************************************************************************
 * TartsPlatform.h :: Platform Specific Implementation header file.               *
 * Created   :: July 2014 by Kelly Lewis - MSEE                                   *
 * Modified  :: November 2014 by Kelly Lewis - MSEE                               *
 * Copyright (c) 2014 Tart Sensors. All rights reserved.                          *
 **********************************************************************************
 *   This file is distributed in the hope that it will be useful, but WITHOUT     *
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
 *   FITNESS FOR A PARTICULAR PURPOSE.  Further inquiries in to licences can be   *
 *   found at www.tartssensors.com/licenses                                       * 
 *********************************************************************************/

#ifndef TartsPlatform_h
#define TartsPlatform_h

//#ifdef __cplusplus
//extern "C" {
//#endif

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//PLATFORM SPECIFIC DEFINITONS AND ABSTRACTIONS
// - Wire or UART definitions (Wire or Wire1 or Serial)
// - I2C Digital Port Translations (SDA and I2C) or UART pin translation
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------


#ifdef ARDUINO
  #if( ARDUINO < 106 )
    #error IDE TOO OLD, DOES NOT SUPPORT FEATURES REQUIRED BY THE TARTS LIBRARY.  PLEASE UPDATE TO IDE 1.0.6 AS A MINIMUM
  #endif
 
//If Arduino IDE is less than 1.53, the following definitions need to be made to stop errors from occurring.  
  #if( ARDUINO < 153 )
    #define ARDUINO_ARCH_AVR
  
    //CHOOSE ONE ONLY
    //#define ARDUINO_AVR_UNO
    #define ARDUINO_AVR_ETHERNET
    //#define ARDUINO_AVR_LEONARDO
    //#define ARDUINO_AVR_ADK
    //#define ARDUINO_AVR_MEGA2560
    
    #if !defined(ARDUINO_AVR_UNO) && !defined(ARDUINO_AVR_ETHERNET) && !defined(ARDUINO_AVR_LEONARDO) && !defined(ARDUINO_AVR_ADK) && !defined(ARDUINO_AVR_MEGA2560)
       #error ARDUINO IDE DOES NOT SUPPORT AUTO-PLATFORM IDENTIFICATION.  THEREFORE USER MUST UNCOMMENT ONE(1) OF THE LISTED TARGET BOARDS IN "TartsPlatorm.h".
    #endif
  #endif
 
  #include <Arduino.h>
  #include <Wire.h>
  
  #ifdef TARTS_DEBUG
    #define PLATFORM_PRINTLN(x)   Serial.println(x);
  #else
    #define PLATFORM_PRINTLN(x)   
  #endif

  //Uno Support 
  #ifdef ARDUINO_AVR_UNO
    //16MHz Clock CPU
    //SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR) * 4^TwPSx[0,1,2,3])
    #define PLATFORM_WIRE   Wire
    #define SDA_PIN         A4    
    #define SCL_PIN         A5
    #define PLATFORM_WIRE_RELEASE_PINS    TWCR = 0
  #endif

  //Ethernet Support
  #ifdef ARDUINO_AVR_ETHERNET
    //16MHz Clock CPU
    //SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR) * 4^TwPSx[0,1,2,3])
    #define PLATFORM_WIRE   Wire
    #define SDA_PIN         A4    
    #define SCL_PIN         A5
    #define PLATFORM_WIRE_RELEASE_PINS    TWCR = 0
  #endif

  //Duo Support
  #ifdef ARDUINO_SAM_DUE
    #define PLATFORM_WIRE   Wire1
    #define SDA_PIN         70    
    #define SCL_PIN         71
    #define PLATFORM_WIRE_RELEASE_PINS    //Nothing required
  #endif

  //Leonardo Support
  #ifdef ARDUINO_AVR_LEONARDO
    //16MHz Clock CPU
    //SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR) * 4^TwPSx[0,1,2,3])
    #define PLATFORM_WIRE     Wire
    #define SDA_PIN           2    
    #define SCL_PIN           3
    #define PLATFORM_WIRE_RELEASE_PINS    TWCR = 0
  #endif

  //Yun Support
  #ifdef ARDUINO_AVR_YUN
    //16MHz Clock CPU
    //SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR) * 4^TwPSx[0,1,2,3])
    #define PLATFORM_WIRE     Wire
    #define SDA_PIN           2    
    #define SCL_PIN           3
    #define PLATFORM_WIRE_RELEASE_PINS    TWCR = 0  
  #endif  
    
  //MEGA-ADK Support
  #ifdef ARDUINO_AVR_ADK
    //16MHz Clock CPU
    //SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR) * 4^TwPSx[0,1,2,3])
    #define PLATFORM_WIRE     Wire
    #define SDA_PIN           2    
    #define SCL_PIN           3
    #define PLATFORM_WIRE_RELEASE_PINS    TWCR = 0
  #endif  

  //MEGA2560 Support
  #ifdef ARDUINO_AVR_MEGA2560
    //16MHz Clock CPU
    //SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR) * 4^TwPSx[0,1,2,3])
    #define PLATFORM_WIRE     Wire
    #define SDA_PIN           20    
    #define SCL_PIN           21
    #define PLATFORM_WIRE_RELEASE_PINS    TWCR = 0                                
  #endif  
    
  //Intel Galileo (Preliminary Attempt)
  #ifdef __ARDUINO_X86__
    //Doesn't support F, PSTR, or String constructors with floating point parameters
    #define STRING_WITH_NO_FLOATS
    
    //#include <mux.h>

    #define PLATFORM_WIRE   Wire
    #define SDA_PIN         18    
    #define SCL_PIN         19
    #define PLATFORM_WIRE_RELEASE_PINS  muxSelect(SCL, FN_GPIO_INPUT_HIZ);    
  #endif

#endif //Arduino
  




#ifdef RASPBERRY_PI_ARCH
  #include <wiringPi.h>
  #include <wiringSerial.h>  
  
  #include <stdio.h>
  #include <errno.h>
  #include <cstring>
  #include <fstream>
  #include <sstream>
  #include <iostream>
  #include <string>
  #include <unistd.h> 
  #include <time.h> 
  #include <stdio.h> 
  #include <stdlib.h> 
  #include <stdint.h>
  #include <stdarg.h>
  #include <string.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/ioctl.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <sys/time.h>
  #include <sys/wait.h>
  #include <pthread.h>

  #ifdef TARTS_DEBUG
    #define PLATFORM_PRINTLN(x)   printf(x); printf("\n")
  #else
    #define PLATFORM_PRINTLN(x)   
  #endif
  
  #define INT_IS_32_SIZED  

  #define nPCTS_READY     digitalWrite (0, LOW)
  #define nPCTS_NOTREADY  digitalWrite (0, HIGH)
  
  #define TARTS_THREAD      PI_THREAD
  #define TARTS_THREADSTART piThreadCreate
  #define TARTS_DELAYMS(x)  delayMicroseconds(1000*x)
  
#endif


#ifdef BB_BLACK_ARCH
  #include <stdio.h>
  #include <errno.h>
  #include <cstring>
  #include <fstream>
  #include <sstream>
  #include <iostream>
  #include <string>
  #include <unistd.h> 
  #include <time.h> 
  #include <stdio.h> 
  #include <stdlib.h> 
  #include <stdint.h>
  #include <stdarg.h>
  #include <string.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/ioctl.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <sys/time.h>
  #include <sys/wait.h>
  #include <pthread.h>
  #include <wiringBBB.h>

  #ifdef TARTS_DEBUG
    #define PLATFORM_PRINTLN(x)   printf(x); printf("\n")
  #else
    #define PLATFORM_PRINTLN(x)   
  #endif

  #define INT_IS_32_SIZED  
  
  #define nPCTS_READY     digitalWrite (PinPCTS, LOW)
  #define nPCTS_NOTREADY  digitalWrite (PinPCTS, HIGH)
   
  #define TARTS_THREAD      BBB_THREAD
  #define TARTS_THREADSTART bbbThreadCreate
  #define TARTS_DELAYMS(x)  delay(x)
#endif

#if defined(BB_BLACK_ARCH)
extern bool Platform_gatewayInitialize(uint8_t uartNum, uint8_t pinActivity, uint8_t pinPCTS, uint8_t pinPRTS, uint8_t pinNRST);
#else
extern bool Platform_gatewayInitialize(uint8_t addr, uint8_t pinReset, uint8_t pinDataReady);
#endif

const char* IntToBase36Array(uint32_t value);
uint32_t Base36ArrayToInt(const char* value);

extern bool isHeapStackVarNotGlobal( int ptrAddress);
extern bool Platform_inboundPacketReady(uint8_t dataReady);
extern void Platform_retrieveInboundPacket(uint8_t addr, uint8_t dataReady, uint8_t* buf);
extern void Platform_sendMessage(uint8_t addr, uint8_t* msg);
extern void Platform_Dispose(void);

//#ifdef __cplusplus
//}
//#endif


#endif //TartsPlatform



