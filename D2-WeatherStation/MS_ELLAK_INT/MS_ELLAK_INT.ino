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
 * Version 1.0 - Henrik EKblad
 * Contribution by a-lurker and Anticimex,
 * Contribution by Norbert Truchsess <norbert.truchsess@t-online.de>
 * Contribution by Ivo Pullens (ESP8266 support)
 * Contribution by Manolis Nikiforakis (ESP8266 standalone)
 
 *
 * DESCRIPTION
 * The EthernetGateway sends data received from sensors to the WiFi link.
 * The gateway also accepts input on ethernet interface, which is then sent out to the radio network.
 *
 * VERA CONFIGURATION:
 * Enter "ip-number:port" in the ip-field of the Arduino GW device. This will temporarily override any serial configuration for the Vera plugin.
 * E.g. If you want to use the defualt values in this sketch enter: 192.168.178.66:5003
 *
 * LED purposes:
 * - To use the feature, uncomment WITH_LEDS_BLINKING in MyConfig.h
 * - RX (green) - blink fast on radio message recieved. In inclusion mode will blink fast only on presentation recieved
 * - TX (yellow) - blink fast on radio message transmitted. In inclusion mode will blink slowly
 * - ERR (red) - fast blink on error during transmission error or recieve crc error
 *
 * See http://www.mysensors.org/build/ethernet_gateway for wiring instructions.
 * The ESP8266 however requires different wiring:
 * nRF24L01+  ESP8266
 * VCC        VCC
 * CE         GPIO4
 * CSN/CS     GPIO15
 * SCK        GPIO14
 * MISO       GPIO12
 * MOSI       GPIO13
 *
 * Not all ESP8266 modules have all pins available on their external interface.
 * This code has been tested on an ESP-12 module.
 * The ESP8266 requires a certain pin configuration to download code, and another one to run code:
 * - Connect REST (reset) via 10K pullup resistor to VCC, and via switch to GND ('reset switch')
 * - Connect GPIO15 via 10K pulldown resistor to GND
 * - Connect CH_PD via 10K resistor to VCC
 * - Connect GPIO2 via 10K resistor to VCC
 * - Connect GPIO0 via 10K resistor to VCC, and via switch to GND ('bootload switch')
 *
  * Inclusion mode button:
 * - Connect GPIO5 via switch to GND ('inclusion switch')
 *
 * Hardware SHA204 signing is currently not supported!
 *
 * Make sure to fill in your ssid and WiFi password below for ssid & pass.
 */
#define NO_PORTB_PINCHANGES

#include <SPI.h>

#include <MySigningNone.h>
#include <MySigningAtsha204Soft.h>
#include <MyTransportNRF24.h>
#include <MyTransportRFM69.h>
#include <EEPROM.h>
#include <MyHwESP8266.h>
#include <ESP8266WiFi.h>

#include <MyParserSerial.h>
#include <MySensor.h>
#include <stdarg.h>
#include "GatewayUtil.h"

//--------- required for I2C OLED
#include <Wire.h>
#include "font.h"
#include "f2s.h"

//--------- required for DS18B20 temp sensor
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 5 // Pin where dallas sensor is connected(GPIO-5=D1 on NodeMCU)
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature.
int numSensors = 0;
boolean metric = true;


const char *ssid =  "XXX";    // cannot be longer than 32 characters!
const char *pass =  "XXXX"; //

#define offset 0x00    // SDD1306                      // offset=0 for SSD1306 controller
#define OLED_address  0x3c                             // all the OLED's I have seen have this address

ADC_MODE(ADC_VCC);
const char* host = "api.thingspeak.com";
const char* thingspeak_key = "XXXX";

#define INCLUSION_MODE_TIME 1 // Number of minutes inclusion mode is enabled
#define INCLUSION_MODE_PIN  16 // Digital pin used for inclusion mode button

#define RADIO_CE_PIN        4   // radio chip enable
#define RADIO_SPI_SS_PIN    15  // radio SPI serial select

#ifdef WITH_LEDS_BLINKING
#define RADIO_ERROR_LED_PIN 7  // Error led pin
#define RADIO_RX_LED_PIN    8  // Receive led pin
#define RADIO_TX_LED_PIN    9  // the PCB, on board LED
#endif


// NRFRF24L01 radio driver (set low transmit power by default)
MyTransportNRF24 transport(RADIO_CE_PIN, RADIO_SPI_SS_PIN, RF24_PA_LEVEL_GW);
//MyTransportRFM69 transport;


