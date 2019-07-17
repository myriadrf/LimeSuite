#include "limeRFE_constants.h"
//#include "limeRFE.h"
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
	//milans 190207
	char* cstr = (char*)str;
	//	write(fd, str, len);
	my_write(fd, cstr, len);
	return len;
	for (n = 0; n<len; n++)
		//		write(fd, &str[n], 1);
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

	//fprintf(stderr,"init_serialport: opening port %s @ %d bps\n",
	//        serialport,baud);
	//fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);

	//milans 190207
	char* cserialport = (char*)serialport;

	//	fd = open(serialport, O_RDWR | O_NOCTTY);
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
	//#ifdef _WIN32
	//milans disable DTR - Disable DTR
	//If DTR is not disabled, each time the port is opened, the Arduino uP is reset
	//If DTR is disabled, then uP is reset only the first time the port is opened after plugging USB
	milans_setDTR(DTR_CONTROL_DISABLE);
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

//void write_buffer(unsigned char* c, int size)
void write_buffer(int commType, lms_device_t *dev, int i2Caddress, int fd, unsigned char* data, int size){
	if (commType == LIMERFE_I2C) {
		i2c_write_buffer(dev, i2Caddress, data, size);
	}
	else {
		write_buffer_fd(fd, data, size);
	}
/*
//void write_buffer(char* c, int size)
{
	int actual_length;

	//milans 190207
	//	actual_length = serialport_write(fd, c, size);
	actual_length = serialport_write(fd, (char*)c, size);
	if (actual_length != size) {
		fprintf(stderr, "Error while sending char\n");
	}
*/
}

void write_buffer_fd(int fd, unsigned char* c, int size)
//void write_buffer(char* c, int size)
{
	int actual_length;

	//milans 190207
	//	actual_length = serialport_write(fd, c, size);
	actual_length = serialport_write(fd, (char*)c, size);
	if (actual_length != size) {
		fprintf(stderr, "Error while sending char\n");
	}
}


//int read_buffer(unsigned char * data, int size)
int read_buffer(int commType, lms_device_t *dev, int i2Caddress, int fd, unsigned char * data, int size)
{
	int len;
	if (commType == LIMERFE_I2C) {
		len = i2c_read_buffer(dev, i2Caddress, data, size);
	}
	else {
		len = read_buffer_fd(fd, data, size);
	}
	return len;

/*
	// To receive characters from the device initiate a bulk_transfer to the
	// Endpoint with address ep_in_addr.

	memset(data, 0, size);
	int len;

	//milans 190207
	//	len = serialport_read(fd, data, size);
	len = serialport_read(fd, (char*)data, size);
	return len;
*/
}

int read_buffer_fd(int fd, unsigned char * data, int size)
{
	memset(data, 0, size);
	int len;

	len = serialport_read(fd, (char*)data, size);
	return len;
}


//******* Command Definitions *******
//boardInfo Cmd_GetInfo(int fd) {
boardInfo Cmd_GetInfo(int commType, lms_device_t *dev, int i2Caddress, int fd){
	boardInfo info;
	unsigned char buf[LIMERFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, LIMERFE_BUFFER_SIZE);

	buf[0] = LIMERFE_CMD_GET_INFO;
	write_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);
	len = read_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);
//	write_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);
	//	Sleep(1000);
//	len = read_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);

	info.fw_ver = buf[1];     // FW_VER
	info.hw_ver = buf[2];     // HW_VER
	info.status1 = buf[3];    // Status
	info.status2 = buf[4];    // Status

	return info;
}

int ReadConfig(const char *filename, boardState *state) {
	char mssg[200];

	typedef INI<string, string, string> ini_t;
	ini_t parser(filename, true);

	if (parser.select("LimeRFE_settings") == false)
		return 1;

	state->channelID = parser.get("channelID", 0);
	state->attValue = parser.get("attValue", 0);
	state->notchOnOff = parser.get("notchOnOff", 0);
	state->selTX2TXRX = parser.get("selTX2TXRX", 0);
	state->mode = parser.get("mode", 0);
	state->i2Caddress = parser.get("I2Caddress", 0);
	state->powerCellCorr = parser.get("CellularPowerCorrection", 0);
	state->powerCorr = parser.get("PowerCorrection", 0);
	state->gammaCorr = parser.get("GammaCorrection", 0);

	return 0;
}

