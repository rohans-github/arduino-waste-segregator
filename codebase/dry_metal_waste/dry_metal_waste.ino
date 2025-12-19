#include <CheapStepper.h>
#include <Servo.h>

Servo servo1;
#define ir 5
#define proxi 6
#define buzzer 12
int potPin = A0;
int soil = 0;
int fsoil;

CheapStepper stepper(8, 9, 10, 11);

// Debouncing variables
unsigned long lastProxiTime = 0;
unsigned long lastIrTime = 0;
const unsigned long debounceDelay = 200; // 200ms debounce
bool proxiLastState = HIGH;
bool irLastState = HIGH;

void setup() {
  Serial.begin(9600);
  pinMode(proxi, INPUT_PULLUP);
  pinMode(ir, INPUT);
  pinMode(buzzer, OUTPUT);
  servo1.attach(7);
  stepper.setRpm(17);
  
  // Initialize servo to closed position without cycling
  servo1.write(70);
  delay(1000);
  
  Serial.println("Waste Sorting System Ready");
}

void loop() {
  // Read sensors with debouncing
  bool proxiCurrent = digitalRead(proxi);
  bool irCurrent = digitalRead(ir);
  
  // Check proximity sensor (metal detection)
  if (proxiCurrent == LOW && proxiLastState == HIGH && 
      (millis() - lastProxiTime) > debounceDelay) {
    
    Serial.println("Metal detected!");
    handleMetalWaste();
    lastProxiTime = millis();
  }
  proxiLastState = proxiCurrent;
  
  // Check IR sensor (dry/wet waste detection)
  if (irCurrent == LOW && irLastState == HIGH && 
      (millis() - lastIrTime) > debounceDelay) {
    
    Serial.println("Waste detected - checking moisture...");
    handleOrganicWaste();
    lastIrTime = millis();
  }
  irLastState = irCurrent;
  
  delay(50); // Small delay to prevent overwhelming the loop
}

void handleMetalWaste() {
  tone(buzzer, 1000, 1000);
  
  // Move to metal bin position
  stepper.moveDegreesCW(240);
  delay(1000);
  
  // Open and close servo to release waste
  releaseWaste();
  
  // Return to home position
  stepper.moveDegreesCCW(240);
  delay(1000);
}

void handleOrganicWaste() {
  tone(buzzer, 1000, 500);
  delay(1000);
  
  // Take multiple moisture readings for accuracy
  fsoil = 0;
  for (int i = 0; i < 3; i++) {
    soil = analogRead(potPin);
    soil = constrain(soil, 485, 1023);
    fsoil += map(soil, 485, 1023, 100, 0);
    delay(75);
  }
  fsoil = fsoil / 3;
  
  Serial.print("Moisture: ");
  Serial.print(fsoil);
  Serial.println("%");
  
  if (fsoil > 20) {
    // Wet waste
    Serial.println("MOISTURE DETECTED - Wet waste");
    Serial.println("Moving to wet waste bin...");
    stepper.moveDegreesCW(120);
    delay(1000);
    releaseWaste();
    stepper.moveDegreesCCW(120);
    delay(1000);
    Serial.println("Wet waste disposal complete");
  } else {
    // Dry waste
    Serial.println("NO MOISTURE DETECTED - Dry waste");
    Serial.println("Disposing in dry waste bin...");
    tone(buzzer, 1000, 500);
    delay(1000);
    releaseWaste();
    Serial.println("Dry waste disposal complete");
  }
}

void releaseWaste() {
  servo1.write(280); // Open
  delay(1000);
  servo1.write(70);  // Close
  delay(1000);
}