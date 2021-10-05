// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#ifndef _MEPA_INDY_PRIVATE_H_
#define _MEPA_INDY_PRIVATE_H_

#include <unistd.h>
#include <microchip/ethernet/phy/api/types.h>
#include <microchip/ethernet/phy/api/phy_ts.h>

#define MEPA_RC(expr) { mesa_rc __rc__ = (expr); if (__rc__ < MESA_RC_OK) return __rc__; }
#define MEPA_ASSERT(x) if((x)) { return MESA_RC_ERROR;}

#define TRUE  1
#define FALSE 0
#define EXT_PAGE 1 // extended page access
#define MMD_DEV  2 // MMD device access

// register access functions
mepa_rc indy_direct_reg_rd(mepa_device_t *dev, uint16_t addr, uint16_t *value);
mepa_rc indy_direct_reg_wr(mepa_device_t *dev, uint16_t addr, uint16_t value, uint16_t mask);
mepa_rc indy_ext_reg_rd(mepa_device_t *dev, uint16_t page, uint16_t addr, uint16_t *value);
mepa_rc indy_ext_reg_wr(mepa_device_t *dev, uint16_t page, uint16_t addr, uint16_t value, uint16_t mask);
mepa_rc indy_mmd_reg_rd(mepa_device_t *dev, uint16_t mmd, uint16_t addr, uint16_t *value);
mepa_rc indy_mmd_reg_wr(mepa_device_t *dev, uint16_t mmd, uint16_t addr, uint16_t value, uint16_t mask);

//Direct register access macros
#define RD(dev, addr, value) indy_direct_reg_rd(dev, addr, value)
#define WR(dev, addr, value) indy_direct_reg_wr(dev, addr, value, 0xffff)
#define WRM(dev, addr, val, mask) indy_direct_reg_wr(dev, addr, val, mask)

//Extended page register access macros
#define EP_RD(dev, page_addr, value) indy_ext_reg_rd(dev, page_addr, value)
#define EP_WR(dev, page_addr, value) indy_ext_reg_wr(dev, page_addr, value, 0xffff)
#define EP_WRM(dev, page_addr, value, mask) indy_ext_reg_wr(dev, page_addr, value, mask)

//MMD device register access macros
#define MMD_RD(dev, mmd_addr, value) indy_mmd_reg_rd(dev, mmd_addr, value)
#define MMD_WR(dev, mmd_addr, value) indy_mmd_reg_wr(dev, mmd_addr, value, 0xffff)
#define MMD_WRM(dev, mmd_addr, val, mask) indy_mmd_reg_wr(dev, mmd_addr, val, mask)

#define PHY_MSLEEP(m) usleep((m)*1000)

#define T_D(data, grp, format, ...) if (data->trace_func) data->trace_func(grp, MEPA_TRACE_LVL_DEBUG, __FUNCTION__, __LINE__, format, ##__VA_ARGS__);
#define T_I(data, grp, format, ...) if (data->trace_func) data->trace_func(grp, MEPA_TRACE_LVL_INFO, __FUNCTION__, __LINE__, format, ##__VA_ARGS__);
#define T_W(data, grp, format, ...) if (data->trace_func) data->trace_func(grp, MEPA_TRACE_LVL_WARNING, __FUNCTION__, __LINE__, format, ##__VA_ARGS__);
#define T_E(data, grp, format, ...) if (data->trace_func) data->trace_func(grp, MEPA_TRACE_LVL_ERROR, __FUNCTION__, __LINE__, format, ##__VA_ARGS__);

// Locking Macros
// The variable 'dev' is passed as macro argument to obtain callback pointers and call actual lock functions. It does not indicate locks per port.
#define MEPA_ENTER(dev) {                               \
    mepa_lock_t lock;                                \
    phy_data_t *lock_data = (phy_data_t *)dev->data; \
    lock.function = __FUNCTION__;                    \
    lock.file = __FILE__;                            \
    lock.line = __LINE__;                            \
    if (lock_data->lock_enter) {                     \
        lock_data->lock_enter(&lock);                \
    }                                                \
}

