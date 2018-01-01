/**********************************************************************************
 * TartsPlatform.cpp :: Platform Specific Implementation source file.             *
 * Created   :: July 2014 by Kelly Lewis - MSEE                                   *
 * Modified  :: November 2014 by Kelly Lewis - MSEE                               *
 * Copyright (c) 2014 Tart Sensors. All rights reserved.                          *
 **********************************************************************************
 *   This file is distributed in the hope that it will be useful, but WITHOUT     *
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
 *   FITNESS FOR A PARTICULAR PURPOSE.  Further inquiries in to licences can be   *
 *   found at www.tartssensors.com/licenses                                       * 
 *********************************************************************************/

#include "Tarts.h"


//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//Helpful Utilities
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------

//from http://jeelabs.org/2011/05/22/atmega-memory-use/
//static int freeRam () {
//  extern int __heap_start, *__brkval; 
//  int v; 
//  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
//}

bool isHeapStackVarNotGlobal( int ptrAddress){
  #if (defined(ARDUINO_ARCH_SAM) || defined(__ARDUINO_X86__) || defined(RASPBERRY_PI_ARCH) || defined(BB_BLACK_ARCH))
    extern int  _end;
    if(ptrAddress >= _end){
       PLATFORM_PRINTLN("IS HEAP/STACK");
       return true;
    }
    return false;
  #elif defined(ARDUINO_ARCH_AVR)
    extern int __heap_start;
    if(ptrAddress >= __heap_start){
      PLATFORM_PRINTLN("IS HEAP/STACK");
      return true;
    }
    return false;
  #else 
    #error UNRECONIZED PLATFORM!  NEED TO HANDLE MEMORY LOCATION FUNCTIONS!
  #endif
}   

//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//Base-36 ID Translator Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
char fixedReturnBuffer[10]; //"T12345<0>"
const char* IntToBase36Array(uint32_t value){
  char baseChars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
  
  int i = sizeof(fixedReturnBuffer); // Maximum size Buffer for Tarts labels
  fixedReturnBuffer[--i] = 0; //Set Null
  
  do{
    fixedReturnBuffer[--i] = baseChars[value % 36];
    value /= 36;
  }
  while (value > 0);
  while (i > 4) fixedReturnBuffer[--i] = '0';
  fixedReturnBuffer[--i] = 'T';
  return &fixedReturnBuffer[i];
}

uint32_t Base36ArrayToInt(const char* value){
  if(value[0] != 'T') return 0; //Didn't start correct
  return (uint32_t) strtoul((const char*) &value[1], 0, 36);
}
  
/*
  
void Tester(void){
 Serial.print("GWID ");
 Serial.print(GATEWAY_ID);
 Serial.print(" translates to ");
 Serial.print(Base36ArrayToInt((char*)GATEWAY_ID));
 Serial.print(".  100384 translates to ");
 Serial.println(IntToBase36Array(100384));
 while(1); 
}

  if( (value[0] != 'T') && (value[0] != 't') ) return 0; //Didn't start correct
  if( value[sizeof(fixedReturnBuffer)-1] != 0) return 0; //Not right length
  uint32_t pos[sizeof(fixedReturnBuffer)-2]; //Do not need to account for the T or NULL in this size
  char baseChars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
  unsigned int i, j;
  for(i = 0; i<sizeof(pos); i++){
    for(j = 0; j<sizeof(baseChars);j++){
      if(value[1+i] == baseChars[j]){
        pos[i] = j;
        break;
      }
    }
    if(j == sizeof(baseChars)) return 0;  //BAD CHARACTOR!
  }

  uint32_t result = pos[--i];
  uint32_t mult = 36;
  while(i != 0){
    result += pos[--i] * mult;
    mult *= 36;
  }
  return result;
*/


/*private static long BaseToInt(string input, char[] baseChars)
{
uint
  int inputLength = input.Length;
  int[] Pos = new int[inputLength];
  char[] inputArr = input.ToCharArray();
  int fromBase = baseChars.Length;


  for (int i = fromBase - 1; i >= 0; i--)
  {
      for (int y = 0; y < inputLength; y++)
      {
          if (inputArr[y] == baseChars[i])
              Pos[y] = i;
      }
  }

  double valD = 0;
  for (int y = 0; y < inputLength; y++)
  {
      valD += Pos[inputLength - 1 - y] * Math.Pow(fromBase, y);
  }
  
  return valD.ToLong();
}
*/

