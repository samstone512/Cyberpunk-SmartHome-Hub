# Cyberpunk-SmartHome-Hub
# 🌆 Cyberpunk Smart Home Hub (osi's home)

![UI Preview](https://raw.githubusercontent.com/samstone512/Cyberpunk-SmartHome-Hub/main/cd.gif)

A highly responsive, cyberpunk-themed local Smart Home controller built with pure C++ and AJAX. This project transforms your Arduino DUE (or ESP8266) into an autonomous web server that can capture and transmit Raw IR signals to legacy appliances (like ACs and TVs), control heavy loads via physical relays, and monitor live environmental data.

## ✨ Features

- **Cyberpunk Glassmorphism UI:** A sleek, dark-mode web interface hosted directly on the microcontroller.
- **AJAX Driven:** Control appliances and fetch sensor data without ever refreshing the web page.
- **Raw IR Blasting:** Custom-built 38kHz PWM generator function that bypasses heavy IR libraries, allowing you to blast extremely long AC remote codes effortlessly.
- **Dual Relay Control:** Hardware-level control for lights, fans, or other 220V/110V appliances.
- **Live DHT11 Integration:** Real-time temperature and humidity feedback displayed on the top sensor bar.
- **Dual Architecture Support:** Fully compatible with both **Arduino DUE + Ethernet Shield + SD Card** and **ESP8266** (standalone Wi-Fi).

## 🛠️ Hardware Requirements

* **Logic Board:** Arduino DUE (with HanRun HR911105A Ethernet Shield) OR ESP8266 (NodeMCU/Wemos D1 Mini).
* **IR Transmitter:** 1x 940nm IR LED (Clear).
* **Transistor:** 1x C945 (or BC547/2N2222) to amplify the IR signal.
* **Resistors:** 1x 100Ω (for LED), 1x 1KΩ (for Transistor Base).
* **Sensors & Relays:** 1x DHT11/DHT22 Module, 1x 2-Channel Relay Module.
* **Storage (DUE Only):** 1x MicroSD Card formatted to `FAT32`.

## ⚙️ Setup & Wiring

### Transistor IR Amplifier Circuit (C945 NPN)
Ensure you wire the NPN transistor correctly to maximize the IR range:
* **Emitter (Left Pin):** Connected to `GND`.
* **Collector (Middle Pin):** Connected to the **Short Leg (-)** of the IR LED.
* **Base (Right Pin):** Connected to `Digital Pin 3` via a **1KΩ Resistor**.
* **IR LED Long Leg (+):** Connected to `5V` via a **100Ω Resistor**.

*(Note: Pin configurations for Relays and DHT11 are mapped at the top of the `.ino` sketch files).*

## 🚀 Installation 

### For Arduino DUE (Ethernet & SD Card)
1. Format a Micro SD card to `FAT32`.
2. Move `index.htm` and `cd.gif` to the root directory of the SD card.
3. Mount the SD card into the Ethernet Shield.
4. Upload `Arduino_DUE_Hub.ino` to your board.
5. Open your Serial Monitor (115200 baud) to find the local IP address.

### For ESP8266 (Wi-Fi)
1. Open `ESP8266_Hub.ino` in the Arduino IDE.
2. Edit lines 8 and 9 to include your local Wi-Fi `SSID` and `PASSWORD`.
3. The UI is embedded directly into the sketch using `PROGMEM`, so no SD card is required! The background GIF is fetched automatically via your raw GitHub link.
4. Upload to the board and navigate to the assigned IP address.

## 📡 Capturing Your Own IR Codes
This repository comes with a placeholder array for an Air Conditioner. To control your own devices:
1. Connect an IR Receiver (VS1838B) to your board.
2. Use a raw IR sniffer sketch to capture your remote's timings.
3. Replace the `acRawCode[]` array in the main sketch with your captured integers.

---
*Built with passion by osi.*