// Message signing driver (signer needed if MY_SIGNING_FEATURE is turned on in MyConfig.h)
#ifdef MY_SIGNING_FEATURE
MySigningNone signer;
//MySigningAtsha204Soft signer;
#endif

// Hardware profile
MyHwESP8266 hw;

// Construct MySensors library (signer needed if MY_SIGNING_FEATURE is turned on in MyConfig.h)
// To use LEDs blinking, uncomment WITH_LEDS_BLINKING in MyConfig.h
MySensor gw(transport, hw
#ifdef MY_SIGNING_FEATURE
            , signer
#endif
#ifdef WITH_LEDS_BLINKING
            , RADIO_RX_LED_PIN, RADIO_TX_LED_PIN, RADIO_ERROR_LED_PIN
#endif
           );


#define IP_PORT 5003         // The port you want to open 
#define MAX_SRV_CLIENTS 5    // how many clients should be able to telnet to this ESP8266

// a R/W server on the port
static WiFiServer server(IP_PORT);
static WiFiClient clients[MAX_SRV_CLIENTS];
static bool clientsConnected[MAX_SRV_CLIENTS];
static inputBuffer inputString[MAX_SRV_CLIENTS];

#define ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))


void output(const char *fmt, ... )
{
  char serialBuffer[MAX_SEND_LENGTH];
  va_list args;
  va_start (args, fmt );
  vsnprintf_P(serialBuffer, MAX_SEND_LENGTH, fmt, args);
  va_end (args);
  Serial.print(serialBuffer);
  for (uint8_t i = 0; i < ARRAY_SIZE(clients); i++)
  {
    if (clients[i] && clients[i].connected())
    {
      //       Serial.print("Client "); Serial.print(i); Serial.println(" write");
      clients[i].write((uint8_t*)serialBuffer, strlen(serialBuffer));
    }
  }
}

void setup()
{
  // Setup console
  hw_init();

  Serial.println(); Serial.println();
  Serial.println("ESP8266 MySensors Gateway");


  Wire.begin(0, 2);                             // Initialize I2C and OLED Display using PINs D3,D4 on NodeMCU
  init_OLED();                                    //
  reset_display();
  clear_display();
  sendStrXY("Hello WeatherXM", 0, 0);            // OLED first message
  sendStrXY("MySensors GW", 2, 0);


  // Startup up the OneWire library
  sensors.begin();
  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);

  float Dtemp;
  do {
    sensors.requestTemperatures();
    Dtemp =  sensors.getTempCByIndex(0);
  } while (Dtemp == 85.0 || Dtemp == (-127.0));
  Serial.print("Temperature:");
  Serial.println(Dtemp);


  Serial.print("Connecting to ");
  Serial.println(ssid);

  (void)WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());
  Serial.flush();

  setupGateway(INCLUSION_MODE_PIN, INCLUSION_MODE_TIME, output);

  // Initialize gateway at maximum PA level, channel 70 and callback for write operations
  gw.begin(incomingMessage, 0, true, 0);

  // start listening for clients
  server.begin();
  server.setNoDelay(true);

  clear_display();
  sendStrXY("WeatherXM.com", 0, 0);            // OLED first message
  sendStrXY("INT temp:", 2, 0);
  sendStrXY("EXT temp:", 3, 0);
  sendStrXY("Humidity:", 4, 0);
  sendStrXY("Pressure:", 5, 0);
  sendStrXY("Wind    :", 6, 0);
  sendStrXY("EXT VCC :", 7, 0);
  
  char buffer[25];
  sendStrXY( floatToString(buffer, sensors.getTempCByIndex(0), 1), 2, 10);
 }


