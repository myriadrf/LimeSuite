#include <iostream>

using namespace std;

#include "LMS7002M.h"
#include "lmsComms.h"
#include "cpp-feather-ini-parser/INI.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <fstream>
#include <chrono>
#include <thread>
#include "lmsComms.h"

#if defined(__GNUC__) || defined(__GNUG__)
#include <unistd.h>
#include <sys/time.h>
#endif

#ifdef __unix__
#include <sys/mman.h>
#include <stdint.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#endif

#define IMX6_EIM_BASE_ADDR 0x021b8000
#define IMX6_EIM_CS0_BASE (0x00)
#define IMX6_EIM_CS1_BASE (0x18)
#define IMX6_EIM_CS2_BASE (0x30)
#define IMX6_EIM_CS3_BASE (0x48)
#define IMX6_EIM_WCR (0x90)
#define IMX6_EIM_WIAR (0x94)

#define IMX6_EIM_CS0_BASE_ADDR 0x08040000
#define IMX6_EIM_CS1_BASE_ADDR 0x0c040000

#define DATA_FIFO_ADDR (IMX6_EIM_CS1_BASE_ADDR + 0xf000)

LMScomms *serPort;
LMS7002M *lmsControl;

static const int cSPI_SPEED = 5000000;

#ifdef __unix__
static int   *mem_32 = 0;
static short *mem_16 = 0;
static char  *mem_8 = 0;
static int   *prev_mem_range = 0;
static int mem_fd = 0;
static int fd = 0;
#endif

/** @brief Helper function to write board spi regiters
    @param address spi address
    @param data register value
*/
int SPI_write(uint16_t address, uint16_t data)
{
    LMScomms::GenericPacket ctrPkt;
    ctrPkt.cmd = CMD_LMS7002_WR;
    ctrPkt.outBuffer.push_back((address >> 8) & 0xFF);
    ctrPkt.outBuffer.push_back(address & 0xFF);
    ctrPkt.outBuffer.push_back((data >> 8) & 0xFF);
    ctrPkt.outBuffer.push_back(data & 0xFF);
    serPort->TransferPacket(ctrPkt);
    return ctrPkt.status == 1 ? 0 : -1;
}

/** @brief Helper function to read board spi registers
    @param address spi address
    @return register value
*/
uint16_t SPI_read(uint16_t address)
{
    LMScomms::GenericPacket ctrPkt;
    ctrPkt.cmd = CMD_LMS7002_RD;
    ctrPkt.outBuffer.push_back((address >> 8) & 0xFF);
    ctrPkt.outBuffer.push_back(address & 0xFF);
    serPort->TransferPacket(ctrPkt);
    if (ctrPkt.status == STATUS_COMPLETED_CMD && ctrPkt.inBuffer.size() >= 4)
        return ctrPkt.inBuffer[2] * 256 + ctrPkt.inBuffer[3];
    else
        return 0;
}

int fpga_read(unsigned const int addr, unsigned short *buf, const int len)
{
#ifdef __unix__
    void* mem_base = 0;

    if(mem_32)
        munmap(mem_32, 0xFFFF);
    if(fd)
        close(fd);

    fd = open("/dev/mem", O_RDONLY | O_RSYNC);
    if( fd < 0 )
    {
        perror("Unable to open /dev/mem");
        fd = 0;
        return -1;
    }

    const unsigned int map_size = 4096;
    const unsigned int map_mask = map_size-1;

    mem_base = mmap(0, map_size, PROT_READ , MAP_SHARED, fd, addr & ~map_mask);
    //mem_base = ioremap_wc(addr & ~map_mask, map_size);
    if(mem_base < 0)
    {
        printf("error mapping memory\n");
        return -1;
    }
    void* virt_addr;
    virt_addr = mem_base + (addr & map_mask);
    memcpy(buf, virt_addr, len);
    if(munmap(mem_base, map_size) == -1)
    {
        printf("munmap failed\n");
        return -2;
    }
    close(fd);
#endif
    return 0;
}

