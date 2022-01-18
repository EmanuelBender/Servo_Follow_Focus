#include <pgmspace.h>



void getButtons() {

  if (ms - buttonTime > 500) {       // button inactive 500ms after press
    buttonIn     = digitalRead(buttonPin);
    buttonBool   = false;

    if (buttonIn) {

      buttonIn   = false;
      buttonBool = true;
      smoothMode = !smoothMode;
      buttonTime = ms;
      sleepTimer = ms;

      if (smoothMode) {
        for (i = 0; i < 60; i++) {  // lets Moving Average catch up with new value
          smooth1.addSample(potiValue);
        }
      }
    }
  } else if (ms - buttonTime > 200 && digitalRead(buttonPin)) {   // else  detect second button press

  }
}


void getPoti() {

  analog1.update();                            // Stage 1 Smoothing
  potiIn = analog1.getValue();

  potiValue = (potiIn * (potiIn * expo / potiEnd + 1.0)) / expo;  // Expo insertion

  if (potiValue > potiEnd) potiValue = potiEnd;                   // set Endpoint
  potiValue = (potiValue / (potiEnd / 2000.0)) + 500.00;          // map to 500 - 2500

  if (smoothMode) {
    potiOut = smooth1.addSample(potiValue);    // Stage 2 Smooth Mode
  } else {
    potiOut = potiValue;
  }
}


void writeServo() {

  if (potiOut - servoTemp >= 1 || potiOut - servoTemp <= -1) {
    servoTemp = potiOut;
    servo.writeMicroseconds(potiOut);
  }
}


void writeScreen() {

  if (potiOut - potiTemp >= 2 || potiOut - potiTemp <= -2 || buttonBool) {

    Serial.println("                                   ...Screen Update");
    potiTemp   = potiOut;
    sleepTimer = ms;

    u8g2.clearBuffer();
    if (smoothMode) {
      u8g2.drawHLine(0, 0,  64);
      u8g2.drawHLine(0, 31, 64);
    }
    u8g2.setCursor(0, fontY);
    u8g2.print((potiOut - 500.0) / 20.0, 2);    // map from 500-2500 to to 0 - 100
    u8g2.sendBuffer();
  }
}


void sleepMode() {

  if (potiOut < 504) {   //  only activate sleep when poti is near 0. change to 2496 for the other end
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
          esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1); // 1 = High, 0 = Low
          u8g2.sleepOn();
          esp_deep_sleep_start();
        }
      }
    }
  }
}
