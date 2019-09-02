#include "limeRFE_constants.h"
#include "INI.h"

/*********************************************************************************************
* USB Communication
**********************************************************************************************/

int fd = 0;

int my_read(int fd, char* buffer, int count) {
	int result;
#ifdef __unix__
	result = read(fd, buffer, count);
#else
	result = readFromSerial(fd, buffer, count);
#endif // LINUX
	return result;
}

int my_write(int fd, char* buffer, int count) {
	int result;
#ifdef __unix__
	result = write(fd, buffer, count);
#else
	result = writeToSerial(fd, buffer, count);
#endif // LINUX
	return result;
}

int my_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout) {
	int result;
#ifdef __unix__
	result = select(nfds, readfds, writefds, exceptfds, timeout);
#else
	result = selectSerial(nfds, readfds, writefds, exceptfds, timeout);
#endif // LINUX
	return result;
}

int my_open(char* portname, int opt) {
	int result;
#ifdef __unix__
	result = open(portname, opt);
#else
	result = openSerial(portname, opt);
#endif // LINUX
	return result;
}

int my_close(int fd) {
	int result;
#ifdef __unix__
	result = close(fd);
#else
	result = closeSerial(fd);
#endif // LINUX
	return result;
}

int serialport_write(int fd, const char* str, int len)
{
	int n;
	char* cstr = (char*)str;
	my_write(fd, cstr, len);
	return len;
	for (n = 0; n<len; n++)
		my_write(fd, &cstr[n], 1);
	return len;
}


int serialport_read(int fd, char* buff, int len)
{
	int n = my_read(fd, buff, len);
	return n;
}


// takes the string name of the serial port (e.g. "/dev/tty.usbserial","COM1")
// and a baud rate (bps) and connects to that port at that speed and 8N1.
// opens the port in fully raw mode so you can send binary data.
// returns valid fd, or -1 on error
int serialport_init(const char* serialport, int baud)
{

	struct termios toptions;
	int fd;

	char* cserialport = (char*)serialport;

	fd = my_open(cserialport, O_RDWR | O_NOCTTY);
	if (fd == -1) {
		perror("init_serialport: Unable to open port ");
		return -1;
	}

	if (tcgetattr(fd, &toptions) < 0) {
		perror("init_serialport: Couldn't get term attributes");
		return -1;
	}

	speed_t brate = baud; // let you override switch below if needed
	switch (baud) {
	case 4800:   brate = B4800;   break;
	case 9600:   brate = B9600;   break;
		// if you want these speeds, uncomment these and set #defines if Linux
		//#ifndef OSNAME_LINUX
		//    case 14400:  brate=B14400;  break;
		//#endif
	case 19200:  brate = B19200;  break;
		//#ifndef OSNAME_LINUX
		//    case 28800:  brate=B28800;  break;
		//#endif
		//case 28800:  brate=B28800;  break;
	case 38400:  brate = B38400;  break;
	case 57600:  brate = B57600;  break;
	case 115200: brate = B115200; break;
	}
	cfsetispeed(&toptions, brate);
	cfsetospeed(&toptions, brate);

#ifdef _MSC_VER
	//If DTR is not disabled, each time the port is opened, the Arduino uP is reset
	//If DTR is disabled, then uP is reset only the first time the port is opened after plugging USB
	setDTR(DTR_CONTROL_DISABLE);
#endif // Windows

	// 8N1
	toptions.c_cflag &= ~PARENB;
	toptions.c_cflag &= ~CSTOPB;
	toptions.c_cflag &= ~CSIZE;
	toptions.c_cflag |= CS8;
	// no flow control
	toptions.c_cflag &= ~CRTSCTS;
	toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
	toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl
	toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
	toptions.c_oflag &= ~OPOST; // make raw
								// see: http://unixwiz.net/techtips/termios-vmin-vtime.html
	toptions.c_cc[VMIN] = 0;
	toptions.c_cc[VTIME] = 20;
	if (tcsetattr(fd, TCSANOW, &toptions) < 0) {
		perror("init_serialport: Couldn't set term attributes");
		return -1;
	}
	return fd;
}

