/**
 * @file OneWire.h
 * 
 * OneWire class. Based off of Petras Saduikis' mbed port of Jim Studt's
 * Arduino OneWire library.
 *
 * Copyright (C) <2012> Cliff Chapman <chapman.cliff@gmail.com>
 *
 * This file is part of the Stellaris OneWire Base Library.
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

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"		// For GPIO pin assignment macros
#include "driverlib/sysctl.h"

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

/**
 * OneWire Master generic operations
 */
class OneWireMaster
{
public:
	OneWireMaster(unsigned long gpio_pin, unsigned int bus_speed);

	// Standard bus functions
	int reset();
	int writeByte(int data);
	int readByte(void);

	// CRC check functions

private:


};