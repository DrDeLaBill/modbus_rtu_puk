/*
 *
 * Copyright © 2023 Georgy E. All rights reserved.
 *
 */
#include "modbus_rtu_master.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

#include "modbus_rtu_base.h"


void _mb_ms_send_simple_message(uint8_t slave_id, uint8_t command, uint16_t reg_addr, uint16_t spec_data);

void _mb_ms_do_internal_error(void);
void _mb_ms_reset_data(void);

void _mb_ms_fsm_response_slave_id(uint8_t byte);
void _mb_ms_fsm_response_command(uint8_t byte);
void _mb_ms_fsm_response_data_len(uint8_t byte);
void _mb_ms_fsm_response_register_addr(uint8_t byte);
void _mb_ms_fsm_response_special_data(uint8_t byte);
void _mb_ms_fsm_response_error(uint8_t byte);
void _mb_ms_fsm_response_crc(uint8_t byte);

void _mb_ms_response_proccess(void);

uint16_t _mb_ms_get_response_bytes_count(void);

bool _mb_ms_is_read_command(void);
bool _mb_ms_is_write_single_reg_command(void);
bool _mb_ms_is_write_multiple_reg_command(void);
bool _mb_ms_is_read_discrete_reg_command(void);
bool _mb_ms_is_read_analog_reg_command(void);
bool _mb_ms_is_recieved_needed_slave_id(void);
bool _mb_ms_check_response_command(void);
bool _mb_ms_check_response_crc(void);

void _mb_ms_make_read_discrete_packet(modbus_response_t* packet);
void _mb_ms_make_read_analog_packet(modbus_response_t* packet);
void _mb_ms_make_write_packet(modbus_response_t* packet);


modbus_master_state_t mb_master_state = {
	.data_counter = 0,
	.request_data_sender = NULL,
	.response_packet_handler = NULL,
	.internal_error_handler = NULL,
	.response_byte_handler = _mb_ms_fsm_response_slave_id,
	.data_req = {0},
	.data_resp = {0},

	.response_bytes_len = 0,
	.response_bytes = {0}
};


void modbus_master_set_request_data_sender(void (*request_data_sender) (uint8_t*, uint8_t))
{
	if (request_data_sender != NULL) {
		mb_master_state.request_data_sender = request_data_sender;
	}
}

void modbus_master_set_internal_error_handler(void (*response_error_handler) (void))
{
	if (response_error_handler != NULL) {
		mb_master_state.internal_error_handler = response_error_handler;
	}
}

void modbus_master_set_response_packet_handler(void (*response_packet_handler) (modbus_response_t*))
{
	if (response_packet_handler == NULL) {
		_mb_ms_do_internal_error();
		return;
	}

	mb_master_state.response_packet_handler = response_packet_handler;
}

void modbus_master_recieve_data_byte(uint8_t byte)
{
	if (mb_master_state.response_bytes_len > sizeof(mb_master_state.response_bytes)) {
		_mb_ms_do_internal_error();
		_mb_ms_reset_data();
	}

	mb_master_state.response_bytes[mb_master_state.response_bytes_len++] = byte;

	if (mb_master_state.response_byte_handler != NULL) {
		mb_master_state.data_counter++;
		mb_master_state.response_byte_handler(byte);
	}
	else {
		_mb_ms_do_internal_error();
		_mb_ms_reset_data();
	}
}

void modbus_master_timeout(void)
{
	_mb_ms_do_internal_error();
	_mb_ms_reset_data();
}

void modbus_master_read_coils(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count)
{
	_mb_ms_send_simple_message(slave_id, MODBUS_READ_COILS, reg_addr, reg_count);
}

void modbus_master_read_input_status(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count)
{
	_mb_ms_send_simple_message(slave_id, MODBUS_READ_INPUT_STATUS, reg_addr, reg_count);
}

void modbus_master_read_holding_registers(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count)
{
	_mb_ms_send_simple_message(slave_id, MODBUS_READ_HOLDING_REGISTERS, reg_addr, reg_count);
}

void modbus_master_read_input_registers(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count)
{
	_mb_ms_send_simple_message(slave_id, MODBUS_READ_INPUT_REGISTERS, reg_addr, reg_count);
}

void modbus_master_force_single_coil(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_val)
{
	_mb_ms_send_simple_message(slave_id, MODBUS_FORCE_SINGLE_COIL, reg_addr, reg_val);
}

