/**
 * @file lime/LimeSuite.h
 *
 * @brief LMS API library
 *
 * Copyright (C) 2016 Lime Microsystems
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef LMS7_API_H
#define LMS7_API_H

#include <stdint.h>
#include <stdlib.h>
#include "LMS7002M_parameters.h"

#ifdef __cplusplus
extern "C" {
#else
/* stdbool.h is not applicable for C++ programs, as the language inherently
 * provides the bool type.
 *
 * Users of Visual Studio 2012 and earlier will need to supply a stdbool.h
 * implementation, as it is not included with the toolchain. Visual Studio 2013
 * onward supplies this header.
 */
#include <stdbool.h>
#endif
    
#if defined _WIN32 || defined __CYGWIN__
#   include <windows.h>
#   define CALL_CONV __cdecl
#   ifdef __GNUC__
#       define API_EXPORT __attribute__ ((dllexport))
#   else
#       define API_EXPORT __declspec(dllexport)
#   endif
#elif defined _DOXYGEN_ONLY_
    /** Marks an API routine to be made visible to the dynamic loader.
     *  This is OS and/or compiler-specific. */
#   define API_EXPORT
    /** Specifies calling convention, if necessary.
     *  This is OS and/or compiler-specific. */
#   define CALL_CONV
#else
#   define API_EXPORT __attribute__ ((visibility ("default")))
#   define CALL_CONV
#endif
    
/**Floating point data type*/
typedef double float_type;

/**convenience constant for good return code*/
static const int LMS_SUCCESS = 0;

/**
 * @defgroup FN_INIT    Initialization/deinitialization
 *
 * The functions in this section provide the ability to query available devices,
 * initialize them, and deinitialize them.
 * @{
 */

/**LMS Device handle */
typedef void lms_device_t;

/**Convenience type for fixed length LMS Device information string*/  
typedef char lms_info_str_t[256];

/**
 * Obtain a list of LMS devices attached to the system
 *
 * @param[out]  dev_list    List of available devices
 *
 * @return      number of devices in the list on success, (-1) on failure
 */  
API_EXPORT int CALL_CONV LMS_GetDeviceList(lms_info_str_t *dev_list);

/**
 * Opens device specified by the provided ::lms_dev_info string
 *
 * This function should be used to open a device based upon the results of
 * LMS_GetDeviceList()
 *
 * @param[out]  device      Updated with device handle on success
 * @param[in]   info        Device information string. If NULL, the first
 *                          available device will be opened.
 * @param[in]   args        additional arguments. Can be NULL. 
 *
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_Open(lms_device_t **device, lms_info_str_t info,
                                   void* args);

/**
 * Close device
 *
 * @note     Failing to close a device will result in memory leaks.
 *
 * @post     device is deallocated and may no longer be used.
 *
 * @param    device  Device handle previously obtained by LMS_Open(). This
 *              function does nothing  and returns (-1) if device is NULL.
 * 
 * @return   0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_Close(lms_device_t *device);


/**
 * Disconnect device but keep configuration cache (device is not deallocated).
 *
 * @param   device  Device handle previously obtained by LMS_Open(). This
 *                  function does nothing  and returns (-1) if device is NULL.
 * 
 * @return   0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_Disconnect(lms_device_t *device);

/**
 * Checks if device port is opened
 *
 * @param   device  Device handle previously obtained by LMS_Open(). This
 *                  function does nothing  and returns (-1) if device is NULL.
 * @param   port    0 - control port, 1- data port
 * 
 * @return   true(1) if port is open, false (0) if - closed
 */
API_EXPORT bool CALL_CONV LMS_IsOpen(lms_device_t *device, int port);


/** @} (End FN_INIT) */



/**
 * @defgroup FN_HIGH_LVL    High-level control functions
 *
 * The functions in this section provide the ability to easily configure the
 * device for operation. 
 * 
 * @{
 */

/**Convenience constants for TX/RX selection*/
const bool LMS_CH_TX = true;
const bool LMS_CH_RX = false;

/**Structure used to represent the allowed value range of various parameters*/
typedef struct
{
    float_type min;     /**<Minimum allowed value*/
    float_type max;     /**<Minimum allowed value*/
    float_type step;    /**<Minimum value step*/
}lms_range_t;

/**Enumeration of loopback modes*/
typedef enum {
    LMS_LOOPBACK_NONE   /**<Return to normal operation (disable loopback)*/
} lms_loopback_t;


/**Enumeration of LMS7 TEST signal types*/
typedef enum
{
    LMS_TESTSIG_NONE=0,     /**<Disable test signals. Return to normal operation*/
    LMS_TESTSIG_NCODIV8,    /**<Test signal from NCO*/
    LMS_TESTSIG_NCODIV4,    /**<Test signal from NCO*/
    LMS_TESTSIG_DC          /**<DC test signal*/  
}lms_testsig_t;

