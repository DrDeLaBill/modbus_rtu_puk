/*
 *
 * Copyright © 2023 Georgy E. All rights reserved.
 *
 */
#include "modbus_rtu_slave.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "modbus_rtu_base.h"


#if MODBUS_SLAVE_OUTPUT_COILS_COUNT
bool mb_discrete_output_coils[MODBUS_SLAVE_OUTPUT_COILS_COUNT] = { 0 };
#endif
#if MODBUS_SLAVE_INPUT_COILS_COUNT
bool mb_discrete_input_coils[MODBUS_SLAVE_INPUT_COILS_COUNT] = { 0 };
#endif
#if MODBUS_SLAVE_INPUT_REGISTERS_COUNT
uint16_t mb_analog_input_registers[MODBUS_SLAVE_INPUT_REGISTERS_COUNT] = { 0 };
#endif
#if MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT
uint16_t mb_analog_output_holding_registers[MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT] = { 0 };
#endif


void _mb_sl_do_internal_error(void);

void _mb_sl_fsm_request_slave_id(uint8_t byte);
void _mb_sl_fsm_request_command(uint8_t byte);
void _mb_sl_fsm_request_register_addr(uint8_t byte);
void _mb_sl_fsm_request_special_data(uint8_t byte);
void _mb_sl_fsm_request_crc(uint8_t byte);

void _mb_sl_request_proccess(void);
void _mb_sl_write_single_register(void);
void _mb_sl_write_multiple_registers(void);
void _mb_sl_make_error_response(modbus_error_types_t error_type);
void _mb_sl_send_response(void);

bool _mb_sl_is_read_command(void);
bool _mb_sl_is_write_single_reg_command(void);
bool _mb_sl_is_write_multiple_reg_command(void);
bool _mb_sl_is_recieved_own_slave_id(void);
bool _mb_sl_check_available_request_command(void);
bool _mb_sl_check_request_register_addr(void);
bool _mb_sl_check_request_registers_count(void);
uint16_t _mb_sl_get_special_data_first_value(void);
uint16_t _mb_sl_get_needed_registers_count(void);
uint16_t _mb_sl_get_registers_count(register_type_t register_type);


void _mb_sl_make_read_response(void);
void _mb_sl_make_write_single_response(void);
void _mb_sl_make_write_multiple_response(void);

register_type_t _mb_sl_get_request_register_type();


modbus_slave_state_t mb_slave_state = {
    .slave_id = 0x00, 
    .response_data_handler = NULL,
    .internal_error_handler = NULL,
    .request_byte_handler = _mb_sl_fsm_request_slave_id,
    .data_req = {0},
    .data_resp = {0},
    .data_handler_counter = 0,
    .is_error_response = false,

    .req_data_bytes_idx = 0,
    .req_data_bytes = {0}
};


void modbus_slave_set_response_data_handler(void (*response_data_handler) (uint8_t*, uint32_t))
{
    if (response_data_handler != NULL) {
        mb_slave_state.response_data_handler = response_data_handler;
    }
}

void modbus_slave_set_internal_error_handler(void (*request_error_handler) (void))
{

    if (request_error_handler != NULL) {
        mb_slave_state.internal_error_handler = request_error_handler;
    }
}

void modbus_slave_recieve_data_byte(uint8_t byte)
{
    if (mb_slave_state.request_byte_handler != NULL) {
        mb_slave_state.req_data_bytes[mb_slave_state.req_data_bytes_idx++] = byte;

        mb_slave_state.data_handler_counter++;
        mb_slave_state.request_byte_handler(byte);
    } else {
        _mb_sl_do_internal_error();
        modbus_slave_clear_data();
    }
}

void modbus_slave_set_slave_id(uint8_t new_slave_id)
{
    mb_slave_state.slave_id = new_slave_id;
    modbus_slave_clear_data();
}

void modbus_slave_timeout(void)
{
    _mb_sl_do_internal_error();
    modbus_slave_clear_data();
}

