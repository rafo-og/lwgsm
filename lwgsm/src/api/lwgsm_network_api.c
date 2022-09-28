/**
 * \file            lwgsm_network_api.c
 * \brief           API functions for multi-thread network functions
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
#include "lwgsm/lwgsm_network_api.h"
#include "lwgsm/lwgsm_private.h"
#include "lwgsm/lwgsm_network.h"

#if LWGSM_CFG_NETWORK || __DOXYGEN__

/* Network credentials used during connect operation */
static const char* network_apn;
static const char* network_user;
static const char* network_pass;
static uint32_t network_counter;

/**
 * \brief           Set system network credentials before asking for attach
 * \param[in]       apn: APN domain. Set to `NULL` if not used
 * \param[in]       user: APN username. Set to `NULL` if not used
 * \param[in]       pass: APN password. Set to `NULL` if not used
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_network_set_credentials(const char* apn, const char* user, const char* pass) {
    network_apn = apn;
    network_user = user;
    network_pass = pass;

    return lwgsmOK;
}

/**
 * \brief           Request manager to attach to network
 * \note            This function is blocking and cannot be called from event functions
 * \return          \ref lwgsmOK on success (when attached), member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_network_request_attach(void) {
    lwgsmr_t res = lwgsmOK;
    uint8_t do_conn = 0;

    /* Check if we need to connect */
    lwgsm_core_lock();
    if (network_counter == 0) {
        if (!lwgsm_network_is_attached()) {
            do_conn = 1;
        }
    }
    if (!do_conn) {
        ++network_counter;
    }
    lwgsm_core_unlock();

    /* Connect to network */
    if (do_conn) {
        res = lwgsm_network_attach(network_apn, network_user, network_pass, NULL, NULL, 1);
        if (res == lwgsmOK) {
            lwgsm_core_lock();
            ++network_counter;
            lwgsm_core_unlock();
        }
    }
    return res;
}

/**
 * \brief           Request manager to detach from network
 *
 * If other threads use network, manager will not disconnect from network
 * otherwise it will disable network access
 *
 * \note            This function is blocking and cannot be called from event functions
 * \return          \ref lwgsmOK on success (when attached), member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_network_request_detach(void) {
    lwgsmr_t res = lwgsmOK;
    uint8_t do_disconn = 0;

    /* Check if we need to disconnect */
    lwgsm_core_lock();
    if (network_counter > 0) {
        if (network_counter == 1) {
            do_disconn = 1;
        } else {
            --network_counter;
        }
    }
    lwgsm_core_unlock();

    /* Connect to network */
    if (do_disconn) {
        res = lwgsm_network_detach(NULL, NULL, 1);
        if (res == lwgsmOK) {
            lwgsm_core_lock();
            --network_counter;
            lwgsm_core_unlock();
        }
    }
    return res;
}

/**
 * \brief           Set system network APN settings before asking for attach
 * \param[in]       idx: APN index. Should be between 1 and 15. If set to `NULL` the index is zero
 * \param[in]       pdp_type: PDP type. If set to `NULL`, PDP type is IP 
 * \param[in]       apn: APN domain. Set to `NULL` if not used
 * \param[in]       pdp_addr: A string parameter that identifies the MT in the address space applicable to the PDP.
 *                              If `NULL` the address is "0.0.0.0"
 * \param[in]       d_comp: A numeric parameter that controls PDP data compression. If set to `NULL` compression is OFF
 * \param[in]       h_comp: A numeric parameter that controls PDP head compression. If set to `NULL` compression is OFF
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
lwgsmr_t
lwgsm_network_request_define_pdp_context(const int idx, const lwgsm_apn_pdp_type_t pdp_type, const char* apn, const char* pdp_addr, const lwgsm_apn_d_comp_t d_comp, const lwgsm_apn_h_comp_t h_comp, const bool ipv4_ctrl, const bool blocking) {

    return lwgsm_network_define_pdp_context(idx, pdp_type, apn, pdp_addr, d_comp, h_comp, ipv4_ctrl, NULL, NULL, (uint32_t) blocking);

}

#endif /* LWGSM_CFG_NETWORK || __DOXYGEN__ */
