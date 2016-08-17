//****************************************************************************
//   GOERTZEL ALGORITHM       For complex input data
//****************************************************************************
#include <iostream>
#include <iomanip>
#include <fstream>      // std::ofstream

using namespace std;

/*
https://www.dsprelated.com/showthread/comp.dsp/44939-1.php

The Goertzel gain is N/2, so if the signal has an amplitude of A, then the Goertzel
output has a magnitude of A*N/2. Don't forget sometimes the magnitude^2 is
used, so you now have (A*N/2)^2.

In order for that gain term to be correct, the frequency that you are
looking for needs to be an integer multiple of the sample rate divided by
the block length.  In other words, the block needs to be able to contain an
integer number of cycles of the frequency you are looking for.
*/
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>

static const double PI = 3.14159265359;

// Number of input data points and
// number of spectral points, i.e. number of bins all the interval is divided to
#define SP 65536/16*4
#ifndef NDEBUG
    #define DEBUG_MODE
#endif

int64_t c64to48(int64_t x);

int64_t resize_int(int64_t x, int16_t n)
{
    /*int64_t mask = ~(0xFFFFFFFFFFFFFFFF << (n-1));
    int64_t sign = (x >> (n+1)) & ~mask;
    // print out masks
    /*cout << hex << setw(16) << right << setfill('0') << sign << " | ";
    cout << hex << setw(16) << right << setfill('0') << mask << endl;*/
    return x; //sign + (mask & x);
}

#include <string.h>
void checkForMinMax(int64_t lr, int64_t & lr_min, int64_t & lr_max)
{
    if(lr < lr_min)
        lr_min = lr;
    if(lr > lr_max)
        lr_max = lr;
}
extern int fftBin;

void CalcGoertzelI(int x[][2], int64_t real[], int64_t imag[], int Sp)
{
  const int16_t a = 0;
  const int16_t b = 0;
  const int16_t register_length = 64;
  const int16_t trig_length = 15;
  const int16_t mul_length  = register_length+trig_length-a-b;
  const int16_t add_length  = register_length+1;

  memset(real, Sp, sizeof(int64_t)*Sp);
  memset(imag, Sp, sizeof(int64_t)*Sp);

  #ifdef DEBUG_MODE
      int64_t lr1_min, lr1_max;
      int64_t li1_min, li1_max;
      int64_t lr2_min, lr2_max;
      int64_t li2_min, li2_max;

      lr1_min = LLONG_MAX;
      lr1_max = LLONG_MIN;
      li1_min = LLONG_MAX;
      li1_max = LLONG_MIN;
      lr2_min = LLONG_MAX;
      lr2_max = LLONG_MIN;
      li2_min = LLONG_MAX;
      li2_max = LLONG_MIN;
  #endif
  int16_t c, s;     // cosine and sine 16 bits registers
  int16_t x_n;      // sample (bin)

  int64_t lr1, lr2, li1, li2, temp; // algorithm registers

  int64_t mul;
  float phi, wn;    // algorithm variables for sine and cosine computation
  int n, k;         // loop variables

  // Prepare for computation
  wn = PI / Sp;
  // Loop through all the bins
  //for(k=0; k<Sp; k++)
  /*if(fftBin != 0)
    k = fftBin-1;
  else*/
  k = fftBin;
  {
    lr1 = lr2 = 0;
    li1 = li2 = 0;
    temp = 0;
    mul = 0;
    // Precompute the constants for the current bin
    phi = wn * k;
    c = (cos(phi) * ((1 << (trig_length-1)) ) );
    s = (sin(phi) * ((1 << (trig_length-1)) ) );
    // Emulate data shift
    for(n=0; n<Sp; n++)
    {
      // first channel
      x_n = x[n][0];

      temp = resize_int(lr1, register_length);

      mul  = (c >> a) * (lr1 >> b);
      mul  = resize_int(mul, mul_length);
      mul  = (mul >> (trig_length-a-b-1)) << 1;

      lr1 = resize_int(mul, add_length) - resize_int(lr2, add_length) + resize_int(x_n, add_length);
      lr2 = temp;
      #ifdef DEBUG_MODE
        checkForMinMax(lr1, lr1_min, lr1_max);
        checkForMinMax(lr2, lr2_min, lr2_max);
      #endif
      lr1 = resize_int(lr1, register_length);

      // second channel
      x_n = x[n][1];

      temp = resize_int(li1, register_length);

      mul  = (c >> a) * (li1 >> b);
      mul  = resize_int(mul, mul_length);
      mul  = (mul >> (trig_length-a-b-1)) << 1;

      li1 = resize_int(mul, add_length) - resize_int(li2, add_length) + resize_int(x_n, add_length);
      li2 = temp;
      #ifdef DEBUG_MODE
        checkForMinMax(li1, li1_min, li1_max);
        checkForMinMax(li2, li2_min, li2_max);
      #endif
      li1 = resize_int(li1, register_length);
    }
    real[k] = (int64_t)( resize_int( resize_int( (c >> a)*(lr1 >> b), mul_length) >> (trig_length-a-b-1), add_length) - resize_int(lr2, add_length) - resize_int( resize_int((s >> a) * (li1 >> b), mul_length )>> (trig_length-a-b-1), add_length) );
    imag[k] = (int64_t)( resize_int( resize_int( (c >> a)*(li1 >> b), mul_length) >> (trig_length-a-b-1), add_length) - resize_int(li2, add_length) + resize_int( resize_int((s >> a) * (lr1 >> b), mul_length )>> (trig_length-a-b-1), add_length) );
  }
  #ifdef DEBUG_MODE
    /*
    cout << lr1_min << " | " << lr1_max << endl;
    cout << li1_min << " | " << lr1_max << endl;
    cout << lr2_min << " | " << lr1_max << endl;
    cout << li2_min << " | " << lr1_max << endl;
    */
  #endif
};
