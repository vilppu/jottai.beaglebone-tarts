/**********************************************************************************
 * created by Kelly Lewis and Lynnette Kehaulani (Oct. 2014)                      *
 *                                                                                *
 **********************************************************************************
 *   This file is distributed in the hope that it will be useful, but WITHOUT     *
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
 *   FITNESS FOR A PARTICULAR PURPOSE.  Further inquiries in to licences can be   *
 *   found at www.tartssensors.com/licenses                                       *
 **********************************************************************************
 *   Modified:   Timo Vilppu, January 2018                                        *
 *               HTTP support using libcurl for sensing events to jottai agent *
 *               https://github.com/vilppu/jottai.agent                        *
 *********************************************************************************/

#include <Tarts.h>
#include <TartsStrings.h>
#include <string>
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <signal.h>
#include <stdlib.h>

/**********************************************************************************
 *BEAGLEBONE BLACK PLATFORM-SPECIFIC DEFINITIONS
 *********************************************************************************/
#ifdef BB_BLACK_ARCH
  #define GATEWAY_CHANNELS      0xFFFFFFFF // All Channels
  #define GATEWAY_UARTNUMBER    1          //<<your UART Selection>>
  #define GATEWAY_PINACTIVITY   P9_42      //<<your Activity Pin location>>
  #define GATEWAY_PINPCTS       P8_26      //<<your PCTS Pin location>>
  #define GATEWAY_PINPRTS       P8_15      //<<your PRTS Pin location>>
  #define GATEWAY_PINRESET      P8_12      //<<your Reset Pin location>>
#endif

char thisSensorID[10];
uint16_t thisSensorType;
const char* GatewayId;

void WaitUntilWebApiIsAvailable()
{    
    CURL *curl;
    CURLcode res;
    const char* apiHost = getenv("JOTTAI_API_HOST");

    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_URL, apiHost);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        
        while(1)
        {
            res = curl_easy_perform(curl);
        
            if(res == CURLE_OK)
            {
				std::cout<<"got response from API"<<std::endl;
                return;
            }
            std::cerr<<"failed to connect to server: "<<curl_easy_strerror(res)<<std::endl;
            std::cout<<"reconnecting in 5 seconds..."<<std::endl;
			sleep(5);
        }
        
        curl_easy_cleanup(curl);
    }
    else
    {
        std::cerr<<"failed to initialize libcurl"<<std::endl;
    }
}

void SendHttpPostWithJsonBody(const std::stringstream& json)
{    
    CURL *curl;
    CURLcode res;
    const char* apiHost = getenv("JOTTAI_API_HOST");
    const char* apiKey = getenv("JOTTAI_API_KEY");
    const char* apiBotId = getenv("JOTTAI_ID");

    curl = curl_easy_init();
    if(curl)
    {
        long httpStatusCode = 0;
        struct curl_slist *headers = NULL;
        const std::string& jsonString = json.str();
        const char* content = jsonString.c_str();
        std::string apiKeyHeader("jottai-sensor-data-key: ");
        std::string apiBotIdHeader("jottai-bot-id: ");
        
        if(apiKey != 0)
        {
            apiKeyHeader += apiKey;
        }
        
        if(apiBotId != 0)
        {
            apiBotIdHeader += apiBotId;
        }

        headers = curl_slist_append(headers, apiKeyHeader.c_str());
        headers = curl_slist_append(headers, apiBotIdHeader.c_str());
        headers = curl_slist_append(headers, "Accept: application/json");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "charsets: utf-8");

        res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
        curl_easy_setopt(curl, CURLOPT_URL, apiHost);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);


        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpStatusCode);

        if(res != CURLE_OK)
        {
            std::cerr<<"curl_easy_perform() failed: "<<curl_easy_strerror(res)<<std::endl;
        }
        else if (httpStatusCode != 201)
        {
            std::cerr<<"HTTP post failed with status code: "<<httpStatusCode<<std::endl;
        }
        else
        {
            // std::cout<<"Posted: "<<content<<std::endl;
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else
    {
        std::cerr<<"failed to initialize libcurl"<<std::endl;
    }
}

void SendGatewayUpEvent()
{       
    std::stringstream json;

    json
    <<"{ "
    <<"  \"event\": \"gateway up\","
    <<"  \"gatewayId\": \""<<GatewayId<<"\""
    <<"}";

    SendHttpPostWithJsonBody(json);
}

void SendGatewayDownEvent()
{       
    std::stringstream json;

    json
    <<"{ "
    <<"  \"event\": \"gateway down\","
    <<"  \"gatewayId\": \""<<GatewayId<<"\""
    <<"}";

    SendHttpPostWithJsonBody(json);
}