void modbus_master_preset_single_register(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_val)
{
	_mb_ms_send_simple_message(slave_id, MODBUS_PRESET_SINGLE_REGISTER, reg_addr, reg_val);
}

void modbus_master_force_multiple_coils(uint8_t slave_id, uint16_t reg_addr, const bool* data, uint16_t reg_count)
{
	if (reg_count == 0 || data == NULL) {
		_mb_ms_do_internal_error();
		return;
	}

	uint8_t request[MODBUS_MESSAGE_DATA_SIZE] = { 0 };
	uint16_t counter = 0;

	mb_master_state.data_req.id = slave_id;
	request[counter++] = slave_id;

	mb_master_state.data_req.command = MODBUS_FORCE_MULTIPLE_COILS;
	request[counter++] = MODBUS_FORCE_MULTIPLE_COILS;

	mb_master_state.data_req.register_addr = reg_addr;
	request[counter++] = (uint8_t)(reg_addr >> 8);
	request[counter++] = (uint8_t)(reg_addr);

	if (reg_count > sizeof(mb_master_state.data_req.special_data)) {
		_mb_ms_do_internal_error();
		return;
	}

	uint8_t spec_data_counter = 0;
	mb_master_state.data_req.special_data[spec_data_counter++] = (uint8_t)(reg_count >> 8);
	mb_master_state.data_req.special_data[spec_data_counter++] = (uint8_t)(reg_count);
	request[counter++] = (uint8_t)(reg_count >> 8);
	request[counter++] = (uint8_t)(reg_count);

	uint8_t bytes_count = (reg_count / 8) + (reg_count % 8 > 0 ? 1 : 0);
	mb_master_state.data_req.special_data[spec_data_counter++] = (uint8_t)(bytes_count);
	request[counter++] = (uint8_t)(bytes_count);

	for (uint8_t i = 0; i < reg_count; i++) {
		uint8_t tmp_data = ((data[i] ? 1 : 0) << (i % 8));

		mb_master_state.data_req.special_data[spec_data_counter + i / 8] |= tmp_data;
		request[counter + i / 8] |= tmp_data;
	}
	uint8_t count = reg_count / 8 + (reg_count % 8 ? 1 : 0);
	counter += count;
	spec_data_counter += count;

	uint16_t crc = modbus_crc16(request, counter);
	mb_master_state.data_req.crc = crc;
	request[counter++] = (uint8_t)(crc);
	request[counter++] = (uint8_t)(crc >> 8);

	if (mb_master_state.request_data_sender == NULL) {
		_mb_ms_do_internal_error();
		return;
	}

	mb_master_state.request_data_sender(request, counter);
}

void modbus_master_preset_multiple_registers(uint8_t slave_id, uint16_t reg_addr, const uint16_t* data, uint16_t reg_count)
{
	if (reg_count == 0 || data == NULL) {
		_mb_ms_do_internal_error();
		return;
	}

	uint8_t request[MODBUS_MESSAGE_DATA_SIZE] = { 0 };
	uint16_t counter = 0;

	mb_master_state.data_req.id = slave_id;
	request[counter++] = slave_id;

	mb_master_state.data_req.command = MODBUS_PRESET_MULTIPLE_REGISTERS;
	request[counter++] = MODBUS_PRESET_MULTIPLE_REGISTERS;

	mb_master_state.data_req.register_addr = reg_addr;
	request[counter++] = (uint8_t)(reg_addr >> 8);
	request[counter++] = (uint8_t)(reg_addr);

	if (reg_count * sizeof(uint16_t) > sizeof(mb_master_state.data_req.special_data)) {
		_mb_ms_do_internal_error();
		return;
	}

	uint8_t spec_data_counter = 0;
	mb_master_state.data_req.special_data[spec_data_counter++] = (uint8_t)(reg_count >> 8);
	mb_master_state.data_req.special_data[spec_data_counter++] = (uint8_t)(reg_count);
	request[counter++] = (uint8_t)(reg_count >> 8);
	request[counter++] = (uint8_t)(reg_count);

	uint8_t bytes_count = reg_count * sizeof(uint16_t);
	mb_master_state.data_req.special_data[spec_data_counter++] = (uint8_t)(bytes_count);
	request[counter++] = (uint8_t)(bytes_count);

	for (uint8_t i = 0; i < reg_count; i++) {
		mb_master_state.data_req.special_data[spec_data_counter++] = (uint8_t)(data[i] >> 8);
		mb_master_state.data_req.special_data[spec_data_counter++] = (uint8_t)(data[i]);
		request[counter++] = (uint8_t)(data[i] >> 8);
		request[counter++] = (uint8_t)(data[i]);
	}

	uint16_t crc = modbus_crc16(request, counter);
	mb_master_state.data_req.crc = crc;
	request[counter++] = (uint8_t)(crc);
	request[counter++] = (uint8_t)(crc >> 8);

	if (mb_master_state.request_data_sender == NULL) {
		_mb_ms_do_internal_error();
		return;
	}

	mb_master_state.request_data_sender(request, counter);
}

