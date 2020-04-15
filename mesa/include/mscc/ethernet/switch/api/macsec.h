/*
 Copyright (c) 2004-2018 Microsemi Corporation "Microsemi".

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

#ifndef _MSCC_ETHERNET_SWITCH_API_MACSEC_
#define _MSCC_ETHERNET_SWITCH_API_MACSEC_

#include <mscc/ethernet/switch/api/port.h>
#include <mscc/ethernet/switch/api/misc.h>
#include <mscc/ethernet/switch/api/types.h>
#include <mscc/ethernet/switch/api/hdr_start.h>  // ALL INCLUDE ABOVE THIS LINE

#define MESA_MACSEC_SA_PER_SC_MAX  4  /**< SAs per SC Max : 4 */

#define MESA_MAC_BLOCK_MTU_MAX 0x2748                   /**< MAC Block Max MTU Size */

/*--------------------------------------------------------------------*/
/* Data types.                                                        */
/*--------------------------------------------------------------------*/

/** \brief Values of the CipherSuite control */
typedef enum {
    MESA_MACSEC_CIPHER_SUITE_GCM_AES_128,     /**< GCM-AES-128 cipher suite */
    MESA_MACSEC_CIPHER_SUITE_GCM_AES_256,     /**< GCM-AES-256 cipher suite. */
    MESA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_128, /**< GCM-AES-XPN_128 cipher suite for XPN mode. */
    MESA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_256  /**< GCM-AES-XPN_256 cipher suite for XPN mode. */
} mesa_macsec_ciphersuite_t CAP(PHY_MACSEC);


#define MESA_MACSEC_CP_RULES   (8 + 16 + 2) /**< DMAC + ETYPE + DMAC/ETYPE */

/** \brief TBD */
typedef enum {
    MESA_MACSEC_VALIDATE_FRAMES_DISABLED, /**< Do not perform integrity check */
    MESA_MACSEC_VALIDATE_FRAMES_CHECK,    /**< Perform integrity check do not drop failed frames */
    MESA_MACSEC_VALIDATE_FRAMES_STRICT    /**< Perform integrity check and drop failed frames */
} mesa_validate_frames_t CAP(PHY_MACSEC);

typedef uint16_t mesa_macsec_vport_id_t CAP(PHY_MACSEC);   /**< Virtual port Id. Corresponds to a SecY.  */
typedef uint32_t mesa_macsec_service_id_t CAP(PHY_MACSEC); /**< Encapsulation service id */

/** \brief packet number of 32-bit or 64-bit size. */
typedef union {
    uint32_t pn;  /**< packet number of 32 bit size. */
    uint64_t xpn; /**< extended packet number of 64 bit size. */
} mesa_macsec_pkt_num_t CAP(PHY_MACSEC);

/** \brief Salt for cryptographic operations */
typedef struct {
    uint8_t buf[12]; /**< Buffer containing 12 byte Salt for XPN. */
} mesa_macsec_salt_t CAP(PHY_MACSEC);

/** \brief An 128-bit or 256-bit AES key */
typedef struct {
    uint8_t buf[32];              /**< Buffer containing the key */
    uint32_t len;                 /**< Length of key in bytes (16 or 32) */
    uint8_t h_buf[16];            /**< Buffer containing the 128-bit AES key hash */
    mesa_macsec_salt_t salt; /**< salt used for XPN */
} mesa_macsec_sak_t CAP(PHY_MACSEC);

/** \brief 8 byte Secure Channel Identifier (SCI)  */
typedef struct {
    mesa_mac_t              mac_addr; /**< 6 byte MAC address */
    mesa_macsec_vport_id_t  port_id;  /**< 2 byte Port Id */
} mesa_macsec_sci_t CAP(PHY_MACSEC);

/** \brief Short SCI (SSCI). Used for XPN. */
typedef struct {
    uint8_t buf[4];   /**< Buffer containing the 4-byte SSCI for XPN. */
} mesa_macsec_ssci_t CAP(PHY_MACSEC);

/** \brief The mesa_macsec_port_t is a unique identifier to a SecY.
 * This identifier is defined by three properties:
 *  - port_no:    A reference the physical port
 *  - service_id: A reference to a given encapsulation service. The user of the
 *                API may choose any number, this is not used in hardware, but
 *                in cases where external-virtual ports are used this is
 *                required to have a unique identifier to a given SecY.
 *  - port_id:    The port ID which used in the SCI tag.
 * */
typedef struct {
    mesa_port_no_t           port_no;    /**< Physical port no */
    mesa_macsec_service_id_t service_id; /**< Service id */
    mesa_macsec_vport_id_t   port_id;    /**< Virtual port id, the port number used in the optional SCI tag */
} mesa_macsec_port_t CAP(PHY_MACSEC);

/** \brief SecY control information (802.1AE section 10.7) */
typedef struct {
    mesa_mac_t mac_addr;                    /**< Mac address of the Tx SecY */
    mesa_validate_frames_t validate_frames; /**< The validateFrames control (802.1AE section 10.7.8) */
    mesa_bool_t replay_protect;                    /**< The replayProtect control (802.1AE section 10.7.8) */
    uint32_t    replay_window;                     /**< The replayWindow control (802.1AE section 10.7.8) */
    mesa_bool_t protect_frames;                    /**< The protectFrames control (802.1AE section 10.7.17) */
    mesa_bool_t always_include_sci;                /**< The alwaysIncludeSCI control (802.1AE section 10.7.17) */
    mesa_bool_t use_es;                            /**< The useES control (802.1AE section 10.7.17) */
    mesa_bool_t use_scb;                           /**< The useSCB control (802.1AE section 10.7.17) */
    mesa_macsec_ciphersuite_t current_cipher_suite; /**< The currentCipherSuite control (802.1AE section 10.7.25) */
    uint32_t confidentiality_offset;             /**< The confidentiality Offset control (802.1AE section 10.7.25), 0-64 bytes supported */
} mesa_macsec_secy_conf_t CAP(PHY_MACSEC);

/** \brief SecY port status as defined by 802.1AE */
typedef struct {
    mesa_bool_t mac_enabled;             /**< MAC is enabled (802.1AE) */
    mesa_bool_t mac_operational;         /**< MAC is operational (802.1AE) */
    mesa_bool_t oper_point_to_point_mac; /**< Point to point oper status (802.1AE) */
} mesa_macsec_port_status_t CAP(PHY_MACSEC);

/** \brief Status for SecY ports */
typedef struct {
    mesa_macsec_port_status_t controlled;   /**< 802.1AE Controlled port status */
    mesa_macsec_port_status_t uncontrolled; /**< 802.1AE Uncontrolled port status */
    mesa_macsec_port_status_t common;       /**< 802.1AE Common port status */
} mesa_macsec_secy_port_status_t CAP(PHY_MACSEC);

/** \brief Tx SC status as defined by 802.1AE */
typedef struct {
    mesa_macsec_sci_t sci;      /**< SCI id (802.1AE) */
    mesa_bool_t transmitting;   /**< Transmitting status (802.1AE) */
    uint16_t    encoding_sa;    /**< Encoding (802.1AE) */
    uint16_t    enciphering_sa; /**< Enciphering (802.1AE)  */
    uint32_t    created_time;   /**< Created time (802.1AE) */
    uint32_t    started_time;   /**< Started time (802.1AE) */
    uint32_t    stopped_time;   /**< Stopped time (802.1AE) */
} mesa_macsec_tx_sc_status_t CAP(PHY_MACSEC);

/** \brief Rx SC status as defined by 802.1AE section 10.7 */
typedef struct {
    mesa_bool_t receiving;    /**< Receiving status (802.1AE) */
    uint32_t    created_time; /**< Created time (802.1AE) */
    uint32_t    started_time; /**< Started time (802.1AE) */
    uint32_t    stopped_time; /**< Stopped time (802.1AE) */
} mesa_macsec_rx_sc_status_t CAP(PHY_MACSEC);

/** \brief Tx SecA XPN status as defined by 802.1AE */
typedef struct {
    mesa_macsec_pkt_num_t  next_pn;  /**< Rev B Next XPN */
} mesa_macsec_tx_sa_pn_status_t CAP(PHY_MACSEC);

/** \brief Tx SA status as defined by 802.1AE */
typedef struct {
    mesa_bool_t in_use;       /**< In use (802.1AE)  */
    uint32_t    next_pn;      /**< Next PN (802.1AE) */
    uint32_t    created_time; /**< Creation time (802.1AE)*/
    uint32_t    started_time; /**< Started time (802.1AE)*/
    uint32_t    stopped_time; /**< Stopped time (802.1AE) */
    mesa_macsec_tx_sa_pn_status_t pn_status; /**< Rev B Tx SecA XPN status */
} mesa_macsec_tx_sa_status_t CAP(PHY_MACSEC);

/** \brief Rx SecA XPN status as defined by 802.1AE */
typedef struct {
    mesa_macsec_pkt_num_t next_pn; /**< Rev B Next XPN (802.1AEW) */
    mesa_macsec_pkt_num_t lowest_pn; /**< Rev B Lowest XPN */
} mesa_macsec_rx_sa_pn_status_t CAP(PHY_MACSEC);

/** \brief Rx SA status as defined by 802.1AE */
typedef struct {
    mesa_bool_t in_use;       /**< In use (802.1AE)  */
    uint32_t    next_pn;      /**< Next pn (802.1AE) */
    uint32_t    lowest_pn;    /**< Lowest_pn (802.1AE) */
    uint32_t    created_time; /**< Created time (802.1AE) */
    uint32_t    started_time; /**< Started time (802.1AE) */
    uint32_t    stopped_time; /**< Stopped time (802.1AE) */
    mesa_macsec_rx_sa_pn_status_t pn_status; /**< Rx SecA XPN status */
} mesa_macsec_rx_sa_status_t CAP(PHY_MACSEC);

/** \brief Rx SC parameters (optional) */
typedef struct {
    mesa_validate_frames_t validate_frames; /**< The validateFrames control (802.1AE section 10.7.8) */
    mesa_bool_t replay_protect;             /**< The replayProtect control (802.1AE section 10.7.8) */
    uint32_t    replay_window;              /**< The replayWindow control (802.1AE section 10.7.8) */
    uint32_t    confidentiality_offset;     /**< The confidentiality Offset control (802.1AE section 10.7.25), 0-64 bytes supported */
} mesa_macsec_rx_sc_conf_t CAP(PHY_MACSEC);

/** \brief Tx SC parameters (optional) */
typedef struct {
    mesa_bool_t protect_frames;         /**< The protectFrames control (802.1AE section 10.7.17) */
    mesa_bool_t always_include_sci;     /**< The alwaysIncludeSCI control (802.1AE section 10.7.17) */
    mesa_bool_t use_es;                 /**< The useES control (802.1AE section 10.7.17) */
    mesa_bool_t use_scb;                /**< The useSCB control (802.1AE section 10.7.17) */
    uint32_t    confidentiality_offset; /**< The confidentiality Offset control (802.1AE section 10.7.25), 0-64 bytes supported */
} mesa_macsec_tx_sc_conf_t CAP(PHY_MACSEC);

/** \brief PHY Line MAC block configuration  */
typedef struct {
    mesa_bool_t dis_length_validate;         /**< Length field (Ether Type) validation is enabled by default, if set length field validation will be disabled. */
} mesa_macsec_lmac_conf_t CAP(PHY_MACSEC);

/** \brief PHY Host MAC block configuration  */
typedef struct {
    mesa_bool_t dis_length_validate;         /**< Length field (Ether Type) validation is enabled by default, if set length field validation will be disabled. */
} mesa_macsec_hmac_conf_t CAP(PHY_MACSEC);

/** \brief PHY MAC block configuration  */
typedef struct {
    mesa_macsec_lmac_conf_t lmac; /**< Line MAC conf. */
    mesa_macsec_hmac_conf_t hmac; /**< Host MAC conf. */
} mesa_macsec_mac_conf_t CAP(PHY_MACSEC);

/** \brief PHY MACSEC block Bypass configuration  */
typedef enum {
    MESA_MACSEC_INIT_BYPASS_NONE, /**< MACSEC block bypass mode None  */
    MESA_MACSEC_INIT_BYPASS_ENABLE, /**< Enable MACSEC block bypass mode  */
    MESA_MACSEC_INIT_BYPASS_DISABLE, /**< Disable Macsec block bypass mode. */
} mesa_macsec_init_bypass_t CAP(PHY_MACSEC);

