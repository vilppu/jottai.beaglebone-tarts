/**********************************************************************************
 * Tarts.h :: Main header file for the Tarts Library                              *
 * Created   :: July 2014 by Kelly Lewis - MSEE                                   *
 * Modified  :: November 2014 by Kelly Lewis - MSEE                               *
 * Copyright (c) 2014 Tart Sensors. All rights reserved.                          *
 **********************************************************************************
 *   This file is distributed in the hope that it will be useful, but WITHOUT     *
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
 *   FITNESS FOR A PARTICULAR PURPOSE.  Further inquiries in to licences can be   *
 *   found at www.tartssensors.com/licenses                                       * 
 *********************************************************************************/


#ifndef TartsLib_h
#define TartsLib_h


#define TARTSLIB_VERSION  100

//Uncomment to see raw traffic and error messages from the tarts library
//Must be set before compilation of library
//#define TARTS_DEBUG


#include "TartsPlatform.h"
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//DATUM CLASS (USED TO PASS DATA TO USER APPLICATION)
//Please see "SensorMessageEvent" in the documentation
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class Datum
{
  public:
    ~Datum(){}
    Datum(){}
    Datum(char* name, char* value, char* formattedvalue){
      Name = name; Value = value; FormattedValue = formattedvalue;
    }
    
    char* Name;
    char* Value;
    char* FormattedValue;
};

class SensorMessage
{
  public:
    ~SensorMessage(){}
    SensorMessage(const char* id, int8_t rssiValue, int16_t batteryVoltageValue, Datum* list){
      ID = id; RSSI = rssiValue; BatteryVoltage = batteryVoltageValue; DatumList = list;
    }
    
    const char* ID;
    int8_t RSSI;
    uint16_t BatteryVoltage;
    Datum* DatumList;
    int8_t DatumCount;
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//Event Handle TypeDefs for User-space Interactions
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
typedef void (*GatewayPersistEvent_t)(const char* id);
typedef void (*GatewayMessageEvent_t)(const char* id, int stringID);
typedef void (*SensorPersistEvent_t)(const char* id);
typedef void (*SensorMessageEvent_t)(SensorMessage* message);
typedef void (*LogExceptionEvent_t)(int stringID);

#include "TartsSensors.h"


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//Public Gateway Class
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

//Gateway State Variables
typedef enum {UNINITALIZED = 0, OFF, INITALIZED, STARTING, REFORMING, REMOVING, LOADING, ACTIVATING, ACTIVE} GatewayState;

//Gateway Device Object Class
class TartsGateway
{
  friend class TartsLib;
  
  public: //Public methods
    ~TartsGateway();  
    
    #if defined(BB_BLACK_ARCH)
      //Specific Constructor used on the Beaglebones Black
      TartsGateway(const char* id, uint32_t channelMask, uint8_t uartNum, uint8_t pinActivity, uint8_t pinPCTS, uint8_t pinPRTS, uint8_t pinNRST);
      static TartsGateway* Create(const char* id, uint32_t channelMask, uint8_t uartNum, uint8_t pinActivity, uint8_t pinPCTS, uint8_t pinPRTS, uint8_t pinNRST);
    #else	
      //Arduino:: When creating a gateway, if declared in global/fixed memory, a user can use this constructor to set up all gateway parameters.
      //Example:
      //-In Global space
      //"Tarts tarts;" 
      //"TartsGateway gw(1000,0xFFFFFFFF,0x30,5,6);" //NOTE THE DEFAULT PRAMATERS HERE for channelMask, address, pinDataReady, pinReset
      //-In Arduino setup()
      //"tarts.RegisterGateway(&gw);
      TartsGateway(const char* id, uint32_t channelMask, uint8_t address, uint8_t pinDataReady, uint8_t pinReset);
      TartsGateway(const char* id, uint32_t channelMask);
      TartsGateway(const char* id);
    
      //Alternately, if the user wishes to reserve dynamic memory to create this gateway, this method will create the object and 
      //return the reference so the tarts object can record it.  
      //Example:
        //-In Global space
        //"Tarts tarts;" 
        //-In Arduino setup()
        //"tarts.RegisterGateway(TartsGateway.Create(1000,0xFFFFFFFF,0x30,5,6));"  //AGAIN, NOTE the default parameters
      static TartsGateway* Create(const char* id, uint32_t channelMask, uint8_t address, uint8_t pinDataReady, uint8_t pinReset);
      static TartsGateway* Create(const char* id, uint32_t channelMask);
      static TartsGateway* Create(const char* id);
    #endif    
   
