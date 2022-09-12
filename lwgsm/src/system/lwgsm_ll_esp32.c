/**
 * \file            lwgsm_ll_template.c
 * \brief           Low-level communication with GSM device template
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
#if LWGSM_UART_DEBUG_LEVEL == ESP_LOG_DEBUG
    #include <string.h>
#endif

#include "system/lwgsm_ll.h"
#include "lwgsm/lwgsm.h"
#include "lwgsm/lwgsm_mem.h"
#include "lwgsm/lwgsm_input.h"
#include "lwgsm/lwgsm_private.h"

#include "lwgsm_opts.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"

#if !defined(LWGSM_UART_NUM)
#define LWGSM_UART_NUM              UART_NUM_0
#endif /* !defined(LWGSM_UART_NUM) */

#if !defined(LWGSM_UART_RX_BUF_SIZE)
#define LWGSM_UART_RX_BUF_SIZE      0x1000
#endif /* !defined(LWGSM_UART_RX_BUF_SIZE) */

#if !defined(LWGSM_UART_TX_BUF_SIZE)
#define LWGSM_UART_TX_BUF_SIZE      0x1000
#endif /* !defined(LWGSM_UART_TX_BUF_SIZE) */

#if !defined(LWGSM_UART_QUEUE_SIZE)
#define LWGSM_UART_QUEUE_SIZE       10
#endif /* !defined(LWGSM_UART_QUEUE_SIZE) */

#if !defined(LWGSM_UART_DEBUG_LEVEL)
#define LWGSM_UART_DEBUG_LEVEL      ESP_LOG_INFO
#endif /* !defined(LWGSM_UART_DEBUG_LEVEL) */

#if !defined(LWGSM_UART_TX)
#define LWGSM_UART_TX               (GPIO_NUM_22)
#endif /* !defined(LWGSM_UART_TX) */

#if !defined(LWGSM_UART_RX)
#define LWGSM_UART_RX               (GPIO_NUM_19)
#endif /* !defined(LWGSM_UART_RX) */

#if defined(LWGSM_RESET_PIN)
#define LWGSM_RESET_PIN_MASK_BIT    (1ULL << LWGSM_RESET_PIN)
#endif

#define CHECK_OS_STATUS(x, func)    do{ \
                                        if(x != pdPASS){ \
                                            ESP_LOGE(TAG, "File [%s] . Line: %d || Error (0x%x) from [%s]", __FILE__, __LINE__, x, func);\
                                        }\
                                        while(0); \
                                    }while(0)

#define CHECK_TIMEOUT(x, func)      do{ \
                                        if(x == LWGSM_SYS_TIMEOUT){ \
                                            ESP_LOGW(TAG, "File [%s] || Line: %d || TIMEOUT from [%s]", __FILE__, __LINE__, func);\
                                        }\
                                        while(0); \
                                    }while(0)

#define TASK_PRIORITY_MEDIUM        configMAX_PRIORITIES/2

#define LWGSM_READING_THREAD_STACK_SIZE         4096

#if !LWGSM_CFG_MEM_CUSTOM
static uint8_t memory[0x10000];             /* Create memory for dynamic allocations with specific size */
#endif
static uint8_t is_running, initialized;
static const char *TAG = "LWGSM_UART";

/* USART thread */
static void usart_ll_thread(void* arg);
static lwgsm_sys_thread_t usart_ll_thread_id;

/* Events queue */
static lwgsm_sys_mbox_t uart_event_ll_mbox_id;

/* Hardware reset function */
#if defined(LWGSM_RESET_PIN)
static uint8_t reset_device(uint8_t state);
#endif

/**
 * \brief           USART data processing
 */