/** \brief MACsec init structure */
typedef struct {
    mesa_bool_t enable;                     /**< Enable the MACsec block  */
    mesa_bool_t dis_ing_nm_macsec_en;       /**< Disable Non Matching MACsec ingress packets processing  */
    mesa_macsec_mac_conf_t mac_conf; /**< MAC block configuration */
    mesa_macsec_init_bypass_t bypass; /**< MACSEC block Bypass configuration */
} mesa_macsec_init_t CAP(PHY_MACSEC);

/** \brief MACsec configuration of MTU for ingress and egress packets  
 *
 * If an egress MACsec packet that causes the MTU to be exceeded will cause the per-SA Out_Pkts_Too_Long*/
typedef struct {
    uint32_t  mtu;      /**< Defines the maximum packet size (in bytes) - VLAN tagged packets are allowed to be 4 bytes longer */
    mesa_bool_t drop;     /**< Set to TRUE in order to drop packets larger than mtu. Set to FALSE in order to allow packets larger than mtu to be transmitted (Out_Pkts_Too_Long will still count). Frames will be "dropped" by corrupting the frame's CRC. Packets with source port as the Common port or the reserved port are ingress, packets from the Controlled or Uncontrolled port are egress.*/
    mesa_bool_t vlan_unaware_en;     /**< Set TRUE for VLAN unaware mode. Set FALSE for VLAN aware mode.*/
} mesa_macsec_mtu_t CAP(PHY_MACSEC);

/*--------------------------------------------------------------------*/
/* MACsec Initialization                                              */
/*--------------------------------------------------------------------*/

/** \brief Initilize the MACsec block
 *
 *  When the MACsec block is disabled all frames are passed through unchanged.  This is the default state after Phy initilization.
 *  When the MACsec block is enabled,  all frames are dropped until SecY (CA) is created.
 *
 * \param inst      [IN]     MESA-API instance.
 * \param port_no   [IN]     MESA-API port no.
 * \param init      [IN]     Initilize configuration.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_init_set(const mesa_inst_t        inst,
                             const mesa_port_no_t     port_no,
                             const mesa_macsec_init_t *const init)
    CAP(PHY_MACSEC);

/** \brief Get the MACsec block init configuration
 *
 * \param inst      [IN]     MESA-API instance.
 * \param port_no   [IN]     MESA-API port no.
 * \param init      [OUT]    Initilize configuration.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_init_get(const mesa_inst_t    inst,
                             const mesa_port_no_t port_no,
                             mesa_macsec_init_t   *const init)
    CAP(PHY_MACSEC);


/*--------------------------------------------------------------------*/
/* MAC Security Entity (SecY) management                              */
/*--------------------------------------------------------------------*/

/** \brief Create a SecY entity of a MACsec port
 *
 * The entity is created with given parameters.
 * NOTE: The controlled port is disabled by default and must be enabled before normal processing.
 * NOTE: Classification pattern must be configured to classify traffic to a SecY instance
 *
 * \param inst      [IN]     MESA-API instance.
 * \param port      [IN]     MACsec port.
 * \param conf      [IN]     SecY configuration.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_secy_conf_add(const mesa_inst_t             inst,
                                  const mesa_macsec_port_t      port,
                                  const mesa_macsec_secy_conf_t *const conf)
    CAP(PHY_MACSEC);

/** \brief Create a SecY entity of a MACsec port
 *
 * The SecY is updated with given parameters.
 * Note that the SecY must exist
 *
 * \param inst      [IN]     MESA-API instance.
 * \param port      [IN]     MACsec port.
 * \param conf      [IN]     SecY configuration.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 *
 * Note: SecY update with new parameters i.e. Replay Window size etc, it will
 *       update newly create SA's only. Existing parameters i.e. Next PN and Lower PN
 *       will not change. Tx/Rx SA Status Next PN and Lowest PN shows different
 *       as compare with existing Tx/Rx SA Status.
 *
 */
mesa_rc mesa_macsec_secy_conf_update(const mesa_inst_t             inst,
                                     const mesa_macsec_port_t      port,
                                     const mesa_macsec_secy_conf_t *const conf)
    CAP(PHY_MACSEC);


/** \brief Get the SecY entry.
 *
 * \param inst       [IN]     MESA-API instance.
 * \param port       [IN]     MACsec port.
 * \param conf       [OUT]    SecY configuration.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if SecY does not exist.
 */
mesa_rc mesa_macsec_secy_conf_get(const mesa_inst_t        inst,
                                  const mesa_macsec_port_t port,
                                  mesa_macsec_secy_conf_t  *const conf)
    CAP(PHY_MACSEC);

/** \brief Delete the SecY and the associated SCs/SAs
 *
 *
 * \param inst      [IN]     MESA-API instance.
 * \param port      [IN]     MACsec port.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_secy_conf_del(const mesa_inst_t        inst,
                                  const mesa_macsec_port_t port)
    CAP(PHY_MACSEC);



/** \brief Enable/Disable the SecY's controlled (secure) port.
 *
 * The controlled port is disabled by default.
 *
 * \param inst       [IN]     MESA-API instance.
 * \param port       [IN]     MACsec port.
 * \param enable     [IN]     Forwarding state of the port.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_secy_controlled_set(const mesa_inst_t        inst,
                                        const mesa_macsec_port_t port,
                                        const mesa_bool_t        enable)
    CAP(PHY_MACSEC);

/** \brief Get the state config of the controlled (secure) port.
 *
 *
 * \param inst       [IN]     MESA-API instance.
 * \param port       [IN]     MACsec port.
 * \param enable     [OUT]    Forwarding state of the port.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_secy_controlled_get(const mesa_inst_t        inst,
                                        const mesa_macsec_port_t port,
                                        mesa_bool_t              *const enable)
    CAP(PHY_MACSEC);

/** \brief Get status from a SecY port, controlled, uncontrolled or common.
 *
 * \param inst        [IN]     MESA-API instance.
 * \param port        [IN]     MACsec port.
 * \param status      [OUT]    SecY port status
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid
 */
mesa_rc mesa_macsec_secy_port_status_get(const mesa_inst_t              inst,
                                         const mesa_macsec_port_t       port,
                                         mesa_macsec_secy_port_status_t *const status)
    CAP(PHY_MACSEC);


/** \brief Browse through available macsec ports (secy's) on a physical port
 *  Use NULL pointer to get the first port and use found ports as a search port in the next round.
 *
 * \param inst                [IN]     MESA-API instance.
 * \param port_no             [IN]     MESA-API port no.
 * \param search_macsec_port  [IN]     MACsec port to search for
 * \param found_macsec_port   [OUT]    MACsec port that comes next
 *
 * \return MESA_RC_OK when port is found; MESA_RC_ERROR if parameters are invalid or if the search port is not found
 */
mesa_rc mesa_macsec_port_get_next(const mesa_inst_t        inst,
                                  const mesa_port_no_t     port_no,
                                  const mesa_macsec_port_t *const search_macsec_port,
                                  mesa_macsec_port_t       *const found_macsec_port)
    CAP(PHY_MACSEC);

/*--------------------------------------------------------------------*/
/* Receive Secure Channel (SC) management                             */
/*--------------------------------------------------------------------*/

/** \brief Create an Rx SC object inside of the SecY.
 *
 * \param inst      [IN]     MESA-API instance.
 * \param port      [IN]     MACsec port.
 * \param sci       [IN]     The peer rx SCI.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid or resources exhausted.
 */

mesa_rc mesa_macsec_rx_sc_add(const mesa_inst_t        inst,
                              const mesa_macsec_port_t port,
                              const mesa_macsec_sci_t  *const sci)
    CAP(PHY_MACSEC);

/** \brief Instead of inheriting the configuration from the SecY the Rx SC can use its own configuration.  
 *
 * \param inst      [IN]     MESA-API instance.
 * \param port      [IN]     MACsec port.
 * \param sci       [IN]     The peer rx SCI.
 * \param conf      [IN]     Configuration which applies to this SC.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid or resources exhausted.
 *
 * Note: RxSC update with new parameters i.e. Replay Window size etc, it will
 *       update newly create SA's only. Existing parameters i.e. Next PN and Lower PN
 *       will not change. Rx SA Status Next PN and Lowest PN shows different
 *       as compare with existing Rx SA Status.
 *
 */

mesa_rc mesa_macsec_rx_sc_update(const mesa_inst_t              inst,
                                 const mesa_macsec_port_t       port,
                                 const mesa_macsec_sci_t        *const sci,
                                 const mesa_macsec_rx_sc_conf_t *const conf)
    CAP(PHY_MACSEC);

/** \brief Get the configuration of the SC.  
 *
 * \param inst      [IN]     MESA-API instance.
 * \param port      [IN]     MACsec port.
 * \param sci       [IN]     The peer rx SCI.
 * \param conf      [OUT]    Configuration which applies to this SC.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid or resources exhausted.
 */

mesa_rc mesa_macsec_rx_sc_get_conf(const mesa_inst_t        inst,
                                   const mesa_macsec_port_t port,
                                   const mesa_macsec_sci_t  *const sci,
                                   mesa_macsec_rx_sc_conf_t *const conf)
    CAP(PHY_MACSEC);


/** \brief Browse through the Rx SCs inside of the SecY.
 *
 * \param inst          [IN]     MESA-API instance.
 * \param port          [IN]     MACsec port.
 * \param search_sci    [IN]     SCI to start the search. The next SCI will be returned.
                                 NULL pointer finds the first SCI.
 * \param found_sci     [OUT]    The next SCI.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid or next SCI not found.
 */

mesa_rc mesa_macsec_rx_sc_get_next(const mesa_inst_t        inst,
                                   const mesa_macsec_port_t port,
                                   const mesa_macsec_sci_t  *const search_sci,
                                   mesa_macsec_sci_t        *const found_sci)
    CAP(PHY_MACSEC);

/** \brief Delete the Rx SC and the associated SAs
 *
 * \param inst       [IN]     MESA-API instance.
 * \param port       [IN]     MACsec port.
 * \param sci        [IN]     SCI to delete.

 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid or SCI not found.
 */
mesa_rc mesa_macsec_rx_sc_del(const mesa_inst_t        inst,
                              const mesa_macsec_port_t port,
                              const mesa_macsec_sci_t  *const sci)
    CAP(PHY_MACSEC);

/** \brief Rx SC status info
 *
 * \param inst         [IN]     MESA-API instance.
 * \param port         [IN]     MACsec port.
 * \param sci          [IN]     SCI of the peer.
 * \param status       [OUT]    SC status
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid
 */
mesa_rc mesa_macsec_rx_sc_status_get(const mesa_inst_t          inst,
                                     const mesa_macsec_port_t   port,
                                     const mesa_macsec_sci_t    *const sci,
                                     mesa_macsec_rx_sc_status_t *const status)
    CAP(PHY_MACSEC);


/*--------------------------------------------------------------------*/
/* Transmit Secure Channel (SC) management                            */
/*--------------------------------------------------------------------*/

/** \brief Create an Tx SC object inside of the SecY.  One TxSC is supported for each SecY.
 *
 * \param inst      [IN]     MESA-API instance.
 * \param port      [IN]     MACsec port.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid or resources exhausted.
 */
mesa_rc mesa_macsec_tx_sc_set(const mesa_inst_t        inst,
                              const mesa_macsec_port_t port)
    CAP(PHY_MACSEC);

/** \brief Instead of inheriting the configuration from the SecY the Tx SC can use its own configuration.  
 *
 * \param inst      [IN]     MESA-API instance.
 * \param port      [IN]     MACsec port.
 * \param conf      [IN]     Configuration which applies to this SC.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid or resources exhausted.
 *
 * Note: TxSC update with new parameters i.e. Replay Window size etc, it will
 *       update newly create SA's only. Existing parameters i.e. Next PN and Lower PN
 *       will not change. Tx SA Status Next PN and Lowest PN shows different
 *       as compare with existing Tx SA Status.
 *
 */
mesa_rc mesa_macsec_tx_sc_update(const mesa_inst_t              inst,
                                 const mesa_macsec_port_t       port,
                                 const mesa_macsec_tx_sc_conf_t *const conf)
    CAP(PHY_MACSEC);


/** \brief Get the SC configuration  
 *
 * \param inst      [IN]     MESA-API instance.
 * \param port      [IN]     MACsec port.
 * \param conf      [OUT]    Configuration which applies to this SC.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid or resources exhausted.
 */
mesa_rc mesa_macsec_tx_sc_get_conf(const mesa_inst_t        inst,
                                   const mesa_macsec_port_t port,
                                   mesa_macsec_tx_sc_conf_t *const conf)
    CAP(PHY_MACSEC);

