# Xiao-6x10-Neopixel-Gear-Display
This repository contains the code used for running the 6x10 neopixel display to show the numbers from 0 to 99 as well as letters N and R.

This project utilizes a [SeeedStudio Xiao ESP32-C6 board](https://www.seeedstudio.com/Seeed-Studio-XIAO-ESP32C6-p-5884.html) and a [6x10 neopixel matrix display](https://www.seeedstudio.com/6x10-RGB-MATRIX-for-XIAO-p-5771.html). The goal was to create a small colorful display using neopixels for use in sim racing with SimHub. 
Unfortnately, the SimHub software does not yet support ESP32 boards out of the box. Rather, it requires some heavy modifications to the Arduino code which I'm not really good at doing. Thus, I made sure that it works over serial communication so I can have a rather slightly modified arduino code that will just send the data to ESP32 via serial. 

Digits are 6x5 in size with the leftmost column being empty to stop digits from overlapping if wrong offsets are set.

ESP-NOW requires the use of aditional ESP32 device. Device connected to the Arduino is the sender as it takes the data from Arduino through Serial and broadcasts it to other devices in range. For now, the broadcast address is set to FF:FF:FF:FF:FF to broadcast to all addresses (I'm new to this, sorry). It also has a built-in BLE capabilites so one can check with a bluetooth device whether the communication is working as intended. So far, I've only set it up to send commands but I have to add some debugging capabilites. 
Receiver side hasn't changed much other than source of the input. It went from serial.read to onDataRecv callback and pretty much everything else is the same.

# How it works
## Serial
The code first gets compiled and uploaded to board and then waits for serial connection to be established. If the communication hasn't begun, the code will just wait in setup part of the code and will not move to the loop. 

If the communication has been established, the code will then loop and wait for data to appear on serial lines. It'll then read it until it hits a new line character ("/c") and the code logic will, through some simple math (devision and modulo) determine the digits that need to be displayed. 

The sent characters could also be R or N where the code has an if statement that checks for that and simply choosews an array which corresponds to those letters. 

Other commands that I've added include "OFF" command to turn of the display because it can get obnoxiously bright and no programmer ever works in a well lit environment, and "COLOR#xxxxxx" where a HEX color can be provided to change the display's color. 

## ESP-NOW
Before uploading the code, there is a #define IS_SENDER at the line 3 in the code. By commenting this out you define the device as receiver or sender. Only the part of the code needed to run one or the other gets compiled, depending on what you defined the device as. MAKE SURE TO CHANGE THIS otherwise it will not share data between ESPs. 
Once the code is compiled and uploaded, the first ESP sets up BLE communication as well as ESP-NOW broadcast which other devices listen to. 

Second ESP32, defined as receiver (commented out #define IS_SENDER) listens for UUID and reads the data when new data is provided. The commands are still the same and functionality remains as it is in Serial version. 

# Known Bugs
Some of these bugs can be turned in to features. 
So far, I've stumbled on a bug where if you input number from 100 to 109, you will get the single digit offset to the right (default is in the middle). 110 to 119 will result in N0 to N9 being displayd and same goes for R with 120 to 129. This happens because digit[10] is a blank digit, digit[11] is an "N" and digit[12] is an "R". Logic should probably have some check implemented to check whether firtDigit variable is larger than 9 to stop this from happening. Only thing I can do is constrain the variable from 0 to 9 but I want to retain the R1 and R2 for games like Euro Truck Simulator 2 where the reverse also has gears. Also, data sent will be formated in a way that will not go in that area of numbers. 

Second, when you set the display to a combination of R and N with numbers and then change color of the display, the display will move to the last know number. For example, you went from first to neutral and then in reverse, sent change color when in reverse and the display will show first gear. Not a big issue if the data stream will be continuous, but something to keep in mind. I'm still working on this. 

# Commands

Commands you can set through serial are as follows:
```
      0 - 99 : Displays the sent number
   100 - 109 : Displays numbers form 0 to 9 offset to the right
           N : Displays the N character
   110 - 119 : Displays the N0 to N9
           R : Displays the R character
   120 - 129 : Displays the R0 to R9
         OFF : Turns the display off
COLOR#rrggbb : Sends a HEX color code to change the color of the displayed characters
```

# To be done
ESP-NOW:

~~What I'd like to add as features is definetly ESP-NOW communication so that one device can take the data from the PC and broadcast it to other devices in the vicinity. Read the serial input and broadcast all the data with their respective values and each device listens for specific data and processes it.~~

Read the Serial input and convert it to ESP-NOW.

Add a UI for sender node (I'm using Elecrow 3.5" Panel with ESP32S3) where user can input a gear, set N or R and set color. Good for debugging.

Color options: 

Having a button box which has small OLED (like generic ssd1306 display) or color diplay (like the 1.3" 240x240 IPS TFT LCD driven by     st7789) to send commands to the other ESPs on the network to change the color, brightness, refresh rate or whatever. 

Also, having an HSL color format might be good for changing the colors with just a potentiometer so implementing a logic that will convert rgb to HSL would be great.

Bugs:

- not-a-bug: Comment the entirety of the code
- ~~Fix the bug where changing the display color changes the number.~~
- Fix a bug that sets a display to 0 if something other than the listed commands is sent
- Add more service and characteristic UUIDs and use more nodes to display different data like speed or lap time and test it
- Optimize the code