int SaveConfig(const char *filename, boardState state) {
	FILE *fout;
	fout = fopen(filename, "w");

	if (fout == NULL) {
		fclose(fout);
		return 1;
	}

	fprintf(fout, "[LimeRFE_settings]\n");

	fprintf(fout, "channelID=%d\n", state.channelID);
	fprintf(fout, "attValue=%d\n", state.attValue);
	fprintf(fout, "notchOnOff=%d\n", state.notchOnOff);
	fprintf(fout, "selTX2TXRX=%d\n", state.selTX2TXRX);
	fprintf(fout, "mode=%d\n", state.mode);
	fprintf(fout, "I2Caddress=%d\n", state.i2Caddress);
	fprintf(fout, "CellularPowerCorrection=%f\n", state.powerCellCorr);
	fprintf(fout, "PowerCorrection=%f\n", state.powerCorr);
	fprintf(fout, "GammaCorrection=%f\n", state.gammaCorr);

	fclose(fout);
	return 0;
}

//int Cmd_GetConfigFull(int fd, boardState *state) {
int Cmd_GetConfigFull(int commType, lms_device_t *dev, int i2Caddress, int fd, boardState *state) {
	unsigned char buf[LIMERFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, LIMERFE_BUFFER_SIZE);

	buf[0] = LIMERFE_CMD_GET_CONFIG_FULL;

	write_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);
	len = read_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);

//	write_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);
//	len = read_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);
	return 0;
}

int Cmd_I2C_Master(int fd, int isMaster) {
	int result = 0;
	unsigned char buf[LIMERFE_BUFFER_SIZE];
	int len;
	memset(buf, 0, LIMERFE_BUFFER_SIZE);

	buf[0] = LIMERFE_CMD_I2C_MASTER;
	buf[1] = isMaster;
	write_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);
	len = read_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);

	//CHECK THIS OUT!!!
	return result;
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

	buf[0] = LIMERFE_CMD_HELLO;

	int attempts = 0;
	bool connected = false;

	while (!connected && (attempts < LIMERFE_MAX_HELLO_ATTEMPTS)) {
		write_buffer_fd(fd, buf, 1);
		mySleep(200);
		len = read_buffer_fd(fd, buf, 1);
		if ((len == 1) && (buf[0] == LIMERFE_CMD_HELLO))
			connected = true;
		attempts++;
	}

	result = (connected) ? 0 : 1;
	return result;
}

//int Cmd_LoadConfig(int fd, const char *filename) {
int Cmd_LoadConfig(int commType, lms_device_t *dev, int i2Caddress, int fd, const char *filename) {
	int result = 0;
	boardState state;
	result = ReadConfig(filename, &state);
	if (result != 0)
		return result;

	

	//WARNING: NOT IMPLEMENTED!!!
	//This should probably load the state from the file directly into the board!
	//NOT TESTED!!!
//	result = Cmd_ConfigureFull(commType, dev, i2Caddress, fd, state.channelID, state.selTX2TXRX, state.notchOnOff, state.attValue);
//	result = Cmd_Configure(commType, dev, i2Caddress, fd, state.channelID, state.selTX2TXRX, state.notchOnOff, state.attValue);
	result = Cmd_ConfigureFull(commType, dev, i2Caddress, fd, state.channelID, state.selTX2TXRX, state.notchOnOff, state.attValue);
//	result = Cmd_Configure(commType, dev, i2Caddress, fd, state.channelID, state.mode, state.selTX2TXRX, state.notchOnOff, state.attValue);

	return result;
}

//int Cmd_Reset(int fd) {
int Cmd_Reset(int commType, lms_device_t *dev, int i2Caddress, int fd) {
	int result = 0;
	unsigned char buf[LIMERFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, LIMERFE_BUFFER_SIZE);

	buf[0] = LIMERFE_CMD_RESET;

	write_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);
	len = read_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);

//	write_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);
//	len = read_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);

	//CHECK THIS OUT!!!
	return result;
}

