////////////////////////////////////////////////////////////////////////////
// Flipdot_Firmware
// CC-BY-SA-NC    8/2016 C-Hack (Ralf@Surasto.de)
//  added support for 9x16 font   by robotfreak
//  added support for graphics    by robotfreak
//
// This main program runs on the Arduino and waits for commands via the RS232
// For simplicity the commands are handed over as an ascii string so that
// they can be easily created by scripts
//
// Command format
//   <Command>,<Color>,<x>,<y>,<size>,<....string....>\n
//
//   Commands:
//     C  Clear Screen
//     B  Draw a Bitmap
//     P  Print Text
//     H  Draw a horizontal line
//     V  Draw a vertical line
//     S  Set a pixel
//     U  Update Panel
//   Color:
//     B  Black
//     Y  Yellow
//   X,Y:
//     Required for all Print commands
//     Only Y is required for the horizontal line command "H"
//     Only X is required for the vertical line command "V"
//   size:
//     S SMALL
//     M MEDIUM
//     L LARGE
//     X EXTRALARGE
//   String:
//     Contains the characters to be printed
//   "\":
//     The command lines is terminated by the return character
//     It gets evaluated after reception of that character
//
////////////////////////////////////////////////////////////////////////////

#include "LedMatrix.h"

int i, j;
int inByte;
String commandLine;

void setup() {

  Serial.begin(115200);
  LedMatrix_begin();
  LedMatrix_update();
}

void loop() {
  char c;
  int color;
  unsigned char cmd;
  int cmdPtr;
  int xVal, yVal;
  int xSiz, ySiz;
  char fontSize;
  int fsize;

  String xStr, yStr;
  String xSizStr, ySizStr;
  String outputString;

  if (Serial.available() > 0) {
    c = Serial.read();
    if (commandLine.length() < 100) {
      commandLine += c;
    }
    else {
      commandLine = "";
      Serial.print("?");
    }

    // ==== If command string is complete... =======
    if (c == '\\') {

      cmd = commandLine.charAt(0);
      if (commandLine.charAt(2) == 'Y') color = 1; else color = 0;
      cmdPtr = 4;
      xStr = ""; yStr = "";
      xVal = 0; yVal = 0;
      while ((cmdPtr < commandLine.length()) && (commandLine.charAt(cmdPtr) != ',')) {
        xStr +=  (char)commandLine.charAt(cmdPtr);
        cmdPtr++;
        xVal = xStr.toInt();
      }

      cmdPtr++;
      while ((cmdPtr < commandLine.length()) && (commandLine.charAt(cmdPtr) != ',')) {
        yStr += (char)commandLine.charAt(cmdPtr);
        cmdPtr++;
        yVal = yStr.toInt();
      }

      if (cmd == 'B')  // Bitmap
      {
        xSizStr = ""; ySizStr = "";
        xSiz = 0; ySiz = 0;
        cmdPtr++;
        while ((cmdPtr < commandLine.length()) && (commandLine.charAt(cmdPtr) != ',')) {
          xSizStr +=  (char)commandLine.charAt(cmdPtr);
          cmdPtr++;
          xSiz = xSizStr.toInt();
        }

        cmdPtr++;
        while ((cmdPtr < commandLine.length()) && (commandLine.charAt(cmdPtr) != ',')) {
          ySizStr += (char)commandLine.charAt(cmdPtr);
          cmdPtr++;
          ySiz = ySizStr.toInt();
        }

      }
      else
      {
        cmdPtr++;
        fontSize = commandLine.charAt(cmdPtr);
        if (fontSize == 'S') fsize = SMALL;
        else if (fontSize == 'M') fsize = MEDIUM;
        else if (fontSize == 'L') fsize = LARGE;
        cmdPtr++;
      }

      cmdPtr++;
      outputString = "";
      while ((cmdPtr < commandLine.length() - 1) && (outputString.length() < 100)) {
        outputString += (char)commandLine.charAt(cmdPtr);
        cmdPtr++;
      }

      commandLine = "";    // Reset command mode

      // ======= Debug only ===========
      Serial.println((char)cmd);
      Serial.print("Color: ");
      Serial.println(color);
      Serial.print("xVal: ");
      Serial.println(xVal);
      Serial.print("yVal: ");
      Serial.println(yVal);
      if (cmd == 'B')
      {
        Serial.print("xSiz: ");
        Serial.println(xSiz);
        Serial.print("ySiz: ");
        Serial.println(ySiz);
      }
      else
      {
        Serial.print("font: ");
        Serial.println(fontSize);
      }
      Serial.println(outputString);

      // ======= Execute the respective command ========
      switch (cmd) {
        case 'C':  clearFrameBuffer(color); Serial.println("C"); break;
        case 'T':  printTest(yVal); Serial.println("T"); break;
        case 'S':  setPixel(xVal, yVal, color); break;
        case 'H':  hLine(yVal, color); Serial.println("H"); break;
        case 'V':  vLine(xVal, color); Serial.println("V"); break;
        case 'P':  printString(xVal, yVal, color, fsize, outputString); Serial.println("P");  break;
        case 'B':  printBitmap(xVal, yVal, color, xSiz, ySiz, outputString); Serial.println("B"); break;
        case 'U':  updatePanel(); Serial.println("U"); break;
      }
    }
    LedMatrix_update();
  }
  LedMatrix_update();
}


//===================================
// For debugging and testing only
//===================================
void printTest(int m) {
  int x,y;
  
  clearFrameBuffer(OFF);
  if (m == 0)
  {
    i = printString(1, 1, ON, XSMALL, "ABCDEFGHIJKLM");
  }
  else if (m == 1)
  {
    i = printBitmap( 2, 1, ON, 4, 4, "09000906");
    i = printBitmap(12, 1, ON, 4, 4, "09000609");
    i = printBitmap(22, 1, ON, 4, 4, "09000f00");
    i = printBitmap(32, 1, ON, 5, 5, "0A1F1F0E04");
  }
  else if (m == 2)
  {
    vLine(0,ON);
    vLine(49,ON);
    hLine(0,ON);
    hLine(6,ON);
  }
  else if (m == 3)
  {
    for(x=0; x<MTX_COLS; x+=2) 
      vLine(x,ON);
    for(y=0; y<MTX_ROWS; y+=2) 
      hLine(y,ON);
  }
}