/**
 * Configure LMS chip with settings that make it ready for operation.
 *
 * @note This configuration differs from default LMS chip configuration which is
 * described in chip datasheet. In order to load default chip configuration use
 * LMS_Reset().
 * 
 * @param[in]   device  Device handle previously obtained by LMS_Open().
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_Init(lms_device_t *device);

/**
 * Obtain number of RX or TX channels. Use this to determine the maximum 
 * channel index (specifying channel index is required by most API functions).
 * The maximum channel index is N-1, where N is number returned by this function
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param dir_tx    Select RX or TX
 * 
 * @return          Number of channels on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetNumChannels(lms_device_t *device, bool dir_tx);

/**
 * Enable or disable specified RX channel.
 *
 * @param[in]   device      Device handle previously obtained by LMS_Open().
 * @param       dir_tx      Select RX or TX 
 * @param       chan        Channel index
 * @param       enabled     true(1) to enable, false(0) to disable.
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_EnableChannel(lms_device_t *device, bool dir_tx,
                                           size_t chan, bool enabled);

/**
 * Set sampling rate for all RX/TX channels. Sample rate is in complex samples
 * (1 sample = I + Q). The function sets sampling rate that is used for data
 * exchange with the host. It also allows to specify higher sampling rate to be 
 * used in RF by setting oversampling ratio. Valid oversampling values are 1, 2,
 * 4, 8, 16, 32 or 0 (use device default oversampling value). 
 *
 * @param[in]   device      Device handle previously obtained by LMS_Open().
 * @param       rate        sampling rate in Hz to set
 * @param       oversample  RF oversampling ratio.
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetSampleRate(lms_device_t *device, float_type rate,
                                           size_t oversample);

/**
 * Get the sampling rate of the specified LMS device RX or TX channel.
 * The function obtains the sample rate used in data interface with the host and 
 * the RF sample rate used by DAC/ADC.
 * 
 * @param[in]   device      Device handle previously obtained by LMS_Open().
 * @param       dir_tx      Select RX or TX
 * @param[out]  host_Hz     sampling rate used for data exchange with the host
 * @param[out]  rf_Hz       RF sampling rate in Hz
 * 
 * @return       0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetSampleRate(lms_device_t *device, bool dir_tx,
                                           size_t chan, float_type *host_Hz,
                                           float_type *rf_Hz);
/**
 * Get the range of supported sampling rates.
 * 
 * @param device        Device handle previously obtained by LMS_Open().
 * @param dir_tx        Select RX or TX
 * @param range[out]    Allowed sample rate range in Hz. 
 * 
 * @return              0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetSampleRateRange(lms_device_t *device, bool dir_tx,
                                                lms_range_t *range);

/**
 * Set RF center frequency in Hz. This automatically selects the appropriate 
 * antenna (band path) for the desired frequency. In oder to override antenna
 * selection use LMS_SetAntenna().
 * 
 * @note setting different frequencies for A and B channels is not supported by
 * LMS7 chip. Changing frequency for one (A or B) channel will result in
 * frequency being changed for both (A and B) channels.
 *
 * @param   device      Device handle previously obtained by LMS_Open().
 * @param   dir_tx      Select RX or TX
 * @param   chan        Channel index
 * @param   frequency   Desired RF center frequency in Hz
 * 
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetLOFrequency(lms_device_t *device, bool dir_tx,
                                            size_t chan, float_type frequency);

/**
 * Obtain the current RF center frequency in Hz.
 * 
 * @param       device      Device handle previously obtained by LMS_Open().
 * @param       dir_tx      Select RX or TX
 * @param       chan        Channel index
 * @param[out]  frequency   Current RF center frequency in Hz
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetLOFrequency(lms_device_t *device, bool dir_tx,
                                            size_t chan, float_type *frequency);

/**
 * Obtain the supported RF center frequency range in Hz.
 * 
 * @param       device      Device handle previously obtained by LMS_Open().
 * @param       dir_tx      Select RX or TX
 * @param       chan        Channel index
 * @param[out]  range       Supported RF center frequency in Hz
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetLOFrequencyRange(lms_device_t *device,bool dir_tx,
                                                 lms_range_t *range);

/**
 * Configure analog front-end for the specified bandwidth.
 *
 * @param   device      Device handle previously obtained by LMS_Open().
 * @param   dir_tx      Select RX or TX
 * @param   chan        Channel index
 * @param   bandwidth   RF bandwidth in Hz 
 *
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetBW(lms_device_t *device, bool dir_tx, size_t chan,
                                   float_type bandwidth);

/**
 * Get the current bandwidth configuration used for analog front-end.
 *
 * @param       device      Device handle previously obtained by LMS_Open().
 * @param       dir_tx      Select RX or TX
 * @param       chan        channel index
 * @param[out]  bandwidth   Currently configured bandwidth in Hz 
 *
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetBW(lms_device_t *device, bool dir_tx, size_t chan,
                                   float_type *bandwidth);

/**
 * Get the range of supported analog front-end bandwidth values 
 *
 * @param       device      Device handle previously obtained by LMS_Open().
 * @param       dir_tx      Select RX or TX
 * @param       chan        channel index
 * @param[out]  range       supported bandwidth range 
 *
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetBWRange(lms_device_t *device, bool dir_tx,
                                        lms_range_t *range);

/**
 * Set the combined gain value
 *
 * This function computes and sets the optimal gain values of various amplifiers
 * that are present in the device based on desired normalized gain value.
 *
 * @param   device      Device handle previously obtained by LMS_Open().
 * @param   dir_tx      Select RX or TX
 * @param   chan        Channel index
 * @param   gain        Desired gain, range [0, 1.0], where 1.0 represents the
 *                      maximum gain
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetNormalizedGain(lms_device_t *device, bool dir_tx,
                                                size_t chan,float_type gain);

/**
 * Obtain the current combined gain value
 *
 * @param       device      Device handle previously obtained by LMS_Open().
 * @param       dir_tx      Select RX or TX
 * @param       chan        Channel index
 * @param[out]  gain        Current gain, range [0, 1.0], where 1.0 represents
 *                          the maximum gain                    
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetNormalizedGain(lms_device_t *device, bool dir_tx,
                                                size_t chan,float_type *gain);

/**
 * Configure analog LPF of the LMS chip. 
 * This function automatically enables LPF.
 *
 * @param   device      Device handle previously obtained by LMS_Open().
 * @param   dir_tx      Select RX or TX
 * @param   chan        Channel index
 * @param   bandwidth   LPF bandwidth in Hz
 *
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetLPFBW(lms_device_t *device, bool dir_tx,
                                             size_t chan, float_type bandwidth);

/**
 * Get the currently configured analog LPF bandwidth. 
 *
 * @param       device      Device handle previously obtained by LMS_Open().
 * @param       dir_tx      Select RX or TX
 * @param       chan        Channel index
 * @param[out]  bandwidth   Current LPF bandwidth in Hz
 *
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetLPFBW(lms_device_t *device, bool dir_tx,
                                            size_t chan, float_type *bandwidth);

/**
 * Get the bandwidth setting range supported by the analog LPF of LMS chip
 *
 * @param       device      Device handle previously obtained by LMS_Open().
 * @param       dir_tx      Select RX or TX
 * @param       chan        Channel index
 * @param[out]  range       Supported bandwidth range in Hz
 *
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetLPFBWRange(lms_device_t *device,
                                  bool dir_tx, lms_range_t *range);

/**
 * Disables or enables the analog LPF of LMS chip without reconfiguring it. 
 *
 * @param   device      Device handle previously obtained by LMS_Open().
 * @param   dir_tx      Select RX or TX
 * @param   chan        Channel index
 * @param   enable      true(1) to enable, false(0) to disable
 *
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetLPF(lms_device_t *device, bool dir_tx,
                                    size_t chan, bool enabled);

/**
 * Set up digital LPF using LMS chip GFIRS. This is a convenience function to 
 * quickly configure GFIRS as LPF with desired bandwidth.
 *
 * @pre sampling rate must be set
 * 
 * @param  device      Device handle previously obtained by LMS_Open().
 * @param  dir_tx      Select RX or TX
 * @param  chan        channel index
 * @param  enabled     Disable (false) or enable (true) GFIRS.   
 * @param  bandwidth   LPF bandwidth in Hz. Has no effect if enabled is false.
 *
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetGFIRLPF(lms_device_t *device, bool dir_tx,
                               size_t chan, bool enabled, float_type bandwidth);

/**
 * Perform the automatic calibration of specified RX/TX channel. The automatic
 * calibration must be run after device configuration is finished because
 * calibration values are dependant on various configuration settings.
 *
 * @note automatic RX calibration is not available when RX_LNA_H path is
 * selected
 * 
 * @pre Device should be configured
 * 
 * @param   device      Device handle previously obtained by LMS_Open().
 * @param   dir_tx      Select RX or TX
 * @param   chan        channel index
 * @param   bw          bandwidth
 *
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_Calibrate(lms_device_t *device, bool dir_tx, 
                                        size_t chan, double bw, unsigned flags);

/**
 * Load LMS chip configuration from a file
 * 
 * @param   device      Device handle
 * @param   filename    path to file 
 *
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_LoadConfig(lms_device_t *device, const char *filename);

/**
 * Save LMS chip configuration to a file
 * 
 * @param   device      Device handle
 * @param   module      path to file with LMS chip configuration
 *
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SaveConfig(lms_device_t *device, const char *filename);

/**
 * Apply the specified loopback mode
 * 
 * @param   device      Device handle previously obtained by LMS_Open().
 * @param   mode        Loopback mode. LMS_LOOPBACK_NONE returns to normal
 *                      operation
 *
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetLoopback(lms_device_t *device,
                                         lms_loopback_t mode);
/**
 * Get the current loopback mode
 * 
 * @param       device      Device handle previously obtained by LMS_Open().
 * @param[out]  mode        Current loopback mode
 *
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetLoopback(lms_device_t *device,
                                         lms_loopback_t* mode);


/**
 * Apply the specified test signal
 * 
 * @param   device  Device handle previously obtained by LMS_Open().
 * @param   dir_tx  Select RX or TX.
 * @param   chan    Channel index.
 * @param   sig     Test signal. LMS_TESTSIG_NONE disables test signal.
 * @param   dc_i    DC I value for LMS_TESTSIG_DC mode. Ignored in other modes.
 * @param   dc_q    DC Q value for LMS_TESTSIG_DC mode. Ignored in other modes.
 *
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetTestSignal(lms_device_t *device, bool dir_tx,
                    size_t chan, lms_testsig_t sig, int16_t dc_i, int16_t dc_q);
/**
 * Get the currently active test signal
 * 
 * @param   device      Device handle previously obtained by LMS_Open().
 * @param   dir_tx      Select RX or TX
 * @param   chan        Channel index
 * @param   sig         Currently active test signal         
 *
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetTestSignal(lms_device_t *device, bool dir_tx,
                                               size_t chan, lms_testsig_t *sig);

/** @} (End FN_HIGH_LVL) */


