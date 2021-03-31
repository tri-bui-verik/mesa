// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#include <microchip/ethernet/phy/api.h>
#include <microchip/ethernet/switch/api.h>

#include "indy_registers.h"
#include "indy_private.h"

typedef struct {
    mesa_inst_t inst;
    mesa_chip_no_t chip_no;
    miim_read_t    miim_read;
    miim_write_t   miim_write;
    mesa_miim_controller_t miim_controller;
    uint8_t        miim_addr;
} phy_switch_access_t;

typedef struct {
    mepa_bool_t init_done;
    mepa_port_no_t port_no;
    phy_switch_access_t access;
    mepa_port_interface_t mac_if;
    mepa_driver_conf_t conf;
    mepa_event_t events;
    mepa_loopback_t loopback;
} phy_data_t;

static mepa_bool_t qsgmii_hard_rst;

static mepa_rc indy_conf_set(mepa_device_t *dev, const mepa_driver_conf_t *config);

mepa_rc indy_direct_reg_rd(mepa_device_t *dev, uint16_t addr, uint16_t *value)
{
    phy_data_t *data = (phy_data_t *)dev->data;
    phy_switch_access_t *access = &data->access;
    miim_read_t read = access->miim_read;

    // Need to unlock to allow locking in switch Api's miim read
    MEPA_EXIT();
    read(NULL, access->chip_no, access->miim_controller, access->miim_addr, addr, value);
    MEPA_ENTER();
    return MEPA_RC_OK;
}
mepa_rc indy_direct_reg_wr(mepa_device_t *dev, uint16_t addr, uint16_t value, uint16_t mask)
{
    phy_data_t *data = (phy_data_t *)dev->data;
    phy_switch_access_t *access = &data->access;
    miim_read_t read = access->miim_read;
    miim_write_t write = access->miim_write;
    uint16_t reg_val = value;

    // Need to unlock to allow locking in switch Api's miim read
    MEPA_EXIT();
    read(NULL, access->chip_no, access->miim_controller, access->miim_addr, addr, &reg_val);
    if (mask != INDY_DEF_MASK) {
        reg_val = (reg_val & ~mask) | (value & mask);
    } else {
        reg_val = value;
    }
    write(NULL, access->chip_no, access->miim_controller, access->miim_addr, addr, reg_val);
    MEPA_ENTER();
    return MEPA_RC_OK;
}

// Extended page read and write functions
// Extended page numbers range : 0 - 31
mepa_rc indy_ext_reg_rd(mepa_device_t *dev, uint16_t page, uint16_t addr, uint16_t *value)
{
    // Set-up to access extended page register.
    MEPA_RC(indy_direct_reg_wr(dev, INDY_EXT_PAGE_ACCESS_CTRL, page, INDY_DEF_MASK));
    MEPA_RC(indy_direct_reg_wr(dev, INDY_EXT_PAGE_ACCESS_ADDR_DATA, addr, INDY_DEF_MASK));
    MEPA_RC(indy_direct_reg_wr(dev, INDY_EXT_PAGE_ACCESS_CTRL,
                INDY_F_EXT_PAGE_ACCESS_CTRL_EP_FUNC | page, INDY_DEF_MASK));

    // Read the value
    MEPA_RC(indy_direct_reg_rd(dev, INDY_EXT_PAGE_ACCESS_ADDR_DATA, value));
    return MEPA_RC_OK;
}
mepa_rc indy_ext_reg_wr(mepa_device_t *dev, uint16_t page, uint16_t addr, uint16_t value, uint16_t mask)
{
    // Set-up to access extended page register.
    MEPA_RC(indy_direct_reg_wr(dev, INDY_EXT_PAGE_ACCESS_CTRL, page, INDY_DEF_MASK));
    MEPA_RC(indy_direct_reg_wr(dev, INDY_EXT_PAGE_ACCESS_ADDR_DATA, addr, INDY_DEF_MASK));
    MEPA_RC(indy_direct_reg_wr(dev, INDY_EXT_PAGE_ACCESS_CTRL,
                INDY_F_EXT_PAGE_ACCESS_CTRL_EP_FUNC | page, INDY_DEF_MASK));

    // write the value
    MEPA_RC(indy_direct_reg_wr(dev, INDY_EXT_PAGE_ACCESS_ADDR_DATA, value, mask));
    return MEPA_RC_OK;
}

