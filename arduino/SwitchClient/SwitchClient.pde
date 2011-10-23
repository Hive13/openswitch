/*
  Door Open // Closed Switch
  Derivative of: Web client by David A. Mellis.
 
 This sketch connects to a website
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Switch is connected to pins 7, 8
 * LED's are connected to pins 14, 15
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "floatToString.h"

#define OPEN 1
#define CLOSED 0
#define UNKNOWN -1

#define openPin 15
#define closedPin 14

#define openLED 7
#define closedLED 8

// Temperature wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 16

// 300,000 ms = 5 minutes
int TEMP_INTERVAL = 300000;

int DOOR_EVENT = 0;
int TEMP_EVENT = 1;
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xD0, 0x07 };
byte ip[] = { 192,168,1,180 }; 
byte server[] = { 216,68,104,242 }; // www.hive13.org
//byte server[] = {192,168,1,37 }; // laptop:eth0@hive13

int switchStatus = UNKNOWN;
boolean needToStop = false;
boolean conSuccess = false;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
Client client(server, 80);

void setup() {

  // Delay for 2 seconds to allow for the ethernet switch
  // to start.
  delay(5000); // 2 seconds did not work, not sure if 5 will
  
  // start the Ethernet connection:
  Ethernet.begin(mac, ip);
  
  // give the Ethernet shield a second to initialize:
  delay(2000);
  
  // start the serial library:
  Serial.begin(9600);
  
  Serial.println("connecting...");

  digitalWrite(openPin, HIGH);
  digitalWrite(closedPin, HIGH);
  pinMode(openPin, INPUT);
  pinMode(closedPin, INPUT);
  pinMode(openLED, OUTPUT);
  pinMode(closedLED, OUTPUT);
  
  switchStatus = digitalRead(openPin);
  Serial.println(switchStatus);
  startGet(switchStatus);
  
  // Start the temperature Sensor
  sensors.begin();
}

int loopDelay = 2000;
// 5 minutes / loopDelay = how m
int sensorTrigger = TEMP_INTERVAL/loopDelay; 
int sensorCount = 0;

void loop()
{
  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    // Did we just disconnect?
    if(needToStop) {
      Serial.println();
      Serial.println("disconnecting.");
      client.stop();
      needToStop = false;
    }
    
    // Check the switch position.
    int curRead = digitalRead(openPin);
    
    // Did it change?
    if(curRead != switchStatus || !conSuccess) {
      startGet(curRead);
      switchStatus = curRead;
    } else if(sensorTrigger <= sensorCount) {
      // Has it been 5 minutes since this section triggered?
      // Yes, do the following:
      startGet(curRead, TEMP_EVENT);
      sensorCount = 0;
      // - Reset the timer
    } else {
      // Wait two seconds.
      delay(loopDelay);
      
      // Up the sensor count by one.
      sensorCount += 1;
    }
  } else { // We are connected now.
    // if there are incoming bytes available 
    // from the server, read them and print them:
    if (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
  }
}

void setLED(int switchState) {
  if(switchState == OPEN) {
    digitalWrite(openLED, HIGH);
    digitalWrite(closedLED, LOW);
  } else {
    digitalWrite(openLED, LOW);
    digitalWrite(closedLED, HIGH);
  }
}

void startGet(int switchState) {
  startGet(switchState, DOOR_EVENT); 
}

void startGet(int switchState, int eventType) {
    // Check if we are already connected.
    if(!client.connected()) {
      conSuccess = false;
      // We are not, so lets connect.
      if(client.connect()) {
        conSuccess = true;
        
        // Yay! We connected.
        Serial.println("connected");
        
        // Lets make the request:
        if(DOOR_EVENT == eventType) {
          client.print("GET /isOpen/logger.php?switch=");
          client.println(switchState, DEC);
        } else if(TEMP_EVENT == eventType) {
          char dataArray[16] = "";
          floatToString(dataArray, sensors.getTempFByIndex(0), 2);
          client.print("GET /isOpen/logger.php?temp=");
          client.println(dataArray);
        }
        
        needToStop = true;
        
        // Now that we have connected, lets change the LED.
        setLED(switchState);
      } else {
        // Failed to connect, and I am not connected.
        // What now?
        Serial.println("connection failed");
        
        // TODO_PTV: Add code here to reset the arduino.
      }
    } else {
      delay(20000);
    }
}