/**
 * @defgroup FN_ADVANCED    Advanced control functions
 *
 * The functions in this section provides some additional control compared to
 * High-Level functions. They are labeled advanced because they require better
 * understanding of hardware and provide functionality that may conflict with 
 * certain High-Level functions. 
 * @{
 */

/**Enumeration of LMS7 GFIRS*/
typedef enum
{
    LMS_GFIR1 = 0,
    LMS_GFIR2,
    LMS_GFIR3
}lms_gfir_t;


/**Number of NCO frequency/phase offset values*/
const size_t LMS_NCO_VAL_COUNT = 16;


/** Convenience type for fixed length name string*/  
typedef char lms_name_t[16];

/**
 * Obtain antenna list with names. First item in the list is the name of antenna
 * index 0.
 *
 * @param       device      Device handle previously obtained by LMS_Open().
 * @param       dir_tx      Select RX or TX
 * @param       chan        channel index
 * @param[out]  list        List of antenna names (can be NULL)
 * 
 * @return      number of items in the list on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetAntennaList(lms_device_t *device, bool dir_tx,
                                        size_t chan, lms_name_t *list);

/**
 * Select the antenna for the specified RX or TX channel.
 *
 * LMS_SetFrequency() automatically selects antenna based on frequency. This 
 * function is meant to override path selected by LMS_SetFrequency() and should
 * be called after LMS_SetFrequency().
 *
 * @param       device      Device handle previously obtained by LMS_Open().
 * @param       dir_tx      Select RX or TX
 * @param       chan        channel index
 * @param       index       Index of antenna to select
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetAntenna(lms_device_t *device, bool dir_tx,
                                        size_t chan, size_t index);

/**
 * Obtain currently selected antenna of the the specified RX or TX channel.
 *
 * @param       dev        Device handle previously obtained by LMS_Open().
 * @param       dir_tx     Select RX or TX
 * @param       chan       channel index
 * @param       index      Index of the currently selected antenna
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetAntenna(lms_device_t *device, bool dir_tx, size_t chan, size_t *index);

/**
 * Obtains bandwidth (lower and upper frequency) of the specified antenna
 *
 * @param       dev        Device handle previously obtained by LMS_Open().
 * @param       dir_tx     Select RX or TX
 * @param       chan       channel index
 * @param       index      Antenna index
 * @param[out]  range      Antenna bandwidth
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetAntennaBW(lms_device_t *device, bool dir_tx, size_t chan, size_t index, lms_range_t *range);

/**
 * Set sampling rate for all RX or TX channels. Sample rate is in complex
 * samples (1 sample = I + Q). The function sets sampling rate that is used for
 * data exchange with the host. It also allows to specify higher sampling rate 
 * to be used in RF by setting oversampling ratio. Valid oversampling values are
 * 1, 2, 4, 8, 16, 32 or 0 (use device default oversampling value). 
 * 
 * @note RX and TX rates sampling are closely tied in LMS7 chip. Changing RX or
 * TX will often result in change of both (RX and TX). RX/TX ratio can only be 
 * power of 2 and is also limited by other factors. Use LMS_GetSampleRate() to 
 * obtain actual sample rate values. The function returns success if it is able
 * to achieve  desired sample rate and oversampling for the specified direction
 * (RX or TX) ignoring possible value changes in other direction channels.  
 * 
 * @param[in]   device      Device handle previously obtained by LMS_Open().
 * @param       dir_tx      Select RX or TX
 * @param       rate        Sampling rate in Hz to set
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetSampleRateDir(lms_device_t *device, bool dir_tx,
                                            float_type rate, size_t oversample);

/**
 * Configure NCO to operate in FCW mode. Configures NCO with up to 16 
 * frequencies that can be quickly switched between.
 * Automatically starts NCO with frequency at index 0
 * Use LMS_SetNCOindex() to switch between NCO frequencies.
 * 
 * @note LMS_SetLOFrequency() may configure NCO in oder to achieve certain RF
 * center frequencies (bellow 100 MHz). This function will override those 
 * settings. 
 * 
 * @param       dev        Device handle previously obtained by LMS_Open().
 * @param       dir_tx     Select RX or TX
 * @param       chan       Channel index
 * @param[in]   freq       List of NCO frequencies. Values cannot be negative.
 *                         Must be at least ::LMS_NCO_VAL_COUNT length;
 * @param       pho        NCO phase offset in deg
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetNCOFrequency(lms_device_t *device, bool dir_tx,
                     size_t chan, const float_type *freq, float_type pho);

/**
 * Get the current NCO FCW mode configuration.
 * 
 * @param       dev        Device handle previously obtained by LMS_Open().
 * @param       dir_tx     Select RX or TX
 * @param       chan       Channel index
 * @param[out]  freq       List of NCO frequencies. Must be at least
 *                         ::LMS_NCO_VAL_COUNT length;
 * @param[out]  pho1       Phase offset in deg
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetNCOFrequency(lms_device_t *device, bool dir_tx,
                          size_t chan, float_type *freq, float_type *pho);

/**
 * Configure NCO to operate in PHO mode. Configures NCO with up to 16 
 * phase offsets that can be quickly switched between.
 * Automatically starts NCO with phase at index 0
 * Use LMS_SetNCOindex() to switch between NCO phases.
 * 
 * @note LMS_SetLOFrequency() may configure NCO in oder to achieve certain RF
 * center frequencies (bellow 100 MHz). This function will override those 
 * settings. 
 * 
 * @param       dev        Device handle previously obtained by LMS_Open().
 * @param       dir_tx     Select RX or TX
 * @param       chan       Channel index
 * @param[in]   phases     List of NCO phases. Values cannot be negative. 
 *                         Must be at least ::LMS_NCO_VAL_COUNT length;
 * @param       fcw        NCO frequency in Hz
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetNCOPhase(lms_device_t *device, bool dir_tx,
                   size_t chan, const float_type *phases, float_type fcw);

/**
 * Get the current NCO PHO mode configuration.
 * 
 * @param       dev       Device handle previously obtained by LMS_Open().
 * @param       dir_tx    Select RX or TX
 * @param       chan      channel index
 * @param[out]  phases    List of configured NCO phases
 *                        Must be at least ::LMS_NCO_VAL_COUNT length;
 * @param[out]  fcw       Current NCO frequency
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetNCOPhase(lms_device_t *device, bool dir_tx,
                        size_t chan, float_type *phases, float_type *fcw);

/**
 * Switches between configured list of NCO frequencies/phase offsets. Also
 * Allows to switch CMIX mode to either downconvert or upconvert.
 * 
 * @note LMS_SetLOFrequency() may configure NCO in oder to achieve certain RF
 * center frequencies (bellow 100 MHz). This function will override those 
 * settings. 
 * 
 * @param dev       Device handle previously obtained by LMS_Open().
 * @param dir_tx    Select RX or TX
 * @param chan      channel index
 * @param index     NCO frequency/phase index to activate
 * @param downconv  true(1) CMIX downconvert, false(0) CMIX upconvert
 * 
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetNCOIndex(lms_device_t *device, bool dir_tx,
                        size_t chan, size_t index, bool downconv);

/**
 * Get the currently active NCO frequency/phase offset index
 * 
 * @param       dev       Device handle previously obtained by LMS_Open().
 * @param       dir_tx    Select RX or TX
 * @param       chan      Channel index
 * @param[out]  index     Current NCO frequency/phase index.
 * 
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetNCOIndex(lms_device_t *device, bool dir_tx,
                        size_t chan, size_t *index);

/**
 * Read device parameter. Parameter defines specific bits in device register.
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param param     Parameter.
 * @param val       Current parameter value.
 * 
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_ReadParam(lms_device_t *device,
                                     struct LMS7Parameter param, uint16_t *val);

/**
 * Write device parameter. Parameter defines specific bits in device register.
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param param     Parameter.
 * @param val       Parameter value to write
 * 
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_WriteParam(lms_device_t *device,
                                      struct LMS7Parameter param, uint16_t val);

/**
 * Generates LPF coefficients for LMS GFIR.
 * Pass and stop band frequencies are relative to sampling rate.
 * 
 * @param      n        number of coefficients to generate.
 * @param      w1       Relative pass band frequency. Range [0, 0.5].
 * @param      w2       Relative Stop band frequency. Range [w1, 0.5].
 * @param      g_stop   Stop_band gain. Range [0, 0.5].
 * @param[out] coef     generated filter coefficients
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GenerateLPFCoef(size_t n, float_type w1,
                        float_type w2, float_type g_stop, float_type *coef);

/**
 * Configure LMS GFIR using specified filter coefficients. Maximum number of 
 * coefficients is 40 for GFIR1 and GFIR2, and 120 for GFIR3. 
 * 
 * @param       dev       Device handle previously obtained by LMS_Open().
 * @param       dir_tx    Select RX or TX
 * @param       chan      Channel index
 * @param       filt      GFIR to configure
 * @param[in]   coef      Array of filter coefficients. Coeff range [-1.0, 1.0]. 
 * @param       count     number of filter coefficients.
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetGFIRCoeff(lms_device_t * device, bool dir_tx, 
             size_t chan, lms_gfir_t filt, const float_type* coef,size_t count);

/**
 * Get currently set GFIR coefficients.
 * 
 * @param       dev       Device handle previously obtained by LMS_Open().
 * @param       dir_tx    Select RX or TX
 * @param       chan      Channel index
 * @param       filt      GFIR to configure
 * @param[out]  coef      Current GFIR coefficients. Array must be big enough to
 *                        hold 40 (GFIR1, GFIR2) or 120 (GFIR3) values.
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetGFIRCoeff(lms_device_t * device, bool dir_tx,
                                size_t chan, lms_gfir_t filt, float_type* coef);

/**
 * Enables or disables specified GFIR.
 * 
 * @param dev       Device handle previously obtained by LMS_Open().
 * @param dir_tx    Select RX or TX
 * @param chan      Channel index
 * @param filt      GFIR to configure
 * @param enabled   true(1) enable, false(0) disable.
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetGFIR(lms_device_t * device, bool dir_tx,
                                    size_t chan, lms_gfir_t filt, bool enabled);

/**
 * @defgroup FN_LOW_LVL    Low-Level control functions
 *
 * @{
 */

