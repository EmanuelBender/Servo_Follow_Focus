/*-------------------------------------------------------------
    Servo Follow Focus
    Dual-Mode Servo Follow Focus with 2-Stage smoothing, Potentiometer, tiny Screen and Sleep Mode

    Parts:
     ESP32 (minimum: Microcontroller /w I2C, 2 Inputs, 1 PWM Output)
     0.42" 32x64 OLED I2C Screen
     25g S0025M Servo (0.06-0.08ms, 3KG, 333Hz, 2BB, MG)
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
     - added button double click detection

   by eBender
  -------------------------------------------------------------*/


#include <Arduino.h>
#include "TaskManagerIO.h"
#include <Wire.h>
#include <ResponsiveAnalogRead.h>
#include "MovingAverage.h"
#include <ESP32Servo.h>
#include <U8g2lib.h>


//=============== ADJUSTABLES ===================================

//#define           DEBUG   // note that smoothing response is skewed by debug mode

#define           SDA1 21
#define           SCL1 22

#define           buttonPin  2
#define           potiPin    4
#define           servoPin   25

#define           smoothValue   120        // Smooth Mode Smoothing 0-255
#define           expo          3.0        // Input Exponential Curve
#define           Hertz         333        // 50-333Hz Servo
unsigned int      potiEnd =     4500.0;    // Poti end stop
#define           servoStart    500        // Servo 500um-2500um pulse width
#define           servoEnd      2500
unsigned int      sleepOff =    30000;     // Power save mode delay in ms

const uint8_t*    font = u8g2_font_logisoso28_tn;   // u8g2_font_logisoso28_tn @ Y30  -  u8g2_font_helvB24_tn @ Y28 ///  u8g2_font_battery19_tn - Battery 19px
byte              fontY = 30;

//=============== ADJUSTABLES END ================================

double            potiIn,     potiOut,    potiValue,  potiTemp,  servoTemp;
unsigned long int buttonTime, codeTime,   sleepTimer, timeOff,   i, ms, us;
bool              buttonIn,   buttonBool, smoothMode;

U8G2_SSD1306_64X32_1F_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
ResponsiveAnalogRead analog1(potiPin, false);   // Stage 1 Smoothing - pin, sleepmode
MovingAverage smooth1(smoothValue);             // Stage 2 Smoothing
Servo servo;


void setup() {

#ifdef DEBUG
  Serial.begin(500000);
  delay(300);
#endif

  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFontDirection(0);
  u8g2.setFontMode(1);
  u8g2.setFont(font);

  Wire.begin(SDA1, SCL1, 400000);

  pinMode(buttonPin, INPUT_PULLDOWN);
  pinMode(potiPin, INPUT_PULLDOWN);

  servo.setPeriodHertz(Hertz);
  servo.attach(servoPin, servoStart, servoEnd);   // Attach Servo

  ms = millis();
  buttonTime = ms;
  sleepTimer = ms;

  fontY += 32;
  for (i = 0; i < 32; i++) {                     // scroll up on startup
    getPoti();
    fontY--;
    u8g2.clearBuffer();
    u8g2.setCursor(0, fontY);
    u8g2.print((potiOut - 500.0) / 20.0, 2);     // map to 0 - 100
    u8g2.sendBuffer();
  }

  taskManager.scheduleFixedRate(50,   getButtons,  TIME_MILLIS);   // 20hz
  taskManager.scheduleFixedRate(1,    getPoti,     TIME_MILLIS);   // 1000hz
  taskManager.scheduleFixedRate(3,    writeServo,  TIME_MILLIS);   // 333hz bc servo updates @ 333hz
  taskManager.scheduleFixedRate(20,   writeScreen, TIME_MILLIS);   // 50fps
  taskManager.scheduleFixedRate(1,    sleepMode,   TIME_SECONDS);  // 1hz

}

/*
  codeTime = micros() - us;
  logIt("250us getPoti", codeTime);

  void logIt(const char* toLog, unsigned long int cTime) {
  #ifdef DEBUG
  Serial.print(millis());
  Serial.print("ms: ");
  Serial.print(cTime);
  Serial.print("us - ");
  Serial.println(toLog);
  #endif
  }
*/

void loop() {
  taskManager.runLoop();
}
