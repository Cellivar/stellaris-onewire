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
 * Please note that while overdrive support is present the Stellaris is not
 * capable of generating a precise enough clock (sub-uS range) to garuntee
 * reliable operation. YMMV.
 */
OneWireMaster::OneWireMaster
	( unsigned int busSpeed
	, unsigned long gpioPeriph
	, unsigned long gpioPort
	, unsigned char gpioPinmask
	)
	: GPIOPin(gpioPeriph, gpioPort, gpioPinmask)
{
	// Set the pin to a 4mA open-drain weak pull up, per 1-wire spec.
	this->GPIOPin.PullMode(GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_OD_WPU);

	if (busSpeed == 0)
		timing = overdriveTime;
	else
		timing = standardTime;
}

/**
 * Same as two argument constructor, except this will default to using pin 
 * A2 as the OneWire bus.
 *
 * @param[in] busSpeed Bus speed timing table to use. 0 for overdrive, 1 for
 * standard speed.
 * 
 * Please note that while overdrive support is present the Stellaris is not
 * capable of generating a precise enough clock (sub-uS range) to garuntee
 * reliable operation. YMMV.
 */
OneWireMaster::OneWireMaster(unsigned int busSpeed)
	:GPIOPin(SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_2)
{
	// Set the pin to a 4mA open-drain weak pull up, per 1-wire spec.
	this->GPIOPin.PullMode(GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_OD_WPU);

	if (busSpeed == 0)
		timing = overdriveTime;
	else
		timing = standardTime;
}

/**
 * Wait for specified number of microeconds
 *
 * Note that the use of SysCtlClockGet() may cause some small clock drift, as
 * that operation takes a finite amount of time to perform. It would be better
 * to use the provided commented out portion if you already know the response
 * value for SysCtlClockGet. 
 *
 * Note that the division by 3 is due to SysCtlDelay using 3 operations for
 * it's wait loop, therefore any clock speed must be divided by 3.
 */
void OneWireMaster::WaitUS(unsigned int us)
{
	SysCtlDelay((SysCtlClockGet() / 3) / (us * 1000000));
	// Alternative with pre-set clock:
	// SysCtlDelay((CLOCKSPEEDVALUE / 3) / (us * 1000000));
}

/**
 * Reset the OneWire bus for new commands. Based off of example code from
 * Dallas Semiconductor.
 *
 * Sends out the RESET signal across the OneWire pin and waits for the response
 * presence detect.
 * 
 * @param[out] presence Returns 1 if presence detect was present, 0 otherwise
 */
int OneWireMaster::Reset(void)
{
	int result = 0;

	WaitUS(timing[6]);	
	GPIOPin.Output();
	GPIOPin.Write(0);	// Bring bus low for reset
	WaitUS(timing[7]);	// Wait for reset duration
	GPIOPin.Input();	// Bring bus to input mode
	WaitUS(timing[8]);	// Wait for presence detect
	result = !(GPIOPin.Read() & 0x01);	// See if there is a presence detect
	WaitUS(timing[9]);	// Finish out presence detect, ready for commands

	return result;
}

/**
 * Write a bit to the line.
 */
void OneWireMaster::WriteBit(int bit)
{
	bit = bit & 0x01; // Make sure we don't have something silly here

	if (bit)	// '1' bit
	{
		GPIOPin.Output();	// Make sure we're in output
		GPIOPin.Write(0);	// Bring bus low for reset
		WaitUS(timing[0]);	// Wait for spec duration
		GPIOPin.Input();	// Release for pullup
		WaitUS(timing[1]);	// Wait for recovery time
	}
	else	// '0' bit
	{
		GPIOPin.Output();	// Set to output
		GPIOPin.Write(0);	// Pull line low
		WaitUS(timing[2]);	// Wait for spec duration
		GPIOPin.Input();	// Release for pullup
		WaitUS(timing[3]);	// Wait for recovery time
	}
}

/**
 * Read a bit from the line
 */