//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//Platform Specific Implementations
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------

#define TARTS_MAX_FRAME_DATA_SIZE       32
#define TARTS_START_FRAME_DELIMINATOR   0xC5


#if defined(BB_BLACK_ARCH)
uint8_t UartNum, PinActivity, PinPCTS, PinPRTS, PinNRST;
#endif

#if defined(RASPBERRY_PI_ARCH) || defined(BB_BLACK_ARCH)

typedef struct tartsserial {
  uint8_t RXbuffer[64];
  uint8_t RXindex;
  uint32_t RXtime;
  int fd;
} TartsSerialMem;

static TartsSerialMem TSerial;
  
static void ResetRXR(void){
  TSerial.RXindex = 0;
  nPCTS_READY;
}

TARTS_THREAD (TSerialThread){
  ResetRXR();

  while(1){
    if(serialDataAvail(TSerial.fd) == -1){
      #ifdef TARTS_DEBUG
      printf("Serial Read Error. Thread exiting! Reason: %s\n", strerror(errno));
      #endif
      break;
    }
    while(serialDataAvail(TSerial.fd)){
      uint8_t c = serialGetchar(TSerial.fd);
      if(TSerial.RXindex == 0){
        if(c == TARTS_START_FRAME_DELIMINATOR){
          TSerial.RXbuffer[TSerial.RXindex++] = c;
          nPCTS_NOTREADY;     //nPCTS is NOT ready
          TSerial.RXtime = millis();
        }
      }
      else if(TSerial.RXindex != 255){
        TSerial.RXbuffer[TSerial.RXindex++] = c;
        if(TSerial.RXindex == (TSerial.RXbuffer[1]+3)){
          TSerial.RXindex = 255;
        }
      }
    }
    
    if((TSerial.RXindex != 0) && (TSerial.RXindex != 255)){
      if(TSerial.RXtime + 50 < millis()) ResetRXR(); //Only allow 50 milliseconds between the start of a packet and the end.
    }
    TARTS_DELAYMS(10);
  }
  
  //Only reason to leave is because there is an error or closing
  return 0;
}

#endif
  

#if defined(__ARDUINO_X86__)
  #include <Mux.h>
#endif

#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(__ARDUINO_X86__)
bool Platform_gatewayInitialize(uint8_t addr, uint8_t pinReset, uint8_t pinDataReady){
  int i;

  //Make sure the WIRE peripheral is not going to block the Digital commands to the SDA/SCL pins
  PLATFORM_WIRE_RELEASE_PINS;
  pinMode(pinDataReady, INPUT);
  pinMode(pinReset, OUTPUT);
  
  //Reset the Tart Gateway board, signal addressing mode
  for(i = 0; i < 2; i++){
  digitalWrite(pinReset, LOW);  //Reset is applied
  pinMode(SDA_PIN, OUTPUT);  
  digitalWrite(SDA_PIN, LOW);  //SDA == LOW signals I2C Mode
  pinMode(SCL_PIN, OUTPUT);
  if((addr & 0xF0) == 0x30) digitalWrite(SCL_PIN, LOW); //8-BIT = 0x60, SCL == LOW
  else digitalWrite(SCL_PIN, HIGH); //SCL == HIGH
  delay(250);
  while(digitalRead(pinDataReady) != LOW);
  pinMode(pinReset, INPUT);        //Reset is released
  delay(750);  			               //wait for the device to reboot
  }
	
  //Set-up I2C Interface
  pinMode(SCL_PIN, INPUT);  //Mimic a stop
  pinMode(SDA_PIN, INPUT); //Mimic a stop 
  PLATFORM_WIRE.begin();  //Master Start
  return true;
}
#endif

