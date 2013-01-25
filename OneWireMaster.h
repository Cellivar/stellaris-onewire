/**
 * @file OneWireMaster.h
 * 
 * OneWireMaster class prototype. Designed to act as a master controller
 * of a OneWire bus off of a Stellaris Launchpad.
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

#ifndef STELLARIS_ONEWIRE_LIBRARY_CHAPMAN_H
#define STELLARIS_ONEWIRE_LIBRARY_CHAPMAN_H


#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "utils/ustdlib.h"

#include "stellaris-pins/DigitalIOPin.h"
#include <vector>
#include <algorithm>

// OneWire bus speed settings
#define OW_SPEED_OVERDRIVE	0
#define OW_SPEED_STANDARD	1

// Standard One Wire command codes
// Used on most OneWire Devices, read the datasheet for more information
#define OW_SEARCH_ROM		0xF0
#define OW_READ_ROM			0x33
#define OW_MATCH_ROM		0x55
#define OW_ALARM_SEARCH		0xEC
#define OW_SKIP_ROM			0xCC

#define OW_OVERDRIVE_SKIP	0x3C


//Maximum number of allowed devices during a search. Default is 50
#define OW_MAX_NUM_DEVICES 50


// The Dallas Semiconductor example code for OneWire CRC checking provides two
// methods for computing the CRC of a line of data. This define allows you to
// select which one you would prefer for your code base. Method 0 uses math to
// compute the value, leading to slightly smaller code size but slightly longer
// computation time. Method 1 uses a lookup table, with slightly larger code
// size but smaller computation time. Set the method you'd like to use in this
// define here.
#ifndef ONEWIRE_CRC8_CALCULATION_METHOD
#define ONEWIRE_CRC8_CALCULATION_METHOD	1
#endif // ONEWIRE_CRC8_CALCULATION_METHOD

typedef unsigned char BYTE;


/**
 * OneWire Master generic operations
 */
class OneWireMaster
{
public:
	OneWireMaster(unsigned int busSpeed);
	OneWireMaster
		( unsigned int busSpeed
		, unsigned long gpioPeriph
		, unsigned long gpioPort
		, unsigned char gpioPinmask
		);

	// Standard bus functions
	int Reset(void);
	int ReadByte(void);
	void WriteByte(int data);
	int TouchByte(int data);
	void Block(BYTE* data, int data_len);
	
	// Wait timer
	void WaitUS(unsigned int us);

	// Address search/select functions
	int Search(void);
	void MatchROM(BYTE rom[8]);
	void SkipROM(void);
	int SkipOverdrive(void);

	// CRC check functions
	static BYTE CRC8(BYTE* address, BYTE length);
	static unsigned short CRC16(unsigned short* data, unsigned short length);

	// Container for device addresses
	std::vector<std::vector<BYTE> > devices;
private:
	// Timing values array, populated based on the bus speed setting
	const int* timing;

	// GPIO port
	DigitalIOPin GPIOPin;

	// read/write single bits to the bus
	void WriteBit(int bit);
	int ReadBit(void);

	// Timing functions/constants
	static const int standardTime[10];
	static const int overdriveTime[10];

};

#endif // STELLARIS_ONEWIRE_LIBRARY_CHAPMAN_H
