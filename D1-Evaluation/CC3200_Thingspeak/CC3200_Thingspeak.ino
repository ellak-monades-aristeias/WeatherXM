

#ifndef __CC3200R1M1RGC__  // Do not include SPI for CC3200 LaunchPad
#include <SPI.h>
#endif

#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_TMP006.h>
#include <stdlib.h>

// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "your api xxxx";
const int updateThingSpeakInterval = 60 * 1000; // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

//buffer for float to string
char buffer[25];
// your network name also called SSID
char ssid[] = "your ssid";
// your network password
char password[] = "your pass";

// initialize the library instance:
WiFiClient client;

unsigned long lastConnectionTime = 0; // last time you connected to the server, in milliseconds
boolean lastConnected = false; // state of the connection last time through the main loop
const unsigned long postingInterval = 10*1000; //delay between updates to thingspeak
int failedCounter = 0;

Adafruit_TMP006 tmp006(0x41); // start with a diferent i2c address!
void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);

  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }

  if (! tmp006.begin()) {
    Serial.println("No sensor found");
    while (1);
  }

  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");

  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nIP Address obtained");
  printWifiStatus();
}
void loop() {
  // if there's incoming data from the net connection.
  // send it out the serial port. This is for debugging
  // purposes only:
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }

  // if you're not connected, and ten seconds have passed since
  // your last connection, then connect again and send data:
  if (!client.connected() && (millis() - lastConnectionTime > postingInterval)) {
    // read the temp sensor:
    float objt = tmp006.readObjTempC();
    Serial.print("Object Temperature: "); 
    String sobjt = dtostrf(objt,3,3,buffer);
    Serial.print(sobjt); 
    Serial.println("*C");
    float diet = tmp006.readDieTempC();
    Serial.print("Die Temperature: "); 
    Serial.print(diet); 
    Serial.println("*C");
    //send to server
    updateThingSpeak("field1=" + sobjt);
  }
  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}
void updateThingSpeak(String tsData)
{
  if (client.connect(thingSpeakAddress, 80))
  { 
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    Serial.println(">>TSDATALength=" + tsData.length());
    client.print("\n\n");

    client.print(tsData);
    Serial.println(">>TSDATA=" + tsData);

    lastConnectionTime = millis();

    if (client.connected())
    {
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();

      failedCounter = 0;
    }
    else
    {
      failedCounter++;

      Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")"); 
      Serial.println();
    }

  }
  else
  {
    failedCounter++;

    Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")"); 
    Serial.println();

    lastConnectionTime = millis(); 
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