static void usart_ll_thread(void* arg)
{
    uart_event_t event;
    uart_event_t* eventPtr = &event;
    uint8_t* dtmp = (uint8_t*) malloc(LWGSM_UART_RX_BUF_SIZE);

    // To avoid non-used warning
    (void)(arg);

    LWGSM_DEBUGF(LWGSM_CFG_DBG_THREAD | LWGSM_DBG_TYPE_TRACE | LWGSM_DBG_LVL_ALL, "[UART THREAD] Thread started");

    while(lwgsm.status.f.runningLLThread){
        //Waiting for UART event.
        if(lwgsm_sys_mbox_get(&uart_event_ll_mbox_id, (void **)&eventPtr, 5000/portTICK_PERIOD_MS) != LWGSM_SYS_TIMEOUT){
            bzero(dtmp, LWGSM_UART_RX_BUF_SIZE);
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    uart_read_bytes(LWGSM_UART_NUM, dtmp, event.size, portMAX_DELAY);
                    printf("[DATA EVT]:");
                    for(int i=0; i<event.size; i++){
                        printf("%c", *(dtmp + i));
                    }
                    printf("\r\n");
                    lwgsm_input_process(dtmp, event.size);
                    break;
                //Others
                default:
                    ESP_LOGW(TAG, "uart event not captured type: %d", event.type);
                    break;
            }
        }
        bzero(&event, sizeof(event));
    }
    
    LWGSM_DEBUGF(LWGSM_CFG_DBG_THREAD | LWGSM_DBG_TYPE_TRACE | LWGSM_DBG_LVL_ALL, "[UART THREAD] Thread exit");
    free(dtmp);
    dtmp = NULL;
    lwgsm_sys_sem_release(&lwgsm.sem_end_sync);
    usart_ll_thread_id = NULL;
    lwgsm_sys_thread_terminate(&usart_ll_thread_id);
}

/**
 * \brief       Initialize UART for communication
 * \param[in]   baudrate: UART baudrate
 */
static void configure_uart(uint32_t baudrate)
{
    // Configure struct
    uart_config_t uart_config = {
        .baud_rate = baudrate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB
    };
    uint32_t currentBaudRate = 0;
    esp_err_t ret;
    uint32_t osStatus;
    QueueHandle_t* queue = &uart_event_ll_mbox_id;

    /* Create mbox and start thread */
    if (uart_event_ll_mbox_id == NULL) {
        osStatus = lwgsm_sys_mbox_create(&uart_event_ll_mbox_id, LWGSM_UART_QUEUE_SIZE);
        CHECK_OS_STATUS(osStatus, __func__);
    }

    LWGSM_DEBUGF(LWGSM_CFG_DBG_THREAD | LWGSM_DBG_TYPE_TRACE | LWGSM_DBG_LVL_ALL,
        "[UART INIT] Initializing UART to baudrate %d", baudrate);

    if (!initialized) {
        // Install UART driver
        ret = uart_driver_install(LWGSM_UART_NUM, LWGSM_UART_RX_BUF_SIZE, LWGSM_UART_TX_BUF_SIZE, LWGSM_UART_QUEUE_SIZE, queue, 0);
        ESP_ERROR_CHECK(ret);

        // Configure the uart driver
        ret = uart_param_config(LWGSM_UART_NUM, &uart_config);
        ESP_ERROR_CHECK(ret);

        //Set UART log level
        esp_log_level_set(TAG, LWGSM_UART_DEBUG_LEVEL);

        //Set UART pins (using UART0 default pins ie no changes.)
        ret = uart_set_pin(LWGSM_UART_NUM, LWGSM_UART_TX, LWGSM_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        ESP_ERROR_CHECK(ret);

        is_running = 1;
    } else{
        lwgsm_delay(10);
        ret = uart_get_baudrate(LWGSM_UART_NUM, &currentBaudRate);
        ESP_ERROR_CHECK(ret);
        if(uart_config.baud_rate != currentBaudRate){
            uart_set_baudrate(LWGSM_UART_NUM, uart_config.baud_rate);
        }
    }

    // Create thread to read the incoming data from UART
    if (usart_ll_thread_id == NULL) {
        osStatus = lwgsm_sys_thread_create(&usart_ll_thread_id, "LwGSM UART reading thread", usart_ll_thread, &uart_event_ll_mbox_id, LWGSM_READING_THREAD_STACK_SIZE, TASK_PRIORITY_MEDIUM);
        CHECK_OS_STATUS(osStatus, __func__);
    }
}

/**
 * \brief           Send data to GSM device, function called from GSM stack when we have data to send
 * \param[in]       data: Pointer to data to send
 * \param[in]       len: Number of bytes to send
 * \return          Number of bytes sent
 */
static size_t send_data(const void* data, size_t len)
 {
    int sent = 0;

#if LWGSM_UART_DEBUG_LEVEL == ESP_LOG_DEBUG
    int i = 0;
    if(len > 0){
        printf("Sending data (%d bytes):", len);
        while(len > i){
            printf("%c", *((const char*)data + i));
            i++;
        }
        printf("\r\n");
    }
#endif
    if(len > 0){
        sent = uart_write_bytes(LWGSM_UART_NUM, (const char*) data, len);
    } 

    return sent;                                 /* Return number of bytes actually sent to AT port */
}

/**
 * \brief           Callback function called from initialization process
 *
 * \note            This function may be called multiple times if AT baudrate is changed from application.
 *                  It is important that every configuration except AT baudrate is configured only once!
 *
 * \note            This function may be called from different threads in GSM stack when using OS.
 *                  When \ref LWGSM_CFG_INPUT_USE_PROCESS is set to 1, this function may be called from user UART thread.
 *
 * \param[in,out]   ll: Pointer to \ref lwgsm_ll_t structure to fill data for communication functions
 * \return          lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t lwgsm_ll_init(lwgsm_ll_t* ll)
{
#if !LWGSM_CFG_MEM_CUSTOM
    /* Step 1: Configure memory for dynamic allocations */
    /*
     * Create region(s) of memory.
     * If device has internal/external memory available,
     * multiple memories may be used
     */
    lwgsm_mem_region_t mem_regions[] = {
        { memory, sizeof(memory) }
    };
    if (!initialized) {
        lwgsm_mem_assignmemory(mem_regions, LWGSM_ARRAYSIZE(mem_regions));  /* Assign memory for allocations to GSM library */
    }
#endif /* !LWGSM_CFG_MEM_CUSTOM */

    /* Step 2: Set AT port send function to use when we have data to transmit */
    if (!initialized) {
        ll->send_fn = send_data;                /* Set callback function to send data */
        #if defined(LWGSM_RESET_PIN)
        ll->reset_fn = reset_device;            /* Set callback for hardware reset */
        #endif /* defined(LWGSM_RESET_PIN) */
    }

    /* Step 3: Configure AT port to be able to send/receive data to/from GSM device */
    configure_uart(ll->uart.baudrate);          /* Initialize UART for communication */

    #if defined(LWGSM_RESET_PIN)
    /* Step 4: Configure reset pin whether it is enabled*/
    gpio_config_t io_conf = {
        .intr_type = GPIO_PIN_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = LWGSM_RESET_PIN_MASK_BIT,
        .pull_down_en = 0,
        .pull_up_en = 0
    };
    // Configure GPIO with the given settings
    gpio_config(&io_conf);
    #endif

    initialized = 1;
    return lwgsmOK;
}

