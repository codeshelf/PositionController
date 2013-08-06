/*
 * Codeshelf
 * © Copyright 2005-2013 Jeffrey B. Williams
 * All rights reserved
 * 
 * $Id$
 * $Name$
 */

#include "Cpu.h"
#include "Events.h"
#include "KBI.h"
#include "ASYNC.h"
#include "IIC.h"
#include "RS485_DRV.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "I2CMaster.h"

#include "SerialBus.h"
#include "Commands.h"

void main(void) {
	uint8_t data;
	FrameDataType frame[MAX_FRAME_BYTES];
	FrameCntType frameSize;

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
	PE_low_level_init();
	/*** End of Processor Expert internal initialization.                    ***/
	
//	Init_I2CM();
	initDisplay();

	for (;;) {
		// Read a frame from the serial bus and then process it.
		frameSize = serialReceiveFrame(frame, MAX_FRAME_BYTES);
		if (frameSize > 0) {
			processFrame(frame, frameSize);
		}
	}

	/*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/
