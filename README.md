<p align="center">
  <h1 align="center">Modbus C library</h1>
</p>

<p align="right">
  <img alt="Test passing" src="https://github.com/DrDeLaBill/modbus_rtu_puk/workflows/C-Build-Test/badge.svg?branch=main">
</p>

Modbus master-slave RTU library for microcontrollers.

### Modbus settings

You need to create ```modbus_settings.h``` in ```./modbus_rtu_puk/src``` path to configurate library registers.
There is an example of configuration in ```modbus_settings.example.h``` file:
```C
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
```

### Master functions

Sets user request function for sending request data array:

```modbus_master_set_request_data_sender(request_data_sender)```

Sets user function that calls when modbus master has an error in request or response:

```modbus_master_set_internal_error_handler(response_error_handler)```

Sets user function that called when the response receives:

```modbus_master_set_response_packet_handler(response_packet_handler)```

Function that must be called when a new byte received:

```modbus_master_recieve_data_byte(byte)```

Calls when the response waiting time is running out:

```modbus_master_timeout()```

Request functions:
```C
modbus_master_read_coils(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count);
modbus_master_read_input_status(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count);
modbus_master_read_holding_registers(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count);
modbus_master_read_input_registers(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_count);
modbus_master_force_single_coil(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_val);
modbus_master_preset_single_register(uint8_t slave_id, uint16_t reg_addr, uint16_t reg_val);
modbus_master_force_multiple_coils(uint8_t slave_id, uint16_t reg_addr, bool* data, uint16_t reg_count);
modbus_master_preset_multiple_registers(uint8_t slave_id, uint16_t reg_addr, uint16_t* data, uint16_t reg_count);
```

### Master example:
```C
#include <stdio.h>
#include <stdint.h>

#include "modbus_rtu_master.h"


void request_data_sender(uint8_t* data, uint8_t len)
{
    for (int i = 0; i < len; i++) {
        // send_request_byte_function(data[i]);
    }
}

void master_internal_error_handler()
{
    printf("MASTER ERROR\n");
}

void response_packet_handler(modbus_response_t* packet)
{
    /* YOUR CODE */
    printf("Recieved data: ");
    for (int i = 0; i < MODBUS_MESSAGE_DATA_SIZE; i++) {
        printf("%02x %02x   ", (uint8_t)(packet->response[i] >> 8), (uint8_t)(packet->response[i]));
    }
    printf("\n");

    if (packet->status == MODBUS_NO_ERROR) {
        printf("SUCCESS\n");
    } else {
        printf("ERROR\n");
    }
}


int main()
{
    // Setup modbus master
    modbus_master_set_request_data_sender(&request_data_sender);
    modbus_master_set_response_packet_handler(&response_packet_handler);
    modbus_master_set_internal_error_handler(&master_internal_error_handler);

    // Send request
    modbus_master_read_coils(0x01, 0x0000, 0x0000);

    while (/* if new response byte available */) {
        // modbus_master_recieve_data_byte(new byte);
    }

    // response proccess

    return 0;
}
```
### Slave functions

Sets user request function for sending response data array:

```modbus_slave_set_response_data_handler(response_data_handler)```

Sets user function that calls when modbus slave has an error in request or response:

```modbus_slave_set_request_error_handler(request_error_handler)```

Function that must be called when a new byte received:

```modbus_slave_recieve_data_byte(new byte)```

Sets modbus slave id:

```modbus_slave_set_slave_id(new_slave_id)```

Calls when the request waiting time is running out:

```modbus_slave_timeout()```

Returns slave register value:

```uint16_t modbus_slave_get_register_value(register_type_t register_type, uint16_t register_id)```

Sets slave register value: 

```modbus_slave_set_register_value(register_type_t register_type, uint16_t register_id, uint16_t value)```

Values of ```register_type_t```:

```
MODBUS_REGISTER_DISCRETE_OUTPUT_COILS
MODBUS_REGISTER_DISCRETE_INPUT_COILS
MODBUS_REGISTER_ANALOG_INPUT_REGISTERS
MODBUS_REGISTER_ANALOG_OUTPUT_HOLDING_REGISTERS
```

### Slave example:
```C
#include <stdio.h>
#include <stdint.h>

#include "modbus_rtu_slave.h"


void response_data_handler(uint8_t* data, uint8_t len)
{
    for (int i = 0; i < len; i++) {
        // send_response_byte_function(data[i]);
    }
}

void slave_internal_error_handler()
{
    printf("SLAVE ERROR\n");
}

int main()
{
    // Setup modbus slave
    modbus_slave_set_slave_id(0x01);
    modbus_slave_set_response_data_handler(&response_data_handler);
    modbus_slave_set_request_error_handler(&slave_internal_error_handler);

    modbus_slave_set_register_value(MODBUS_REGISTER_ANALOG_OUTPUT_HOLDING_REGISTERS, 0x0000, 0x1234);

    while (1) {
        if (/* if new request byte available */) {
            // modbus_slave_recieve_data_byte(new byte);
        }
    }

    return 0;
}
```

### SDCC test compile

```
cd <project path>/build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-sdcc.cmake -DMODE_SDCC=1 ..
make
```