/** \brief Delete the Tx SC object and the associated SAs
 *
 * \param inst       [IN]     MESA-API instance.
 * \param port       [IN]     MACsec port.

 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_tx_sc_del(const mesa_inst_t        inst,
                              const mesa_macsec_port_t port)
    CAP(PHY_MACSEC);


/** \brief Tx SC status
 *
 * \param inst         [IN]     MESA-API instance.
 * \param port         [IN]     MACsec port.
 * \param status       [OUT]    SC status
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_tx_sc_status_get(const mesa_inst_t          inst,
                                     const mesa_macsec_port_t   port,
                                     mesa_macsec_tx_sc_status_t *const status)
    CAP(PHY_MACSEC);


/*--------------------------------------------------------------------*/
/* Receive Secure Association (SA) management                         */
/*--------------------------------------------------------------------*/

/** \brief Create an Rx SA which is associated with an SC within the SecY.
 *         This SA is not enabled until mesa_macsec_rx_sa_activate() is performed.
 *
 * \param inst        [IN]     MESA-API instance.
 * \param port        [IN]     MACsec port.
 * \param sci         [IN]     SCI of the peer.
 * \param an          [IN]     Association number, 0-3
 * \param lowest_pn   [IN]     Lowest acceptable packet number.
 * \param sak         [IN]     The 128 or 256 bit Secure Association Key and the calculated hash value.

 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid or resources exhausted.
 */
mesa_rc mesa_macsec_rx_sa_set(const mesa_inst_t        inst,
                              const mesa_macsec_port_t port,
                              const mesa_macsec_sci_t  *const sci,
                              const uint16_t           an,
                              const uint32_t           lowest_pn,
                              const mesa_macsec_sak_t  *const sak)
    CAP(PHY_MACSEC);

/** \brief Get the Rx SA configuration of the active SA.

 *
 * \param inst       [IN]     MESA-API instance.
 * \param port       [IN]     MACsec port.
 * \param sci        [IN]     SCI of the peer.
 * \param an         [IN]     Association number, 0-3
 * \param lowest_pn  [OUT]    Lowest acceptable packet number.
 * \param sak        [OUT]    The 128 or 256 bit Secure Association Key and the hash value.
 * \param active     [OUT]    Flag that tells if this SA is activated or not.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 *
 * Note: If SA was created before any change on parameter like Replay Widow etc. Lowest PN may appear to be consistent with newly
 *       updated value, but the actual value will be according to the SA's creation time. One has to subtract the change in the
 *       the value obtained from API to get the actual value. Updating parameters like Replay Window doesn't change the older SA's.
 *
 */
mesa_rc mesa_macsec_rx_sa_get(const mesa_inst_t        inst,
                              const mesa_macsec_port_t port,
                              const mesa_macsec_sci_t  *const sci,
                              const uint16_t           an,
                              uint32_t                 *const lowest_pn,
                              mesa_macsec_sak_t        *const sak,
                              mesa_bool_t              *const active)
    CAP(PHY_MACSEC);


/** \brief Activate the SA associated with the AN.
           The reception switches from a previous SA to the SA identified by the AN.
           Note that the reception using the new SA does not necessarily begin immediately.
 *
 * \param inst   [IN]     MESA-API instance.
 * \param port   [IN]     MACsec port.
 * \param sci    [IN]     SCI of the peer.
 * \param an     [IN]     Association number, 0-3
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_rx_sa_activate(const mesa_inst_t        inst,
                                   const mesa_macsec_port_t port,
                                   const mesa_macsec_sci_t  *const sci,
                                   const uint16_t           an)
    CAP(PHY_MACSEC);

/** \brief This function disables Rx SA identified by an. Frames still in the pipeline are not discarded.

 *
 * \param inst            [IN]     MESA-API instance.
 * \param port            [IN]     MACsec port.
 * \param sci             [IN]     SCI of the peer.
 * \param an              [IN]     Association number, 0-3
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid, or if the AN has not previously been set.
 */
mesa_rc mesa_macsec_rx_sa_disable(const mesa_inst_t        inst,
                                  const mesa_macsec_port_t port,
                                  const mesa_macsec_sci_t  *const sci,
                                  const uint16_t           an)
    CAP(PHY_MACSEC);


/** \brief This function deletes Rx SA object identified by an. The Rx SA must be disabled before deleted.

 *
 * \param inst            [IN]     MESA-API instance.
 * \param port            [IN]     MACsec port.
 * \param sci             [IN]     SCI of the peer.
 * \param an              [IN]     Association number, 0-3
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid, or if the AN has not previously been set.
 */
mesa_rc mesa_macsec_rx_sa_del(const mesa_inst_t        inst,
                              const mesa_macsec_port_t port,
                              const mesa_macsec_sci_t  *const sci,
                              const uint16_t           an)
    CAP(PHY_MACSEC);


/** \brief Set (update) the packet number (pn) value to value in lowest_pn
 *
 * \param inst      [IN]     MESA-API instance.
 * \param port      [IN]     MACsec port.
 * \param sci       [IN]     SCI of the peer.
 * \param an        [IN]     Association number, 0-3
 * \param lowest_pn [IN]     Lowest accepted packet number
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_rx_sa_lowest_pn_update(const mesa_inst_t        inst,
                                           const mesa_macsec_port_t port,
                                           const mesa_macsec_sci_t  *const sci,
                                           const uint16_t           an,
                                           const uint32_t           lowest_pn)
    CAP(PHY_MACSEC);

/** \brief Rx SA status
 *
 * \param inst         [IN]     MESA-API instance.
 * \param port         [IN]     MACsec port.
 * \param sci          [IN]     SCI of the peer.
 * \param an           [IN]     Association number, 0-3
 * \param status       [OUT]    SC status
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 * Note: If SA was created before any change on parameter like Replay Widow etc. Lowest PN may appear to be consistent with newly
 *       updated value, but the actual value will be according to the SA's creation time. One has to subtract the change in the
 *       the value obtained from API to get the actual value. Updating parameters like Replay Window doesn't change the older SA's.
 */

mesa_rc mesa_macsec_rx_sa_status_get(const mesa_inst_t          inst,
                                     const mesa_macsec_port_t   port,
                                     const mesa_macsec_sci_t    *const sci,
                                     const uint16_t             an,
                                     mesa_macsec_rx_sa_status_t *const status)
    CAP(PHY_MACSEC);

/*--------------------------------------------------------------------*/
/* For XPN supported devices                                          */
/* Receive Secure Association (SA) management                         */
/*--------------------------------------------------------------------*/

/** \brief Create an Rx SA which is associated with an SC within the SecY.
 *
 * \param inst        [IN]     MESA-API instance.
 * \param port        [IN]     MACsec port.
 * \param sci         [IN]     SCI of the peer.
 * \param an          [IN]     Association number, 0-3
 * \param lowest_pn   [IN]     Lowest acceptable packet number in 32-bit or 64-bit size.
 * \param sak         [IN]     The 128 or 256 bit Secure Association Key and the calculated hash value.
 * \param ssci        [IN]     Short SCI associated with this peer. Used for XPN only.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid or resources exhausted.
 */
mesa_rc mesa_macsec_rx_seca_set(const mesa_inst_t           inst,
                                const mesa_macsec_port_t    port,
                                const mesa_macsec_sci_t     *const sci,
                                const uint16_t              an,
                                const mesa_macsec_pkt_num_t lowest_pn,
                                const mesa_macsec_sak_t     *const sak,
                                const mesa_macsec_ssci_t    *const ssci)
    CAP(PHY_MACSEC);

/** \brief Get the Rx SA configuration of the active SA.
 *
 * \param inst       [IN]     MESA-API instance.
 * \param port       [IN]     MACsec port.
 * \param sci        [IN]     SCI of the peer.
 * \param an         [IN]     Association number, 0-3
 * \param lowest_pn  [OUT]    Lowest acceptable packet number in 32-bit or 64-bit size.
 * \param sak        [OUT]    The 128 or 256 bit Secure Association Key and the hash value.
 * \param active     [OUT]    Flag that tells if this SA is activated or not.
 * \param ssci       [OUT]    Short SCI associated with this peer.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_rx_seca_get(const mesa_inst_t        inst,
                                const mesa_macsec_port_t port,
                                const mesa_macsec_sci_t  *const sci,
                                const uint16_t           an,
                                mesa_macsec_pkt_num_t    *const lowest_pn,
                                mesa_macsec_sak_t        *const sak,
                                mesa_bool_t              *const active,
                                mesa_macsec_ssci_t       *const ssci)
CAP(PHY_MACSEC);



/** \brief Update the lowest_pn packet number in 64-bit or 32-bit for Rx SA.
 *
 * \param inst          [IN]     MESA-API instance.
 * \param port          [IN]     MACsec port.
 * \param sci           [IN]     SCI of the peer.
 * \param an            [IN]     Association number, 0-3
 * \param lowest_pn     [IN]     Lowest accepted packet number of 32-bit or 64-bit size
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_rx_seca_lowest_pn_update(const mesa_inst_t           inst,
                                             const mesa_macsec_port_t    port,
                                             const mesa_macsec_sci_t     *const sci,
                                             const uint16_t              an,
                                             const mesa_macsec_pkt_num_t lowest_pn)
    CAP(PHY_MACSEC);

/*--------------------------------------------------------------------*/
/* Transmit Secure Association (SA) management                        */
/*--------------------------------------------------------------------*/

/** \brief Create an Tx SA which is associated with the Tx SC within the SecY.
 *         This SA is not in use until mesa_macsec_tx_sa_activate() is performed.
 *
 * \param inst            [IN]     MESA-API instance.
 * \param port            [IN]     MACsec port.
 * \param an              [IN]     Association number, 0-3
 * \param next_pn         [IN]     The packet number of the first packet sent using the new SA (1 or greater)
 * \param confidentiality [IN]     If true, packets are encrypted, otherwise integrity protected only.
 * \param sak             [IN]     The 128 or 256 bit Secure Association Key and the calculated hash value.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 *
 * Note: If SA was created before any change in parameters like Replay Widow etc. Lowest PN may appear to be consistent with newly
 *       updated value, but the actual value will be according to the SA's creation time. One has to subtract the change in the
 *       the value obtained from API to get the actual value. Updating parameters like Replay Window doesn't change the older SA's.
 *
 */
mesa_rc mesa_macsec_tx_sa_set(const mesa_inst_t        inst,
                              const mesa_macsec_port_t port,
                              const uint16_t           an,
                              const uint32_t           next_pn,
                              const mesa_bool_t        confidentiality,
                              const mesa_macsec_sak_t  *const sak)
    CAP(PHY_MACSEC);


/** \brief Get the  Tx SA configuration.
 *
 * \param inst            [IN]     MESA-API instance.
 * \param port            [IN]     MACsec port.
 * \param an              [IN]     Association number, 0-3
 * \param next_pn         [OUT]    The packet number of the first packet sent using the new SA (1 or greater)
 * \param confidentiality [OUT]    If true, packets are encrypted, otherwise integrity protected only.
 * \param sak             [OUT]    The 128 or 256 bit Secure Association Key and the hash value.
 * \param active          [OUT]    Flag that tells if this SA is activated or not.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid, or if the AN has not previously been set.
 */
mesa_rc mesa_macsec_tx_sa_get(const mesa_inst_t        inst,
                              const mesa_macsec_port_t port,
                              const uint16_t           an,
                              uint32_t                 *const next_pn,
                              mesa_bool_t              *const confidentiality,
                              mesa_macsec_sak_t        *const sak,
                              mesa_bool_t              *const active)
CAP(PHY_MACSEC);


/** \brief This function switches transmission from a previous Tx SA to the Tx SA identified by an.
           Transmission using the new SA is in effect immediately.
 *
 * \param inst            [IN]     MESA-API instance.
 * \param port            [IN]     MACsec port.
 * \param an              [IN]     Association number, 0-3
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid, or if the AN has not previously been set.
 */
mesa_rc mesa_macsec_tx_sa_activate(const mesa_inst_t        inst,
                                   const mesa_macsec_port_t port,
                                   const uint16_t           an)
    CAP(PHY_MACSEC);


/** \brief This function disables Tx SA identified by an. Frames still in the pipeline are not discarded.

 *
 * \param inst            [IN]     MESA-API instance.
 * \param port            [IN]     MACsec port.
 * \param an              [IN]     Association number, 0-3
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid, or if the AN has not previously been set.
 */
