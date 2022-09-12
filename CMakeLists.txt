set(target lwgsm)

set(SRC_FILES   ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/api/${target}_netconn.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/api/${target}_network_api.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/apps/mqtt/${target}_mqtt_client_api.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/apps/mqtt/${target}_mqtt_client_evt.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/apps/mqtt/${target}_mqtt_client.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_buff.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_call.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_conn.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_debug.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_device_info.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_evt.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_ftp.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_http.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_input.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_int.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_mem.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_network.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_operator.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_parser.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_pbuf.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_phonebook.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_ping.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_sim.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_sms.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_threads.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_timeout.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_unicode.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_ussd.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}_utils.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/${target}/${target}.c
                # LIBRARY PORT
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/system/${target}_ll_esp32.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/system/${target}_sys_freertos_os.c
                # SNIPPETS
                ${CMAKE_CURRENT_SOURCE_DIR}/snippets/sim_manager.c
                ${CMAKE_CURRENT_SOURCE_DIR}/snippets/device_info.c
                ${CMAKE_CURRENT_SOURCE_DIR}/snippets/netconn_client.c
                ${CMAKE_CURRENT_SOURCE_DIR}/snippets/network_utils.c
                ${CMAKE_CURRENT_SOURCE_DIR}/snippets/client.c
                ${CMAKE_CURRENT_SOURCE_DIR}/snippets/mqtt_client_api.c
                ${CMAKE_CURRENT_SOURCE_DIR}/snippets/operator_utils.c
                # ${CMAKE_CURRENT_SOURCE_DIR}/snippets/call_sms.c
                # ${CMAKE_CURRENT_SOURCE_DIR}/snippets/call.c
                # ${CMAKE_CURRENT_SOURCE_DIR}/snippets/sms_send_receive_thread.c
                # ${CMAKE_CURRENT_SOURCE_DIR}/snippets/sms_send_receive.c
    )

set(HEADER_FILES 
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/include
                ${CMAKE_CURRENT_SOURCE_DIR}/${target}/src/include/system/port/esp32
                ${CMAKE_CURRENT_SOURCE_DIR}/snippets/include
                ${LWGSM_OPTS_DIR}
    )

idf_component_register(
    SRCS
        "${SRC_FILES}"
    INCLUDE_DIRS
        "${HEADER_FILES}"
    PRIV_REQUIRES
        log
)