void SendGatewayActiveOnChannelEvent(int channel)
{       
    std::stringstream json;

    json
    <<"{ "
    <<"  \"event\": \"gateway active\","
    <<"  \"gatewayId\": \""<<GatewayId<<"\","
    <<"  \"channel\": \""<<channel<<"\""
    <<"}";

    SendHttpPostWithJsonBody(json);
}

void SendSensorUpEvent(const char* deviceId, const char* sensorName)
{       
    std::stringstream json;

    json
    <<"{ "
    <<"  \"event\": \"sensor up\","
    <<"  \"deviceId\": \""<<deviceId<<"\","
    <<"  \"gatewayId\": \""<<GatewayId<<"\","
    <<"  \"sensorName\": \""<<sensorName<<"\""
    <<"}";

    SendHttpPostWithJsonBody(json);
}

void SendSensorDataEvent(const SensorMessage* msg)
{       
    std::stringstream json;

    json
    <<"{ "
    <<"  \"event\": \"sensor data\","
    <<"  \"deviceId\": \""<<msg->ID<<"\","
    <<"  \"gatewayId\": \""<<GatewayId<<"\","
    <<"  \"data\": [";

    for(int i=0; i< msg->DatumCount; i++)
    {
        json
        <<"  {"
        <<"    \"name\": \""<<msg->DatumList[i].Name<<"\","
        <<"    \"value\": \""<<msg->DatumList[i].Value<<"\","
        <<"    \"formattedValue\": \""<<msg->DatumList[i].FormattedValue<<"\""
        <<"  }";

        if(i != (msg->DatumCount -1)) {
            json<<",";
        }
    }

	json
    <<"  ],"
	<<"  \"batteryVoltage\": \""<<msg->BatteryVoltage/100<<"."<<msg->BatteryVoltage%100<<" VDC\","
	<<"  \"rssi\": \""<<(int)msg->RSSI<<" dBm\"";

    json<<"}";

    SendHttpPostWithJsonBody(json);
}

void RegisterSensor(TartsSensorBase* sensor, const char* sensorID, const char* sensorName)
{
    if(Tarts.RegisterSensor(GatewayId, sensor))
    {
        SendSensorUpEvent(sensorID, sensorName);
        std::cout<<sensorName<<" ("<<sensorID<<"): Registered."<<std::endl;
    }
    else
    {
        std::cerr<<sensorName<<" ("<<sensorID<<"): Registration Failed!"<<std::endl;
    }
}

/**********************************************************************************
 *Sniffer Create Sensor Function
 *********************************************************************************/
 void CreateSensor(const char* sensorID, uint16_t type){
    switch (type){
        case 1:
        RegisterSensor(TartsMeasure1VDC::Create(sensorID), sensorID, "1 VDC Sensor");
        break;
        case 2:
        RegisterSensor(TartsTemperature::Create(sensorID), sensorID, "Temperature Sensor");
        break;
        case 3:
        RegisterSensor(TartsDryContact::Create(sensorID), sensorID, "Dry Contact Sensor");
        break;
        case 4:
        RegisterSensor(TartsWaterDetect::Create(sensorID), sensorID, "Water Detection Sensor");
        break;
        case 5:
        RegisterSensor(TartsActivity::Create(sensorID), sensorID, "Activity Sensor");
        break;
        case 9:
        RegisterSensor(TartsOpenClose::Create(sensorID), sensorID, "Open Close Sensor");
        break;
        case 11:
        RegisterSensor(TartsButton::Create(sensorID), sensorID, "Button Sensor");
        break;
        case 22:
        RegisterSensor(TartsMeasure20mA::Create(sensorID), sensorID, "20 mA Current Sensor");
        break;
        case 23:
        RegisterSensor(TartsPassiveIR::Create(sensorID), sensorID, "Passive IR Sensor");
        break;
        case 28:
        RegisterSensor(TartsCompass::Create(sensorID), sensorID, "Compass Sensor");
        break;
        case 32:
        RegisterSensor(TartsMeasure500VAC::Create(sensorID), sensorID, "500 VAC Sensor");
        break;
        case 43:
        RegisterSensor(TartsHumidity::Create(sensorID), sensorID, "Humidity Sensor");
        break;
        case 59:
        RegisterSensor(TartsMeasure50VDC::Create(sensorID), sensorID, "50 VDC Sensor");
        break;
        case 64:
        RegisterSensor(TartsVACDetect::Create(sensorID), sensorID, "VAC Detect Sensor");
        break;
        case 65:
        RegisterSensor(TartsWaterTemperature::Create(sensorID), sensorID, "Water Temperature Sensor");
        break;
        case 66:
        RegisterSensor(TartsAsset::Create(sensorID), sensorID, "Assets Sensor");        
        break;
        case 70:
        RegisterSensor(TartsResistance::Create(sensorID), sensorID, "Resistance Sensor");
        break;
        case 71:
        RegisterSensor(TartsVDCDetect::Create(sensorID), sensorID, "VDC Detect Sensor");
        break;
        case 72:
        RegisterSensor(TartsMeasure5VDC::Create(sensorID), sensorID, "5 VDC Sensor");
        break;
        case 74:
        RegisterSensor(TartsMeasure10VDC::Create(sensorID), sensorID, "10 VDC Sensor");
        break;
        case 75:
        RegisterSensor(TartsTilt::Create(sensorID), sensorID, "Tilt Sensor");
        break;
        case 76:
        RegisterSensor(TartsBasicControl::Create(sensorID), sensorID, "Basic Control Sensor");
        break;
        case 78:
        RegisterSensor(TartsWaterRope::Create(sensorID), sensorID, "Water Rope Sensor");
        break; 
        default:
        fprintf(stderr, "The sensor could not be registered because the Sensor Type %d is not recognized. \n", thisSensorType);
    }
}

 void OnGatewayMessageReceived(const char* gID, int stringID)
 {
    printf("TARTS-GWM[%s]-%d: %s\n",gID, stringID, TartsGatewayStringTable[stringID]);

    if(stringID == 2) 
    {
        printf("Found GW: %s\n", Tarts.FindGateway(GatewayId)->getLastUnknownID());
    }

    if(stringID == 3)
    {
        TartsGateway* gateway = Tarts.FindGateway(GatewayId);
        strncpy(thisSensorID, gateway->getLastUnknownID(), sizeof(thisSensorID));
        thisSensorType = gateway->getLastUnknownSensorType();
        if(thisSensorID > 0 && thisSensorType > 0)
        {
            std::cout<<"Found sensor: "<<thisSensorID<<" type "<<thisSensorType<<std::endl;
            CreateSensor(thisSensorID, thisSensorType);
            TartsSensorBase* sen = Tarts.FindSensor(thisSensorID);
            if(sen!=NULL) {
                sen->requestConfigurations();
            }
            thisSensorID[0] = 0;
            thisSensorType = 0;
        }
    }
    else if(stringID == 10)
    {
        int channel = Tarts.FindGateway(GatewayId)->getOperatingChannel();
        printf("ACTIVE - Channel: %d\n", channel);
        SendGatewayActiveOnChannelEvent(channel);
    }

    fflush(stdout);
}