uint16_t modbus_slave_get_register_value(register_type_t register_type, uint16_t register_id)
{
    if (register_id >= _mb_sl_get_registers_count(register_type)) {
        return 0;
    }
#if MODBUS_SLAVE_OUTPUT_COILS_COUNT
    if (register_type == MODBUS_REGISTER_DISCRETE_OUTPUT_COILS) {
        return mb_discrete_output_coils[register_id];
    }
#endif
#if MODBUS_SLAVE_INPUT_COILS_COUNT
    if (register_type == MODBUS_REGISTER_DISCRETE_INPUT_COILS) {
        return mb_discrete_input_coils[register_id];
    }
#endif
#if MODBUS_SLAVE_INPUT_REGISTERS_COUNT
    if (register_type == MODBUS_REGISTER_ANALOG_INPUT_REGISTERS) {
        return mb_analog_input_registers[register_id];
    }
#endif
#if MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT
    if (register_type == MODBUS_REGISTER_ANALOG_OUTPUT_HOLDING_REGISTERS) {
        return mb_analog_output_holding_registers[register_id];
    }
#endif
    return 0;
}

void modbus_slave_set_register_value(register_type_t register_type, uint16_t register_id, uint16_t value)
{
    if (register_id >= _mb_sl_get_registers_count(register_type)) {
        _mb_sl_do_internal_error();
        return;
    }
#if MODBUS_SLAVE_OUTPUT_COILS_COUNT
    if (register_type == MODBUS_REGISTER_DISCRETE_OUTPUT_COILS) {
        mb_discrete_output_coils[register_id] = value > 0;
    }
#endif
#if MODBUS_SLAVE_INPUT_COILS_COUNT
    if (register_type == MODBUS_REGISTER_DISCRETE_INPUT_COILS) {
        mb_discrete_input_coils[register_id] = value > 0;
    }
#endif
#if MODBUS_SLAVE_INPUT_REGISTERS_COUNT
    if (register_type == MODBUS_REGISTER_ANALOG_INPUT_REGISTERS) {
        mb_analog_input_registers[register_id] = value;
    }
#endif
#if MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT
    if (register_type == MODBUS_REGISTER_ANALOG_OUTPUT_HOLDING_REGISTERS) {
        mb_analog_output_holding_registers[register_id] = value;
    }
#endif
}

void _mb_sl_request_proccess(void)
{
    if (!_mb_sl_is_recieved_own_slave_id()) {
        modbus_slave_clear_data();
        return;
    }

    if (mb_slave_state.response_data_handler == NULL) {
        _mb_sl_do_internal_error();
        return;
    }

    mb_slave_state.data_resp.id = mb_slave_state.data_req.id;

    /* CHECK ERRORS BEGIN */
    if (!_mb_sl_check_available_request_command()) {
        _mb_sl_make_error_response(MODBUS_ERROR_ILLEGAL_FUNCTION);
        goto do_send;
    }

    if (!_mb_sl_check_request_register_addr()) {
        _mb_sl_make_error_response(MODBUS_ERROR_ILLEGAL_DATA_ADDRESS);
        goto do_send;
    }

    if (!_mb_sl_check_request_registers_count()) {
        _mb_sl_make_error_response(MODBUS_ERROR_ILLEGAL_DATA_VALUE);
        goto do_send;
    }
    if (mb_slave_state.is_error_response) {
        goto do_reset;
    }
    /* CHECK ERRORS END */

    mb_slave_state.data_resp.command = mb_slave_state.data_req.command;

    /* WRITE REGISTERS BEGIN */
    if (_mb_sl_is_write_single_reg_command()) {
        _mb_sl_write_single_register();
    }

    if (_mb_sl_is_write_multiple_reg_command()) {
        _mb_sl_write_multiple_registers();
    }
    /* WRITE REGISTERS END */

    /* MAKE RESPONSE DATA BEGIN */
    if (_mb_sl_is_read_command()) {
        _mb_sl_make_read_response();
    }
    if (_mb_sl_is_write_single_reg_command()) {
        _mb_sl_make_write_single_response();
    }
    if (_mb_sl_is_write_multiple_reg_command()) {
        _mb_sl_make_write_multiple_response();
    }
    /* MAKE RESPONSE DATA END */

    goto do_send;

do_send:
    _mb_sl_send_response();

    goto do_reset;

do_reset:
    modbus_slave_clear_data();
}