// MMD read and write functions
// MMD device range : 0 - 31
mepa_rc indy_mmd_reg_rd(mepa_device_t *dev, uint16_t mmd, uint16_t addr, uint16_t *value)
{
    // Set-up to MMD register.
    MEPA_RC(indy_direct_reg_wr(dev, INDY_MMD_ACCESS_CTRL, mmd, INDY_DEF_MASK));
    MEPA_RC(indy_direct_reg_wr(dev, INDY_MMD_ACCESS_ADDR_DATA, addr, INDY_DEF_MASK));
    MEPA_RC(indy_direct_reg_wr(dev, INDY_MMD_ACCESS_CTRL,
                INDY_F_MMD_ACCESS_CTRL_MMD_FUNC | mmd, INDY_DEF_MASK));

    // Read the value
    MEPA_RC(indy_direct_reg_rd(dev, INDY_MMD_ACCESS_ADDR_DATA, value));
    return MEPA_RC_OK;
}

mepa_rc indy_mmd_reg_wr(mepa_device_t *dev, uint16_t mmd, uint16_t addr, uint16_t value, uint16_t mask)
{
    // Set-up to MMD register.
    MEPA_RC(indy_direct_reg_wr(dev, INDY_MMD_ACCESS_CTRL, mmd, INDY_DEF_MASK));
    MEPA_RC(indy_direct_reg_wr(dev, INDY_MMD_ACCESS_ADDR_DATA, addr, INDY_DEF_MASK));
    MEPA_RC(indy_direct_reg_wr(dev, INDY_MMD_ACCESS_CTRL,
                INDY_F_MMD_ACCESS_CTRL_MMD_FUNC | mmd, INDY_DEF_MASK));

    // write the value
    MEPA_RC(indy_direct_reg_wr(dev, INDY_MMD_ACCESS_ADDR_DATA, value, mask));
    return MEPA_RC_OK;
}

static mepa_rc indy_delete(mepa_device_t *dev)
{
    phy_data_t *data = (phy_data_t *)dev->data;
    free(data);
    free(dev);
    dev = NULL;
    return MEPA_RC_OK;
}

static mepa_device_t *indy_probe(
    mepa_driver_t *drv, const mepa_driver_address_t *mode) {
    if (mode->mode != mscc_phy_driver_address_mode) return NULL;
    mepa_device_t *device =
        (mepa_device_t *)calloc(1, sizeof(mepa_device_t));

    if (device == NULL) goto out_device;

    phy_data_t *data =
        (phy_data_t *)calloc(1, sizeof(phy_data_t));

    if (data == NULL) goto out_data;

    device->drv = drv;
    data->port_no = mode->val.mscc_address.port_no;
    data->access.miim_read = mode->val.mscc_address.miim_read;
    data->access.miim_write = mode->val.mscc_address.miim_write;
    data->mac_if = mode->val.mscc_address.mac_if;
    data->access.miim_controller = mode->val.mscc_address.miim_controller;
    data->access.miim_addr = mode->val.mscc_address.miim_addr;
    data->access.chip_no = mode->val.mscc_address.chip_no;
    data->events = 0;
    device->data = data;

    return device;

out_data:
    free(device);
out_device:
    return NULL;
}
static uint16_t get_base_addr(mepa_device_t *dev)
{
    uint16_t val;

    EP_RD(dev, INDY_STRAP_STATUS_1, &val);
    return INDY_X_STRAP_STATUS_STRAP_PHYAD(val);
}

static mepa_rc indy_init_conf(mepa_device_t *dev)
{
    // QSGMII hard reset
    if (!qsgmii_hard_rst) {
        EP_WR(dev, INDY_QSGMII_HARD_RESET, 1);
        qsgmii_hard_rst = 1;
        PHY_MSLEEP(1);
        // Disable QSGMII auto-negotiation
        EP_WRM(dev, INDY_QSGMII_PCS1G_ANEG_CONFIG, 0, INDY_F_QSGMII_PCS1G_ANEG_CONFIG_ANEG_ENA);
    }
    // Disable QSGMII auto-negotiation
    EP_WRM(dev, INDY_QSGMII_AUTO_ANEG, 0, INDY_F_QSGMII_AUTO_ANEG_AUTO_ANEG_ENA);
}

