
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
#define PANEL_NUM  3
#define MTX_COLS   (PANEL_SIZE*PANEL_NUM)
#define MTX_ROWS   7

#define ON  1
#define OFF 0

const byte rowPins[MTX_ROWS]  = { 8, 7, 6, 5, 4, 3, 2 };


byte ledMtx[MTX_COLS];

void LedMatrix_setRow(int row)
{
  int i;

  for (i = 0; i < MTX_ROWS; i++)
  {
    if (i != row)
    {
      digitalWrite(rowPins[i], HIGH);
    }
  }

  for (i = 0; i < MTX_ROWS; i++)
  {
    if (i == row)
    {
      digitalWrite(rowPins[i], LOW);
    }
  }
}

void LedMatrix_setBuffer(int color)
{
  int x,y;
  for(x=0; x<MTX_COLS; x++)
  {
    for(y=0; y<MTX_ROWS; y++)
    {
      LedMatrix_setPixel(x,y,color);
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



void LedMatrix_begin(void)
{
  int i;

  SPI.begin();
  pinMode(strobePin, OUTPUT);
  pinMode(oePin, OUTPUT);
  digitalWrite(strobePin, LOW);
  digitalWrite(MTX_ROWS, HIGH);
  LedMatrix_setRow(-1);
  LedMatrix_setBuffer(OFF);
}


void LedMatrix_shiftOut(int row)
{
  int i, p, x, val;
  SPI.beginTransaction(SPISettings(400000, MSBFIRST, SPI_MODE1));
  for (p = 0; p < PANEL_NUM; p++)
  {
    val = 0;
    for (i = 0; i < 8; i++)
    {
      val = 0;
      for (x = 0; x < 8; x++)
      {
        if (LedMatrix_getPixel(p * PANEL_SIZE + i * 8 + x, row))
        {
          bitSet(val, x);
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
}

void setup() {
  // put your setup code here, to run once:
  LedMatrix_begin();
  LedMatrix_update();

}

int i = 0;

void loop() {
  if (i== 100 || i == 300)
    LedMatrix_setBuffer(ON);
  else if (i == 200 ||i == 400)  
    LedMatrix_setBuffer(OFF);
   
  LedMatrix_update();
  i++;
  if (i>400) i = 0;

}
