#ifndef SNIPPET_HDR_SIM_MANAGER_H
#define SNIPPET_HDR_SIM_MANAGER_H

#include <stdint.h>
#include "lwgsm/lwgsm.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

uint8_t     configure_sim_card(void);
uint8_t     process_sim_evt(lwgsm_evt_t* evt);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SNIPPET_HDR_SIM_MANAGER_H */
