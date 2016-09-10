       
//House monitoring framework with Arduino and Raspberry Pi
//Cezar Chirila  
//AllAboutCircuits.com
//epilepsynerd.wordpress.com

#include "ArduinoJson.h"
#include "dht.h"

#define DHT11_PIN A0 //Pin for DHT11 data
#define LED_PIN A1 //PIN for LED

#define ARRAYSIZE(x)  (sizeof(x) / sizeof(x[0]))
#define MAX_ACTUATORS 2
#define MAX_SENSORS 4

struct Sensor {
  int pin;
  String type;
  int value;
};

struct Actuator {
  int pin;
  String status;
  long time;
};

struct Status {
  Actuator actuators[MAX_ACTUATORS];
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
  
  globalStatus.actuators[0] = Actuator{0, "OFF", 0};
  globalStatus.actuators[1] = Actuator{1, "OFF", 0};
  globalStatus.sensors[0] = Sensor{2, "DHT11_TEMP", 23};
  globalStatus.sensors[1] = Sensor{3, "DHT11_HUM", 50};
  globalStatus.sensors[2] = Sensor{4, "SOIL", 30};
  globalStatus.sensors[3] = Sensor{5, "LDR", 500};
  
}

void loop() {
  lastTime = millis();
  
  checkStatus();
  
  getData();
  
  while (Serial.available()) {
    
    String inputJson = Serial.readString();
    
    StaticJsonBuffer<1024> jsonInputBuffer;
    JsonObject& root = jsonInputBuffer.parseObject(inputJson);
    
    if (!root.success()) { 
      Serial.println("ERROR: JSON_PARSE_ERROR");
    } else {
      
      Serial.print("JSON_RECEIVED:");
      root.printTo(Serial);
      Serial.println();
      
      String action = root["action"];
      
      // {"action":"ACTUATOR_ON","time":20, "pin": 0}
      if(action == "ACTUATOR_ON"){
        String time = root["time"];
        int pin = root["pin"];
        
        for (int i = 0; i < ARRAYSIZE(globalStatus.actuators); i++) {
          if(pin == globalStatus.actuators[i].pin){
            globalStatus.actuators[0].status = "ON";
            globalStatus.actuators[0].time = lastTime + (time.toInt() * 1000);
            
            break;
          }
        }
        
      // {"action":"ACTUATOR_OFF", "pin": 0}
      } else if(action == "ACTUATOR_OFF") {
        int pin = root["pin"];
        
        for (int i = 0; i < ARRAYSIZE(globalStatus.actuators); i++) {
          if(pin == globalStatus.actuators[i].pin){
            globalStatus.actuators[i].status = "OFF";
            
            break;
          }
        }
        
      } else {
        Serial.print("INVALID_OPTION");
      }
    
    }
    
  }
  
  sendData();
  
  delay(60000);
    
}

void checkStatus() {
  for (int i = 0; i < ARRAYSIZE(globalStatus.actuators); i++) {
    if(lastTime > globalStatus.actuators[0].time){
      globalStatus.actuators[i].status = "OFF";
      globalStatus.actuators[i].time = 0;
    }
  }
}

void getData() {
  for (int i = 0; i < ARRAYSIZE(globalStatus.sensors); i++) {
    
    if(globalStatus.sensors[i].type == "DHT11_TEMP"){
      globalStatus.sensors[i].value = getTemperatureDHT11(globalStatus.sensors[i].pin, globalStatus.sensors[i].value);
    } else if(globalStatus.sensors[i].type == "DHT11_HUM"){
      globalStatus.sensors[i].value = getHumidityDHT11(globalStatus.sensors[i].pin, globalStatus.sensors[i].value);
    } else if(globalStatus.sensors[i].type == "SOIL"){
      globalStatus.sensors[i].value = getSoil(globalStatus.sensors[i].pin, globalStatus.sensors[i].value);
    } else if(globalStatus.sensors[i].type == "LDR"){
      globalStatus.sensors[i].value = getLDR(globalStatus.sensors[i].pin, globalStatus.sensors[i].value);
    }
    
  }
}

void sendData() {
  Serial.print("STATUS [" + String(lastTime, DEC) + "]: ");
  
  StaticJsonBuffer<1024> jsonOutputBuffer;
  JsonObject& root = jsonOutputBuffer.createObject();
  
  JsonArray& actuators = root.createNestedArray("actuators");

  for (int i = 0; i < ARRAYSIZE(globalStatus.actuators); i++) {
    JsonObject &actuator = actuators.createNestedObject();
    actuator["pin"] = globalStatus.actuators[i].pin;
    actuator["status"] = globalStatus.actuators[i].status;
    actuator["time"] = globalStatus.actuators[i].time;
  }
  
  JsonArray& sensors = root.createNestedArray("sensors");

  for (int i = 0; i < ARRAYSIZE(globalStatus.sensors); i++) {
    
    JsonObject &sensor = sensors.createNestedObject();
    sensor["pin"] = globalStatus.sensors[i].pin;
    sensor["type"] = globalStatus.sensors[i].type;
    sensor["value"] = globalStatus.sensors[i].value;
    
  }
  
  root.printTo(Serial);
  Serial.println();
}

// function that return the temperature as an integer
int getTemperatureDHT11(int pin, int value) {
  
  return value + random(-1, 2);
  /*
  int temperature;
  
  DHT.read11(pin);
  temperature = DHT.temperature;
  return (temperature);
  */
}

// function that return the humidity as an integer
int getHumidityDHT11(int pin, int value) {
  
  return value + random(-1, 2);
  /*
  int humidity;
  
  DHT.read11(pin);
  humidity = DHT.humidity;
  return (humidity);
  */
}

// function that return the Soil meter as an integer
int getSoil(int pin, int value) {
  
  return value + random(-1, 2);
}

// function that return the LDR value as an integer
int getLDR(int pin, int value) {
  
  return value + random(-1, 2);
}
