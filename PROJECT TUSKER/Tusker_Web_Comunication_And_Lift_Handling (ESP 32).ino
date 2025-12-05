#include <WiFi.h>
#include <WebServer.h>
#include <HardwareSerial.h>

// ------------------- WiFi Setup -------------------
const char* ssid = "ESP32_COLOR_MODULE";
const char* password = "12345678";

WebServer server(80);

// ------------------- Color Sensor Pins -------------------
#define S0  14
#define S1  21
#define S2  23
#define S3  22
#define OUT 4

// ------------------- Motor Pins (L298N) -------------------
#define ENA 25
#define IN1 26
#define IN2 27

// ------------------- Variables -------------------
long counter = 0;
bool triggered = false;
int stopVariable = 0;
bool waitForExit = false;
bool forwardResumeSent = false;

// ------------------- ESP32 -> Arduino Serial -------------------
#define ESP_TX 32
HardwareSerial SerialESP(2);

// ------------------- Sensor Reading -------------------
unsigned long readColorFrequency(int s2State, int s3State) {
  digitalWrite(S2, s2State);
  digitalWrite(S3, s3State);
  delay(20);
  return pulseIn(OUT, LOW);
}

// ------------------- Web Page -------------------
String htmlPage() {
  return R"=====(
  <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
      body {
        font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        text-align: center;
        background: linear-gradient(135deg, #74ABE2, #5563DE);
        color: #fff;
        margin: 0;
        padding: 0;
      }
      h1 {
        margin-top: 50px;
        font-size: 36px;
        text-shadow: 2px 2px 8px rgba(0,0,0,0.3);
        animation: fadeInDown 1s ease forwards;
      }
      p {
        font-size: 20px;
        margin: 20px auto 40px auto;
        max-width: 700px;
        line-height: 1.5;
        animation: fadeIn 2s ease forwards;
      }
      .button {
        background-color: #FF6F61;
        border: none;
        color: white;
        padding: 20px 40px;
        margin: 15px;
        font-size: 22px;
        border-radius: 12px;
        cursor: pointer;
        box-shadow: 0 8px 15px rgba(0,0,0,0.3);
        transition: all 0.3s ease;
        text-decoration: none;
        display: inline-block;
      }
      .button:hover {
        background-color: #FF3B2E;
        transform: translateY(-5px);
        box-shadow: 0 12px 20px rgba(0,0,0,0.4);
      }
      @keyframes fadeInDown {
        0% {opacity: 0; transform: translateY(-50px);}
        100% {opacity: 1; transform: translateY(0);}
      }
      @keyframes fadeIn {
        0% {opacity: 0;}
        100% {opacity: 1;}
      }
    </style>
  </head>
  <body>
    <h1>Welcome Client</h1>
    <p><strong>Tusker:</strong> Your Warehouse assistant for loading and unloading is now ready to function.<br><br>
    Tell me which item do you want to pick</p>
    
    <a href="/opt1" class="button">Item 1</a>
    <a href="/opt2" class="button">Item 2</a>
    <a href="/opt3" class="button">Item 3</a>
  </body>
  </html>
  )=====";
}

// ------------------- Motor Control -------------------
void motorForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 200);
}

void motorBackward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 200);
}

void motorStop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}

// ------------------- Option Handlers -------------------
void handleOption(int value) {
  stopVariable = value;
  counter = 0;
  triggered = false;
  waitForExit = false;
  forwardResumeSent = false;

  SerialESP.println("START_FORWARD");
  server.send(200, "text/html", htmlPage());
}

void handleOpt1() { handleOption(1); }
void handleOpt2() { handleOption(2); }
void handleOpt3() { handleOption(3); }

// ------------------- Setup -------------------
void setup() {
  Serial.begin(115200);
  SerialESP.begin(9600, SERIAL_8N1, -1, ESP_TX);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  motorStop();

  WiFi.softAP(ssid, password);

  server.on("/", []() { server.send(200, "text/html", htmlPage()); });
  server.on("/opt1", handleOpt1);
  server.on("/opt2", handleOpt2);
  server.on("/opt3", handleOpt3);
  server.begin();
}

// ------------------- Loop -------------------
void loop() {
  server.handleClient();

  if (stopVariable == 0 && waitForExit == false) return;

  unsigned long red   = readColorFrequency(LOW, LOW);
  unsigned long green = readColorFrequency(HIGH, HIGH);
  unsigned long blue  = readColorFrequency(LOW, HIGH);

  bool belowThreshold = (red < 100 && green < 100 && blue < 100);
  bool aboveThreshold = (red > 100 && green > 100 && blue > 100);

  // Counting logic
  if (!waitForExit) {
    if (belowThreshold && !triggered) {
      counter++;
      triggered = true;
    }
    if (!belowThreshold) {
      triggered = false;
    }
  }

  // -------- When stop count reached --------
  if (!waitForExit && counter == stopVariable) {
    SerialESP.println("STOP");
    motorForward();
    delay(7000);       // 9 seconds forward
    motorStop();

    waitForExit = true;
    forwardResumeSent = false;
  }

  // -------- Resume Arduino --------
  if (waitForExit && !forwardResumeSent) {
    SerialESP.println("START_FORWARD");
    forwardResumeSent = true;
  }

  // -------- Detect Exit --------
  if (waitForExit && aboveThreshold) {
    SerialESP.println("STOP");
    motorBackward();
    delay(7000);      // 9 seconds backward
    motorStop();

    SerialESP.println("START_FORWARD");

    stopVariable = 0;
    counter = 0;
    triggered = false;
    waitForExit = false;
    forwardResumeSent = false;
  }

  delay(150);
}