//int Cmd_ConfigureFull(int fd, int channelID, int selTX2TXRX = 0, int notch = 0, int attenuation = 0){
//int Cmd_ConfigureFull(int fd, int channelID, int selTX2TXRX, int notch, int attenuation) {
//int Cmd_ConfigureFull(int commType, lms_device_t *dev, int i2Caddress, int fd, int channelID, int selTX2TXRX, int notch, int attenuation) {
int Cmd_ConfigureFull(int commType, lms_device_t *dev, int i2Caddress, int fd, int channelID, int mode, int selTX2TXRX, int notch, int attenuation) {
	int result = 0;

	unsigned char buf[LIMERFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, LIMERFE_BUFFER_SIZE);

	buf[0] = LIMERFE_CMD_CONFIG_FULL;

	getDefaultConfiguration(channelID, &buf[1]);
//	getDefaultMode(channelID, LIMERFE_MODE_RX, &buf[13]);
	getDefaultMode(channelID, mode, &buf[13]);

	int notchBit;
	if (notch == LIMERFE_NOTCH_VALUE_OFF)
		notchBit = LIMERFE_NOTCH_BIT_OFF;
	if (notch == LIMERFE_NOTCH_VALUE_ON)
		notchBit = LIMERFE_NOTCH_BIT_ON;

	//	(notch == 1) ? buf[NOTCH_BYTE] |= (1 << NOTCH_BIT) : buf[NOTCH_BYTE] &= ~(1 << NOTCH_BIT);
	(notchBit == 1) ? buf[LIMERFE_NOTCH_BYTE] |= (1 << LIMERFE_NOTCH_BIT) : buf[LIMERFE_NOTCH_BYTE] &= ~(1 << LIMERFE_NOTCH_BIT);

	unsigned char usatt = attenuation;

	for (int i = 0; i<3; i++)
		((usatt >> i) & 0x01) ? buf[LIMERFE_ATTEN_BYTE] |= (1 << (LIMERFE_ATTEN_BIT + i)) : buf[LIMERFE_ATTEN_BYTE] &= ~(1 << (LIMERFE_ATTEN_BIT + i));

	(selTX2TXRX == 1) ? buf[LIMERFE_TX2TXRX_BYTE] |= (1 << LIMERFE_TX2TXRX_BIT) : buf[LIMERFE_TX2TXRX_BYTE] &= ~(1 << LIMERFE_TX2TXRX_BIT);

	write_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);
	len = read_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);

//	write_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);
//	len = read_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);

	//CHECK THIS OUT!!!
	return result;
}

//int Cmd_Configure(int commType, lms_device_t *dev, int i2Caddress, int fd, int channelID, int selTX2TXRX, int notch, int attenuation) {
int Cmd_Configure(int commType, lms_device_t *dev, int i2Caddress, int fd, int channelID, int mode, int selTX2TXRX, int notch, int attenuation) {
	int result = 0;

	unsigned char buf[LIMERFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, LIMERFE_BUFFER_SIZE);

	buf[0] = LIMERFE_CMD_CONFIG;

	buf[1] = channelID;

	// Check the values in firmware
	buf[2] = selTX2TXRX;

	// Check the values in firmware
	buf[3] = notch;

	buf[4] = attenuation;

	buf[5] = mode;

	//Check what is with the mode!!!

	write_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);
	len = read_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);

	//	write_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);
	//	len = read_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);

	//CHECK THIS OUT!!!
	return result;
}

