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

// This delay is used to give the Ethernet shield time to finish initializing.
// Apparently it has a hefty draw on the 3v line on startup, if the Arduino
// is also drawing significant current on the 3v line it causes issues w/
// the ethernet shield.
#define START_DELAY 200

#define OPEN 1
#define CLOSED 0
#define UNKNOWN -1

#define openPin 7
#define closedPin 8

#define openLED 14
#define closedLED 15

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192,168,1,177 }; 
byte server[] = { 216,68,104,242 }; // www.hive13.org
//byte server[] = {192,168,1,37 }; // laptop:eth0@hive13

int switchStatus = UNKNOWN;
boolean needToStop = false;
boolean conSuccess = false;

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
Client client(server, 80);

void setup() {
  // See comment at #define START_DELAY
  delay(START_DELAY);

  // start the Ethernet connection:
  Ethernet.begin(mac, ip);
  // start the serial library:
  Serial.begin(9600);
  // give the Ethernet shield a second to initialize:
  delay(1000);
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
}

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
    } else {
      // Wait two seconds.
      delay(2000);
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
    // Check if we are already connected.
    if(!client.connected()) {
      conSuccess = false;
      // We are not, so lets connect.
      if(client.connect()) {
        conSuccess = true;
        
        // Yay! We connected.
        Serial.println("connected");
        
        // Lets make the request:
        client.print("GET /isOpen/logger.php?switch=");
        client.println(switchState, DEC);
        
        needToStop = true;
        
        // Now that we have connected, lets change the LED.
        setLED(switchState);
      } else {
        // Failed to connect, and I am not connected.
        // What now?
        Serial.println("connection failed");
        
        // TODO_PTV: Add code here to reset the arduino.
        // TODO_PTV: We should probably also keep track of how many times
        //           or perhaps how long, we have been failing to connect
        //           if it crosses a certain threshold, then we should somehow
        //           show some kind of alert, perhaps blink both LEDs?
      }
    } else {
      delay(20000);
    }
}

