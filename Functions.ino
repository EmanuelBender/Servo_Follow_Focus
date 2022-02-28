#include <pgmspace.h>

void idle() { // detect idling to slow CPU and TaskManagerIO down when not explicitly put into sleep mode
  us = micros();

  if (ms - sleepTimer > idleTimer) {
    // u8g2.clearBuffer();
    u8g2.setDrawColor(0);
    u8g2.drawRBox(15, 8, 36, 18, 5);
    u8g2.setDrawColor(1);
    u8g2.setFont(u8g2_font_freedoomr10_tu);
    u8g2.setCursor(16, 24);
    u8g2.print("IDLE");

    // u8g2.setFont(u8g2_font_tenstamps_mn);
    // u8g2.setCursor(5, 27);
    // u8g2.print((potiOut - 500.0) / 20.0, 2);
    u8g2.sendBuffer();

    idleOn = true;
    taskManager.reset();
    setCpuFrequencyMhz(80);
    taskManager.scheduleFixedRate(125,              getPoti,      TIME_MILLIS);   // 8hz
    // taskManager.scheduleFixedRate(2 * tmMultiplier, writeScreen,  TIME_SECONDS);
    if (sleepMode) {
      sleepID = taskManager.scheduleFixedRate(1,    getSleepMode, TIME_SECONDS);  // 1hz
    }
#ifdef DEBUG
    Serial.println("Idling...");
    Serial.print("CPU: ");
    Serial.print(getCpuFrequencyMhz());
    Serial.println("Mhz");
#endif

    while (ms - sleepTimer > idleTimer) {
      taskManager.runLoop();
      if (potiOut - servoTemp >= 4 || potiOut - servoTemp <= -4) {
        break;
      }
    }

    taskManager.reset();
    setCpuFrequencyMhz(240);
    idleOn   = false;
    idleTemp = potiOut;
    u8g2.setFont(font);

#ifdef DEBUG
    Serial.println("");
    Serial.println("Normal Mode");
    Serial.print("CPU: ");
    Serial.print(getCpuFrequencyMhz());
    Serial.println("Mhz");
#endif

    taskManager.scheduleFixedRate(3  * tmMultiplier,   getPoti,      TIME_MILLIS);   // 333hz
    taskManager.scheduleFixedRate(3  * tmMultiplier,   writeServo,   TIME_MILLIS);   // 333hz bc servo updates @ 333hz
    taskManager.scheduleFixedRate(20 * tmMultiplier,   writeScreen,  TIME_MILLIS);   // 50fps
    if (sleepMode) {
      sleepID = taskManager.scheduleFixedRate(1,       getSleepMode, TIME_SECONDS);  // 1hz
    }
    taskManager.scheduleFixedRate(250,                 idle,         TIME_MILLIS);   // 4hz
    taskManager.runLoop();
  }

#ifdef DEBUG
  codeTime = micros() - us;
  logIt(" 1s     checkIdle", codeTime);
#endif

}

void interruptTask(pintype_t thePin) {
  getButtons();
}

void getButtons() {
  us = micros();

  if (ms - buttonTime > 300) {                                   // button inactive 300ms after press

    buttonBool = true;
    smoothMode = !smoothMode;
    buttonTime = ms;
    sleepTimer = ms;

    if (smoothMode) {              // lets Moving Average catch up with new value when changing modes
      for (i = 0; i < 145 / tmMultiplier + 5; i++) {
        smooth1.addSample(potiValue);
      }
    }
#ifdef DEBUG
    Serial.println("                        Button Press");
#endif

  } else if (ms - buttonTime < 750) {  // double click detection
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_freedoomr10_tu);

    if (!sleepMode) {
      u8g2.setCursor(13, 14);
      u8g2.print("SLEEP");
      u8g2.setCursor(4, 30);
      u8g2.print("ENABLED");
    } else if (sleepMode) {
      u8g2.setCursor(13, 14);
      u8g2.print("SLEEP");
      u8g2.setCursor(0, 30);
      u8g2.print("DISABLED");
    }
    u8g2.sendBuffer();
    delay(1000);
    u8g2.clearBuffer();
    u8g2.setFont(font);

    sleepMode  = !sleepMode;
    if (sleepMode) {
      sleepID = taskManager.scheduleFixedRate(1, getSleepMode, TIME_SECONDS);  // 1hz
    } else {
      taskManager.cancelTask(sleepID);
    }
    buttonBool = true;
    buttonTime = ms;
    sleepTimer = ms;
#ifdef DEBUG
    Serial.println("                        Button Double Press");
    Serial.print("                        Sleep Mode: ");
    Serial.println(sleepMode);
#endif
  }

