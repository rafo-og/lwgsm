/**
 * @file lwgsm_provision_esp32.h
 * @author Rafael de la Rosa Vidal (derosa.rafael@gmail.com)
 * @brief Device information provisioning
 * @version 0.1
 * @date 2022-11-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef LWGSM_PROVISION_H
#define LWGSM_PROVISION_H

#include "lwgsm/lwgsm.h"

lwgsmr_t lwgsm_provision_get_pin_code(char** pin_code);

#endif /* LWGSM_PROVISION_ESP32_H */