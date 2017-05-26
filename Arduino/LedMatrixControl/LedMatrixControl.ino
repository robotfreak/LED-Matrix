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
//   "\n":
//     The command lines is terminated by the '\n' new line character
//     It gets evaluated after reception of that character
//
////////////////////////////////////////////////////////////////////////////

#include "LedMatrix.h"

int i, j;
int inByte;
String commandLine;
String outputString;

#define TXT_BUF_SIZ 150
#define SCROLL_TIME 10

char textBuf[TXT_BUF_SIZ];
int textBufLen;
bool scroll = false;
int scrollDelay = SCROLL_TIME;
int mode = 0;
int fdMode = 0;
int fdState = 0;
unsigned long current_time;
unsigned long last_time;

void setup() {

  Serial.begin(115200);
  Serial.println("LED Matrix Control v0.4");
  //showHelp();
  LedMatrix_begin();
  LedMatrix_update();
  fdMode = 1;

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

  if (Serial.available() > 0) {
    c = Serial.read();
    if (commandLine.length() < 160) {
      if (c != '\r')
        commandLine += c;
    }
    else {
      commandLine = "";
      Serial.print("?");
    }

    // ==== If command string is complete... =======
    if (c == '\n' && commandLine.length() > 1) {

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
      if (outputString != "")
      {
        Serial.println(outputString);
        scroll = false;
      }
      // ======= Execute the respective command ========
      switch (cmd) {
        case 'C':  clearFrameBuffer(color); Serial.println("C"); break;
        case 'T':  printTest(yVal); Serial.println("T"); break;
        case 'S':  setPixel(xVal, yVal, color); break;
        case 'H':  hLine(yVal, color); Serial.println("H"); break;
        case 'V':  vLine(xVal, color); Serial.println("V"); break;
        case 'P':  if (outputString.length() > 25) scrollText(outputString);
          else printString(xVal, yVal, color, fsize, outputString);
          Serial.println("P");  break;
        case 'B':  printBitmap(xVal, yVal, color, xSiz, ySiz, outputString); Serial.println("B"); break;
        case 'U':  updatePanel(); Serial.println("U"); break;
      }
      printFrameBuffer();
    }
    LedMatrix_update();
  }

  if (scroll == true)
  {
    if (scrollDelay) scrollDelay--;
    else
    {
      //      for (i = 0; i < textBufLen * 5; i++)
      {
        scrollDelay = SCROLL_TIME;
        Serial.print("scroll ");
        Serial.println(textBufLen, DEC);
        for (j = 0; j < textBufLen; j++)
        {
          //if (scrollDelay) scrollDelay--;
          //else
          {
            for (i = 0; i < 6; i++)
            {
              scrollDelay = SCROLL_TIME;
              if (i == 0) {

                if ((j + 25) < textBufLen )
                  c = textBuf[j + 25];
                else
                  c = ' ';

                printChar(MTX_COLS, 0, 1, LARGE, c);
                printChar(MTX_COLS + 6, 0, 1, LARGE, ' ');
                printFrameBuffer();
                Serial.println("");
              }
              shiftFrameBuffer();
            }
            LedMatrix_update();
          }
          LedMatrix_update();
        }
      }
      scroll = false;
      Serial.println("no scroll");
    }
  }
  else
  {
    switch (fdMode)
    {
      case 1:
        //printNews();
        break;
      default:
        break;
    }
  }
  LedMatrix_update();
}

void scrollText(String str)
{
  textBufLen = str.length();
  if (textBufLen < TXT_BUF_SIZ)
  {
    strcpy(textBuf, str.c_str());
    i = printString(0, 0, ON, LARGE, str );
    printFrameBuffer();
    scrollDelay = 100;
    scroll = true;
  }
}

