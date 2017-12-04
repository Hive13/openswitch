/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define OPEN_PIN        D7
#define CLOSED_PIN      D5
#define OPEN_LED_PIN    D1
#define CLOSED_LED_PIN  D2
#define ONE_WIRE_BUS    D3

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
int led_flash = LOW;

const char* ssid     = "hive13int";
const char* password = "hive13int";

const char* host = "portal.hive13.org";

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// arrays to hold device address
DeviceAddress tempSensor;

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
  Serial.println(digitalRead(OPEN_PIN));
  return digitalRead(OPEN_PIN);
}

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
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
  
    // Setup the digital IO pins
  pinMode(OPEN_PIN,   INPUT_PULLUP);
  pinMode(CLOSED_PIN, INPUT_PULLUP);
  pinMode(OPEN_LED_PIN,   OUTPUT);
  pinMode(CLOSED_LED_PIN, OUTPUT);
  setLED(UNKNOWN);  // Turn on all LED's to indicate we are in "startup" mode.
  
  // Start setup for the temperature sensors
  Serial.println("Setting up temperature sensor...");
  sensors.begin();
}

bool startGetRequest(int requestType, int data) {
  delay(500);

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return 0;
  }
  
  // We now create a URI for the request
  String url = "/isOpen/logger.php";
  
  switch(requestType) {
    case REQUEST_DOOR_EVENT:
      url += "?switch=";
      url += data;
      break;
    case REQUEST_TEMP_EVENT:
      url += "?temp=";
      url += data;
      break;
    case REQUEST_FINISHED: // Should not occur, rather than error, lets continue.
    case REQUEST_STATUS_CHECK:
      // We can use the base request string.
      break;
  }
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return 0;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
  return 1;
}



/*void loop() {
  delay(5000);
  startGetRequest(1,1);
  if (WiFi.status() == WL_CONNECTED)
*/

int curPos = STATUS_INTERVAL;

void loop() {
  Serial.println(millis() - last_status_check);
  Serial.println(STATUS_INTERVAL);
  Serial.println(getSwitchPosition());

  delay(1000);
  if (millis() - last_status_check > STATUS_INTERVAL) { // Is it time to check for a status update?
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
  int curPos = -1;
}
