// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT


#include <vtss_phy_api.h>
#include <microchip/ethernet/board/api.h>

#include "meba_generic.h"

mesa_ptp_event_type_t meba_generic_ptp_source_to_event(meba_inst_t inst, meba_event_t event_id)
{
    switch (event_id) {
        case MEBA_EVENT_SYNC:
            return MESA_PTP_SYNC_EV;
        case MEBA_EVENT_EXT_SYNC:
            return MESA_PTP_EXT_SYNC_EV;
        case MEBA_EVENT_EXT_1_SYNC:
            return MESA_PTP_EXT_1_SYNC_EV;
        case MEBA_EVENT_CLK_ADJ:
            return MESA_PTP_CLK_ADJ_EV;
        case MEBA_EVENT_CLK_TSTAMP:
            return MESA_PTP_TX_TSTAMP_EV;
        case MEBA_EVENT_PTP_PIN_0:
            return MESA_PTP_PIN_0_SYNC_EV;
        case MEBA_EVENT_PTP_PIN_1:
            return MESA_PTP_PIN_1_SYNC_EV;
        case MEBA_EVENT_PTP_PIN_2:
            return MESA_PTP_PIN_2_SYNC_EV;
        case MEBA_EVENT_PTP_PIN_3:
            return MESA_PTP_PIN_3_SYNC_EV;
        default:
            T_E(inst, "Unknown event %d", event_id);
            MEBA_ASSERT(0);

    }
    return (mesa_ptp_event_type_t)0;
}

mepa_ts_event_t meba_generic_phy_ts_source_to_event(meba_inst_t inst, meba_event_t event_id)
{
    switch (event_id) {
        case MEBA_EVENT_INGR_ENGINE_ERR:
            return MEPA_TS_INGR_ENGINE_ERR;
        case MEBA_EVENT_INGR_RW_PREAM_ERR:
            return MEPA_TS_INGR_RW_PREAM_ERR;
        case MEBA_EVENT_INGR_RW_FCS_ERR:
            return MEPA_TS_INGR_RW_FCS_ERR;
        case MEBA_EVENT_EGR_ENGINE_ERR:
            return MEPA_TS_EGR_ENGINE_ERR;
        case MEBA_EVENT_EGR_RW_FCS_ERR:
            return MEPA_TS_EGR_RW_FCS_ERR;
        case MEBA_EVENT_EGR_TIMESTAMP_CAPTURED:
            return MEPA_TS_EGR_TIMESTAMP_CAPTURED;
        case MEBA_EVENT_EGR_FIFO_OVERFLOW:
            return MEPA_TS_EGR_FIFO_OVERFLOW;
        default:
            T_E(inst, "Unknown event %d", event_id);
            MEBA_ASSERT(0);
    }

    return (mepa_ts_event_t)0;
}

mesa_rc meba_generic_ptp_handler(meba_inst_t inst, meba_event_signal_t signal_notifier)
{
    mesa_ptp_event_type_t ptp_events;
    mesa_rc               rc;

    if ((rc = mesa_ptp_event_poll(NULL, &ptp_events)) != MESA_RC_OK) {
        T_E(inst, "mesa_ptp_event_poll = %d", rc);
    } else {
        int handled = 0;
        if (ptp_events) {
            if ((rc = mesa_ptp_event_enable(NULL, ptp_events, false)) != MESA_RC_OK) {
                T_E(inst, "mesa_ptp_event_enable = %d", rc);
            }

            // Fan out
            if (ptp_events & MESA_PTP_SYNC_EV) {
                signal_notifier(MEBA_EVENT_SYNC, 0);
                handled++;
            }

            if (ptp_events & MESA_PTP_EXT_SYNC_EV) {
                signal_notifier(MEBA_EVENT_EXT_SYNC, 0);
                handled++;
            }

            if (ptp_events & MESA_PTP_CLK_ADJ_EV) {
                signal_notifier(MEBA_EVENT_CLK_ADJ, 0);
                handled++;
            }

            if (ptp_events & MESA_PTP_PIN_0_SYNC_EV) {
                signal_notifier(MEBA_EVENT_PTP_PIN_0, 0);
                handled++;
            }

            if (ptp_events & MESA_PTP_PIN_1_SYNC_EV) {
                signal_notifier(MEBA_EVENT_PTP_PIN_1, 0);
                handled++;
            }

            if (ptp_events & MESA_PTP_PIN_2_SYNC_EV) {
                signal_notifier(MEBA_EVENT_PTP_PIN_2, 0);
                handled++;
            }

            if (ptp_events & MESA_PTP_PIN_3_SYNC_EV) {
                signal_notifier(MEBA_EVENT_PTP_PIN_3, 0);
                handled++;
            }
        }
        rc = handled ? MESA_RC_OK : MESA_RC_ERROR;
    }

    return rc;
}

