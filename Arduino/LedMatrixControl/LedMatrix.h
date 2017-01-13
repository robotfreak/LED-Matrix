#ifndef __LEDMATRIX_H__
#define __LEDMATRIX_H__

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif 

#define PANEL_SIZE 50
#define PANEL_NUM  1
#define MTX_COLS   (PANEL_SIZE*PANEL_NUM)
#define MTX_ROWS   7

#define ON  1
#define OFF 0

#define YELLOW 1
#define BLACK 0

#define XSMALL 0
#define SMALL 1
#define MEDIUM 2
#define LARGE 3
#define XLARGE 4

#endif /* __LEDMATRIX_H__ */

