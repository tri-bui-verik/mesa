// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT


#include "vtss_lan966x_cil.h"

#if defined(VTSS_ARCH_LAN966X)

/* ================================================================= *
 *  Register access
 * ================================================================= */
void vtss_lan966x_reg_error(const char *file, int line) {
    printf("\n\nFATAL ERROR at %s:%d> Index exceed replication!\n\n", file, line);
    vtss_callout_trace_printf(VTSS_TRACE_LAYER, VTSS_TRACE_GROUP_DEFAULT,
                              VTSS_TRACE_LEVEL_ERROR, file, line, file,
                              "Index exceed replication!");
}

/* Read target register using current CPU interface */
static inline vtss_rc lan966x_rd_direct(vtss_state_t *vtss_state, u32 reg, u32 *value)
{
    return vtss_state->init_conf.reg_read(0, reg, value);
}

/* Write target register using current CPU interface */
static inline vtss_rc lan966x_wr_direct(vtss_state_t *vtss_state, u32 reg, u32 value)
{
    return vtss_state->init_conf.reg_write(0, reg, value);
}

vtss_rc (*vtss_lan966x_wr)(vtss_state_t *vtss_state, u32 addr, u32 value) = lan966x_wr_direct;
vtss_rc (*vtss_lan966x_rd)(vtss_state_t *vtss_state, u32 addr, u32 *value) = lan966x_rd_direct;

/* Read-modify-write target register using current CPU interface */
vtss_rc vtss_lan966x_wrm(vtss_state_t *vtss_state, u32 reg, u32 value, u32 mask)
{
    vtss_rc rc;
    u32     val;

    if ((rc = vtss_lan966x_rd(vtss_state, reg, &val)) == VTSS_RC_OK) {
        val = ((val & ~mask) | (value & mask));
        rc = vtss_lan966x_wr(vtss_state, reg, val);
    }
    return rc;
}

/* ================================================================= *
 *  Utility functions
 * ================================================================= */
u32 vtss_lan966x_clk_period_ps(vtss_state_t *vtss_state)
{
#if defined(VTSS_ARCH_LAN966X_FPGA)
    return 15125;
#else
    return 6154;
#endif
}

u32 vtss_lan966x_port_mask(vtss_state_t *vtss_state, const BOOL member[])
{
    vtss_port_no_t port_no;
    u32            port, mask = 0;

    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (member[port_no]) {
            port = VTSS_CHIP_PORT(port_no);
            mask |= VTSS_BIT(port);
        }
    }
    return mask;
}

