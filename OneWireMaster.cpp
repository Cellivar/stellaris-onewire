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
 * Maximum number of allowed devices during a search. Default is 50
 */
#define OW_MAX_NUM_DEVICES 50


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
	:GPIOPin(SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_7)
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
	if (us <= 0) return;
	SysCtlDelay((us / 3) * (SysCtlClockGet() / 1000000 ));
	// Alternative with pre-set clock:
	// SysCtlDelay((us / 3) * (CLOCKSPEEDVALUE / 1000000 ));
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
	result = GPIOPin.Read();
	result = result == 0 ? 1 : 0;
	//result = GPIOPin.Read() == 0 ? 1 : 0; // See if there is a presence detect
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
	WriteByte(OW_OVERDRIVE_SKIP);	// Run Overdrive Skip command
	timing = overdriveTime;		// Set to ovedrive timings
	return Reset();				// Return result of overdrive presence
}

/**
 * Perform a ROM select operation
 */
void OneWireMaster::MatchROM(BYTE rom[8])
{
	WriteByte(OW_MATCH_ROM);	// Perform Match Rom command

	// Write out the address
	for (int i = 0; i < 8; ++i) WriteByte(rom[i]);
}

/**
 * Perform a ROM skip, for single device use.
 */
void OneWireMaster::SkipROM()
{
	WriteByte(OW_SKIP_ROM);	// Perform Skip ROM command
}

/**
 * Perform a ROM search and populate the Addresses 2D vector with addresses of
 * found devices. The primary vector is a list of addresses, the secondary is
 * the bytes of the address itself. The return value is used to determine if
 * something went wrong during the search process. If this is 0, something bad
 * happened. Anything else indicates the number of returned addresses.
 *
 * This makes use of the seacrh algorithm described by Dallas Semiconductor.
 */
int OneWireMaster::Search(void)
{

	// Set up variables before we start talking to the line, avoiding
	// any weird timing issues
	unsigned char bit, bitComp, buffNum;
	std::vector<unsigned char> device;	// Container for device address
	std::vector<unsigned char> stack;	// Container depth-first search stack

	int safeCount = 0;	// Make sure we don't blow the heap with infinite devices

	while (true)
	{
		device.clear();
		buffNum = 0;

		this->Reset();

		//this->WaitUS(1000);

		this->WriteByte(OW_SEARCH_ROM);	// Run the search ROM command
		this->WaitUS(100);

		for (int i = 0; i < 64; ++i)
		{
			//this->WaitUS(100);

			bit = this->ReadBit();
			bitComp = this->ReadBit();

			if ((bit == 1) && (bitComp == 1))
			{
				// Both values are 1, indicating that something went wrong, we should have
				// gotten at least some signal. We're either down the wrong path, or there's nothing
				// on the line. Either way, hang up.
				return 0; // Something bad happened
			}
			else if (bit != bitComp)
			{
				// Two different values, meaning this single value is the one to take

				this->WriteBit(bit);
				buffNum >>= 1;
				bit <<= 7;
				buffNum |= bit;

			}
			else if ((bit == 0) && (bitComp == 0))
			{
				if (stack.size() == 0)
				{
					// Nothing here yet, start it up
					stack.push_back(i);
					this->WriteBit(0);
					buffNum >>= 1;
				}
				else
				{
					// Two different numbers. See if it's in the stack already
					if (i == stack.back())
					{
						// We've been down these roads, time to choose a new one
						stack.pop_back();
						this->WriteBit(1);
						buffNum >>= 1;
						buffNum |= 0x80;
					}
					else if (i > stack.back())
					{
						// New decision to make, choose 0
						stack.push_back(i);
						this->WriteBit(0);
						buffNum >>= 1;
					}
					else if (i < stack.back())
					{
						// Still going down a road, continue 0
						this->WriteBit(0);
						buffNum >>= 1;
					}
				}
			}

			if ((i % 8) == 7)	// End of a byte, add it to the vector for the device
			{
				device.push_back(buffNum);
				buffNum = 0;
			}

		}

		// Sort into human format
		std::reverse(device.begin(), device.end());

		// Store the new device address in the table
		this->devices.push_back(device);

		++safeCount;

		if (stack.size() <= 0)
			break;

		if (safeCount > OW_MAX_NUM_DEVICES)
		{
			return 0;
		}

	}

	return this->devices.size();
}


// The 1-Wire CRC scheme is described in Maxim Application Note 27:
// "Understanding and Using Cyclic Redundancy Checks with Maxim iButton Products"
//

#if ONEWIRE_CRC8_CALCULATION_METHOD
//
// Compute a Dallas Semiconductor 8 bit CRC. These show up in the ROM
// and the registers.  (note: this might better be done without the
// table, it would probably be smaller and certainly fast enough
// compared to all those delayMicrosecond() calls.  But I got
// confused, so I use this table from the examples.)  
//
BYTE OneWireMaster::CRC8(BYTE* addr, BYTE len)
{
	// This table comes from Dallas sample code where it is freely reusable, 
	// though Copyright (C) 2000 Dallas Semiconductor Corporation
	static BYTE dscrc_table[] = 
	{
		  0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
		157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
		 35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
		190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
		 70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
		219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
		101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
		248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
		140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
		 17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
		175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
		 50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
		202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
		 87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
		233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
		116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53
	};

	BYTE i;
	BYTE crc = 0;
	
	for (i = 0; i < len; i++)
	{
		crc  = dscrc_table[crc ^ addr[i] ];
	}
	
	return crc;
}
#else
//
// Compute a Dallas Semiconductor 8 bit CRC directly. 
//
BYTE OneWireMaster::CRC8(BYTE* addr, BYTE len)
{
	BYTE i, j;
	BYTE crc = 0;
	
	for (i = 0; i < len; i++) 
	{
		BYTE inbyte = addr[i];
		for (j = 0; j < 8; j++) 
		{
			BYTE mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	
	return crc;
}
#endif


//
// Compute a Dallas Semiconductor 16 bit CRC. I have never seen one of
// these, but here it is.
//
unsigned short OneWireMaster::CRC16(unsigned short* data, unsigned short len)
{
	static short oddparity[16] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

	unsigned short i;
	unsigned short crc = 0;
	
	for ( i = 0; i < len; i++) 
	{
		unsigned short cdata = data[len];
	
		cdata = (cdata ^ (crc & 0xff)) & 0xff;
		crc >>= 8;
	
		if (oddparity[cdata & 0xf] ^ oddparity[cdata >> 4]) crc ^= 0xc001;
	
		cdata <<= 6;
		crc ^= cdata;
		cdata <<= 1;
		crc ^= cdata;
	}
	
	return crc;
}