void getDefaultConfigurations(unsigned char** bytes) {
#define TEST 2
	//ovdi!!!
	unsigned char configurations[LIMERFE_CID_COUNT][12] = {
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

	memcpy(bytes, configurations, LIMERFE_CID_COUNT * 12 * sizeof(unsigned char));
}

void getDefaultConfiguration(int channelID, unsigned char* bytes) {
	switch (channelID) {
	case LIMERFE_CID_WB_1000: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 18 11 00 70 00 00 10 21 14 00 38 60
		bytes[0] = 0x18; bytes[1] = 0x11; bytes[2] = 0x00; bytes[3] = 0x70; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x21; bytes[8] = 0x14; bytes[9] = 0x00; bytes[10] = 0x38; bytes[11] = 0x60;
	} break;
	case LIMERFE_CID_WB_4000: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 10 42 22 00 00 10 00 00 64 3A 00
		bytes[0] = 0x00; bytes[1] = 0x10; bytes[2] = 0x42; bytes[3] = 0x22; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x64; bytes[10] = 0x3A; bytes[11] = 0x00;
	} break;
	case LIMERFE_CID_HAM_0030: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 81 50 00 00 00 00 10 09 18 00 58 60
		bytes[0] = 0x81; bytes[1] = 0x50; bytes[2] = 0x00; bytes[3] = 0x00; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x09; bytes[8] = 0x18; bytes[9] = 0x00; bytes[10] = 0x58; bytes[11] = 0x60;
	} break;
	case LIMERFE_CID_HAM_0145: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 42 14 00 30 00 00 10 11 12 00 38 60
		bytes[0] = 0x42; bytes[1] = 0x14; bytes[2] = 0x00; bytes[3] = 0x30; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x11; bytes[8] = 0x12; bytes[9] = 0x00; bytes[10] = 0x38; bytes[11] = 0x60;
	} break;
	case LIMERFE_CID_HAM_0435: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 24 12 00 50 00 00 10 05 11 00 38 60
		bytes[0] = 0x24; bytes[1] = 0x12; bytes[2] = 0x00; bytes[3] = 0x50; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x05; bytes[8] = 0x11; bytes[9] = 0x00; bytes[10] = 0x38; bytes[11] = 0x60;
	} break;
	case LIMERFE_CID_HAM_1280: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 10 48 11 00 00 10 00 00 44 38 00
		bytes[0] = 0x00; bytes[1] = 0x10; bytes[2] = 0x48; bytes[3] = 0x11; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x44; bytes[10] = 0x38; bytes[11] = 0x00;
	} break;
	case LIMERFE_CID_HAM_2400: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 10 41 43 00 00 10 00 00 24 3E 00
		bytes[0] = 0x00; bytes[1] = 0x10; bytes[2] = 0x41; bytes[3] = 0x43; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x24; bytes[10] = 0x3E; bytes[11] = 0x00;
	} break;
	case LIMERFE_CID_HAM_3500: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 10 44 00 00 00 10 00 00 04 3C 00
		bytes[0] = 0x00; bytes[1] = 0x10; bytes[2] = 0x44; bytes[3] = 0x00; bytes[4] = 0x00; bytes[5] = 0x00; bytes[6] = 0x10; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x04; bytes[10] = 0x3C; bytes[11] = 0x00;
	} break;
	case LIMERFE_CID_CELL_BAND01: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 00 00 00 52 35 86 00 00 00 11 18
		bytes[0] = 0x00; bytes[1] = 0x00; bytes[2] = 0x00; bytes[3] = 0x00; bytes[4] = 0x52; bytes[5] = 0x35; bytes[6] = 0x86; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x00; bytes[10] = 0x11; bytes[11] = 0x18;
	} break;
	case LIMERFE_CID_CELL_BAND02: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 00 00 00 60 63 46 00 00 00 11 18
		bytes[0] = 0x00; bytes[1] = 0x00; bytes[2] = 0x00; bytes[3] = 0x00; bytes[4] = 0x60; bytes[5] = 0x63; bytes[6] = 0x46; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x00; bytes[10] = 0x11; bytes[11] = 0x18;
	} break;
	case LIMERFE_CID_CELL_BAND03: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 00 00 00 44 21 06 00 00 00 11 18
		bytes[0] = 0x00; bytes[1] = 0x00; bytes[2] = 0x00; bytes[3] = 0x00; bytes[4] = 0x44; bytes[5] = 0x21; bytes[6] = 0x06; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x00; bytes[10] = 0x11; bytes[11] = 0x18;
	} break;
	case LIMERFE_CID_CELL_BAND07: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 00 00 00 01 02 65 00 00 00 11 18
		bytes[0] = 0x00; bytes[1] = 0x00; bytes[2] = 0x00; bytes[3] = 0x00; bytes[4] = 0x01; bytes[5] = 0x02; bytes[6] = 0x65; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x00; bytes[10] = 0x11; bytes[11] = 0x18;
	} break;
	case LIMERFE_CID_CELL_BAND38: {
		//             0  1  2  3  4  5  6  7  8  9 10 11
		//v03 190628: 00 00 00 00 11 90 25 00 00 00 11 18
		bytes[0] = 0x00; bytes[1] = 0x00; bytes[2] = 0x00; bytes[3] = 0x00; bytes[4] = 0x11; bytes[5] = 0x90; bytes[6] = 0x25; bytes[7] = 0x00; bytes[8] = 0x00; bytes[9] = 0x00; bytes[10] = 0x11; bytes[11] = 0x18;
	} break;
	}
}

