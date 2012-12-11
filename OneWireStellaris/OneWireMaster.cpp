/**
 * @file OneWireMaster.cpp
 *
 * OneWire master controller class, for handling OneWire devices
 * 
 * Based off of Petras Saduikis' mbed port of Jim Studt's
 * Arduino OneWire library. Some pieces of this code have been taken from
 * Dallas Semiconductor's sample code, bearing the copyright below.
 * Additionally, though I believe very little of Petras' code remains, I have
 * included his copyright block just in case.
 *
 * Copyright (C) <2012> Cliff Chapman <chapman.cliff@gmail.com>
 *
 * This file is part of the Stellaris OneWire Library.
 * 
 * The Stellaris OneWire Library is free software: you can redistribute it 
 * and/or modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 3 of the License, 
 * or (at your option) any later version.
 * 
 * OneWireCRC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OneWireCRC.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
* OneWireCRC. This is a port to mbed of Jim Studt's Adruino One Wire
* library. Please see additional copyrights below this one, including
* references to other copyrights.
*
* Copyright (C) <2009> Petras Saduikis <petras@petras.co.uk>
*
* This file is part of OneWireCRC.
*
* OneWireCRC is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* OneWireCRC is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with OneWireCRC.  If not, see <http://www.gnu.org/licenses/>.
*/
//---------------------------------------------------------------------------
// Copyright (C) 2000 Dallas Semiconductor Corporation, All Rights Reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL DALLAS SEMICONDUCTOR BE LIABLE FOR ANY CLAIM, DAMAGES
// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// Except as contained in this notice, the name of Dallas Semiconductor
// shall not be used except as stated in the Dallas Semiconductor
// Branding Policy.
//--------------------------------------------------------------------------


#include "OneWireMaster.h"


/**
 * Timing array for overdrive bus speed values
 *
 * This array contains the datasheet recommended timings in MS/uS values
 */
const int OneWireMaster::standardTime[10] = {6, 64, 60, 10, 9, 55, 0, 480, 70, 410};

/**
 * Timing array for standard bus speed values
 *
 * This array contains the datasheet recommended timings in MS/uS values
 */
const int OneWireMaster::overdriveTime[10] = {1.5, 7.5, 7.5, 2.5, 0.75, 7, 2.5, 70, 8.5, 40};


/**
 * OneWireMaster constructor
 * 
 * @param[in] gpio_periph Peripherial address of the GPIO port from sysctl.h
 * @param[in] gpio_port GPIO port from hw_memmap.h
 * @param[in] gpio_pinmask GPIO pin from gpio.h
 * @param[in] bus_speed Bus speed timing table to use. 0 for overdrive, 1 for
 * standard speed
 * 
 * This constructor will test to make sure the supplied values are correct 
 * before attempting to use them. If the supplied values don't work for
 * whatever reason it will throw an error condition.
 */
OneWireMaster::OneWireMaster
	( unsigned int busSpeed
	, unsigned long gpioPeriph
	, unsigned long gpioPort
	, unsigned char gpioPinmask
	)
	: GPIOPin(gpioPeriph, gpioPort, gpioPinmask)
{

	if (busSpeed == 0)
		timing = overdriveTime;
	else
		timing = standardTime;
}
/**
 * Same as two argument constructor, except this will default to using pin 
 * A2 as the OneWire bus.
 */
OneWireMaster::OneWireMaster(unsigned int bus_speed)
	:GPIOPin(SYSCTL_PERIPH_GPIOA, GPIO_PORTF_BASE, GPIO_PIN_1)
{

	if (bus_speed == 0)
		timing = overdriveTime;
	else
		timing = standardTime;
}

/**
 * Wait for specified number of milliseconds
 *
 * Note that the use of SysCtlClockGet() may cause some small clock drift, as
 * that operation takes a finite amount of time to perform. It would be better
 * to use the provided commented out portion if you already know the response
 * value for SysCtlClockGet. 
 *
 * Note that the division by 3 is due to SysCtlDelay using 3 operations for
 * it's wait loop, therefore any clock speed must be divided by 3.
 */
void OneWireMaster::WaitUS(unsigned int ms)
{
	SysCtlDelay((SysCtlClockGet() / 3) / (ms * 1000));
	// Alternative with pre-set clock:
	// SysCtlDelay((CLOCKSPEEDVALUE / 3) / (ms * 1000));
}

/**
 * Reset the OneWire bus for new commands
 *
 * Sends out the RESET signal across the OneWire pin and waits for the response
 * presence detect.
 * 
 * @param[out] presence Returns 1 if presence detect was present, 0 otherwise
 */
int OneWireMaster::Reset(void)
{
	int result = 0;

	

}