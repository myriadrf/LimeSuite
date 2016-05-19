# Lime Suite

https://myriadrf.org/projects/lime-suite/

The Lime Suite application software provides drivers
and SDR application support for the LMS7002M RFIC,
and hardware like the LimeSDR, NovenaRF7, and others.

## Building from source

### Dependencies

Depedencies change based on how much of the suite will be used.
The GUI requires wx development files, the SoapySDR support module
requires SoapySDR development files, USB devices require libusb, etc.

Installing the dependencies on Ubuntu:

```
#packages for soapysdr available at myriadrf PPA
sudo add-apt-repository -y ppa:myriadrf/drivers
sudo apt-get update

#install core dependencies
sudo apt-get install g++ cmake libsqlite3-dev

#install hardware support dependencies
sudo apt-get install libsoapysdr-dev libi2c-dev libusb-1.0-0-dev

#install graphics dependencies
sudo apt-get install libwxgtk3.0-dev freeglut3-dev
```

### Build with cmake

```
git clone https://github.com/myriadrf/LimeSuite.git
cd LimeSuite
mkdir builddir && cd builddir
cmake ../
make -j4
sudo make install
```

## Help and support

* https://discourse.myriadrf.org/

## Additional documentation

This is software to control Lime Microsystems EVB7 and Stream boards.

Detailed instructions how to compile this software is given in:
	docs/lms7suite_compilation_guide.doc

Source code is is grouped into modules by functionality in ./src directory.

The main library is LMS7002M it is designed to configure LMS7002M transceiver and
perform data transfering operations to boards. Basic functionality is documented
in ./docs/lms7api.pdf file.

LTEpackets module is designed to experiment with receiving, processing and 
transmitting samples.

Windows binaries can be found in build/bin/Release directory
	
NOTICE for Linux users:
	To be able to use USB or COM devices, lms7suite might need to be executed with
	administrative privileges. Or the USB and COM device permissions have to be changed
	to allow non-root users to acces them.