//int Cmd_ModeFull(int fd, int channelID, int mode) {
int Cmd_ModeFull(int commType, lms_device_t *dev, int i2Caddress, int fd, int channelID, int mode) {
	int result = 0;

	unsigned char buf[LIMERFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, LIMERFE_BUFFER_SIZE);

	buf[0] = LIMERFE_CMD_MODE_FULL;

	int moderes = getDefaultMode(channelID, mode, &buf[1]);
	if (moderes != 0)
		return moderes;

	write_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);
	len = read_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);

//	write_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);
//	len = read_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);

	//CHECK THIS OUT!!!
	return result;
}

int Cmd_Mode(int commType, lms_device_t *dev, int i2Caddress, int fd, int mode) {
	int result = 0;

	unsigned char buf[LIMERFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, LIMERFE_BUFFER_SIZE);

	buf[0] = LIMERFE_CMD_MODE;

	buf[1] = mode;

	write_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);
	len = read_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);

	//	write_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);
	//	len = read_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);

	//CHECK THIS OUT!!!
	return result;
}

int getDefaultMode(int channelID, int mode, unsigned char* mcu_byte) {
	int result = 0;

	int iTXRX0 = 0; //V2
	int iTXRX1 = 0; //V3
	int iLNA_EN = 0;
	int iPA_EN = 0;
	//	int iRELAY = 0; //0 - TX; 1 - RX
	int iRELAY = 0; //0 - RX; 1 - TX

					// For Cellular channels the mode cannot be changed
	if ((channelID == LIMERFE_CID_CELL_BAND01) ||
		(channelID == LIMERFE_CID_CELL_BAND02) ||
		(channelID == LIMERFE_CID_CELL_BAND03) ||
		(channelID == LIMERFE_CID_CELL_BAND07) ||
		(channelID == LIMERFE_CID_CELL_BAND38)) {
		iTXRX0 = 0;
		iTXRX1 = 1;
		//		return(1);
	}
	else {
		if (mode == LIMERFE_MODE_NONE) {
			iLNA_EN = 0;
			iPA_EN = 0;
		}

		if (mode == LIMERFE_MODE_TX) {
			iLNA_EN = 0;
			iPA_EN = 1;
			if (channelID == LIMERFE_CID_HAM_0030) {
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
		if ((mode == LIMERFE_MODE_RX) ||
			(mode == LIMERFE_MODE_TXRX)) {
			if (channelID == LIMERFE_CID_HAM_0030) {
				//				iRELAY = 1;
				iRELAY = 0;
			}
			else if ((channelID == LIMERFE_CID_HAM_0145) || //RX_L
				(channelID == LIMERFE_CID_HAM_0435) ||
				(channelID == LIMERFE_CID_WB_1000)) {
				iTXRX0 = 1;
				iTXRX1 = 0;
			}
			else { //RX_H
				iTXRX0 = 0;
				iTXRX1 = 0;
			}
			if (mode == LIMERFE_MODE_RX) {
				iLNA_EN = 1;
				iPA_EN = 0;
			}
			if (mode == LIMERFE_MODE_TXRX) {
				iLNA_EN = 1;
				iPA_EN = 1;
			}
		}
	}

	(iTXRX0 == 1) ? *mcu_byte |= (1 << (LIMERFE_MCU_BYTE_TXRX0_BIT)) : *mcu_byte &= ~(1 << (LIMERFE_MCU_BYTE_TXRX0_BIT));
	(iTXRX1 == 1) ? *mcu_byte |= (1 << (LIMERFE_MCU_BYTE_TXRX1_BIT)) : *mcu_byte &= ~(1 << (LIMERFE_MCU_BYTE_TXRX1_BIT));
	(iRELAY == 1) ? *mcu_byte |= (1 << (LIMERFE_MCU_BYTE_RELAY_BIT)) : *mcu_byte &= ~(1 << (LIMERFE_MCU_BYTE_RELAY_BIT));
	(iLNA_EN == 1) ? *mcu_byte |= (1 << (LIMERFE_MCU_BYTE_LNA_EN_BIT)) : *mcu_byte &= ~(1 << (LIMERFE_MCU_BYTE_LNA_EN_BIT));
	(iPA_EN == 1) ? *mcu_byte |= (1 << (LIMERFE_MCU_BYTE_PA_EN_BIT)) : *mcu_byte &= ~(1 << (LIMERFE_MCU_BYTE_PA_EN_BIT));

	return result;
}

//int Cmd_ReadADC(int fd, int adcID, int *value) {
int Cmd_ReadADC(int commType, lms_device_t *dev, int i2Caddress, int fd, int adcID, int* value) {
	int result = LIMERFE_COM_SUCCESS;
	unsigned char buf[LIMERFE_BUFFER_SIZE];
	int len;

	memset(buf, 0, LIMERFE_BUFFER_SIZE);

	if (adcID == LIMERFE_ADC1)
		buf[0] = LIMERFE_CMD_READ_ADC1;
	else
		buf[0] = LIMERFE_CMD_READ_ADC2;

	write_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);
	len = read_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);

//	write_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);
//	len = read_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);
	if (len == -1) {
		*value = 0;
		return(LIMERFE_COM_ERROR);
	}

	*value = buf[2] * pow(2, 8) + buf[1];

	return result;
}

