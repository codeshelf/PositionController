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
#define QUERY_COMMAND			0x04
#define RESPONSE_COMMAND		0x05
#define IDSETUPSTART_COMMAND	0x06
#define IDSETUPINC_COMMAND		0x07
#define BUTTONCREATE_COMMAND		0x08

#define COMMANDID_POS			0x00
#define COMMAND_BUSADDR_POS		COMMANDID_POS + 1

#define COMMAND_CREATE_NUM		COMMAND_BUSADDR_POS + 1

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

typedef enum {
	eInactive, eActive, eConfigMode, eFirmwareDisplayMode
} EDeviceState;

void processFrame(FramePtrType, FrameCntType);
void setValues(FramePtrType, FrameCntType);

void startConfigMode(void);
void incrementConfigMode(void);

void sendResponseCommand(void);
void sendIdSetupIncCommand(void);
void sendAckCommand(void);

void createButtonPress(FramePtrType framePtr, FrameCntType frameByteCount);

#endif /* COMMANDS_H_ */