int OneWireMaster::ReadBit(void)
{
	BYTE result;

	GPIOPin.Output();	// Set to output
	GPIOPin.Write(0);	// Pull line low
	WaitUS(timing[0]);	// Wait for control
	GPIOPin.Input();	// Release for pullup
	WaitUS(timing[4]);	// Wait for signal from slaves
	result = GPIOPin.Read() & 0x01;	// Read the value on the line
	WaitUS(timing[5]);	// Wait for bus to finish operation

	return result;
}

/**
 * Write a byte out to the line. This is a batch operation of WriteBit.
 */
void OneWireMaster::WriteByte(int data)
{
	// Loop through the byte, least significant bit first.
	for (int i = 0; i < 8; ++i)
	{
		WriteBit(data & 0x01);

		// Shift byte for next bit
		data >>= 1;
	}
}

/**
 * Read a byte from the line. This is a batch operation of ReadBit.
 */
int OneWireMaster::ReadByte(void)
{
	int result = 0;

	for (int i = 0; i < 8; ++i)
	{
		// Shift result
		result >>= 1;

		// If result is one, set MS bit
		if (ReadBit()) result |= 0x80;
	}

	return result;
}

/**
 * I've never seen this actually used, but here you go. Write a OneWire data
 * byte and return the sampled result.
 */
int OneWireMaster::TouchByte(int data)
{
	int result = 0;
	
	for (int loop = 0; loop < 8; loop++)
	{
		// shift the result to get it ready for the next bit
		result >>= 1;
		
		// If sending a '1' then read a bit else write a '0'
		if (data & 0x01)
		{
			if (ReadBit()) result |= 0x80;
		}
		else WriteBit(0);
		
		// shift the data byte for the next bit
		data >>= 1;
	}
	
	return result;
}

/**
 * Not seen this one used either, but it is for writing a block of OneWire data
 * byes and returning the sampled results in the same buffer.
 */
void OneWireMaster::Block(BYTE* data, int data_len)
{
	for (int loop = 0; loop < data_len; loop++)
	{
		data[loop] = TouchByte(data[loop]);
	}
}

/**
 * Set timing to overdrive, and perform overdrive skip operation. Returns 0 if
 * no devices are found on a standard speed reset. Returns result of an
 * overdrive presence detect, if any OD devices are detected it will be 1.
 *
 * Note that the timing accuracy of lower clock speeds can result in inaccurate
 * timing pulses for the clock. 50Mhz will usually result in accurate values
 * however the clock drift of individual operations can throw off the timing.
 * If you find overdrive does not work, fall back onto standard timings.
 */
int OneWireMaster::SkipOverdrive()
{
	timing = standardTime;		// Make sure we're on standard timings
	if (!Reset()) return 0;		// If nothing shows up, fail out
	WriteByte(OVERDRIVE_SKIP);	// Run Overdrive Skip command
	timing = overdriveTime;		// Set to ovedrive timings
	return Reset();				// Return result of overdrive presence
}

/**
 * Perform a ROM select operation
 */
void OneWireMaster::MatchROM(BYTE rom[8])
{
	WriteByte(MATCH_ROM);	// Perform Match Rom command

	// Write out the address
	for (int i = 0; i < 8; ++i) WriteByte(rom[i]);
}

/**
 * Perform a ROM skip, for single device use.
 */
void OneWireMaster::SkipROM()
{
	WriteBytes(SKIP_ROM);	// Perform Skip ROM command
}

/**
 * Perform a ROM search. This function will return the ROM address of
 * the last found device. To begin a new search, call this function with
 * no arguments. If a ROM address is provided as an argument, then it will
 * attempt to find a new device with a different ROM address. If no new devices
 * are found then the function will return 0.
 *
 * This makes use of the seacrh algorithm described by Dallas Semiconductor.
 */
unsigned long OneWireMaster::Search(unsigned long

	// Send a reset
	// Write the SEARCH_ROM command

	// Read bit A -> Address
	// Read bit B -> Inverse of address
	// Check if A == B
	// Yes -> Devices on the network with both 1 and 0 here.
	//		Choose a direction based on previous directions
	// No -> A is the finalized bit