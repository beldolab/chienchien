// --------------------------------------------------------------------------
//       Chien_display_BT
//
//        Librairie Arduino pour I2C Display et Bluetooth Module commandes
//        (C) 2025, BeldoLAB Creation
//        Author: Bill
// ---------------------------------------------------------------------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
float vbat;

// -------------- Local Variables for Display --------------------------------
LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
char _lbuf[32];                      // Buffer for one line
// --------------- Motor Pin Assignement -------------------------------------
int MOTR_PIN_IN1 = 45;
int MOTR_PIN_IN2 = 44;
int MOTR_PIN_EN = 46;
int MOTL_PIN_IN1 = 6;
int MOTL_PIN_IN2 = 7;
int MOTL_PIN_EN = 8;
// ---- PWM Board mawximum value (must be with 98% at 16KHz)
int PWM_MIN = 6;
int PWM_MAX = 249;

// --------------- VBat function ---------------------------------------------
int sensorPin = A0;
int sensorValue;

void initVbat() {
}

void readVbat() {
  sensorValue = analogRead(sensorPin);
  vbat = (((float)sensorValue) / 1024.0) * 55.0;
}
// --------------- Display Functions -----------------------------------------
//
// Clear a part of a display line
void clearZoneDisplay(int line, int col, int nc) {
  if ((line >= 0) && (line < 4)) {
    if ((col >= 0) && (col < 20)) {
      if ((nc > 0) && ((col + nc) <= 20)) {
        lcd.setCursor(col, line);
        strcpy(_lbuf, "");
        for (int i = 0; i < nc; i++) {
          strcat(_lbuf, " ");
        }
        lcd.print(_lbuf);
      }
    }
  }
}
// Clear a complete display line
void clearLineDisplay(int line) {
  clearZoneDisplay(line, 0, 20);
}
// Print text on the display
void printDisplayText(int line, int col, char *text) {
  if ((line >= 0) && (line < 4)) {
    if ((col >= 0) && (col < 20)) {
      lcd.setCursor(col, line);
      lcd.print(text);
    }
  }
}
// Print integer on the display
void printDisplayInt(int line, int col, int vi) {
  if ((line >= 0) && (line < 4)) {
    if ((col >= 0) && (col < 20)) {
      lcd.setCursor(col, line);
      sprintf(_lbuf, "%i", vi);
      lcd.print(_lbuf);
    }
  }
}
// Print float on the display
void printDisplayFloat(int line, int col, float vf, int nc, int nd) {
  if ((line >= 0) && (line < 4)) {
    if ((col >= 0) && (col < 20)) {
      lcd.setCursor(col, line);
      dtostrf(vf, nc, nd, _lbuf);
      lcd.print(_lbuf);
    }
  }
}
// Check Command line coherency
bool CheckCoherency(char *cl) {
  bool coher = false;
  char cmd = cl[0];
  // Check allowed Commands
  coher = (cmd == 'Z') || (cmd == 'P') || (cmd == 'V') || (cmd = 'y') || (cmd = 'Y');
  return (coher);
}

// Initialize the display
void SetBLCDisplay(bool disp) {
  lcd.init();  // initialize the lcd
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  if (disp) {
    lcd.setCursor(1, 0);
    lcd.print("BledoLAB Creation");
    for (int ii = 0; ii <= 12; ii++) {
      lcd.setCursor(ii, 3);
      lcd.print("(C) 2025");
      delay(200);
      if (ii < 12) clearZoneDisplay(3, ii, 8);
    }
  }
}

