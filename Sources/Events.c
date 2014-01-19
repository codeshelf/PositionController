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
extern uint8_t POSITION_NUM;

bool gKbReady = TRUE;
void KBI_OnInterrupt(void) {

	uint8_t myBusId = 0x0;
	if (Flash_GetByteFlash((unsigned int) (&POSITION_NUM), &myBusId) == ERR_OK) {

		uint8_t commandBytes[] = { BUTTON_COMMAND, 0x00, 0x00 };
		uint32_t loops = 0;
		uint8_t buttonNum = 0;
		uint8_t pos;
		uint8_t kbiVal = 0;
		
		commandBytes[1] = myBusId;

		if (gKbReady) {
			while ((buttonNum == 0) && (loops++ < 50000)) {
				kbiVal = KBI_GetVal();
				if ((kbiVal & UP_BUTTON == 0) || (kbiVal & DOWN_BUTTON == 0) || (kbiVal & ACK_BUTTON == 0)) {
					// All three buttons are down.
					gKbReady = FALSE;
					ConfigModeWait_Enable();
				} else if ((kbiVal & UP_BUTTON) == 0) {
					buttonNum = 1;
					if (gCurValue < gMaxValue) {
						gCurValue++;
						displayValue(gCurValue);
					}
				} else if ((kbiVal & DOWN_BUTTON) == 0) {
					buttonNum = 2;
					if (gCurValue > gMinValue) {
						gCurValue--;
						displayValue(gCurValue);
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
			if (buttonNum != 0) {
				gKbReady = FALSE;
				DebounceTimer_Enable();
			}
		}}
}

/*
 ** ===================================================================
 **     Event       :  DebounceTimer_OnInterrupt (module Events)
 **
 **     Component   :  DebounceTimer [TimerInt]
 **     Description :
 **         When a timer interrupt occurs this event is called (only
 **         when the component is enabled - <Enable> and the events are
 **         enabled - <EnableEvent>). This event is enabled only if a
 **         <interrupt service/event> is enabled.
 **     Parameters  : None
 **     Returns     : Nothing
 ** ===================================================================
 */
void DebounceTimer_OnInterrupt(void) {
	/* Write your code here ... */
	gKbReady = TRUE;
	DebounceTimer_Disable();
}

/*
** ===================================================================
**     Event       :  ConfigModeWait_OnInterrupt (module Events)
**
**     Component   :  ConfigModeWait [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the component is enabled - <Enable> and the events are
**         enabled - <EnableEvent>). This event is enabled only if a
**         <interrupt service/event> is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void ConfigModeWait_OnInterrupt(void)
{
  /* Write your code here ... */
	uint8_t kbiVal = 0;
	uint8_t myBusId = 0x0;

	gKbReady = TRUE;
	ConfigModeWait_Disable();
	
	kbiVal = KBI_GetVal();
	if ((kbiVal & UP_BUTTON == 0) || (kbiVal & DOWN_BUTTON == 0) || (kbiVal & ACK_BUTTON == 0)) {
		if (Flash_GetByteFlash((unsigned int) (&POSITION_NUM), &myBusId) == ERR_OK) {
			displayValueBlink(myBusId);
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
