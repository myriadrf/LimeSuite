
#include <cstdlib>
#include <stdio.h>
#include <chrono>
#include "lms7_api.h"
#include "Lms_glWindow.h"
#include <thread>
#include "kiss_fft.h"
using namespace std;

#ifdef NDEBUG
#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#endif
int running = 1;
lms_device_t* device;
Lms_glWindow* win;

int filesize;
unsigned char filebuffer[(1<<27)]={0};

void StreamTest()
{
    int channelsCount = 2;
    float** buffers;
    buffers = new float*[channelsCount];
    for (int i = 0; i < channelsCount; ++i)
        buffers[i] = new float[1024*1024*64];
    uint64_t last_ts = 0;
    LMS_SetStreamingMode(device, LMS_STREAM_MD_SISO | LMS_STREAM_FMT_F32);
    LMS_InitStream(device,LMS_CH_TX,32,4096*2,0);
    LMS_InitStream(device,LMS_CH_RX,32,4096*2,0);
    int totalBytesSent = 0;  
    static auto t1 = chrono::high_resolution_clock::now();
    static auto t2 = chrono::high_resolution_clock::now();
    static auto t3 = chrono::high_resolution_clock::now();
    const int test_count = 9999;//4096*16;
    const int fftSize = 8192;
    kiss_fft_cfg m_fftCalcPlan = kiss_fft_alloc(fftSize, 0, 0, 0);
    kiss_fft_cpx* m_fftCalcIn = new kiss_fft_cpx[fftSize];
    kiss_fft_cpx* m_fftCalcOut = new kiss_fft_cpx[fftSize];
    float fftBins_dbFS[fftSize];
    int time = ~0;//100000000/test_count;
    unsigned a = 0;
    unsigned nco = 0;
    lms_stream_meta_t meta;
    meta.start_of_burst = true;
    meta.end_of_burst = false;
    LMS_RecvStream(device,(void**)buffers,test_count,&meta,0);
    meta.timestamp += 1024*400;
    LMS_SendStream(device,(const void**)buffers,test_count,&meta,0);
    meta.start_of_burst = false;
    //meta.end_of_burst = false;
    while ((time--)&&(running))
    {
        last_ts = meta.timestamp;
        int ret = LMS_RecvStream(device,(void**)buffers,test_count,&meta,100);
        if (ret <= 0)
        {
            printf("rx error\n");
        }
        totalBytesSent += 4*ret;
        meta.timestamp += 1024*400;
        LMS_SendStream(device,(const void**)buffers,test_count,&meta,100);
    
        t2 = chrono::high_resolution_clock::now();
        auto timePeriod = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (timePeriod >= 40)
        {
            
            if (a++ == 20)
            {
            //total number of bytes sent per second
            auto dt =std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t3).count();
            double m_dataRate = 1000.0*totalBytesSent / dt;
            //total number of samples from all channels per second
            
            totalBytesSent = 0;
            printf("Tx rate: %.3f MB/s, TS: %lu (%lu)\n", m_dataRate / 1000000.0,meta.timestamp,meta.timestamp-last_ts);
            for (int i = 0; i<2*test_count;i++)
            {
                if (buffers[0][i]!=buffers[0][i%16])
                {
                    printf("RX sample check failed\n");
                    break;
                    
                }
                if (i == 2*test_count-1)
                printf("RX sample check passed!\n");
            }
            
            a =0;
            t3 = t2;
            }
            t1 = t2;
                for (int i = 0; i < fftSize; ++i)
                {
                    m_fftCalcIn[i].r = buffers[0][2 * i];
                    m_fftCalcIn[i].i = buffers[0][2 * i + 1];
                }
            
             kiss_fft(m_fftCalcPlan, m_fftCalcIn, m_fftCalcOut);
                /*for (int i = 0; i < fftSize; ++i)
                {
                    // normalize FFT results
                    m_fftCalcOut[i].r /= fftSize;
                    m_fftCalcOut[i].i /= fftSize;
                }*/

                int output_index = 0;
                for (int i = fftSize / 2 + 1; i < fftSize; ++i)
                    fftBins_dbFS[output_index++] = sqrt(m_fftCalcOut[i].r * m_fftCalcOut[i].r + m_fftCalcOut[i].i * m_fftCalcOut[i].i);
                for (int i = 0; i < fftSize / 2 + 1; ++i)
                    fftBins_dbFS[output_index++] = sqrt(m_fftCalcOut[i].r * m_fftCalcOut[i].r + m_fftCalcOut[i].i * m_fftCalcOut[i].i);
                for (int s = 0; s < fftSize; ++s)
                    fftBins_dbFS[s] = (fftBins_dbFS[s] != 0 ? (20 * log10(fftBins_dbFS[s])) - 69.2369 : -300);
                   
                win->SetData(fftBins_dbFS,fftSize,-2000,2000);
                //LMS_SetNCOIndex(device,LMS_CH_RX,0,(nco++)&0xF,true);
            }
    }
    kiss_fft_free(m_fftCalcPlan);
}

