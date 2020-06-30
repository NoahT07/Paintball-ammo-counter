#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <D:\Schnellzugriffordner\Noah\Documents\Arduino\projects\Bullet Counter\arduino\sketch_jun27a\modified_font.h>
#include <EEPROM.h>

//DISPLAY SETUP ----------------------------------------------------------------------------

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//DISPLAY SETUP END ------------------------------------------------------------------------


bool plusbuttonpressed = false;           // will check if plusbutton has been released to stop registering it multiple times
bool minusbuttonpressed = false;          // same here
bool detectbuttonpressed = false;         // same here
bool splash = true;                       // will check if splash screen has been shown
bool screenblinkon = true;                // will check if screen has previously been on or off for blinking at 00
const int cursorx = 12;                   // cursor X position for displaying Counter 
const int cursory = 60;                   // cursor Y position for displaying Counter 
const int textsize = 2;                   // size of number
const int detectbutton = 4;               // arduino pin of the bullet-detect-button
const int plusbutton = 2;                 // arduino pin of the plus-button
const int minusbutton = 3;                // arduino pin of the minus-button
int counter = 20;                         // Counts ammo
int magcapacity = 20;                     // Set magcapacity to 20 in case reading from EEPROM fails
int EEPROMaddress = 0;

String counterstring = String(counter);   // stores value of counter converted to a String for displaying

unsigned long previousMillis = 0;         // will store last time screen blinking was updated in milliseconds since power-up
int interval = 250;                       // interval at which to blink (milliseconds)
unsigned long currentMillis = millis();   // set currentMillis to time in milliseconds since power-up

void setup(){
Serial.begin(9600);

if (EEPROM.read(EEPROMaddress) > 0 && EEPROM.read(EEPROMaddress) < 99) {    // read magcapacity from EEPROM
  magcapacity = EEPROM.read(EEPROMaddress);
  counter = magcapacity;
}

pinMode(detectbutton, INPUT_PULLUP);      // pinMOdes set to INPUT_PULLUP so no pullup-resistor is needed (doesn't work on all Arduinos)
pinMode(minusbutton, INPUT_PULLUP);
pinMode(plusbutton, INPUT_PULLUP);

  // DISPLAY SETUP ----------------------------------------------------------------------------

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("TMC Ammo Counter v0.1"));
  display.display();

  
  // DISPLAY SETUP END -------------------------------------------------------------------------
}



void loop() {

  currentMillis = millis();                           // set currentMillis to milliseconds since power-up
  
  if (splash == true) {                               // clear Display after Splash screen shown for 3 seconds, then start showing counter
    delay(3000);
    display.clearDisplay();
    display.display();
    displayCounter();
    splash = false;
  }
  
  
  while(digitalRead(detectbutton) == HIGH) {          // since I am using the internal pullup resistors, button states are switched. HIGH = not pressed, LOW = pressed.
    currentMillis = millis();
    detectbuttonpressed = false;
    
    if (digitalRead(plusbutton) == HIGH) {
      plusbuttonpressed = false;                      // If button release is detected, set plusbuttonpressed to false, stops multiple button registrations
      delay(10);                                      // 10ms delay because I still had issues with multiple registrations, this fixed it.
    }
  
    if (digitalRead(minusbutton) == HIGH) {
      minusbuttonpressed = false;                     // same as above
      delay(10);
    }
  
    if (digitalRead(detectbutton) == HIGH) {
      detectbuttonpressed = false;                    // same as above
      delay(10);
    }
  
  
    
    if (digitalRead(plusbutton) == LOW) {             // If plus-button pressed
      if (plusbuttonpressed == false) {               // If release has been registered before
        if (counter != magcapacity) {                 // If counter is not equal to magcapacity set it to magcapacity and display it.
          counter = magcapacity;
          counterstring = String(counter);
          displayCounter();
          plusbuttonpressed = true;                   // register a button press
          delay(10);
        } else {
          magcapacity = magcapacity + 1;              // If counter is equal to magcapacity, add 1 to magcapacity, set counter to magcapcity and display it.
          counter = magcapacity;
          //WRITE MAGCAPACITY TO EEPROM
          EEPROM.update(0, magcapacity);
          // --------------------------
          counterstring = String(counter);
          displayCounter();
          plusbuttonpressed = true;                   // register a button press
          delay(10);
        }
      }
    }
  
  
  
    if (digitalRead(minusbutton) == LOW) {            // Same as plus-button
      if (minusbuttonpressed == false) {
        if (counter != magcapacity) {
          counter = magcapacity;
          counterstring = String(counter);
          displayCounter();
          minusbuttonpressed = true;                       
          delay(10);
        } else {
          magcapacity = magcapacity -1;
          counter = magcapacity;
          //WRITE MAGCAPACITY TO EEPROM
          EEPROM.update(0, magcapacity);
          // --------------------------
          displayCounter();
          minusbuttonpressed = true;                       
          delay(10);
        }
      }
    }
    
    blinking();                                       // call blinking() to start blinking if counter is at 00 
                                                      // or to make sure display is on if blinking has been interrupted while screen is off 
    
  } // END WHILE
  
  if (detectbuttonpressed == false) {
    if (counter > 0) {
      counter = counter -1;
    }
    displayCounter();
    detectbuttonpressed = true;
  }

    currentMillis = millis();
    blinking();
    
} // END LOOP




// DISPLAY COUNTER ------------------------------------------------
void displayCounter() {
  
  // Put 0 in front of counter if counter < 10 and >=0 ---------
  if (counter < 10 && counter >= 0) {
    counterstring = "0";
    counterstring.concat(String(counter));
  } else {
    counterstring = String(counter);
  }
// -------------------------------------------------------------
  
  if (screenblinkon == true) {                                        // checks if screen is currently supposed to be on
    display.clearDisplay();                                           // then displays Counter on screen
    display.setFont(&telegrama_render20pt7b);
    display.setTextSize(textsize);
    display.setCursor(cursorx, cursory);
    display.println(counterstring);
    display.display();
  }

}
// DISPLAY COUNTER END --------------------------------------------




// MAKE NUMBER BLINK AT 00 ----------------------------------------
void blinking() {
  if (counter == 0) {
    int timepassed = currentMillis - previousMillis;
    if (timepassed >= interval) {
      previousMillis = currentMillis;
      if (screenblinkon == true) {                                     // if screen is off turn it on and vice-versa:
        screenblinkon = false;
        display.clearDisplay();
        display.display();
        delay(10);
      } else {
        screenblinkon = true;
        displayCounter();
        delay(10);
      }
      
    }
  } else {
    screenblinkon = true;
    displayCounter();
  }
}
// MAKE NUMBER BLINK AT 00 END-------------------------------------
