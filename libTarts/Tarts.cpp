/**********************************************************************************
 * Tarts.cpp :: Main worker file for the Tarts Library                            *
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
//Private GWAPI Class
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
  
#define TARTS_MAX_FRAME_DATA_SIZE       32
#define TARTS_START_FRAME_DELIMINATOR   0xC5

//Options 
typedef enum { NOOPTS = 0, URGENT = 0x02, SENSOR_WAITING = 0x04 } Options;

//Commands
typedef enum {
  //Local Commands
  FORM_NETWORK_REQUEST = 0x20, UPDATE_NETWORK_STATE, REGISTER_SENSOR_REQUEST, NETWORK_STATUS_MESSAGE, MESSAGE_QUEUED_NOTIFY, SENSOR_STATUS_INTICATOR,
  UNREGISTER_SENSOR_REQUEST = 0x28,
  //Wireless Commands
  DATA_MESSAGE = 0x55, DATA_MESSAGE_DL,
  ERROR = 0xFF,
  READ_DATASECTOR_REQUEST = 0x70, READ_DATASECTOR_RESPONSE, WRITE_DATASECTOR_REQUEST, WRITE_DATASECTOR_RESPONSE, APPCMD_REQUEST, APPCMD_RESPONSE
} GWAPI_Commands;
    
class GWAPI{
  friend class Tarts;  
  
  public:
    uint8_t  buffer[TARTS_MAX_FRAME_DATA_SIZE];
    
    GWAPI(); 
    GWAPI(GWAPI_Commands cmd, Options opt, uint8_t* payload, uint8_t length);
    bool        valid(void);
    GWAPI_Commands  getCommand(void);
    uint8_t     calculateCRC8(void);
    uint32_t    extractID(void);
    bool        isLocalCommand(void);
    void        clear(void);
};


//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//Implementation of GWAPI Methods
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------

GWAPI::GWAPI(){
  clear();
}
GWAPI::GWAPI(GWAPI_Commands cmd, Options opt, uint8_t* payload, uint8_t length){
  buffer[0] = TARTS_START_FRAME_DELIMINATOR;
  buffer[1] = length+2;		//API Length is Options - Data
  buffer[2] = opt;
  buffer[3] = cmd;
  memcpy((void*)&buffer[4], payload, length);
  buffer[buffer[1]+2] = calculateCRC8();
}
bool GWAPI::valid(void){
  if(buffer[0] == TARTS_START_FRAME_DELIMINATOR){
    if(calculateCRC8() == buffer[buffer[1]+2]){
      return true;
    }
  }
  clear();
  return false;
}
GWAPI_Commands  GWAPI::getCommand(void){
  return (GWAPI_Commands)buffer[3];
}
uint8_t GWAPI::calculateCRC8(void){
  uint8_t i, j, crc = 0;
  for(i=0; i<buffer[1]; i++){
    crc ^= buffer[2+i];
    for(j=8; j>0; j--){
      if(crc & 0x80) crc = (crc << 1) ^ 0x97;
      else crc = crc << 1;
    } 
  }
	return crc;
}
uint32_t GWAPI::extractID(void){
  if(isLocalCommand()) return 0;  //Id needs to be known by caller because this is a local message.
  else return  ((uint32_t)buffer[7] << 24) | ((uint32_t)buffer[6] << 16) | ((uint32_t)buffer[5] << 8) | (uint32_t)buffer[4];
}
bool GWAPI::isLocalCommand(void){
  switch(getCommand()){
    case FORM_NETWORK_REQUEST:
    case UPDATE_NETWORK_STATE:
    case REGISTER_SENSOR_REQUEST:
    case NETWORK_STATUS_MESSAGE:
    case MESSAGE_QUEUED_NOTIFY:
    case SENSOR_STATUS_INTICATOR:
    case UNREGISTER_SENSOR_REQUEST:
      return true;
    default:
      return false;
  }
}
void GWAPI::clear(void){
  memset((void*)buffer, 0, TARTS_MAX_FRAME_DATA_SIZE);
}


//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//Private (STATIC) Gateway Method Implementations
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------

static void TartsGateway_setGatewayIdle(uint8_t addr){
  uint8_t buf[5];
  memset((void*)buf, 0, sizeof(buf));
  GWAPI msg = GWAPI(UPDATE_NETWORK_STATE, NOOPTS, buf, sizeof(buf));
  Platform_sendMessage(addr , msg.buffer);
}

static void TartsGateway_setGatewayActive(uint8_t addr){
  uint8_t buf[5];
  buf[0] = 1;
  memset((void*)&buf[1], 0, sizeof(buf)-1);
  GWAPI msg = GWAPI(UPDATE_NETWORK_STATE, NOOPTS, buf, sizeof(buf));
  Platform_sendMessage(addr , msg.buffer);
}

static void TartsGateway_sendReformNetwork(uint8_t addr, uint32_t mask){
  uint8_t buf[5] = { (uint8_t) mask, (uint8_t) (mask >> 8), (uint8_t) (mask >> 16), (uint8_t) (mask >> 24), 0 };
  GWAPI msg = GWAPI(FORM_NETWORK_REQUEST, NOOPTS, buf, sizeof(buf));
  Platform_sendMessage(addr , msg.buffer);
}

static void TartsGateway_sendAssignSensor(uint8_t addr, uint32_t sensorID){
  uint8_t buf[4] = { (uint8_t) sensorID, (uint8_t) (sensorID >> 8), (uint8_t) (sensorID >> 16), (uint8_t) (sensorID >> 24) };
  GWAPI msg = GWAPI(REGISTER_SENSOR_REQUEST, NOOPTS, buf, sizeof(buf));
  Platform_sendMessage(addr , msg.buffer);
} 

static void TartsGateway_sendRemoveSensor(uint8_t addr, uint32_t sensorID){
  uint8_t buf[4];
  buf[0] = (uint8_t) (sensorID >> 0);
  buf[1] = (uint8_t) (sensorID >> 8);
  buf[2] = (uint8_t) (sensorID >> 16);
  buf[3] = (uint8_t) (sensorID >> 24);
  GWAPI msg = GWAPI(UNREGISTER_SENSOR_REQUEST, NOOPTS, buf, sizeof(buf));
  Platform_sendMessage(addr , msg.buffer);
}
    
static void TartsGateway_sendQueuedNotfication(uint8_t addr, uint32_t id, uint8_t set){
  uint8_t buf[5];
  buf[0] = (uint8_t) (id >> 0);
  buf[1] = (uint8_t) (id >> 8);
  buf[2] = (uint8_t) (id >> 16);
  buf[3] = (uint8_t) (id >> 24);
  buf[4] = set;
  GWAPI msg = GWAPI(MESSAGE_QUEUED_NOTIFY, NOOPTS, buf, sizeof(buf));
  Platform_sendMessage(addr , msg.buffer);
}

static void TartsGateway_sendConfigUpdate(uint8_t addr, uint32_t id, uint8_t sector, uint8_t* data){
  uint8_t buf[21];
  buf[0] = (uint8_t) (id >> 0);
  buf[1] = (uint8_t) (id >> 8);
  buf[2] = (uint8_t) (id >> 16);
  buf[3] = (uint8_t) (id >> 24);
  buf[4] = sector;
  memcpy((void*)&buf[5], (void*)data, 16);
  GWAPI msg = GWAPI(WRITE_DATASECTOR_REQUEST, NOOPTS, buf, sizeof(buf));
  Platform_sendMessage(addr , msg.buffer);
}

static void TartsGateway_sendConfigRead(uint8_t addr, uint32_t id, uint8_t sector){
  uint8_t buf[5];
  buf[0] = (uint8_t) (id >> 0);
  buf[1] = (uint8_t) (id >> 8);
  buf[2] = (uint8_t) (id >> 16);
  buf[3] = (uint8_t) (id >> 24);
  buf[4] = sector;
  GWAPI msg = GWAPI(READ_DATASECTOR_REQUEST, NOOPTS, buf, sizeof(buf));
  Platform_sendMessage(addr , msg.buffer);
}

static void TartsGateway_sendApplicationCommand(uint8_t addr, uint32_t id, uint8_t* data, uint8_t len, bool setUrgent){
  uint8_t buf[4+len];
  buf[0] = (uint8_t) (id >> 0);
  buf[1] = (uint8_t) (id >> 8);
  buf[2] = (uint8_t) (id >> 16);
  buf[3] = (uint8_t) (id >> 24);
  memcpy((void*)&buf[4], (void*)data, len);
  GWAPI msg = GWAPI(APPCMD_REQUEST, (setUrgent) ? URGENT : NOOPTS, buf, sizeof(buf));
  Platform_sendMessage(addr , msg.buffer);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//Gateway Class Implementation
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------

TartsGateway::~TartsGateway(){
  TartsGateway_setGatewayIdle(Address);
  delay(100);
  free(_senObjList);
  free(_senObjRemoveList);
  Platform_Dispose();
}

#if defined(BB_BLACK_ARCH)
  //Specific Constructor used on the Beaglebones Black
TartsGateway::TartsGateway(const char* id, uint32_t channelMask, uint8_t uartNum, uint8_t pinActivity, uint8_t pinPCTS, uint8_t pinPRTS, uint8_t pinNRST){
  //Initialized Protected Parameters
  GatewayID = Base36ArrayToInt(id);
  ChannelMask = channelMask;
  UartNum = uartNum;
  PinActivity = pinActivity;
  PinPCTS = pinPCTS;
  PinPRTS = pinPRTS;
  PinNRST = pinNRST;
  Address = 0;
  PinDataReady = 0;
  PinReset = 0;
  _freeOnRemove = isHeapStackVarNotGlobal((int)this);   //Decide on how to dispose of this
      
  //Initialize internal/private variables
  _lastTransactionTime = 0;
  _senObjRemoveList = NULL;
  _senObjList = NULL;
  _senObjListCount = 0;
  _senObjRemoveListCount = 0;
  _senObjProcessListCount = 0;
  _wirelessState = 0;
  _channel = 0;
  _sensorCount = 0;
  _errors = 0;
  _reformNetworkNeeded = false;
  _removeNeeded = false;
  _loadNeeded = false;
  _netStatsRXD = false;
  _queuePending = false;
  _firstActive = true;
  _state = UNINITALIZED;
}

TartsGateway* TartsGateway::Create(const char* id, uint32_t channelMask, uint8_t uartNum, uint8_t pinActivity, uint8_t pinPCTS, uint8_t pinPRTS, uint8_t pinNRST){
  return new TartsGateway(id, channelMask, uartNum, pinActivity, pinPCTS, pinPRTS, pinNRST);
}

#else	
TartsGateway::TartsGateway(const char* id, uint32_t channelMask, uint8_t address, uint8_t pinDataReady, uint8_t pinReset){
  //Initialized Protected Parameters
  GatewayID = Base36ArrayToInt(id);
  ChannelMask = channelMask;
  Address = address;
  PinDataReady = pinDataReady;
  PinReset = pinReset;
  _freeOnRemove = isHeapStackVarNotGlobal((int)this);   //Decide on how to dispose of this
      
  //Initialize internal/private variables
  _lastTransactionTime = 0;
  _senObjRemoveList = NULL;
  _senObjList = NULL;
  _senObjListCount = 0;
  _senObjRemoveListCount = 0;
  _senObjProcessListCount = 0;
  _wirelessState = 0;
  _channel = 0;
  _sensorCount = 0;
  _errors = 0;
  _reformNetworkNeeded = false;
  _removeNeeded = false;
  _loadNeeded = false;
  _netStatsRXD = false;
  _queuePending = false;
  _firstActive = true;
  _state = UNINITALIZED;
}

TartsGateway::TartsGateway(const char* id, uint32_t channelMask){
  TartsGateway(id, channelMask, 0x30, 5, 6);
}

TartsGateway::TartsGateway(const char* id){
  TartsGateway(id, 0xFFFFFFFF, 0x30, 5, 6);
}

TartsGateway* TartsGateway::Create(const char* id, uint32_t channelMask, uint8_t address, uint8_t pinDataReady, uint8_t pinReset){
  return new TartsGateway(id, channelMask, address, pinDataReady, pinReset);
}

TartsGateway* TartsGateway::Create(const char* id, uint32_t channelMask){
  return new TartsGateway(id, channelMask, 0x30, 5, 6);
}

TartsGateway* TartsGateway::Create(const char* id){
  return new TartsGateway(id, 0xFFFFFFFF, 0x30, 5, 6);
}

#endif    

   
const char* TartsGateway::getGatewayID(){
  return IntToBase36Array(GatewayID);
}
uint32_t TartsGateway::getChannelMask(){
  return ChannelMask;
}
uint8_t  TartsGateway::getOperatingChannel(){
  if(_state != ACTIVE) return 0xFF;
  else return _channel;
}
GatewayState TartsGateway::getState(){
  return _state;
}

void TartsGateway::reformNetwork(){
  _reformNetworkNeeded = true;
}

void TartsGateway::reformNetwork(uint32_t newMask){
  ChannelMask = newMask;
  reformNetwork();
}

//Return of 0 means the ID is invalid
const char* TartsGateway::getLastUnknownID(){
  return IntToBase36Array(_lastUnknownID);
}

//Return of 0 means the Type is invalid
uint16_t TartsGateway::getLastUnknownSensorType(){
  return _lastUnknownSensorType;
}
    
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//PRIMARY TartsLib Class Implementation
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------

#define LOGGWP(id)          do{ if(GatewayPersistEvent != 0) GatewayPersistEvent(id);              } while(0)
#define LOGSENP(id)         do{ if(GatewayPersistEvent != 0) SensorPersistEvent(id);               } while(0)
#define LOGGWM(id,sid)      do{ if(GatewayMessageEvent != 0) GatewayMessageEvent(id, sid);         } while(0)
#define LOGEX(sid)          do{ if(LogExceptionEvent != 0)   LogExceptionEvent(sid);               } while(0)


//Start class with everything clear and ready to start initializing
TartsLib::TartsLib(){
  GatewayPersistEvent = NULL;
  GatewayMessageEvent = NULL;
  SensorPersistEvent  = NULL;
  SensorMessageEvent  = NULL;
  LogExceptionEvent   = NULL;
  gwObjList           = NULL;
  gwObjListCount      = 0;
}

TartsLib::~TartsLib(){
  free(gwObjList);
}

bool TartsLib::RegisterGateway(TartsGateway* gateway){
  //First: Verify that GWObj is good
  if(gateway == NULL){
    LOGEX(0); //"ERROR :: RegisterGateway :: Gateway object is NULL"
    return false;
  }
  
  //Second: Verify that this is the only instance of that gateway.
  for(int i = 0; i < gwObjListCount; i++){
    if( gateway == gwObjList[i] ){
      LOGEX(1); //"WARN  :: RegisterGateway :: Duplicate gateway object"
      return true;
    }
    if( gateway->GatewayID == gwObjList[i]->GatewayID ){
      LOGEX(2); //"ERROR :: RegisterGatway :: Identical gateway being registered"
      return false;
    }
  }
  
  //Initialize Hardware
  #if defined(BB_BLACK_ARCH)
  if(Platform_gatewayInitialize(gateway->UartNum, gateway->PinActivity, gateway->PinPCTS, gateway->PinPRTS, gateway->PinNRST) == false){
    LOGEX(3); //"ERROR :: RegisterGateway :: Hardware failed to initialize");
    return false;
  }
  #else
  if(Platform_gatewayInitialize(gateway->Address, gateway->PinReset, gateway->PinDataReady) == false){
    LOGEX(3); //"ERROR :: RegisterGateway :: Hardware failed to initialize");
    return false;
  }
  #endif

  //We are here, so add the gateway.
  gwObjListCount++;
  TartsGateway** newList = (TartsGateway**) realloc((void*)gwObjList, sizeof(TartsGateway*) * gwObjListCount);
	if (newList) {
		gwObjList = newList;
    gwObjList[gwObjListCount-1] = gateway;
    LOGGWM(gateway->getGatewayID(),0); //"Gateway Registered"
		return true; //ALL DONE!!!
	}
  else {
    gwObjListCount--;
    LOGEX(4); //"ERROR :: RegisterGateway :: Memory Exception!"
    return false;
  }
}

void TartsLib::RemoveGateway(const char* gatewayID){
  //Look to see if we have a gateway list to approve
  if(gwObjListCount == 0){
    LOGEX(5); //"WARN  :: RemoveGateway :: Internal gateway list is empty");
    return; 
  }
  
  uint32_t gwid = Base36ArrayToInt(gatewayID);
  //Find Gateway in List and populate a new list without that gateway
  int j = 0;
  TartsGateway** newList = NULL;
  for(int i = 0; i < gwObjListCount; i++){
    if(gwObjList[i]->GatewayID == gwid){
      if(gwObjList[i]->_freeOnRemove) delete gwObjList[i];
      continue;
    }
    j++; //We are keeping this one
    newList = (TartsGateway**) realloc((void*)newList, sizeof(TartsGateway*) * j);
    if(newList) newList[j-1] = gwObjList[i]; 
    else{
      LOGEX(6); //"ERROR :: RemoveGateway :: Memory Exception!"
      return;  //BAD!!! Do not modify anything
    }
  }
  
  if(gwObjListCount == j){
    LOGEX(7); //"WARN :: RemoveGateway :: Gateway ID not found"
    free(newList); //Keep old list
  }
  else{
    free(gwObjList);
    gwObjList = newList;
    gwObjListCount = j;
    LOGGWM(gatewayID,1); //"Gateway Unregistered"
  }
}

TartsGateway* TartsLib::FindGateway(const char* gatewayID){
  uint32_t gwid = Base36ArrayToInt(gatewayID);
  for(int i = 0; i < gwObjListCount; i++){
    if(gwObjList[i]->GatewayID == gwid) return gwObjList[i];
  }
  return NULL;
}

bool TartsLib::RegisterSensor(const char* gatewayID, TartsSensorBase* sensor){
  //First: Verify that SensorObj is good
  if((sensor == NULL) || (sensor->SensorID == 0)){
    LOGEX(8); //"ERROR :: RegisterSensor :: Sensor object is NULL/INVALID"
    return false;
  }
  
  uint32_t gwid = Base36ArrayToInt(gatewayID);
  
  //Next: Have to confirm that if there are duplicate IDs, that the memory address match meaning the objects are the same.  If duplicate don't match, then an error is thrown.
  //Also, pick up that targeted Gateway and make sure there are no duplicates on the same gateway
  TartsGateway* targetGW = NULL;
  for(int i = 0; i < gwObjListCount; i++){
    if(gwObjList[i]->GatewayID == gwid) targetGW = gwObjList[i]; //Pick up the targeted Gateway
    for(int j = 0; j < gwObjList[i]->_senObjListCount; j++){
      if(gwObjList[i]->_senObjList[j]->SensorID == sensor->SensorID){
        if(gwObjList[i]->_senObjList[j] != sensor){  //Duplicate has different address
          LOGEX(9); //"ERROR :: RegisterSensor :: Duplicate ID detected"
          return false;
        }
        else if(targetGW == gwObjList[i]){ //Duplicate is on the same gateway, No error flagged
          LOGEX(10); //"WARN  :: RegisterSensor :: Sensor already registered"
          return true;
        }
        //else: This is OK to have the sensor on another gateway
      }      
    }
  }

  //If the target GW is NULL, then cannot Register to nothing
  if(targetGW == NULL){ 
    LOGEX(11); //"ERROR :: RegisterSensor :: Invalid gateway ID"
    return false;
  }
   
  //Sensor is able to register!!!
  targetGW->_senObjListCount++;
  TartsSensorBase** newList = (TartsSensorBase**) realloc((void*)targetGW->_senObjList, sizeof(TartsSensorBase*) * targetGW->_senObjListCount);
	if (newList) {
		targetGW->_senObjList = newList;
    targetGW->_senObjList[targetGW->_senObjListCount-1] = sensor;
    targetGW->_loadNeeded = true;
    LOGSENP(sensor->getSensorID());
		return true;  //All Succeeded 
	}
  else {
    targetGW->_senObjListCount--;
    LOGEX(12); //"ERROR :: RegisterSensor :: Memory Exception!"
    return false;
  }
}

void TartsLib::RemoveSensor(const char* sensorID){
  int k = 0;
  TartsSensorBase** newList;
  TartsSensorBase* senObj = NULL;
  uint32_t senid = Base36ArrayToInt(sensorID);
  //Find all Gateways that can talk to the sensor specified and populate new lists without that sensor
  for(int i = 0; i < gwObjListCount; i++){
    k = 0;
    newList = NULL;
    for(int j = 0; j < gwObjList[i]->_senObjListCount; j++){
      if(gwObjList[i]->_senObjList[j]->SensorID == senid) continue;
      k++; //We are keeping this one
      newList = (TartsSensorBase**) realloc((void*)newList, sizeof(TartsSensorBase*) * k);
      if(newList) newList[k-1] = gwObjList[i]->_senObjList[j];
      else{
        LOGEX(13); //"ERROR :: RemoveSensor :: Memory Exception!"
        return;  //BAD!!! Do not modify anything
      }
    }
    free(gwObjList[i]->_senObjList);
    gwObjList[i]->_senObjList = newList;
    if(gwObjList[i]->_senObjListCount != k){
      //ADD TO DELETED SENSOR LIST (during the application loop)
      uint32_t* rlist = NULL;
      gwObjList[i]->_senObjRemoveListCount++;
      rlist = (uint32_t*) realloc((void*)gwObjList[i]->_senObjRemoveList, sizeof(uint32_t*) * gwObjList[i]->_senObjRemoveListCount);
      if(rlist){
        gwObjList[i]->_senObjRemoveList = rlist;
        rlist[gwObjList[i]->_senObjRemoveListCount-1] = senid;
        gwObjList[i]->_removeNeeded = true;
      }
      else{
        gwObjList[i]->_senObjRemoveListCount--;
        LOGEX(14); //"ERROR :: RemoveSensor :: Memory Exception2!"
      }
    }
    gwObjList[i]->_senObjListCount = k;
  }
  
  if(senObj != NULL){
    if(senObj->_freeOnRemove) delete senObj;
  }        
}

TartsSensorBase* TartsLib::FindSensor(const char* sensorID){
  uint32_t senid = Base36ArrayToInt(sensorID);
  return FindSensorInternal(senid);
}

TartsSensorBase* TartsLib::FindSensorInternal(uint32_t sensorID){
  for(int i = 0; i < gwObjListCount; i++){
    for(int j = 0; j < gwObjList[i]->_senObjListCount; j++){
      if(gwObjList[i]->_senObjList[j]->SensorID == sensorID) return gwObjList[i]->_senObjList[j];
    }
  }
  return NULL;
}


void TartsLib::RegisterEvent_GatewayPersist(GatewayPersistEvent_t function){
  if(function != NULL) GatewayPersistEvent = function;  
}
void TartsLib::RegisterEvent_GatewayMessage(GatewayMessageEvent_t function){
  if(function != NULL) GatewayMessageEvent = function;  
}
void TartsLib::RegisterEvent_SensorPersist(SensorPersistEvent_t function){
  if(function != NULL) SensorPersistEvent = function; 
}
void TartsLib::RegisterEvent_SensorMessage(SensorMessageEvent_t function){
  if(function != NULL) SensorMessageEvent = function; 
}
void TartsLib::RegisterEvent_LogException(LogExceptionEvent_t function){
  if(function != NULL) LogExceptionEvent = function;  
}

void TartsLib::Process(void){
  for(int i = 0; i < gwObjListCount; i++){
    //Handle Inbound Messages!!!
    if(Platform_inboundPacketReady(gwObjList[i]->PinDataReady)){
      GWAPI inmsg = GWAPI(); 
      Platform_retrieveInboundPacket(gwObjList[i]->Address, gwObjList[i]->PinDataReady, inmsg.buffer);
      
      //Verify that the message is valid and process
      if(inmsg.valid()){
        PLATFORM_PRINTLN("-OK");
        uint32_t id = inmsg.extractID();
        gwObjList[i]->_lastTransactionTime = millis();
        
        if (id == 0){ //Process AP TRAFFIC
          if(inmsg.getCommand() == NETWORK_STATUS_MESSAGE){ //Handle Network Status Messages
            uint32_t gwid = ((uint32_t)inmsg.buffer[7] << 24) | ((uint32_t)inmsg.buffer[6] << 16) | ((uint32_t)inmsg.buffer[5] << 8) | (uint32_t)inmsg.buffer[4];
            if(gwObjList[i]->GatewayID != gwid){
              gwObjList[i]->_lastUnknownID = gwid;
              LOGGWM(gwObjList[i]->getGatewayID(),2); //"Unexpected gateway ID detected!"
              gwObjList[i]->_state = OFF; //Finish recovering
              gwObjList[i]->_lastTransactionTime = 0;
            }
            gwObjList[i]->_sensorCount = (uint16_t)((inmsg.buffer[9] << 8) | inmsg.buffer[8]);
            gwObjList[i]->_wirelessState = (GatewayState)inmsg.buffer[12];
            if(inmsg.buffer[11] == 0) gwObjList[i]->_channel = 0xFF;
            else gwObjList[i]->_channel = inmsg.buffer[10];
            gwObjList[i]->_netStatsRXD = true;
          }
          else if(inmsg.getCommand() == SENSOR_STATUS_INTICATOR){
            uint32_t id =  ((uint32_t)inmsg.buffer[7] << 24) | ((uint32_t)inmsg.buffer[6] << 16) | ((uint32_t)inmsg.buffer[5] << 8) | (uint32_t)inmsg.buffer[4];
            //if(inmsg.buffer[10] == 0){} //JOINED
            //else if(inmsg.buffer[10] == 2) {} //QUEUE SET CORRECTLY
            if(inmsg.buffer[10] == 1) { //NOT ALLOWED TO JOIN
              gwObjList[i]->_lastUnknownID = id;
              gwObjList[i]->_lastUnknownSensorType = (((uint16_t)inmsg.buffer[9]) << 8) | (uint16_t)inmsg.buffer[8] ;
              LOGGWM(gwObjList[i]->getGatewayID(),3); //"Unregistered sensor traffic detected!"
            }
            else if(inmsg.buffer[10] == 2){ //QUEUE SET CORRECTLY
              TartsSensorBase* senObj = FindSensorInternal(id);
              if(senObj != NULL) senObj->_queueRequired = false;
              gwObjList[i]->_queuePending = false;
              gwObjList[i]->_errors = 0;
            }
            else if(inmsg.buffer[10] == 3) { //Message not Handled
              TartsGateway_sendQueuedNotfication(gwObjList[i]->Address, id, 1);
              LOGEX(15); //"WARN  :: Process :: Requested sensor ID not recognized");
            }
          }
          //inmsg.clear(); //Do not preserve / interpret any other any other AP messages
        }
        else if(gwObjList[i]->_state == ACTIVE) {         //Process SENSOR TRAFFIC when Active
          TartsSensorBase* senObj = FindSensorInternal(id);
          if(senObj == NULL){
            if((inmsg.getCommand() == DATA_MESSAGE) || (inmsg.getCommand() == DATA_MESSAGE_DL)){
              gwObjList[i]->_lastUnknownID = id;
              gwObjList[i]->_lastUnknownSensorType = (((uint16_t)inmsg.buffer[12]) << 8) | (uint16_t)inmsg.buffer[11];
              LOGGWM(gwObjList[i]->getGatewayID(),3); //"Unregistered sensor traffic detected!"
            }
          }
          else{ //Good sensor object
            //Handle Pending Configs
            if(senObj->pendingActions()){
              //HANDLE INBOUND IF ACK-2-COMMAND
              if(inmsg.getCommand() == READ_DATASECTOR_RESPONSE){
                if(inmsg.buffer[8] == 24) senObj->_parseGeneralConfig1(inmsg.buffer[9], &inmsg.buffer[10]);
                else if(inmsg.buffer[8] == 25) senObj->_parseGeneralConfig2(inmsg.buffer[9], &inmsg.buffer[10]);
                else if(inmsg.buffer[8] == 28) senObj->_parseProfileConfig1(inmsg.buffer[9], &inmsg.buffer[10]);
                else if(inmsg.buffer[8] == 29) senObj->_parseProfileConfig2(inmsg.buffer[9], &inmsg.buffer[10]);
              }
              else if(inmsg.getCommand() == APPCMD_RESPONSE) senObj->_parseAppCommand(&inmsg.buffer[8]);
              else if(inmsg.getCommand() == WRITE_DATASECTOR_RESPONSE){
                if(inmsg.buffer[8] == 24) senObj->_dirtyConfig1 = false;
                else if(inmsg.buffer[8] == 25) senObj->_dirtyConfig2 = false;
                else if(inmsg.buffer[8] == 28) senObj->_dirtyConfig3 = false;
                else if(inmsg.buffer[8] == 29) senObj->_dirtyConfig4 = false;
              }
              
              //Send Outbound Write Packets
              if(senObj->_dirtyConfig1){
                uint8_t cpg1[16];
                senObj->_getGeneralConfig1(cpg1);
                TartsGateway_sendConfigUpdate(gwObjList[i]->Address, id, 24, cpg1);
              }
              else if(senObj->_dirtyConfig2){
                uint8_t cpg2[16];
                senObj->_getGeneralConfig2(cpg2);
                TartsGateway_sendConfigUpdate(gwObjList[i]->Address, id, 25, cpg2);
              }
              else if(senObj->_dirtyConfig3){
                uint8_t cpg3[16];
                senObj->_getProfileConfig1(cpg3);
                TartsGateway_sendConfigUpdate(gwObjList[i]->Address, id, 28, cpg3);
              }
              else if(senObj->_dirtyConfig4){
                uint8_t cpg4[16];
                senObj->_getProfileConfig2(cpg4);
                TartsGateway_sendConfigUpdate(gwObjList[i]->Address, id, 29, cpg4);
              }
              else if(senObj->_readConfig1) TartsGateway_sendConfigRead(gwObjList[i]->Address, id, 24);
              else if(senObj->_readConfig2) TartsGateway_sendConfigRead(gwObjList[i]->Address, id, 25);
              else if(senObj->_readConfig3) TartsGateway_sendConfigRead(gwObjList[i]->Address, id, 28);
              else if(senObj->_readConfig4) TartsGateway_sendConfigRead(gwObjList[i]->Address, id, 29);
              else if(senObj->_appCommandPending){
                uint8_t appcmd[32];
                uint8_t len;
                bool isUrgent = senObj->_getAppCommand(appcmd, &len);
                if(len != 0) TartsGateway_sendApplicationCommand(gwObjList[i]->Address, id, appcmd, len, isUrgent);
              }
              
              if(!senObj->pendingActions()){
                TartsGateway_sendQueuedNotfication(gwObjList[i]->Address, id, 0); 
                LOGSENP(IntToBase36Array(id));
              }
            }
        
            //Handle DataMessages
            if(inmsg.getCommand() == DATA_MESSAGE){
              //RSSI: inmsg.buffer[9]
              //Battery: ((float) (((uint16_t)inmsg.buffer[10])+150)/100.0);
              SensorMessage sensorMessage = SensorMessage(senObj->getSensorID(), (int8_t)inmsg.buffer[9], (int16_t)inmsg.buffer[10] + 150, NULL);
              uint16_t type = (uint16_t)((inmsg.buffer[12] << 8) | inmsg.buffer[11]);
              if((senObj->SensorType != type) && (type != 0xFFFF)) LOGEX(16); //"WARN  :: Sensor type mismatch!"
              if(GatewayMessageEvent != NULL) senObj->_parseData(SensorMessageEvent, &sensorMessage, &inmsg.buffer[13]); //Start at State!
            }
            else if(inmsg.getCommand() == DATA_MESSAGE_DL){
              //RSSI: inmsg.buffer[13]
              //Battery: ((float) (((uint16_t)inmsg.buffer[14])+150)/100.0);
              SensorMessage sensorMessage = SensorMessage(senObj->getSensorID(), (int8_t)inmsg.buffer[13], (int16_t)inmsg.buffer[14] + 150, NULL);
              uint16_t type = (uint16_t)((inmsg.buffer[16] << 8) | inmsg.buffer[15]);
              if((senObj->SensorType != type) && (type != 0xFFFF)) LOGEX(16); //"WARN  :: Sensor type mismatch!"
              if(GatewayMessageEvent != NULL) senObj->_parseData(SensorMessageEvent, &sensorMessage, &inmsg.buffer[17]); //Start at State!
            }
          }
        }          
      }
      else {
        PLATFORM_PRINTLN("-ERR");
        gwObjList[i]->_lastTransactionTime = millis();
      }
    }
    
    
    //=================================================================================================================
    //STATE MANAGEMENT
    //=================================================================================================================
    switch(gwObjList[i]->_state){
      case UNINITALIZED:
        gwObjList[i]->_state = INITALIZED;
        gwObjList[i]->_lastTransactionTime = millis(); 
        break;
      case OFF:
        if(gwObjList[i]->_lastTransactionTime == 0){
          LOGGWM(gwObjList[i]->getGatewayID(),4); //"STATE::OFF"
          gwObjList[i]->_lastTransactionTime = millis();
          #ifdef BB_BLACK_ARCH
          Platform_gatewayInitialize(gwObjList[i]->UartNum, gwObjList[i]->PinActivity, gwObjList[i]->PinPCTS, gwObjList[i]->PinPRTS, gwObjList[i]->PinNRST);
          #else  
          Platform_gatewayInitialize(gwObjList[i]->Address, gwObjList[i]->PinReset, gwObjList[i]->PinDataReady);
          #endif
          gwObjList[i]->_errors = 0;
        }
        else if ((gwObjList[i]->_lastTransactionTime + 2000) < millis()){
          gwObjList[i]->_state = INITALIZED; //Finish recovering
          gwObjList[i]->_lastTransactionTime = millis();
        }
        break;
      case INITALIZED:
        gwObjList[i]->_netStatsRXD = false;
        gwObjList[i]->_state = STARTING;
        TartsGateway_setGatewayIdle(gwObjList[i]->Address);    //Send first communication
        if((gwObjList[i]->_lastTransactionTime + 500) > millis()) LOGGWM(gwObjList[i]->getGatewayID(),5); //"STATE::STARTING"
        break;
      case STARTING:
        if(gwObjList[i]->_netStatsRXD){
          gwObjList[i]->_netStatsRXD = false;
          gwObjList[i]->_errors = 0;
          if((gwObjList[i]->_channel == 0xFF) || (gwObjList[i]->_reformNetworkNeeded)){
            gwObjList[i]->_state = REFORMING;
            LOGGWM(gwObjList[i]->getGatewayID(),6); //"STATE::REFORMING"
            TartsGateway_sendReformNetwork(gwObjList[i]->Address, gwObjList[i]->ChannelMask);
          }
          else if(gwObjList[i]->_removeNeeded){
            gwObjList[i]->_state = REMOVING;
            LOGGWM(gwObjList[i]->getGatewayID(),7); //"STATE::REMOVING"
            TartsGateway_sendRemoveSensor(gwObjList[i]->Address, gwObjList[i]->_senObjRemoveList[gwObjList[i]->_senObjRemoveListCount-1]);
          }
          else if(gwObjList[i]->_senObjListCount != 0){
            gwObjList[i]->_state = LOADING;
            LOGGWM(gwObjList[i]->getGatewayID(),8); //"STATE::LOADING"
            gwObjList[i]->_senObjProcessListCount = gwObjList[i]->_senObjListCount;  
            TartsGateway_sendAssignSensor(gwObjList[i]->Address, gwObjList[i]->_senObjList[gwObjList[i]->_senObjProcessListCount-1]->SensorID);
          }
          else{
            gwObjList[i]->_state = ACTIVATING;
            LOGGWM(gwObjList[i]->getGatewayID(),9); //"STATE::ACTIVATING"
            TartsGateway_setGatewayActive(gwObjList[i]->Address);
          }
        }
        else if ((gwObjList[i]->_lastTransactionTime + 3000) < millis()){
          gwObjList[i]->_state = OFF;
          gwObjList[i]->_lastTransactionTime = 0;
        }
        else if ( ((gwObjList[i]->_lastTransactionTime + 1000) < millis()) && (gwObjList[i]->_errors == 0) ){ //Retry
          gwObjList[i]->_errors++;
          gwObjList[i]->_state = INITALIZED;
        }
        else if ( ((gwObjList[i]->_lastTransactionTime + 2000) < millis()) && (gwObjList[i]->_errors == 1) ){ //Retry
          gwObjList[i]->_errors++;
          gwObjList[i]->_state = INITALIZED;
        }
        break;
      case REFORMING:
        if((gwObjList[i]->_netStatsRXD) && (gwObjList[i]->_wirelessState == 1)){
          gwObjList[i]->_firstActive = true;
          gwObjList[i]->_reformNetworkNeeded = false;
          gwObjList[i]->_netStatsRXD = false;
          gwObjList[i]->_removeNeeded = false;
          if(gwObjList[i]->_senObjRemoveListCount != 0) free(gwObjList[i]->_senObjRemoveList);
          gwObjList[i]->_senObjRemoveListCount = 0;
          if(gwObjList[i]->_senObjListCount != 0){
            gwObjList[i]->_state = LOADING;
            LOGGWM(gwObjList[i]->getGatewayID(),8); //"STATE::LOADING"
            gwObjList[i]->_senObjProcessListCount = gwObjList[i]->_senObjListCount;  
            TartsGateway_sendAssignSensor(gwObjList[i]->Address, gwObjList[i]->_senObjList[gwObjList[i]->_senObjProcessListCount-1]->SensorID);
          }
          else{
            gwObjList[i]->_state = ACTIVATING;
            gwObjList[i]->_errors = 0;
            LOGGWM(gwObjList[i]->getGatewayID(),9); //"STATE::ACTIVATING"
            TartsGateway_setGatewayActive(gwObjList[i]->Address);
          }
        }
        else if ((gwObjList[i]->_lastTransactionTime + 20000) < millis()){
          gwObjList[i]->_state = OFF;
          gwObjList[i]->_lastTransactionTime = 0;
        }
        break;
      case REMOVING:
        if(gwObjList[i]->_netStatsRXD){
          gwObjList[i]->_netStatsRXD = false;
          gwObjList[i]->_errors = 0;
          gwObjList[i]->_senObjRemoveListCount--;
          if(gwObjList[i]->_senObjRemoveListCount != 0){
            TartsGateway_sendRemoveSensor(gwObjList[i]->Address, gwObjList[i]->_senObjRemoveList[gwObjList[i]->_senObjRemoveListCount-1]);
          }
          else{
            free(gwObjList[i]->_senObjRemoveList);
            gwObjList[i]->_removeNeeded = false;
            if(gwObjList[i]->_loadNeeded){
              gwObjList[i]->_state = LOADING;
              LOGGWM(gwObjList[i]->getGatewayID(),8); //"STATE::LOADING"
              gwObjList[i]->_senObjProcessListCount = gwObjList[i]->_senObjListCount;  
              TartsGateway_sendAssignSensor(gwObjList[i]->Address, gwObjList[i]->_senObjList[gwObjList[i]->_senObjProcessListCount-1]->SensorID);
            }
            else{
              gwObjList[i]->_state = ACTIVATING;
              LOGGWM(gwObjList[i]->getGatewayID(),9); //"STATE::ACTIVATING"
              TartsGateway_setGatewayActive(gwObjList[i]->Address);
            }
          }
        }
        else if ((gwObjList[i]->_lastTransactionTime + 3000) < millis()){
          gwObjList[i]->_state = OFF;
          gwObjList[i]->_lastTransactionTime = 0;
        }
        else if ( ((gwObjList[i]->_lastTransactionTime + 1000) < millis()) && (gwObjList[i]->_errors == 0) ){ //Retry
          gwObjList[i]->_errors++;
          gwObjList[i]->_netStatsRXD = false;
          TartsGateway_sendRemoveSensor(gwObjList[i]->Address, gwObjList[i]->_senObjRemoveList[gwObjList[i]->_senObjRemoveListCount-1]);
        }
        else if ( ((gwObjList[i]->_lastTransactionTime + 2000) < millis()) && (gwObjList[i]->_errors == 1) ){ //Retry
          gwObjList[i]->_errors++;
          gwObjList[i]->_netStatsRXD = false;
          TartsGateway_sendRemoveSensor(gwObjList[i]->Address, gwObjList[i]->_senObjRemoveList[gwObjList[i]->_senObjRemoveListCount-1]);
        }
        break;
      case LOADING:
        if(gwObjList[i]->_netStatsRXD){
          gwObjList[i]->_netStatsRXD = false;
          gwObjList[i]->_errors = 0;
          gwObjList[i]->_senObjProcessListCount--;
          if(gwObjList[i]->_senObjProcessListCount != 0){ 
            TartsGateway_sendAssignSensor(gwObjList[i]->Address, gwObjList[i]->_senObjList[gwObjList[i]->_senObjProcessListCount-1]->SensorID);
          }
          else{
            gwObjList[i]->_loadNeeded = false;            
            gwObjList[i]->_state = ACTIVATING;
            TartsGateway_setGatewayActive(gwObjList[i]->Address);
            LOGGWM(gwObjList[i]->getGatewayID(),9); //"STATE::ACTIVATING"
          }
        }
        else if ((gwObjList[i]->_lastTransactionTime + 3000) < millis()){
          gwObjList[i]->_state = OFF;
          gwObjList[i]->_lastTransactionTime = 0;
        }
        else if ( ((gwObjList[i]->_lastTransactionTime + 1000) < millis()) && (gwObjList[i]->_errors == 0) ){ //Retry
          gwObjList[i]->_errors++;
          gwObjList[i]->_netStatsRXD = false;
          TartsGateway_sendAssignSensor(gwObjList[i]->Address, gwObjList[i]->_senObjList[gwObjList[i]->_senObjProcessListCount-1]->SensorID);
        }
        else if ( ((gwObjList[i]->_lastTransactionTime + 2000) < millis()) && (gwObjList[i]->_errors == 1) ){ //Retry
          gwObjList[i]->_errors++;
          gwObjList[i]->_netStatsRXD = false;
          TartsGateway_sendAssignSensor(gwObjList[i]->Address, gwObjList[i]->_senObjList[gwObjList[i]->_senObjProcessListCount-1]->SensorID);
        }
        break;
      case ACTIVATING:
        //if((gwObjList[i]->_netStatsRXD) && (gwObjList[i]->_wirelessState == 1)){
        if(gwObjList[i]->_wirelessState == 1){
          gwObjList[i]->_state = ACTIVE;
          gwObjList[i]->_errors = 0;
          LOGGWM(gwObjList[i]->getGatewayID(),10); //"STATE::ACTIVE"
        }
        else if ((gwObjList[i]->_lastTransactionTime + 3000) < millis()){
          gwObjList[i]->_state = OFF;
          gwObjList[i]->_lastTransactionTime = 0;
        }
        else if ( ((gwObjList[i]->_lastTransactionTime + 1000) < millis()) && (gwObjList[i]->_errors == 0) ){ //Retry
          gwObjList[i]->_errors++;
          TartsGateway_setGatewayActive(gwObjList[i]->Address);
        }
        else if ( ((gwObjList[i]->_lastTransactionTime + 2000) < millis()) && (gwObjList[i]->_errors == 1) ){ //Retry
          gwObjList[i]->_errors++;
          TartsGateway_setGatewayActive(gwObjList[i]->Address);
        }
        break;
      case ACTIVE:
        //Handle first active state
        if(gwObjList[i]->_firstActive){
          gwObjList[i]->_firstActive = false;
          LOGGWP(gwObjList[i]->getGatewayID());
        }
        //Detect if there is a reform request pending
        else if(gwObjList[i]->_reformNetworkNeeded){
          gwObjList[i]->_reformNetworkNeeded = false;
          gwObjList[i]->_netStatsRXD = false;
          gwObjList[i]->_lastTransactionTime = millis();
          gwObjList[i]->_state = REFORMING;
          LOGGWM(gwObjList[i]->getGatewayID(),6); //"STATE::REFORMING"
          TartsGateway_sendReformNetwork(gwObjList[i]->Address,gwObjList[i]->ChannelMask);
        }
        //Look for any Remove Sensor Requests
        else if(gwObjList[i]->_removeNeeded){
          gwObjList[i]->_state = REMOVING;
          LOGGWM(gwObjList[i]->getGatewayID(),7); //"STATE::REMOVING"
          TartsGateway_sendRemoveSensor(gwObjList[i]->Address, gwObjList[i]->_senObjRemoveList[gwObjList[i]->_senObjRemoveListCount-1]);
        }
        //Look for any Load Sensor Requests
        else if(gwObjList[i]->_loadNeeded){
          gwObjList[i]->_state = LOADING;
          LOGGWM(gwObjList[i]->getGatewayID(),8); //"STATE::LOADING"
          gwObjList[i]->_senObjProcessListCount = gwObjList[i]->_senObjListCount;  
          TartsGateway_sendAssignSensor(gwObjList[i]->Address, gwObjList[i]->_senObjList[gwObjList[i]->_senObjProcessListCount-1]->SensorID);
        }  
        //Look for Idle Gateway and confirm everything is working.
        else if ((gwObjList[i]->_lastTransactionTime + 300000) < millis()){ //300 seconds == 5 mins
          gwObjList[i]->_wirelessState = 0;
          gwObjList[i]->_netStatsRXD = false;
          gwObjList[i]->_errors = 0;
          gwObjList[i]->_lastTransactionTime = millis();
          gwObjList[i]->_state = ACTIVATING; //Silently do this, so no LOGGWM
          TartsGateway_setGatewayActive(gwObjList[i]->Address);
        }
        //NOW SEE IF THERE IS ANY QUEUEING THAT NEEDS TO BE DONE (Per sensor)
        else if(gwObjList[i]->_queuePending){
          if((gwObjList[i]->_lastTransactionTime + 1000) < millis()){
            gwObjList[i]->_queuePending = false;
            gwObjList[i]->_errors++;
          }
        }
        else if(gwObjList[i]->_errors > 4){
          gwObjList[i]->_state = OFF;
          gwObjList[i]->_lastTransactionTime = 0;
        }
        else{
          for(int j=0; j< gwObjList[i]->_senObjListCount; j++){
            if(gwObjList[i]->_senObjList[j]->_queueRequired){
              gwObjList[i]->_queuePending = true;
              gwObjList[i]->_lastTransactionTime = millis();
              TartsGateway_sendQueuedNotfication(gwObjList[i]->Address, gwObjList[i]->_senObjList[j]->SensorID, 1);
              break; //Do not allow two of these messages at a time
            }
            if((gwObjList[i]->_senObjList[j]->_appCommandPending) && (gwObjList[i]->_senObjList[j]->_nextAppCommandSendTime <= millis())){
              //Time to push something out.
              gwObjList[i]->_senObjList[j]->_appCommandRetryCount++;
              if(gwObjList[i]->_senObjList[j]->_appCommandRetryCount <= 2) gwObjList[i]->_senObjList[j]->_nextAppCommandSendTime = millis() + 1000;
              else if(gwObjList[i]->_senObjList[j]->_appCommandRetryCount == 3) gwObjList[i]->_senObjList[j]->_nextAppCommandSendTime = millis() + 3000;
              else if(gwObjList[i]->_senObjList[j]->_appCommandRetryCount <= 5) gwObjList[i]->_senObjList[j]->_nextAppCommandSendTime = millis() + 5000;
              else gwObjList[i]->_senObjList[j]->_nextAppCommandSendTime = 157680000;  //5 years into the future!!!            
              
              uint8_t appcmd[32];
              uint8_t len;
              bool isUrgent = gwObjList[i]->_senObjList[j]->_getAppCommand(appcmd, &len);
              if(len != 0) TartsGateway_sendApplicationCommand(gwObjList[i]->Address, gwObjList[i]->_senObjList[j]->SensorID, appcmd, len, isUrgent);
              break; //Do not allow two of these messages at a time
            }
          } 
        }
        break;
      default:
        LOGEX(17); //"ERROR :: Process :: Gateway in unknown state"
    }
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//Tartslib Class Persistent object
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsLib Tarts = TartsLib();