void _mb_ms_send_simple_message(uint8_t slave_id, uint8_t command, uint16_t reg_addr, uint16_t spec_data)
{
	uint8_t request[MODBUS_MESSAGE_DATA_SIZE] = { 0 };
	uint16_t counter = 0;

	mb_master_state.data_req.id = slave_id;
	request[counter++] = slave_id;

	mb_master_state.data_req.command = command;
	request[counter++] = command;

	mb_master_state.data_req.register_addr = reg_addr;
	request[counter++] = (uint8_t)(reg_addr >> 8);
	request[counter++] = (uint8_t)(reg_addr);

	mb_master_state.data_req.special_data[0] = (uint8_t)(spec_data >> 8);
	mb_master_state.data_req.special_data[1] = (uint8_t)(spec_data);
	request[counter++] = (uint8_t)(spec_data >> 8);
	request[counter++] = (uint8_t)(spec_data);

	uint16_t crc = modbus_crc16(request, counter);
	mb_master_state.data_req.crc = crc;
	request[counter++] = (uint8_t)(crc);
	request[counter++] = (uint8_t)(crc >> 8);

	if (mb_master_state.request_data_sender == NULL) {
		_mb_ms_do_internal_error();
		return;
	}

	mb_master_state.request_data_sender(request, counter);
}


void _mb_ms_response_proccess(void)
{
	if (!_mb_ms_is_recieved_needed_slave_id()) {
		_mb_ms_reset_data();
		return;
	}
	if (mb_master_state.response_packet_handler == NULL) {
		_mb_ms_do_internal_error();
		return;
	}

	modbus_response_t mb_resp_packet = {
		.status = MODBUS_NO_ERROR,
		.slave_id = mb_master_state.data_resp.id,
		.command = mb_master_state.data_resp.command,
		.response = {0}
	};

	/* CHECK ERRORS BEGIN */
	if (!_mb_ms_check_response_command()) {
		mb_master_state.data_resp.command ^= MODBUS_ERROR_COMMAND_CODE;
		mb_resp_packet.status = MODBUS_ERROR_DATA;
	}

	if (!_mb_ms_check_response_command()) {
		mb_resp_packet.status = MODBUS_ERROR_COMMAND;
		goto do_response_packet_handler;
	}

	if (!_mb_ms_check_response_crc()) {
		mb_resp_packet.status = MODBUS_ERROR_CRC;
		goto do_response_packet_handler;
	}
	/* CHECK ERRORS END */

	/* MAKE PACKET DATA BEGIN */
	if (_mb_ms_is_read_discrete_reg_command()) {
		_mb_ms_make_read_discrete_packet(&mb_resp_packet);
	}
	if (_mb_ms_is_read_analog_reg_command()) {
		_mb_ms_make_read_analog_packet(&mb_resp_packet);
	}
	if (_mb_ms_is_write_single_reg_command() || _mb_ms_is_write_multiple_reg_command()) {
		_mb_ms_make_write_packet(&mb_resp_packet);
	}
	/* MAKE PACKET DATA END */

do_response_packet_handler:
	mb_master_state.response_packet_handler(&mb_resp_packet);
	_mb_ms_reset_data();
}

void _mb_ms_reset_data(void)
{
	memset((uint8_t*)&mb_master_state.data_req, 0, sizeof(mb_master_state.data_req));
	memset((uint8_t*)&mb_master_state.data_resp, 0, sizeof(mb_master_state.data_resp));
	memset((uint8_t*)&mb_master_state.response_bytes, 0, sizeof(mb_master_state.response_bytes));

	mb_master_state.response_byte_handler = _mb_ms_fsm_response_slave_id;
	mb_master_state.data_counter = 0;
	mb_master_state.response_bytes_len = 0;
}

