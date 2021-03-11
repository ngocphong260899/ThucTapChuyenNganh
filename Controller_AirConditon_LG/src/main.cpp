#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "IRremoteESP8266.h"
#include "IRutils.h"
#include "IRtext.h"
#include "IRrecv.h"
#include "IRsend.h"
#include "PubSubClient.h"
#include <ArduinoJson.h>
#include "ir_LG.h"
#include <string.h>
/*
*define info mqtt
*/
const char* ssid =            "";
const char* password =        "";
const char* mqtt_client_id =  "ir_controller";
const char* mqtt_server_ip =  "";
const char* mqtt_user =       "";
const char* mqtt_password =   "";
const char* mqtt_topic_sub =  "recv";
const char* mqtt_topic_pub =  "gui";
int   mqtt_port = 1883; 
const int kIrled = D1;
const int led_state_mqtt = D2;
const int button = 0;


WiFiClient espClient;
PubSubClient client(espClient);
IRsend  irsend(kIrled);
IRLgAc  lgAc(kIrled);

#define jsonName  "name"
#define jsonPower "power"
#define jsonTemp  "temp"
#define jsonMode  "mode"
#define jsonFan   "fan"
#define LG  1

void on_air_condition();
void off_air_condition();

void callback(char *p_toppic, uint8_t *p_data, unsigned int length)
{
    StaticJsonBuffer<1024> JSONBuffer;
    JsonObject &root = JSONBuffer.parseObject((char *)p_data);
    Serial.println((char*)p_data);
    int name = (char)root[jsonName];
    if(name == LG){
       if (root[jsonPower] == 1)
        {
          lgAc.on();
          lgAc.setTemp(root[jsonTemp]);
          lgAc.setMode(root[jsonMode]);
          lgAc.setFan(root[jsonFan]);
          lgAc.send();
          Serial.println(lgAc.toString());  
        }
        else if (root[jsonPower] == 0)
        {
          lgAc.off();
          lgAc.send();
          Serial.println(lgAc.toString());
        }
    }
    JSONBuffer.clear();
    
    
}
void reconnect()
{
  while (!client.connected())
  {
    Serial.println("Dang ket noi MQTT...");
    // Connect MQTT
    if (client.connect(mqtt_client_id, mqtt_user, mqtt_password))
    {
      Serial.println("Da ket noi xong MQTT"); // Ket noi xong , hien thi
      client.subscribe(mqtt_topic_sub);
    }
    else
    {
      Serial.print("No connect: ");
      Serial.print(client.state());
      Serial.println("Doi 5 giay");
      delay(5000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  irsend.begin();
  lgAc.begin();
  WiFi.begin(ssid,password);
  //WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Dang ket noi wifi ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(10);
    Serial.print("..........");
  }
  Serial.print("INFO: IP address: ");
  Serial.println(WiFi.localIP());
  
  client.setServer(mqtt_server_ip,mqtt_port);
  client.setCallback(callback);
  pinMode(led_state_mqtt,OUTPUT);
  pinMode(button,INPUT);

}


int dem =0;
void loop() {
  // put your main code here, to run repeatedly:
   if (!client.connected())
    {
      reconnect();
      digitalWrite(led_state_mqtt,LOW);
    }
    client.loop(); 
    //set button controller air condition
    static int btnState, lastState;
    btnState = digitalRead(button);
    if(btnState !=lastState && !btnState ){
       on_air_condition();  
    }
    lastState = btnState;
    
}
//function on condition
void on_air_condition(){
  lgAc.on();
  lgAc.send();
  Serial.println(lgAc.toString()); 
}
//function off condition
void off_air_condition(){
  lgAc.off();
  lgAc.send();
  Serial.println(lgAc.toString()); 
}