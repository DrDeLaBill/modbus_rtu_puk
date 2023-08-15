/*
 *
 * Copyright © 2023 Georgy E. All rights reserved.
 *
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "modbus_rtu_slave.h"
#include "modbus_rtu_master.h"


#if _WIN32 && __GNUC__
#include <Windows.h>
#endif


#define SLAVE_ID 0x01
#define DETAILS  false

// Master
void request_data_sender(uint8_t* data, uint8_t len);
void response_packet_handler(modbus_response_t* packet);
void master_internal_error_handler(void);

// Slave
void response_data_handler(uint8_t* data, uint8_t len);
void slave_internal_error_handler(void);

// Tests
void print_test_name(const char* name, uint16_t counter);
void base_read_tests(void (*read_func) (uint8_t, uint16_t, uint16_t), uint16_t conunter);
void print_error(char* text);
void print_success(char* text);


uint8_t expected_master_result[MODBUS_MESSAGE_DATA_SIZE] = { 0 };
uint16_t  expected_master_result_len = 0;
bool wait_error = false;
bool test_error = false;


int main(void)
{
    modbus_master_set_request_data_sender(&request_data_sender);
    modbus_master_set_response_packet_handler(&response_packet_handler);
    modbus_master_set_internal_error_handler(&master_internal_error_handler);

    modbus_slave_set_slave_id(SLAVE_ID);
    modbus_slave_set_response_data_handler(&response_data_handler);
    modbus_slave_set_internal_error_handler(&slave_internal_error_handler);

    uint16_t counter = 1;
    /* READ OUTPUT COILS BEGIN */
#ifdef __GNUC__
    printf("\nREAD OUTPUT COILS TESTS:\n");
#endif
    base_read_tests(modbus_master_read_coils, counter);
    counter += 5;

    modbus_slave_set_register_value(MODBUS_REGISTER_DISCRETE_OUTPUT_COILS, 0, 1);
    modbus_slave_set_register_value(MODBUS_REGISTER_DISCRETE_OUTPUT_COILS, 2, 2);
    print_test_name("%u: Test read three registers", counter++);
    const uint8_t test_read_coils_06[] = { 0x00, 0x05 };
    memcpy(expected_master_result, test_read_coils_06, sizeof(test_read_coils_06));
    expected_master_result_len = sizeof(test_read_coils_06);
    modbus_master_read_coils(SLAVE_ID, 0, 3);
    /* READ OUTPUT COILS END */



    /* READ INPUT COILS BEGIN */
#ifdef __GNUC__
    printf("\nREAD INPUT COILS TESTS:\n");
#endif
    counter = 1;
    base_read_tests(modbus_master_read_input_status, counter);
    counter += 5;

    modbus_slave_set_register_value(MODBUS_REGISTER_DISCRETE_INPUT_COILS, 0, 2);
    modbus_slave_set_register_value(MODBUS_REGISTER_DISCRETE_INPUT_COILS, 2, 3);
    print_test_name("%u: Test read three registers", counter++);
    const uint8_t test_read_coils_12[] = { 0x00, 0x05 };
    memcpy(expected_master_result, test_read_coils_12, sizeof(test_read_coils_12));
    expected_master_result_len = sizeof(test_read_coils_12);
    modbus_master_read_input_status(SLAVE_ID, 0, 3);
    /* READ INPUT COILS END */



    /* READ INPUT REGISTERS BEGIN */
#ifdef __GNUC__
    printf("\nREAD INPUT REGISTERS TESTS:\n");
#endif
    counter = 1;
    base_read_tests(modbus_master_read_input_registers, counter);
    counter += 5;

    modbus_slave_set_register_value(MODBUS_REGISTER_ANALOG_INPUT_REGISTERS, 0, 4);
    modbus_slave_set_register_value(MODBUS_REGISTER_ANALOG_INPUT_REGISTERS, 2, 5);
    print_test_name("%u: Test read three registers", counter++);
    const uint8_t test_read_coils_18[] = { 0x00, 0x04, 0x00, 0x00, 0x00, 0x05 };
    memcpy(expected_master_result, test_read_coils_18, sizeof(test_read_coils_18));
    expected_master_result_len = sizeof(test_read_coils_18);
    modbus_master_read_input_registers(SLAVE_ID, 0, 3);
    /* READ INPUT REGISTERS END */



    /* READ OUTPUT REGISTERS BEGIN */