void serialport_close(int fd) {
	my_close(fd);
}

int write_buffer(lms_device_t *dev, int fd, unsigned char* data, int size) {
	int result = 0;
	if (dev != NULL) {
		result = i2c_write_buffer(dev, data, size);
	}
	else {
		result = write_buffer_fd(fd, data, size);
	}
	return result;
}

int write_buffer_fd(int fd, unsigned char* c, int size)
{
	int actual_length;

	actual_length = serialport_write(fd, (char*)c, size);
	if (actual_length != size) {
//		fprintf(stderr, "Error while sending char\n");
		return -1;
	}
	return 0;
}

int read_buffer(lms_device_t * dev, int fd, unsigned char * data, int size)
{
	int len;
	if (dev != NULL) {
		len = i2c_read_buffer(dev, data, size);
	}
	else {
		len = read_buffer_fd(fd, data, size);
	}
	return len;
}

int read_buffer_fd(int fd, unsigned char * data, int size)
{
	memset(data, 0, size);
	int len;

	len = serialport_read(fd, (char*)data, size);
	return len;
}


//******* Command Definitions *******
int Cmd_GetInfo(lms_device_t *dev, int fd, boardInfo* info) {
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_GET_INFO;
	if (write_buffer(dev, fd, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, fd, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	info->fw_ver = buf[1];     // FW_VER
	info->hw_ver = buf[2];     // HW_VER
	info->status1 = buf[3];    // Status
	info->status2 = buf[4];    // Status

	return RFE_SUCCESS;
}

int ReadConfig(const char *filename, rfe_boardState *stateBoard, guiState *stateGUI) {
	typedef INI<string, string, string> ini_t;
	ini_t parser(filename, true);

	if (parser.select("LimeRFE_Board_Settings") == false)
		return RFE_ERROR_CONF_FILE;

	stateBoard->channelIDRX = parser.get("channelIDRX", 0);
	stateBoard->channelIDTX = parser.get("channelIDTX", 0);
	stateBoard->selPortRX = parser.get("selPortRX", 0);
	stateBoard->selPortTX = parser.get("selPortTX", 0);
	stateBoard->notchOnOff = parser.get("notchOnOff", 0);
	stateBoard->mode = parser.get("mode", 0);
	stateBoard->attValue = parser.get("attValue", 0);
	stateBoard->enableSWR = parser.get("enableSWR", 0);
	stateBoard->sourceSWR = parser.get("sourceSWR", 0);

	if (parser.select("LimeRFE_GUI_Settings")) {
		stateGUI->powerCellCorr = parser.get("CellularPowerCorrection", 0);
		stateGUI->powerCorr = parser.get("PowerCorrection", 0);
		stateGUI->rlCorr = parser.get("GammaCorrection", 0);
	}

	return RFE_SUCCESS;
}

int SaveConfig(const char *filename, rfe_boardState state, guiState stateGUI) {
	FILE *fout;
	fout = fopen(filename, "w");

	if (fout == NULL) {
		fclose(fout);
		return 1;
	}

	fprintf(fout, "[LimeRFE_Board_Settings]\n");

	fprintf(fout, "channelIDRX=%d\n", state.channelIDRX);
	fprintf(fout, "channelIDTX=%d\n", state.channelIDTX);
	fprintf(fout, "selPortRX=%d\n", state.selPortRX);
	fprintf(fout, "selPortTX=%d\n", state.selPortTX);
	fprintf(fout, "mode=%d\n", state.mode);
	fprintf(fout, "notchOnOff=%d\n", state.notchOnOff);
	fprintf(fout, "attValue=%d\n", state.attValue);
	fprintf(fout, "enableSWR=%d\n", state.enableSWR);
	fprintf(fout, "sourceSWR=%d\n", state.sourceSWR);

	fprintf(fout, "[LimeRFE_GUI_Settings]\n");

	fprintf(fout, "CellularPowerCorrection=%f\n", stateGUI.powerCellCorr);
	fprintf(fout, "PowerCorrection=%f\n", stateGUI.powerCorr);
	fprintf(fout, "GammaCorrection=%f\n", stateGUI.rlCorr);

	fclose(fout);
	return 0;
}

// This function has not yet been implemented
int Cmd_GetConfigFull(lms_device_t *dev, int fd, rfe_boardState *state) {
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_GET_CONFIG_FULL;

	if (write_buffer(dev, fd, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, fd, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	return 0;
}

int Cmd_GetConfig(lms_device_t *dev, int fd, rfe_boardState *state) {
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_GET_CONFIG;

	if (write_buffer(dev, fd, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, fd, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	state->channelIDRX = buf[1];
	state->channelIDTX = buf[2];

	state->selPortRX = buf[3];
	state->selPortTX = buf[4];

	state->mode = buf[5];

	state->notchOnOff = buf[6];
	state->attValue = buf[7];

	state->enableSWR = buf[8];
	state->sourceSWR = buf[9];

	return 0;
}

void mySleep(int sleepms)
{
#ifdef __unix__
	usleep(sleepms * 1000);   // usleep takes sleep time in us (1 millionth of a second)
#endif
#ifdef _WIN32
	Sleep(sleepms);
#endif
}

int Cmd_Hello(int fd) {
	int result = 0;
	unsigned char buf[1];
	int len;
	//	memset(buf, 0, 1);

	buf[0] = RFE_CMD_HELLO;

	int attempts = 0;
	bool connected = false;

	while (!connected && (attempts < RFE_MAX_HELLO_ATTEMPTS)) {
		write_buffer_fd(fd, buf, 1);
		mySleep(200);
		len = read_buffer_fd(fd, buf, 1);
		if ((len == 1) && (buf[0] == RFE_CMD_HELLO))
			connected = true;
		attempts++;
	}

	result = (connected) ? 0 : RFE_ERROR_COMM;
	return result;
}

int Cmd_LoadConfig(lms_device_t *dev, int fd, const char *filename) {
	int result = 0;
	rfe_boardState state;
	guiState stateGUI;
	result = ReadConfig(filename, &state, &stateGUI);
	if (result != 0)
		return result;

	result = Cmd_Configure(dev, fd, state.channelIDRX, state.channelIDTX, state.selPortRX, state.selPortTX, state.mode, state.notchOnOff, state.attValue, state.enableSWR, state.sourceSWR);

	return result;
}

int Cmd_Reset(lms_device_t *dev, int fd) {
	int result = 0;
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_RESET;

	if (write_buffer(dev, fd, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, fd, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	return result;
}

int Cmd_ConfigureFull(lms_device_t *dev, int fd, int channelID, int mode, int selPortTX, int notch, int attenuation) {
	int result = 0;

	unsigned char buf[RFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_CONFIG_FULL;

	getDefaultConfiguration(channelID, &buf[1]);
	getDefaultMode(channelID, mode, &buf[13]);

	int notchBit;
	if (notch == RFE_NOTCH_OFF)
		notchBit = RFE_NOTCH_BIT_OFF;
	if (notch == RFE_NOTCH_ON)
		notchBit = RFE_NOTCH_BIT_ON;

	(notchBit == 1) ? buf[RFE_NOTCH_BYTE] |= (1 << RFE_NOTCH_BIT) : buf[RFE_NOTCH_BYTE] &= ~(1 << RFE_NOTCH_BIT);

	unsigned char usatt = attenuation;

	for (int i = 0; i<3; i++)
		((usatt >> i) & 0x01) ? buf[RFE_ATTEN_BYTE] |= (1 << (RFE_ATTEN_BIT + i)) : buf[RFE_ATTEN_BYTE] &= ~(1 << (RFE_ATTEN_BIT + i));

	(selPortTX == 1) ? buf[RFE_PORTTX_BYTE] |= (1 << RFE_PORTTX_BIT) : buf[RFE_PORTTX_BYTE] &= ~(1 << RFE_PORTTX_BIT);

	if (write_buffer(dev, fd, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, fd, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	return result;
}

int Cmd_Configure(lms_device_t *dev, int fd, int channelIDRX, int channelIDTX, int selPortRX, int selPortTX, int mode, int notch, int attenuation, int enableSWR, int sourceSWR) {

	int result = 0;

	if (channelIDTX == -1)
		channelIDTX = channelIDRX;

	unsigned char buf[RFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_CONFIG;

	buf[1] = channelIDRX;
	buf[2] = channelIDTX;

	buf[3] = selPortRX;
	buf[4] = selPortTX;

	buf[5] = mode;

	buf[6] = notch;

	buf[7] = attenuation;

	buf[8] = enableSWR;
	buf[9] = sourceSWR;

	if(write_buffer(dev, fd, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, fd, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	result = buf[1]; // buf[0] is the command, buf[1] is the result
	return result;
}

void getDefaultConfigurations(unsigned char** bytes) {
#define TEST 2
	//Warning: Not Implemented!!!
	unsigned char configurations[RFE_CID_COUNT][12] = {
		{ 0x18, 0x11, 0x00, 0x70, 0x00, 0x00, 0x10, 0x20, 0x14, 0x00, 0x28, 0x40 },
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //EMPTY TEST!!!
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //EMPTY TEST!!!
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //EMPTY TEST!!!
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //EMPTY TEST!!!
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //EMPTY TEST!!!
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //EMPTY TEST!!!
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //EMPTY TEST!!!
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //EMPTY TEST!!!
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //EMPTY TEST!!!
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //EMPTY TEST!!!
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }  //EMPTY TEST!!!
	};

	memcpy(bytes, configurations, RFE_CID_COUNT * 12 * sizeof(unsigned char));
}

void getDefaultConfiguration(int channelID, unsigned char* bytes) {
	switch (channelID) {
	case RFE_CID_WB_1000: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 18 11 00 70 00 00 10 21 14 00 38 60
		bytes[0] = 0x18; bytes[1] = 0x11; bytes[2] = 0x00; bytes[3] = 0x70; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x21; bytes[8] = 0x14; bytes[9] = 0x00; bytes[10] = 0x38; bytes[11] = 0x60;
	} break;
	case RFE_CID_WB_4000: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 10 42 22 00 00 10 00 00 64 3A 00
		bytes[0] = 0x00; bytes[1] = 0x10; bytes[2] = 0x42; bytes[3] = 0x22; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x64; bytes[10] = 0x3A; bytes[11] = 0x00;
	} break;
	case RFE_CID_HAM_0030: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 81 50 00 00 00 00 10 09 18 00 58 60
		bytes[0] = 0x81; bytes[1] = 0x50; bytes[2] = 0x00; bytes[3] = 0x00; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x09; bytes[8] = 0x18; bytes[9] = 0x00; bytes[10] = 0x58; bytes[11] = 0x60;
	} break;
	case RFE_CID_HAM_0145: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 42 14 00 30 00 00 10 11 12 00 38 60
		bytes[0] = 0x42; bytes[1] = 0x14; bytes[2] = 0x00; bytes[3] = 0x30; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x11; bytes[8] = 0x12; bytes[9] = 0x00; bytes[10] = 0x38; bytes[11] = 0x60;
	} break;
	case RFE_CID_HAM_0435: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 24 12 00 50 00 00 10 05 11 00 38 60
		bytes[0] = 0x24; bytes[1] = 0x12; bytes[2] = 0x00; bytes[3] = 0x50; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x05; bytes[8] = 0x11; bytes[9] = 0x00; bytes[10] = 0x38; bytes[11] = 0x60;
	} break;
	case RFE_CID_HAM_1280: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 10 48 11 00 00 10 00 00 44 38 00
		bytes[0] = 0x00; bytes[1] = 0x10; bytes[2] = 0x48; bytes[3] = 0x11; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x44; bytes[10] = 0x38; bytes[11] = 0x00;
	} break;
	case RFE_CID_HAM_2400: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 10 41 43 00 00 10 00 00 24 3E 00
		bytes[0] = 0x00; bytes[1] = 0x10; bytes[2] = 0x41; bytes[3] = 0x43; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x24; bytes[10] = 0x3E; bytes[11] = 0x00;
	} break;
	case RFE_CID_HAM_3500: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 10 44 00 00 00 10 00 00 04 3C 00
		bytes[0] = 0x00; bytes[1] = 0x10; bytes[2] = 0x44; bytes[3] = 0x00; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x04; bytes[10] = 0x3C; bytes[11] = 0x00;
	} break;
	case RFE_CID_CELL_BAND01: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 00 00 00 52 35 86 00 00 00 11 18
		bytes[0] = 0x00; bytes[1] = 0x00; bytes[2] = 0x00; bytes[3] = 0x00; bytes[4] = 0x52; bytes[5] = 0x35; bytes[6] = 0x86; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x00; bytes[10] = 0x11; bytes[11] = 0x18;
	} break;
	case RFE_CID_CELL_BAND02: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 00 00 00 60 63 46 00 00 00 11 18
		bytes[0] = 0x00; bytes[1] = 0x00; bytes[2] = 0x00; bytes[3] = 0x00; bytes[4] = 0x60; bytes[5] = 0x63; bytes[6] = 0x46; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x00; bytes[10] = 0x11; bytes[11] = 0x18;
	} break;
	case RFE_CID_CELL_BAND03: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 00 00 00 44 21 06 00 00 00 11 18
		bytes[0] = 0x00; bytes[1] = 0x00; bytes[2] = 0x00; bytes[3] = 0x00; bytes[4] = 0x44; bytes[5] = 0x21; bytes[6] = 0x06; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x00; bytes[10] = 0x11; bytes[11] = 0x18;
	} break;
	case RFE_CID_CELL_BAND07: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 00 00 00 01 02 65 00 00 00 11 18
		bytes[0] = 0x00; bytes[1] = 0x00; bytes[2] = 0x00; bytes[3] = 0x00; bytes[4] = 0x01; bytes[5] = 0x02; bytes[6] = 0x65; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x00; bytes[10] = 0x11; bytes[11] = 0x18;
	} break;
	case RFE_CID_CELL_BAND38: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 00 00 00 11 90 25 00 00 00 11 18
		bytes[0] = 0x00; bytes[1] = 0x00; bytes[2] = 0x00; bytes[3] = 0x00; bytes[4] = 0x11; bytes[5] = 0x90; bytes[6] = 0x25; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x00; bytes[10] = 0x11; bytes[11] = 0x18;
	} break;
	}
}

