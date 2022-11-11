# Stage2-IndEnt-Major-USB-Reader
This is the source code for my SACE Stage 2 Industry and Entrepreneurial Solutions Major project. I used an Arduino Mega, USB host shield and touchscreen shield to create a portable USB reader.

## Parts Used
 - [Touchscreen (Jaycar)](https://www.jaycar.com.au/duinotech-mega-2560-r3-board-for-arduino/p/XC4420)
 - [USB Host Board (Jaycar)](https://www.jaycar.com.au/arduino-compatible-usb-host-expansion-board/p/XC4456)
 - [Arduino Mega 2560 (Jaycar)](https://www.jaycar.com.au/duinotech-mega-2560-r3-board-for-arduino/p/XC4420)
 
## Libraries
USB Host Board: [UsbFat](https://github.com/greiman/UsbFat)\
Touchscreen: [Adafruit_GFX](https://github.com/adafruit/Adafruit-GFX-Library), [MCUFRIEND_kbv](https://github.com/prenticedavid/MCUFRIEND_kbv), [Adafruit Touchscreen](https://github.com/adafruit/Adafruit_TouchScreen)
 
## Instructions
 1) Clone this repo locally and open it in the Arduino IDE or Visual Studio Code (or any other supported Arduino IDE)
 2) Download and install the libraries above
 3) Find the controller used for your touchscreen and uncomment it's line in the MCUFRIEND_kbv.cpp file
 4) Cut the PCB trace on the USB host board between the INT solder pads
 5) Run the calibration example program for your touchscreen in the MCUFRIEND_kbv library, pasting the values from the serial port into the program
 6) Place the screen on top of the USB host board and the USB host board on top of the Arduino Mega
 7) Upload this program to the Mega
