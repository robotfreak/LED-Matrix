/////////////////////////////////////////////////////////////////
//  Testenvironment and base for displaying Fonts on the
//  Flipdot display.
//  CC-BY SA NC 2016 c-hack.de    ralf@surasto.de
/////////////////////////////////////////////////////////////////

#include "LedMatrix.h"
#include "font3x5.h"
#include "font5x7.h"

//================== Constants ===============================
#define X_SIZE MTX_COLS    // 115 column
#define Y_SIZE MTX_ROWS     // 7 rows 
#define OFF 0
#define ON 1

//================ global Variables ==========================
// frameBuffer stores the content of the Flipdotmatrix
// Each pixel is one bit (I.e only an eigth of storage is required
unsigned char frameBuffer[X_SIZE];


//#################### Public Functions ####################################

//====================================================
// Clears the entire framebuffer and flips every pixel
// of the display
// color = BLACK   all pixels set to black
// color = YELLOW  all pixels set to yellow
//====================================================
void clearFrameBuffer(int color) {
  int i, j;

  Serial.println("C");
  for (i = 0; i < X_SIZE; i++) {
    for (j = 0; j < Y_SIZE; j++) {
      setFrameBuffer(i, j, color);
    }
  }
}

void updatePanel(void)
{
  printFrameBuffer();
  LedMatrix_printLedMtx();
}
//====================================================
// Sets a Pixel at row Y column X
// color = BLACK   all pixels set to black
// color = YELLOW  all pixels set to yellow
//====================================================
void setPixel(int x, int y, int color) {

  setFrameBuffer(x, y, color);
}

int hex2int(char *hex) {
  int val = 0;
  while (*hex) {
    // get current character then increment
    uint8_t byte = *hex++;
    // transform hex character to the 4bit equivalent number, using the ascii table indexes
    if (byte >= '0' && byte <= '9') byte = byte - '0';
    else if (byte >= 'a' && byte <= 'f') byte = byte - 'a' + 10;
    else if (byte >= 'A' && byte <= 'F') byte = byte - 'A' + 10;
    // shift 4 to make space for new digit, and add the 4 bits of the new digit
    val = (val << 4) | (byte & 0xF);
  }
  return val;
}

//============================================
// printBitmap(int xOffs, int yOffs, int color, int xSize, int ySize, char *s)
// xOffs = position of the left side of the bitmap
// yOffs = position of the top of the bitmap
// color = ON means yellow, OFF means black
// xSize = horizontal size of the bitmap
// ySize = vertical sizw of the bitmap
// s = string
//============================================
int printBitmap(int xOffs, int yOffs, int color, int xSize, int ySize, String s) {
  int i, x, y, xs, xt, xo, w;
  char stmp[3];

  i = 0;

  if (xSize > 0 && xSize <= X_SIZE && ySize > 0 && ySize <= Y_SIZE )
  {
    while ((s[i] != '\0') && (i < 200))
    {
      for (y = 0; y < Y_SIZE; y++)
      {
        xs = xSize;
        xo = 0;
        while (xs > 0)
        {
          stmp[0] = s.charAt(i);
          stmp[1] = s.charAt(i + 1);
          stmp[2] = 0;
          w = hex2int(stmp);
          //printf("w=%0x x=%d y=%d\n", w, xo, y+yo);
          i += 2;
          if (xs < 8) xt = xs; else xt = 8;
          for (x = 0; x < xt; x++)
          {
            if (w & 1) {
              setFrameBuffer(xt - 1 - x + xo + xOffs, y + yOffs, color);
            }
            else {
              setFrameBuffer(xt - 1 - x + xo + xOffs, y + yOffs, !color);
            }
            w = w >> 1;
          }
          xs -= xt;
          xo += xt;
        }
      }
    }
  }
  else printf("error size, x %d, y %d", xSize, ySize);
  return (x);
}


//====================================================
// Draws a horizotal line at row Y
// color = BLACK   all pixels set to black
// color = YELLOW  all pixels set to yellow
//===================================================
void hLine(int y, int color) {
  int i;

  for (i = 0; i < X_SIZE; i++) {
    setFrameBuffer(i, y, color);
  }
}

//====================================================
// Draws a vertical line at column X
// color = BLACK   all pixels set to black
// color = YELLOW  all pixels set to yellow
//====================================================
void vLine(int x, int color) {
  int i;

  for (i = 0; i < Y_SIZE; i++) {
    setFrameBuffer(x, i, color);
  }
}