static mepa_rc indy_rev_a_workaround(mepa_device_t *dev)
{
    EP_WR(dev, INDY_OPERATION_MODE_STRAP_LOW, 0x2);
    EP_WR(dev, INDY_OPERATION_MODE_STRAP_HIGH, 0xc001);
}

static mepa_rc indy_reset(mepa_device_t *dev, const mepa_reset_param_t *rst_conf)
{
    phy_data_t *data = (phy_data_t *) dev->data;
    MEPA_ENTER();
    if (!data->init_done) {
        indy_init_conf(dev);
        data->init_done = TRUE;
    }
    indy_rev_a_workaround(dev);
    WRM(dev, INDY_BASIC_CONTROL, INDY_F_BASIC_CTRL_SOFT_RESET, INDY_F_BASIC_CTRL_SOFT_RESET);
    PHY_MSLEEP(1);
    MEPA_EXIT();
    // Reconfigure the phy after reset
    indy_conf_set(dev, &data->conf);
    return MEPA_RC_OK;
}

static mepa_rc indy_poll(mepa_device_t *dev, mepa_driver_status_t *status)
{
    uint16_t val, val2 = 0;
    phy_data_t *data = (phy_data_t *) dev->data;

    MEPA_ENTER();
    RD(dev, INDY_BASIC_STATUS, &val);
    status->link = (val & INDY_F_BASIC_STATUS_LINK_STATUS) ? 1 : 0;
    if (data->conf.speed == MEPA_SPEED_AUTO) {
        uint16_t lp_sym_pause = 0, lp_asym_pause = 0;
        uint8_t ext_status = 0;
        // Default values
        status->speed = MEPA_SPEED_UNDEFINED;
        status->fdx = 1;
        // check if auto-negotiation is completed or not.
        if (!(val & INDY_F_BASIC_STATUS_ANEG_COMPLETE)) {
            status->link = 0;
            goto end;
        }
        // Obtain speed and duplex from link partner's advertised capability.
        RD(dev, INDY_ANEG_LP_BASE, &val);
        RD(dev, INDY_ANEG_MSTR_SLV_STATUS, &val2);
        // 1G half duplex is not supported. Refer direct register - 9
        if (val2 & INDY_F_ANEG_MSTR_SLV_STATUS_1000_T_FULL_DUP) {
            status->speed = MEPA_SPEED_1G;
            status->fdx = 1;
        } else if (val & INDY_F_ANEG_LP_BASE_100_X_FULL_DUP) {
            status->speed = MEPA_SPEED_100M;
            status->fdx = 1;
        } else if (val & INDY_F_ANEG_LP_BASE_100_X_HALF_DUP) {
            status->speed = MEPA_SPEED_100M;
            status->fdx = 0;
        } else if (val & INDY_F_ANEG_LP_BASE_10_T_FULL_DUP) {
            status->speed = MEPA_SPEED_10M;
            status->fdx = 1;
        } else if (val & INDY_F_ANEG_LP_BASE_10_T_HALF_DUP) {
            status->speed = MEPA_SPEED_10M;
            status->fdx = 0;
        }
        // Get flow control status
        lp_sym_pause = (val & INDY_F_ANEG_LP_BASE_SYM_PAUSE) ? 1 : 0;
        lp_asym_pause = (val & INDY_F_ANEG_LP_BASE_ASYM_PAUSE) ? 1 : 0;
        status->aneg.obey_pause = data->conf.flow_control && (lp_sym_pause || lp_asym_pause);
        status->aneg.generate_pause = data->conf.flow_control && lp_sym_pause;
    } else {
        uint8_t speed;
        // Forced speed
        MEPA_RC(RD(dev, INDY_BASIC_CONTROL, &val2));
        speed = !!(val2 & INDY_F_BASIC_CTRL_SPEED_SEL_BIT_0) |
                (!!(val2 & INDY_F_BASIC_CTRL_SPEED_SEL_BIT_1) << 1);
        status->speed = (speed == 0) ? MEPA_SPEED_10M :
                        (speed == 1) ? MEPA_SPEED_100M :
                        (speed == 2) ? MEPA_SPEED_1G : MEPA_SPEED_UNDEFINED;
        status->fdx = !!(val & INDY_F_BASIC_CTRL_DUP_MODE);
        //check that aneg is not enabled.
        if (val2 & INDY_F_BASIC_CTRL_ANEG_ENA) {
            // TODO: Enable trace
        }
    }

    end:
    MEPA_EXIT();
    return MEPA_RC_OK;
}

