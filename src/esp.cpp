#ifdef ESP8266_BOARD  // Only compile this file for ESP8266

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

WebSocketsServer webSocket = WebSocketsServer(81);

float x = 0.0, y = 0.0;
bool mineDetected = false;

// Function prototype declaration
void webSocketEvent(uint8_t client, WStype_t type, uint8_t * payload, size_t length);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected. IP:");
  Serial.println(WiFi.localIP());

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
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
    String json = "{\"x\":" + String(x, 1) +
                  ", \"y\":" + String(y, 1) +
                  ", \"mine\":" + String(mineDetected ? 1 : 0) + "}";
    webSocket.broadcastTXT(json);
  }
}

void webSocketEvent(uint8_t client, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String cmd = String((char *)payload);
    Serial.println("Command: " + cmd);

    if (cmd == "forward") {
      // Move forward
    } else if (cmd == "backward") {
      // Turn left
    } else if (cmd == "left") {
      // Turn left
    } else if (cmd == "right") {
      // Stop motors
    }
    } else if (cmd == "right") {
      // Stop motors
    }
  }
}

#endif // ESP8266_BOARD