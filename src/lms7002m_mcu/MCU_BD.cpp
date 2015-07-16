/**
@file	MCU_BD.cpp
@author	Lime Microsystems
@brief  Implementation of interacting with on board MCU
*/

#include "MCU_BD.h"
using namespace std;
#include <string>
#include "MCU_File.h"
#include <sstream>
#include <fstream>
#include "lmsComms.h"
#include <assert.h>

MCU_BD::MCU_BD()
{
    stepsTotal = 0;
    stepsDone = 0;
    aborted = false;
    //ctor
    int i=0;
    m_serPort=NULL;
    //default value,
    //must be verified during program exploatation
    m_iLoopTries=20;
    // array initiallization
    for (i=0; i<=255; i++){
			m_SFR[i]=0x00;
			m_IRAM[i]=0x00;
	}
	for (i=0; i<8192; i++){
	    byte_array[i]=0x00;
    };
}

MCU_BD::~MCU_BD()
{
    //dtor
}

void MCU_BD::Initialize(LMScomms* pSerPort)
{
    m_serPort = pSerPort;
}

void MCU_BD:: GetProgramCode(const char* inFileName, bool bin)
{
    unsigned char ch=0x00;
    bool find_byte=false;
    int i=0;

    if(!bin)
    {
        MCU_File	inFile(inFileName, "rb");
        inFile.ReadHex(8191);

        for (i=0; i<8192; i++)
        {
            find_byte=inFile.GetByte(i, ch);
            if (find_byte==true)
                byte_array[i]=ch;
            else
                byte_array[i]=0x00;
        };
    }
    else
    {
        char inByte = 0;
        fstream fin;
        fin.open(inFileName, ios::in | ios::binary);
        memset(byte_array, 0, 8192);
        for(int i=0; i<8192 && !fin.eof(); ++i)
        {
            inByte = 0;
            fin.read(&inByte, 1);
            byte_array[i]=inByte;
        }
    }
}

void MCU_BD:: mSPI_write(
            unsigned short addr_reg,  // takes 16 bit address
            unsigned short data_reg)  // takes 16 bit value
{
    assert(m_serPort != nullptr);
    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_LMS7002_WR;
    pkt.outBuffer.push_back((addr_reg >> 8) & 0xFF);
    pkt.outBuffer.push_back(addr_reg & 0xFF);
    pkt.outBuffer.push_back((data_reg >> 8) & 0xFF);
    pkt.outBuffer.push_back(data_reg & 0xFF);
    if (m_serPort->IsOpen() == true)
        m_serPort->TransferPacket(pkt);
}


unsigned short MCU_BD:: mSPI_read(
            unsigned short addr_reg)  // takes 16 bit address
{// returns 16 bit value
    assert(m_serPort != nullptr);
    if (m_serPort->IsOpen()==true)
    {
        LMScomms::GenericPacket pkt;
        pkt.cmd = CMD_LMS7002_RD;
        pkt.outBuffer.push_back((addr_reg >> 8) & 0xFF);
        pkt.outBuffer.push_back(addr_reg & 0xFF);
        if (m_serPort->TransferPacket(pkt) == LMScomms::TRANSFER_SUCCESS)
            if (pkt.status == STATUS_COMPLETED_CMD)
                return pkt.inBuffer[2] * 256 | pkt.inBuffer[3];
    }
    return 0x0000;
}

int MCU_BD::WaitUntilWritten(){

	 // waits if WRITE_REQ (REG3[2]) flag is equal to '1'
	 // this means that  write operation is in progress
	unsigned short tempi=0x0000;
	int countDown=m_iLoopTries;  // Time out value
	tempi=mSPI_read(0x0003); // REG3 read

	while (( (tempi&0x0004) == 0x0004) && (countDown>0))
    {
		tempi=mSPI_read(0x0003); // REG3 read
		countDown--;
	}
	if (countDown==0)
        return -1; // an error occured, timer elapsed
	else
        return 0; // Finished regularly
	// pass if WRITE_REQ is '0'
}