mesa_rc mesa_macsec_tx_sa_disable(const mesa_inst_t        inst,
                                  const mesa_macsec_port_t port,
                                  const uint16_t           an)
    CAP(PHY_MACSEC);


/** \brief This function deletes Tx SA object identified by an. The Tx SA must be disabled before deleted.

 *
 * \param inst            [IN]     MESA-API instance.
 * \param port            [IN]     MACsec port.
 * \param an              [IN]     Association number, 0-3
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid, or if the AN has not previously been set.
 */
mesa_rc mesa_macsec_tx_sa_del(const mesa_inst_t        inst,
                              const mesa_macsec_port_t port,
                              const uint16_t           an)
    CAP(PHY_MACSEC);

/** \brief Tx SA status
 *
 * \param inst         [IN]     MESA-API instance.
 * \param port         [IN]     MACsec port.
 * \param an           [IN]     Association number, 0-3
 * \param status       [OUT]    SC status
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 * Note: If SA was created before any change on parameter like Replay Widow etc. Lowest PN may appear to be consistent with newly
 *       updated value, but the actual value will be according to the SA's creation time. One has to subtract the change in the
 *       the value obtained from API to get the actual value. Updating parameters like Replay Window doesn't change the older SA's.
 */
mesa_rc mesa_macsec_tx_sa_status_get(const mesa_inst_t          inst,
                                     const mesa_macsec_port_t   port,
                                     const uint16_t             an,
                                     mesa_macsec_tx_sa_status_t *const status)
    CAP(PHY_MACSEC);

/*--------------------------------------------------------------------*/
/* For XPN supported devices                                          */
/* Transmit Secure Association (SA) management                        */
/*--------------------------------------------------------------------*/

/** \brief Create an Tx SA which is associated with the Tx SC within the SecY.
 *         This SA is not in use until mesa_macsec_tx_sa_activate() is performed.
 *
 * \param inst            [IN]     MESA-API instance.
 * \param port            [IN]     MACsec port.
 * \param an              [IN]     Association number, 0-3
 * \param next_pn         [IN]     The packet number of the first packet sent using the new SA (1 or greater).
 * \param confidentiality [IN]     If true, packets are encrypted, otherwise integrity protected only.
 * \param sak             [IN]     The 128 or 256 bit Secure Association Key and the calculated hash value.
 * \param ssci            [IN]     Short SCI associated with this peer. Used for XPN only.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_tx_seca_set(const mesa_inst_t           inst,
                                const mesa_macsec_port_t    port,
                                const uint16_t              an,
                                const mesa_macsec_pkt_num_t next_pn,
                                const mesa_bool_t           confidentiality,
                                const mesa_macsec_sak_t     *const sak,
                                const mesa_macsec_ssci_t    *const ssci)
    CAP(PHY_MACSEC);

/** \brief Get the Tx SA configuration supporting 64-bit and 32-bit PN.
 *
 * \param inst            [IN]     MESA-API instance.
 * \param port            [IN]     MACsec port.
 * \param an              [IN]     Association number, 0-3
 * \param next_pn         [OUT]    The packet number of the first packet sent using the new SA (1 or greater)
 * \param confidentiality [OUT]    If true, packets are encrypted, otherwise integrity protected only.
 * \param sak             [OUT]    The 128 or 256 bit Secure Association Key and the hash value.
 * \param active          [OUT]    Flag that tells if this SA is activated or not.
 * \param ssci            [OUT]     Short SCI associated with this peer. Used for XPN only. 
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid, or if the AN has not previously been set.
 */
mesa_rc mesa_macsec_tx_seca_get(const mesa_inst_t        inst,
                                const mesa_macsec_port_t port,
                                const uint16_t           an,
                                mesa_macsec_pkt_num_t    *const next_pn,
                                mesa_bool_t              *const confidentiality,
                                mesa_macsec_sak_t        *const sak,
                                mesa_bool_t              *const active,
                                mesa_macsec_ssci_t       *const ssci)
    CAP(PHY_MACSEC);


/*--------------------------------------------------------------------*/
/* SecY Counters                                                      */
/*--------------------------------------------------------------------*/
/** \brief Counter structure for common counters */
typedef struct {
    uint64_t if_in_octets;           /**< In octets       */
    uint64_t if_in_ucast_pkts;       /**< In unicasts     */
    uint64_t if_in_multicast_pkts;   /**< In multicasts   */
    uint64_t if_in_broadcast_pkts;   /**< In broadcasts   */
    uint64_t if_in_discards;         /**< In discards     */
    uint64_t if_in_errors;           /**< In errors       */
    uint64_t if_out_octets;          /**< Out octets      */
    uint64_t if_out_ucast_pkts;      /**< Out unicasts    */
    uint64_t if_out_multicast_pkts;  /**< Out multicasts  */
    uint64_t if_out_broadcast_pkts;  /**< Out broadcasts  */
    uint64_t if_out_errors;          /**< Out errors      */
} mesa_macsec_common_counters_t CAP(PHY_MACSEC);

/** \brief Counter structure for uncontrolled counters */
typedef struct {
    uint64_t if_in_octets;           /**< In octets       */
    uint64_t if_in_ucast_pkts;       /**< In unicasts     */
    uint64_t if_in_multicast_pkts;   /**< In multicasts   */
    uint64_t if_in_broadcast_pkts;   /**< In broadcasts   */
    uint64_t if_in_discards;         /**< In discards     */
    uint64_t if_in_errors;           /**< In errors       */
    uint64_t if_out_octets;          /**< Out octets      */
    uint64_t if_out_ucast_pkts;      /**< Out unicasts    */
    uint64_t if_out_multicast_pkts;  /**< Out multicasts  */
    uint64_t if_out_broadcast_pkts;  /**< Out broadcasts  */
    uint64_t if_out_errors;          /**< Out errors      */
} mesa_macsec_uncontrolled_counters_t CAP(PHY_MACSEC);

/** \brief Counter structure for SecY ports */
typedef struct {
    uint64_t if_in_octets;           /**< In octets       */
    uint64_t if_in_pkts;             /**< Out octets      */
    uint64_t if_in_ucast_pkts;       /**< In unicasts   - available from Rev B */
    uint64_t if_in_multicast_pkts;   /**< In multicasts - available from Rev B */
    uint64_t if_in_broadcast_pkts;   /**< In broadcasts - available from Rev B */
    uint64_t if_in_discards;         /**< In discards     */
    uint64_t if_in_errors;           /**< In errors       */
    uint64_t if_out_octets;          /**< Out octets      */
    uint64_t if_out_pkts;            /**< Out packets     */
    uint64_t if_out_errors;          /**< Out errors      */
    uint64_t if_out_ucast_pkts;      /**< Out unicasts   - available from Rev B */
    uint64_t if_out_multicast_pkts;  /**< Out multicasts - available from Rev B */
    uint64_t if_out_broadcast_pkts;  /**< Out broadcasts - available from Rev B */
} mesa_macsec_secy_port_counters_t CAP(PHY_MACSEC);

/** \brief SecY counters as defined by 802.1AE */
typedef struct {
    uint64_t in_pkts_untagged;    /**< Received packets without the secTAG when secyValidateFrames is not in strict mode. NOTE: Theses packets will be counted in the uncontrolled if_in_pkts and not in the controlled if_in_pkts  */
    uint64_t in_pkts_no_tag;      /**< Received packets discarded without the secTAG when secyValidateFrames is in strict mode. */
    uint64_t in_pkts_bad_tag;     /**< Received packets discarded with an invalid secTAG or zero value PN or an invalid PN. */
    uint64_t in_pkts_unknown_sci; /**< Received packets with unknown SCI when secyValidateFrames is not in strict mode
                                  and the C bit in the SecTAG is not set. */
    uint64_t in_pkts_no_sci;      /**< Received packets discarded with unknown SCI when  secyValidateFrames is in strict mode
                                  or the C bit in the SecTAG is set. */
    uint64_t in_pkts_overrun;     /**< Received packets discarded because the number of receive packets exceeded the
                                  cryptographic performace capabilities. */
    uint64_t in_octets_validated; /**< Received octets validated */
    uint64_t in_octets_decrypted; /**< Received octets decrypted */
    uint64_t out_pkts_untagged;   /**< Number of packets transmitted without the MAC security TAG. NOTE: Theses packets will be counted in the uncontrolled if_out_pkts and not in the controlled if_out_pkts for Rev A of macsec. From Rev B onwards, they will be counted under controlled port for successful SA lookup */
    uint64_t out_pkts_too_long;   /**< Number of transmitted packets discarded because the packet length is larger than the interface MTU. */
    uint64_t out_octets_protected;/**< The number of octets integrity protected but not encrypted. */
    uint64_t out_octets_encrypted;/**< The number of octets integrity protected and encrypted. */
} mesa_macsec_secy_counters_t CAP(PHY_MACSEC);

/* Possible values for the mesa_macsec_secy_cap_t:ciphersuite_cap */
#define MESA_MACSEC_CAP_GCM_AES_128       0x0001 /**< GCM-AES-128 cipher suite capability */ 
#define MESA_MACSEC_CAP_GCM_AES_256       0x0002 /**< GCM-AES-256 cipher suite capability */ 
#define MESA_MACSEC_CAP_GCM_AES_XPN_128   0x0004 /**< GCM-AES-XPN-256 cipher suite capability (extended PN) */
#define MESA_MACSEC_CAP_GCM_AES_XPN_256   0x0008 /**< GCM-AES-XPN-256 cipher suite capability (extended PN) */

/** \brief Capabilities as defined by 802.1AE */
typedef struct {
    uint16_t max_peer_scs;           /**< Max peer SCs (802.1AE) */
    uint16_t max_receive_keys;       /**< Max Rx keys  (802.1AE) */
    uint16_t max_transmit_keys;      /**< Max Tx keys  (802.1AE) */
    uint32_t ciphersuite_cap;        /**< The cipher suite capability offered by the API and chip */
} mesa_macsec_secy_cap_t CAP(PHY_MACSEC);

/** \brief Get counters from a SecY controlled (802-1AE) port.
 *
 * \param inst            [IN]     MESA-API instance.
 * \param port            [IN]     MACsec port.
 * \param counters        [OUT]    Port counters
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid
 */

mesa_rc mesa_macsec_controlled_counters_get(const mesa_inst_t                inst,
                                            const mesa_macsec_port_t         port,
                                            mesa_macsec_secy_port_counters_t *const counters)
    CAP(PHY_MACSEC);

/** \brief Get counters from a physical uncontrolled (802-1AE) port.
 *
 * \param inst            [IN]     MESA-API instance.
 * \param port_no         [IN]     MESA-API port no.
 * \param counters        [OUT]    Global uncontrolled port counters
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid
 */
mesa_rc mesa_macsec_uncontrolled_counters_get(const mesa_inst_t                   inst,
                                              const mesa_port_no_t                port_no,
                                              mesa_macsec_uncontrolled_counters_t *const counters)
    CAP(PHY_MACSEC);

/** \brief Get counters from a physical common (802-1AE) port.
 *
 * \param inst            [IN]     MESA-API instance.
 * \param port_no         [IN]     MESA-API port no.
 * \param counters        [OUT]    Global common port counters
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid
 */
mesa_rc mesa_macsec_common_counters_get(const mesa_inst_t             inst,
                                        const mesa_port_no_t          port_no,
                                        mesa_macsec_common_counters_t *const counters)
    CAP(PHY_MACSEC);


/** \brief Get the capabilities of the SecY as define by 802.1AE.
 *
 * \param inst        [IN]     MESA-API instance.
 * \param port_no     [IN]     MESA-API port no.
 * \param cap         [OUT]    SecY capabilities
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid
 */
mesa_rc mesa_macsec_secy_cap_get(const mesa_inst_t      inst,
                                 const mesa_port_no_t   port_no,
                                 mesa_macsec_secy_cap_t *const cap)
    CAP(PHY_MACSEC);


/** \brief SecY counters
 *
 * \param inst            [IN]     MESA-API instance.
 * \param port            [IN]     MESA-API port no.
 * \param counters        [OUT]    SecY counters 
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid
 */
mesa_rc mesa_macsec_secy_counters_get(const mesa_inst_t           inst,
                                      const mesa_macsec_port_t    port,
                                      mesa_macsec_secy_counters_t *const counters)
    CAP(PHY_MACSEC);


/** \brief MacSec counter update.  Keep the API internal SW counters updated. 
 *  Should be called periodically, but no special requirement to interval (the chip counters are 40bit).
 *
 * \param inst            [IN]     MESA-API instance.
 * \param port_no         [IN]     MESA-API port no.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid
 */
