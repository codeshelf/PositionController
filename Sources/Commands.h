/*
 * Codeshelf
 * © Copyright 2005-2013 Jeffrey B. Williams
 * All rights reserved
 * 
 * $Id$
 * $Name$
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#include "SerialBus.h"
#include "I2CMaster.h"

#define INIT_COMMAND			0x00
#define CLEAR_COMMAND			0x01
#define DISPLAY_COMMAND			0x02
#define BUTTON_COMMAND			0x03

#define COMMANDID_POS			0x00
#define COMMAND_BUSADDR_POS		COMMANDID_POS + 1

#define BROADCAST_BUSADDR		0x00
#define UNSET_BUSADDR			0xff

#define DISPLAY_CMD_VAL_POS		COMMAND_BUSADDR_POS + 1
#define DISPLAY_CMD_MIN_POS		DISPLAY_CMD_VAL_POS + 1
#define DISPLAY_CMD_MAX_POS		DISPLAY_CMD_MIN_POS + 1
#define DISPLAY_CMD_FREQ_POS	DISPLAY_CMD_MAX_POS + 1
#define DISPLAY_CMD_DUTY_POS	DISPLAY_CMD_FREQ_POS + 1

#define BUTTON_CMD_DATA_POS		COMMAND_BUSADDR_POS + 1

#define DOWN_BUTTON				KBI_NUM_PIN0
#define UP_BUTTON				KBI_NUM_PIN1
#define ACK_BUTTON				KBI_NUM_PIN2

#define I2C_DELAY_40MS 			400

#define BLINK_FREQ 				0x15
#define BLINK_DUTYCYCLE 		0x40

typedef enum {
	eInactive, eActive, eConfigMode
} EDeviceState;

void processFrame(FramePtrType, FrameCntType);
void initDisplay(void);
void clearDisplay(void);
void setValues(FramePtrType, FrameCntType);
void displayValue(uint8_t currentValue);
void displayValueBlink(uint8_t currentValue);
void displayValueAsCode(uint8_t controlValue);
void setLedIndicator(uint8_t redValue, uint8_t greenValue, uint8_t blueValue);
void setLedSegments(uint8_t* displayBytesPtr);

#endif /* COMMANDS_H_ */