static mepa_rc indy_conf_set(mepa_device_t *dev, const mepa_driver_conf_t *config)
{
    phy_data_t *data = (phy_data_t *)dev->data;
    uint16_t new_value, mask, old_value;
    data->conf = *config;
    mepa_bool_t restart_aneg = FALSE;

    MEPA_ENTER();
    data->conf = *config;
    if (config->admin.enable) {
        if (config->speed == MEPA_SPEED_AUTO) {
            RD(dev, INDY_ANEG_MSTR_SLV_CTRL, &old_value);
            new_value = config->aneg.speed_1g_fdx ? INDY_F_ANEG_MSTR_SLV_CTRL_1000_T_FULL_DUP : 0;
            if ((old_value & INDY_F_ANEG_MSTR_SLV_CTRL_1000_T_FULL_DUP) != new_value) {
                restart_aneg = TRUE;
            }
            WRM(dev, INDY_ANEG_MSTR_SLV_CTRL, new_value,
                INDY_F_ANEG_MSTR_SLV_CTRL_1000_T_FULL_DUP);
            // Set up auo-negotiation advertisement in register 4
            new_value = (((config->aneg.tx_remote_fault ? 1 : 0) << 13) |
                     ((config->flow_control ? 1 : 0) << 11) |
                     ((config->flow_control ? 1 : 0) << 10) |
                     ((config->aneg.speed_100m_fdx ? 1 : 0) << 8) |
                     ((config->aneg.speed_100m_hdx ? 1 : 0) << 7) |
                     ((config->aneg.speed_10m_fdx ? 1 : 0) << 6) |
                     ((config->aneg.speed_10m_hdx ? 1 : 0) << 5) |
                     (1 << 0)); // default selector field - 1
            RD(dev, INDY_ANEG_ADVERTISEMENT, &old_value);
            if (old_value != new_value) {
                restart_aneg = TRUE;
            }
            WR(dev, INDY_ANEG_ADVERTISEMENT, new_value);
            // Enable & restart auto-negotiation
            new_value = INDY_F_BASIC_CTRL_ANEG_ENA;
            WRM(dev, INDY_BASIC_CONTROL, new_value, new_value | INDY_F_BASIC_CTRL_SOFT_POW_DOWN);
            if (restart_aneg) {
                WRM(dev, INDY_BASIC_CONTROL, INDY_F_BASIC_CTRL_RESTART_ANEG, INDY_F_BASIC_CTRL_RESTART_ANEG);
            }
        } else if (config->speed != MEPA_SPEED_UNDEFINED) {
            new_value = ((config->speed == MEPA_SPEED_100M ? 1 : 0) << 13) | (0 << 12) |
                    ((config->fdx ? 1 : 0) << 8) |
                    ((config->speed == MEPA_SPEED_1G ? 1 : 0) << 6);
            mask = INDY_BIT(13) | INDY_BIT(12) | INDY_BIT(8) | INDY_BIT(6) | INDY_F_BASIC_CTRL_SOFT_POW_DOWN | INDY_F_BASIC_CTRL_ANEG_ENA;
            WRM(dev, INDY_BASIC_CONTROL, new_value, mask);
        }
    } else {
        // set soft power down bit
        WRM(dev, INDY_BASIC_CONTROL, INDY_F_BASIC_CTRL_SOFT_POW_DOWN, INDY_F_BASIC_CTRL_SOFT_POW_DOWN);
    }
    MEPA_EXIT();

    return MEPA_RC_OK;
}

static mepa_rc indy_if_get(mepa_device_t *dev, mepa_port_speed_t speed,
                           mepa_port_interface_t *mac_if)
{
    phy_data_t *data = (phy_data_t *)dev->data;
    // Indy uses only QSGMII and hence returned always.
    *mac_if = data->mac_if;
    return MEPA_RC_OK;
}

static mepa_rc indy_power_set(mepa_device_t *dev, mepa_power_mode_t power)
{
    MEPA_ENTER();

    MEPA_EXIT();
    return MEPA_RC_OK;
}

