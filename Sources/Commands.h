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
#define BUTTONCREATE_COMMAND	0x08
#define DISPLAY_ADDR_COMMAND	0x09
#define SET_LED					0x0b
#define DISPLAY_FWVER_COMMAND	0x0c

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

#define DISPLAY_CMD_RED_POS		COMMAND_BUSADDR_POS + 1
#define DISPLAY_CMD_GREEN_POS	DISPLAY_CMD_RED_POS + 1
#define DISPLAY_CMD_BLUE_POS	DISPLAY_CMD_GREEN_POS + 1
#define DISPLAY_CMD_SYTLE_POS	DISPLAY_CMD_BLUE_POS + 1

#define IDSETUPINC_COMMAND_NUM_POS	COMMAND_BUSADDR_POS + 1

#define BUTTON_CMD_DATA_POS		COMMAND_BUSADDR_POS + 1

#define DOWN_BUTTON				KBI_NUM_PIN0
#define UP_BUTTON				KBI_NUM_PIN1
#define ACK_BUTTON				KBI_NUM_PIN2

typedef enum {
	eInactive, eActive, eConfigMode, eFirmwareDisplayMode, eAddressDisplayMode, eMassConfigMode
} EDeviceState;

void processFrame(FramePtrType, FrameCntType);
void setValues(FramePtrType, FrameCntType);
void setLedValues(FramePtrType, FrameCntType);

void startMassConfigMode(void);
void incrementMassConfigMode(FramePtrType framePtr, FrameCntType frameByteCount);

void sendResponseCommand(void);
void sendIdSetupIncCommand(uint8_t n);
void sendAckCommand(void);

void displayBusAddress(void);

void createButtonPress(FramePtrType framePtr, FrameCntType frameByteCount);

#endif /* COMMANDS_H_ */
