#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <DHT.h>

// --- تنظیمات شبکه ---
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177); 
EthernetServer server(80);

// --- تنظیمات سخت‌افزار و پین‌ها ---
const int SD_CS_PIN = 4;     // مموری کارت
const int IR_SEND_PIN = 3;   // فرستنده مادون قرمز

const int DHT_PIN = 5;       // پین دیتای سنسور دما و رطوبت
#define DHT_TYPE DHT11       // اگر سنسورت DHT22 است، این خط را به DHT22 تغییر بده
DHT dht(DHT_PIN, DHT_TYPE);

const int RELAY1_PIN = 6;    // رله اول
const int RELAY2_PIN = 7;    // رله دوم

// =========================================================
// آرایه خام برای دکمه کولر گازی (اعداد خود را جایگزین کنید)
// =========================================================
uint16_t acRawCode[] = {
  8400, 4200, 550, 1600, 550, 500, 550, 500, 550, 1600, 550, 500, 550, 1600, 550, 1600, 550 
};

// تابع ارسال سیگنال مادون قرمز
void sendCustomIR(const uint16_t buf[], uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    if (i % 2 == 0) {
      uint32_t startTimer = micros();
      while (micros() - startTimer < buf[i]) {
        digitalWrite(IR_SEND_PIN, HIGH);
        delayMicroseconds(13);
        digitalWrite(IR_SEND_PIN, LOW);
        delayMicroseconds(13);
      }
    } else {
      digitalWrite(IR_SEND_PIN, LOW);
      delayMicroseconds(buf[i]);
    }
  }
  digitalWrite(IR_SEND_PIN, LOW); 
}

// تابع ارسال فایل از مموری کارت
void serveFile(EthernetClient &client, const char* filename, const char* contentType) {
  File dataFile = SD.open(filename);
  if (dataFile) {
    client.println("HTTP/1.1 200 OK");
    client.print("Content-Type: ");
    client.println(contentType);
    client.println("Connection: close");
    client.println();

    byte buffer[64]; 
    while (dataFile.available()) {
      int bytesRead = dataFile.read(buffer, sizeof(buffer));
      client.write(buffer, bytesRead);
    }
    dataFile.close();
  } else {
    client.println("HTTP/1.1 404 Not Found");
    client.println("Connection: close");
    client.println();
  }
}

// تابع ارسال پاسخ کوتاه (برای جلوگیری از رفرش شدن صفحه مرورگر)
void sendActionOK(EthernetClient &client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/plain");
  client.println("Connection: close");
  client.println();
  client.println("OK");
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // تنظیمات پین‌های خروجی
  pinMode(IR_SEND_PIN, OUTPUT);
  digitalWrite(IR_SEND_PIN, LOW);

  // ماژول‌های رله معمولا با LOW روشن و با HIGH خاموش می‌شوند
  // به همین دلیل در ابتدای کار آن‌ها را HIGH می‌کنیم تا خاموش بمانند
  pinMode(RELAY1_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, HIGH); 
  
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY2_PIN, HIGH);

  // راه‌اندازی سنسور دما
  dht.begin();

  Serial.println("=== Booting osi's home Hub ===");

  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD Card FAILED!");
    while (true); 
  }

  Ethernet.init(10);
  Ethernet.begin(mac, ip);
  
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield not found!");
    while (true);
  }

  server.begin();
  Serial.print("System LIVE at: http://");
  Serial.println(Ethernet.localIP());
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    String request = "";
    boolean currentLineIsBlank = true;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (request.length() < 100) {
          request += c;
        }
        
        if (c == '\n' && currentLineIsBlank) {
          
          // --- ۱. دستورات کنترلی (دکمه‌ها) ---
          if (request.indexOf("GET /ac_on") >= 0) {
            Serial.println("[ACTION] AC Power");
            sendCustomIR(acRawCode, sizeof(acRawCode) / sizeof(acRawCode[0]));
            sendActionOK(client);
          } 
          else if (request.indexOf("GET /r1_on") >= 0) {
            Serial.println("[ACTION] Relay 1 ON");
            digitalWrite(RELAY1_PIN, LOW); // فعال شدن رله
            sendActionOK(client);
          }
          else if (request.indexOf("GET /r1_off") >= 0) {
            Serial.println("[ACTION] Relay 1 OFF");
            digitalWrite(RELAY1_PIN, HIGH); // غیرفعال شدن رله
            sendActionOK(client);
          }
          else if (request.indexOf("GET /r2_on") >= 0) {
            Serial.println("[ACTION] Relay 2 ON");
            digitalWrite(RELAY2_PIN, LOW);
            sendActionOK(client);
          }
          else if (request.indexOf("GET /r2_off") >= 0) {
            Serial.println("[ACTION] Relay 2 OFF");
            digitalWrite(RELAY2_PIN, HIGH);
            sendActionOK(client);
          }
          
          // --- ۲. درخواست اطلاعات سنسور (مخفیانه در پس‌زمینه توسط موبایل انجام می‌شود) ---
          else if (request.indexOf("GET /api_sensors") >= 0) {
            // خواندن دما و رطوبت بدون اعشار
            int h = dht.readHumidity();
            int t = dht.readTemperature();
            
            // ارسال اطلاعات با فرمت استاندارد JSON
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            client.println("Connection: close");
            client.println();
            
            client.print("{\"t\":");
            // اگر سنسور متصل نباشد یا ارور بدهد، عدد 0 می‌فرستیم
            if(isnan(t)) client.print(0); else client.print(t);
            client.print(", \"h\":");
            if(isnan(h)) client.print(0); else client.print(h);
            client.println("}");
          }
          
          // --- ۳. درخواست فایل‌های گرافیکی از مموری کارت ---
          else if (request.indexOf("GET /cd.gif") >= 0) {
            serveFile(client, "cd.gif", "image/gif");
          } 
          else {
            serveFile(client, "index.htm", "text/html");
          }
          
          break; 
        }
        
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop(); 
  }
}