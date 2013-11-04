
/*************************************************** 
 * HD Tune - Motorcycle OBD Analyzer
 * 
 ****************************************************/
#include <SoftTimer.h>
#include <MenuBackend.h>
//#include <aJSON.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1331.h>
#include <SD.h>
#include <SPI.h>
//#include <stdio.h>

// Oled
// If we are using the hardware SPI interface, these are the pins (for future ref)
#define sclk 13
#define mosi 11
#define cs   10
#define rst  9
#define dc   8

// NeoPixel
#define NEO_PIN 3


// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

// to draw images from the SD card, we will share the hardware SPI interface
Adafruit_SSD1331 display = Adafruit_SSD1331(cs, dc, rst);  

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, NEO_PIN, NEO_GRB + NEO_KHZ800);


// Menu
MenuBackend menu = MenuBackend(menuUseEvent,menuChangeEvent);

MenuItem miSettings = "Settings";
MenuItem miShiftLight = MenuItem("Shift Light");
MenuItem miConfigReset = MenuItem("Reset Configs");

MenuItem miRevMode = MenuItem("Set Rev Mode");
MenuItem miRevLimitMode = MenuItem("Set Shift Mode");
MenuItem miRevLimitRPM = MenuItem("Set Shift RPM");

MenuItem miTemps = MenuItem("Bike Temps");
MenuItem miGears = MenuItem("Gear Indicator");

// For Arduino Uno/Duemilanove, etc
//  connect the SD card with MOSI going to pin 11, MISO going to pin 12 and SCK going to pin 13 (standard)
//  Then pin 4 goes to CS (or whatever you have set up)
#define SD_CS 4    // Set the chip select line to whatever you use (4 doesnt conflict with the library)



// the file itself
//File splashImage;
//File cfgFile;

String content = "";
char character;


const int BUTTON_PIN = 2;
const int BUTTON2_PIN = 12;
const int LED_PIN =  13;

int revLimit = 9000;

// Config params
// 0 : revshift_rpm (9000) 0x2328
// 1 : rev_ledmode (0) 0x0000
// 2 : rev_ledcolor (0x07E0)
// 3 : revshift_ledmode (0) 0x0000 (1) 0x0001
// 4 : revshift_ledcolor (0xF800)
Task t1(2000, checkSerial);
Task t2(100, checkSwitch);
Task t3(100, checkSwitch2);

void setup(void) {
  
  Serial.begin(9600);
  
  pinMode(cs, OUTPUT);
  digitalWrite(cs, HIGH);
  
  //pinMode(A5, INPUT_PULLUP);
  //pinMode(BUTTON_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP); 
  
  display.begin();
  strip.begin();

  if (!SD.begin(SD_CS)) {
     showMessage("SD Card Error", 0);
     return;
  }
  
  initConfig();
  
  setupDisplays();
  menuSetup();
  
  SoftTimer.add(&t1);
  SoftTimer.add(&t2);
  SoftTimer.add(&t3);
}

void initConfig() {
  if(!SD.exists("hdconfig.txt")) {
    createConfig();
  }
}

void createConfig() {
  File cfgFile = SD.open("hdconfig.txt", FILE_WRITE);
  char cfg[] = "9000,0,0x07E0,0,0xF800";
  //unsigned short rpm = sscanf("0x2328", "%hu");
  cfgFile.print(cfg);
  cfgFile.close();
  showMessage("Config Initialized", 1000);
}

void resetConfig() {
   SD.remove("hdconfig.txt");
   initConfig(); 
}


void menuSetup()
{
  menu.getRoot().addChild(miSettings);//.addChild(miShiftLight).addSibling(miConfigReset);
  //miTemps.addSibling(miGears);
  //miGears.addSibling(miSettings);
  //miTemps.addLeft(menu.getRoot());
  //miGears.addLeft(menu.getRoot());
  //miSettings.addLeft(menu.getRoot());
  //menu.getRoot().add(miGears);
  //menu.getRoot().add(miSettings); 
  menu.moveRight();//(miTemps);
  
}

int buttonState = 1;
int button2State = 1;