#ifdef __GNUC__
    printf("\nREAD OUTPUT REGISTERS TESTS:\n");
#endif
    counter = 1;
    base_read_tests(modbus_master_read_holding_registers, counter);
    counter += 5;

    modbus_slave_set_register_value(MODBUS_REGISTER_ANALOG_OUTPUT_HOLDING_REGISTERS, 0, 6);
    modbus_slave_set_register_value(MODBUS_REGISTER_ANALOG_OUTPUT_HOLDING_REGISTERS, 2, 7);
    print_test_name("%u: Test read three registers", counter++);
    const uint8_t test_read_coils_24[] = { 0x00, 0x06, 0x00, 0x00, 0x00, 0x07 };
    memcpy(expected_master_result, test_read_coils_24, sizeof(test_read_coils_24));
    expected_master_result_len = sizeof(test_read_coils_24);
    modbus_master_read_holding_registers(SLAVE_ID, 0, 3);
    /* READ OUTPUT REGISTERS END */



    /* WRITE SINGLE COIL BEGIN */
#ifdef __GNUC__
    printf("\nWRITE OUTPUT COIL TEST:\n");
#endif
    const uint8_t test_read_coils_25[] = { 0x00, 0x01 };
    memcpy(expected_master_result, test_read_coils_25, sizeof(test_read_coils_25));
    expected_master_result_len = sizeof(test_read_coils_25);
    modbus_master_force_single_coil(SLAVE_ID, 0, true);
    /* WRITE SINGLE COIL END */



    /* WRITE SINGLE REGISTER BEGIN */
#ifdef __GNUC__
    printf("\nWRITE OUTPUT REGISTER TEST:\n");
#endif
    const uint8_t test_read_coils_26[] = {0x01, 0x02 };
    memcpy(expected_master_result, test_read_coils_26, sizeof(test_read_coils_26));
    expected_master_result_len = sizeof(test_read_coils_26);
    modbus_master_preset_single_register(SLAVE_ID, 0, 0x0102);
    /* WRITE SINGLE REGISTER END */



    /* WRITE MULTIPLE COIL BEGIN */
    counter = 1;
#ifdef __GNUC__
    printf("\nWRITE MULTIPLE OUTPUT COIL TEST:\n");
#endif
    print_test_name("%u: Test write four registers", counter++);
    const uint8_t test_read_coils_27[] = { 0x00, 0x04 };
    const bool write_vals_01[] = { false, true, false, true };
    memcpy(expected_master_result, test_read_coils_27, sizeof(test_read_coils_27));
    expected_master_result_len = sizeof(test_read_coils_27);
    modbus_master_force_multiple_coils(SLAVE_ID, 0, write_vals_01, 4);

    print_test_name("%u: Test check mulltiple registers", counter++);
    const uint8_t test_read_coils_28[] = { 0x00, 0x0A };
    memcpy(expected_master_result, test_read_coils_28, sizeof(test_read_coils_28));
    expected_master_result_len = sizeof(test_read_coils_28);
    modbus_master_read_coils(SLAVE_ID, 0, 4);
    /* WRITE MULTIPLE COIL END */



    /* WRITE MULTIPLE REGISTER BEGIN */
    counter = 1;
#ifdef __GNUC__
    printf("\nWRITE MULTIPLE OUTPUT REGISTER TEST:\n");
#endif
    print_test_name("%u: Test write two registers", counter++);
    const uint8_t test_read_coils_29[] = { 0x00, 0x02 };
    const uint16_t write_vals_02[] = { 0x0004, 0x0005 };
    memcpy(expected_master_result, test_read_coils_29, sizeof(test_read_coils_29));
    expected_master_result_len = sizeof(test_read_coils_29);
    modbus_master_preset_multiple_registers(SLAVE_ID, 0, write_vals_02, 2);

    print_test_name("%u: Test check mulltiple registers", counter++);
    const uint8_t test_read_coils_30[] = { 0x00, 0x04, 0x00, 0x05 };
    memcpy(expected_master_result, test_read_coils_30, sizeof(test_read_coils_30));
    expected_master_result_len = sizeof(test_read_coils_30);
    modbus_master_read_holding_registers(SLAVE_ID, 0, 2);
    /* WRITE MULTIPLE COIL END */



    /* ERROR REQUEST BEGIN */
    counter = 1;
    memset(expected_master_result, 0xFF, sizeof(expected_master_result));
    expected_master_result_len = sizeof(expected_master_result);
    wait_error = true;

