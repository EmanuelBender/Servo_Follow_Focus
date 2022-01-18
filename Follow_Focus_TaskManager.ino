/**
   @file SimpleTasks.ino
   A very simple example of how to use task manager to schedule tasks to be done

   In this example we demonstrate how to create tasks that execute at a point in time,
   that repeat at a given interval, and tasks that are executed as soon as possible
   by task manager. We also show how to cancel a running task.

*/

// To use task manager we must include the library
#include <Arduino.h>
#include "TaskManagerIO.h"
#include <Wire.h>
#include <ResponsiveAnalogRead.h>
#include "MovingAverage.h"
#include <ESP32Servo.h>
#include <U8g2lib.h>


//=============== ADJUSTABLES ===================================

#define           DEBUG

#define           SDA1 21
#define           SCL1 22

#define           buttonPin  2
#define           potiPin    4
#define           servoPin   25

#define           smoothValue   60         // Smooth Mode Smoothing 0-255
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
unsigned long int buttonTime, codeTime,   sleepTimer, timeOff,   i, ms;
bool              buttonIn,   buttonBool, smoothMode;

U8G2_SSD1306_64X32_1F_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
ResponsiveAnalogRead analog1(potiPin, false);   // Stage 1 Smoothing - pin, sleepmode
MovingAverage smooth1(smoothValue);             // Stage 2 Smoothing
Servo servo;

//
// A simple logging function that logs the time and the log line.
//
void logIt(const char* toLog) {
  Serial.print(millis());
  Serial.print(':');
  Serial.println(toLog);
}


void setup() {

#ifdef DEBUG
  Serial.begin(115200);
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


  taskManager.scheduleFixedRate(20, [] {
    logIt("20ms  writeScreen");
    writeScreen();
  });

  taskManager.scheduleFixedRate(30, [] {
    logIt("30ms  getButtons");
    getButtons();
  });

  taskManager.scheduleFixedRate(1000, [] {
    logIt("1000ms    SleepMode...");
    sleepMode();
  });

  taskManager.scheduleFixedRate(250, onMicrosJobPoti, TIME_MICROS);
  taskManager.scheduleFixedRate(500, onMicrosJobServo, TIME_MICROS);
}

void onMicrosJobPoti() {
  getPoti();
  logIt("250us getPoti");
}

void onMicrosJobServo() {
  ms = millis();
  writeServo();
  logIt("500us writeServo");
}

void loop() {
  taskManager.runLoop();
}
