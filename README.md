This is software to control Lime Microsystems EVB7 and Stream boards.
Detailed instructions how to compile this software is given in:
	docs/lms7suite_compilation_guide.doc

Source code is is grouped into modules by functionality in ./src directory.

The main library is LMS7002M it is designed to configure LMS7002M transceiver and
perform data transfering operations to boards. Basic functionality is documented
in ./docs/lms7api.doc file.

LTEpackets module is designed to experiment with receiving, processing and 
transmitting samples.

Windows binaries can be found in build/bin/Release directory
	
NOTICE for Linux users:
	To be able to use USB or COM devices, lms7suite might need to be executed with
	administrative privileges. Or the USB and COM device permissions have to be changed
	to allow non-root users to acces them.