#ifdef __GNUC__
    printf("\nERRORS TEST:\n");
#endif
    print_test_name("%u: Trash request", counter++);
    const uint8_t trash_01[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B };
    request_data_sender(trash_01, sizeof(trash_01));
    modbus_slave_timeout();

    print_test_name("%u: Slave unexceptable input coil", counter++);
    modbus_master_read_input_status(SLAVE_ID, 0xFFFF, 1);

    print_test_name("%u: Slave input coils out of range", counter++);
    modbus_master_read_input_status(SLAVE_ID, 0, 0xFFFF);

    print_test_name("%u: Slave unexceptable output coil", counter++);
    modbus_master_read_coils(SLAVE_ID, 0xFFFF, 1);

    print_test_name("%u: Slave output coils out of range", counter++);
    modbus_master_read_coils(SLAVE_ID, 0, 0xFFFF);

    print_test_name("%u: Slave unexceptable input registers", counter++);
    modbus_master_read_input_registers(SLAVE_ID, 0xFFFF, 1);

    print_test_name("%u: Slave input registers out of range", counter++);
    modbus_master_read_input_registers(SLAVE_ID, 0, 0xFFFF);

    print_test_name("%u: Slave unexceptable output registers", counter++);
    modbus_master_read_holding_registers(SLAVE_ID, 0xFFFF, 1);

    print_test_name("%u: Slave input registers out of range", counter++);
    modbus_master_read_holding_registers(SLAVE_ID, 0, 0xFFFF);

    print_test_name("%u: Slave write unexceptable single coil", counter++);
    modbus_master_force_single_coil(SLAVE_ID, 0xFFFF, 0x0001);

    print_test_name("%u: Slave write unexceptable single register", counter++);
    modbus_master_preset_single_register(SLAVE_ID, 0xFFFF, 0x0001);

    print_test_name("%u: Slave write out of range multiple coils", counter++);
    const bool error_test_02[] = {true, true, true};
    modbus_master_force_multiple_coils(SLAVE_ID, 0, error_test_02, 0xFFFF);

    print_test_name("%u: Slave unexceptable multiple coils", counter++);
    modbus_master_force_multiple_coils(SLAVE_ID, 0xFFFF, error_test_02, 3);

    print_test_name("%u: Slave write unexceptable values to multiple coils", counter++);
    modbus_master_force_multiple_coils(SLAVE_ID, 0, NULL, 1);

    print_test_name("%u: Slave write out of range multiple coils", counter++);
    modbus_master_force_multiple_coils(SLAVE_ID, 0, error_test_02, 0xFFFF);


    print_test_name("%u: Slave unexceptable multiple registers", counter++);
    const bool error_test_03[] = { 0x001F, 0x002F, 0x003F };
    modbus_master_preset_multiple_registers(SLAVE_ID, 0xFFFF, error_test_03, 3);

    print_test_name("%u: Slave write unexceptable values to multiple registers", counter++);
    modbus_master_preset_multiple_registers(SLAVE_ID, 0, NULL, 1);

    print_test_name("%u: Slave write out of range multiple registers", counter++);
    modbus_master_preset_multiple_registers(SLAVE_ID, 0, error_test_03, 0xFFFF);

    wait_error = false;
    /* ERROR REQUEST END */

    if (test_error) {
        return -1;
    }

    return 0;
}