int MCU_BD::ReadOneByte(unsigned char * data)
{
	unsigned short tempi=0x0000;
	int countDown=m_iLoopTries;

     // waits when READ_REQ (REG3[3]) flag is equal to '0'
	 // this means that there is nothing to read
	tempi=mSPI_read(0x0003); // REG3 read

    while (((tempi&0x0008)==0x0000) && (countDown>0))
    {
        // wait if READ_REQ is '0'
		tempi=mSPI_read(0x0003); // REG3 read
		countDown--;
	}

	if (countDown>0)
    { // Time out has not occured
		 tempi=mSPI_read(0x0005); // REG5 read
		  // return the read byte
		 (* data) = (unsigned char) (tempi);
	}
	else
        (* data) =0;
	 // return the zero, default value

	if (countDown==0)
        return -1; // an error occured
	else
        return 0; // finished regularly
}

int MCU_BD::One_byte_command(unsigned short data1, unsigned char * rdata1)
{
	unsigned char tempc=0x00;
	int retval=0;
	*rdata1=0x00; //default return value

	// sends the one byte command
	mSPI_write(0x8004, data1); //REG4 write
	retval=WaitUntilWritten();
	if (retval==-1) return -1;
	// error if operation executes too long

    // gets the one byte answer
	retval=ReadOneByte(&tempc);
    if (retval==-1) return -1;
	// error if operation takes too long

	*rdata1=tempc;
	return 0;
}

int MCU_BD::Three_byte_command(
	    unsigned char data1,unsigned char data2,unsigned char data3,
		unsigned char * rdata1,unsigned char * rdata2,unsigned char * rdata3){


		int retval=0;
        *rdata1=0x00;
		*rdata2=0x00;
		*rdata3=0x00;

		mSPI_write(0x8004, (unsigned short)(data1)); //REG4 write
		retval=WaitUntilWritten();
		if (retval==-1) return -1;

		mSPI_write(0x8004, (unsigned short)(data2)); //REG4 write
		retval=WaitUntilWritten();
		if (retval==-1) return -1;

		mSPI_write(0x8004, (unsigned short)(data3)); //REG4 write
		retval=WaitUntilWritten();
		if (retval==-1) return -1;

		retval= ReadOneByte(rdata1);
		if (retval==-1) return -1;

		retval= ReadOneByte(rdata2);
		if (retval==-1) return -1;

		retval= ReadOneByte(rdata3);
		if (retval==-1) return -1;

		return 0;
}

int MCU_BD::Change_MCUFrequency(unsigned char data) {

	unsigned char tempc1, tempc2, tempc3=0x00;
	int retval=0;
    // code 0x7E is for writing the SFR registers
	retval=Three_byte_command(0x7E, 0x8E, data, &tempc1, &tempc2, &tempc3);
	// PMSR register, address 0x8E
	return retval;
}

int MCU_BD::Read_IRAM()
{
	unsigned char tempc1, tempc2, tempc3=0x00;
	int i=0;
	int retval=0;

	//default
	//IRAM array initialization
	for (i=0; i<=255; i++)
		m_IRAM[i]=0x00;

    stepsTotal.store(256);
    stepsDone.store(0);
    aborted.store(false);
	for (i=0; i<=255; i++)
    {
        // code 0x78 is for reading the IRAM locations
		retval=Three_byte_command(0x78, ((unsigned char)(i)), 0x00,&tempc1, &tempc2, &tempc3);
		if (retval==0)
            m_IRAM[i]=tempc3;
		else
        {
            i=256; // error, stop
            aborted.store(true);
        }
        ++stepsDone;
#ifndef NDEBUG
        printf("MCU reading IRAM: %2i/256\r", stepsDone.load());
#endif
        Wait_CLK_Cycles(64);
	}
#ifndef NDEBUG
    printf("\nMCU reading IRAM finished\n");
#endif
	return retval;
}