int Cmd_LEDonOff(int commType, lms_device_t *dev, int i2Caddress, int fd, int onOff) {
	int result = 0;
	unsigned char buf[LIMERFE_BUFFER_SIZE];
	int len;
	memset(buf, 0, LIMERFE_BUFFER_SIZE);

	buf[0] = LIMERFE_CMD_LED_ONOFF;
	//	buf[0] = 0x02;
	buf[1] = onOff;

	write_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);
	len = read_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);

//	i2c_write_buffer(lms, addressI2C, buf, LIMERFE_BUFFER_SIZE);
	//	len = i2c_read_buffer(addressI2C, buf, BUFFER_SIZE);
//	i2c_read_buffer(lms, addressI2C, buf, LIMERFE_BUFFER_SIZE);

	//CHECK THIS OUT!!!
	return result;
}

int Cmd_Cmd(int commType, lms_device_t *dev, int i2Caddress, int fd, unsigned char* buf) {
	int result = 0;
	int len;

	write_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);
	len = read_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);

	//	write_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);
	//	len = read_buffer_fd(fd, buf, LIMERFE_BUFFER_SIZE);

	//CHECK THIS OUT!!!
	return result;
}

int Cmd_ConfGPIO45(int commType, lms_device_t *dev, int i2Caddress, int fd, int gpioNum, int direction) {
	int result = 0;
	unsigned char buf[LIMERFE_BUFFER_SIZE];
	int len;
	memset(buf, 0, LIMERFE_BUFFER_SIZE);

	buf[0] = LIMERFE_CMD_CONFGPIO45;
	buf[1] = gpioNum;
	buf[2] = direction;

	write_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);
	len = read_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);

	//CHECK THIS OUT!!!
	return result;
}

int Cmd_SetGPIO45(int commType, lms_device_t *dev, int i2Caddress, int fd, int gpioNum, int val) {
	int result = 0;
	unsigned char buf[LIMERFE_BUFFER_SIZE];
	int len;
	memset(buf, 0, LIMERFE_BUFFER_SIZE);

	buf[0] = LIMERFE_CMD_SETGPIO45;
	buf[1] = gpioNum;
	buf[2] = val;

	write_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);
	len = read_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);

	//CHECK THIS OUT!!!
	return result;
}

int Cmd_GetGPIO45(int commType, lms_device_t *dev, int i2Caddress, int fd, int gpioNum, int * val) {
	int result = 0;
	unsigned char buf[LIMERFE_BUFFER_SIZE];
	int len;
	memset(buf, 0, LIMERFE_BUFFER_SIZE);

	buf[0] = LIMERFE_CMD_GETGPIO45;
	buf[1] = gpioNum;

	write_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);
	len = read_buffer(commType, dev, i2Caddress, fd, buf, LIMERFE_BUFFER_SIZE);

	*val = buf[1];

	//CHECK THIS OUT!!!
	return result;
}

/******************************************************************************
* I2C Communications
*******************************************************************************/

