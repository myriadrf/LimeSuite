/**
    @file ConnectionNovenaRF7Entry.cpp
    @author Lime Microsystems
    @brief NovenaRF connection over SPI-DEV linux.
*/

#include "ConnectionNovenaRF7.h"
#include "ErrorReporting.h"
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <iostream>

using namespace lime;

/** @brief Tries to read EEPROM for Novena board signature
    @return true if Novena board
*/
bool IsNovenaBoard()
{
#ifdef __unix__
    char data[8];
    int count = 6;
    memset(data, 0, 8);
    int addr = 0;
    struct i2c_rdwr_ioctl_data session;
    struct i2c_msg messages[2];
    char set_addr_buf[2];
    memset(set_addr_buf, 0, sizeof(set_addr_buf));
    memset(data, 0, count);
    set_addr_buf[0] = addr>>8;
    set_addr_buf[1] = addr;
    messages[0].addr = 0xac>>1;
    messages[0].flags = 0;
    messages[0].len = sizeof(set_addr_buf);
    messages[0].buf = reinterpret_cast<__u8*> (set_addr_buf);
    messages[1].addr = 0xac>>1;
    messages[1].flags = I2C_M_RD;
    messages[1].len = count;
    messages[1].buf = reinterpret_cast<__u8*> (data);
    session.msgs = messages;
    session.nmsgs = 2;

    bool isNovena = false;

    int fd = open("/dev/i2c-2", O_RDWR);
    if(fd > 0)
    {
        if(ioctl(fd, I2C_RDWR, &session) < 0)
        {
            perror("Unable to communicate with i2c device");
            isNovena = false;
        }
        if(strcmp("Novena", data) == 0)
            isNovena = true;
    }
    close(fd);
    return isNovena;
#else
    return false;
#endif
}

//! make a static-initialized entry in the registry
void __loadConnectionNovenaRF7Entry(void) //TODO fixme replace with LoadLibrary/dlopen
{
static ConnectionNovenaRF7Entry NovenaRF7Entry;
}

ConnectionNovenaRF7Entry::ConnectionNovenaRF7Entry(void):
    ConnectionRegistryEntry("NovenaRF7")
{
    return;
}

std::vector<ConnectionHandle> ConnectionNovenaRF7Entry::enumerate(const ConnectionHandle &hint)
{
    std::vector<ConnectionHandle> result;

    const char *spiDevPath = "/dev/spidev2.0";
    int spidev = open(spiDevPath, O_RDWR);
    const bool hasSpiDev = (spidev >= 0);
    close(spidev);

    if (hasSpiDev and IsNovenaBoard())
    {
        ConnectionHandle handle;
        handle.media = "SPI";
        handle.name = "NOVENA";
        handle.addr = spiDevPath;
        result.push_back(handle);
    }
    return result;
}

IConnection *ConnectionNovenaRF7Entry::make(const ConnectionHandle &handle)
{
    auto conn = new ConnectionNovenaRF7();
    if (conn->Open(handle.addr.c_str()) != 0)
        std::cerr << GetLastErrorMessage() << std::endl;
    return conn;
}