int MCU_BD::Erase_IRAM()
{
	unsigned char tempc1, tempc2, tempc3=0x00;
	int retval=0;
	int i=0;

	//default ini.
	for (i=0; i<=255; i++)
			m_IRAM[i]=0x00;

    stepsTotal.store(256);
    stepsDone.store(0);
    aborted.store(false);
	for (i=0; i<=255; i++)
    {
			m_IRAM[i]=0x00;
            // code 0x7C is for writing the IRAM locations
			retval=Three_byte_command(0x7C, ((unsigned char)(i)), 0x00,&tempc1, &tempc2, &tempc3);
            if (retval == -1)
            {
                i = 256;
                aborted.store(true);
            }
            ++stepsDone;
#ifndef NDEBUG
            printf("MCU erasing IRAM: %2i/256\r", stepsDone.load());
#endif
	}
#ifndef NDEBUG
    printf("\nMCU erasing IRAM finished\n");
#endif
	return retval;
}

int MCU_BD::Read_SFR()
{
    int i=0;
	unsigned char tempc1, tempc2, tempc3=0x00;
	int retval=0;

    stepsTotal.store(48);
    stepsDone.store(0);
    aborted.store(false);

	//default m_SFR array initialization
	for (i=0; i<=255; i++)
			m_SFR[i]=0x00;

	// code 0x7A is for reading the SFR registers
	retval=Three_byte_command(0x7A, 0x80, 0x00, &tempc1, &tempc2, &tempc3); // P0
	if (retval==-1) return -1;
	m_SFR[0x80]=tempc3;

	retval=Three_byte_command(0x7A, 0x81, 0x00, &tempc1, &tempc2, &tempc3); // SP
	if (retval==-1) return -1;
	m_SFR[0x81]=tempc3;

	retval=Three_byte_command(0x7A, 0x82, 0x00, &tempc1, &tempc2, &tempc3); // DPL0
	if (retval==-1) return -1;
	m_SFR[0x82]=tempc3;

	retval=Three_byte_command(0x7A, 0x83, 0x00, &tempc1, &tempc2, &tempc3); // DPH0
	if (retval==-1) return -1;
	m_SFR[0x83]=tempc3;

	retval=Three_byte_command(0x7A, 0x84, 0x00, &tempc1, &tempc2, &tempc3); // DPL1
	if (retval==-1) return -1;
	m_SFR[0x84]=tempc3;

	retval=Three_byte_command(0x7A, 0x85, 0x00, &tempc1, &tempc2, &tempc3); // DPH1
	if (retval==-1) return -1;
	m_SFR[0x85]=tempc3;

    stepsDone.store(6);

	retval=Three_byte_command(0x7A, 0x86, 0x00, &tempc1, &tempc2, &tempc3); // DPS
	if (retval==-1) return -1;
	m_SFR[0x86]=tempc3;

	retval=Three_byte_command(0x7A, 0x87, 0x00, &tempc1, &tempc2, &tempc3); // PCON
	if (retval==-1) return -1;
	m_SFR[0x87]=tempc3;

	retval=Three_byte_command(0x7A, 0x88, 0x00, &tempc1, &tempc2, &tempc3); // TCON
	if (retval==-1) return -1;
	m_SFR[0x88]=tempc3;

	retval=Three_byte_command(0x7A, 0x89, 0x00, &tempc1, &tempc2, &tempc3); // TMOD
	if (retval==-1) return -1;
	m_SFR[0x89]=tempc3;

	retval=Three_byte_command(0x7A, 0x8A, 0x00, &tempc1, &tempc2, &tempc3); // TL0
	if (retval==-1) return -1;
	m_SFR[0x8A]=tempc3;

	retval=Three_byte_command(0x7A, 0x8B, 0x00, &tempc1, &tempc2, &tempc3); // TL1
	if (retval==-1) return -1;
	m_SFR[0x8B]=tempc3;

    stepsDone.store(12);

	retval=Three_byte_command(0x7A, 0x8C, 0x00, &tempc1, &tempc2, &tempc3); // TH0
	if (retval==-1) return -1;
	m_SFR[0x8C]=tempc3;

	retval=Three_byte_command(0x7A, 0x8D, 0x00, &tempc1, &tempc2, &tempc3); // TH1
	if (retval==-1) return -1;
    m_SFR[0x8D]=tempc3;

	retval=Three_byte_command(0x7A, 0x8E, 0x00, &tempc1, &tempc2, &tempc3); // PMSR
	if (retval==-1) return -1;
	m_SFR[0x8E]=tempc3;

	retval=Three_byte_command(0x7A, 0x90, 0x00, &tempc1, &tempc2, &tempc3); // P1
	if (retval==-1) return -1;
	m_SFR[0x90]=tempc3;

	retval=Three_byte_command(0x7A, 0x91, 0x00, &tempc1, &tempc2, &tempc3); // DIR1
	if (retval==-1) return -1;
	m_SFR[0x91]=tempc3;

	retval=Three_byte_command(0x7A, 0x98, 0x00, &tempc1, &tempc2, &tempc3); // SCON
	if (retval==-1) return -1;
	m_SFR[0x98]=tempc3;

    stepsDone.store(18);

	retval=Three_byte_command(0x7A, 0x99, 0x00, &tempc1, &tempc2, &tempc3); // SBUF
	if (retval==-1) return -1;
	m_SFR[0x99]=tempc3;

	retval=Three_byte_command(0x7A, 0xA0, 0x00, &tempc1, &tempc2, &tempc3); // P2
	if (retval==-1) return -1;
	m_SFR[0xA0]=tempc3;

	retval=Three_byte_command(0x7A, 0xA1, 0x00, &tempc1, &tempc2, &tempc3); // DIR2
	if (retval==-1) return -1;
	m_SFR[0xA1]=tempc3;

	retval=Three_byte_command(0x7A, 0xA2, 0x00, &tempc1, &tempc2, &tempc3); // DIR0
	if (retval==-1) return -1;
	m_SFR[0xA2]=tempc3;

	retval=Three_byte_command(0x7A, 0xA8, 0x00, &tempc1, &tempc2, &tempc3); // IEN0
	if (retval==-1) return -1;
	m_SFR[0xA8]=tempc3;

    stepsDone.store(24);

	retval=Three_byte_command(0x7A, 0xA9, 0x00, &tempc1, &tempc2, &tempc3); // IEN1
	if (retval==-1) return -1;
	m_SFR[0xA9]=tempc3;

	retval=Three_byte_command(0x7A, 0xB0, 0x00, &tempc1, &tempc2, &tempc3); // EECTRL
	if (retval==-1) return -1;
    m_SFR[0xB0]=tempc3;

	retval=Three_byte_command(0x7A, 0xB1, 0x00, &tempc1, &tempc2, &tempc3); // EEDATA
	if (retval==-1) return -1;
	m_SFR[0xB1]=tempc3;

	retval=Three_byte_command(0x7A, 0xB8, 0x00, &tempc1, &tempc2, &tempc3); // IP0
	if (retval==-1) return -1;
	m_SFR[0xB8]=tempc3;

	retval=Three_byte_command(0x7A, 0xB9, 0x00, &tempc1, &tempc2, &tempc3); // IP1
	if (retval==-1) return -1;
	m_SFR[0xB9]=tempc3;

	retval=Three_byte_command(0x7A, 0xBF, 0x00, &tempc1, &tempc2, &tempc3); // USR2
	if (retval==-1) return -1;
	m_SFR[0xBF]=tempc3;

    stepsDone.store(30);

	retval=Three_byte_command(0x7A, 0xC0, 0x00, &tempc1, &tempc2, &tempc3); // IRCON
	if (retval==-1) return -1;
	m_SFR[0xC0]=tempc3;

	retval=Three_byte_command(0x7A, 0xC8, 0x00, &tempc1, &tempc2, &tempc3); // T2CON
	if (retval==-1) return -1;
	m_SFR[0xC8]=tempc3;

	retval=Three_byte_command(0x7A, 0xCA, 0x00, &tempc1, &tempc2, &tempc3); // RCAP2L
	if (retval==-1) return -1;
	m_SFR[0xCA]=tempc3;

	retval=Three_byte_command(0x7A, 0xCB, 0x00, &tempc1, &tempc2, &tempc3); // RCAP2H
	if (retval==-1) return -1;
	m_SFR[0xCB]=tempc3;

	retval=Three_byte_command(0x7A, 0xCC, 0x00, &tempc1, &tempc2, &tempc3); // TL2
	if (retval==-1) return -1;
	m_SFR[0xCC]=tempc3;

	retval=Three_byte_command(0x7A, 0xCD, 0x00, &tempc1, &tempc2, &tempc3); // TH2
	if (retval==-1) return -1;
	m_SFR[0xCD]=tempc3;

    stepsDone.store(36);

	retval=Three_byte_command(0x7A, 0xD0, 0x00, &tempc1, &tempc2, &tempc3); // PSW
	if (retval==-1) return -1;
    m_SFR[0xD0]=tempc3;

	retval=Three_byte_command(0x7A, 0xE0, 0x00, &tempc1, &tempc2, &tempc3); // ACC
	if (retval==-1) return -1;
    m_SFR[0xE0]=tempc3;

	retval=Three_byte_command(0x7A, 0xF0, 0x00, &tempc1, &tempc2, &tempc3); // B
	if (retval==-1) return -1;
	m_SFR[0xF0]=tempc3;

	retval=Three_byte_command(0x7A, 0xEC, 0x00, &tempc1, &tempc2, &tempc3); // REG0
	if (retval==-1) return -1;
    m_SFR[0xEC]=tempc3;

	retval=Three_byte_command(0x7A, 0xED, 0x00, &tempc1, &tempc2, &tempc3); // REG1
	if (retval==-1) return -1;
	m_SFR[0xED]=tempc3;

	retval=Three_byte_command(0x7A, 0xEE, 0x00, &tempc1, &tempc2, &tempc3); // REG2
	if (retval==-1) return -1;
	m_SFR[0xEE]=tempc3;

    stepsDone.store(42);

	retval=Three_byte_command(0x7A, 0xEF, 0x00, &tempc1, &tempc2, &tempc3); // REG3
	if (retval==-1) return -1;
	m_SFR[0xEF]=tempc3;

	retval=Three_byte_command(0x7A, 0xF4, 0x00, &tempc1, &tempc2, &tempc3); // REG4
	if (retval==-1) return -1;
	m_SFR[0xF4]=tempc3;

	retval=Three_byte_command(0x7A, 0xF5, 0x00, &tempc1, &tempc2, &tempc3); // REG5
	if (retval==-1) return -1;
	m_SFR[0xF5]=tempc3;

	retval=Three_byte_command(0x7A, 0xF6, 0x00, &tempc1, &tempc2, &tempc3); // REG6
	if (retval==-1) return -1;
	m_SFR[0xF6]=tempc3;

	retval=Three_byte_command(0x7A, 0xF7, 0x00, &tempc1, &tempc2, &tempc3); // REG7
	if (retval==-1) return -1;
	m_SFR[0xF7]=tempc3;

	retval=Three_byte_command(0x7A, 0xFC, 0x00, &tempc1, &tempc2, &tempc3); // REG8
	if (retval==-1) return -1;
	m_SFR[0xFC]=tempc3;

	retval=Three_byte_command(0x7A, 0xFD, 0x00, &tempc1, &tempc2, &tempc3); // REG9
	if (retval==-1) return -1;
	m_SFR[0xFD]=tempc3;

    stepsDone.store(48);

	return 0;
}