int error()
{
  printf("ERROR: %s\n",LMS_GetLastErrorMessage());  
  LMS_Close(device);
  exit(-1);  
}


int main(int argc, char** argv)
{
    FILE* file;
    
    win = new Lms_glWindow(0,0,640,480);
    win->show();
    win->SetDisplayLimits(-2500,2500,-200,2);
    lms_info_str_t list[16]={0};
    
    if ((file = fopen("/home/ignas/wcdma.wfm","r"))!=NULL)
    {
       filesize = fread(filebuffer,1,sizeof(filebuffer),file) ;
       printf("%d bytes read\n",filesize);
       fclose(file); 
    }
    else  printf("error opening file\n");
    
    
    
    int n= LMS_GetDeviceList(list);
    float_type f1, f2;
    lms_range_t range;
    printf("%d\n",n);
    if (n > 0)
    {      
        printf("%s\n",list[0]);
        printf("%s\n",list[1]);
        if (LMS_Open(&device,NULL)!=0)
            error();
     
        printf("%d\n",LMS_GetNumChannels(device,LMS_CH_RX));
        
        if ( LMS_Init(device)!=0)
            error();
    
        if (LMS_EnableChannel(device,LMS_CH_RX,0,true)!=0)
            error();
        if (LMS_EnableChannel(device,LMS_CH_TX,0,true)!=0)
            error();
        
        if (LMS_GetSampleRateRange(device,LMS_CH_TX,&range)!=0)
            error();      
        printf("Sample rate range [%1.0f, %1.0f]\n",range.min,range.max);

        if (LMS_SetSampleRate(device,5760000,2)!=0)
            error();
        
        /*if (LMS_SetSampleRateDir(device, LMS_CH_TX,5000000,8)!=0)
            error();
        if (LMS_GetSampleRate(device,LMS_CH_TX,0,&f1,&f2)!=0)
            error();
        printf("TX sample rate %1.0f : RF sample rate %1.0f\n",f1,f2);
        if (LMS_GetSampleRate(device,LMS_CH_RX,0,&f1,&f2)!=0)
            error();
        printf("RX sample rate %1.0f : RF sample rate %1.0f\n",f1,f2);
        
        if (LMS_SetSampleRateDir(device, LMS_CH_RX,5000000,8)!=0)
            error();*/
        
        if (LMS_GetSampleRate(device,LMS_CH_TX,0,&f1,&f2)!=0)
            error();
        printf("TX sample rate %1.0f : RF sample rate %1.0f\n",f1,f2);
        if (LMS_GetSampleRate(device,LMS_CH_RX,0,&f1,&f2)!=0)
            error();
        printf("RX sample rate %1.0f : RF sample rate %1.0f\n",f1,f2);

       
        
        if (LMS_SetLOFrequency(device,LMS_CH_RX, 0, 2600000000)!=0)
            error();
        
        if (LMS_SetLOFrequency(device,LMS_CH_TX, 0, 2600000000)!=0)
            error();
        
       if (LMS_GetLOFrequencyRange(device,LMS_CH_RX, &range)!=0)
            error();
        printf("Center frquency range [%1.0f, %1.0f]\n",range.min,range.max);
        
        if (LMS_GetLOFrequency(device,LMS_CH_RX, 0, &f1)!=0)
            error();
        printf("Center frquency %1.0f\n",f1);
        
       /* if (LMS_GetBWRange(device,LMS_CH_RX, &range)!=0)
            error();
        printf("RF bandwidth range [%1.0f, %1.0f]\n",range.min,range.max);
         
        if (LMS_SetBW(device,LMS_CH_RX, 0, 5000000)!=0)
            error();
        
        if (LMS_GetBW(device,LMS_CH_TX, 0, &f1)!=0)
            error();
        printf("RF bandwidth %1.0f\n",f1);
        
        if (LMS_GetLPFBWRange(device,LMS_CH_RX, &range)!=0)
            error();
        printf("LPF bandwidth range [%1.0f, %1.0f]\n",range.min,range.max);
        
        if (LMS_SetLPFBW(device,LMS_CH_TX, 0, 4000000)!=0)
            error();
        
        if (LMS_SetLPFBW(device,LMS_CH_RX, 0, 4000000)!=0)
            error();
         
        if (LMS_GetLPFBW(device,LMS_CH_RX, 0, &f1)!=0)
            error();
        printf("LPF bandwidth %1.0f\n",f1);*/
        
        if (LMS_SetNormalizedGain(device,LMS_CH_RX,0,0.3)!=0)
            error();
        
        if (LMS_SetNormalizedGain(device,LMS_CH_TX,0,0.07)!=0)
            error();
        
        if (LMS_GetNormalizedGain(device,LMS_CH_TX,0,&f1)!=0)
            error();
        printf("Gain %0.4f\n",f1);
        
        if (LMS_SetTestSignal(device,LMS_CH_RX,0,LMS_TESTSIG_NCODIV8,0,0)!=0)
            error();
        uint16_t val;
        LMS_ReadFPGAReg(device, 0x1F, &val);
        printf("%d\n",val);
        
       /* if (LMS_Calibrate(device,LMS_CH_RX,0,4500000)!=0)
            error();*/
       /*float_type f_nco[16];
        for (int i = 0; i<16;i++)
            f_nco[i]= 100000*i;
        
        if (LMS_SetNCOFrequency(device,LMS_CH_TX,0,f_nco,0.0)!=0)
            error();
        
         if (LMS_SetNCOIndex(device,LMS_CH_TX,0,3,true)!=0)
            error();*/
        
        /*if (LMS_SetGFIRLPF(device, LMS_CH_RX,0,true,2.0e6)!=0)
            error();*/
                           
#if 0         
        if (LMS_SetLPFBW(device,LMS_CH_TX, 0, 5000000)!=0)
            error();
         
        if (LMS_GetLPFBW(device,LMS_CH_RX, 0, &f1)!=0)
            error();
        printf("LPF bandwidth %1.0f\n",f1);
        
        LMS_SetNCOIndex(device,LMS_CH_RX,0,0xF,true);
        double gain;
        LMS_GetNormalizedGain(device,LMS_CH_RX,0,&gain);
        printf("RX G: %f\n",gain);
        LMS_GetNormalizedGain(device,LMS_CH_TX,0,&gain);
        printf("TX G: %f\n",gain);

#endif      
        LMS_VCTCXORead(device,&val);
        printf("%d\n",val);
        std::thread thread = std::thread(StreamTest);
        //StreamTest();
        Fl::run();
        running = 0;
        thread.join();

        if (LMS_Close(device)==0)
            printf("Closed\n");  
    }
   
    return 0;
}

