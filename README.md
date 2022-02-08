# **Smooth Dual-mode Servo driven Follow Focus**
**3D printed enclosure, 0.42" OLED, Potentiometer, 2-Stage smoothing, Expo Setting, and Sleep Mode for ESP32**
###### by eBender [Emanuel Bender] 
<br/>

 
**Features**  <br/>
- Automatic Idling to save power and gear<br/>
- Expo Setting <br/>
- Deep Sleep Mode /w Timer (engages when poti set to 0) <br/>
- Button switches between:<br/> 
Responsive Mode (Stage 1 Smoothing with ResponsiveAnalogRead) <br/>
Smooth Mode&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; (Stage 1 + Stage 2 Smoothing with Moving Average)  <br/><br/>

**Specs & Speeds** <br/> 
Servo Speed:&nbsp;&nbsp; 0.06sec     <br/>
Servo Update: 333Hz      <br/>
Poti: &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 333Hz      <br/>
Screen: &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;50fps     <br/>

**Variables**  <br/>
smoothValue&nbsp;       0-255 -    applies Moving Average smoothing strength <br/>
Expo&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  0.0-4.0 -  applies exponential curve to the poti input <br/>
Hertz&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;       50-333Hz - for different Servo models and update speeds <br/>
TaskManagerIO speed 1-4&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;       changes update frequency off all tasks and subsequent update speeds  <br/>
<br/>

**Parts** <br/>
ESP32 (minimum: Microcontroller /w I2C, 2 Inputs, 1 PWM Output)  <br/>
0.42" 32x64 OLED I2C Screen  <br/>
25g S0025M Servo (0.06-0.08s, 3KG, 333Hz, 2BB, MG)  <br/>
10k Potentiometer  <br/>
1x Momentary Button  <br/>
INA219 Voltage Current Meter  <br/>
2S BMS Balance Module  <br/>
2S Battery Charger TP5100  <br/>
6v 3A Buck Voltage Converter  <br/>
2x 18650 Battery  <br/>
3D Printed enclosure (files coming to thingyverse)  <br/>
22/24AWG Wires, M3 or M5 Screws & Inserts  <br/>

**Issues:** <br/>
Servo Motor is too loud for quiet filming / Audio recording  <br/>
might switch to Stepper Motor
