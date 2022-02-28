/*-------------------------------------------------------------
    Servo Follow Focus v1
    Dual-Mode Servo Follow Focus with 2-Stage smoothing, Potentiometer,
    tiny Screen, auto Idle and Sleep Mode
     by eBender

    Parts
     ESP32 (minimum: Microcontroller /w I2C, 1 ADC Input, 1 Digital Input, 1 PWM Output)
     0.42" 32x64 OLED I2C Screen
     25g S0025M Servo (0.06-0.08ms, 2.6-3KG, 333Hz, 2BB, MG)
     10k Potentiometer
     1x Momentary Button
     INA219 Voltage Current Meter
     2S BMS Balance Module
     2S Battery Charger TP5100
     6v 3A Buck Voltage Converter
     2x 18650 Battery
     3D Printed enclosure (files coming to thingyverse)
     22/24AWG Wires
     M3 Screws, M3 Inserts

   ChangeLog
     - fixed sleep Mode
     - added idle indicator UI message
     - added Sleep Mode selection UI message

   Issues
     - idle mode not engaging while still displaying idle message for first few times

   Credits
     - TaskManagerIO - Dave Cherry, Jorropo
     - Moving Average - Jack Christensen
     - ResponisveAnalogRead - Damien Clarke
  -------------------------------------------------------------*/


#include <Arduino.h>
#include "TaskManagerIO.h"
#include <BasicInterruptAbstraction.h>
#include <Wire.h>
#include <ResponsiveAnalogRead.h>
#include "MovingAverage.h"
#include <ESP32Servo.h>
#include <U8g2lib.h>


//=============== ADJUSTABLE ===================================

#define           DEBUG   // note that response time is being slowed by debug mode

#define           SDA 21
#define           SCL 22

#define           buttonPin  2
#define           potiPin    4
#define           servoPin   25

byte              tmMultiplier = 1;        // variable task update frequency - 1 normal, 2 half speed.. etc
#define           smoothValue   90 / tmMultiplier // Smooth Mode Smoothing 0-255
#define           expo          3.0        // Input Exponential Curve 0.0 - 10.0, fine control at startpoint
#define           Hertz         333        // 50-333Hz Servo
unsigned int      potiEnd =     4500.0;    // Poti end stop (reduce for less poti range - scaling correctly)
#define           servoStart    500        // Servo 500um-2500um pulse width
#define           servoEnd      2500
bool              sleepMode =   true;      // button double click switches sleepMode
unsigned int      sleepOff =    25000;     // deep Sleep in ms (don't put below 15000)
#define           idleTimer     5000       // idle in ms
#define           font          u8g2_font_logisoso28_tn   // u8g2_font_logisoso28_tn @ Y30  -  u8g2_font_helvB24_tn @ Y28,  (u8g2_font_battery19_tn - Battery 19px)
byte              fontY =       30;        // font y position in px

//=============== ADJUSTABLE END ================================

float             potiIn,     potiOut,    potiValue,  potiTemp,  servoTemp,  idleTemp;
unsigned long int buttonTime, codeTime,   sleepTimer, timeOff,   i, ms, us,  taskID, sleepID;
bool              buttonBool, smoothMode, idleOn;

U8G2_SSD1306_64X32_1F_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
ResponsiveAnalogRead analog1(potiPin, false);  // Stage 1 Smoothing
MovingAverage smooth1(smoothValue);            // Stage 2 Smoothing
Servo servo;


void setup() {

#ifdef DEBUG
  Serial.begin(115200);
  delay(300);
  Serial.print("CPU: ");
  Serial.println(getCpuFrequencyMhz());
#endif

  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFontDirection(0);
  u8g2.setFontMode(1);
  u8g2.setDrawColor(1);
  u8g2.setFont(font);

  Wire.begin(SDA, SCL, 400000);

  pinMode(buttonPin, INPUT_PULLDOWN);
  pinMode(potiPin, INPUT_PULLDOWN);

  if (getCpuFrequencyMhz() != 240) setCpuFrequencyMhz(240); // for coming out of sleep mode
  servo.setPeriodHertz(Hertz);
  servo.attach(servoPin, servoStart, servoEnd);   // attach Servo
  BasicArduinoInterruptAbstraction interruptAbstraction;  // INT for button

  ms = millis();
  buttonTime = ms;
  sleepTimer = ms;
  idleTemp   = potiOut;


  for (i = 64; i >= 32; i--) {                   // screen roll-in on startup
    getPoti();
    writeServo();
    u8g2.clearBuffer();
    u8g2.setCursor(0, i);
    u8g2.print((potiOut - 500.0) / 20.0, 2);     // map to 0 - 100
    u8g2.sendBuffer();
  }

  taskManager.setInterruptCallback(interruptTask);   // add interrupt routine for the button input
  taskManager.addInterrupt(&interruptAbstraction, buttonPin, RISING);

  taskManager.scheduleFixedRate(3  * tmMultiplier,   getPoti,      TIME_MILLIS);   // 333hz
  taskManager.scheduleFixedRate(3  * tmMultiplier,   writeServo,   TIME_MILLIS);   // 333hz bc servo updates @ 333hz
  taskManager.scheduleFixedRate(20 * tmMultiplier,   writeScreen,  TIME_MILLIS);   // 50fps
  if (sleepMode) {
    sleepID = taskManager.scheduleFixedRate(1,       getSleepMode, TIME_SECONDS);  // 1hz
  }
  taskManager.scheduleFixedRate(1,                   idle,         TIME_SECONDS);  // 1hz

}

#ifdef DEBUG
void logIt(const char* toLog, unsigned long int cTime) {
  Serial.print(millis());
  Serial.print("ms: ");
  Serial.print(cTime);
  Serial.print("us - ");
  Serial.println(toLog);
}
#endif

void loop() {
  taskManager.runLoop();
}