void _mb_ms_do_internal_error(void)
{
	if (mb_master_state.internal_error_handler != NULL) {
		mb_master_state.internal_error_handler();
	}
}

void _mb_ms_make_read_discrete_packet(modbus_response_t* packet)
{
	for (uint16_t i = 0; i < mb_master_state.data_resp.data_len; i++) {
		packet->response[i] = mb_master_state.data_resp.data_resp[i];
	}
}

void _mb_ms_make_read_analog_packet(modbus_response_t* packet)
{
	for (uint16_t i = 0; i < mb_master_state.data_resp.data_len; i += 2) {
		packet->response[i / 2] = (mb_master_state.data_resp.data_resp[i] << 8) | (mb_master_state.data_resp.data_resp[i + 1]);
	}
}

void _mb_ms_make_write_packet(modbus_response_t* packet)
{
	packet->response[0] = (mb_master_state.data_resp.data_resp[0] << 8) | (mb_master_state.data_resp.data_resp[1]);
}

void _mb_ms_fsm_response_slave_id(uint8_t byte)
{
	mb_master_state.data_resp.id = byte;
	mb_master_state.data_counter = 0;
	mb_master_state.response_byte_handler = _mb_ms_fsm_response_command;
}

void _mb_ms_fsm_response_command(uint8_t byte)
{
	mb_master_state.data_resp.command = byte;
	mb_master_state.data_counter = 0;
	if (_mb_ms_is_read_command()) {
		mb_master_state.response_byte_handler = _mb_ms_fsm_response_data_len;
	} else if (_mb_ms_is_write_single_reg_command() || _mb_ms_is_write_multiple_reg_command()) {
		mb_master_state.response_byte_handler = _mb_ms_fsm_response_register_addr;
	} else {
		mb_master_state.response_byte_handler = _mb_ms_fsm_response_error;
	}
}

void _mb_ms_fsm_response_data_len(uint8_t byte)
{
	mb_master_state.data_resp.data_len = byte;

	mb_master_state.data_counter = 0;
	mb_master_state.response_byte_handler = _mb_ms_fsm_response_special_data;
}

void _mb_ms_fsm_response_register_addr(uint8_t byte)
{
	mb_master_state.data_resp.register_addr <<= 8;
	mb_master_state.data_resp.register_addr |= byte;
	if (mb_master_state.data_counter == sizeof(mb_master_state.data_resp.register_addr)) {
		mb_master_state.data_counter = 0;
		mb_master_state.response_byte_handler = _mb_ms_fsm_response_special_data;
	}
	if (mb_master_state.data_resp.register_addr >= MODBUS_REGISTER_SIZE) {
		_mb_ms_reset_data();
	}
}

void _mb_ms_fsm_response_special_data(uint8_t byte)
{
	if (!_mb_ms_is_recieved_needed_slave_id()) {
		goto do_count_special_data;
	}

	if (_mb_ms_get_response_bytes_count() > MODBUS_REGISTER_SIZE * sizeof(uint16_t)) {
		_mb_ms_do_internal_error();
		_mb_ms_reset_data();
		return;
	}

	if (mb_master_state.data_counter > sizeof(mb_master_state.data_resp.data_resp)) {
		_mb_ms_do_internal_error();
		_mb_ms_reset_data();
		return;
	}

	mb_master_state.data_resp.data_resp[mb_master_state.data_counter - 1] = byte;


do_count_special_data:
	if (mb_master_state.data_counter == _mb_ms_get_response_bytes_count()) {
		mb_master_state.data_counter = 0;
		mb_master_state.response_byte_handler = _mb_ms_fsm_response_crc;
	}
}

void _mb_ms_fsm_response_error(uint8_t byte)
{
	mb_master_state.data_resp.data_resp[0] = byte;
	mb_master_state.data_counter = 0;
	mb_master_state.response_byte_handler = _mb_ms_fsm_response_crc;
}

void _mb_ms_fsm_response_crc(uint8_t byte)
{
	mb_master_state.data_resp.crc >>= 8;
	mb_master_state.data_resp.crc |= (byte << 8);

	if (mb_master_state.data_counter < sizeof(mb_master_state.data_resp.crc)) {
		return;
	}

	if (!_mb_ms_is_recieved_needed_slave_id()) {
		mb_master_state.data_counter = 0;
		mb_master_state.response_byte_handler = _mb_ms_fsm_response_slave_id;
		goto do_reset_data;
	}

	_mb_ms_response_proccess();

do_reset_data:
	_mb_ms_reset_data();
}

