#include "network_utils.h"
#include "lwgsm/lwgsm.h"
#include "lwgsm/lwgsm_mem.h"
#include <string.h>


/**
 * \brief           RSSI state on network
 */
static int16_t
rssi;

/**
 * \brief           Process and report network registration status update
 * \param[in]       evt: GSM event data
 * \param[in]       out: Pointer to string report
 */
void
network_utils_process_reg_change(lwgsm_evt_t* evt, char** out) {
    lwgsm_network_reg_status_t stat;
    char* header = "Network registration status changed. New status is: ";
    char* body;
    size_t report_len;

    stat = lwgsm_network_get_reg_status();        /* Get network status */

    /* Print to console */
    switch (stat) {
        case LWGSM_NETWORK_REG_STATUS_CONNECTED:
            body = "Connected to home network.\0";
            break;
        case LWGSM_NETWORK_REG_STATUS_CONNECTED_ROAMING:
            body = "Connected to network and roaming.\0";
            break;
        case LWGSM_NETWORK_REG_STATUS_DENIED:
            body = "Connection denied.\0";
            break;
        case LWGSM_NETWORK_REG_STATUS_SEARCHING:
            body = "Searching for network.\0";
            break;
        case LWGSM_NETWORK_REG_STATUS_SIM_ERR:
            body = "SIM CARD ERROR.\0";
            break;
        default:
            body = "Other.\0";
    }

    report_len = strlen(header) + strlen(body);
    *out = lwgsm_mem_calloc(report_len, sizeof(char));
    snprintf(*out, strlen(header)+1, header);
    snprintf(*out+strlen(header), strlen(body)+1, body);

    LWGSM_UNUSED(evt);
}

/**
 * \brief           Process and report network current operator status
 * \param[in]       evt: GSM event data
 * \param[in]       out: Pointer to string report
 */
void
network_utils_process_curr_operator(lwgsm_evt_t* evt, char** out) {
    const lwgsm_operator_curr_t* o;
    size_t report_len;

    o = lwgsm_evt_network_operator_get_current(evt);
    if (o != NULL) {
        switch (o->format) {
            case LWGSM_OPERATOR_FORMAT_LONG_NAME:
                report_len = snprintf(NULL, 0, "Operator long name: %s", o->data.long_name);
                *out = lwgsm_mem_calloc(report_len, sizeof(char));
                snprintf(*out, ++report_len, "Operator long name: %s", o->data.long_name);
                break;
            case LWGSM_OPERATOR_FORMAT_SHORT_NAME:
                report_len = snprintf(NULL, 0, "Operator short name: %s", o->data.short_name);
                *out = lwgsm_mem_calloc(report_len, sizeof(char));
                snprintf(*out, ++report_len, "Operator short name: %s", o->data.short_name);
                break;
            case LWGSM_OPERATOR_FORMAT_NUMBER:
                report_len = snprintf(NULL, 0, "Operator number: %d", (int)o->data.num);
                *out = lwgsm_mem_calloc(report_len, sizeof(char));
                snprintf(*out, ++report_len, "Operator number: %d", (int)o->data.num);
                break;
            default:
                break;
        }
    }
    /* Start RSSI info */
    lwgsm_network_rssi(&rssi, NULL, NULL, 0);
}

/**
 * \brief           Process and report RSSI info
 * \param[in]       evt: GSM event data
 * \param[in]       out: Pointer to string report
 */
void
network_utils_process_rssi(lwgsm_evt_t* evt, char** out) {
    int16_t rssi;
    size_t report_len;

    /* Get RSSi from event */
    rssi = lwgsm_evt_signal_strength_get_rssi(evt);

    /* Write the report in memory */
    report_len = snprintf(NULL, 0, "Network operator RSSI: %d dBm\r\n", (int)rssi);
    *out = lwgsm_mem_calloc(report_len, sizeof(char));
    snprintf(*out, report_len, "Network operator RSSI: %d dBm\r\n", (int)rssi);
}
