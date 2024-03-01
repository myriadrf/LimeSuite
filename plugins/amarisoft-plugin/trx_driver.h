/*
 * Amarisoft Transceiver API version 2019-09-30
 * Copyright (C) 2013-2019 Amarisoft
 */
#ifndef TRX_DRIVER_H
#define TRX_DRIVER_H

#include <inttypes.h>

#define TRX_API_VERSION 15

#define TRX_MAX_CHANNELS 16
#define TRX_MAX_RF_PORT  TRX_MAX_CHANNELS

typedef struct TRXState TRXState;

typedef int64_t trx_timestamp_t;

/* Currently only complex floating point samples are supported */
typedef struct {
    float re;
    float im;
} TRXComplex;

typedef struct {
    int num;
    int den;
} TRXFraction;

typedef struct {

    int         rf_port_index;
    uint32_t    dl_earfcn;  /* aka arfcn for NR */
    uint32_t    ul_earfcn;  /* aka arfcn for NR */
    int         n_rb_dl;
    int         n_rb_ul;

    enum {
        TRX_CYCLIC_PREFIX_NORMAL,
        TRX_CYCLIC_PREFIX_EXTENDED,
    } dl_cyclic_prefix, ul_cyclic_prefix;

    enum {
        TRX_CELL_TYPE_FDD,
        TRX_CELL_TYPE_TDD,
        TRX_NBCELL_TYPE_FDD,
        TRX_NBCELL_TYPE_TDD, /* Not available */
        TRX_NRCELL_TYPE_FDD,
        TRX_NRCELL_TYPE_TDD,
    } type;

    union {
        struct {
            uint8_t uldl_config;
            uint8_t special_subframe_config;
        } tdd;
        struct {
            int band;
            int dl_subcarrier_spacing; /* mu: [0..3] */
            int ul_subcarrier_spacing; /* mu: [0..3] */
        } nr_fdd;
        struct {
            int band;
            int dl_subcarrier_spacing; /* mu: [0..3] */
            int ul_subcarrier_spacing; /* mu: [0..3] */

            /* TDD */
            uint8_t period_asn1;
            uint8_t dl_slots;
            uint8_t dl_symbs;
            uint8_t ul_slots;
            uint8_t ul_symbs;
        } nr_tdd;

        uint64_t pad[32];
    } u;

} TRXCellInfo;

typedef struct {
    /* number of RX channels (=RX antennas) */
    int rx_channel_count;
    /* number of TX channels (=TX antennas) */
    int tx_channel_count;
    /* RX center frequency in Hz for each channel */
    int64_t rx_freq[TRX_MAX_CHANNELS];
    /* TX center frequency in Hz for each channel */
    int64_t tx_freq[TRX_MAX_CHANNELS];
    /* initial rx_gain for each channel, same unit as trx_set_rx_gain_func() */
    double rx_gain[TRX_MAX_CHANNELS];
    /* initial tx_gain for each channel, same unit as trx_set_tx_gain_func() */
    double tx_gain[TRX_MAX_CHANNELS];
    /* RX bandwidth, in Hz for each channel */
    int rx_bandwidth[TRX_MAX_CHANNELS];
    /* TX bandwidth, in Hz for each channel */
    int tx_bandwidth[TRX_MAX_CHANNELS];

    /* Number of RF ports.
     * A separate trx_write() is be done for each TX port on different thread.
     * Each TX port can have a different TDD configuration.
     * A separate trx_read() is be done for each RX port on different thread.
     */
    int rf_port_count;

    /* the sample rate for both transmit and receive.
     * One for each port */
    TRXFraction sample_rate[TRX_MAX_RF_PORT];

    /* Arrays of rf_port_count elements containing the number of
       channels per TX/RX port. Their sum must be equal to
       tx_channel_count/rx_channel_count. */
    int tx_port_channel_count[TRX_MAX_RF_PORT];
    int rx_port_channel_count[TRX_MAX_RF_PORT];

    /* Array of cell_count elements pointing to information
     * on each cell */
    TRXCellInfo *cell_info;
    int cell_count;

} TRXDriverParams;

typedef struct {
    /* Number of times the data was sent too late by the application. */
    int64_t tx_underflow_count;
    /* Number of times the receive FIFO overflowed. */
    int64_t rx_overflow_count;
} TRXStatistics;

typedef void __attribute__ ((format (printf, 2, 3))) (*trx_printf_cb)(void *, const char *fmt, ... );


