/* Copyright © 2023 Georgy E. All rights reserved. */

#ifndef _MODBUS_RTU_BASE_H_
#define _MODBUS_RTU_BASE_H_


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>

#include "modbus_settings.h"


#define SPECIAL_DATA_REGISTERS_COUNT_IDX                ((uint8_t)0)
#define SPECIAL_DATA_VALUE_SIZE                         ((uint8_t)2)
#define SPECIAL_DATA_META_COUNT                         ((uint8_t)3)

#define MODBUS_ERROR_COMMAND_CODE                       ((uint8_t)0x80)


typedef enum _modbus_command_t {
    MODBUS_READ_COILS                = (uint8_t)0x01,
    MODBUS_READ_INPUT_STATUS         = (uint8_t)0x02,
    MODBUS_READ_HOLDING_REGISTERS    = (uint8_t)0x03,
    MODBUS_READ_INPUT_REGISTERS      = (uint8_t)0x04,
    MODBUS_FORCE_SINGLE_COIL         = (uint8_t)0x05,
    MODBUS_PRESET_SINGLE_REGISTER    = (uint8_t)0x06,
    MODBUS_FORCE_MULTIPLE_COILS      = (uint8_t)0x0F,
    MODBUS_PRESET_MULTIPLE_REGISTERS = (uint8_t)0x10
} modbus_command_t;


typedef enum _register_type_t {
    MODBUS_REGISTER_DISCRETE_OUTPUT_COILS           = (uint8_t)0x01,
    MODBUS_REGISTER_DISCRETE_INPUT_COILS            = (uint8_t)0x02,
    MODBUS_REGISTER_ANALOG_INPUT_REGISTERS          = (uint8_t)0x03,
    MODBUS_REGISTER_ANALOG_OUTPUT_HOLDING_REGISTERS = (uint8_t)0x04
} register_type_t;


typedef enum _modbus_error_types_t {
    MODBUS_ERROR_ILLEGAL_FUNCTION         = (uint8_t)0x01,
    MODBUS_ERROR_ILLEGAL_DATA_ADDRESS     = (uint8_t)0x02,
    MODBUS_ERROR_ILLEGAL_DATA_VALUE       = (uint8_t)0x03,
    MODBUS_ERROR_SLAVE_DEVICE_FAILURE     = (uint8_t)0x04,
    MODBUS_ERROR_ACKNOWLEDGE              = (uint8_t)0x05,
    MODBUS_ERROR_SLAVE_DEVICE_BUSY        = (uint8_t)0x06,
    MODBUS_ERROR_MEMORY_PARITY            = (uint8_t)0x08,
    MODBUS_ERROR_GATEWAY_PATH_UNAVAILABLE = (uint8_t)0x0A,
    MODBUS_ERROR_FAILED_TO_RESPOND        = (uint8_t)0x0B
} modbus_error_types_t;


#ifndef MB_MIN
#define MB_MIN(var1, var2)       ((var1 < var2) ? (var1) : (var2))
#endif
#ifndef MB_MAX
#define MB_MAX(var1, var2)       ((var1 > var2) ? (var1) : (var2))
#endif

#define MODBUS_MESSAGE_DATA_SIZE ((sizeof(uint16_t) * MB_MAX(MB_MAX(MODBUS_SLAVE_INPUT_COILS_COUNT, MODBUS_SLAVE_OUTPUT_COILS_COUNT), MB_MAX(MODBUS_SLAVE_INPUT_REGISTERS_COUNT, MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT))) + 1)
typedef struct _modbus_request_message_t {
    uint8_t  id;
    uint8_t  command;
    uint16_t register_addr;
    uint16_t crc;
} modbus_request_message_t;


typedef struct _modbus_response_message_t {
    uint8_t  id;
    uint8_t  command;
    uint16_t register_addr;
    uint8_t  data_len;
    uint16_t crc;
} modbus_response_message_t;


#define MODBUS_RESPONSE_MESSAGE_SIZE  ((uint16_t)(MB_MAX(sizeof(struct _modbus_response_message_t), sizeof(struct _modbus_request_message_t)) + MODBUS_MESSAGE_DATA_SIZE))


uint16_t modbus_crc16(const uint8_t* data, uint16_t len);


#ifdef __cplusplus
}
#endif


#endif