static mepa_rc indy_cable_diag_start(mepa_device_t *dev, int mode)
{
    uint16_t value, mask = 0;

    MEPA_ENTER();
    //check if cable diagnostics has not started.
    RD(dev, INDY_CABLE_DIAG, &value);
    if (!(value & INDY_F_CABLE_DIAG_TEST_ENA)) {
        value |= INDY_F_CABLE_DIAG_TEST_ENA;
        value |= INDY_F_CABLE_TEST_PAIR(0); // Pair A by default
        mask |= INDY_F_CABLE_DIAG_TEST_ENA | INDY_M_CABLE_TEST_PAIR;
        WRM(dev, INDY_CABLE_DIAG, value, mask);
    }
    MEPA_EXIT();
    return MEPA_RC_OK;
}

static mepa_rc indy_cable_diag_get(mepa_device_t *dev, mepa_cable_diag_result_t *res)
{
    uint16_t value, status;
    mepa_rc rc = MEPA_RC_OK;
    MEPA_ENTER();
    RD(dev, INDY_CABLE_DIAG, &value);
    if (value & INDY_F_CABLE_DIAG_TEST_ENA) {
        res->status[0] = MEPA_CABLE_DIAG_STATUS_RUNNING; // Pair A
        rc = MEPA_RC_INCOMPLETE;
    } else {
        status = INDY_X_CABLE_DIAG_STATUS(value);
        switch (status) { // Only Pair A result
        case 0: res->status[0] = MEPA_CABLE_DIAG_STATUS_OK;
                res->length[0] = 0; // cannot measure length
                res->link = TRUE;
                break;
        case 1: res->status[0] = MEPA_CABLE_DIAG_STATUS_OPEN;
                break;
        case 2: res->status[0] = MEPA_CABLE_DIAG_STATUS_SHORT;
                break;
        case 3: res->status[0] = MEPA_CABLE_DIAG_STATUS_ABNORM;
                break;
        default:res->status[0] = MEPA_CABLE_DIAG_STATUS_OPEN;
                break;
        }
    }
    MEPA_EXIT();
    return MEPA_RC_OK;
}

static mepa_rc indy_aneg_status_get(mepa_device_t *dev, mepa_aneg_status_t *status)
{
    uint16_t val;

    MEPA_ENTER();
    RD(dev, INDY_ANEG_MSTR_SLV_STATUS, &val);
    status->master_cfg_fault = (val & INDY_F_ANEG_MSTR_SLV_STATUS_CFG_FAULT) ? TRUE : FALSE;
    status->master = val & INDY_F_ANEG_MSTR_SLV_STATUS_CFG_RES ? TRUE : FALSE;
    MEPA_EXIT();
    return MEPA_RC_OK;
}

// read direct registers for debugging
static mepa_rc indy_direct_reg_read(mepa_device_t *dev, uint32_t address, uint16_t *const value)
{
    mepa_rc rc;
    uint16_t addr = address & 0x1f;

    MEPA_ENTER();
    rc = indy_direct_reg_rd(dev, addr, value);
    MEPA_EXIT();
    return rc;
}

// write direct registers. Used for debugging.
static mepa_rc indy_direct_reg_write(mepa_device_t *dev, uint32_t address, uint16_t value)
{
    mepa_rc rc;
    uint16_t addr = address & 0x1f;

    MEPA_ENTER();
    rc = indy_direct_reg_wr(dev, addr, value, 0xFFFF);
    MEPA_EXIT();
    return rc;
}

// read extended page/mmd register for debugging
static mepa_rc indy_ext_mmd_reg_read(mepa_device_t *dev, uint32_t address, uint16_t *const value)
{
    mepa_rc rc;
    uint16_t page_mmd = (address >> 16) & 0xffff;
    uint16_t addr = address & 0xffff;
    uint16_t mmd = (page_mmd >> 11);

    MEPA_ENTER();
    if (mmd) {
        rc = indy_mmd_reg_rd(dev, mmd, addr, value);
    } else {
        rc = indy_ext_reg_rd(dev, page_mmd, addr, value);
    }
    MEPA_EXIT();
    return rc;
}

// write extended page/mmd register. Used for debugging.
static mepa_rc indy_ext_mmd_reg_write(mepa_device_t *dev, uint32_t address, uint16_t value)
{
    mepa_rc rc;
    uint16_t page_mmd = (address >> 16) & 0xffff;
    uint16_t addr = address & 0xffff;
    uint16_t mmd = (page_mmd >> 11);

    MEPA_ENTER();
    if (mmd) {
        rc = indy_mmd_reg_wr(dev, mmd, addr, value, 0xFFFF);
    } else {
        rc = indy_ext_reg_wr(dev, page_mmd, addr, value, 0xFFFF);
    }
    MEPA_EXIT();
    return rc;
}

