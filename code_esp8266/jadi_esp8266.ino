#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <EEPROM.h>
#define DEBUG
#define AP_SSID     "xxx"
#define AP_PASSWORD "xxx"  

#define AP_CONNECT_TIME 10 //s

#define CONFIG_START 0
#define CONFIG_VERSION "v01"

#define EIOTCLOUD_USERNAME "wellyf"
#define EIOTCLOUD_PASSWORD "welly"

// create MQTT object
#define EIOT_CLOUD_ADDRESS "cloud.iot-playground.com"

#define DO_TOPIC        "/Sensor.Parameter1"

#define PIN_DO_1         D0  // DO pin1 
#define MODULE_ID_1     1


#define PIN_DO_2         D1  // DO pin2 
#define MODULE_ID_2     2


#define PIN_DO_3         D2  // DO pin3 
#define MODULE_ID_3     3


#define PIN_DO_4         D3  // DO pin4 
#define MODULE_ID_4     4

char ap_ssid[16];
char ap_pass[16];


WiFiServer server(80);

MQTT myMqtt("", EIOT_CLOUD_ADDRESS, 1883);

struct StoreStruct {
  // This is for mere detection if they are your settings
  char version[4];
  // The variables of your settings
  uint moduleId;  // module id
  bool state;     // state
  char ssid[20];
  char pwd[20];
} storage = {
  CONFIG_VERSION,
  // The default module 0
  0,
  0, // off
  AP_SSID,
  AP_PASSWORD
};


void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
loadConfig();
  WiFi.mode(WIFI_STA);  
  WiFi.begin(storage.ssid, storage.pwd);
  int i = 0;
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(storage.ssid);
    
  while (WiFi.status() != WL_CONNECTED && i++ < (AP_CONNECT_TIME*2) ) {
    delay(500);
#ifdef DEBUG
    Serial.print(".");
#endif
  }
  if (!(i < (AP_CONNECT_TIME*2)))
  {
    AP_Setup();
    AP_Loop();
    ESP.reset();
  }

  Serial.println("WiFi connected");
  Serial.println("Connecting to MQTT server");  

  //set client id
  // Generate client name based on MAC address and last 8 bits of microsecond counter
  String clientName;
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);
  myMqtt.setClientId((char*) clientName.c_str());

  Serial.print("MQTT client id:");
  Serial.println(clientName);

  // setup callbacks
  myMqtt.onConnected(myConnectedCb);
  myMqtt.onDisconnected(myDisconnectedCb);
  myMqtt.onPublished(myPublishedCb);
  myMqtt.onData(myDataCb);
  
  //////Serial.println("connect mqtt...");
  myMqtt.setUserPwd(EIOTCLOUD_USERNAME, EIOTCLOUD_PASSWORD);  
  myMqtt.connect();

  delay(500);

  pinMode(PIN_DO_1, OUTPUT); 
  pinMode(PIN_DO_2, OUTPUT); 
  pinMode(PIN_DO_3, OUTPUT); 
  pinMode(PIN_DO_4, OUTPUT); 
   
  subscribe();
 while (WiFi.status() != WL_CONNECTED && i++ < (AP_CONNECT_TIME*2) ) {
    delay(500);
#ifdef DEBUG
    Serial.print(".");
#endif
  }
  if (!(i < (AP_CONNECT_TIME*2)))
  {
    AP_Setup();
    AP_Loop();
    ESP.reset();
  }
}

void loop() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
#ifdef DEBUG        
    Serial.print(".");
#endif
  }
}


String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}


void subscribe()
{
    myMqtt.subscribe("/" + String(MODULE_ID_1) + DO_TOPIC); //DO 1
    myMqtt.subscribe("/" + String(MODULE_ID_2) + DO_TOPIC); //DO 2
    myMqtt.subscribe("/" + String(MODULE_ID_3) + DO_TOPIC); //DO 3
    myMqtt.subscribe("/" + String(MODULE_ID_4) + DO_TOPIC); //DO 4
}


void myConnectedCb() {
  Serial.println("connected to MQTT server");
  subscribe();
}

void myDisconnectedCb() {
  Serial.println("disconnected. try to reconnect...");
  delay(500);
  myMqtt.connect();
}