int readKernelMemory(long offset, int virtualized, int size)
{
#ifdef __unix__
    int result;

    int *mem_range = (int *)(offset & ~0xFFFF);
    if (mem_range != prev_mem_range)
    {
        prev_mem_range = mem_range;

        if (mem_32)
            munmap(mem_32, 0xFFFF);
        if (mem_fd)
            close(mem_fd);

        if (virtualized)
        {
            mem_fd = open("/dev/kmem", O_RDWR);
            if (mem_fd < 0)
            {
                printf("%s %s\n", "Couldn't open /dev/mem", strerror(errno));
                mem_fd = 0;
                return -1;
            }
        }
        else
        {
            mem_fd = open("/dev/mem", O_RDWR);
            if (mem_fd < 0)
            {
                printf("%s %s\n", "Couldn't open /dev/mem", strerror(errno));
                mem_fd = 0;
                return -1;
            }
        }

        mem_32 = (int *)mmap(0, 0xffff, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, offset&~0xFFFF);
        if ((void *)-1 == mem_32)
        {
            printf("%s %s\n", "Couldn't mmap /dev/kmem: ", strerror(errno));

            if (-1 == close(mem_fd))
                printf("%s %s\n", "Also couldn't close /dev/kmem: ", strerror(errno));

            mem_fd = 0;
            return -1;
        }
        mem_16 = (short *)mem_32;
        mem_8 = (char *)mem_32;
    }

    int scaled_offset = (offset - (offset&~0xFFFF));
    if (size == 1)
    {
        if (mem_8)
            result = mem_8[scaled_offset / sizeof(char)];
    }
    else if (size == 2)
    {
        if (mem_16)
            result = mem_16[scaled_offset / sizeof(short)];
    }
    else
    {
        if (mem_32)
            result = mem_32[scaled_offset / sizeof(long)];
    }

    return result;
#else
    return 0;
#endif
}

int writeKernelMemory(long offset, long value, int virtualized, int size)
{
#ifdef __unix__
    int old_value = readKernelMemory(offset, virtualized, size);
    int scaled_offset = (offset - (offset&~0xFFFF));
    if (size == 1)
    {
        if (mem_8)
            mem_8[scaled_offset / sizeof(char)] = value;
    }
    else if (size == 2)
    {
        if (mem_16)
            mem_16[scaled_offset / sizeof(short)] = value;
    }
    else
    {
        if (mem_32)
            mem_32[scaled_offset / sizeof(long)] = value;
    }
    return old_value;
#endif // __unix__
    return 0;
}

