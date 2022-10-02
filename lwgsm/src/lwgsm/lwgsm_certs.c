/**
 * \file            lwgsm_certs.c
 * \brief           Write and read SSL certificates API functions
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
 * Author:          Rafael de la Rosa <derosa.rafael@gmail.com>
 * Version:         v0.0.0
 */

#include "lwgsm/lwgsm_private.h"
#include "lwgsm/lwgsm_mem.h"
#include "lwgsm/lwgsm_certs.h"

#define SIMCOM7080_FS_MAX_FILE_SIZE  10240

/**
 * \brief           Write a given certificate to GSM module
 * \param[in]       cert: The certificate content
 * \param[in]       cert_len: Certificate length in bytes
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsmi_certs_write(const char* cert_filename, const char* cert, uint32_t cert_len, const lwgsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking)
{    
    LWGSM_MSG_VAR_DEFINE(msg);

    LWGSM_MSG_VAR_ALLOC(msg, blocking);
    LWGSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    LWGSM_MSG_VAR_REF(msg).cmd_def = LWGSM_CMD_FS_WRITE;
    LWGSM_MSG_VAR_REF(msg).cmd = LWGSM_CMD_CFSINIT;

    if(cert == NULL){
        return lwgsmPARERR;
    }

    if(cert_len > 0){
        LWGSM_MSG_VAR_REF(msg).msg.fs_write.file_name = cert_filename;
        LWGSM_MSG_VAR_REF(msg).msg.fs_write.data = cert;
        LWGSM_MSG_VAR_REF(msg).msg.fs_write.file_size = cert_len;
        LWGSM_MSG_VAR_REF(msg).msg.fs_write.idx = 3;
        LWGSM_MSG_VAR_REF(msg).msg.fs_write.mode = 0;
        LWGSM_MSG_VAR_REF(msg).msg.fs_write.timeout = 5000;
    }

    return lwgsmi_send_msg_to_producer_mbox(&LWGSM_MSG_VAR_REF(msg), lwgsmi_initiate_cmd, 200000);
}

/**
 * \brief           Write certificates needed to two-way SSL/TLS connection
 * \param[in]       ca_root_crt: CA root server certificate
 * \param[in]       client_crt: Client certificate
 * \param[in]       client_key: Client key
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t
lwgsm_certs_set_ssl(const char* ca_root_crt, const char* client_crt, const char* client_key)
{
    lwgsmr_t ret = lwgsmERR;
    size_t ca_root_crt_len = 0;
    size_t client_crt_len = 0;
    size_t client_key_len = 0;

    if(ca_root_crt != NULL){
        ca_root_crt_len = strlen(ca_root_crt);
        if(ca_root_crt_len > SIMCOM7080_FS_MAX_FILE_SIZE){
            ret = lwgsmERRMEM;
        }else{
            ret = lwgsmi_certs_write(CA_ROOT_LABEL, ca_root_crt, ca_root_crt_len, NULL, NULL, 1);            
        }
    }

    if(ret != lwgsmOK){
        return ret;
    }

    if(client_crt != NULL){
        client_crt_len = strlen(client_crt);
        if(client_crt_len > SIMCOM7080_FS_MAX_FILE_SIZE){
            ret = lwgsmERRMEM;
        }else{
            ret = lwgsmi_certs_write(CLIENT_CERT_LABEL, client_crt, client_crt_len, NULL, NULL, 1);            
        }
    }

    if(ret != lwgsmOK){
        return ret;
    }

    if(client_key != NULL){
        client_key_len = strlen(client_key);
        if(client_key_len > SIMCOM7080_FS_MAX_FILE_SIZE){
            ret = lwgsmERRMEM;
        }else{
            ret = lwgsmi_certs_write(CLIENT_KEY_LABEL, client_key, client_key_len, NULL, NULL, 1);
        }
    }

    return ret;
}