/**
 * Send Reset signal to LMS chip. This initializes LMS chip with default
 * configuration as described in LMS chip datasheet.
 *
 * @param device  Device handle previously obtained by LMS_Open().
 *  
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_Reset(lms_device_t *device);

/**
 * Read device LMS chip register
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param address   Register address
 * @param val       Current register value
 * 
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_ReadLMSReg(lms_device_t *device, uint32_t address,
                                     uint16_t *val);

/**
 * Write device LMS chip register
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param address   Register address
 * @param val       Value to write
 * 
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_WriteLMSReg(lms_device_t *device, uint32_t address,
                                      uint16_t val);

/**
 * Perform register test
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * 
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_RegisterTest(lms_device_t *device);

/**
 * Read device FPGA register
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param address   Register address
 * @param val       Current register value
 * 
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_ReadFPGAReg(lms_device_t *device, uint32_t address,
                                     uint16_t *val);

/**
 * Write device FPGA register
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param address   Register address
 * @param val       Value to write
 * 
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_WriteFPGAReg(lms_device_t *device, uint32_t address,
                                      uint16_t val);

/**
 * Read custom parameter from board
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param param_id  Register address
 * @param val       Current register value
 * @param units     [optional] measurement units of parameter if available
 * 
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_ReadCustomBoardParam(lms_device_t *device,
                           uint16_t param_id, float_type *val, lms_name_t units);

/**
 * Write custom parameter from board
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param param_id  Register address
 * @param val       Value to write
 * @param units     [optional] measurement units of parameter if available
 * 
 * @return  0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_WriteCustomBoardParam(lms_device_t *device,
                        uint16_t address, float_type val, const lms_name_t units);

/**
 * Changes device reference clock used by API for various calculations.
 * Normally reference clock should be detected automatically based on device. 
 * Use this function in case you have replaced the reference crystal.
 * 
 * @param device      Device handle previously obtained by LMS_Open().
 * @param clock_Hz    reference clock in Hz.
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetReferenceClock(lms_device_t * device,
                                               float_type clock_Hz);

/**
 * Get the currently set reference clock
 * 
 * @param device      Device handle previously obtained by LMS_Open().
 * @param clock_Hz    reference clock in Hz.
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetReferenceClock(lms_device_t * device,
                                               float_type * clock_Hz);

/**
 * Write value to VCTCXO trim DAC
 *
 * @param   dev         Device handle previously obtained by LMS_Open().
 * @param   val         Value to write to VCTCXO trim DAC
 *
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_VCTCXOWrite(lms_device_t * dev, uint16_t val);

/**
 * Read value from VCTCXO trim DAC.
 *
 * @param[in]   dev     Device handle previously obtained by LMS_Open().
 * @param[out]  val     Value to read from VCTCXO trim DAC
 *
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_VCTCXORead(lms_device_t * dev, uint16_t *val);

/**
 * Get VCO value range. 
 *
 * @param   dev     Device handle previously obtained by LMS_Open().
 * @param   vco_id  VCO identifier
 * @param   range   VCO range
 *
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetVCORange(lms_device_t * dev, size_t vco_id,
                                         lms_range_t* range);

/**
 * Set VCO value range. 
 *
 * @param   dev     Device handle previously obtained by LMS_Open().
 * @param   vco_id  VCO identifier
 * @param   range   VCO range
 *
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetVCORange(lms_device_t * dev, size_t vco_id,
                                         lms_range_t range);

typedef enum
{
    LMS_RX_LPF_TIA,
    LMS_RX_LPF_LOWBAND,
    LMS_RX_LPF_HIGHBAND,   
    LMS_TX_LPF_REALPOLE,
    LMS_TX_LPF_LADDER,
    LMS_TX_LPF_HIGHBAND,
    LMS_TX_LPF_LOWCHAIN     /**<tunes TX REALPOLE and TX LADDER filters,
                             * requires 2 bandwidth values to be passed*/
}lms_filter_t;

