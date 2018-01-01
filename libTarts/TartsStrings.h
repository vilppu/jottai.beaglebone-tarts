/**********************************************************************************
 * TartsSensors.cpp :: String Descriptions of Gateway and Exception Messages IDs  *
 * Created   :: October 2014 by Kelly Lewis - MSEE                                *
 * Modified  :: November 2014 by Kelly Lewis - MSEE                               *
 * Copyright (c) 2014 Tart Sensors. All rights reserved.                          *
 **********************************************************************************
 *   This file is distributed in the hope that it will be useful, but WITHOUT     *
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
 *   FITNESS FOR A PARTICULAR PURPOSE.  Further inquiries in to licences can be   *
 *   found at www.tartssensors.com/licenses                                       * 
 *********************************************************************************/

//Arduino is notorious for its lack of proper support for strings (and the memory required for strings).  
//This file allows a user to choose if they want to incorporate strings in to their application.  
//If these tables are not called by the application, then they are not included in to compiled program.

#ifndef TARTS_STRINGS_H
#define TARTS_STRINGS_H

const char* TartsGatewayStringTable[] = {
  /*0*/   "Gateway Registered", \
  /*1*/   "Gateway Unregistered", \
  /*2*/   "Unexpected gateway ID detected!, Check GATEWAY_ID in program.", \
  /*3*/   "Unregistered sensor traffic detected!", \
  /*4*/   "STATE::OFF", \
  /*5*/   "STATE::STARTING", \
  /*6*/   "STATE::REFORMING", \
  /*7*/   "STATE::REMOVING", \
  /*8*/   "STATE::LOADING", \
  /*9*/   "STATE::ACTIVATING", \
  /*10*/  "STATE::ACTIVE" \
};

const char* TartsExceptionStringTable[] = { 
  /*0*/   "ERROR :: RegisterGateway :: Gateway object is NULL", \
  /*1*/   "WARN  :: RegisterGateway :: Duplicate gateway object", \
  /*2*/   "ERROR :: RegisterGatway :: Identical gateway being registered", \
  /*3*/   "ERROR :: RegisterGateway :: Hardware failed to initialize", \
  /*4*/   "ERROR :: RegisterGateway :: Memory Exception!", \
  /*5*/   "WARN  :: RemoveGateway :: Internal gateway list is empty", \
  /*6*/   "ERROR :: RemoveGateway :: Memory Exception!", \
  /*7*/   "WARN  :: RemoveGateway :: Gateway ID not found", \
  /*8*/   "ERROR :: RegisterSensor :: Sensor object is NULL/INVALID", \
  /*9*/   "ERROR :: RegisterSensor :: Duplicate ID detected", \
  /*10*/  "WARN  :: RegisterSensor :: Sensor already registered", \
  /*11*/  "ERROR :: RegisterSensor :: Invalid gateway ID", \
  /*12*/  "ERROR :: RegisterSensor :: Memory Exception!", \
  /*13*/  "ERROR :: RemoveSensor :: Memory Exception!", \
  /*14*/  "ERROR :: RemoveSensor :: Memory Exception2!", \
  /*15*/  "WARN  :: Process :: Requested sensor ID not recognized", \
  /*16*/  "WARN  :: Sensor type mismatch!", \
  /*17*/  "ERROR :: Process :: Gateway in unknown state", \
};

#endif