    //These properties can be read out and saved for "persistence" call backs from the main Tarts object.
    const char* getGatewayID();    //ID of the Gateway Shield
    uint32_t getChannelMask();     //Permitted channels available during a reform. (0xFFFFFFFF -> All channels enabled, 0x000000FF -> Chan[0-7] enabled)
    
    //Methods to verify the gateway's current state and operating channel.
    uint8_t getOperatingChannel();
    GatewayState getState();
    
    //Method to force the gateway to reform its network settings.  This causes the the gateway to forget all its registered sensors, 
    //pick a new channel, then get reloaded with all currently registered sensors.  This is most commonly called to clear off old
    //sensors that were previously registered to this gateway or if the gateway is in conflict with another gateway (rare occurrence).
    void reformNetwork();
    void reformNetwork(uint32_t newMask);
    
    //Method to discover gateways and sensors that have talked in.  See the Sniffer example program for implementation details on these parameters
    const char* getLastUnknownID();            //Return of 0 means the ID is invalid
    uint16_t getLastUnknownSensorType(); //Return of 0 means the Type is invalid
    
  protected:
    uint32_t GatewayID;       
    uint32_t ChannelMask;    

    #if defined(BB_BLACK_ARCH)
    uint8_t  UartNum;
    uint8_t  PinActivity;
    uint8_t  PinPCTS;
    uint8_t  PinPRTS;
    uint8_t  PinNRST;
    #endif
    uint8_t  Address;         
    uint8_t  PinDataReady;    
    uint8_t  PinReset;

    bool _freeOnRemove; // Set to true if this class object was created dynamically (heap).
    
    
  private:
    uint32_t _lastUnknownID;
    uint16_t _lastUnknownSensorType;
    unsigned long _lastTransactionTime;
    TartsSensorBase** _senObjList;
    uint32_t* _senObjRemoveList;
    uint8_t  _senObjListCount;
    uint8_t  _senObjRemoveListCount;
    uint8_t  _senObjProcessListCount;
    uint8_t  _wirelessState;
    uint8_t  _sensorCount;
    uint8_t  _channel;
    uint8_t  _errors;
    bool _reformNetworkNeeded;
    bool _removeNeeded;
    bool _loadNeeded;
    bool _netStatsRXD;
    bool _firstActive;
    bool _queuePending;
    GatewayState _state;
};



//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//Public Tarts Class
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

class TartsLib
{
  public:
    ~TartsLib();

    //It is recommended that this object be created in the global space and always accessible to your user sketch.
    TartsLib();
    
    //Gateway Operations----------------------------------------------------------
    bool RegisterGateway(TartsGateway* gateway);
    void RemoveGateway(const char* gatewayID);
    TartsGateway* FindGateway(const char* gatewayID);
    
    //Sensor Operations-----------------------------------------------------------
    bool RegisterSensor(const char* gatewayID, TartsSensorBase* sensor);
    void RemoveSensor(const char* sensorID);
    TartsSensorBase* FindSensor(const char* sensorID);
    
    //Event Handlers--------------------------------------------------------------
    void RegisterEvent_GatewayPersist(GatewayPersistEvent_t function);
    void RegisterEvent_GatewayMessage(GatewayMessageEvent_t function);
    void RegisterEvent_SensorPersist(SensorPersistEvent_t function);
    void RegisterEvent_SensorMessage(SensorMessageEvent_t function);
    void RegisterEvent_LogException(LogExceptionEvent_t function);

    //Method called frequently to enable Tarts Gateway and Sensor Processing
    void Process();
    
  private:
    int gwObjListCount;
    TartsGateway** gwObjList;
    GatewayPersistEvent_t GatewayPersistEvent;
    GatewayMessageEvent_t GatewayMessageEvent;
    SensorPersistEvent_t  SensorPersistEvent;
    SensorMessageEvent_t  SensorMessageEvent;
    LogExceptionEvent_t   LogExceptionEvent;
    TartsSensorBase* FindSensorInternal(uint32_t sensorID);
};

extern TartsLib Tarts;



#endif //TartsLib_h



