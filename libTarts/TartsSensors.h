/**********************************************************************************
 * TartsSensors.cpp :: Sensor Specific Implementation header file.                *
 * Created   :: July 2014 by Kelly Lewis - MSEE                                   *
 * Modified  :: November 2014 by Kelly Lewis - MSEE                               *
 * Copyright (c) 2014 Tart Sensors. All rights reserved.                          *
 **********************************************************************************
 *   This file is distributed in the hope that it will be useful, but WITHOUT     *
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
 *   FITNESS FOR A PARTICULAR PURPOSE.  Further inquiries in to licences can be   *
 *   found at www.tartssensors.com/licenses                                       * 
 *********************************************************************************/

#ifndef TartsSensorLib_h
#define TartsSensorLib_h

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//SENSOR DEFAULTS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
#define TartsSensorDefaults_ReportInterval  60
#define TartsSensorDefaults_LinkInterval    110
#define TartsSensorDefaults_RetryCount      2
#define TartsSensorDefaults_Recovery        2

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//ENUMERATED SUPPORTED SENSOR TYPES
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

typedef enum {Temperature = 2, WaterTemperature = 65, Humidity = 43,
              DryContact = 3, WaterDetect = 4, WaterRope = 78, OpenClose=9, 
              Button = 11, Asset = 66, PassiveIR = 23, Activity = 5,
              VACDetect= 64, VDCDetect = 71, Measure20mA = 22,
              Measure1VDC = 1, Measure5VDC = 72, Measure10VDC = 74, 
              Measure50VDC = 59, Measure500VAC = 32, Resistance = 70,
              Tilt=75, Compass=28, BasicControl=76,
              Unknown = 0xFFFF} TartsSensorTypes;

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//ABSTRACT SENSOR BASE CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------

//Abstract Base Class - USER, YOU CANNOT CREATE AN OBJECT USING THIS DEFINITION.  However, thanks to inheritance and polymorphism, any 
//specific sensor type can have this common definition.

class TartsSensorBase
{ 
  friend class TartsLib;  
  public:
    ~TartsSensorBase();                                      //Destructor
    TartsSensorBase(const char* sensorID, TartsSensorTypes type, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
    //Methods to change sensor configurations.
    void setReportInterval(uint16_t reportInterval);
    void setLinkInterval(uint8_t linkInterval);
    void setRetryCount(uint16_t retryCount);
    void setRecovery(uint16_t recovery);
   
    //These properties can be read out and saved for "persistence" call backs from the main Tarts object.
    const char* getSensorID();
    TartsSensorTypes getSensorType();
    uint16_t getReportInterval();
    uint8_t  getLinkInterval();
    uint8_t  getRetryCount();
    uint8_t  getRecovery();
    
    //Actions...
    void requestConfigurations();
    bool pendingActions();
    
  protected:
    uint32_t SensorID;
    TartsSensorTypes SensorType;
    uint16_t ReportInterval;
    uint8_t  LinkInterval;
    uint8_t  RetryCount;
    uint8_t  Recovery;    
    uint8_t _profileType;
    bool    _freeOnRemove;
    bool    _dirtyConfig1;
    bool    _dirtyConfig2;
    bool    _dirtyConfig3;
    bool    _dirtyConfig4;
    bool    _readConfig1;
    bool    _readConfig2;
    bool    _readConfig3;
    bool    _readConfig4;
    bool    _queueRequired;
    bool    _appCommandPending;
    uint8_t _appCommandRetryCount;
    unsigned long _nextAppCommandSendTime;
    
  private:
    virtual void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data) = 0;
    virtual void _parseGeneralConfig1(uint8_t status, uint8_t* page);
    virtual void _parseGeneralConfig2(uint8_t status, uint8_t* page);
    virtual void _parseProfileConfig1(uint8_t status, uint8_t* page);
    virtual void _parseProfileConfig2(uint8_t status, uint8_t* page);
    virtual void _parseAppCommand(uint8_t* data);
    virtual void _getGeneralConfig1(uint8_t* page);
    virtual void _getGeneralConfig2(uint8_t* page);
    virtual void _getProfileConfig1(uint8_t* page);
    virtual void _getProfileConfig2(uint8_t* page);
    virtual bool _getAppCommand(uint8_t* data, uint8_t* len);
    
};