void _mb_sl_make_error_response(modbus_error_types_t error_type)
{
    mb_slave_state.is_error_response = true;
    mb_slave_state.data_resp.command = mb_slave_state.data_req.command | MODBUS_ERROR_COMMAND_CODE;
    mb_slave_state.data_resp.data_resp[0] = error_type;
}

void _mb_sl_send_response(void)
{
    if (mb_slave_state.response_data_handler == NULL) {
        _mb_sl_do_internal_error();
        return;
    }

    uint8_t data[MODBUS_RESPONSE_MESSAGE_SIZE] = { 0 };
    uint16_t counter = 0;
    data[counter++] = mb_slave_state.data_resp.id;
    data[counter++] = mb_slave_state.data_resp.command;
    if (mb_slave_state.is_error_response) {
        data[counter++] = mb_slave_state.data_resp.data_resp[0];
    } else {
        memcpy(data + counter, mb_slave_state.data_resp.data_resp, mb_slave_state.data_resp.data_len);
        counter += mb_slave_state.data_resp.data_len;
    }
    uint16_t crc = modbus_crc16(data, counter);
    data[counter++] = crc & 0xFF;
    data[counter++] = crc >> 8;

    mb_slave_state.response_data_handler(data, counter);
}

void _mb_sl_write_single_register(void)
{
#if MODBUS_SLAVE_OUTPUT_COILS_COUNT
    if (mb_slave_state.data_req.command == MODBUS_FORCE_SINGLE_COIL) {
        mb_discrete_output_coils[mb_slave_state.data_req.register_addr] = _mb_sl_get_special_data_first_value() > 0;
    }
#endif
#if MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT
    if (mb_slave_state.data_req.command == MODBUS_PRESET_SINGLE_REGISTER) {
        mb_analog_output_holding_registers[mb_slave_state.data_req.register_addr] = _mb_sl_get_special_data_first_value();
    }
#endif
}

void _mb_sl_write_multiple_registers(void)
{
    uint16_t count = _mb_sl_get_special_data_first_value();

    if (mb_slave_state.data_req.register_addr >= _mb_sl_get_registers_count(_mb_sl_get_request_register_type())) {
        _mb_sl_do_internal_error();
        return;
    }
    
    if (SPECIAL_DATA_META_COUNT + count - 1 > _mb_sl_get_registers_count(_mb_sl_get_request_register_type())) {
        count = _mb_sl_get_registers_count(_mb_sl_get_request_register_type()) - SPECIAL_DATA_META_COUNT;
    }

#if MODBUS_SLAVE_OUTPUT_COILS_COUNT
    if (mb_slave_state.data_req.command == MODBUS_FORCE_MULTIPLE_COILS) {
        for (uint8_t i = 0; i < count; i++) {
            bool value = ((mb_slave_state.data_req.special_data[SPECIAL_DATA_META_COUNT] >> i) & 0x01);
            mb_discrete_output_coils[mb_slave_state.data_req.register_addr + i] = value > 0;
        }
    }
#endif
#if MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT
    if (mb_slave_state.data_req.command == MODBUS_PRESET_MULTIPLE_REGISTERS) {
        for (uint8_t i = 0; i < count * 2; i += 2) {
            uint16_t value = (uint16_t)mb_slave_state.data_req.special_data[SPECIAL_DATA_META_COUNT + i] << 8 |
                (uint16_t)mb_slave_state.data_req.special_data[SPECIAL_DATA_META_COUNT + i + 1];
            mb_analog_output_holding_registers[mb_slave_state.data_req.register_addr + i / 2] = value;
        }
    }
#endif
}

