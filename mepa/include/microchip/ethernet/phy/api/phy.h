// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT


#ifndef _MICROCHIP_ETHERNET_PHY_API_PHY_H_
#define _MICROCHIP_ETHERNET_PHY_API_PHY_H_

#include <microchip/ethernet/phy/api/types.h>
#include <microchip/ethernet/switch/api/phy_1g.h>
#include <microchip/ethernet/hdr_start.h>  // ALL INCLUDE ABOVE THIS LINE

// PHY DRIVER
//
// This API defines the interface used by the switch application to interact
// with the PHY. Using this interface the switch application should not know
// which PHY it is using.
//
// Each PHY driver needs to implement the following methods for a minimum
// configuration:
//  mepa_driver_delete_t
//  mepa_driver_probe_t
//  mepa_driver_poll_t
//  mepa_driver_conf_set_t

// Contains methods that are specific to each phy.
struct mepa_driver;

// Represents an instance of the mepa_driver.
struct mepa_device;

typedef mepa_rc (*mmd_read_t)(const mesa_inst_t                inst,
                              const mepa_port_no_t             port_no,
                              const uint8_t                    mmd,
                              const uint16_t                   addr,
                              uint16_t                        *const value);

typedef mepa_rc (*mmd_write_t)(const mesa_inst_t               inst,
                               const mepa_port_no_t            port_no,
                               const uint8_t                   mmd,
                               const uint16_t                  addr,
                               const uint16_t                  value);

typedef mepa_rc (*miim_read_t)(const mesa_inst_t               inst,
                               const mepa_chip_no_t            chip_no,
                               const mesa_miim_controller_t    miim_controller,
                               const uint8_t                   miim_addr,
                               const uint8_t                   addr,
                               uint16_t                       *const value);

typedef mepa_rc (*miim_write_t)(const mesa_inst_t              inst,
                                const mepa_chip_no_t           chip_no,
                                const mesa_miim_controller_t   miim_controller,
                                const uint8_t                  miim_addr,
                                const uint8_t                  addr,
                                const uint16_t                 value);

typedef void (*debug_func_t)(mepa_trace_level_t                level,
                             const char                       *location,
                             uint32_t                          line_no,
                             const char                       *fmt,
                                                               ...);

// Address mode that is specific for mscc phy.
typedef struct {
    mmd_read_t              mmd_read;
    mmd_write_t             mmd_write;
    miim_read_t             miim_read;
    miim_write_t            miim_write;
    mesa_inst_t             inst;
    struct meba_inst        *meba_inst;
    mepa_port_no_t          port_no;
    debug_func_t            debug_func;
    mepa_port_interface_t   mac_if;
} mscc_phy_driver_address_t;

// Union that contains all the values for address mode. Enumeration
// mepa_driver_address_mode_t decides which address type to be used.
typedef union {
    mscc_phy_driver_address_t mscc_address;
} mepa_driver_address_val_t;

// Enumeration of all possible address modes.
typedef enum {
    mscc_phy_driver_address_mode,
} mepa_driver_address_mode_t;

// Main structure that contains the address mode and the address value, these
// values has to be filled up by the switch application.
typedef struct mepa_driver_address {
    mepa_driver_address_mode_t mode;
    mepa_driver_address_val_t val;
} mepa_driver_address_t;

// Represents the status of the PHY.
typedef struct {
    mepa_bool_t link;        // Link is up
    mepa_port_speed_t speed; // Speed
    mepa_bool_t fdx;         // Full duplex
    mepa_aneg_t aneg;        // Auto-negotiation
    mepa_bool_t copper;      // For dual-media ports
    mepa_bool_t fiber;       // For dual-media ports
} mepa_driver_status_t;

// Represents the configuration that is applied to PHY.
typedef struct {
    mepa_port_speed_t speed;       // Forced port speed
    mepa_bool_t fdx;               // Forced duplex mode
    mepa_bool_t flow_control;      // Flow control (Standard 802.3x)
    uint32_t adv_dis;              // Auto neg advertisement disable
    mepa_port_admin_state_t admin; // Admin state
    mepa_aneg_adv_t aneg;          // Auto-negitiation advertisement
} mepa_driver_conf_t;

// Advertise disable flags.
typedef enum {
    MEPA_ADV_DIS_HDX = 0x00000001,     // Disable Half duplex
    MEPA_ADV_DIS_FDX = 0x00000002,     // Disable Full duplex
    MEPA_ADV_UP_MEP_LOOP = 0x00000004, // Use port for UP MEP loop port
    MEPA_ADV_DIS_2500M = 0x00000008,   // Disable 2.5G mode
    MEPA_ADV_DIS_1G = 0x00000010,      // Disable 1G mode
    MEPA_ADV_DIS_100M = 0x00000040,    // Disable 100Mbit mode
    MEPA_ADV_DIS_10M = 0x00000080,     // Disable 10Mbit mode
    MEPA_ADV_DIS_5G = 0x00000100,      // Disable 5G mode
    MEPA_ADV_DIS_10G = 0x00000200,     // Disable 10G mode
    MEPA_ADV_DIS_RESTART_ANEG = 0x00000400, // Do not restart aneg
    MEPA_ADV_DIS_SPEED =
        (MEPA_ADV_DIS_10M | MEPA_ADV_DIS_100M | MEPA_ADV_DIS_1G |
         MEPA_ADV_DIS_2500M | MEPA_ADV_DIS_5G |
         MEPA_ADV_DIS_10G), // All speed bits
    MEPA_ADV_DIS_DUPLEX =
        (MEPA_ADV_DIS_HDX | MEPA_ADV_DIS_FDX), // All duplex bits
    MEPA_ADV_DIS_ALL = (MEPA_ADV_DIS_SPEED | MEPA_ADV_DIS_DUPLEX |
                            MEPA_ADV_UP_MEP_LOOP) // All valid bits
} mepa_adv_dis_t;

