
/*
    This sketch sends a string to a TCP server, and prints a one-line response.
    You must run a TCP server in your local network.
    For example, on Linux you can use this command: nc -v -l 3000
*/
#include <stdio.h>
#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_PIN 4
 
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

#define STASSID "slowfi"
#define STASSID2 "slowfi_garage"
#define STAPSK  "barbapappa!!"


// Set your Static IP address
IPAddress local_IP(192, 168, 1, 202);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(192, 168, 1, 1);   //optional

//ESP8266WiFiMulti WiFiMulti;

#define SERVER      "192.168.1.38"
#define PORT  1883                   // use 8883 for SSL

  // Use WiFiClient class to create TCP connections
  WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, SERVER, PORT);

/****************************** Feeds ***************************************/

// Setup a feed called 'arb_packet' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
#define BASE_TOPIC "homeassistant/sensor/D1/" + mac + ""
#define STATE_TOPIC BASE_TOPIC"/state"
#define ATTR_TOPIC BASE_TOPIC"/attr"
#define CONF_TOPIC BASE_TOPIC"/config"


#define DS18B_PIN 12

void MQTT_connect();
String mac, maclong;

//gather sleep routine
void sleep()
{
    digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
    digitalWrite(DS18B_PIN, LOW); // Turn the ds18b20 off 
    ESP.restart(); 
    //ESP.deepSleep(120*1000*1000); 
}


void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  pinMode(DS18B_PIN, OUTPUT);     // Initialize the DS18B_PIN pin as an output
  digitalWrite(DS18B_PIN, LOW); // Turn the ds18b20 off 

  Serial.begin(115200);

    // Configures static IP address
  // if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) 
  // {
  //   Serial.println("STA Failed to configure");
  // }

  // We start by connecting to a WiFi network
  WiFi.mode(WIFI_STA);
//  WiFiMulti.addAP(STASSID, STAPSK);
//  WiFiMulti.addAP(STASSID2, STAPSK);

  Serial.print("\nWait for WiFi");
  WiFi.begin(STASSID2, STAPSK);

  int i = 0;
  while( WiFi.status() != WL_CONNECTED ) 
  {
    if(i > 1500)
      sleep();  //if no wifi after 10 sec, sleep
    i++;

    Serial.print(".");
    delay(100);
  }

  Serial.print(" WiFi connected");
  Serial.print(" IP address: ");
  Serial.println(WiFi.localIP());

  MQTT_connect();

 
  //Fetch mac
  mac = maclong = WiFi.macAddress();
  mac.replace(":", "");
  String test1 = CONF_TOPIC;
  Adafruit_MQTT_Publish confT = Adafruit_MQTT_Publish(&mqtt, test1.c_str());
  
  //String conftemplate = "{\"dev_cla\": \"temperature\", \"exp_aft\": \"270\", \"name\": \"D1 temp\", \"uniq_id\": \"wemosD1_" + mac + "\", \"stat_t\": \"" STATE_TOPIC "\", \"unit_of_meas\": \"C\", \"val_tpl\": \"{{value_json.temperature}}\", \"dev\":{\"ids\":[\"" +mac+ "\"],\"mdl\":\"WemosD1\",\"mf\":\"Petter\"} }";
  String conftemplate = "{\"dev_cla\":\"temperature\",\"exp_aft\":\"270\",\"name\":\"D1_temp\",\"uniq_id\":\"wemosD1_" + mac + "\",\"stat_t\":\"" STATE_TOPIC "\",\"json_attr_t\":\"" ATTR_TOPIC "\",\"unit_of_meas\":\"C\",\"val_tpl\":\"{{value_json.temperature}}\",\"dev\":{\"ids\":[\"" +mac+ "\"],\"mf\":\"Petter\",\"mdl\":\"WemosD1\",\"name\":\"WemosD1\"}}";


//  Serial.println(conftemplate.length());
  Serial.println(conftemplate.c_str());
//*
  if (! confT.publish((uint8_t*)conftemplate.c_str(), conftemplate.length()))
    Serial.println(F("Publish conf Failed."));
  else 
  {
    Serial.println(F("Publish conf Success!"));
  }//*/

  char pac[50] = {0};
  int len = snprintf(pac, sizeof(pac), "{\"IP\": \"%s\",\"Mac\":\"%s\"}", WiFi.localIP().toString().c_str() , maclong.c_str());

  String test = ATTR_TOPIC;
  Adafruit_MQTT_Publish attrT = Adafruit_MQTT_Publish(&mqtt, test.c_str());

  if (! attrT.publish((uint8_t*)pac, len))
    Serial.println(F("Publish attrT Failed."));
  else 
  {
    Serial.println(F("Publish attrT Success!"));
  }
  
}

//{"dev_cla":"temperature","exp_aft":"270","name":"D1_temp","uniq_id":"wemosD1_84:CC:A8:A1:CB:2F","stat_t":"homeassistant/sensor/wemos/D1/state","unit_of_meas":"C","val_tpl":"{{value_json.temperature}}","dev":{"ids":["84:CC:A8:A1:CB:2F"],"mf":"Petter","mdl":"We
void loop() 
{
    digitalWrite(DS18B_PIN, HIGH);   // Turn the ds18b20 on 
    delay(20);
    oneWire.reset();
    sensors.begin();  
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);
    if(temp == -127.0)
    {
      Serial.println(F("Sensor failed, sleeping..."));
      delay(10);  // wait 60 ms
      sleep();  //bail if no reading
    }

  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
    MQTT_connect();

    Serial.println(temp);


    char pac[50] = {0};
    int len = snprintf(pac, sizeof(pac), "{\"temperature\": %.1f}", temp);

    String test = STATE_TOPIC;
    Adafruit_MQTT_Publish stateT = Adafruit_MQTT_Publish(&mqtt, test.c_str());

    if (! stateT.publish((uint8_t*)pac, len))
      Serial.println(F("Publish Failed."));
    else 
    {
      Serial.println(F("Publish Success!"));
    }

  delay(70);  // wait 70 ms for transaction to complete
  

  //Serial.print("Number of sensors = ");
  //Serial.println(sensors.getDeviceCount());

 // sleep();
  digitalWrite(LED_BUILTIN, HIGH);   // Turn the LED on (Note that LOW is the voltage level
  delay(30* 1000); 
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() 
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) 
  {
    return;
  }

  Serial.print(F("Connecting to MQTT... "));

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) 
  { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println(F("Retrying MQTT connection in 5 seconds..."));
       mqtt.disconnect();
       delay(5);  // wait 5 ms
       retries--;
       if (retries == 0) 
       {
        sleep(); // basically die 
       }
  }
  Serial.println(F("MQTT Connected!"));
}