/**
 * Tune filter for the specified bandwidth. 
 *
 * @param   dev     Device handle previously obtained by LMS_Open().
 * @param   chan    channel index
 * @param   filt    filter
 * @param   bw      filter bandwidth
 *
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_TuneFilter(lms_device_t *dev, size_t chan, lms_filter_t filt,
                                         const float_type *bw);

/**
 *  
 *
 * @param   dev     Device handle previously obtained by LMS_Open().
 * @param   func    callback function
 *
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetDataLogCallback(lms_device_t *dev, void (*func)(bool, const unsigned char*, const unsigned int));



/**
 * @defgroup LMS_CLOCK_ID   Streaming mode flags
 *
 * Flags for configuring device streaming mode
 * @{
 */
#define LMS_CLOCK_REF   0x0000
#define LMS_CLOCK_SXR   0x0001  /**<*/
#define LMS_CLOCK_SXT   0x0002  /**<*/
#define LMS_CLOCK_CGEN  0x0003
#define LMS_CLOCK_RXTSP 0x0004
#define LMS_CLOCK_TXTSP 0x0005


/** @} (End LMS_CLOCK_ID) */

/**
 * Get frequency of the specified clock. 
 *
 * @param   dev     Device handle previously obtained by LMS_Open().
 * @param   clk_id  Clock identifier
 * @param   freq    Clock frequency in Hz  
 *
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetClockFreq(lms_device_t *dev, size_t clk_id,
                                         float_type *freq);

/**
 * Set frequency of the specified clock 
 *
 * @param   dev     Device handle previously obtained by LMS_Open().
 * @param   clk_id  Clock identifier
 * @param   freq    Clock frequency in Hz. Pass zero or negative value to only
 *                  perform tune (if supported) without recalculating values
 *
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetClockFreq(lms_device_t *dev, size_t clk_id, 
                                         float_type freq);

/**
 * Load Si5351C configuration from file
 *
 * @param   dev         Device handle previously obtained by LMS_Open().
 * @param   filename    file containing Si5351C configuration
 *
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_LoadConfigSi5351C(lms_device_t *dev,
                                                const char* filename);

/**
 * Configure Si5351C frequencies
 *
 * @param   dev         Device handle previously obtained by LMS_Open().
 * @param   clkin       PLLs input clock frequency. 
 * @param   clks        output clock frequencies (must be 8). 
 *                      0 - disabled, negative - inverted
 * @param   src         input clock source 0-XTAL, 1-CLKIN
 * 
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_ConfigureSi5351C(lms_device_t *dev,
                   float_type clkin, float_type *clks, uint32_t src);


/**
 * Get Si5351C status bits
 *
 * @param   dev         Device handle previously obtained by LMS_Open().
 * @param   status      Status bits. Pass Null to clear status bits 
 * 
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_StatusSi5351C(lms_device_t *dev, uint32_t *status);


typedef struct
{
    double fRef;
    double fVCO;
    //Reference counter latch
    unsigned lockDetectPrec;   /**<3 cycles (0), or 5 cycles (1)*/
    unsigned antiBacklash;     /**<2.9ns (0), 6ns (1)*/
    unsigned referenceCounter; /**<range [0, 16383]*/
    //N counter latch
    unsigned cpGain;           /**<0 or 1*/
    unsigned nCounter;         /**<range [0, 8191]*/
    //Function latch
    unsigned flCurrent1;
    unsigned flCurrent2;
    unsigned flTimerCounter;
    unsigned flFastlock;
    unsigned flMuxCtrl;
    unsigned flPDPolarity;
    unsigned flPD1;
    unsigned flPD2;
    unsigned flCounterReset;
    unsigned flCPState;
    //Initialization latch
    unsigned ilCurrent1;
    unsigned ilCurrent2;
    unsigned ilTimerCounter;
    unsigned ilFastlock;
    unsigned ilMuxCtrl;
    unsigned ilPDPolarity;
    unsigned ilPD1;
    unsigned ilPD2;
    unsigned ilCounterReset;
    unsigned ilCPState;
    //Extra
    uint32_t flags;
    
}lms_adf4002_conf_t;

