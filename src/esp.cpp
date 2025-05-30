#ifdef ESP8266_BOARD  // Only compile this file for ESP8266

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>  // Add this library

// Access point credentials
const char* ap_ssid = "ESP_Robot"; // Name of the WiFi network ESP will create
const char* ap_password = "12345678"; // Password (min 8 characters)

WebSocketsServer webSocket = WebSocketsServer(81);

float x = 0.0, y = 0.0;
bool mineDetected = false;

// Function prototype declaration
void webSocketEvent(uint8_t client, WStype_t type, uint8_t * payload, size_t length);

void setup() {
  Serial.begin(115200);
  
  // Set up the ESP8266 as an access point
  WiFi.softAP(ap_ssid, ap_password);
  
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("AP Mode Setup Complete");
  Serial.print("AP SSID: ");
  Serial.println(ap_ssid);
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket server started");
}

void loop() {

  webSocket.loop();

  // Simulate data
  x += 1.0;
  y += 0.5;
  mineDetected = (random(0, 10) > 8);

  // Send data to PC every 500 ms
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 500) {
    lastSend = millis();
    
    // Create JSON document
    StaticJsonDocument<128> doc;
    doc["x"] = x;
    doc["y"] = y;
    doc["mine"] = mineDetected ? 1 : 0;
    
    // Serialize JSON to string
    String json;
    serializeJson(doc, json);
    
    // Send the JSON string
    webSocket.broadcastTXT(json);
  }
}

void webSocketEvent(uint8_t client, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", client);
      break;
      
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(client);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", client, ip[0], ip[1], ip[2], ip[3]);
      }
      break;
      
    case WStype_TEXT:
      {
        // Parse JSON command
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload, length);
        
        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }
        
        // Check if the command field exists
        if (doc.containsKey("cmd")) {
          String command = doc["cmd"];
          Serial.println("Received cmd: " + command);
          
          if (command == "forward") {
            Serial.println("f");
          } else if (command == "backward") {
            Serial.println("b");
          } else if (command == "left") {
            Serial.println("l");
          } else if (command == "right") {
            Serial.println("r");
          } else if (command == "none") {
            Serial.println('s');
          } else {
            Serial.println("Unknown command: " + command);
          }
        } else {
          Serial.println("Invalid JSON: missing 'cmd`' field");
        }
      }
      break;
  }
}

#endif // ESP8266_BOARD