mesa_rc meba_generic_phy_timestamp_check(meba_inst_t inst,
                                         mesa_port_no_t port_no,
                                         meba_event_signal_t signal_notifier)
{
    mesa_rc             rc;
    mepa_ts_init_conf_t ts_init_conf;

    // poll for TS interrupt only after ts_init is done
    if ((rc = meba_phy_ts_init_conf_get(inst, port_no, &ts_init_conf)) == MESA_RC_OK) {
        mepa_ts_event_t ts_events = 0;
        if ((rc = meba_phy_ts_event_poll(inst, port_no, &ts_events)) == MESA_RC_OK) {
            int handled = 0;

            T_I(inst, "ts_events: 0x%x, port = %u", ts_events, port_no);

            if ((rc = meba_phy_ts_event_set(inst, port_no, false, ts_events)) != MESA_RC_OK) {
                T_E(inst, "meba_phy_ts_event_enable_set = %d", rc);
            }

            if (ts_events & MEPA_TS_INGR_ENGINE_ERR) {
                signal_notifier(MEBA_EVENT_INGR_ENGINE_ERR, port_no);
                handled++;
            }

            if (ts_events & MEPA_TS_INGR_RW_PREAM_ERR) {
                signal_notifier(MEBA_EVENT_INGR_RW_PREAM_ERR, port_no);
                handled++;
            }

            if (ts_events & MEPA_TS_INGR_RW_FCS_ERR) {
                signal_notifier(MEBA_EVENT_INGR_RW_FCS_ERR, port_no);
                handled++;
            }

            if (ts_events & MEPA_TS_EGR_ENGINE_ERR) {
                signal_notifier(MEBA_EVENT_EGR_ENGINE_ERR, port_no);
                handled++;
            }

            if (ts_events & MEPA_TS_EGR_RW_FCS_ERR) {
                signal_notifier(MEBA_EVENT_EGR_RW_FCS_ERR, port_no);
                handled++;
            }

            if (ts_events & MEPA_TS_EGR_TIMESTAMP_CAPTURED) {
                signal_notifier(MEBA_EVENT_EGR_TIMESTAMP_CAPTURED, port_no);
                handled++;
            }

            if (ts_events & MEPA_TS_EGR_FIFO_OVERFLOW) {
                signal_notifier(MEBA_EVENT_EGR_FIFO_OVERFLOW, port_no);
                handled++;
            }

            rc = handled ? MESA_RC_OK : MESA_RC_ERROR;
        }
        T_I(inst, "ts_events: 0x%x, port = %u", ts_events, port_no);
    }

    return rc;
}

mesa_rc meba_generic_phy_event_check(meba_inst_t inst,
                                     mesa_port_no_t port_no,
                                     meba_event_signal_t signal_notifier)
{
    mepa_event_t events;
    mesa_rc      rc;

    if ((rc = meba_phy_event_poll(inst, port_no, &events)) != MESA_RC_OK) {
        T_E(inst, "meba_phy_event_poll = %d", rc);
    } else {
        int handled = 0;
        if (events) {
            T_I(inst, "Port %u, event: 0x%x", port_no, events);

            if ((rc = meba_phy_event_enable_set(inst, port_no, events, false)) != MESA_RC_OK) {
                T_E(inst, "meba_phy_event_enable_set = %d", rc);
            }

            if (events & VTSS_PHY_LINK_FFAIL_EV) {
                signal_notifier(MEBA_EVENT_FLNK, port_no);
                handled++;
            }

            if (events & VTSS_PHY_LINK_LOS_EV) {
                signal_notifier(MEBA_EVENT_LOS, port_no);
                handled++;
            }

            if (events & VTSS_PHY_LINK_AMS_EV) {
                signal_notifier(MEBA_EVENT_AMS, port_no);
                handled++;
            }
        }
        rc = handled ? MESA_RC_OK : MESA_RC_ERROR;
    }
    return rc;
}
/* Returns the phy id, which is a combination of reg2 with reg3, where reg2 is MSB. */
uint32_t meba_get_phy_id(meba_inst_t inst, uint32_t port_no, meba_port_entry_t port_entry)
{

    uint16_t reg2_val = 0;
    uint16_t reg3_val = 0;
    mesa_port_map_t map = {};
    uint32_t phy_id = 0;

    // Initialize our state
    MEBA_ASSERT(inst->private_data != NULL);
    map = port_entry.map;
    if (port_entry.cap & MEBA_PORT_CAP_VTSS_10G_PHY) {
        mesa_mmd_read(PHY_INST, map.chip_no, map.miim_controller, map.miim_addr, 30, 0, &reg3_val);
    } else {
        mesa_miim_read(NULL, map.chip_no, map.miim_controller, map.miim_addr, 2, &reg2_val);
        mesa_miim_read(NULL, map.chip_no, map.miim_controller, map.miim_addr, 3, &reg3_val);
        // maybe it is a side board, so we try to read it using mmd bus
        if (reg2_val == 0 || reg3_val == 0) {
            mesa_mmd_read(PHY_INST, map.chip_no, map.miim_controller, map.miim_addr, 0x1, 0x2, &reg2_val);
            mesa_mmd_read(PHY_INST, map.chip_no, map.miim_controller, map.miim_addr, 0x1, 0x3, &reg3_val);
        }
    }
    phy_id = ((uint32_t)reg2_val) << 16 | reg3_val;

    T_D(inst, "port %d phy_id: %x", port_no, phy_id);
    return phy_id;
}

