/**
 * @file lwgsm_provision_esp32.c
 * @author Rafael de la Rosa (derosa.rafael@gmail.com)
 * @brief Get stack values from NVS partition in flash for ESP32
 * @version 0.1
 * @date 2022-11-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

/* AWS provisioning library */
#include "aws_provisioning.h"

/* Own header */
#include "lwgsm_provision.h"


/**
 * @brief Get the pin code from NVS partition
 * 
 * @param pin_code Pointer where the pin code is going to be stored
 * @return lwgsmOK if sucess, lwgsmERR otherwise
 */
lwgsmr_t lwgsm_provision_get_pin_code(char** pin_code)
{
    esp_err_t ret;
    ret = aws_prov_get_pin_code(pin_code);

    if(ret == ESP_OK){
        return lwgsmOK;
    }else{
        return lwgsmERR;
    }
}