/*
 *  This sketch sends data via HTTP GET requests to thingspeak service every 10 minutes
 *  You have to set your wifi credentials and your thingspeak key.
 */

#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
}

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 5  // DS18B20 pin
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

const char* ssid     = "your ssid";
const char* password = "your pass";

ADC_MODE(ADC_VCC);

const char* host = "api.thingspeak.com";
const char* thingspeak_key = "your api key";

void turnOff(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, 1);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Hello from serial");
  // disable all output to save power
//  turnOff(0);
//  turnOff(2);
//  turnOff(4);
//  turnOff(5);
//  turnOff(12);
//  turnOff(13);
//  turnOff(14);
//  turnOff(15);


  // We start by connecting to a WiFi network


  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() {


  DS18B20.requestTemperatures();

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  float Dtemp;
  do {
    DS18B20.requestTemperatures();
    Dtemp =  DS18B20.getTempCByIndex(0);
  } while (Dtemp == 85.0 || Dtemp == (-127.0));

  String voltage = String( ESP.getVcc());
  String url = "/update?key=";
  url += thingspeak_key;
  url += "&field1=";
  url += String(Dtemp);
  url += "&field2=";
  url += voltage;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  //Serial.println("TEMP=" + String(dht.readTemperature()));
  Serial.println("closing connection. going to sleep...");
  delay(1000);

  
  // go to deepsleep for 10 minutes
  // system_deep_sleep_set_option(0);
  // ESP.deepSleep(1 * 60 * 1000000);

}

