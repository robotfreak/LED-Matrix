
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <SPI.h>

#define sckPin  13  // hardware SPI Pin for UNO (can't be changed)
#define mosiPin 11  // hardware SPI Pin for UNO (can't be changed) 
const byte oePin     = 9; // output enable pin
const byte strobePin = 10; // strobe Pin

#define PANEL_SIZE 50
#define PANEL_NUM  1
#define MTX_COLS   (PANEL_SIZE*PANEL_NUM)
#define MTX_ROWS   7

#define ON  1
#define OFF 0

const byte rowPins[MTX_ROWS]  = { 8, 7, 6, 5, 4, 3, 2 };


byte ledMtx[MTX_COLS+14*PANEL_NUM];

void LedMatrix_setRow(int row)
{
  int i;

  for (i = 0; i < MTX_ROWS; i++)
  {
    if (i != row)
    {
      digitalWrite(rowPins[i], LOW);
    }
  }

  for (i = 0; i < MTX_ROWS; i++)
  {
    if (i == row)
    {
      digitalWrite(rowPins[i], HIGH);
    }
  }
}

void LedMatrix_setBuffer(int color)
{
  int x, y;
  for (x = 0; x < MTX_COLS; x++)
  {
    for (y = 0; y < MTX_ROWS; y++)
    {
      LedMatrix_setPixel(x, y, color);
    }
  }
  //  memset(ledMtx, color, sizeof(ledMtx));
}

void LedMatrix_setPattern(int pattern1, int pattern2)
{
  int x, y, w, color, pattern;

  for (x = 0; x < MTX_COLS; x++)
  {
    if (x % 2 == 0)
    {
      pattern = pattern1;
    }
    else
    {
      pattern = pattern2;
    }
    w = 1;
    for (y = 0; y < MTX_ROWS; y++)
    {
      if (pattern & w) color = ON; else color = OFF;
      LedMatrix_setPixel(x, y, color);
      w = w << 1;
    }
  }
  //  memset(ledMtx, color, sizeof(ledMtx));
}

void LedMatrix_setPixel(int x, int y, int color)
{
  if (y < MTX_ROWS && x < MTX_COLS && y >= 0 && x >= 0)
  {
    if (color)
      ledMtx[x] |=  (1 << y);
    else
      ledMtx[x] &= ~(1 << y);
  }

}

int LedMatrix_getPixel(int x, int y)
{
  int val = 0;
  if (y < MTX_ROWS && x < MTX_COLS && y >= 0 && x >= 0)
  {
    if (ledMtx[x] & (1 << y)) val = 1;
  }
  return val;
}


void ledMatrix_printLedMtx() {
  int x, y;
  unsigned char w;

  w = 1;   // most right bit set
  for (y = 0; y < MTX_ROWS; y++) {
    for (x = 0; x < MTX_COLS; x++) {
      if (ledMtx[x] & w) Serial.print("#"); else Serial.print(".");
    }
    w = w << 1;
    Serial.println("");
  }
}

void LedMatrix_begin(void)
{
  int i;

  SPI.begin();
  pinMode(strobePin, OUTPUT);
  pinMode(oePin, OUTPUT);
  digitalWrite(strobePin, LOW);
  for (i = 0; i < MTX_ROWS; i++)
  {
    pinMode(rowPins[i], OUTPUT);
  }

  LedMatrix_setRow(-1);
  LedMatrix_setBuffer(OFF);
}


void LedMatrix_shiftOut(int row)
{
  int i, j, p, x, y, val;
  SPI.beginTransaction(SPISettings(400000, LSBFIRST, SPI_MODE0));
  y = 1 << row;
  for (p = 0; p < PANEL_NUM; p++)
  {
    val = 0;
    for (i = 0; i < 8; i++)
    {
      val = 0;
      for (j = 0; j < 8; j++)
      {
        x = MTX_COLS+13 - (p * PANEL_SIZE + i * 8 + j);
        if (ledMtx[x] & y)
        {
          val |= (1 << j);
        }
      }
      SPI.transfer (val);
    }
  }

  SPI.endTransaction();
  delayMicroseconds(40);
  digitalWrite(oePin, LOW);
  delayMicroseconds(4);
  digitalWrite(strobePin, HIGH);
  delayMicroseconds(1);
  digitalWrite(strobePin, LOW);
  delayMicroseconds(5);
  LedMatrix_setRow(row);
  digitalWrite(oePin, HIGH);
}

void LedMatrix_update(void)
{
  int row;
  for (row = 0; row < MTX_ROWS; row++)
  {
    LedMatrix_shiftOut(row);
    delay(1);
  }
  // ledMatrix_printLedMtx();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  LedMatrix_begin();
  LedMatrix_update();

}

int i = 0;
int x = 0;
int y = 0;
int val = ON;
char mode = '0';
char oldMode = 0;

void initTest()
{
  i = 0;
  x = 0;
  y = 0;
  LedMatrix_setBuffer(OFF);
}

void test0(void) {
  if (i % 2 == 0)
  {
    LedMatrix_setPixel(x, y, val);
    y++;
    if (y >= 7) {
      y = 0;
      x++;
      if (x > 50)
      {
        x = 0;
        if (val == ON)
          val = OFF;
        else
          val = ON;
      }
    }
  }
}

void test1(void) {
  if (i % 2 == 0)
  {
    LedMatrix_setPixel(x, y, val);
    x++;
    if (x > 50)
    {
      x = 0;
      y++;
      if (y >= 7) {
        y = 0;
        if (val == ON)
          val = OFF;
        else
          val = ON;
      }
    }
  }
}

void test2(void) {
  LedMatrix_setBuffer(ON);
}

void test3(void) {
  LedMatrix_setPattern(0x55, 0xAA);
}

void test4(void) {
  LedMatrix_setPattern(0xFF, 0x00);
}

void loop() {

  if (Serial.available())
  {
    mode = Serial.read();
    if (mode != 'p' && mode != oldMode)
    {
      initTest();
      oldMode = mode;
    }
  }

  switch (mode)
  {
    case '0':
      test0();
      break;
    case '1':
      test1();
      break;
    case '2':
      test2();
      break;
    case '3':
      test3();
      break;
    case '4':
      test4();
      break;
    case 'p':
      ledMatrix_printLedMtx();
      mode = 0;
      break;
  }

  LedMatrix_update();
  i++;
  //if (i>400) i = 0;

}
