#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// ==========================================
// 1. Wi-Fi Configuration
// ==========================================
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

ESP8266WebServer server(80);

// ==========================================
// 2. Hardware Pins Configuration (NodeMCU/Wemos)
// ==========================================
const int IR_SEND_PIN = D1;  // IR Transmitter Pin (GPIO5)
const int DHT_PIN = D2;      // DHT Sensor Pin (GPIO4)
const int RELAY1_PIN = D5;   // Relay 1 Pin (GPIO14)
const int RELAY2_PIN = D6;   // Relay 2 Pin (GPIO12)

#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

// ==========================================
// 3. Raw IR Codes (Replace with your own)
// ==========================================
uint16_t acRawCode[] = {
  8400, 4200, 550, 1600, 550, 500, 550, 500, 550, 1600, 550, 500, 550, 1600, 550, 1600, 550
};

// ==========================================
// 4. Web Interface (HTML/CSS/JS)
// ==========================================
// Note: The background GIF is hosted on GitHub for easy deployment without LittleFS
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>osi's home</title>
  <style>
    body {
      margin: 0; padding: 0; height: 100vh;
      display: flex; flex-direction: column; align-items: center; justify-content: center;
      /* Replace YOUR_USERNAME with your actual GitHub username below */
      background: url('https://raw.githubusercontent.com/YOUR_USERNAME/Cyberpunk-SmartHome-Hub/main/cd.gif') no-repeat center center fixed;
      background-size: cover;
      font-family: 'Courier New', Courier, monospace;
    }
    .sensor-bar {
      background: rgba(10, 15, 30, 0.4);
      backdrop-filter: blur(5px); -webkit-backdrop-filter: blur(5px);
      border: 1px solid rgba(0, 255, 255, 0.2);
      border-radius: 30px; padding: 10px 25px;
      color: #0ff; font-weight: bold; font-size: 16px; margin-bottom: 30px;
      box-shadow: 0 0 15px rgba(0, 255, 255, 0.1);
      display: flex; gap: 20px;
    }
    .sensor-value { color: #fff; text-shadow: 0 0 5px #fff; }
    .glass-panel {
      background: rgba(10, 15, 30, 0.5);
      backdrop-filter: blur(5px); -webkit-backdrop-filter: blur(5px);
      border: 1px solid rgba(0, 255, 255, 0.2);
      border-radius: 15px; padding: 20px; text-align: center;
      width: 90%; max-width: 380px;
    }
    h2 { color: #0ff; margin: 0 0 20px 0; letter-spacing: 2px; text-shadow: 0 0 8px #0ff; text-transform: lowercase; }
    .btn-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; }
    .btn-full { grid-column: span 2; }
    .btn {
      padding: 12px; background: transparent; color: #fff;
      border: 1px solid; border-radius: 8px; font-size: 15px; font-weight: bold;
      cursor: pointer; transition: 0.3s;
    }
    .btn-cyan { border-color: #0ff; }
    .btn-cyan:active { background: rgba(0,255,255,0.3); box-shadow: 0 0 15px #0ff; }
    .btn-magenta { border-color: #f0f; }
    .btn-magenta:active { background: rgba(255,0,255,0.3); box-shadow: 0 0 15px #f0f; }
    .btn-yellow { border-color: #ff0; }
    .btn-yellow:active { background: rgba(255,255,0,0.3); box-shadow: 0 0 15px #ff0; }
  </style>
</head>
<body>
  <div class="sensor-bar">
    <div>TEMP: <span class="sensor-value" id="temp">--</span>&deg;C</div>
    <div>HUM: <span class="sensor-value" id="hum">--</span>%</div>
  </div>
  <div class="glass-panel">
    <h2>osi's home</h2>
    <div class="btn-grid">
      <button class="btn btn-magenta btn-full" onclick="sendCommand('ac_on')">&#9889; AC POWER</button>
      <button class="btn btn-cyan" onclick="sendCommand('r1_on')">RELAY 1 ON</button>
      <button class="btn btn-cyan" onclick="sendCommand('r1_off')">RELAY 1 OFF</button>
      <button class="btn btn-yellow" onclick="sendCommand('r2_on')">RELAY 2 ON</button>
      <button class="btn btn-yellow" onclick="sendCommand('r2_off')">RELAY 2 OFF</button>
    </div>
  </div>
  <script>
    function sendCommand(cmd) { fetch('/' + cmd).catch(e => console.log(e)); }
    function fetchSensors() {
      fetch('/api_sensors')
        .then(response => response.json())
        .then(data => {
          document.getElementById('temp').innerText = data.t;
          document.getElementById('hum').innerText = data.h;
        }).catch(e => console.log(e));
    }
    fetchSensors(); setInterval(fetchSensors, 5000);
  </script>
</body>
</html>
)rawliteral";

// ==========================================
// 5. Functions
// ==========================================
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

void setup() {
  Serial.begin(115200);
  
  pinMode(IR_SEND_PIN, OUTPUT);
  digitalWrite(IR_SEND_PIN, LOW);
  
  pinMode(RELAY1_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, HIGH);
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY2_PIN, HIGH);

  dht.begin();

  Serial.println("\nConnecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected! IP Address: ");
  Serial.println(WiFi.localIP());

  // Web Server Routes
  server.on("/", []() {
    server.send(200, "text/html", index_html);
  });

  server.on("/ac_on", []() {
    sendCustomIR(acRawCode, sizeof(acRawCode) / sizeof(acRawCode[0]));
    server.send(200, "text/plain", "OK");
  });

  server.on("/r1_on", []() { digitalWrite(RELAY1_PIN, LOW); server.send(200, "text/plain", "OK"); });
  server.on("/r1_off", []() { digitalWrite(RELAY1_PIN, HIGH); server.send(200, "text/plain", "OK"); });
  server.on("/r2_on", []() { digitalWrite(RELAY2_PIN, LOW); server.send(200, "text/plain", "OK"); });
  server.on("/r2_off", []() { digitalWrite(RELAY2_PIN, HIGH); server.send(200, "text/plain", "OK"); });

  server.on("/api_sensors", []() {
    int h = dht.readHumidity();
    int t = dht.readTemperature();
    String json = "{\"t\":";
    json += isnan(t) ? "0" : String(t);
    json += ", \"h\":";
    json += isnan(h) ? "0" : String(h);
    json += "}";
    server.send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  server.handleClient();
}
