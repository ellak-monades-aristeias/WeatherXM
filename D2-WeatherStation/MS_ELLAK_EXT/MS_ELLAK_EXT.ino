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
 *
 * DESCRIPTION
 * Early attempts for a weather station which utilizes the mysensors.org framework
 * uses inertial sensors for wind detection, and capacitive for rain
 * by Manolis Nikiforakis - WeatherXM.com
 */

#include <SPI.h>
#include <MySensor.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <SI7021.h>

#define MY_NODE_ID 1

#define BARO_CHILD 0
#define TEMP_CHILD 1
#define TEMP_CHILD2 2
#define HUM_CHILD 3
#define RAIN_CHILD 4
#define VOLT1_CHILD 5
#define VOLT2_CHILD 6
#define VOLT3_CHILD 7

#define DIGITAL_INPUT_RAIN_SENSOR 3   // Digital input for capacitive/rain sensor. 
#define V1_SENSE_PIN 0
#define V2_SENSE_PIN 1
#define V3_SENSE_PIN 2

const float ALTITUDE = 35; // Athens, Greece = 25m <-- adapt this value to your own location's altitude.

// Sleep time between reads (in seconds). Do not change this value as the forecast algorithm needs a sample every minute.
const unsigned long SLEEP_TIME = 3000;

Adafruit_BMP085 bmp = Adafruit_BMP085();      // Digital Pressure Sensor
SI7021 si;
MySensor gw;


boolean metric;
MyMessage tempMsg(TEMP_CHILD, V_TEMP);
MyMessage pressureMsg(BARO_CHILD, V_PRESSURE);
MyMessage temp2Msg(TEMP_CHILD2, V_TEMP);
MyMessage humMsg(HUM_CHILD, V_HUM);
MyMessage rainMsg(RAIN_CHILD, V_RAINRATE);
MyMessage v1Msg(VOLT1_CHILD, V_VAR1);
MyMessage v2Msg(VOLT2_CHILD, V_VAR2);
MyMessage v3Msg(VOLT3_CHILD, V_VAR3);
void setup()
{
  gw.begin();
  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("WeatherXM external unit", "1.1");
  pinMode(DIGITAL_INPUT_RAIN_SENSOR, INPUT);
  metric = gw.getConfig().isMetric;

  if (!bmp.begin())  {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }


  if (!si.begin())  {
    Serial.println("Could not find a valid SI7021 sensor, check wiring!");
    while (1) {}
  }


  // Register sensors to gw (they will be created as child devices)
  gw.present(BARO_CHILD, S_BARO);
  gw.present(TEMP_CHILD, S_TEMP);
  gw.present(TEMP_CHILD2, S_TEMP);
  gw.present(HUM_CHILD, S_HUM);
  gw.present(RAIN_CHILD, S_RAIN);
  gw.present(VOLT1_CHILD, S_MULTIMETER);
  gw.present(VOLT2_CHILD, S_MULTIMETER);
  gw.present(VOLT3_CHILD, S_MULTIMETER);
}

void loop()
{
  float pressure = bmp.readSealevelPressure(ALTITUDE) / 100.0;
  //float pressure = bmp.readPressure()/100;
  float temperature = bmp.readTemperature();

  si7021_env sidata = si.getHumidityAndTemperature();
  float temp2 = sidata.celsiusHundredths / 100;
  int hum =  sidata.humidityPercent ;

  Serial.print("Temperature2 = ");
  Serial.print(temp2);
  Serial.println(metric ? " *C" : " *F");
  //gw.send(temp2Msg.set(temp2, TEMP_CHILD2));

  Serial.print("Humidity = ");
  Serial.println(hum);
  gw.send(humMsg.set(hum, HUM_CHILD));

  if (!metric)  {
    // Convert to fahrenheit
    temperature = temperature * 9.0 / 5.0 + 32.0;
  }

  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(metric ? " *C" : " *F");
  gw.send(tempMsg.set(temperature, TEMP_CHILD));

  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" hPa");
  gw.send(pressureMsg.set(pressure, BARO_CHILD));

  int rainValue = digitalRead(DIGITAL_INPUT_RAIN_SENSOR);
  Serial.print("Rain = ");
  Serial.println(rainValue);
  gw.send(rainMsg.set((byte)rainValue));

  // get the battery Voltage
  int v1 = analogRead(V1_SENSE_PIN);
  Serial.print("Volt1 = ");
  Serial.println(v1*2*3.3/1023);
  gw.send(v1Msg.set( v1*2*3.3/1000 , VOLT1_CHILD));

  int v2 = analogRead(V2_SENSE_PIN);
  Serial.print("Volt2 = ");
  Serial.println(v2*2*3.3/1023);
  gw.send(v2Msg.set( v2*2*3.3/1023 , VOLT2_CHILD));

  int v3 = analogRead(V3_SENSE_PIN);
  Serial.print("Volt3 = ");
  Serial.println(v3*2*3.3/1023);
  gw.send(v3Msg.set( v3*2*3.3/1023 , VOLT3_CHILD));

  //gw.sendBatteryLevel(sensorValue);


  gw.sleep(SLEEP_TIME);

}



