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
extern uint8_t gCurValue;
extern uint8_t gMinValue;
extern uint8_t gMaxValue;
void KBI_OnInterrupt(void) {

	uint8_t commandBytes[] = { BUTTON_COMMAND, MYBUSID, 0x00 };
	uint32_t loops = 0;
	uint8_t buttonNum = 0;
	uint8_t pos;
	uint8_t kbiVal = 0;

	while ((buttonNum == 0) && (loops++ < 50000)) {
		kbiVal = KBI_GetVal();
		if ((kbiVal & UP_BUTTON) == 0) {
			buttonNum = 1;
			if (gCurValue < gMaxValue) {
				gCurValue++;
				displayCurrentValue();
			}
		} else if ((kbiVal & DOWN_BUTTON) == 0) {
			buttonNum = 2;
			if (gCurValue > gMinValue) {
				gCurValue--;
				displayCurrentValue();
			}
		} else if ((kbiVal & ACK_BUTTON) == 0) {
			buttonNum = 3;
			commandBytes[BUTTON_CMD_DATA_POS] = gCurValue;
			// Turn on the RS485 driver.
			RS485_DRV_PutVal(1);
			for (pos = 0; pos <= 3; ++pos) {
				// Wait while the TX buffer is full.
				while (SCIS1_TDRE == 0) {
					
				}
				ASYNC_SendChar(commandBytes[pos]);
			}
			// Wait while the TX buffer is full.
			while (SCIS1_TDRE == 0) {
				
			}
			// The last TX character takes a few ms to transmit through.
			Cpu_Delay100US(15);
			
			// Turn off the 485 driver.
			RS485_DRV_PutVal(0);
			clearDisplay();
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
