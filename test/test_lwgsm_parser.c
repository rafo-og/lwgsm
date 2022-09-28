#include <stdio.h>
#include "unity.h"
#include "lwgsm/lwgsm.h"
#include "operator_utils.h"

static lwgsmr_t lwgsm_callback_func(lwgsm_evt_t* evt);

TEST_CASE("Operator scan parser (AT+COPS=?)", "[lwgsm_parser]")
{
        const char* inputStr = "+COPS:(2,\"CHINA MOBILE\",\"CMCC\",\"46000\",0),(1,\"CHINA MOBILE\",\"CMCC\",\"46000\",1),(3,\"CHN-UNICOM\",    \
                                \"UNICOM\",\"46001\",2),(1,\"CHN-CT\",\"CT\",\"46011\",3),(3,\"CHN-UNICOM\",\"UNICOM\",\"46001\",4),,(0,1,2,3,4),(0,1,2)\r\nOK\r\n\r\n";
        lwgsmr_t ret;
        size_t ops_size, ops_found;
        lwgsm_operator_t* ops_array;

        /* Allocate memory for operator scanning */
        ops_size = 10;
        ops_array = pvPortMalloc(sizeof(*ops_array) * ops_size);
        memset(ops_array, 0, sizeof(*ops_array) * ops_size);
        configASSERT(ops_array);

        /* Initialize GSM with default callback function */
        printf("Initializing LWGSM... \n");
        ret = lwgsm_init(lwgsm_callback_func, 1);
        if (ret != lwgsmOK) {
                printf("Cannot initialize LwGSM\r\n");
        }

        /* Operator scanning */
        ret = lwgsm_operator_scan(ops_array, ops_size, &ops_found, NULL, NULL, 0);
        configASSERT(ret == lwgsmOK);

        lwgsm_delay(1000/portTICK_PERIOD_MS);

        if(ret == lwgsmOK){
                ret = lwgsmERR;
                ret = lwgsm_input_process(inputStr, strlen(inputStr));
                if(ret != lwgsmOK){
                        printf("Data processing error.\r\n");
                }
                else{
                        printf("Data processed correctly.\r\n");
                }
        }

        lwgsm_delay(5000/portTICK_PERIOD_MS);

        lwgsm_deinit();

        vPortFree(ops_array);
}

/**
 * \brief           Event callback function for GSM stack
 * \param[in]       evt: Event information with data
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t otherwise
 */
static lwgsmr_t lwgsm_callback_func(lwgsm_evt_t* evt)
{
    switch (lwgsm_evt_get_type(evt)) {
        case LWGSM_EVT_INIT_FINISH: 
            printf("Library initialized!\r\n");
            break;
        case LWGSM_EVT_RESET:
            printf("Device reset complete.\r\n");
            break;
        // case LWGSM_EVT_CONN_ACTIVE:
        //     ESP_LOGI(TAG, "Keep alive event.");
        //     break;
        // case LWGSM_EVT_DEVICE_IDENTIFIED:
        //     ESP_LOGI(TAG, "Device identified.");
        //     break;
        // case LWGSM_EVT_CMD_TIMEOUT:
        //     ESP_LOGW(TAG, "LWGSM timeout!");
        //     break;
        // /* Process and print registration change */
        // case LWGSM_EVT_NETWORK_REG_CHANGED:
        //     network_utils_process_reg_change(evt);
        //     break;
        // /* Process current network operator */
        // case LWGSM_EVT_NETWORK_OPERATOR_CURRENT:
        //     network_utils_process_curr_operator(evt);
        //     break;
        // /* Process signal strength */
        // case LWGSM_EVT_SIGNAL_STRENGTH:
        //     network_utils_process_rssi(evt);
        //     break;
        // case LWGSM_EVT_SIM_STATE_CHANGED:
        //     ESP_LOGI(TAG, "SIM state changed to: %d", evt->evt.cpin.state);
        //     simState = evt->evt.cpin.state;
        //     break;
        case LWGSM_EVT_OPERATOR_SCAN:
            operator_utils_print_scan(evt);
            break;
        /* Other user events here... */

        default:
            printf("CB called but not captured. Event: %d\r\n", lwgsm_evt_get_type(evt));
            break;
    }
    return lwgsmOK;
}