uint16_t _mb_ms_get_response_bytes_count(void)
{
#if MODBUS_ENABLE_FORCE_SINGLE_COIL || MODBUS_ENABLE_PRESET_SINGLE_REGISTER || MODBUS_ENABLE_FORCE_MULTIPLE_COILS || MODBUS_ENABLE_PRESET_MULTIPLE_REGISTERS
	if (!_mb_ms_is_read_command()) {
		return 2;
	}
#endif
#if MODBUS_ENABLE_READ_COIL_STATUS || MODBUS_ENABLE_READ_INPUT_STATUS || MODBUS_ENABLE_READ_HOLDING_REGISTERS || MODBUS_ENABLE_READ_INPUT_REGISTERS
	if (_mb_ms_is_read_command()) {
		return mb_master_state.data_resp.data_len;
	}
#endif
	return 0;
}

bool _mb_ms_is_read_command(void)
{
	return mb_master_state.data_resp.command == MODBUS_READ_COILS || mb_master_state.data_resp.command == MODBUS_READ_HOLDING_REGISTERS || mb_master_state.data_resp.command == MODBUS_READ_INPUT_REGISTERS || mb_master_state.data_resp.command == MODBUS_READ_INPUT_STATUS;
}

bool _mb_ms_is_write_single_reg_command(void)
{
	return mb_master_state.data_resp.command == MODBUS_FORCE_SINGLE_COIL || mb_master_state.data_resp.command == MODBUS_PRESET_SINGLE_REGISTER;
}

bool _mb_ms_is_write_multiple_reg_command(void)
{
	return mb_master_state.data_resp.command == MODBUS_FORCE_MULTIPLE_COILS || mb_master_state.data_resp.command == MODBUS_PRESET_MULTIPLE_REGISTERS;
}

bool _mb_ms_is_read_discrete_reg_command(void)
{
	return mb_master_state.data_resp.command == MODBUS_READ_COILS || mb_master_state.data_resp.command == MODBUS_READ_INPUT_STATUS;
}

bool _mb_ms_is_read_analog_reg_command(void)
{
	return mb_master_state.data_resp.command == MODBUS_READ_HOLDING_REGISTERS || mb_master_state.data_resp.command == MODBUS_READ_INPUT_REGISTERS;
}

bool _mb_ms_is_recieved_needed_slave_id(void)
{
	return mb_master_state.data_req.id == mb_master_state.data_resp.id;
}

bool _mb_ms_check_response_crc(void) {
	return mb_master_state.data_resp.crc == modbus_crc16(mb_master_state.response_bytes, mb_master_state.response_bytes_len - sizeof(uint16_t));
}

bool _mb_ms_check_response_command(void)
{
	return false
#if MODBUS_ENABLE_READ_COIL_STATUS
		|| mb_master_state.data_resp.command == MODBUS_READ_COILS
#endif
#if MODBUS_ENABLE_READ_INPUT_STATUS
		|| mb_master_state.data_resp.command == MODBUS_READ_INPUT_STATUS
#endif
#if MODBUS_ENABLE_READ_HOLDING_REGISTERS
		|| mb_master_state.data_resp.command == MODBUS_READ_HOLDING_REGISTERS
#endif
#if MODBUS_ENABLE_READ_INPUT_REGISTERS
		|| mb_master_state.data_resp.command == MODBUS_READ_INPUT_REGISTERS
#endif
#if MODBUS_ENABLE_FORCE_SINGLE_COIL
		|| mb_master_state.data_resp.command == MODBUS_FORCE_SINGLE_COIL
#endif
#if MODBUS_ENABLE_PRESET_SINGLE_REGISTER
		|| mb_master_state.data_resp.command == MODBUS_PRESET_SINGLE_REGISTER
#endif
#if MODBUS_ENABLE_FORCE_MULTIPLE_COILS
		|| mb_master_state.data_resp.command == MODBUS_FORCE_MULTIPLE_COILS
#endif
#if MODBUS_ENABLE_PRESET_MULTIPLE_REGISTERS
		|| mb_master_state.data_resp.command == MODBUS_PRESET_MULTIPLE_REGISTERS
#endif
		;
}