void MCU_BD::Wait_CLK_Cycles(int delay)
{
	//// some delay
	int i=0;
	for (i=0;i<(delay/64);i++)
           mSPI_read(0x0003);
}

int MCU_BD::Program_MCU(int m_iMode1, int m_iMode0)
{
    bool success = true;
	unsigned short tempi=0x0000;
	unsigned short tempi2=0x0000;
	int CntEnd=0;
	int i=0;
    int countDown=m_iLoopTries;
    int m_iExt2=0;

	if ((m_iMode1==0)&&(m_iMode0==0)) return 0;
    // MCU is in reset state
    // the programming mode should be selected first

	tempi2=0x0000;  // was 0x0000
	if (m_iExt2==1)  tempi2=tempi2|0x0004;
	if (m_iMode1==1) tempi2=tempi2|0x0002;
	if (m_iMode0==1) tempi2=tempi2|0x0001;


	tempi=0x0000;  // was 0x0000
	if (m_iExt2==1)  tempi=tempi|0x0004;
	if (m_iMode1==1) tempi=tempi|0x0002;
	if (m_iMode0==1) tempi=tempi|0x0001;

	//mSPI_write(0x8002, tempi);
	// REG2 write
	// selects programming mode
    CntEnd=0;
    int packetNumber = 0;
    int status = STATUS_UNDEFINED;

    LMScomms::GenericPacket pkt;
    pkt.cmd = CMD_PROG_MCU;

    stepsTotal.store(8192);
    stepsDone.store(0);
    aborted.store(false);

	while  (CntEnd<8192)
    {
		//tempi=mSPI_read(0x0003);
		//REG3 read
		//if ((tempi&0x0001)==0x0001){ // Flag EmptyFIFO==1

        pkt.outBuffer.clear();
        pkt.outBuffer.push_back(tempi);
        pkt.outBuffer.push_back(packetNumber++);
        for (i=0; i<32; i++)
        {
            pkt.outBuffer.push_back(byte_array[CntEnd + i]);
            //mSPI_write(0x8004, (unsigned short) (byte_array[CntEnd+i]));
            // REG4 write
        }

        m_serPort->TransferPacket(pkt);
        status = pkt.status;
        stepsDone.store(stepsDone.load() + 32);
#ifndef NDEBUG
        printf("MCU programming : %4i/%4i\r", stepsDone.load(), stepsTotal.load());
#endif

        if(status != STATUS_COMPLETED_CMD)
        {
            stringstream ss;
            ss << "Programing MCU: status : not completed, block " << packetNumber << endl;
            success = false;
            aborted.store(true);
            break;
        }

        if((m_iMode0 == 1) && (m_iMode1 == 1)) // if boot mode , send only first packet
        {
            stepsDone.store(1);
            stepsTotal.store(1);
            break;
        }

        CntEnd+=32;
        countDown=m_iLoopTries;

        //if (CntEnd==32*128) {
        //    mSPI_write(0x8002, tempi2);
        //}
        //Wait_CLK_Cycles(256);
	};

	/*tempi=mSPI_read(0x0003);
	// REG3 read

	while (((tempi&0x0040)== 0x0000)&&(countDown>0)) {
    // wait if bit Programmed=='0'
		tempi=mSPI_read(0x0003);  // REG3 read
		countDown--;
	}*/

	//if (countDown==0) return -1;
	// an error occured during programming
	//else return 0; // programming successful
#ifndef NDEBUG
    printf("\nMCU programming Finished\n");
#endif
	if(success)
        Log("PROGRAMMING MCU SUCCESS\n");
	return 0;
}

