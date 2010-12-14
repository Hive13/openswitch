/*
  Web client
 
 This sketch connects to a website (http://www.google.com)
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 created 18 Dec 2009
 by David A. Mellis
 
 */

#include <SPI.h>
#include <Ethernet.h>

#define OPEN 1
#define CLOSED 0
#define UNKNOWN -1

#define openPin 7
#define closedPin 8


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192,168,1,177 }; 
byte server[] = { 216,68,104,242 }; // www.hive13.org
//byte server[] = {192.168.1.37 }; // laptop:eth0@hive13

int switchStatus = UNKNOWN;
boolean needToStop = false;
boolean conSuccess = false;

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
Client client(server, 80);

void setup() {
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
      } else {
        // Failed to connect, and I am not connected.
        // What now?
        Serial.println("connection failed");
      }
    } else {
      delay(20000);
    }
}