#define MEPA_EXIT(dev) {          \
    mepa_lock_t lock;                                \
    phy_data_t *lock_data = (phy_data_t *)dev->data; \
    lock.function = __FUNCTION__;                    \
    lock.file = __FILE__;                            \
    lock.line = __LINE__;                            \
    if (lock_data->lock_exit) {                      \
        lock_data->lock_exit(&lock);                 \
    }                                                \
}


typedef struct {
    mesa_inst_t inst;
    mesa_chip_no_t chip_no;
    miim_read_t    miim_read;
    miim_write_t   miim_write;
    mesa_miim_controller_t miim_controller;
    uint8_t        miim_addr;
} phy_switch_access_t;

typedef struct {
    uint8_t  model;
    uint8_t  rev;
} phy_dev_info_t;

typedef enum {
    INDY_TS_MODE_DISABLED = 0,      // PTP functions are disabled
    INDY_TS_MODE_STANDALONE = 1,    // PTP Standalone Mode
    INDY_TS_MODE_PCH = 2,           // PTP PCH Mode (MCH support disabled)
    INDY_TS_MODE_PCHMCH = 3,        // PTP PCH Mode (MCH support enabled)
} indy_ts_tsu_op_mode_t;

typedef struct {
    mepa_bool_t                     tsu_en;            // Port TSU enabled/disabled
    mepa_timeinterval_t             ingress_latency;   //
    mepa_timeinterval_t             egress_latency;    //
    mepa_timeinterval_t             path_delay;        //
    mepa_timeinterval_t             delay_asym;        //
    mepa_ts_scaled_ppb_t            rate_adj;          // clock rate adjustment
    mepa_ts_event_t                 event_mask;        // interrupt mask config
    mepa_ts_pps_conf_t              pps_conf;
    mepa_ts_classifier_t            rx_pkt_conf;
    mepa_ts_classifier_t            tx_pkt_conf;
    mepa_ts_ptp_clock_conf_t        rx_clock_conf;
    mepa_ts_ptp_clock_conf_t        tx_clock_conf;
} indy_ts_port_conf_t;

typedef struct {
    mepa_bool_t                   ptp_en;           // Chip PTP enabled/disabled
    indy_ts_tsu_op_mode_t         tsu_op_mode;      // TSU operating Mode: Standalone/PCH/PCH-MCH
    mepa_ts_clock_freq_t          clk_freq;         // reference clock frequency */
    mepa_ts_clock_src_t           clk_src;          // clock source
    mepa_ts_rx_timestamp_pos_t    rx_ts_pos;        // Rx timestamp position
    mepa_ts_rx_timestamp_len_t    rx_ts_len;        // Rx timestamp length
    mepa_ts_fifo_timestamp_len_t  tx_fifo_ts_len;   // Tx TS length in FIFO
    mepa_ts_fifo_mode_t           tx_fifo_mode;     // Tx TSFIFO access mode
    indy_ts_port_conf_t           ts_port_conf;     // Port specific TSU data
    mepa_bool_t                   tx_spi_en;        // TS access through SPI enaled
    mepa_ts_fifo_read_t           fifo_cb;          // Fifo TS callback
} indy_ts_data_t;

typedef struct {
    mepa_bool_t             init_done;
    mepa_port_no_t          port_no;
    phy_switch_access_t     access;
    mepa_port_interface_t   mac_if;
    mepa_driver_conf_t      conf;
    mepa_event_t            events;
    mepa_loopback_t         loopback;
    mepa_bool_t             qsgmii_phy_aneg_dis;
    phy_dev_info_t          dev;
    mepa_synce_clock_conf_t synce_conf;
    mepa_device_t           *base_dev; // Pointer to the device of base port on the phy chip
    mepa_bool_t             link_status;
	indy_ts_data_t          ts_state;

    mepa_trace_func_t       trace_func;
    mepa_lock_func_t        lock_enter;
    mepa_lock_func_t        lock_exit;
} phy_data_t;

#endif