void OnSensorMessageReceived(SensorMessage* msg)
{
    printf("TARTS-SEN[%s]: RSSI: %d dBm, Battery Voltage: %d.%02d VDC, Data: ",
        msg->ID, msg->RSSI,
        msg->BatteryVoltage/100,
        msg->BatteryVoltage%100);

    for(int i=0; i< msg->DatumCount; i++)
    {
        if(i != 0) printf(" || ");
        printf("%s | %s | %s", msg->DatumList[i].Name, msg->DatumList[i].Value, msg->DatumList[i].FormattedValue);
    }

    printf("\n");
    fflush(stdout);

    SendSensorDataEvent(msg);
}


int setup(){
    std::cout<<"starting..."<<std::endl;

    GatewayId = getenv("GATEWAY_ID");

    WaitUntilWebApiIsAvailable();
    
    Tarts.RegisterEvent_GatewayMessage(OnGatewayMessageReceived);
    Tarts.RegisterEvent_SensorMessage(OnSensorMessageReceived);

  //Register Gateway
#ifdef BB_BLACK_ARCH
    if(!Tarts.RegisterGateway(TartsGateway::Create(GatewayId, GATEWAY_CHANNELS, GATEWAY_UARTNUMBER, GATEWAY_PINACTIVITY, GATEWAY_PINPCTS, GATEWAY_PINPRTS, GATEWAY_PINRESET))){
#else
      if(!Tarts.RegisterGateway(TartsGateway::Create(GatewayId))){
#endif
        std::cerr<<"gateway registration failed"<<std::endl;
        return 1;
    }
    
    SendGatewayUpEvent();
    std::cout<<"started..."<<std::endl;

    return 0;
}

void TerminationHandler (int signum)
{
    Tarts.RemoveGateway(GatewayId);
    SendGatewayDownEvent();
    std::cout<<"shutting down..."<<std::endl;
	exit(0);
}

 int main(void){
    if (signal (SIGINT, TerminationHandler) == SIG_IGN)
		signal (SIGINT, SIG_IGN);
    if (signal (SIGHUP, TerminationHandler) == SIG_IGN)
        signal (SIGHUP, SIG_IGN);
    if (signal (SIGTERM, TerminationHandler) == SIG_IGN)
        signal (SIGTERM, SIG_IGN);

    if(setup() != 0)
    {
        exit(1);
    }
    
    while(1)
    {
        Tarts.Process();
        TARTS_DELAYMS(100);
    }
}
