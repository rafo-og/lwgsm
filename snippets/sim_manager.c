#include "sim_manager.h"

/**
 * \brief           SIM card pin code
 */
static const char*
pin_code = "5873";

/**
 * \brief           SIM card puk code
 */
static const char*
puk_code = "12926752";

/**
 * \brief           Configure and enable SIM card
 * \return          `1` on success, `0` otherwise
 */
uint8_t
configure_sim_card(void) {
    LWGSM_UNUSED(puk_code);
    if (pin_code != NULL && strlen(pin_code)) {
        if (lwgsm_sim_pin_enter(pin_code, NULL, NULL, 1) == lwgsmOK) {
            return 1;
        }
        return 0;
    }
    return 1;
}

/**
 * \brief   Print SIM state
 * \param[in]   evt: The event received from stack
 * \return          `1` on success, `0` otherwise
 */
uint8_t
process_sim_evt(lwgsm_evt_t* evt){
    if(evt->type != LWGSM_EVT_SIM_STATE_CHANGED){
        return 0;
    }

    switch(evt->evt.cpin.state){
        case LWGSM_SIM_STATE_NOT_INSERTED:
            printf("SIM state: LWGSM_SIM_STATE_NOT_INSERTED.\r\n");
        break;
        case LWGSM_SIM_STATE_READY:
            printf("SIM state: LWGSM_SIM_STATE_READY.\r\n");
        break;
        case LWGSM_SIM_STATE_NOT_READY:
            printf("SIM state: LWGSM_SIM_STATE_NOT_READY.\r\n");
        break;
        case LWGSM_SIM_STATE_PIN:
            printf("SIM state: LWGSM_SIM_STATE_PIN.\r\n");
        break;
        case LWGSM_SIM_STATE_PUK:
            printf("SIM state: LWGSM_SIM_STATE_PUK.\r\n");
        break;
        case LWGSM_SIM_STATE_PH_PIN:
            printf("SIM state: LWGSM_SIM_STATE_PH_PIN.\r\n");
        break;
        case LWGSM_SIM_STATE_PH_PUK:
            printf("SIM state: LWGSM_SIM_STATE_PH_PUK.\r\n");
        break;
        default:
            printf("SIM state: UNKNOWN.\r\n");
        break;
    }

    return 1;
}