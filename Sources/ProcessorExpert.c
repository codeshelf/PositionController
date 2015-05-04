/*
 * Codeshelf
 * � Copyright 2005-2013 Jeffrey B. Williams
 * All rights reserved
 * 
 * $Id$
 * $Name$
 */

#include "Cpu.h"
#include "Events.h"
#include "KBI.h"
#include "Uart.h"
#include "Rs485Dir.h"
#include "StatusLedClk.h"
#include "StatusLedData.h"
#include "DebounceTimer.h"
#include "Flash.h"
#include "ConfigModeWait.h"
#include "AckButtonDelay.h"
#include "SharpDisplayCS.h"
#include "Wait.h"
#include "SharpDisplay.h"
#include "LdoEnable.h"
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "StatusLed.h"
#include "SerialBus.h"
#include "Commands.h"
#include "Display.h"

void main(void) {
	FrameDataType frame[MAX_FRAME_BYTES];
	FrameCntType frameSize;

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
	PE_low_level_init();
	/*** End of Processor Expert internal initialization.                    ***/
		
#ifdef __IIC
	Init_I2CM();
#endif
	
#ifdef __SharpDisplay
	SPIC2_SPC0 = 0x01;
	SPIC2_BIDIROE = 0x01;
#endif
	
	Wait_Waitms(20);
	initDisplay();
	setStatusLed(0, 0, 0);
	RS485_RX;

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
