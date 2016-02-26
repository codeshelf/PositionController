/* ###################################################################
**     Filename    : Events.h
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
** @file Events.h
** @version 01.02
** @brief
**         This is user's event module.
**         Put your event handler code here.
*/         
/*!
**  @addtogroup Events_module Events module documentation
**  @{
*/         

#ifndef __Events_H
#define __Events_H
/* MODULE Events */

#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "KBI.h"
#include "Uart.h"
#include "IIC.h"
#include "Rs485Dir.h"
#include "LedDriverReset.h"
#include "DebounceTimer.h"
#include "Flash.h"
#include "ConfigModeWait.h"
#include "AckButtonDelay.h"
#include "Wait.h"
#include "StatusLedClkDo.h"
#include "TPM1.h"


/*
** ===================================================================
**     Event       :  ASl1_OnError (module Events)
**
**     Component   :  ASl1 [AsynchroSlave]
**     Description :
**         This event is called when a channel error (not the error
**         returned by a given method) occurs. The errors can be read
**         using <GetError> method.
**         The event is available only when the <Interrupt
**         service/event> property is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

/*
** ===================================================================
**     Event       :  ASl1_OnRxChar (module Events)
**
**     Component   :  ASl1 [AsynchroSlave]
**     Description :
**         This event is called after a correct character is received.
**         The event is available only when the <Interrupt
**         service/event> property is enabled and either the <Receiver>
**         property is enabled or the <SCI output mode> property (if
**         supported) is set to Single-wire mode.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

/*
** ===================================================================
**     Event       :  ASl1_OnTxChar (module Events)
**
**     Component   :  ASl1 [AsynchroSlave]
**     Description :
**         This event is called after a character is transmitted.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

/*
** ===================================================================
**     Event       :  ASl1_OnSelect (module Events)
**
**     Component   :  ASl1 [AsynchroSlave]
**     Description :
**         This event is called when the slave is selected by master to
**         communicate.
**         The event is available only when the <Interrupt
**         service/event> property is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

/*
** ===================================================================
**     Event       :  ASl1_OnUnselect (module Events)
**
**     Component   :  ASl1 [AsynchroSlave]
**     Description :
**         This event is called when the slave stops communicating with
**         the master - the master has selected another slave to
**         communicate.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

/*
** ===================================================================
**     Event       :  I2C_OnReceiveData (module Events)
**
**     Component   :  I2C [InternalI2C]
**     Description :
**         This event is invoked when I2C finishes the reception of the
**         data successfully. This event is not available for the SLAVE
**         mode and if both RecvChar and RecvBlock are disabled. This
**         event is enabled only if interrupts/events are enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

/*
** ===================================================================
**     Event       :  I2C_OnTransmitData (module Events)
**
**     Component   :  I2C [InternalI2C]
**     Description :
**         This event is invoked when I2C finishes the transmission of
**         the data successfully. This event is not available for the
**         SLAVE mode and if both SendChar and SendBlock are disabled.
**         This event is enabled only if interrupts/events are enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

void KBI_OnInterrupt(void);
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

/*
** ===================================================================
**     Event       :  I2C_OnReceiveData (module Events)
**
**     Component   :  I2C [InternalI2C]
**     Description :
**         This event is invoked when I2C finishes the reception of the
**         data successfully. This event is not available for the SLAVE
**         mode and if both RecvChar and RecvBlock are disabled. This
**         event is enabled only if interrupts/events are enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

/*
** ===================================================================
**     Event       :  I2C_OnTransmitData (module Events)
**
**     Component   :  I2C [InternalI2C]
**     Description :
**         This event is invoked when I2C finishes the transmission of
**         the data successfully. This event is not available for the
**         SLAVE mode and if both SendChar and SendBlock are disabled.
**         This event is enabled only if interrupts/events are enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/

void DebounceTimer_OnInterrupt(void);
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

void ConfigModeWait_OnInterrupt(void);
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

void AckButtonDelay_OnInterrupt(void);
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

void Cpu_OnSwINT(void);
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

void Cpu_OnReset(byte Reason);
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

void Cpu_OnLvwINT(void);
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

/* END Events */

void HandleKeypress(void);

#endif /* __Events_H*/

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