int prep_eim_burst()
{
    typedef INI<string, string, string> ini_t;
    ini_t parser("EIM.ini", true);
    parser.select("EIM");

    // set up pads to be mapped to EIM
    for (int i = 0; i < 16; i++)
    {
        writeKernelMemory(0x20e0114 + i*4, 0x0, 0, 4); // mux mapping
        writeKernelMemory(0x20e0428 + i*4, 0xb0b1, 0, 4); // pad strength config'd for a 100MHz rate
    }

    // mux mapping
    writeKernelMemory(0x20e046c - 0x314, 0x0, 0, 4); // BCLK
    writeKernelMemory(0x20e040c - 0x314, 0x0, 0, 4); // CS0
    writeKernelMemory(0x20e0410 - 0x314, 0x0, 0, 4); // CS1
    writeKernelMemory(0x20e0414 - 0x314, 0x0, 0, 4); // OE
    writeKernelMemory(0x20e0418 - 0x314, 0x0, 0, 4); // RW
    writeKernelMemory(0x20e041c - 0x314, 0x0, 0, 4); // LBA
    writeKernelMemory(0x20e0468 - 0x314, 0x0, 0, 4); // WAIT
    writeKernelMemory(0x20e0408 - 0x314, 0x0, 0, 4); // A16
    writeKernelMemory(0x20e0404 - 0x314, 0x0, 0, 4); // A17
    writeKernelMemory(0x20e0400 - 0x314, 0x0, 0, 4); // A18
    writeKernelMemory(0x20e0400 - 0x314, 0x0, 0, 4); // A18

    // pad strength
    writeKernelMemory(0x20e046c, 0xb0b1, 0, 4); // BCLK
    writeKernelMemory(0x20e040c, 0xb0b1, 0, 4); // CS0
    writeKernelMemory(0x20e0410, 0xb0b1, 0, 4); // CS1
    writeKernelMemory(0x20e0414, 0xb0b1, 0, 4); // OE
    writeKernelMemory(0x20e0418, 0xb0b1, 0, 4); // RW
    writeKernelMemory(0x20e041c, 0xb0b1, 0, 4); // LBA
    writeKernelMemory(0x20e0468, 0xb0b1, 0, 4); // WAIT
    writeKernelMemory(0x20e0408, 0xb0b1, 0, 4); // A16
    writeKernelMemory(0x20e0404, 0xb0b1, 0, 4); // A17
    writeKernelMemory(0x20e0400, 0xb0b1, 0, 4); // A18

    writeKernelMemory(0x020c4080, 0xcf3, 0, 4); // ungate eim slow clocks

    // EIM_CSnGCR1
    // rework timing for sync use
    // PSZ WP GBC AUS CSREC SP DSZ BCS BCD WC BL CREP CRE RFL WFL MUM SRD SWR CSEN

    int PSZ = parser.get("PSZ", 3) << 28; // 2048 words page size
    int WP = parser.get("WP", 0) << 27; //(not protected)
    int GBC = parser.get("GBC", 1) << 24; //min 1 cycles between chip select changes
    int AUS = parser.get("AUS", 1) << 23; //0 address shifted according to port size / 1: unshifted
    int CSREC = parser.get("CSREC", 1) << 20; //min 1 cycles between CS, OE, WE signals
    int SP = parser.get("SP", 0) << 19; //no supervisor protect (user mode access allowed)
    int DSZ = parser.get("DSZ", 1) << 16; //16-bit port resides on DATA[15:0]
    int BCS = parser.get("BCS", 0) << 14; //0 clock delay for burst generation
    int BCD = parser.get("BCD", 0) << 12; //divide EIM clock by 1 for burst clock
    int WC = parser.get("WC", 1) << 11; //write accesses are continuous burst length
    int BL = parser.get("BL", 3) << 8; //32 word memory wrap length
    //int BL = 4 << 8; //32 word memory wrap length
    int CREP = parser.get("CREP", 1) << 7; //non-PSRAM, set to 1
    int CRE = parser.get("CRE", 0)  << 6; //CRE is disabled
    int RFL = parser.get("RFL", 0) << 5; //fixed latency reads
    int WFL = parser.get("WFL", 0) << 4; //fixed latency writes
    int MUM = parser.get("MUM", 1) << 3; //multiplexed mode enabled
    int SRD = parser.get("SRD", 1) << 2; //synch reads
    int SWR = parser.get("SWR", 1) << 1; //synch writes
    int CSEN = parser.get("CSEN", 1); //chip select is enabled
    int EIM_CSnGCR1 = PSZ|WP|GBC|AUS|CSREC|SP|DSZ|BCS|BCD|WC|BL|CREP|CRE|RFL|WFL|MUM|SRD|SWR|CSEN;
    printf("EIM_CSnGCR1 0x%08X\n", EIM_CSnGCR1);

    writeKernelMemory(0x21b8000, EIM_CSnGCR1, 0, 4);
    writeKernelMemory(0x21b8000+24, EIM_CSnGCR1, 0, 4);
    writeKernelMemory(0x21b8000+48, EIM_CSnGCR1, 0, 4);

    // EIM_CS0GCR2
    int MUX16_BYP_GRANT = parser.get("MUX16_BYP_GRANT", 1) << 12;
    int DAP = parser.get("DAP", 0) << 9;
    int DAE = parser.get("DAE", 0) << 8;
    int DAPS = parser.get("DAPS", 0) << 4;
    int ADH = parser.get("ADH", 0); // address hold time after ADC negation(0 cycles)
    int EIM_CSnGCR2 = MUX16_BYP_GRANT|DAP|DAE|DAPS|ADH;
    printf("EIM_CSnGCR2 0x%08X\n", EIM_CSnGCR2);
    writeKernelMemory(0x21b8004, EIM_CSnGCR2, 0, 4);
    writeKernelMemory(0x21b8004+24, EIM_CSnGCR2, 0, 4);
    writeKernelMemory(0x21b8004+48, EIM_CSnGCR2, 0, 4);

    // EIM_CS0RCR1
    // RWSC RADVA RAL RADVN OEA OEN RCSA RCSN
    int RWSC = parser.get("RWSC", 3) << 24;
    int RADVA = parser.get("RADVA", 0) << 20;
    int RAL = parser.get("RAL", 0) << 19;
    int RADVN = parser.get("RADVN", 0) << 16;
    int OEA = parser.get("OEA", 0) << 12;
    int OEN = parser.get("OEN", 0) << 8;
    int RCSA = parser.get("RCSA", 0) << 4;
    int RCSN = parser.get("RCSN", 0);
    int EIM_CSnRCR1 = RWSC|RADVA|RAL|RADVN|OEA|OEN|RCSA|RCSN;
    writeKernelMemory(0x21b8008, EIM_CSnRCR1, 0, 4);
    writeKernelMemory(0x21b8008+24, EIM_CSnRCR1, 0, 4);
    writeKernelMemory(0x21b8008+48, EIM_CSnRCR1, 0, 4);
    printf("EIM_CSnRCR1 0x%08X\n", EIM_CSnRCR1);

    // EIM_CS0RCR2
    // APR PAT RL RBEA RBEN
    int APR = parser.get("APR", 0) << 15; // 0 mandatory because MUM = 1
    int PAT = parser.get("PAT", 0) << 12; // XXX because APR = 0
    int RL = parser.get("RL", 0) << 8; //
    int RBEA = parser.get("RBEA", 0) << 4; //these match RCSA/RCSN from previous field
    int RBE = parser.get("RBE", 0) << 3;
    int RBEN = parser.get("RBEN", 0);
    int EIM_CSnRCR2 = APR|PAT|RL|RBEA|RBE|RBEN;
    writeKernelMemory(0x21b800c, EIM_CSnRCR2, 0, 4);
    writeKernelMemory(0x21b800c+24, EIM_CSnRCR2, 0, 4);
    writeKernelMemory(0x21b800c+48, EIM_CSnRCR2, 0, 4);
    printf("EIM_CSnRCR2 0x%08X\n", EIM_CSnRCR2);

    // EIM_CS0WCR1
    // WAL WBED WWSC WADVA WADVN WBEA WBEN WEA WEN WCSA WCSN
    int WAL = parser.get("WAL", 0) << 31; //use WADVN
    int WBED = parser.get("WBED", 0) << 30; //allow BE during write
    int WWSC = parser.get("WWSC", 1) << 24; // write wait states
    int WADVA = parser.get("WADVA", 0) << 21; //same as RADVA
    int WADVN = parser.get("WADVN", 2) << 18; //this sets WE length to 1 (this value +1)
    int WBEA = parser.get("WBEA", 0) << 15; //same as RBEA
    int WBEN = parser.get("WBEN", 0) << 12; //same as RBEN
    //int WEA = 2 << 9; //2 cycles between beginning of access and WE assertion
    int WEA = parser.get("WEA", 0) << 9; //0 cycles between beginning of access and WE assertion
    int WEN = parser.get("WEN", 0) << 6; //1 cycles to end of WE assertion
    int WCSA = parser.get("WCSA", 0) << 3; //cycles to CS assertion
    int WCSN = parser.get("WCSN", 0) ; //cycles to CS negation
    int EIM_CSnWCR1 = WAL|WBED|WWSC|WADVA|WADVN|WBEA|WBEN|WEA|WEN|WCSA|WCSN;
    printf("EIM_CSnWCR1 0x%08X\n", EIM_CSnWCR1);
    writeKernelMemory(0x21b8010, EIM_CSnWCR1, 0, 4);
    writeKernelMemory(0x21b8010+24, EIM_CSnWCR1, 0, 4); //cs1
    writeKernelMemory(0x21b8010+48, EIM_CSnWCR1, 0, 4); //cs2

    // EIM_WCR
    // BCM = 1 free-run BCLK
    // GBCD = 0 don't divide the burst clock
    // EIM_WCR
    int WDOG_LIMIT = parser.get("WDOG_LIMIT", 3) << 9;
    int WDOG_EN = parser.get("WDOG_EN", 1) << 8;
    int INTPOL = parser.get("INTPOL", 0) << 5;
    int INTEN = parser.get("INTEN", 0) << 4;
    int GBCD = parser.get("GBCD", 0) << 1; //0 //don't divide the burst clock
    int BCM = parser.get("BCM", 1); //free-run BCLK
    int EIM_WCR = WDOG_LIMIT|WDOG_EN|INTPOL|INTEN|GBCD|BCM;
    writeKernelMemory(0x21b8090, EIM_WCR, 0, 4);
    printf("EIM_WCR 0x%08X\n", EIM_WCR);

    // EIM_WIAR
    // ACLK_EN = 1
    // EIM_WIAR
    int ACLK_EN = parser.get("ACLK_EN", 1) << 4;
    int ERRST = parser.get("ERRST", 0) << 3;
    int INT = parser.get("INT", 0) << 2;
    int IPS_ACK = parser.get("IPS_ACK", 0) << 1;
    int IPS_REQ = parser.get("IPS_REQ", 0);
    int EIM_WIAR = ACLK_EN|ERRST|INT|IPS_ACK|IPS_REQ;
    writeKernelMemory(0x21b8094, EIM_WIAR, 0, 4);
    printf("EIM_WIAR 0x%08X\n", EIM_WIAR);

    printf( "resetting CS0 space to 64M and enabling 32M CS1 and 32M CS2 space.\n" );
    writeKernelMemory( 0x20e0004, (readKernelMemory(0x20e0004, 0, 4) & 0xFFFFF000) | 0x04B, 0, 4);
    printf("EIM configured\n");
    return 0;
}