// --------------------------------------------------------------------
void MotorAssignSpeed(int vr, int vl, int sp, bool ab) {
  int motorRp, motorRn, motorLp, motorLn;
  bool err = false;
  if (ab) {
    motorRp = 0;
    motorRn = 0;
    motorLp = 0;
    motorLn = 0;
  } else {
    if ((sp >= 0) && (sp <= 25)) {
      if ((vr >= -10) && (vr <= 10)) {
        if (vr >= 0) {
          motorRn = 0;
          motorRp = (int)vr * sp;
        } else {
          motorRp = 0;
          motorRn = (int)-vr * sp;
        }
      } else {
        err |= true;
      }
      if ((vl >= -10) && (vl <= 10)) {
        if (vl >= 0) {
          motorLn = 0;
          motorLp = (int)vl * sp;
        } else {
          motorLp = 0;
          motorLn = (int)-vl * sp;
        }
      } else {
        err |= true;
      }
    }
  }
  if (!err) {
    // Guaranty Boundary, value muts be within 98%
    if (motorRp < PWM_MIN) motorRp = 0;
    if (motorRp > PWM_MAX) motorRp = PWM_MAX;
    if (motorRn < PWM_MIN) motorRn = 0;
    if (motorRn > PWM_MAX) motorRn = PWM_MAX;
    if (motorLp < PWM_MIN) motorLp = 0;
    if (motorLp > PWM_MAX) motorLp = PWM_MAX;
    if (motorLn < PWM_MIN) motorLn = 0;
    if (motorLn > PWM_MAX) motorLn = PWM_MAX;
    // Display Motor PWM values
    printDisplayText(2, 0, "Mot=");
    clearZoneDisplay(2, 4, 16);
    printDisplayInt(2, 5, motorLp);
    printDisplayInt(2, 9, motorLn);
    printDisplayInt(2, 13, motorRn);
    printDisplayInt(2, 17, motorRp);
    // Assign value to the motors
    analogWrite(MOTR_PIN_IN2, motorRp);
    analogWrite(MOTR_PIN_IN1, motorRn);
    analogWrite(MOTL_PIN_IN2, motorLp);
    analogWrite(MOTL_PIN_IN1, motorLn);
  }
}

// ------------------------------------------------------------------
//      Main Loop
// ------------------------------------------------------------------

// Motor speed for test
int speed = 0;
int vright = 0;
int vleft = 0;
bool mabort = false;

// ------------------------------------------------------------------

void setup() {
  // Initialise Bluetooth Serial Port
  Serial3.begin(9600);  // Default Baud Rate for HC05
  // Initialize Display
  SetBLCDisplay(true);
  // Initialise Motor
  pinMode(MOTR_PIN_EN, OUTPUT);
  pinMode(MOTL_PIN_EN, OUTPUT);
  digitalWrite(MOTR_PIN_EN, LOW);  // Disabled
  digitalWrite(MOTL_PIN_EN, LOW);  // Disabled
  // Initialize VBat measurement
  initVbat();
}

void loop() {
  // Main Loop
  if (Serial3.available() > 0) {
    // Print message
    printDisplayText(1, 0, "BT port:");
    // Read a line from the Bluetooth Smartphone Application
    char btl[16] = "";
    Serial3.readBytesUntil(';', btl, sizeof(btl) / sizeof(char));
    // Check that it is a valid one
    if (CheckCoherency(btl)) {
      // Display it
      clearZoneDisplay(1, 11, 9);
      printDisplayText(1, 10, btl);
      // Decode the stream and apply the associted action
      char cmd = btl[0];
      if (cmd == 'Z') {
        vright = 0;
        vleft = 0;
        speed = 0;
        mabort = true;
        MotorAssignSpeed(vright, vleft, speed, mabort);
      }
      if (cmd == 'V') {
        sscanf(&btl[1], "%2d", &speed);
        MotorAssignSpeed(vright, vleft, speed, mabort);
      }
      if (cmd == 'P') {
        sscanf(&btl[1], "%2d,%2d", &vright, &vleft);
        MotorAssignSpeed(vright, vleft, speed, mabort);
      }
      if (cmd = 'Y') {
        digitalWrite(MOTR_PIN_EN, HIGH);  // Enabled
        digitalWrite(MOTL_PIN_EN, HIGH);  // Enabled
      }
      if (cmd = 'y') {
        digitalWrite(MOTR_PIN_EN, LOW);  // Disabled
        digitalWrite(MOTL_PIN_EN, LOW);  // Disabled
      }
    }
  }
  printDisplayText(3, 0, "Vbat=");
  readVbat();
  printDisplayFloat(3, 5, vbat, 5, 2);
}
