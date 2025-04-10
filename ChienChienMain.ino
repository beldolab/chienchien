// --------------------------------------------------------------------------
//       Chien_display_BT
//
//        Programme pour Robot Chien
//        (C) 2025, BeldoLAB Creation
//        Author: BLC
//        Date: 10/04/2025
// ---------------------------------------------------------------------------
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HUSKYLENS.h"
#include "DYPlayerArduino.h"

// Initialise the player, it defaults to using Serial.
DY::Player player(&Serial1);

HUSKYLENS huskylens;

void printResult(HUSKYLENSResult result);

float vbat;

// -------------- Local Variables for Display --------------------------------
LiquidCrystal_I2C lcd(0x27, 20, 4);  // set the LCD address to 0x27 for a 16 chars and 2 line display
char _lbuf[32];                      // Buffer for one line
// --------------- Motor Pin Assignement -------------------------------------
int MOTR_PIN_IN1 = 44;
int MOTR_PIN_IN2 = 45;
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
  analogReference(INTERNAL2V56);
}

void readVbat() {
  sensorValue = analogRead(sensorPin);
  //  vbat = (((float)sensorValue) / 1024.0) * 55.0;
  vbat = (((float)sensorValue) / 1024.0) * 19.69 + 1.0;
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
// Print text on the display and on BT
void printDisplayText(int line, int col, char *text, bool bt) {
  if ((line >= 0) && (line < 4)) {
    if ((col >= 0) && (col < 20)) {
      lcd.setCursor(col, line);
      lcd.print(text);
      if (bt) {
        char st[32];
        sprintf(st, "*M%s*\n", text);
        Serial3.print(st);
      }
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
    printDisplayText(2, 0, "Mot=", false);
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

void Stopmotors() {
  int speed = 0;
  int vright = 0;
  int vleft = 0;
  bool mabort = false;

  vright = 0;
  vleft = 0;
  speed = 0;
  MotorAssignSpeed(vright, vleft, speed, mabort);
}


// ------------------------------------------------------------------
// HUSKY Functions
void printResult(HUSKYLENSResult result) {
  if (result.command == COMMAND_RETURN_BLOCK) {
    Serial.println(String() + F("Block:xCenter=") + result.xCenter + F(",yCenter=") + result.yCenter + F(",width=") + result.width + F(",height=") + result.height + F(",ID=") + result.ID);
  } else if (result.command == COMMAND_RETURN_ARROW) {
    Serial.println(String() + F("Arrow:xOrigin=") + result.xOrigin + F(",yOrigin=") + result.yOrigin + F(",xTarget=") + result.xTarget + F(",yTarget=") + result.yTarget + F(",ID=") + result.ID);
  } else {
    Serial.println("Object unknown!");
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
// Check Bluetooth Command line coherency
bool CheckCoherency(char *cl) {
  bool coher = false;
  char cmd = cl[0];
  // Check allowed Commands
  coher = (cmd == 'Z') || (cmd == 'P') || (cmd == 'V') || (cmd = 'A') || (cmd = 'M') || (cmd = 'W');
  return (coher);
}

// ------------------------------------------------------------------
unsigned long HUSKmili = 0;
unsigned long MAXmili = 1000;  // no command time

unsigned int AUTOMATIC = 1;
unsigned int MANUAL = 2;
unsigned int ModeChien = AUTOMATIC;

#define ISAUTO (ModeChien == AUTOMATIC)
#define ISMANU (ModeChien == MANUAL)

void setup() {
  // Initialise Debug Port
  Serial.begin(115200);  // For debug while wired
  // Initialise Bluetooth Serial Port
  Serial3.begin(9600);  // Default Baud Rate for HC05
  // Initialize Display
  SetBLCDisplay(true);
  // Change PWM
  // Initialise Motor
  pinMode(MOTR_PIN_EN, OUTPUT);
  pinMode(MOTL_PIN_EN, OUTPUT);
  digitalWrite(MOTR_PIN_EN, LOW);  // Disabled [TODO] revoir cablage du MOS N
  digitalWrite(MOTL_PIN_EN, LOW);  // Disabled [TODO] revoir cablage du MOS N
  // Initialize VBat measurement
  initVbat();
  // Initialize Pattern Recognition
  Serial2.begin(9600);
  delay(1000);
  while (!huskylens.begin(Serial2)) {
    Serial.println(F("Begin failed!"));
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }
  printDisplayText(1, 0, "HUSKY OK", true);
  // Start Player
  player.begin();
  player.setVolume(31);  // Max Volume
  player.playSpecified(1);
}

// ------------------------------------------------------------------
void loop() {
  // Main Loop
  if (millis() >= HUSKmili + MAXmili) {
    if (ISAUTO) {
      Stopmotors();
    }
  }
  //    Serial.println(F("###########"));
  if (!huskylens.request()) {
    Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
    printDisplayText(1, 0, "HUSKY ERR NO CONNECT", true);
  } else if (!huskylens.isLearned()) {
    Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
    printDisplayText(1, 0, "HUSKY ERR NO PATTERN", true);
  }
  if (huskylens.available()) {
    HUSKYLENSResult result = huskylens.read();
    printDisplayText(1, 0, "HUSKY: ", false);
    clearZoneDisplay(1, 7, 10);
    // Get time
    HUSKmili = millis();
    printResult(result);
    // Print object ID
    int iobj = result.ID;
    char st[32];
    sprintf(st, "*I%d*\n", iobj);
    Serial3.print(st);
    // Manage Angle of View
    int x = result.xCenter;
    if (ISAUTO) {
      vright = 0;
      vleft = 0;
      speed = 0;
    }
    if ((x < 240) && (x > 120)) {  // Aller tout droit
      if (ISAUTO) {
        vright = 10;
        vleft = 10;
        speed = 25;
      }
      printDisplayText(1, 8, "TOUT DROIT", true);
      Serial.println(F("TOUT DROIT"));
    } else if (x < 120) {  // Tourner à gauche
      if (ISAUTO) {
        vright = 0;
        vleft = 10;
        speed = 20;
      }
      Serial.println(F("A GAUCHE"));
      printDisplayText(1, 8, "A GAUCHE", true);
    } else if (x > 240) {
      if (ISAUTO) {
        vright = 10;
        vleft = 0;
        speed = 20;
      }
      printDisplayText(1, 8, "A DROITE", true);
      Serial.println(F("A DROITE"));
    }
    // Manage Distance of View
    int w = result.width;  // Taille ( augmenter vitesse / se rapprocher )
    if ((w > 100) && (w < 180)) {
      if (ISAUTO) speed = speed * map(w, 101, 179, 100, 0) / 100;
      Serial.print("RALENTIS : ");
      Serial.println(speed);
    } else if (w >= 180) {
      if (ISAUTO) speed = 0;
      printDisplayText(1, 8, "STOP", true);
      Serial.print("STOOOOOOP");
      Serial.println(speed);
    }
    if (ISAUTO) {
      if (speed < 0) speed = 0;
      if (speed > 25) speed = 25;
      MotorAssignSpeed(vright, vleft, speed, mabort);
    }
  }
  // Check Bluetooth command
  if (Serial3.available() > 0) {
    // Print message
    printDisplayText(1, 0, "BT port:", false);
    // Read a line from the Bluetooth Smartphone Application
    char btl[16] = "";
    Serial3.readBytesUntil(';', btl, sizeof(btl) / sizeof(char));
    // Check that it is a valid one
    if (CheckCoherency(btl)) {
      // Display it
      clearZoneDisplay(1, 11, 9);
      printDisplayText(1, 10, btl, false);
      // Decode the stream and apply the associted action
      char cmd = btl[0];
      if (cmd == 'Z') {
        vright = 0;
        vleft = 0;
        speed = 0;
        mabort = true;
        ModeChien = MANUAL;
        MotorAssignSpeed(vright, vleft, speed, mabort);
      }
      if ((cmd == 'V') && (ISMANU)) {
        sscanf(&btl[1], "%2d", &speed);
        MotorAssignSpeed(vright, vleft, speed, mabort);
      }
      if ((cmd == 'P') && (ISMANU)) {
        sscanf(&btl[1], "%2d,%2d", &vright, &vleft);
        MotorAssignSpeed(vright, vleft, speed, mabort);
      }
      if (cmd == 'A') {
        // Automatic mode
        ModeChien = AUTOMATIC;
        Serial.println(F("AUTOMATIQUE"));
      }
      if (cmd == 'M') {
        // Manual mode
        ModeChien = MANUAL;
        Serial.println(F("MANUEL"));
      }
      if (cmd == 'W') {
        // Check sound
        int voice = 0;
        sscanf(&btl[1], "%2d", &voice);
        // Execute the sound
        if ((voice > 0) && (voice < 255))
          player.playSpecified(voice);
      }
    }
  }
  // Display Battery and Motors power
  printDisplayText(3, 0, "Vbat=", false);
  readVbat();
  printDisplayFloat(3, 5, vbat, 5, 2);
  // Display on BT terminal
  int ibat;
  ibat = int(map(vbat, 11, 14, 0, 100));
  ibat = max(min(ibat, 100), 0);
  char st[32];
  sprintf(st, "*B%d*\n", ibat);
  Serial3.print(st);
  // Motor power
  int ipow;
  ipow = (abs(vright) + abs(vleft)) * speed;
  sprintf(st, "*P%d*\n", ipow);
  Serial3.print(st);
}
