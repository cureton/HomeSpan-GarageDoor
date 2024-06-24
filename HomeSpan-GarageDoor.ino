/*********************************************************************************
 *  MIT License
 *  
 *  Copyright (c) 2020-2022 Gregg E. Berman
 *  
 *  https://github.com/HomeSpan/HomeSpan
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *  
 ********************************************************************************/
 

#include "HomeSpan.h"

struct invertedLED : Blinkable {        // create a child class derived from Blinkable

  int pin;                              // variable to store the pin number
  
  invertedLED(int pin) : pin{pin} {     // constructor that initializes the pin parameter
    pinMode(pin,OUTPUT);                // set the pin to OUTPUT
    digitalWrite(pin,HIGH);             // set pin HIGH (which is off for an inverted LED)
  }

  void on() override { digitalWrite(pin,LOW); }        // required function on() - sets pin LOW
  void off() override { digitalWrite(pin,HIGH); }      // required function off() - sets pin HIGH
  int getPin() override { return(pin); }               // required function getPin() - returns pin number
};

struct DEV_GarageDoor : Service::GarageDoorOpener {
  Characteristic::CurrentDoorState *current_state;
  Characteristic::TargetDoorState *target_state;
  Characteristic::ObstructionDetected *obstruction_state;
  
  DEV_GarageDoor() : Service::GarageDoorOpener() { 

    current_state=new Characteristic::CurrentDoorState(1);              // initial value of 1 means closed
    target_state=new Characteristic::TargetDoorState(1);                // initial value of 1 means closed
    obstruction_state=new Characteristic::ObstructionDetected(false);   // initial value of false means NO obstruction is detected
    
    Serial.print("Configuring Garage Door Opener");   // initialization message
    Serial.print("\n");
  } // end constructor

  // Finally, we over-ride the default update() method with instructions that actually turn on/off the LED.  Note update() returns type boolean

  boolean update(){            
    Serial.print("Update Called\n");
    switch(target_state->getVal()) {
      case 0: //Closed
        Serial.print("Target is CLOSED\n");
        if(current_state->getNewVal() != 1)
          current_state->setVal(3);
      break;

      case 1: //Open
        Serial.print("Target is OPEN\n");
        if(current_state->getNewVal() != 0) 
          current_state->setVal(2);
      break;
    } 
    return(true);
  }

  void loop(){ 
    //Serial.print("Loop Called\n");
    switch (current_state->getVal()) {
      case 0: //Open
      break;

      case 1: //Closed
      break;
      case 2: //Opening
        if (target_state->timeVal() > 10000) {
          current_state->setVal(0);
          Serial.print("Finished Opening\n");
        }
      break;
      case 3: //Closing
      if (target_state->timeVal() > 10000) {
          current_state->setVal(1);
          Serial.print("Finished Closing\n");
        }
      break;
      case 4: //Stopped
      break;
      default:
      break;
    }
  }
};

void setup() {

  /* Set control pin to pin zero of the Helteck WIFI 32 V3 */
  homeSpan.setControlPin(0, PushButton::TRIGGER_ON_LOW);
  
  /* Set status device to ping 32 for Heltec WIFI 32 V3 */
  homeSpan.setStatusDevice(new invertedLED(35));    // set Status LED to be a new Blinkable device attached to pin 0

  Serial.begin(115200);       // Start a serial connection so you can receive HomeSpan diagnostics and control the device using HomeSpan's Command-Line Interface (CLI)

  // IMPORTANT: The Name you choose below MUST BE UNIQUE across all your HomeSpan devices!

  homeSpan.begin(Category::GarageDoorOpeners,"Virtual Garage Door2");   

  new SpanAccessory();                              // Begin by creating a new Accessory using SpanAccessory(), no arguments needed
    new Service::AccessoryInformation();            // HAP requires every Accessory to implement an AccessoryInformation Service
      new Characteristic::Identify();               // Create the required Identify Characteristic  

      new Characteristic::Manufacturer("dcureton.com");   // Manufacturer of the Accessory (arbitrary text string, and can be the same for every Accessory)
      new Characteristic::SerialNumber("000001");    // Serial Number of the Accessory (arbitrary text string, and can be the same for every Accessory)
      new Characteristic::Model("Garage Door Controller");     // Model of the Accessory (arbitrary text string, and can be the same for every Accessory)
      new Characteristic::FirmwareRevision("0.0");    // Firmware of the Accessory (arbitrary text string, and can be the same for every Accessory)
    new DEV_GarageDoor();
}

void loop(){
  homeSpan.poll();
}

