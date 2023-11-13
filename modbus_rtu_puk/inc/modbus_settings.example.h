/*
 *
 * Copyright © 2023 Georgy E. All rights reserved.
 *
 */
#ifndef _MODBUS_SETTINGS_EXAMPLE_H_
#define _MODBUS_SETTINGS_EXAMPLE_H_

/*************************** MODBUS REGISTER SETTINGS BEGIN ***************************/

/* Slave registers count */
#define MODBUS_SLAVE_INPUT_COILS_COUNT                  (16)    // MODBUS default: 9999
#define MODBUS_SLAVE_OUTPUT_COILS_COUNT                 (16)    // MODBUS default: 9999
#define MODBUS_SLAVE_INPUT_REGISTERS_COUNT              (16)    // MODBUS default: 9999
#define MODBUS_SLAVE_OUTPUT_HOLDING_REGISTERS_COUNT     (16)    // MODBUS default: 9999

/* Expected registers count (master) */
#define MODBUS_MASTER_INPUT_COILS_COUNT                 (16)    // MODBUS default: 9999
#define MODBUS_MASTER_OUTPUT_COILS_COUNT                (16)    // MODBUS default: 9999
#define MODBUS_MASTER_INPUT_REGISTERS_COUNT             (16)    // MODBUS default: 9999
#define MODBUS_MASTER_OUTPUT_HOLDING_REGISTERS_COUNT    (16)    // MODBUS default: 9999

/**************************** MODBUS REGISTER SETTINGS END ****************************/

#endif