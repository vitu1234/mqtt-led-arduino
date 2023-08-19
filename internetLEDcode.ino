#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <ArduinoJson.h>
// GPIO 5 D1
#define LED_PIN          5
int button = 16;  // push button is connected
int buttonState = LOW;  // current state of the button
int lastButtonState = LOW;  // previous state of the button

// WiFi
const char *ssid = "onap_wifi"; // Enter your WiFi name
const char *password = "87654321";  // Enter WiFi password

// MQTT Broker
String device_id = "esp8266-q2svd53zb9p4";
const char *mqtt_broker = "192.168.12.225";
const char *topic = "device/edge/upstream/wifi";
const char *s_topic = "cloud/device/downstream/wifi/";
const char *sub_topic = "cloud/device/downstream/wifi/esp8266-q2svd53zb9p4";

const char *mqtt_username = "flotta";
const char *mqtt_password = "flotta";
const int mqtt_port = 1883;

const int EEPROM_ADDRESS = 2;  // Address in the EEPROM to store the LED state
// MQTT: topic
const PROGMEM char* WILL_TOPIC = "device/edge/status/availability";
int WILL_QoS= 0;
bool WILL_Retain= true;
const PROGMEM char* WILL_Message= "offline";
String client_id = "esp8266-switch-";

bool ledState = false;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);
    delay(1000); // Delay for stability

    // Connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to the WiFi network");

    // Setting LED pin as output
    pinMode(LED_PIN, OUTPUT);
    // digitalWrite(LED_PIN, LOW);  // Turn off the LED initially
    pinMode(button, INPUT); 

    // Connecting to an MQTT broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password, WILL_TOPIC, WILL_QoS,WILL_Retain, WILL_Message)) {
            Serial.println("MQTT broker connected");
        } else {
            Serial.print("Failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }

    publishData();
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    String message;
    for (int i = 0; i < length; i++) {
        message += (char) payload[i];  // Convert *byte to string
    }
    message.toUpperCase();
    Serial.print(message);
    if (message == "ON" ) {
      Serial.print("message HEHEHEH");
      pinMode(LED_PIN, OUTPUT);
      digitalWrite(LED_PIN, HIGH);  // Turn on the LED
      ledState = true;
      publishData();
    }
    if (message == "OFF") {
      pinMode(LED_PIN, OUTPUT);
      digitalWrite(LED_PIN, LOW); // Turn off the LED
      ledState = false;
      publishData();
    }


    publishData();
    Serial.println();
    Serial.println("-----------------------");
}

void loop() {
    buttonEvents();
    client.loop();
    delay(1000); // Delay for a short period in each loop iteration
}

void publishData(){

   String publishState = "";
   if (ledState) {
    publishState= "ON";
   }
   if (!ledState) {
    publishState = "OFF";
   }


    String jsonString = "{\"name\":\"NodeMCU\",\"manufacturer\":\"DCN\",\"model\":\"ESP32\",\"id\":\""+device_id+"\",\"properties\":[{\"id\":\"q2s2e35xx32\",\"mode\":\"ReadWrite\",\"name\":\"LED Switch\",\"state\":\"" + publishState +"\"}]}";

//  char data[200];
//  root.printTo(data, root.measureLength() + 1);
  client.publish(topic, jsonString.c_str(), true);


  client.subscribe(sub_topic);
  yield();
}


void buttonEvents() {
  buttonState = digitalRead(button);  // read the current state of the button

  // Check if button state has changed
  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Toggle LED state
      if (digitalRead(LED_PIN) == HIGH) {
        Serial.println("LED Turned ON");
        ledState=true;
      } else {
        Serial.println("LED Turned OFF");
        ledState=false;
      }
    }
    publishData();
    delay(50);  // Debounce delay to avoid multiple readings due to button bounce
  }

  lastButtonState = buttonState;  // save the current button state for comparison
}


