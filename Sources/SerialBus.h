/*
 * Codeshelf
 * © Copyright 2005-2013 Jeffrey B. Williams
 * All rights reserved
 * 
 * $Id$
 * $Name$
 */

#ifndef SERIALBUS_H_
#define SERIALBUS_H_

#include "IO_Map.h"
#include "PE_Types.h"
#include "Rs485Dir.h"
//#include "Watchdog.h"

#define MAX_FRAME_BYTES		16
#define START_FRAME			0xff
#define END_FRAME			0xfe

typedef uint8_t	FrameDataType;
typedef FrameDataType *FramePtrType;
typedef uint8_t FrameCntType;

// --------------------------------------------------------------------------
// Defines.

#define RS485_TX				Rs485Dir_PutVal(1)
#define RS485_RX				Rs485Dir_PutVal(0);

#define GATEWAY_MGMT_QUEUE_SIZE		10

#define END							0300    /* indicates end of frame */
#define ESC							0333    /* indicates byte stuffing */
#define ESC_END						0334    /* ESC ESC_END means END data byte */
#define ESC_ESC						0335    /* ESC ESC_ESC means ESC data byte */

#define __RESET_WATCHDOG1() SRS = 0x55U; SRS = 0xAAU;
//#define __RESET_WATCHDOG1() Watchdog_Clear();

#define readOneChar(readChar)		while (!SCIS1_RDRF) {__RESET_WATCHDOG1()}; readChar = SCID;


// --------------------------------------------------------------------------
// Functions prototypes.

void sendOneChar(FrameDataType data);
void serialReceiveTask(void *pvParameters);
void serialTransmitFrame(FramePtrType framePtr, FrameCntType frameSize);
FrameCntType serialReceiveFrame(FramePtrType framePtr, FrameCntType maxFrameSize);

void processFrame(FramePtrType framePtr, FrameCntType frameSize);

#endif /* SERIALBUS_H_ */
