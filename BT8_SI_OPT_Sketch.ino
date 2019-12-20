/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0: Yveaux
 * 
 * DESCRIPTION
 * This sketch provides an example of how to implement a humidity/temperature
 * sensor using a Si7021 sensor.
 *  
 * For more information, please visit:
 * http://www.mysensors.org/build/humiditySi7021
 * 
 */

// if you uncomment this, you can get test and debug updates about everything the sensor is doing by using the serial monitor tool.
#define MY_DEBUG

#define MY_NODE_ID 35 // Passive mode requires static node ID

//#define MY_PASSIVE_NODE // This node does not listen for messages from the controller

#define MY_RADIO_NRF5_ESB

// LIBRARIES
#include <stdio.h>
#include "SEGGER_RTT.h"
#include <MySensors.h> // The MySensors library. Hurray!
#include "MySensorsNRF5setup.h"

/*
//Battery Level Set Up (LiFePo4)
#define BAT_RANGE 3.0 // BAT_HIGH - BAT_LOW in mv divided by 100 (to facilitate conversion to percent)
#define BAT_HIGH 3350 // Fully charged battery pack in mV (lifepo4)
#define BAT_LOW 3050  //Drained battery pack in mV (this is fine for controller, but min for BME280 and for the LiFePo4 battery)
*/

//Battery Level Set Up (CR2032)
#define BAT_RANGE 17.0 // BAT_HIGH - BAT_LOW in mv divided by 100 (to facilitate conversion to percent)
#define BAT_HIGH 3400  // Fully charged battery pack in mV (CR2032)
#define BAT_LOW 1700   //Drained battery pack in mV

#define SENDVOLTAGE

class BatteryLevel
{
  // Class Member Variables
  // These are initialized at startup
  uint8_t BatInt;
  float Bat;
  float lastBat; // static to preserve from one call of the function to the next.
#ifdef SENDVOLTAGE
  MyMessage Msg_battery;
#endif //SENDVOLTAGE

  // These maintain the current state
  unsigned long previousMillis, intervalBat;

public:
  // Default Constructor
  BatteryLevel()
  {
    intervalBat = 1800000;
    previousMillis = 0;
#ifdef SENDVOLTAGE
    Msg_battery.setSensor(244);
    Msg_battery.setType(V_VOLTAGE);
#endif //SENDVOLTAGE
  }

  BatteryLevel(const uint8_t sensor, unsigned long interval)
  {
    intervalBat = interval;
    previousMillis = 0;
#ifdef SENDVOLTAGE
    Msg_battery.setSensor(sensor);
    Msg_battery.setType(V_VOLTAGE);
#endif //SENDVOLTAGE
  }

  void update(unsigned long current)
  {
    // Store the current millis
    unsigned long currentMillis = current;

    Bat = (float)hwCPUVoltage(); //take voltage measurement
#ifdef MY_DEBUG
    Serial.print("\nBat //  lastBat:  ");
    Serial.print(Bat);
    Serial.print(" //  ");
    Serial.println(lastBat);
    SEGGER_RTT_printf(0, "Bat // lastBat: %u // %u.\r\n", Bat, lastBat);

#endif // MY_DEBUG

    if (Bat > BAT_LOW)
    {
#ifdef MY_DEBUG
      Serial.println("Bat > BAT_LOW");
      SEGGER_RTT_WriteString(0, "\r\nBat > BAT_LOW\r\n");

#endif // MY_DEBUG
      if ((currentMillis - previousMillis) > intervalBat)
      {
        BatInt = (uint8_t)((Bat - BAT_LOW) / BAT_RANGE);
        sendBatteryLevel(BatInt);
#ifdef SENDVOLTAGE
        send(Msg_battery.set(Bat / 1000.0, 3)); //send battery voltage with 3 decimal places
#endif                                          //SENDVOLTAGE
        previousMillis = currentMillis;
#ifdef MY_DEBUG
        Serial.print("\nBat //  BatInt:  ");
        Serial.print(Bat);
        Serial.print(" //  ");
        Serial.println(BatInt);
        SEGGER_RTT_printf(0, "Bat // BatInt: %u // %u.\r\n", Bat, BatInt);

#endif // MY_DEBUG

        lastBat = Bat;
      }
      else
      {
#ifdef MY_DEBUG
        Serial.println("not time yet.  No bat update");
        SEGGER_RTT_WriteString(0, "\r\nNot time yet.  No bat update\r\n");
#endif // MY_DEBUG
      }
    }
    else
    { //Bat <= BAT_LOW
      sendBatteryLevel(0);
#ifdef SENDVOLTAGE
      send(Msg_battery.set(Bat / 1000.0, 3)); //send battery voltage with 3 decimal places
#endif                                        //ID_BATTERY
#ifdef LIFEPO4
      sleep(0); //Go to sleep forever to try and save the battery (bad for LifePo4 to get less than 2.8V)
#endif          //LIFEPO4
    }
  }
};
#define SHORT_WAIT 50
#define SKETCH_NAME "BT8_SQ"
#define SKETCH_VERSION "v0.1"

