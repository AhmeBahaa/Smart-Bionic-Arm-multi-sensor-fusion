// ============================================================
//  ESP32-C3 — Bionic Arm "Brain" + Blynk IoT
//  Serial to Arduino: TX=GPIO21 → D13(Arduino RX)
//                     RX=GPIO20 ← D12(Arduino TX)
//  MPU6050: SDA=GPIO8, SCL=GPIO9
//  Armband Matrix: cols={5,6} rows={4,3}
//  LED indicator: GPIO2
//
//  Blynk Datastreams:
//    V0  = Tmp  (Label °C)
//    V1  = IR   (Label)
//    V2  = FSR  (Label)
//    V3  = open/close switch (1=close, 0=open)
//    V4  = Battery (Gauge, Volts)
//    V5  = aband1 | V6 = aband2
//    V7  = speed slider (moveTime ms)
//    V8  = aband3 | V9 = aband4
//    V10 = LED right wrist (stepper right)
//    V11 = LED balm open/close
//    V12 = LED left wrist (stepper left)
// ============================================================

#define BLYNK_TEMPLATE_ID   "TMPL24CaerRLz"
#define BLYNK_TEMPLATE_NAME "esptoarduimo"
#define BLYNK_AUTH_TOKEN    "BMhzVizgZM-JNCi_4Ay0MUku_t_Rtpdc"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>

// ─── WiFi ─────────────────────────────────────────────────────
char ssid[] = "Vodafone_bahaa_VDSL_E6AA_2.4G";
char pass[] = "SSABD5BOB#1QAZ@2WSX@3ed";

// ─── Pins ─────────────────────────────────────────────────────
#define LED_PIN  2
#define RX_PIN  20   // يستقبل من Arduino D12
#define TX_PIN  21   // يرسل لـ Arduino D13

// Armband Matrix
const int colPins[] = {5, 6};
const int rowPins[] = {4, 3};

// MPU6050
#define MPU_ADDR 0x68

// Battery (calculated on Arduino side, sent as raw ADC)
#define BAT_MULTIPLIER 3.1276f
#define ADC_REF_ARD    5.0f   // Arduino reference voltage
#define ARD_ADC_MAX    1024.0f

// ─── Armband ──────────────────────────────────────────────────
float filteredMatrix[2][2] = {{0,0},{0,0}};
const int   NOISE_FLOOR  = 100;
const int   MAX_PRESSURE = 3000;
const float FILTER_ALPHA = 0.08f;
const float DEADBAND     = 5.0f;

// ─── Sensor Data from Arduino ─────────────────────────────────
int currIR = 0, currFSR = 0, currTemp = 0, currBatRaw = 0;

// ─── Logic State ──────────────────────────────────────────────
bool          irActive    = false;
unsigned long irTimer     = 0;
#define IR_HOLD_MS  3000   // 3 ثواني ← غيّر هنا لو عاوز وقت تاني

bool          fsrClosed   = false;
unsigned long fsrOpenAt   = 0;   // وقت ما هيتفتح الـ FSR
bool          fsrPending  = false; // ننتظر الـ 3 ثواني للفتح

bool          armbandClosed  = false;
unsigned long armbandOpenAt  = 0;
bool          armbandPending = false;

// LED indicator state for Blynk
bool stepperRight = false;
bool stepperLeft  = false;
bool palmClosed   = false;

BlynkTimer timer;

// ============================================================
//  Blynk Callbacks
// ============================================================
BLYNK_WRITE(V3) {
  // السويتش: يفتح/يقفل كل الأصابع
  int val = param.asInt();
  if (val == 1) {
    Serial1.println("A:1");
    palmClosed = true;
  } else {
    Serial1.println("A:0");
    palmClosed = false;
  }
  Blynk.virtualWrite(V11, palmClosed ? 1 : 0);
}
// --- تحكم فردي في الأصابع بنفس ستايل كودك القديم ---

// صباع السبابة (Index)
BLYNK_WRITE(V13) {
  if (param.asInt() == 1) Serial1.println("M1:1");
  else Serial1.println("M1:0");
}

// الصباع الأوسط (Middle)
BLYNK_WRITE(V14) {
  if (param.asInt() == 1) Serial1.println("M2:1");
  else Serial1.println("M2:0");
}