#if defined(RASPBERRY_PI_ARCH)
bool Platform_gatewayInitialize(uint8_t addr, uint8_t pinReset, uint8_t pinDataReady){
  int i;
  if(wiringPiSetup() == -1){
    #ifdef TARTS_DEBUG
      printf("Unable to start wiringPi Library: %s\n", strerror(errno));
    #endif
    return false;
  }

  //Setup IO pins to make device
  //UART pins are already set, so only work about the GPIO
  pinMode(7, INPUT); pullUpDnControl(7, PUD_DOWN);                        //Activity  : GPIO_4    ->  WiringPi_7 set to IPD
  pinMode(0, OUTPUT); pullUpDnControl(0, PUD_OFF); digitalWrite (0, LOW); //PCTS      : GPIO_17   ->  WiringPi_0 set to O-L
  pinMode(3, INPUT); pullUpDnControl(3, PUD_UP);                          //PRTS      : GPIO_22   ->  WiringPi_3 set to IPU
  pinMode(4, OUTPUT); pullUpDnControl(4, PUD_OFF); digitalWrite (0, LOW); //RESET     : GPIO_23   ->  WiringPi_4 set to O-L  !!RESET IS ENABLED!!
  delay(250);
  i = 250;
  while(digitalRead(7) != LOW){
    delay(10);
    i--;
    if(i == 0){
      pinMode(4, INPUT); pullUpDnControl(4, PUD_UP);         //Reset is released
      pinMode(0, INPUT); pullUpDnControl(0, PUD_UP);
      #ifdef TARTS_DEBUG
        printf("No Plate Detected!\n");
      #endif
      return false;
    }
  }
  pinMode(4, INPUT); pullUpDnControl(4, PUD_UP);         //Reset is released
  delay(750);  			               //wait for the device to reboot


  //Setup the Serial Interface
  if((TSerial.fd = serialOpen ("/dev/ttyAMA0", 115200)) < 0){
    #ifdef TARTS_DEBUG
      printf("Unable to open serial device: %s\n", strerror(errno));
    #endif
    pinMode(0, INPUT); pullUpDnControl(0, PUD_UP);
    return false;
  }

  if(piThreadCreate (TSerialThread) != 0){
    #ifdef TARTS_DEBUG
      printf("Unable to start Serial thread!");
    #endif
    serialClose(TSerial.fd);
    pinMode(0, INPUT); pullUpDnControl(0, PUD_UP);
    return false;
  }
  return true;
}
#endif

#if defined(BB_BLACK_ARCH)

bool Platform_gatewayInitialize(uint8_t uartNum, uint8_t pinActivity, uint8_t pinPCTS, uint8_t pinPRTS, uint8_t pinNRST){
  int i;
  UartNum = uartNum;
  PinActivity = pinActivity;
  PinPCTS = pinPCTS;
  PinPRTS = pinPRTS;
  PinNRST = pinNRST;


  if(wiringbbb_Setup(UartNum, PinActivity, PinPCTS, PinPRTS, PinNRST) != 0){
    #ifdef TARTS_DEBUG
      printf("Unable to gain access to all GPIO or UART Resources\n");
    #endif
    return false;
  }

  //Setup IO pins to make device
  //UART pins are already set, so only work about the GPIO
  pinMode(PinActivity, INPUT);                            //Activity  : set to IPD
  pinMode(PinPCTS, OUTPUT); digitalWrite (pinPCTS, LOW);  //PCTS      : set to O-L
  pinMode(PinPRTS, INPUT);                                //PRTS      : set to IPU
  pinMode(PinNRST, OUTPUT); digitalWrite (PinNRST, LOW);  //RESET     : set to O-L  !!RESET IS ENABLED!!
  delay(250);
  i = 250;
  while(digitalRead(pinActivity) != LOW){
    delay(10);
    i--;
    if(i == 0){
      digitalWrite (PinNRST, HIGH);    //Reset is released
      pinMode(PinPCTS, INPUT);         //pcts is released
      #ifdef TARTS_DEBUG
        printf("No Plate Detected!\n");
      #endif
      return false;
    }
  }
  digitalWrite (PinNRST, HIGH);          //Reset is released
  delay(750);  			         //wait for the device to reboot

  //Setup the Serial Interface
  char* uartstr;
  if(uartNum == 5) uartstr = (char*)"/dev/ttyO5";
  else if(uartNum == 4) uartstr = (char*)"/dev/ttyO4";
  if(uartNum == 2) uartstr = (char*)"/dev/ttyO2";
  else  uartstr = (char*)"/dev/ttyO1";
  
  if((TSerial.fd = serialOpen((const char*)uartstr, 115200)) < 0){
    #ifdef TARTS_DEBUG
      printf("Unable to open serial device: %s\n", strerror(errno));
    #endif
    pinMode(pinPCTS, INPUT);         //pcts is released
    return false;
  }

  if(bbbThreadCreate (TSerialThread) != 0){
    #ifdef TARTS_DEBUG
      printf("Unable to start Serial thread!");
    #endif
    serialClose(TSerial.fd);
    pinMode(PinPCTS, INPUT);         //pcts is released
    return false;
  }
  return true;
}
#endif

