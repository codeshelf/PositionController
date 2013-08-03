/* ###################################################################
 **     Filename    : Events.c
 **     Project     : ProcessorExpert
 **     Processor   : MC9S08QG8CPB
 **     Component   : Events
 **     Version     : Driver 01.02
 **     Compiler    : CodeWarrior HCS08 C Compiler
 **     Date/Time   : 2013-07-27, 15:56, # CodeGen: 0
 **     Abstract    :
 **         This is user's event module.
 **         Put your event handler code here.
 **     Settings    :
 **     Contents    :
 **         No public methods
 **
 ** ###################################################################*/
/*!
 ** @file Events.c
 ** @version 01.02
 ** @brief
 **         This is user's event module.
 **         Put your event handler code here.
 */
/*!
 **  @addtogroup Events_module Events module documentation
 **  @{
 */
/* MODULE Events */

#include "Cpu.h"
#include "Events.h"
#include "Commands.h"

/* User includes (#include below this line is not maintained by Processor Expert) */

/*
 ** ===================================================================
 **     Event       :  KBI_OnInterrupt (module Events)
 **
 **     Component   :  KBI [KBI]
 **     Description :
 **         This event is called when the active signal edge/level
 **         occurs. This event is enabled only if <Interrupt
 **         service/event> property is enabled.
 **     Parameters  : None
 **     Returns     : Nothing
 ** ===================================================================
 */
void KBI_OnInterrupt(void) {

	uint8_t commandBytes[] = { BUTTON_COMMAND, MYBUSID, 0x00 };
	uint32_t loops = 0;
	uint8_t buttonNum = 0;
	uint8_t pos;
	uint8_t kbiVal = 0;

	while ((buttonNum == 0) && (loops++ < 50000)) {
		kbiVal = KBI_GetVal();
		if (kbiVal & KBI_NUM_PIN0) {
			buttonNum = 1;
		} else if (kbiVal & KBI_NUM_PIN1) {
			buttonNum = 2;
		} else if (kbiVal & KBI_NUM_PIN2) {
			buttonNum = 3;
		}
	}

	if (buttonNum != 0) {
		commandBytes[BUTTON_CMD_BNUM_POS];
		for (pos = 0; pos < 4; ++pos) {
			ASYNC_SendChar(commandBytes[pos]);
		}
	}
}

/* END Events */

/*!
 ** @}
 */
/*
 ** ###################################################################
 **
 **     This file was created by Processor Expert 10.2 [05.07]
 **     for the Freescale HCS08 series of microcontrollers.
 **
 ** ###################################################################
 */