// Enable events
static mepa_rc indy_event_enable_set(mepa_device_t *dev, mepa_event_t event, mesa_bool_t enable)
{
    mepa_rc rc = MEPA_RC_OK;
    uint16_t ev_mask;
    phy_data_t *data = (phy_data_t *)dev->data;
    MEPA_ENTER();
    RD(dev, INDY_GPHY_INTR_ENA, &ev_mask);
    switch(event) {
        case MEPA_LINK_LOS:
            ev_mask = enable ? (ev_mask | INDY_F_GPHY_INTR_ENA_LINK_DOWN) :
                               (ev_mask & ~INDY_F_GPHY_INTR_ENA_LINK_DOWN);
            break;
        case MEPA_FAST_LINK_FAIL:
            ev_mask = enable ? (ev_mask | INDY_F_GPHY_INTR_ENA_FLF_INTR) :
                               (ev_mask & ~INDY_F_GPHY_INTR_ENA_FLF_INTR);
            break;
        default:
            // Not yet implemented
            break;
    }
    WR(dev, INDY_GPHY_INTR_ENA, ev_mask);
    data->events = enable ? (data->events | event) :
                            (data->events & ~event);
    MEPA_EXIT();
    return rc;
}

// Get current enabled events
static mepa_rc indy_event_enable_get(mepa_device_t *dev, mepa_event_t *const event)
{
    mepa_rc rc = MEPA_RC_OK;
    phy_data_t *data = (phy_data_t *)dev->data;
    MEPA_ENTER();
    *event = data->events;
    MEPA_EXIT();
    return rc;
}

// Poll the status of currently enabled events
static mepa_rc indy_event_status_poll(mepa_device_t *dev, mepa_event_t *const status)
{
    uint16_t val;
    mepa_rc rc = MEPA_RC_OK;
    phy_data_t *data = (phy_data_t *)dev->data;
    *status = 0;
    MEPA_ENTER();
    rc = RD(dev, INDY_GPHY_INTR_STATUS, &val);
    if (val & INDY_F_GPHY_INTR_ENA_LINK_DOWN) {
        *status |= data->events & MEPA_LINK_LOS;
    }
    if (val & INDY_F_GPHY_INTR_ENA_FLF_INTR) {
        *status |= data->events & MEPA_FAST_LINK_FAIL;
    }
    MEPA_EXIT();
    return rc;
}