mesa_rc mesa_macsec_counters_update(const mesa_inst_t    inst,
                                    const mesa_port_no_t port_no)
    CAP(PHY_MACSEC);


/** \brief MacSec counter clear.  Clear all counters.
 *
 * \param inst            [IN]     MESA-API instance.
 * \param port_no         [IN]     MESA-API port no.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid
 */
mesa_rc mesa_macsec_counters_clear(const mesa_inst_t    inst,
                                   const mesa_port_no_t port_no)
    CAP(PHY_MACSEC);

/*--------------------------------------------------------------------*/
/* SC Counters                                                        */
/*--------------------------------------------------------------------*/
/** \brief SC Counters as defined by 802.1AE. */
typedef struct {
    // Bugzilla#12752 
    uint64_t in_pkts_unchecked;    /**< Unchecked packets (802.1AE) - Due to a chip limitation InOctetsValidated/Decrypted is not incremented. The API will not correctly count "if_in_octets" since this counter is indirectly derived from InOctetsValidated/Decrypted which per standard. Hence if_in_octets calculation of controlled port is incorrect and only the DMAC and SMAC octets are counted.*/

    uint64_t in_pkts_delayed;        /**< Delayed packets (802.1AE) */
    uint64_t in_pkts_late;           /**< Late packets (802.1AE) */
    uint64_t in_pkts_ok;             /**< Ok packets (802.1AE) */
    uint64_t in_pkts_invalid;        /**< Invalid packets (802.1AE) */
    uint64_t in_pkts_not_valid;      /**< No valid packets (802.1AE) */
    uint64_t in_pkts_not_using_sa;   /**< Packets not using SA (802.1AE) */
    uint64_t in_pkts_unused_sa;      /**< Unused SA (802.1AE) */
    uint64_t in_octets_validated;    /**< Received octets validated */
    uint64_t in_octets_decrypted;    /**< Received octets decrypted */
} mesa_macsec_rx_sc_counters_t CAP(PHY_MACSEC);


/** \brief Tx SC counters as defined by 802.1AE */
typedef struct {
    uint64_t out_pkts_protected; /**< Protected but not encrypted (802.1AE) */
    uint64_t out_pkts_encrypted; /**< Both protected and encrypted (802.1AE) */
    uint64_t out_octets_protected;/**< The number of octets integrity protected but not encrypted. */
    uint64_t out_octets_encrypted;/**< The number of octets integrity protected and encrypted. */
} mesa_macsec_tx_sc_counters_t CAP(PHY_MACSEC);



/** \brief RX SC counters
 *
 * \param inst         [IN]     MESA-API instance.
 * \param port         [IN]     MacSec port.
 * \param sci          [IN]     SCI of the peer.
 * \param counters     [OUT]    SecY counters
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid
 */
mesa_rc mesa_macsec_rx_sc_counters_get(const mesa_inst_t            inst,
                                       const mesa_macsec_port_t     port,
                                       const mesa_macsec_sci_t      *const sci,
                                       mesa_macsec_rx_sc_counters_t *const counters)
    CAP(PHY_MACSEC);

/** \brief Rx SC counters
 *
 * \param inst         [IN]     MESA-API instance.
 * \param port         [IN]     MacSec port.
 * \param counters     [OUT]    SC counters
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_tx_sc_counters_get(const mesa_inst_t            inst,
                                       const mesa_macsec_port_t     port,
                                       mesa_macsec_tx_sc_counters_t *const counters)
    CAP(PHY_MACSEC);


/*--------------------------------------------------------------------*/
/* SA Counters                                                        */
/*--------------------------------------------------------------------*/
/** \brief Tx SA counters as defined by 802.1AE */
typedef struct {
  uint64_t out_pkts_protected; /**< Protected but not encrypted (802.1AE) */
  uint64_t out_pkts_encrypted; /**< Both protected and encrypted (802.1AE) */
} mesa_macsec_tx_sa_counters_t CAP(PHY_MACSEC);


/** \brief Rx SA counters as defined by 802.1AE */
typedef struct {
    uint64_t in_pkts_ok;           /**< Ok packets  (802.1AE) */
    uint64_t in_pkts_invalid;      /**< Invalid packets (802.1AE) */
    uint64_t in_pkts_not_valid;    /**< Not valid packets (802.1AE) */
    uint64_t in_pkts_not_using_sa; /**< Not using SA (802.1AE) */
    uint64_t in_pkts_unused_sa;    /**< Unused SA (802.1AE) */

    // Bugzilla#12752 
    uint64_t in_pkts_unchecked;    /**< Unchecked packets (802.1AE) - Due to a chip limitation InOctetsValidated/Decrypted is not incremented. The API will not correctly count "if_in_octets" since this counter is indirectly derived from InOctetsValidated/Decrypted which per standard. Hence if_in_octets calculation of controlled port is incorrect and only the DMAC and SMAC octets are counted.*/

    uint64_t in_pkts_delayed;      /**< Delayed packets (802.1AE) */
    uint64_t in_pkts_late;         /**< Late packets (802.1AE) */
} mesa_macsec_rx_sa_counters_t CAP(PHY_MACSEC);


/** \brief Tx SA counters
 *
 * \param inst         [IN]     MESA-API instance.
 * \param port         [IN]     MacSec port.
 * \param an           [IN]     Association number, 0-3
 * \param counters     [OUT]    SA counters
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_tx_sa_counters_get(const mesa_inst_t            inst,
                                       const mesa_macsec_port_t     port,
                                       const uint16_t               an,
                                       mesa_macsec_tx_sa_counters_t *const counters)
    CAP(PHY_MACSEC);


/** \brief Tx SA counters
 *
 * \param inst         [IN]     MESA-API instance.
 * \param port         [IN]     MacSec port.
 * \param sci          [IN]     SCI of the peer.
 * \param an           [IN]     Association number, 0-3
 * \param counters     [OUT]    SA counters
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_rx_sa_counters_get(const mesa_inst_t            inst,
                                       const mesa_macsec_port_t     port,
                                       const mesa_macsec_sci_t      *const sci,
                                       const uint16_t               an,
                                       mesa_macsec_rx_sa_counters_t *const counters)
    CAP(PHY_MACSEC);

/*--------------------------------------------------------------------*/
/* VP / Uncontrolled classification                                   */
/*--------------------------------------------------------------------*/
#define MESA_MACSEC_MATCH_DISABLE        0x0001 /**< Disable match  */
#define MESA_MACSEC_MATCH_DMAC           0x0002 /**< DMAC match  */
#define MESA_MACSEC_MATCH_ETYPE          0x0004 /**< ETYPE match */
#define MESA_MACSEC_MATCH_VLAN_ID        0x0008 /**< VLAN match  */
#define MESA_MACSEC_MATCH_VLAN_ID_INNER  0x0010 /**< Inner VLAN match */
#define MESA_MACSEC_MATCH_BYPASS_HDR     0x0020 /**< MPLS header match */
#define MESA_MACSEC_MATCH_IS_CONTROL     0x0040 /**< Control frame match e.g. Ethertype 0x888E */
#define MESA_MACSEC_MATCH_HAS_VLAN       0x0080 /**< The frame contains a VLAN tag */
#define MESA_MACSEC_MATCH_HAS_VLAN_INNER 0x0100 /**< The frame contains an inner VLAN tag */
#define MESA_MACSEC_MATCH_SMAC           0x0200 /**< Source MAC address  */

#define MESA_MACSEC_MATCH_PRIORITY_LOWEST 15 /**< Lowest possible matching priority */
#define MESA_MACSEC_MATCH_PRIORITY_LOW    12 /**< Low matching priority */
#define MESA_MACSEC_MATCH_PRIORITY_MID     8 /**< Medium matching priority */
#define MESA_MACSEC_MATCH_PRIORITY_HIGH    4 /**< High matching priority */
#define MESA_MACSEC_MATCH_PRIORITY_HIGHEST 0 /**< Hihhest possible matching priority */

/** \brief MACsec control frame matching */
typedef struct {
    uint32_t            match;         /**< Use combination of (OR): MESA_MACSEC_MATCH_DMAC,
                                       MESA_MACSEC_MATCH_ETYPE */
    mesa_mac_t     dmac;          /**< DMAC address to match (SMAC not supported) */
    mesa_etype_t   etype;         /**< Ethernet type to match  */
} mesa_macsec_control_frame_match_conf_t CAP(PHY_MACSEC);

/** \brief Set the control frame matching rules.
 *  16 rules are supported for ETYPE (8 for 1G Phy).
 *   8 rules are supported for DMACs 
 *   2 rules are supported for ETYPE & DMAC
 * \param inst    [IN]     MESA-API instance.
 * \param port_no [IN]     MESA-API port no.
 * \param conf    [IN]     Matching configuration
 * \param rule_id [OUT]    Rule id for getting and deleting.  Can be ignored by passing NULL as a parameter.
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_control_frame_match_conf_set(const mesa_inst_t                            inst,
                                                 const mesa_port_no_t                         port_no,
                                                 const mesa_macsec_control_frame_match_conf_t *const conf,
                                                 uint32_t                                     *const rule_id)
    CAP(PHY_MACSEC);

/** \brief Delete a control frame matching rule.
 * \param inst    [IN]     MESA-API instance.
 * \param port_no [IN]     MESA-API port no.
 * \param rule_id [IN]     The rule id (retrieved from mesa_macsec_control_frame_match_conf_set()).
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_control_frame_match_conf_del(const mesa_inst_t    inst,
                                                 const mesa_port_no_t port_no,
                                                 const uint32_t       rule_id)
    CAP(PHY_MACSEC);

/**
 * \brief Get the control frame matching rules.

 * \param inst    [IN]     MESA-API instance.
 * \param port_no [IN]     MESA-API port no.
 * \param conf    [OUT]    Matching configuration
 * \param rule_id [IN]     The rule id (retrieved from mesa_macsec_control_frame_match_conf_set()).

 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_control_frame_match_conf_get(const mesa_inst_t                      inst,
                                                 const mesa_port_no_t                   port_no,
                                                 mesa_macsec_control_frame_match_conf_t *const conf,
                                                 uint32_t                               rule_id)
    CAP(PHY_MACSEC);

/** \brief Matching patterns,
 * When traffic is passed through the MACsec processing block, it will be match
 * against a set of rules. If non of the rules matches, it will be matched
 * against the default rules (one and only on of the default rules will always
 * match) defined in mesa_macsec_default_action_policy_t.
 *
 * The classification rules are associated with a MACsec port and an action. The
 * action is defined in mesa_macsec_match_action_t and defines if frames
 * should be dropped, forwarded to the controlled or the un-controlled port of
 * the given virtual MACsec port.
 *
 * These classification rules are used both on the ingress and the egress side.
 * On the ingress side, only tags located before the SECtag will be used.
 *
 * These rules are a limited resource, and the HW is limited to allow the same
 * amount of classification rules as concurrent SA streams. Therefore to utilize
 * the hardware 100%, they should only be used to associate traffic with the
 * controlled port of a MACsec port. In simple scenarios where a single peer is
 * connected to a single PHY, there are more then sufficiet resources to use
 * this mechanism for associate traffic with the uncontrolled port.
 *
 * Instead of using this to forward control frames to the uncontrolled port,
 * the default rules may be used to bypass those frames. This will however have
 * the following consequences:
 *  - The controlled frames will not be included in uncontrolled port
 *    counters. To get the correct counter values, the application will need to
 *    gather all the control frames, calculate the statistics and use this to
 *    compensate the uncontrolled port counters.
 *  - All frames which are classified as control frames are passed through. If
 *    the control frame matches against the ether-type, it will
 *    evaluate to true in the following three cases:
 *     * If the ether-type located directly after the source MAC address matches
 *     * If the ether-type located the first VLAN tag matches
 *     * If the ether-type located a double VLAN tag matches
 * */
