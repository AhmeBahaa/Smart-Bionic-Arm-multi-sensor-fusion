// ============================================================
//  ARDUINO UNO — Bionic Arm "Muscles" (Modified with Detach Logic)
// ============================================================

#include <SoftwareSerial.h>
#include <ServoTimer2.h>
#include <Stepper.h>

SoftwareSerial espSerial(13, 12); // RX=D13, TX=D12

// ─── Servo Settings ───────────────────────────────────────────
#define STOP_US    1500   // microseconds للوقوف التام
#define CLOSE_US   2000   // microseconds للقفل 
#define OPEN_US    1000   // microseconds للفتح

// ─── Pins ─────────────────────────────────────────────────────
#define PIN_F1  3 // INDEX
#define PIN_F2  4 //MIDDLE
#define PIN_F3  5 //RING
#define PIN_F4  2 //PINKY
#define PIN_TH1 6   // THUMB 1
#define PIN_TH2 7  //  THUMB 2

#define STEP_IN1  8
#define STEP_IN2 10
#define STEP_IN3  9
#define STEP_IN4 11

#define PIN_IR   A0
#define PIN_FSR  A1
#define PIN_TEMP A2
#define PIN_BAT  A3

#define BAT_MULTIPLIER 3.1276f
#define ADC_REF        5.0f

// ─── Struct ───────────────────────────────────────────────────
struct Finger {
  ServoTimer2 motor;
  int           pin;
  unsigned long startTime;
  bool           isMoving;
  bool           isClosed;
};

// ─── Objects ──────────────────────────────────────────────────
Finger f1, f2, f3, f4, th1, th2;//3 INDEX F1
//4MIDDLE F2 
 //5RING F3
 //2PINKY F4
// 6THUMB 1 TH1
//  7THUMB 2  TH2





Stepper myStepper(2048, STEP_IN1, STEP_IN3, STEP_IN2, STEP_IN4);

int           moveTime = 2200;
unsigned long lastSend = 0;
float filteredTemp = 27.0;

// ─── Setup ────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  espSerial.begin(9600);

  // إعداد الأصابع وفصلها في البداية لضمان السكون
  setupFinger(f1, PIN_F1);
  setupFinger(f2, PIN_F2);
  setupFinger(f3, PIN_F3);
  setupFinger(f4, PIN_F4);
  setupFinger(th1, PIN_TH1);
  setupFinger(th2, PIN_TH2);

  myStepper.setSpeed(19); // السرعة اللي اتفقنا عليها للقوة والدقة
  Serial.println("Arduino Muscles Ready with Detach Logic!");
}

// ─── Loop ─────────────────────────────────────────────────────
void loop() {
  if (millis() - lastSend > 200) {
    int irRaw  = analogRead(PIN_IR);
    int fsrRaw = analogRead(PIN_FSR);
    int batRaw = analogRead(PIN_BAT);
    int tempRaw = analogRead(PIN_TEMP);
    float tempC   = map(tempRaw, 20, 55, 45, 27);
    filteredTemp  = (tempC * 0.1f) + (filteredTemp * 0.9f);

    String msg = "D," + String(irRaw) + "," + String(fsrRaw) +
                 "," + String((int)filteredTemp) + "," + String(batRaw);
    espSerial.println(msg);
    lastSend = millis();
  }

  if (espSerial.available()) {
    String cmd = espSerial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) {
      executeCommand(cmd);
    }
  }

  updateFingers();
}

// ─── تنفيذ الأوامر ────────────────────────────────────────────
void executeCommand(String cmd) {
  if      (cmd == "A:1") allMove(CLOSE_US);
  else if (cmd == "A:0") allMove(OPEN_US);
  else if (cmd == "F:1") fsrGroup(CLOSE_US);
  else if (cmd == "F:0") fsrGroup(OPEN_US);
  else if (cmd == "I:1") irGroup(CLOSE_US);
  else if (cmd == "I:0") irGroup(OPEN_US);

// --- إضافة التحكم الفردي هنا ---
  else if (cmd == "M1:1") startMove(f1, CLOSE_US); // قفل السبابة
  else if (cmd == "M1:0") startMove(f1, OPEN_US);  // فتح السبابة
  
  else if (cmd == "M2:1") startMove(f2, CLOSE_US); // قفل الأوسط
  else if (cmd == "M2:0") startMove(f2, OPEN_US);
  
  else if (cmd == "M3:1") startMove(f3, CLOSE_US); // قفل البنصر
  else if (cmd == "M3:0") startMove(f3, OPEN_US);
  
  else if (cmd == "M4:1") startMove(f4, CLOSE_US); // قفل الخنصر
  else if (cmd == "M4:0") startMove(f4, OPEN_US);
  
  else if (cmd == "M5:1") startMove(th1, CLOSE_US); // قفل الإبهام 1
  else if (cmd == "M5:0") startMove(th1, OPEN_US);
  
  else if (cmd == "M6:1") startMove(th2, CLOSE_US); // قفل الإبهام 2
  else if (cmd == "M6:0") startMove(th2, OPEN_US);
  
  // أوامر الستبر والوقت (زي ما هي)

  else if (cmd == "S:1") myStepper.step(20); // زيادة الخطوات لسرعة أكبر
  else if (cmd == "S:0") myStepper.step(-20);
  else if (cmd.startsWith("T:")) {
    moveTime = cmd.substring(2).toInt();
  }
}

// ─── مجموعات الحركة ───────────────────────────────────────────
void fsrGroup(int us) {
  startMove(f2, us); startMove(f1, us);//F3
  startMove(th1, us); startMove(th2, us);
}

void irGroup(int us) {
  startMove(f1, us); startMove(f2, us);
  startMove(f3, us); startMove(f4, us);
}

void allMove(int us) {
  startMove(f1, us); startMove(f2, us);
  startMove(f3, us); startMove(f4, us);
  startMove(th1, us); startMove(th2, us);
}

// ─── Finger Helpers ───────────────────────────────────────────
void setupFinger(Finger &f, int p) {
  f.pin = p;
  f.motor.attach(p);      // وصله مؤقتاً
  f.motor.write(STOP_US); // قوله يقف
  delay(200);             // استنى يتأكد
  f.motor.detach();       // افصله تماماً عشان ميزنش وهو لسه بيبدأ
  f.isMoving = false;
  f.isClosed = false;
}

void startMove(Finger &f, int us) {
  bool wantClose = (us > STOP_US);
  if (wantClose  && f.isClosed  && !f.isMoving) return;
  if (!wantClose && !f.isClosed && !f.isMoving) return;

  f.motor.attach(f.pin);  // "السر هنا": بنوصل السلك برمجياً قبل الحركة
  f.motor.write(us);      // ابدأ الحركة
  f.startTime = millis();
  f.isMoving  = true;
  f.isClosed  = wantClose;
}

void updateFingers() {
  Finger* all[] = {&f1, &f2, &f3, &f4, &th1, &th2};
  for (int i = 0; i < 6; i++) {
    if (all[i]->isMoving && (millis() - all[i]->startTime >= (unsigned long)moveTime)) {
      all[i]->motor.write(STOP_US); // قوله يقف
      delay(50);                    // تأكيد وصول الإشارة
      all[i]->motor.detach();       // "السر هنا": افصل السلك تماماً بعد الحركة
      all[i]->isMoving = false;
      Serial.println("Motor detached and silent.");
    }
  }
}