void loop() {
  gw.process();
  checkButtonTriggeredInclusion();
  checkInclusionFinished();

  // Go over list of clients and stop any that are no longer connected.
  // If the server has a new client connection it will be assigned to a free slot.
  bool allSlotsOccupied = true;
  for (uint8_t i = 0; i < ARRAY_SIZE(clients); i++)
  {
    if (!clients[i].connected())
    {
      if (clientsConnected[i])
      {
        Serial.print("Client "); Serial.print(i); Serial.println(" disconnected");
        clients[i].stop();
      }
      //check if there are any new clients
      if (server.hasClient())
      {
        clients[i] = server.available();
        inputString[i].idx = 0;
        Serial.print("Client "); Serial.print(i); Serial.println(" connected");
        output(PSTR("0;0;%d;0;%d;Gateway startup complete.\n"),  C_INTERNAL, I_GATEWAY_READY);
      }
    }
    bool connected = clients[i].connected();
    clientsConnected[i] = connected;
    allSlotsOccupied &= connected;
  }
  if (allSlotsOccupied && server.hasClient())
  {
    //no free/disconnected spot so reject
    Serial.println("No free slot available");
    WiFiClient c = server.available();
    c.stop();
  }

  // Loop over clients connect and read available data
  for (uint8_t i = 0; i < ARRAY_SIZE(clients); i++)
  {
    while (clients[i].connected() && clients[i].available())
    {
      char inChar = clients[i].read();
      if ( inputString[i].idx < MAX_RECEIVE_LENGTH - 1 )
      {
        // if newline then command is complete
        if (inChar == '\n')
        {
          // a command was issued by the client
          // we will now try to send it to the actuator
          inputString[i].string[inputString[i].idx] = 0;

          // echo the string to the serial port
          Serial.print("Client "); Serial.print(i); Serial.print(": "); Serial.println(inputString[i].string);

          parseAndSend(gw, inputString[i].string);

          // clear the string:
          inputString[i].idx = 0;
          // Finished with this client's message. Next loop() we'll see if there's more to read.
          break;
        } else {
          // add it to the inputString:
          inputString[i].string[inputString[i].idx++] = inChar;
        }
      } else {
        // Incoming message too long. Throw away
        Serial.print("Client "); Serial.print(i); Serial.println(": Message too long");
        inputString[i].idx = 0;
        // Finished with this client's message. Next loop() we'll see if there's more to read.
        break;
      }
    }
  }
}


void incomingMessage(const MyMessage & message) {

  Serial.print("Sensor:");
  serial(PSTR("%d;%d;%d;%d;%d;%s\n"), message.sender, message.sensor, mGetCommand(message), mGetAck(message), message.type, message.getString(convBuf));

  if (message.type == V_TEMP) {
    sendStrXY(message.getString(convBuf), 3, 10);
    thingspeak("3", message.getString(convBuf));
  }
  else if (message.type == V_HUM) {
    sendStrXY(message.getString(convBuf), 4, 10);
    thingspeak("4" , message.getString(convBuf));
  }

  else if (message.type == V_PRESSURE) {
    sendStrXY(message.getString(convBuf), 5, 10);
    thingspeak("5", message.getString(convBuf));
  }

  else if (message.type == V_WIND) {
    sendStrXY(message.getString(convBuf), 6, 10);
    thingspeak("6", message.getString(convBuf));
  }

  else if (message.type == V_VOLTAGE) {
    sendStrXY(message.getString(convBuf), 7, 10);
    thingspeak("7", message.getString(convBuf));
  }

  // also update internal temp
  sensors.requestTemperatures();
  char buffer[25];
  sendStrXY( floatToString(buffer, sensors.getTempCByIndex(0), 1), 2, 10);
}


//--------------- OLED FUNCTIONS ------------------

//==========================================================//
// Resets display depending on the actual mode.
static void reset_display(void)
{
  displayOff();
  clear_display();
  displayOn();
}

//==========================================================//
// Turns display on.
void displayOn(void)
{
  sendcommand(0xaf);        //display on
}

//==========================================================//
// Turns display off.
void displayOff(void)
{
  sendcommand(0xae);    //display off
}

//==========================================================//
// Clears the display by sendind 0 to all the screen map.
static void clear_display(void)
{
  unsigned char i, k;
  for (k = 0; k < 8; k++)
  {
    setXY(k, 0);
    {
      for (i = 0; i < (128 + 2 * offset); i++) //locate all COL
      {
        SendChar(0);         //clear all COL
        //delay(10);
      }
    }
  }
}

//==========================================================//
// Actually this sends a byte, not a char to draw in the display.
// Display's chars uses 8 byte font the small ones and 96 bytes
// for the big number font.
static void SendChar(unsigned char data)
{
  //if (interrupt && !doing_menu) return;   // Stop printing only if interrupt is call but not in button functions

  Wire.beginTransmission(OLED_address); // begin transmitting
  Wire.write(0x40);//data mode
  Wire.write(data);
  Wire.endTransmission();    // stop transmitting
}

//==========================================================//
// Prints a display char (not just a byte) in coordinates X Y,
// being multiples of 8. This means we have 16 COLS (0-15)
// and 8 ROWS (0-7).
static void sendCharXY(unsigned char data, int X, int Y)
{
  setXY(X, Y);
  Wire.beginTransmission(OLED_address); // begin transmitting
  Wire.write(0x40);//data mode

  for (int i = 0; i < 8; i++)
    Wire.write(pgm_read_byte(myFont[data - 0x20] + i));

  Wire.endTransmission();    // stop transmitting
}