typedef struct {
    /** This field is used to specify which part of the matching pattern is
     * active. If multiple fields are active, they must all match if the
     * pattern is to match.  */
    uint32_t          match;

    /** Signals if the frame has been classified as a control frame. This allow
     * to match if a frame must be classified as a control frame, or if it has
     * not be classified as a control frame. The classification is controlled
     * by the mesa_macsec_control_frame_match_conf_t struct. This field is
     * activated by setting the MESA_MACSEC_MATCH_IS_CONTROL in "match" */
    mesa_bool_t         is_control;

    /** Signals if the frame contains a VLAN tag. This allows to match if a VLAN
     * tag must exists, and if a VLAN tag must not exists. This field is
     * activated by setting the MESA_MACSEC_MATCH_HAS_VLAN bit in "match" */
    mesa_bool_t         has_vlan_tag;

    /** Signals if the frame contains an inner VLAN tag. This allows to match if
     * an inner VLAN tag must exists, and if an inner VLAN tag must not exists.
     * This field is activated by setting the MESA_MACSEC_MATCH_HAS_VLAN_INNER
     * bit in "match" */
    mesa_bool_t         has_vlan_inner_tag;

    /** This field can be used to match against a parsed ether-type. This
     * field is activated by setting the MESA_MACSEC_MATCH_ETYPE bit in "match"
     */
    mesa_etype_t etype;

    /** This field can be used to match against the VLAN id. This field is
     * activated by setting the MESA_MACSEC_MATCH_VLAN_ID bit in "match" */
    mesa_vid_t   vid;

    /** This field can be used to match against the inner VLAN id. This field
     * is activated by setting the MESA_MACSEC_MATCH_VLAN_ID_INNER bit in
     * "match" */
    mesa_vid_t   vid_inner;

    /** This field along with hdr_mask is used to do a binary matching of a MPLS
     * header. This is activated by setting the MESA_MACSEC_MATCH_BYPASS_HDR bit
     * in "match" */
    uint8_t           hdr[8];

    /** Full mask set for the 'hdr' field. */
    uint8_t           hdr_mask[8];

    /** In case multiple rules matches a given frame, the rule with the highest
     * priority wins. Valid range is 0 (highest) - 15 (lowest).*/
    uint8_t           priority;

    /** This field can be used to match against the Source MAC address.  This field is
     * activated by setting the MESA_MACSEC_MATCH_SMAC bit in "match" */
    mesa_mac_t   src_mac;

    /** This field can be used to match against the Destination MAC address.  This field is
     * activated by setting the MESA_MACSEC_MATCH_DMAC bit in "match" */
    mesa_mac_t   dest_mac;
} mesa_macsec_match_pattern_t CAP(PHY_MACSEC);

/** \brief Pattern matching actions */
typedef enum {
    /** Drop the packet */
    MESA_MACSEC_MATCH_ACTION_DROP=0,

   /** Forward the packet to the controlled port */
    MESA_MACSEC_MATCH_ACTION_CONTROLLED_PORT=1,

    /** Forward the packet to the uncontrolled port */
    MESA_MACSEC_MATCH_ACTION_UNCONTROLLED_PORT=2,

    /** Number of actions - always add new actions above this line */
    MESA_MACSEC_MATCH_ACTION_CNT = 3,
} mesa_macsec_match_action_t CAP(PHY_MACSEC);


/** \brief Type used to state direction  */
typedef enum {
    /** Ingress. Traffic which is received by the port. */
    MESA_MACSEC_DIRECTION_INGRESS=0,

    /** Egress. Traffic which is transmitted on the port. */
    MESA_MACSEC_DIRECTION_EGRESS=1,

    /** Number of directions - will always be 2 */
    MESA_MACSEC_DIRECTION_CNT = 2,
} mesa_macsec_direction_t CAP(PHY_MACSEC);


/** \brief Configure the Matching pattern for a given MACsec port, for a given
 * action. Only one action may be associated with each actions. One matching
 * slot will be acquired immediately when this is called for the "DROP" or the
 * "UNCONTROLLED_PORT" actions. When matching pattern is configured for the
 * "CONTROLLED_PORT" action, HW a matching resource will be acquired for every
 * SA added.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port      [IN] MACsec port.
 * \param direction [IN] Direction
 * \param action    [IN] Action
 * \param pattern   [IN] Pattern
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid,
 * or if we are out of resources.
 */
mesa_rc mesa_macsec_pattern_set(const mesa_inst_t                 inst,
                                const mesa_macsec_port_t          port,
                                const mesa_macsec_direction_t     direction,
                                const mesa_macsec_match_action_t  action,
                                const mesa_macsec_match_pattern_t *const pattern)
    CAP(PHY_MACSEC);

/** \brief   Delete a pattern matching rule.
 *
 * \param inst      [IN]     MESA-API instance.
 * \param port      [IN] MACsec port.
 * \param direction [IN] Direction
 * \param action    [IN] Action
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_pattern_del(const mesa_inst_t                inst,
                                const mesa_macsec_port_t         port,
                                const mesa_macsec_direction_t    direction,
                                const mesa_macsec_match_action_t action)
    CAP(PHY_MACSEC);

/** \brief   Get the pattern matching rule.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port      [IN] MACsec port.
 * \param direction [IN] Direction
 * \param action    [IN] Action.
 * \param pattern   [OUT] Pattern.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_pattern_get(const mesa_inst_t                inst,
                                const mesa_macsec_port_t         port,
                                const mesa_macsec_direction_t    direction,
                                const mesa_macsec_match_action_t action,
                                mesa_macsec_match_pattern_t      *const pattern)
    CAP(PHY_MACSEC);

/** \brief Default matching actions */
typedef enum {
    MESA_MACSEC_DEFAULT_ACTION_DROP   = 0,  /**< Drop frame */
    MESA_MACSEC_DEFAULT_ACTION_BYPASS = 1,  /**< Bypass frame */
} mesa_macsec_default_action_t CAP(PHY_MACSEC);

/** \brief Default policy.
 * Frames not matched by any of the MACsec patterns will be evaluated against
 * the default policy.
 */
typedef struct {
    /**  Defines action for ingress frames which are not classified as MACsec
     *   frames and not classified as control frames. */
    mesa_macsec_default_action_t ingress_non_control_and_non_macsec;

    /**  Defines action for ingress frames which are not classified as MACsec
     *   frames and are classified as control frames. */
    mesa_macsec_default_action_t ingress_control_and_non_macsec;

    /**  Defines action for ingress frames which are classified as MACsec frames
     *   and are not classified as control frames. */
    mesa_macsec_default_action_t ingress_non_control_and_macsec;

    /**  Defines action for ingress frames which are classified as MACsec frames
     *   and are classified as control frames. */
    mesa_macsec_default_action_t ingress_control_and_macsec;

    /**  Defines action for egress frames which are classified as control frames. */
    mesa_macsec_default_action_t egress_control;

    /**  Defines action for egress frames which are not classified as control frames. */
    mesa_macsec_default_action_t egress_non_control;
} mesa_macsec_default_action_policy_t CAP(PHY_MACSEC);

/**
 * \brief   Assign default policy
 *
 * \param inst    [IN]  MESA-API instance.
 * \param port_no [IN]  MESA-API port no.
 * \param policy  [IN]  Policy
 *
 * \returns MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_default_action_set(const mesa_inst_t                         inst,
                                       const mesa_port_no_t                      port_no,
                                       const mesa_macsec_default_action_policy_t *const policy)
    CAP(PHY_MACSEC);

/**
 * \brief   Get default policy
 *
 * \param inst    [IN]  MESA-API instance.
 * \param port_no [IN]  MESA-API port no.
 * \param policy  [OUT] Policy
 *
 * \returns MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_default_action_get(const mesa_inst_t                   inst,
                                       const mesa_port_no_t                port_no,
                                       mesa_macsec_default_action_policy_t *const policy)
    CAP(PHY_MACSEC);


/*--------------------------------------------------------------------*/
/* Header / TAG Bypass                                                */
/*--------------------------------------------------------------------*/

/** \brief  Enum for Bypass mode, Tag or Header  */
typedef enum {
    MESA_MACSEC_BYPASS_NONE,   /**< Disable bypass mode  */
    MESA_MACSEC_BYPASS_TAG,    /**< Enable TAG bypass mode  */
    MESA_MACSEC_BYPASS_HDR,    /**< Enable Header bypass mode */
} mesa_macsec_bypass_t CAP(PHY_MACSEC);

/** \brief Structure for Bypass mode */
typedef struct {
    mesa_macsec_bypass_t  mode;            /**< Bypass Mode, Tag bypass or Header bypass */
    uint32_t                   hdr_bypass_len;  /**< (ignored for TAG bypass) Header Bypass length, possible values: 2,4,6..16 bytes.  
                                           * The bypass includes MPLS DA + MPLS SA + MPLS Etype (before frame DA/SA)
                                           * E.g. the value '4' means 6+6+2+4=18 bytes (MPLS dmac + MPLS smac + MPLS etype + 4) */ 
    mesa_etype_t          hdr_etype;       /**< (ignored for TAG bypass) Header Bypass: Etype to match (at frame index 12)   
                                           * When matched, process control packets using DMAC/SMAC/Etype after the header 
                                           * If not matched process control packets using the first DMAC/SMAC/Etype (as normally done) */
} mesa_macsec_bypass_mode_t CAP(PHY_MACSEC);

/** \brief Enum for number of TAGs  */
typedef enum {
    MESA_MACSEC_BYPASS_TAG_ZERO, /**< Disable */
    MESA_MACSEC_BYPASS_TAG_ONE,  /**< Bypass 1 tag */
    MESA_MACSEC_BYPASS_TAG_TWO,  /**< Bypass 2 tags */
} mesa_macsec_tag_bypass_t CAP(PHY_MACSEC);


/** \brief Set header bypass mode globally for the port
 *
 * \param inst         [IN]     MESA-API instance.
 * \param port_no      [IN]     MESA-API port no
 * \param bypass       [IN]     The bypass mode, TAG or Header.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_bypass_mode_set(const mesa_inst_t               inst,
                                    const mesa_port_no_t            port_no,
                                    const mesa_macsec_bypass_mode_t *const bypass)
    CAP(PHY_MACSEC);

/** \brief Get the header bypass mode
 *
 * \param inst         [IN]    MESA-API instance.
 * \param port_no      [IN]    MESA-API port no
 * \param bypass       [OUT]   The bypass mode, TAG or Header.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_bypass_mode_get(const mesa_inst_t         inst,
                                    const mesa_port_no_t      port_no,
                                    mesa_macsec_bypass_mode_t *const bypass)
    CAP(PHY_MACSEC);


/** \brief Set the bypass tag mode i.e. number of Tags to bypass: 0(disable), 1 or 2 tags.
 *
 * \param inst         [IN]    MESA-API instance.
 * \param port         [IN]    MacSec port
 * \param tag          [IN]    Number (enum) of TAGS to bypass.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_bypass_tag_set(const mesa_inst_t              inst,
                                   const mesa_macsec_port_t       port,
                                   const mesa_macsec_tag_bypass_t tag)
    CAP(PHY_MACSEC);

/** \brief Get the bypass Tag mode i.e. 0, 1 or 2 tags.
 *
 * \param inst         [IN]    MESA-API instance.
 * \param port         [IN]    MacSec port
 * \param tag          [OUT]   Number (enum) of TAGS to bypass.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_bypass_tag_get(const mesa_inst_t        inst,
                                   const mesa_macsec_port_t port,
                                   mesa_macsec_tag_bypass_t *const tag)
    CAP(PHY_MACSEC);



/*--------------------------------------------------------------------*/
/* Others                                                             */
/*--------------------------------------------------------------------*/

#define MESA_MACSEC_FRAME_CAPTURE_SIZE_MAX 504 /**< The maximum frame size supported for MACSEC capturing */

/** \brief Enum for frame capturing  */
typedef enum {
    MESA_MACSEC_FRAME_CAPTURE_DISABLE, /**< Disable frame capturing */
    MESA_MACSEC_FRAME_CAPTURE_INGRESS, /**< Enable ingress frame capturing */
    MESA_MACSEC_FRAME_CAPTURE_EGRESS,  /**< Enable egress frame capturing */
} mesa_macsec_frame_capture_t CAP(PHY_MACSEC);