// Clears up the data allocated in the probe function.
typedef mepa_rc (*mepa_driver_delete_t)(struct mepa_device *dev);

// Resets PHY.
// intf   [IN] Interface to which to reset the phy.
typedef mepa_rc (*mepa_driver_reset_t)(
        struct mepa_device *dev,
        const mepa_reset_param_t *rst_conf);

// Get the current status of the PHY.
// status        [OUT] PHY status.
typedef mepa_rc (*mepa_driver_poll_t)(
        struct mepa_device *dev,
        mepa_driver_status_t *status);

// Set the configuration to the PHY.
// conf          [IN] PHY configuration.
typedef mepa_rc (*mepa_driver_conf_set_t)(
    struct mepa_device *dev, const mepa_driver_conf_t *conf);

// Get the PHY interface based on speed.
// speed         [IN] Speed.
// intf          [OUT] Interface that is needed to be used by the port.
typedef mepa_rc (*mepa_driver_if_get_t)(
        struct mepa_device *dev,
        mepa_port_speed_t speed,
        mepa_port_interface_t *intf);

//  Sets the power mode.
//  power         [IN] Power.
typedef mepa_rc (*mepa_driver_power_set_t)(
        struct mepa_device *dev,
        mepa_power_mode_t power);

//  Starts cable_diag.
//  mode          [IN] Mode in which to start.
typedef mepa_rc (*mepa_driver_cable_diag_start_t)(
    struct mepa_device *dev, int mode);

//  Gets result from cable diagnostics.
//  result        [OUT] Result from cable diagnostics.
typedef mepa_rc (*mepa_driver_cable_diag_get_t)(
    struct mepa_device *dev, mepa_cable_diag_result_t *result);

//  Sets the media type in case the port is a dual media port with external phy.
//  phy_media_if  [IN] Media type.
typedef mepa_rc (*mepa_driver_media_set_t)(
    struct mepa_device *dev, mepa_media_interface_t phy_media_if);

//  Create an instance of the driver and initialize the PHY.
//  mode           [IN] Address mode.
typedef struct mepa_device *(*mepa_driver_probe_t)(
    struct mepa_driver *dev, const mepa_driver_address_t *mode);

//  Gets copper PHY auto-negotiation status.
//  mode           [IN] PHY 1G status.
typedef mepa_rc (*mepa_driver_aneg_status_get_t)(
    struct mepa_device *dev, mepa_aneg_status_t *status);

// Full list of PHY driver interface
#define MEBA_LIST_OF_API_PHY_DRIVER_CALLS \
    X(mepa_driver_delete)             \
    X(mepa_driver_reset)              \
    X(mepa_driver_poll)               \
    X(mepa_driver_conf_set)           \
    X(mepa_driver_if_get)             \
    X(mepa_driver_power_set)          \
    X(mepa_driver_cable_diag_start)   \
    X(mepa_driver_cable_diag_get)     \
    X(mepa_driver_media_set)          \
    X(mepa_driver_probe)              \
    X(mepa_driver_aneg_status_get)

typedef struct mepa_driver {
    mepa_driver_delete_t            mepa_driver_delete;
    mepa_driver_reset_t             mepa_driver_reset;
    mepa_driver_poll_t              mepa_driver_poll;
    mepa_driver_conf_set_t          mepa_driver_conf_set;
    mepa_driver_if_get_t            mepa_driver_if_get;
    mepa_driver_power_set_t         mepa_driver_power_set;
    mepa_driver_cable_diag_start_t  mepa_driver_cable_diag_start;
    mepa_driver_cable_diag_get_t    mepa_driver_cable_diag_get;
    mepa_driver_media_set_t         mepa_driver_media_set;
    mepa_driver_probe_t             mepa_driver_probe;
    mepa_driver_aneg_status_get_t   mepa_driver_aneg_status_get;

    uint32_t id;                  // Id of the driver
    uint32_t mask;                // Mask of the driver
    struct mepa_driver *next; // Pointer to the next driver
} mepa_driver_t;

// Represents the instance of the driver
typedef struct mepa_device {
    // Pointer to the driver that creates the device
    mepa_driver_t *drv;

    void *data; // Private data
} mepa_device_t;

// Wrapper over an array and counter. It is used by init functions to return the
// array of drivers
typedef struct mepa_drivers {
    mepa_driver_t *phy_drv; // Pointer to an array of drivers
    unsigned int count;         // Number of entries in phy_drv
} mepa_drivers_t;

// Default driver that match any PHY
mepa_drivers_t mepa_default_phy_driver_init();

// Returns drivers for mscc PHY
mepa_drivers_t mepa_mscc_driver_init();

// Returns drivers for malibu PHY
mepa_drivers_t mepa_malibu_driver_init();

// Returns drivers for venice PHY
mepa_drivers_t mepa_venice_driver_init();

// Returns drivers for AQR PHY
mepa_drivers_t mepa_aqr_driver_init();

// Returns drivers for intel PHY
mepa_drivers_t mepa_intel_driver_init();
#include <microchip/ethernet/hdr_end.h>
#endif // _MICROCHIP_ETHERNET_PHY_API_PHY_H_