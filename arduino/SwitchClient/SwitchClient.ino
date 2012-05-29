/*
  Web client
 
 This sketch connects to a website (http://www.google.com)
 using an Arduino Wiznet Ethernet shield. 
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 
 created 18 Dec 2009
 modified 9 Apr 2012
 by David A. Mellis
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Pin mode setup
// - ethernet pins: 10, 11, 12, 13
#define OPEN_PIN        15
#define CLOSED_PIN      14
#define OPEN_LED_PIN    7
#define CLOSED_LED_PIN  8
#define ONE_WIRE_BUS    16

// Enums for tracking door status
#define OPEN     1
#define CLOSED   0
#define UNKNOWN -1

// Enums for specifying request types
#define REQUEST_FINISHED      0
#define REQUEST_DOOR_EVENT    1
#define REQUEST_TEMP_EVENT    2
#define REQUEST_STATUS_CHECK  3

// Interval for status checks, 300,000 ms = 5 minutes
#define STATUS_INTERVAL 300000

// Interval for temperature updates, 300,000 ms = 5 minutes
#define TEMP_INTERVAL 300000

// State tracking variables
unsigned long last_status_check = -1;
unsigned long last_temp_update = -1;
int current_switch_status = UNKNOWN;
int current_request_state = REQUEST_FINISHED;

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress server(216,68,104,242); // Hive13.org IP
char serverName[] = "hive13.org";  // Hive13 URL

IPAddress ip(192,168,1,180); // Backup local IP in case of DHCP fail

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress tempSensor;


void setup() {
  // Setup the digital IO pins
  pinMode(OPEN_PIN,   INPUT_PULLUP);
  pinMode(CLOSED_PIN, INPUT_PULLUP);
  pinMode(OPEN_LED_PIN,   OUTPUT);
  pinMode(CLOSED_LED_PIN, OUTPUT);
  setLED(UNKNOWN);
  
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.println("Attempting to acquire ip through DHCP...");
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    // if DHCP fails, start with a hard-coded address:
    Serial.println("failed to get an IP address using DHCP, trying manually");
    Ethernet.begin(mac, ip);
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);

  Serial.print("My address:");
  Serial.println(Ethernet.localIP());
  
  // Start setup for the temperature sensors
  Serial.println("Setting up temperature sensor...");
  sensors.begin();
  
}

// Perform a get request to the logger page.
// Parameters:
//  requestType   
void startGetRequest(int requestType, int data = 0) {
  current_request_state = requestType;
  
  String requestString = "GET /isOpen/logger.php";
  // build request parameters
  switch(requestType) {
    case REQUEST_DOOR_EVENT:
      requestString.concat("?switch=");
      requestString.concat(data);
      break;
    case REQUEST_TEMP_EVENT:
      requestString.concat("?temp=");
      requestString.concat(data);
      break;
    case REQUEST_FINISHED: // Should not occur, rather than error, lets continue.
    case REQUEST_STATUS_CHECK:
      // We can use the base request string.
      break;
  }
  //Serial.println(requestString);
  
  // Try to use DNS first, if that fails, fallback to the IP address
  if(client.connect(serverName, 80) || client.connect(server, 80)) {
    Serial.print("Connected, making request: ");
    Serial.println(requestString);
    
    // Make the HTTP Get request to hive13:
    client.println(requestString);
    client.print("HOST: ");
    client.println(serverName);
    client.println();
  } else {
    Serial.print("HTTP connect for ");
    Serial.print(requestString);
    Serial.println(" failed.");
  }
  
}

// Returns true if the switch has changed position since
// the last time this function was called.
bool checkSwitchStatus() {
  int curPos = getSwitchPosition();
  if(curPos != current_switch_status) {
    current_switch_status = curPos;
    return true;  // Switch position has changed.
  }
  return false; // Switch position has not changed
}

// Returns OPEN, CLOSED depending on
// the position of the door switch.
int getSwitchPosition() {
  return digitalRead(OPEN_PIN);
}

void setLED(int switchState) {
  if(switchState == OPEN) {
    digitalWrite(OPEN_LED_PIN, HIGH);
    digitalWrite(CLOSED_LED_PIN, LOW);
  } else if(switchState == CLOSED){
    digitalWrite(OPEN_LED_PIN, LOW);
    digitalWrite(CLOSED_LED_PIN, HIGH);
  } else {
    digitalWrite(OPEN_LED_PIN, HIGH);
    digitalWrite(CLOSED_LED_PIN, HIGH);
  }
}

void loop()
{
  if( client.connected()) {
    Serial.println("Client is connected.");
    delay(1000);
    while( client.available() && client.connected()) {
      // read incoming bytes:
      char inChar = client.read();
      Serial.print(inChar);
      delay(30);
    }
    Serial.println("Finished reading...");
    client.stop();
  }
  else if (millis() - last_status_check > STATUS_INTERVAL) { // Is it time to check for a status update?
    Serial.println("Checking status...");
    startGetRequest(REQUEST_STATUS_CHECK, 0);
    last_status_check = millis();
  }
  else if (millis() - last_temp_update > TEMP_INTERVAL) { // Is it time for a temperature update 'Get'?
    Serial.println("Logging temperature...");
    float tempC = sensors.getTempCByIndex(0);
    sensors.requestTemperatures();
    startGetRequest(REQUEST_TEMP_EVENT, DallasTemperature::toFahrenheit(tempC)); // TODO: Add code to figure out the temperature.
    last_temp_update = millis();
  }
  else if (checkSwitchStatus()) { // If switch position has changed perform 'Get'
    Serial.print("Switch status is now: ");
    Serial.println(current_switch_status, DEC);
    startGetRequest(REQUEST_DOOR_EVENT, current_switch_status);
    setLED(current_switch_status);
  }
  else {
    client.stop(); // Close the connection.
    client.flush();
    Serial.println("...");
    delay(5000);
  }
}