/** \brief Sets MTU for both ingress and egress.
 * \param inst         [IN]    MESA-API instance.
 * \param port_no      [IN]    MESA-API port no
 * \param mtu_conf     [IN]    New MTU configuration
 *     
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_mtu_set(const mesa_inst_t       inst,
                            const mesa_port_no_t    port_no,
                            const mesa_macsec_mtu_t *const mtu_conf)
    CAP(PHY_MACSEC);

/** \brief Gets current MTU configuration
 * \param inst         [IN]    MESA-API instance.
 * \param port_no      [IN]    MESA-API port no
 * \param mtu_conf     [IN]    current MTU configuration
 *     
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_mtu_get(const mesa_inst_t    inst,
                            const mesa_port_no_t port_no,
                            mesa_macsec_mtu_t    *mtu_conf)
    CAP(PHY_MACSEC);

/** \brief Enable frame capture.  Used for test/debugging.
 *   The buffer will only capture the first frame received after capturing has been started 
 *   The procedure for frame capturing is as follow:
 *   1) Start capturing (Call mesa_macsec_frame_capture_set with MESA_MACSEC_FRAME_CAPTURE_INGRESS/MESA_MACSEC_FRAME_CAPTURE_EGRESS)
 *   2) Send in the frame to be captured
 *   3) Disable capturing (Call mesa_macsec_frame_capture_set with MESA_MACSEC_FRAME_CAPTURE_DISABLE) in order to prepare for next capturing.
 *   4) Get the captured frame using mesa_macsec_frame_get.
 *
 * \param inst         [IN]    MESA-API instance.
 * \param port_no      [IN]    MESA-API port no
 * \param capture      [IN]    Selects  Ingress/Egress frame capture or disable capture.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_frame_capture_set(const mesa_inst_t                 inst,
                                      const mesa_port_no_t              port_no,
                                      const mesa_macsec_frame_capture_t capture)
    CAP(PHY_MACSEC);


/** \brief Get a frame from an internal capture buffer. Used for test/debugging.
 *
 * \param inst          [IN]    MESA-API instance.
 * \param port_no       [IN]    MESA-API port no
 * \param buf_length    [IN]    Length of frame buffer.
 * \param return_length [OUT]   Returned length of the frame
 * \param frame         [OUT]   Frame buffer.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid, or if no frame is received.
 */
mesa_rc mesa_macsec_frame_get(const mesa_inst_t    inst,
                              const mesa_port_no_t port_no,
                              const uint32_t       buf_length,
                              uint32_t             *const return_length,
                              uint8_t              *const frame)
    CAP(PHY_MACSEC);

/** \brief Enum for events  */
typedef enum {
    MESA_MACSEC_SEQ_NONE  = 0x0,
    MESA_MACSEC_SEQ_THRESHOLD_EVENT = 0x1,
    MESA_MACSEC_SEQ_ROLLOVER_EVENT  = 0x2,
    MESA_MACSEC_SEQ_ALL   = 0x3
} mesa_macsec_event_t CAP(PHY_MACSEC);


/**
 * \brief Enabling / Disabling of events
 *
 * \param inst    [IN]  Target instance reference.
 * \param port_no [IN]  Port number 
 * \param ev_mask [IN]  Mask containing events that are enabled/disabled 
 * \param enable  [IN]  Enable/disable of event
 *
 * \return Return code.
 **/

mesa_rc mesa_macsec_event_enable_set(const mesa_inst_t         inst,
                                     const mesa_port_no_t      port_no,
                                     const mesa_macsec_event_t ev_mask,
                                     const mesa_bool_t         enable)
    CAP(PHY_MACSEC);

/**
 * \brief Get Enabling of events
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number 
 * \param ev_mask [OUT]  Mask containing events that are enabled 
 *
 * \return Return code.
 **/

mesa_rc mesa_macsec_event_enable_get(const mesa_inst_t    inst,
                                     const mesa_port_no_t port_no,
                                     mesa_macsec_event_t  *const ev_mask)
    CAP(PHY_MACSEC);

/**
 * \brief Polling for active events
 *
 * \param inst    [IN]  Target instance reference.
 * \param port_no [IN]  Port number 
 * \param ev_mask [OUT] Mask containing events that are active
 *
 * \return Return code.
 **/
mesa_rc mesa_macsec_event_poll(const mesa_inst_t    inst,
                               const mesa_port_no_t port_no,
                               mesa_macsec_event_t  *const ev_mask)
    CAP(PHY_MACSEC);


/**
 * \brief Configure the SEQ threshold
 *
 * \param inst      [IN]  Target instance reference.
 * \param port_no   [IN]  Port number 
 * \param threshold [IN]  Event sequence threshold
 *
 * \return Return code.
 **/
mesa_rc mesa_macsec_event_seq_threshold_set(const mesa_inst_t    inst,
                                            const mesa_port_no_t port_no,
                                            const uint32_t       threshold)
    CAP(PHY_MACSEC);

/**
 * \brief Get the SEQ threshold
 *
 * \param inst      [IN]   Target instance reference.
 * \param port_no   [IN]   Port number 
 * \param threshold [OUT]  Event sequence threshold
 *
 * \return Return code.
 **/
mesa_rc mesa_macsec_event_seq_threshold_get(const mesa_inst_t    inst,
                                            const mesa_port_no_t port_no,
                                            uint32_t             *const threshold)
    CAP(PHY_MACSEC);

/**
 * \brief Chip register read
 *
 **/
mesa_rc mesa_macsec_csr_read(const mesa_inst_t    inst,
                             const mesa_port_no_t port_no,
                             const uint16_t       mmd,
                             const uint32_t       addr,
                             uint32_t             *const value)
    CAP(PHY_MACSEC);



/**
 * \brief Chip register write
 *
 **/
mesa_rc mesa_macsec_csr_write(const mesa_inst_t    inst,
                              const mesa_port_no_t port_no,
                              const uint32_t       mmd,
                              const uint32_t       addr,
                              const uint32_t       value)
    CAP(PHY_MACSEC);


 /** \brief Debug counters for counting the number error return codes.  */
typedef struct {
    uint32_t invalid_sci_macaddr;     /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_INVALID_SCI_MACADDR*/
    uint32_t macsec_not_enabled;      /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_NOT_ENABLED*/
    uint32_t secy_already_in_use;     /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_SECY_ALREADY_IN_USE*/
    uint32_t no_secy_found;           /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_NO_SECY_FOUND*/
    uint32_t no_secy_vacency;         /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_NO_SECY_VACANCY*/
    uint32_t invalid_validate_frm;    /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_INVALID_VALIDATE_FRM*/
    uint32_t invalid_hdr_bypass_len;  /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_INVALID_BYPASS_HDR_LEN*/
    uint32_t sc_not_found;            /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_SC_NOT_FOUND*/
    uint32_t could_not_prg_sa_match;  /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_COULD_NOT_PRG_SA_MATCH*/
    uint32_t could_not_prg_sa_flow;   /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_COULD_NOT_PRG_SA_FLOW*/
    uint32_t could_not_ena_sa;        /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_COULD_NOT_ENA_SA*/
    uint32_t could_not_set_sa;        /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_COULD_NOT_SET_SA*/
    uint32_t no_ctrl_frm_match;       /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_NO_CTRL_FRM_MATCH*/
    uint32_t could_not_set_pattern;   /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_COULD_NOT_SET_PATTERN*/
    uint32_t timeout_issue;           /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_TIMEOUT_ISSUE*/
    uint32_t could_not_empty_egress;  /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_COULD_NOT_EMPTY_EGRESS*/
    uint32_t an_not_created;          /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_AN_NOT_CREATED*/
    uint32_t could_not_empty_ingress; /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_COULD_NOT_EMPTY_INGRESS*/
    uint32_t tx_sc_not_exist;         /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_TX_SC_NOT_EXIST*/
    uint32_t could_not_disable_sa;    /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_COULD_NOT_DISABLE_SA*/
    uint32_t could_not_del_rx_sa;     /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_COULD_NOT_DEL_RX_SA*/
    uint32_t could_not_del_tx_sa;     /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_COULD_NOT_DEL_TX_SA*/
    uint32_t pattern_not_set;         /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_PATTERN_NOT_SET*/
    uint32_t hw_resource_exhusted;    /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_HW_RESOURCE_EXHUSTED*/
    uint32_t sci_already_exist;       /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_SCI_ALREADY_EXISTS*/
    uint32_t sc_resource_not_found;   /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_SC_RESOURCE_NOT_FOUND*/
    uint32_t rx_an_already_in_use;    /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_RX_AN_ALREADY_IN_USE*/
    uint32_t empty_record;            /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_EMPTY_RECORD*/
    uint32_t could_not_prg_xform;     /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_COULD_NOT_PRG_XFORM*/
    uint32_t could_not_toggle_sa;     /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_COULD_NOT_TOGGLE_SA*/
    uint32_t tx_an_already_in_use;    /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_TX_AN_ALREADY_IN_USE*/
    uint32_t all_available_sa_in_use; /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_ALL_AVAILABLE_SA_IN_USE*/
    uint32_t match_disable;           /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_MATCH_DISABLE*/
    uint32_t all_cp_rules_in_use;     /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_ALL_CP_RULES_IN_USE*/
    uint32_t pattern_prio_not_valid;  /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_PATTERN_PRIO_NOT_VALID*/
    uint32_t buffer_too_small;        /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_BUFFER_TOO_SMALL*/
    uint32_t frm_too_long;            /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_FRAME_TOO_LONG*/
    uint32_t frm_truncated;           /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_FRAME_TRUNCATED*/
    uint32_t phy_powered_down;        /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_PHY_POWERED_DOWN*/
    uint32_t phy_not_macsec_capable;  /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_PHY_NOT_MACSEC_CAPABLE*/
    uint32_t an_not_exist;            /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_AN_NOT_EXIST*/
    uint32_t no_pattern_cfg;          /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_NO_PATTERN_CFG*/
    uint32_t unexpected_speed;        /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_UNEXPECT_SPEED*/
    uint32_t max_mtu;                 /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_MAX_MTU*/
    uint32_t unexpected_cp_mode;      /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_UNEXPECT_CP_MODE*/
    uint32_t could_not_disable_an;    /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_COULD_NOT_DISABLE_AN*/
    uint32_t rule_out_of_range;       /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_RULE_OUT_OF_RANGE*/
    uint32_t rule_not_exit;           /**< Number of errors happen with error code MESA_RC_ERR_MACSEC_RULE_NOT_EXIT*/
    uint32_t csr_read;                /**< Number of errors happen with error code MESA_RC_ERR_CSR_READ*/
    uint32_t csr_write;               /**< Number of errors happen with error code MESA_RC_ERR_CSR_WRITE*/
    uint32_t unknown_rc_code;         /**< Number of errors happen with unknown error code*/
} mesa_macsec_rc_dbg_counters_t CAP(PHY_MACSEC);

/**
 * \brief Get return code debug counters
 *
 * \param inst      [IN]   Target instance reference.
 * \param port_no   [IN]   Port number 
 * \param counters [OUT]   The Return Code Debug counters
 *
 * \return Return code.
 **/
mesa_rc mesa_macsec_dbg_counter_get(const mesa_inst_t             inst,
                                    const mesa_port_no_t          port_no,
                                    mesa_macsec_rc_dbg_counters_t *const counters)
    CAP(PHY_MACSEC);

/*--------------------------------------------------------------------*/
/* Line MAC / Host MAC / FC                                           */
/*--------------------------------------------------------------------*/

/** \brief Host/Line Mac Counters */
typedef struct {
    /* Rx RMON counters */
    uint64_t if_rx_octets;           /**< In octets       */
    uint64_t if_rx_in_bytes;         /**< In bytes        */
    uint32_t if_rx_ucast_pkts;       /**< In unicasts     */
    uint32_t if_rx_multicast_pkts;   /**< In multicasts   */
    uint32_t if_rx_broadcast_pkts;   /**< In broadcasts   */
    uint32_t if_rx_discards;         /**< In discards     */
    uint32_t if_rx_errors;           /**< In errors       */
  
    uint64_t if_rx_StatsPkts;        /**< In All Pkt cnts    */
    uint32_t if_rx_CRCAlignErrors;   /**< In CRC errors      */
    uint32_t if_rx_UndersizePkts;    /**< In Undersize pkts  */
    uint32_t if_rx_OversizePkts;     /**< In Oversize pkts   */
    uint32_t if_rx_Fragments;        /**< In Fragments       */
    uint32_t if_rx_Jabbers;          /**< In Jabbers         */
    uint32_t if_rx_Pkts64Octets;         /**< In Pkts64Octets         */
    uint32_t if_rx_Pkts65to127Octets;    /**< In Pkts65to127Octets    */
    uint32_t if_rx_Pkts128to255Octets;   /**< In Pkts128to255Octets   */
    uint32_t if_rx_Pkts256to511Octets;   /**< In Pkts256to511Octets   */
    uint32_t if_rx_Pkts512to1023Octets;  /**< In Pkts512to1023Octets  */
    uint32_t if_rx_Pkts1024to1518Octets; /**< In Pkts1024to1518Octets */
    uint32_t if_rx_Pkts1519toMaxOctets;  /**< In Pkts1519toMaxOctets  */

    /* Tx RMON counters */ 
    uint64_t if_tx_octets;           /**< Out octets      */
    uint32_t if_tx_ucast_pkts;       /**< Out unicasts    */
    uint32_t if_tx_multicast_pkts;   /**< Out multicasts  */
    uint32_t if_tx_broadcast_pkts;   /**< Out broadcasts  */
    uint32_t if_tx_errors;           /**< Out errors      */

    uint32_t if_tx_DropEvents;            /**< Out _DropEvents          */
    uint64_t if_tx_StatsPkts;             /**< Out StatsPkts            */
    uint32_t if_tx_Collisions;            /**< Out Collisions           */
    uint32_t if_tx_Pkts64Octets;          /**< Out Pkts64Octets         */
    uint32_t if_tx_Pkts65to127Octets;     /**< Out Pkts65to127Octets    */
    uint32_t if_tx_Pkts128to255Octets;    /**< Out Pkts128to255Octets   */
    uint32_t if_tx_Pkts256to511Octets;    /**< Out Pkts256to511Octets   */
    uint32_t if_tx_Pkts512to1023Octets;   /**< Out Pkts512to1023Octets  */
    uint32_t if_tx_Pkts1024to1518Octets;  /**< Out Pkts1024to1518Octets */
    uint32_t if_tx_Pkts1519toMaxOctets;   /**< Out Pkts1519toMaxOctets  */
} mesa_macsec_mac_counters_t CAP(PHY_MACSEC);

