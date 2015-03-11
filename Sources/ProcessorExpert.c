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
#include "Rs485Dir.h"
#include "StatusLedClk.h"
#include "StatusLedSdi.h"
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
//#include "I2CMaster.h"
#include "Display.h"
#include "SerialBus.h"
#include "Commands.h"

void main(void) {
	FrameDataType frame[MAX_FRAME_BYTES];
	FrameCntType frameSize;

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
	PE_low_level_init();
	/*** End of Processor Expert internal initialization.                    ***/
	
	// Disable SPI MISO.  (Don't know why.)
	SPIC2_SPC0 = 1;
	
//	Init_I2CM();
	initDisplay();
	setStatusLed(0, 2, 10);

	clearLcdDisplay();
	displayString(20, 10,  "32C", 2);
	displayString(2, 100, "PACK LIGHT", 1);
	displayBarcode(20, 50, "*01*", 1);
	
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
