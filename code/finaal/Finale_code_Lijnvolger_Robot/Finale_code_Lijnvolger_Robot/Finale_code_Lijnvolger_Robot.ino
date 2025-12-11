#include "SerialCommand.h"
#include "EEPROMAnything.h"

#define SerialPort Serial
#define Baudrate 115200

#define MotorLeftForward 3
#define MotorLeftBackward 9
#define MotorRightForward 11
#define MotorRightBackward 10

const int IR = 13; // Sensor LED

SerialCommand sCmd(SerialPort);

unsigned long previous, calculationTime;

const int sensor[] = {A7, A6, A4, A5, A3, A2, A1, A0};

struct param_t
{
  unsigned long cycleTime;
  int black[8];
  int white[8];
  int power;
  float diff;
  float kp;
  float ki;
  float kd;
} params;

int normalised[8];
float debugPosition;
float output;
float error;
float iTerm = 0;
float lastErr;

bool runState = true;   // <-- robot begint automatisch met rijden

// -------- EEPROM INIT FUNCTION --------
void initParams() {
  params.cycleTime = 2000;   // PID-cyclus in microseconden
  params.power     = 200;    // basis motorkracht
  params.diff      = 0.6;    // verschilfactor PID
  params.kp        = 0.8;    // P-factor
  params.ki        = 0.2;    // I-factor
  params.kd        = 0.05;   // D-factor

  for (int i = 0; i < 8; i++) {
    params.black[i] = 0;     // dummy zwart (later kalibreren)
    params.white[i] = 1023;  // dummy wit (later kalibreren)
  }

  EEPROM_writeAnything(0, params);
  Serial.println("EEPROM initialized with default values");
}

// -------- Command handlers --------
void onUnknownCommand(char *command);
void onSet();
void onDebug();
void onCalibrate();
void onRun();
void onStop();
void onSearchLeft();
void onSearchRight();

void setup()
{
  pinMode(MotorLeftForward, OUTPUT);
  pinMode(MotorLeftBackward, OUTPUT);
  pinMode(MotorRightForward, OUTPUT);
  pinMode(MotorRightBackward, OUTPUT);

  SerialPort.begin(Baudrate);

  sCmd.addCommand("set", onSet);
  sCmd.addCommand("debug", onDebug);
  sCmd.addCommand("calibrate", onCalibrate);
  sCmd.addCommand("run", onRun);
  sCmd.addCommand("stop", onStop);
  sCmd.addCommand("sleft", onSearchLeft);
  sCmd.addCommand("sright", onSearchRight);
  sCmd.setDefaultHandler(onUnknownCommand);

  EEPROM_readAnything(0, params);

  // Check of EEPROM leeg/corrupt is
  if (params.cycleTime == 0xFFFFFFFF) {
    initParams();
  }

  pinMode(IR, OUTPUT);
  digitalWrite(IR, HIGH);

  // Automatisch rijden bij opstart
  runState = true;

  SerialPort.println("Ready (auto run)");
}

void loop()
{
  sCmd.readSerial();

  unsigned long current = micros();

  if (current - previous >= params.cycleTime)
  {
    previous = current;

    // Normaliseren
    for (int i = 0; i < 8; i++) {
      normalised[i] = map(analogRead(sensor[i]), params.black[i], params.white[i], 0, 1000);
    }

    // Zoek laagste sensorwaarde
    int index = 0;
    for (int i = 1; i < 8; i++)
      if (normalised[i] < normalised[index])
        index = i;

    // Als > 950: lijn kwijt → stoppen
    if (normalised[index] > 950) {
      runState = false;
    }

    float position;

    if (index == 0) position = -28.5;
    else if (index == 7) position = 28.5;
    else 
    {
      int s0 = normalised[index];
      int s1 = normalised[index - 1];
      int s2 = normalised[index + 1];

      float b = (s2 - s1) / 2.0;
      float a = s2 - b - s0;

      position = -b / (2 * a);
      position += index;
      position -= 3.5;
      position *= 9.525;
    }

    debugPosition = position;

    // PID
    error = position;

    output = params.kp * error;

    iTerm += params.ki * error;
    iTerm = constrain(iTerm, -510, 510);
    output += iTerm;

    output += params.kd * (error - lastErr);
    lastErr = error;

    output = constrain(output, -510, 510);

    int powerLeft = 0;
    int powerRight = 0;

    if (runState)
    {
      if (output >= 0)
      {
        powerLeft = constrain(params.power + params.diff * output, -255, 255);
        powerRight = powerLeft - output;
      }
      else
      {
        powerRight = constrain(params.power - params.diff * output, -255, 255);
        powerLeft = powerRight + output;
      }
    }

    // Motoroutput
    analogWrite(MotorLeftForward,  powerLeft > 0 ? powerLeft : 0);
    analogWrite(MotorLeftBackward, powerLeft < 0 ? -powerLeft : 0);
    analogWrite(MotorRightForward, powerRight > 0 ? powerRight : 0);
    analogWrite(MotorRightBackward, powerRight < 0 ? -powerRight : 0);
  }

  unsigned long diffMicros = micros() - current;
  if (diffMicros > calculationTime) calculationTime = diffMicros;
}

