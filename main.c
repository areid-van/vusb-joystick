/* 8 button one d-pad joystick on AtTiny2313

Copyright (C) 2014 Shay Green
Copyright (C) 2017 Adam Reid
Licensed under GPL v2 or later. See License.txt. */

#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "usbdrv/usbdrv.h"

static uint8_t report [3]; // current
static uint8_t report_out [3]; // last sent over USB

static void read_joy( void )
{
	report [0] = 0;
	report [1] = 0;
	report [2] = 0;
	
	// Buttons
	report[0] = ~PINB;
	
	// X
	if ( ! (PIND & 0x01) ) report [1] = -127;
	if ( ! (PIND & 0x02) ) report [1] = +127;

	// Y
	if ( ! (PIND & 0x10) ) report [2] = -127;
	if ( ! (PIND & 0x20) ) report [2] = +127;
	
	
}

PROGMEM const char usbHidReportDescriptor [USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
    0x05, 0x01,     //USAGE_PAGE (Generic Desktop)
    0x09, 0x05,     //USAGE (Game Pad)
    0xa1, 0x01,     //COLLECTION (Application)
    0xa1, 0x00,     //  COLLECTION (Physical)
    0x05, 0x09,     //    USAGE_PAGE (Button)
    0x19, 0x01,     //    USAGE_MINIMUM (Button 1)
    0x29, 0x08,     //    USAGE_MAXIMUM (Button 8)
    0x15, 0x00,     //    LOGICAL_MINIMUM (0)
    0x25, 0x01,     //    LOGICAL_MAXIMUM (1)
    0x95, 0x08,     //    REPORT_COUNT (8)
    0x75, 0x01,     //    REPORT_SIZE (1)
    0x81, 0x02,     //    INPUT (Data,Var,Abs)
    0x05, 0x01,     //    USAGE_PAGE (Generic Desktop)
    0x09, 0x30,     //    USAGE (X)
    0x09, 0x31,     //    USAGE (Y)
    0x15, 0x81,     //    LOGICAL_MINIMUM (-127)
    0x25, 0x7f,     //    LOGICAL_MAXIMUM (127)
    0x75, 0x08,     //    REPORT_SIZE (8)
    0x95, 0x02,     //    REPORT_COUNT (2)
    0x81, 0x02,     //    INPUT (Data,Var,Abs)
    0xc0,           //  END COLLECTION
    0xc0            //END COLLECTION
};

uint8_t usbFunctionSetup( uint8_t data [8] )
{
	usbRequest_t const* rq = (usbRequest_t const*) data;

	if ( (rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS )
		return 0;
	
	switch ( rq->bRequest )
	{
	case USBRQ_HID_GET_REPORT: // HID joystick only has to handle this
		usbMsgPtr = (usbMsgPtr_t) report_out;
		return sizeof report_out;
	
	//case USBRQ_HID_SET_REPORT: // LEDs on joystick?
	
	default:
		return 0;
	}
}

int main( void )
{
	wdt_enable(WDTO_1S);
	usbInit();

	usbDeviceDisconnect();
	int i;
    	for(i = 0; i<250; i++) {
        	wdt_reset();
        	_delay_ms(2);
    	}
    	usbDeviceConnect();

	sei();
	
	DDRB  &= ~0xff;
	PORTB |=  0xff;
	DDRD  &= ~0x33;
	PORTD |=  0x33;
	
	for ( ;; )
	{
		wdt_reset();
		usbPoll();
		
		if ( usbInterruptIsReady() )
		{
			read_joy();
			
			if ( memcmp( report_out, report, sizeof report ) )
			{
				memcpy( report_out, report, sizeof report );
				usbSetInterrupt( report_out, sizeof report_out );
			}
		}
	}
	
	return 0;
}