/**
 * \brief Host Mac counters (To be moved)
 *
 **/
mesa_rc mesa_macsec_hmac_counters_get(const mesa_inst_t          inst,
                                      const mesa_port_no_t       port_no,
                                      mesa_macsec_mac_counters_t *const counters,
                                      const mesa_bool_t          clear)
    CAP(PHY_MACSEC);

/**
 * \brief Line Mac counters (To be moved)
 *
 **/
mesa_rc mesa_macsec_lmac_counters_get(const mesa_inst_t          inst,
                                      const mesa_port_no_t       port_no,
                                      mesa_macsec_mac_counters_t *const counters,
                                      const mesa_bool_t          clear)
    CAP(PHY_MACSEC);
/**
 * \brief Function for getting if a port is MACSEC capable 
 * \param inst    [IN]  Target instance reference.
 * \param port_no [IN]  Port number 
 * \param capable [OUT] TRUE is port is macsec capable else FALSE
 *
 * \return MESA_RC_OK if secy_id is valid, else MESA_RC_ERROR.
 **/
mesa_rc mesa_macsec_is_capable(const mesa_inst_t    inst,
                               const mesa_port_no_t port_no,
                               mesa_bool_t          *capable)
    CAP(PHY_MACSEC);
/**
 * \brief Function for dump MACSEC registers  
 * \param inst    [IN]  Target instance reference.
 * \param port_no [IN]  Port number 
 * \param pr [IN] Callback function to print the register values 
 *
 * \return MESA_RC_OK if secy_id is valid, else MESA_RC_ERROR.
 **/

mesa_rc mesa_macsec_dbg_reg_dump(const mesa_inst_t         inst,
                                 const mesa_port_no_t      port_no,
                                 const mesa_debug_printf_t pr)
    CAP(PHY_MACSEC);

/*--------------------------------------------------------------------*/
/* Macsec SC Instance Counters structures                                */
/*--------------------------------------------------------------------*/

/** \brief No. of  Tx SA or Rx SA Information */
typedef struct {
    uint8_t no_sa;                              /**< No. of SAs configured */
    uint8_t sa_id[MESA_MACSEC_SA_PER_SC_MAX];   /**< Configured SA ids */
} mesa_sc_inst_count_t CAP(PHY_MACSEC);

/*--------------------------------------------------------------------*/
/* Macsec Instance Counters structures                                */
/*--------------------------------------------------------------------*/

/** \brief  */
typedef struct {
    // Index part
    uint8_t                 valid;      /**< Indicate if the entry is valid */
    mesa_macsec_port_t      port;       /**< Macsec port */
    mesa_macsec_direction_t direction;  /**< Direction */
    uint8_t                 sc_id;      /**< ID of the secure channel */

    // Data part
    mesa_macsec_sci_t       sci;        /**< sci */
    mesa_sc_inst_count_t    sa_ids;     /**< Tx SC Instances */
} mesa_macsec_port_sc_data_t;

// TODO, create a capability to inform how many entries this method can return

/** \brief Dump configured instance data for a given port.
 *
 * \param inst      [IN]  MESA-API instance.
 * \param port_no   [IN]  Port number
 * \param max       [IN]  Length of the array represented by 'data'
 * \param cnt       [OUT] Number of entries written to 'data'
 * \param data      [OUT] Data buffer
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_port_inst_dump(const mesa_inst_t           inst,
                                   const mesa_port_no_t        port_no,
                                   uint32_t                    max,
                                   uint32_t                    *cnt,
                                   mesa_macsec_port_sc_data_t  *data)
    CAP(PHY_MACSEC);

/** \brief Clear the RMON Line mac counters.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port_no   [IN]  Port number
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_lmac_counters_clear(const mesa_inst_t    inst,
                                        const mesa_port_no_t port_no)
    CAP(PHY_MACSEC);

/** \brief Clear the RMON Host mac counters.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port_no   [IN]  Port number
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_hmac_counters_clear(const mesa_inst_t    inst,
                                        const mesa_port_no_t port_no)
    CAP(PHY_MACSEC);

/** \brief Clear the Macsec Debug counters.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port_no   [IN] Port number
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_debug_counters_clear(const mesa_inst_t    inst,
                                         const mesa_port_no_t port_no)
    CAP(PHY_MACSEC);

/** \brief Clear the Common counters.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port_no   [IN] Port number
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_common_counters_clear(const mesa_inst_t    inst,
                                          const mesa_port_no_t port_no)
    CAP(PHY_MACSEC);

/** \brief Clear the Uncontrolled port counters.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port_no   [IN] Port number
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_uncontrolled_counters_clear(const mesa_inst_t    inst,
                                                const mesa_port_no_t port_no)
    CAP(PHY_MACSEC);

/** \brief Clear the Controlled port counters.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port      [IN] MACsec port.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_controlled_counters_clear (const mesa_inst_t        inst,
                                               const mesa_macsec_port_t port)
    CAP(PHY_MACSEC);

/** \brief Clear the Rx SA counters.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port      [IN] MACsec port.
 * \param sci       [IN] The rx SCI.
 * \param an        [IN] Association number, 0-3
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_rxsa_counters_clear(const mesa_inst_t        inst,
                                        const mesa_macsec_port_t port,
                                        const mesa_macsec_sci_t  *const sci,
                                        const uint16_t           an)
    CAP(PHY_MACSEC);

/** \brief Clear the Rx SC counters.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port      [IN] MACsec port.
 * \param sci       [IN] The rx SCI.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_rxsc_counters_clear(const mesa_inst_t        inst,
                                        const mesa_macsec_port_t port,
                                        const mesa_macsec_sci_t  *const sci)
    CAP(PHY_MACSEC);

/** \brief Clear the Tx SA counters.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port      [IN] MACsec port.
 * \param an        [IN] Association number, 0-3
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_txsa_counters_clear(const mesa_inst_t        inst,
                                        const mesa_macsec_port_t port,
                                        const uint16_t           an)
    CAP(PHY_MACSEC);

/** \brief Clear the Tx SC counters.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port      [IN] MACsec port.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_txsc_counters_clear (const mesa_inst_t        inst,
                                         const mesa_macsec_port_t port)
    CAP(PHY_MACSEC);

/** \brief Clear the SecY counters.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port      [IN] MACsec port.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_secy_counters_clear (const mesa_inst_t        inst,
                                         const mesa_macsec_port_t port)
    CAP(PHY_MACSEC);

/** \brief Get the Macsec Enable Status.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port_no   [IN] Port number
 * \param status    [OUT] MACsec Enable status.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_port_enable_status_get (const mesa_inst_t    inst,
                                            const mesa_port_no_t port_no,
                                            mesa_bool_t          *status)
    CAP(PHY_MACSEC);

/** \brief Get the Macsec RxSA AN Status.
 *
 * \param inst      [IN] MESA-API instance.
 * \param port      [IN] MACsec port.
 * \param sci       [IN] The Rx SCI.
 * \param an        [IN] Association number, 0-3
 * \param status    [OUT] MACsec RxSA AN status.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_rxsa_an_status_get (const mesa_inst_t        inst,
                                        const mesa_macsec_port_t port,
                                        const mesa_macsec_sci_t  *const sci,
                                        const uint16_t           an,
                                        mesa_bool_t              *status)
    CAP(PHY_MACSEC);

/** \brief Get MAC Block MTU and Tag Check configuration.
 *
 * \param inst          [IN] MESA-API instance.
 * \param port_no       [IN] Port number
 * \param mtu_value     [OUT] Max MTU size .
 * \param mtu_tag_check [OUT] Length Check to consider Q-Tags.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_mac_block_mtu_get(const mesa_inst_t    inst,
                               const mesa_port_no_t port_no,
                               uint16_t             *const mtu_value,
                               mesa_bool_t          *const mtu_tag_check)
    CAP(PHY_MACSEC);

/** \brief Set MAC Block MTU and Tag Check configuration.
 *
 * \param inst          [IN] MESA-API instance.
 * \param port_no       [IN] Port number
 * \param mtu_value     [IN] Max MTU size .
 * \param mtu_tag_check [IN] Length Check to consider Q-Tags.
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_mac_block_mtu_set(const mesa_inst_t    inst,
                               const mesa_port_no_t port_no,
                               const uint16_t       mtu_value,
                               const mesa_bool_t    mtu_tag_check)
    CAP(PHY_MACSEC);

/** \brief Set frame gap compensation in FC Buffer.
 *
 * \param inst          [IN] MESA-API instance.
 * \param port_no       [IN] Port number
 * \param frm_gap       [IN] frame gap
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_fcbuf_frame_gap_comp_set(const mesa_inst_t    inst,
                                             const mesa_port_no_t port_no,
                                             const uint8_t        frm_gap)
    CAP(PHY_MACSEC);

/** \brief Flow Control buffer Block Reg Dump.
 *
 * \param inst          [IN] MESA-API instance.
 * \param port_no       [IN] Port number
 * \param pr            [IN] Callback function to print the register values 
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_dbg_fcb_block_reg_dump(const mesa_inst_t         inst,
                                           const mesa_port_no_t      port_no,
                                           const mesa_debug_printf_t pr)
    CAP(PHY_MACSEC);


/** \brief Flow Control buffer Block Reg Dump.
 *
 * \param inst          [IN] MESA-API instance.
 * \param port_no       [IN] Port number
 * \param pr            [IN] Callback function to print the register values 
 *
 * \return MESA_RC_OK when successful; MESA_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_dbg_frm_match_handling_ctrl_reg_dump(const mesa_inst_t         inst,
                                                         const mesa_port_no_t      port_no,
                                                         const mesa_debug_printf_t pr)
    CAP(PHY_MACSEC);

/** \brief Configure MACsec Update sequence number.
 *
 * \param inst          [IN] MESA-API instance.
 * \param port          [IN] MACsec Port
 * \param sci           [IN] SCI of the peer
 * \param an            [IN] Association number, 0-3
 * \param egr           [IN] Direction Egress/Ingress
 * \param disable       [IN] Operation 1:Disable/0:Enable
 *
 * \return VTSS_RC_OK when successful; VTSS_RC_ERROR if parameters are invalid.
 */
mesa_rc mesa_macsec_dbg_update_seq_set(const mesa_inst_t        inst,
                                       const mesa_macsec_port_t port,
                                       const mesa_macsec_sci_t  *const sci,
                                       uint16_t                 an,
                                       mesa_bool_t              egr,
                                       const mesa_bool_t        disable)

    CAP(PHY_MACSEC);

/**
 * \brief Get the Egress Interrupt SA Active AN
 *
 * \param inst      [IN]   Target instance reference.
 * \param port_no   [IN]   Port number
 * \param port      [OUT]  MACsec port
 * \param an        [OUT]  Egress Interrupted Association number
 *
 * \return Return code.
 **/
mesa_rc mesa_macsec_egr_intr_sa_get(const mesa_inst_t    inst,
                                    const mesa_port_no_t port_no,
                                    mesa_macsec_port_t   *const port,
                                    uint16_t             *const an)
    CAP(PHY_MACSEC);

#include <mscc/ethernet/switch/api/hdr_end.h>
#endif // _MSCC_ETHERNET_SWITCH_API_MACSEC_
