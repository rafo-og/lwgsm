#include "sim_manager.h"
#include "lwgsm/lwgsm_mem.h"
#include "lwgsm_provision.h"

/**
 * \brief           SIM card pin code
 */
// static const char*
// pin_code = "1234";

/**
 * \brief           SIM card puk code
 */
// static const char*
// puk_code = "12926752";

/**
 * \brief           Configure and enable SIM card
 * \return          `1` on success, `0` otherwise
 */
uint8_t
configure_sim_card(void) {

    lwgsmr_t ret;
    char* pin_code;

    ret = lwgsm_provision_get_pin_code(&pin_code);
    if(ret != ESP_OK){
        return 0;
    }

    if (pin_code != NULL && strlen(pin_code)) {
        if (lwgsm_sim_pin_enter(pin_code, NULL, NULL, 1) == lwgsmOK) {
            lwgsm_mem_free(pin_code);
            return 1;
        }
        return 0;
    }
    
    lwgsm_mem_free(pin_code);
    return 1;
}

/**
 * \brief   Report SIM card state
 * \param[in]   evt: The event received from stack
 * \param[in]   out: Pointer to string report
 * \return          `1` on success, `0` otherwise
 */
uint8_t
process_sim_evt(lwgsm_evt_t* evt, char** out)
{
    char* body;
    size_t report_len;

    if(evt->type != LWGSM_EVT_SIM_STATE_CHANGED){
        return 0;
    }

    switch(evt->evt.cpin.state){
        case LWGSM_SIM_STATE_NOT_INSERTED:
            body = "SIM state: LWGSM_SIM_STATE_NOT_INSERTED. ";
        break;
        case LWGSM_SIM_STATE_READY:
            body = "SIM state: LWGSM_SIM_STATE_READY. ";
        break;
        case LWGSM_SIM_STATE_NOT_READY:
            body = "SIM state: LWGSM_SIM_STATE_NOT_READY. ";
        break;
        case LWGSM_SIM_STATE_PIN:
            body = "SIM state: LWGSM_SIM_STATE_PIN. ";
        break;
        case LWGSM_SIM_STATE_PUK:
            body = "SIM state: LWGSM_SIM_STATE_PUK. ";
        break;
        case LWGSM_SIM_STATE_PH_PIN:
            body = "SIM state: LWGSM_SIM_STATE_PH_PIN. ";
        break;
        case LWGSM_SIM_STATE_PH_PUK:
            body = "SIM state: LWGSM_SIM_STATE_PH_PUK. ";
        break;
        default:
            body = "SIM state: UNKNOWN. ";
        break;
    }

    report_len = snprintf(NULL, 0, body);
    *out = lwgsm_mem_malloc(report_len);
    snprintf(*out, report_len, body);

    return 1;
}