#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <BH1750.h>
#include <DHT.h>
#include <ArduinoJson.h>

// --- CẤU HÌNH WIFI (KẾT NỐI TRỰC TIẾP) ---
const char* ssid = "703 - 704 - 719";      // Điền tên WiFi nhà bạn vào đây
const char* password = "23456789"; // Điền mật khẩu WiFi vào đây

// --- CẤU HÌNH HIVEMQ CLOUD ---
const char* mqtt_server = "41dbbd06096c41eaa83d9d7dac2297f8.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;

// ĐIỀN TÀI KHOẢN/MẬT KHẨU HIVEMQ CỦA BẠN VÀO ĐÂY
const char* mqtt_username = "esp32"; 
const char* mqtt_password = "VuTran2000@"; 

// --- CÁC TOPIC MQTT ĐỂ NHẬN DỮ LIỆU ---
const char* topic_light = "home/livingroom/light";
const char* topic_water = "home/garden/irrigation";
const char* topic_auto  = "esp32/auto"; // Topic nhận lệnh auto

// --- CẤU HÌNH CHÂN (PINS) ---
#define DHTPIN 4    
#define DHTTYPE DHT11
#define I2C_SDA 16    
#define I2C_SCL 17    
#define LED1_PIN 27   // LED xanh
#define LED2_PIN 12   // Relay / Bơm

// --- KHỞI TẠO ĐỐI TƯỢNG ---
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;
WiFiClientSecure espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

// Biến quản lý chế độ Auto
bool isAutoMode = true; // Mặc định bật chế độ tự động khi khởi động

// --- HÀM XỬ LÝ KHI NHẬN ĐƯỢC DỮ LIỆU MQTT ---
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Đã nhận tin nhắn từ Topic [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("Lỗi phân tích JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // XỬ LÝ ĐIỀU KHIỂN THỦ CÔNG (ĐÈN)
  if (String(topic) == topic_light) {
    if (doc.containsKey("led")) {
      isAutoMode = false; // TỰ ĐỘNG TẮT CHẾ ĐỘ AUTO ĐỂ ƯU TIÊN LỆNH THỦ CÔNG
      bool ledState = doc["led"];
      if (ledState == false) {
        digitalWrite(LED1_PIN, LOW);
        Serial.println("=> (MQTT) Đã TẮT LED Xanh (D27). Chế độ Auto hiện bị ngắt.");
      } else {
        digitalWrite(LED1_PIN, HIGH);
        Serial.println("=> (MQTT) Đã BẬT LED Xanh (D27). Chế độ Auto hiện bị ngắt.");
      }
    }
  }
  // XỬ LÝ ĐIỀU KHIỂN THỦ CÔNG (BƠM/RELAY)
  else if (String(topic) == topic_water) {
    if (doc.containsKey("water")) {
      isAutoMode = false; // TỰ ĐỘNG TẮT CHẾ ĐỘ AUTO ĐỂ ƯU TIÊN LỆNH THỦ CÔNG
      bool waterState = doc["water"];
      if (waterState == false) {
        digitalWrite(LED2_PIN, LOW);
        Serial.println("=> (MQTT) Đã TẮT Relay (D12). Chế độ Auto hiện bị ngắt.");
      } else {
        digitalWrite(LED2_PIN, HIGH);
        Serial.println("=> (MQTT) Đã BẬT Relay (D12). Chế độ Auto hiện bị ngắt.");
      }
    }
  }
  // XỬ LÝ CẬP NHẬT TRẠNG THÁI AUTO
  else if (String(topic) == topic_auto) {
    if (doc.containsKey("auto")) {
      isAutoMode = doc["auto"];
      Serial.print("=> (MQTT) Chế độ Tự Động hiện đang: ");
      Serial.println(isAutoMode ? "BẬT" : "TẮT");
    }
  }
}

// Hàm kết nối lại MQTT 
void reconnect() {
  while (!client.connected()) {
    Serial.print("Đang kết nối MQTT... ");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    yield(); 

    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Thành công!");
      client.subscribe(topic_light);
      client.subscribe(topic_water);
      client.subscribe(topic_auto); 
    } else {
      Serial.print("Thất bại, mã lỗi: ");
      Serial.print(client.state());
      Serial.println(" Thử lại sau 5 giây");
      
      for(int i = 0; i < 50; i++) {
        delay(100);
        yield();
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- BẮT ĐẦU KHỞI TẠO ---");

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);

  dht.begin();
  Wire.begin(I2C_SDA, I2C_SCL);

  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("BH1750: OK");
  } else {
    Serial.println("BH1750: Lỗi!");
  }

  // --- KẾT NỐI WIFI ---
  Serial.println();
  Serial.print("Đang kết nối vào WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Đợi cho đến khi kết nối thành công
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nĐã kết nối WiFi thành công!");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());

  espClient.setInsecure();
  client.setBufferSize(1024); 
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); 

  unsigned long now = millis();

  // --- CẬP NHẬT CẢM BIẾN MỖI 3 GIÂY ---
  if (now - lastMsg > 3000) {
    lastMsg = now;

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float lux = lightMeter.readLightLevel();

    if (!isnan(h) && !isnan(t)) {
      Serial.print("Nhiệt độ: "); Serial.print(t); Serial.print(" °C | ");
      Serial.print("Độ ẩm: "); Serial.print(h); Serial.print(" % | ");
      Serial.print("Ánh sáng: "); Serial.print(lux); Serial.println(" lx");

      client.publish("esp32/nhietdo", String(t).c_str());
      client.publish("esp32/doam", String(h).c_str());
      client.publish("esp32/anhsang", String(lux).c_str());

      // Thực thi logic tự động chỉ khi isAutoMode == true
      if (isAutoMode) {
        
        // --- 1. CƠ CHẾ TỰ ĐỘNG LED XANH (D27) ---
        if (lux < 20.0) {
          if (digitalRead(LED1_PIN) == LOW) { 
            digitalWrite(LED1_PIN, HIGH);
            Serial.println("=> (AUTO) Ánh sáng < 20lx: Đã tự động BẬT LED Xanh (D27).");
          }
        } 
        else if (lux > 500.0) {
          if (digitalRead(LED1_PIN) == HIGH) { 
            digitalWrite(LED1_PIN, LOW);
            Serial.println("=> (AUTO) Ánh sáng > 500lx: Đã tự động TẮT LED Xanh (D27).");
          }
        }

        // --- 2. CƠ CHẾ TỰ ĐỘNG RELAY (D12) ---
        if (t > 30.0) {
          if (digitalRead(LED2_PIN) == LOW) {
            digitalWrite(LED2_PIN, HIGH);
            Serial.println("=> (AUTO) Nhiệt độ > 30°C: Đã tự động BẬT Relay (D12).");
          }
        } 
        else if (t < 29.0) { // Thiết lập ngưỡng < 29°C để tắt, tránh tình trạng bơm bật/tắt liên tục khi nhiệt độ quanh mức 30°C
          if (digitalRead(LED2_PIN) == HIGH) {
            digitalWrite(LED2_PIN, LOW);
            Serial.println("=> (AUTO) Nhiệt độ < 29°C: Đã tự động TẮT Relay (D12).");
          }
        }
      }
      
    } else {
       Serial.println("Lỗi: Không đọc được dữ liệu từ DHT11!");
    }
  }
}