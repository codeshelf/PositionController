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
#define COMMAND_BUSID_POS		COMMANDID_POS + 1

#define DISPLAY_CMD_VAL_POS		COMMAND_BUSID_POS + 1
#define DISPLAY_CMD_MIN_POS		DISPLAY_CMD_VAL_POS + 1
#define DISPLAY_CMD_MAX_POS		DISPLAY_CMD_MIN_POS + 1

#define BUTTON_CMD_DATA_POS		COMMAND_BUSID_POS + 1

#define UP_BUTTON				KBI_NUM_PIN0
#define DOWN_BUTTON				KBI_NUM_PIN1
#define ACK_BUTTON				KBI_NUM_PIN2

void processFrame(FramePtrType, FrameCntType);
void initDisplay(void);
void clearDisplay(void);
void setValues(FramePtrType, FrameCntType);
void displayCurrentValue(void);

#endif /* COMMANDS_H_ */
