#include <Access_Point.h>

const char *ssid = "AP-ESP32";
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

static int counter = 0;


void handleRoot(AsyncWebServerRequest *request) {
  // Serve HTML page from SPIFFS
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  String htmlContent = file.readString();
  file.close();

  request->send(200, "text/html", htmlContent);
}

void Init_AP(){

  if(!SPIFFS.begin()){
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  WiFi.softAP(ssid);

  Serial.print("Access Point IP-Adresse: ");
  Serial.println(WiFi.softAPIP());

  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
    if(type == WS_EVT_CONNECT){
      Serial.print("Client (IP: ");
      Serial.print(client->remoteIP());
      Serial.println(") connected.");
    } else if(type == WS_EVT_DISCONNECT){
      Serial.println("Client disconnected.");   
    }
  });

  server.addHandler(&ws);

  server.on("/", HTTP_GET, handleRoot);

  server.begin();
}

void CAN_Mockup() {
  counter++;
  // Mock CAN data for demonstration
  int canId = 0x123 + counter;
  int canData = 0x04;
  int canDataSize = sizeof(canData);

  // Encode CAN data as JSON
  DynamicJsonDocument doc(256);
  doc["canId"] = canId;
  doc["canData"] = canData;
  doc["canDataSize"] = canDataSize;

  // Convert JSON to a string
  String jsonString;
  serializeJson(doc, jsonString);

  // Notify connected WebSocket clients with the JSON-encoded CAN data
  ws.textAll(jsonString);

  delay(1000);
}