void myPublishedCb() {
  Serial.println("published.");
}

void myDataCb(String& topic, String& data) {  
  if (topic == String("/"+String(MODULE_ID_1)+ DO_TOPIC))
  {
    if (data == String("1"))
      digitalWrite(PIN_DO_1, HIGH);     
    else
      digitalWrite(PIN_DO_1, LOW);

    Serial.print("Do 1:");
    Serial.println(data);
  }


  if (topic == String("/"+String(MODULE_ID_2)+ DO_TOPIC))
  {
    if (data == String("1"))
      digitalWrite(PIN_DO_2, HIGH);     
    else
      digitalWrite(PIN_DO_2, LOW);

    Serial.print("Do 2:");
    Serial.println(data);
  }


  if (topic == String("/"+String(MODULE_ID_3)+ DO_TOPIC))
  {
    if (data == String("1"))
      digitalWrite(PIN_DO_3, HIGH);     
    else
      digitalWrite(PIN_DO_3, LOW);

    Serial.print("Do 3:");
    Serial.println(data);
  }

  if (topic == String("/"+String(MODULE_ID_4)+ DO_TOPIC))
  {
    if (data == String("1"))
      digitalWrite(PIN_DO_4, HIGH);     
    else
      digitalWrite(PIN_DO_4, LOW);

    Serial.print("Do 4:");
    Serial.println(data);
  }
  
}


void loadConfig() {
  // To make sure there are settings, and they are YOURS!
  // If nothing is found it will use the default settings.
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2])
    for (unsigned int t=0; t<sizeof(storage); t++)
      *((char*)&storage + t) = EEPROM.read(CONFIG_START + t);
}

void saveConfig() {
  for (unsigned int t=0; t<sizeof(storage); t++)
    EEPROM.write(CONFIG_START + t, *((char*)&storage + t));

  EEPROM.commit();
}



void AP_Setup(void){
  Serial.println("setting mode");
  WiFi.mode(WIFI_AP);

  String clientName;
  clientName += "Thing-";
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  
  Serial.println("starting ap");
  WiFi.softAP((char*) clientName.c_str(), "");
  Serial.println("running server");
  server.begin();
}

void AP_Loop(void){

  bool  inf_loop = true;
  int  val = 0;
  WiFiClient client;

  Serial.println("AP loop");

  while(inf_loop){
    while (!client){
      Serial.print("");
      delay(100);
      client = server.available();
    }
    String ssid;
    String passwd;
    // Read the first line of the request
    String req = client.readStringUntil('\r');
    client.flush();

    // Prepare the response. Start with the common header:
    String s = "HTTP/1.1 200 OK\r\n";
    s += "Content-Type: text/html\r\n\r\n";
    s += "<!DOCTYPE HTML>\r\n<html>\r\n";

    if (req.indexOf("&") != -1){
      int ptr1 = req.indexOf("ssid=", 0);
      int ptr2 = req.indexOf("&", ptr1);
      int ptr3 = req.indexOf(" HTTP/",ptr2);
      ssid = req.substring(ptr1+5, ptr2);
      passwd = req.substring(ptr2+10, ptr3);    
      val = -1;
    }

    if (val == -1){
      strcpy(storage.ssid, ssid.c_str());
      strcpy(storage.pwd, passwd.c_str());
      
      saveConfig();
      //storeAPinfo(ssid, passwd);
      s += "Setting OK";
      s += "<br>"; // Go to the next line.
      s += "Continue / reboot";
      inf_loop = false;
    }

    else{
      String content="";
      // output the value of each analog input pin
      content += "<form method=get>";
      content += "<label>SSID</label><br>";
      content += "<input  type='text' name='ssid' maxlength='19' size='15' value='"+ String(storage.ssid) +"'><br>";
      content += "<label>Password</label><br>";
      content += "<input  type='password' name='password' maxlength='19' size='15' value='"+ String(storage.pwd) +"'><br><br>";
      content += "<input  type='submit' value='Submit' >";
      content += "</form>";
      s += content;
    }
    
    s += "</html>\n";
    // Send the response to the client
    client.print(s);
    delay(1);
    client.stop();
  }
}
