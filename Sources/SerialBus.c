/*
 * Codeshelf
 * © Copyright 2005-2013 Jeffrey B. Williams
 * All rights reserved
 * 
 * $Id$
 * $Name$
 */

#include "SerialBus.h"
#include "Uart.h"

// --------------------------------------------------------------------------

void sendOneChar(FrameDataType data) {
	Uart_SendChar(data);
	// Wait while the TX buffer is full.
	while (SCIS1_TDRE == 0) {
	}
}

// --------------------------------------------------------------------------

void serialTransmitFrame(FramePtrType framePtr, FrameCntType frameSize) {

	FrameCntType totalBytesSent;
	FrameCntType charsSent;
	
	RS485_TX;

	// Send the frame contents to the controller via the serial port.
	// First send the framing character.
#pragma MESSAGE DISABLE C2706 /* WARNING C2706: Octal # */
	// Send another framing character. (For some stupid reason the USB routine doesn't try very hard, so we have to loop until it succeeds.)
	sendOneChar(END);

	totalBytesSent = 0;

	for (charsSent = 0; charsSent < frameSize; charsSent++) {

		switch (*framePtr) {
			/* if it's the same code as an END character, we send a
			 * special two character code so as not to make the
			 * receiver think we sent an END
			 */
			case END:
				sendOneChar(ESC);
				sendOneChar(ESC_END);
				break;

				/* if it's the same code as an ESC character,
				 * we send a special two character code so as not
				 * to make the receiver think we sent an ESC
				 */
			case ESC:
				sendOneChar(ESC);
				sendOneChar(ESC_ESC);
				break;

				/* otherwise, we just send the character
				 */
			default:
				sendOneChar(*framePtr);
		}

		framePtr++;
	}

	// Send another framing character. (For some stupid reason the USB routine doesn't try very hard, so we have to loop until it succeeds.)
	sendOneChar(END);
	sendOneChar(END);
	
	// Wait while the TX buffer is full.
	while (SCIS1_TDRE == 0) {
	}
	// The last TX character takes a few ms to transmit through.
	Cpu_Delay100US(20);

	RS485_RX;
}

// --------------------------------------------------------------------------

FrameCntType serialReceiveFrame(FramePtrType framePtr, FrameCntType maxFrameSize) {
	FrameDataType nextByte;
	FrameCntType bytesReceived = 0;

	// Loop reading bytes until we put together a whole frame.
	// Make sure not to copy them into the frame if we run out of room.

	while (bytesReceived < maxFrameSize) {

		readOneChar(nextByte);
		switch (nextByte) {

			// If it's an END character then we're done with the frame.
			case END:
				if (bytesReceived) {
					return bytesReceived;
				} else {
					break;
				}

				/* If it's the same code as an ESC character, wait and get another character and then figure out
				 * what to store in the frame based on that.
				 */
			case ESC:
				readOneChar(nextByte);

				/* If "c" is not one of these two, then we have a protocol violation.  The best bet
				 * seems to be to leave the byte alone and just stuff it into the frame
				 */
				switch (nextByte) {
					case ESC_END:
						nextByte = END;
						break;
					case ESC_ESC:
						nextByte = ESC;
						break;
				}
				if (bytesReceived < maxFrameSize)
					framePtr[bytesReceived++] = nextByte;
				break;

				// Here we fall into the default handler and let it store the character for us.
			default:
				if (bytesReceived < maxFrameSize)
					framePtr[bytesReceived++] = nextByte;
				break;
		}
	}

	return bytesReceived;
}
