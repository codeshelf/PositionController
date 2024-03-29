#include "Display.h"
#include "DisplayLcd.h"
#include "PE_Types.h"
#include "IO_Map.h"
#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "SharpDisplayCS.h"
#include "FontBarcode.h"
#include "FontArial.h"
#include "StatusLedClk.h"
#include "StatusLedData.h"

#include <stdio.h>

char* itoa(int i, char b[]) {
	int shifter;
	char const digit[] = "0123456789";
	char* p = b;
	if (i < 0) {
		*p++ = '-';
		i *= -1;
	}
	shifter = i;
	do { //Move to where representation ends
		++p;
		shifter = shifter / 10;
	} while (shifter);
	*p = '\0';
	do { //Move back, inserting digits as u go
		*--p = digit[i % 10];
		i = i / 10;
	} while (i);
	return b;
}

void sendByte(uint8_t data) {

	// Wait until TX buffer empty.
	while (!SPI_TX_READY)
		;
	SPI_WriteData8Bits(data);
	Wait_Waitns(20000);
}

void sendByteLSB(uint8_t data) {

	SPI_SetDataShiftOrder(SPI_PDD_LSB_FIRST);

	while (!SPI_TX_READY)
		;
	SPI_WriteData8Bits(data);
	Wait_Waitns(20000);

	SPI_SetDataShiftOrder(SPI_PDD_MSB_FIRST);
}

void sendRowBuffer(uint16_t row, byte* rowBuffer) {
	uint16_t pos;
	char c;

	DISPLAY_CS_ON

	// Send the write command
	sendByte(LCDCMD_WRITE + LCDCMD_VCOM);

	// Send image buffer
	sendByteLSB(row);
	for (pos = 0; pos < ROW_BUFFER_BYTES; pos++) {
		c = rowBuffer[pos];
		sendByteLSB(~c);
	}
	sendByteLSB(LCDCMD_DUMMY);

	// Send another trailing 8 bits for the last line
	sendByteLSB(LCDCMD_DUMMY);

	DISPLAY_CS_OFF

	DISPLAY_CS_ON

	sendByte(LCDCMD_DISPLAY);
	sendByteLSB(LCDCMD_DUMMY);

	DISPLAY_CS_OFF
}

void initDisplay(void) {
	clearLcdDisplay();
}

void clearDisplay(void) {
	clearLcdDisplay();
}

void displayValue(uint8_t value) {
	char buffer[4];
	itoa(value, buffer);
	displayString(10, 25, buffer, 2);
}

void displayValueBlink(uint8_t value) {
	displayValue(value);
}

void clearLcdDisplay() {
	// Send the clear screen command rather than doing a HW refresh (quicker)
	DISPLAY_CS_ON
	sendByte(LCDCMD_CLEAR);
	DISPLAY_CS_OFF
}

void drawPixelInRowBuffer(int16_t x, byte *rowBufferPtr) {
	uint16_t loc = x / 8;
	uint8_t value = (1 << x % 8);
	if ((loc >= 0) && (loc < ROW_BUFFER_BYTES)) {
		rowBufferPtr[loc] |= value;
	}
}

void putCharSliceInRowBuffer(uint16_t x, unsigned char drawChar, uint8_t *rowBufferPtr, uint8_t slice, uint8_t size) {
	uint8_t hzPixelNum;
	uint8_t pixelsByte;
	uint8_t extra;
	uint8_t sliceByteOffset;
	uint8_t charDescOffset;
	uint16_t pixDataOffset;

	charDescOffset = (uint8_t) (drawChar - arial_16ptFontInfo.startChar);
	pixDataOffset = arial_16ptDescriptors[charDescOffset].offset;

	sliceByteOffset = 0;
	if (slice > 0) {
		sliceByteOffset = slice * (arial_16ptDescriptors[charDescOffset].widthBits / 8);
		if (arial_16ptDescriptors[charDescOffset].widthBits % 8) {
			sliceByteOffset += slice;
		}
	}

	for (hzPixelNum = 0; hzPixelNum < arial_16ptDescriptors[charDescOffset].widthBits; hzPixelNum++) {
		pixelsByte = arial_16ptBitmaps[pixDataOffset + sliceByteOffset + hzPixelNum / 8];
		if (pixelsByte & (0x80 >> (hzPixelNum % 8))) {
			for (extra = 0; extra < size; extra++) {
				drawPixelInRowBuffer(x + hzPixelNum + extra, rowBufferPtr);
			}
		}
	}
}