// Set loopback modes in phy
static mepa_rc indy_loopback_set(mepa_device_t *dev, mepa_loopback_t loopback)
{
    uint16_t val = 0;
    phy_data_t *data = (phy_data_t *)dev->data;

    MEPA_ENTER();
    // Far end loopback
    if (loopback.far_end_enable == TRUE) {
        // TODO: Check if we need to set-up speed configuration.
        WRM(dev, INDY_PCS_LOOP_POLARITY_CTRL, INDY_F_PCS_LOOP_CTRL_PORT_LOOP,
                 INDY_F_PCS_LOOP_CTRL_PORT_LOOP);
    }
    data->loopback = loopback;
    MEPA_EXIT();
    return MEPA_RC_OK;
}
// Returns gpio using port number and led number
static uint8_t led_num_to_gpio_mapping(mepa_device_t *dev, mepa_led_num_t led_num)
{
    phy_data_t *data = (phy_data_t *) dev->data;
    uint16_t port_addr = data->access.miim_addr - get_base_addr(dev);
    uint8_t gpio = 11;// port 0 as default.
    switch(port_addr) {
        case 0:
               gpio = led_num == MEPA_LED0 ? 11 : 12;
               break;
        case 1:
               gpio = led_num == MEPA_LED0 ? 17 : 18;
               break;
        case 2:
               gpio = led_num == MEPA_LED0 ? 19 : 20;
               break;
        case 3:
               gpio = led_num == MEPA_LED0 ? 13 : 14;
               break;
        default:
               // invalid.
               break;
    }
    return gpio;
}
static uint8_t led_mepa_mode_to_indy(mepa_gpio_mode_t mode)
{
    uint8_t ret=0;
    switch(mode) {
        case MEPA_GPIO_MODE_LED_LINK_ACTIVITY: ret = 0;
             break;
        case MEPA_GPIO_MODE_LED_LINK1000_ACTIVITY: ret = 1;
             break;
        case MEPA_GPIO_MODE_LED_LINK100_ACTIVITY: ret = 2;
             break;
        case MEPA_GPIO_MODE_LED_LINK10_ACTIVITY: ret = 3;
             break;
        case MEPA_GPIO_MODE_LED_LINK100_1000_ACTIVITY: ret = 4;
             break;
        case MEPA_GPIO_MODE_LED_LINK10_1000_ACTIVITY: ret = 5;
             break;
        case MEPA_GPIO_MODE_LED_LINK10_100_ACTIVITY: ret = 6;
             break;
        case MEPA_GPIO_MODE_LED_DUPLEX_COLLISION: ret = 8;
             break;
        case MEPA_GPIO_MODE_LED_COLLISION: ret = 9;
             break;
        case MEPA_GPIO_MODE_LED_ACTIVITY: ret = 10;
             break;
        case MEPA_GPIO_MODE_LED_AUTONEGOTIATION_FAULT: ret = 12;
             break;
        case MEPA_GPIO_MODE_LED_FORCE_LED_OFF: ret = 14;
            break;
        case MEPA_GPIO_MODE_LED_FORCE_LED_ON: ret = 15;
            break;
        case MEPA_GPIO_MODE_LED_DISABLE_EXTENDED: ret = 0xf0;
            break;
        default:
            ret = 0xff; // Not valid for indy
            break;
    }
    return ret;
}
// In Indy, LED0 of software Api maps to LED1 of hardware,
//          LED1 of software Api maps to LED2 of hardware.
static mepa_rc indy_led_mode_set(mepa_device_t *dev, mepa_gpio_mode_t led_mode, mepa_led_num_t led_num)
{
    uint16_t mode;
    // Indy supports only LED0 and LED1
    if ((led_num != MEPA_LED0) && (led_num != MEPA_LED1)) {
        return MEPA_RC_NOT_IMPLEMENTED;
    }
    if ((mode = led_mepa_mode_to_indy(led_mode)) == 0xff) {// Not valid
        return MEPA_RC_NOT_IMPLEMENTED;
    }
    if (mode == MEPA_GPIO_MODE_LED_DISABLE_EXTENDED) {
        // Normal operation
        EP_WRM(dev, INDY_LED_CONTROL_REG1, INDY_F_LED_CONTROL_KSZ_LED_MODE, INDY_F_LED_CONTROL_KSZ_LED_MODE);
    } else { // extended mode
        EP_WRM(dev, INDY_LED_CONTROL_REG1, 0, INDY_F_LED_CONTROL_KSZ_LED_MODE);
        EP_WRM(dev, INDY_LED_CONTROL_REG2, INDY_ENCODE_BITFIELD(mode, led_num * 4, 4), INDY_ENCODE_BITMASK(led_num * 4, 4));
    }
}
static mepa_rc indy_gpio_mode_private(mepa_device_t *dev, const mepa_gpio_conf_t *data)
{
    uint16_t gpio_en = 0, dir, val = 0, gpio_no = data->gpio_no;
    mepa_gpio_mode_t mode = data->mode;

    if (mode == MEPA_GPIO_MODE_OUT || mode == MEPA_GPIO_MODE_IN) {
        gpio_en = 1;
        dir = mode == MEPA_GPIO_MODE_OUT ? 1 : 0;
    } else if (mode >= MEPA_GPIO_MODE_LED_LINK_ACTIVITY && mode <= MEPA_GPIO_MODE_LED_DISABLE_EXTENDED) {
        MEPA_RC(indy_led_mode_set(dev, mode, data->led_num));
        // Enable alternative gpio mode for led.
        gpio_no = led_num_to_gpio_mapping(dev, data->led_num);
    }
    if (gpio_no < 16) {
        val = 1 << gpio_no;
        dir = dir << gpio_no;
        EP_WRM(dev, INDY_GPIO_EN2, gpio_en ? val : 0, val);
        if (gpio_en) {
            EP_WRM(dev, INDY_GPIO_DIR2, dir, val);
        }
    } else if (gpio_no < 24) {
        val = 1 << (gpio_no - 16);
        dir = dir << (gpio_no - 16);
        EP_WRM(dev, INDY_GPIO_EN1, gpio_en ? val : 0, val);
        if (gpio_en) {
            EP_WRM(dev, INDY_GPIO_DIR1, dir, val);
        }
    } else {
        // Not supported. Illegal for Indy.
    }
    return MEPA_RC_OK;
}