// صباع البنصر (Ring)
BLYNK_WRITE(V15) {
  if (param.asInt() == 1) Serial1.println("M3:1");
  else Serial1.println("M3:0");
}

// صباع الخنصر (Pinky)
BLYNK_WRITE(V16) {
  if (param.asInt() == 1) Serial1.println("M4:1");
  else Serial1.println("M4:0");
}

// محرك الإبهام الأول (Thumb 1)
BLYNK_WRITE(V17) {
  if (param.asInt() == 1) Serial1.println("M5:1");
  else Serial1.println("M5:0");
}

// محرك الإبهام الثاني (Thumb 2)
BLYNK_WRITE(V18) {
  if (param.asInt() == 1) Serial1.println("M6:1");
  else Serial1.println("M6:0");
}
BLYNK_WRITE(V7) {
  // السلايدر: تعديل سرعة الموتور
  int t = param.asInt();
  Serial1.println("T:" + String(t));
}

// ============================================================
//  Setup
// ============================================================
void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // MPU6050
  Wire.begin(8, 9);
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); Wire.write(0);
  Wire.endTransmission(true);

  // Armband pins
  for (int i = 0; i < 2; i++) {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], LOW);
  }
  analogReadResolution(12);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(1000L, updateBlynkDash);
  timer.setInterval(30L,   readArmband);

  Serial.println(">>> ESP32-C3 Brain Online");
}

// ============================================================
//  Loop
// ============================================================
void loop() {
  Blynk.run();
  timer.run();

  processArduinoData();
  checkMPU();
  logicIR();
  logicFSR();
  logicArmbandTimers(); // check if 3s passed → open
}

// ============================================================
//  استقبال بيانات الأردوينو: D,IR,FSR,TEMP,BAT
// ============================================================
void processArduinoData() {
  if (!Serial1.available()) return;
  String data = Serial1.readStringUntil('\n');
  data.trim();
  if (!data.startsWith("D,")) return;

  int c1 = data.indexOf(',', 2);
  int c2 = data.indexOf(',', c1 + 1);
  int c3 = data.indexOf(',', c2 + 1);
  if (c1 < 0 || c2 < 0 || c3 < 0) return;

  currIR     = data.substring(2,      c1).toInt();
  currFSR    = data.substring(c1 + 1, c2).toInt();
  currTemp   = data.substring(c2 + 1, c3).toInt();
  currBatRaw = data.substring(c3 + 1).toInt();
}

// ============================================================
//  تحديث الـ Blynk Dashboard
// ============================================================
void updateBlynkDash() {
  // Battery: Arduino ADC (0-1023) → voltage
  float pinV   = (currBatRaw / ARD_ADC_MAX) * ADC_REF_ARD;
  float batV   = pinV * BAT_MULTIPLIER;

  Blynk.virtualWrite(V0, currTemp);
  Blynk.virtualWrite(V1, currIR);
  Blynk.virtualWrite(V2, currFSR);
  Blynk.virtualWrite(V4, batV);

  Blynk.virtualWrite(V5, (int)filteredMatrix[0][0]);
  Blynk.virtualWrite(V6, (int)filteredMatrix[1][0]);
  Blynk.virtualWrite(V8, (int)filteredMatrix[0][1]);
  Blynk.virtualWrite(V9, (int)filteredMatrix[1][1]);

  // LEDs حالة السستم
  Blynk.virtualWrite(V10, stepperRight ? 1 : 0);
  Blynk.virtualWrite(V11, palmClosed   ? 1 : 0);
  Blynk.virtualWrite(V12, stepperLeft  ? 1 : 0);
}

// ============================================================
//  قراءة الـ Armband
// ============================================================
void readArmband() {
  for (int c = 0; c < 2; c++) {
    digitalWrite(colPins[c], HIGH);
    delayMicroseconds(200);
    for (int r = 0; r < 2; r++) {
      int   raw = analogRead(rowPins[r]);
      float val = (float)map(raw, NOISE_FLOOR, MAX_PRESSURE, 0, 100);
      val = constrain(val, 0.0f, 100.0f);
      if (val < DEADBAND) val = 0;
      filteredMatrix[c][r] = (val * FILTER_ALPHA) +
                             (filteredMatrix[c][r] * (1.0f - FILTER_ALPHA));
    }
    digitalWrite(colPins[c], LOW);
    delayMicroseconds(100);
  }

  // P1 (أقوى نقطة في المصفوفة) تتحكم
  float p1 = filteredMatrix[0][0];
  static unsigned long lastArm = 0;

  if (p1 > 30 && !armbandClosed && !armbandPending &&
      (millis() - lastArm > 1000)) {
    // قفل الـ FSR group: f2, f3, th1, th2
    Serial1.println("F:1");
    armbandClosed  = true;
    armbandPending = true;
    armbandOpenAt  = millis() + IR_HOLD_MS; // فتح بعد 3 ثواني
    palmClosed     = true;
    digitalWrite(LED_PIN, HIGH);
    lastArm = millis();
    Serial.println(">>> Armband: CLOSE");
  }
}

