/*
 * Display.h
 *
 *  Created on: Jun 12, 2014
 *      Author: jeffw
 */

#ifndef DISPLAYLCD_H_
#define DISPLAYLCD_H_

#include "PE_Types.h"
#include "Display.h"
#include "SharpDisplayCS.h"
#include "Wait.h"

#define DISPLAY_WIDTH		128
#define DISPLAY_HEIGHT		128

#define WHITE				1
#define	BLACK				0

#define LCDCMD_DUMMY		0x00
#define LCDCMD_DISPLAY		0x00
#define LCDCMD_WRITE		0x80
#define LCDCMD_VCOM			0x40
#define LCDCMD_CLEAR		0x20

#define DISPLAY_CS_ON     	SharpDisplayCS_SetVal();
#define DISPLAY_CS_OFF    	while (!SPI_TX_READY); /* Wait_Waitus(1); */ SharpDisplayCS_ClrVal();

#define readFontByte(addr) (*(const unsigned char *)(addr))
#define swap(a, b) { uint16_t t = a; a = b; b = t; }
#define getMax(a,b)    (((a) > (b)) ? (a) : (b))
#define getMin(a,b)    (((a) < (b)) ? (a) : (b))

#define SPI_TX_READY					SPIS_SPTEF
#define SPI_WriteData8Bits(data)		SPID = data
#define SPI_SetDataShiftOrder(lsb)		SPIC1_LSBFE = lsb

#define SPI_PDD_LSB_FIRST	1
#define SPI_PDD_MSB_FIRST	0

#define ROW_BUFFER_BYTES	DISPLAY_WIDTH / 8

void sendByte(uint8_t data);
void sendByteLSB(uint8_t data);

void clearLcdDisplay(void);
void drawPixelInRowBuffer(int16_t x, byte *rowBufferPtr);
void putCharSliceInRowBuffer(uint16_t x, unsigned char c, uint8_t *rowBufferPtr, uint8_t slice, uint8_t size);
void putBarcodeInRowBuffer(uint16_t x, unsigned char c, uint8_t *rowBufferPtr, uint8_t slice, uint8_t size);
void sendRowBuffer(uint16_t row, byte *rowBufferPtr);
void displayString(uint16_t x, uint16_t y, char_t *stringPtr, uint8_t size);
void displayBarcode(uint16_t x, uint16_t y, char_t *stringPtr, uint8_t size);
void displayMessage(uint8_t line, char_t *stringPtr, uint8_t maxChars, uint8_t size);

#endif /* DISPLAY_H_ */