void modbus_slave_clear_data(void)
{
    memset((uint8_t*)&mb_slave_state.data_req, 0, sizeof(mb_slave_state.data_req));
    memset((uint8_t*)&mb_slave_state.data_resp, 0, sizeof(mb_slave_state.data_resp));
    memset(mb_slave_state.req_data_bytes, 0, sizeof(mb_slave_state.req_data_bytes));
    mb_slave_state.req_data_bytes_idx = 0;

    mb_slave_state.is_error_response    = false;
    mb_slave_state.data_handler_counter = 0;
    mb_slave_state.request_byte_handler = _mb_sl_fsm_request_slave_id;
}

void _mb_sl_do_internal_error(void)
{
    if (mb_slave_state.internal_error_handler != NULL) {
        mb_slave_state.internal_error_handler();
    }
    modbus_slave_clear_data();
}

void _mb_sl_make_read_response(void)
{
    uint8_t command      = mb_slave_state.data_req.command;
    uint8_t req_data_len = _mb_sl_get_special_data_first_value();

    uint8_t counter      = 0;
    while (counter < req_data_len) {
        uint16_t cur_idx = mb_slave_state.data_req.register_addr + counter;
#if MODBUS_SLAVE_OUTPUT_COILS_COUNT
        if (command == MODBUS_READ_COILS) {
            mb_slave_state.data_resp.data_resp[1 + (counter / 8)] |= (mb_discrete_output_coils[cur_idx] << (cur_idx % 8));
        }
#endif
#if MODBUS_SLAVE_INPUT_COILS_COUNT
        if (command == MODBUS_READ_INPUT_STATUS) {
            mb_slave_state.data_resp.data_resp[1 + (counter / 8)] |= (mb_discrete_input_coils[cur_idx] << (cur_idx % 8));
        }
#endif
#if MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT
        if (command == MODBUS_READ_HOLDING_REGISTERS) {
            mb_slave_state.data_resp.data_resp[1 + counter * 2] = mb_analog_output_holding_registers[cur_idx] >> 8;
            mb_slave_state.data_resp.data_resp[1 + counter * 2 + 1] = mb_analog_output_holding_registers[cur_idx];
        }
#endif
#if MODBUS_SLAVE_INPUT_REGISTERS_COUNT
        if (command == MODBUS_READ_INPUT_REGISTERS) {
            mb_slave_state.data_resp.data_resp[1 + counter * 2] = mb_analog_input_registers[cur_idx] >> 8;
            mb_slave_state.data_resp.data_resp[1 + counter * 2 + 1] = mb_analog_input_registers[cur_idx];
        }
#endif
        counter++;
    }

    uint8_t resp_data_len = 0;
    if (command == MODBUS_READ_HOLDING_REGISTERS || command == MODBUS_READ_INPUT_REGISTERS) {
        resp_data_len = counter * 2;
    }
    else {
        resp_data_len = counter;
    }
    mb_slave_state.data_resp.data_len     = 1 + resp_data_len;
    mb_slave_state.data_resp.data_resp[0] = resp_data_len;
}

void _mb_sl_make_write_single_response(void)
{
    uint8_t counter = 0;

    uint8_t command = mb_slave_state.data_req.command;
    uint16_t reg_addr = mb_slave_state.data_req.register_addr;

    mb_slave_state.data_resp.data_resp[counter++] = reg_addr >> 8;
    mb_slave_state.data_resp.data_resp[counter++] = reg_addr;

#if MODBUS_SLAVE_OUTPUT_COILS_COUNT
    if (mb_slave_state.data_req.command == MODBUS_FORCE_SINGLE_COIL) {
        mb_slave_state.data_resp.data_resp[counter++] = mb_discrete_output_coils[reg_addr] >> 8;
        mb_slave_state.data_resp.data_resp[counter++] = mb_discrete_output_coils[reg_addr];
    }
#endif
#if MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT
    if (mb_slave_state.data_req.command == MODBUS_PRESET_SINGLE_REGISTER) {
        mb_slave_state.data_resp.data_resp[counter++] = mb_analog_output_holding_registers[reg_addr] >> 8;
        mb_slave_state.data_resp.data_resp[counter++] = mb_analog_output_holding_registers[reg_addr];
    }
#endif

    mb_slave_state.data_resp.data_len = counter;
}