#ifdef DEBUG
  codeTime = micros() - us;
  logIt(" 50ms   getButtons", codeTime);
#endif
}


void getPoti() {
  us = micros();
  ms = millis();

  analog1.update();                            // Stage 1 Smoothing
  potiIn = analog1.getValue();

  potiValue = (potiIn * (potiIn * expo / potiEnd + 1.0)) / expo;  // Expo insertion
  if (potiValue > potiEnd) potiValue = potiEnd;                   // set Endpoint
  potiValue = (potiValue / (potiEnd / 2000.0)) + 500.00;          // map to 500 - 2500 for the servo

  if (smoothMode && !idleOn) {
    potiOut = smooth1.addSample(potiValue);    // Stage 2 Smooth Mode
  } else {
    potiOut = potiValue;
  }

  if (potiOut - idleTemp >= 4 || potiOut - idleTemp <= -4) { // trigger wakeup with poti movement +-4
    idleTemp   = potiOut;
    sleepTimer = ms;
  }

#ifdef DEBUG
  codeTime = micros() - us;
  if (idleOn) {
    logIt("125ms  getPoti", codeTime);
  } else {
    logIt("3ms    getPoti", codeTime);
  }
#endif

}

void writeServo() {
  us = micros();

  if (potiOut - servoTemp >= 1 || potiOut - servoTemp <= -1) {
    servoTemp = potiOut;
    servo.writeMicroseconds(potiOut);
  }
#ifdef DEBUG
  codeTime = micros() - us;
  logIt(" 3ms    writeServo", codeTime);
#endif
}


void writeScreen() {
  us = micros();


  if (potiOut - potiTemp >= 1 || potiOut - potiTemp <= -1 || buttonBool) {
    buttonBool = false;
    potiTemp   = potiOut;

    u8g2.clearBuffer();
    if (smoothMode) {
      u8g2.drawHLine(0, 0,  64);
      u8g2.drawHLine(0, 31, 64);
    }
    u8g2.setCursor(0, fontY);
    u8g2.print((potiOut - 500.0) / 20.0, 2);    // map from 500-2500 to 0 - 100
    u8g2.sendBuffer();
  }

#ifdef DEBUG
  codeTime = micros() - us;
  logIt("20ms    writeScreen", codeTime);
#endif

}


void getSleepMode() {
  us = micros();

  if (potiOut < 530 && idleOn) {   //  only activate sleep when poti is near 0. change to 'potiOut > 2450' for the other end
    if (ms - sleepTimer > sleepOff - 10000) {
      timeOff = ms;

      while (ms - sleepTimer > sleepOff - 10000) {
        ms = millis();

        getPoti();
        // getButtons();

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_freedoomr10_tu);
        u8g2.setCursor(13, 12);
        u8g2.print("SLEEP");

        u8g2.setFont(u8g2_font_tenstamps_mn);
        u8g2.setCursor(27, 26);
        u8g2.print((((ms - timeOff) * -1) + 10000) / 1000);
        u8g2.sendBuffer();

        if (ms - sleepTimer > sleepOff) {
          esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1); // 1 = activeHigh, 0 = activeLow
          u8g2.sleepOn();
          servo.detach();
          esp_deep_sleep_start();
        }

        if (potiOut - servoTemp >= 4 || potiOut - servoTemp <= -4 || buttonBool) {
          u8g2.setFont(font);
          sleepTimer = ms;
          break;
        }
      }
    }
  }
#ifdef DEBUG
  codeTime = micros() - us;
  logIt("  1s     sleepMode ", codeTime);
#endif
}
