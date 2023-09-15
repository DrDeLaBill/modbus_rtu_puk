/*
 *
 * Copyright © 2023 Georgy E. All rights reserved.
 *
 */
#ifndef _MODBUS_PROTOCOL_H_
#define _MODBUS_PROTOCOL_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>

#include "modbus_rtu_base.h"


typedef struct _modbus_slave_state_t {
    uint8_t slave_id;
    void (*response_data_handler) (uint8_t*, uint32_t);
    void (*request_byte_handler) (uint8_t);
    void (*internal_error_handler) (void);
    modbus_request_message_t data_req;
    uint8_t data_handler_counter;
    modbus_response_message_t data_resp;
    bool is_error_response;
    
    uint16_t req_data_bytes_idx;
    uint8_t req_data_bytes[MODBUS_RESPONSE_MESSAGE_SIZE];
} modbus_slave_state_t;


void modbus_slave_set_response_data_handler(void (*response_data_handler) (uint8_t*, uint32_t));
void modbus_slave_set_internal_error_handler(void (*request_error_handler) (void));
void modbus_slave_recieve_data_byte(uint8_t byte);
void modbus_slave_set_slave_id(uint8_t new_slave_id);
void modbus_slave_timeout(void);
void modbus_slave_clear_data(void);

uint16_t modbus_slave_get_register_value(register_type_t register_type, uint16_t register_id);
void     modbus_slave_set_register_value(register_type_t register_type, uint16_t register_id, uint16_t value);


#ifdef __cplusplus
}
#endif


#endif