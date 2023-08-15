/*
 *
 * Copyright © 2023 Georgy E. All rights reserved.
 *
 */
#ifndef INC_MODBUS_RTU_MASTER_H_
#define INC_MODBUS_RTU_MASTER_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>

#include "modbus_rtu_base.h"


    typedef enum _modbus_error_response_t {
        MODBUS_NO_ERROR       = (uint8_t)0x00,
        MODBUS_ERROR_COMMAND  = (uint8_t)0x01,
        MODBUS_ERROR_REG_ADDR = (uint8_t)0x02,
        MODBUS_ERROR_DATA     = (uint8_t)0x03,
        MODBUS_ERROR_CRC      = (uint8_t)0x04
    } modbus_error_response_t;


    typedef struct _modbus_response_t {
        modbus_error_response_t status;
        uint8_t slave_id;
        modbus_command_t command;
        uint16_t response[MODBUS_MESSAGE_DATA_SIZE];
    } modbus_response_t;


    typedef struct _modbus_master_state_t {
        void (*request_data_sender) (uint8_t*, uint8_t);
        void (*response_byte_handler) (uint8_t);
        void (*response_packet_handler) (modbus_response_t*);
        void (*internal_error_handler) (void);
        uint8_t data_counter;
        modbus_request_message_t data_req;
        modbus_response_message_t data_resp;

        uint16_t response_bytes_len;
        uint8_t response_bytes[MODBUS_MESSAGE_DATA_SIZE];
    } modbus_master_state_t;


    void modbus_master_set_request_data_sender(void (*request_data_sender) (uint8_t*, uint8_t));
    void modbus_master_set_internal_error_handler(void (*response_error_handler) (void));
    void modbus_master_set_response_packet_handler(void (*response_packet_handler) (modbus_response_t*));

    void modbus_master_recieve_data_byte(uint8_t byte);
    void modbus_master_timeout(void);

    void modbus_master_read_coils(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count);
    void modbus_master_read_input_status(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count);
    void modbus_master_read_holding_registers(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count);
    void modbus_master_read_input_registers(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count);
    void modbus_master_force_single_coil(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_val);
    void modbus_master_preset_single_register(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_val);
    void modbus_master_force_multiple_coils(uint8_t slave_id, uint16_t reg_addr, const bool* data, uint16_t reg_count);
    void modbus_master_preset_multiple_registers(uint8_t slave_id, uint16_t reg_addr, const uint16_t* data, uint16_t reg_count);


#ifdef __cplusplus
}
#endif


#endif /* INC_MODBUS_RTU_MASTER_H_ */
