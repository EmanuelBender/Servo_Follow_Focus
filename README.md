# **Servo Follow Focus with Task Manager**
**Dual-Mode 3D-Printed Servo Follow Focus, 2-Stage smoothing, Expo, Potentiometer, 0.42" OLED and Sleep Mode for ESP32**
###### by eBender [Emanuel Bender]
 <br/>
 
**Features**  <br/>
Normal Mode (Stage 1 Smoothing with ResponsiveAnalogRead) <br/>
Smooth Mode (Stage 1 + Stage 2 Smoothing with Moving Average)  <br/>
Sleep Mode Timer  0-64s - engages when poti set to 0 <br/>
    
**Specs & Speeds** <br/> 
Servo Speed S0025M: 0.06sec     <br/>
Servo Update: 333Hz      <br/>
Poti: 333Hz      <br/>
Screen: 50fps     <br/>

**Variables**  <br/>
smoothValue       0-255 -    applies Moving Average smoothing strength <br/>
Expo              0.0-4.0 -  applies exponential to the poti input <br/>
Hertz             50-333Hz - for different Servo models and update speeds <br/>

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
