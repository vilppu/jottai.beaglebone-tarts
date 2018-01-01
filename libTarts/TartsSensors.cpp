/**********************************************************************************
 * TartsSensors.cpp :: Sensor Specific Implementation source file.                *
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



#define DEFAULT_PROFILE_INT	1
#define DEFAULT_PROFILE_TRG	2

//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//Sensor Base Class Implementation
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------

TartsSensorBase::~TartsSensorBase(){}

TartsSensorBase::TartsSensorBase(const char * sensorID, TartsSensorTypes type, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){
  SensorID = Base36ArrayToInt(sensorID);
  SensorType = type;
  ReportInterval = reportInterval;
  LinkInterval = linkInterval;
  RetryCount = retryCount;
  Recovery = recovery;
  _dirtyConfig1 = false;
  _dirtyConfig2 = false;
  _dirtyConfig3 = false;
  _dirtyConfig4 = false;
  _readConfig1  = false;
  _readConfig2  = false;
  _readConfig3  = false;
  _readConfig4  = false;
  _appCommandPending = false;
  _queueRequired = false;
  _freeOnRemove = isHeapStackVarNotGlobal((int)this);   //Decide on how to dispose of this 
}

//Parameters to sets
void TartsSensorBase::setReportInterval(uint16_t reportInterval){
  ReportInterval = reportInterval;
  _dirtyConfig1 = true;
  _readConfig1  = false;
  _queueRequired = true;
}
void TartsSensorBase::setLinkInterval(uint8_t linkInterval){
  LinkInterval = linkInterval;
  _dirtyConfig1 = true;
  _readConfig1  = false;
  _queueRequired = true;
}
void TartsSensorBase::setRetryCount(uint16_t retryCount){
  RetryCount = retryCount;
  _dirtyConfig2 = true;
  _readConfig2  = false;
  _queueRequired = true;
}
void TartsSensorBase::setRecovery(uint16_t recovery){
  Recovery = recovery;
  _dirtyConfig2 = true;
  _readConfig2  = false;
  _queueRequired = true;
}

//Parameters to get
const char * TartsSensorBase::getSensorID(){ return IntToBase36Array(SensorID); }
TartsSensorTypes TartsSensorBase::getSensorType(){ return SensorType; }
uint16_t TartsSensorBase::getReportInterval(){ return ReportInterval; }
uint8_t  TartsSensorBase::getLinkInterval(){ return LinkInterval; }
uint8_t  TartsSensorBase::getRetryCount(){ return RetryCount; }
uint8_t  TartsSensorBase::getRecovery(){ return Recovery; }

//Things to do
void TartsSensorBase::requestConfigurations(){
  _readConfig1 = true;
  _readConfig2 = true;
  _readConfig3 = true;
  _readConfig4 = true;
  _dirtyConfig1 = false;
  _dirtyConfig2 = false;
  _dirtyConfig3 = false;
  _dirtyConfig4 = false;
  _queueRequired = true;
}
bool TartsSensorBase::pendingActions(){
  return (_dirtyConfig1 || _dirtyConfig2 || _dirtyConfig3 || _dirtyConfig4 || _readConfig1 || _readConfig2 || _readConfig3 || _readConfig4 || _appCommandPending) ? true : false;
}

//Defaulted functions for Configurations Pages
void TartsSensorBase::_parseGeneralConfig1(uint8_t status, uint8_t* page){ //Success = 0, then page[16]
  _readConfig1 = false;
  if(status == 0){
    ReportInterval = (((uint16_t)page[11]) << 8) | ((uint16_t)page[10]);
    LinkInterval = page[12];
  }
}
void TartsSensorBase::_parseGeneralConfig2(uint8_t status, uint8_t* page){
  _readConfig2 = false;
  if(status == 0){
    RetryCount = page[0];
    Recovery = page[1];
  }
}
void TartsSensorBase::_parseProfileConfig1(uint8_t status, uint8_t* page){
  _readConfig3 = false;
}
void TartsSensorBase::_parseProfileConfig2(uint8_t status, uint8_t* page){
  _readConfig4 = false;
}

void TartsSensorBase::_getGeneralConfig1(uint8_t* page){
  page[0] = 0xFF; page[1] = 0xFF; page[2] = 0xFF; page[3] = 0xFF;
  page[4] = (uint8_t)SensorType; page[5] = (uint8_t)(SensorType>>8); page[6] = 50; page[7] = 0;
  page[8] = (uint8_t)ReportInterval; page[9] = (uint8_t)(ReportInterval>>8); page[10] = (uint8_t)ReportInterval; page[11] = (uint8_t)(ReportInterval>>8);
  page[12] = LinkInterval; page[13] = 126; page[14] = 1; page[15] = 1;
}
void TartsSensorBase::_getGeneralConfig2(uint8_t* page){
  page[0] = RetryCount; page[1] = Recovery; page[2] = 0xFF; page[3] = 0xFF;
  page[4] = 0xFF; page[5] = 0xFF; page[6] = 0xFF; page[7] = 0xFF;
  page[8] = 0xFF; page[9] = 0xFF; page[10] = 0xFF; page[11] = 0xFF;
  page[12] = 0xFF; page[13] = 0xFF; page[14] = 0xFF; page[15] = 0xFF;
}
void TartsSensorBase::_getProfileConfig1(uint8_t * page){
  //default page for INT type
  if(_profileType == DEFAULT_PROFILE_INT){ 
    page[0] = DEFAULT_PROFILE_INT;
    page[1] = 1;
    page[2] = 0; page[3] = 0;
    page[4] = 0; page[5] = 0; page[6] = 0; page[7] = 0;
    page[8] = 255; page[9] = 255; page[10] = 255; page[11] = 255;
    page[12] = 255; page[13] = 255; page[14] = 255; page[15] = 255;
  }
  else{ //Trigger
    page[0] = DEFAULT_PROFILE_TRG;
    page[1] = 0;
    page[2] = 50; page[3] = 0;
    page[4] = 6; page[5] = 0;
    page[6] = 1; page[7] = 0;
    page[8] = 1;
    page[9] = 0; page[10] = 0; page[11] = 0; page[12] = 0; page[13] = 0; page[14] = 0; page[15] = 0;
  }
}
void TartsSensorBase::_getProfileConfig2(uint8_t * page){
  memset((void*)page, 0xFF, 16);
}

//Empty functions for Application Commands
bool TartsSensorBase::_getAppCommand(uint8_t* data, uint8_t* len){
  _appCommandPending = false;
  *len = 0;
  return false;
}
void TartsSensorBase::_parseAppCommand(uint8_t* data){
  _appCommandPending = false;
}


//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//Tarts Sensor Class Implementation
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------------

//All devices will have 3 Datums:  NAME, VALUE, INTERPRETATION OR FORMATTED VALUE


//-----------------------------------------------------------------------------------------------------------------------------------------------
//Tarts Temperature Sensor Class Implementation
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsTemperature::~TartsTemperature() {}
TartsTemperature::TartsTemperature(const char * sensorID) : TartsSensorBase(sensorID, Temperature, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsTemperature::TartsTemperature(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, Temperature, reportInterval, linkInterval, retryCount, recovery) { _profileType = DEFAULT_PROFILE_INT; }
TartsTemperature* TartsTemperature::Create(const char * sensorID){ return new TartsTemperature(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsTemperature* TartsTemperature::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){  return new TartsTemperature(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsTemperature::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  char value[16];
  char valueF[16];

  int16_t temp10 = ((int16_t)data[2] << 8) | (int16_t)data[1];
  sprintf(value, "%d", temp10);
  if((data[0] & 0xF0) == 0x20) strncpy(valueF, "ERROR", sizeof(valueF));
  else{
	char isNegative[2];
    int16_t dec = temp10%10;
	if((temp10 < 0) && (temp10 > -10)){
		isNegative[0] = '-'; 
		isNegative[1] = 0;
	}
	else isNegative[0] = 0;
    if(dec < 0) dec *= -1;
    sprintf(valueF, "%s%d.%d C", isNegative, temp10/10, dec);
  }
  
  Datum dlist[1] = { Datum((char*)"TEMPERATURE", value, valueF) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS WATER TEMPERATURE SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsWaterTemperature::~TartsWaterTemperature() {}
TartsWaterTemperature::TartsWaterTemperature(const char * sensorID) : TartsSensorBase(sensorID, WaterTemperature, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsWaterTemperature::TartsWaterTemperature(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, WaterTemperature, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsWaterTemperature* TartsWaterTemperature::Create(const char * sensorID){  return new TartsWaterTemperature(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsWaterTemperature* TartsWaterTemperature::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){  return new TartsWaterTemperature(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsWaterTemperature::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  char value[16];
  char valueF[16];

  int16_t temp10 = ((int16_t)data[2] << 8) | (int16_t)data[1];
  sprintf(value, "%d", temp10); 
  if((data[0] & 0xF0) == 0x20) strncpy(valueF, "ERROR", sizeof(valueF));
  else{
    char isNegative[2];
    int16_t dec = temp10%10;
    if((temp10 < 0) && (temp10 > -10)){
      isNegative[0] = '-';
      isNegative[1] = 0;
    }
    else isNegative[0] = 0;
    if(dec < 0) dec *= -1;
    sprintf(valueF, "%s%d.%d C", isNegative, temp10/10, dec);
  }
  
  Datum dlist[1] = { Datum((char*)"TEMPERATURE", value, valueF) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS HUMIDITY SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsHumidity::~TartsHumidity() {}
TartsHumidity::TartsHumidity(const char * sensorID) : TartsSensorBase(sensorID, Humidity, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsHumidity::TartsHumidity(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, Humidity, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsHumidity* TartsHumidity::Create(const char * sensorID){ return new TartsHumidity(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsHumidity* TartsHumidity::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsHumidity(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsHumidity::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  char value1[16];
  char valueF1[16];
  char value2[16];
  char valueF2[16];

  int16_t temp100 = ((int16_t)data[2] << 8) | (int16_t)data[1];
  int16_t rh100 = ((int16_t)data[4] << 8) | (int16_t)data[3];
  sprintf(value1, "%d", rh100);
  sprintf(value2, "%d", temp100);
  if((data[0] & 0xF0) == 0x20){
    strncpy(valueF1, "ERROR", sizeof(valueF1));
    strncpy(valueF2, "ERROR", sizeof(valueF2));
  }
  else{
    char isNegative[2];
    int16_t dec = temp100%100;
    if((temp100 < 0) && (temp100 > -100)){
      isNegative[0] = '-';
      isNegative[1] = 0;
    }
    else isNegative[0] = 0;
    if(dec < 0) dec *= -1;
    sprintf(valueF1, "%d.%d %%", rh100/100, rh100%100);
    sprintf(valueF2, "%s%d.%d C", isNegative, temp100/100, dec);
  }
  
  Datum dlist[2] = { Datum((char*)"RH", value1, valueF1), Datum((char*)"TEMPERATURE", value2, valueF2) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 2;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS DRY CONTACT SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsDryContact::~TartsDryContact() {}
TartsDryContact::TartsDryContact(const char * sensorID) : TartsSensorBase(sensorID, DryContact, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsDryContact::TartsDryContact(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, DryContact, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsDryContact* TartsDryContact::Create(const char * sensorID){ return new TartsDryContact(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsDryContact* TartsDryContact::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsDryContact(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsDryContact::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  Datum dlist[1] = { Datum((char*)"CONTACT", (char*)((data[1] == 0) ? "0" : "1") , (char*)((data[1] == 0) ? "OPEN" : "CLOSED")) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS WATER DETECTION SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsWaterDetect::~TartsWaterDetect() {}
TartsWaterDetect::TartsWaterDetect(const char * sensorID) : TartsSensorBase(sensorID, WaterDetect, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsWaterDetect::TartsWaterDetect(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, WaterDetect, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsWaterDetect* TartsWaterDetect::Create(const char * sensorID){  return new TartsWaterDetect(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsWaterDetect* TartsWaterDetect::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){  return new TartsWaterDetect(sensorID, reportInterval, linkInterval, retryCount, recovery);}
void TartsWaterDetect::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  Datum dlist[1] = { Datum((char*)"DETECT", (char*)((data[1] == 0) ? "0" : "1") , (char*)((data[1] == 0) ? "NOT PRESENT" : "PRESENT")) }; 
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS WATER ROPE DETECTION SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsWaterRope::~TartsWaterRope() {}
TartsWaterRope::TartsWaterRope(const char * sensorID) : TartsSensorBase(sensorID, WaterDetect, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsWaterRope::TartsWaterRope(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, WaterDetect, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsWaterRope* TartsWaterRope::Create(const char * sensorID){  return new TartsWaterRope(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsWaterRope* TartsWaterRope::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){  return new TartsWaterRope(sensorID, reportInterval, linkInterval, retryCount, recovery);}
void TartsWaterRope::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  Datum dlist[1] = { Datum((char*)"DETECT", (char*)((data[1] == 0) ? "0" : "1") , (char*)((data[1] == 0) ? "NOT PRESENT" : "PRESENT")) }; 
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS OPEN CLOSE SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsOpenClose::~TartsOpenClose() {}
TartsOpenClose::TartsOpenClose(const char * sensorID) : TartsSensorBase(sensorID, OpenClose, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsOpenClose::TartsOpenClose(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, OpenClose, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsOpenClose* TartsOpenClose::Create(const char * sensorID){ return new TartsOpenClose(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsOpenClose* TartsOpenClose::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsOpenClose(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsOpenClose::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  Datum dlist[1] = { Datum((char*)"CONTACT", (char*)((data[1] == 0) ? "0" : "1") , (char*)((data[1] == 0) ? "OPEN" : "CLOSED")) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS BUTTON SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsButton::~TartsButton() {}
TartsButton::TartsButton(const char * sensorID) : TartsSensorBase(sensorID, Button, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsButton::TartsButton(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, Button, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsButton* TartsButton::Create(const char * sensorID){ return new TartsButton(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsButton* TartsButton::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsButton(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsButton::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  Datum dlist[1] = { Datum((char*)"BUTTON", (char*)((data[1] == 0) ? "0" : "1") , (char*)((data[1] == 0) ? "NOT PRESSED" : "PRESSED")) }; 
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS ASSET SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsAsset::~TartsAsset() {}
TartsAsset::TartsAsset(const char * sensorID) : TartsSensorBase(sensorID, Asset, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsAsset::TartsAsset(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, Asset, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsAsset* TartsAsset::Create(const char * sensorID){ return new TartsAsset(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsAsset* TartsAsset::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsAsset(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsAsset::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  Datum dlist[1] = { Datum((char*)"ASSET", (char*)"", (char*)"PRESENT") }; 
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS PASSIVE IR SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsPassiveIR::~TartsPassiveIR() {}
TartsPassiveIR::TartsPassiveIR(const char * sensorID) : TartsSensorBase(sensorID, PassiveIR, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsPassiveIR::TartsPassiveIR(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, PassiveIR, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsPassiveIR* TartsPassiveIR::Create(const char * sensorID){ return new TartsPassiveIR(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsPassiveIR* TartsPassiveIR::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsPassiveIR(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsPassiveIR::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  Datum dlist[1] = { Datum((char*)"PIR", (char*)((data[1] == 0) ? "0" : "1") , (char*)((data[1] == 0) ? "NO MOTION" : "MOTION")) }; 
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS ACTIVITY SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsActivity::~TartsActivity() {}
TartsActivity::TartsActivity(const char * sensorID) : TartsSensorBase(sensorID, Activity, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsActivity::TartsActivity(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, Activity, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsActivity* TartsActivity::Create(const char * sensorID){ return new TartsActivity(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsActivity* TartsActivity::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsActivity(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsActivity::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  Datum dlist[1] = { Datum((char*)"ACTIVITY",(char*)((data[1] == 0) ? "0" : "1") , (char*)((data[1] == 0) ? "NO MOTION" : "MOTION")) }; 
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS VAC DETECTION SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsVACDetect::~TartsVACDetect() {}
TartsVACDetect::TartsVACDetect(const char * sensorID) : TartsSensorBase(sensorID, VACDetect, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsVACDetect::TartsVACDetect(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, VACDetect, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsVACDetect* TartsVACDetect::Create(const char * sensorID){ return new TartsVACDetect(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsVACDetect* TartsVACDetect::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsVACDetect(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsVACDetect::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  Datum dlist[1] = { Datum((char*)"DETECT", (char*)((data[1] == 0) ? "0" : "1") , (char*)((data[1] == 0) ? "NOT PRESENT" : "PRESENT")) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS VDC DETECTION SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsVDCDetect::~TartsVDCDetect() {}
TartsVDCDetect::TartsVDCDetect(const char * sensorID) : TartsSensorBase(sensorID, VDCDetect, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsVDCDetect::TartsVDCDetect(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, VDCDetect, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_TRG; }
TartsVDCDetect* TartsVDCDetect::Create(const char * sensorID){ return new TartsVDCDetect(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsVDCDetect* TartsVDCDetect::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsVDCDetect(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsVDCDetect::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  Datum dlist[1] = { Datum((char*)"DETECT", (char*)((data[1] == 0) ? "0" : "1") , (char*)((data[1] == 0) ? "NOT PRESENT" : "PRESENT")) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS MEASURE 0-20mA SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsMeasure20mA::~TartsMeasure20mA() {}
TartsMeasure20mA::TartsMeasure20mA(const char * sensorID) : TartsSensorBase(sensorID, Measure20mA, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsMeasure20mA::TartsMeasure20mA(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, Measure20mA, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsMeasure20mA* TartsMeasure20mA::Create(const char * sensorID){ return new TartsMeasure20mA(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsMeasure20mA* TartsMeasure20mA::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsMeasure20mA(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsMeasure20mA::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  char value[16];
  char valueF[16];

  uint16_t i100 = ((uint16_t)data[2] << 8) | (uint16_t)data[1];
  sprintf(value, "%d",  i100); 
  sprintf(valueF, "%d.%d mA", i100/100, i100%100);
  
  Datum dlist[1] = { Datum((char*)"CURRENT", value, valueF) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS MEASURE 0-1VDC SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsMeasure1VDC::~TartsMeasure1VDC() {}
TartsMeasure1VDC::TartsMeasure1VDC(const char * sensorID) : TartsSensorBase(sensorID, Measure1VDC, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsMeasure1VDC::TartsMeasure1VDC(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, Measure1VDC, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsMeasure1VDC* TartsMeasure1VDC::Create(const char * sensorID){ return new TartsMeasure1VDC(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsMeasure1VDC* TartsMeasure1VDC::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsMeasure1VDC(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsMeasure1VDC::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  char value[16];
  char valueF[16];

  uint16_t v1000 = ((uint16_t)data[2] << 8) | (uint16_t)data[1];
  sprintf(value, "%d",  v1000); 
  sprintf(valueF, "%d.%d VDC", v1000/1000, v1000%1000);
  
  Datum dlist[1] = { Datum((char*)"VOLTAGE", value, valueF) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS MEASURE 0-5VDC SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsMeasure5VDC::~TartsMeasure5VDC() {}
TartsMeasure5VDC::TartsMeasure5VDC(const char * sensorID) : TartsSensorBase(sensorID, Measure5VDC, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsMeasure5VDC::TartsMeasure5VDC(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, Measure5VDC, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsMeasure5VDC* TartsMeasure5VDC::Create(const char * sensorID){ return new TartsMeasure5VDC(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsMeasure5VDC* TartsMeasure5VDC::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsMeasure5VDC(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsMeasure5VDC::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  char value[16];
  char valueF[16];

  uint16_t v1000 = ((uint16_t)data[2] << 8) | (uint16_t)data[1];
  sprintf(value, "%d",  v1000); 
  sprintf(valueF, "%d.%d VDC", v1000/1000, v1000%1000);
  
  Datum dlist[1] = { Datum((char*)"VOLTAGE", value, valueF) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS MEASURE 0-10VDC SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsMeasure10VDC::~TartsMeasure10VDC() {}
TartsMeasure10VDC::TartsMeasure10VDC(const char * sensorID) : TartsSensorBase(sensorID, Measure10VDC, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsMeasure10VDC::TartsMeasure10VDC(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, Measure10VDC, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsMeasure10VDC* TartsMeasure10VDC::Create(const char * sensorID){ return new TartsMeasure10VDC(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsMeasure10VDC* TartsMeasure10VDC::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsMeasure10VDC(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsMeasure10VDC::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  char value[16];
  char valueF[16];

  uint16_t v1000 = ((uint16_t)data[2] << 8) | (uint16_t)data[1];
  sprintf(value, "%d",  v1000); 
  sprintf(valueF, "%d.%d VDC", v1000/1000, v1000%1000);
  
  Datum dlist[1] = { Datum((char*)"VOLTAGE", value, valueF) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS MEASURE 0-50VDC SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsMeasure50VDC::~TartsMeasure50VDC() {}
TartsMeasure50VDC::TartsMeasure50VDC(const char * sensorID) : TartsSensorBase(sensorID, Measure50VDC, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsMeasure50VDC::TartsMeasure50VDC(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, Measure50VDC, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsMeasure50VDC* TartsMeasure50VDC::Create(const char * sensorID){ return new TartsMeasure50VDC(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsMeasure50VDC* TartsMeasure50VDC::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsMeasure50VDC(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsMeasure50VDC::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  char value[16];
  char valueF[16];

  uint16_t v1000 = ((uint16_t)data[2] << 8) | (uint16_t)data[1];
  sprintf(value, "%d",  v1000); 
  sprintf(valueF, "%d.%d VDC", v1000/1000, v1000%1000);
  
  Datum dlist[1] = { Datum((char*)"VOLTAGE", value, valueF) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS MEASURE 0-500VAC SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsMeasure500VAC::~TartsMeasure500VAC() {}
TartsMeasure500VAC::TartsMeasure500VAC(const char * sensorID) : TartsSensorBase(sensorID, Measure500VAC, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsMeasure500VAC::TartsMeasure500VAC(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, Measure500VAC, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsMeasure500VAC* TartsMeasure500VAC::Create(const char * sensorID){ return new TartsMeasure500VAC(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsMeasure500VAC* TartsMeasure500VAC::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsMeasure500VAC(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsMeasure500VAC::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  char value[16];
  char valueF[16];

  uint16_t v10 = ((uint16_t)data[2] << 8) | (uint16_t)data[1];
  sprintf(value, "%d",  v10); 
  sprintf(valueF, "%d.%d VAC", v10/10, v10%10);
  
  Datum dlist[1] = { Datum((char*)"VOLTAGE", value, valueF) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS RESISTANCE SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsResistance::~TartsResistance() {}
TartsResistance::TartsResistance(const char * sensorID) : TartsSensorBase(sensorID, Resistance, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsResistance::TartsResistance(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, Resistance, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsResistance* TartsResistance::Create(const char * sensorID){ return new TartsResistance(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsResistance* TartsResistance::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsResistance(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsResistance::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  char value[16];
  char valueF[16];

  uint32_t res = ((uint32_t)data[4] << 24) | ((uint32_t)data[3] << 16) | ((uint32_t)data[2] << 8) | (uint32_t)data[1];
  #ifdef INT_IS_32_SIZED
    sprintf(value, "%u",  res); 
  if((data[0] & 0xF0) == 0x20) strncpy(valueF, "ERROR", sizeof(valueF));
    else sprintf(valueF, "%u.%u Ohms", res/10, res%10);
  #else
    sprintf(value, "%lu",  res); 
    if((data[0] & 0xF0) == 0x20) strncpy(valueF, "ERROR", sizeof(valueF));
    else sprintf(valueF, "%lu.%lu Ohms", res/10, res%10);
  #endif
  
  Datum dlist[1] = { Datum((char*)"RESISTANCE", value, valueF) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS TILT SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsTilt::~TartsTilt() {}
TartsTilt::TartsTilt(const char * sensorID) : TartsSensorBase(sensorID, Tilt, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_INT; } 
TartsTilt::TartsTilt(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, Tilt, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsTilt* TartsTilt::Create(const char * sensorID){ return new TartsTilt(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsTilt* TartsTilt::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsTilt(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsTilt::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  char value1[16];
  char valueF1[16];
  char value2[16];
  char valueF2[16];

  int16_t pitch100 = ((int16_t)data[2] << 8) | (int16_t)data[1];
  int16_t roll100  = ((int16_t)data[4] << 8) | (int16_t)data[3];
  sprintf(value1, "%d", pitch100);
  sprintf(value2, "%d", roll100); 
  if((data[0] & 0xF0) == 0x00){
    char isNegative[2];
    int16_t dec = pitch100%100;
    if((pitch100 < 0) && (pitch100 > -100)){
      isNegative[0] = '-';
      isNegative[1] = 0;
    }
    else isNegative[0] = 0;
    if(dec < 0) dec *= -1;
    sprintf(valueF1, "%s%d.%d DEG", isNegative, pitch100/100, dec);
    dec = roll100%100;
    if((roll100 < 0) && (roll100 > -100)){
      isNegative[0] = '-';
      isNegative[1] = 0;
    }
    else isNegative[0] = 0;
    if(dec < 0) dec *= -1;
    sprintf(valueF2, "%s%d.%d DEG", isNegative, roll100/100, dec);
  }
  else{
    strncpy(valueF1, "ERROR", sizeof(valueF1));
    strncpy(valueF2, "ERROR", sizeof(valueF2));
  }
    
  Datum dlist[2] = { Datum((char*)"PITCH", value1, valueF1), Datum((char*)"ROLL", value2, valueF2) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 2;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS COMPASS SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsCompass::~TartsCompass() {}
TartsCompass::TartsCompass(const char * sensorID) : TartsSensorBase(sensorID, Compass, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsCompass::TartsCompass(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, Compass, reportInterval, linkInterval, retryCount, recovery){ _profileType = DEFAULT_PROFILE_INT; }
TartsCompass* TartsCompass::Create(const char * sensorID){ return new TartsCompass(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsCompass* TartsCompass::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsCompass(sensorID, reportInterval, linkInterval, retryCount, recovery); }
void TartsCompass::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  char value[16];
  char valueF[16];

  int16_t heading = ((int16_t)data[2] << 8) | (int16_t)data[1];
  sprintf(value, "%d", heading);
  if((data[0] & 0xF0) == 0x00) sprintf(valueF, "%d DEG", heading);
  else strncpy(valueF, "ERROR", sizeof(valueF));
  Datum dlist[1] = { Datum((char*)"HEADING", value, valueF) };
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------
//TARTS BASIC CONTROL SENSOR CLASS
//-----------------------------------------------------------------------------------------------------------------------------------------------
TartsBasicControl::~TartsBasicControl() {}
TartsBasicControl::TartsBasicControl(const char * sensorID) : TartsSensorBase(sensorID, BasicControl, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery){
  _defaultSwitchClosed = TartsBasicControlSensorDefaults_defaultSwitchClosed;
  _useLowPower = TartsBasicControlSensorDefaults_useLowPower;
  _ledMode = TartsBasicControlSensorDefaults_ledMode;    
  _pollrate = TartsBasicControlSensorDefaults_pollrate;
  _ctl_option = OPEN;
  _commandDuration = 0;
  _commandAck = 0;
  _profileType = DEFAULT_PROFILE_INT; 
}
TartsBasicControl::TartsBasicControl(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery) : TartsSensorBase(sensorID, BasicControl, reportInterval, linkInterval, retryCount, recovery){
  _defaultSwitchClosed = TartsBasicControlSensorDefaults_defaultSwitchClosed;
  _useLowPower = TartsBasicControlSensorDefaults_useLowPower;
  _ledMode = TartsBasicControlSensorDefaults_ledMode;    
  _pollrate = TartsBasicControlSensorDefaults_pollrate;
  _ctl_option = OPEN;
  _commandDuration = 0;
  _commandAck = 0;
  _profileType = DEFAULT_PROFILE_INT; 
}
TartsBasicControl::TartsBasicControl(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery, bool defaultSwitchClosed, bool useLowPower, TartsBasicControl::ledOptions ledMode, uint16_t pollrate) : TartsSensorBase(sensorID, BasicControl, reportInterval, linkInterval, retryCount, recovery){
  _defaultSwitchClosed = defaultSwitchClosed;
  _useLowPower = useLowPower;
  _ledMode = ledMode;    
  _pollrate = pollrate;
  _ctl_option = OPEN;
  _commandDuration = 0;
  _commandAck = 0;
  _profileType = DEFAULT_PROFILE_INT; 
}
TartsBasicControl* TartsBasicControl::Create(const char * sensorID){ return new TartsBasicControl(sensorID, TartsSensorDefaults_ReportInterval, TartsSensorDefaults_LinkInterval, TartsSensorDefaults_RetryCount, TartsSensorDefaults_Recovery); }
TartsBasicControl* TartsBasicControl::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery){ return new TartsBasicControl(sensorID, reportInterval, linkInterval, retryCount, recovery); }
TartsBasicControl* TartsBasicControl::Create(const char * sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery, bool defaultSwitchClosed, bool useLowPower, TartsBasicControl::ledOptions ledMode, uint16_t pollrate){ return new TartsBasicControl(sensorID, reportInterval, linkInterval, retryCount, recovery, defaultSwitchClosed, useLowPower, ledMode, pollrate); }
void TartsBasicControl::setDefaultSwitchClosed(bool value){ 
  _defaultSwitchClosed = value;
  _dirtyConfig4 = true;
  _queueRequired = true;
}
void TartsBasicControl::setUseLowPower(bool value){
  _useLowPower = value;  
  _dirtyConfig4 = true;
  _queueRequired = true;
}
void TartsBasicControl::setLedMode(TartsBasicControl::ledOptions value){
  _ledMode = value;
  _dirtyConfig4 = true;
  _queueRequired = true;
}
void TartsBasicControl::setPollrate(uint16_t value){
  _pollrate = value;
  _dirtyConfig4 = true;
  _queueRequired = true;
}
bool TartsBasicControl::getDefaultSwitchClosed(){ return _defaultSwitchClosed; }
bool TartsBasicControl::getUseLowPower(){ return _useLowPower; }
TartsBasicControl::ledOptions TartsBasicControl::getLedMode(){ return _ledMode; }
uint16_t TartsBasicControl::getPollrate(){ return _pollrate; }
void TartsBasicControl::sendControl(TartsBasicControl::switchOptions option){ sendControl( option, 0); }
void TartsBasicControl::sendControl(TartsBasicControl::switchOptions option, uint16_t commandDuration){
  _ctl_option = option;
  _commandDuration = commandDuration;
  _appCommandPending = true;
  _queueRequired = true;
  if(_useLowPower){ //the device is expected to be sleeping until poll
    _appCommandRetryCount = 10;
    _nextAppCommandSendTime = 157680000;  //5 years into the future!!! 
  }
  else{
    _appCommandRetryCount = 0;
    _nextAppCommandSendTime = 0;          //Go Now!!! 
  }
}

void TartsBasicControl::_parseData(SensorMessageEvent_t function,  SensorMessage* smsg, uint8_t* data){
  Datum dlist[1] = { Datum((char*)"SWITCH",(char*)((data[1] == 0) ? "0" : "1") , (char*)((data[1] == 0) ? "OPEN" : "CLOSED")) }; 
  smsg->DatumList = (Datum*)&dlist;
  smsg->DatumCount = 1;
  function(smsg);
}

bool TartsBasicControl::_getAppCommand(uint8_t* data, uint8_t* len){
  data[0] = 3; //Sub ID
  data[1] = (uint8_t) _ctl_option;
  data[2] = (uint8_t) (_commandDuration >> 0);
  data[3] = (uint8_t) (_commandDuration >> 8);
  data[4] = (uint8_t) ++_commandAck;
  *len = 5;
  if(_useLowPower) return false; //the device is expected to be sleeping until poll
  else return true;
}
void TartsBasicControl::_parseAppCommand(uint8_t* data){
  if((data[0] == 3) && (data[1] == 0) && (data[2] == _commandAck)){
    _appCommandPending = false; // remove cause we are done
  }
  else{
    _appCommandRetryCount++;
    if(_appCommandRetryCount > 15) _appCommandPending = false;  //IF WE GOT A NO SUCCESS TO COMMAND, remove the appCommand Requirement.
  }
}
void TartsBasicControl::_getProfileConfig2(uint8_t * page){
  //page is for INT type
  page[0] = (_defaultSwitchClosed) ? 1 : 0;  page[1] = 0;  page[2] = 0;  page[3] = 0;
  page[4] = (uint8_t) (_pollrate >> 0); page[5] = (uint8_t) (_pollrate >> 8);  page[6] = 0;  page[7] = 0;
  page[8] = (_useLowPower) ? 0 : 1;  page[9] = 0;  page[10] = 0;  page[11] = 0;
  page[12] = (uint8_t)_ledMode;  page[13] = 0;  page[14] = 0;  page[15] = 0;
}
void TartsBasicControl::_parseProfileConfig2(uint8_t status, uint8_t* page){
  _readConfig4 = false;
  if(status == 0){
  _defaultSwitchClosed = (page[0] == 0) ? false : true;
  _pollrate = (((uint16_t)page[5])<<8) | (uint16_t)page[4];
  _useLowPower = (page[8] != 0) ? false : true;
  _ledMode = (ledOptions)page[12];
}
}