void Platform_Dispose(void){
#if defined(RASPBERRY_PI_ARCH)
  if(TSerial.fd != 0) serialClose(TSerial.fd);
  pinMode(4, INPUT); pullUpDnControl(4, PUD_UP);         //Reset is released
  pinMode(0, INPUT); pullUpDnControl(0, PUD_UP);
#endif

#if defined(BB_BLACK_ARCH)
  if(TSerial.fd != 0) serialClose(TSerial.fd);
  pinMode(PinNRST, INPUT);         //Reset is released
  pinMode(PinPCTS, INPUT);         //pcts is released 
  wiringbbb_Close(PinActivity, PinPCTS, PinPRTS, PinNRST);
#endif
}

bool Platform_inboundPacketReady(uint8_t dataReady){
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(__ARDUINO_X86__)
  if(digitalRead(dataReady) == HIGH) return true;
  else return false;
#endif
#if defined(RASPBERRY_PI_ARCH) || defined(BB_BLACK_ARCH)
  if(TSerial.RXindex == 255) return true;
  else return false;
#endif
}

void Platform_retrieveInboundPacket(uint8_t addr, uint8_t dataReady, uint8_t* buf){
  if(Platform_inboundPacketReady(dataReady)){
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(__ARDUINO_X86__)
    int j = 0;
    #ifdef TARTS_DEBUG
      Serial.print("Inbound: 0x");
    #endif
    PLATFORM_WIRE.requestFrom(addr, (uint8_t)TARTS_MAX_FRAME_DATA_SIZE);
    while(PLATFORM_WIRE.available()){    // slave may send less than requested
      buf[j] = PLATFORM_WIRE.read(); // receive a byte as character
      #ifdef TARTS_DEBUG
        if(buf[j] < 16) Serial.print("0");
        Serial.print(buf[j], HEX);
      #endif
      j++;
    }
#endif
#if defined(RASPBERRY_PI_ARCH) || defined(BB_BLACK_ARCH)
    if(TSerial.RXbuffer[1]+3 < 64){
      #ifdef TARTS_DEBUG
        printf("Inbound: 0x");
      #endif
      for(int j = 0; j < (TSerial.RXbuffer[1]+3); j++){
        buf[j] = TSerial.RXbuffer[j];
        #ifdef TARTS_DEBUG
          printf("%.2X", buf[j]);
        #endif
      }
    }
    ResetRXR();
#endif
  }
}

void Platform_sendMessage(uint8_t addr, uint8_t* msg){
#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(__ARDUINO_X86__)
  uint8_t count = 0;
  do{
    if(count > 10) break;
    if(count != 0) delay(100);
    count++;
  PLATFORM_WIRE.beginTransmission(addr); // start transmission to device 
  PLATFORM_WIRE.write(msg, msg[1]+3); 
  } while(PLATFORM_WIRE.endTransmission() != 0); // end transmission

#elif defined(RASPBERRY_PI_ARCH)
  uint8_t count = 0;
  while(digitalRead(3) != LOW){ //PRTS is invalid
    if(count > 10) {
      PLATFORM_PRINTLN("PRTS BLOCKING - MODULE IS DEAF!");
      return;
    }
    delay(10);
    count++;
  }
 
  PLATFORM_PRINTLN("Sending!");

  for(int i=0; i < (msg[1]+3); i++) serialPutchar (TSerial.fd, msg[i]);

#elif defined(BB_BLACK_ARCH)
  uint8_t count = 0;
  while(digitalRead(PinPRTS) != LOW){ //PRTS is invalid
    if(count > 10) {
      PLATFORM_PRINTLN("PRTS BLOCKING - MODULE IS DEAF!");
      return;
    }
    delay(10);
    count++;
  }

  PLATFORM_PRINTLN("Sending!");
 
  for(int i=0; i < (msg[1]+3); i++) serialPutchar (TSerial.fd, msg[i]);
#endif
}