void printNews() {
  static unsigned long previousMillis = 0;        // will store last time from update

  unsigned long currentMillis = millis();
  String str;

  if (currentMillis - previousMillis >= 30000) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    clearFrameBuffer(OFF);
    switch (fdState)
    {
      case 0:
        Serial.println(" Maker Faire Berlin 2017");
        i = printString(0, 0, ON, XLARGE, " Maker Faire Berlin 2017");
        printFrameBuffer();
        fdState++;
        break;
      case 1:
        Serial.println(" eLAB Makerspace Berlin");
        i = printString(0, 0, ON, XLARGE, " eLAB Makerspace Berlin");
        printFrameBuffer();
        fdState++;
        break;
      case 2:
        Serial.println("http://elab.in-berlin.de");
        i = printString(0, 0, ON, XLARGE, "http://elab.in-berlin.de");
        printFrameBuffer();
        fdState = 0;
        break;
      default:
        fdState = 0;
        break;

    }
    LedMatrix_update();
  }

}

#if 0
void showHelp(void)
{
  Serial.println("LED Matrix Control v0.3");
  Serial.println("-----------------------");
  Serial.println("Command format");
  Serial.println("<Command>,<Color>,<x>,<y>,<size>,[<xsize>,<ysize>]<....string....>\\\\n");
  //
  Serial.println("Commands:");
  Serial.println("  C  Clear Screen");
  Serial.println("  B  Draw a Bitmap");
  Serial.println("  P  Print Text");
  Serial.println("  H  Draw a horizontal line");
  Serial.println("  V  Draw a vertical line");
  Serial.println("  S  Set a pixel");
  Serial.println("  U  Update Panel");
  Serial.println("Color:");
  Serial.println("  B  Black");
  Serial.println("  Y  Yellow");
  Serial.println("X,Y:");
  Serial.println("  Required for all Print commands");
  Serial.println("  Only Y is required for the horizontal line command \"H\"");
  Serial.println("  Only X is required for the vertical line command \"V\"");
  Serial.println("size:");
  Serial.println("  Not required for Bitmap commands");
  Serial.println("  S SMALL");
  Serial.println("  M MEDIUM");
  Serial.println("  L LARGE");
  Serial.println("  X EXTRALARGE");
  Serial.println("XSize,YSize:");
  Serial.println("  Required only for Bitmap commands");
  Serial.println("String:");
  Serial.println("  Contains the characters to be printed");
  Serial.println("\\n:");
  Serial.println("  The command lines is terminated by the \\ character");
  Serial.println("  It gets evaluated after reception of that character");
}
#endif


//===================================
// For debugging and testing only
//===================================
void printTest(int m) {
  int x, y;

  clearFrameBuffer(OFF);
  if (m == 0)
  {
    i = printString(0, 0, ON, LARGE, "ABCDEFGHIJKLM");
  }
  else if (m == 1)
  {
    i = printBitmap( 1, 1, ON, 4, 4, "09000906");
    i = printBitmap( 6, 1, ON, 4, 4, "09000609");
    i = printBitmap(12, 1, ON, 4, 4, "09000f00");
    i = printBitmap(18, 1, ON, 5, 5, "0A1F1F0E04");
    i = printBitmap(25, 0, ON, 7, 7, "AA55AA55AA55AA");
    i = printBitmap(33, 0, ON, 7, 7, "AAFFAAFFAAFFAA");
    i = printBitmap(41, 0, ON, 8, 7, "82442810284482");
  }
  else if (m == 2)
  {
    vLine(0, ON);
    vLine(49, ON);
    hLine(0, ON);
    hLine(6, ON);
  }
  else if (m == 3)
  {
    for (x = 0; x < MTX_COLS; x += 2)
      vLine(x, ON);
    for (y = 0; y < MTX_ROWS; y += 2)
      hLine(y, ON);
  }
  if (m == 4)
  {
    strcpy(textBuf, "@roboterfreak");
    i = printString(0, 0, ON, LARGE, textBuf );
    textBufLen = 24;
    scrollDelay = 100;
    scroll = true;
  }
}