vtss_rc vtss_lan966x_counter_update(vtss_state_t *vtss_state,
                                    u32 *addr, vtss_chip_counter_t *counter, BOOL clear)
{
    u32 value;

    REG_RD(SYS_CNT(*addr), &value);
    *addr = (*addr + 1); /* Next counter address */
    vtss_cmn_counter_32_update(value, counter, clear);
    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Debug print utility functions
 * ================================================================= */

void vtss_lan966x_debug_print_port_header(vtss_state_t *vtss_state,
                                          const vtss_debug_printf_t pr, const char *txt)
{
    vtss_debug_print_port_header(vtss_state, pr, txt, VTSS_CHIP_PORTS + 1, 1);
}

void vtss_lan966x_debug_print_mask(const vtss_debug_printf_t pr, u32 mask)
{
    u32 port;

    for (port = 0; port <= VTSS_CHIP_PORTS; port++) {
        pr("%s%s", port == 0 || (port & 7) ? "" : ".", ((1<<port) & mask) ? "1" : "0");
    }
    pr("  0x%08x\n", mask);
}

void vtss_lan966x_debug_reg_header(const vtss_debug_printf_t pr, const char *name)
{
    char buf[64];

    sprintf(buf, "%-32s  ", name);
    vtss_debug_print_reg_header(pr, buf);
}

void vtss_lan966x_debug_reg(vtss_state_t *vtss_state,
                            const vtss_debug_printf_t pr, u32 addr, const char *name)
{
    u32 value;
    char buf[200];

    if (vtss_lan966x_rd(vtss_state, addr, &value) == VTSS_RC_OK) {
        sprintf(buf, "%-32s  ", name);
        vtss_debug_print_reg(pr, buf, value);
    }
}

void vtss_lan966x_debug_reg_inst(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr, u32 addr, u32 i, const char *name)
{
    char buf[64];

    sprintf(buf, "%s_%u", name, i);
    vtss_lan966x_debug_reg(vtss_state, pr, addr, buf);
}

void vtss_lan966x_debug_cnt(const vtss_debug_printf_t pr, const char *col1, const char *col2,
                            vtss_chip_counter_t *c1, vtss_chip_counter_t *c2)
{
    char buf[80];

    if (col1 != NULL) {
        sprintf(buf, "rx_%s:", col1);
        pr("%-28s%10" PRIu64 "   ", buf, c1->prev);
    } else {
        pr("%-41s", "");
    }
    if (col2 != NULL) {
        sprintf(buf, "tx_%s:", strlen(col2) ? col2 : col1);
        pr("%-28s%10" PRIu64, buf, c2 ? c2->prev : (u64) 0);
    }
    pr("\n");
}

/* ================================================================= *
 *  Debug print
 * ================================================================= */
static vtss_rc lan966x_debug_info_print(vtss_state_t *vtss_state,
                                        const vtss_debug_printf_t pr,
                                        const vtss_debug_info_t   *const info)
{
    VTSS_RC(vtss_lan966x_misc_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_lan966x_port_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_lan966x_l2_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_lan966x_vcap_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_lan966x_qos_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_lan966x_packet_debug_print(vtss_state, pr, info));
    VTSS_RC(vtss_lan966x_afi_debug_print(vtss_state, pr, info));
#if defined(VTSS_FEATURE_TIMESTAMP)
    VTSS_RC(vtss_lan966x_ts_debug_print(vtss_state, pr, info));
#endif /* VTSS_FEATURE_TIMESTAMP */
#if defined(VTSS_FEATURE_VOP)
    VTSS_RC(vtss_lan966x_oam_debug_print(vtss_state, pr, info));
#endif /* VTSS_FEATURE_VOP */
    return VTSS_RC_OK;
}

vtss_rc vtss_lan966x_init_groups(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    VTSS_RC(vtss_lan966x_port_init(vtss_state, cmd));
    VTSS_RC(vtss_lan966x_misc_init(vtss_state, cmd));
    VTSS_RC(vtss_lan966x_packet_init(vtss_state, cmd));
    VTSS_RC(vtss_lan966x_afi_init(vtss_state, cmd));
    VTSS_RC(vtss_lan966x_l2_init(vtss_state, cmd));
    VTSS_RC(vtss_lan966x_vcap_init(vtss_state, cmd));
    VTSS_RC(vtss_lan966x_qos_init(vtss_state, cmd));
#if defined(VTSS_FEATURE_TIMESTAMP)
    VTSS_RC(vtss_lan966x_ts_init(vtss_state, cmd));
#endif

#if defined(VTSS_FEATURE_VOP)
    VTSS_RC(vtss_lan966x_oam_init(vtss_state, cmd));
#endif

    return VTSS_RC_OK;
}

static vtss_rc lan966x_port_map_set(vtss_state_t *vtss_state)
{
    return vtss_lan966x_init_groups(vtss_state, VTSS_INIT_CMD_PORT_MAP);
}

static vtss_rc lan966x_restart_conf_set(vtss_state_t *vtss_state)
{
    return VTSS_RC_OK;
}

static vtss_rc lan966x_init_conf_set(vtss_state_t *vtss_state)
{
    u32 val, diff, err;

    REG_RD(GCB_BUILDID, &val);
    if (val > LAN966X_BUILD_ID) {
        diff = (val - LAN966X_BUILD_ID);
    } else {
        diff = (LAN966X_BUILD_ID - val);
    }
#if defined(VTSS_CHIP_9668)
    err = (diff != 0);
#else
    err = (diff > 1000);
#endif
    if (err) {
        VTSS_E("Unexpected build id. Got: %08x, Expected %08x, diff: %u", val, LAN966X_BUILD_ID, diff);
        return VTSS_RC_ERROR;
    }

    VTSS_FUNC_RC(misc.chip_id_get, &vtss_state->misc.chip_id);

    /* Initialize RAM */
    REG_WRM(SYS_RESET_CFG, SYS_RESET_CFG_CORE_ENA(0), SYS_RESET_CFG_CORE_ENA_M);
    REG_WRM(SYS_RAM_INIT, SYS_RAM_INIT_RAM_INIT(1), SYS_RAM_INIT_RAM_INIT_M);
    do {
        REG_RD(SYS_RAM_INIT, &val);
    } while (SYS_RAM_INIT_RAM_INIT_X(val != 0));

    /* Enable switch core */
    REG_WRM(SYS_RESET_CFG, SYS_RESET_CFG_CORE_ENA(1), SYS_RESET_CFG_CORE_ENA_M);

    return vtss_lan966x_init_groups(vtss_state, VTSS_INIT_CMD_INIT);
}

static vtss_rc lan966x_register_access_mode_set(vtss_state_t *vtss_state)
{
    return VTSS_RC_OK;
}

vtss_rc vtss_lan966x_inst_create(vtss_state_t *vtss_state)
{
    /* Initialization */
    vtss_state->cil.init_conf_set = lan966x_init_conf_set;
    vtss_state->cil.register_access_mode_set = lan966x_register_access_mode_set;
    vtss_state->cil.restart_conf_set = lan966x_restart_conf_set;
    vtss_state->cil.debug_info_print = lan966x_debug_info_print;
    vtss_state->port.map_set = lan966x_port_map_set;

    /* Create function groups */
    return vtss_lan966x_init_groups(vtss_state, VTSS_INIT_CMD_CREATE);
}
#endif /* VTSS_ARCH_LAN966X */