void MCU_BD::Reset_MCU()
{
    unsigned short tempi=0x0000;  // was 0x0000
	mSPI_write(0x8002, tempi);
	tempi=0x0000;
	mSPI_write(0x8000, tempi);
}

void MCU_BD::RunTest_MCU(int m_iMode1, int m_iMode0, unsigned short test_code, int m_iDebug) {

	int i=0;
	int limit=0;
	unsigned short tempi=0x0000;
	unsigned short basei=0x0000;

	if  (test_code<=14) basei=(test_code<<4);
	else basei=0x0000;

	basei=basei&0xFFF0; // not necessery
	// 4 LSBs are zeros

	// variable basei contains test no. value at bit positions 7-4
	// used for driving the P0 input
	// P0 defines the test no.

	if ((test_code>7)||(test_code==0))
        limit=1;
	else
        limit=50;

	// tests 8 to 14 have short duration

	if (m_iDebug==1) return; // normal MCU operating mode required

	// EXT_INT2=1, external interrupt 2 is raised
	tempi=0x0000;  // changed
	int m_iExt2=1;

	if (m_iExt2==1)  tempi=tempi|0x0004;
	if (m_iMode1==1) tempi=tempi|0x0002;
	if (m_iMode0==1) tempi=tempi|0x0001;

	// tempi variable is driving the mspi_REG2

	mSPI_write(0x8002, tempi); // REG2 write

	// generating waveform
	for (i=0; i<=limit; i++)
    {
		tempi=basei|0x000C;
		mSPI_write(0x8000, tempi);
		// REG0 write
		Wait_CLK_Cycles(256);
        tempi=basei|0x000D;
		mSPI_write(0x8000, tempi);
		// REG0 write  - P0(0) set
		Wait_CLK_Cycles(256);
		tempi=basei|0x000C;
		mSPI_write(0x8000, tempi);
		// REG0 write
		Wait_CLK_Cycles(256);
		tempi=basei|0x000E;
		mSPI_write(0x8000, tempi);
		// REG0 write - PO(1) set
		Wait_CLK_Cycles(256);

		if (i==0) {
            // EXT_INT2=0
            // external interrupt 2 is pulled down
			tempi=0x0000; // changed
			m_iExt2=0;
			if (m_iExt2==1)  tempi=tempi|0x0004;
			if (m_iMode1==1) tempi=tempi|0x0002;
			if (m_iMode0==1) tempi=tempi|0x0001;
			mSPI_write(0x8002, tempi);
            // REG2 write
		}
	}
}


