// USB Reader Header Files
#include <SPI.h>
#include <UsbFat.h>
#include <masstorage.h>

// Display Header Files
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

// Misc Header Files
#include "Colours.hpp"

// Instantiating the display class
MCUFRIEND_kbv display;

// Touchscreen Variables
#define MINPRESSURE 1//200
#define MAXPRESSURE 1000
const int XP=8,XM=A2,YP=A3,YM=9; //240x320 ID=0x7575
const int TS_LEFT=886,TS_RT=120,TS_TOP=86,TS_BOT=903;
int touch_x, touch_y;     //Touch_getXY() updates global vars
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// Instantiating the USB reader
USB usb;
BulkOnly bulk(&usb);
UsbFat key(&bulk);

// The file
File openedFile;

// Buttons
Adafruit_GFX_Button fileButtons[6];
Adafruit_GFX_Button backButton;
Adafruit_GFX_Button upButton;
Adafruit_GFX_Button downButton;

// Constants
const int TEXT_CHARACTERS_ON_SCREEN = 1100;

// Global variables
bool isTouched = false;
int currentFileLevel = 0;
int currentTextLevel = 0;
bool viewingFile = false;
bool usbConnected = false;
String currentDirectory = "/";
int filesInCurrentDirectoryView = 0;

void setup() {
  Serial.begin(9600);
  // Initialising the screen and USB
  usbConnected = InitialiseUSB();
  InitialiseDisplay();
  display.setTextColor(TEXT_COLOUR);
  DrawUniversalBackground();

  // If a USB is connected, the directory view is drawn
  if (usbConnected) {
    DrawDirectoryView();
  }
  else {
    // If a USB is not connected, text is drawn instructing the user to do so
    display.setCursor(0, 85);
    display.setTextSize(3);
    display.println(" Please insert\n a USB drive\n and reset this\n device!");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  isTouched = Touch_getXY();
  
  if (!viewingFile) CheckButtonsDirectoryView();

  // Checking the press logic for the navigation buttons
  // Back button
  backButton.press(isTouched && backButton.contains(touch_x, touch_y));
  if (backButton.justPressed()) NavigateBack();
  if (backButton.justReleased()) backButton.drawButton(false);
  // Up button
  upButton.press(isTouched && upButton.contains(touch_x, touch_y));
  if (upButton.justPressed()) NavigateUp();
  if (upButton.justReleased()) upButton.drawButton(false);
  // Down button
  downButton.press(isTouched && downButton.contains(touch_x, touch_y));
  if (downButton.justPressed()) NavigateDown();
  if (downButton.justReleased()) downButton.drawButton(false);
}

/*--MY FUNCTIONS--*/

// Function to run the USB initialisation functions
bool InitialiseUSB() {
  if (!initUSB(&usb)) return false;
  if (!key.begin()) return false;

  return true;
}

// Function to initialise the display
void InitialiseDisplay() {
  uint16_t ID = display.readID();
  display.begin(ID);

  display.setRotation(1);
  display.fillScreen(BACKGROUND_COLOUR);
}

// Function from example to get whether the screen is touched and map the touch position
bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z >= MINPRESSURE && p.z <= MAXPRESSURE);
    if (pressed) {
        touch_x = 320 - map(p.y, TS_LEFT, TS_RT, 0, 320);
        touch_y = map(p.x, TS_TOP, TS_BOT, 0, 240);
    }
    return pressed;
}

// Function to get the files in a directory, sending it to an array
void GetFilesInDirectory(String path, String *fileNames, int arraySize) {
  // Opening the folder
  File dir = key.open(path.c_str());

  int arrayIndex = 0;
  int nameArraySize = 50;

  while (true) {
    // Opening the next file in the directory
    File entry = dir.openNextFile();
    if (!entry) break;

    // Getting the name of the file as a string
    char nameArr[nameArraySize];
    entry.getName(nameArr, nameArraySize);
    String name = String(nameArr);
    if (entry.isDir()) name = "/" + name + "/";

    // Returning if the array index is greater than the size of the array
    if (arrayIndex >= arraySize) break;

    // Adding the name to the array and incrementing the index
    *(fileNames + arrayIndex) = name;
    arrayIndex++;
  }
}

