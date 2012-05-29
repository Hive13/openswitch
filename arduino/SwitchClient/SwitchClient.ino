/*
  Switch Client
 
 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * OneWire Dallas temp sensor connected to pin 16
 * Open // Closed LED's connected to pins 7, 8
 * One // Closed switch connected to pins 15, 14
 
 created 26 May 2012
 modified 29 May 2012
 by Paul Vincent
 
 Loosely derived from the Arduino Ethernet WebClient
 example program by David A. Mellis
 
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
#define REQUEST_FAILED       -1
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
  setLED(UNKNOWN);  // Turn on all LED's to indicate we are in "startup" mode.
  
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
//  requestType   Use one of the REQUEST_* enums
//         data   Tacked onto URL parameters for certain requests, 
//                ignored if not needed.
//
// Returns:
//    True if the "Get" request succeeded
//    False if the "Get" request failed.
bool startGetRequest(int requestType, int data = 0) {
  bool result = false;
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
    result = true;
  } else {
    Serial.print("HTTP connect for ");
    Serial.print(requestString);
    Serial.println(" failed.");
    result = false;
  }
  
  return result;
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
  int curPos = -1;
  if( client.connected()) {
    Serial.print("Got [");
    delay(1000);  // First delay to allow the client to cache some of the incoming data
    while( client.available() && client.connected()) {
      // read incoming bytes:
      char inChar = client.read();
      Serial.print(inChar);
      delay(30);  // Second delay to ensure that we do not overrun the end of the buffer before it is empty.
    }
    Serial.println("]");
    client.stop();  // If this is missing, all subsequent "get" requests WILL fail.
  }
  else if (millis() - last_status_check > STATUS_INTERVAL) { // Is it time to check for a status update?
    Serial.println("Checking status...");
    if(startGetRequest(REQUEST_STATUS_CHECK, 0))
      last_status_check = millis(); // Only update the "Last Check" time if the get request succeeded.
  }
  else if (millis() - last_temp_update > TEMP_INTERVAL) { // Is it time for a temperature update 'Get'?
    Serial.println("Logging temperature...");
    float tempC = sensors.getTempCByIndex(0);
    sensors.requestTemperatures();
    if(startGetRequest(REQUEST_TEMP_EVENT, DallasTemperature::toFahrenheit(tempC)))
      last_temp_update = millis();  // Only update the "Last Update" time if the get request succeeded.
  }
  else if ((curPos = getSwitchPosition()) != current_switch_status) { // If switch position has changed perform 'Get'
    Serial.print("Switch status is now: ");
    Serial.println(current_switch_status, DEC);
    if(startGetRequest(REQUEST_DOOR_EVENT, curPos)) {
      setLED(curPos); // Only update the LED if get request succeeded.
      current_switch_status = curPos; // Same goes for the saved status.
    }
  }
  else {
    client.stop();  // If the client is not connected, lets make sure all connections are stopped.
    client.flush(); // Also, lets make sure that the client has nothing waiting in the buffer.
    Serial.println("...");  // Spit out a progress debug message.
    
    // TODO: Maybe move this delay out into the main loop? Arguement against: There would be a 5 second delay between the "get" and status update locally.
    delay(5000);    // Now sleep for 5 seconds before we check our status.
  }
}





