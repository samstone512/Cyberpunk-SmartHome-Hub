#include <Arduino.h>

const int IR_PIN = 11; // پایه متصل به گیرنده مشکی ۳ پایه
uint32_t pulses[500];  // آرایه بزرگ برای ذخیره کدهای طولانی کولر گازی

void setup() {
  Serial.begin(115200);
  pinMode(IR_PIN, INPUT);
  
  while(!Serial); 
  
  Serial.println("=== AC Remote Scanner ===");
  Serial.println("System Ready! Point your AC remote and press POWER...");
}

void loop() {
  int currentState = digitalRead(IR_PIN);
  
  if (currentState == LOW) { 
    unsigned long timer = micros();
    int pulseCount = 0;
    
    // ضبط سیگنال به مدت 250 میلی‌ثانیه (کافی برای طولانی‌ترین کدهای کولر)
    while(micros() - timer < 250000) { 
      int newState = digitalRead(IR_PIN);
      if (newState != currentState) {
        pulses[pulseCount] = micros() - timer;
        timer = micros();                      
        currentState = newState;               
        pulseCount++;
      }
      if(pulseCount >= 500) break; 
    }
    
    if (pulseCount > 10) {
      Serial.println("\n-----------------------------------------");
      Serial.println("-> [SUCCESS] AC Signal Captured!");
      Serial.println("Copy the array below and paste it into the FINAL Web Server code:");
      Serial.println();
      
      Serial.print("uint16_t acRawCode[] = {");
      for(int i = 0; i < pulseCount; i++) {
         Serial.print(pulses[i]);
         if(i < pulseCount - 1) Serial.print(", ");
      }
      Serial.println("};");
      Serial.println("-----------------------------------------");
    }
    
    delay(1500); // جلوگیری از چاپ کدهای تکراری
  }
}