void base_read_tests(void (*read_func) (uint8_t, uint16_t, uint16_t), uint16_t counter)
{
#ifdef __GNUC__
    print_test_name("%u: Test read first", counter++);
#endif
    const uint8_t test_read_coils_01[] = { 0x00, 0x00 };
    memcpy(expected_master_result, test_read_coils_01, sizeof(test_read_coils_01));
    expected_master_result_len = sizeof(test_read_coils_01);
    read_func(SLAVE_ID, 0, 1);

#ifdef __GNUC__
    print_test_name("%u: Test read last", counter++);
#endif
    const uint8_t test_read_coils_02[] = { 0x00, 0x00 };
    memcpy(expected_master_result, test_read_coils_02, sizeof(test_read_coils_02));
    expected_master_result_len = sizeof(test_read_coils_02);
    read_func(SLAVE_ID, MODBUS_REGISTER_SIZE - 1, 1);

#ifdef __GNUC__
    print_test_name("%u: Test read unavailable", counter++);
#endif
    wait_error = true;
    read_func(SLAVE_ID, MODBUS_REGISTER_SIZE, 1);
    wait_error = false;

#ifdef __GNUC__
    print_test_name("%u: Test read all", counter++);
#endif
    const uint8_t test_read_coils_04[MODBUS_REGISTER_SIZE] = { 0x00, 0x00 };
    memcpy(expected_master_result, test_read_coils_04, sizeof(test_read_coils_04));
    expected_master_result_len = sizeof(test_read_coils_04);
    read_func(SLAVE_ID, 0, MODBUS_REGISTER_SIZE);

#ifdef __GNUC__
    print_test_name("%u: Test read unavailable", counter++);
#endif
    wait_error = true;
    read_func(SLAVE_ID, MODBUS_REGISTER_SIZE / 2, MODBUS_REGISTER_SIZE);
    wait_error = false;
}

void print_test_name(const char* name, uint16_t counter)
{
#ifdef __GNUC__
    printf(name, counter);
#if DETAILS
    printf("\n");
#else
    printf(" ");
#endif
#endif
}

void request_data_sender(uint8_t* data, uint8_t len)
{
#ifdef __GNUC__
#if DETAILS
    printf("SENDED  : ");
    for (int i = 0; i < len; i++) {
        printf("%02x ", data[i]);
    }
    printf("\n");
#endif
#endif
    for (int i = 0; i < len; i++) {
        modbus_slave_recieve_data_byte(data[i]);
    }
}

void response_packet_handler(modbus_response_t* packet)
{
    if (packet->status != MODBUS_NO_ERROR) {
#ifdef __GNUC__
        char error[12] = { 0 };
        snprintf(error, sizeof(error) - 1, "ERROR: %02x", packet->status);
        print_error(error);
#endif
        test_error = (wait_error ? test_error : true);
        return;
    }
#ifdef __GNUC__
#if DETAILS
    printf("EXPECTED: ");
    for (int i = 0; i < expected_master_result_len; i++) {
        printf("%02x ", expected_master_result[i]);
    }
    printf("\nRECIEVED: ");
    for (int i = 0; i < expected_master_result_len / 2 + expected_master_result_len % 2; i++) {
        printf("%02x %02x ", (uint8_t)(packet->response[i]>> 8), (uint8_t)(packet->response[i]));
    }
    printf("\n");
#endif
#endif

    for (uint16_t i = 0; i < expected_master_result_len; i+=2) {
        if (expected_master_result[i] != (uint8_t)(packet->response[i / 2] >> 8) || expected_master_result[i + 1] != (uint8_t)(packet->response[i / 2])) {
            print_error("ERROR");
            test_error = (wait_error ? test_error : true);
            return;
        }
    }

    print_success("SUCCESS");
}

void response_data_handler(uint8_t* data, uint8_t len)
{
    for (int i = 0; i < len; i++) {
        modbus_master_recieve_data_byte(data[i]);
    }
}

void master_internal_error_handler(void)
{
    print_error("MASTER ERROR");
    test_error = (wait_error ? test_error : true);
}

void slave_internal_error_handler(void)
{
    print_error("SLAVE ERROR");
    test_error = (wait_error ? test_error : true);
}


void print_error(char* text)
{
#ifdef __GNUC__
#if _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (wait_error ? FOREGROUND_GREEN : FOREGROUND_RED));
#elif __linux__
    printf((wait_error ? "\x1b[32m" : "\x1b[31m"));
#endif
    printf("%s\n", text);
#if _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
#elif __linux__
    printf((wait_error ? "\x1b[0m" : "\x1b[0m"));
#endif
#endif
}

void print_success(char* text)
{
#ifdef __GNUC__
#if _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (wait_error ? FOREGROUND_RED : FOREGROUND_GREEN));
#elif __linux__
    printf((wait_error ? "\x1b[31m" : "\x1b[32m"));
#endif
    printf("%s\n", text);
#if _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
#elif __linux__
    printf((wait_error ? "\x1b[0m" : "\x1b[0m"));
#endif
#endif
}