/*
 * Structure used for trx_write_func2:
 */
typedef struct {

#define TRX_WRITE_MD_FLAG_PADDING            (1 << 0)       /* Cf trx_write_func */
#define TRX_WRITE_MD_FLAG_END_OF_BURST       (1 << 1)       /* Cf trx_write_func */
#define TRX_WRITE_MD_FLAG_HARQ_ACK_PRESENT   (1 << 2)       /* Cf harq_flags */
#define TRX_WRITE_MD_FLAG_TA_PRESENT         (1 << 3)       /* Cf ta */
#define TRX_WRITE_MD_FLAG_CUR_TIMESTAMP_REQ  (1 << 4)       /* Cf cur_timestamp field */

    uint32_t flags;

    /* HARQ
     * only used for TDD
     * HARQ/ACK info, only used for eNodeB testing
     * set if HARQ ACK/NACK info is present in the subframe
     */
#define TRX_WRITE_MD_HARQ_ACK0              (1 << 0)
#define TRX_WRITE_MD_HARQ_ACK1              (1 << 1)
    uint8_t harq_ack;

    /* TA: 6 bits */
    uint8_t timing_advance;

    /* Current real time timestamp.
     * If TRX_WRITE_MD_FLAG_CUR_TIMESTAMP_REQ, cur_timestamp may be set by callee.
     * It represents real time timestamp used to compute RX/TX latency.
     * Set cur_timestamp_set to non 0 to notify caller.
     */
    uint8_t cur_timestamp_set;
    trx_timestamp_t cur_timestamp;

} TRXWriteMetadata;

/*
 * Structure used for trx_read_func2:
 *
 * Future use
 */
typedef struct {
    uint32_t flags;

} TRXReadMetadata;


/* Deprecated, used for trx_write_func
 * Refer to trx_write_func2 flags */
#define TRX_WRITE_FLAG_PADDING              TRX_WRITE_MD_FLAG_PADDING
#define TRX_WRITE_FLAG_END_OF_BURST         TRX_WRITE_MD_FLAG_END_OF_BURST


typedef struct TRXMsg TRXMsg;
struct TRXMsg {
    void *opaque;

    /* Message in API
     * Only used for received messages
     */
    int (*get_double)(TRXMsg *msg, double *pval, const char *prop_name);
    int (*get_string)(TRXMsg *msg, const char **pval, const char *prop_name);

    /* Message out API */
    void (*set_double)(TRXMsg *msg, const char *prop_name, double val);
    void (*set_string)(TRXMsg *msg, const char *prop_name, const char *string);

    /* Always call this once, even in timeout_cb */
    void (*send)(TRXMsg *msg);

    /* Async handling
     * Only used for received messages
     */
    void (*set_timeout)(TRXMsg *msg, void (*timeout_cb)(TRXMsg*), int timeout_ms);

    /* User defined data */
    void *user_data;
};


struct TRXState {
    /* API version */
    int trx_api_version;

    /* set by the application - do not modify */
    void *app_opaque;
    /* set by the application - do not modify */
    char *(*trx_get_param_string)(void *app_opaque, const char *prop_name);
    /* set by the application - do not modify */
    int (*trx_get_param_double)(void *app_opaque, double *pval,
                                const char *prop_name);

    /* set by the application - do not modify */
    void *reserved[127];

    /* Path of the config file, not termnating by / */
    const char *path;

    /* Can be set by the driver to point to internal data */
    void *opaque;

    /* the following functions are called by the application */

    /* Return in *psample_rate the sample rate supported by the device
       corresponding to a LTE bandwidth of 'bandwidth' Hz. Also return
       in n=*psample_rate_num the wanted sample rate before the
       interpolator as 'n * 1.92' MHz. 'n' must currently be of the
       form 2^n1*3^n2*5^n3.

       Return 0 if OK, -1 if none. */
    int (*trx_get_sample_rate_func)(TRXState *s, TRXFraction *psample_rate,
                                    int *psample_rate_num,
                                    int bandwidth);

    /* Called to start the tranceiver. Return 0 if OK, < 0 if  */
    int (*trx_start_func)(TRXState *s, const TRXDriverParams *p);

