#include <SoftwareSerial.h>

// ----------------- SoftwareSerial -----------------
SoftwareSerial espSerial(2, 3);

// ----------------- Motor Pins -----------------
int mr1 = 8, mr2 = 9, ml1 = 10, ml2 = 11;
int enr = 3, enl = 5;

// ----------------- IR Sensor Pins -----------------
int sr = 6, sl = 7;
int srb = 12, slb = 13;
int svr = 0, svl = 0;
int svrb = 0, svlb = 0;

// ----------------- Speed Settings -----------------
int vspeed = 100;
int tspeed = 220;
int tdelay = 20;

// ----------------- Movement Flags -----------------
bool moveForward = false;
bool moveBackward = false;

// ----------------- Setup -----------------
void setup() {
  // Motor pins
  pinMode(mr1, OUTPUT);
  pinMode(mr2, OUTPUT);
  pinMode(ml1, OUTPUT);
  pinMode(ml2, OUTPUT);
  pinMode(enr, OUTPUT);
  pinMode(enl, OUTPUT);

  // IR sensors
  pinMode(sr, INPUT);
  pinMode(sl, INPUT);
  pinMode(srb, INPUT);
  pinMode(slb, INPUT);

  // Serial communication
  Serial.begin(9600);
  espSerial.begin(9600);
}

// ----------------- Main Loop -----------------
void loop() {
  // ----------------- ESP Serial Commands -----------------
  if (espSerial.available()) {
    String msg = espSerial.readStringUntil('\n');
    msg.trim();

    if (msg == "START_FORWARD") {
      moveForward = true;
      moveBackward = false;
    }
    else if (msg == "BACKWARD") {
      moveForward = false;
      moveBackward = true;
    }
    else if (msg == "STOP") {
      moveForward = false;
      moveBackward = false;
      stopMotor();
    }
  }

  // ----------------- Read IR Sensors -----------------
  svr = digitalRead(sr);
  svl = digitalRead(sl);
  svrb = digitalRead(srb);
  svlb = digitalRead(slb);

  // ----------------- Motor Logic -----------------
  if (moveForward) {
    if (svl == LOW && svr == LOW) forwardMotors();
    else if (svl == LOW && svr == HIGH) leftMotors();
    else if (svl == HIGH && svr == LOW) rightMotors();
    else stopMotor();
  }
  else if (moveBackward) {
    if (svlb == LOW && svrb == LOW) backwardMotors();
    else if (svlb == LOW && svrb == HIGH) leftMotorsBackward();  // FIXED
    else if (svlb == HIGH && svrb == LOW) rightMotorsBackward();  // FIXED
    else stopMotor();
  }
}

// ----------------- Motor Functions -----------------
void forwardMotors() {
  digitalWrite(mr1, HIGH);
  digitalWrite(mr2, LOW);
  digitalWrite(ml1, HIGH);
  digitalWrite(ml2, LOW);
  analogWrite(enr, vspeed);
  analogWrite(enl, vspeed);
}

void backwardMotors() {
  digitalWrite(mr1, LOW);
  digitalWrite(mr2, HIGH);
  digitalWrite(ml1, LOW);
  digitalWrite(ml2, HIGH);
  analogWrite(enr, vspeed);
  analogWrite(enl, vspeed);
}

void leftMotors() {
  digitalWrite(mr1, HIGH);
  digitalWrite(mr2, LOW);
  digitalWrite(ml1, LOW);
  digitalWrite(ml2, HIGH);
  analogWrite(enr, tspeed);
  analogWrite(enl, tspeed);
  delay(tdelay);
}

void rightMotors() {
  digitalWrite(mr1, LOW);
  digitalWrite(mr2, HIGH);
  digitalWrite(ml1, HIGH);
  digitalWrite(ml2, LOW);
  analogWrite(enr, tspeed);
  analogWrite(enl, tspeed);
  delay(tdelay);
}

// -------- Corrected Backward Turns --------
void leftMotorsBackward() {
  digitalWrite(mr1, HIGH);
  digitalWrite(mr2, LOW);
  digitalWrite(ml1, LOW);
  digitalWrite(ml2, HIGH);
  analogWrite(enr, tspeed);
  analogWrite(enl, tspeed);
  delay(tdelay);
}

void rightMotorsBackward() {
  digitalWrite(mr1, LOW);
  digitalWrite(mr2, HIGH);
  digitalWrite(ml1, HIGH);
  digitalWrite(ml2, LOW);
  analogWrite(enr, tspeed);
  analogWrite(enl, tspeed);
  delay(tdelay);
}

void stopMotor() {
  analogWrite(enr, 0);
  analogWrite(enl, 0);
}
