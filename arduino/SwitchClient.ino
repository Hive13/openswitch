#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SWITCH          8
#define GREEN_LED       6
#define RED_LED         7
#define ONE_WIRE_BUS    9

// Enums for tracking door status
#define OPEN     0
#define CLOSED   1
#define UNKNOWN -1

// Enums for LED status
#define OFF       0
#define RED       1
#define GREEN     2
#define SOLID     3
#define BLINK     4
#define ALTERNATE 5

#define STATUS_INTERVAL 300000  // Interval for switch status checks (ms)
#define TEMP_INTERVAL   300000  // Interval for temperature updates (ms)
#define FAIL_INTERVAL   10000   // Interval for retrying failed http requests (ms)
#define BLINK_INTERVAL  500     // Interval between blinks (ms)

// State tracking variables
unsigned long last_switch_update = -1;
unsigned long last_temp_update = TEMP_INTERVAL;
unsigned long last_blink = -1;
int current_switch_status = UNKNOWN;
int blink_status = OFF; // Yes, these are different.
bool blink_state = false; // Yes, these are different.
bool online = false;

//Network
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; //MAC address
const char* host = "portal.hive13.org";
EthernetClient client;

//Temperature Sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempSensor;

int switchState() {
  int newstate = digitalRead(SWITCH); //Get current Switch Position
  if (newstate != current_switch_status) {
    current_switch_status = newstate;
    return true; //return true if position has changed
  }
  else {
    return false; //return false if position has not changed
  }
}

void setLED(int LEDSTATE) {
  switch(LEDSTATE) {
     case OFF:
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      blink_status=OFF;
      break;
    case RED:
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      blink_status=OFF;
      break;
    case GREEN:
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);
      blink_status=OFF;
      break;
    case SOLID:
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, HIGH);
      blink_status=OFF;
      break;
    case ALTERNATE:
      switch(blink_state) {
        case true:
          digitalWrite(GREEN_LED, HIGH);
          digitalWrite(RED_LED, LOW);
          blink_state = false;
          break;
        case false:
          digitalWrite(GREEN_LED, LOW);
          digitalWrite(RED_LED, HIGH);
          blink_state = true;
          break;
      }
      last_blink = millis();
      break;
    case BLINK:
      switch(blink_state) {
        case true:
          digitalWrite(GREEN_LED, HIGH);
          digitalWrite(RED_LED, HIGH);
          blink_state = false;
          break;
        case false:
          digitalWrite(GREEN_LED, LOW);
          digitalWrite(RED_LED, LOW);
          blink_state = true;
          break;
      }
      last_blink = millis();
      break;
    }
}

bool logSwitch() {
  String url="/isOpen/logger.php?switch=";
  switch(current_switch_status) {
    case OPEN:
      url += "1";
      break;
    case CLOSED:
      url += "0";
      break;
  }
  return httpRequest(url);
}

bool logTemp(float tempf) {
  String url="/isOpen/logger.php?temp=";
  url += tempf;
  return httpRequest(url);
}

bool httpRequest(String requrl) {
  Ethernet.maintain();
  
  Serial.print("Connecting to ");
  Serial.print(host);
  Serial.print("... ");
  
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.print("Failed! ");
    return false;
  }
  
  // This will send the request to the server
  client.print(String("GET ")+requrl+" HTTP/1.1\r\n"+"Host: "+host+"\r\n"+"Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.print("Timeout! ");
      client.stop();
      return false;
    }
  }
  
  Serial.println("Success!");
  return true;
}

void setup() {
  //Initialize Serial Output
  Serial.begin(115200);
  
  //Set inital pin states
  pinMode(SWITCH, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  setLED(SOLID); //Turn on all LEDs to indicate "startup" mode

  while (!Serial) {}; // to prevent lost serial messages
  Serial.println("Initializing.");
  
  // Set up temperature sensor
  sensors.begin();
  sensors.requestTemperatures();
  
  Ethernet.begin(mac);
}

void loop() {
  bool switchoverride = (millis() - last_switch_update > STATUS_INTERVAL);
  if (switchState() || switchoverride) {
    //setLED(SOLID);
    if (switchoverride) { Serial.print("Update interval elapsed. Status "); }
    else { Serial.print("Switch status changed to "); }
    if (current_switch_status == OPEN) {
      Serial.print("on. ");
      if (logSwitch()) {
        last_switch_update = millis();
        setLED(GREEN);
      } else {
        Serial.print("Retry in ");
        Serial.print(FAIL_INTERVAL/1000);
        Serial.println("s.");
        last_switch_update = millis() - STATUS_INTERVAL + FAIL_INTERVAL;
        blink_status=BLINK;
      }
    } else if (current_switch_status == CLOSED) {
      Serial.print("off. ");
      if (logSwitch()) {
        last_switch_update = millis();
        setLED(RED);
      } else {
        Serial.print("Retry in ");
        Serial.print(FAIL_INTERVAL/1000);
        Serial.println("s.");
        last_switch_update = millis() - STATUS_INTERVAL + FAIL_INTERVAL;
        blink_status=BLINK;
      }
    } else {
      Serial.println("Confused... ");
      setLED(BLINK);
    }
  }
  if (millis() - last_temp_update > TEMP_INTERVAL) {
    float tempF = DallasTemperature::toFahrenheit(sensors.getTempCByIndex(0));
    sensors.requestTemperatures();
    if (tempF != 185.00) {
      Serial.print("Logging current temperature: ");
      Serial.print(tempF);
      Serial.print("°F. ");
      if (logTemp(tempF)) {
        last_temp_update = millis();
      } else {
        Serial.print("Retry in ");
        Serial.print(FAIL_INTERVAL/1000);
        Serial.println("s.");
        last_temp_update = millis() - TEMP_INTERVAL + FAIL_INTERVAL;
      }
    }
  }
  if (blink_status != OFF) {
    if (millis() - last_blink > BLINK_INTERVAL) {
      switch(blink_status) {
        case BLINK:
          setLED(BLINK);
          break;
        case ALTERNATE:
          setLED(ALTERNATE);
          break;
      }
    }
  }
}