//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS TEMPERATURE SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsTemperature : public TartsSensorBase
{
  public:
    ~TartsTemperature();
    //When creating a sensor, if declared in global/fixed memory, a user can use these constructors to set up some or all sensor parameters.
    //Example:
      //-In Global space
      //"Tarts tarts;"
      //"TartsGateway gw(1000,0xFFFFFFFF,0x30,5,6);"
      //"TartsTemperature sensor(51234);" or "TartsTemperature sensor(51234, 600, 2, 2, 2);"
      //-In Arduino setup()
      //"tarts.RegisterSensor(1000, &sensor);
    TartsTemperature(const char* sensorID);
    TartsTemperature(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
    //Alternately, if the user wishes to reserve dynamic memory to create this sensor, these methods will create the object and 
    //return the reference so the tarts object can record it.  
    //Example:
      //-In Global space
      //"Tarts tarts;" 
      //-In Arduino setup()
      //"tarts.RegisterGateway(TartsGateway.Create(1000,0xFFFFFFFF,0x30,5,6));"
      //"tarts.RegisterSensor(1000, TartsTemperature.Create(51234)); or "tarts.RegisterSensor(1000, TartsTemperature.Create(51234, 600, 2, 2, 2));
    static TartsTemperature* Create(const char* sensorID);
    static TartsTemperature* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS WATER TEMPERATURE SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsWaterTemperature : public TartsSensorBase
{
  public:
    ~TartsWaterTemperature();
    TartsWaterTemperature(const char* sensorID);
    TartsWaterTemperature(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsWaterTemperature* Create(const char* sensorID);
    static TartsWaterTemperature* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS HUMIDITY SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsHumidity : public TartsSensorBase
{
  public:
    ~TartsHumidity();
    TartsHumidity(const char* sensorID);
    TartsHumidity(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsHumidity* Create(const char* sensorID);
    static TartsHumidity* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS DRY CONTACT SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsDryContact : public TartsSensorBase
{
  public:
    ~TartsDryContact();
    TartsDryContact(const char* sensorID);
    TartsDryContact(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsDryContact* Create(const char* sensorID);
    static TartsDryContact* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS WATER DETECTION SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsWaterDetect : public TartsSensorBase
{
  public:
    ~TartsWaterDetect();
    TartsWaterDetect(const char* sensorID);
    TartsWaterDetect(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsWaterDetect* Create(const char* sensorID);
    static TartsWaterDetect* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS WATER ROPE DETECTION SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsWaterRope : public TartsSensorBase
{
  public:
    ~TartsWaterRope();
    TartsWaterRope(const char* sensorID);
    TartsWaterRope(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsWaterRope* Create(const char* sensorID);
    static TartsWaterRope* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS OPEN CLOSE SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsOpenClose : public TartsSensorBase
{
  public:
    ~TartsOpenClose();
    TartsOpenClose(const char* sensorID);
    TartsOpenClose(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsOpenClose* Create(const char* sensorID);
    static TartsOpenClose* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS BUTTON SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsButton : public TartsSensorBase
{
  public:
    ~TartsButton();
    TartsButton(const char* sensorID);
    TartsButton(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsButton* Create(const char* sensorID);
    static TartsButton* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};              

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS ASSET SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsAsset : public TartsSensorBase
{
  public:
    ~TartsAsset();
    TartsAsset(const char* sensorID);
    TartsAsset(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsAsset* Create(const char* sensorID);
    static TartsAsset* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS PASSIVE IR SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsPassiveIR : public TartsSensorBase
{
  public:
    ~TartsPassiveIR();
    TartsPassiveIR(const char* sensorID);
    TartsPassiveIR(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsPassiveIR* Create(const char* sensorID);
    static TartsPassiveIR* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);;
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS ACTIVITY SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsActivity : public TartsSensorBase
{
  public:
    ~TartsActivity();
    TartsActivity(const char* sensorID);
    TartsActivity(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsActivity* Create(const char* sensorID);
    static TartsActivity* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);;
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS VAC DETECTION SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsVACDetect : public TartsSensorBase
{
  public:
    ~TartsVACDetect();
    TartsVACDetect(const char* sensorID);
    TartsVACDetect(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsVACDetect* Create(const char* sensorID);
    static TartsVACDetect* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS VDC DETECTION SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsVDCDetect : public TartsSensorBase
{
  public:
    ~TartsVDCDetect();
    TartsVDCDetect(const char* sensorID);
    TartsVDCDetect(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsVDCDetect* Create(const char* sensorID);
    static TartsVDCDetect* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS MEASURE 0-20mA SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsMeasure20mA : public TartsSensorBase
{
  public:
    ~TartsMeasure20mA();
    TartsMeasure20mA(const char* sensorID);
    TartsMeasure20mA(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsMeasure20mA* Create(const char* sensorID);
    static TartsMeasure20mA* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS MEASURE 0-1VDC SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsMeasure1VDC : public TartsSensorBase
{
  public:
    ~TartsMeasure1VDC();
    TartsMeasure1VDC(const char* sensorID);
    TartsMeasure1VDC(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsMeasure1VDC* Create(const char* sensorID);
    static TartsMeasure1VDC* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS MEASURE 0-5VDC SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsMeasure5VDC : public TartsSensorBase
{
  public:
    ~TartsMeasure5VDC();
    TartsMeasure5VDC(const char* sensorID);
    TartsMeasure5VDC(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsMeasure5VDC* Create(const char* sensorID);
    static TartsMeasure5VDC* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS MEASURE 0-10VDC SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsMeasure10VDC : public TartsSensorBase
{
  public:
    ~TartsMeasure10VDC();
    TartsMeasure10VDC(const char* sensorID);
    TartsMeasure10VDC(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsMeasure10VDC* Create(const char* sensorID);
    static TartsMeasure10VDC* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS MEASURE 0-50VDC SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsMeasure50VDC : public TartsSensorBase
{
  public:
    ~TartsMeasure50VDC();
    TartsMeasure50VDC(const char* sensorID);
    TartsMeasure50VDC(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsMeasure50VDC* Create(const char* sensorID);
    static TartsMeasure50VDC* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS MEASURE 0-500VAC SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsMeasure500VAC : public TartsSensorBase
{
  public:
    ~TartsMeasure500VAC();
    TartsMeasure500VAC(const char* sensorID);
    TartsMeasure500VAC(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsMeasure500VAC* Create(const char* sensorID);
    static TartsMeasure500VAC* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS RESISTANCE SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsResistance : public TartsSensorBase
{
  public:
    ~TartsResistance();
    TartsResistance(const char* sensorID);
    TartsResistance(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsResistance* Create(const char* sensorID);
    static TartsResistance* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS TILT SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsTilt : public TartsSensorBase
{
  public:
    ~TartsTilt();
    TartsTilt(const char* sensorID);
    TartsTilt(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsTilt* Create(const char* sensorID);
    static TartsTilt* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS COMPASS SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class TartsCompass : public TartsSensorBase
{
  public:
    ~TartsCompass();
    TartsCompass(const char* sensorID);
    TartsCompass(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsCompass* Create(const char* sensorID);
    static TartsCompass* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
};

//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
//TARTS BASIC CONTROL SENSOR CLASS
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
#define TartsBasicControlSensorDefaults_defaultSwitchClosed  false
#define TartsBasicControlSensorDefaults_useLowPower          true
#define TartsBasicControlSensorDefaults_ledMode              FLASH_WITH_POLL
#define TartsBasicControlSensorDefaults_pollrate             60
   
class TartsBasicControl : public TartsSensorBase
{
  public:
    enum ledOptions{ALWAYS_OFF = 0, ALWAYS_ON, FLASH_WITH_POLL};
    enum switchOptions{OPEN = 1, CLOSE, TOGGLE};

    ~TartsBasicControl();
    TartsBasicControl(const char* sensorID);
    TartsBasicControl(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    TartsBasicControl(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery, bool defaultSwitchClosed, bool useLowPower, TartsBasicControl::ledOptions ledMode, uint16_t pollrate);
    static TartsBasicControl* Create(const char* sensorID);
    static TartsBasicControl* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery);
    static TartsBasicControl* Create(const char* sensorID, uint16_t reportInterval, uint8_t linkInterval, uint8_t retryCount, uint8_t recovery, bool defaultSwitchClosed, bool useLowPower, TartsBasicControl::ledOptions ledMode, uint16_t pollrate);
    
    //Methods to change sensor configurations.
    void setDefaultSwitchClosed(bool value);
    void setUseLowPower(bool value);
    void setLedMode(TartsBasicControl::ledOptions value);
    void setPollrate(uint16_t value);
    
    //Methods to get the sensor configurations.  (Good to call during persistence calls)
    bool getDefaultSwitchClosed();
    bool getUseLowPower();
    TartsBasicControl::ledOptions getLedMode();
    uint16_t getPollrate();
    
    //Method to set the current state of the relay
    void sendControl(TartsBasicControl::switchOptions option);
    void sendControl(TartsBasicControl::switchOptions option, uint16_t commandDuration);
 
  protected:
    bool _defaultSwitchClosed;
    bool _useLowPower;
    TartsBasicControl::ledOptions _ledMode;    
    uint16_t _pollrate;
     
  private:
    void _parseData(SensorMessageEvent_t function, SensorMessage* smsg, uint8_t* data);
    void _getProfileConfig2(uint8_t* page);
    void _parseProfileConfig2(uint8_t status, uint8_t* page);
    bool _getAppCommand(uint8_t* data, uint8_t* len);
    void _parseAppCommand(uint8_t* data);
    
    TartsBasicControl::switchOptions _ctl_option;
    uint16_t _commandDuration;
    uint8_t  _commandAck;
}; 
    

#endif //TartsSensorLib_h




