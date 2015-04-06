/*
 * Display.h
 *
 *  Created on: Jun 12, 2014
 *      Author: jeffw
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

void initDisplay(void);
void clearDisplay(void);
void setValues(FramePtrType, FrameCntType);
void displayValue(uint8_t displayValue);
void displayValueBlink(uint8_t displayValue);
void displayFirmwareVersion(void);

#endif /* DISPLAY_H_ */