void checkSwitch2(Task* me) {
    int last2State = button2State;
    button2State = digitalRead(BUTTON2_PIN);
    Serial.println(button2State);
     if (button2State != last2State) {
       if (button2State == 0) {
         menu.moveRight();
       } 
     } 
}

void checkSwitch(Task* me) {
  int lastState = buttonState;
  buttonState = digitalRead(BUTTON_PIN);//analogRead(5);//
  //Serial.println(buttonState);
  if (buttonState != lastState) { 
    if (buttonState == 0) {
          menu.moveLeft();
          //return;
    }
  }
     //Serial.println(buttonState);

      //      int sensorValue = digitalRead(12);
      //    Serial.println(sensorValue);
      //    if (sensorValue == 1) {
      //      menu.moveDown(); 
      //    }
}

void checkSerial(Task* me) {
  
 if (Serial.available()){

    while(Serial.available()) {
      character = Serial.read();
      content.concat(character);
    }

    if (content != "") {

//      //display.print(content);
      if (content == "s") {
        //display.print("down");
        menu.moveDown();
        //showMenu();
      } 
      else if (content == "w") {
        //display.print("up");
        menu.moveUp();
        //showMenu();
      }
      else if (content == "a") {
        //display.print("up");
        menu.moveLeft();
        //showMenu();
      } 
      else if (content == "d") {
        //display.print("up");
        menu.moveRight();
        //showMenu();
      }else if (content == "r") {
         resetConfig();
         showMessage("Config restored to defaults", 1000);
      } else if (content == "d") {
         SD.remove("hdconfig.txt");
         showMessage("Config deleted", 1000);
      } else {

      }
      content = "";
    }
  }
}

//void loop() {
  
  
      // buttonState = digitalRead(BUTTON_PIN);//analogRead(5);//
   
        //  Serial.println(buttonState);
          //delay(150);
      //      int sensorValue = digitalRead(12);
      //    Serial.println(sensorValue);
      //    if (sensorValue == 1) {
      //      menu.moveDown(); 
      //    }
      //    delay(1); 
//      
 
  
//}


void menuUseEvent(MenuUseEvent used)
{
  //Serial.print("Menu use ");
  //Serial.println(used.item.getName());
  if (used.item == miTemps) //comparison using a string literal
  {
    Serial.println("menuUseEvent found Bike Temps");
  }
}

void menuChangeEvent(MenuChangeEvent changed)
{
  //Serial.print("Menu change ");
  //Serial.print(changed.from.getName());
  //Serial.print(" -> ");
  //Serial.println(changed.to.getName());
  showMenu();
  //showMenu(changed.to); 
}


void showMenu() {
  display.setCursor(0,0);
  display.fillScreen(BLACK);
  display.setTextColor(BLACK, WHITE);
  MenuItem current = menu.getCurrent();
  MenuItem* parent = current.getParent();
  display.print(current.getName());
  display.print("\n");
  display.setTextColor(GREEN, WHITE);
  
  //String parentname = parent->getName();
  //display.print(parentname);
  //display.setTextColor(WHITE, BLACK);
  //MenuItem* next = parent->getChild();
  //Serial.println(next->getName());
  //bool run = true;
//  while(next) {
//    String name = next->getName();
//    display.print(name+"\n");
//       next = next->getSiblingNext();
//      //if(next) run = false; 
//  }
}



void setupDisplays() {

  //Serial.println("init");
  strip.show();

  display.fillScreen(RED);
  colorWipe(strip.Color(55, 0, 0), 10); // Red

  delay(100);

  display.fillScreen(GREEN);
  colorWipe(strip.Color(0, 255, 0), 10); // Green

  delay(100);

  colorWipe(strip.Color(0, 0, 0), 10); // Green

  //Serial.print("Initializing SD card...");

  display.setCursor(0,0);

  display.fillScreen(BLACK);
  display.print("HD-Tune v0.1.0");
//  unsigned short rpm = sscanf("0x2328", "%hu");
//  char rpmValue[5];
//  sprintf(rpmValue, "%x", rpm);
//  display.print(rpmValue);
  delay(1000);
  display.fillScreen(BLACK); 
}

void showMessage(char* message, int time) {
  display.setCursor(0,0);
  display.fillScreen(BLACK); 
  display.print(message);
  if (time > 0) {
     //delay(time); 
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}