/**
 * Get Si5351C status bits
 *
 * @param   dev         Device handle previously obtained by LMS_Open().
 * @param   config      ADF4002 configuration structure ::lms_adf4002_conf_t
 * 
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_ConfigureADF4002(lms_device_t *dev, lms_adf4002_conf_t *config);

/**
 * Synchronizes register values between API cache and chip
 *
 * @param   dev         Device handle previously obtained by LMS_Open().
 * @param   toChip      if true copies values from API cache to chip.
 * 
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_Synchronize(lms_device_t *dev, bool toChip);


/**
 *
 * @param   dev         Device handle previously obtained by LMS_Open().
 * @param   buffer      read values
 * @param   len         number of bytes to read
 * 
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GPIORead(lms_device_t *dev, uint8_t* buffer, size_t len);

/**
 *
 * @param   dev         Device handle previously obtained by LMS_Open().
 * @param   buffer      values to write
 * @param   len         number bytes to write
 * 
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GPIOWrite(lms_device_t *dev, const uint8_t* buffer, size_t len);

/**
 *  Enables or disable caching of calibration values
 * 
 * @param   dev         Device handle previously obtained by LMS_Open().
 * @param   enable      true to enable cache
 * 
 * @return 0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_EnableCalibCache(lms_device_t *dev, bool enable);

/** @} (End FN_LOW_LVL) */

