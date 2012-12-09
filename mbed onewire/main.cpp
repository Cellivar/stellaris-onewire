/*
* OneWireCRC/OneWireThermometer demo.
*
* Copyright (C) <2009> Petras Saduikis <petras@petras.co.uk>
*
* This file is part of OneWireCRC/OneWireThermometer.
*
* OneWireCRC/OneWireThermometer is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* OneWireCRC/OneWireThermometer is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with OneWireCRC/OneWireThermometer0.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////
// Test code to read temperature from a Maxim DS18B20 or DS18S20
// 1-wire device 
////////////////////////////////////////////////////////////////////

#include <mbed.h>
#include "DS18S20.h"
#include "DS18B20.h"
#include "OneWireDefs.h"

//#define THERMOMETER DS18S20
#define THERMOMETER DS18B20

int main()
{
    // device( crcOn, useAddress, parasitic, mbed pin )
    THERMOMETER device(true, true, false, p25);
    
    while (!device.initialize());    // keep calling until it works
    
    while (true)
    {
        // changing the resolutions only affects the DS18B20. The DS18S20 is fixed.
        device.setResolution(nineBit);
        device.readTemperature(); 
        wait(2);
        device.setResolution(tenBit);
        device.readTemperature(); 
        wait(2);
        device.setResolution(elevenBit);
        device.readTemperature(); 
        wait(2);
        device.setResolution(twelveBit);
        device.readTemperature(); 
        wait(2);
    }
     
    return EXIT_SUCCESS;
}