void fifo_reset()
{
    short controlRegValue = SPI_read(0x0802);
    SPI_write(0x0802, (controlRegValue & 0x7FFF));
    SPI_write(0x0802, (controlRegValue & 0x7FFF) | 0x8000);
    SPI_write(0x0802, (controlRegValue & 0x7FFF));
}

void fifo_request_data(int src)
{
    short controlRegValue = SPI_read(0x0802);
    SPI_write(0x0802, (controlRegValue & 0xCFFF) | (src << 12));
    controlRegValue = SPI_read(0x0802);
    SPI_write(0x0802, (controlRegValue & 0xBFFF));
    SPI_write(0x0802, (controlRegValue & 0xBFFF) | 0x4000);
    //SPI_write(0x0802, (controlRegValue & 0xBFFF));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    SPI_write(0x0802, (controlRegValue & 0xBFFF));
}

int main(int argc, char** argv)
{
	int bb=0;
	serPort = new LMScomms();
	int devCount = serPort->RefreshDeviceList();
	if(devCount == 0)
    {
        printf("No devices\n");
        return -1;
    }
	cout << "Connecting to " << serPort->GetDeviceList()[0] << endl;
	serPort->Open(0);

    lmsControl = new LMS7002M(serPort);
    if(lmsControl->LoadConfig("test.ini") != LIBLMS7_SUCCESS)
    {
        printf("Failed to load lms7 ini file\n");
        return -2;
    }
    lmsControl->UploadAll();
    printf("LMS7 Configured\n\n");

    prep_eim_burst();

    const int buffer_size = 65536;
    static char buffer[buffer_size] __attribute__((aligned(0x10000)));
    memset(buffer, 0, buffer_size);
    short* shortsBuf = (short*)&buffer;

    fifo_reset();
    fifo_request_data(0);

    const int bytesToRead = 4096;
    //const int bytesToRead = 32*2;
    const int FPGAbufferSize = 32768*2;
    //const int FPGAbufferSize = 32*2;
    for (int bb = 0; bb<FPGAbufferSize; bb += bytesToRead)
    {
        short* currentPage = (short*)(&buffer[bb]);
        char* currentPageChar = (char*)(&buffer[bb]);
        fpga_read(0xC000000, (unsigned short*)&buffer[bb], bytesToRead);
    }

    const short testPattern[] = {0x17ff, 0x0000,
                                 0x1000, 0x07ff,
                                 0x1800, 0x0000,
                                 0x1000, 0x0800
                                 };

    fstream fout;

    if(argc > 1)
    {
        int source;
        stringstream ss;
        ss << argv[1];
        ss >> source;
    if( source == 0)
    {

    fout.open("src0.txt", ios::out);
    fout << "word: read: expected:\n";
    int src0ok = 1;
    char ctemp[128];
    for(int i=0; i<FPGAbufferSize/2; ++i)
    {
        sprintf(ctemp, "%3i: 0x%04X  |  0x%04X\t ", i, shortsBuf[i], testPattern[i%8]);
        if((shortsBuf[i] != testPattern[i%8]))
        {
            sprintf(ctemp+strlen(ctemp), "don't match");
            src0ok = 0;
        }
        sprintf(ctemp+strlen(ctemp), "\n");
        if(i < 24)
            printf("%s", ctemp);
        fout << ctemp;
    }
    fout.close();

    if(src0ok)
        printf("FPGA test pattern OK\n");
    else
        printf("FPGA test pattern FAILED\n");

    memset(buffer, 0, buffer_size);
    }

    if(source == 1)
    {

    //counter test
    fifo_reset();
    fifo_request_data(1);

    for (int bb = 0; bb<FPGAbufferSize; bb += bytesToRead)
    {
        short* currentPage = (short*)(&buffer[bb]);
        char* currentPageChar = (char*)(&buffer[bb]);
        fpga_read(0xC000000, (unsigned short*)&buffer[bb], bytesToRead);
    }

    fout.open("src1.txt", ios::out);
    fout << "word: read: expected:\n";
    int src1ok = 1;
    char ctemp[128];
    int testInt = 0;
    for(int i=0; i<FPGAbufferSize/2; ++i)
    {
        sprintf(ctemp, "%3i: 0x%04X  |  0x%04X\t ", i, shortsBuf[i], testInt);
        if((shortsBuf[i] != testInt))
        {
            sprintf(ctemp+strlen(ctemp), "don't match");
            src1ok = 0;
        }
        sprintf(ctemp+strlen(ctemp), "\n");
        if(i < 24)
            printf("%s", ctemp);
        fout << ctemp;
	if(i==0)
	    continue;
	else
            ++testInt;
    }
    fout.close();

    if(src1ok)
        printf("FPGA counter OK\n");
    else
        printf("FPGA counter FAILED\n");
    memset(buffer, 0, buffer_size);
    }

    if(source == 3)
    {

    //counter test
    fifo_reset();
    fifo_request_data(3);

    for (int bb = 0; bb<FPGAbufferSize; bb += bytesToRead)
    {
        short* currentPage = (short*)(&buffer[bb]);
        char* currentPageChar = (char*)(&buffer[bb]);
        fpga_read(0xC000000, (unsigned short*)&buffer[bb], bytesToRead);
    }

    fout.open("src3.txt", ios::out);
    fout << "word: read: expected:\n";
    char ctemp[128];
    for(int i=0; i<FPGAbufferSize/2; ++i)
    {
        sprintf(ctemp, "%3i: 0x%04X ", i, shortsBuf[i]);
        sprintf(ctemp+strlen(ctemp), "\n");
        if(i < 24)
            printf("%s", ctemp);
        fout << ctemp;
    }
    fout.close();
    memset(buffer, 0, buffer_size);
    }

   /* fifo_reset();
    fifo_request_data(2);

    fout.open("src2.txt", ios::out);
    fout << "word: read: expected:\n";
    int src2ok = 1;
    short testSrc2[] = {0x1fff, 0xfff};
    for(int i=0; i<bytesToRead; ++i)
    {
        sprintf(ctemp, "%3i: 0x%04X  |  0x%04X\t ", i, shortsBuf[i], testSrc2[i%2]);
        fout << ctemp;
        if((shortsBuf[i] != testSrc2[i%2]))
        {
            sprintf(ctemp, "don't match");
            fout << ctemp;
            src2ok = 0;
        }
        fout << endl;
    }
    fout.close();*/
    memset(buffer, 0, buffer_size);

    printf("FPGA reading done\n");
    }
	return 0;
}