#define CHILD_ID_HUM  0
#define CHILD_ID_TEMP 1
#define CHILD_ID_BAT 244

BatteryLevel MyBatteryLevel(CHILD_ID_BAT, 60000); //Should use the other constructor

//------------------------------------------------------------------------


static bool metric = true;

// Sleep time between sensor updates (in milliseconds)
static const uint64_t UPDATE_INTERVAL = 60000;
unsigned long currentMillis; // The time since the sensor started, counted in milliseconds. This script tries to avoid using the Sleep function, so that it could at the same time be a MySensors repeater.
unsigned long sleeptime;

unsigned long HeartbeatMotion, HeartbeatBell, HeartbeatBat; // Last send times for use in hearrbeat


#include <SI7021.h>
static SI7021 sensor;

void before()
{
#ifdef MY_DEBUG
  SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
  wait(5);

  Serial.println("\nbefore() - Complete");
  SEGGER_RTT_WriteString(0, "\r\nbefore() - Complete\r\n");
#endif //MY_DEBUG
}

void presentation()  
{ 
  // Send the sketch info to the gateway
  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);

  // Present sensors as children to gateway
  present(CHILD_ID_HUM, S_HUM,   "BT8 sq 35/22 Humidity");
  present(CHILD_ID_TEMP, S_TEMP, "BT8 sq 35/22 Temperature");
  present(CHILD_ID_BAT, S_MULTIMETER, "BT8 sq 35/22");

}

void setup()
{
  while (not sensor.begin())
  {
    Serial.println(F("Sensor not detected!"));
    delay(5000);
  }
  #ifdef MY_DEBUG
  Serial.println("\nsetup() - Complete");
#endif //MY_DEBUG

//  blinkityBlink(2);

}


void loop()      
{  
   currentMillis = millis();

   #ifdef MY_DEBUG
  Serial.print("Sleep for:  ");
  Serial.println(sleeptime);
  SEGGER_RTT_printf(0, "Sleeping for: %u.\r\n", sleeptime);
#endif //MY_DEBUG


  // Read temperature & humidity from sensor.
  const float temperature = float( metric ? sensor.getCelsiusHundredths() : sensor.getFahrenheitHundredths() ) / 100.0;
  const float humidity    = float( sensor.getHumidityBasisPoints() ) / 100.0;

#ifdef MY_DEBUG
  Serial.print(F("Temp "));
  Serial.print(temperature);
  Serial.print(metric ? 'C' : 'F');
  Serial.print(F("\tHum "));
  Serial.println(humidity);
#endif

  static MyMessage msgHum( CHILD_ID_HUM,  V_HUM );
  static MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);

  send(msgTemp.set(temperature, 2));
  send(msgHum.set(humidity, 2));
 MyBatteryLevel.update(currentMillis); //This should be a low level at the end of calculaitons 

  // Sleep until next update to save energy
  #ifdef MY_DEBUG
  Serial.print("BME280 - zzzzZZZZzzzzZZZZzzzz:  ");
  Serial.println(sleeptime);
#endif //MY_DEBUG
  mySleepPrepare();

  sleep(UPDATE_INTERVAL); 
}