// Function to get the amount of files in a directory
uint16_t GetAmountOfFilesInDirectory(String path) {
    uint16_t amountOfFilesInDir = 0;
    
    // Opening the directory path
    File dir = key.open(path.c_str());

    while (true) {
        // Opening the next file in the directory, if it does not exist the loop is broken
        File entry = dir.openNextFile();
        if (!entry) break;

        // Incrementing the amount of files in the directory
        amountOfFilesInDir++;
    }

    return amountOfFilesInDir;
}

// Function to draw the universal background for each window type
void DrawUniversalBackground() {
  // The white background
  display.fillScreen(BACKGROUND_COLOUR);

  // The top bar
  display.fillRect(0, 0, 320, 30, BUTTON_COLOUR);
  display.drawRect(0, 0, 160 * 2, 30, TEXT_COLOUR);
  //display.drawRect(160, 0, 160, 30, BLACK);

  // The side bar
  display.fillRect(270, 29, 50, 211, BUTTON_COLOUR);
  display.drawRect(270, 29, 50, 211, TEXT_COLOUR);

  // Triangle test
  //display.drawTriangle(304-11, 107, 293-11, 142, 316-11, 142, BLACK);
  //display.fillTriangle(304-11, 107, 293-11, 142, 316-11, 142, WHITE);

  // The directory and usb information
  display.setTextColor(TEXT_COLOUR);
  display.setCursor(5, 12);
  display.print(currentDirectory);

  // The bounding black box
  display.drawRect(0, 0, 320, 240, TEXT_COLOUR);
}

// Function to initialise the navigation buttons
void InitialiseNavigationButtons() {
  // The back button
  backButton.initButton(&display, 295, 63, 50-2, 67, BUTTON_COLOUR, BUTTON_COLOUR, TEXT_COLOUR, "Back", 1);
  backButton.drawButton(false);
  // The up button
  upButton.initButton(&display, 295, 130, 50-2, 67, BUTTON_COLOUR, BUTTON_COLOUR, TEXT_COLOUR, "Up", 1);
  upButton.drawButton(false);
  // The down button
  downButton.initButton(&display, 295, 197, 50-2, 67, BUTTON_COLOUR, BUTTON_COLOUR, TEXT_COLOUR, "Down", 1);
  downButton.drawButton(false);
}

/*--THE DIRECTORY VIEW FUNCTIONS--*/
// Function to draw the directory view window
void DrawDirectoryView() {
  // Initialising the navigation buttons
  InitialiseNavigationButtons();

  viewingFile = false;
  filesInCurrentDirectoryView = 0;

  // Loading all files in directory into an array
  uint16_t amountOfFiles = GetAmountOfFilesInDirectory(currentDirectory);
  String files[amountOfFiles];
  GetFilesInDirectory(currentDirectory, files, amountOfFiles);
  
  // Initialising the file buttons
  for (int i = 0; i < 6; i++) {
    if (i + currentFileLevel * 6 >= amountOfFiles) break;

    bool nextRow = i > 2;
    fileButtons[i].initButton(&display, 45 + ((i % 3) * 90), 85 + (int) (85 * nextRow * 1.1), 
                              80, 80, TEXT_COLOUR, BUTTON_COLOUR, TEXT_COLOUR, 
                              files[i + currentFileLevel * 6].c_str(), 1);
    fileButtons[i].drawButton(false);
    filesInCurrentDirectoryView += 1;
  }
}

// Function to check the button presses in the directory view window
void CheckButtonsDirectoryView() {
  // Checking the press logic for all file buttons
  for (int i = 0; i < 6; i++) {
    // If the current button index is greater than the amount of buttons on screen, no press logic is run
    if (i + 1 > filesInCurrentDirectoryView) break;

    // Pressing the current button if the screen is touched and if the touch coords are in the button
    fileButtons[i].press(isTouched && fileButtons[i].contains(touch_x, touch_y));

    // Checking press and release logic for the buttons
    if (fileButtons[i].justPressed()) OpenFile(i);
    if (fileButtons[i].justReleased()) fileButtons[i].drawButton(false);
  }
}
/*--THE DIRECTORY VIEW FUNCTIONS--*/