//============================================
// printString(int xOffs, int yOffs, int color, int size char s)
// xOffs = position of the left side of the string
// yOffs = position of the top of the string
// color = ON means yellow, OFF means black
// size = SMALL | MEDIUM | LARGE
// s = string
//============================================
int printString(int xOffs, int yOffs, int color, int size, String s) {
  int i, x, y;
  char c;

  i = 0;
  x = xOffs;
  y = yOffs;

  if (size == SMALL) x = xOffs - 2; // Somehow I need to shift the text to the left

  while ((i < s.length()) && (i < 100)) {
    switch (size) {
      case XSMALL:
        x = printChar3x5(x, y, color, s.charAt(i));
        break;
      case SMALL:
        x = printChar3x5(x, y, color, s.charAt(i));
        break;
      case LARGE:
        x = printChar5x7(x, y, color, s.charAt(i));
        break;
      case MEDIUM:
        x = printChar5x7(x, y, color, s.charAt(i));
        break;
      default: x = printChar5x7(x, y, color, s[i]);
    }
    //Serial.print(s.charAt(i));
    i++;
  }
  return (x);
}

int printChar(int xOffs, int yOffs, int color, int size, char c) {
  int i, x, y;

  i = 0;
  x = xOffs;
  y = yOffs;

  if (size == SMALL) x = xOffs - 2; // Somehow I need to shift the text to the left

    switch (size) {
      case XSMALL:
        x = printChar3x5(x, y, color, c);
        break;
      case SMALL:
        x = printChar3x5(x, y, color, c);
        break;
      case LARGE:
        x = printChar5x7(x, y, color, c);
        break;
      case MEDIUM:
        x = printChar5x7(x, y, color, c);
        break;
      default: x = printChar5x7(x, y, color, c);
    }
  return (x);
}



//###################### Internal Functions ##########################################


//===========================================
// setFrameBuffer(int x, int y, int value)
// Set one Pixel at x,y-Position
// value can be ON or OFF
//===========================================
void setFrameBuffer(int x, int y, int value) {
  unsigned char w, wNot;
  int yBitNo;

  w = 1;
  if ((y < Y_SIZE) && (x < X_SIZE) && (x >= 0) && (y >= 0)) {
    yBitNo = y % 8;   // modulo division (residual) to select the bit in that byte
    w = w << yBitNo;
    if (value == ON) {
      frameBuffer[x] = frameBuffer[x] | w; // Logical OR adds one bit to the existing byte
    } else {
      wNot = 0xFF - w;
      frameBuffer[x] = frameBuffer[x] & wNot; // Logical AND set one bit to zero in the existing byte
    }
    LedMatrix_setPixel(x, y, value);
  }
}

//===========================================
// int getFrameBuffer(int x, int y)
// Gets color of one Pixel at x,y-Position
// returns value can be ON or OFF
//===========================================
int getFrameBuffer(int x, int y) {
  unsigned char w, wNot;
  int yBitNo;
  int value = 0;

  w = 1;
  if ((y < Y_SIZE) && (x < X_SIZE) && (x >= 0) && (y >= 0)) {
    yBitNo = y % 8;   // modulo division (residual) to select the bit in that byte
    w = w << yBitNo;
    if (frameBuffer[x] & w) value = 1; else value = 0;
  }
  return value;
}

void shiftFrameBuffer(void) {
  unsigned char x;
  
  for (x = 0; x < X_SIZE-1; x++) {
    frameBuffer[x] = frameBuffer[x+1];
  }
  LedMatrix_copy(frameBuffer);
}

//============================================
// printChar6x8(int xOffs, int yOffs, char c)
// printChar8x8(int xOffs, int yOffs, char c)
// printChar8x12(int xOffs, int yOffs, char c)
//
// xOffs = position of the left side of the character
// yOffs = position of the top of the character
// color = ON means yellow, OFF means black
// c = ASCII Character
// returns new x position
//============================================
int printChar3x5(int xOffs, int yOffs, int color, unsigned char c)
{
  unsigned char x, y, w, ctmp;
  ctmp = c - 32;
  for (y = 0; y < 6; y++)
  {
    w = pgm_read_byte(&(font3x5[ctmp][y]));
    for (x = 0; x < 8; x++)
    {
      if (w & 1)
        setFrameBuffer(6 - x + xOffs, y + yOffs, color);
      w = w >> 1;
    }
  }
  return (xOffs + 4);
}

int printChar5x7(int xOffs, int yOffs, int color, unsigned char c)
{
  unsigned char x, y, w, ctmp;
  ctmp = c - 32;
  for (x = 0; x < 5; x++)
  {
    w = pgm_read_byte(&(font5x7[ctmp][x]));
    for (y = 0; y < 8; y++)
    {
      if (w & 1)
        setFrameBuffer(x + xOffs, y + yOffs, color);
      else
        setFrameBuffer(x + xOffs, y + yOffs, !color);
      w = w >> 1;
    }
  }
  return (xOffs + 6);
}


//============================================
// DEBUG ONLY
// printFrameBuffer is only used to see the
// content on the screen for debug
//============================================
void printFrameBuffer() {
  int x, y;
  unsigned char w;

  w = 1;   // most right bit set
  for (y = 0; y < Y_SIZE; y++) {
    for (x = 0; x < X_SIZE; x++) {
      if (frameBuffer[x] & w) Serial.print("#"); else Serial.print(".");
    }
    w = w << 1;
    Serial.println("");
  }
}