// DO PROPER ERROR HANDLING HERE!!!
//int i2c_error()
//{
//	if (device != NULL)
//		LMS_Close(device);
////	exit(-1);
//}

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
	double sleepms = 0.5 * 1 / LIMERFE_I2C_FSCL * 1000; //milans - for example 0.5 * period - Check this!!!
	mySleep(sleepms);
}

void i2c_setVal(lms_device_t* lms, int bitGPIO, int value) {
	uint8_t gpio_dir = 0;
	int direction;
//	if (LMS_GPIODirRead(lms, &gpio_dir, 1) != 0)
//		i2c_error();
	LMS_GPIODirRead(lms, &gpio_dir, 1);
	// Direction: 1-output, 0-input
	// If writting 0, then output => direction = 1
	// If writting 1, then input (high Z, and pull-up will do the trick) => direction = 0
	(value == 1) ? direction = 0 : direction = 1;

	(direction == 1) ? gpio_dir |= (1 << (bitGPIO)) : gpio_dir &= ~(1 << (bitGPIO));

//	if (LMS_GPIODirWrite(lms, &gpio_dir, 1) != 0)
//		i2c_error();
	LMS_GPIODirWrite(lms, &gpio_dir, 1);

	uint8_t gpio_val = 0;
//	if (LMS_GPIORead(lms, &gpio_val, 1) != 0)
//		i2c_error();
	LMS_GPIORead(lms, &gpio_val, 1);

	(value == 1) ? gpio_val |= (1 << (bitGPIO)) : gpio_val &= ~(1 << (bitGPIO));

//	if (LMS_GPIOWrite(lms, &gpio_val, 1) != 0)
//		i2c_error();
	LMS_GPIOWrite(lms, &gpio_val, 1);

	i2c_dly(); // milans - Is this the right place for this function???
}

int i2c_getVal(lms_device_t* lms, int bitGPIO) {
	uint8_t gpio_val = 0;
//	if (LMS_GPIORead(lms, &gpio_val, 1) != 0)
//		i2c_error();
	LMS_GPIORead(lms, &gpio_val, 1);

	int result = 0;

	result = gpio_val & (1 << (bitGPIO));

	return result;
}

void i2c_start(lms_device_t* lms)
{
	//	SDA = 1;             // i2c start bit sequence
	i2c_setVal(lms, GPIO_SDA, 1);
	//	i2c_dly();
	//	SCL = 1;
	i2c_setVal(lms, GPIO_SCL, 1);
	//	i2c_dly();
	//	SDA = 0;
	i2c_setVal(lms, GPIO_SDA, 0);
	//	i2c_dly();
	//	SCL = 0;
	i2c_setVal(lms, GPIO_SCL, 0);
	//	i2c_dly();
}

void i2c_stop(lms_device_t* lms)
{
	//	SDA = 0;             // i2c stop bit sequence
	i2c_setVal(lms, GPIO_SDA, 0);
	//	i2c_dly();
	//	SCL = 1;
	i2c_setVal(lms, GPIO_SCL, 1);
	//	i2c_dly();
	//	SDA = 1;
	i2c_setVal(lms, GPIO_SDA, 1);
	//	i2c_dly();
}

unsigned char i2c_rx(lms_device_t* lms, char ack)
{
	char x, d = 0;
	//	SDA = 1;
	i2c_setVal(lms, GPIO_SDA, 1);
	for (x = 0; x<8; x++) {
		d <<= 1;
		do {
			//			SCL = 1;
			i2c_setVal(lms, GPIO_SCL, 1);
			//		} while (SCL_IN == 0);    // wait for any SCL clock stretching
		} while (i2c_getVal(lms, GPIO_SCL) == 0);    // wait for any SCL clock stretching
		i2c_dly();
		//		if (SDA_IN) d |= 1;
		if (i2c_getVal(lms, GPIO_SDA)) d |= 1;
		//		SCL = 0;
		i2c_setVal(lms, GPIO_SCL, 0);
	}
	//	if (ack) SDA = 0;
	if (ack) i2c_setVal(lms, GPIO_SDA, 0);
	//	else SDA = 1;
	else i2c_setVal(lms, GPIO_SDA, 1);
	//	SCL = 1;
	i2c_setVal(lms, GPIO_SCL, 1);
	i2c_dly();             // send (N)ACK bit
						   //	SCL = 0;
	i2c_setVal(lms, GPIO_SCL, 0);
	//	SDA = 1;
	i2c_setVal(lms, GPIO_SDA, 1);
	return d;
}