/** @} (End FN_ADVANCED) */


/**
 * @defgroup FN_STREAM    Sample Streaming functions
 * The functions in this section provides support for sending and receiving
 * IQ data samples.
 * @{
 */

/**Metadata structure used in sample transfers*/
typedef struct
{
    /* Timestamp is a value of HW counter with a tick based on sample rate.
     * In RX: time when the first sample in the returned buffer was received
     * In TX: time when the first sample in the submitted buffer should be send
     */
    uint64_t timestamp;    
    
    /**In TX indicated whether submitted buffer should be sent using
     * synchronization based on timestamp.*/
    bool has_timestamp;    
    
    /**Indicates the beginning of send/receive transaction. When set attempts
     * to discard old data in transfer buffers and start new transactions*/
    bool start_of_burst;
    
    /**Indicates the end of send/receive transaction. Discards data remainder
     * in buffer (if there is any) in RX or flushes transfer buffer in TX (even 
     * if the buffer is not full yet).*/
    bool end_of_burst;
    
    /**Indicates that previous packet(s) were dropped.
     * This usually happens when:
     * TX: The data arrived to HW too late based on timestamp
     * RX: Packet(s) were discarded because RX buffers were full when new data
     * was received*/
    bool packet_droped;
    
}lms_stream_meta_t;




/**
 * @defgroup LMS_STREAM_FLAGS   Streaming mode flags
 *
 * Flags for configuring device streaming mode
 * @{
 */
#define LMS_STREAM_MD_AUTO  0x0000  /**<Automatic stream configuration*/
#define LMS_STREAM_MD_SISO  0x0001  /**<Stream data from single channel*/
#define LMS_STREAM_MD_MIMO  0x0002  /**<Stream data from multiple channels*/
#define LMS_STREAM_SMP_12B  0x0000  /**<Use compressed 12-bit sample values*/
#define LMS_STREAM_SMP_16B  0x0010  /**<Use uncompressed 16-bit sample values*/
#define LMS_STREAM_FMT_I16  0x0000  
#define LMS_STREAM_FMT_F32  0x0200

/** @} (End LMS_STREAM_FLAGS) */