// Set gpio mode to input, output or alternate function
static mepa_rc indy_gpio_mode_set(mepa_device_t *dev, const mepa_gpio_conf_t *gpio_conf)
{
    mepa_rc rc = MEPA_RC_OK;
    // Indy has 0-23 gpios.
    if (gpio_conf->gpio_no > 23) {
        return MEPA_RC_NOT_IMPLEMENTED;
    }
    MEPA_ENTER();
    rc = indy_gpio_mode_private(dev, gpio_conf);
    MEPA_EXIT();
    return rc;
}
static mepa_rc indy_gpio_out_set(mepa_device_t *dev, uint8_t gpio_no, mepa_bool_t value)
{
    uint16_t val = 0;
    // Indy has 0-23 gpios.
    if (gpio_no > 23) {
        return MEPA_RC_NOT_IMPLEMENTED;
    }
    MEPA_ENTER();
    if (gpio_no < 16) {
        val = 1 << gpio_no;
        EP_WRM(dev, INDY_GPIO_DATA2, value ? val : 0, val);
    } else if (gpio_no < 24) {
        val = 1 << (gpio_no - 16);
        EP_WRM(dev, INDY_GPIO_DATA1, value ? val : 0, val);
    } else {
        // Not supported. Illegal for Indy.
    }

    MEPA_EXIT();
    return MEPA_RC_OK;
}
static mepa_rc indy_gpio_in_get(mepa_device_t *dev, uint8_t gpio_no, mepa_bool_t * const value)
{
    uint16_t val = 0;
    // Indy has 0-23 gpios.
    if (gpio_no > 23) {
        return MEPA_RC_NOT_IMPLEMENTED;
    }
    MEPA_ENTER();
    if (gpio_no < 16) {
        EP_RD(dev, INDY_GPIO_DATA2, &val);
        *value = (val >> gpio_no) & 0x1 ? TRUE : FALSE;
    } else if (gpio_no < 24) {
        EP_RD(dev, INDY_GPIO_DATA1, &val);
        *value = ((val >> (gpio_no - 16)) & 0x1) ? TRUE : FALSE;
    }
    MEPA_EXIT();
    return MEPA_RC_OK;
}
mepa_drivers_t mepa_indy_driver_init() {
    static const int nr_indy_drivers = 1;
    static mepa_driver_t indy_drivers[] = {{
        .id = 0x221400,
        .mask = 0xff0000,
        .mepa_driver_delete = indy_delete,
        .mepa_driver_reset = indy_reset,
        .mepa_driver_poll = indy_poll,
        .mepa_driver_conf_set = indy_conf_set,
        .mepa_driver_if_get = indy_if_get,
        .mepa_driver_power_set = indy_power_set,
        .mepa_driver_cable_diag_start = indy_cable_diag_start,
        .mepa_driver_cable_diag_get = indy_cable_diag_get,
        .mepa_driver_probe = indy_probe,
        .mepa_driver_aneg_status_get = indy_aneg_status_get,
        .mepa_driver_clause22_read = indy_direct_reg_read,
        .mepa_driver_clause22_write = indy_direct_reg_write,
        .mepa_driver_clause45_read  = indy_ext_mmd_reg_read,
        .mepa_driver_clause45_write = indy_ext_mmd_reg_write,
        .mepa_driver_event_enable_set = indy_event_enable_set,
        .mepa_driver_event_enable_get = indy_event_enable_get,
        .mepa_driver_event_poll = indy_event_status_poll,
        .mepa_driver_loopback_set = indy_loopback_set,
        .mepa_driver_gpio_mode_set = indy_gpio_mode_set,
        .mepa_driver_gpio_out_set = indy_gpio_out_set,
        .mepa_driver_gpio_in_get = indy_gpio_in_get,
    }};

    mepa_drivers_t result;
    result.phy_drv = indy_drivers;
    result.count = nr_indy_drivers;

    return result;
}
