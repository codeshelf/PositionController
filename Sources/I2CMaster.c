/*
 * Codeshelf
 * © Copyright 2005-2013 Jeffrey B. Williams
 * All rights reserved
 * 
 * $Id$
 * $Name$
 */

#include "I2CMaster.h"

#define IIC_ERROR_STATUS 0
#define IIC_READY_STATUS 1
#define IIC_HEADER_SENT_STATUS 2
#define IIC_DATA_TRANSMISSION_STATUS 3
#define IIC_DATA_SENT_STATUS 4

static byte I2CM_RX_Data[IIC_BUFFER_SIZE];
static byte I2CM_TX_Data[IIC_BUFFER_SIZE];
static byte I2CM_Counter = 0;
static byte I2CM_Step = IIC_READY_STATUS;
static byte I2CM_Length = 1;
static byte I2CM_Data_Direction = 0;
static byte Last_But_One = 0;
static bool No_Resp = TRUE;

void Init_I2CM(void) {
	IICC_IICEN = 1; /* Enable IIC module        */
	IICF = IIC_DIV; /* Set IIC frequency = 100K Baud  */
	IICF_ICR = 0x0e;
	IICF_MULT = 0x00;
	I2CM_Step = IIC_READY_STATUS;
	IICC_IICIE = 1; /* Enable IIC interrupts */
}

interrupt void I2C_ISR(void) {
	byte Temp;

	/* Interrupts generated after the slave address has been sent and after each data byte
	 ,write or read mode, when performing a read the IIC needs to be disabled straight after the data
	 byte is read or another interrupt will be generated.*/

	Temp = IICS; /* ACK the interrupt */
	IICS_IICIF = 1;
	/* Verify the Arbitration lost status */
	if (IICS_ARBL == 1) {
		IICS_ARBL = 1; // clear=1
		IICC_MST = 0; // reset master bit
		IICC_IICEN = 0;
		I2CM_Step = IIC_ERROR_STATUS;
		return;
	}
	/* if State = Header Sent */
	if (I2CM_Step == IIC_HEADER_SENT_STATUS) {
		if (IICS_RXAK == 1) {
			IICC_MST = 0; // reset master bit
			IICC_IICEN = 0;
			I2CM_Step = IIC_READY_STATUS;
			return;
		} else {
			IICC_TX = I2CM_Data_Direction; // set direction
			I2CM_Step = IIC_DATA_TRANSMISSION_STATUS; // set next state   
			if (IICC_TX == 0) { /* If we are reading data clock in first slave byte */
				Temp = IICD; // read data byte
				return;
			}
		}
	}

	/* If state = byte transmision is in progress.*/
	if (I2CM_Step == IIC_DATA_TRANSMISSION_STATUS) {
		if (IICC_TX == 1) { /* If Master is sending data to slave */
			IICD = I2CM_TX_Data[I2CM_Counter]; /* Send the next byte */
			I2CM_Counter += 1;
			/* Mark we are done sending Bytes */
			if (I2CM_Length == I2CM_Counter) {
				I2CM_Step = IIC_DATA_SENT_STATUS;
			}
		} else { // reading data from slave      
			if (I2CM_Counter == Last_But_One) { // last byte ?                  
				I2CM_RX_Data[I2CM_Counter] = IICD; /* Read the byte */
				IICC_MST = 0; // reset master bit to
				              // Generate a stop condition
				IICC_IICEN = 0; // disable IIC                 
				I2CM_Counter += 1; // do not optimise by moving above
				I2CM_Step = IIC_READY_STATUS; //finished
				No_Resp = FALSE;
			} else {
				I2CM_RX_Data[I2CM_Counter] = IICD; /* Read the next byte */
				I2CM_Counter += 1;
			}

			/*  Master should not ACK the last byte */
			if (I2CM_Counter == Last_But_One)
				IICC_TXAK = 1; /* If penultimate byte indicate end of transfer */
		}

	}
	/* if state = We are done with the transmition.*/
	else {

		IICC_TX = 0; // set RX mode       
		IICC_MST = 0; // reset master bit to
		              // Generate a stop condition
		IICC_IICEN = 0; // disable IIC
		I2CM_Step = IIC_READY_STATUS; // finished
		No_Resp = FALSE;
	}
}

bool I2CM_Get_Busy_Status(void) {
	return (I2CM_Step > IIC_READY_STATUS);
}

bool I2CM_Get_No_Resp_Status(void) {
	// return TRUE if no response from last requested slave
	return No_Resp;
}

void I2CM_Read_Bytes(byte Slave_Addr, byte Number_Of_Bytes) {
	byte Temp;

	IICC_IICEN = 0;
	I2CM_Length = Number_Of_Bytes;
	Last_But_One = Number_Of_Bytes - 1; // save time calculating later
	I2CM_Counter = 0; // reset counter
	I2CM_Step = IIC_HEADER_SENT_STATUS;
	I2CM_Data_Direction = 0; // set for read
	No_Resp = TRUE;
	IICC_IICEN = 1;
	/* Format the Address to fit in the IICA register and place a 1 on the R/W bit. */
	Slave_Addr <<= 1; // shift left 1 bit
	Slave_Addr &= 0xFE;
	Slave_Addr |= 0x01; /* Set the Read from slave bit. */
	Temp = IICS; /* Clear any pending interrupt  */
	IICS_IICIF = 1;
	IICC_MST = 0; // reset master bit
	IICC_TX = 1; /* Select Transmit Mode          */
	IICC_MST = 1; /* Select Master Mode (Send Start Bit)*/
	IICD = Slave_Addr; /* Send selected slave address   */
}

void I2CM_Get_Read_Data(byte* Data) {
	byte Temp;

	for (Temp = 0; Temp < I2CM_Length; Temp++)
		Data[Temp] = I2CM_RX_Data[Temp];
}

void I2CM_Write_Bytes(byte Slave_Addr, byte Number_Of_Bytes, const byte* Data) {
	byte Temp;
	IICC_IICEN = 0;
	I2CM_Length = Number_Of_Bytes;
	I2CM_Counter = 0;
	I2CM_Step = IIC_HEADER_SENT_STATUS;
	I2CM_Data_Direction = 1;
	No_Resp = TRUE;
	// copy data into buffer
	for (Temp = 0; Temp < Number_Of_Bytes; Temp++)
		I2CM_TX_Data[Temp] = Data[Temp];
	IICC_IICEN = 1;
	/* Format the Address to fit in the IICA register and place a 0 on the R/W bit.*/
	Slave_Addr <<= 1; // shift left 1 bit
	Slave_Addr &= 0xFE;
	Temp = IICS; /* Clear any pending interrupt */
	IICS_IICIF = 1;
	IICC_MST = 0; // reset master bit
	IICS_SRW = 0;
	IICC_TX = 1; /* Select Transmit Mode */
	IICC_MST = 1; /* Select Master Mode (Send Start Bit) */
	IICD = Slave_Addr; /* Send selected slave address */
}