//bit i2c_tx(unsigned char d)
int i2c_tx(lms_device_t* lms, unsigned char d)
{
	char x;
	//	static bit b;
	int b;

	for (x = 8; x; x--) {
		//		if (d & 0x80) SDA = 1;
		if (d & 0x80)
			i2c_setVal(lms, GPIO_SDA, 1);
		//		else SDA = 0;
		else
			i2c_setVal(lms, GPIO_SDA, 0);
		//		SCL = 1;
		i2c_setVal(lms, GPIO_SCL, 1);
		d <<= 1;
		//		SCL = 0;
		i2c_setVal(lms, GPIO_SCL, 0);
	}
	//	SDA = 1;
	i2c_setVal(lms, GPIO_SDA, 1);
	//	SCL = 1;
	i2c_setVal(lms, GPIO_SCL, 1);
	i2c_dly();
	//	b = SDA_IN;          // possible ACK bit
	b = i2c_getVal(lms, GPIO_SDA);          // possible ACK bit
								   //	SCL = 0;
	i2c_setVal(lms, GPIO_SCL, 0);
	return b;
}

void i2c_write_buffer(lms_device_t* lms, unsigned char addressI2C, unsigned char* c, int size) {
	unsigned char addressByte = addressI2C << 1;
	unsigned char addressByteW = addressByte & ~1;
	unsigned char addressByteR = addressByte | 1;

	i2c_start(lms);              // send start sequence

							  //	i2c_tx(0xE0);             // SRF08 I2C address with R/W bit clear
	i2c_tx(lms, addressByteW);      // I2C address with R/W bit clear

							   //	i2c_tx(0x00);             // SRF08 command register address
							   //	i2c_tx(0x00);             // command register address - ???

							   //	i2c_tx(0x51);             // command to start ranging in cm
	for (int i = 0; i < size; i++) {
		i2c_tx(lms, c[i]);
	}

	i2c_stop(lms);               // send stop sequence
}

int i2c_read_buffer(lms_device_t* lms, unsigned char addressI2C, unsigned char* c, int size) {
	unsigned char addressByte = addressI2C << 1;
	unsigned char addressByteW = addressByte & ~1;
	unsigned char addressByteR = addressByte | 1;
	/*
	i2c_start();              // send start sequence
	//	i2c_tx(0xE0);             // SRF08 I2C address with R/W bit clear
	i2c_tx(addressByteW);     // I2C address with R/W bit clear

	//	i2c_tx(0x01);             // SRF08 light sensor register address
	//	i2c_tx(0x00);             // command register address - ???
	*/
	i2c_start(lms);              // send a restart sequence
							  //	i2c_tx(0xE1);             // SRF08 I2C address with R/W bit set
	i2c_tx(lms, addressByteR);     // I2C address with R/W bit set
							  //	lightsensor = i2c_rx(1);  // get light sensor and send acknowledge. Internal register address will increment automatically.
							  //	rangehigh = i2c_rx(1);    // get the high byte of the range and send acknowledge.
							  //	rangelow = i2c_rx(0);     // get low byte of the range - note we don't acknowledge the last byte.
	int i;
	for (i = 0; i < LIMERFE_BUFFER_SIZE; i++) {
		char ack = 1;
		if (i == (LIMERFE_BUFFER_SIZE - 1))
			ack = 0;
		c[i] = i2c_rx(lms, ack);
	}
	i2c_stop(lms);               // send stop sequence
	return i;
}

int I2C_Cmd_LEDonOff(lms_device_t* lms, unsigned char addressI2C, int onOff) {
	int result = 0;
	unsigned char buf[LIMERFE_BUFFER_SIZE];
	int len;
	memset(buf, 0, LIMERFE_BUFFER_SIZE);

	buf[0] = LIMERFE_CMD_LED_ONOFF;
	//	buf[0] = 0x02;
	buf[1] = onOff;
	i2c_write_buffer(lms, addressI2C, buf, LIMERFE_BUFFER_SIZE);
	//	len = i2c_read_buffer(addressI2C, buf, BUFFER_SIZE);
	i2c_read_buffer(lms, addressI2C, buf, LIMERFE_BUFFER_SIZE);

	//CHECK THIS OUT!!!
	return result;
}