void _mb_sl_make_write_multiple_response(void)
{
    uint8_t counter = 0;

    uint16_t written_count = _mb_sl_get_special_data_first_value();

    uint16_t reg_addr = mb_slave_state.data_req.register_addr;

    mb_slave_state.data_resp.data_resp[counter++] = reg_addr >> 8;
    mb_slave_state.data_resp.data_resp[counter++] = reg_addr;
    mb_slave_state.data_resp.data_resp[counter++] = written_count >> 8;
    mb_slave_state.data_resp.data_resp[counter++] = written_count;

    mb_slave_state.data_resp.data_len = counter;
}

void _mb_sl_fsm_request_slave_id(uint8_t byte)
{
    mb_slave_state.data_req.id = byte;
    mb_slave_state.data_handler_counter = 0;
    mb_slave_state.request_byte_handler = _mb_sl_fsm_request_command;
}

void _mb_sl_fsm_request_command(uint8_t byte)
{
    mb_slave_state.data_req.command = byte;
    mb_slave_state.data_handler_counter = 0;
    mb_slave_state.request_byte_handler = _mb_sl_fsm_request_register_addr;

    if (!_mb_sl_is_recieved_own_slave_id()) {
        return;
    }

    if (!_mb_sl_check_available_request_command()) {
        _mb_sl_request_proccess();
    }
}

void _mb_sl_fsm_request_register_addr(uint8_t byte)
{
    mb_slave_state.data_req.register_addr <<= 8;
    mb_slave_state.data_req.register_addr |= byte;
    if (mb_slave_state.data_handler_counter == sizeof(mb_slave_state.data_req.register_addr)) {
        mb_slave_state.data_handler_counter = 0;
        mb_slave_state.request_byte_handler = _mb_sl_fsm_request_special_data;
    }

    if (!_mb_sl_is_recieved_own_slave_id()) {
        return;
    }

    if (mb_slave_state.request_byte_handler == _mb_sl_fsm_request_register_addr) {
        return;
    }

    if (!_mb_sl_check_request_register_addr()) {
        _mb_sl_request_proccess();
    }
}

void _mb_sl_fsm_request_special_data(uint8_t byte)
{
    uint16_t needed_count_bytes = 0;
    uint16_t needed_count       = 0;
    uint16_t cur_count          = mb_slave_state.data_handler_counter;

    if (mb_slave_state.data_req.register_addr + _mb_sl_get_needed_registers_count() >= _mb_sl_get_registers_count(_mb_sl_get_request_register_type())) {
        goto do_count_special_data;
    }

    if (mb_slave_state.data_handler_counter > sizeof(mb_slave_state.data_req.special_data)) {
        goto do_count_special_data;
    }

    mb_slave_state.data_req.special_data[mb_slave_state.data_handler_counter - 1] = byte;


do_count_special_data:
    needed_count_bytes = 0;
    needed_count       = 0;

    if (_mb_sl_is_read_command() || _mb_sl_is_write_single_reg_command()) {
        needed_count_bytes = SPECIAL_DATA_VALUE_SIZE;
        needed_count       = SPECIAL_DATA_VALUE_SIZE;
    }

    if (_mb_sl_is_write_multiple_reg_command()) {
        needed_count_bytes = SPECIAL_DATA_META_COUNT + mb_slave_state.data_req.special_data[SPECIAL_DATA_META_COUNT - 1];
        needed_count       = SPECIAL_DATA_VALUE_SIZE + 1;
    }

    if (mb_slave_state.data_handler_counter == needed_count_bytes) {
        mb_slave_state.data_handler_counter = 0;
        mb_slave_state.request_byte_handler = _mb_sl_fsm_request_crc;
    }

    if (!_mb_sl_is_recieved_own_slave_id()) {
        return;
    }

    if (cur_count <= needed_count) {
        return;
    }

    if (!_mb_sl_check_request_registers_count()) {
        _mb_sl_request_proccess();
    }
}

