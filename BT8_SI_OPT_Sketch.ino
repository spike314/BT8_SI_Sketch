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

#define MY_PASSIVE_NODE // This node does not listen for messages from the controller

#define MY_RADIO_NRF5_ESB
#define MY_SIGNING_SOFT

/**
* @def MY_SIGNING_REQUEST_SIGNATURES
* @brief Enable this to inform gateway to sign all messages sent to this node.
*
* If used for a gateway, gateway will only request signatures from nodes that in turn
* request signatures from gateway.
*/
//#define MY_SIGNING_REQUEST_SIGNATURES

#define MY_SIGNING_SOFT_RANDOMSEED_PIN 29

#define CR2032
#define SENDVOLTAGE

// LIBRARIES
#include <stdio.h>
#include "SEGGER_RTT.h"
#include <MySensors.h> // The MySensors library. Hurray!
#include "MySensorsNRF5setup.h"
#include "BatteryLevel.h"

#define SHORT_WAIT 50
#define SKETCH_NAME "BT8_SQ"
#define SKETCH_VERSION "v0.1"

#define CHILD_ID_HUM 0
#define CHILD_ID_TEMP 1
#define CHILD_ID_BAT 244

BatteryLevel MyBatteryLevel(CHILD_ID_BAT, 60000); //This constructor lets you choose the id and time between updates

//------------------------------------------------------------------------

static bool metric = true; // iti s always true for Domoticz

// Sleep time between sensor updates (in milliseconds)
static const uint64_t UPDATE_INTERVAL = 60000;
unsigned long currentMillis; // The time since the sensor started, counted in milliseconds. This script tries to avoid using the Sleep function, so that it could at the same time be a MySensors repeater.

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
  present(CHILD_ID_HUM, S_HUM, "BT8 sq 35/22 Humidity");
  present(CHILD_ID_TEMP, S_TEMP, "BT8 sq 35/22 Temperature");
  present(CHILD_ID_BAT, S_MULTIMETER, "BT8 sq 35/22"); //This should be containerized with BatteryLevel.h, but not sure how to do that.
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

  // Read temperature & humidity from sensor.
  const float temperature = float(metric ? sensor.getCelsiusHundredths() : sensor.getFahrenheitHundredths()) / 100.0;
  const float humidity = float(sensor.getHumidityBasisPoints()) / 100.0;

#ifdef MY_DEBUG
  Serial.print(F("Temp "));
  Serial.print(temperature);
  Serial.print(metric ? 'C' : 'F');
  Serial.print(F("\tHum "));
  Serial.println(humidity);
#endif

  static MyMessage msgHum(CHILD_ID_HUM, V_HUM);
  static MyMessage msgTemp(CHILD_ID_TEMP, V_TEMP);

  send(msgTemp.set(temperature, 2));
  send(msgHum.set(humidity, 2));
  MyBatteryLevel.update(currentMillis); //This should be a low level at the end of calculaitons

#ifdef MY_DEBUG
  Serial.print("Sleep for:  ");
  Serial.println((unsigned long) UPDATE_INTERVAL);
  SEGGER_RTT_printf(0, "Sleeping for: %u.\r\n",  UPDATE_INTERVAL);
#endif //MY_DEBUG
  mySleepPrepare();

  sleep(UPDATE_INTERVAL);
}