int Cmd_ModeFull(lms_device_t *dev, int fd, int channelID, int mode) {
	int result = 0;

	unsigned char buf[RFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_MODE_FULL;

	int moderes = getDefaultMode(channelID, mode, &buf[1]);
	if (moderes != 0)
		return moderes;

	if (write_buffer(dev, fd, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, fd, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	return result;
}

int Cmd_Mode(lms_device_t *dev, int fd, int mode) {
	int result = 0;

	unsigned char buf[RFE_BUFFER_SIZE_MODE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE_MODE);

	buf[0] = RFE_CMD_MODE;

	buf[1] = mode;

	if(write_buffer(dev, fd, buf, RFE_BUFFER_SIZE_MODE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, fd, buf, RFE_BUFFER_SIZE_MODE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	result = buf[1]; // buf[0] is the command, buf[1] is the result
	return result;
}

int getDefaultMode(int channelID, int mode, unsigned char* mcu_byte) {
	int result = 0;

	int iTXRX0 = 0; //V2
	int iTXRX1 = 0; //V3
	int iLNA_EN = 0;
	int iPA_EN = 0;
	int iRELAY = 0; //0 - RX; 1 - TX

	// For Cellular channels the mode cannot be changed
	if ((channelID == RFE_CID_CELL_BAND01) ||
		(channelID == RFE_CID_CELL_BAND02) ||
		(channelID == RFE_CID_CELL_BAND03) ||
		(channelID == RFE_CID_CELL_BAND07) ||
		(channelID == RFE_CID_CELL_BAND38)) {
		iTXRX0 = 0;
		iTXRX1 = 1;
		//		return(1);
	}
	else {
		if (mode == RFE_MODE_NONE) {
			iLNA_EN = 0;
			iPA_EN = 0;
		}

		if (mode == RFE_MODE_TX) {
			iLNA_EN = 0;
			iPA_EN = 1;
			if (channelID == RFE_CID_HAM_0030) {
				//				iRELAY = 0;
				iRELAY = 1;
			}
			else {
				iTXRX0 = 1;
				iTXRX1 = 1;
			}
		}

		// Here it is necessary that the firmware forbids the potentially dangerous combinations!!!!

		// Here, we presume that it is not possible in GUI to select TXRX unless TX is routed to TX connector (not on TX/RX connector)
		if ((mode == RFE_MODE_RX) ||
			(mode == RFE_MODE_TXRX)) {
			if (channelID == RFE_CID_HAM_0030) {
				//				iRELAY = 1;
				iRELAY = 0;
			}
			else if ((channelID == RFE_CID_HAM_0145) || //RX_L
				(channelID == RFE_CID_HAM_0435) ||
				(channelID == RFE_CID_WB_1000)) {
				iTXRX0 = 1;
				iTXRX1 = 0;
			}
			else { //RX_H
				iTXRX0 = 0;
				iTXRX1 = 0;
			}
			if (mode == RFE_MODE_RX) {
				iLNA_EN = 1;
				iPA_EN = 0;
			}
			if (mode == RFE_MODE_TXRX) {
				iLNA_EN = 1;
				iPA_EN = 1;
			}
		}
	}

	(iTXRX0 == 1) ? *mcu_byte |= (1 << (RFE_MCU_BYTE_TXRX0_BIT)) : *mcu_byte &= ~(1 << (RFE_MCU_BYTE_TXRX0_BIT));
	(iTXRX1 == 1) ? *mcu_byte |= (1 << (RFE_MCU_BYTE_TXRX1_BIT)) : *mcu_byte &= ~(1 << (RFE_MCU_BYTE_TXRX1_BIT));
	(iRELAY == 1) ? *mcu_byte |= (1 << (RFE_MCU_BYTE_RELAY_BIT)) : *mcu_byte &= ~(1 << (RFE_MCU_BYTE_RELAY_BIT));
	(iLNA_EN == 1) ? *mcu_byte |= (1 << (RFE_MCU_BYTE_LNA_EN_BIT)) : *mcu_byte &= ~(1 << (RFE_MCU_BYTE_LNA_EN_BIT));
	(iPA_EN == 1) ? *mcu_byte |= (1 << (RFE_MCU_BYTE_PA_EN_BIT)) : *mcu_byte &= ~(1 << (RFE_MCU_BYTE_PA_EN_BIT));

	return result;
}

int Cmd_ReadADC(lms_device_t *dev, int fd, int adcID, int* value) {
	int result = RFE_SUCCESS;
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, RFE_BUFFER_SIZE);

	if (adcID == RFE_ADC1)
		buf[0] = RFE_CMD_READ_ADC1;
	else
		buf[0] = RFE_CMD_READ_ADC2;

	if(write_buffer(dev, fd, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, fd, buf, RFE_BUFFER_SIZE);
	if (len == -1) {
		*value = 0;
		return(RFE_ERROR_COMM);
	}

	*value = buf[2] * pow(2, 8) + buf[1];

	return result;
}

int Cmd_Cmd(lms_device_t *dev, int fd, unsigned char* buf) {
	int result = 0;
	int len;

	if (write_buffer(dev, fd, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, fd, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	return result;
}

int Cmd_ConfGPIO(lms_device_t *dev, int fd, int gpioNum, int direction) {
	if ((gpioNum != RFE_GPIO4) & (gpioNum != RFE_GPIO5))
		return RFE_ERROR_GPIO_PIN;

	int result = 0;
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;
	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_CONFGPIO45;
	buf[1] = gpioNum;
	buf[2] = direction;

	if (write_buffer(dev, fd, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, fd, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	return result;
}

int Cmd_SetGPIO(lms_device_t *dev, int fd, int gpioNum, int val) {
	if ((gpioNum != RFE_GPIO4) & (gpioNum != RFE_GPIO5))
		return RFE_ERROR_GPIO_PIN;

	int result = 0;
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;
	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_SETGPIO45;
	buf[1] = gpioNum;
	buf[2] = val;

	if (write_buffer(dev, fd, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, fd, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	return result;
}

int Cmd_GetGPIO(lms_device_t *dev, int fd, int gpioNum, int * val) {
	if ((gpioNum != RFE_GPIO4) & (gpioNum != RFE_GPIO5))
		return RFE_ERROR_GPIO_PIN;

	int result = 0;
	unsigned char buf[RFE_BUFFER_SIZE];
	int len;
	memset(buf, 0, RFE_BUFFER_SIZE);

	buf[0] = RFE_CMD_GETGPIO45;
	buf[1] = gpioNum;

	if (write_buffer(dev, fd, buf, RFE_BUFFER_SIZE) != 0)
		return RFE_ERROR_COMM;
	len = read_buffer(dev, fd, buf, RFE_BUFFER_SIZE);
	if (len == -1)
		return(RFE_ERROR_COMM);

	*val = buf[1];

	return result;
}

/******************************************************************************
* I2C Communications
*******************************************************************************/

void mySleep(double sleepms)
{
#ifdef __unix__
	usleep(sleepms * 1000);   // usleep takes sleep time in us (1 millionth of a second)
#endif
#ifdef _WIN32
	Sleep(sleepms);
#endif
}

void i2c_dly(void)
{
	double sleepms = 0.5 * 1 / RFE_I2C_FSCL * 1000; //for example 0.5 * period - Check this!!!
	mySleep(sleepms);
}

int i2c_setVal(lms_device_t* lms, int bitGPIO, int value) {
	uint8_t gpio_dir = 0;
	int direction;
	if (LMS_GPIODirRead(lms, &gpio_dir, 1) != 0)
		return -1;
	// Direction: 1-output, 0-input
	// If writting 0, then output => direction = 1
	// If writting 1, then input (high Z, and pull-up will do the trick) => direction = 0
	(value == 1) ? direction = 0 : direction = 1;

	(direction == 1) ? gpio_dir |= (1 << (bitGPIO)) : gpio_dir &= ~(1 << (bitGPIO));

	if (LMS_GPIODirWrite(lms, &gpio_dir, 1) != 0)
		return -1;

	uint8_t gpio_val = 0;
	if (LMS_GPIORead(lms, &gpio_val, 1) != 0)
		return -1;

	(value == 1) ? gpio_val |= (1 << (bitGPIO)) : gpio_val &= ~(1 << (bitGPIO));

	if (LMS_GPIOWrite(lms, &gpio_val, 1) != 0)
		return -1;

	i2c_dly();

	return 0;
}

int i2c_getVal(lms_device_t* lms, int bitGPIO) {
	uint8_t gpio_val = 0;
	if (LMS_GPIORead(lms, &gpio_val, 1) != 0)
		return -1;

	int result = 0;

	result = gpio_val & (1 << (bitGPIO));

	return result;
}

int i2c_start(lms_device_t* lms)
{
	if(i2c_setVal(lms, GPIO_SDA, 1) != 0)
		return -1;
	i2c_setVal(lms, GPIO_SCL, 1);
	i2c_setVal(lms, GPIO_SDA, 0);
	i2c_setVal(lms, GPIO_SCL, 0);
	return 0;
}

int i2c_stop(lms_device_t* lms)
{
	if(i2c_setVal(lms, GPIO_SDA, 0) != 0)
		return -1;
	i2c_setVal(lms, GPIO_SCL, 1);
	i2c_setVal(lms, GPIO_SDA, 1);
	return 0;
}

unsigned char i2c_rx(lms_device_t* lms, char ack)
{
	char x, d = 0;

	i2c_setVal(lms, GPIO_SDA, 1);
	for (x = 0; x<8; x++) {
		d <<= 1;
		do {
			i2c_setVal(lms, GPIO_SCL, 1);
		} while (i2c_getVal(lms, GPIO_SCL) == 0);    // wait for any SCL clock stretching
		i2c_dly();
		if (i2c_getVal(lms, GPIO_SDA)) d |= 1;
		i2c_setVal(lms, GPIO_SCL, 0);
	}
	if (ack) i2c_setVal(lms, GPIO_SDA, 0);
	else i2c_setVal(lms, GPIO_SDA, 1);
	i2c_setVal(lms, GPIO_SCL, 1);
	i2c_dly();             // send (N)ACK bit
	i2c_setVal(lms, GPIO_SCL, 0);
	i2c_setVal(lms, GPIO_SDA, 1);
	return d;
}

int i2c_tx(lms_device_t* lms, unsigned char d)
{
	char x;
	int b;

	for (x = 8; x; x--) {
		if (d & 0x80)
			i2c_setVal(lms, GPIO_SDA, 1);
		else
			i2c_setVal(lms, GPIO_SDA, 0);
		i2c_setVal(lms, GPIO_SCL, 1);
		d <<= 1;
		i2c_setVal(lms, GPIO_SCL, 0);
	}
	i2c_setVal(lms, GPIO_SDA, 1);
	i2c_setVal(lms, GPIO_SCL, 1);
	i2c_dly();
	b = i2c_getVal(lms, GPIO_SDA);          // possible ACK bit
	i2c_setVal(lms, GPIO_SCL, 0);
	return b;
}

int i2c_write_buffer(lms_device_t* lms, unsigned char* c, int size) {
	unsigned char addressI2C = RFE_I2C_ADDRESS;
	unsigned char addressByte = addressI2C << 1;
	unsigned char addressByteW = addressByte & ~1;
	unsigned char addressByteR = addressByte | 1;

	if(i2c_start(lms) != 0) // send start sequence
		return -1;	
	i2c_tx(lms, addressByteW);	// I2C address with R/W bit clear

	for (int i = 0; i < size; i++) {
		i2c_tx(lms, c[i]);
	}

	i2c_stop(lms);	// send stop sequence

	return 0;
}

int i2c_read_buffer(lms_device_t* lms, unsigned char* c, int size) {
	unsigned char addressI2C = RFE_I2C_ADDRESS;
	unsigned char addressByte = addressI2C << 1;
	unsigned char addressByteW = addressByte & ~1;
	unsigned char addressByteR = addressByte | 1;

	i2c_start(lms);	// send a restart sequence
	i2c_tx(lms, addressByteR);	// I2C address with R/W bit set

	int i;
	for (i = 0; i < size; i++) {
		char ack = 1;
		if (i == (size - 1))
			ack = 0;
		c[i] = i2c_rx(lms, ack);
	}
	i2c_stop(lms);	// send stop sequence
	return i;
}