//==========================================================//
// Used to send commands to the display.
static void sendcommand(unsigned char com)
{
  Wire.beginTransmission(OLED_address);     //begin transmitting
  Wire.write(0x80);                          //command mode
  Wire.write(com);
  Wire.endTransmission();                    // stop transmitting
}

//==========================================================//
// Set the cursor position in a 16 COL * 8 ROW map.
static void setXY(unsigned char row, unsigned char col)
{
  sendcommand(0xb0 + row);              //set page address
  sendcommand(offset + (8 * col & 0x0f)); //set low col address
  sendcommand(0x10 + ((8 * col >> 4) & 0x0f)); //set high col address
}


//==========================================================//
// Prints a string regardless the cursor position.
static void sendStr(unsigned char *string)
{
  unsigned char i = 0;
  while (*string)
  {
    for (i = 0; i < 8; i++)
    {
      SendChar(pgm_read_byte(myFont[*string - 0x20] + i));
    }
    *string++;
  }
}

//==========================================================//
// Prints a string in coordinates X Y, being multiples of 8.
// This means we have 16 COLS (0-15) and 8 ROWS (0-7).
static void sendStrXY( char *string, int X, int Y)
{
  setXY(X, Y);
  unsigned char i = 0;
  while (*string)
  {
    for (i = 0; i < 8; i++)
    {
      SendChar(pgm_read_byte(myFont[*string - 0x20] + i));
    }
    *string++;
  }
}


//==========================================================//
// Inits oled and draws logo at startup
static void init_OLED(void)
{
  sendcommand(0xae);    //display off
  sendcommand(0xa6);            //Set Normal Display (default)
  // Adafruit Init sequence for 128x64 OLED module
  sendcommand(0xAE);             //DISPLAYOFF
  sendcommand(0xD5);            //SETDISPLAYCLOCKDIV
  sendcommand(0x80);            // the suggested ratio 0x80
  sendcommand(0xA8);            //SSD1306_SETMULTIPLEX
  sendcommand(0x3F);
  sendcommand(0xD3);            //SETDISPLAYOFFSET
  sendcommand(0x0);             //no offset
  sendcommand(0x40 | 0x0);      //SETSTARTLINE
  sendcommand(0x8D);            //CHARGEPUMP
  sendcommand(0x14);
  sendcommand(0x20);             //MEMORYMODE
  sendcommand(0x00);             //0x0 act like ks0108

  //sendcommand(0xA0 | 0x1);      //SEGREMAP   //Rotate screen 180 deg
  sendcommand(0xA0);

  //sendcommand(0xC8);            //COMSCANDEC  Rotate screen 180 Deg
  sendcommand(0xC0);

  sendcommand(0xDA);            //0xDA
  sendcommand(0x12);           //COMSCANDEC
  sendcommand(0x81);           //SETCONTRAS
  sendcommand(0xCF);           //
  sendcommand(0xd9);          //SETPRECHARGE
  sendcommand(0xF1);
  sendcommand(0xDB);        //SETVCOMDETECT
  sendcommand(0x40);
  sendcommand(0xA4);        //DISPLAYALLON_RESUME
  sendcommand(0xA6);        //NORMALDISPLAY

  clear_display();
  sendcommand(0x2e);            // stop scroll
  //----------------------------REVERSE comments----------------------------//
  sendcommand(0xa0);    //seg re-map 0->127(default)
  sendcommand(0xa1);    //seg re-map 127->0
  sendcommand(0xc8);
  delay(1000);
  //----------------------------REVERSE comments----------------------------//
  // sendcommand(0xa7);  //Set Inverse Display
  // sendcommand(0xae);   //display off
  sendcommand(0x20);            //Set Memory Addressing Mode
  sendcommand(0x00);            //Set Memory Addressing Mode ab Horizontal addressing mode
  //  sendcommand(0x02);         // Set Memory Addressing Mode ab Page addressing mode(RESET)
}














//-------------- ThingSpeak -----------------------------

void thingspeak( String FieldNo, String VALUE) {

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }


  String voltage = String( ESP.getVcc());
  String url = "/update?key=";
  url += thingspeak_key;
  url += "&field" + FieldNo + "=";
  url += String(VALUE);
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
}

