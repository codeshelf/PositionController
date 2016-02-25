/* ###################################################################
 **     Filename    : Events.c
 **     Project     : ProcessorExpert
 **     Processor   : MC9S08SH8CTG
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
#include "Display.h"

/* User includes (#include below this line is not maintained by Processor Expert) */

#define RESET_MCU()		__asm DCB 0x8D

extern uint8_t gCurValue;
extern uint8_t gMinValue;
extern uint8_t gMaxValue;
extern uint8_t kMyPermanentBusAddr;
extern uint8_t gMyBusAddr;
extern EDeviceState gDeviceState;
extern uint8_t gLedRedValue;
extern uint8_t gLedGreenValue;
extern uint8_t gLedBlueValue;
extern uint8_t gLedLightStyle;

void handleKeypress(void);
void handleFlashANewBusAddr(void);

bool gKeypressPending = FALSE;
bool gAckButtonLockout = FALSE;

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
		handleKeypress();
	}
}

/*
 * After a debounce we finally handle a keypress.
 */
void handleKeypress() {

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
					} else if (gDeviceState == eConfigMode || gDeviceState == eMassConfigMode) {
						displayValueBlink(gCurValue);
					}
				}
			} else if (((kbiValue & DOWN_BUTTON) == 0) && (gDeviceState != eInactive)) {
				if (gCurValue > gMinValue) {
					gCurValue--;
					if (gDeviceState == eActive) {
						displayValue(gCurValue);
					} else if (gDeviceState == eConfigMode || gDeviceState == eMassConfigMode) {
						displayValueBlink(gCurValue);
					}
				}
			} else if ((kbiValue & ACK_BUTTON) == 0) {
				if (!gAckButtonLockout) {
					if (gDeviceState == eActive) {
						gAckButtonLockout = TRUE;
						AckButtonDelay_Enable();
						sendAckCommand();
					} else if (gDeviceState == eConfigMode || gDeviceState == eMassConfigMode) {
						handleFlashANewBusAddr();
					} else { // eAddressDisplayMode, eFirmwareDisplayMode
						clearDisplay();
					}
				}
			}
		}
	}
}

/*
 * 
 */
void handleFlashANewBusAddr() {
	
	byte error;

	// Write the current value to flash for permanent storage.
	error = Flash_SetByteFlash((Flash_TAddress) &kMyPermanentBusAddr, gCurValue);

	if (error != ERR_OK) {
		RESET_MCU();
	}

	gCurValue = 0;
	gMinValue = 0;
	gMaxValue = 0;
	gMyBusAddr = kMyPermanentBusAddr;
	
	clearDisplay();
	

	if (gDeviceState == eMassConfigMode) {
		sendIdSetupIncCommand(gMyBusAddr);
	}
	
	gDeviceState = eInactive;
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
	uint8_t kbiValue = KBI_GetVal();

	// If any buttons are still down then re-fire the debounce timer.
	if (((kbiValue & UP_BUTTON) == 0) || ((kbiValue & DOWN_BUTTON) == 0) || ((kbiValue & ACK_BUTTON) == 0)) {
		gKeypressPending = TRUE;
		DebounceTimer_Enable();
	} else {
		gKeypressPending = FALSE;
		DebounceTimer_Disable();
	}
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
	} else if (((kbiVal & UP_BUTTON) == 0) && ((kbiVal & DOWN_BUTTON) == 0) && ((kbiVal & ACK_BUTTON) != 0)) {
		// Up/Down buttons only are down.
		gDeviceState = eFirmwareDisplayMode;
		displayFirmwareVersion();
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

/*
** ===================================================================
**     Event       :  Cpu_OnSwINT (module Events)
**
**     Component   :  Cpu [MC9S08SH8_16]
**     Description :
**         This event is called when the SWI interrupt had occurred.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void Cpu_OnSwINT(void)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  Cpu_OnReset (module Events)
**
**     Component   :  Cpu [MC9S08SH8_16]
**     Description :
**         This software event is called after a reset.
**     Parameters  :
**         NAME            - DESCRIPTION
**         Reason          - The content of the system reset
**                           status register.
**     Returns     : Nothing
** ===================================================================
*/
void Cpu_OnReset(byte Reason)
{
  /* Write your code here ... */
}

/*
** ===================================================================
**     Event       :  Cpu_OnLvwINT (module Events)
**
**     Component   :  Cpu [MC9S08SH8_16]
**     Description :
**         This event is called when the Low voltage warning (LVW)
**         interrupt had occurred. This event can be enabled only if
**         both <LVD module> and <LVW Interrupt > are enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void Cpu_OnLvwINT(void)
{
  /* Write your code here ... */
}

//ISR(LedOn) {
//	//setStatusLed(gLedRedValue,gLedBlueValue,gLedGreenValue);
//	//TPM1SC;
//	//TPM1SC_TOF = 0;
//}
//
//ISR(LedOff) {
//	
//	//if (gLedLightStyle == 0) {
//	//	setStatusLed(0,0,0);
//	//}
//	//TPM1C0SC;
//	//TPM1C0SC_CH0F = 0;
//}

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
