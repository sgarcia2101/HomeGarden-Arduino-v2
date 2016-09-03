       
//House monitoring framework with Arduino and Raspberry Pi
//Cezar Chirila  
//AllAboutCircuits.com
//epilepsynerd.wordpress.com

#include "ArduinoJson.h"
#include "dht.h"

#define DHT11_PIN A0 //Pin for DHT11 data
#define LED_PIN A1 //PIN for LED

#define ARRAYSIZE(x)  (sizeof(x) / sizeof(x[0]))
#define MAX_SENSORS 2

struct Sensor {
  String type;
  String pin;
  String value;
};

struct Actuator {
  String pin;
  String status;
  long time;
};

struct Status {
  Actuator ligth;
  Actuator water;
  Sensor sensors[MAX_SENSORS];
};

dht DHT; //give name to your DHT sensorç

Status globalStatus;

long lastTime = millis();

void setup(){
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  globalStatus.ligth = {"A0", "OFF", 0};
  globalStatus.water = {"A1", "OFF", 0};
  globalStatus.sensors[0] = Sensor{"A2", "DHT11", "23.00"};
  globalStatus.sensors[1] = Sensor{"A3", "SOIL", "30"};
  
}

void loop() {
  lastTime = millis();
  
  checkStatus();
  
  while (Serial.available()) {
    
    String inputJson = Serial.readString();
    
    StaticJsonBuffer<1024> jsonInputBuffer;
    JsonObject& root = jsonInputBuffer.parseObject(inputJson);
    
    if (!root.success()) { 
      Serial.println("ERROR: JSON_PARSE_ERROR");
    } else {
      
      Serial.print("JSON_RECEIVED:");
      root.prettyPrintTo(Serial);
      Serial.println();
      
      String action = root["action"];
      
      // {"action":"WATER_ON","time":20}
      if(action == "WATER_ON"){
        String time = root["time"];
        
        globalStatus.water.status = "ON";
        globalStatus.water.time = lastTime + (time.toInt() * 1000);
        
      // {"action":"WATER_OFF"}
      } else if(action == "WATER_OFF") {
        globalStatus.water.status = "OFF";
        
      // {"action":"LIGHT_ON","time":20}
      } else if(action == "LIGHT_ON") {
        String time = root["time"];
        
        globalStatus.ligth.status = "ON";
        globalStatus.ligth.time = lastTime + (time.toInt() * 1000);
      
      // {"action":"LIGHT_OFF"}
      } else if(action == "LIGHT_OFF") {
        globalStatus.water.status = "OFF";
        
      } else {
        Serial.print("INVALID_OPTION");
      }
    
    }
    
  }
  
  sendData();
  
  delay(5000);
    
}

void checkStatus() {
  if(lastTime > globalStatus.ligth.time){
    globalStatus.ligth.status = "OFF";
    globalStatus.ligth.time = 0;
  }
  
  if(lastTime > globalStatus.water.time){
    globalStatus.water.status = "OFF";
    globalStatus.water.time = 0;
  }
  
}

void  sendData() {
  Serial.print("STATUS [" + String(lastTime, DEC) + "]: ");
  
  StaticJsonBuffer<1024> jsonOutputBuffer;
  JsonObject& root = jsonOutputBuffer.createObject();
  
  JsonObject& light = root.createNestedObject("light");
  light["pin"] = globalStatus.ligth.pin;
  light["status"] = globalStatus.ligth.status;
  light["time"]= globalStatus.ligth.time;
  
  JsonObject& water = root.createNestedObject("water");
  water["pin"] = globalStatus.water.pin;
  water["status"] = globalStatus.water.status;
  water["time"]= globalStatus.water.time;
  
  JsonArray& sensors = root.createNestedArray("sensors");

  for (int i = 0; i < ARRAYSIZE(globalStatus.sensors); i++) {
    
    JsonObject &sensor = sensors.createNestedObject();
    sensor["type"] = globalStatus.sensors[i].type;
    sensor["pin"] = globalStatus.sensors[i].pin;
    sensor["value"] = globalStatus.sensors[i].value;
    
  }
  
  //root.printTo(Serial);
  //Serial.println();
  
  root.prettyPrintTo(Serial);
  Serial.println();
}

// function that return the temperature as an integer
int get_temperature() {
  int temperature;
  
  DHT.read11(DHT11_PIN);
  temperature = DHT.temperature;
  return (temperature);
}

// function that return the temperature as an integer
int get_humidity() {
  int humidity;
  
  DHT.read11(DHT11_PIN);
  humidity = DHT.humidity;
  return (humidity);
}
