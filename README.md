stellaris-onewire
=================

Stellaris library for OneWire devices

This library brings OneWire support to Stellaris devices, in the form of an 
easy-to-interface C++ library. 

The controller class is easy to extend into different device handler classes, 
for controlling various OneWire devices. Included in this library are several 
devices that I have worked with, and include enough documentation to help with 
constructing any additional devices that may be required.

If you write a class descriptor for a device not included, please submit a pull 
request with your class! I will add it to the library attributed to you.


Use
================
The namespace of all functionality in this code is "OneWire", i.e.:
<pre>
OneWire::OneWireMaster OWM(OneWire::STANDARD_TIMING);
</pre>
Will create your new OneWire master controller object. From this you can run
<pre>
OWM.Search();
</pre>
and find all the devices on your OneWire Micro-LAN.

Perhaps you don't enjoy doing things manually? Just for you there are several
device classes already handy to implement various types of OneWire devices!
All you have to do to get these running is feed their constructors a Controller
object.

For example:
<pre>
OneWire::OneWireMaster OWM(OneWire::STANDARD_TIMING);
OneWire::DS1822 Thermo(OWM, DEVICE_UNIQUE_ID);
</pre>
will create a handler object for the DS1822 Econo Digital Thermometer device.
From this object you can call
<pre>
int curTemp = Thermo.GetTemp();
</pre>
and be fed the current temperature from the device.

Wait, what's that? You don't know the address of the devices on your network?
No problem, that's what the search function is for. By running the
<pre>
OWM.Search();
</pre>
method you get a vector of devices available at
<pre>
OWM.Devices
</pre>
You can iterate through this list, find the device type and create appropriate
objects for the devices available on the network.

The OneWireDevice superclass is supplied for you to create any other devices
that are not currently included in this library. Create something robust enough
to show off? Send me a pull request with your class and I'll include it with
proper attribution.

List of presupported devices.


Nota Bene:
===============
There is a 50 device limit built into the search function. This limit can be 
modified inside the OneWireMaster.h header file Once 50 devices are
found on the network it will stop searching right then and there. This is to
prevent blowing the stack memory when you have an improperly configured pin, or
your network is not working correctly. Symptoms include all of the devices
showing as 0x800000000000 or something along those lines. This indicates that
your bus is constantly low. No devices being found indicates something is wrong
with the pulldown or the devices are not responding. You're on your own for
those types of problems.

