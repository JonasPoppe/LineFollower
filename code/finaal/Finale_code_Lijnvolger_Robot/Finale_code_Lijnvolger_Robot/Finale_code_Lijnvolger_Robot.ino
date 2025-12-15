#include "SerialCommand.h"
#include "EEPROMAnything.h"

#define SerialPort Serial
#define Baudrate 9600

#define MotorLeftForward 11
#define MotorLeftBackward 10
#define MotorRightForward 3
#define MotorRightBackward 9


const int IR =13; //SensorLED

volatile bool runState = false;
volatile bool buttonEdgePending = false;
volatile unsigned long lastDebounceTime = 0;  // Tijd voor debouncing
unsigned long debounceDelay = 50;  // Debounce-tijd in milliseconden

SerialCommand sCmd(SerialPort);

bool debug;
bool run;



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
  //float ki;
  //float kd;
  
  /* andere parameters die in het eeprom geheugen moeten opgeslagen worden voeg je hier toe ... */
} params;

int normalised[8];
float debugPosition;
float output;
float error;
float iTerm = 0;
float lastErr;
void onUnknownCommand(char *command);
void onSet();
void onDebug();
void onCalibrate();
void onRun();
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
  SerialPort.println("ready");

 


  pinMode(IR, OUTPUT);
  digitalWrite(IR, HIGH);

}

void loop()
{
  sCmd.readSerial();


 if (buttonEdgePending) {
    // vlag atoom “pakken en wissen”
    noInterrupts();
    bool pending = buttonEdgePending;
    buttonEdgePending = false;
    interrupts();

    if (pending) {
    unsigned long now = millis();
    if (now - lastDebounceTime > debounceDelay) {
      runState = false;       // toggle toestand
      iTerm = 0;
      lastDebounceTime = now;
 
    }
  }
}
  run = runState;
  unsigned long current = micros();
  if (current - previous >= params.cycleTime)
    {
    previous = current;

    /* Normalize*/
  
    for (int i = 0; i < 8; i++) 
    {
      normalised[i]=map(analogRead(sensor[i]), params.black[i], params.white[i], 0, 1000);
    }

    float position;
    int index = 0;

    for(int i = 1; i < 8; i++) if (normalised[i] < normalised[index]) index = i;

    //if (normalised[index] > 950) 
    //{
      //runState = false;

    //}
     
    if (index == 0) position = -28.5;
    else if (index == 7) position = 28.5;
    else
    {
    int sNul=normalised[index]; 
    int sMinEen = normalised[index-1];
    int sPlusEen = normalised[index+1];

    float b = sPlusEen - sMinEen;
    b = b / 2;

    float a = sPlusEen - b -sNul;

    position = -b / (2 * a);
    position += index;
    position -= 3.5;
    position *= 9.525;
    }

    debugPosition = position;

    //P-regelaar
    error = position;
    output = error * params.kp;

    //I-Regelaar
    //iTerm += params.ki * error;
    //iTerm = constrain(iTerm, -510, 510);
    //output += iTerm;

    //D-Regelaar
    //output += params.kd * (error - lastErr);
    //lastErr = error;

    output = constrain(output, -510, 510);

    int powerLeft = 0;
    int powerRight = 0;

    if(run) if (output >= 0)
    {
      powerLeft = constrain(params.power + params.diff * output, -255, 255);
      powerRight = constrain(powerLeft - output, -255, 255);
      powerLeft = powerRight + output;
    }
    else
    {
      powerRight = constrain(params.power - params.diff * output, -255, 255);
      powerLeft = constrain(powerRight + output, -255, 255);
      powerRight = powerLeft - output;
    }

    analogWrite(MotorLeftForward, powerLeft > 0 ? powerLeft : 0);
    analogWrite(MotorLeftBackward, powerLeft < 0 ? -powerLeft : 0);
    analogWrite(MotorRightForward, powerRight > 0 ? powerRight : 0);
    analogWrite(MotorRightBackward, powerRight < 0 ? -powerRight : 0);

  }

  unsigned long difference = micros() - current;
  if (difference > calculationTime) calculationTime = difference;
}

void onUnknownCommand(char *command)
{
  SerialPort.print("unknown command: \"");
  SerialPort.print(command);
  SerialPort.println("\"");
}

void onSet()
{
  char* param = sCmd.next();
  char* value = sCmd.next();  
  
  if (strcmp(param, "cycle") == 0) 
  {
    long newCycleTime = atol(value);
    float ratio = ((float) newCycleTime) / ((float) params.cycleTime);

    //params.ki *= ratio;
    //params.kd /= ratio;

    params.cycleTime = newCycleTime;
  }

    //else if (strcmp(param, "ki") == 0)
  {
    //float cycleTimeInSec = ((float) params.cycleTime) / 1000000;
    //params.ki = atof(value) * cycleTimeInSec;
  }

  //else if (strcmp(param, "kd") == 0)
  {
    //float cycleTimeInSec = ((float) params.cycleTime) / 1000000;
    //params.kd = atof(value) / cycleTimeInSec;
  }
  
  if (strcmp(param, "power") == 0) params.power = atol(value);
  else if (strcmp(param, "diff") == 0) params.diff = atof(value);
  else if (strcmp(param, "kp") == 0) params.kp = atof(value);

 
  /* parameters een nieuwe waarde geven via het set commando doe je hier ... */
  
  EEPROM_writeAnything(0, params);
}