    /* Deprecated, use trx_write_func2 instead.
       Write 'count' samples on each channel of the TX port
       'tx_port_index'. samples[0] is the array for the first
       channel. timestamp is the time (in samples) at which the first
       sample must be sent. When the TRX_WRITE_FLAG_PADDING flag is
       set, samples is set to NULL. It indicates that no data should
       be sent (TDD receive time). TRX_WRITE_FLAG_END_OF_BURST is set
       to indicate in advance that the next write call will have the
       TRX_WRITE_FLAG_PADDING flag set. Note:
       TRX_WRITE_FLAG_END_OF_BURST and TRX_WRITE_FLAG_PADDING are
       never set simultaneously.
     */
    void (*trx_write_func)(TRXState *s, trx_timestamp_t timestamp, const void **samples, int count, int flags, int tx_port_index);

    /* Deprecated, use trx_read_func2 instead.
       Read 'count' samples from each channel. samples[0] is the array
       for the first channel. *ptimestamp is the time at which the
       first samples was received. Return the number of sample read
       (=count).

       Note: It is explicitely allowed that the application calls
       trx_write_func, trx_read_func, trx_set_tx_gain_func and
       trx_set_rx_gain_func from different threads.
    */
    int (*trx_read_func)(TRXState *s, trx_timestamp_t *ptimestamp, void **samples, int count, int rx_port_index);

    /* Dynamic set the transmit gain (in dB). The origin and range are
       driver dependent.

       Note: this function is only used for user supplied dynamic
       adjustements.
    */
    void (*trx_set_tx_gain_func)(TRXState *s, double gain, int channel_num);

    /* Dynamic set the receive gain (in dB). The origin and range are
       driver dependent.

       Note: this function is only used for user supplied dynamic
       adjustements.
    */
    void (*trx_set_rx_gain_func)(TRXState *s, double gain, int channel_num);

    /* Terminate operation of the transceiver - free all associated
       resources */
    void (*trx_end_func)(TRXState *s);

    /* Return the maximum number of samples per TX packet. Called by
       the application after trx_start_func. */
    int (*trx_get_tx_samples_per_packet_func)(TRXState *s);

    /* Return some statistics. Return 0 if OK, < 0 if not available. */
    int (*trx_get_stats)(TRXState *s, TRXStatistics *m);

    /* Callback must allocate info buffer that will be displayed */
    void (*trx_dump_info)(TRXState *s, trx_printf_cb cb, void *opaque);

    /* Return the absolute TX power in dBm for the TX channel
       'channel_num' assuming a square signal of maximum
       amplitude. This function can be called from any thread and
       needs to be fast. Return 0 if OK, -1 if the result is not
       available. */
    int (*trx_get_abs_tx_power_func)(TRXState *s,
                                     float *presult, int channel_num);

    /* Return the absolute RX power in dBm for the RX channel
       'channel_num' assuming a square signal of maximum
       amplitude. This function can be called from any thread and
       needs to be fast. Return 0 if OK, -1 if the result is not
       available. */
    int (*trx_get_abs_rx_power_func)(TRXState *s,
                                     float *presult, int channel_num);

    /* Read/Write IQ samples
     * Available since API v13
     */
    void (*trx_write_func2)(TRXState *s, trx_timestamp_t timestamp, const void **samples,
                            int count, int tx_port_index, TRXWriteMetadata *md);
    int (*trx_read_func2)(TRXState *s, trx_timestamp_t *ptimestamp, void **samples, int count,
                          int rx_port_index, TRXReadMetadata *md);

    /* Remote API communication
     * Available since API v14
     * trx_msg_recv_func: called for each trx received messages
     * trx_msg_send_func: call it to send trx messages (They must be registered by client)
     * For each message, a call to send API must be done
     */
    void (*trx_msg_recv_func)(TRXState *s, TRXMsg *msg);
    TRXMsg* (*trx_msg_send_func)(TRXState *s); /* set by application, do notmodify it */
};

/* return 0 if OK, < 0 if error. */
int trx_driver_init(TRXState *s);

/* Get string parameter from configuration. Must be freed with
   free(). Return NULL if property does not exists. Can only be called
   in trx_driver_init(). */
static inline char *trx_get_param_string(TRXState *s, const char *prop_name)
{
    return s->trx_get_param_string(s->app_opaque, prop_name);
}

/* Get floating point parameter from configuration. Must be freed with
   free(). Return 0 if OK or < 0 if property does not exists. Can only be called
   in trx_driver_init(). */
static inline int trx_get_param_double(TRXState *s, double *pval,
                                       const char *prop_name)
{
    return s->trx_get_param_double(s->app_opaque, pval, prop_name);
}

#endif /* TRX_DRIVER_H */
