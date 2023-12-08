#include <Access_Point.h>

/* TODO:
- Konfiguration der CAN-Schnittstelle möglich machen
- CAN-Botschaft auf Bus legen möglich machen  */

const char *ssid = "CAN2WLAN";
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

WiFiUDP udp;

static int counter = 0;

typedef struct {
    std::vector<uint8_t> magic = {0x49, 0x53, 0x4f, 0x31, 0x31, 0x38, 0x39, 0x38};
    std::vector<uint8_t> version = {0x01};
    std::vector<uint8_t> framesCnt = {0x01};
    std::vector<uint8_t> options = {0x01, 0x60, 0xff};
    std::vector<uint8_t> identifier; 
    std::vector<uint8_t> frameLength; 
    std::vector<uint8_t> data; 
    std::vector<uint8_t> extFlag; 
    std::vector<uint8_t> rtrFlag;
} CANFrame;

void handleRoot(AsyncWebServerRequest *request) {
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


std::vector<uint8_t> CANFrame_to_Vec(CANFrame canFrame ) {
    std::vector<uint8_t> canFrameVec;

    // Assemble CAN Frame Vector
    canFrameVec.insert(canFrameVec.end(), canFrame.magic.begin(), canFrame.magic.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.version.begin(), canFrame.version.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.framesCnt.begin(), canFrame.framesCnt.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.identifier.begin(), canFrame.identifier.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.frameLength.begin(), canFrame.frameLength.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.data.begin(), canFrame.data.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.extFlag.begin(), canFrame.extFlag.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.rtrFlag.begin(), canFrame.rtrFlag.end());
    canFrameVec.insert(canFrameVec.end(), canFrame.options.begin(), canFrame.options.end());

    return canFrameVec;
}

String vectorToHexString(const std::vector<uint8_t>& vec) {
    String result;
    for (size_t i = 0; i < vec.size(); ++i) {
        // Convert each value to a two-digit hexadecimal string
        result += String(vec[i], HEX);

        // Add a space between every two characters, except for the last character
        if (i < vec.size() - 1) {
            result += " ";
        }
    }
    return result;
}

String CANFrame_to_JSON (CANFrame canFrame){

  DynamicJsonDocument doc(256);

  doc["identifier"] = vectorToHexString(canFrame.identifier);
  doc["frameLength"] = vectorToHexString(canFrame.frameLength);
  doc["data"] = vectorToHexString(canFrame.data);
  doc["extFlag"] = vectorToHexString(canFrame.extFlag);
  doc["rtrFlag"] = vectorToHexString(canFrame.rtrFlag);

  String CANString;
  serializeJson(doc, CANString);

  return CANString;
}


CANFrame Rand_CAN_Frame() {

  CANFrame randCanFrame;

  // Seed the random number generator with the current time
  esp_random() % (UINT32_MAX - 1);
  vTaskDelay(1);  // Ensuring a delay to let the generator initialize

  std::vector<uint8_t> identifier = {
      static_cast<uint8_t>(esp_random() % 0xFF + 1),
      static_cast<uint8_t>(esp_random() % 0xFF + 1),
      static_cast<uint8_t>(esp_random() % 0xFF + 1),
      static_cast<uint8_t>(esp_random() % 0xFF + 1)
  };

  std::vector<uint8_t> frameLength = {0x08};
  std::vector<uint8_t> data = {
    static_cast<uint8_t>(esp_random() % 0xFF + 1),
    static_cast<uint8_t>(esp_random() % 0xFF + 1),
    static_cast<uint8_t>(esp_random() % 0xFF + 1),
    static_cast<uint8_t>(esp_random() % 0xFF + 1),
    static_cast<uint8_t>(esp_random() % 0xFF + 1),
    static_cast<uint8_t>(esp_random() % 0xFF + 1),
    static_cast<uint8_t>(esp_random() % 0xFF + 1),
    static_cast<uint8_t>(esp_random() % 0xFF + 1)
  };

  std::vector<uint8_t> extFlag = {0x10};
  std::vector<uint8_t> rtrFlag = {0x00};

  randCanFrame.identifier = identifier;
  randCanFrame.frameLength = frameLength;
  randCanFrame.data = data;
  randCanFrame.extFlag = extFlag;
  randCanFrame.rtrFlag = rtrFlag;

  return randCanFrame;
}

void BroadcastUDP(std::vector<uint8_t> data, size_t len) {
  udp.beginPacket("255.255.255.255", 1060);
  udp.write(data.data(), len);
  udp.endPacket();
}


void sendRandCANData() {

  CANFrame canFrame = Rand_CAN_Frame();

  std::vector<uint8_t> canFrameVec = CANFrame_to_Vec(canFrame);
  String canFrameString = CANFrame_to_JSON(canFrame);
      
  size_t canFrameLen = canFrameVec.size();

  BroadcastUDP(canFrameVec, canFrameLen);
  ws.textAll(canFrameString);

  delay(1000);
}