void _mb_sl_fsm_request_crc(uint8_t byte)
{
    mb_slave_state.data_req.crc >>= 8;
    mb_slave_state.data_req.crc |= (byte << 8);

    if (mb_slave_state.data_handler_counter < sizeof(mb_slave_state.data_req.crc)) {
        return;
    }

    if (!_mb_sl_is_recieved_own_slave_id()) {
        mb_slave_state.data_handler_counter = 0;
        mb_slave_state.request_byte_handler = _mb_sl_fsm_request_slave_id;
        goto do_reset_data;
    }
       
    uint16_t crc = modbus_crc16(mb_slave_state.req_data_bytes, mb_slave_state.req_data_bytes_idx - sizeof(uint16_t));
    if (mb_slave_state.data_req.crc != crc) {
        mb_slave_state.is_error_response = true;
    }

    _mb_sl_request_proccess();

    goto do_reset_data;

do_reset_data:
    modbus_slave_clear_data();
}

bool _mb_sl_is_read_command(void)
{
    return mb_slave_state.data_req.command == MODBUS_READ_COILS || mb_slave_state.data_req.command == MODBUS_READ_HOLDING_REGISTERS || mb_slave_state.data_req.command == MODBUS_READ_INPUT_REGISTERS || mb_slave_state.data_req.command == MODBUS_READ_INPUT_STATUS;
}

bool _mb_sl_is_write_single_reg_command(void)
{
    return mb_slave_state.data_req.command == MODBUS_FORCE_SINGLE_COIL || mb_slave_state.data_req.command == MODBUS_PRESET_SINGLE_REGISTER;
}

bool _mb_sl_is_write_multiple_reg_command(void)
{
    return mb_slave_state.data_req.command == MODBUS_FORCE_MULTIPLE_COILS || mb_slave_state.data_req.command == MODBUS_PRESET_MULTIPLE_REGISTERS;
}

bool _mb_sl_is_recieved_own_slave_id(void)
{
    return mb_slave_state.slave_id == mb_slave_state.data_req.id;
}

bool _mb_sl_check_available_request_command(void)
{
    return false
#if MODBUS_SLAVE_OUTPUT_COILS_COUNT
        || mb_slave_state.data_req.command == MODBUS_READ_COILS
        || mb_slave_state.data_req.command == MODBUS_FORCE_SINGLE_COIL
        || mb_slave_state.data_req.command == MODBUS_FORCE_MULTIPLE_COILS
#endif
#if MODBUS_SLAVE_INPUT_COILS_COUNT
        || mb_slave_state.data_req.command == MODBUS_READ_INPUT_STATUS
#endif
#if MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT
        || mb_slave_state.data_req.command == MODBUS_READ_HOLDING_REGISTERS
        || mb_slave_state.data_req.command == MODBUS_PRESET_SINGLE_REGISTER
        || mb_slave_state.data_req.command == MODBUS_PRESET_MULTIPLE_REGISTERS
#endif
#if MODBUS_SLAVE_INPUT_REGISTERS_COUNT
        || mb_slave_state.data_req.command == MODBUS_READ_INPUT_REGISTERS
#endif
        ;
}

bool _mb_sl_check_request_register_addr(void)
{
    return mb_slave_state.data_req.register_addr < (uint16_t)_mb_sl_get_registers_count(_mb_sl_get_request_register_type());
}