void onDebug()
{
  SerialPort.print("cycle time: ");
  SerialPort.println(params.cycleTime);

  SerialPort.print("calculation time: ");
SerialPort.println(calculationTime);
calculationTime = 0;
 SerialPort.println(" ");

  /* parameters weergeven met behulp van het debug commando doe je hier ... */
  
/*SerialPort.print("black: ");
for (int i = 0; i < 8; i++)
{
  SerialPort.print(params.black[i]);
  SerialPort.print (" ");
}
SerialPort.println(" ");

SerialPort.print("white: ");
for (int i = 0; i < 8; i++)
{
  SerialPort.print(params.white[i]);
  SerialPort.print (" ");
}
SerialPort.println(" ");

SerialPort.print("normalised: ");
for (int i = 0; i < 8; i++)
{
  SerialPort.print(normalised[i]);
  SerialPort.print (" ");
}
SerialPort.println(" ");
*/
SerialPort.print("position: ");
SerialPort.println (debugPosition);

 SerialPort.println(" ");

SerialPort.print("power: ");
SerialPort.println (params.power);

SerialPort.print("diff: ");
SerialPort.println (params.diff);

SerialPort.print("kp: ");
SerialPort.println (params.kp);

//float cycleTimeInSec = ((float) params.cycleTime) / 1000000;
  //float ki = params.ki / cycleTimeInSec;
  //SerialPort.print("ki: ");
  //SerialPort.println(ki);

  //float kd = params.kd * cycleTimeInSec;
  //SerialPort.print("kd: ");
  //SerialPort.println(kd);

  //SerialPort.print("RUN: ");
  //SerialPort.println(params.kp);
  
  //SerialPort.println(" ");

}

void onCalibrate()
{
  char* param= sCmd.next();

  if(strcmp(param, "black") == 0)
  {
    SerialPort.print("start calibrating black... ");
    for (int i = 0; i < 8; i++) params.black[i]=analogRead(sensor[i]);
    SerialPort.println("done");
  }

  else if (strcmp(param, "white") == 0)
  {
    SerialPort.print("start calibrating white... ");    
    for (int i = 0; i < 8; i++) params.white[i]=analogRead(sensor[i]);  
    SerialPort.println("done");      
  }
  EEPROM_writeAnything(0, params);

}

void onRun() 
{
  runState = true;

}

void onStop()
{
  runState = false;

}


void knopISR() {
  // Doe bijna niets in de ISR: alleen een vlag zetten.
  buttonEdgePending = true;
}

void onSearchLeft() {
  // Kleine zoekactie naar links
  const uint8_t SEARCH_PWM  = 50;
  const int     LINE_THRESH = 200;
  const unsigned long MAX_SEARCH_MS = 1500; // vang eindeloze loop af

  runState = false;  
  unsigned long t0 = millis();


  while (true) {
    // draai naar links
    analogWrite(MotorLeftBackward, SEARCH_PWM);
    analogWrite(MotorLeftForward,  0);
    analogWrite(MotorRightForward, 0);
    analogWrite(MotorRightBackward,0);

    int v3 = map(analogRead(sensor[3]), params.black[3], params.white[3], 0, 1000);
    int v4 = map(analogRead(sensor[4]), params.black[4], params.white[4], 0, 1000);

    // lijn gevonden?
    if (v3 < LINE_THRESH || v4 < LINE_THRESH) {
      onStop();   // hier stoppen we netjes
      break;
    }

    // timeout?
    if (millis() - t0 > MAX_SEARCH_MS) {
      onStop();
      break;
    }

    delay(5);
  }
}

void onSearchRight() {
  // Kleine zoekactie naar rechts
  const uint8_t SEARCH_PWM  = 50;
  const int     LINE_THRESH = 200;
  const unsigned long MAX_SEARCH_MS = 1500;

  runState = false;  
  unsigned long t0 = millis();



  while (true) {
    // draai naar rechts
    analogWrite(MotorRightBackward, SEARCH_PWM);
    analogWrite(MotorRightForward, 0);
    analogWrite(MotorLeftForward,  0);
    analogWrite(MotorLeftBackward, 0);

    int v3 = map(analogRead(sensor[3]), params.black[3], params.white[3], 0, 1000);
    int v4 = map(analogRead(sensor[4]), params.black[4], params.white[4], 0, 1000);

    if (v3 < LINE_THRESH || v4 < LINE_THRESH) {
      onStop();
      break;
    }
    if (millis() - t0 > MAX_SEARCH_MS) {
      onStop();
      break;
    }

    delay(5);
  }
 
}
 