/**
 * \brief           Callback function to de-init low-level communication part
 * \param[in,out]   ll: Pointer to \ref lwgsm_ll_t structure to fill data for communication functions
 * \return          \ref lwgsmOK on success, member of \ref lwgsmr_t enumeration otherwise
 */
lwgsmr_t lwgsm_ll_deinit(lwgsm_ll_t* ll)
{
    esp_err_t ret;

#if !LWGSM_CFG_MEM_CUSTOM
    if (initialized) {
        lwgsm_mem_free_s((void**) &memory);
    }
#endif /* !LWGSM_CFG_MEM_CUSTOM */

    if (!initialized) {
        ll->send_fn = NULL;
        #if defined(LWGSM_RESET_PIN)
        ll->reset_fn = NULL;
        #endif /* defined(LWGSM_RESET_PIN) */
    }

#if defined(LWGSM_RESET_PIN)
    gpio_reset_pin((gpio_num_t) LWGSM_RESET_PIN);
#endif
    if (uart_event_ll_mbox_id != NULL) {
        uart_event_t event;
        uart_event_t* eventPtr = &event;
        /* Empty queue if there are messages */
        while(lwgsm_sys_mbox_get(&uart_event_ll_mbox_id, (void **)&eventPtr, 10/portTICK_PERIOD_MS) != LWGSM_SYS_TIMEOUT);
        lwgsm_sys_mbox_delete(&uart_event_ll_mbox_id);
        uart_event_ll_mbox_id = NULL;
    }
    if(uart_is_driver_installed(LWGSM_UART_NUM)){
        ret = uart_driver_delete(LWGSM_UART_NUM);
        ESP_ERROR_CHECK(ret);
    }
    initialized = 0;
    
    return lwgsmOK;
}

#if defined(LWGSM_RESET_PIN)
/**
 * \brief           Hardware reset callback
 */
static uint8_t reset_device(uint8_t state)
{
    esp_err_t ret;

    if (state) {                                /* Activate reset line */
        ret = gpio_set_level(LWGSM_RESET_PIN, 0);
        ESP_ERROR_CHECK(ret);
    } else {
        ret = gpio_set_level(LWGSM_RESET_PIN, 1);
        ESP_ERROR_CHECK(ret);
    }
    return 1;
}
#endif /* defined(LWGSM_RESET_PIN) */