// ============================================================
//  منطق الـ Timers (فتح بعد 3 ثواني)
// ============================================================
void logicArmbandTimers() {
  // Armband → فتح بعد 3 ثواني
  if (armbandPending && millis() >= armbandOpenAt) {
    Serial1.println("F:0");
    armbandClosed  = false;
    armbandPending = false;
    palmClosed     = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println(">>> Armband: OPEN (3s elapsed)");
  }
}

// ============================================================
//  منطق حساس الـ IR
//  يقفل f1, f2, f3, f4 → يفتحهم بعد 3 ثواني
// ============================================================
void logicIR() {
  if (currIR > 599 && !irActive) {
    Serial1.println("I:1");
    irActive  = true;
    irTimer   = millis();
    digitalWrite(LED_PIN, HIGH);
    Serial.println(">>> IR: CLOSE (f1-f4)");
  }

  if (irActive && (millis() - irTimer >= IR_HOLD_MS)) {
    Serial1.println("I:0");
    irActive = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println(">>> IR: OPEN (3s elapsed)");
  }
}

// ============================================================
//  منطق حساس الـ FSR
//  يقفل f2, f3, th1, th2 → يفتحهم بعد 3 ثواني
// ============================================================
void logicFSR() {
  static unsigned long lastFSR = 0;

  if (currFSR > 400 && !fsrClosed && !fsrPending &&
      (millis() - lastFSR > 1000)) {
    Serial1.println("F:1");
    fsrClosed   = true;
    fsrPending  = true;
    fsrOpenAt   = millis() + IR_HOLD_MS; // فتح بعد 3 ثواني
    palmClosed  = true;
    lastFSR     = millis();
    digitalWrite(LED_PIN, HIGH);
    Serial.println(">>> FSR: CLOSE");
  }

  if (fsrPending && millis() >= fsrOpenAt) {
    Serial1.println("F:0");
    fsrClosed  = false;
    fsrPending = false;
    palmClosed = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println(">>> FSR: OPEN (3s elapsed)");
  }
}

// ============================================================
//  MPU6050 — Tilt Control
// ============================================================
void checkMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return;

  Wire.requestFrom(MPU_ADDR, 4, true);
  if (Wire.available() < 4) return;

  int16_t AcX = Wire.read() << 8 | Wire.read();
  int16_t AcY = Wire.read() << 8 | Wire.read();

  if (abs(AcX) > 32000 || abs(AcY) > 32000) return; // noise filter

  // ── Stepper: إمالة محور Y ──
  static unsigned long lastStep = 0;
  if (millis() - lastStep > 20) {
    if (AcY > 9000) {
      Serial1.println("S:1");
      stepperRight = true;
      stepperLeft  = false;
      lastStep = millis();
    } else if (AcY < -9000) {
      Serial1.println("S:0");
      stepperLeft  = true;
      stepperRight = false;
      lastStep = millis();
    } else {
      stepperRight = false;
      stepperLeft  = false;
    }
  }

  // ── Gesture: إمالة محور X → فتح/قفل كل الأصابع ──
  static unsigned long lastGesture = 0;
  if (millis() - lastGesture > 500) {
    if (AcX > 18000) {
      Serial1.println("A:1");
      palmClosed = true;
      digitalWrite(LED_PIN, HIGH);
      lastGesture = millis();
      Serial.println(">>> MPU: CLOSE ALL");
    } else if (AcX < -18000) {
      Serial1.println("A:0");
      palmClosed = false;
      digitalWrite(LED_PIN, LOW);
      lastGesture = millis();
      Serial.println(">>> MPU: OPEN ALL");
    }
  }
}
