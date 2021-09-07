/********************************************************
 * WIFI configuration
 ********************************************************/
const char* ssid = "Freebox-A241D5"; //replace this with your WiFi network name
const char* password = "lividius42-grossiore-inbuis4-conligatis"; //replace this with your WiFi network password

/********************************************************
 * Pinning Configuration
 ********************************************************/
int hallSensorPin = D5;     

/******************************************************** 
 * Tuning parameter
 ********************************************************/
#define ACUMULATION_PERIOD_S 10
#define DELAY_FOR_MOVEMENT_VALIDATION_MS 300
#define BUCKET_VOLUME_MICROL 1500
#define RAIN_COLLECTOR_SURFACE_CM2 130

#include <ArduinoMqttClient.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

/********************************************************
 * Internal variable definition
 ********************************************************/
unsigned long g_bucket_count=0;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char mqttuser[] = "public";
const char mqttpassword[] = "public";
const char broker[] = "192.168.1.12";
int        port     = 1883;
const char topic[]  = "weather_data/rain_gauge";

void IRAM_ATTR hallSensorCallback() {
  static int l_latest_millis = 0;
  int l_current_millis = millis();
  
  if (l_current_millis - l_latest_millis > DELAY_FOR_MOVEMENT_VALIDATION_MS ) {   
    l_latest_millis = l_current_millis;

    // Increment the bucket counter
    g_bucket_count++;
  }
}

void sendRainGaugeData() {
  static unsigned long l_latest_message_time = 0;
  unsigned long epochTime = timeClient.getEpochTime();
  
  // Do we reach the accumulation periode end ?
  if ( epochTime - l_latest_message_time > ACUMULATION_PERIOD_S ) {
  
    String jsonString = "{\"time\":";
    jsonString += epochTime;
    jsonString += ", \"volume_µl\":";
    jsonString += g_bucket_count * BUCKET_VOLUME_MICROL;
    jsonString += ", \"collector_surface_cm2\":";
    jsonString += RAIN_COLLECTOR_SURFACE_CM2;
    jsonString += ", \"duration\":";
    jsonString += l_latest_message_time - epochTime;
    jsonString += "}";

    Serial.println(jsonString);

    // send message, the Print interface can be used to set the message contents
    mqttClient.beginMessage(topic);
    mqttClient.print(jsonString);
    mqttClient.endMessage();

    // Store current date to detect next accumulation period end
    l_latest_message_time = epochTime;

    // Set bucket counter to 0 to restart properly the accumulation period
    g_bucket_count=0;
  }
}

void messageReceived(String &topic, String &payload) {
  Serial.println(topic + ": " + payload);
}

void setup() {
  // Serial connexion setup
  Serial.begin(115200);  

  // WIFI connexion setup
  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("success!");
  Serial.print("IP Address is: ");
  Serial.println(WiFi.localIP());
 
  mqttClient.setUsernamePassword(mqttuser, mqttpassword);
  
  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }
  
  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(3600);  //Grenoble is UTC+1 Hr
  
  pinMode(hallSensorPin, INPUT_PULLUP);    
  attachInterrupt(digitalPinToInterrupt(hallSensorPin), hallSensorCallback, CHANGE);
}

void loop(){
  unsigned long epochTime = 0;

  timeClient.update();

  sendRainGaugeData();

  delay(10000);
}