/**
 * Configures devices for specific streaming mode overriding default/automatic
 * settings.
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param flags     Mode flags. Refer to \ref LMS_STREAM_FLAGS.
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SetStreamingMode(lms_device_t *device, uint32_t flags);

/**
 * Initializes/configures RX/TX stream buffers. 
 * 
 * @param device        Device handle previously obtained by LMS_Open().
 * @param dir_tx        Select RX or TX
 * @param num_tranfers  Number of  buffers used for data transfer
 * @param transfer_size Size of a single transfer buffer in bytes
 * @param fifo_size     Size of FIFO buffer (0 = Disabled)
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_InitStream(lms_device_t *device, bool dir_tx,
                   size_t num_tranfers, size_t transfer_size, size_t fifo_size);

/**
 * Completely stops RX/TX stream buffers. 
 * 
 * @param device        Device handle previously obtained by LMS_Open().
 * @param dir_tx        Select RX or TX
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_StopStream(lms_device_t *device, bool dir_tx);

/**
 * Receive samples from all active RX channels.
 * The number of sample buffers must be at least the same as active channels.
 * Sample buffers must be big enough to hold requested number of samples. 
 * 
 * @pre RX stream is configured using LMS_ConfigRx()
 * 
 * @param device        Device handle previously obtained by LMS_Open().
 * @param samples       pointers to sample buffers (for each active channel).
 * @param sample_count  Number of samples to read from each channel
 * @param meta          Metadata. See the ::lms_stream_meta_t description.
 * @param timeout_ms    how long to wait for data before timing out(0=infinite).
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_RecvStream(lms_device_t *device, void **samples,
         size_t sample_count, lms_stream_meta_t *meta, unsigned timeout_ms);

/**
 * Send samples to active TX channels.
 * The number of sample buffers must be at least the same as active channels.
 * 
 * @pre TX stream is configured using LMS_ConfigTx()
 * 
 * @param device        Device handle previously obtained by LMS_Open().
 * @param samples       pointers to sample buffers (for each active channel).
 * @param sample_count  Number of samples to write to each channel
 * @param meta          Metadata. See the ::lms_stream_meta_t description.
 * @param timeout_ms    how long to wait for data before timing out(0=infinite).
 * 
 * @return      0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_SendStream(lms_device_t *device, 
                              const void **samples,size_t sample_count, 
                              lms_stream_meta_t *meta, unsigned timeout_ms);

/** @} (End FN_STREAM) */




/**
 * @defgroup FN_VERSION   Version and update functions
 *
 * The functions in this section provides ability to check device version
 * and perform updates
 * @{
 */

/**Enumeration of device programming target*/
typedef enum 
{
    LMS_TARGET_RAM = 0,    /**<load firmware/bitstream to volatile storage*/
    LMS_TARGET_FLASH = 1,  /**<load firmware/bitstream to non-volatile storage*/
    LMS_TARGET_BOOT = 2    /**<reset and boot from flash*/
}lms_target_t;



/**Device information structure*/
typedef struct 
{
    //! The displayable name for the device
    char deviceName[32];

    /*! The displayable name for the expansion card
     */
    char expansionName[32];

    //! The firmware version as a string
    char firmwareVersion[16];

    //! The hardware version as a string
    char hardwareVersion[16];

    //! The protocol version as a string
    char protocolVersion[16];

    //! A unique board serial number
    uint32_t boardSerialNumber;
    
    uint32_t reserved[15];
}lms_dev_info_t;

/**
 * Get device serial number and version information
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param info      Device information. See the ::lms_dev_info.
 * @return          0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_GetDeviceInfo(lms_device_t *device, lms_dev_info_t *info);


/*!
 * Callback from programming processes
 * @param bsent number of bytes transferred
 * @param btotal total number of bytes to send
 * @param progressMsg string describing current progress state
 * @return 0-continue programming, 1-abort operation
 */
typedef bool (*lms_prog_callback_t)(int bsent, int btotal, const char* progressMsg);

/**
 * Write binary FPGA image to device.
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param data      Pointer to memory containing FPGA image
 * @param size      Size of FPGA image in bytes.
 * @param target    load to volatile or non-volatile storage
 * @return          0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_ProgramFPGA(lms_device_t *device, const char *data,
                 size_t size, lms_target_t target,lms_prog_callback_t callback);

/**
 * Read FPGA image from the specified file and write it to device.
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param file      File containing FPGA image
 * @param target    load to volatile or non-volatile storage
 * @return          0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_ProgramFPGAFile(lms_device_t *device,
           const char *file, lms_target_t target, lms_prog_callback_t callback);

/**
 * Write firmware image to device.
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param data      Pointer to memory containing firmware image
 * @param size      Size of firmware image in bytes.
 * @param target    load to volatile or non-volatile storage
 * @return          0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_ProgramFirmware(lms_device_t *device, const char *data,
                size_t size, lms_target_t target, lms_prog_callback_t callback);

/**
 * Read firmware image from file and write it to device.
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param file      File containing Firmware image
 * @param target    load to volatile or non-volatile storage
 * @return          0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_ProgramFirmwareFile(lms_device_t *device,
           const char *file, lms_target_t target, lms_prog_callback_t callback);

/**
 * Program LMS7 internal MCU.
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * @param file      Pointer to memory containing binary MCU firmware image data
 * @param target    load to volatile or non-volatile storage
 * @param size      Size of MCU firmware image in bytes.
 * 
 * @return          0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_ProgramLMSMCU(lms_device_t *device, const char *data,
                size_t size, lms_target_t target, lms_prog_callback_t callback);
/**
 * Boots LMS7 internal MCU from flash memory.
 * 
 * @param device    Device handle previously obtained by LMS_Open().
 * 
 * @return          0 on success, (-1) on failure
 */
API_EXPORT int CALL_CONV LMS_ResetLMSMCU(lms_device_t *device);

/** @} (End FN_VERSION) */

/**
 * @defgroup FN_ERRORS    Error reporting
 *
 * LMS API functions return 0 or positive value on success and (-1) on failure.
 * The functions in this section provides ability to get more detailed
 * error descriptions.
 * @{
 */

/**
 * Get the last error code. Error codes are as defined in errno.h
 * @return last error code        
 */
API_EXPORT int CALL_CONV LMS_GetLastError(void);

/**
 * Get the error message detailing why the last error occurred. 
 *
 * @return last error message.      
 */
API_EXPORT const char * CALL_CONV LMS_GetLastErrorMessage(void);


/** @} (End FN_ERRORS) */

#ifdef __cplusplus
} //extern "C"
#endif

#endif //LMS_SDR_INTERFACE_H