void meba_phy_driver_init(meba_inst_t inst)
{
    mesa_port_no_t      port_no;
    meba_port_entry_t   entry;
    mepa_device_t       *phy_dev;
    // Initialize all the drivers needed

    memset(&entry, 0, sizeof(meba_port_entry_t));
    for (port_no = 0; port_no < inst->phy_device_cnt; port_no++) {

        // Identify and initialize PHYs (no CuSFPs at this point)
        // This PHY initialization needs to take place before the
        // port interface is initialized.
        // Reason for that is that some PHYs perform calibration
        // steps during their initialization that require no
        // activity on the MAC interface as it interferes with
        // the calibration.
        inst->api.meba_port_entry_get(inst, port_no, &entry);
        meba_port_cap_t port_cap = entry.cap;

        if ((port_cap & (MEBA_PORT_CAP_COPPER | MEBA_PORT_CAP_DUAL_COPPER)) ||
            (port_cap & MEBA_PORT_CAP_VTSS_10G_PHY)) {

            uint32_t phy_id = meba_get_phy_id(inst, port_no, entry);
            mepa_driver_address_t address_mode = {};
            address_mode.mode = mscc_phy_driver_address_mode;
            // Shall only be given if 10G
            address_mode.val.mscc_address.mmd_read = mesa_port_mmd_read;
            address_mode.val.mscc_address.mmd_read_inc = mesa_port_mmd_read_inc;
            address_mode.val.mscc_address.mmd_write = mesa_port_mmd_write;

            // Shall only be given if 1G
            address_mode.val.mscc_address.miim_read = mesa_miim_read;
            address_mode.val.mscc_address.miim_write = mesa_miim_write;

            // Shall only be given for vtss
            address_mode.val.mscc_address.port_miim_read = mesa_port_miim_read;
            address_mode.val.mscc_address.port_miim_write = mesa_port_miim_write;
            address_mode.val.mscc_address.inst = PHY_INST;
            address_mode.val.mscc_address.port_no = port_no;
            address_mode.val.mscc_address.meba_inst = inst;
            address_mode.val.mscc_address.mac_if = entry.mac_if;
            address_mode.val.mscc_address.miim_controller = entry.map.miim_controller;
            address_mode.val.mscc_address.miim_addr = entry.map.miim_addr;
            address_mode.val.mscc_address.chip_no = entry.map.chip_no;

            // Should really be common
            address_mode.val.mscc_address.trace_func = inst->iface.trace_func;
            address_mode.val.mscc_address.vtrace_func = inst->iface.vtrace_func;
            address_mode.val.mscc_address.lock_enter = inst->iface.lock_enter;
            address_mode.val.mscc_address.lock_exit  = inst->iface.lock_exit;

            inst->phy_devices[port_no] = mepa_create(&address_mode, phy_id);
            if (inst->phy_devices[port_no]) {
                T_I(inst, "Phy has been probed on port %d", port_no);
            } else {
                T_I(inst, "No probing");
            }
        }
    }

    // Enable accessing the shared resources by linking the base port on each port
    for (port_no = 0; port_no < inst->phy_device_cnt; port_no++) {
        inst->api.meba_port_entry_get(inst, port_no, &entry);
        phy_dev = inst->phy_devices[port_no];
        if (phy_dev && inst->phy_devices[entry.phy_base_port]) {
            (void)mepa_link_base_port(phy_dev, inst->phy_devices[entry.phy_base_port]);
        }
    }
}
