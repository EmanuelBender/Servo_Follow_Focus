#include <pgmspace.h>

void idle() { // detect idling to slow CPU and TaskManagerIO down when not explicitly put into sleep mode
  us = micros();

  if (potiOut - idleTemp >= 3 || potiOut - idleTemp <= -3) {
    idleTemp   = potiOut;
    sleepTimer = ms;
  }

  if (ms - sleepTimer > idleTimer) {
    idleOn = true;
    taskManager.reset();
    setCpuFrequencyMhz(80);
    taskManager.scheduleFixedRate(100,            getPoti,      TIME_MILLIS);   // 10hz
    if (sleepMode) {
      sleepID = taskManager.scheduleFixedRate(1,  getSleepMode, TIME_SECONDS);  // 1hz
    }
#ifdef DEBUG
    Serial.println("Idling...");
    Serial.print("CPU: ");
    Serial.print(getCpuFrequencyMhz());
    Serial.println("Mhz");
#endif

    while (ms - sleepTimer > idleTimer) {
      taskManager.runLoop();
      if (potiOut - servoTemp >= 3 || potiOut - servoTemp <= -3) {
        break;
      }
    }

    taskManager.reset();
    setCpuFrequencyMhz(240);
    idleOn   = false;
    idleTemp = potiOut;
#ifdef DEBUG
    Serial.println("");
    Serial.println("Normal Mode");
    Serial.print("CPU: ");
    Serial.print(getCpuFrequencyMhz());
    Serial.println("Mhz");
#endif

    taskManager.scheduleFixedRate(3 *  tmMultiplier,   getPoti,      TIME_MILLIS);   // 333hz
    taskManager.scheduleFixedRate(3 *  tmMultiplier,   writeServo,   TIME_MILLIS);   // 333hz bc servo updates @ 333hz
    taskManager.scheduleFixedRate(20 * tmMultiplier,   writeScreen,  TIME_MILLIS);   // 50fps
    if (sleepMode) {
      sleepID = taskManager.scheduleFixedRate(1,       getSleepMode, TIME_SECONDS);  // 1hz
    }
    taskManager.scheduleFixedRate(250,                 idle,         TIME_MILLIS);   // 4hz
    taskManager.runLoop();
  }

#ifdef DEBUG
  codeTime = micros() - us;
  logIt(" 250ms  checkIdle", codeTime);
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

  } else if (ms - buttonTime < 600) {  // double click detection
    sleepMode  = !sleepMode;
    if (sleepMode) {
      sleepID = taskManager.scheduleFixedRate(1, getSleepMode, TIME_SECONDS);  // 1hz
    } else {
      taskManager.cancelTask(sleepID);
    }
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
#ifdef DEBUG
  codeTime = micros() - us;
  if (idleOn) {
    logIt("100ms  getPoti", codeTime);
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

  if (potiOut < 510) {   //  only activate sleep when poti is near 0. change to 'potiOut > 2495' for the other end
    if (ms - sleepTimer > sleepOff - 10000) {
      timeOff = ms;

      while (ms - sleepTimer > sleepOff - 10000) {
        ms = millis();

        getPoti();
        getButtons();

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_freedoomr10_tu);
        u8g2.setCursor(14, 12);
        u8g2.print("SLEEP");

        u8g2.setFont(u8g2_font_tenstamps_mn);
        u8g2.setCursor(28, 26);
        u8g2.print((((ms - timeOff) * -1) + 10000) / 1000);
        u8g2.sendBuffer();

        if (potiOut - servoTemp >= 4 || potiOut - servoTemp <= -4 || buttonBool) {
          u8g2.setFont(font);
          sleepTimer = ms;
          break;
        }

        if (ms - sleepTimer > sleepOff) {
          esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1); // 1 = activeHigh, 0 = activeLow
          u8g2.sleepOn();
          esp_deep_sleep_start();
        }
      }
    }
  }
#ifdef DEBUG
  codeTime = micros() - us;
  logIt("  1s     sleepMode ", codeTime);
#endif
}