// -------- Command implementations --------
void onUnknownCommand(char *command)
{
  SerialPort.print("unknown command: ");
  SerialPort.println(command);
}

void onSet()
{
  char* param = sCmd.next();
  char* value = sCmd.next();  

  if      (strcmp(param, "cycle") == 0) { long newCycle = atol(value); float r = (float)newCycle / params.cycleTime; params.ki *= r; params.kd /= r; params.cycleTime = newCycle; }
  else if (strcmp(param, "power") == 0) params.power = atol(value);
  else if (strcmp(param, "diff") == 0)  params.diff = atof(value);
  else if (strcmp(param, "kp") == 0)    params.kp = atof(value);
  else if (strcmp(param, "ki") == 0) {
    float sec = params.cycleTime / 1e6;
    params.ki = atof(value) * sec;
  }
  else if (strcmp(param, "kd") == 0) {
    float sec = params.cycleTime / 1e6;
    params.kd = atof(value) / sec;
  }

  EEPROM_writeAnything(0, params);
}

void onDebug()
{
  SerialPort.print("cycle time: ");
  SerialPort.println(params.cycleTime);
  SerialPort.print("calculation time: ");
  SerialPort.println(calculationTime);
  calculationTime = 0;

  SerialPort.print("position: ");
  SerialPort.println(debugPosition);

  SerialPort.print("power: ");
  SerialPort.println(params.power);

  SerialPort.print("diff: ");
  SerialPort.println(params.diff);

  SerialPort.print("kp: ");
  SerialPort.println(params.kp);

  float sec = params.cycleTime / 1e6;
  SerialPort.print("ki: ");
  SerialPort.println(params.ki / sec);
  SerialPort.print("kd: ");
  SerialPort.println(params.kd * sec);
}

void onCalibrate()
{
  char* p = sCmd.next();

  if (strcmp(p, "black") == 0) {
    for (int i = 0; i < 8; i++) params.black[i] = analogRead(sensor[i]);
    SerialPort.println("black done");
  }
  else if (strcmp(p, "white") == 0) {
    for (int i = 0; i < 8; i++) params.white[i] = analogRead(sensor[i]);
    SerialPort.println("white done");
  }

  EEPROM_writeAnything(0, params);
}

void onRun() { runState = true; }
void onStop() { runState = false; }

void onSearchLeft()
{
  const uint8_t PWM = 50;
  const int TH = 200;
  unsigned long start = millis();

  while (millis() - start < 1500)
  {
    analogWrite(MotorLeftBackward, PWM);
    analogWrite(MotorLeftForward, 0);
    analogWrite(MotorRightForward, 0);
    analogWrite(MotorRightBackward, 0);

    int v3 = map(analogRead(sensor[3]), params.black[3], params.white[3], 0, 1000);
    int v4 = map(analogRead(sensor[4]), params.black[4], params.white[4], 0, 1000);

    if (v3 < TH || v4 < TH) break;

    delay(5);
  }

  onStop();
}

void onSearchRight()
{
  const uint8_t PWM = 50;
  const int TH = 200;
  unsigned long start = millis();

  while (millis() - start < 1500)
  {
    analogWrite(MotorRightBackward, PWM);
    analogWrite(MotorRightForward, 0);
    analogWrite(MotorLeftForward, 0);
    analogWrite(MotorLeftBackward, 0);

    int v3 = map(analogRead(sensor[3]), params.black[3], params.white[3], 0, 1000);
    int v4 = map(analogRead(sensor[4]), params.black[4], params.white[4], 0, 1000);

    if (v3 < TH || v4 < TH) break;

    delay(5);
  }

  onStop();
}