void displayString(uint16_t x, uint16_t y, char_t *stringPtr, uint8_t size) {
	byte rowBuffer[ROW_BUFFER_BYTES];
	uint8_t slice;
	uint8_t charPos;
	uint8_t extra;
	uint8_t charDescOffset;
	uint16_t totalBits;

	for (slice = 0; slice < arial_16ptFontInfo.charInfo->heightBits; slice++) {
		memset(&rowBuffer, 0x00, ROW_BUFFER_BYTES);
		totalBits = 0;
		for (charPos = 0; charPos < strlen(stringPtr); charPos++) {
			if (stringPtr[charPos] == ' ') {
				totalBits += arial_16ptFontInfo.spacePixels + size;
			} else {
				putCharSliceInRowBuffer(x + totalBits, stringPtr[charPos], rowBuffer, slice, size);
				charDescOffset = stringPtr[charPos] - arial_16ptFontInfo.startChar;
				totalBits += arial_16ptDescriptors[charDescOffset].widthBits + size;
			}
		}
		for (extra = 0; extra < size; extra++) {
			sendRowBuffer(y + (slice * size) + extra, rowBuffer);
		}
	}
}


void displayMessage(uint8_t line, char_t *stringPtr, uint8_t maxChars, uint8_t size) {
	byte rowBuffer[ROW_BUFFER_BYTES];
	uint8_t slice;
	uint8_t charPos;
	uint8_t extra;

	uint8_t x = 5;
	uint8_t y = 10 + (7 * size * (line - 1));
	uint8_t max = getMin(maxChars, strlen(stringPtr));
	for (slice = 0; slice < 8; slice++) {
		// Clear the row buffer.
		memset(&rowBuffer, 0x00, ROW_BUFFER_BYTES);

		for (charPos = 0; charPos < max; charPos++) {
			putCharSliceInRowBuffer(x + (charPos * size * 6), stringPtr[charPos], rowBuffer, slice, size);
		}

		for (extra = 0; extra < size; extra++) {
			sendRowBuffer(y + (slice * size) + extra, rowBuffer);
		}
	}
}

void putBarcodeSliceInRowBuffer(uint16_t x, unsigned char drawChar, uint8_t *rowBufferPtr, uint8_t slice, uint8_t size) {
	uint8_t hzPixelNum;
	uint8_t pixelsByte;
	uint8_t extra;
	uint8_t sliceByteOffset;
	uint8_t charDescOffset;
	uint16_t pixDataOffset;

	charDescOffset = (uint8_t) (drawChar - barcodeFontInfo.startChar);
	pixDataOffset = barcodeDescriptors[charDescOffset].offset;

	sliceByteOffset = 0;
	if (slice > 0) {
		sliceByteOffset = slice * (barcodeDescriptors[charDescOffset].widthBits / 8);
		if (barcodeDescriptors[charDescOffset].widthBits % 8) {
			sliceByteOffset += slice;
		}
	}

	for (hzPixelNum = 0; hzPixelNum < barcodeDescriptors[charDescOffset].widthBits; hzPixelNum++) {
		pixelsByte = barcodeBitmaps[pixDataOffset + sliceByteOffset + hzPixelNum / 8];
		if (pixelsByte & (0x80 >> (hzPixelNum % 8))) {
			for (extra = 0; extra < size; extra++) {
				drawPixelInRowBuffer(x + hzPixelNum + extra, rowBufferPtr);
			}
		}
	}
}

void displayBarcode(uint16_t x, uint16_t y, char_t *stringPtr, uint8_t size) {
	byte rowBuffer[ROW_BUFFER_BYTES];
	uint8_t slice;
	uint8_t charPos;
	uint8_t extra;
	uint8_t charDescOffset;
	uint16_t totalBits;

	for (slice = 0; slice < barcodeFontInfo.charInfo->heightBits; slice++) {
		memset(&rowBuffer, 0x00, ROW_BUFFER_BYTES);
		totalBits = 0;
		for (charPos = 0; charPos < strlen(stringPtr); charPos++) {
			if (stringPtr[charPos] == ' ') {
				totalBits += barcodeFontInfo.spacePixels + size;
			} else {
				putBarcodeSliceInRowBuffer(x + totalBits, stringPtr[charPos], rowBuffer, slice, size);
				charDescOffset = stringPtr[charPos] - barcodeFontInfo.startChar;
				totalBits += barcodeDescriptors[charDescOffset].widthBits + size;
			}
		}
		for (extra = 0; extra < size; extra++) {
			sendRowBuffer(y + (slice * size) + extra, rowBuffer);
		}
	}
}

void displayFirmwareVersion() {
	
}
