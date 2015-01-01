/* ###################################################################
 **     Filename    : Events.c
 **     Project     : ProcessorExpert
 **     Processor   : MC9S08QG8CDT
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

#define RESET_MCU()		__asm DCB 0x8D

void HandleKeypress();
void HandleSendAckCommand(void);
void HandleFlashANewBusAddr(void);

bool gKeypressPending = FALSE;
bool gAckButtonLockout = FALSE;
extern uint8_t gCurValue;
extern uint8_t gMinValue;
extern uint8_t gMaxValue;
extern uint8_t kMyPermanentBusAddr;
extern EDeviceState gDeviceState;

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

	// A key's been pressed, so now wait until the debounce time or config time to see what state the keys are in.
	if (!gKeypressPending) {
		gKeypressPending = TRUE;
		DebounceTimer_Enable();
		ConfigModeWait_Enable();
		HandleKeypress();
	}
}

/*
 * After a debounce we finally handle a keypress.
 */
void HandleKeypress() {

	uint8_t kbiValue = KBI_GetVal();

	// Only handle keypress when exactly one button pressed.
	if ((((kbiValue & UP_BUTTON) == 0) + ((kbiValue & DOWN_BUTTON) == 0) + ((kbiValue & ACK_BUTTON) == 0)) == 1) {
		// Only handle key presses when in active or config modes.
		if (gDeviceState != eInactive) {
			if (((kbiValue & UP_BUTTON) == 0)) {
				if (gCurValue < gMaxValue) {
					gCurValue++;
					if (gDeviceState == eActive) {
						displayValue(gCurValue);
					} else {
						displayValueBlink(gCurValue);
					}
				}
			} else if (((kbiValue & DOWN_BUTTON) == 0) && (gDeviceState != eInactive)) {
				if (gCurValue > gMinValue) {
					gCurValue--;
					if (gDeviceState == eActive) {
						displayValue(gCurValue);
					} else {
						displayValueBlink(gCurValue);
					}
				}
			} else if ((kbiValue & ACK_BUTTON) == 0) {
				if (!gAckButtonLockout) {
					if (gDeviceState == eActive) {
						gAckButtonLockout = TRUE;
						AckButtonDelay_Enable();
						HandleSendAckCommand();
					} else {
						HandleFlashANewBusAddr();
					}
				}
			}
		}
	}
}

/*
 * 
 */
void HandleSendAckCommand() {
	uint8_t commandBytes[] = { BUTTON_COMMAND, 0x00, 0x00 };
	uint8_t pos;

	commandBytes[1] = kMyPermanentBusAddr;

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
	Cpu_Delay100US(20);

	// Turn off the 485 driver.
	RS485_DRV_PutVal(0);

	// Don't clear the display - wait for the host to ACK that it got our button press correctly.
	//clearDisplay();
}

/*
 * 
 */
void HandleFlashANewBusAddr() {
	
	byte error;

	// Write the current value to flash for permanent storage.
	error = Flash_SetByteFlash((Flash_TAddress) &kMyPermanentBusAddr, gCurValue);

	if (error != ERR_OK) {
		RESET_MCU();
	}

	gDeviceState = eInactive;
	gCurValue = 0;
	gMinValue = 0;
	gMaxValue = 0;
	
	clearDisplay();
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
	DebounceTimer_Disable();
	gKeypressPending = FALSE;
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
void ConfigModeWait_OnInterrupt(void) {

	uint8_t kbiVal = KBI_GetVal();

	if (((kbiVal & UP_BUTTON) == 0) && ((kbiVal & DOWN_BUTTON) == 0) && ((kbiVal & ACK_BUTTON) == 0)) {
		// All three buttons are down.
		gDeviceState = eConfigMode;
		if (kMyPermanentBusAddr == UNSET_BUSADDR) {
			gCurValue = 0x01;
		} else {
			gCurValue = kMyPermanentBusAddr;
		}
		// Don't let the user set the address to 0 (the broadcast addr).
		gMinValue = 1;
		gMaxValue = 99;
		displayValueBlink(gCurValue);
	}
	ConfigModeWait_Disable();
}

/*
 ** ===================================================================
 **     Event       :  AckButtonDelay_OnInterrupt (module Events)
 **
 **     Component   :  AckButtonDelay [TimerInt]
 **     Description :
 **         When a timer interrupt occurs this event is called (only
 **         when the component is enabled - <Enable> and the events are
 **         enabled - <EnableEvent>). This event is enabled only if a
 **         <interrupt service/event> is enabled.
 **     Parameters  : None
 **     Returns     : Nothing
 ** ===================================================================
 */
void AckButtonDelay_OnInterrupt(void) {
	gAckButtonLockout = FALSE;
	AckButtonDelay_Disable();
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