void MCU_BD::RunFabTest_MCU(int m_iMode1, int m_iMode0, int m_iDebug) {

	unsigned short tempi=0x0000;

  	if (m_iDebug==1) return; // normal MCU operating mode required

	// EXT_INT2=1, external interrupt 2 is raised
	tempi=0x0000;  // changed
	int m_iExt2=1;
	if (m_iExt2==1)  tempi=tempi|0x0004;
	if (m_iMode1==1) tempi=tempi|0x0002;
	if (m_iMode0==1) tempi=tempi|0x0001;
	mSPI_write(0x8002, tempi); // REG2 write

	Wait_CLK_Cycles(256);

	// EXT_INT2=0, external interrupt 2 is pulled down
	tempi=0x0000; // changed
    m_iExt2=0;
	if (m_iExt2==1)  tempi=tempi|0x0004;
	if (m_iMode1==1) tempi=tempi|0x0002;
	if (m_iMode0==1) tempi=tempi|0x0001;
	mSPI_write(0x8002, tempi);

	Wait_CLK_Cycles(256);

}

void MCU_BD::DebugModeSet_MCU(int m_iMode1, int m_iMode0)
{
        unsigned short tempi=0x00C0;
        // bit DEBUG is set
		int m_iExt2=0;
		if (m_iExt2==1)  tempi=tempi|0x0004;
		if (m_iMode1==1) tempi=tempi|0x0002;
		if (m_iMode0==1) tempi=tempi|0x0001;

		// Select debug mode
		mSPI_write(0x8002, tempi);
		// REG2 write
}

 void MCU_BD::DebugModeExit_MCU(int m_iMode1, int m_iMode0)
{

        unsigned short tempi=0x0000; // bit DEBUG is zero
		int m_iExt2=0;

		if (m_iExt2==1)  tempi=tempi|0x0004;
		if (m_iMode1==1) tempi=tempi|0x0002;
		if (m_iMode0==1) tempi=tempi|0x0001;
		// To run mode
		mSPI_write(0x8002, tempi);  // REG2 write
}

int MCU_BD::ResetPC_MCU()
{
     unsigned char tempc1=0x00;
     int retval=0;
     retval=One_byte_command(0x70, &tempc1);
     return retval;
}

int MCU_BD::RunInstr_MCU(unsigned short * pPCVAL)
{
    unsigned char tempc1, tempc2, tempc3=0x00;
    int retval=0;
    retval=Three_byte_command(0x74, 0x00, 0x00, &tempc1, &tempc2, &tempc3);
	if (retval==-1) (*pPCVAL)=0;
	else (*pPCVAL)=tempc2*256+tempc3;
	return retval;
}

void MCU_BD::Log(const char* msg)
{
    printf("%s", msg);
}

/** @brief Returns information about programming or reading data progress
*/
MCU_BD::ProgressInfo MCU_BD::GetProgressInfo() const
{
    ProgressInfo info;
    info.stepsDone = stepsDone.load();
    info.stepsTotal = stepsTotal.load();
    info.aborted = aborted.load();
    return info;
}