bool _mb_sl_check_request_registers_count(void)
{
    uint16_t reg_count = _mb_sl_get_needed_registers_count();
    uint8_t data_count = mb_slave_state.data_req.special_data[SPECIAL_DATA_META_COUNT - 1];
    uint16_t reg_addr  = mb_slave_state.data_req.register_addr;

    return reg_count > 0
        && reg_addr + reg_count <= _mb_sl_get_registers_count(_mb_sl_get_request_register_type())
        && SPECIAL_DATA_META_COUNT + data_count <= sizeof(mb_slave_state.data_req.special_data);
}

uint16_t _mb_sl_get_special_data_first_value(void)
{
    uint8_t regh = mb_slave_state.data_req.special_data[SPECIAL_DATA_REGISTERS_COUNT_IDX];
    uint8_t regl = mb_slave_state.data_req.special_data[SPECIAL_DATA_REGISTERS_COUNT_IDX + 1];
    return (uint16_t)(((uint16_t)regh) << 8) + (uint16_t)regl;
}

uint16_t _mb_sl_get_needed_registers_count(void)
{
#if MODBUS_SLAVE_OUTPUT_COILS_COUNT || MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT
    if (_mb_sl_is_write_single_reg_command()) {
        return 1;
    }
#endif
#if MODBUS_SLAVE_INPUT_COILS_COUNT || MODBUS_SLAVE_OUTPUT_COILS_COUNT || MODBUS_SLAVE_INPUT_REGISTERS_COUNT || MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT
    if (_mb_sl_is_read_command() || _mb_sl_is_write_multiple_reg_command()) {
        return _mb_sl_get_special_data_first_value();
    }
#endif
    return 0;
}

uint16_t _mb_sl_get_registers_count(register_type_t register_type)
{
#if MODBUS_SLAVE_INPUT_COILS_COUNT
    if (register_type == MODBUS_REGISTER_DISCRETE_INPUT_COILS) {
        return MODBUS_SLAVE_INPUT_COILS_COUNT;
    }
#endif
#if MODBUS_SLAVE_OUTPUT_COILS_COUNT
    if (register_type == MODBUS_REGISTER_DISCRETE_OUTPUT_COILS) {
        return MODBUS_SLAVE_OUTPUT_COILS_COUNT;
    }
#endif
#if MODBUS_SLAVE_INPUT_REGISTERS_COUNT
    if (register_type == MODBUS_REGISTER_ANALOG_INPUT_REGISTERS) {
        return MODBUS_SLAVE_INPUT_REGISTERS_COUNT;
    }
#endif
#if MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT
    if (register_type == MODBUS_REGISTER_ANALOG_OUTPUT_HOLDING_REGISTERS) {
        return MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT;
    }
#endif
    return 0;
}

register_type_t _mb_sl_get_request_register_type()
{
    register_type_t  register_type = 0;
    modbus_command_t command = mb_slave_state.data_req.command;
#if MODBUS_SLAVE_INPUT_COILS_COUNT
    if (command == MODBUS_READ_INPUT_STATUS) {
        register_type = MODBUS_REGISTER_DISCRETE_INPUT_COILS;
    }
#endif
#if MODBUS_SLAVE_OUTPUT_COILS_COUNT
    if (command == MODBUS_FORCE_MULTIPLE_COILS || command == MODBUS_FORCE_SINGLE_COIL || command == MODBUS_READ_COILS) {
        register_type = MODBUS_REGISTER_DISCRETE_OUTPUT_COILS;
    }
#endif
#if MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT
    if (command == MODBUS_PRESET_MULTIPLE_REGISTERS || command == MODBUS_PRESET_SINGLE_REGISTER || command == MODBUS_READ_HOLDING_REGISTERS) {
        register_type = MODBUS_REGISTER_ANALOG_OUTPUT_HOLDING_REGISTERS;
    }
#endif
#if MODBUS_SLAVE_INPUT_REGISTERS_COUNT
    if (command == MODBUS_READ_INPUT_REGISTERS) {
        register_type = MODBUS_REGISTER_ANALOG_INPUT_REGISTERS;
    }
#endif
    return register_type;
}