// MOTOR TEST PROGRAMMA
// Werkt met jouw motorpins:
// LeftF  = 3
// LeftB  = 9
// RightF = 10
// RightB = 11

#define MotorLeftForward 3
#define MotorLeftBackward 9
#define MotorRightForward 11
#define MotorRightBackward 10

void setup() {
  pinMode(MotorLeftForward, OUTPUT);
  pinMode(MotorLeftBackward, OUTPUT);
  pinMode(MotorRightForward, OUTPUT);
  pinMode(MotorRightBackward, OUTPUT);

  Serial.begin(9600);
  Serial.println("Motor test start...");
}

void loop() {

  // ----------------------
  // TEST 1: LINKSE MOTOR
  // ----------------------

  Serial.println("Linkse motor vooruit");
  motorLeft(150);       // vooruit PWM 150
  motorRight(0);        // rechts uit
  delay(1000);

  Serial.println("Linkse motor achteruit");
  motorLeft(-150);      // achteruit PWM 150
  delay(1000);

  motorLeft(0);
  delay(500);


  // ----------------------
  // TEST 2: RECHTSE MOTOR
  // ----------------------

  Serial.println("Rechtse motor vooruit");
  motorRight(150);
  motorLeft(0);
  delay(1000);

  Serial.println("Rechtse motor achteruit");
  motorRight(-150);
  delay(1000);

  motorRight(0);
  delay(500);


  // ----------------------
  // TEST 3: BEIDE MOTORREN
  // ----------------------

  Serial.println("Beide vooruit");
  motorLeft(150);
  motorRight(150);
  delay(1000);

  Serial.println("Beide achteruit");
  motorLeft(-150);
  motorRight(-150);
  delay(1000);

  Serial.println("STOP");
  motorLeft(0);
  motorRight(0);
  delay(2000);     // pauze

}



// ----------------------
// Motor functies
// ----------------------

void motorLeft(int pwm) {
  if (pwm > 0) {
    analogWrite(MotorLeftForward, pwm);
    analogWrite(MotorLeftBackward, 0);
  }
  else if (pwm < 0) {
    analogWrite(MotorLeftForward, 0);
    analogWrite(MotorLeftBackward, -pwm);
  }
  else {
    analogWrite(MotorLeftForward, 0);
    analogWrite(MotorLeftBackward, 0);
  }
}

void motorRight(int pwm) {
  if (pwm > 0) {
    analogWrite(MotorRightForward, pwm);
    analogWrite(MotorRightBackward, 0);
  }
  else if (pwm < 0) {
    analogWrite(MotorRightForward, 0);
    analogWrite(MotorRightBackward, -pwm);
  }
  else {
    analogWrite(MotorRightForward, 0);
    analogWrite(MotorRightBackward, 0);
  }
}

