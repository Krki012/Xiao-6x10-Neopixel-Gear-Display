// Takes serial data and updates the display accordingly
// Serial inputs:
// Number results in the number being displayed. Ranges from 0 to 99
// N results in letter N being displayed
// R results in letter R being displayed
// OFF turns off the display
// COLOR#xxyyzz changes the color of display

#include <Adafruit_NeoPixel.h>
#include "Digits.h"

const bool* digits[] = {
  number0, number1, number2, number3, number4, number5, number6, number7, number8, number9, blank, letterN, letterR
};


int r = 0;    //
int g = 255;  // Default color
int b = 0;    //
//int c;
int number = 0; // Used as index for digits[] array
int firstDigit;   //First digit of the displayed number if it's greater than 10
int secondDigit;  //Second digit of the displayed number if it's greater than 10 or the only digit if it's under 10, N or R
int xOffset;  //Number of offset pixels in x-direction (0-9) to offset the displayed digit
int yOffset;  //Number of offset pixels in y-direction (0-5) to offset the displayed digit
int lastNumber = 0;  // store last displayed number
String input = "";  //Declare input as string, clear it, leave it of unknown size

#define PIN D0  // Data in pin of the 6x10 neopixel matrix display

#define NUMPIXELS 60  // 6x10=60

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 500  // Time (in milliseconds) to pause between pixels

void setup() {
  pixels.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(10); //Set max brightness of pixels; comment out if you want to use full range of colors (0-255)
  Serial.begin(115200); //Initialize the serial communication
  while (!Serial) { //wait for serial to connect before proceeding to the loop
    ;
  }
}

void loop() {
  pixels.clear(); //clear the contents of display
  while (Serial.available()) {  //if there is data on the serial lines...
    char c = Serial.read(); //store it in to a variable c
    if (c == '\n' || c == '\r') { //if the read characters are new line or whetever the other one is...
      input.trim(); //remove the last part (the new line mark)
      if (input == "N") { //if input is N, or 11X proceed to show letter N
        drawDigit(11, 2, 0, r, g, b);  // 'N'
      } else if (input == "R") {  //if input is R, or bigger than 12X proceed to show letter R
        drawDigit(12, 3, 0, r, g, b);  // 'R'
      } else if (input == "OFF") { //self explanatory
        drawDigit(10, 0, 0, r, g, b);
      } 
      else if (input.startsWith("COLOR#")) {  //If input starts with "Color#"...
        String hexColor = input.substring(6); //read the input and ignore the Color# part
        if (hexColor.length() == 6) { //set the color data to 6 characters
          long number = strtol(hexColor.c_str(), NULL, 16);  //store the data in hex format to number
          r = (number >> 16) & 0xFF;  //extract the data for red color
          g = (number >> 8) & 0xFF; //extract the data for green color
          b = number & 0xFF;  //extract the data for blue color
          Serial.print("Color set to: ");
          Serial.print(r); Serial.print(", ");
          Serial.print(g); Serial.print(", ");
          Serial.println(b);
          //this next part is needed to update the display after color has been changed, otherwise it will be off. Does not work yet for OFF command
          int firstDigit = lastNumber / 10; //Devide the last number by 10 to get the first digit (int will ignore the decimal palces)
          int secondDigit = lastNumber % 10;  //Modulo the last number by 10 to get the reminder when deviding by 10
          int xOffset = (firstDigit == 0) ? -3 : 0; //if the number has 1 digit (smaller than 10), offset the first digit to be in center
          if (firstDigit == 0) firstDigit = 10; 
          pixels.clear();
          drawDigit(firstDigit, 0, 0, r, g, b);
          drawDigit(secondDigit, xOffset + 5, 0, r, g, b);
          pixels.show();
        }
      } else {
        int number = input.toInt();
        lastNumber = number;
        int firstDigit = number / 10;
        int secondDigit = number % 10;
        int xOffset = (firstDigit == 0) ? -3 : 0;
        if (firstDigit != 0) drawDigit(firstDigit, 0, 0, r, g, b);
        drawDigit(secondDigit, xOffset + 5, 0, r, g, b);
      }
      pixels.show();
      Serial.println(input);
      input = ""; // Reset after processing
    } else {
      input += c;
    }
  }
}

void drawDigit(int number, int xOffset, int yOffset, int r, int g, int b) {
  const bool* layout = digits[number];
  for (int y = 0; y < 6; y++) {
    for (int x = 0; x < 5; x++) {
      int i = y * 5 + x;
      if (layout[i]) {
        int px = x + xOffset;
        int py = y + yOffset;
        int index = getPixelIndex(px, py);
        pixels.setPixelColor(index, r, g, b);
      }
    }
  }
}

int getPixelIndex(int x, int y) {
  return x * 6 + (5 - y);
}
