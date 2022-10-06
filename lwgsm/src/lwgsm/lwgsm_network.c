/**
 * \file            lwgsm_network.c
 * \brief           Network API
 */

/*
 * Copyright (c) 2020 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwGSM - Lightweight GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.1.0
 */
#include "lwgsm/lwgsm_private.h"
#include "lwgsm/lwgsm_network.h"
#include "lwgsm/lwgsm_mem.h"
#if LWGSM_SIM7080
#include "lwgsm/lwgsm_parser.h"
#endif

#if LWGSM_CFG_NETWORK || __DOXYGEN__

/**
 * \brief           Attach to network and active PDP context
 * \param[in]       apn: APN name
 * \param[in]       user: User name to attach. Set to `NULL` if not used
 * \param[in]       pass: User password to attach. Set to `NULL` if not used
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_network_attach(const char* apn, const char* user, const char* pass,
                   const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_NETWORK_ATTACH;
#if LWGSM_CFG_CONN
#if !LWGSM_SIM7080
    LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CIPSTATUS;
#else
    LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CGATT_GET;
#endif
#endif /* LWGSM_CFG_CONN */
#if !LWGSM_SIM7080
    LWGSM_MSG_VAR_REF(msg).msg.network_attach.apn = apn;
    LWGSM_MSG_VAR_REF(msg).msg.network_attach.user = user;
    LWGSM_MSG_VAR_REF(msg).msg.network_attach.pass = pass;
#else
    LWGSM_UNUSED(apn);
    LWGSM_UNUSED(user);
    LWGSM_UNUSED(pass);
#endif

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 200000);
}

/**
 * \brief           Detach from network
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_network_detach(const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_NETWORK_DETACH;
#if LWGSM_CFG_CONN
    /* LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CIPSTATUS; */
#endif /* LWGSM_CFG_CONN */

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Check network PDP status
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_network_check_status(const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
#if !LWGSM_SIM7080
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CIPSTATUS;
#else
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CGATT_GET;
#endif

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 60000);
}

/**
 * \brief           Copy IP address from internal value to user variable
 * \param[out]      ip: Pointer to output IP variable
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_network_copy_ip(lwgsm_ip_t* ip) {
    if (lwgsm_network_is_attached()) {
        lwgsm_core_lock();
        LWGSM_MEMCPY(ip, &lwgsm.m.network.ip_addr, sizeof(*ip));
        lwgsm_core_unlock();
        return lwgsmOK;
    }
    return lwgsmERR;
}

/**
 * \brief           Check if device is attached to network and PDP context is active
 * \return          `1` on success, `0` otherwise
 */
uint8_t
lwgsm_network_is_attached(void) {
    uint8_t res;
    lwgsm_core_lock();
    res = LWGSM_U8(lwgsm.m.network.is_attached);
    lwgsm_core_unlock();
    return res;
}

#endif /* LWGSM_CFG_NETWORK || __DOXYGEN__ */

/**
 * \brief           Read RSSI signal from network operator
 * \param[out]      rssi: RSSI output variable. When set to `0`, RSSI is not valid
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_network_rssi(int16_t* rssi,
                 const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking) {
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_CSQ_GET;
    LWGSM_MSG_VAR_REF(msg).msg.csq.rssi = rssi;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 120000);
}

/**
 * \brief           Get network registration status
 * \return          Member of \ref lwgsm_network_reg_status_t enumeration
 */
lwgsm_network_reg_status_t
lwgsm_network_get_reg_status(void) {
    lwgsm_network_reg_status_t ret;
    lwgsm_core_lock();
    ret = lwgsm.m.network.status;
    lwgsm_core_unlock();
    return ret;
}

#if LWGSM_SIM7080
/**
 * \brief           Set system network APN settings before asking for attach
 * \param[in]       idx: APN index. Should be between 1 and 15. If set to `NULL` the index is zero
 * \param[in]       pdp_type: PDP type. If set to `NULL`, PDP type is IP 
 * \param[in]       apn: APN domain. Set to `NULL` if not used
 * \param[in]       pdp_addr: A string parameter that identifies the MT in the address space applicable to the PDP.
 *                              If `NULL` the address is "0.0.0.0"
 * \param[in]       d_comp: A numeric parameter that controls PDP data compression. If set to `NULL` compression is OFF
 * \param[in]       h_comp: A numeric parameter that controls PDP head compression. If set to `NULL` compression is OFF
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_network_define_pdp_context(const int idx, const lwgsm_apn_pdp_type_t pdp_type, const char* apn, const char* pdp_addr, const lwgsm_apn_d_comp_t d_comp, const lwgsm_apn_h_comp_t h_comp, const bool ipv4_ctrl, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking)
{
    lwgsm_ip_t* pdp_addr_tmp = lwgsm_mem_calloc(1, sizeof(lwgsm_ip_t));

    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_DEFINE_PDP;
    LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CGACT_SET_0;

    if(idx < 1 || idx > 15){
        return lwgsmERR;
    }

    LWGSM_MSG_VAR_REF(msg).msg.pdp_context.idx = idx;
    
    switch (pdp_type)
    {
    case LWGSM_PDP_TYPE_IP:
        LWGSM_MSG_VAR_REF(msg).msg.pdp_context.pdp_type = "IP";
        break;
    case LWGSM_PDP_TYPE_PPP:
        LWGSM_MSG_VAR_REF(msg).msg.pdp_context.pdp_type = "PPP";
        break;
    case LWGSM_PDP_TYPE_IPV6:
        LWGSM_MSG_VAR_REF(msg).msg.pdp_context.pdp_type = "IPV6";
        break;
    case LWGSM_PDP_TYPE_IPV4V6:
        LWGSM_MSG_VAR_REF(msg).msg.pdp_context.pdp_type = "IPV4V6";
        break;
    case LWGSM_PDP_TYPE_NON_IP:
        LWGSM_MSG_VAR_REF(msg).msg.pdp_context.pdp_type = "Non-IP";
        break;
    default:
        break;
    }

    LWGSM_MSG_VAR_REF(msg).msg.pdp_context.apn = apn;

    lwgsmi_parse_ip(&pdp_addr, pdp_addr_tmp);
    LWGSM_MSG_VAR_REF(msg).msg.pdp_context.pdp_addr = pdp_addr_tmp;

    LWGSM_MSG_VAR_REF(msg).msg.pdp_context.d_comp = d_comp;

    LWGSM_MSG_VAR_REF(msg).msg.pdp_context.h_comp = h_comp;

    LWGSM_MSG_VAR_REF(msg).msg.pdp_context.ipv4_ctrl = (uint8_t) ipv4_ctrl;

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 200000);
}
#endif