// Function to navigate up
void NavigateUp() {
  upButton.drawButton(true);

  if (!viewingFile) {
    if (currentFileLevel <= 0) return;
    currentFileLevel --;

    display.fillScreen(BACKGROUND_COLOUR);
    DrawUniversalBackground();
    DrawDirectoryView();
  }
  else {
    if (currentTextLevel <= 0) return;
    currentTextLevel --;

    display.fillScreen(BACKGROUND_COLOUR);
    DrawUniversalBackground();
    DrawTextInFile();
  }
}

// Function to navigate down
void NavigateDown() {
  downButton.drawButton(true);
  
  if (!viewingFile) {
    currentFileLevel ++;
    
    display.fillScreen(BACKGROUND_COLOUR);
    DrawUniversalBackground();
    DrawDirectoryView();
  }
  else {
    currentTextLevel ++;

    display.fillScreen(BACKGROUND_COLOUR);
    DrawUniversalBackground();
    DrawTextInFile();
  }
}

// Function to navigate back
void NavigateBack() {
  backButton.drawButton(true);
  // TODO: File navigation
  if (viewingFile) {
    // Getting the name of the current file and removing it from the directory
    int fileNameBufferSize = 128;
    char fileName[fileNameBufferSize];
    openedFile.getName(fileName, fileNameBufferSize);
    currentDirectory = currentDirectory.substring(0, currentDirectory.length() - String(fileName).length());

    // Closing the file and moving to the directory view
    openedFile.close();
  }
  else {
    // Finding the index of the slash separator between directories and slicing the current directory string to remove anything beyond it
    int indexOfDirectorySlash = currentDirectory.lastIndexOf('/', currentDirectory.length() - 2);
    currentDirectory = currentDirectory.substring(0, indexOfDirectorySlash);

    // Refreshing the directory view
    currentFileLevel = 0;
  }

  display.fillScreen(BACKGROUND_COLOUR);
  DrawUniversalBackground();
  DrawDirectoryView();
}

/*--THE FILE VIEW FUNCTIONS--*/
// Function to open the file view
void OpenFile(int buttonIndex) {
  // Loading all files in directory into an array
  uint16_t amountOfFiles = GetAmountOfFilesInDirectory(currentDirectory);
  String files[amountOfFiles];
  GetFilesInDirectory(currentDirectory, files, amountOfFiles);

  // Opening the file
  String fileToOpen = currentDirectory + files[buttonIndex + currentFileLevel * 6];
  openedFile = key.open(fileToOpen.c_str());

  // Redrawing the screen to prepare for text display
  currentDirectory = fileToOpen;
  display.fillScreen(BACKGROUND_COLOUR);
  DrawUniversalBackground();
  InitialiseNavigationButtons();

  if (!openedFile.isDir()) {
    // If the file is a file then we run the draw text in file function
    currentTextLevel = 0;
    viewingFile = true;
    DrawTextInFile();
  }
  else {
    // If the file is a directory, we instead open that directory
    currentFileLevel = 0;
    DrawDirectoryView();
  }
}

// Function to draw the text in a file
void DrawTextInFile() {
  // Initialising the navigation buttons
  InitialiseNavigationButtons();

  // Setting text position settings
  int yPos = 35;
  display.setCursor(5, yPos);
  int textCharactersOnScreenTemp = TEXT_CHARACTERS_ON_SCREEN;

  // Finding the start of the file based on scroll amount and seeking to it
  int startOfFile = currentTextLevel * TEXT_CHARACTERS_ON_SCREEN;
  openedFile.seek(startOfFile);

  // Looping through each character in the file
  for (int i = startOfFile; i < ((currentTextLevel + 1) * textCharactersOnScreenTemp); i++) {
    // If the file is at the end of the line, another line is added
    if (i % 44 == 0 && i != startOfFile) {
      yPos += 8;
      display.setCursor(5, yPos);
    }

    // Reading the next character of the file
    char currentCharacter = (char) openedFile.read();
    
    // If the current character is a newline, a character is printed to show a newline
    if (currentCharacter == '\n')
      display.print((char) 176); // Long Term TODO: Add newlines that actually work (The character displayed is '░')
    // If not then the character is printed
    else
      display.print(currentCharacter);
  }
}
/*--THE FILE VIEW FUNCTIONS--*/

/*
  As of 26/08/2022 12:32pm, the coding for my major project is complete!
  There are no bugs that I have been able to encounter with this current codebase
  The only future thing I could add is proper newline support, as right now newlines are replaced with '░' due to it breaking my text system
*/
